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

#include "../../out/native-inl-js.h"
#include "../../out/native-lib-js.h"
#include "./api/types.h"
#include "../errno.h"
#include "../ui/app.h"
#include <stdlib.h>
#if Qk_ANDROID
#include "../platforms/android/android.h"
#endif

namespace qk {
	extern int (*__qk_run_main__)(int, char**);
	bool is_process_exit();
namespace js {
	std::atomic_int workers_count(0);
	Worker* first_worker = nullptr;

	Maybe<WeakBuffer> JSValue::asBuffer(Worker *worker) const {
		if (isTypedArray()) {
			return static_cast<const JSTypedArray*>(this)->value(worker);
		}
		else if (isArrayBuffer()) {
			return static_cast<const JSArrayBuffer*>(this)->value(worker);
		}
		return Maybe<WeakBuffer>();
	}

	/**
	 * Convert the JSString to a Buffer using the specified encoding.
	*/
	Buffer JSString::toBuffer(Worker* w, Encoding en) const {
		switch (en) {
			case kBinary_Encoding:
			case kAscii_Encoding: {
				auto ucs2 = value2(w); // as ucs2 codec
				return codec_encode(en, ucs2);
			}
			case kHex_Encoding: // parse buffer from hex or base64 string
			case kBase64_Encoding: {
				auto str = value(w).collapse(); // as string for hex or base64
				return codec_decode_to_ucs1(en, str);
			}
			case kUTF8_Encoding: {
				return value(w).collapse();
			}
			case kUTF16_Encoding: {
				auto utf16 = value2(w).collapse();
				auto len = utf16.length() << 1;
				auto capacity = utf16.capacity() << 1;
				return Buffer((char*)utf16.collapse(), len, capacity);
			}
			default: { // kUCS4_Encoding
				auto unicode = value4(w).collapse();
				return codec_encode(en, unicode);
			}
		}
	}

	Maybe<Dict<String, int>> JSObject::toIntegerDict(Worker* worker) {
		Dict<String, int> r;

		if ( isObject() ) {
			JSArray* names = getPropertyNames(worker);
			if ( !names )
				return Maybe<Dict<String, int>>();
			
			for ( uint32_t i = 0, len = names->length(); i < len; i++ ) {
				JSValue* key = names->get(worker, i);
				if ( !key )
					return Maybe<Dict<String, int>>();

				JSValue* val = get(worker, key);
				if ( !val ) return Maybe<Dict<String, int>>();
				if ( val->isNumber() ) {
					r.set( key->toString(worker)->value(worker), val->toInt32(worker)->value() );
				} else {
					r.set( key->toString(worker)->value(worker), val->toBoolean(worker) );
				}
			}
		}
		return Maybe<Dict<String, int>>(std::move(r));
	}

	Maybe<Dict<String, String>> JSObject::toStringDict(Worker* worker) {
		Dict<String, String> r;

		if ( isObject() ) {
			auto names = getPropertyNames(worker);
			if ( !names )
				return Maybe<Dict<String, String>>();
			
			for ( uint32_t i = 0, len = names->length(); i < len; i++ ) {
				auto key = names->get(worker, i);
				if ( !key ) {
					return Maybe<Dict<String, String>>();
				}
				auto val = get(worker, key);
				if ( !val ) {
					return Maybe<Dict<String, String>>();
				}
				r.set( key->toString(worker)->value(worker), val->toString(worker)->value(worker) );
			}
		}
		return Maybe<Dict<String, String>>(std::move(r));
	}

	template<>
	JSValue* JSObject::get(Worker* worker, cString& key) {
		return get(worker, worker->newValue(key));
	}

	template<>
	bool JSObject::setFor(Worker* worker, cString& key, JSValue* value) {
		return set(worker, worker->newValue(key), value);
	}

	Maybe<Array<String>> JSArray::toStringArray(Worker* worker) {
		Array<String> rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(); i < len; i++ ) {
				auto val = get(worker, i);
				if ( !val )
					return Maybe<Array<String>>();
				rv.push( val->toString(worker)->value(worker) );
			}
		}
		return Maybe<Array<String>>(std::move(rv));
	}

	Maybe<Array<double>> JSArray::toNumberArray(Worker* worker) {
		Array<double> rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(); i < len; i++ ) {
				auto val = get(worker, i);
				if (!val) {
					return Maybe<Array<double>>();
				}
				auto num = val->toNumber(worker);
				if (!num) {
					return Maybe<Array<double>>();
				}
				rv.push( num->value() );
			}
		}
		return Maybe<Array<double>>(std::move(rv));
	}

	Maybe<Buffer> JSArray::toBuffer(Worker* worker) {
		Buffer rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(); i < len; i++ ) {
				rv.push( get(worker, i)->toInt32(worker)->value() );
			}
		}
		return Maybe<Buffer>(std::move(rv));
	}

	WeakBuffer JSArrayBuffer::value(Worker* worker) const {
		int size = byteLength(worker);
		char* ptr = const_cast<JSArrayBuffer*>(this)->data(worker);
		return WeakBuffer(ptr, size);
	}

	WeakBuffer JSTypedArray::value(Worker* worker) const {
		auto abf = const_cast<JSTypedArray*>(this)->buffer(worker);
		char* ptr = abf->data(worker);
		int offset = byteOffset(worker);
		int len = byteLength(worker);
		return WeakBuffer(ptr + offset, len);
	}

	Strings::Strings(Worker* worker) {
		#define _Fun(name) \
			__##name##__.reset(worker, worker->newStringOneByte(#name));
		Js_Strings_Each(_Fun);
		__Errno__.reset(worker, worker->newStringOneByte("errno"));
		#undef _Fun
	}

	// ----------------------------------- W o r k e r -----------------------------------

	struct NativeJSCode {
		int count;
		const char* code;
		const char* name;
		const char* ext;
	};

	struct NativeModuleLib {
		String name;
		String pathname;
		BindingCallback binding;
		const NativeJSCode* native_code;
	};

	static Dict<String, NativeModuleLib>* NativeModulesLib = nullptr;

	void Worker::setModule(cString& name, BindingCallback binding, cChar* pathname) {
		Qk_DLog("%s %s", "Worker_Set_Module ", *name);
		if (!NativeModulesLib) {
			NativeModulesLib = new Dict<String, NativeModuleLib>();
		}
		NativeModulesLib->set(name, { name, pathname ? pathname: name, binding, 0 });
	}

	JSValue* WorkerInl::binding(JSValue* name) {
		auto mod = _nativeModules->get(this, name);
		if (!mod->isUndefined())
			return mod;
		auto exports = newObject();
		auto name_str = name->toString(this)->value(this);
		const NativeModuleLib* lib;
		Qk_DLog("binding: %s", *name_str);

		if ( NativeModulesLib->get(name_str, lib) ) {
			if (lib->binding) {
				lib->binding(exports, this);
			}
			else if (lib->native_code) {
				auto nc = lib->native_code;
				_nativeModules->set(this, name, exports);

				exports = runNativeScript(
					nc->code, nc->count, String(nc->name) + nc->ext, exports)->cast<JSObject>();

				if ( !exports ) { // error
					_nativeModules->deleteFor(this, name);
					return exports;
				}
			}
			_nativeModules->set(this, name, exports);
			return exports;
		}

		auto worker = this;
		Js_Throw("Module does not exist, %s", *name_str), nullptr;
	}

	JSValue* Worker::bindingModule(cString& name) {
		return Qk_WorkerInl(this)->binding(newStringOneByte(name));
	}

	static void __binding__(FunctionArgs args) {
		Js_Worker(args);
		if (args.length() < 1) {
			Js_Throw("Bad argument.");
		}
		auto r = Qk_WorkerInl(worker)->binding(args[0]);
		if (r)
			Js_Return(r);
	}

	#define _Fun(N) void Js_Module_##N##__();
	Js_All_Modules(_Fun)
	#undef _Fun

	Worker::Worker()
		: _types(nullptr)
		, _strs(nullptr)
		, _classes(nullptr)
		, _thread_id(thread_self_id())
		, _loop(RunLoop::current())
	{
		if (!NativeModulesLib) {
			#define _Fun(N) Js_Module_##N##__();
			Js_All_Modules(_Fun)
			#undef _Fun
		}
		Qk_ASSERT(NativeModulesLib);

		// register core native module
		if ( !NativeModulesLib->has("_pkg") ) {
			for (int i = 0; i < native_js::INL_native_js_count_; i++) {
				const NativeJSCode* code = (const NativeJSCode*)native_js::INL_native_js_ + i;
				if (!NativeModulesLib->has(code->name)) { // skip _event / _types
					NativeModulesLib->set(code->name, { code->name, code->name, 0, code });
				}
			}
			for (int i = 0; i < native_js::LIB_native_js_count_; i++) {
				const NativeJSCode* code = (const NativeJSCode*)native_js::LIB_native_js_ + i;
				NativeModulesLib->set(code->name, { code->name, code->name, 0, code });
			}
		}
	}

	void Worker::init() {
		Qk_ASSERT(_global->isObject());

		first_worker = workers_count++ ? nullptr: this;

		HandleScope scope(this);
		_nativeModules.reset(this, newObject());
		_strs = new Strings(this);
		_classes = new JsClasses(this);
		_global->setFor(this, "global", *_global);
		_global->setMethod(this, "__binding__", __binding__);

		auto globalThis = newStringOneByte("globalThis");
		if (!_global->has(this, globalThis)) {
			_global->set(this, globalThis, *_global);
		}
		Qk_WorkerInl(this)->initGlobalAPIs();
	}

	void Worker::release() {
		markAsInvalid();
		Releasep(_types);
		Releasep(_strs);
		Releasep(_classes);
		_nativeModules.reset();
		_global.reset();
		_console.reset();

		if (first_worker == this)
			first_worker = nullptr;
		workers_count--;
	}

	JSObject* Worker::global() {
		return *_global;
	}

	JSObject* Worker::newError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		String str = _Str::printfv(errmsg, arg);
		va_end(arg);
		Error err(ERR_UNKNOWN_ERROR, str);
		return newValue(err);
	}

	JSObject* Worker::newError(cError& err) {
		return newValue(err);
	}

	JSObject* Worker::newError(JSObject* value) {
		auto err = newValue(Error(""));
		auto names = value->getPropertyNames(this);
		for (int i = 0, j = 0; i < names->length(); i++) {
			auto key = names->get(this, i);
			err->set(this, key, value->get(this, key));
		}
		return err;
	}

	JSValue* Worker::newValue(Object* val) {
		if (val) {
			auto mix = MixObject::mix(val);
			if (mix)
				return mix->handle();
		}
		return newNull();
	}

	JSObject* Worker::newValue(cHttpError& err) {
		auto rv = newValue(*static_cast<cError*>(&err));
		if ( rv ) {
			if (!rv->set(this, strs()->status(), newValue(err.status())))
				return nullptr;
			if (!rv->set(this, strs()->url(), newValue(err.url())))
				return nullptr;
		}
		return rv;
	}

	JSObject* Worker::newValue(cDictSS& data) {
		auto rev = newObject();
		{ HandleScope scope(this);
			for (auto& i : data)
				rev->set(this, newStringOneByte(i.first), newValue(i.second));
		}
		return rev;
	}

	JSString* Worker::newValue(cString4& data) {
		return newValue(codec_unicode_to_utf16(data.array().buffer()).collapseString());
	}

	template<typename T>
	static JSArray* newArrayTempl(cArray<T>& data, Worker *worker) {
		auto rev = worker->newArray();
		{ HandleScope scope(worker);
			for (int i = 0, e = data.length(); i < e; i++) {
				rev->set(worker, i, worker->newValue(data[i]));
			}
		}
		return rev;
	}

	JSArray* Worker::newValue(cArray<String>& data) {
		return newArrayTempl(data, this);
	}

	JSArray* Worker::newValue(cArray<uint32_t>& data) {
		return newArrayTempl(data, this);
	}

	JSUint8Array* Worker::newValue(Buffer& buff) {
		return newValue(std::move(buff));
	}

	JSUint8Array* Worker::newUint8Array(JSString* str, Encoding en) {
		return newValue(str->toBuffer(this, en));
	}

	JSUint8Array* Worker::newUint8Array(int size, char fill) {
		auto ab = newArrayBuffer(size);
		if (fill)
			memset(ab->data(this), fill, size);
		return newUint8Array(ab);
	}

	JSUint8Array* Worker::newUint8Array(JSArrayBuffer* abuffer) {
		return newUint8Array(abuffer, 0, abuffer->byteLength(this));
	}

	void Worker::throwError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		String str = _Str::printfv(errmsg, arg);
		va_end(arg);
		throwError(newError(*str));
	}

	bool Worker::instanceOf(JSValue* val, uint64_t alias) {
		return _classes->instanceOf(val, alias);
	}

	JSClass* Worker::jsclass(uint64_t alias) {
		return _classes->get(alias);
	}

	JSValue* Worker::runNativeScript(cChar* source, int sLen, cString& name, JSObject* exports) {
		EscapableHandleScope scope(this);
		if (!exports) {
			exports = newObject();
		}
		auto func = runScript(newString(WeakBuffer(source, sLen).buffer()), newValue(name))->
			cast<JSFunction>();
		if ( func ) {
			Qk_ASSERT(func->isFunction());
			auto mod = newObject();
			mod->set(this, strs()->exports(), exports);
			JSValue *args[] = { exports->cast(), mod->cast(), global()->cast() };
			// (function(exports,module,global){ ... })
			if (func->call(this, 3, args)) {
				return scope.escape( mod->get(this, strs()->exports()) );
			}
		}
		return nullptr;
	}

	// ---------------------------------------------------------------------------------------------

	static JSValue* TriggerEventFromUtil(Worker* worker, cString& name, int argc, JSValue* argv[]) {
		auto _util = worker->bindingModule("_util")->cast<JSObject>();
		Qk_ASSERT(_util);

		auto func = _util->get(worker, String("__on").append(name).append("_native"));
		if (!func->isFunction()) {
			return nullptr;
		}
		return func->cast<JSFunction>()->call(worker, argc, argv);
	}

	static int TriggerExit(Worker* worker, cString& name, int code) {
		Js_Handle_Scope();
		auto argv = worker->newValue(code)->cast<JSValue>();
		auto rc = TriggerEventFromUtil(worker, name, 1, &argv);
		if (rc && rc->isInt32()) {
			return rc->toInt32(worker)->value();
		} else {
			return code;
		}
	}

	static bool TriggerException(Worker* worker, cString& name, int argc, JSValue* argv[]) {
		Js_Handle_Scope();
		auto rc = TriggerEventFromUtil(worker, name, argc, argv);
		return rc && rc->toBoolean(worker);
	}

	int triggerExit(Worker* worker, int code) {
		return TriggerExit(worker, "Exit", code);
	}

	int triggerBeforeExit(Worker* worker, int code) {
		return TriggerExit(worker, "BeforeExit", code);
	}

	bool triggerUncaughtException(Worker* worker, JSValue* err) {
		return TriggerException(worker, "UncaughtException", 1, &err);
	}

	bool triggerUnhandledRejection(Worker* worker, JSValue* reason, JSValue* promise) {
		JSValue* argv[] = { reason, promise };
		return TriggerException(worker, "UnhandledRejection", 2, argv);
	}

	// ---------------------------------------------------------------------------------------------

	static void onProcessExitHandle(Event<void, int>& e, void* ctx) {
		int rc = e.data();
		if (RunLoop::first()->runing()) {
			typedef Callback<RunLoop::PostSyncData> Cb;
			RunLoop::first()->post_sync(Cb([&](Cb::Data& e) {
				auto worker = Worker::worker();
				Qk_DLog("onProcessSafeHandle");
				if (worker)
					rc = triggerExit(worker, rc);
				e.data->complete();
			}));
		}
		e.return_value = rc;
	}

	int Start(cString &cmd, cArray<String>& argv_in) {
		static String argv_s;
		argv_s = fs_executable();
		Array<uint32_t> argv_idx = {argv_s.length()};
		ArrayString args = cmd.split(' ');

		for (auto& arg: args.concat(argv_in)) {
			if (!arg.isEmpty()) {
				argv_s.append(' ');
				argv_s.append(arg);
				argv_idx.push(argv_s.length());
			}
		}
		char* arg = const_cast<char*>(*argv_s);
		Array<char*> argv;

		for (auto idx: argv_idx) {
			argv.push(arg);
			arg = const_cast<char*>(*argv_s + idx + 1);
			arg[-1] = '\0';
		}

		return Start(argv.length(), *argv);
	}

	Arguments *arguments = nullptr;

	int Start(int argc, char** argv) {
		Qk_ASSERT(!arguments);
		String lastKey;
		Arguments args = { argc, argv };

		auto putkv = [&args](cString& k, cString& v) {
			String *old;
			if (args.options.get(k, old)) {
				if (old->isEmpty())
					*old = v;
				else
					old->append(" ").append(v);
			} else {
				args.options[k] = v;
			}
		};

		for (int i = 1; i < argc; i++) {
			String arg(argv[i]);
			if (arg[0] == '-') {
				auto kv = arg.split('=');
				auto k = kv[0].replaceAll('-', '_');
				auto v = kv.length() > 1 ? kv[1]: String();
				if (arg.length() > 1 && arg[1] != '-') { // -
					if (kv.length() > 1)
						goto val;
					lastKey = k.substr(1);
					putkv(lastKey, v);
					if (lastKey.length() > 1 && kv.length() == 1) {
						for (auto i = 0u; i < lastKey.length(); i++)
							putkv(lastKey[i], String());
					}
				} else if (arg.length() > 2) { // --
					putkv(k.substr(2), v);
					lastKey = String();
				}
			} else if (arg.length() > 0) {
				val:
				if (lastKey.length()) {
					putkv(lastKey, arg);
					lastKey = String();
				} else if (args.options.has("__main__")) {
					putkv(String("__unknown__"), arg);
				} else {
					args.options.set("__main__", arg);
					args.options.set("__mainIdx__", i);
				}
			}
		}

		setFlagsFromCommandLine(&args);

		if (args.options.has("help"))
			return 0;

		Object::setHeapAllocator(new JsHeapAllocator()); // set object heap allocator

		::setenv("UV_THREADPOOL_SIZE", "1", 0); // set uv thread loop size as 1

		// Mark the current main thread and check current thread
		Qk_ASSERT_EQ(RunLoop::current(), RunLoop::first());

		Qk_On(ProcessExit, onProcessExitHandle);

		arguments = &args;

		int rc = startPlatform([](Worker* worker) -> int {
			String *inspect, *mainPath = nullptr;
			if (!arguments->options.get("__main__", mainPath)) {
				if (!arguments->options.has("e") && !arguments->options.has("eval")) {
					return Qk_ELog("No input js file"), ERR_INVALID_FILE_PATH;
				}
			}
			if (
				arguments->options.get("debug", inspect) ||
				arguments->options.get("inspect", inspect) ||
				arguments->options.get("inspect_brk", inspect)
			) {
				auto script_path = mainPath ?
					fs_reader()->format(*mainPath): String("eval");
				bool brk = arguments->options.has("brk") || arguments->options.has("inspect_brk");
				// Startup debugger
				if (inspect->length() == 0) {
					runDebugger(worker, {brk, 9229, "127.0.0.1", script_path});
				} else {
					auto host = inspect->split(':');
					int port = 9229;
					if (host.length() > 1)
						host[1].toNumber<int>(&port);
					runDebugger(worker, {brk, port, host[0], script_path});
				}

				if (brk) {
#if !DEBUG
					if (arguments->options.has("inner_brk"))
#endif
						debuggerBreakNextStatement(worker);
				}
			}

			int rc = 0;
			auto loop = RunLoop::current();

			{ // run main
				Js_Handle_Scope();
				auto _pkg = worker->bindingModule("_pkg");
				Qk_ASSERT(_pkg && _pkg->isObject(), "Can't start worker");
				auto fn = _pkg->cast<JSObject>()->
					get<JSObject>(worker, "Module")->get<JSFunction>(worker, "runMain");
				if (!fn->call(worker)) {
					Qk_ELog("ERROR: Can't call runMain()");
					return ERR_RUN_MAIN_EXCEPTION;
				}
			}

			do {
				loop->run();
				if (is_process_exit())
					break;
				// Emit `beforeExit` if the loop became alive either after emitting
				// event, or after running some callbacks.
				rc = triggerBeforeExit(worker, rc);
			} while (loop->is_alive());

			if (!is_process_exit())
				rc = triggerExit(worker, rc);

			Release(shared_app()); // delete global object

			loop->clear(); // clear all async handles

			return rc;
		});

		Qk_Off(ProcessExit, onProcessExitHandle);

		arguments = nullptr;
		return rc;
	}

	// Default main function
	Qk_Init_Func(set_default_main) {
		__qk_run_main__ = [](int argc, char** argv) -> int {
			String start;

#if Qk_ANDROID
			start = JNI::jvm() ? Android_startup_argv(): String();
			if (start.isEmpty())
#endif
			{
				auto index = fs_resources("index");
				if (fs_reader()->exists_sync(index)) {
					for (auto s: String(fs_reader()->read_file_sync(index)).split('\n')) {
						s = s.trim();
						if ( s[0] != '#' ) {
							start = s;
							break;
						}
					}
				}
			}
	
			if (!start.isEmpty()) {
				return js::Start(start);
			} else if (argc > 0) {
				return js::Start(argc, argv);
			} else {
				return 0;
			}
		};
	};
} }
