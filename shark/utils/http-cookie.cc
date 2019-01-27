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

#include "http-cookie.h"
#include "fs.h"
#include <sqlite3.h>

XX_NS(shark)

#define _db _http_cookie_db

static Mutex mutex;
static sqlite3* _http_cookie_db = nullptr;
static sqlite3_stmt* _http_cookie_get = nullptr;
static sqlite3_stmt* _http_cookie_get_all = nullptr;
static sqlite3_stmt* _http_cookie_has = nullptr;
static sqlite3_stmt* _http_cookie_add = nullptr;
static sqlite3_stmt* _http_cookie_set = nullptr;
static sqlite3_stmt* _http_cookie_del = nullptr;
static sqlite3_stmt* _http_cookie_del_all = nullptr;
static sqlite3_stmt* _http_cookie_clear = nullptr;
static int64         _http_cookie_date = 0;
static int           _http_cookie_initializ_ok = 0;
static const String DEFAULT_PATH('/');
static const String SET_PATH_START('^');
static const String PERIOD('.');
static const String DIV('/');
static const String EXPIRES("expires");
static const String MAX_AGE("max-age");
static const String PATH("path");
static const String DOMAIN_STR("domain");
static const String SECURE("secure");

#define check(c) if ( !initializ_check(c) ) return

#if DEBUG
static void assert_sqlite3_func(int c) {
	if ( c == SQLITE_ERROR ) {
		LOG(sqlite3_errmsg(_db));
		XX_ASSERT(0);
	}
}
# define assert_sqlite3(c) assert_sqlite3_func(c)
#else
# define assert_sqlite3(c) ((void)0)
#endif

static void http_cookie_close() {
	sqlite3* __db = _db;
	_db = nullptr;
	if ( _http_cookie_get ) sqlite3_finalize(_http_cookie_get); 
	_http_cookie_get = nullptr;
	if ( _http_cookie_get_all ) sqlite3_finalize(_http_cookie_get_all); 
	_http_cookie_get_all = nullptr;
	if ( _http_cookie_has ) sqlite3_finalize(_http_cookie_has); 
	_http_cookie_has = nullptr;
	if ( _http_cookie_add ) sqlite3_finalize(_http_cookie_add); 
	_http_cookie_add = nullptr;
	if ( _http_cookie_set ) sqlite3_finalize(_http_cookie_set); 
	_http_cookie_set = nullptr;
	if ( _http_cookie_del ) sqlite3_finalize(_http_cookie_del); 
	_http_cookie_del = nullptr;
	if ( _http_cookie_del_all ) sqlite3_finalize(_http_cookie_del_all); 
	_http_cookie_del_all = nullptr;
	if ( _http_cookie_clear ) sqlite3_finalize(_http_cookie_clear); 
	_http_cookie_clear = nullptr;
	if ( __db ) sqlite3_close(__db);
}

static bool initializ_check(int c) {
	if ( c == SQLITE_ERROR ) {
		LOG(sqlite3_errmsg(_db));
		http_cookie_close();
		XX_FATAL("Cannot initializ http cookie sqlite database!");
		return 0;
	}
	return 1;
}

static int get_domain_level(cString& domain) {
	int i = -1;
	int count = -1;
	do {
		i = domain.index_of(PERIOD, i + 1);
		count++;
	} while ( i != -1 );
	
	return count;
}

static int get_path_level(cString& path) {
	if ( path.length() == 1 ) {
		return 0;
	}
	int i = -1;
	int count = -1;
	do {
		i = path.index_of(DIV, i + 1);
		count++;
	} while ( i != -1 );
	
	return count;
}

static void http_cookie_initialize() {
	if ( _http_cookie_initializ_ok++ == 0 ) {
		sqlite3_initialize();
		
		_http_cookie_date = sys::time();
		
		int r = sqlite3_open(Path::fallback_c(Path::temp(".cookie.db")), &_db);
		
		if ( r == SQLITE_OK ) {
			
			static char* errmsg = nullptr;
			
			r = sqlite3_exec(_db,
											 "create table if not exists Cookie("
											 "  domain  Text, "
											 "  path    Text, "
											 "  name    Text, "
											 "  value   Text, "
											 "  expires INTEGER, "
											 "  date    INTEGER, "
											 "  secure  INTEGER, "
											 "  domain_level INTEGER, "
											 "  path_level INTEGER, "
											 "  ext     Text"
											 ")"
											 , nullptr, nullptr, &errmsg);
			check(r);
			
			cchar* sql_get =
			"select value from Cookie "
			"where name=? "
			"and instr(?,domain)!=0 "
			"and instr(?,path)=1 "
			"and (expires>? or (expires=-1 and date=?)) "
			"and (secure=0 or ?=1) "
			"order by domain_level desc, path_level desc";
			
			cchar* sql_get_all =
			"select * from (select name,value from Cookie "
			"where instr(?,domain)!=0 "
			"and instr(?,path)=1 "
			"and (expires>? or (expires=-1 and date=?)) "
			"and (secure=0 or ?=1) "
			"order by domain_level, path_level) group by name";
			
			cchar* sql_has = "select value from Cookie where domain=? and path=? and name=?";
			
			cchar* sql_add =
			"insert into Cookie(domain,path,name,value,expires,date,secure,domain_level,path_level,ext) "
			"values(?1,?2,?3,?4,?5,?6,?7,?8,?9,'')";
			
			cchar* sql_set = "update Cookie set value=?4,expires=?5,date=?6,secure=?7 where domain=?1 and path=?2 and name=?3";
			cchar* sql_del = "delete from Cookie where domain=? and path=? and name=?";
			cchar* sql_del_all = "delete from Cookie where domain=?";
			cchar* sql_clear = "delete from Cookie";
			
			r = sqlite3_prepare(_db, sql_get, int(strlen(sql_get)), &_http_cookie_get, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_get_all, int(strlen(sql_get_all)), &_http_cookie_get_all, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_has, int(strlen(sql_has)), &_http_cookie_has, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_add, int(strlen(sql_add)), &_http_cookie_add, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_set, int(strlen(sql_set)), &_http_cookie_set, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_del, int(strlen(sql_del)), &_http_cookie_del, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_del_all, int(strlen(sql_del_all)), &_http_cookie_del_all, nullptr); check(r);
			r = sqlite3_prepare(_db, sql_clear, int(strlen(sql_clear)), &_http_cookie_clear, nullptr); check(r);
			
			atexit(http_cookie_close);
		} else {
			_db = nullptr;
			XX_ERR(sqlite3_errstr(r));
		}
	}
}

static String get_storage_domain_value(cString& domain) {
	String str = (domain[0] == '.' ? domain.substr(1) : domain);
	if ( domain[domain.length() - 1] != '$' ) {
		str.push("$", 1);
	}
	return str;
}

String http_cookie_get(cString& domain, cString& name, cString& path, bool secure) {
	ScopeLock scope(mutex);
	http_cookie_initialize();
	String result;
	if ( _db ) {
		String domain_ = get_storage_domain_value(domain);
		String path_ = path.is_empty() ? DEFAULT_PATH : path;
		
		int r;
		
		r = sqlite3_bind_text(_http_cookie_get, 1, *name, name.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_get, 2, *domain_, domain_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_get, 3, *path_, path_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_get, 4, sys::time()); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_get, 5, _http_cookie_date); assert_sqlite3(r);
		r = sqlite3_bind_int(_http_cookie_get, 6, secure); assert_sqlite3(r);
		r = sqlite3_step(_http_cookie_get); assert_sqlite3(r);
		if ( r == SQLITE_ROW ) {
			result = (cchar*)sqlite3_column_text(_http_cookie_get, 0);
		}
		r = sqlite3_reset(_http_cookie_get); assert_sqlite3(r);
	}
	return result;
}

String http_cookie_get_all_string(cString& domain, cString& path, bool secure) {
	ScopeLock scope(mutex);
	http_cookie_initialize();
	Array<String> result;
	if ( _db ) {
		String domain_ = get_storage_domain_value(domain);
		String path_ = path.is_empty() ? DEFAULT_PATH : path;
		
		int r;
		
		r = sqlite3_bind_text(_http_cookie_get_all, 1, *domain_, domain_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_get_all, 2, *path_, path_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_get_all, 3, sys::time()); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_get_all, 4, _http_cookie_date); assert_sqlite3(r);
		r = sqlite3_bind_int(_http_cookie_get_all, 5, secure); assert_sqlite3(r);
		
		while ( sqlite3_step(_http_cookie_get_all) == SQLITE_ROW ) {
			String str;
			str.push((cchar*)sqlite3_column_text(_http_cookie_get_all, 0));
			str.push("=", 1);
			str.push((cchar*)sqlite3_column_text(_http_cookie_get_all, 1));
			result.push(str);
		}
		r = sqlite3_reset(_http_cookie_get_all); assert_sqlite3(r);
	}
	return result.join( "; " );
}

Map<String, String> http_cookie_get_all(cString& domain, cString& path, bool secure) {
	ScopeLock scope(mutex);
	http_cookie_initialize();
	Map<String, String> result;
	if ( _db ) {
		String domain_ = get_storage_domain_value(domain);
		String path_ = path.is_empty() ? DEFAULT_PATH : path;
		
		int r;
		
		r = sqlite3_bind_text(_http_cookie_get_all, 1, *domain_, domain_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_get_all, 2, *path_, path_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_get_all, 3, sys::time()); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_get_all, 4, _http_cookie_date); assert_sqlite3(r);
		r = sqlite3_bind_int(_http_cookie_get_all, 5, secure); assert_sqlite3(r);
		
		while ( sqlite3_step(_http_cookie_get_all) == SQLITE_ROW ) {
			result.set((cchar*)sqlite3_column_text(_http_cookie_get_all, 0),
								 (cchar*)sqlite3_column_text(_http_cookie_get_all, 1));
		}
		r = sqlite3_reset(_http_cookie_get_all); assert_sqlite3(r);
	}
	return result;
}

static void http_cookie_set2(String domain,
														 cString& name,
														 cString& value, int64 expires, cString& path, bool secure) {
	int r;
	
	r = sqlite3_bind_text(_http_cookie_has, 1, *domain, domain.length(), nullptr); assert_sqlite3(r);
	r = sqlite3_bind_text(_http_cookie_has, 2, *path, path.length(), nullptr); assert_sqlite3(r);
	r = sqlite3_bind_text(_http_cookie_has, 3, *name, name.length(), nullptr); assert_sqlite3(r);
	
	r = sqlite3_step(_http_cookie_has); assert_sqlite3(r); sqlite3_reset(_http_cookie_has);
	
	if ( r == SQLITE_ROW ) { // update
		r = sqlite3_bind_text(_http_cookie_set, 1, *domain, domain.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_set, 2, *path, path.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_set, 3, *name, name.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_set, 4, *value, value.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_set, 5, expires); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_set, 6, _http_cookie_date); assert_sqlite3(r);
		r = sqlite3_bind_int(_http_cookie_set, 7, secure); assert_sqlite3(r);
		r = sqlite3_step(_http_cookie_set); assert_sqlite3(r);
		r = sqlite3_reset(_http_cookie_set); assert_sqlite3(r);
	} else { // insert
		r = sqlite3_bind_text(_http_cookie_add, 1, *domain, domain.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_add, 2, *path, path.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_add, 3, *name, name.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_add, 4, *value, value.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_add, 5, expires); assert_sqlite3(r);
		r = sqlite3_bind_int64(_http_cookie_add, 6, _http_cookie_date); assert_sqlite3(r);
		r = sqlite3_bind_int(_http_cookie_add, 7, secure); assert_sqlite3(r);
		r = sqlite3_bind_int(_http_cookie_add, 8, get_domain_level(domain)); assert_sqlite3(r);
		r = sqlite3_bind_int(_http_cookie_add, 9, get_path_level(path)); assert_sqlite3(r);
		r = sqlite3_step(_http_cookie_add); assert_sqlite3(r);
		r = sqlite3_reset(_http_cookie_add); assert_sqlite3(r);
	}
}

void http_cookie_set_with_expression(cString& domain, cString& expression) {
	ScopeLock scope(mutex);
	//Set-Cookie: BAIDU_WISE_UID=bd_1491295526_455; expires=Thu, 04-Apr-2019 08:45:26 GMT; path=/; domain=baidu.com
	//Set-Cookie: BAIDUID=C9DD3739AD81A91137099489A6DA4C2F:FG=1; expires=Wed, 04-Apr-18 08:45:27 GMT; max-age=31536000; path=/; domain=.baidu.com; version=1
	http_cookie_initialize();
	if ( _db ) {
		String name, value;
		String domain_ = get_storage_domain_value(domain);
		String path_ = DEFAULT_PATH;
		
		Map<String, String> options;
		for ( auto& i : expression.split("; ") ) {
			int j = i.value().index_of('=');
			if ( j != -1 ) {
				if ( name.is_empty() ) {
					name = i.value().substr(0, j);
					value = i.value().substr(j + 1);
				} else {
					options.set(i.value().substr(0, j).to_lower_case(), i.value().substr(j + 1));
				}
			}
		}
		
		if ( name.is_empty() ) {
			return;
		}
		
		int64 expires = -1;
		
		auto end = options.end();
		auto it = options.find(DOMAIN_STR);
		
		if ( it != end ) {
			String new_domain = get_storage_domain_value(it.value());
			if ( domain_.index_of(new_domain) == -1 ) {
				return; // Illegal operation
			} else {
				domain_ = new_domain;
			}
		}
		
		it = options.find(PATH);
		if ( it != end ) {
			path_ = it.value();
		}
		
		it = options.find(MAX_AGE);
		if ( it != end ) {
			expires = it.value().to_int64() * 1e6 + sys::time();
		} else {
			it = options.find(EXPIRES);
			if ( it != end ) {
				int64 time = parse_time(it.value());
				if ( time > 0 ) {
					expires = time;
				}
			}
		}
		
		bool secure = options.has(SECURE);
		
		http_cookie_set2(domain_, name, value, expires, path_, secure);
	}
}

void http_cookie_set(cString& domain,
										 cString& name,
										 cString& value, int64 expires, cString& path, bool secure) {
	ScopeLock scope(mutex);
	http_cookie_initialize();
	if ( _db ) {
		String domain_ = get_storage_domain_value(domain);
		String path_ = path.is_empty() ? DEFAULT_PATH : path;
		http_cookie_set2(get_storage_domain_value(domain), name, value, expires, path_, secure);
	}
}

void http_cookie_delete(cString& domain, cString& name, cString& path) {
	ScopeLock scope(mutex);
	http_cookie_initialize();
	if ( _db ) {
		String domain_ = get_storage_domain_value(domain);
		String path_ = path.is_empty() ? DEFAULT_PATH : path;
		int r;
		r = sqlite3_bind_text(_http_cookie_del, 1, *domain_, domain_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_del, 2, *path_, path_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_bind_text(_http_cookie_del, 3, *name, name.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_step(_http_cookie_del); assert_sqlite3(r);
		r = sqlite3_reset(_http_cookie_del); assert_sqlite3(r);
	}
}

void http_cookie_delete_all(cString& domain) {
	ScopeLock scope(mutex);
	http_cookie_initialize();
	if ( _db ) {
		String domain_ = get_storage_domain_value(domain);
		int r;
		r = sqlite3_bind_text(_http_cookie_del_all, 1, *domain_, domain_.length(), nullptr); assert_sqlite3(r);
		r = sqlite3_step(_http_cookie_del_all); assert_sqlite3(r);
		r = sqlite3_reset(_http_cookie_del_all); assert_sqlite3(r);
	}
}

void http_cookie_clear() {
	ScopeLock scope(mutex);
	http_cookie_initialize();
	if ( _db ) {
		int r;
		r = sqlite3_step(_http_cookie_clear); assert_sqlite3(r);
		r = sqlite3_reset(_http_cookie_clear); assert_sqlite3(r);
	}
}

XX_END
