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

#include "noug/js/noug.h"

using namespace noug;

#define IP_REMOTE "127.0.0.1"
#define USE_REMOTE 0
#define USE_INSPECT 1
#define USE_NODE 1

static bool has_argv(cchar* name, int argc, char **argv) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], name) == 0) {
			return 1;
		}
	}
	return 0;
}

void test_demo(int argc, char **argv) {
	String cmd = "noug ";

#if USE_NODE
	cmd += " ";
#else
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--no-node") == 0) {
			cmd += "--no-node "; break;
		}
	}
#endif

#if USE_INSPECT
	cmd += "--inspect-brk=0.0.0.0:9229 ";
#else
	if (has_argv("--inspect", argc, argv)) {
		cmd += "--inspect=0.0.0.0:9229 ";
	} else if (has_argv("--inspect-brk", argc, argv)) {
		cmd += "--inspect-brk=0.0.0.0:9229 ";
	}
#endif

	char* mod = nullptr;
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == 'h' && argv[i][1] == 't' && argv[i][2] == 't' && argv[i][3] == 'p') {
			mod = argv[i];
			break;
		}
	}
	if (mod) {
		cmd += mod;
		cmd += ' ';
	} else {
#if USE_REMOTE
		cmd += "http://" IP_REMOTE ":1026/examples ";
#else
		cmd += "examples ";
#endif
	}

	for (int i = 1; i < argc; i++) {
		// LOG("%s, %s", argv[i], strstr(argv[i], "--"));
		if (strstr(argv[i], "--") == argv[i] && 
			strcmp(argv[i], "--no-node") != 0 && 
			strstr(argv[i], "--inspect") != argv[i]) 
		{
			cmd += argv[i];
			cmd += ' ';
		}
	}

	N_LOG(cmd);

	js::Start(cmd);
}

extern "C" {

#if FX_ANDROID
#include <noug/util/android-jni.h>
	JNIEXPORT extern void
	Java_org_noug_examples_MainActivity_test(JNIEnv *env, jclass clazz, jint count) {
		N_LOG("Java_org_noug_examples_MainActivity_test");
	}
#endif
}
