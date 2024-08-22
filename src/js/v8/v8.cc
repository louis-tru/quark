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
#include "../js_.h"
#include "../../errno.h"
#include "../../util/codec.h"
#include "./inspector_agent.h"

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

	template<class T = JSValue, class S>
	inline T* Cast(v8::MaybeLocal<S> o) { return *reinterpret_cast<T**>(&o); }

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
		inspector::Agent* _inspector;
		Isolate::CreateParams _params;

		WorkerImpl(): _locker(nullptr), _handle_scope(nullptr), _inspector(nullptr)
		{
			_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
			_isolate = Isolate::New(_params);
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

		void runDebugger(const DebugOptions &opts) {
			if (!_inspector)
				_inspector = new inspector::Agent(this);
			_inspector->Start(opts);
		}

		void stopDebugger() {
			if (_inspector)
				_inspector->Stop();
		}

		void debuggerBreakNextStatement() {
			if (_inspector) {
				_inspector->PauseOnNextJavascriptStatement("Break on start");
			}
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
			if (!exports) exports = newObject();

			v8::EscapableHandleScope scope(_isolate);
			v8::Local<v8::Value> _name = Back(newInstance(name));
			v8::Local<v8::Value> _souece = Back(newString(source));
			v8::MaybeLocal<v8::Value> rv;

			rv = runScript(_souece.As<v8::String>(),
											_name.As<v8::String>(), v8::Local<v8::Object>());
			if ( !rv.IsEmpty() ) {
				auto mod = newObject();
				mod->set(this, strs()->exports(), exports);
				v8::Local<v8::Function> func = rv.ToLocalChecked().As<v8::Function>();
				v8::Local<v8::Value> args[] = { Back(exports), Back(mod), Back(global()) };
				// '(function (exports, require, module, __filename, __dirname) {', '\n})'
				rv = func->Call(_context, v8::Undefined(_isolate), 3, args);
				if (!rv.IsEmpty()) {
					auto rv = mod->get(this, strs()->exports());
					Qk_ASSERT(rv->isObject());
					return Cast(scope.Escape(Back(rv)));
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
			// print_exception(message, error);
			if ( !triggerUncaughtException(this, Cast(error)) ) {
				print_exception(message, error);
				qk::thread_exit(ERR_UNCAUGHT_EXCEPTION);
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
				qk::thread_exit(ERR_UNHANDLED_REJECTION);
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

	v8::Isolate* getIsolate(Worker* worker) {
		return static_cast<WorkerImpl*>(worker)->_isolate;
	}

	v8::Local<v8::Context> getContext(Worker* worker) {
		return static_cast<WorkerImpl*>(worker)->_context;
	}

	// ----------------------------------------------------------------------------------

	class V8JSClass: public JSClass {
	public:
		V8JSClass(Worker* worker, cString& name,
							FunctionCallback constructor, AttachCallback attach, V8JSClass* base,
							v8::Local<v8::Function> baseFunc = v8::Local<v8::Function>())
			: JSClass(constructor, attach), _base(base)
		{
			_worker = worker;
			auto data = External::New(ISOLATE(worker), this);
			auto cb = (v8::FunctionCallback)[](const v8::FunctionCallbackInfo<v8::Value>& info) {
				auto self = (V8JSClass*)info.Data().As<External>()->Value();
				if (!self->_worker->classsinfo()->isAttachFlag()) {
					self->_constructor(*reinterpret_cast<const FunctionCallbackInfo*>(&info));
				}
			};
			v8::Local<v8::FunctionTemplate> ft = v8::FunctionTemplate::New(ISOLATE(worker), cb, data);
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
			auto f = v8cls->Template()->GetFunction(CONTEXT(_worker)).ToLocalChecked();
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
		return Cast<JSString>(reinterpret_cast<const v8::Value*>(this)->ToString(CONTEXT(worker)));
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

	JSObject* JSValue::toObject(Worker* worker) const {
		return Cast<JSObject>(reinterpret_cast<const v8::Value*>(this)->ToObject(CONTEXT(worker)));
	}

	JSBoolean* JSValue::toBoolean(Worker* worker) const {
		return Cast<JSBoolean>(reinterpret_cast<const v8::Value*>(this)->ToBoolean(CONTEXT(worker)));
	}

	String JSValue::toStringValue(Worker* worker, bool oneByte) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		if (!str->Length()) return String();
		if ( oneByte ) {
			Buffer buffer(str->Length());
			str->WriteOneByte(ISOLATE(worker), (uint8_t*)*buffer, 0, buffer.capacity());
			return buffer.collapseString();
		} else {
			uint16_t source[128];
			int start = 0, count;
			auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;
			Array<String> rev;

			while ( (count = str->Write(ISOLATE(worker), source, start, 128, opts)) ) {
				auto unicode = codec_decode_form_utf16(ArrayWeak<uint16_t>(source, count).buffer());
				rev.push(codec_encode(kUTF8_Encoding, unicode).collapseString());
				start += count;
			}

			return rev.length() == 0 ? String():
						 rev.length() == 1 ? rev[0]:
						 rev.join(String());
		}
	}

	String2 JSValue::toStringValue2(Worker* worker) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		if (!str->Length()) return String2();
		ArrayBuffer<uint16_t> source(str->Length());
		str->Write(ISOLATE(worker), *source, 0, str->Length());
		return source.collapseString();
	}

	String4 JSValue::toStringValue4(Worker* worker) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		if (!str->Length()) return String4();

		uint16_t source[128];
		int start = 0, count, revOffset = 0;
		auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;
		Array<uint32_t> rev(str->Length());

		while ( (count = str->Write(ISOLATE(worker), source, start, 128, opts)) ) {
			auto unicode = codec_decode_form_utf16(ArrayWeak<uint16_t>(source, count).buffer());
			rev.write(*unicode, unicode.length(), revOffset);
			revOffset += unicode.length();
			start += count;
		}
		rev.reset(revOffset);
		return rev.collapseString();
	}

	Maybe<float> JSValue::toFloatValue(Worker* worker) const {
		auto v = reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker));
		return v.IsEmpty() ? Maybe<float>(): Maybe<float>(Cast<JSNumber>(v)->value());
	}

	Maybe<double> JSValue::toNumberValue(Worker* worker) const {
		auto v = reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker));
		return v.IsEmpty() ? Maybe<double>(): Maybe<double>(Cast<JSNumber>(v)->value());
	}

	Maybe<int> JSValue::toInt32Value(Worker* worker) const {
		auto v = reinterpret_cast<const v8::Value*>(this)->ToInt32(CONTEXT(worker));
		return v.IsEmpty() ? Maybe<int>(): Maybe<int>(Cast<JSInt32>(v)->value());
	}

	Maybe<uint32_t> JSValue::toUint32Value(Worker* worker) const {
		auto v = reinterpret_cast<const v8::Value*>(this)->ToUint32(CONTEXT(worker));
		return v.IsEmpty() ? Maybe<uint32_t>(): Maybe<uint32_t>(Cast<JSUint32>(v)->value());
	}

	bool JSValue::toBooleanValue(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToBoolean(CONTEXT(worker)).ToLocalChecked()->Value();
	}

	bool JSValue::instanceOf(Worker* worker, JSObject* value) {
		return reinterpret_cast<v8::Value*>(this)->
			InstanceOf(CONTEXT(worker), Back<v8::Object>(value)).FromMaybe(false);
	}

	JSValue* JSObject::get(Worker* worker, JSValue* key) {
		return Cast(reinterpret_cast<v8::Object*>(this)->Get(CONTEXT(worker), Back(key)));
	}

	JSValue* JSObject::get(Worker* worker, uint32_t index) {
		return Cast(reinterpret_cast<v8::Object*>(this)->Get(CONTEXT(worker), index));
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

	bool JSObject::defineOwnProperty(Worker *worker, JSValue *key, JSValue *value, int flags) {
		auto name = v8::Name::Cast(reinterpret_cast<v8::Value*>(key));
		auto name_ = *reinterpret_cast<v8::Local<v8::Name>*>(&name);
		return reinterpret_cast<v8::Object*>(this)->
			DefineOwnProperty(CONTEXT(worker), name_, Back(value), v8::PropertyAttribute(flags))
			.FromMaybe(false);
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
	JSString* JSString::Empty(Worker* worker) {
		return Cast<JSString>(v8::String::Empty(ISOLATE(worker)));
	}
	int JSArray::length() const {
		return reinterpret_cast<const v8::Array*>(this)->Length();
	}
	double JSDate::valueOf() const {
		return reinterpret_cast<const v8::Date*>(this)->ValueOf();
	}
	double JSNumber::value() const {
		return reinterpret_cast<const v8::Number*>(this)->Value();
	}
	int JSInt32::value() const {
		return reinterpret_cast<const v8::Int32*>(this)->Value();
	}
	int64_t JSInteger::value() const {
		return reinterpret_cast<const v8::Integer*>(this)->Value();
	}
	uint32_t JSUint32::value() const {
		return reinterpret_cast<const v8::Uint32*>(this)->Value();
	}
	bool JSBoolean::value() const {
		return reinterpret_cast<const v8::Boolean*>(this)->Value();
	}

	JSValue* JSFunction::call(Worker* worker, int argc, JSValue* argv[], JSValue* recv) {
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
		auto fn = reinterpret_cast<v8::Function*>(this);
		v8::MaybeLocal<v8::Object> r = fn->NewInstance(CONTEXT(worker), argc,
																										reinterpret_cast<v8::Local<v8::Value>*>(argv));
		return Cast<JSObject>(r);
	}

	JSObject* JSFunction::getPrototype(Worker* worker) {
		auto fn = reinterpret_cast<v8::Function*>(this);
		auto str = Back(worker->strs()->prototype());
		auto r = fn->Get(CONTEXT(worker), str);
		return Cast<JSObject>(r);
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
		v8::Local<AccessorSignature> sign = AccessorSignature::New(ISOLATE(_worker), temp);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		temp->PrototypeTemplate()->SetAccessor(fn_name, get2, set2,
																					v8::Local<v8::Value>(), v8::DEFAULT, v8::None, sign);
		return true;
	}

	/*
	bool JSClass::setLazyDataProperty(cString& name, AccessorGetterCallback get) {
		v8::Local<v8::FunctionTemplate> temp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		// v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		v8::Local<AccessorSignature> s = AccessorSignature::New(ISOLATE(_worker), temp);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		temp->PrototypeTemplate()->SetAccessor(fn_name, get2, nullptr,
																					v8::Local<v8::Value>(), v8::DEFAULT, v8::None, s);
		return true;
	}*/

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

	template<>
	JSValue* Persistent<JSValue>::operator*() const {
		// return Cast(Local<Value>::New(ISOLATE(_worker), Back(_val)));
		return _val;
	}

	JSNumber* Worker::newInstance(float data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newInstance(double data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSBoolean* Worker::newBool(bool data) {
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
		));
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
		auto str = _Str::printfv(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::RangeError(Back<v8::String>(newInstance(str))));
	}

	JSObject* Worker::newReferenceError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		auto str = _Str::printfv(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::ReferenceError(Back<v8::String>(newInstance(str))));
	}

	JSObject* Worker::newSyntaxError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		auto str = _Str::printfv(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::SyntaxError(Back<v8::String>(newInstance(str))));
	}

	JSObject* Worker::newTypeError(cChar* errmsg, ...) {
		va_list arg;
		va_start(arg, errmsg);
		auto str = _Str::printfv(errmsg, arg);
		va_end(arg);
		return Cast<JSObject>(v8::Exception::TypeError(Back<v8::String>(newInstance(str))));
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
															AttachCallback attach, JSClass* base) {
		auto cls = new V8JSClass(this, name, constructor, attach, static_cast<V8JSClass*>(base));
		_classsinfo->add(id, cls);
		return cls;
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach, uint64_t base) {
		return newClass(name, id, constructor, attach, _classsinfo->get(base));
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
															FunctionCallback constructor,
															AttachCallback attach, JSFunction* base) {
		auto cls = new V8JSClass(this, name, constructor, attach, nullptr, Back<v8::Function>(base));
		_classsinfo->add(id, cls);
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
		return WORKER(this)->runNativeScript(source, name, exports);
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

	int platformStart(int argc, Char** argv, int (*exec)(Worker *worker)) {
		auto platform = v8::platform::NewDefaultPlatform(1);
		v8::V8::InitializePlatform(platform.get());
		v8::V8::Initialize();
		v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

		int rc = 0;
		{ //
			Sp<Worker> worker = Worker::Make();
			v8::SealHandleScope sealhandle(ISOLATE(*worker));
			v8::HandleScope handle(ISOLATE(*worker));
			// Startup debugger
			for (int i = 2; i < argc; i++) {
				String arg(argv[i]);
				if (arg.indexOf("--inspect") == 0) {
					auto script_path = fs_reader()->format(argv[1]);
					auto kv = arg.split('=');
					bool brk = arg.indexOf("-brk") != -1;
					if (kv.length() == 1) {
						runDebugger(*worker, {brk,9229,"127.0.0.1",script_path});
					} else {
						auto host = kv[1].split(':');
						int port = 9229;
						if (host.length() > 1) host[1].toNumber<int>(&port);
						runDebugger(*worker, {brk,port,host[0],script_path});
					}
					break;
				}
			}
			rc = exec(*worker); // exec main script
			
		}
		v8::V8::ShutdownPlatform();
		v8::V8::Dispose();

		return rc;
	}
} }
