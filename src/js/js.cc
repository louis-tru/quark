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

#include <native-inl-js.h>
#include "../util/string.h"
#include "./_js.h"
#include "../util/http.h"
#include "../util/codec.h"
#include "./types.h"

namespace qk { namespace js {

	Buffer JSValue::toBuffer(Worker* worker, Encoding en) const {
		if (en == Encoding::kUTF16_Encoding) {
			String2 str = toStringValue2(worker);
			uint32_t len = str.length() * 2;
			return Buffer((Char*)str.collapse().collapse(), len);
		} else {
			return codec_encode(en, toStringValue2(worker));
		}
	}

	bool JSValue::isBuffer() const {
		return isTypedArray() || isArrayBuffer();
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
			Local<JSArray> names = getPropertyNames(worker);
			if ( names.isEmpty() )
				return Maybe<Dict<String, int>>();
			
			for ( uint32_t i = 0, len = names->length(worker); i < len; i++ ) {
				Local<JSValue> key = names->get(worker, i);
				if ( key.isEmpty() )
					return Maybe<Dict<String, int>>();

				Local<JSValue> val = get(worker, key);
				if ( val.isEmpty() ) return Maybe<Dict<String, int>>();
				if ( val->isNumber() ) {
					r.set( key->toStringValue(worker), val->toNumberValue(worker) );
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
			Local<JSArray> names = getPropertyNames(worker);
			if ( names.isEmpty() )
				return Maybe<Dict<String, String>>();
			
			for ( uint32_t i = 0, len = names->length(worker); i < len; i++ ) {
				Local<JSValue> key = names->get(worker, i);
				if ( key.isEmpty() ) {
					return Maybe<Dict<String, String>>();
				}
				Local<JSValue> val = get(worker, key);
				if ( val.isEmpty() ) {
					return Maybe<Dict<String, String>>();
				}
				r.set( key->toStringValue(worker), val->toStringValue(worker) );
			}
		}
		return Maybe<Dict<String, String>>(std::move(r));
	}

	Maybe<JSON> JSObject::toJSON(Worker* worker) {
		JSON r = JSON::object();
		
		if ( isObject() ) {
			Local<JSArray> names = getPropertyNames(worker);
			if ( names.isEmpty() )
				return Maybe<JSON>();
			
			for ( uint32_t i = 0, len = names->length(worker); i < len; i++ ) {
				Local<JSValue> key = names->get(worker, i);
				if ( key.isEmpty() ) return Maybe<JSON>();
				Local<JSValue> val = get(worker, key);
				if ( val.isEmpty() ) return Maybe<JSON>();
				String key_s = key->toStringValue(worker);
				if (val->isUint32()) {
					r[key_s] = val->toUint32Value(worker);
				} else if (val->isInt32()) {
					r[key_s] = val->toInt32Value(worker);
				} else if (val->isNumber()) {
					r[key_s] = val->toInt32Value(worker);
				} else if (val->isBoolean()) {
					r[key_s] = val->toBooleanValue(worker);
				} else if (val->isNull()) {
					r[key_s] = JSON::null();
				} else {
					r[key_s] = val->toStringValue(worker);
				}
			}
		}
		return Maybe<JSON>(std::move(r));
	}

	Local<JSValue> JSObject::getProperty(Worker* worker, cString& name) {
		return get(worker, worker->newStringOneByte(name)/*One Byte ??*/);
	}

	Maybe<Array<String>> JSArray::toStringArray(Worker* worker) {
		Array<String> rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(worker); i < len; i++ ) {
				Local<JSValue> val = get(worker, i);
				if ( val.isEmpty() )
					return Maybe<Array<String>>();
				rv.push( val->toStringValue(worker) );
			}
		}
		return Maybe<Array<String>>(std::move(rv));
	}

	Maybe<Array<double>> JSArray::toNumberArray(Worker* worker) {
		Array<double> rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(worker); i < len; i++ ) {
				rv.push( get(worker, i)->toNumberValue(worker) );
			}
		}
		return Maybe<Array<double>>(std::move(rv));
	}

	Maybe<Buffer> JSArray::toBuffer(Worker* worker) {
		Buffer rv;
		if ( isArray() ) {
			for ( uint32_t i = 0, len = length(worker); i < len; i++ ) {
				rv.push( get(worker, i)->toInt32Value(worker) );
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

	// --------------------------- J S . C l a s s ---------------------------

	void JSClass::exports(cString& name, Local<JSObject> exports) {
		_func.reset(); // reset func
		exports->setProperty(_worker, name, getFunction());
	}

	Local<JSObject> JSClass::newInstance(uint32_t argc, Local<JSValue>* argv) {
		auto f = getFunction();
		Qk_ASSERT( !f.isEmpty() );
		return f->newInstance(_worker, argc, argv);
	}

	JsClassInfo::JsClassInfo(Worker* worker)
		: _worker(worker)
	{
		auto cls = _worker->newClass("JsAttachConstructorEmpty", 0xffffffff, [](FunctionArgs args) {}, 0);
		_jsAttachConstructorEmpty.reset(_worker, cls->getFunction());
	}

	JsClassInfo::~JsClassInfo() {
		for ( auto i : _jsclass )
			delete i.value;
		_jsAttachConstructorEmpty.reset();
	}

	Local<JSClass> JsClassInfo::get(uint64_t id) {
		JSClass *out;
		if ( _jsclass.get(id, out) ) {
			return *reinterpret_cast<Local<JSClass>*>(&out);
		}
		return Local<JSClass>();
	}

	Local<JSFunction> JsClassInfo::getFunction(uint64_t id) {
		JSClass *out;
		if ( _jsclass.get(id, out) ) {
			return out->getFunction();
		}
		return Local<JSFunction>();
	}

	void JsClassInfo::add(uint64_t id, JSClass *cls,
												AttachCallback attach) throw(Error) {
		Qk_Check( ! _jsclass.has(id), "Set native Constructors ID repeat");
		cls->_worker = _worker;
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

	bool JsClassInfo::instanceOf(Local<JSValue> val, uint64_t id) {
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
		if (!NativeModulesLib) {
			NativeModulesLib = new Dict<String, NativeModuleLib>();
		}
		NativeModulesLib->set(name, { name, pathname ? pathname: name, binding, 0 });
	}

 	Local<JSValue> BindingModule::binding(Local<JSValue> name) {
		auto r = _nativeModules.toLocal()->get(this, name);
		if (!r->isUndefined())
			return r;
		const NativeModuleLib* lib;
		auto exports = newObject();
		auto ok = NativeModulesLib->get(name->toStringValue(this), lib);

		if (ok) {
			if (lib->binding) {
				lib->binding(exports, this);
			}
			else if (lib->native_code) {
				exports = runNativeScript(
					WeakBuffer(lib->native_code->code, lib->native_code->count).buffer(),
					String(lib->native_code->name) + lib->native_code->ext, exports
				).cast<JSObject>();

				if ( exports.isEmpty() ) { // error
					return exports;
				}
			}
			_nativeModules.toLocal()->set(this, name, exports);
		}
		return exports;
	}

	Local<JSValue> Worker::bindingModule(cString& name) {
		return static_cast<BindingModule*>(this)->binding(newStringOneByte(name));
	}

	static void __bindingModule__(FunctionArgs args) {
		Js_Worker(args);
		if (args.length() < 1) {
			Js_Throw("Bad argument.");
		}
		// Js_Handle_Scope();
		auto r = static_cast<BindingModule*>(worker)->binding(args[0]);
		if (r) {
			Js_Return(r);
		}
	}

	Worker::Worker()
		: _types(nullptr)
		, _strs(nullptr)
		, _classsinfo(nullptr)
		, _thread_id(thread_current_id())
	{
		// register core native module
		static int initializ_core_native_module = 0;
		if ( initializ_core_native_module++ == 0 ) {
			Qk_ASSERT(NativeModulesLib);
			for (int i = 0; i < native_js::INL_native_js_count_; i++) {
				const NativeJSCode* code = (const NativeJSCode*)native_js::INL_native_js_ + i;
				if (!NativeModulesLib->has(code->name)) { // skip _event / _types
					NativeModulesLib->set(code->name, { code->name, code->name, 0, code });
				}
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
		_strs = new CommonStrings(this);
		_classsinfo = new JsClassInfo(this);
		Qk_ASSERT(_global->isObject(this));
		_global->setProperty(this, "global", _global.toLocal());
		_global->setMethod(this, "__bindingModule__", __bindingModule__);

		auto globalThis = newInstance("globalThis");
		if ( !_global->has(this, globalThis.cast<JSValue>()) ) {
			_global->set(this, globalThis.cast<JSValue>(), _global.toLocal());
		}
	}

	Local<JSObject> Worker::global() {
		return _global.toLocal();
	}

	Local<JSObject> Worker::newError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		String str = _Str::string_format(errmsg, arg);
		va_end(arg);
		Error err(ERR_UNKNOWN_ERROR, str);
		return newInstance(err);
	}

	Local<JSObject> Worker::newInstance(const HttpError& err) {
		Local<JSObject> rv = newInstance(*static_cast<cError*>(&err));
		if ( !rv.isEmpty() ) {
			if (!rv->set(this, strs()->status(), newInstance(err.status()))) return Local<JSObject>();
			if (!rv->set(this, strs()->url(), newInstance(err.url()))) return Local<JSObject>();
			if (!rv->set(this, strs()->code(), newInstance(err.code()))) return Local<JSObject>();
		}
		return rv;
	}

	Local<JSArray> Worker::newInstance(Array<Dirent>& ls) { return newInstance(std::move(ls)); }
	Local<JSArray> Worker::newInstance(Array<FileStat>& ls) { return newInstance(std::move(ls)); }
	Local<JSUint8Array> Worker::newInstance(Buffer& buff) { return newInstance(std::move(buff)); }
	Local<JSObject> Worker::newInstance(FileStat& stat) { return newInstance(std::move(stat)); }
	Local<JSObject> Worker::newError(cError& err) { return newInstance(err); }
	Local<JSObject> Worker::newError(const HttpError& err) { return newInstance(err); }

	Local<JSObject> Worker::newInstance(FileStat&& stat) {
		Local<JSFunction> func = _classsinfo->getFunction(Js_Typeid(FileStat));
		Qk_ASSERT( !func.IsEmpty() );
		auto r = func->newInstance(this);
		*WrapObject::wrap<FileStat>(r)->self() = std::move(stat);
		return r;
	}

	Local<JSUint8Array> Worker::newUint8Array(Local<JSString> str, Encoding en) {
		return newInstance(str->toBuffer(this, en));
	}

	Local<JSUint8Array> Worker::newUint8Array(int size, Char fill) {
		auto ab = newArrayBuffer(size);
		if (fill)
			memset(ab->data(this), fill, size);
		return newUint8Array(ab);
	}

	Local<JSUint8Array> Worker::newUint8Array(Local<JSArrayBuffer> ab) {
		return newUint8Array(ab, 0, ab->byteLength(this));
	}

	void Worker::throwError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		String str = _Str::string_format(errmsg, arg);
		va_end(arg);
		throwError(newError(*str));
	}

	bool Worker::instanceOf(Local<JSValue> val, uint64_t id) {
		return _classsinfo->instanceOf(val, id);
	}

	Local<JSClass> Worker::jsclass(uint64_t id) {
		return _classsinfo->get(id);
	}

	// ---------------------------------------------------------------------------------------------

	static Local<JSValue> TriggerEventFromUtil(Worker* worker,
		cString& name, int argc = 0, Local<JSValue> argv[] = 0)
	{
		Local<JSObject> _util = worker->bindingModule("_util").cast();
		Qk_ASSERT(!_util.IsEmpty());

		Local<JSValue> func = _util->getProperty(worker, String("__on").append(name).append("_native"));
		if (!func->isFunction()) {
			return Local<JSValue>();
		}
		return func.cast<JSFunction>()->call(worker, argc, argv);
	}

	static int TriggerExit(Worker* worker, cString& name, int code) {
		Js_Handle_Scope();
		Local<JSValue> argv = worker->newInstance(code);
		Local<JSValue> rc = TriggerEventFromUtil(worker, name, 1, &argv);
		if (!rc.isEmpty() && rc->isInt32()) {
			return rc->toInt32Value(worker);
		} else {
			return code;
		}
	}

	static bool TriggerException(Worker* worker, cString& name, int argc, Local<JSValue> argv[]) {
		Js_Handle_Scope();
		Local<JSValue> rc = TriggerEventFromUtil(worker, name, argc, argv);
		if (!rc.isEmpty() && rc->toBooleanValue(worker)) {
			return true;
		} else {
			return false;
		}
	}

	int triggerExit(Worker* worker, int code) {
		return TriggerExit(worker, "Exit", code);
	}

	int triggerBeforeExit(Worker* worker, int code) {
		return TriggerExit(worker, "BeforeExit", code);
	}

	bool triggerUncaughtException(Worker* worker, Local<JSValue> err) {
		return TriggerException(worker, "UncaughtException", 1, &err);
	}

	bool triggerUnhandledRejection(Worker* worker, Local<JSValue> reason, Local<JSValue> promise) {
		Local<JSValue> argv[] = { reason, promise };
		return TriggerException(worker, "UnhandledRejection", 2, argv);
	}

} }
