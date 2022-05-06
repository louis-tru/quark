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

#include "noug/util/http.h"
#include "noug/os/os.h"

using namespace noug;

void test_http_cookie(int argc, char **argv) {
	
	N_LOG(http_get_cookie("noug.cc", "test"));
	
	http_set_cookie("noug.cc", "test", "noug.cc");
	
	N_LOG("A, %s", *http_get_cookie("noug.cc", "test"));
	
	N_LOG("B, %s", *http_get_cookie("www.noug.cc", "test"));

	http_set_cookie("www.noug.cc", "test", "$");

	N_LOG("B2, %s", *http_get_cookie("www.noug.cc", "test"));

	http_set_cookie("noug.cc", "test2", "*");
	
	N_LOG("D, %s", *http_get_cookie("noug.cc", "test2"));
	
	N_LOG("E, %s", *http_get_cookie("www.noug.cc", "test2"));
	
	http_set_cookie("noug.cc", "test2", "-----------------------------", -1, "/AA");
	
	N_LOG("F, %s", *http_get_cookie("noug.cc", "test2"));
	
	N_LOG("H, %s", *http_get_cookie("noug.cc", "test2", "/AA"));
	
	N_LOG(http_get_all_cookie_string("www.noug.cc", "/AA"));
	
	http_set_cookie_with_expression("noug.cc", "test3=HHHH--l; path=/AA; max-age=60");
	
	N_LOG(http_get_cookie("noug.cc", "test3"));
	
	N_LOG(http_get_cookie("noug.cc", "test3", "/AA"));
	
	N_LOG("http_cookie_get_all_string 1, %s", *http_get_all_cookie_string("www.noug.cc", "/AA"));
	N_LOG("http_cookie_get_all_string 2, %s", *http_get_all_cookie_string("noug.cc", "/AA"));
	
	// test delete
	
	http_delete_cookie("noug.cc", "test");
	
	N_LOG(http_get_cookie("noug.cc", "test"));
	
	http_set_cookie("noug.cc", "test", "noug.cc2");
	http_set_cookie("noug.cc", "test9", "noug.cc3");
	http_set_cookie("noug.cc", "test8", "noug.cc4");
	http_set_cookie("www.noug.cc", "test7", "noug.cc5");
	
	N_LOG("E, %s", *http_get_cookie("noug.cc", "test"));

	http_set_cookie("noug.orh", "test--------A", "noug.cc%", -1, "KKK/MMM");

	N_LOG("http_cookie_get_all_string 3, %s", *http_get_all_cookie_string("noug.cc"));
	
	http_delete_all_cookie("noug.cc");
	
	N_LOG(http_get_cookie("noug.cc", "test"));
	
	http_set_cookie("noug.cc", "test", "noug.cc");
	
	N_LOG("F, %s", *http_get_cookie("noug.cc", "test--------A", "KKK/MMM", 1));
	
	http_clear_cookie();
	
	N_LOG(http_get_cookie("noug.cc", "test"));
	
	http_set_cookie("noug.cc", "test", "END test cookie", time_micro() + 6e7); // 60s expires
	
	N_LOG(http_get_cookie("noug.cc", "test"));
	
}
