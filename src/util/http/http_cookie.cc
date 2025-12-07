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

#include "../lmdb.h"
#include "../http.h"

namespace qk {
	#define _db LMDB::shared()
	#define _dbi get_cookie_db()
	static LMDB_DBIPtr _lmdb_dbi = nullptr;
	inline LMDB_DBIPtr get_cookie_db() {
		if (_lmdb_dbi == nullptr)
			_lmdb_dbi = _db->dbi("cookie");
		return _lmdb_dbi;
	}

	// Session ID used to distinguish session cookies (invalid after process restart)
	int64_t _cookieSessionId = 0;

	static cString EXPIRES("expires");
	static cString MAX_AGE("max-age");
	static cString PATH("path");
	static cString DOMAIN_STR("domain");
	static cString SECURE("secure");
	// static cString HTTP_ONLY("httponly");

	// Generate session id (process-level)
	// Using monotonic time ensures uniqueness within the current process session
	static int64_t session_id() {
		if (_cookieSessionId == 0) {
			_cookieSessionId = time_monotonic(); // monotonic timestamp, ensures uniqueness
		}
		return _cookieSessionId;
	}

	// key = [secureFlag]/[reversed-domain]/[path]/'@'[name]
	// secureFlag: "1" or "0"
	// reversed-domain: com/example/www
	// Path: Remove leading and trailing "/"
	// Example: secure=false, domain=www.example.com, path=/a/b/, name=token
	// key: "0/com/example/www/a/b/@token"
	static String get_key_prefix(bool secure, cString& domain, cString& path) {
		String r(secure ? "1" : "0");

		// Reverse domain: com/example/www
		auto arr = domain.trim().split('.'); // Split by "."
		for (int i = Qk_Minus(arr.length(), 1); i > -1; i--) {
			if (!arr[i].isEmpty()) {
				r.append('/').append(arr[i]);
			}
		}

		auto trimed = path.trim(); // Trim whitespace
		if (trimed.length() == 1 && trimed[0] == '/') {
			return r; // Root path special case
		}

		// Path: Remove trailing "/"
		if (!trimed.isEmpty()) {
			if (trimed[0] != '/') {
				r.append('/'); // Ensure leading "/"
			}
			auto len = trimed.length();
			if (trimed[len - 1] == '/') {
				len--;
			}
			r.append(trimed.c_str(), len);
		}
		return r;
	}

	// Full key
	static String get_key(bool secure, cString& domain, cString& path, cString& name) {
		return get_key_prefix(secure, domain, path).append("/@").append(name);
	}

	// Get domain levels for prefix scan
	static Array<String> get_key_domain_levels(bool secure, cString& domain) {
		Array<String> levels({secure ? "1" : "0"}); // www.example.com -> [0/com/example/, www]
		// Reverse domain: com/example/www
		auto arr = domain.trim().split('.'); // Split by "."
		int i = Qk_Minus(arr.length(), 1);
		for (int j = Qk_Max(i - 2, -1); i > j; i--) {
			if (!arr[i].isEmpty())
				levels[0].append('/').append(arr[i]);
		}
		for (; i > -1; i--) {
			if (!arr[i].isEmpty())
				levels.push(arr[i]);
		}
		levels[0].append('/'); // add trailing "/"
		Qk_ReturnLocal(levels);
	}

	// Get path levels for prefix scan
	static Array<String> get_key_path_levels(cString& path) {
		Array<String> levels; // a/b/ -> [a, b]
		// Path: split by "/"
		for (auto& segment : path.trim().split('/')) {
			if (!segment.isEmpty())
				levels.push(segment);
		}
		Qk_ReturnLocal(levels);
	}

	// ------------------------ Expiration check ------------------------
	// Storage format: "<expires>; <sessionId>; <value>"
	// expires: microseconds, -1 means session cookie
	static bool http_cookie_check_expires(const String& raw, String* valueOut) {
		// Find first "; "
		auto idx = raw.indexOf("; ");
		if (idx == -1) return false;

		// Second "; "
		auto idx2 = raw.indexOf("; ", idx + 2);
		if (idx2 == -1) return false;

		int64_t expires = raw.substring(0, idx).toNumber<int64_t>();
		int64_t sid = raw.substring(idx + 2, idx2).toNumber<int64_t>();

		// Session cookie: expires == -1 and sid == current session
		// or not expired
		if (expires == -1 && sid == session_id() || expires > time_microsecond()) {
			*valueOut = raw.substring(idx2 + 2);
			return true;
		}

		// expired
		return false;
	}

	// Set value and expiration timestamp
	static void http_cookie_set_value_and_expires(cString& key, cString& value, int64_t expires) {
		String val = String::format("%ld; %ld; %s",
																expires,
																session_id(),
																value.c_str());
		_db->set(_dbi, key, val);
	}

	// -------------------- Public API --------------------

	// Read a specific cookie
	String http_get_cookie(cString& domain, cString& name, cString& path, bool secure) {
		String result;
		// Generate key levels
		auto domainLevels = get_key_domain_levels(secure, domain);
		auto pathLevels = get_key_path_levels(path);

		for (int j = 0; j < domainLevels.length(); j++) {
			if (j) {
				domainLevels[0].append(domainLevels[j]).append('/'); // Append next domain segment
			}
			// Pre-check prefix existence to avoid unnecessary range scans
			if (!_db->fuzz_exists(_dbi, domainLevels[0]))
				continue;

			auto prefix = domainLevels[0]; // copy domain prefix
			int i = 0; // Start from first path segment
			// Start from the shortest path: domain + "", then /a/, /a/b/, ... until full path
			do {
				String curKey = prefix + '@' + name;
				String stored;
				if (_db->get(_dbi, curKey, &stored) == 0) {
					http_cookie_check_expires(stored, &result); // Evaluate from short to long; longest prefix wins
				}
				if (i >= pathLevels.length())
					break;
				prefix.append(pathLevels[i++]).append('/'); // Append next path segment
			} while (true);
		}

		return result;
	}

	// Parse RFC-style Set-Cookie expression and store it
	void http_set_cookie_with_expression(cString& _domain, cString& expression) {
		// Examples:
		//Set-Cookie: BAIDU_WISE_UID=bd_1491295526_455; expires=Thu, 04-Apr-2019 08:45:26 GMT; path=/; domain=baidu.com
		//Set-Cookie: BAIDUID=C9DD3739AD81A91137099489A6DA4C2F:FG=1; expires=Wed, 04-Apr-18 08:45:27 GMT; max-age=31536000; path=/; domain=.baidu.com; version=1
		//Set-Cookie: _octo=GH1.1.827978307.1724214581; domain=github.com; path=/; expires=Thu, 21 Aug 2025 04:29:41 GMT; secure; SameSite=Lax

		// Parse Set-Cookie expression
		String name, value;
		DictSS opts;

		for (auto& i : expression.split("; ")) {
			int j = i.indexOf('=');
			if (j != -1) {
				if (name.isEmpty()) {
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
			if (domain.indexOf(_domain) == -1) return; // Illegal operation
		}

		int64_t expires = -1;
		bool    secure = opts.has(SECURE);
		String  outStr;
		String  path('/');

		opts.get(PATH, path);

		if (opts.get(MAX_AGE, outStr)) {
			expires = outStr.toNumber<int64_t>() * 1e6 + time_microsecond();
		} else if (opts.get(EXPIRES, outStr)) {
			expires = Int64::max(parse_time(outStr), expires);
		}

		// Example: BD_NOT_HTTPS=1; path=/; Max-Age=300
		http_cookie_set_value_and_expires(
			get_key(secure, domain, path, name), value, expires
		);
	}

	// Direct cookie set
	void http_set_cookie(cString& domain, cString& name,
											cString& value, int64_t expires, cString& path, bool secure) {
		http_cookie_set_value_and_expires(
			get_key(secure, domain, path, name), value, expires
		);
	}

	// get_all_cookie_string: "k1=v1; k2=v2"
	String http_get_all_cookie_string(cString& domain, cString& path, bool secure) {
		DictSS all = http_get_all_cookie(domain, path, secure);
		String result;
		if (all.length()) {
			Array<String> arr;
			for (auto& i : all) {
				arr.push(String(i.first).append('=').append(i.second));
			}
			result = arr.join("; "); // Using join is faster
		}
		return result;
	}

	// Read all cookies under domain + path
	DictSS http_get_all_cookie(cString& domain, cString& path, bool secure) {
		DictSS result;
		// Generate key levels
		auto domainLevels = get_key_domain_levels(secure, domain);
		auto pathLevels = get_key_path_levels(path);

		for (int j = 0; j < domainLevels.length(); j++) {
			if (j) {
				domainLevels[0].append(domainLevels[j]).append('/'); // Append next domain segment
			}
			auto prefix = domainLevels[0]; // copy domain prefix
			int i = 0; // Start from first path segment

			// Start from the shortest path: domain + "", then /a/, /a/b/, ... until full path
			while (_db->fuzz_exists(_dbi, prefix)) { // Precheck prefix, reduce unnecessary scans
				String curKey = prefix + '@'; // fuzz key = prefix + '@'
				Array<Pair<String,String>> listOut;
				_db->scan_prefix(_dbi, curKey, &listOut); // Scan all cookies under this prefix

				for (auto& kv : listOut) {
					String value;
					if (http_cookie_check_expires(kv.second, &value)) {
						// Extract name part
						const char* s = strchr(kv.first.c_str(), '@') + 1;
						result.set(String(s, uint32_t(kv.first.length() - (s - kv.first.c_str()))), value);
					}
				}
				if (i >= pathLevels.length())
					break;
				prefix.append(pathLevels[i++]).append('/'); // Append path segment
			}
		}

		Qk_ReturnLocal(result);
	}

	// Delete a single cookie
	void http_delete_cookie(cString& domain, cString& name, cString& path, bool secure) {
		String key = get_key(secure, domain, path, name);
		_db->remove(_dbi, key);
	}

	// Delete all cookies under a path (including deeper subpaths)
	void http_delete_all_cookie(cString& domain, cString& path, bool secure) {
		_db->remove_prefix(_dbi, get_key_prefix(secure, domain, path).append('/'));
	}

	// Clear all cookies
	void http_clear_cookie() {
		_db->clear(_dbi);
	}

} // namespace qk
