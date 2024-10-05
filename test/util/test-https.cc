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

#include <quark/util/util.h>
#include <quark/util/http.h>
#include <quark/util/string.h>
#include <quark/util/fs.h>
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/view/root.h>

using namespace qk;

void test_download(HttpClientRequest *cl);

class TestHttpClient: public HttpClientRequest, HttpClientRequest::Delegate {
public:
	TestHttpClient() {
		set_delegate(this);
	}
	void trigger_http_error(HttpClientRequest* req, cError& error) {
		Qk_Log("trigger_http_error, %s", *error.message());
	}
	void trigger_http_write(HttpClientRequest* req) {
		Qk_Log("Write, %d/%d, %d/%d", download_size(), download_total(), upload_size(), upload_total());
	}
	void trigger_http_header(HttpClientRequest* req) {
		Qk_Log("Header: %d", status_code());
		for ( auto& i : get_all_response_headers() ) {
			//Qk_Log("  %s: %s", i.key.c_str(), i.value.c_str());
		}
		Qk_Log("");
	}
	void trigger_http_data(HttpClientRequest* req, Buffer &buffer) {
		//Qk_Log("Read, %d/%d, %d/%d", download_size(), download_total(), upload_size(), upload_total());
		//Qk_Log("http ondata: %s", buffer.val());
	}
	void trigger_http_end(HttpClientRequest* req) {
		pause();
		resume();
		abort();

		Qk_Log("http_end, status: %d, %s", status_code(), url().c_str());
		//Qk_Log(fs_read_file_sync(fs_documents("baidu.html")));

		test_download(this);
		//send();
	}
	void trigger_http_readystate_change(HttpClientRequest* req) {
		Qk_Log("http_readystate_change, %d", ready_state() );
	}
	void trigger_http_timeout(HttpClientRequest* req) {
		Qk_Log("trigger_http_timeout" );
	}
	void trigger_http_abort(HttpClientRequest* req) {
		Qk_Log("trigger_http_abort" );
	}
};

void test_https(int argc, char **argv) {
	Qk_Log("\nHttpClientRequest:\n");

	Sp<TestHttpClient> cl = new TestHttpClient();

	cl->set_method(HTTP_METHOD_GET);
	cl->set_url("https://www.baidu.com");
	cl->set_save_path(fs_documents("baidu.html"));
	cl->set_username("louis");
	cl->set_password("Alsk106612");
	cl->clear_request_header();
	cl->clear_form_data();
	cl->set_request_header("test_set_request_header", "test");
	cl->get_response_header("expires");
	cl->get_all_response_headers();
	//cl->set_keep_alive(false);
	cl->set_timeout(10000000); // 10s
	// cl->disable_cache(true);
	cl->disable_cookie(true);
	cl->disable_send_cookie(true);
	cl->pause();
	cl->resume();
	cl->abort();
	cl->send();

	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {500,500}}});
	win->activate();
	win->root()->set_background_color(Color(255, 0, 0));

	RunLoop::current()->run();
}

void test_download(HttpClientRequest *cl) {
	Qk_Log("\nTest test_download:\n");

	cl->set_url("https://github.com/louis-tru/quark/blob/master/doc/index.md");
	cl->set_save_path(""); // no save path
	//cl->disable_cookie(false);
	//cl->disable_send_cookie(false);
	cl->set_method(HTTP_METHOD_GET);
	cl->disable_cache(true);
	cl->send();
}
