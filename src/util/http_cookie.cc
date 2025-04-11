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
#include "./http.h"
#include "./fs.h"

namespace qk {

	#define _db _http_cookie_db
	#define assert_r(c) Qk_ASSERT_EQ(c, BP_OK)
	#define OPEN(...) ScopeLock lock(_mutex); http_cookie_open(); if (!_db) return __VA_ARGS__

	static Mutex   _mutex;
	static bp_db_t*_http_cookie_db = nullptr;
	static int     _initialize = 0;
	static int64_t _tempFlag = 0;
	static cString EXPIRES("expires");
	static cString MAX_AGE("max-age");
	static cString PATH("path");
	static cString DOMAIN_STR("domain");
	static cString SECURE("secure");

	int bp_get_str(bp_db_t *tree, const bp_key_t* key, String *value);

	static String get_db_filename() {
		return fs_temp(".cookie.bp");
	}

	static int bp__fuzz_compare_cb(void *arg, const bp_key_t *a, const bp_key_t *b) {
		auto len = (uint32_t)Qk_Min(a->length, b->length);
		for (uint32_t i = 0; i < len; i++) {
			if (a->value[i] != b->value[i])
				return a->value[i] - b->value[i];// ? 1 : -1;
		}
		return 0;
	}

	static int bp__default_compare_cb(void *arg, const bp_key_t *a, const bp_key_t *b) {
		int r = bp__fuzz_compare_cb(arg, a, b);
		if (r == 0) {
			return int(a->length - b->length);
		}
		return r;
	}

	static void http_cookie_close() {
		bp_db_t* db = _http_cookie_db;
		_db = nullptr;
		if ( db ) {
			bp_fsync(db);
			bp_close(db);
		}
	}

	static void http_cookie_open() {
		if ( _db == nullptr ) {
			if ( bp_open(&_db, fs_fallback_c(get_db_filename())) == BP_OK ) {
				bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);
				if (_initialize++ == 0) {
					_tempFlag = time_monotonic();
					Qk_On(ProcessExit, [](auto e) { http_cookie_close(); });
				}
			}
		}
	}

	static String get_indexed_key_prefix(bool secure, cString& domain, cString& path) {
		String r(secure ? "1/": "0/");
		auto arr = domain.split('.');
		for (int i = Qk_Minus(arr.length(), 1); i > -1; i--) {
			if (!arr[i].isEmpty()) {
				r.append(arr[i]);
				r.append('/');
			}
		}
		if (!path.isEmpty()) {
			if (path[0] == '/') {
				r.append(path.c_str() + 1, path.length()-1);
			} else {
				r.append(path);
			}
			if (r[r.length()-1] != '/') {
				r.append('/');
			}
		}
		return  r;
	}

	static String get_indexed_key(bool secure, cString& domain, cString& path, cString& name) {
		return get_indexed_key_prefix(secure, domain, path).append('@').append(name);
	}

	static bool http_cookie_check_expires(cString &str, String *value) {
		auto idx = str.indexOf("; ");
		if (idx != -1) {
			auto expires = str.substring(0, idx).toNumber<int64_t>();
			auto idx2 = str.indexOf("; ", idx + 2);
			if (idx2 != -1) {
				auto tempFlag = str.substring(idx + 2, idx2).toNumber<int64_t>();
				if ((expires == -1 && tempFlag == _tempFlag) || expires > time_micro()) {
					*value = str.substring(idx2 + 2);
					return true;
				}
			}
		}
		return false;
	}

	static void http_cookie_set_value_and_expires(cString& key, cString& value, int64_t expires) {
		String val = String::format("%ld; %ld; %s", expires, _tempFlag, value.c_str());
		bp_key_t bp_key = { key.length(), (Char*)*key };
		bp_value_t bp_val = { val.length(), (Char*)*val };
		assert_r(bp_set(_db, &bp_key, &bp_val));
	}

	static bool http_cookie_fuzz_query(cString& fuzz, bool reverse, String* keyOut) {
		bp_value_t val;
		bp_set_compare_cb(_db, [](void* arg, const bp_key_t *a, const bp_key_t *b) {
			int r = bp__fuzz_compare_cb(arg, a, b);
			if (r == 0) {
				if (b->length > a->length)
					return -1;
				if (arg)
					*static_cast<String*>(arg) = String(a->value, uint32_t(a->length));
				// match, a:ABCDE <=== b:ABCD
			}
			return r;
		}, keyOut);

		// fuzz query begin or end node
		bp_key_t bp_key = { fuzz.length(), (char*)*fuzz };
		int r = reverse ?
			bp_get_reverse(_db, &bp_key, &val): bp_get(_db, &bp_key, &val);

		return r == BP_OK ? (free(val.value), true): false;
	}

	// ------------------------------------------------------------------------------------

	String http_get_cookie(cString& domain, cString& name, cString& path, bool secure) {
		String result;
		OPEN(result);
		auto dir = get_indexed_key_prefix(secure, domain, String());
		auto paths = path.split('/');
		int i = 0, len = paths.length();

		while (http_cookie_fuzz_query(dir, false, nullptr)) { // Is there a directory "key"
			String key = dir + '@' + name;
			bp_key_t bp_key = { key.length(), (Char*)*key };
			bp_key_t val;
			bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);
			String s;
			if (bp_get_str(_db, &bp_key, &s) == BP_OK) {
				http_cookie_check_expires(s, &result);
			}
			if (i == len)
				break;
			dir.append(paths[i++]).append('/');
		}
		bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);
		return result;
	}

	void http_set_cookie_with_expression(cString& _domain, cString& expression) {
		//Set-Cookie: BAIDU_WISE_UID=bd_1491295526_455; expires=Thu, 04-Apr-2019 08:45:26 GMT; path=/; domain=baidu.com
		//Set-Cookie: BAIDUID=C9DD3739AD81A91137099489A6DA4C2F:FG=1; expires=Wed, 04-Apr-18 08:45:27 GMT; max-age=31536000; path=/; domain=.baidu.com; version=1
		//Set-Cookie: _octo=GH1.1.827978307.1724214581; domain=github.com; path=/; expires=Thu, 21 Aug 2025 04:29:41 GMT; secure; SameSite=Lax
		OPEN();
		String name, value;
		DictSS opts;

		for ( auto& i : expression.split("; ") ) {
			int j = i.indexOf('=');
			if ( j != -1 ) {
				if ( name.isEmpty() ) {
					name = i.substr(0, j);
					value = i.substr(j + 1);
				} else {
					opts.set(i.substr(0, j).lowerCase(), i.substr(j + 1));
				}
			} else {
				opts.set(i.lowerCase(), String());
			}
		}
		if (name.isEmpty()) return;

		auto domain = _domain;
		if (opts.get(DOMAIN_STR, domain)) {
			if ( domain.indexOf(_domain) == -1 ) return; // Illegal operation
		}

		int64_t expires = -1;
		bool    secure = opts.has(SECURE);
		String  outStr;
		String  path('/');
		opts.get(PATH, path);

		if ( opts.get(MAX_AGE, outStr) ) {
			expires = outStr.toNumber<int64_t>() * 1e6 + time_micro();
		}
		else if (opts.get(EXPIRES, outStr)) {
			expires = Int64::max(parse_time(outStr), expires);
		}

		// BD_NOT_HTTPS=1; path=/; Max-Age=300
		http_cookie_set_value_and_expires(get_indexed_key(secure, domain, path, name), value, expires);
	}

	void http_set_cookie(cString& domain, cString& name,
											cString& value, int64_t expires, cString& path, bool secure) {
		OPEN();
		http_cookie_set_value_and_expires(get_indexed_key(secure, domain, path, name), value, expires);
	}

	String http_get_all_cookie_string(cString& domain, cString& path, bool secure) {
		String result;
		DictSS all = http_get_all_cookie(domain, path, secure);
		if (all.length()) {
			Array<String> arr;
			for (auto& i : all)
				arr.push( String(i.key).append('=').append(i.value) );
			result = arr.join("; ");
		}
		return result;
	}

	DictSS http_get_all_cookie(cString& domain, cString& path, bool secure) {
		DictSS result;
		OPEN(result);
		auto dir = get_indexed_key_prefix(secure, domain, String());
		auto paths = path.split('/');
		int i = 0, len = paths.length();

		String range[2];
		while (http_cookie_fuzz_query(dir, false, nullptr)) { // Is there a directory "key"
			if (http_cookie_fuzz_query(dir + '@', false, range)) {
				http_cookie_fuzz_query(dir + '@', true, range+1);
				bp_key_t start = { range[0].length(), (char*)range[0].c_str() };
				bp_key_t end = { range[1].length(), (char*)range[1].c_str() };

				bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);
				bp_get_range(_db, &start, &end, [](void *arg, const bp_key_t *key, const bp_value_t *val) {
					String value;
					if (http_cookie_check_expires(String(val->value, uint32_t(val->length)), &value)) {
						auto ss = reinterpret_cast<DictSS*>(arg);
						char* s = strchr(key->value, '@') + 1;
						ss->set(String(s, uint32_t(key->length - (s - key->value))), value);
					}
				}, &result);
			}
			if (i == len)
				break;
			dir.append(paths[i++]).append('/');
		}
		bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);
		Qk_ReturnLocal(result);
	}

	void http_delete_cookie(cString& domain, cString& name, cString& path, bool secure) {
		OPEN();
		String key = get_indexed_key(secure, domain, path, name);
		bp_key_t bp_key = { key.length(), (Char*)*key };
		assert_r(bp_remove(_db, &bp_key));
	}

	void http_delete_all_cookie(cString& domain, cString& path, bool secure) {
		OPEN();
		String range[2];
		auto key = get_indexed_key_prefix(secure, domain, path);
		auto ok = http_cookie_fuzz_query(key, false, range);
		if (ok)
			Qk_ASSERT(http_cookie_fuzz_query(key, true, range+1));
		bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);

		if (ok) {
			Array<String> out;
			bp_key_t start = { range[0].length(), (char*)*range[0] };
			bp_key_t end = { range[1].length(), (char*)*range[1] };

			bp_get_range(_db, &start, &end, [](void *arg, const bp_key_t *key, const bp_value_t *val) {
				((Array<String>*)arg)->push(String(key->value, (uint32_t)key->length));
			}, &out);

			for (auto& i : out) {
				bp_key_t key = {
					i.length(), (char*)i.c_str()
				};
				assert_r(bp_remove(_db, &key));
			}
		}
	}

	void http_clear_cookie() {
		ScopeLock lock(_mutex);
		if ( _db ) {
			http_cookie_close();
			fs_unlink_sync(get_db_filename());
		} else {
			if (fs_is_file_sync(get_db_filename())) {
				fs_unlink_sync(get_db_filename());
			}
		}
	}
}
