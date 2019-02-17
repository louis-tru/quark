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

#include "qgr/app.h"
#include "qgr/view.h"
#include "qgr/js/qgr.h"
#include "qgr/utils/http.h"
#include "binding/event-1.h"
#if XX_ANDROID
# include "android/android.h"
#endif
#include "depe/node/src/qgr.h"

extern int (*__xx_default_gui_main)(int, char**);

/**
 * @ns qgr::js
 */

JS_BEGIN

void WrapViewBase::destroy() {
	GUILock lock;
	delete this;
}

template<class T, class Self>
static void add_event_listener_1(Wrap<Self>* wrap, const GUIEventName& type, 
																 cString& func, int id, Cast* cast = nullptr) 
{
	auto f = [wrap, func, cast](typename Self::EventType& evt) {
		// if (worker()->is_terminate()) return;
		HandleScope scope(wrap->worker());
		// arg event
		Wrap<T>* ev = Wrap<T>::pack(static_cast<T*>(&evt), JS_TYPEID(T));

		if (cast)
			ev->set_private_data(cast); // set data cast func
		
		Local<JSValue> args[2] = { ev->that(), wrap->worker()->New(true) };
		// call js trigger func
		Local<JSValue> r = wrap->call( wrap->worker()->New(func,1), 2, args );
		
		// test:
		//if (r->IsNumber(worker)) {
		//  LOG("--------------number,%s", *r->ToStringValue(wrap->worker()));
		//} else {
		//  LOG("--------------string,%s", *r->ToStringValue(wrap->worker()));
		//}
	};
	
	Self* self = wrap->self();
	self->add_event_listener(type, f, id);
}

bool WrapViewBase::add_event_listener(cString& name_s, cString& func, int id) 
{
	auto i = GUI_EVENT_TABLE.find(name_s);
	if ( i.is_null() ) {
		return false;
	}
	
	GUIEventName name = i.value();
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
	
	switch ( name.category() ) {
		case GUI_EVENT_CATEGORY_CLICK:
			add_event_listener_1<GUIClickEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_KEYBOARD:
			add_event_listener_1<GUIKeyEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_MOUSE:
		 add_event_listener_1<GUIMouseEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_TOUCH:
			add_event_listener_1<GUITouchEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_HIGHLIGHTED:
			add_event_listener_1<GUIHighlightedEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_ACTION:
			add_event_listener_1<GUIActionEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_FOCUS_MOVE:
			add_event_listener_1<GUIFocusMoveEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_ERROR:
			add_event_listener_1<GUIEvent>(wrap, name, func, id, Cast::entity<Error>()); break;
		case GUI_EVENT_CATEGORY_FLOAT:
			add_event_listener_1<GUIEvent>(wrap, name, func, id, Cast::entity<Float>()); break;
		case GUI_EVENT_CATEGORY_UINT64:
			add_event_listener_1<GUIEvent>(wrap, name, func, id, Cast::entity<Uint64>()); break;
		case GUI_EVENT_CATEGORY_DEFAULT:
			add_event_listener_1<GUIEvent>(wrap, name, func, id); break;
		default:
			return false;
	}
	return true;
}

bool WrapViewBase::remove_event_listener(cString& name, int id) {
	auto i = GUI_EVENT_TABLE.find(name);
	if ( i.is_null() ) {
		return false;
	}
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
	wrap->self()->remove_event_listener(i.value(), id); // off event listener
	return true;
}

// -------------------------------------------------------------------------------------

void open_rlog(cString& r_url);
void close_rlog();
void* object_allocator_alloc(size_t size);
void object_allocator_release(Object* obj);
void object_allocator_retain(Object* obj);

/**
 * @class QgrApiImpl
 */
class QgrApiImpl: public node::QgrApi {
 public:

	Worker* create_worker(node::Environment* env, bool is_inspector,
												int argc, const char* const* argv) {
		if (argc > 1) {
			Map<String, String> opts;
			for (int i = 2; i < argc; i++) {
				String arg = argv[i];
				if ( arg[0] == '-' ) {
					Array<String> ls = arg.split('=');
					opts.set( ls[0].substr(arg[1] == '-' ? 2: 1), ls.length() > 1 ? ls[1] : String() );
				}
			}
			if (opts.has("rlog")) {
				open_rlog(opts["rlog"]);
			} else if (is_inspector || opts.has("dev")) {
				open_rlog(argv[1]);
			}
		}
		return new Worker();
	}

	void delete_worker(qgr::js::Worker* worker) {
		Release(worker);
	}

	void run_loop() {
		qgr::RunLoop::main_loop()->run();
	}

	char* encoding_to_utf8(const uint16_t* src, int length, int* out_len) {
		auto buff = Codec::encoding(Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}

	uint16_t* decoding_utf8_to_uint16(const char* src, int length, int* out_len) {
		auto buff = Codec::decoding_to_uint16(Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}

	void print(const char* msg, ...) {
		XX_STRING_FORMAT(msg, str);
		LOG(str);
	}

	bool is_process_exit() {
		return RunLoop::is_process_exit();
	}
};

// startup argv
static Array<char*>* worker_start_argv = nullptr;
static int is_start_initializ = 0;

int start(cString& argv_str) {
	if ( is_start_initializ++ == 0 ) {
		HttpHelper::initialize();

		node::set_qgr_api(new QgrApiImpl);

		ObjectAllocator allocator = {
			object_allocator_alloc, object_allocator_release, object_allocator_retain,
		};
		qgr::set_object_allocator(&allocator);

		// qgr::set_ssl_root_x509_store_function(node::crypto::NewRootCertStore);
	}

	RunLoop* loop = RunLoop::main_loop();
	
	static String str = argv_str.trim();
	// add prefix
	if (argv_str.index_of("qgr") != 0) {
		str = String("qgr ") + str;
	}
	
	char* str2 = const_cast<char*>(*str);
	Array<char*> argv = { str2 };
	
	int index = 0;
	while ((index = str.index_of(" ", index)) != -1) {
		str2[index] = '\0';
		index++;
		argv.push(str2 + index);
	}

	XX_CHECK(!worker_start_argv);
	worker_start_argv = &argv;
	
	int code = 0;
	
	// TODO
	// int code = node::Start(argv.length(), const_cast<char**>(&argv[0]));

	worker_start_argv = nullptr;

	return code;
}

/**
 * @func __default_main
 */
int __default_main(int argc, char** argv) {
	String path;

 #if XX_ANDROID
	path = Android::start_path();
	if ( path.is_empty() )
 #endif
	{
		FileReader* reader = FileReader::shared();
		path = Path::resources("index");
		Array<String> ls = String(reader->read_file_sync( path )).split('\n');
		path = String();
	
		for ( uint i = 0; i < ls.length(); i++ ) {
			String s = ls[i].trim();
			if ( s[0] != '#' ) {
				path = s;
				break;
			}
		}
	}
	if ( ! path.is_empty() ) {
		return start(path);
	}

	return 0;
}

XX_INIT_BLOCK(__default_main) {
	__xx_default_gui_main = __default_main;
}

JS_END
