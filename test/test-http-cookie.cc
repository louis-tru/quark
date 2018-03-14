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

#include "ngui/base/http-cookie.h"
#include "ngui/base/sys.h"

using namespace ngui;

void test_http_cookie() {
  
  LOG(http_cookie_get("ngui.io", "test"));
  
  http_cookie_set("ngui.io", "test", "ngui.io");
  
  LOG(http_cookie_get("ngui.io", "test"));
  
  LOG(http_cookie_get("www.ngui.io", "test"));
  
  http_cookie_set("www.ngui.io", "test2", "*********************************");
  
  LOG(http_cookie_get("ngui.io", "test2"));
  
  LOG(http_cookie_get("www.ngui.io", "test2"));
  
  http_cookie_set("ngui.io", "test2", "-----------------------------", -1, "/AA");
  
  LOG(http_cookie_get("ngui.io", "test2"));
  
  LOG(http_cookie_get("ngui.io", "test2", "/AA"));
  
  LOG(http_cookie_get_all_string("www.ngui.io", "/AA"));
  
  http_cookie_set_with_expression("ngui.io", "test3=HHHH--l; path=/AA; max-age=60");
  
  LOG(http_cookie_get("ngui.io", "test3"));
  
  LOG(http_cookie_get("ngui.io", "test3", "/AA"));
  
  LOG(http_cookie_get_all_string("www.ngui.io", "/AA"));
  
  // test delete
  
  http_cookie_delete("ngui.io", "test");
  
  LOG(http_cookie_get("ngui.io", "test"));
  
  http_cookie_set("ngui.io", "test", "ngui.io");
  
  LOG(http_cookie_get("ngui.io", "test"));
  
  http_cookie_delete_all("ngui.io");
  
  LOG(http_cookie_get("ngui.io", "test"));
  
  http_cookie_set("ngui.io", "test", "ngui.io");
  
  LOG(http_cookie_get("ngui.io", "test"));
  
  http_cookie_clear();
  
  LOG(http_cookie_get("ngui.io", "test"));
  
  http_cookie_set("ngui.io", "test", "END test cookie", sys::time() + 6e7); // 60s expires
  
  LOG(http_cookie_get("ngui.io", "test"));
  
}
