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

#include <v8.h>
#include <libplatform/libplatform.h>
#include "nutils/util.h"
#include "nutils/http.h"
#include "nutils/string-builder.h"
#include "ngui/view.h"
#include "ngui/errno.h"
#include "js-1.h"
#include "wrap.h"
#include "native-inl-js.h"
#include <uv.h>
#include "value.h"
#include "depe/node/src/ngui.h"

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
	struct JSCStringTraits: public ngui::NonObjectTraits {
		inline static void Release(JSStringRef str) {
			if ( str ) JSStringRelease(str);
		}
	};
	typedef ngui::Handle<OpaqueJSString, JSCStringTraits> JSCStringPtr;
}
#endif

JS_BEGIN

using namespace native_js;
using namespace v8;

#ifndef ISOLATE_INL_WORKER_DATA_INDEX
# define ISOLATE_INL_WORKER_DATA_INDEX (0)
#endif

#define ISOLATE(...) WorkerIMPL::current( __VA_ARGS__ )->isolate_
#define CONTEXT(...) WorkerIMPL::current( __VA_ARGS__ )->context_
#define WORKER(...) WorkerIMPL::current( __VA_ARGS__ )
#define CURRENT Worker::worker()

static v8::Platform* platform = nullptr;
static String unknown("[Unknown]");
static Ucs2String unknown_ucs2(String("[Unknown]"));

typedef const v8::FunctionCallbackInfo<Value>& V8FunctionCall;
typedef const v8::PropertyCallbackInfo<Value>& V8PropertyCall;
typedef const v8::PropertyCallbackInfo<void>&  V8PropertySetCall;

template<class T = JSValue, class S>
XX_INLINE Local<T> Cast(v8::Local<S> o) {
	auto _ = reinterpret_cast<Local<T>*>(&o);
	return *_;
}

template<class T = v8::Value, class S>
XX_INLINE v8::Local<T> Back(Local<S> o) {
	auto _ = reinterpret_cast<v8::Local<T>*>(&o);
	return *_;
}

/**
 * @class V8ExternalOneByteStringResource
 */
class V8ExternalOneByteStringResource: public v8::String::ExternalOneByteStringResource {
	String _str;
 public:
	V8ExternalOneByteStringResource(cString& value): _str(value) { }
	virtual cchar* data() const { return *_str; }
	virtual size_t length() const { return _str.length(); }
};

/**
 * @class V8ExternalStringResource
 */
class V8ExternalStringResource: public v8::String::ExternalStringResource {
	Ucs2String _str;
 public:
	V8ExternalStringResource(const Ucs2String& value): _str(value) { }
	virtual const uint16* data() const { return *_str; }
	virtual size_t length() const { return _str.length(); }
};

/**
 * @class WorkerIMPL
 */
class WorkerIMPL: public IMPL {
 public:
 	template<class T>
	struct Wrap {
		T value;
		inline Wrap(Isolate* isolate): value(isolate) {}
	};
	Isolate*  isolate_;
	Locker*   locker_;
	Wrap<v8::HandleScope>* handle_scope_;
	Wrap<v8::SealHandleScope>* handle_scope_seal_;
	v8::Local<v8::Context> context_;

	WorkerIMPL()
		: locker_(nullptr), handle_scope_(nullptr), handle_scope_seal_(nullptr) 
	{
		Isolate::CreateParams params;
		params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		m_is_node = 0;
		isolate_ = Isolate::New(params);
		locker_ = new Locker(isolate_);
		isolate_->Enter();
		handle_scope_ = new Wrap<v8::HandleScope>(isolate_);
		context_ = v8::Context::New(isolate_);
		context_->Enter();
		isolate_->SetFatalErrorHandler(OnFatalError);
		isolate_->AddMessageListener(MessageCallback);
		isolate_->SetPromiseRejectCallback(PromiseRejectCallback);
	}

	// use node
	WorkerIMPL(void* v8_isolate, void* v8_context)
		: locker_(nullptr), handle_scope_(nullptr), handle_scope_seal_(nullptr) 
	{
		m_is_node = 1;
		isolate_ = reinterpret_cast<Isolate*>(v8_isolate);
		context_ = *reinterpret_cast<v8::Local<v8::Context>*>(v8_context);
	}

	virtual void initialize() {
		isolate_->SetData(ISOLATE_INL_WORKER_DATA_INDEX, m_host);
		m_global.Reset(m_host, Cast<JSObject>(context_->Global()) );
		if (!m_is_node) {
			handle_scope_seal_ = new Wrap<v8::SealHandleScope>(isolate_);
		}
		IMPL::initialize();
	}

	virtual void release() {
		IMPL::release();
		if (!m_is_node) {
			context_->Exit();
			context_.Clear();
			delete handle_scope_seal_; handle_scope_seal_ = nullptr;
			delete handle_scope_; handle_scope_ = nullptr;
			isolate_->Exit();
			delete locker_; locker_ = nullptr;
			isolate_->Dispose(); isolate_ = nullptr;
		}
	}

	inline static Worker* worker(Isolate* isolate) {
		return static_cast<Worker*>( isolate->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
	}
	
	inline static Worker* worker(V8FunctionCall args) {
		return static_cast<Worker*>( args.GetIsolate()->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
	}
	
	inline static Worker* worker(V8PropertyCall args) {
		return static_cast<Worker*>( args.GetIsolate()->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
	}
	
	inline static Worker* worker(V8PropertySetCall args) {
		return static_cast<Worker*>( args.GetIsolate()->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
	}

	inline static WorkerIMPL* current(Worker* worker = Worker::worker()) {
		return static_cast<WorkerIMPL*>(worker->m_inl);
	}
	
	inline static WorkerIMPL* current(WorkerIMPL* worker) {
		return worker;
	}
	
	XX_INLINE v8::Local<v8::String> NewFromOneByte(cchar* str) {
		return v8::String::NewFromOneByte(isolate_, (byte*)str);
	}
	
	XX_INLINE v8::Local<v8::String> NewFromUtf8(cchar* str) {
		return v8::String::NewFromUtf8(isolate_, str);
	}
	
	template <class T, class M = NonCopyablePersistentTraits<T>>
	XX_INLINE v8::Local<T> strong(const v8::Persistent<T, M>& persistent) {
		return *reinterpret_cast<v8::Local<T>*>(const_cast<v8::Persistent<T, M>*>(&persistent));
	}
	
	v8::MaybeLocal<v8::Value> runScript(v8::Local<v8::String> source_string,
																	 v8::Local<v8::String> name, v8::Local<v8::Object> sandbox) 
	{
		v8::ScriptCompiler::Source source(source_string, ScriptOrigin(name));
		v8::MaybeLocal<v8::Value> result;
		
		if ( sandbox.IsEmpty() ) { // use default sandbox
			v8::Local<v8::Script> script;
			if ( v8::ScriptCompiler::Compile(context_, &source).ToLocal(&script) ) {
				result = script->Run(context_);
			}
		} else {
			v8::Local<v8::Function> func;
			if (v8::ScriptCompiler::
					CompileFunctionInContext(context_, &source, 0, NULL, 1, &sandbox)
					.ToLocal(&func)
					) {
				result = func->Call(context_, v8::Undefined(isolate_), 0, NULL);
			}
		}
		return result;
	}
	
	Local<JSValue> runNativeScript(cBuffer& source, cString& name, Local<JSObject> exports) {
		v8::Local<v8::Value> _name = Back(m_host->New(String::format("%s", *name)));
		v8::Local<v8::Value> _souece = Back(m_host->NewString(source));
		
		v8::MaybeLocal<v8::Value> rv;
		
		rv = runScript(_souece.As<v8::String>(),
										_name.As<v8::String>(), v8::Local<v8::Object>());
		if ( !rv.IsEmpty() ) {
			Local<JSObject> module = m_host->NewObject();
			module->Set(m_host, m_host->strs()->exports(), exports);
			v8::Local<v8::Function> func = rv.ToLocalChecked().As<v8::Function>();
			v8::Local<v8::Value> args[] = { Back(exports), Back(module), Back(m_global.local()) };
			rv = func->Call(CONTEXT(m_host), v8::Undefined(ISOLATE(this)), 3, args);
			if (!rv.IsEmpty()) {
				Local<JSValue> rv = module->Get(m_host, m_host->strs()->exports());
				XX_ASSERT(rv->IsObject(m_host));
				return rv;
			}
		}
		return Local<JSValue>();
	}
	
	// Extracts a C string from a V8 Utf8Value.
	static cchar* to_cstring(const v8::String::Utf8Value& value) {
		return *value ? *value : "<string conversion failed>";
	}
	
	String parse_exception_message(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
		v8::HandleScope handle_scope(isolate_);
		v8::String::Utf8Value exception(error);
		const char* exception_string = to_cstring(exception);
		if (message.IsEmpty()) {
			// V8 didn't provide any extra information about this error; just
			return exception_string;
		} else {
			StringBuilder out;
			// Print (filename):(line number): (message).
			v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
			v8::Local<v8::Context> context(isolate_->GetCurrentContext());
			const char* filename_string = to_cstring(filename);
			int linenum = message->GetLineNumber(context).FromJust();
			out.push(String::format("%s:%d: %s\n", filename_string, linenum, exception_string));
			// Print line of source code.
			v8::String::Utf8Value sourceline(message->GetSourceLine(context).ToLocalChecked());
			const char* sourceline_string = to_cstring(sourceline);
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
				
				if (error.As<v8::Object>()->Get(context_, NewFromOneByte("stack"))
						.ToLocal(&stack_trace_string) &&
						stack_trace_string->IsString() &&
						v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
					v8::String::Utf8Value stack_trace(stack_trace_string);
					const char* stack_trace_string = to_cstring(stack_trace);
					out.push( stack_trace_string ); out.push('\n');
				}
			}
			return out.to_string();
		}
	}
	
	void print_exception(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
		XX_ERR( parse_exception_message(message, error) );
	}

	static void OnFatalError(const char* location, const char* message) {
		if (location) {
			XX_FATAL("FATAL ERROR: %s %s\n", location, message);
		} else {
			XX_FATAL("FATAL ERROR: %s\n", message);
		}
	}

	static void MessageCallback(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
		current()->uncaught_exception(message, error);
	}

	static void PromiseRejectCallback(PromiseRejectMessage message) {
		if (message.GetEvent() == v8::kPromiseRejectWithNoHandler) {
			current()->unhandled_rejection(message);
		}
	}

	void uncaught_exception(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
		if ( !TriggerUncaughtException(Cast(error)) ) {
			print_exception(message, error);
			ngui::exit(ERR_UNCAUGHT_EXCEPTION);
		}
	}

	void unhandled_rejection(PromiseRejectMessage& message) {
		v8::Local<v8::Promise> promise = message.GetPromise();
		v8::Local<v8::Value> reason = message.GetValue();
		if (reason.IsEmpty())
			reason = v8::Undefined(isolate_);
		if ( !TriggerUnhandledRejection(Cast(reason), Cast(promise)) ) {
			v8::HandleScope scope(isolate_);
			v8::Local<v8::Message> message = v8::Exception::CreateMessage(isolate_, reason);
			print_exception(message, reason);
			ngui::exit(ERR_UNHANDLED_REJECTION);
		}
	}

};

Worker* Worker::worker() {
	void* worker = v8::Isolate::GetCurrent()->GetData(ISOLATE_INL_WORKER_DATA_INDEX);
	return reinterpret_cast<Worker*>(worker);
}

Worker* IMPL::create() {
	auto inl = new WorkerIMPL();
	inl->initialize();
	return inl->host();
}

XX_EXPORT Worker* createWorkerWithNode(void* isolate, void* context) {
	auto inl = new WorkerIMPL(isolate, context);
	inl->initialize();
	return inl->host();
}

WrapObject* IMPL::GetObjectPrivate(Local<JSObject> object) {
	if (Back<v8::Object>(object)->InternalFieldCount() > 0) {
		return (WrapObject*)Back<v8::Object>(object)->GetAlignedPointerFromInternalField(0);
	}
	return nullptr;
}

bool IMPL::SetObjectPrivate(Local<JSObject> object, WrapObject* value) {
	if (Back<v8::Object>(object)->InternalFieldCount() > 0) {
		Back<v8::Object>(object)->SetAlignedPointerInInternalField(0, value);
		return true;
	}
	return false;
}

/**
 * @class V8JSClass
 */
class V8JSClass: public JSClassIMPL {
 public:
	V8JSClass(Worker* worker, uint64 id, cString& name,
						FunctionCallback constructor, V8JSClass* base,
						v8::Local<v8::Function> base2 = v8::Local<v8::Function>())
	: JSClassIMPL(worker, id, name), parent_(base)
	{ //
		v8::FunctionCallback cb = reinterpret_cast<v8::FunctionCallback>(constructor);
		v8::Local<v8::FunctionTemplate> temp = v8::FunctionTemplate::New(ISOLATE(worker), cb);
		v8::Local<v8::String> class_name = Back<v8::String>(worker->New(name, true));
		if ( base ) {
			temp->Inherit( base->Template() );
		}
		else if ( !base2.IsEmpty() ) {
			parent_2_.Reset(ISOLATE(worker), base2);
		}
		temp->SetClassName(class_name);
		temp_.Reset(ISOLATE(worker), temp);
	}
	
	~V8JSClass() {
		temp_.Reset();
		parent_2_.Reset();
	}
	
	v8::Local<v8::FunctionTemplate> Template() {
		auto _ = reinterpret_cast<v8::Local<v8::FunctionTemplate>*>(&temp_);
		return *_;
	}
	
	v8::Local<v8::Function> ParentFromFunction() {
		auto _ = reinterpret_cast<v8::Local<v8::Function>*>(&parent_2_);
		return *_;
	}
	
	bool HasParentFromFunction() {
		return !parent_2_.IsEmpty();
	}
	
 private:
	V8JSClass* parent_;
	v8::Persistent<v8::Function> parent_2_;
	v8::Persistent<v8::FunctionTemplate> temp_;
};

Worker* WeakCallbackInfo::worker() const {
	auto info = reinterpret_cast<const v8::WeakCallbackInfo<Object>*>(this);
	return WorkerIMPL::worker(info->GetIsolate());
}

void* WeakCallbackInfo::GetParameter() const {
	return reinterpret_cast<const v8::WeakCallbackInfo<void>*>(this)->GetParameter();
}

bool IMPL::IsWeak(PersistentBase<JSObject>& handle) {
	XX_ASSERT( !handle.IsEmpty() );
	auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(&handle);
	return h->IsWeak();
}

void IMPL::SetWeak(PersistentBase<JSObject>& handle,
												WrapObject* ptr, WeakCallbackInfo::Callback callback) {
	XX_ASSERT( !handle.IsEmpty() );
	auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(&handle);
	h->MarkIndependent();
	h->SetWeak(ptr, reinterpret_cast<v8::WeakCallbackInfo<WrapObject>::Callback>(callback),
						 v8::WeakCallbackType::kParameter);
}

void IMPL::ClearWeak(PersistentBase<JSObject>& handle, WrapObject* ptr) {
	XX_ASSERT( !handle.IsEmpty() );
	auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(&handle);
	h->ClearWeak();
}

Local<JSFunction> IMPL::GenConstructor(Local<JSClass> cls) {
	V8JSClass* v8cls = reinterpret_cast<V8JSClass*>(*cls);
	auto f = v8cls->Template()->GetFunction(CONTEXT(m_host)).FromMaybe(v8::Local<v8::Function>());
	if ( v8cls->HasParentFromFunction() ) {
		bool ok;
		// function.__proto__ = base
		// ok = f->SetPrototype(v8cls->ParentFromFunction());
		// XX_ASSERT(ok);
		// function.prototype.__proto__ = base.prototype
		auto b = v8cls->ParentFromFunction();
		auto s = Back(m_host->strs()->prototype());
		auto p = f->Get(CONTEXT(m_host), s).ToLocalChecked().As<v8::Object>();
		auto p2 = b->Get(CONTEXT(m_host), s).ToLocalChecked().As<v8::Object>();
		ok = p->SetPrototype(p2);
		XX_ASSERT(ok);
	}
	return Cast<JSFunction>(f);
}

Local<JSValue> IMPL::binding_node_module(cString& name) {
	if (node::ngui_node_api) {
		void* r = node::ngui_node_api->binding_node_module(*name);
		auto _ = reinterpret_cast<Local<JSValue>*>(&r);
		return *_;
	}
	m_host->throwError(m_host->NewError("Cannot find module %s", *name));
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
  return reinterpret_cast<const v8::Value*>(this)->IsArrayBufferView();
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
			count = str->WriteOneByte((byte*)*buffer, index, 128);
			rev.push(*buffer, count);
			index += count;
		} while(count);
		return rev;
	} else {
		StringBuilder rev;
		ArrayBuffer<uint16> buffer(128);
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
Ucs2String JSValue::ToUcs2StringValue(Worker* worker) const {
	JSC_ENV(worker);
	v8::JSCStringPtr s = JSValueToStringCopy(ctx, reinterpret_cast<JSValueRef>(this),
																					 OK(unknown_ucs2));
	size_t len = JSStringGetLength(*s);
	const uint16* ptr = JSStringGetCharactersPtr(*s);
	WeakArrayBuffer<uint16> bf(ptr, uint(len));
	return bf.copy().collapse_string();
}
#else
Ucs2String JSValue::ToUcs2StringValue(Worker* worker) const {
	v8::Local<v8::String> str = ((v8::Value*)this)->ToString();
	if ( str.IsEmpty() ) return unknown_ucs2;
	Ucs2String rev;
	uint16 buffer[512];
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

uint JSValue::ToUint32Value(Worker* worker) const {
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

Local<JSValue> JSObject::Get(Worker* worker, uint index) {
	return Cast(reinterpret_cast<v8::Object*>(this)->
							Get(CONTEXT(worker), index).FromMaybe(v8::Local<v8::Value>()));
}

bool JSObject::Set(Worker* worker, Local<JSValue> key, Local<JSValue> val) {
	return reinterpret_cast<v8::Object*>(this)->
		Set(CONTEXT(worker), Back(key), Back(val)).FromMaybe(false);
}

bool JSObject::Set(Worker* worker, uint index, Local<JSValue> val) {
	return reinterpret_cast<v8::Object*>(this)->
		Set(CONTEXT(worker), index, Back(val)).FromMaybe(false);
}

bool JSObject::Has(Worker* worker, Local<JSValue> key) {
	return reinterpret_cast<v8::Object*>(this)->
		Has(CONTEXT(worker), Back(key)).FromMaybe(false);
}

bool JSObject::Has(Worker* worker, uint index) {
	return reinterpret_cast<v8::Object*>(this)->
		Has(CONTEXT(worker), index).FromMaybe(false);
}

bool JSObject::JSObject::Delete(Worker* worker, Local<JSValue> key) {
	return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), Back(key)).FromMaybe(false);
}

bool JSObject::Delete(Worker* worker, uint index) {
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

int JSString::Length(Worker* worker) const {
	return reinterpret_cast<const v8::String*>(this)->Length();
}
String JSString::Value(Worker* worker, bool ascii) const {
	return ToStringValue(worker, ascii);
}
Ucs2String JSString::Ucs2Value(Worker* worker) const {
	return ToUcs2StringValue(worker);
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
int64 JSInteger::Value(Worker* worker) const {
	return reinterpret_cast<const v8::Integer*>(this)->Value();
}
uint JSUint32::Value(Worker* worker) const {
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

int JSArrayBuffer::ByteLength(Worker* worker) const {
	return (uint)reinterpret_cast<const v8::ArrayBuffer*>(this)->ByteLength();
}
char* JSArrayBuffer::Data(Worker* worker) {
	return (char*)reinterpret_cast<v8::ArrayBuffer*>(this)->GetContents().Data();
}
Local<JSArrayBuffer> JSTypedArray::Buffer(Worker* worker) {
	auto ab = reinterpret_cast<v8::ArrayBufferView*>(this);
	v8::Local<v8::ArrayBuffer> ab2 = ab->Buffer();
	return Cast<JSArrayBuffer>(ab2);
}

int JSTypedArray::ByteLength(Worker* worker) {
  auto ab = reinterpret_cast<v8::ArrayBufferView*>(this);
  return (uint)ab->ByteLength();
}

int JSTypedArray::ByteOffset(Worker* worker) {
  auto ab = reinterpret_cast<v8::ArrayBufferView*>(this);
  return (uint)ab->ByteOffset();
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
	v8::Local<Signature> s = Signature::New(ISOLATE(worker), temp);
	v8::Local<v8::FunctionTemplate> t =
		FunctionTemplate::New(ISOLATE(worker), func2, v8::Local<v8::Value>(), s);
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

template<> bool JSClass::SetMemberProperty<Local<JSValue>>
(
 Worker* worker, cString& name, Local<JSValue> value
) {
	reinterpret_cast<V8JSClass*>(this)->Template()->
					PrototypeTemplate()->Set(Back<v8::String>(worker->New(name, 1)), Back(value));
	return true;
}

template<> bool JSClass::SetStaticProperty<Local<JSValue>>
(
 Worker* worker, cString& name, Local<JSValue> value
) {
	reinterpret_cast<V8JSClass*>(this)->Template()->
					Set(Back<v8::String>(worker->New(name, 1)), Back(value));
	return true;
}

template <> XX_EXPORT Local<JSValue> MaybeLocal<JSValue>::ToLocalChecked() {
	return Cast(reinterpret_cast<v8::MaybeLocal<v8::Value>*>(this)->ToLocalChecked());
}

void JSClass::SetInstanceInternalFieldCount(int count) {
	reinterpret_cast<V8JSClass*>(this)->Template()->
		InstanceTemplate()->SetInternalFieldCount(count);
}

int JSClass::InstanceInternalFieldCount() {
	return reinterpret_cast<V8JSClass*>(this)->Template()->
		InstanceTemplate()->InternalFieldCount();
}

void ReturnValue::Set(bool value) {
	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(value);
}

void ReturnValue::Set(double i) {
	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
}

void ReturnValue::Set(int i) {
	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
}

void ReturnValue::Set(uint i) {
	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
}

void ReturnValue::SetNull() {
	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetNull();
}

void ReturnValue::SetUndefined() {
	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetUndefined();
}

void ReturnValue::SetEmptyString() {
	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetEmptyString();
}

template<> void ReturnValue::Set<JSValue>(const Local<JSValue> value) {
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
	return WorkerIMPL::worker(info->GetIsolate());
}

Worker* PropertyCallbackInfo::worker() const {
	auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
	return WorkerIMPL::worker(info->GetIsolate());
}

Worker* PropertySetCallbackInfo::worker() const {
	auto info = reinterpret_cast<const v8::PropertyCallbackInfo<void>*>(this);
	return WorkerIMPL::worker(info->GetIsolate());
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

template <> void PersistentBase<JSValue>::Reset() {
	reinterpret_cast<v8::PersistentBase<v8::Value>*>(this)->Reset();
}

template <> template <>
void PersistentBase<JSValue>::Reset(Worker* worker, const Local<JSValue>& other) {
	XX_ASSERT(worker);
	reinterpret_cast<v8::PersistentBase<v8::Value>*>(this)->
		Reset(ISOLATE(worker), *reinterpret_cast<const v8::Local<v8::Value>*>(&other));
	worker_ = worker;
}

template<> template<>
void PersistentBase<JSValue>::Copy(const PersistentBase<JSValue>& that) {
	XX_ASSERT(that.worker_);
	typedef v8::CopyablePersistentTraits<v8::Value>::CopyablePersistent Handle;
	reinterpret_cast<Handle*>(this)->operator=(*reinterpret_cast<const Handle*>(&that));
	worker_ = that.worker_;
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

Local<JSInt32> Worker::New(char data) {
	return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
}

Local<JSUint32> Worker::New(byte data) {
	return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
}

Local<JSInt32> Worker::New(int16 data) {
	return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
}

Local<JSUint32> Worker::New(uint16 data) {
	return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
}

Local<JSInt32> Worker::New(int data) {
	return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
}

Local<JSUint32> Worker::New(uint data) {
	return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
}

Local<JSNumber> Worker::New(int64 data) {
	return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
}

Local<JSNumber> Worker::New(uint64 data) {
	return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
}

Local<JSString> Worker::New(cchar* data, int len) {
  return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this),
                                                data, v8::String::kNormalString, len < 0? -1: len));
}

Local<JSString> Worker::New(cString& data, bool is_ascii) {
	if ( is_ascii ) {
		return Cast<JSString>(v8::String::NewExternal(ISOLATE(this),
																									new V8ExternalOneByteStringResource(data)));
	} else {
		return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									v8::String::kNormalString, data.length()));
	}
}

Local<JSString> Worker::New(cUcs2String& data) {
	return Cast<JSString>(v8::String::NewExternal(ISOLATE(this), new V8ExternalStringResource(data)));
}

Local<JSArray> Worker::New(const Array<String>& data) {
	auto worker = CURRENT;
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

Local<JSObject> Worker::New(const Map<String, String>& data) {
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

Local<JSValue> Worker::New(const PersistentBase<JSValue>& value) {
	v8::Local<v8::Value> r =
		reinterpret_cast<const v8::PersistentBase<v8::Value>*>(&value)->Get(ISOLATE(this));
	return Cast(r);
}
Local<JSArrayBuffer> Worker::NewArrayBuffer(char* use_buff, uint len) {
  return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), use_buff, len));
}
Local<JSArrayBuffer> Worker::NewArrayBuffer(uint len) {
  return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), len));
}
Local<JSUint8Array> Worker::NewUint8Array(Local<JSArrayBuffer> ab, uint offset, uint size) {
  auto ab2 = Back<v8::ArrayBuffer>(ab);
  offset = XX_MIN((uint)ab2->ByteLength(), offset);
  if (size + offset > ab2->ByteLength()) {
    size = (uint)ab2->ByteLength() - offset;
  }
  return Cast<JSUint8Array>(v8::Uint8Array::New(ab2, offset, size));
}
Local<JSObject> Worker::NewObject() {
	return Cast<JSObject>(v8::Object::New(ISOLATE(this)));
}

Local<JSArray> Worker::NewArray(uint len) {
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

Local<JSString> Worker::NewString(const Buffer& data) {
	return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this),
																								*data, v8::String::kNormalString,
																								data.length()));
}

Local<JSString> Worker::NewAscii(cchar* str) {
  return Cast<JSString>(v8::String::NewFromOneByte(ISOLATE(this),
                                                   (const uint8_t *)str, v8::String::kNormalString));
}
  
  Local<JSString> NewAscii(cchar* str);

Local<JSObject> Worker::NewRangeError(cchar* errmsg, ...) {
	XX_STRING_FORMAT(errmsg, str);
	return Cast<JSObject>(v8::Exception::RangeError(Back(New(str))->ToString()));
}

Local<JSObject> Worker::NewReferenceError(cchar* errmsg, ...) {
	XX_STRING_FORMAT(errmsg, str);
	return Cast<JSObject>(v8::Exception::ReferenceError(Back(New(str))->ToString()));
}

Local<JSObject> Worker::NewSyntaxError(cchar* errmsg, ...) {
	XX_STRING_FORMAT(errmsg, str);
	return Cast<JSObject>(v8::Exception::SyntaxError( Back(New(str))->ToString() ));
}

Local<JSObject> Worker::NewTypeError(cchar* errmsg, ...) {
	XX_STRING_FORMAT(errmsg, str);
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
	for (uint i = 0, j = 0; i < names->Length(); i++) {
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
    char* data = buff.collapse();
    ab = v8::ArrayBuffer::New(ISOLATE(this), data, len);
  } else {
    ab = v8::ArrayBuffer::New(ISOLATE(this), 0);
  }
  return Cast<JSUint8Array>(v8::Uint8Array::New(ab, offset, len));
}

void Worker::throwError(Local<JSValue> exception) {
	ISOLATE(this)->ThrowException(Back(exception));
}

Local<JSClass> Worker::NewClass(uint64 id,
																cString& name,
																FunctionCallback constructor,
																WrapAttachCallback attach_callback, Local<JSClass> base) {
	auto cls = new V8JSClass(this, id, name, constructor, reinterpret_cast<V8JSClass*>(*base));
	Local<JSClass> rv(reinterpret_cast<JSClass*>(cls));
	m_inl->m_classs->set_class(id, rv, attach_callback);
	return rv;
}

Local<JSClass> Worker::NewClass(uint64 id, cString& name,
																FunctionCallback constructor,
																WrapAttachCallback attach_callback, uint64 base) {
	return NewClass(id, name, constructor, attach_callback, m_inl->m_classs->get_class(base));
}

/**
 * @func NewClass js class
 */
Local<JSClass> Worker::NewClass(uint64 id, cString& name,
																FunctionCallback constructor,
																WrapAttachCallback attach_callback, Local<JSFunction> base) {
	auto cls = new V8JSClass(this, id, name, constructor, nullptr, Back<v8::Function>(base));
	Local<JSClass> rv(reinterpret_cast<JSClass*>(cls));
	m_inl->m_classs->set_class(id, rv, attach_callback);
	return rv;
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

/**
 * @func garbage_collection()
 */
void Worker::garbageCollection() {
	ISOLATE(this)->LowMemoryNotification();
}

int IMPL::start(int argc, char** argv) {
	v8::Platform* platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();

	// Unconditionally force typed arrays to allocate outside the v8 heap. This
	// is to prevent memory pointers from being moved around that are returned by
	// Buffer::Data().
	const char no_typed_array_heap[] = "--typed_array_max_size_in_heap=0";
	v8::V8::SetFlagsFromString(no_typed_array_heap, sizeof(no_typed_array_heap) - 1);
	v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

	int rc = 0;
	{
		Handle<Worker> worker = IMPL::create();
		{
			HandleScope scope(*worker);
			Local<JSValue> module = worker->runNativeScript(WeakBuffer((char*)
					INL_native_js_code_module_, 
					INL_native_js_code_module_count_), "module.js"
			);
			XX_CHECK(!module.IsEmpty(), "Can't start worker");

			Local<JSValue> r = module.To()->
				GetProperty(*worker, "runMain").To<JSFunction>()->Call(*worker);
			if (r.IsEmpty()) {
				XX_ERR("ERROR: Can't call runMain()");
				return ERR_RUN_MAIN_EXCEPTION;
			}
		}

		auto loop = RunLoop::main_loop();

		do {
			loop->run();
			/* IOS forces the process to terminate, but it does not quit immediately.
			 This may cause a process to run in the background for a long time, so force break here */
			if (is_exited())
				break;

			if (loop->is_alive())
				continue;

			rc = worker->m_inl->TriggerBeforeExit(rc);

			// Emit `beforeExit` if the loop became alive either after emitting
			// event, or after running some callbacks.
		} while (loop->is_alive());

		if (!is_exited())
			rc = worker->m_inl->TriggerExit(rc);
	}

	v8::V8::ShutdownPlatform();
	v8::V8::Dispose();
	delete platform;

	return rc;
}

JS_END
