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

#include "quark/util/util.h"
#include "quark/util/fs.h"
#include "quark/util/loop.h"
#include <uv.h>

using namespace quark;

static uv_loop_t* uv_loop = nullptr;

// ----------------------------- test uv file -----------------------------

static uv_fs_t open_req;
static uv_fs_t read_req;
static uv_fs_t close_req;
static uv_buf_t buffer;

void on_read(uv_fs_t *req);
void on_open(uv_fs_t *req);

void on_close(uv_fs_t *req) {
	Qk_LOG("close file ok");
	uv_fs_req_cleanup(req);
}

void on_read(uv_fs_t *req) {
	if (req->result < 0) {
		Qk_ERR("Read error: %s, %s\n", uv_err_name((int)req->result), uv_strerror((int)req->result));
	}
	else if (req->result == 0) {
		// asynchronous
		uv_fs_close(uv_loop, &close_req, (int)open_req.result, &on_close);
	}
	else {
		Qk_LOG(String(buffer.base, (int)req->result));
		uv_fs_read(uv_loop, req, (int)open_req.result, &buffer, 1, -1, on_read);
	}
	uv_fs_req_cleanup(req);
}

void on_open(uv_fs_t *req) {
	if (req->result > 0) {
		uv_fs_read(uv_loop, &read_req, (int)open_req.result, &buffer, 1, -1, on_read);
	}
	else {
		Qk_ERR("error opening file: %s, %s\n", uv_err_name((int)req->result), uv_strerror((int)req->result));
	}
	uv_fs_req_cleanup(req);
}

void test_uv_file() {
	Qk_LOG("test uv file:");
	buffer.base = (char*)malloc(1024);
	buffer.len = 1024;
	uv_fs_open(uv_loop, &open_req, fs_fallback_c(fs_resources("res/bg.svg")), O_RDONLY, 0, on_open);
	
	uv_run(uv_loop, UV_RUN_DEFAULT); // run loop
	Qk_LOG("test uv file ok");
}

// ----------------------------- test uv async check idle -----------------------------

static uv_idle_t uv_idler_handle;
static uv_check_t uv_check_handle;
static uv_async_t uv_async_handle;

void test_uv_idle_cb(uv_idle_t* handle) {
	Qk_LOG("idle");
	uv_idle_stop(handle);
	uv_close((uv_handle_t*)handle, nullptr);
}

void test_uv_check_cb(uv_check_t* handle) {
	Qk_LOG("CHECK");
	uv_check_stop(handle);
	uv_close((uv_handle_t*)handle, nullptr);
}

void test_uv_async_cb(uv_async_t* handle) {
	Qk_LOG("ASYNC");
	static int i = 5;
	i--;
	if (!i) {
		uv_close((uv_handle_t*)handle, nullptr);
	}
}

void test_uv_async_check_idle() {
	Qk_LOG("test uv async:");
	uv_idle_init(uv_loop, &uv_idler_handle);
	uv_idle_start(&uv_idler_handle, &test_uv_idle_cb);
	uv_check_init(uv_loop, &uv_check_handle);
	uv_check_start(&uv_check_handle, test_uv_check_cb);
	uv_async_init(uv_loop, &uv_async_handle, test_uv_async_cb);
	
	Thread::create([](Thread& t, void* arg) {
		Qk_LOG("Send message:");
		for ( int i = 0; i < 5; i++ ) {
			Thread::sleep(1e6);
			uv_async_send(&uv_async_handle);
		}
		Qk_LOG("Send message ok");
	}, nullptr, "test");
	
	uv_run(uv_loop, UV_RUN_DEFAULT); // run loop
	Qk_LOG("test uv async ok");
}

// ----------------------------- test uv timer -----------------------------

static uv_timer_t uv_timer_handle;

void test_uv_timer_cb(uv_timer_t* handle) {
	static int i = 0;
	Qk_LOG("test_uv_timer_cb, %d", i++);
}

void test_timer_uv() {
	Qk_LOG("test uv timer:");
	uv_timer_init(uv_loop, &uv_timer_handle);
	uv_timer_start(&uv_timer_handle, test_uv_timer_cb, 0, 0);
	uv_close((uv_handle_t*)&uv_timer_handle, nullptr);
	uv_run(uv_loop, UV_RUN_DEFAULT); // run loop
	Qk_LOG("test uv timer ok");
}

void test_uv(int argc, char **argv) {
	uv_loop = uv_loop_new();
	test_uv_file();
	test_uv_async_check_idle();
	test_timer_uv();
	uv_loop_delete(uv_loop);
}
