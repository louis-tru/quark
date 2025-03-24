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

#define Func_Tables(F) \
	F(atomic) \
	F(buffer) \
	F(event) \
	F(fs_async) \
	F(fs) \
	F(fs2) \
	F(http_cookie) \
	F(http) \
	F(http2) \
	F(http3) \
	F(https) \
	F(ios_run_loop) \
	F(json) \
	F(list) \
	F(localstorage) \
	F(loop) \
	F(map) \
	F(mutex) \
	F(net_ssl) \
	F(net) \
	F(number) \
	F(os_info) \
	F(sizeof) \
	F(ssl) \
	F(string) \
	F(thread) \
	F(util) \
	F(uv) \
	F(zlib) \
	/* UI*/ \
	F(action) \
	F(alsa_ff) \
	F(blur) \
	F(canvas) \
	F(css) \
	F(draw_efficiency) \
	F(ffmpeg) \
	F(media) \
	F(freetype) \
	F(gui) \
	F(input) \
	F(jsc) \
	F(jsx) \
	F(layout) \
	F(linux_input_2) \
	F(linux_input) \
	F(openurl) \
	F(outimg) \
	F(rrect) \
	F(subcanvas) \
	F(jsapi) \
	F(v8) \

#define DEFAULT_CALLS(F) \
	F(v8) \

#define _Fun(n) \
	void test_##n(int argc, char** argv);
Func_Tables(_Fun)
#undef _Fun

using namespace qk;

Qk_Main() {
	uint64_t st = time_micro();

	#define _Fun(n) else if (strcmp(argv[1], #n) == 0) { test_##n(argc, argv); }
	#define _Fun2(n) (test_ ## n)(argc, argv);

	if (argc < 2) {
		DEFAULT_CALLS(_Fun2);
	}
	Func_Tables(_Fun)
	else {
		DEFAULT_CALLS(_Fun2);
	}

	Qk_Log("Test eclapsed time:%dMs\n", (time_micro() - st) / 1000);
	return 0;
}

#if Qk_ANDROID
int main(int argc, char *argv[]) {
	Application::runMain(argc, argv);
	return 0;
}
#endif
