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

#include "./js.h"
#include "../app.h"

#if F_UNIX
# include <dlfcn.h>
#endif

extern int (*__fx_default_gui_main)(int, Char**);

JS_BEGIN

void* object_allocator_alloc(size_t size);
void  object_allocator_release(Object* obj);
void  object_allocator_retain(Object* obj);

// startup argv
Array<Char*>* __fx_flare_argv = nullptr;
int __fx_flare_have_node = 1;
int __fx_flare_have_debug = 0;

// parse argv
static void parseArgv(const Array<String> argv_in, Array<Char*>& argv, Array<Char*>& flare_argv) {
	static String argv_str;

	F_ASSERT(argv_in.length(), "Bad start argument");
	__fx_flare_have_node = 1;
	__fx_flare_have_debug = 0;
	argv_str = argv_in[0];
	Array<int> indexs = {-1};

	for (int i = 1, index = argv_in[0].length(); i < argv_in.length(); i++) {
		if (argv_in[i].index_of("--no-node") == 0) { // flare arg
			__fx_flare_have_node = 0; // disable node
		} else if (argv_in[i].index_of("--debug") == 0) {
			__fx_flare_have_debug = 1;
		} else {
			if (argv_in[i].index_of("--inspect") == 0) {
				__fx_flare_have_debug = 1;
			}
			argv_str.push(' ').push(argv_in[i]);
			indexs.push(index);
			index += argv_in[i].length() + 1;
		}
	}

	Char* str_c = const_cast<Char*>(*argv_str);
	argv.push(str_c);
	flare_argv.push(str_c);

	for (int i = 1, flare_ok = 0; i < indexs.length(); i++) {
		int index = indexs[i];
		str_c[index] = '\0';
		Char* arg = str_c + index + 1;
		if (flare_ok || arg[0] != '-') {
			flare_ok = 1; // flare argv start
			flare_argv.push(arg);
		}
		argv.push(arg);
	}
}

static void on_process_safe_handle(Event<>& e, Object* data) {
	int rc = static_cast<const Int*>(e.data())->value;
	if (RunLoop::main_loop()->runing()) {
		typedef Callback<RunLoop::PostSyncData> Cb;
		RunLoop::main_loop()->post_sync(Cb([&](Cb::Data& e) {
			auto worker = Worker::worker();
			F_DEBUG("on_process_safe_handle");
			if (worker) {
				rc = IMPL::inl(worker)->TriggerExit(rc);
			}
			e.data->complete();
		}));
	}
	e.return_value = rc;
}

int Start(cString& cmd) {
	Array<String> argv_in;
	for (auto& i : cmd.trim().split(' ')) {
		String arg = i.value().trim();
		if (!arg.is_empty())
			argv_in.push(arg);
	}
	return Start(argv_in);
}

int Start(const Array<String>& argv_in) {
	static int is_start_initializ = 0;
	if ( is_start_initializ++ == 0 ) {
		HttpHelper::initialize();
		Object::set_object_allocator(
			&object_allocator_alloc, &object_allocator_release, &object_allocator_retain);
	}
	F_ASSERT(!__fx_flare_argv);

	Array<Char*> argv, flare_argv;
	parseArgv(argv_in, argv, flare_argv);

	Thread::F_On(SafeExit, on_process_safe_handle);

	__fx_flare_argv = &flare_argv;
	int rc = 0;
	int argc = argv.length();
	Char** argv_c = const_cast<Char**>(&argv[0]);

	// Mark the current main thread and check current thread
	F_ASSERT(RunLoop::main_loop() == RunLoop::current());

	if (__fx_flare_have_node ) {
		if (node::node_api) {
			rc = node::node_api->start(argc, argv_c);
		} else {
			#if F_LINUX
				// try loading nxnode
				void* handle = dlopen("libflare-node.so", RTLD_LAZY | RTLD_GLOBAL);
				if (!handle) {
					F_WARN("No node library loaded, %s", dlerror());
					goto no_node_start;
				} else {
					rc = node::node_api->start(argc, argv_c);
				}
			#else
				F_WARN("No node library loaded");
				goto no_node_start;
			#endif
		}
	} else {
	 no_node_start:
		__fx_flare_have_node = 0;
		rc = IMPL::start(argc, argv_c);
	}
	__fx_flare_argv = nullptr;
	Thread::F_Off(SafeExit, on_process_safe_handle);

	return rc;
}

int Start(int argc, Char** argv) {
	Array<String> argv_in;
	for (int i = 0; i < argc; i++) {
		argv_in.push(argv[i]);
	}
	return Start(argv_in);
}

/**
 * @func __default_main
 */
int __default_main(int argc, Char** argv) {
	String cmd;

	#if F_ANDROID
		cmd = API::start_cmd();
		if ( cmd.is_empty() )
	#endif 
	{
		FileReader* reader = FileReader::shared();
		String index = Path::resources("index");
		Array<String> ls = String(reader->read_file_sync( index )).split('\n');
	
		for ( int i = 0; i < ls.length(); i++ ) {
			String s = ls[i].trim();
			if ( s[0] != '#' ) {
				cmd = s;
				break;
			}
		}
	}

	if ( cmd.is_empty() ) {
		return argc != 0 ? Start(argc, argv): 0;
	} else {
		return Start(cmd);
	}
}

F_INIT_BLOCK(__default_main) {
	__fx_default_gui_main = __default_main;
}