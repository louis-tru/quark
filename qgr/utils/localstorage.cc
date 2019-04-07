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
#include <bplus.h>

XX_NS(qgr)

#define _db _localstorage_db
#define assert_r(c) XX_ASSERT(c == BP_OK)

static bp_db_t* _localstorage_db = nullptr;
static int _has_initialize = 0;

static String get_db_filename() {
	return Path::temp(".localstorage.bp");
}

static void localstorage_close() {
	bp_db_t* __db = _localstorage_db;
	_db = nullptr;
	if ( __db ) bp_close(__db);
}

static void localstorage_open() {
	if ( _localstorage_db == nullptr ) {
		int r = bp_open(&_db, Path::fallback_c(get_db_filename()));
		if ( r == BP_OK ) {
			if (_has_initialize++ == 0)
				atexit(localstorage_close);
		} else {
			_db = nullptr;
		}
	}
}

String localstorage_get(cString& name) {
	localstorage_open();
	String result;
	if ( _db ) {
		bp_key_t key = { name.length(), (char*)*name };
		bp_key_t val;
		if (bp_get(_db, &key, &val) == BP_OK) {
			result = Buffer(val.value, val.length);
		}
	}
	return result;
}

void localstorage_set(cString& name, cString& value) {
	localstorage_open();
	if ( _db ) {
		bp_key_t   key = { name.length(), (char*)*name };
		bp_value_t val = { value.length(), (char*)*value };
		int r = bp_set(_db, &key, &val); assert_r(r);
	}
}

void localstorage_delete(cString& name) {
	localstorage_open();
	if ( _db ) {
		bp_key_t key = { name.length(), (char*)*name };
		int r = bp_remove(_db, &key); assert_r(r);
	}
}

void localstorage_clear() {
	if ( !_db ) {
		if (FileHelper::is_file_sync(get_db_filename())) {
			FileHelper::unlink_sync(get_db_filename());
		}
	} else {
		localstorage_close();
		FileHelper::unlink_sync(get_db_filename());
	}
}

void localstorage_transaction(cCb& cb) {
	sync_callback(cb);
}

XX_END
