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

#include "qgr/utils/http-cookie.h"
#include "qgr/utils/fs.h"
#include "qgr/utils/json.h"
#include <bplus.h>

XX_NS(qgr)

#define _db _http_cookie_db
#define assert_r(c) XX_ASSERT(c == BP_OK)

static Mutex mutex;
static bp_db_t* _http_cookie_db = nullptr;
static int _has_initialize = 0;
static int64 _http_cookie_date = 0;
static const String EXPIRES("expires");
static const String MAX_AGE("max-age");
static const String PATH("path");
static const String DOMAIN_STR("domain");
static const String SECURE("secure");

static String get_db_filename() {
	return Path::temp(".cookie.dp");
}

static void http_cookie_close() {
	bp_db_t* __db = _http_cookie_db;
	_db = nullptr;
	if ( __db ) bp_close(__db);
}

static int bp__fuzz_compare_cb(void *arg, const bp_key_t *a, const bp_key_t *b) 
{
	uint32_t len = a->length < b->length ? a->length : b->length;

	for (uint32_t i = 0; i < len; i++) {
		if (a->value[i] != b->value[i])
			return (uint8_t) a->value[i] > (uint8_t) b->value[i] ? 1 : -1;
	}
	return 0;
}

static int bp__default_compare_cb(void *arg, const bp_key_t *a, const bp_key_t *b)
{
	int r = bp__fuzz_compare_cb(arg, a, b);
	if (r == 0) {
		return a->length - b->length;	
	}
	return r;
}

static int bp__default_filter_cb(void *arg, const bp_key_t *key)
{
	/* default filter accepts all keys */
	return 1;
}

static void http_cookie_open() {
	if ( _db == nullptr ) {
		int r = bp_open(&_db, Path::fallback_c(get_db_filename()));
		if ( r == BP_OK ) {
			bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);
			if (_has_initialize++ == 0) {
				_http_cookie_date = sys::time_monotonic();
				atexit(http_cookie_close);
			}
		} else {
			_db = nullptr;
		}
	}
}

static String get_storage_key_prefix(bool secure, cString& domain) {
	String r(secure ? '1': '0'); 
	r.push('.');
	auto domains = domain.split('.');
	for (int i = domains.length() - 1; i > -1; i--) {
		if (!domains[i].is_empty())
			r.push(domains[i]).push('.');
	}

	return  r;
}

static String get_storage_key(cString& domain, 
	cString& name, cString& path, bool secure) {
	String r = get_storage_key_prefix(secure, domain);

	if (!path.is_empty()) {
		if (path[0] != '/')
			r.push('/');
		r.push(path);
		if (r[r.length() - 1] != '/')
			r.push('/');
	}
	else {
		r.push('/');
	}

	r.push('@');
	r.push(name);
	return r;
}

static int http_cookie_fuzz_query_compare_cb(void* arg, const bp_key_t *a, const bp_key_t *b) {
	auto cur = (Buffer*)arg;
	int r = bp__fuzz_compare_cb(arg, a, b);
	if (r != 0) return r;
	if (b->length > a->length) return -1;
	// LOG("a: %s, b: %s", a->value, b->value);
	cur->write(a->value, 0, a->length);
	return 0;
};

static int http_cookie_fuzz_query(cString& domain, bool secure, Buffer *buf) 
{
	String _key = get_storage_key_prefix(secure, domain);
	bp_key_t key = { _key.length(), (char*)*_key };
	bp_value_t value;
	int r;

	// fuzz query begin end node 
	bp_set_compare_cb(_db, http_cookie_fuzz_query_compare_cb, buf);
	r = bp_get(_db, &key, &value);

	if (r != BP_OK) goto end;

	bp_set_compare_cb(_db, http_cookie_fuzz_query_compare_cb, buf+1);
	r = bp_get_reverse(_db, &key, &value);

	if (r != BP_OK) goto end;

end:
	bp_set_compare_cb(_db, bp__default_compare_cb, nullptr);

	return r;
}

String http_cookie_get(cString& domain, cString& name, cString& path, bool secure) {
	http_cookie_open();

	if ( _db ) {
		String _key = get_storage_key(domain, name, path, secure);
		bp_key_t key = { _key.length(), (char*)*_key };
		bp_key_t val;

		if (bp_get_reverse(_db, &key, &val) == BP_OK) {
			try { 
				JSON json = JSON::parse(Buffer(val.value, val.length));
				int64 expires = json[0].to_int64();
				int64 date = json[1].to_int64();

				if ((expires == -1 && date == _http_cookie_date) || expires > sys::time()) {
					return json[2].to_string();
				}
			} catch(cError& err) {
				XX_ERR(err);
			}
		}
	}
	return String();
}

static void http_cookie_set2(String& _key, cString& value, int64 expires) 
{
	int r;

	JSON json = JSON::array();
	json[0] = expires;
	json[1] = _http_cookie_date;
	json[2] = value;

	String _val = JSON::stringify(json);
	// LOG("---- %s, %d", *_val, _val.length());

	bp_key_t key = { _key.length(), (char*)*_key };
	bp_value_t val = { _val.length(), (char*)*_val };

	r = bp_set(_db, &key, &val);
	assert_r(r);
}

void http_cookie_set_with_expression(cString& domain, cString& expression) 
{
	//Set-Cookie: BAIDU_WISE_UID=bd_1491295526_455; expires=Thu, 04-Apr-2019 08:45:26 GMT; path=/; domain=baidu.com
	//Set-Cookie: BAIDUID=C9DD3739AD81A91137099489A6DA4C2F:FG=1; expires=Wed, 04-Apr-18 08:45:27 GMT; max-age=31536000; path=/; domain=.baidu.com; version=1
	http_cookie_open();

	if ( _db ) {
		String name, value;
		String domain_ = domain;
		String path('/');

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
			String new_domain = it.value();
			if ( domain_.index_of(new_domain) == -1 ) {
				return; // Illegal operation
			} else {
				domain_ = new_domain;
			}
		}
		
		it = options.find(PATH);
		if ( it != end ) {
			path = it.value();
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
		String key = get_storage_key(domain_, name, path, secure);
		
		http_cookie_set2(key, value, expires);
	}
}

void http_cookie_set(cString& domain,
										 cString& name,
										 cString& value, int64 expires, cString& path, bool secure) 
{
	http_cookie_open();
	if ( _db ) {
		String key = get_storage_key(domain, name, path, secure);
		http_cookie_set2(key, value, expires);
	}
}

void http_cookie_delete(cString& domain, cString& name, cString& path, bool secure) {
	http_cookie_open();
	if ( _db ) {
		int r;

		String _key = get_storage_key(domain, name, path, secure);
		bp_key_t key = { _key.length(), (char*)*_key };

		r =  bp_remove(_db, &key); assert_r(r);
	}
}

String http_cookie_get_all_string(cString& domain, cString& path, bool secure) {

	Map<String, String> all = http_cookie_get_all(domain, path, secure);

	if (all.length()) {
		Array<String> result;
		for (auto& i : all) {
			result.push(move(String(i.key()).push('=').push(i.value())));
		}
		return result.join( "; " );
	}

	return String();
}

Map<String, String> http_cookie_get_all(cString& domain, cString& path, bool secure) {
	ScopeLock scope(mutex);
	http_cookie_open();
	Map<String, String> result;

	if ( _db ) {

		Buffer buf[2];
		int r = http_cookie_fuzz_query(domain, secure, buf);

		if (r == BP_OK) {
			bp_key_t start = { buf[0].length(), buf[0].value() };
			bp_key_t end = { buf[1].length(), buf[1].value() };

			struct tmp_data_t {
				Map<String, String> *result;
				String path;
			} _tmp = { &result, path.is_empty() ? String('/'): path };

			r = bp_get_filtered_range(_db, &start, &end, [](void* arg, const bp_key_t *key) {
				auto path = &reinterpret_cast<tmp_data_t*>(arg)->path;
				char* s = strchr(key->value, '/');
				if (s) {
					int i = 0, t_len = path->length();
					cchar* t = path->c();

					// LOG("bp_get_filtered_range, %s, %s", s, t);

					while(s[i] != '@') {
						if (s[i] != t[i] || i >= t_len) {
							return 0;
						}
						i++;
					}
					return 1;
				}
				return 0;
			},
			[](void *arg, const bp_key_t *key, const bp_value_t *val) {
				auto m = reinterpret_cast<tmp_data_t*>(arg)->result;
				try { 
					JSON json = JSON::parse(WeakBuffer(val->value, val->length));
					int64 expires = json[0].to_int64();
					int64 date = json[1].to_int64();

					if ((expires == -1 && date == _http_cookie_date) || expires > sys::time()) {
						char* s = strchr(key->value, '@') + 1;
						m->set(String(s, key->length - (s - key->value)), json[2].to_string());
					}
				} catch(cError& err) {
					XX_ERR(err);
				}
			}, &_tmp);
		}
	}
	return result;
}

void http_cookie_delete_all(cString& domain, bool secure) {
	ScopeLock scope(mutex);
	http_cookie_open();
	if ( _db ) {

		Buffer buf[2];
		int r = http_cookie_fuzz_query(domain, secure, buf);

		if (r == BP_OK) {
			bp_key_t start = { buf[0].length(), buf[0].value() };
			bp_key_t end = { buf[1].length(), buf[1].value() };
			Array<String> rms;

			//LOG("http_cookie_delete_all, %s, %s", start.value, end.value);

			r = bp_get_range(_db, &start, &end, [](void *arg,
														const bp_key_t *key,
														const bp_value_t *value) {
				//LOG("http_cookie_delete_all 1, %s", value->value);
				reinterpret_cast<Array<String>*>(arg)->push(String(key->value, key->length));
			}, &rms);

			assert_r(r);

			for (auto& i : rms) {
				bp_key_t key = { 
					i.value().length(), (char*)*i.value(),
				};
				r = bp_remove(_db, &key); assert_r(r);
				//LOG("http_cookie_delete_all 2, %s", key.value);
			}
		}
	}
}

void http_cookie_clear() {
	if ( !_db ) {
		if (FileHelper::is_file_sync(get_db_filename())) {
			FileHelper::unlink_sync(get_db_filename());
		}
	} else {
		http_cookie_close();
		FileHelper::unlink_sync(get_db_filename());
	}
}

XX_END
