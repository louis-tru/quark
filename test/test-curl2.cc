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

#include <shark/utils/util.h>
#include <shark/utils/http.h>
#include <shark/utils/string.h>
#include <shark/utils/fs.h>
#include <curl/curl.h>

using namespace shark;

static int req_progress_cb(void* req,
													 curl_off_t dltotal, curl_off_t dlnow,
													 curl_off_t ultotal, curl_off_t ulnow) {
	LOG("progress download: %ld/%ld, upload: %ld/%ld", dlnow, dltotal, ulnow, ultotal);
	return 0;
}

static size_t req_header_cb(void* buffer, size_t size, size_t nmemb, void* data) {
	uint len = (uint)(size * nmemb);
	
	String str((cchar*)buffer, len);
	
	LOG("%s", *str);
	
	return len;
}

static size_t req_writer_cb(void* buffer, size_t size, size_t nmemb, void* data) {
	uint len = (uint)(size * nmemb);
	
	LOG("BODY:");
	LOG(String((cchar*)buffer, len));
	
	return len;
}

static size_t req_reader_cb(void* buffer, size_t size, size_t nmemb, void* data) {
	LOG("req_reader_cb");
	return 0;
}

void test_curl2(int argc, char **argv) {
	
	HttpHelper::initialize();
	
	static cString url = "http://shark-x.org:1026/TestService/test?asasas=pppp";
	//  static cString url = "http://sharkf.org:1026/js/inltest/test_build.js";
	
	static cString cookie = Path::format("%s/cookie.txt", *Path::temp());
	
	CURL* _curl = curl_easy_init();
	
	XX_ASSERT(_curl);
	
	char  _curl_errbuf[256];
	struct curl_slist* chunk = NULL;
	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;
	
	memset(_curl_errbuf, 0, 256);
	
	curl_easy_setopt(_curl, CURLOPT_URL, *url);
	curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _curl_errbuf);
	curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(_curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 100L);
	
	// set header
	chunk = curl_slist_append(chunk, "accept: text/html,application/xhtml+xml,application/xml");
	chunk = curl_slist_append(chunk, "accept-language: en,en-US;q=0.8,zh-CN;q=0.6,zh");
	curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, chunk);
	curl_easy_setopt(_curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
	curl_easy_setopt(_curl, CURLOPT_USERAGENT, *HttpHelper::user_agent());
	// set header end
	
	// set cookie save path
	curl_easy_setopt(_curl, CURLOPT_COOKIEJAR, *cookie);
	curl_easy_setopt(_curl, CURLOPT_COOKIEFILE, *cookie);
	// set cookie end
	
	// 进度
	curl_easy_setopt(_curl, CURLOPT_XFERINFOFUNCTION, req_progress_cb);
	curl_easy_setopt(_curl, CURLOPT_PROGRESSDATA, NULL);
	
	// 收到头部信息
	curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, req_header_cb);
	curl_easy_setopt(_curl, CURLOPT_HEADERDATA, NULL);
	
	// 下载数据写入到本地
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, req_writer_cb);
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA, NULL);
	
	// 读取数据写入到服务器
	curl_easy_setopt(_curl, CURLOPT_READFUNCTION, req_reader_cb);
	curl_easy_setopt(_curl, CURLOPT_READDATA, NULL);
	
	//
//  curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);  // get
	curl_easy_setopt(_curl, CURLOPT_POST, 1L);      // post
//  curl_easy_setopt(_curl, CURLOPT_NOBODY, 1L);    // head
//  curl_easy_setopt(_curl, CURLOPT_CUSTOMREQUEST, "DELETE"); //delete
//
//  curl_easy_setopt(_curl, CURLOPT_UPLOAD, 1L);    // put
//  curl_easy_setopt(_curl, CURLOPT_PUT, 1L);       // put
	
//  curl_easy_setopt(_curl, CURLOPT_USERNAME, "");
//  curl_easy_setopt(_curl, CURLOPT_PASSWORD, "");
	
//  CURLOPT_DEBUGFUNCTION
	
	// post data
	curl_formadd(&formpost,
							 &lastptr,
							 CURLFORM_COPYNAME, "upload",
							 CURLFORM_FILE, *cookie,
							 CURLFORM_END);
	curl_formadd(&formpost,
							 &lastptr,
							 CURLFORM_COPYNAME, "upload1",
							 CURLFORM_FILE, *cookie,
							 CURLFORM_END);
	curl_formadd(&formpost,
							 &lastptr,
							 CURLFORM_COPYNAME, "name",
							 CURLFORM_COPYCONTENTS, "楚学文",
							 CURLFORM_END);
	curl_easy_setopt(_curl, CURLOPT_HTTPPOST, formpost);
//  curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, "logintype=uid&u=xieyan&psw=xxx86");
	
	// post data end
	
	CURLcode rc = curl_easy_perform(_curl);
	
	XX_ASSERT(! rc );
		
	curl_easy_cleanup(_curl);
	curl_slist_free_all(chunk);
	curl_formfree(formpost);
}
