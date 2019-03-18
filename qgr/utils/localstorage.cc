/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "qgr/utils/localstorage.h"
#include "qgr/utils/fs.h"
#include <sqlite3.h>

XX_NS(qgr)

#define _db _localstorage_db

static Mutex mutex;
static sqlite3* _localstorage_db = nullptr;
static sqlite3_stmt* _localstorage_get = nullptr;
static sqlite3_stmt* _localstorage_set = nullptr;
static sqlite3_stmt* _localstorage_del = nullptr;
static sqlite3_stmt* _localstorage_clear = nullptr;
static int           _localstorage_initializ_ok = 0;

#define check(c) if ( initializ_check(c) ) return

#if DEBUG
static void assert_sqlite3_func(int c) {
	if ( c == SQLITE_ERROR ) {
		XX_ERR(sqlite3_errmsg(_db));
		XX_ASSERT(0);
	}
}
# define assert_sqlite3(c) assert_sqlite3_func(c)
#else
# define assert_sqlite3(c) ((void)0)
#endif

static void localstorage_close() {
	sqlite3* __db = _db;
	_db = nullptr;
	
	sqlite3_exec(__db, "commit;", 0, 0, 0);
	if ( _localstorage_get ) sqlite3_finalize(_localstorage_get); 
	_localstorage_get = nullptr;
	if ( _localstorage_set ) sqlite3_finalize(_localstorage_set); 
	_localstorage_set = nullptr;
	if ( _localstorage_del ) sqlite3_finalize(_localstorage_del); 
	_localstorage_del = nullptr;
	if ( _localstorage_clear ) sqlite3_finalize(_localstorage_clear); 
	_localstorage_clear = nullptr;
	if ( __db ) sqlite3_close(__db);
}

static bool initializ_check(int c) {
	if ( c == SQLITE_ERROR ) {
		XX_ERR(sqlite3_errmsg(_db));
		localstorage_close();
		return 1;
	}
	return 0;
}

static void localstorage_initialize() {
	if ( _localstorage_initializ_ok++ == 0 ) {
		sqlite3_initialize();
		
		int r = sqlite3_open(Path::fallback_c(Path::temp(".localstorage.db")), &_db);
		
		if ( r == SQLITE_OK ) {
			
			static char* errmsg = nullptr;
			
			r = sqlite3_exec(_db,
											 "create table if not exists LocalStorage("
											 "  id      INT PRIMARY KEY, "
											 "  name    Text, "
											 "  value   Text"
											 ")"
											 , nullptr, nullptr, &errmsg);
			check(r);
			
			cchar* sql_get = "select value from LocalStorage where id=?";
			cchar* sql_set = "replace into LocalStorage values(?1,?2,?3)";
			cchar* sql_del = "delete from LocalStorage where id=?";
			cchar* sql_clear = "delete from LocalStorage";
			
			r = sqlite3_prepare(_db, sql_get, int(strlen(sql_get)), &_localstorage_get, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_set, int(strlen(sql_set)), &_localstorage_set, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_del, int(strlen(sql_del)), &_localstorage_del, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_clear, int(strlen(sql_clear)), &_localstorage_clear, nullptr); check(r);
			
			// r = sqlite3_exec(_db, "begin;", 0, 0, 0); check(r);
			// r = sqlite3_exec(_db, "PRAGMA synchronous = OFF; ", 0, 0, 0); check(r);
			
			atexit(localstorage_close);
		} else {
			_db = nullptr;
			XX_ERR(sqlite3_errstr(r));
		}
	}
}

String localstorage_get(cString& name) {
	ScopeLock scope(mutex);
	localstorage_initialize();
	String result;
	if ( _db ) {
		uint id = name.hash_code();
		int r = sqlite3_bind_int(_localstorage_get, 1, id); assert_sqlite3(r);
		if ( sqlite3_step(_localstorage_get) == SQLITE_ROW ) {
			result = (cchar*)sqlite3_column_text(_localstorage_get, 0);
		}
		r = sqlite3_reset(_localstorage_get); assert_sqlite3(r);
	}
	return result;
}

void localstorage_set(cString& name, cString& value) {
	ScopeLock scope(mutex);
	localstorage_initialize();
	if ( _db ) {
		uint id = name.hash_code();
		int r;
		r = sqlite3_bind_int(_localstorage_set, 1, id); assert_sqlite3(r);
		r = sqlite3_bind_text(_localstorage_set, 2, *name, name.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_localstorage_set, 3, *value, value.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_step(_localstorage_set); assert_sqlite3(r);
		r = sqlite3_reset(_localstorage_set); assert_sqlite3(r);
	}
}

void localstorage_delete(cString& name) {
	ScopeLock scope(mutex);
	localstorage_initialize();
	if ( _db ) {
		uint id = name.hash_code();
		int r;
		r = sqlite3_bind_int(_localstorage_del, 1, id); assert_sqlite3(r);
		r = sqlite3_step(_localstorage_del); assert_sqlite3(r);
		r = sqlite3_reset(_localstorage_del); assert_sqlite3(r);
	}
}

void localstorage_clear() {
	ScopeLock scope(mutex);
	localstorage_initialize();
	if ( _db ) {
		int r;
		r = sqlite3_step(_localstorage_clear); assert_sqlite3(r);
		r = sqlite3_reset(_localstorage_clear); assert_sqlite3(r);
	}
}

void localstorage_transaction(cCb& cb) {
	{
		ScopeLock scope(mutex);
		localstorage_initialize();
	}
	if ( _db ) {
		int r;
		{
			ScopeLock scope(mutex);
			r = sqlite3_exec(_db, "begin;", 0, 0, 0); check(r);
		}
		sync_callback(cb);
		{
			ScopeLock scope(mutex);
			r = sqlite3_exec(_db, "commit;", 0, 0, 0); check(r);
		}
	}
}

XX_END
