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

namespace qk {
	bool is_process_exit();
	namespace js {

	bool JSValue::isBuffer() const {
		return isTypedArray() || isArrayBuffer();
	}

	Buffer JSValue::toBuffer(Worker* worker, Encoding en) const {
		if (en == Encoding::kUTF16_Encoding) {
			String2 str = toStringValue2(worker);
			uint32_t len = str.length() * 2;
			return Buffer((Char*)str.collapse().collapse(), len);
		} else {
			return codec_encode(en, toStringValue4(worker).array().buffer());
		}
	}

	WeakBuffer JSValue::asBuffer(Worker *worker) {
		if (isTypedArray()) {
			return static_cast<JSTypedArray*>(this)->weakBuffer(worker);
		}
		else if (isArrayBuffer()) {
			return static_cast<JSArrayBuffer*>(this)->weakBuffer(worker);
		}
		return WeakBuffer();
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
					r.set( key->toStringValue(worker), val->toInt32Value(worker).unsafe() );
				} else {
					r.set( key->toStringValue(worker), val->toBooleanValue(worker) );
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
				r.set( key->toStringValue(worker), val->toStringValue(worker) );
			}
		}
		return Maybe<Dict<String, String>>(std::move(r));
	}

	JSValue* JSObject::getProperty(Worker* worker, cString& name) {
		return get(worker, worker->newStringOneByte(name)/*One Byte ??*/);
	}

	Maybe<Array<String>> JSArray::toStringArray(Worker* worker) {
		Array<String> rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(); i < len; i++ ) {
				auto val = get(worker, i);
				if ( !val )
					return Maybe<Array<String>>();
				rv.push( val->toStringValue(worker) );
			}
		}
		return Maybe<Array<String>>(std::move(rv));
	}

	Maybe<Array<double>> JSArray::toNumberArray(Worker* worker) {
		Array<double> rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(); i < len; i++ ) {
				rv.push( get(worker, i)->toNumberValue(worker).unsafe() );
			}
		}
		return Maybe<Array<double>>(std::move(rv));
	}

	Maybe<Buffer> JSArray::toBuffer(Worker* worker) {
		Buffer rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(); i < len; i++ ) {
				rv.push( get(worker, i)->toInt32Value(worker).unsafe() );
			}
		}
		return Maybe<Buffer>(std::move(rv));
	}

	WeakBuffer JSArrayBuffer::weakBuffer(Worker* worker) {
		int size = byteLength(worker);
		Char* ptr = data(worker);
		return WeakBuffer(ptr, size);
	}

	WeakBuffer JSTypedArray::weakBuffer(Worker* worker) {
		auto buff = buffer(worker);
		Char* ptr = buff->data(worker);
		int offset = byteOffset(worker);
		int len = byteLength(worker);
		return WeakBuffer(ptr + offset, len);
	}

	// --------------------------- S t r i n g s ---------------------------

	Strings::Strings(Worker* worker) {
		#define _Fun(name) \
			__##name##__.reset(worker, worker->newStringOneByte(#name));
		Js_Strings_Each(_Fun);
		__Errno__.reset(worker, worker->newStringOneByte("errno"));
		#undef _Fun
	}

	// --------------------------- J S . C l a s s ---------------------------

	void JSClass::exports(cString& name, JSObject* exports) {
		_func.reset(); // reset func
		exports->setProperty(_worker, name, getFunction());
	}

	JSObject* JSClass::newInstance(uint32_t argc, JSValue* argv[]) {
		auto f = getFunction();
		Qk_ASSERT( f );
		return f->newInstance(_worker, argc, argv);
	}

	JsClassInfo::JsClassInfo(Worker* worker)
		: _worker(worker)
	{
		auto cls = JSClass::MakeEmpty(worker, "JsAttachConstructorEmpty");
		add(0xffffffff, cls, 0);
		// auto cls = _worker->newClass("JsAttachConstructorEmpty", 0xffffffff, [](FunctionArgs args) {}, 0);
		_jsAttachConstructorEmpty.reset(_worker, cls->getFunction());
	}

	JsClassInfo::~JsClassInfo() {
		for ( auto i : _jsclass )
			delete i.value;
		_jsAttachConstructorEmpty.reset();
	}

	JSClass* JsClassInfo::get(uint64_t id) {
		JSClass *out = nullptr;
		_jsclass.get(id, out);
		return out;
	}

	JSFunction* JsClassInfo::getFunction(uint64_t id) {
		JSClass *out;
		if ( _jsclass.get(id, out) ) {
			return out->getFunction();
		}
		return nullptr;
	}

	void JsClassInfo::add(uint64_t id, JSClass *cls,
												AttachCallback attach) throw(Error) {
		Qk_Check( ! _jsclass.has(id), "Set native Constructors ID repeat");
		// cls->_worker = _worker;
		cls->_id = id;
		cls->_attachConstructor = attach;
		_jsclass.set(id, cls);
	}

	WrapObject* JsClassInfo::attach(uint64_t id, Object* object) {
		auto wrap = reinterpret_cast<WrapObject*>(object) - 1;
		Qk_ASSERT( !wrap->worker() );
		JSClass *out;
		if ( _jsclass.get(id, out) ) {
			out->_attachConstructor(wrap);
			// auto jsobj = out->getFunction()->newInstance(_worker);
			auto jsobj = _jsAttachConstructorEmpty->newInstance(_worker);
			auto prototype = out->getFunction()->getPrototype(_worker);
			auto ok = jsobj->set__Proto__(_worker, prototype);
			Qk_ASSERT(ok);
			return wrap->attach(_worker, jsobj);
		}
		return nullptr;
	}

	bool JsClassInfo::instanceOf(JSValue* val, uint64_t id) {
		JSClass *out;
		if ( _jsclass.get(id, out) )
			return out->hasInstance(val);
		return false;
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
		Qk_DEBUG("%s %s", "Worker_Set_Module ", *name);
		if (!NativeModulesLib) {
			NativeModulesLib = new Dict<String, NativeModuleLib>();
		}
		NativeModulesLib->set(name, { name, pathname ? pathname: name, binding, 0 });
	}

	JSValue* BindingModule::binding(JSValue* name) {
		auto r = _nativeModules->get(this, name);
		if (!r->isUndefined())
			return r;
		auto exports = newObject();
		auto name_str = name->toStringValue(this);
		const NativeModuleLib* lib;
		Qk_DEBUG("binding: %s", *name_str);

		if ( NativeModulesLib->get(name_str, lib) ) {
			if (lib->binding) {
				lib->binding(exports, this);
			}
			else if (lib->native_code) {
				exports = runNativeScript(
					WeakBuffer(lib->native_code->code, lib->native_code->count).buffer(),
					String(lib->native_code->name) + lib->native_code->ext, exports
				)->as<JSObject>();

				if ( !exports ) // error
					return exports;
			}
			_nativeModules->set(this, name, exports);
			return exports;
		} else {
			auto worker = this;
			Js_Throw("Module does not exist, %s", *name_str), nullptr;
		}
	}

	JSValue* Worker::bindingModule(cString& name) {
		return static_cast<BindingModule*>(this)->binding(newStringOneByte(name));
	}

	static void __binding__(FunctionArgs args) {
		Js_Worker(args);
		if (args.length() < 1) {
			Js_Throw("Bad argument.");
		}
		auto r = static_cast<BindingModule*>(worker)->binding(args[0]);
		if (r)
			Js_Return(r);
	}

	#define _Fun(N) void Js_Set_Module_##N##__();
	Js_All_Modules(_Fun)
	#undef _Fun

	Worker::Worker()
		: _types(nullptr)
		, _strs(nullptr)
		, _classsinfo(nullptr)
		, _thread_id(thread_self_id())
	{
		if (!NativeModulesLib) {
			#define _Fun(N) Js_Set_Module_##N##__();
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

	void Worker::release() {
		delete _types; _types = nullptr;
		delete _strs; _strs = nullptr;
		delete _classsinfo; _classsinfo = nullptr;
		_nativeModules.reset();
		_global.reset();
	}

	void Worker::init() {
		HandleScope scope(this);
		_nativeModules.reset(this, newObject());
		_strs = new Strings(this);
		_classsinfo = new JsClassInfo(this);
		Qk_ASSERT(_global->isObject());
		_global->setProperty(this, "global", *_global);
		_global->setMethod(this, "__binding__", __binding__);

		auto globalThis = newInstance("globalThis");
		if ( !_global->has(this, globalThis) ) {
			_global->set(this, globalThis, *_global);
		}
		initGlobalAPIs(this);
	}

	JSObject* Worker::global() {
		return *_global;
	}

	JSObject* Worker::newError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		String str = _Str::string_format(errmsg, arg);
		va_end(arg);
		Error err(ERR_UNKNOWN_ERROR, str);
		return newInstance(err);
	}

	JSObject* Worker::newError(cError& err) {
		return newInstance(err);
	}

	JSObject* Worker::newError(JSObject* value) {
		auto err = newInstance(Error(""));
		auto names = value->getPropertyNames(this);
		for (uint32_t i = 0, j = 0; i < names->length(); i++) {
			auto key = names->get(this, i);
			err->set(this, key, value->get(this, key));
		}
		return err;
	}

	JSValue* Worker::newInstance(Object* val) {
		if (!val) {
			auto wrap = WrapObject::wrap(val);
			if (wrap)
				return wrap->that();
		}
		return newNull();
	}

	JSObject* Worker::newInstance(const HttpError& err) {
		auto rv = newInstance(*static_cast<cError*>(&err));
		if ( rv ) {
			if (!rv->set(this, strs()->status(), newInstance(err.status())))
				return nullptr;
			if (!rv->set(this, strs()->url(), newInstance(err.url())))
				return nullptr;
		}
		return rv;
	}

	JSObject* Worker::newInstance(cDictSS& data) {
		auto rev = newObject();
		{ HandleScope scope(this);
			for (auto& i : data)
				rev->set(this, newStringOneByte(i.key), newInstance(i.value));
		}
		return rev;
	}

	JSString* Worker::newInstance(cString4& data) {
		return newInstance(codec_encode_to_utf16(data.array().buffer()).collapseString());
	}

	JSArray* Worker::newInstance(cArray<String>& data) {
		auto rev = newArray();
		{ HandleScope scope(this);
			for (int i = 0, e = data.length(); i < e; i++) {
				rev->set(this, i, newInstance(data[i]));
			}
		}
		return rev;
	}

	JSUint8Array* Worker::newInstance(Buffer& buff) {
		return newInstance(std::move(buff));
	}

	JSUint8Array* Worker::newUint8Array(JSString* str, Encoding en) {
		return newInstance(str->toBuffer(this, en));
	}

	JSUint8Array* Worker::newUint8Array(int size, Char fill) {
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
		String str = _Str::string_format(errmsg, arg);
		va_end(arg);
		throwError(newError(*str));
	}

	bool Worker::instanceOf(JSValue* val, uint64_t id) {
		return _classsinfo->instanceOf(val, id);
	}

	JSClass* Worker::jsclass(uint64_t id) {
		return _classsinfo->get(id);
	}

	// ---------------------------------------------------------------------------------------------

	static JSValue* TriggerEventFromUtil(
		Worker* worker, cString& name, int argc = 0, JSValue* argv[] = 0
	) {
		auto _util = worker->bindingModule("_util")->as<JSObject>();
		Qk_ASSERT(_util);

		auto func = _util->getProperty(worker, String("__on").append(name).append("_native"));
		if (!func->isFunction()) {
			return nullptr;
		}
		return func->as<JSFunction>()->call(worker, argc, argv);
	}

	static int TriggerExit(Worker* worker, cString& name, int code) {
		Js_Handle_Scope();
		auto argv = worker->newInstance(code)->as<JSValue>();
		auto rc = TriggerEventFromUtil(worker, name, 1, &argv);
		if (rc && rc->isInt32()) {
			return rc->toInt32Value(worker).unsafe();
		} else {
			return code;
		}
	}

	static bool TriggerException(Worker* worker, cString& name, int argc, JSValue* argv[]) {
		Js_Handle_Scope();
		auto rc = TriggerEventFromUtil(worker, name, argc, argv);
		return rc && rc->toBooleanValue(worker);
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

	static void onProcessExitHandle(Event<>& e, void* ctx) {
		int rc = static_cast<const Int32*>(e.data())->value;
		if (RunLoop::first()->runing()) {
			typedef Callback<RunLoop::PostSyncData> Cb;
			RunLoop::first()->post_sync(Cb([&](Cb::Data& e) {
				auto worker = Worker::worker();
				Qk_DEBUG("onProcessSafeHandle");
				if (worker)
					rc = triggerExit(worker, rc);
				e.data->complete();
			}));
		}
		e.return_value = rc;
	}

	int Start(const Array<String>& argv_in) {
		Qk_ASSERT(!__quark_js_argv);

		Object::setAllocator(
			&object_allocator_alloc,
			&object_allocator_free, &object_allocator_strong, &object_allocator_weak
		);
		Array<char*> argv, quark_argv;
		parseArgv(argv_in, argv, quark_argv);

		// Mark the current main thread and check current thread
		Qk_ASSERT(RunLoop::first() == RunLoop::current());

		Qk_On(ProcessExit, onProcessExitHandle);

		__quark_js_argv = &quark_argv;

		char** argv_c = const_cast<char**>(&argv[0]);
		int rc = platformStart(argv.length(), argv_c, [](Worker* worker) -> int {
			{
				auto _pkg = worker->bindingModule("_pkg");
				Qk_ASSERT(_pkg && _pkg->isObject(), "Can't start worker");
				auto r = _pkg->as<JSObject>()->
					getProperty(worker, "Module")->as<JSObject>()->
					getProperty(worker, "runMain")->as<JSFunction>()->call(worker);
				if (!r) {
					Qk_ERR("ERROR: Can't call runMain()");
					return ERR_RUN_MAIN_EXCEPTION;
				}
			}
			int rc = 0;
			auto loop = RunLoop::first();
			do {
				loop->run();
				/* IOS forces the process to terminate, but it does not quit immediately.
				This may cause a process to run in the background for a long time, so force break here */
				if (is_process_exit())
					break;

				if (loop->is_alive())
					continue;

				rc = triggerBeforeExit(worker, rc);

				// Emit `beforeExit` if the loop became alive either after emitting
				// event, or after running some callbacks.
			} while (loop->is_alive());

			if (!is_process_exit())
				rc = triggerExit(worker, rc);
			return rc;
		});

		__quark_js_argv = nullptr;

		Qk_Off(ProcessExit, onProcessExitHandle);
		// Object::setAllocator(nullptr, nullptr, nullptr, nullptr);

		return rc;
	}

} }
