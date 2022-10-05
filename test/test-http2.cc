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

#include <quark/util/util.h>
#include <quark/util/http.h>
#include <quark/util/string.h>
#include <quark/util/fs.h>

using namespace quark;

class MyClient: public HttpClientRequest, HttpClientRequest::Delegate {
 public:
	MyClient(): HttpClientRequest( ), count(0) {
		set_delegate(this);
	}
	
	virtual void trigger_http_error(HttpClientRequest* req, cError& error) {
		Qk_LOG("trigger_http_error, %s", *error.message());
	}
	virtual void trigger_http_write(HttpClientRequest* req) {
		Qk_LOG("Write, %d/%d, %d/%d", download_size(), download_total(), upload_size(), upload_total());
	}
	virtual void trigger_http_header(HttpClientRequest* req) {
		Qk_LOG("Header: %d", status_code());
		for ( auto& i : get_all_response_headers() ) {
			Qk_LOG("  %s: %s", i.key.c_str(), i.value.c_str());
		}
		Qk_LOG("");
	}
	virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) {
		Qk_LOG("Read, %d/%d, %d/%d", download_size(), download_total(), upload_size(), upload_total());
		Qk_LOG( String(buffer.val(), buffer.length()) );
	}
	virtual void trigger_http_end(HttpClientRequest* req) {
		Qk_LOG("http_end, status: %d, %s", status_code(), url().c_str());
		// LOG( fs_read_file_sync(fs_documents("http.cc")) );
		release();
		//RunLoop::current()->stop();
	}
	virtual void trigger_http_readystate_change(HttpClientRequest* req) {
		Qk_LOG("http_readystate_change, %d", ready_state() );
	}
	virtual void trigger_http_timeout(HttpClientRequest* req) {
		Qk_LOG("trigger_http_timeout" );
	}
	virtual void trigger_http_abort(HttpClientRequest* req) {
		Qk_LOG("trigger_http_abort" );
	}
	
	int count;
	
};

void test_http2(int argc, char **argv) {
	
	MyClient* cli = new MyClient();
	
	String url = "https://www.baidu.com";
	cli->set_method(HTTP_METHOD_GET);
	cli->set_url(url);
	//cli->set_keep_alive(false);
	//req->disable_cache(true);
	//req->disable_cookie(true);
	//req->set_save_path(fs_documents("http.cc"));
	cli->send();
	
	RunLoop::current()->run();
}

