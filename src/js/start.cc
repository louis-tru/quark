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

#include "./_js.h"
#include "../ui/app.h"

namespace qk { namespace js {

	void* object_allocator_alloc(size_t size);
	void  object_allocator_free(void *ptr);
	void  object_allocator_strong(Object* obj);
	void  object_allocator_weak(Object* obj);

	// startup argv
	Array<Char*>* __quark_js_argv = nullptr;
	int           __quark_js_have_debug = 0;

	// parse argv
	static void parseArgv(const Array<String> argv_in, Array<Char*>& argv, Array<Char*>& quark_argv) {
		static String argv_str;

		Qk_ASSERT(argv_in.length(), "Bad start argument");
		__quark_js_have_debug = 0;
		argv_str = argv_in[0];
		Array<int> indexs = {-1};

		for (int i = 1, index = argv_in[0].length(); i < argv_in.length(); i++) {
			if (argv_in[i].indexOf("--debug") == 0) {
				__quark_js_have_debug = 1;
			} else {
				if (argv_in[i].indexOf("--inspect") == 0) {
					__quark_js_have_debug = 1;
				}
				argv_str.append(' ');
				argv_str.append(argv_in[i]);
				indexs.push(index);
				index += argv_in[i].length() + 1;
			}
		}

		Char* str_c = const_cast<Char*>(*argv_str);
		argv.push(str_c);
		quark_argv.push(str_c);

		for (int i = 1, quark_ok = 0; i < indexs.length(); i++) {
			int index = indexs[i];
			str_c[index] = '\0';
			Char* arg = str_c + index + 1;
			if (quark_ok || arg[0] != '-') {
				quark_ok = 1; // quark argv start
				quark_argv.push(arg);
			}
			argv.push(arg);
		}
	}

	static void onProcessSafeHandle(Event<>& e) {
		int rc = static_cast<const Int32*>(e.data())->value;
		if (RunLoop::first()->runing()) {
			typedef Callback<RunLoop::PostSyncData> Cb;
			RunLoop::first()->post_sync(Cb([&](Cb::Data& e) {
				auto worker = Worker::worker();
				Qk_DEBUG("onProcessSafeHandle");
				if (worker) {
					rc = triggerExit(worker, rc);
				}
				e.data->complete();
			}));
		}
		e.return_value = rc;
	}

	int Start(const Array<String>& argv_in) {
		Qk_ASSERT(!__quark_argv);

		Object::setAllocator(
			&object_allocator_alloc, &object_allocator_free, &object_allocator_strong, &object_allocator_weak);

		Array<char*> argv, quark_argv;
		parseArgv(argv_in, argv, quark_argv);

		Qk_On(ProcessExit, onProcessSafeHandle);

		__quark_js_argv = &quark_argv;

		// Mark the current main thread and check current thread
		Qk_ASSERT(RunLoop::first() == RunLoop::current());

		char** argv_c = const_cast<char**>(&argv[0]);
		int rc = platformStart(argv.length(), argv_c);
		__quark_js_argv = nullptr;

		Qk_Off(ProcessExit, onProcessSafeHandle);
	
		// Object::setAllocator(nullptr, nullptr, nullptr, nullptr);

		return rc;
	}

	Qk_Main() {
		String cmd;

		#if Qk_ANDROID
			cmd = API::start_cmd();
			if ( cmd.isEmpty() )
		#endif
		{
			auto arr = String(FileReader::shared()->read_file_sync( fs_resources("index") )).split('\n');

			for ( int i = 0; i < arr.length(); i++ ) {
				auto s = arr[i].trim();
				if ( s[0] != '#' ) {
					cmd = s;
					break;
				}
			}
		}

		Array<String> argvArr;

		if ( cmd.isEmpty() ) {
			if (argc > 0) {
				for (int i = 0; i < argc; i++)
					argvArr.push(argv[i]);
			}
		} else {
			for (auto& i : cmd.trim().split(' ')) {
				auto arg = i.trim();
				if (!arg.isEmpty()) argvArr.push(arg);
			}
		}

		return argvArr.length() ? js::Start(argvArr): 0;
	}

} }