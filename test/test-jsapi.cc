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

#if !defined(USE_JS)
void test_jsapi(int argc, char **argv) {}
#else
#include "src/js/js.h"
#include "src/util/fs.h"

using namespace qk;

#define IP_REMOTE "127.0.0.1"
#define USE_REMOTE 0
#define USE_INSPECT 1

void test_jsapi(int argc, char **_) {
	Array<String> argv;

	for (int i = 1; i < argc; i++)
		argv.push(_[i]);

#if USE_INSPECT
	argv.push("--inspect-brk=0.0.0.0:9229");
#endif

#if USE_REMOTE
	js::Start("http://" IP_REMOTE ":1026/jsapi", argv);
#else
	js::Start(fs_resources("jsapi"), argv);
#endif
}
#endif

#if Qk_ANDROID
#include <src/platforms/android/android.h>
extern "C" {
	JNIEXPORT extern void
	Java_org_quark_examples_MainActivity_test(JNIEnv *env, jclass clazz, jint count) {
		Qk_Log("Java_org_quark_examples_MainActivity_test");
	}
}
#endif
