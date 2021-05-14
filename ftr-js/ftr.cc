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

#include "js-1.h"
#include "ftr/app.h"
#include "ftr/view.h"
#include "ftr-js/ftr.h"
#include "ftr/util/http.h"
#include "binding/event-1.h"
#include "android/android.h"
#include "native-inl-js.h"
#include "deps/node/src/ftr.h"
#include "uv.h"

#if FX_UNIX
# include <dlfcn.h>
#endif

extern int (*__fx_default_gui_main)(int, char**);

/**
 * @ns ftr::js
 */

JS_BEGIN

void WrapViewBase::destroy() {
	GUILock lock;
	delete this;
}

template<class T, class Self>
static void addEventListener_1(
	Wrap<Self>* wrap, const GUIEventName& type, cString& func, int id, Cast* cast = nullptr) 
{
	auto f = [wrap, func, cast](typename Self::EventType& evt) {
		auto worker = wrap->worker();
		JS_HANDLE_SCOPE();
		JS_CALLBACK_SCOPE();

		// arg event
		Wrap<T>* ev = Wrap<T>::pack(static_cast<T*>(&evt), JS_TYPEID(T));
		if (cast) 
			ev->setPrivateData(cast); // set data cast func
		Local<JSValue> args[2] = { ev->that(), worker->New(true) };
		
		DLOG("addEventListener_1, %s, EventType: %s", *func, *evt.name());

		// call js trigger func
		Local<JSValue> r = wrap->call( worker->New(func,1), 2, args );
	};
	
	Self* self = wrap->self();
	self->add_event_listener(type, f, id);
}

bool WrapViewBase::addEventListener(cString& name_s, cString& func, int id)
{
	auto i = GUI_EVENT_TABLE.find(name_s);
	if ( i.is_null() ) {
		return false;
	}
	GUIEventName name = i.value();
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
  
	switch ( name.category() ) {
		case GUI_EVENT_CATEGORY_CLICK:
			addEventListener_1<GUIClickEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_KEYBOARD:
			addEventListener_1<GUIKeyEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_MOUSE:
		 addEventListener_1<GUIMouseEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_TOUCH:
			addEventListener_1<GUITouchEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_HIGHLIGHTED:
			addEventListener_1<GUIHighlightedEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_ACTION:
			addEventListener_1<GUIActionEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_FOCUS_MOVE:
			addEventListener_1<GUIFocusMoveEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_ERROR:
			addEventListener_1<GUIEvent>(wrap, name, func, id, Cast::Entity<Error>()); break;
		case GUI_EVENT_CATEGORY_FLOAT:
			addEventListener_1<GUIEvent>(wrap, name, func, id, Cast::Entity<Float>()); break;
		case GUI_EVENT_CATEGORY_UINT64:
			addEventListener_1<GUIEvent>(wrap, name, func, id, Cast::Entity<Uint64>()); break;
		case GUI_EVENT_CATEGORY_DEFAULT:
			addEventListener_1<GUIEvent>(wrap, name, func, id); break;
		default:
			return false;
	}
	return true;
}

bool WrapViewBase::removeEventListener(cString& name, int id) {
	auto i = GUI_EVENT_TABLE.find(name);
	if ( i.is_null() ) {
		return false;
	}
	
	DLOG("removeEventListener, name:%s, id:%d", *name, id);
	
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
	wrap->self()->remove_event_listener(i.value(), id); // off event listener
	return true;
}

// -------------------------------------------------------------------------------------

void* object_allocator_alloc(size_t size);
void  object_allocator_release(Object* obj);
void  object_allocator_retain(Object* obj);

// startup argv
Array<char*>* __fx_ftr_argv = nullptr;
int           __fx_ftr_have_node = 1;
int           __fx_ftr_have_debug = 0;

// parse argv
static void parseArgv(const Array<String> argv_in, Array<char*>& argv, Array<char*>& ftr_argv) {
	static String argv_str;

	ASSERT(argv_in.length(), "Bad start argument");
	__fx_ftr_have_node = 1;
	__fx_ftr_have_debug = 0;
	argv_str = argv_in[0];
	Array<int> indexs = {-1};

	for (int i = 1, index = argv_in[0].length(); i < argv_in.length(); i++) {
		if (argv_in[i].index_of("--no-node") == 0) { // ftr arg
			__fx_ftr_have_node = 0; // disable node
		} else if (argv_in[i].index_of("--debug") == 0) {
			__fx_ftr_have_debug = 1;
		} else {
			if (argv_in[i].index_of("--inspect") == 0) {
				__fx_ftr_have_debug = 1;
			}
			argv_str.push(' ').push(argv_in[i]);
			indexs.push(index);
			index += argv_in[i].length() + 1;
		}
	}

	char* str_c = const_cast<char*>(*argv_str);
	argv.push(str_c);
	ftr_argv.push(str_c);

	for (int i = 1, ftr_ok = 0; i < indexs.length(); i++) {
		int index = indexs[i];
		str_c[index] = '\0';
		char* arg = str_c + index + 1;
		if (ftr_ok || arg[0] != '-') {
			ftr_ok = 1; // ftr argv start
			ftr_argv.push(arg);
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
			DLOG("on_process_safe_handle");
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
		ObjectAllocator allocator = {
			object_allocator_alloc, object_allocator_release, object_allocator_retain,
		};
		ftr::set_object_allocator(&allocator);
	}
	ASSERT(!__fx_ftr_argv);

	Array<char*> argv, ftr_argv;
	parseArgv(argv_in, argv, ftr_argv);

	Thread::FX_ON(ProcessSafeExit, on_process_safe_handle);

	__fx_ftr_argv = &ftr_argv;
	int rc = 0;
	int argc = argv.length();
	char** argv_c = const_cast<char**>(&argv[0]);

	// Mark the current main thread and check current thread
	ASSERT(RunLoop::main_loop() == RunLoop::current());

	if (__fx_ftr_have_node ) {
		if (node::node_api) {
			rc = node::node_api->start(argc, argv_c);
		} else {
#if FX_LINUX
			// try loading nxnode
			void* handle = dlopen("libftr-node.so", RTLD_LAZY | RTLD_GLOBAL);
			if (!handle) {
				FX_WARN("No node library loaded, %s", dlerror());
				goto no_node_start;
			} else {
				rc = node::node_api->start(argc, argv_c);
			}
#else
			FX_WARN("No node library loaded");
			goto no_node_start;
#endif
		}
	} else {
	 no_node_start:
		__fx_ftr_have_node = 0;
		rc = IMPL::start(argc, argv_c);
	}
	__fx_ftr_argv = nullptr;
	Thread::FX_OFF(ProcessSafeExit, on_process_safe_handle);

	return rc;
}

int Start(int argc, char** argv) {
	Array<String> argv_in;
	for (int i = 0; i < argc; i++) {
		argv_in.push(argv[i]);
	}
	return Start(argv_in);
}

/**
 * @func __default_main
 */
int __default_main(int argc, char** argv) {
	String cmd;

#if FX_ANDROID
	cmd = Android::start_cmd();
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

FX_INIT_BLOCK(__default_main) {
	__fx_default_gui_main = __default_main;
}

JS_END
