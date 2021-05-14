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

#include "flare/util/http-cookie.h"
#include "flare/sys.h"

using namespace flare;

void test_http_cookie(int argc, char **argv) {
	
	LOG(http_cookie_get("flare.cool", "test"));
	
	http_cookie_set("flare.cool", "test", "flare.cool");
	
	LOG("A, %s", *http_cookie_get("flare.cool", "test"));
	
	LOG("B, %s", *http_cookie_get("www.flare.cool", "test"));

	http_cookie_set("www.flare.cool", "test", "$");

	LOG("B2, %s", *http_cookie_get("www.flare.cool", "test"));

	http_cookie_set("flare.cool", "test2", "*");
	
	LOG("D, %s", *http_cookie_get("flare.cool", "test2"));
	
	LOG("E, %s", *http_cookie_get("www.flare.cool", "test2"));
	
	http_cookie_set("flare.cool", "test2", "-----------------------------", -1, "/AA");
	
	LOG("F, %s", *http_cookie_get("flare.cool", "test2"));
	
	LOG("H, %s", *http_cookie_get("flare.cool", "test2", "/AA"));
	
	LOG(http_cookie_get_all_string("www.flare.cool", "/AA"));
	
	http_cookie_set_with_expression("flare.cool", "test3=HHHH--l; path=/AA; max-age=60");
	
	LOG(http_cookie_get("flare.cool", "test3"));
	
	LOG(http_cookie_get("flare.cool", "test3", "/AA"));
	
	LOG("http_cookie_get_all_string 1, %s", *http_cookie_get_all_string("www.flare.cool", "/AA"));
	LOG("http_cookie_get_all_string 2, %s", *http_cookie_get_all_string("flare.cool", "/AA"));
	
	// test delete
	
	http_cookie_delete("flare.cool", "test");
	
	LOG(http_cookie_get("flare.cool", "test"));
	
	http_cookie_set("flare.cool", "test", "flare.cool2");
	http_cookie_set("flare.cool", "test9", "flare.cool3");
	http_cookie_set("flare.cool", "test8", "flare.cool4");
	http_cookie_set("www.flare.cool", "test7", "flare.cool5");
	
	LOG("E, %s", *http_cookie_get("flare.cool", "test"));

	http_cookie_set("flare.orh", "test--------A", "flare.cool%", -1, "KKK/MMM");

	LOG("http_cookie_get_all_string 3, %s", *http_cookie_get_all_string("flare.cool"));
	
	http_cookie_delete_all("flare.cool");
	
	LOG(http_cookie_get("flare.cool", "test"));
	
	http_cookie_set("flare.cool", "test", "flare.cool");
	
	LOG("F, %s", *http_cookie_get("flare.cool", "test--------A", "KKK/MMM", 1));
	
	http_cookie_clear();
	
	LOG(http_cookie_get("flare.cool", "test"));
	
	http_cookie_set("flare.cool", "test", "END test cookie", sys::time() + 6e7); // 60s expires
	
	LOG(http_cookie_get("flare.cool", "test"));
	
}
