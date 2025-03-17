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

#include "quark/ui/app.h"
#include "quark/util/loop.h"

#ifndef TEST_FUNC_NAME
////
// #define TEST_FUNC_NAME test_atomic
// #define TEST_FUNC_NAME test_buffer
// #define TEST_FUNC_NAME test_event
// #define TEST_FUNC_NAME test_fs_async
// #define TEST_FUNC_NAME test_fs
// #define TEST_FUNC_NAME test_fs2
// #define TEST_FUNC_NAME test_http_cookie
// #define TEST_FUNC_NAME test_http
// #define TEST_FUNC_NAME test_http2
// #define TEST_FUNC_NAME test_http3
// #define TEST_FUNC_NAME test_https
// #define TEST_FUNC_NAME test_ios_run_loop
// #define TEST_FUNC_NAME test_json
// #define TEST_FUNC_NAME test_list
// #define TEST_FUNC_NAME test_localstorage
// #define TEST_FUNC_NAME test_loop
// #define TEST_FUNC_NAME test_map
// #define TEST_FUNC_NAME test_mutex
// #define TEST_FUNC_NAME test_net-ssl
// #define TEST_FUNC_NAME test_net
// #define TEST_FUNC_NAME test_number
// #define TEST_FUNC_NAME test_os_info
// #define TEST_FUNC_NAME test_sizeof
// #define TEST_FUNC_NAME test_ssl
// #define TEST_FUNC_NAME test_string
// #define TEST_FUNC_NAME test_thread
// #define TEST_FUNC_NAME test_util
// #define TEST_FUNC_NAME test_uv
// #define TEST_FUNC_NAME test_zlib
/// UI
// #define TEST_FUNC_NAME test_action
// #define TEST_FUNC_NAME test_alsa_ff
// #define TEST_FUNC_NAME test_blur
// #define TEST_FUNC_NAME test_canvas
// #define TEST_FUNC_NAME test_css
// #define TEST_FUNC_NAME test_draw_efficiency
// #define TEST_FUNC_NAME test_ffmpeg
// #define TEST_FUNC_NAME test_media
#define TEST_FUNC_NAME test_freetype
// #define TEST_FUNC_NAME test_gui
// #define TEST_FUNC_NAME test_input
// #define TEST_FUNC_NAME test_jsc
// #define TEST_FUNC_NAME test_jsx
// #define TEST_FUNC_NAME test_layout
// #define TEST_FUNC_NAME test_linux_input_2
// #define TEST_FUNC_NAME test_linux_input
// #define TEST_FUNC_NAME test_openurl
// #define TEST_FUNC_NAME test_outimg
// #define TEST_FUNC_NAME test_rrect
// #define TEST_FUNC_NAME test_subcanvas
// #define TEST_FUNC_NAME test_testing
// #define TEST_FUNC_NAME test_v8
#endif

using namespace qk;

void TEST_FUNC_NAME(int argc, char** argv);

Qk_Main() {

	uint64_t st = time_micro();

	TEST_FUNC_NAME(argc, argv);
	
	Qk_Log("Test eclapsed time:%dMs\n", (time_micro() - st) / 1000);

	return 0;
}

#if Qk_ANDROID
int main(int argc, char *argv[]) {
	Application::runMain(argc, argv);
	return 0;
}
#endif
