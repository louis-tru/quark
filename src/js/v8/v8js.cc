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

#include "./v8js.h"

namespace qk { namespace js {
	// -------------------------- W o r k e r . I m p l --------------------------

	// Extracts a C string from a V8 Utf8Value.
	static cChar* ToCstring(const v8::String::Utf8Value& value) {
		return *value ? *value : "<string conversion failed>";
	}

	WorkerImpl::WorkerImpl(): _locker(nullptr), _handle_scope(nullptr), _inspector(nullptr)
	{
		_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		_isolate = Isolate::New(_params);
		_locker = new Locker(_isolate);
		_isolate->Enter();
		_handle_scope = new HandleScopeMix(_isolate);
		_context = v8::Context::New(_isolate);
		_context->Enter();
		_isolate->SetData(ISOLATE_INL_WORKER_DATA_INDEX, this);
		_global.reset(this, Cast<JSObject>(_context->Global()) );

		_isolate->SetFatalErrorHandler([](cChar* location, cChar* message) {
			if (location) {
				Qk_Fatal("FATAL ERROR: %s %s\n", location, message);
			} else {
				Qk_Fatal("FATAL ERROR: %s\n", message);
			}
		});

		_isolate->AddMessageListener([](v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
			auto worker = WORKER();
			if ( !triggerUncaughtException(worker, Cast(error)) ) {
				worker->printException(message, error);
				qk::thread_exit(ERR_UNCAUGHT_EXCEPTION);
			}
		});

		_isolate->SetPromiseRejectCallback([](PromiseRejectMessage message) {
			if (message.GetEvent() != v8::kPromiseRejectWithNoHandler)
				return;
			auto worker = WORKER();
			v8::Local<v8::Promise> promise = message.GetPromise();
			v8::Local<v8::Value> reason = message.GetValue();
			if (reason.IsEmpty())
				reason = v8::Undefined(_isolate);
			if ( !triggerUnhandledRejection(this, Cast(reason), Cast(promise)) ) {
				v8::HandleScope scope(_isolate);
				v8::Local<v8::Message> message = v8::Exception::CreateMessage(_isolate, reason);
				worker->printException(message, reason);
				qk::thread_exit(ERR_UNHANDLED_REJECTION);
			}
		});
	}

	void WorkerImpl::release() {
		delete _inspector; _inspector = nullptr;
		Worker::release();
		_context->Exit();
		_context.Clear();
		delete _handle_scope; _handle_scope = nullptr;
		_isolate->Exit();
		delete _locker; _locker = nullptr;
		_isolate->Dispose(); _isolate = nullptr;
		Object::release();
		delete _params.array_buffer_allocator;
	}

	void WorkerImpl::runDebugger(const DebugOptions &opts) {
		if (!_inspector)
			_inspector = new inspector::Agent(this);
		_inspector->Start(opts);
	}

	void WorkerImpl::stopDebugger() {
		if (_inspector)
			_inspector->Stop();
	}

	void WorkerImpl::debuggerBreakNextStatement() {
		if (_inspector) {
			_inspector->PauseOnNextJavascriptStatement("Break on start");
		}
	}

	void WorkerImpl::printException(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
		v8::HandleScope handle_scope(_isolate);
		v8::String::Utf8Value exception(_isolate, error);
		// V8 didn't provide any extra information about this error; just
		cChar* exception_string = ToCstring(exception);

		if (message.IsEmpty())
			return Qk_ELog("%s", exception_string );
	
		Array<String> out;

		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(_isolate, message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(_isolate->GetCurrentContext());
		cChar* filename_string = ToCstring(filename);
		int linenum = message->GetLineNumber(context).FromJust();
		out.push(String::format("%s:%d: %s\n", filename_string, linenum, exception_string));

		// Print line of source code.
		v8::String::Utf8Value sourceline(_isolate, message->GetSourceLine(context).ToLocalChecked());
		cChar* sourceline_string = ToCstring(sourceline);
		out.push(sourceline_string).push('\n');

		// Print line of source code position.
		int start = message->GetStartColumn(context).FromJust();
		int end = message->GetEndColumn(context).FromJust();
		for (int i = 0; i < start; i++) {
			out.push(sourceline_string[i] == 9 ? '\t': ' ');
		}
		for (int i = start; i < end; i++) {
			out.push('^');
		}
		// Print stack.
		if (error->IsObject()) {
			out.push('\n');
			v8::Local<v8::Value> stack_trace_string;
			if (error.As<v8::Object>()->Get(_context, newFromOneByte("stack"))
					.ToLocal(&stack_trace_string) &&
					stack_trace_string->IsString() &&
					v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
				v8::String::Utf8Value stack_trace(_isolate, stack_trace_string);
				out.push(ToCstring(stack_trace)).push('\n');
			}
		}

		Qk_ELog(out.join(String()));
	}

	Worker* Worker::current() {
		return first_worker ? first_worker:
			reinterpret_cast<Worker*>(v8::Isolate::GetCurrent()->GetData(ISOLATE_INL_WORKER_DATA_INDEX));
	}

	Worker* Worker::Make() {
		auto o = new WorkerImpl();
		o->init();
		return o;
	}

	v8::Isolate* getIsolate(Worker* worker) {
		return static_cast<WorkerImpl*>(worker)->_isolate;
	}

	v8::Local<v8::Context> getContext(Worker* worker) {
		return static_cast<WorkerImpl*>(worker)->_context;
	}

	// ----------------------------------------------------------------------------------

	struct V8HandleScope {
		v8::HandleScope value;
		inline V8HandleScope(Isolate* isolate): value(isolate) {}
	};

	struct V8EscapableHandleScope {
		v8::EscapableHandleScope value;
		inline V8EscapableHandleScope(Isolate* isolate): value(isolate) {}
	};

	HandleScope::HandleScope(Worker* worker) {
		new(this) V8HandleScope(ISOLATE(worker));
	}

	HandleScope::~HandleScope() {
		reinterpret_cast<V8HandleScope*>(this)->~V8HandleScope();
	}

	EscapableHandleScope::EscapableHandleScope(Worker* worker) {
		new(this) V8EscapableHandleScope(ISOLATE(worker));
	}

	template<>
	JSValue* EscapableHandleScope::escape(JSValue* val) {
		return Cast(reinterpret_cast<V8EscapableHandleScope*>(this)->value.Escape(Back(val)));
	}

	struct TryCatchMix {
		TryCatchMix(Worker *worker)
			: _try(ISOLATE(worker)), _worker(WORKER(worker)) {}
		v8::TryCatch _try;
		WorkerImpl *_worker;
	};

	TryCatch::TryCatch(Worker *worker) {
		_val = new TryCatchMix(worker);
	}

	TryCatch::~TryCatch() {
		delete reinterpret_cast<TryCatchMix*>(_val);
		_val = nullptr;
	}

	bool TryCatch::hasCaught() const {
		return reinterpret_cast<TryCatchMix*>(_val)->_try.HasCaught();
	}

	JSValue* TryCatch::exception() const {
		return Cast(reinterpret_cast<TryCatchMix*>(_val)->_try.Exception());
	}

	void TryCatch::reThrow() {
		reinterpret_cast<TryCatchMix*>(_val)->_try.ReThrow();
	}

	void TryCatch::print() const {
		auto mix = reinterpret_cast<TryCatchMix*>(_val);
		mix->_worker->printException(mix->_try.Message(), mix->_try.Exception());
	}

	// ----------------------------------------------------------------------------------

	bool JSValue::isUndefined() const { return reinterpret_cast<const v8::Value*>(this)->IsUndefined(); }
	bool JSValue::isNull() const { return reinterpret_cast<const v8::Value*>(this)->IsNull(); }
	bool JSValue::isString() const { return reinterpret_cast<const v8::Value*>(this)->IsString(); }
	bool JSValue::isBoolean() const { return reinterpret_cast<const v8::Value*>(this)->IsBoolean(); }
	bool JSValue::isObject() const { return reinterpret_cast<const v8::Value*>(this)->IsObject(); }
	bool JSValue::isArray() const { return reinterpret_cast<const v8::Value*>(this)->IsArray(); }
	bool JSValue::isDate() const { return reinterpret_cast<const v8::Value*>(this)->IsDate(); }
	bool JSValue::isNumber() const { return reinterpret_cast<const v8::Value*>(this)->IsNumber(); }
	bool JSValue::isUint32() const { return reinterpret_cast<const v8::Value*>(this)->IsUint32(); }
	bool JSValue::isInt32() const { return reinterpret_cast<const v8::Value*>(this)->IsInt32(); }
	bool JSValue::isFunction() const { return reinterpret_cast<const v8::Value*>(this)->IsFunction(); }
	bool JSValue::isArrayBuffer() const { return reinterpret_cast<const v8::Value*>(this)->IsArrayBuffer(); }
	bool JSValue::isTypedArray() const { return reinterpret_cast<const v8::Value*>(this)->IsTypedArray(); }
	bool JSValue::isUint8Array() const { return reinterpret_cast<const v8::Value*>(this)->IsUint8Array(); }
	bool JSValue::isBuffer() const { return isTypedArray() || isArrayBuffer(); }
	bool JSValue::equals(Worker *worker, JSValue* val) const {
		return reinterpret_cast<const v8::Value*>(this)->Equals(CONTEXT(worker), Back(val)).ToChecked();
	}
	bool JSValue::strictEquals(JSValue* val) const {
		return reinterpret_cast<const v8::Value*>(this)->StrictEquals(Back(val));
	}

	bool JSValue::instanceOf(Worker* worker, JSObject* value) const {
		return reinterpret_cast<v8::Value*>(this)->
			InstanceOf(CONTEXT(worker), Back<v8::Object>(value)).FromMaybe(false);
	}

	JSString* JSValue::toString(Worker* worker) const {
		return Cast<JSString>(reinterpret_cast<const v8::Value*>(this)->ToString(CONTEXT(worker)));
	}

	JSBoolean* JSValue::toBoolean(Worker* worker) const {
		return Cast<JSBoolean>(reinterpret_cast<const v8::Value*>(this)->ToBoolean(CONTEXT(worker)));
	}

	JSNumber* JSValue::toNumber(Worker* worker) const {
		Cast<JSNumber>(reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker)));
	}

	JSInt32* JSValue::toInt32(Worker* worker) const {
		return Cast<JSInt32>(reinterpret_cast<const v8::Value*>(this)->ToInt32(CONTEXT(worker)));
	}

	JSUint32* JSValue::toUint32(Worker* worker) const {
		return Cast<JSUint32>(reinterpret_cast<const v8::Value*>(this)->ToUint32(CONTEXT(worker)));
	}

	Maybe<String> JSValue::asString(Worker *worker) const {
		if (isString()) {
			return static_cast<const JSString*>(this)->value(worker);
		} else {
			return Maybe<String>();
		}
	}

	Maybe<bool> JSValue::asBoolean(Worker* worker) const {
		return Back(this)->ToBoolean(CONTEXT(worker)).ToLocalChecked()->Value();
	}

	Maybe<double> JSValue::asNumber(Worker* worker) const {
		if (isNumber()) {
			return static_cast<const JSNumber*>(this)->value();
		} else {
			return Maybe<double>();
		}
	}

	Maybe<float> JSValue::asFloat32(Worker* worker) const {
		if (isNumber()) {
			return Maybe<float>(static_cast<const JSNumber*>(this)->value());
		} else {
			return Maybe<float>();
		}
	}

	Maybe<int32_t> JSValue::asInt32(Worker* worker) const {
		if (isInt32()) {
			return static_cast<const JSInt32*>(this)->value();
		} else {
			return Maybe<int32_t>();
		}
	}

	Maybe<uint32_t> JSValue::asUint32(Worker* worker) const {
		if (isUint32()) {
			return static_cast<const JSUint32*>(this)->value();
		} else {
			return Maybe<uint32_t>();
		}
	}

	JSValue* JSObject::get(Worker* worker, JSValue* key) {
		DCHECK(isObject());
		return Cast(reinterpret_cast<v8::Object*>(this)->Get(CONTEXT(worker), Back(key)));
	}

	JSValue* JSObject::get(Worker* worker, uint32_t index) {
		DCHECK(isObject());
		return Cast(reinterpret_cast<v8::Object*>(this)->Get(CONTEXT(worker), index));
	}

	bool JSObject::set(Worker* worker, JSValue* key, JSValue* val) {
		DCHECK(isObject());
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), Back(key), Back(val)).FromMaybe(false);
	}

	bool JSObject::set(Worker* worker, uint32_t index, JSValue* val) {
		DCHECK(isObject());
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), index, Back(val)).FromMaybe(false);
	}

	bool JSObject::has(Worker* worker, JSValue* key) {
		DCHECK(isObject());
		return reinterpret_cast<v8::Object*>(this)->
			Has(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::has(Worker* worker, uint32_t index) {
		DCHECK(isObject());
		return reinterpret_cast<v8::Object*>(this)->
			Has(CONTEXT(worker), index).FromMaybe(false);
	}

	bool JSObject::JSObject::deleteFor(Worker* worker, JSValue* key) {
		DCHECK(isObject());
		return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::deleteFor(Worker* worker, uint32_t index) {
		DCHECK(isObject());
		return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), index).FromMaybe(false);
	}

	JSArray* JSObject::getPropertyNames(Worker* worker) {
		DCHECK(isObject());
		return Cast<JSArray>(reinterpret_cast<v8::Object*>(this)->
												GetPropertyNames(CONTEXT(worker)).FromMaybe(v8::Local<v8::Array>()));
	}

	Maybe<Array<String>> JSObject::getPropertyKeys(Worker* w) {
		DCHECK(isObject());
		auto arr = getPropertyNames(this);
		if (arr) {
			return arr->toStringArray(w)
		}
		return Maybe<Array<String>>();
	}

	JSFunction* JSObject::getConstructor(Worker* worker) {
		DCHECK(isObject());
		auto rv = reinterpret_cast<v8::Object*>(this)->
			Get(CONTEXT(worker), Back(worker->strs()->constructor()));
		return Cast<JSFunction>(rv.FromMaybe(v8::Local<v8::Value>()));
	}

	bool JSObject::setMethod(Worker* worker, cString& name, FunctionCallback func) {
		DCHECK(isObject());
		v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(ISOLATE(worker), func2);
		v8::Local<v8::Function> fn = t->GetFunction(CONTEXT(worker)).ToLocalChecked();
		v8::Local<v8::String> fn_name = Back<v8::String>(worker->newStringOneByte(name));
		fn->SetName(fn_name);
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), fn_name, fn).FromMaybe(false);
	}

	bool JSObject::setAccessor(Worker* worker, cString& name,
														AccessorGetterCallback get, AccessorSetterCallback set) {
		DCHECK(isObject());
		auto get2 = reinterpret_cast<v8::AccessorNameGetterCallback>(get);
		auto set2 = reinterpret_cast<v8::AccessorNameSetterCallback>(set);
		v8::Local<v8::String> fn_name = Back<v8::String>(worker->newStringOneByte(name));
		return reinterpret_cast<v8::Object*>(this)->SetAccessor(CONTEXT(worker), fn_name, get2, set2).ToChecked();
	}

	bool JSObject::defineOwnProperty(Worker *worker, JSValue *key, JSValue *value, int flags) {
		DCHECK(isObject());
		auto name = v8::Name::Cast(reinterpret_cast<v8::Value*>(key));
		auto name_ = *reinterpret_cast<v8::Local<v8::Name>*>(&name);
		return reinterpret_cast<v8::Object*>(this)->
			DefineOwnProperty(CONTEXT(worker), name_, Back(value), v8::PropertyAttribute(flags))
			.FromMaybe(false);
	}

	bool JSObject::setPrototype(Worker* worker, JSObject* __proto__) {
		DCHECK(isObject());
		return reinterpret_cast<v8::Object*>(this)->
			SetPrototype(CONTEXT(worker), Back(__proto__)).FromMaybe(false);
	}

	int JSString::length() const {
		DCHECK(isString());
		return reinterpret_cast<const v8::String*>(this)->Length();
	}

	String JSString::value(Worker* worker) const {
		DCHECK(isString());
		// v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		v8::Local<v8::String> str = Back<v8::String>(this);
		if (!str->Length()) {
			return String();
		}
		if ( str->IsOneByte() ) {
			Buffer buffer(str->Length());
			str->WriteOneByte(ISOLATE(worker), (uint8_t*)*buffer, 0, buffer.capacity());
			return buffer.collapseString();
		} else {
			uint16_t source[128];
			int start = 0, count;
			auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;
			Array<String> rev;

			while ( (count = str->Write(ISOLATE(worker), source, start, 128, opts)) ) {
				auto buf = codec_utf16_to_utf8(ArrayWeak<uint16_t>(source, count).buffer());
				rev.push(buf.collapseString());
				start += count;
			}

			return rev.length() == 0 ? String():
						 rev.length() == 1 ? rev[0]:
						 rev.join(String());
		}
	}

	String2 JSString::value2(Worker* worker) const {
		DCHECK(isString());
		//v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		v8::Local<v8::String> str = Back<v8::String>(this);
		if (!str->Length()) {
			return String2();
		}
		ArrayBuffer<uint16_t> source(str->Length());
		str->Write(ISOLATE(worker), *source, 0, str->Length());
		return source.collapseString();
	}

	String4 JSString::value4(Worker* worker) const {
		DCHECK(isString());
		// v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		v8::Local<v8::String> str = Back<v8::String>(this);
		if (!str->Length()) {
			return String4();
		}
		uint16_t source[128];
		int start = 0, count, revOffset = 0;
		auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;
		Array<uint32_t> rev(str->Length());

		while ( (count = str->Write(ISOLATE(worker), source, start, 128, opts)) ) {
			auto unicode = codec_utf16_to_unicode(ArrayWeak<uint16_t>(source, count).buffer());
			rev.write(*unicode, unicode.length(), revOffset);
			revOffset += unicode.length();
			start += count;
		}
		rev.reset(revOffset);
		return rev.collapseString();
	}

	int JSArray::length() const {
		DCHECK(isArray());
		return reinterpret_cast<const v8::Array*>(this)->Length();
	}

	double JSDate::valueOf() const {
		DCHECK(isDate());
		return reinterpret_cast<const v8::Date*>(this)->ValueOf();
	}

	double JSNumber::value() const {
		DCHECK(isNumber());
		return reinterpret_cast<const v8::Number*>(this)->Value();
	}

	int JSInt32::value() const {
		DCHECK(isInt32());
		return reinterpret_cast<const v8::Int32*>(this)->Value();
	}

	uint32_t JSUint32::value() const {
		DCHECK(isUint32());
		return reinterpret_cast<const v8::Uint32*>(this)->Value();
	}

	bool JSBoolean::value() const {
		DCHECK(isBoolean());
		return reinterpret_cast<const v8::Boolean*>(this)->Value();
	}

	JSValue* JSFunction::call(Worker* worker, int argc, JSValue* argv[], JSValue* recv) {
		DCHECK(isFunction());
		if ( !recv ) {
			recv = worker->newUndefined();
		}
		auto fn = reinterpret_cast<v8::Function*>(this);
		v8::MaybeLocal<v8::Value> r = fn->Call(CONTEXT(worker), Back(recv), argc,
																						reinterpret_cast<v8::Local<v8::Value>*>(argv));
		Local<v8::Value> out;
		return r.ToLocal(&out) ? Cast(out): nullptr;
	}

	JSValue* JSFunction::call(Worker* worker, JSValue* recv) {
		return call(worker, 0, nullptr, recv);
	}

	JSObject* JSFunction::newInstance(Worker* worker, int argc, JSValue* argv[]) {
		DCHECK(isFunction());
		auto fn = reinterpret_cast<v8::Function*>(this);
		v8::MaybeLocal<v8::Object> r = fn->NewInstance(CONTEXT(worker), argc,
																										reinterpret_cast<v8::Local<v8::Value>*>(argv));
		return Cast<JSObject>(r);
	}

	JSObject* JSFunction::getFunctionPrototype(Worker* worker) {
		DCHECK(isFunction());
		auto fn = reinterpret_cast<v8::Function*>(this);
		auto str = Back(worker->strs()->prototype());
		auto r = fn->Get(CONTEXT(worker), str);
		return Cast<JSObject>(r);
	}

	uint32_t JSArrayBuffer::byteLength(Worker* worker) const {
		DCHECK(isArrayBuffer());
		return (uint32_t)reinterpret_cast<const v8::ArrayBuffer*>(this)->ByteLength();
	}

	Char* JSArrayBuffer::data(Worker* worker) {
		DCHECK(isArrayBuffer());
		return (Char*)reinterpret_cast<v8::ArrayBuffer*>(this)->GetContents().Data();
	}

	JSArrayBuffer* JSTypedArray::buffer(Worker* worker) {
		DCHECK(isTypedArray());
		auto typedArray = reinterpret_cast<v8::TypedArray*>(this);
		v8::Local<v8::ArrayBuffer> abuff = typedArray->Buffer();
		return Cast<JSArrayBuffer>(abuff);
	}

	uint32_t JSTypedArray::byteLength(Worker* worker) {
		DCHECK(isTypedArray());
		return (uint32_t)reinterpret_cast<v8::TypedArray*>(this)->ByteLength();
	}

	uint32_t JSTypedArray::byteOffset(Worker* worker) {
		DCHECK(isTypedArray());
		return (uint32_t)reinterpret_cast<v8::TypedArray*>(this)->ByteOffset();
	}

	bool JSSet::add(Worker* worker, JSValue* key) {
		DCHECK(Back(this)->IsSet());
		auto set = reinterpret_cast<v8::Set*>(this);
		return !set->Add(CONTEXT(worker), Back(key)).IsEmpty();
	}

	bool JSSet::has(Worker* worker, JSValue* key) {
		DCHECK(Back(this)->IsSet());
		auto set = reinterpret_cast<v8::Set*>(this);
		return set->Has(CONTEXT(worker), Back(key)).ToChecked();
	}
		
	bool JSSet::deleteFor(Worker* worker, JSValue* key) {
		DCHECK(Back(this)->IsSet());
		auto set = reinterpret_cast<v8::Set*>(this);
		return set->Delete(CONTEXT(worker), Back(key)).ToChecked();
	}

	template <> void Persistent<JSValue>::reset() {
		reinterpret_cast<v8::Persistent<v8::Value>*>(this)->Reset();
	}

	template <> template <>
	void Persistent<JSValue>::reset(Worker* worker, JSValue* other) {
		Qk_Assert_Ne(worker, nullptr);
		reinterpret_cast<v8::Persistent<v8::Value>*>(this)->
			Reset(ISOLATE(worker), *reinterpret_cast<const v8::Local<v8::Value>*>(&other));
		_worker = worker;
	}

	template<> template<>
	void Persistent<JSValue>::copy(const Persistent<JSValue>& from) {
		reset();
		if (from.isEmpty())
			return;
		Qk_Assert_Ne(from._worker, nullptr);
		typedef v8::CopyablePersistentTraits<v8::Value>::CopyablePersistent Handle;
		reinterpret_cast<Handle*>(this)->operator=(*reinterpret_cast<const Handle*>(&from));
		_worker = from._worker;
	}

	MixObject* MixObject::unpack(JSValue* obj) {
		Qk_Assert_Ne(obj, nullptr);
		auto v8obj = reinterpret_cast<v8::Object*>(obj);
		Qk_Assert_Gt(v8obj->InternalFieldCount(), 0);
		return static_cast<MixObject*>(v8obj->GetAlignedPointerFromInternalField(0));
	}

	template<>
	JSValue* Persistent<JSValue>::operator*() const {
		//return Cast(Local<Value>::New(ISOLATE(_worker), Back(_val)));
		return _val;
	}

	JSNumber* Worker::newValue(float data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newValue(double data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSBoolean* Worker::newBool(bool data) {
		return Cast<JSBoolean>(v8::Boolean::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newValue(Char data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newValue(uint8_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newValue(int16_t data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newValue(uint16_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newValue(int32_t data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newValue(uint32_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newValue(int64_t data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newValue(uint64_t data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSString* Worker::newString(cBuffer& data) {
		return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newValue(cString& data) {
		return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newValue(cString2& data) {
		return Cast<JSString>(v8::String::NewExternalTwoByte(
			ISOLATE(this),
			new V8ExternalStringResource(data)
		));
	}

	JSUint8Array* Worker::newValue(Buffer&& buff) {
		size_t offset = 0;
		size_t len = buff.length();
		v8::Local<v8::ArrayBuffer> ab;
		if (buff.length()) {
			size_t len = buff.length();
			char* data = buff.collapse();
			ab = v8::ArrayBuffer::New(ISOLATE(this), data, len, ArrayBufferCreationMode::kInternalized);
		} else {
			ab = v8::ArrayBuffer::New(ISOLATE(this), 0);
		}
		return Cast<JSUint8Array>(v8::Uint8Array::New(ab, offset, len));
	}

	JSObject* Worker::newObject() {
		return Cast<JSObject>(v8::Object::New(ISOLATE(this)));
	}

	JSArray* Worker::newArray(uint32_t len) {
		return Cast<JSArray>(v8::Array::New(ISOLATE(this), len));
	}

	JSValue* Worker::newNull() {
		return Cast(v8::Null(ISOLATE(this)));
	}

	JSValue* Worker::newUndefined() {
		return Cast(v8::Undefined(ISOLATE(this)));
	}

	JSSet* Worker::newSet() {
		return Cast<JSSet>(v8::Set::New(ISOLATE(this)));
	}

	JSString* Worker::newEmpty() {
		return Cast<JSString>(v8::String::Empty(ISOLATE(this)));
	}

	JSString* Worker::newStringOneByte(cString& data) {
		return Cast<JSString>(v8::String::NewExternal(ISOLATE(this),
																									new V8ExternalOneByteStringResource(data)));
	}

	JSArrayBuffer* Worker::newArrayBuffer(Char* use_buff, uint32_t len) {
		return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), use_buff, len));
	}

	JSArrayBuffer* Worker::newArrayBuffer(uint32_t len) {
		return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), len));
	}

	JSUint8Array* Worker::newUint8Array(JSArrayBuffer* abuffer, uint32_t offset, uint32_t size) {
		auto ab2 = Back<v8::ArrayBuffer>(abuffer);
		offset = Qk_Min((uint)ab2->ByteLength(), offset);
		if (size + offset > ab2->ByteLength()) {
			size = (uint)ab2->ByteLength() - offset;
		}
		return Cast<JSUint8Array>(v8::Uint8Array::New(ab2, offset, size));
	}

	JSObject* Worker::newRangeError(cChar* errmsg, ...) {
		Js_Format_Str(errmsg);
		return Cast<JSObject>(v8::Exception::RangeError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newReferenceError(cChar* errmsg, ...) {
		Js_Format_Str(errmsg);
		return Cast<JSObject>(v8::Exception::ReferenceError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newSyntaxError(cChar* errmsg, ...) {
		Js_Format_Str(errmsg);
		return Cast<JSObject>(v8::Exception::SyntaxError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newTypeError(cChar* errmsg, ...) {
		Js_Format_Str(errmsg);
		return Cast<JSObject>(v8::Exception::TypeError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newValue(cError& err) {
		v8::Local<v8::Object> e =
			v8::Exception::Error(Back<v8::String>(newValue(err.message()))).As<v8::Object>();
		e->Set(Back(strs()->Errno()), Back(newValue(err.code())));
		return Cast<JSObject>(e);
	}

	void Worker::throwError(JSValue* exception) {
		ISOLATE(this)->ThrowException(Back(exception));
	}

	JSValue* Worker::runScript(JSString* source, JSString* name, JSObject* sandbox) {
		v8::ScriptCompiler::Source _source(
			Back<v8::String>(source), ScriptOrigin(Back<v8::String>(name)));
		v8::MaybeLocal<v8::Value> result;

		auto isolate = ISOLATE(this);
		auto ctx = CONTEXT(this);
		if ( sandbox ) {
			v8::Local<v8::Function> func;
			if (v8::ScriptCompiler::
					CompileFunctionInContext(ctx, &_source, 0, nullptr, 1, &sandbox).ToLocal(&func)
			) {
				result = func->Call(ctx, v8::Undefined(isolate), 0, nullptr);
			}
		} else { // use default sandbox
			v8::Local<v8::Script> script;
			if (v8::ScriptCompiler::Compile(ctx, &_source).ToLocal(&script)) {
				result = script->Run(ctx);
			}
		}
		return Cast(result.FromMaybe(v8::Local<v8::Value>()));
	}

	JSValue* Worker::runScript(JSString* source, JSString* name, JSObject* sandbox) {
		return runScript(source, name, sandbox);
	}

	void Worker::garbageCollection() {
		ISOLATE(this)->LowMemoryNotification();
	}

	void runDebugger(Worker* worker, const DebugOptions &opts) {
		WORKER(worker)->runDebugger(opts);
	}

	void stopDebugger(Worker* worker) {
		WORKER(worker)->stopDebugger();
	}

	void debuggerBreakNextStatement(Worker* worker) {
		WORKER(worker)->debuggerBreakNextStatement();
	}

	int StartPlatform(int argc, Char** argv, int (*exec)(Worker*)) {
		auto platform = v8::platform::NewDefaultPlatform(1);
		v8::V8::InitializePlatform(platform.get());
		v8::V8::Initialize();
		v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
		int rc = 0;
		{ //
			Sp<Worker> worker = Worker::Make();
			v8::SealHandleScope sealhandle(ISOLATE(*worker));
			rc = exec(*worker); // exec main script
		}
		v8::V8::ShutdownPlatform();
		v8::V8::Dispose();

		return rc;
	}
} }
