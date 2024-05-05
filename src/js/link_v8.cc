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

#include <v8.h>
#include <libplatform/libplatform.h>
#include "./types.h"
#include "../errno.h"
#include "../util/codec.h"

namespace qk { namespace js {
	using namespace v8;

	#ifndef ISOLATE_INL_WORKER_DATA_INDEX
	# define ISOLATE_INL_WORKER_DATA_INDEX (0)
	#endif

	#define WORKER(...) WorkerImpl::worker( __VA_ARGS__ )
	#define ISOLATE(...) WorkerImpl::worker( __VA_ARGS__ )->_isolate
	#define CONTEXT(...) WorkerImpl::worker( __VA_ARGS__ )->_context

	typedef const v8::FunctionCallbackInfo<Value>& V8FunctionCall;
	typedef const v8::PropertyCallbackInfo<Value>& V8PropertyCall;
	typedef const v8::PropertyCallbackInfo<void>& V8PropertySetCall;

	template<class T = JSValue, class S>
	inline T* Cast(v8::Local<S> o) { return reinterpret_cast<T*>(*o); }

	template<class T = v8::Value>
	inline v8::Local<T> Back(JSValue* o) { return *reinterpret_cast<v8::Local<T>*>(&o); }


	class V8ExternalOneByteStringResource: public v8::String::ExternalOneByteStringResource {
		String _str;
	public:
		V8ExternalOneByteStringResource(cString& value): _str(value) {}
		virtual cChar* data() const { return _str.c_str(); }
		virtual size_t length() const { return _str.length(); }
	};

	class V8ExternalStringResource: public v8::String::ExternalStringResource {
		String2 _str;
	public:
		V8ExternalStringResource(const String2& value): _str(value) {}
		virtual const uint16_t* data() const { return _str.c_str(); }
		virtual size_t length() const { return _str.length(); }
	};

	// -------------------------- W o r k e r . I m p l --------------------------

	class WorkerImpl: public Worker {
	public:
		struct HandleScopeWrap {
			v8::HandleScope value;
			inline HandleScopeWrap(Isolate* isolate): value(isolate) {}
		};
		Isolate*  _isolate;
		Locker*   _locker;
		HandleScopeWrap* _handle_scope;
		v8::Local<v8::Context> _context;

		WorkerImpl(): _locker(nullptr), _handle_scope(nullptr)
		{
			Isolate::CreateParams params;
			params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
			_isolate = Isolate::New(params);
			_locker = new Locker(_isolate);
			_isolate->Enter();
			_handle_scope = new HandleScopeWrap(_isolate);
			_context = v8::Context::New(_isolate);
			_context->Enter();
			_isolate->SetFatalErrorHandler(OnFatalError);
			_isolate->AddMessageListener(MessageCallback);
			_isolate->SetPromiseRejectCallback(PromiseRejectCallback);
		}

		virtual void init() override {
			_isolate->SetData(ISOLATE_INL_WORKER_DATA_INDEX, this);
			_global.reset(this, Cast<JSObject>(_context->Global()) );
			Worker::init();
		}

		void release() override {
			Worker::release();
			_context->Exit();
			_context.Clear();
			delete _handle_scope; _handle_scope = nullptr;
			_isolate->Exit();
			delete _locker; _locker = nullptr;
			_isolate->Dispose(); _isolate = nullptr;
			Object::release();
		}

		inline static WorkerImpl* worker() {
			return static_cast<WorkerImpl*>(Worker::current());
		}

		template<class Args>
		inline static WorkerImpl* worker(Args args) {
			return static_cast<WorkerImpl*>( args.GetIsolate()->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
		}

		inline v8::Local<v8::String> newFromOneByte(cChar* str) {
			return v8::String::NewFromOneByte(_isolate, (uint8_t*)str, NewStringType::kNormal).ToLocalChecked();
		}

		inline v8::Local<v8::String> newFromUtf8(cChar* str) {
			return v8::String::NewFromUtf8(_isolate, str);
		}

		template <class T, class M = NonCopyablePersistentTraits<T>>
		inline v8::Local<T> strong(const v8::Persistent<T, M>& persistent) {
			return *reinterpret_cast<v8::Local<T>*>(const_cast<v8::Persistent<T, M>*>(&persistent));
		}

		v8::MaybeLocal<v8::Value> runScript(v8::Local<v8::String> source_string,
																		v8::Local<v8::String> name, v8::Local<v8::Object> sandbox) 
		{
			v8::ScriptCompiler::Source source(source_string, ScriptOrigin(name));
			v8::MaybeLocal<v8::Value> result;
			
			if ( sandbox.IsEmpty() ) { // use default sandbox
				v8::Local<v8::Script> script;
				if ( v8::ScriptCompiler::Compile(_context, &source).ToLocal(&script) ) {
					result = script->Run(_context);
				}
			} else {
				v8::Local<v8::Function> func;
				if (v8::ScriptCompiler::
						CompileFunctionInContext(_context, &source, 0, NULL, 1, &sandbox)
						.ToLocal(&func)
						) {
					result = func->Call(_context, v8::Undefined(_isolate), 0, NULL);
				}
			}
			return result;
		}

		JSValue* runNativeScript(cBuffer& source, cString& name, JSObject* exports) {
			v8::Local<v8::Value> _name = Back(newInstance(name));
			v8::Local<v8::Value> _souece = Back(newString(source));

			v8::MaybeLocal<v8::Value> rv;

			rv = runScript(_souece.As<v8::String>(),
											_name.As<v8::String>(), v8::Local<v8::Object>());
			if ( !rv.IsEmpty() ) {
				auto module = newObject();
				module->set(this, strs()->exports(), exports);
				v8::Local<v8::Function> func = rv.ToLocalChecked().As<v8::Function>();
				v8::Local<v8::Value> args[] = { Back(exports), Back(module), Back(global()) };
				rv = func->Call(_context, v8::Undefined(_isolate), 3, args);
				if (!rv.IsEmpty()) {
					auto rv = module->get(this, strs()->exports());
					Qk_ASSERT(rv->isObject());
					return rv;
				}
			}
			return nullptr;
		}

		// Extracts a C string from a V8 Utf8Value.
		static cChar* to_cstring(const v8::String::Utf8Value& value) {
			return *value ? *value : "<string conversion failed>";
		}

		String parse_exception_message(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
			v8::HandleScope handle_scope(_isolate);
			v8::String::Utf8Value exception(_isolate, error);
			cChar* exception_string = to_cstring(exception);
			if (message.IsEmpty()) {
				// V8 didn't provide any extra information about this error; just
				return exception_string;
			} else {
				Array<String> out;
				// Print (filename):(line number): (message).
				v8::String::Utf8Value filename(_isolate, message->GetScriptOrigin().ResourceName());
				v8::Local<v8::Context> context(_isolate->GetCurrentContext());
				cChar* filename_string = to_cstring(filename);
				int linenum = message->GetLineNumber(context).FromJust();
				out.push(String::format("%s:%d: %s\n", filename_string, linenum, exception_string));
				// Print line of source code.
				v8::String::Utf8Value sourceline(_isolate, message->GetSourceLine(context).ToLocalChecked());
				cChar* sourceline_string = to_cstring(sourceline);
				out.push(sourceline_string); out.push('\n');
				int start = message->GetStartColumn(context).FromJust();
				for (int i = 0; i < start; i++) {
					if (sourceline_string[i] == 9) { // \t
						out.push( '\t' );
					} else {
						out.push( ' ' );
					}
				}
				int end = message->GetEndColumn(context).FromJust();
				for (int i = start; i < end; i++) {
					out.push( '^' );
				}
				out.push( '\n' );
				
				if (error->IsObject()) {
					v8::Local<v8::Value> stack_trace_string;
					
					if (error.As<v8::Object>()->Get(_context, newFromOneByte("stack"))
							.ToLocal(&stack_trace_string) &&
							stack_trace_string->IsString() &&
							v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
						v8::String::Utf8Value stack_trace(_isolate, stack_trace_string);
						cChar* stack_trace_string = to_cstring(stack_trace);
						out.push( stack_trace_string ); out.push('\n');
					}
				}
				return out.join(String());
			}
		}

		void print_exception(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
			Qk_ERR("%s", *parse_exception_message(message, error) );
		}

		static void OnFatalError(cChar* location, cChar* message) {
			if (location) {
				Qk_FATAL("FATAL ERROR: %s %s\n", location, message);
			} else {
				Qk_FATAL("FATAL ERROR: %s\n", message);
			}
		}

		static void MessageCallback(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
			worker()->uncaught_exception(message, error);
		}

		static void PromiseRejectCallback(PromiseRejectMessage message) {
			if (message.GetEvent() == v8::kPromiseRejectWithNoHandler) {
				worker()->unhandled_rejection(message);
			}
		}

		void uncaught_exception(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
			if ( !triggerUncaughtException(this, Cast(error)) ) {
				print_exception(message, error);
				qk::thread_try_abort_and_exit(ERR_UNCAUGHT_EXCEPTION);
			}
		}

		void unhandled_rejection(PromiseRejectMessage& message) {
			v8::Local<v8::Promise> promise = message.GetPromise();
			v8::Local<v8::Value> reason = message.GetValue();
			if (reason.IsEmpty())
				reason = v8::Undefined(_isolate);
			if ( !triggerUnhandledRejection(this, Cast(reason), Cast(promise)) ) {
				v8::HandleScope scope(_isolate);
				v8::Local<v8::Message> message = v8::Exception::CreateMessage(_isolate, reason);
				print_exception(message, reason);
				qk::thread_try_abort_and_exit(ERR_UNHANDLED_REJECTION);
			}
		}

	};
	
	template<>
	inline WorkerImpl* WorkerImpl::worker<Worker*>(Worker* worker) {
		return static_cast<WorkerImpl*>(worker);
	}

	template<>
	inline WorkerImpl* WorkerImpl::worker<Isolate*>(Isolate* isolate) {
		return static_cast<WorkerImpl*>( isolate->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
	}

	Worker* Worker::current() {
		return reinterpret_cast<Worker*>(v8::Isolate::GetCurrent()->GetData(ISOLATE_INL_WORKER_DATA_INDEX));
	}

	Worker* Worker::Make() {
		auto o = new WorkerImpl();
		o->init();
		return o;
	}

	// ----------------------------------------------------------------------------------

	class V8JSClass: public JSClass {
	public:
		V8JSClass(Worker* worker, cString& name,
							FunctionCallback constructor, V8JSClass* base,
							v8::Local<v8::Function> baseFunc = v8::Local<v8::Function>())
			: JSClass(), _base(base)
		{ //
			v8::FunctionCallback cb = reinterpret_cast<v8::FunctionCallback>(constructor);
			v8::Local<v8::FunctionTemplate> ft = v8::FunctionTemplate::New(ISOLATE(worker), cb);
			v8::Local<v8::String> className = Back<v8::String>(worker->newStringOneByte(name));

			if ( base ) {
				ft->Inherit( base->Template() );
			}
			else if ( !baseFunc.IsEmpty() ) {
				_baseFunc.Reset(ISOLATE(worker), baseFunc);
			}

			ft->SetClassName(className);
			ft->InstanceTemplate()->SetInternalFieldCount(1);
			_funcTemplate.Reset(ISOLATE(worker), ft);
		}

		~V8JSClass() {
			_baseFunc.Reset();
			_funcTemplate.Reset();
		}

		v8::Local<v8::FunctionTemplate> Template() {
			auto _ = reinterpret_cast<v8::Local<v8::FunctionTemplate>*>(&_funcTemplate);
			return *_;
		}

		v8::Local<v8::Function> BaseFunction() {
			auto _ = reinterpret_cast<v8::Local<v8::Function>*>(&_baseFunc);
			return *_;
		}

		bool HasBaseFunction() {
			return !_baseFunc.IsEmpty();
		}

	private:
		V8JSClass*                   _base;
		v8::Persistent<v8::Function> _baseFunc; // base constructor function
		v8::Persistent<v8::FunctionTemplate> _funcTemplate; // v8 func template
	};

	JSFunction* JSClass::getFunction() {
		if (_func.isEmpty()) {
			// Gen constructor
			auto v8cls = static_cast<V8JSClass*>(this);
			auto f = v8cls->Template()->GetFunction(CONTEXT(_worker)).FromMaybe(v8::Local<v8::Function>());
			if (v8cls->HasBaseFunction()) {
				bool ok;
				// function.__proto__ = base;
				// f->SetPrototype(v8cls->BaseFunction());
				// function.prototype.__proto__ = base.prototype;
				auto str = Back(_worker->strs()->prototype());
				auto base = v8cls->BaseFunction();
				auto proto = f->Get(CONTEXT(_worker), str).ToLocalChecked().As<v8::Object>();
				auto baseProto = base->Get(CONTEXT(_worker), str).ToLocalChecked().As<v8::Object>();
				ok = proto->SetPrototype(CONTEXT(_worker), baseProto).ToChecked();
				Qk_ASSERT(ok);
			}
			auto func = Cast<JSFunction>(f);
			_func.reset(_worker, func);
		}
		return *_func;
	}

	struct V8HandleScopeWrap {
		v8::HandleScope value;
		inline V8HandleScopeWrap(Isolate* isolate): value(isolate) { }
	};

	HandleScope::HandleScope(Worker* worker) {
		new(this) V8HandleScopeWrap(ISOLATE(worker));
	}

	HandleScope::~HandleScope() {
		reinterpret_cast<V8HandleScopeWrap*>(this)->~V8HandleScopeWrap();
	}

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
	bool JSValue::equals(Worker *worker, JSValue* val) const {
		return reinterpret_cast<const v8::Value*>(this)->Equals(CONTEXT(worker), Back(val)).ToChecked();
	}
	bool JSValue::strictEquals(JSValue* val) const {
		return reinterpret_cast<const v8::Value*>(this)->StrictEquals(Back(val));
	}

	JSString* JSValue::toString(Worker* worker) const {
		return Cast<JSString>(reinterpret_cast<const v8::Value*>(this)->ToString(CONTEXT(worker)).ToLocalChecked());
	}

	JSNumber* JSValue::toNumber(Worker* worker) const {
		Cast<JSNumber>(reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker)).ToLocalChecked());
	}

	JSInt32* JSValue::toInt32(Worker* worker) const {
		return Cast<JSInt32>(reinterpret_cast<const v8::Value*>(this)->ToInt32(CONTEXT(worker)).ToLocalChecked());
	}

	JSUint32* JSValue::toUint32(Worker* worker) const {
		return Cast<JSUint32>(reinterpret_cast<const v8::Value*>(this)->ToUint32(CONTEXT(worker)).ToLocalChecked());
	}

	JSObject* JSValue::toObject(Worker* worker) const {
		return Cast<JSObject>(reinterpret_cast<const v8::Value*>(this)->ToObject(CONTEXT(worker)).ToLocalChecked());
	}

	JSBoolean* JSValue::toBoolean(Worker* worker) const {
		return Cast<JSBoolean>(reinterpret_cast<const v8::Value*>(this)->ToBoolean(CONTEXT(worker)).ToLocalChecked());
	}

	String JSValue::toStringValue(Worker* worker, bool oneByte) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		Qk_ASSERT(!str.IsEmpty());
		if (!str->Length()) return String();
		if ( oneByte ) {
			Buffer buffer(str->Length());
			str->WriteOneByte(ISOLATE(worker), (uint8_t*)*buffer, 0, buffer.length() + 1);
			return buffer.collapseString();
		} else {
			Array<String> rev;
			Array<uint16_t> source(128);
			int start = 0, count;
			auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;

			while ( (count = str->Write(ISOLATE(worker), *source, start, 128, opts)) ) {
				auto unicode = codec_decode_form_utf16(source.slice(0, count).buffer());
				rev.push(
					codec_encode(kUTF8_Encoding, unicode).collapseString()
				);
				start += count;
			}
			return rev.join(String());
		}
	}

	String2 JSValue::toStringValue2(Worker* worker) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		Qk_ASSERT(!str.IsEmpty());
		if (!str->Length()) return String2();
		ArrayBuffer<uint16_t> source(str->Length());
		str->Write(ISOLATE(worker), *source, 0, str->Length());
		return source.collapseString();
	}

	String4 JSValue::toStringValue4(Worker* worker) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		Qk_ASSERT(!str.IsEmpty());
		if (!str->Length()) return String4();

		Array<uint32_t> rev(str->Length());
		Array<uint16_t> source(128);
		int start = 0, count, revOffset = 0;
		auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;

		while ( (count = str->Write(ISOLATE(worker), *source, start, 128, opts)) ) {
			auto unicode = codec_decode_form_utf16(source.slice(0, count).buffer());
			rev.write(*unicode, unicode.length(), revOffset);
			start += count;
			revOffset += unicode.length();
		}
		rev.reset(revOffset);
		return rev.collapseString();
	}

	double JSValue::toNumberValue(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker)).ToLocalChecked()->Value();
	}

	int JSValue::toInt32Value(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToInt32(CONTEXT(worker)).ToLocalChecked()->Value();
	}

	uint32_t JSValue::toUint32Value(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToUint32(CONTEXT(worker)).ToLocalChecked()->Value();
	}

	bool JSValue::toBooleanValue(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToBoolean(CONTEXT(worker)).ToLocalChecked()->Value();
	}

	bool JSValue::instanceOf(Worker* worker, JSObject* value) {
		return reinterpret_cast<v8::Value*>(this)->
			InstanceOf(CONTEXT(worker), Back<v8::Object>(value)).FromMaybe(false);
	}

	JSValue* JSObject::get(Worker* worker, JSValue* key) {
		return Cast(reinterpret_cast<v8::Object*>(this)->
								Get(CONTEXT(worker), Back(key)).ToLocalChecked());
	}

	JSValue* JSObject::get(Worker* worker, uint32_t index) {
		return Cast(reinterpret_cast<v8::Object*>(this)->
								Get(CONTEXT(worker), index).ToLocalChecked());
	}

	bool JSObject::set(Worker* worker, JSValue* key, JSValue* val) {
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), Back(key), Back(val)).FromMaybe(false);
	}

	bool JSObject::set(Worker* worker, uint32_t index, JSValue* val) {
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), index, Back(val)).FromMaybe(false);
	}

	bool JSObject::has(Worker* worker, JSValue* key) {
		return reinterpret_cast<v8::Object*>(this)->
			Has(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::has(Worker* worker, uint32_t index) {
		return reinterpret_cast<v8::Object*>(this)->
			Has(CONTEXT(worker), index).FromMaybe(false);
	}

	bool JSObject::JSObject::Delete(Worker* worker, JSValue* key) {
		return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::Delete(Worker* worker, uint32_t index) {
		return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), index).FromMaybe(false);
	}

	JSArray* JSObject::getPropertyNames(Worker* worker) {
		return Cast<JSArray>(reinterpret_cast<v8::Object*>(this)->
												GetPropertyNames(CONTEXT(worker)).FromMaybe(v8::Local<v8::Array>()));
	}

	JSFunction* JSObject::getConstructor(Worker* worker) {
		auto rv = reinterpret_cast<v8::Object*>(this)->
			Get(CONTEXT(worker), Back(worker->strs()->constructor()));
		return Cast<JSFunction>(rv.FromMaybe(v8::Local<v8::Value>()));
	}

	bool JSObject::setMethod(Worker* worker, cString& name, FunctionCallback func) {
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
		auto get2 = reinterpret_cast<v8::AccessorNameGetterCallback>(get);
		auto set2 = reinterpret_cast<v8::AccessorNameSetterCallback>(set);
		v8::Local<v8::String> fn_name = Back<v8::String>(worker->newStringOneByte(name));
		return reinterpret_cast<v8::Object*>(this)->SetAccessor(CONTEXT(worker), fn_name, get2, set2).ToChecked();
	}

	void* JSObject::objectPrivate() {
		auto self = reinterpret_cast<v8::Object*>(this);
		if (self->InternalFieldCount() > 0) {
			return self->GetAlignedPointerFromInternalField(0);
		}
		return nullptr;
	}

	bool JSObject::setObjectPrivate(void *value) {
		auto self = reinterpret_cast<v8::Object*>(this);
		if (self->InternalFieldCount() > 0) {
			self->SetAlignedPointerInInternalField(0, value);
			return true;
		}
		return false;
	}

	bool JSObject::set__Proto__(Worker* worker, JSObject* __proto__) {
		return reinterpret_cast<v8::Object*>(this)->
			SetPrototype(CONTEXT(worker), Back(__proto__)).FromMaybe(false);
	}

	int JSString::length() const {
		return reinterpret_cast<const v8::String*>(this)->Length();
	}
	String JSString::value(Worker* worker, bool oneByte) const {
		return toStringValue(worker, oneByte);
	}
	String2 JSString::value2(Worker* worker) const {
		return toStringValue2(worker);
	}
	JSString* JSString::Empty(Worker* worker) {
		return Cast<JSString>(v8::String::Empty(ISOLATE(worker)));
	}
	int JSArray::length() const {
		return reinterpret_cast<const v8::Array*>(this)->Length();
	}
	double JSDate::valueOf(Worker* worker) const {
		return reinterpret_cast<const v8::Date*>(this)->ValueOf();
	}
	double JSNumber::value(Worker* worker) const {
		return reinterpret_cast<const v8::Number*>(this)->Value();
	}
	int JSInt32::value(Worker* worker) const {
		return reinterpret_cast<const v8::Int32*>(this)->Value();
	}
	int64_t JSInteger::value(Worker* worker) const {
		return reinterpret_cast<const v8::Integer*>(this)->Value();
	}
	uint32_t JSUint32::value(Worker* worker) const {
		return reinterpret_cast<const v8::Uint32*>(this)->Value();
	}
	bool JSBoolean::value(Worker* worker) const {
		return reinterpret_cast<const v8::Boolean*>(this)->Value();
	}

	JSValue* JSFunction::call(Worker* worker, int argc, JSValue* argv[], JSValue* recv) {
		if ( !recv ) {
			recv = worker->newUndefined();
		}
		auto fn = reinterpret_cast<v8::Function*>(this);
		v8::MaybeLocal<v8::Value> r = fn->Call(CONTEXT(worker), Back(recv), argc,
																						reinterpret_cast<v8::Local<v8::Value>*>(argv));
		return Cast(r.ToLocalChecked());
	}

	JSValue* JSFunction::call(Worker* worker, JSValue* recv) {
		return call(worker, 0, nullptr, recv);
	}

	JSObject* JSFunction::newInstance(Worker* worker, int argc, JSValue* argv[]) {
		auto fn = reinterpret_cast<v8::Function*>(this);
		v8::MaybeLocal<v8::Object> r = fn->NewInstance(CONTEXT(worker), argc,
																										reinterpret_cast<v8::Local<v8::Value>*>(argv));
		return Cast<JSObject>(r.ToLocalChecked());
	}

	JSObject* JSFunction::getPrototype(Worker* worker) {
		auto fn = reinterpret_cast<v8::Function*>(this);
		auto str = Back(worker->strs()->prototype());
		auto r = fn->Get(CONTEXT(worker), str);
		return Cast<JSObject>(r.ToLocalChecked());
	}

	uint32_t JSArrayBuffer::byteLength(Worker* worker) const {
		return (uint32_t)reinterpret_cast<const v8::ArrayBuffer*>(this)->ByteLength();
	}

	Char* JSArrayBuffer::data(Worker* worker) {
		return (Char*)reinterpret_cast<v8::ArrayBuffer*>(this)->GetContents().Data();
	}

	JSArrayBuffer* JSTypedArray::buffer(Worker* worker) {
		auto typedArray = reinterpret_cast<v8::TypedArray*>(this);
		v8::Local<v8::ArrayBuffer> abuff = typedArray->Buffer();
		return Cast<JSArrayBuffer>(abuff);
	}

	uint32_t JSTypedArray::byteLength(Worker* worker) {
		return (uint32_t)reinterpret_cast<v8::TypedArray*>(this)->ByteLength();
	}

	uint32_t JSTypedArray::byteOffset(Worker* worker) {
		return (uint32_t)reinterpret_cast<v8::TypedArray*>(this)->ByteOffset();
	}

	bool JSSet::add(Worker* worker, JSValue* key) {
		auto set = reinterpret_cast<v8::Set*>(this);
		return !set->Add(CONTEXT(worker), Back(key)).IsEmpty();
	}

	bool JSSet::has(Worker* worker, JSValue* key) {
		auto set = reinterpret_cast<v8::Set*>(this);
		return set->Has(CONTEXT(worker), Back(key)).ToChecked();
	}
		
	bool JSSet::Delete(Worker* worker, JSValue* key) {
		auto set = reinterpret_cast<v8::Set*>(this);
		return set->Delete(CONTEXT(worker), Back(key)).ToChecked();
	}
		
	bool JSClass::hasInstance(JSValue* val) {
		return reinterpret_cast<V8JSClass*>(this)->Template()->HasInstance(Back(val));
	}

	bool JSClass::setMemberMethod(cString& name, FunctionCallback func) {
		v8::Local<v8::FunctionTemplate> ftemp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		v8::Local<Signature> sign = Signature::New(ISOLATE(_worker), ftemp);
		v8::Local<v8::FunctionTemplate> t =
			FunctionTemplate::New(ISOLATE(_worker), func2, v8::Local<v8::Value>(), sign);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		t->SetClassName(fn_name);
		ftemp->PrototypeTemplate()->Set(fn_name, t);
		return true;
	}

	bool JSClass::setMemberAccessor(cString& name,
																	AccessorGetterCallback get, AccessorSetterCallback set) {
		v8::Local<v8::FunctionTemplate> temp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		v8::Local<AccessorSignature> s = AccessorSignature::New(ISOLATE(_worker), temp);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		temp->PrototypeTemplate()->SetAccessor(fn_name, get2, set2,
																					v8::Local<v8::Value>(), v8::DEFAULT, v8::None, s);
		return true;
	}

	bool JSClass::setMemberIndexedAccessor(IndexedAccessorGetterCallback get,
																				IndexedAccessorSetterCallback set) {
		v8::IndexedPropertyGetterCallback get2 = reinterpret_cast<v8::IndexedPropertyGetterCallback>(get);
		v8::IndexedPropertySetterCallback set2 = reinterpret_cast<v8::IndexedPropertySetterCallback>(set);
		v8::IndexedPropertyHandlerConfiguration cfg(get2, set2);
		reinterpret_cast<V8JSClass*>(this)->Template()->PrototypeTemplate()->SetHandler(cfg);
		return true;
	}

	template<>
	bool JSClass::setMemberProperty<JSValue*>(cString& name, JSValue* value) {
		reinterpret_cast<V8JSClass*>(this)->Template()->
						PrototypeTemplate()->Set(Back<v8::String>(_worker->newStringOneByte(name)), Back(value));
		return true;
	}

	template<>
	bool JSClass::setStaticProperty<JSValue*>(cString& name, JSValue* value) {
		reinterpret_cast<V8JSClass*>(this)->Template()->
						Set(Back<v8::String>(_worker->newStringOneByte(name)), Back(value));
		return true;
	}

	void ReturnValue::set(bool value) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(value);
	}

	void ReturnValue::set(double i) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::set(int i) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::set(uint32_t i) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::setNull() {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetNull();
	}

	void ReturnValue::setUndefined() {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetUndefined();
	}

	void ReturnValue::setEmptyString() {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetEmptyString();
	}

	void ReturnValue::set(JSValue* value) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(Back(value));
	}

	int FunctionCallbackInfo::length() const {
		return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->Length();
	}

	JSValue* FunctionCallbackInfo::operator[](int i) const {
		return Cast(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->operator[](i));
	}

	JSObject* FunctionCallbackInfo::This() const {
		return Cast<JSObject>(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->This());
	}

	bool FunctionCallbackInfo::isConstructCall() const {
		return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->IsConstructCall();
	}

	ReturnValue FunctionCallbackInfo::returnValue() const {
		auto info = reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this);
		v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		auto _ = reinterpret_cast<ReturnValue*>(&rv);
		return *_;
	}

	JSObject* PropertyCallbackInfo::This() const {
		return Cast<JSObject>(reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this)->This());
	}

	ReturnValue PropertyCallbackInfo::returnValue() const {
		auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
		v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		auto _ = reinterpret_cast<ReturnValue*>(&rv);
		return *_;
	}

	JSObject* PropertySetCallbackInfo::This() const {
		return Cast<JSObject>(reinterpret_cast<const v8::PropertyCallbackInfo<void>*>(this)->This());
	}

	Worker* FunctionCallbackInfo::worker() const {
		auto info = reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

	Worker* PropertyCallbackInfo::worker() const {
		auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

	Worker* PropertySetCallbackInfo::worker() const {
		auto info = reinterpret_cast<const v8::PropertyCallbackInfo<void>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

	struct TryCatchWrap {
		TryCatchWrap(Worker *worker)
			: _try(ISOLATE(worker)) {}
		v8::TryCatch _try;
	};

	TryCatch::TryCatch(Worker *worker) {
		_val = new TryCatchWrap(worker);
	}

	TryCatch::~TryCatch() {
		delete reinterpret_cast<TryCatchWrap*>(_val);
		_val = nullptr;
	}

	bool TryCatch::hasCaught() const {
		return reinterpret_cast<TryCatchWrap*>(_val)->_try.HasCaught();
	}

	Worker* WeakCallbackInfo::worker() const {
		auto info = reinterpret_cast<const v8::WeakCallbackInfo<Object>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

	void* WeakCallbackInfo::getParameter() const {
		return reinterpret_cast<const v8::WeakCallbackInfo<void>*>(this)->GetParameter();
	}

	template <> void Persistent<JSValue>::reset() {
		reinterpret_cast<v8::Persistent<v8::Value>*>(this)->Reset();
	}

	template <> template <>
	void Persistent<JSValue>::reset(Worker* worker, JSValue* other) {
		Qk_ASSERT(worker);
		reinterpret_cast<v8::Persistent<v8::Value>*>(this)->
			Reset(ISOLATE(worker), *reinterpret_cast<const v8::Local<v8::Value>*>(&other));
		_worker = worker;
	}

	template<> template<>
	void Persistent<JSValue>::copy(const Persistent<JSValue>& that) {
		reset();
		if (that.isEmpty())
			return;
		Qk_ASSERT(that._worker);
		typedef v8::CopyablePersistentTraits<v8::Value>::CopyablePersistent Handle;
		reinterpret_cast<Handle*>(this)->operator=(*reinterpret_cast<const Handle*>(&that));
		_worker = that._worker;
	}

	template <>
	bool Persistent<JSValue>::isWeak() {
		Qk_ASSERT( _val );
		auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(this);
		return h->IsWeak();
	}

	template <>
	void Persistent<JSValue>::setWeak(void* ptr, WeakCallback callback) {
		Qk_ASSERT( !isEmpty() );
		auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(this);
		h->MarkIndependent();
		h->SetWeak(ptr, reinterpret_cast<v8::WeakCallbackInfo<void>::Callback>(callback),
							v8::WeakCallbackType::kParameter);
	}

	template <>
	void Persistent<JSValue>::clearWeak() {
		Qk_ASSERT( !isEmpty() );
		auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(this);
		h->ClearWeak();
	}

	JSNumber* Worker::newInstance(float data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newInstance(double data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSBoolean* Worker::newInstance(bool data) {
		return Cast<JSBoolean>(v8::Boolean::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newInstance(Char data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newInstance(uint8_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newInstance(int16_t data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newInstance(uint16_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newInstance(int data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newInstance(uint32_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newInstance(int64_t data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newInstance(uint64_t data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSString* Worker::newString(cBuffer& data) {
		return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newInstance(cString& data) {
		return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newInstance(cString2& data) {
		return Cast<JSString>(v8::String::NewExternalTwoByte(
			ISOLATE(this),
			new V8ExternalStringResource(data)
		).ToLocalChecked());
	}

	JSUint8Array* Worker::newInstance(Buffer&& buff) {
		size_t offset = 0;
		size_t len = buff.length();
		v8::Local<v8::ArrayBuffer> ab;
		if (buff.length()) {
			size_t len = buff.length();
			Char* data = buff.collapse();
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
		offset = Qk_MIN((uint)ab2->ByteLength(), offset);
		if (size + offset > ab2->ByteLength()) {
			size = (uint)ab2->ByteLength() - offset;
		}
		return Cast<JSUint8Array>(v8::Uint8Array::New(ab2, offset, size));
	}

	JSObject* Worker::newRangeError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		auto str = _Str::string_format(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::RangeError(Back(newInstance(str))->ToString(CONTEXT(this)).ToLocalChecked()));
	}

	JSObject* Worker::newReferenceError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		auto str = _Str::string_format(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::ReferenceError(Back(newInstance(str))->ToString(CONTEXT(this)).ToLocalChecked()));
	}

	JSObject* Worker::newSyntaxError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		auto str = _Str::string_format(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::SyntaxError(Back(newInstance(str))->ToString(CONTEXT(this)).ToLocalChecked()));
	}

	JSObject* Worker::newTypeError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		auto str = _Str::string_format(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::TypeError(Back(newInstance(str))->ToString(CONTEXT(this)).ToLocalChecked()));
	}

	JSObject* Worker::newInstance(cError& err) {
		v8::Local<v8::Object> e =
			v8::Exception::Error(Back<v8::String>(newInstance(err.message()))).As<v8::Object>();
		e->Set(Back(strs()->Errno()), Back(newInstance(err.code())));
		return Cast<JSObject>(e);
	}

	void Worker::throwError(JSValue* exception) {
		ISOLATE(this)->ThrowException(Back(exception));
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach_callback, JSClass* base) {
		auto cls = new V8JSClass(this, name, constructor, static_cast<V8JSClass*>(base));
		_classsinfo->add(id, cls, attach_callback);
		return cls;
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach_callback, uint64_t base) {
		return newClass(name, id, constructor, attach_callback, _classsinfo->get(base));
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach_callback, JSFunction* base) {
		auto cls = new V8JSClass(this, name, constructor, nullptr, Back<v8::Function>(base));
		_classsinfo->add(id, cls, attach_callback);
		return cls;
	}

	void Worker::reportException(TryCatch* try_catch) {
		TryCatchWrap* wrap = *reinterpret_cast<TryCatchWrap**>(try_catch);
		WORKER(this)->print_exception(wrap->_try.Message(), wrap->_try.Exception());
	}

	JSValue* Worker::runScript(JSString* source, JSString* name, JSObject* sandbox) {
		v8::MaybeLocal<v8::Value> r = WORKER(this)->runScript(Back<v8::String>(source),
																													Back<v8::String>(name),
																													Back<v8::Object>(sandbox));
		return Cast(r.FromMaybe(v8::Local<v8::Value>()));
	}

	JSValue* Worker::runScript(cString& source, cString& name, JSObject* sandbox) {
		return runScript(newInstance(source), newInstance(name), sandbox);
	}

	JSValue* Worker::runNativeScript(cBuffer& source, cString& name, JSObject* exports) {
		v8::EscapableHandleScope scope(ISOLATE(this));
		if (!exports) {
			exports = newObject();
		}
		auto r = WORKER(this)->runNativeScript(source, name, exports);
		return Cast(scope.Escape(Back(r)));
	}

	void Worker::garbageCollection() {
		ISOLATE(this)->LowMemoryNotification();
	}

	int platformStart(int argc, Char** argv, int (*exec)(Worker *worker)) {
		auto platform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(platform.get());
		v8::V8::Initialize();

		// Unconditionally force typed arrays to allocate outside the v8 heap. This
		// is to prevent memory pointers from being moved around that are returned by
		// Buffer::Data().
		cChar no_typed_array_heap[] = "--typed_array_max_size_in_heap=0";
		v8::V8::SetFlagsFromString(no_typed_array_heap, sizeof(no_typed_array_heap) - 1);
		v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

		int rc = 0;
		{
			Sp<Worker> worker = Worker::Make();
			v8::SealHandleScope sealhandle(ISOLATE(*worker));
			rc = exec(*worker);
		}
		v8::V8::ShutdownPlatform();
		v8::V8::Dispose();

		return rc;
	}

} }
