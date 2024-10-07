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

#include <bplus.h>
#include "./storage.h"
#include "./fs.h"
#include "./event.h"

namespace qk {

	#define _db _storage_db
	#define assert_r(c) Qk_Assert_Eq(c, BP_OK)
	#define OPEN(...) ScopeLock lock(_mutex); storage_open(); if (!_db) return __VA_ARGS__

	static Mutex    _mutex;
	static bp_db_t *_storage_db = nullptr;
	static int      _initialize = 0;

	static String get_db_filename() {
		return fs_temp(".storage.bp");
	}

	static void storage_close() {
		bp_db_t* db = _storage_db;
		_db = nullptr;
		if (db) {
			bp_fsync(db);
			bp_close(db);
		}
	}

	static void storage_open() {
		if ( _db == nullptr ) {
			if ( bp_open(&_db, fs_fallback_c(get_db_filename())) == BP_OK ) {
				if (_initialize++ == 0)
					Qk_On(ProcessExit, [](auto e) { storage_close(); });
			}
		}
	}

	// --------------------------------------------------------------------------------------

	String storage_get(cString& name) {
		String result;
		OPEN(result);
		bp_key_t key = { name.length(), (Char*)name.c_str() };
		bp_key_t val;
		if (bp_get(_db, &key, &val) == BP_OK) {
			result = Buffer(val.value, (uint32_t)val.length).collapseString();
		}
		return result;
	}

	void storage_set(cString& name, cString& value) {
		OPEN();
		bp_key_t   key = { name.length(), (Char*)name.c_str() };
		bp_value_t val = { value.length(), (Char*)value.c_str() };
		assert_r(bp_set(_db, &key, &val));
	}

	void storage_remove(cString& name) {
		OPEN();
		bp_key_t key = { name.length(), (Char*)name.c_str() };
		assert_r(bp_remove(_db, &key));
	}

	void storage_clear() {
		ScopeLock lock(_mutex);
		if ( !_db ) {
			auto f = get_db_filename();
			if (fs_is_file_sync(f)) {
				fs_unlink_sync(f);
			}
		} else {
			storage_close();
			fs_unlink_sync(get_db_filename());
		}
	}

}
