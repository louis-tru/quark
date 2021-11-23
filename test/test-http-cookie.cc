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

#include "flare/util/http.h"
#include "flare/util/os.h"

using namespace flare;

void test_http_cookie(int argc, char **argv) {
	
	F_LOG(HttpHelper::get_cookie("flare.cool", "test"));
	
	HttpHelper::set_cookie("flare.cool", "test", "flare.cool");
	
	F_LOG("A, %s", *HttpHelper::get_cookie("flare.cool", "test"));
	
	F_LOG("B, %s", *HttpHelper::get_cookie("www.flare.cool", "test"));

	HttpHelper::set_cookie("www.flare.cool", "test", "$");

	F_LOG("B2, %s", *HttpHelper::get_cookie("www.flare.cool", "test"));

	HttpHelper::set_cookie("flare.cool", "test2", "*");
	
	F_LOG("D, %s", *HttpHelper::get_cookie("flare.cool", "test2"));
	
	F_LOG("E, %s", *HttpHelper::get_cookie("www.flare.cool", "test2"));
	
	HttpHelper::set_cookie("flare.cool", "test2", "-----------------------------", -1, "/AA");
	
	F_LOG("F, %s", *HttpHelper::get_cookie("flare.cool", "test2"));
	
	F_LOG("H, %s", *HttpHelper::get_cookie("flare.cool", "test2", "/AA"));
	
	F_LOG(HttpHelper::get_all_cookie_string("www.flare.cool", "/AA"));
	
	HttpHelper::set_cookie_with_expression("flare.cool", "test3=HHHH--l; path=/AA; max-age=60");
	
	F_LOG(HttpHelper::get_cookie("flare.cool", "test3"));
	
	F_LOG(HttpHelper::get_cookie("flare.cool", "test3", "/AA"));
	
	F_LOG("http_cookie_get_all_string 1, %s", *HttpHelper::get_all_cookie_string("www.flare.cool", "/AA"));
	F_LOG("http_cookie_get_all_string 2, %s", *HttpHelper::get_all_cookie_string("flare.cool", "/AA"));
	
	// test delete
	
	HttpHelper::delete_cookie("flare.cool", "test");
	
	F_LOG(HttpHelper::get_cookie("flare.cool", "test"));
	
	HttpHelper::set_cookie("flare.cool", "test", "flare.cool2");
	HttpHelper::set_cookie("flare.cool", "test9", "flare.cool3");
	HttpHelper::set_cookie("flare.cool", "test8", "flare.cool4");
	HttpHelper::set_cookie("www.flare.cool", "test7", "flare.cool5");
	
	F_LOG("E, %s", *HttpHelper::get_cookie("flare.cool", "test"));

	HttpHelper::set_cookie("flare.orh", "test--------A", "flare.cool%", -1, "KKK/MMM");

	F_LOG("http_cookie_get_all_string 3, %s", *HttpHelper::get_all_cookie_string("flare.cool"));
	
	HttpHelper::delete_all_cookie("flare.cool");
	
	F_LOG(HttpHelper::get_cookie("flare.cool", "test"));
	
	HttpHelper::set_cookie("flare.cool", "test", "flare.cool");
	
	F_LOG("F, %s", *HttpHelper::get_cookie("flare.cool", "test--------A", "KKK/MMM", 1));
	
	HttpHelper::clear_cookie();
	
	F_LOG(HttpHelper::get_cookie("flare.cool", "test"));
	
	HttpHelper::set_cookie("flare.cool", "test", "END test cookie", os::time() + 6e7); // 60s expires
	
	F_LOG(HttpHelper::get_cookie("flare.cool", "test"));
	
}
