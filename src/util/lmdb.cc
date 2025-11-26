/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <lmdb.h>
#include "./lmdb.h"
#include "./fs.h"
#include "./event.h"
#include "./thread/inl.h"

namespace qk {
	#define Open(...) if (opendbi(dbi)) return __VA_ARGS__
	#define CHECK_RC(code, ...) do { rc = (code); if (rc != MDB_SUCCESS) return __VA_ARGS__; } while(0)
	#define CHECK_BEGIN(flags, ...) \
		Open(__VA_ARGS__); \
		MDB_txn* txn; \
		int rc; \
		CHECK_RC( mdb_txn_begin((MDB_env*)_env, nullptr, flags, &txn), rc)
	#define CHECK_EXE(code, ...) do { rc = (code); if (rc != MDB_SUCCESS) { \
			mdb_txn_abort(txn); \
			return __VA_ARGS__; \
		} \
	} while(0)
	#define CHECK_COMMIT(...) CHECK_RC( mdb_txn_commit(txn),  __VA_ARGS__)

	// ------------------------------------------------

	// All created environments mapped by absolute path
	static Dict<String, LMDB*>* _envInstances = nullptr;
	static Mutex*               _envMutex = nullptr;

	Sp<LMDB> LMDB::Make(cString& path, uint32_t max_dbis, uint32_t map_size) {
		static int init_one_ = ([]() { // one-time init
			_envMutex = new Mutex();
			_envInstances = new Dict<String, LMDB*>();
		}(), 0);

		ScopeLock lock(*_envMutex);
		LMDB* lmdb;
		if (_envInstances->get(path, lmdb)) {
			return lmdb; // existing instance
		} else {
			// create new instance
			lmdb = new LMDB(path, max_dbis, map_size);
			_envInstances->set(path, lmdb); // weak ref in global map
		}
		return lmdb;
	}

	LMDB* LMDB::shared() {
		// Singleton shared LMDB instance
		static LMDB* _shared_lmdb = LMDB::Make(fs_temp(".shared_lmdb"), 64, 512ULL * 1024 * 1024)
				.collapse(); // retain ownership
		return _shared_lmdb;
	}

	struct LMDB::DBI {
		LMDB *lmdb;
		MDB_dbi dbi;
		String name;
	};

	static void onProcessExit(Event<void, int>&e, LMDB* lmdb) {
		lmdb->close(); // auto close env on process exit
	}

	LMDB::LMDB(cString& path, uint32_t max_dbis, uint32_t map_size)
			: _path(path), _env(nullptr), _max_dbis(max_dbis), _map_size(map_size) {
	}

	LMDB::~LMDB() {
		close();
		for (auto it : _dbis) {
			*it.second = { nullptr, 0, "" }; // clear global memory
			delete it.second; // free dbi handles
		}
		ScopeLock lock(*_envMutex);
		_envInstances->erase(_path); // remove from global env map
	}

	bool LMDB::opened() const {
		return _env != nullptr;
	}

	int LMDB::open() {
		if (_env)
			return MDB_SUCCESS;
		ScopeLock lock(_mutex);
		if (_env)
			return MDB_SUCCESS;

		fs_mkdirs_sync(_path);

		int rc;
		MDB_env* env;

		CHECK_RC( mdb_env_create(&env), rc);

		mdb_env_set_maxdbs(env, _max_dbis);
		// setttine map size 1GB
		mdb_env_set_mapsize(env, _map_size);

		rc = mdb_env_open(env, fs_fallback_c(_path), 0, 0664);
		if (rc != MDB_SUCCESS) {
			mdb_env_close(env);
			return rc;
		}

		// auto close env on process exit
		Qk_On(ProcessExit, onProcessExit, this);

		_env = env;
		return MDB_SUCCESS;
	}

	int LMDB::close() {
		int rc = MDB_SUCCESS;
		if (_env) {
			ScopeLock lock(_mutex);
			if (_env) {
				rc = mdb_env_sync((MDB_env*)_env, 1);
				mdb_env_close((MDB_env*)_env);
				for (auto it : _dbis)
					it.second->dbi = 0; // invalidate dbi handle
				_env = nullptr;
			}
			if (!is_process_exit()) { // avoid deadlock on process exit
				Qk_Off(ProcessExit, onProcessExit, this); // remove process exit listener
			}
		}
		return rc;
	}

	LMDB::DBI* LMDB::dbi(cString& name) {
		ScopeLock lock(_mutex);
		DBI* dbi;
		if (_dbis.get(name, dbi)) {
			return dbi; // existing dbi handle
		} else {
			// allocate new dbi handle
			dbi = _dbis.set(name, new DBI{ this, 0, name });
		}
		return dbi;
	}

	int LMDB::opendbi(DBI *dbi) {
		Qk_ASSERT(dbi, "LMDB DBI is null");
		if (dbi->dbi) {
			return dbi->lmdb == this ? MDB_SUCCESS: MDB_BAD_DBI;
		}
		if (dbi->lmdb != this) {
			return MDB_BAD_DBI; // invalid dbi handle
		}
		int rc;
		CHECK_RC(open(), rc); // open env
		MDB_dbi mdb_dbi;
		MDB_txn* txn;
		CHECK_RC( mdb_txn_begin((MDB_env*)_env, nullptr, MDB_CREATE, &txn), rc);
		CHECK_EXE( mdb_dbi_open(txn, dbi->name.c_str(), MDB_CREATE, &mdb_dbi), rc);
		CHECK_COMMIT(rc);
		dbi->dbi = mdb_dbi;
		return MDB_SUCCESS;
	}

	// ----------------------- KV API -------------------------

	int LMDB::get_buf(DBI* dbi, cString& key, Buffer* out) {
		CHECK_BEGIN(MDB_RDONLY, MDB_BAD_DBI);
		MDB_val k{ key.length(), (void*)key.c_str() };
		MDB_val v;
		CHECK_EXE( mdb_get(txn, dbi->dbi, &k, &v), rc);
		*out = WeakBuffer((char*)v.mv_data, (uint32_t)v.mv_size).copy(); // make a copy
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

	int LMDB::set_buf(DBI* dbi, cString& key, cArray<char>& val) {
		CHECK_BEGIN(0, MDB_BAD_DBI);
		MDB_val k{ key.length(), (void*)key.c_str() };
		MDB_val v{ val.length(), (void*)*val };
		CHECK_EXE( mdb_put(txn, dbi->dbi, &k, &v, 0), rc);
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

	int LMDB::get(DBI* dbi, cString& key, String* out) {
		CHECK_BEGIN(MDB_RDONLY, MDB_BAD_DBI);
		MDB_val k{ key.length(), (void*)key.c_str() };
		MDB_val v;
		CHECK_EXE( mdb_get(txn, dbi->dbi, &k, &v), rc);
		*out = String((char*)v.mv_data, (uint32_t)v.mv_size);
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

	int LMDB::set(DBI* dbi, cString& key, cString& val) {
		CHECK_BEGIN(0, MDB_BAD_DBI);
		MDB_val k{ key.length(), (void*)key.c_str() };
		MDB_val v{ val.length(), (void*)val.c_str() };
		CHECK_EXE( mdb_put(txn, dbi->dbi, &k, &v, 0), rc);
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

	int LMDB::remove(DBI* dbi, cString& key) {
		CHECK_BEGIN(0, MDB_BAD_DBI);
		MDB_val k{ key.length(), (void*)key.c_str() };
		CHECK_EXE( mdb_del(txn, dbi->dbi, &k, nullptr), rc);
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

	bool LMDB::has(DBI* dbi, cString& key) {
		CHECK_BEGIN(MDB_RDONLY, false);
		MDB_val k{ key.length(), (void*)key.c_str() };
		MDB_val v;
		rc = mdb_get(txn, dbi->dbi, &k, &v);
		mdb_txn_commit(txn);
		return rc == MDB_SUCCESS;
	}

	bool LMDB::fuzz_exists(DBI* dbi, cString& prefix) {
		CHECK_BEGIN(0, false);
		MDB_cursor* cur;
		CHECK_EXE( mdb_cursor_open(txn, dbi->dbi, &cur), false);
		// search first >= prefix
		String end(prefix + "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF");
		MDB_val v;
		MDB_val k{ prefix.length(), (void*)prefix.c_str() };
		MDB_val ek{ end.length(), (void*)end.c_str() };
		rc = mdb_cursor_get(cur, &k, &v, MDB_SET_RANGE);
		if (rc == MDB_SUCCESS) {
			if (mdb_cmp(txn, dbi->dbi, &k, &ek) > 0)
				rc = -1; // not found
		}
		mdb_cursor_close(cur);
		mdb_txn_commit(txn);
		return rc == MDB_SUCCESS;
	}

	int LMDB::scan_prefix(DBI* dbi, cString& prefix, Array<Pair>* out, uint32_t limit) {
		// scan range with prefix
		return scan_range(dbi, prefix, prefix + "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", out, limit);
	}

	int LMDB::scan_range(DBI* dbi, cString& start, cString& end, Array<Pair>* out, uint32_t limit) {
		CHECK_BEGIN(MDB_RDONLY, MDB_BAD_DBI);
		MDB_cursor* cur;
		CHECK_EXE( mdb_cursor_open(txn, dbi->dbi, &cur), rc);
		MDB_val v;
		MDB_val k{ start.length(), (void*)start.c_str() };
		MDB_val ek{ end.length(), (void*)end.c_str() };

		if (limit == 0)
			limit = 0xffffffff; // no limit if 0
		// seek to start
		rc = mdb_cursor_get(cur, &k, &v, MDB_SET_RANGE);

		while (rc == MDB_SUCCESS && out->length() < limit) {
			if (mdb_cmp(txn, dbi->dbi, &k, &ek) > 0)
				break; // exceed end key
			out->push({
				String((char*)k.mv_data, (uint32_t)k.mv_size),
				String((char*)v.mv_data, (uint32_t)v.mv_size)
			});
			rc = mdb_cursor_get(cur, &k, &v, MDB_NEXT);
		}
		mdb_cursor_close(cur);
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

	int LMDB::remove_prefix(DBI* dbi, cString& prefix) {
		CHECK_BEGIN(0, MDB_BAD_DBI);
		MDB_cursor* cur;
		CHECK_EXE( mdb_cursor_open(txn, dbi->dbi, &cur), rc);
		String end(prefix + "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF");
		MDB_val v;
		MDB_val k{ prefix.length(), (void*)prefix.c_str() };
		MDB_val ek{ end.length(), (void*)end.c_str() };
		rc = mdb_cursor_get(cur, &k, &v, MDB_SET_RANGE);

		while (rc == MDB_SUCCESS) {
			String s((char*)k.mv_data, (uint32_t)k.mv_size);
			Qk_DLog(s);
			if (mdb_cmp(txn, dbi->dbi, &k, &ek) > 0)
				break; // exceed end key
			// erase current key
			mdb_cursor_del(cur, 0);
			rc = mdb_cursor_get(cur, &k, &v, MDB_NEXT);
		}
		mdb_cursor_close(cur);
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

	int LMDB::clear(DBI* dbi) {
		CHECK_BEGIN(0, MDB_BAD_DBI);
		CHECK_EXE( mdb_drop(txn, dbi->dbi, 0), rc);
		CHECK_COMMIT(rc);
		return MDB_SUCCESS;
	}

} // namespace qk
