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

#include "ftr/util/http-cookie.h"
#include "ftr/sys.h"

using namespace ftr;

void test_http_cookie(int argc, char **argv) {
	
	LOG(http_cookie_get("ngui.fun", "test"));
	
	http_cookie_set("ngui.fun", "test", "ngui.fun");
	
	LOG("A, %s", *http_cookie_get("ngui.fun", "test"));
	
	LOG("B, %s", *http_cookie_get("www.ngui.fun", "test"));

	http_cookie_set("www.ngui.fun", "test", "$");

	LOG("B2, %s", *http_cookie_get("www.ngui.fun", "test"));

	http_cookie_set("ngui.fun", "test2", "*");
	
	LOG("D, %s", *http_cookie_get("ngui.fun", "test2"));
	
	LOG("E, %s", *http_cookie_get("www.ngui.fun", "test2"));
	
	http_cookie_set("ngui.fun", "test2", "-----------------------------", -1, "/AA");
	
	LOG("F, %s", *http_cookie_get("ngui.fun", "test2"));
	
	LOG("H, %s", *http_cookie_get("ngui.fun", "test2", "/AA"));
	
	LOG(http_cookie_get_all_string("www.ngui.fun", "/AA"));
	
	http_cookie_set_with_expression("ngui.fun", "test3=HHHH--l; path=/AA; max-age=60");
	
	LOG(http_cookie_get("ngui.fun", "test3"));
	
	LOG(http_cookie_get("ngui.fun", "test3", "/AA"));
	
	LOG("http_cookie_get_all_string 1, %s", *http_cookie_get_all_string("www.ngui.fun", "/AA"));
	LOG("http_cookie_get_all_string 2, %s", *http_cookie_get_all_string("ngui.fun", "/AA"));
	
	// test delete
	
	http_cookie_delete("ngui.fun", "test");
	
	LOG(http_cookie_get("ngui.fun", "test"));
	
	http_cookie_set("ngui.fun", "test", "ngui.fun2");
	http_cookie_set("ngui.fun", "test9", "ngui.fun3");
	http_cookie_set("ngui.fun", "test8", "ngui.fun4");
	http_cookie_set("www.ngui.fun", "test7", "ngui.fun5");
	
	LOG("E, %s", *http_cookie_get("ngui.fun", "test"));

	http_cookie_set("ftr.orh", "test--------A", "ngui.fun%", -1, "KKK/MMM");

	LOG("http_cookie_get_all_string 3, %s", *http_cookie_get_all_string("ngui.fun"));
	
	http_cookie_delete_all("ngui.fun");
	
	LOG(http_cookie_get("ngui.fun", "test"));
	
	http_cookie_set("ngui.fun", "test", "ngui.fun");
	
	LOG("F, %s", *http_cookie_get("ngui.fun", "test--------A", "KKK/MMM", 1));
	
	http_cookie_clear();
	
	LOG(http_cookie_get("ngui.fun", "test"));
	
	http_cookie_set("ngui.fun", "test", "END test cookie", sys::time() + 6e7); // 60s expires
	
	LOG(http_cookie_get("ngui.fun", "test"));
	
}
