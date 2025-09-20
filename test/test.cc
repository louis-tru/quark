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

#include "src/ui/app.h"
#include "src/util/loop.h"
#include "src/util/string.h"
#include "./test.h"

using namespace qk;

class TestLog;
static TestLog* testLog;

class TestLog: public Log {
public:
	TestLog(): _indentation(0) {
		testLog = this;
	}
	void log(cChar* log, cChar* end) {
		Log::log(String::format("%s%s", _indentation_s, log).c_str(), end);
	}
	void set_indentation(int v) {
		_indentation = v;
		memset(_indentation_s, 32, v);
		_indentation_s[v] = '\0';
	}
	void set_funcName(cChar* name) {
		_funcName = name;
	}
	static void Assert(const char* tag, bool cond, const char* msg, ...) {
		if (Qk_UNLIKELY(!(cond))) {
			if (msg) {
				va_list __arg;
				va_start(__arg, msg);
				String str = _Str::printfv(msg, __arg);
				va_end(__arg);
				Qk_Log("  %s %s \033[0;31mfail\033[0m, %s", testLog->_funcName, tag, str.c_str());
			} else {
				Qk_Log("  %s \033[0;31m%s fail", testLog->_funcName, tag);
			}
			abort();
		} else {
			Qk_Log("  %s %s \033[0;32mok", testLog->_funcName, tag);
		}
	}
private:
	const char * _funcName = "";
	int _indentation;
	char _indentation_s[128];
};

void call_test(int argc, char** argv, const char* funcName, TestFunc func) {
	Qk_Log("Test %s:", funcName);

	testLog->set_indentation(2);

	uint64_t st = time_micro();

	func(argc, argv, funcName, TestLog::Assert);

	testLog->set_indentation(0);

	Qk_Log("\nTest %s eclapsed, time: %dMs\n", funcName, (time_micro() - st) / 1000);
}

//-----------------------------------------------------------------------------

#define TEST_UTILS(F) \
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

#define TEST_All(F) \
	TEST_UTILS(F) \
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
	F(spine) \
	F(little_border) \

#define _Fun(n) Qk_TEST_Func(n);
TEST_All(_Fun)
#undef _Fun

Qk_Main() {
	#define _Fun(n) else if (strcmp(argv[1], #n) == 0) { call_test(argc, argv, #n, test_##n); }
	#define _Fun2(n) call_test(argc, argv, #n, test_##n);

	// #define TEST_DEFAULT(F) TEST_UTILS(F) // test all of utils
	// #define TEST_DEFAULT(F) TEST_All(F) // test all of tests
	#define TEST_DEFAULT(F) F(spine) // Only test jsapi

	Log::set_shared(new TestLog());

	if (argc < 2) {
		TEST_DEFAULT(_Fun2);
	}
	TEST_All(_Fun)
	else {
		TEST_DEFAULT(_Fun2);
	}
	return 0;
}
