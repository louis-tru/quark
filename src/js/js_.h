// @private head
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

// @private head

#ifndef __quark__js__js___
#define __quark__js__js___

#include "./js.h"
#include "../util/codec.h"
#include "../util/fs.h"

namespace qk { namespace js {

	#define Js_On( name, block, id, ...) \
		Qk_On(name, [this,##__VA_ARGS__]( auto & evt) { qk::js::HandleScope scope(worker()); block }, id)

	#define Js_Native_On(name, func, id) \
		Js_On(name, { call(worker()->newStringOneByte(func)); }, id, func)

	#define Js_All_Modules(F)\
		F(_init)    F(_action)   F(_buffer)\
		F(_css)     F(_event)    F(_font)\
		F(_fs)      F(_http)     F(_os)\
		F(_storage) F(_types)    F(_ui)\
		F(_net)     F(_path)     F(_lmdb)\

	#define Js_Strings_Each(F)  \
		F(exports)         F(constructor)    F(__proto__)\
		F(prototype)       F(toStringStyled) F(window) \
		F(type)            F(kind)           F(value) \
		F(width)           F(height)         F(r) \
		F(g)               F(b)              F(a) \
		F(x)               F(y)              F(z) \
		F(w)               F(ended)          F(size) \
		F(toJSON)          F(status)         F(Errno) \
		F(url)             F(id)             F(startPosition) \
		F(position)        F(force)          F(clickIn) \
		F(view)            F(_data)          F(p1x) \
		F(p1y)             F(p2x)            F(p2y) \
		F(_change_touches) F(name)           F(pathname) \
		F(data)            F(total)          F(complete) \
		F(httpVersion)     F(statusCode)     F(responseHeaders) \
		F(_log) F(_warn) F(_error) F(_clear) F(_staticData) \
		F(p0) F(p1) F(p2) F(p3) F(p4) F(p5) F(p6) F(p7) F(axis) F(overlap) \

	#define Js_Format_Str(msg) \
		va_list arg; \
		va_start(arg, errmsg); \
		auto str = _Str::printfv(errmsg, arg); \
		va_end(arg) \

	extern Worker* first_worker;

	class Strings {
	public:
		Strings(Worker* worker);
	#define _Fun(name) \
	inline JSValue* name() { return *__##name##__; } \
	private: Persistent<JSValue> __##name##__; public:
		Js_Strings_Each(_Fun);
	#undef _Fun
	};

	class JsHeapAllocator: public Object::HeapAllocator {
	public:
		void* alloc(size_t size) override;
		void free(void *ptr) override;
		void strong(Object* obj) override;
		void weak(Object* obj) override;
	};

	class JsClasses {
	public:
		JsClasses(Worker* worker);
		~JsClasses();
		void add(uint64_t id, JSClass *cls) throw(Error);
		JSClass* get(uint64_t id);
		JSFunction* getFunction(uint64_t id);
		bool instanceOf(JSValue* val, uint64_t id);
		bool hasClass(JSClass *cls);
	private:
		Worker *_worker;
		MixObject *_attachObject;
		JSClass *_runClass;
		Dict<uint64_t, JSClass*> _jsclass; // id => JSClass
		Set<JSClass*> _jsclass_set;
		friend class MixObject;
		friend class JSClass;
	};

	struct WorkerInl: public Worker {
		#define Qk_WorkerInl(worker) static_cast<WorkerInl*>(worker)
		JSValue* binding(JSValue* name);
		JSObject* console() { return *_console; }
		void     initGlobalAPIs();
	};

	struct DebugOptions {
		bool waiting_for_connect;
		int  port;
		String host_name, script_path;
	};

	struct Arguments {
		int    argc;
		char** argv;
		DictSS options;
	} extern *arguments;

	void runDebugger(Worker* worker, const DebugOptions &opts);
	void stopDebugger(Worker* worker);
	void debuggerBreakNextStatement(Worker* worker);
	void setFlagsFromCommandLine(const Arguments* args);
	int  startPlatform(int (*exec)(Worker*));
	int  triggerExit(Worker* worker, int code);
	int  triggerBeforeExit(Worker* worker, int code);
	bool triggerUncaughtException(Worker* worker, JSValue* err);
	bool triggerUnhandledRejection(Worker* worker, JSValue* reason, JSValue* promise);
	bool parseEncoding(FunctionArgs args, JSValue* arg, Encoding& en);

	// callback
	JSValue* convert_buffer(Worker* worker, Buffer& buffer, Encoding en = kInvalid_Encoding);
	Callback<Buffer> get_callback_for_buffer(Worker* worker, JSValue* cb, Encoding en = kInvalid_Encoding);
	Callback<Buffer> get_callback_for_buffer_http_error(Worker* worker, JSValue* cb, Encoding en = kInvalid_Encoding);
	Callback<ResponseData> get_callback_for_response_data_http_error(Worker* worker, JSValue* cb);
	Callback<StreamResponse> get_callback_for_io_stream(Worker* worker, JSValue* cb);
	Callback<StreamResponse> get_callback_for_io_stream_http_error(Worker* worker, JSValue* cb);
	Callback<Array<Dirent>> get_callback_for_array_dirent(Worker* worker, JSValue* cb);
	Callback<Bool> get_callback_for_bool(Worker* worker, JSValue* cb);
	Callback<Int32> get_callback_for_int(Worker* worker, JSValue* cb);
	Callback<FileStat> get_callback_for_file_stat(Worker* worker, JSValue* cb);
	Cb get_callback_for_none(Worker* worker, JSValue* cb);

} }
#endif
