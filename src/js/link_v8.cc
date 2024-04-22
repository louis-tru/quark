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
#include "../util/util.h"
#include "../util/http.h"
#include "../ui/view/view.h"
#include "../errno.h"
#include "./js.h"
#include "./types.h"
#include <native-inl-js.h>
#include <uv.h>

#if USE_JSC
#include <JavaScriptCore/JavaScriptCore.h>
#define JSC_ENV(...) \
auto isolate = ISOLATE(__VA_ARGS__); \
auto ctx = v8::javascriptcore_context(isolate); \
JSValueRef ex = nullptr
#define OK(...) &ex); do { \
if (ex) { \
	isolate->ThrowException(*reinterpret_cast<v8::Local<v8::Value>*>(&ex)); \
	return __VA_ARGS__ ;\
}}while(0
namespace v8 {
	JSGlobalContextRef javascriptcore_context(v8::Isolate* isolate);
	struct JSCStringTraits: public qk::NonObjectTraits {
		inline static void Release(JSStringRef str) {
			if ( str ) JSStringRelease(str);
		}
	};
	typedef qk::Handle<OpaqueJSString, JSCStringTraits> JSCStringPtr;
}
#endif

namespace qk { namespace js {
	using namespace native_js;
	using namespace v8;

	#ifndef ISOLATE_INL_WORKER_DATA_INDEX
	# define ISOLATE_INL_WORKER_DATA_INDEX (0)
	#endif

	#define WORKER(...) WorkerImpl::woeker( __VA_ARGS__ )
	#define ISOLATE(...) WorkerImpl::woeker( __VA_ARGS__ )->_isolate
	#define CONTEXT(...) WorkerImpl::woeker( __VA_ARGS__ )->_context

	static v8::Platform* platform = nullptr;
	static String unknown("[Unknown]");
	static String2 unknown_ucs2(String("[Unknown]"));

	typedef const v8::FunctionCallbackInfo<Value>& V8FunctionCall;
	typedef const v8::PropertyCallbackInfo<Value>& V8PropertyCall;
	typedef const v8::PropertyCallbackInfo<void>&  V8PropertySetCall;

	template<class T = JSValue, class S>
	inline Local<T> Cast(v8::Local<S> o) {
		return *reinterpret_cast<Local<T>*>(&o);
	}

	template<class T = v8::Value, class S>
	inline v8::Local<T> Back(Local<S> o) {
		return *reinterpret_cast<v8::Local<T>*>(&o);
	}

	// ----------------------------------------------------------------------------------

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

	// ----------------------------------------------------------------------------------

	Worker* Worker::current() {
		return reinterpret_cast<Worker*>(v8::Isolate::GetCurrent()->GetData(ISOLATE_INL_WORKER_DATA_INDEX));
	}

	Worker* Worker::Make() {
		auto o = new WorkerImpl();
		o->init();
		return o;
	}

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

		virtual void init() {
			_isolate->SetData(ISOLATE_INL_WORKER_DATA_INDEX, _host);
			_global.reset(_host, Cast<JSObject>(_context->Global()) );
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

		inline static WorkerImpl* worker(Worker* worker = Worker::current()) {
			return static_cast<WorkerImpl*>(worker);
		}

		inline static WorkerImpl* worker(Isolate* isolate) {
			return static_cast<WorkerImpl*>( isolate->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
		}

		template<class Args>
		inline static WorkerImpl* worker(const Args &args) {
			return static_cast<WorkerImpl*>( args.GetIsolate()->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
		}

		inline v8::Local<v8::String> newFromOneByte(cChar* str) {
			return v8::String::NewFromOneByte(_isolate, (uint8_t*)str);
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

		Local<JSValue> runNativeScript(cBuffer& source, cString& name, Local<JSObject> exports) {
			v8::Local<v8::Value> _name = Back(_host->New(String::format("%s", *name)));
			v8::Local<v8::Value> _souece = Back(_host->NewString(source));

			v8::MaybeLocal<v8::Value> rv;

			rv = runScript(_souece.As<v8::String>(),
											_name.As<v8::String>(), v8::Local<v8::Object>());
			if ( !rv.IsEmpty() ) {
				Local<JSObject> module = _host->NewObject();
				module->Set(_host, _host->strs()->exports(), exports);
				v8::Local<v8::Function> func = rv.ToLocalChecked().As<v8::Function>();
				v8::Local<v8::Value> args[] = { Back(exports), Back(module), Back(global()) };
				rv = func->Call(CONTEXT(_host), v8::Undefined(ISOLATE(this)), 3, args);
				if (!rv.IsEmpty()) {
					Local<JSValue> rv = module->Get(_host, _host->strs()->exports());
					Qk_ASSERT(rv->IsObject(_host));
					return rv;
				}
			}
			return Local<JSValue>();
		}

		// Extracts a C string from a V8 Utf8Value.
		static cChar* to_cstring(const v8::String::Utf8Value& value) {
			return *value ? *value : "<string conversion failed>";
		}

		String parse_exception_message(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
			v8::HandleScope handle_scope(_isolate);
			v8::String::Utf8Value exception(error);
			cChar* exception_string = to_cstring(exception);
			if (message.IsEmpty()) {
				// V8 didn't provide any extra information about this error; just
				return exception_string;
			} else {
				StringBuilder out;
				// Print (filename):(line number): (message).
				v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
				v8::Local<v8::Context> context(_isolate->GetCurrentContext());
				cChar* filename_string = to_cstring(filename);
				int linenum = message->GetLineNumber(context).FromJust();
				out.push(String::format("%s:%d: %s\n", filename_string, linenum, exception_string));
				// Print line of source code.
				v8::String::Utf8Value sourceline(message->GetSourceLine(context).ToLocalChecked());
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
					
					if (error.As<v8::Object>()->Get(_context, NewFromOneByte("stack"))
							.ToLocal(&stack_trace_string) &&
							stack_trace_string->IsString() &&
							v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
						v8::String::Utf8Value stack_trace(stack_trace_string);
						cChar* stack_trace_string = to_cstring(stack_trace);
						out.push( stack_trace_string ); out.push('\n');
					}
				}
				return out.to_string();
			}
		}

		void print_exception(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
			Qk_ERR(parse_exception_message(message, error) );
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
			if ( !triggerUncaughtException(Cast(error)) ) {
				print_exception(message, error);
				qk::exit(ERR_UNCAUGHT_EXCEPTION);
			}
		}

		void unhandled_rejection(PromiseRejectMessage& message) {
			v8::Local<v8::Promise> promise = message.GetPromise();
			v8::Local<v8::Value> reason = message.GetValue();
			if (reason.IsEmpty())
				reason = v8::Undefined(_isolate);
			if ( !triggerUnhandledRejection(Cast(reason), Cast(promise)) ) {
				v8::HandleScope scope(_isolate);
				v8::Local<v8::Message> message = v8::Exception::CreateMessage(_isolate, reason);
				print_exception(message, reason);
				qk::exit(ERR_UNHANDLED_REJECTION);
			}
		}

	};

	class V8JSClass: public JSClass {
	public:
		V8JSClass(Worker* worker, cString& name,
							FunctionCallback constructor, V8JSClass* base,
							v8::Local<v8::Function> baseFunc = v8::Local<v8::Function>())
			: JSClass(), _base(base)
		{ //
			v8::FunctionCallback cb = reinterpret_cast<v8::FunctionCallback>(constructor);
			v8::Local<v8::FunctionTemplate> ft = v8::FunctionTemplate::New(ISOLATE(worker), cb);
			v8::Local<v8::String> className = Back<v8::String>(worker->New(name, true));

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

	Local<JSFunction> JSClass::getFunction() {
		if (_func.IsEmpty()) {
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
				ok = proto->SetPrototype(baseProto);
				Qk_ASSERT(ok);
			}
			Local<JSFunction> func = Cast<JSFunction>(f);
			_func.Reset(_worker, func);
		}
		return _func.toLocal();
	}

	Local<JSValue> IMPL::binding_node_module(cString& name) {
		if (node::node_api) {
			void* r = node::node_api->binding_node_module(*name);
			auto _ = reinterpret_cast<Local<JSValue>*>(&r);
			return *_;
		}
		_host->throwError(_host->NewError("Cannot find module %s", *name));
		return Local<JSValue>();
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

	CallbackScope::CallbackScope(Worker* worker) {
		auto impl = WORKER(worker);
		if (impl->is_node()) {
			val_ = node::node_api->callback_scope(node::quark_api->env());
		} else {
			val_ = nullptr;
		}
	}

	CallbackScope::~CallbackScope() {
		if (val_) {
			node::node_api->delete_callback_scope(reinterpret_cast<node::NodeCallbackScope*>(val_));
		}
		val_ = nullptr;
	}
		
	// ------------------------ JSValue ------------------------

	bool JSValue::IsUndefined() const {
		return reinterpret_cast<const v8::Value*>(this)->IsUndefined();
	}
	bool JSValue::IsNull() const {
		return reinterpret_cast<const v8::Value*>(this)->IsNull();
	}
	bool JSValue::IsString() const {
		return reinterpret_cast<const v8::Value*>(this)->IsString();
	}
	bool JSValue::IsBoolean() const {
		return reinterpret_cast<const v8::Value*>(this)->IsBoolean();
	}
	bool JSValue::IsObject() const {
		return reinterpret_cast<const v8::Value*>(this)->IsObject();
	}
	bool JSValue::IsArray() const {
		return reinterpret_cast<const v8::Value*>(this)->IsArray();
	}
	bool JSValue::IsDate() const {
		return reinterpret_cast<const v8::Value*>(this)->IsDate();
	}
	bool JSValue::IsNumber() const {
		return reinterpret_cast<const v8::Value*>(this)->IsNumber();
	}
	bool JSValue::IsUint32() const {
		return reinterpret_cast<const v8::Value*>(this)->IsUint32();
	}
	bool JSValue::IsInt32() const {
		return reinterpret_cast<const v8::Value*>(this)->IsInt32();
	}
	// bool JSValue::IsRegExp() const {
	//   return reinterpret_cast<const v8::Value*>(this)->IsRegExp();
	// }
	bool JSValue::IsFunction() const {
		return reinterpret_cast<const v8::Value*>(this)->IsFunction();
	}
	bool JSValue::IsArrayBuffer() const {
		return reinterpret_cast<const v8::Value*>(this)->IsArrayBuffer();
	}
	bool JSValue::IsTypedArray() const {
		return reinterpret_cast<const v8::Value*>(this)->IsArrayWeakBuffer();
	}
	bool JSValue::IsUint8Array() const {
		return reinterpret_cast<const v8::Value*>(this)->IsUint8Array();
	}
	bool JSValue::Equals(Local<JSValue> val) const {
		return reinterpret_cast<const v8::Value*>(this)->Equals(Back(val));
	}
	bool JSValue::StrictEquals(Local<JSValue> val) const {
		return reinterpret_cast<const v8::Value*>(this)->StrictEquals(Back(val));
	}
	bool JSValue::IsUndefined(Worker* worker) const { return IsUndefined(); }
	bool JSValue::IsNull(Worker* worker) const { return IsNull(); }
	bool JSValue::IsString(Worker* worker) const { return IsString(); }
	bool JSValue::IsBoolean(Worker* worker) const { return IsBoolean(); }
	bool JSValue::IsObject(Worker* worker) const { return IsObject(); }
	bool JSValue::IsArray(Worker* worker) const { return IsArray(); }
	bool JSValue::IsDate(Worker* worker) const { return IsDate(); }
	bool JSValue::IsNumber(Worker* worker) const { return IsNumber(); }
	bool JSValue::IsUint32(Worker* worker) const { return IsUint32(); }
	bool JSValue::IsInt32(Worker* worker) const { return IsInt32(); }
	bool JSValue::IsFunction(Worker* worker) const { return IsFunction(); }
	bool JSValue::IsArrayBuffer(Worker* worker) const { return IsArrayBuffer(); }
	bool JSValue::IsTypedArray(Worker* worker) const { return IsTypedArray(); }
	bool JSValue::IsUint8Array(Worker* worker) const { return IsUint8Array(); }
	bool JSValue::Equals(Worker* worker, Local<JSValue> val) const { return Equals(val); }
	bool JSValue::StrictEquals(Worker* worker, Local<JSValue> val) const { return StrictEquals(val); }

	Local<JSString> JSValue::ToString(Worker* worker) const {
		return Cast<JSString>(reinterpret_cast<const v8::Value*>(this)->ToString());
	}

	Local<JSNumber> JSValue::ToNumber(Worker* worker) const {
		return Cast<JSNumber>(reinterpret_cast<const v8::Value*>(this)->ToNumber());
	}

	Local<JSInt32> JSValue::ToInt32(Worker* worker) const {
		return Cast<JSInt32>(reinterpret_cast<const v8::Value*>(this)->ToInt32());
	}

	Local<JSUint32> JSValue::ToUint32(Worker* worker) const {
		return Cast<JSUint32>(reinterpret_cast<const v8::Value*>(this)->ToUint32());
	}

	Local<JSObject> JSValue::ToObject(Worker* worker) const {
		return Cast<JSObject>(reinterpret_cast<const v8::Value*>(this)->ToObject());
	}

	Local<JSBoolean> JSValue::ToBoolean(Worker* worker) const {
		return Cast<JSBoolean>(reinterpret_cast<const v8::Value*>(this)->ToBoolean());
	}

	#if USE_JSC
	String JSValue::ToStringValue(Worker* worker, bool ascii) const {
		JSC_ENV(worker);
		v8::JSCStringPtr s = JSValueToStringCopy(ctx, reinterpret_cast<JSValueRef>(this),
																						OK(unknown));
		size_t len = JSStringGetMaximumUTF8CStringSize(*s);
		Buffer bf((uint)len);
		len = JSStringGetUTF8CString(*s, *bf, len);
		if (len > 1) {
			bf.realloc(uint(len - 1));
			return bf.collapse_string();
		} else {
			return String();
		}
	}
	#else
	String JSValue::ToStringValue(Worker* worker, bool ascii) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString();
		if ( str.IsEmpty() ) return unknown;
		if ( ascii ) {
			String rev;
			Buffer buffer(128);
			int index = 0, count;
			do {
				count = str->WriteOneByte((uint8_t*)*buffer, index, 128);
				rev.push(*buffer, count);
				index += count;
			} while(count);
			return rev;
		} else {
			StringBuilder rev;
			Array<uint16> buffer(128);
			int index = 0; int count;
			while( (count = str->Write(*buffer, index, 128)) ) {
				Buffer buff = Coder::encoding(Encoding::utf8, *buffer, count);
				rev.push(move(buff));
				index += count;
			}
			return rev.to_string();
		}
	}
	#endif

	#if USE_JSC
	String2 JSValue::ToString2Value(Worker* worker) const {
		JSC_ENV(worker);
		v8::JSCStringPtr s = JSValueToStringCopy(ctx, reinterpret_cast<JSValueRef>(this),
																						OK(unknown_ucs2));
		size_t len = JSStringGetLength(*s);
		const uint16* ptr = JSStringGetCharactersPtr(*s);
		WeakArray<uint16_t> bf(ptr, uint(len));
		return bf.copy().collapse_string();
	}
	#else
	String2 JSValue::ToString2Value(Worker* worker) const {
		v8::Local<v8::String> str = ((v8::Value*)this)->ToString();
		if ( str.IsEmpty() ) return unknown_ucs2;
		String2 rev;
		uint16_t buffer[512];
		int index = 0, count;
		do {
			count = str->Write(buffer, index, 512);
			rev.push(buffer, count);
			index += count;
		} while(count);
		return rev;
	}
	#endif

	double JSValue::ToNumberValue(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToNumber()->Value();
	}

	int JSValue::ToInt32Value(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToInt32()->Value();
	}

	uint32_t JSValue::ToUint32Value(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToUint32()->Value();
	}

	bool JSValue::ToBooleanValue(Worker* worker) const {
		return reinterpret_cast<const v8::Value*>(this)->ToBoolean()->Value();
	}

	Maybe<double> JSValue::ToNumberMaybe(Worker* worker) const {
		v8::Local<v8::Number> r;
		if (reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker)).ToLocal(&r)) {
			return Maybe<double>(r->Value());
		} else {
			return Maybe<double>();
		}
	}
	Maybe<int> JSValue::ToInt32Maybe(Worker* worker) const {
		v8::Local<v8::Int32> r;
		if (reinterpret_cast<const v8::Value*>(this)->ToInt32(CONTEXT(worker)).ToLocal(&r)) {
			return Maybe<int>(r->Value());
		} else {
			return Maybe<int>();
		}
	}
	Maybe<uint> JSValue::ToUint32Maybe(Worker* worker) const {
		v8::Local<v8::Uint32> r;
		if (reinterpret_cast<const v8::Value*>(this)->ToUint32(CONTEXT(worker)).ToLocal(&r)) {
			return Maybe<uint>(r->Value());
		} else {
			return Maybe<uint>();
		}
	}

	bool JSValue::InstanceOf(Worker* worker, Local<JSObject> value) {
		return reinterpret_cast<v8::Value*>(this)->
			InstanceOf(CONTEXT(worker), Back<v8::Object>(value)).FromMaybe(false);
	}

	Local<JSValue> JSObject::Get(Worker* worker, Local<JSValue> key) {
		return Cast(reinterpret_cast<v8::Object*>(this)->
								Get(CONTEXT(worker), Back(key)).FromMaybe(v8::Local<v8::Value>()));
	}

	Local<JSValue> JSObject::Get(Worker* worker, uint32_t index) {
		return Cast(reinterpret_cast<v8::Object*>(this)->
								Get(CONTEXT(worker), index).FromMaybe(v8::Local<v8::Value>()));
	}

	bool JSObject::Set(Worker* worker, Local<JSValue> key, Local<JSValue> val) {
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), Back(key), Back(val)).FromMaybe(false);
	}

	bool JSObject::Set(Worker* worker, uint32_t index, Local<JSValue> val) {
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), index, Back(val)).FromMaybe(false);
	}

	bool JSObject::Has(Worker* worker, Local<JSValue> key) {
		return reinterpret_cast<v8::Object*>(this)->
			Has(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::Has(Worker* worker, uint32_t index) {
		return reinterpret_cast<v8::Object*>(this)->
			Has(CONTEXT(worker), index).FromMaybe(false);
	}

	bool JSObject::JSObject::Delete(Worker* worker, Local<JSValue> key) {
		return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::Delete(Worker* worker, uint32_t index) {
		return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), index).FromMaybe(false);
	}

	Local<JSArray> JSObject::GetPropertyNames(Worker* worker) {
		return Cast<JSArray>(reinterpret_cast<v8::Object*>(this)->
												GetPropertyNames(CONTEXT(worker)).FromMaybe(v8::Local<v8::Array>()));
	}

	Local<JSFunction> JSObject::GetConstructor(Worker* worker) {
		auto rv = reinterpret_cast<v8::Object*>(this)->
			Get(CONTEXT(worker), Back(worker->strs()->constructor()));
		return Cast<JSFunction>(rv.FromMaybe(v8::Local<v8::Value>()));
	}

	bool JSObject::SetMethod(Worker* worker, cString& name, FunctionCallback func) {
		v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(ISOLATE(worker), func2);
		v8::Local<v8::Function> fn = t->GetFunction();
		v8::Local<v8::String> fn_name = Back<v8::String>(worker->New(name, 1));
		fn->SetName(fn_name);
		return reinterpret_cast<v8::Object*>(this)->
			Set(CONTEXT(worker), fn_name, fn).FromMaybe(false);
	}

	bool JSObject::SetAccessor(Worker* worker, cString& name,
														AccessorGetterCallback get, AccessorSetterCallback set) {
		v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		v8::Local<v8::String> fn_name = Back<v8::String>(worker->New(name, 1));
		return reinterpret_cast<v8::Object*>(this)->SetAccessor(fn_name, get2, set2);
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

	bool JSObject::set__Proto__(Worker* worker, Local<JSObject> __proto__) {
		return reinterpret_cast<v8::Object*>(this)->
			SetPrototype(CONTEXT(worker), Back(__proto__)).FromMaybe(false);
	}

	int JSString::length(Worker* worker) const {
		return reinterpret_cast<const v8::String*>(this)->Length();
	}
	String JSString::Value(Worker* worker, bool ascii) const {
		return ToStringValue(worker, ascii);
	}
	String2 JSString::Ucs2Value(Worker* worker) const {
		return ToString2Value(worker);
	}
	Local<JSString> JSString::Empty(Worker* worker) {
		return Cast<JSString>(v8::String::Empty(ISOLATE(worker)));
	}
	int JSArray::Length(Worker* worker) const {
		return reinterpret_cast<const v8::Array*>(this)->Length();
	}
	double JSDate::ValueOf(Worker* worker) const {
		return reinterpret_cast<const v8::Date*>(this)->ValueOf();
	}
	double JSNumber::Value(Worker* worker) const {
		return reinterpret_cast<const v8::Number*>(this)->Value();
	}
	int JSInt32::Value(Worker* worker) const {
		return reinterpret_cast<const v8::Int32*>(this)->Value();
	}
	int64_t JSInteger::Value(Worker* worker) const {
		return reinterpret_cast<const v8::Integer*>(this)->Value();
	}
	uint32_t JSUint32::Value(Worker* worker) const {
		return reinterpret_cast<const v8::Uint32*>(this)->Value();
	}
	bool JSBoolean::Value(Worker* worker) const {
		return reinterpret_cast<const v8::Boolean*>(this)->Value();
	}

	Local<JSValue> JSFunction::Call(Worker* worker, int argc,
																	Local<JSValue> argv[], Local<JSValue> recv) {
		if ( recv.IsEmpty() ) {
			recv = worker->NewUndefined();
		}
		v8::Local<v8::Function> fn2 = Back(Local<JSValue>(this)).As<v8::Function>();
		v8::MaybeLocal<v8::Value> r = fn2->Call(CONTEXT(worker), Back(recv), argc,
																						reinterpret_cast<v8::Local<v8::Value>*>(argv));
		return Cast(r.FromMaybe(v8::Local<v8::Value>()));
	}

	Local<JSValue> JSFunction::Call(Worker* worker, Local<JSValue> recv) {
		return Call(worker, 0, nullptr, recv);
	}

	Local<JSObject> JSFunction::NewInstance(Worker* worker, int argc, Local<JSValue> argv[]) {
		v8::Local<v8::Function> fn2 = Back(Local<JSValue>(this)).As<v8::Function>();
		v8::MaybeLocal<v8::Object> r = fn2->NewInstance(CONTEXT(worker), argc,
																										reinterpret_cast<v8::Local<v8::Value>*>(argv));
		return Cast<JSObject>(r.FromMaybe(v8::Local<v8::Object>()));
	}

	Local<JSObject> JSFunction::getPrototype(Worker* worker) {
		auto str = Back(worker->strs()->prototype());
		auto proto = f->Get(CONTEXT(worker), str);
		return Cast<JSObject>(proto.FromMaybe(v8::Local<v8::Object>()));
	}

	int JSArrayBuffer::ByteLength(Worker* worker) const {
		return (uint32_t)reinterpret_cast<const v8::ArrayBuffer*>(this)->ByteLength();
	}
	Char* JSArrayBuffer::Data(Worker* worker) {
		return (Char*)reinterpret_cast<v8::ArrayBuffer*>(this)->GetContents().Data();
	}
	Local<JSArrayBuffer> JSTypedArray::Buffer(Worker* worker) {
		auto ab = reinterpret_cast<v8::ArrayWeakBuffer*>(this);
		v8::Local<v8::ArrayBuffer> ab2 = ab->Buffer();
		return Cast<JSArrayBuffer>(ab2);
	}

	int JSTypedArray::ByteLength(Worker* worker) {
		auto ab = reinterpret_cast<v8::ArrayWeakBuffer*>(this);
		return (uint32_t)ab->ByteLength();
	}

	int JSTypedArray::ByteOffset(Worker* worker) {
		auto ab = reinterpret_cast<v8::ArrayWeakBuffer*>(this);
		return (uint32_t)ab->ByteOffset();
	}

	MaybeLocal<JSSet> JSSet::Add(Worker* worker, Local<JSValue> key) {
		auto set = reinterpret_cast<v8::Set*>(this);
		v8::Local<v8::Set> out;
		if ( set->Add(CONTEXT(worker), Back(key)).ToLocal(&out) ) {
			return MaybeLocal<JSSet>(Cast<JSSet>(out));
		}
		return MaybeLocal<JSSet>();
	}
		
	Maybe<bool> JSSet::Has(Worker* worker, Local<JSValue> key) {
		auto set = reinterpret_cast<v8::Set*>(this);
		v8::Local<v8::Set> out;
		bool v;
		if (set->Has(CONTEXT(worker), Back(key)).To(&v)) {
			return Maybe<bool>(v);
		}
		return Maybe<bool>();
	}
		
	Maybe<bool> JSSet::Delete(Worker* worker, Local<JSValue> key) {
		auto set = reinterpret_cast<v8::Set*>(this);
		v8::Local<v8::Set> out;
		bool v;
		if (set->Delete(CONTEXT(worker), Back(key)).To(&v)) {
			return Maybe<bool>(v);
		}
		return Maybe<bool>();
	}
		
	bool JSClass::HasInstance(Worker* worker, Local<JSValue> val) {
		return reinterpret_cast<V8JSClass*>(this)->Template()->HasInstance(Back(val));
	}

	bool JSClass::SetMemberMethod(Worker* worker, cString& name, FunctionCallback func) {
		v8::Local<v8::FunctionTemplate> temp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		v8::Local<Signature> sign = Signature::New(ISOLATE(worker), temp);
		v8::Local<v8::FunctionTemplate> t =
			FunctionTemplate::New(ISOLATE(worker), func2, v8::Local<v8::Value>(), sign);
		v8::Local<v8::String> fn_name = Back<v8::String>(worker->New(name, 1));
		t->SetClassName(fn_name);
		temp->PrototypeTemplate()->Set(fn_name, t);
		return true;
	}

	bool JSClass::SetMemberAccessor(Worker* worker, cString& name,
																	AccessorGetterCallback get, AccessorSetterCallback set) {
		v8::Local<v8::FunctionTemplate> temp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		v8::Local<AccessorSignature> s = AccessorSignature::New(ISOLATE(worker), temp);
		v8::Local<v8::String> fn_name = Back<v8::String>(worker->New(name, 1));
		temp->PrototypeTemplate()->SetAccessor(fn_name, get2, set2,
																					v8::Local<v8::Value>(), v8::DEFAULT, v8::None, s);
		return true;
	}

	bool JSClass::SetMemberIndexedAccessor(Worker* worker,
																				IndexedPropertyGetterCallback get,
																				IndexedPropertySetterCallback set) {
		v8::IndexedPropertyGetterCallback get2 = reinterpret_cast<v8::IndexedPropertyGetterCallback>(get);
		v8::IndexedPropertySetterCallback set2 = reinterpret_cast<v8::IndexedPropertySetterCallback>(set);
		v8::IndexedPropertyHandlerConfiguration cfg(get2, set2);
		reinterpret_cast<V8JSClass*>(this)->Template()->PrototypeTemplate()->SetHandler(cfg);
		return true;
	}

	template<>
	bool JSClass::setMemberProperty<Local<JSValue>>(cString& name, Local<JSValue> value) {
		reinterpret_cast<V8JSClass*>(this)->Template()->
						PrototypeTemplate()->Set(Back<v8::String>(worker->newInstance(name, 1)), Back(value));
		return true;
	}

	template<>
	bool JSClass::setStaticProperty<Local<JSValue>>(cString& name, Local<JSValue> value) {
		reinterpret_cast<V8JSClass*>(this)->Template()->
						Set(Back<v8::String>(_worker->newInstance(name, 1)), Back(value));
		return true;
	}

	template<>
	Local<JSValue> MaybeLocal<JSValue>::toLocalChecked() {
		return Cast(reinterpret_cast<v8::MaybeLocal<v8::Value>*>(this)->ToLocalChecked());
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

	template<> void ReturnValue::set<JSValue>(const Local<JSValue> value) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(Back(value));
	}

	int FunctionCallbackInfo::Length() const {
		return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->Length();
	}

	Local<JSValue> FunctionCallbackInfo::operator[](int i) const {
		return Cast(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->operator[](i));
	}

	Local<JSObject> FunctionCallbackInfo::This() const {
		return Cast<JSObject>(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->This());
	}

	bool FunctionCallbackInfo::IsConstructCall() const {
		return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->IsConstructCall();
	}

	ReturnValue FunctionCallbackInfo::GetReturnValue() const {
		auto info = reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this);
		v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		auto _ = reinterpret_cast<ReturnValue*>(&rv);
		return *_;
	}

	Local<JSObject> PropertyCallbackInfo::This() const {
		return Cast<JSObject>(reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this)->This());
	}

	ReturnValue PropertyCallbackInfo::GetReturnValue() const {
		auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
		v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		auto _ = reinterpret_cast<ReturnValue*>(&rv);
		return *_;
	}

	Local<JSObject> PropertySetCallbackInfo::This() const {
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
		v8::TryCatch try_;
	};

	TryCatch::TryCatch() {
		val_ = new TryCatchWrap();
	}

	TryCatch::~TryCatch() {
		delete reinterpret_cast<TryCatchWrap*>(val_); val_ = nullptr;
	}

	bool TryCatch::HasCaught() const {
		return reinterpret_cast<TryCatchWrap*>(val_)->try_.HasCaught();
	}

	Worker* WeakCallbackInfo::worker() const {
		auto info = reinterpret_cast<const v8::WeakCallbackInfo<Object>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

	void* WeakCallbackInfo::getParameter() const {
		return reinterpret_cast<const v8::WeakCallbackInfo<void>*>(this)->GetParameter();
	}

	bool Persistent::isWeak(PersistentBase<JSObject>& handle) {
		Qk_ASSERT( !handle.IsEmpty() );
		auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(&handle);
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

	template <> void Persistent<JSValue>::reset() {
		reinterpret_cast<v8::Persistent<v8::Value>*>(this)->Reset();
	}

	template <> template <>
	void Persistent<JSValue>::reset(Worker* worker, const Local<JSValue>& other) {
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

	template<>
	Local<JSValue> Persistent<JSValue>::toLocal() const {
		// return *reinterpret_cast<Local<JSValue>*>(const_cast<Persistent*>(this));
		if (isEmpty())
			return Local<JSValue>();
		v8::Local<v8::Value> r =
			reinterpret_cast<const v8::PersistentBase<v8::Value>*>(this)->Get(ISOLATE(_worker));
		return Cast(r);
	}

	Local<JSNumber> Worker::New(float data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	Local<JSNumber> Worker::New(double data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	Local<JSBoolean> Worker::New(bool data) {
		return Cast<JSBoolean>(v8::Boolean::New(ISOLATE(this), data));
	}

	Local<JSInt32> Worker::New(Char data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	Local<JSUint32> Worker::New(uint8_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	Local<JSInt32> Worker::New(int16_t data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	Local<JSUint32> Worker::New(uint16_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	Local<JSInt32> Worker::New(int data) {
		return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	Local<JSUint32> Worker::New(uint32_t data) {
		return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	Local<JSNumber> Worker::New(int64_t data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	Local<JSNumber> Worker::New(uint64_t data) {
		return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	Local<JSString> Worker::New(cString& data, bool oneByte) {
		if ( oneByte ) {
			return Cast<JSString>(v8::String::NewExternal(ISOLATE(this),
																										new V8ExternalOneByteStringResource(data)));
		} else {
			return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																										v8::String::kNormalString, data.length()));
		}
	}

	Local<JSString> Worker::New(cString2& data) {
		return Cast<JSString>(v8::String::NewExternal(ISOLATE(this), new V8ExternalStringResource(data)));
	}

	Local<JSArray> Worker::New(const Array<String>& data) {
		auto worker = WORKER();
		v8::Local<v8::Array> rev = v8::Array::New(ISOLATE(worker));
		{ v8::HandleScope scope(ISOLATE(worker));
			for (int i = 0, e = data.length(); i < e; i++) {
				v8::Local<v8::Value> value = Back(New(data[i]));
				rev->Set(i, value);
			}
		}
		return Cast<JSArray>(rev);
	}

	Local<JSArray> Worker::New(Array<FileStat>&& ls) {
		v8::Local<v8::Array> rev = v8::Array::New(ISOLATE(this));
		{ v8::HandleScope scope(ISOLATE(this));
			for (int i = 0, e = ls.length(); i < e; i++) {
				v8::Local<v8::Value> value = Back( New(move(ls[i])) );
				rev->Set(CONTEXT(this), i, value).IsJust();
			}
		}
		return Cast<JSArray>(rev);
	}

	Local<JSObject> Worker::New(const Dict<String, String>& data) {
		v8::Local<v8::Object> rev = v8::Object::New(ISOLATE(this));
		{ v8::HandleScope scope(ISOLATE(this));
			for (auto& i : data) {
				v8::Local<v8::Value> key = Back(New(i.key(), 1));
				v8::Local<v8::Value> value = Back(New(i.value()));
				rev->Set(key, value);
			}
		}
		return Cast<JSObject>(rev);
	}

	Local<JSObject> NewDirent(Worker* w, const Dirent& dir) {
		v8::Local<v8::Object> rev = v8::Object::New(ISOLATE(w));
		rev->Set(CONTEXT(w), Back(w->strs()->name()), Back(w->New(dir.name))).IsJust();
		rev->Set(CONTEXT(w), Back(w->strs()->pathname()), Back(w->New(dir.pathname))).IsJust();
		rev->Set(CONTEXT(w), Back(w->strs()->type()), Back(w->New(dir.type))).IsJust();
		return Cast<JSObject>(rev);
	}

	Local<JSObject> Worker::New(const Dirent& dir) {
		return NewDirent(this, dir);
	}

	Local<JSArray> Worker::New(Array<Dirent>&& ls) {
		v8::Local<v8::Array> rev = v8::Array::New(ISOLATE(this));
		for (int i = 0, e = ls.length(); i < e; i++) {
			v8::Local<v8::Value> value = Back(NewDirent(this, ls[i]));
			rev->Set(CONTEXT(this), i, value).IsJust();
		}
		return Cast<JSArray>(rev);
	}
	Local<JSArrayBuffer> Worker::NewArrayBuffer(Char* use_buff, uint32_t len) {
		return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), use_buff, len));
	}
	Local<JSArrayBuffer> Worker::NewArrayBuffer(uint32_t len) {
		return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), len));
	}
	Local<JSUint8Array> Worker::NewUint8Array(Local<JSArrayBuffer> ab, uint32_t offset, uint32_t size) {
		auto ab2 = Back<v8::ArrayBuffer>(ab);
		offset = Qk_MIN((uint)ab2->ByteLength(), offset);
		if (size + offset > ab2->ByteLength()) {
			size = (uint)ab2->ByteLength() - offset;
		}
		return Cast<JSUint8Array>(v8::Uint8Array::New(ab2, offset, size));
	}
	Local<JSObject> Worker::NewObject() {
		return Cast<JSObject>(v8::Object::New(ISOLATE(this)));
	}

	Local<JSArray> Worker::NewArray(uint32_t len) {
		return Cast<JSArray>(v8::Array::New(ISOLATE(this), len));
	}

	Local<JSValue> Worker::NewNull() {
		return Cast(v8::Null(ISOLATE(this)));
	}

	Local<JSValue> Worker::NewUndefined() {
		return Cast(v8::Undefined(ISOLATE(this)));
	}

	Local<JSSet> Worker::NewSet() {
		return Cast<JSSet>(v8::Set::New(ISOLATE(this)));
	}

	Local<JSString> Worker::NewString(cBuffer& data) {
		return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this),
																									*data, v8::String::kNormalString,
																									data.length()));
	}

	Local<JSString> Worker::NewAscii(cChar* str) {
		return Cast<JSString>(v8::String::NewFromOneByte(ISOLATE(this),
																										(const uint8_t  *)str, v8::String::kNormalString));
	}
		
		Local<JSString> NewAscii(cChar* str);

	Local<JSObject> Worker::NewRangeError(cChar* errmsg, ...) {
		Qk_STRING_FORMAT(errmsg, str);
		return Cast<JSObject>(v8::Exception::RangeError(Back(New(str))->ToString()));
	}

	Local<JSObject> Worker::NewReferenceError(cChar* errmsg, ...) {
		Qk_STRING_FORMAT(errmsg, str);
		return Cast<JSObject>(v8::Exception::ReferenceError(Back(New(str))->ToString()));
	}

	Local<JSObject> Worker::NewSyntaxError(cChar* errmsg, ...) {
		Qk_STRING_FORMAT(errmsg, str);
		return Cast<JSObject>(v8::Exception::SyntaxError( Back(New(str))->ToString() ));
	}

	Local<JSObject> Worker::NewTypeError(cChar* errmsg, ...) {
		Qk_STRING_FORMAT(errmsg, str);
		return Cast<JSObject>(v8::Exception::TypeError( Back(New(str))->ToString() ));
	}

	Local<JSObject> Worker::New(cError& err) {
		v8::Local<v8::Object> e =
		v8::Exception::Error( Back<v8::String>(New(err.message())) ).As<v8::Object>();
		e->Set( Back(strs()->code()), Back(New(err.code())) );
		return Cast<JSObject>(e);
	}

	Local<JSObject> Worker::NewError(Local<JSObject> value) {
		v8::Local<v8::Object> v = Back<v8::Object>(value);
		v8::Local<v8::Value> msg = v->Get( Back(strs()->message()) );
		v8::Local<v8::Object> e = v8::Exception::Error(msg->ToString()).As<v8::Object>();
		v8::Local<v8::Array> names = v->GetPropertyNames();
		for (uint32_t i = 0, j = 0; i < names->Length(); i++) {
			v8::Local<v8::Value> key = names->Get(i);
			e->Set(key, v->Get(key));
		}
		return Cast<JSObject>(e);
	}

	Local<JSUint8Array> Worker::New(Buffer&& buff) {
		size_t offset = 0;
		size_t len = buff.length();
		v8::Local<v8::ArrayBuffer> ab;

		if (buff.length()) {
			size_t len = buff.length();
			Char* data = buff.collapse();
			ab = v8::ArrayBuffer::New(ISOLATE(this), data, len);
		} else {
			ab = v8::ArrayBuffer::New(ISOLATE(this), 0);
		}
		return Cast<JSUint8Array>(v8::Uint8Array::New(ab, offset, len));
	}

	void Worker::throwError(Local<JSValue> exception) {
		ISOLATE(this)->ThrowException(Back(exception));
	}

	Local<JSClass> Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach_callback, Local<JSClass> base) {
		auto cls = new V8JSClass(this, name, constructor, static_cast<V8JSClass*>(*base));
		_classsinfo->add(id, cls, attach_callback);
		return Local<JSClass>(cls);
	}

	Local<JSClass> Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach_callback, uint64_t base) {
		return newClass(name, id, constructor, attach_callback, _classsinfo->get(base));
	}

	Local<JSClass> Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach_callback, Local<JSFunction> base) {
		auto cls = new V8JSClass(this, name, constructor, nullptr, Back<v8::Function>(base));
		_classsinfo->add(id, cls, attach_callback);
		return Local<JSClass>(cls);
	}

	void Worker::reportException(TryCatch* try_catch) {
		TryCatchWrap* wrap = *reinterpret_cast<TryCatchWrap**>(try_catch);
		WORKER(this)->print_exception(wrap->try_.Message(), wrap->try_.Exception());
	}

	Local<JSValue> Worker::runScript(Local<JSString> source,
																		Local<JSString> name, Local<JSObject> sandbox) {
		v8::MaybeLocal<v8::Value> r = WORKER(this)->runScript(Back<v8::String>(source),
																													Back<v8::String>(name),
																													Back<v8::Object>(sandbox));
		return Cast(r.FromMaybe(v8::Local<v8::Value>()));
	}

	Local<JSValue> Worker::runScript(cString& source,
																		cString& name, Local<JSObject> sandbox) {
		return runScript(New(source), New(name), sandbox);
	}

	Local<JSValue> Worker::runNativeScript(
		cBuffer& source, cString& name, Local<JSObject> exports) {
		v8::EscapableHandleScope scope(ISOLATE(this));
		if (exports.IsEmpty()) {
			exports = NewObject();
		}
		Local<JSValue> r = WORKER(this)->runNativeScript(source, name, exports);
		return Cast(scope.Escape(Back(r)));
	}

	void Worker::garbageCollection() {
		ISOLATE(this)->LowMemoryNotification();
	}

	int platformStart(int argc, Char** argv) {
		v8::Platform* platform = v8::platform::CreateDefaultPlatform();
		v8::V8::InitializePlatform(platform);
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

			{
				HandleScope scope(*worker);
				auto _pkg = worker->bindingModule("_pkg");
				Qk_ASSERT(!_pkg.IsEmpty(), "Can't start worker");
				Local<JSValue> r = _pkg.cast()->
					getProperty(*worker, "Module").cast()->
					getProperty(*worker, "runMain").cast<JSFunction>()->call(*worker);
				if (r.isEmpty()) {
					Qk_ERR("ERROR: Can't call runMain()");
					return ERR_RUN_MAIN_EXCEPTION;
				}
			}

			auto loop = RunLoop::first();
			do {
				loop->run();
				/* IOS forces the process to terminate, but it does not quit immediately.
				This may cause a process to run in the background for a long time, so force break here */
				if (is_exited())
					break;

				if (loop->is_alive())
					continue;

				rc = triggerBeforeExit(*worker, rc);

				// Emit `beforeExit` if the loop became alive either after emitting
				// event, or after running some callbacks.
			} while (loop->is_alive());

			if (!is_exited())
				rc = triggerExit(*worker, rc);
		}

		v8::V8::ShutdownPlatform();
		v8::V8::Dispose();
		delete platform;

		return rc;
	}

} }