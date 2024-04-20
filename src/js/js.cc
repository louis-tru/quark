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

// #include <native-inl-js.h>
// #include <native-lib-js.h>
#include "../util/string.h"
#include "./_js.h"
#include "../util/http.h"
#include "../util/codec.h"
#include "./types.h"

namespace qk { namespace js {
	// using namespace native_js;

	Buffer JSValue::toBuffer(Worker* worker, Encoding en) const {
		switch (en) {
			case Encoding::hex: // 解码 hex and base64
			case Encoding::base64:
				return Coder::decoding_to_byte(en, ToStringValue(worker,1));
			case Encoding::utf8:// ucs2编码成utf8
				return ToStringValue(worker).collapse_buffer();
			case Encoding::ucs2:
			case Encoding::utf16: {
				String2 str = ToString2Value(worker);
				uint32_t len = str.length() * 2;
				return Buffer((Char*)str.collapse(), len);
			}
			default: // 编码
				return Coder::encoding(en, ToStringValue(worker));
		}
	}

	bool JSValue::isBuffer() const {
		return isTypedArray() || isArrayBuffer();
	}

	bool JSValue::isBuffer(Worker *worker) const {
		return isBuffer();
	}

	WeakBuffer JSValue::asBuffer(Worker *worker) {
		if (IsTypedArray(worker)) {
			return static_cast<JSTypedArray*>(this)->weakBuffer(worker);
		} else if (IsArrayBuffer()) {
			return static_cast<JSArrayBuffer*>(this)->weakBuffer(worker);
		}
		return WeakBuffer();
	}

	Maybe<Dict<String, int>> JSObject::toIntegerMap(Worker* worker) {
		Dict<String, int> r;
		
		if ( IsObject(worker) ) {
		
			Local<JSArray> names = GetPropertyNames(worker);
			if ( names.IsEmpty() ) return Maybe<Dict<String, int>>();
			
			for ( uint32_t i = 0, len = names->Length(worker); i < len; i++ ) {
				Local<JSValue> key = names->Get(worker, i);
				if ( names.IsEmpty() ) return Maybe<Dict<String, int>>();
				Local<JSValue> val = Get(worker, key);
				if ( val.IsEmpty() ) return Maybe<Dict<String, int>>();
				if ( val->IsNumber(worker) ) {
					r.set( key->ToStringValue(worker), val->ToNumberValue(worker) );
				} else {
					r.set( key->ToStringValue(worker), val->ToBooleanValue(worker) );
				}
			}
		}
		return Maybe<Dict<String, int>>(std::move(r));
	}

	Maybe<Dict<String, String>> JSObject::toStringMap(Worker* worker) {
		Dict<String, String> r;
		
		if ( IsObject(worker) ) {
			Local<JSArray> names = GetPropertyNames(worker);
			if ( names.IsEmpty() ) return Maybe<Dict<String, String>>();
			
			for ( uint32_t i = 0, len = names->Length(worker); i < len; i++ ) {
				Local<JSValue> key = names->Get(worker, i);
				if ( names.IsEmpty() ) return Maybe<Dict<String, String>>();
				Local<JSValue> val = Get(worker, key);
				if ( val.IsEmpty() ) return Maybe<Dict<String, String>>();
				r.set( key->ToStringValue(worker), val->ToStringValue(worker) );
			}
		}
		return Maybe<Dict<String, String>>(std::move(r));
	}

	Maybe<JSON> JSObject::toJSON(Worker* worker) {
		JSON r = JSON::object();
		
		if ( IsObject(worker) ) {
			Local<JSArray> names = GetPropertyNames(worker);
			if ( names.IsEmpty() ) return Maybe<JSON>();
			
			for ( uint32_t i = 0, len = names->Length(worker); i < len; i++ ) {
				Local<JSValue> key = names->Get(worker, i);
				if ( names.IsEmpty() ) return Maybe<JSON>();
				Local<JSValue> val = Get(worker, key);
				if ( val.IsEmpty() ) return Maybe<JSON>();
				String key_s = key->ToStringValue(worker);
				if (val->IsUint32(worker)) {
					r[key_s] = val->ToUint32Value(worker);
				} else if (val->IsInt32(worker)) {
					r[key_s] = val->ToInt32Value(worker);
				} else if (val->IsNumber(worker)) {
					r[key_s] = val->ToInt32Value(worker);
				} else if (val->IsBoolean(worker)) {
					r[key_s] = val->ToBooleanValue(worker);
				} else if (val->IsNull(worker)) {
					r[key_s] = JSON::null();
				} else {
					r[key_s] = val->ToStringValue(worker);
				}
			}
		}
		return Maybe<JSON>(move(r));
	}

	Local<JSValue> JSObject::getProperty(Worker* worker, cString& name) {
		return Get(worker, worker->New(name, 1));
	}

	Maybe<Array<String>> JSArray::toStringArrayMaybe(Worker* worker) {
		Array<String> rv;
		if ( IsArray(worker) ) {
			for ( uint32_t i = 0, len = Length(worker); i < len; i++ ) {
				Local<JSValue> val = Get(worker, i);
				if ( val.IsEmpty() )
					return Maybe<Array<String>>();
				rv.push( val->ToStringValue(worker) );
			}
		}
		return Maybe<Array<String>>(move(rv));
	}

	Maybe<Array<double>> JSArray::toNumberArrayMaybe(Worker* worker) {
		Array<double> rv;
		if ( IsArray(worker) ) {
			double out;
			for ( uint32_t i = 0, len = Length(worker); i < len; i++ ) {
				Local<JSValue> val = Get(worker, i);
				if ( val.IsEmpty() || !val->ToNumberMaybe(worker).To(out) )
					return Maybe<Array<double>>();
				rv.push( out );
			}
		}
		return Maybe<Array<double>>(move(rv));
	}

	Maybe<Buffer> JSArray::toBufferMaybe(Worker* worker) {
		Buffer rv;
		if ( IsArray(worker) ) {
			double out;
			for ( uint32_t i = 0, len = Length(worker); i < len; i++ ) {
				Local<JSValue> val = Get(worker, i);
				if ( val.IsEmpty() || !val->ToNumberMaybe(worker).To(out) )
					return Maybe<Buffer>();
				rv.push( out );
			}
		}
		return Maybe<Buffer>(move(rv));
	}

	WeakBuffer JSArrayBuffer::weakBuffer(Worker* worker) {
		int size = ByteLength(worker);
		Char* data = Data(worker);
		return WeakBuffer(data, size);
	}

	WeakBuffer JSTypedArray::weakBuffer(Worker* worker) {
		auto buffer = Buffer(worker);
		Char* data = buffer->Data(worker);
		int offset = ByteOffset(worker);
		int len = ByteLength(worker);
		return WeakBuffer(data + offset, len);
	}

	// ----------------------------------- W o r k e r -----------------------------------

	struct NativeModuleLib {
		String name;
		String file;
		BindingCallback binding;
		const LIB_NativeJSCode* native_code;
	};

	static Dict<String, NativeModuleLib>* NativeModulesLib = nullptr;

	Worker* Worker::create() {
		// TODO ...
	}

	void Worker::RegisterModule(cString& name, BindingCallback binding, cChar* file) {
		if (!NativeModulesLib) {
			NativeModulesLib = new Dict<String, NativeModule>();
		}
		NativeModulesLib->set(name, { name, file ? file : __FILE__, binding, 0 });
	}

	Local<JSValue> Worker::bindingModule(cString& name) {
		auto str = New(name);
		auto r = _nativeModules.local()->get(this, str);
		if (!r->IsUndefined()) {
			return r.To<JSObject>();
		}

		NativeModuleLib* lib;
		auto exports = NewObject();

		if (NativeModulesLib->get(name, lib)) {
			if (lib->binding) {
				lib->binding(exports, this);
			}
			else if (lib->native_code) {
				exports = runNativeScript(
					WeakBuffer((Char*) lib->native_code->code, lib->native_code->count),
					String(lib->native_code->name) + lib->native_code->ext, exports
				).To();
				if ( exports.IsEmpty() ) { // error
					return exports;
				}
			}
			_nativeModules.local()->set(this, str, exports);
		}

		return exports;
	}

	Worker::Worker()
		: _types(nullptr), _strs(nullptr)
		, _jsclassinfo(nullptr)
		, _thread_id(Thread::current_id())
	{
		// register core native module
		static int initializ_core_native_module = 0;
		if ( initializ_core_native_module++ == 0 ) {
			if (!NativeModulesLib) {
				NativeModulesLib = new Dict<String, NativeModuleLib>();
			}
			// for (int i = 0; i < LIB_native_js_count_; i++) {
			// 	const LIB_NativeJSCode* code = LIB_native_js_ + i;
			// 	NativeModulesLib->set(code->name, { code->name, code->name, 0, code });
			// }
		}
	}

	Worker::~Worker() {
		Release(_values); _values = nullptr;
		Release(_strs); _strs = nullptr;
		delete _classsinfo; _classsinfo = nullptr;
		_native_modules.Reset();
		_global.Reset();
	}

	Local<JSObject> Worker::global() {
		return _global.local();
	}

	Local<JSObject> Worker::NewError(cChar* errmsg, ...) {
		Qk_STRING_FORMAT(errmsg, str);
		Error err(ERR_UNKNOWN_ERROR, str);
		return New(err);
	}

	Local<JSObject> Worker::New(const HttpError& err) {
		Local<JSObject> rv = New(*static_cast<cError*>(&err));
		if ( !rv.IsEmpty() ) {
			if (!rv->Set(this, strs()->status(), New(err.status()))) return Local<JSObject>();
			if (!rv->Set(this, strs()->url(), New(err.url()))) return Local<JSObject>();
			if (!rv->Set(this, strs()->code(), New(err.code()))) return Local<JSObject>();
		}
		return rv;
	}

	Local<JSArray> Worker::New(Array<Dirent>& ls) { return New(move(ls)); }
	Local<JSArray> Worker::New(Array<FileStat>& ls) { return New(move(ls)); }
	Local<JSUint8Array> Worker::New(Buffer& buff) { return New(move(buff)); }
	Local<JSObject> Worker::New(FileStat& stat) { return New(move(stat)); }
	Local<JSObject> Worker::NewError(cError& err) { return New(err); }
	Local<JSObject> Worker::NewError(const HttpError& err) { return New(err); }

	Local<JSObject> Worker::New(FileStat&& stat) {
		Local<JSFunction> func = _inl->_classs->get_constructor(JS_TYPEID(FileStat));
		Qk_ASSERT( !func.IsEmpty() );
		Local<JSObject> r = func->NewInstance(this);
		*Wrap<FileStat>::unpack(r)->self() = move(stat);
		return r;
	}

	Local<JSObject> Worker::NewInstance(uint64_t id, uint32_t argc, Local<JSValue>* argv) {
		Local<JSFunction> func = _inl->_classs->get_constructor(id);
		Qk_ASSERT( !func.IsEmpty() );
		return func->NewInstance(this, argc, argv);
	}

	Local<JSUint8Array> Worker::NewUint8Array(Local<JSString> str, Encoding en) {
		Buffer buff = str->ToBuffer(this, en);
		return New(buff);
	}

	Local<JSUint8Array> Worker::NewUint8Array(int size, Char fill) {
		auto ab = NewArrayBuffer(size);
		if (fill)
			memset(ab->Data(this), fill, size);
		return NewUint8Array(ab);
	}

	Local<JSUint8Array> Worker::NewUint8Array(Local<JSArrayBuffer> ab) {
		return NewUint8Array(ab, 0, ab->ByteLength(this));
	}

	void Worker::throwError(cChar* errmsg, ...) {
		Qk_STRING_FORMAT(errmsg, str);
		throwError(NewError(*str));
	}

	bool Worker::hasView(Local<JSValue> val) {
		return _inl->_classs->instanceof(val, qk::View::VIEW);
	}

	bool Worker::hasInstance(Local<JSValue> val, uint64_t id) {
		return _inl->_classs->instanceof(val, id);
	}

	// ----------------------------- W o r k e r . I m p l ----------------------------- 

	// @private __require__
	static void require_native(FunctionCall args) {
		JS_WORKER(args);
		JS_HANDLE_SCOPE();
		if (args.Length() < 1) {
			JS_THROW_ERR("Bad argument.");
		}
		String name = args[0]->ToStringValue(worker);
		Local<JSValue> r = worker->bindingModule(name);
		if (!r.IsEmpty()) {
			JS_RETURN(r);
		}
	}

	WorkerImpl::WorkerImpl() {
	}
	WorkerImpl::~WorkerImpl() {
	}

	void WorkerImpl::initialize() {
		HandleScope scope(this);
		_nativeModules.Reset(this, NewObject());
		_strs = new CommonStrings(this);
		_classsinfo = new JSClassInfo(this);
		Qk_ASSERT(_global->isObject(this));
		_global->SetProperty(this, "global", _global.local());
		_global->SetMethod(this, "__require__", require_native);

		auto globalThis = New("globalThis");
		if ( !_global->has(this, globalThis) ) {
			_global->set(this, globalThis, _global.local());
		}
	}

	static Local<JSValue> TriggerEventFromUtil(Worker* worker,
		cString& name, int argc = 0, Local<JSValue> argv[] = 0)
	{
		Local<JSObject> _util = worker->bindingModule("_util").To();
		Qk_ASSERT(!_util.IsEmpty());

		Local<JSValue> func = _util->GetProperty(worker, String("__on").push(name).push("_native"));
		if (!func->IsFunction(worker)) {
			return Local<JSValue>();
		}
		return func.To<JSFunction>()->Call(worker, argc, argv);
	}

	static int TriggerExit_1(Worker* worker, cString& name, int code) {
		JS_HANDLE_SCOPE();
		Local<JSValue> argv = worker->New(code);
		Local<JSValue> rc = TriggerEventFromUtil(worker, name, 1, &argv);
		if (!rc.IsEmpty() && rc->IsInt32(worker)) {
			return rc->ToInt32Value(worker);
		} else {
			return code;
		}
	}

	static bool TriggerException(Worker* worker, cString& name, int argc, Local<JSValue> argv[]) {
		JS_HANDLE_SCOPE();
		Local<JSValue> rc = TriggerEventFromUtil(worker, name, argc, argv);
		if (!rc.IsEmpty() && rc->ToBooleanValue(worker)) {
			return true;
		} else {
			return false;
		}
	}

	int IMPL::TriggerExit(int code) {
		return TriggerExit_1(_host, "Exit", code);
	}

	int IMPL::TriggerBeforeExit(int code) {
		return TriggerExit_1(_host, "BeforeExit", code);
	}

	bool IMPL::TriggerUncaughtException(Local<JSValue> err) {
		return TriggerException(_host, "UncaughtException", 1, &err);
	}

	bool IMPL::TriggerUnhandledRejection(Local<JSValue> reason, Local<JSValue> promise) {
		Local<JSValue> argv[] = { reason, promise };
		return TriggerException(_host, "UnhandledRejection", 2, argv);
	}

} }