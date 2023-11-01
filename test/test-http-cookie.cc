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

#include "quark/util/http.h"
#include "quark/os/info.h"

using namespace qk;

void test_http_cookie(int argc, char **argv) {
	
	Qk_LOG(http_get_cookie("quarks.cc", "test"));
	
	http_set_cookie("quarks.cc", "test", "quarks.cc");
	
	Qk_LOG("A, %s", *http_get_cookie("quarks.cc", "test"));
	
	Qk_LOG("B, %s", *http_get_cookie("www.quarks.cc", "test"));

	http_set_cookie("www.quarks.cc", "test", "$");

	Qk_LOG("B2, %s", *http_get_cookie("www.quarks.cc", "test"));

	http_set_cookie("quarks.cc", "test2", "*");
	
	Qk_LOG("D, %s", *http_get_cookie("quarks.cc", "test2"));
	
	Qk_LOG("E, %s", *http_get_cookie("www.quarks.cc", "test2"));
	
	http_set_cookie("quarks.cc", "test2", "-----------------------------", -1, "/AA");
	
	Qk_LOG("F, %s", *http_get_cookie("quarks.cc", "test2"));
	
	Qk_LOG("H, %s", *http_get_cookie("quarks.cc", "test2", "/AA"));
	
	Qk_LOG(http_get_all_cookie_string("www.quarks.cc", "/AA"));
	
	http_set_cookie_with_expression("quarks.cc", "test3=HHHH--l; path=/AA; max-age=60");
	
	Qk_LOG(http_get_cookie("quarks.cc", "test3"));
	
	Qk_LOG(http_get_cookie("quarks.cc", "test3", "/AA"));
	
	Qk_LOG("http_cookie_get_all_string 1, %s", *http_get_all_cookie_string("www.quarks.cc", "/AA"));
	Qk_LOG("http_cookie_get_all_string 2, %s", *http_get_all_cookie_string("quarks.cc", "/AA"));
	
	// test delete
	
	http_delete_cookie("quarks.cc", "test");
	
	Qk_LOG(http_get_cookie("quarks.cc", "test"));
	
	http_set_cookie("quarks.cc", "test", "quarks.cc2");
	http_set_cookie("quarks.cc", "test9", "quarks.cc3");
	http_set_cookie("quarks.cc", "test8", "quarks.cc4");
	http_set_cookie("www.quarks.cc", "test7", "quarks.cc5");
	
	Qk_LOG("E, %s", *http_get_cookie("quarks.cc", "test"));

	http_set_cookie("quark.orh", "test--------A", "quarks.cc%", -1, "KKK/MMM");

	Qk_LOG("http_cookie_get_all_string 3, %s", *http_get_all_cookie_string("quarks.cc"));
	
	http_delete_all_cookie("quarks.cc");
	
	Qk_LOG(http_get_cookie("quarks.cc", "test"));
	
	http_set_cookie("quarks.cc", "test", "quarks.cc");
	
	Qk_LOG("F, %s", *http_get_cookie("quarks.cc", "test--------A", "KKK/MMM", 1));
	
	http_clear_cookie();
	
	Qk_LOG(http_get_cookie("quarks.cc", "test"));
	
	http_set_cookie("quarks.cc", "test", "END test cookie", time_micro() + 6e7); // 60s expires
	
	Qk_LOG(http_get_cookie("quarks.cc", "test"));
	
}
