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

#include <JavaScriptCore/JavaScript.h>
#include <v8.h>
#include "util.h"
#include <pthread.h>
#include <string>
#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <limits>
#include <sstream>
#include "native-js.h"
#include <v8-version-string.h>

#define RETURN_TO_LOCAL_UNCHECKED(maybe_local, T) \
	return maybe_local.FromMaybe(Local<T>());

// V has parameters (Type, type, TYPE, C type, element_size)
#define TYPED_ARRAYS(V) \
	V(Uint8, uint8, UINT8, uint8_t, 1)	\
	V(Int8, int8, INT8, int8_t, 1)	\
	V(Uint16, uint16, UINT16, uint16_t, 2)	\
	V(Int16, int16, INT16, int16_t, 2)	\
	V(Uint32, uint32, UINT32, uint32_t, 4)	\
	V(Int32, int32, INT32, int32_t, 4)	\
	V(Float32, float32, FLOAT32, float, 4)	\
	V(Float64, float64, FLOAT64, double, 8)	\
	V(Uint8Clamped, uint8_clamped, UINT8_CLAMPED, uint8_t, 1)

#define NOTHING OK(v8::Nothing<bool>())

#define JS_CONST_LIST(F) \
F(__native__) \
F(prototype) \
F(set) \
F(get) \
F(configurable) \
F(enumerable) \
F(writable) \
F(length) \
F(name) \
F(column) \
F(line) \
F(message) \
F(stack) \
F(_stack) \
F(constructor) \
F(parent) \
F(data) \
F(value) \
F(result) \
F(source) \
F(flags) \
F(undefined) \
F(null) \
F(object) \
F(boolean) \
F(string) \
F(number) \
F(size) \
F(scriptId) \
F(isWasm) \
F(functionName) \
F(scriptName) \
F(sourceMapUrl) \
F(programFunc) \
F(isEval) \
F(isConstructor) \
F(prototype_instance) \
F(function_instance) \
F(instance_template) \
F(prototype_template) \
F(toString) \

#define JS_ISOLATE_DATA(F) \
F(getPropertyNames) \
F(getOwnPropertyNames) \
F(getOwnPropertyNames2) \
F(getOwnPropertyDescriptor) \
F(getOwnPropertyDescriptors) \
F(defineProperty) \
F(defineProperties) \
F(hasOwnProperty) \
F(isSymbol) \
F(promiseState) \
F(promiseCatch) \
F(promiseThen) \
F(stringLength) \
F(typeOf) \
F(valueOf) \
F(newRegExp) \
F(wrapFunctionScript) \
F(mapAsArray) \
F(setAsArray) \
F(newPrivateValue) \
F(symbolName) \
F(NativeToString) \
F(Function_prototype) \
F(symbol_for_api) \
F(private_for_api) \
F(JSONStringify) \
F(JSONParse) \
F(mapSet) \
F(mapGet) \
F(mapClear) \
F(mapHas) \
F(mapDelete) \
F(setClear) \
F(setAdd) \
F(setHas) \
F(setDelete) \
F(symbolFor) \
F(stringConcat) \
F(getProperty) \
F(setProperty) \
F(deleteProperty) \
F(hasProperty) \

#define JS_CONTEXT_DATA(F) \
F(Object) \
F(Function) \
F(Number) \
F(Boolean) \
F(String) \
F(Date) \
F(RegExp) \
F(Error) \
F(RangeError) \
F(ReferenceError) \
F(SyntaxError) \
F(TypeError) \
F(Map) \
F(Set) \
F(WeakMap) \
F(WeakSet) \
F(Symbol) \
F(Proxy) \
F(Promise) \
F(DataView) \
/*F(SharedArrayBuffer)*/ \
F(ArrayBuffer) \
F(TypedArray) \
F(Uint8Array) \
F(Int8Array) \
F(Uint16Array) \
F(Int16Array) \
F(Uint32Array) \
F(Int32Array) \
F(Float32Array) \
F(Float64Array) \
F(Uint8ClampedArray) \
F(AsyncFunction) \
F(SetIterator) \
F(MapIterator) \

#define ENV(...) \
auto isolate = ISOLATE(__VA_ARGS__); \
auto ctx = isolate->jscc(); \
JSValueRef ex = nullptr
#define ISOLATE(...)  v8::internal::Isolate::Current(__VA_ARGS__)
#define CONTEXT(...)  v8::internal::Isolate::Current(__VA_ARGS__)->GetCurrentContext()
#define JSC_CTX(...)  ISOLATE(__VA_ARGS__)->jscc()
#define OK(...) &ex); do { \
if (ex) { \
	isolate->ThrowException(ex); \
	return __VA_ARGS__ ;\
}}while(0

#define JSStringWithUTF8(S) JSStringCreateWithUTF8CString(S)

#ifndef XX_MEMORY_TRACE_MARK
# define XX_MEMORY_TRACE_MARK 0
#endif

v8_ns(v8)

namespace i = internal;

v8_ns(internal)

static JSStringRef __native__body_s = JSStringWithUTF8
(
 "return arguments.callee.__native__.apply(this,arguments)"
 );
static JSStringRef anonymous_s = JSStringWithUTF8("<anonymous>");
static JSStringRef eval_s = JSStringWithUTF8("<eval>");
static JSStringRef empty_s = JSStringWithUTF8("");

#define DEF_JSSTRING(NAME) \
	static JSStringRef NAME##_s = JSStringWithUTF8(#NAME);
JS_CONST_LIST(DEF_JSSTRING)
JS_ISOLATE_DATA(DEF_JSSTRING)
JS_CONTEXT_DATA(DEF_JSSTRING)
#undef DEF_JSSTRING

class Object {};
class Context;
class Message;
class Template;
class ObjectTemplate;
class FunctionTemplate;
class PrivateData;
class Wrap;
typedef class CallbackInfo CInfo;

// Hypothesis:
// struct OpaqueJSValue: public Object { };
// typedef const struct OpaqueJSValue* JSValueRef;
// typedef struct OpaqueJSValue* JSObjectRef;

v8_ns_end

#include "jsc-v8-isolate.cc.inl"

v8_ns(internal)

struct EmbedderData {
	enum Type {
		CPointer = 0,
		JSValue = 1,
	};
	
	void Reset(JSContextRef ctx) {
		if (value && type == JSValue)
			JSValueUnprotect(ctx, (JSValueRef)value);
		type = CPointer;
		value = nullptr;
	}
	void SetEmbedderData(JSContextRef ctx, Local<Value> v) {
		Reset(ctx);
		JSValueProtect(ctx, Back(v));
		type = JSValue;
		value = (void*)*v;
	}
	void SetAlignedPointer(JSContextRef ctx, void* v) {
		Reset(ctx);
		type = CPointer;
		value = v;
	}
	Local<Value> GetEmbedderData() {
		if (value && type == JSValue) {
			return Cast((JSValueRef)value);
		}
		return Local<Value>();
	}
	void* GetAlignedPointer() {
		if (type == CPointer) {
			return value;
		}
		return nullptr;
	}
	void* value = nullptr;
	Type type = CPointer;
};

class PrivateDataBase: public Object {
 public:
	virtual PrivateData* AsPrivateData() { return nullptr; }
	virtual CallbackInfo* AsCallbackInfo() { return nullptr; }
	virtual Wrap* AsWrap() { return nullptr; }
};

#if XX_MEMORY_TRACE_MARK
static int PrivateData_count = 0;
#endif

class PrivateData: public PrivateDataBase {
 public:
	typedef DefaultTraits Traits;
	
	enum Flags {
		kINDEPENDENT = 1,
		kWEAK = 1 << 1,
		kNEAR_DEATH = 1 << 2,
		kETERNAL = 1 << 3,
	};
	
	typedef void (*WeakCallback)(const WeakCallbackInfo<void>& data);
	virtual ~PrivateData();
	inline void Destroy();
	virtual PrivateData* AsPrivateData() { return this; }
	inline Isolate* GetIsolate() const { return m_isolate; }
	static void DefaultWeakCallback(const WeakCallbackInfo<void>& data) {}
	inline JSObjectRef Handle() const { return m_handle; }
	inline ObjectTemplate* InstanceTemplate() const { return m_instance_template; }
	inline void MarkIndependent() { m_flags |= kINDEPENDENT; UpdateWeakFlags(); }
	inline bool IsIndependent() const { return m_flags & kINDEPENDENT; }
	inline bool IsWeak() const { return m_flags & kWEAK; }
	inline bool IsNearDeath() const { return m_flags & kNEAR_DEATH; }
	inline void Eternalize() { m_flags |= kETERNAL; UpdateWeakFlags(); }
	void ExecWeakCallback();
	void UpdateWeakFlags();
	
	inline void SetWeak() {
		SetWeak(nullptr, DefaultWeakCallback, WeakCallbackType::kParameter);
	}
	
	inline void SetWeak(void* parameter, WeakCallback callback, WeakCallbackType type) {
		DCHECK(callback);
		m_weak_parameter = parameter;
		m_weak_callback = callback;
		UpdateWeakFlags();
	}
	
	inline void* ClearWeak() {
		void* parameter = m_weak_parameter;
		m_weak_parameter = nullptr;
		m_weak_callback = nullptr;
		UpdateWeakFlags();
		return parameter;
	}
	
	inline void retain() {
		m_ref_count++;
		UpdateWeakFlags();
	}
	
	inline void release() {
		if (m_ref_count > 0) {
			m_ref_count--;
			UpdateWeakFlags();
		}
	}
	
	inline void SetHandle(JSObjectRef handle) {
		DCHECK(m_handle == nullptr);
		DCHECK(handle);
		bool ok = JSObjectSetPrivate(handle, this);
		DCHECK(ok);
		m_handle = handle;
		ScopeRetain(GetIsolate(), handle);
	}
	
	inline void SetHandleNotBindPrivate(JSObjectRef handle) {
		DCHECK(m_handle == nullptr);
		DCHECK(handle);
		m_handle = handle;
		ScopeRetain(GetIsolate(), handle);
	}
	
	inline static PrivateData* Private(JSObjectRef value) {
		auto priv = (PrivateData*)JSObjectGetPrivate(value);
		return priv ? priv->AsPrivateData() : nullptr;
	}
	inline static PrivateData* Private(Object* o);
	static PrivateData* EnsurePrivate(Isolate* isolate, JSObjectRef value);
	inline int InternalFieldCount() const;
	
	/**
	 * If there is a circular reference to `EmbedderData` form JSValueRef,
	 * it will cause Object to be destroyed
	 */
	inline EmbedderData* InternalField(int index);
	
	static PrivateData* New(Isolate* isolate, JSObjectRef handle,
													ObjectTemplate* instance_template);
	
 protected:
	inline PrivateData(Isolate* isolate, JSObjectRef handle,
												 ObjectTemplate* instance_template)
	: m_handle(nullptr), m_instance_template(Retain(instance_template))
	, m_isolate(isolate)
	, m_weak_parameter(nullptr), m_weak_callback(nullptr)
	, m_flags(kWEAK), m_ref_count(0) {
		DCHECK(m_instance_template);
		if (handle) SetHandle(handle);
#if XX_MEMORY_TRACE_MARK
		PrivateData_count++;
		LOG("PrivateData::PrivateData, %d", PrivateData_count);
#endif
	}
	inline PrivateData(Isolate* isolate, JSObjectRef handle)
	: PrivateData(isolate, handle, isolate->DefaultPlaceholderTemplate()) {}
 private:
	JSObjectRef m_handle;
	ObjectTemplate* m_instance_template;
	Isolate* m_isolate;
	void* m_weak_parameter;
	WeakCallback m_weak_callback;
	uint16_t m_flags;
	uint16_t m_ref_count;
	friend class WrapperPrivateData;
	friend class ObjectPrivateData;
	friend class CallbackInfo;
};

class CallbackInfo: public PrivateData {
 public:
	enum Style {
		ACCESSOR = 0,
		FUNCTION,
		FUNCTION_CONSTRUCTOR,
		FUNCTION_TEMPLATE,
	};
	inline CallbackInfo(Isolate* isolate,
											JSObjectRef handle,
											FunctionTemplate* receiver_signature,
											JSValueRef data,
											JSValueRef name,
											void* front_callback,
											Style front_callback_style)
	: PrivateData(isolate, handle, isolate->CInfoPlaceholderTemplate())
	, m_receiver_signature(Retain(receiver_signature))
	, m_data(Retain(JSC_CTX(isolate), data))
	, m_name(Retain(JSC_CTX(isolate), name))
	, m_front_callback(front_callback)
	, m_front_callback_style(front_callback_style) {}
	virtual ~CallbackInfo();
	virtual CallbackInfo* AsCallbackInfo() { return this; }
	
	inline static CallbackInfo* Private(JSObjectRef value) {
		auto base = (PrivateData*)JSObjectGetPrivate(value);
		return base ? base->AsCallbackInfo(): nullptr;
	}
	inline bool VerificationSignature(JSObjectRef holder);
	
	// class
	static JSClassRef ObjectClass;
	static JSClassRef WrapHandleClass;
	static JSClassRef FunctionCallbackClass;
	static JSClassRef AccessorGetterCallbackClass;
	static JSClassRef AccessorSetterCallbackClass;
	// callback
	static void CallbackDestructor(JSObjectRef object);
	static void ObjectDestructor(JSObjectRef object);
	static void WrapDestructor(JSObjectRef object);
	
	static JSValueRef Function(JSContextRef ctx, JSObjectRef func, JSObjectRef self,
														 size_t argc, const JSValueRef argv[], JSValueRef* exception);
	static JSValueRef Function(CallbackInfo* cinfo, JSObjectRef func, JSObjectRef self,
														 size_t argc, const JSValueRef argv[], JSValueRef* exception);
	static JSValueRef Getter(JSContextRef ctx, JSObjectRef func, JSObjectRef self,
													 size_t argc, const JSValueRef argv[], JSValueRef* exception);
	static JSValueRef Setter(JSContextRef ctx, JSObjectRef func, JSObjectRef self,
													 size_t argc, const JSValueRef argv[], JSValueRef* exception);
	static JSValueRef GetProperty(JSContextRef ctx, JSObjectRef self,
																JSStringRef name, JSValueRef* exception);
	static bool SetProperty(JSContextRef ctx, JSObjectRef self,
													JSStringRef name, JSValueRef value, JSValueRef* exception);
	static bool HasProperty(JSContextRef ctx, JSObjectRef self,
													JSStringRef name);
	static bool DeleteProperty(JSContextRef ctx, JSObjectRef self,
														 JSStringRef name, JSValueRef* exception);
	static void GetPropertyNames(JSContextRef ctx, JSObjectRef self,
															 JSPropertyNameAccumulatorRef names);
	
	inline FunctionTemplate* ReceiverSignature() const { return m_receiver_signature; }
	inline JSValueRef Data() const { return m_data; }
	inline JSValueRef Name() const { return m_name; }
	inline Style FrontCallbackStyle() const { return m_front_callback_style; }
	inline void* FrontCallback() const { return m_front_callback; }
 private:
	FunctionTemplate* m_receiver_signature;
	JSValueRef m_data;
	JSValueRef m_name;
	void* m_front_callback;
	Style m_front_callback_style;
};

v8_ns_end
#include "jsc-v8-wrap.cc.inl"
#include "jsc-v8-template.cc.inl"
#include "jsc-v8-context.cc.inl"
v8_ns(internal)

void PrivateData::Destroy() {
	m_flags |= kNEAR_DEATH;
	auto isolate = m_isolate;
	if (isolate->ThreadID() == std::this_thread::get_id() || isolate->HasDestroy()) {
		delete this;
	} else {
		std::lock_guard<std::mutex> scope(isolate->m_garbage_handle_mutex);
		isolate->m_garbage_handle.push_back(this);
	}
}

PrivateData::~PrivateData() {
	if (!m_isolate->HasDestroy()) {
		auto instance_template = InstanceTemplate();
		auto ctx = instance_template->GetIsolate()->jscc();
		int count = instance_template->InternalFieldCount();
		for (int i = 0; i < count; i++) {
			InternalField(i)->Reset(ctx);
		}
		Release(instance_template);
		ExecWeakCallback();
	}
#if XX_MEMORY_TRACE_MARK
	PrivateData_count--;
	LOG("PrivateData::~PrivateData, %d", PrivateData_count);
#endif
}

void PrivateData::ExecWeakCallback() {
	if (m_weak_callback && IsWeak()) {
		auto isolate = m_isolate;
		void* embedder_fields[kEmbedderFieldsInWeakCallback] = { nullptr, nullptr };
		WeakCallbackInfo<void> info(reinterpret_cast<v8::Isolate*>(isolate),
																m_weak_parameter, embedder_fields, &m_weak_callback);
		m_weak_callback(info);
	}
}

void PrivateData::UpdateWeakFlags() {
	bool is_weak = false;
	if (m_flags & kETERNAL) {
		//
	} else if (m_ref_count == 0) {
		is_weak = true;
	} else if (m_flags & kINDEPENDENT) {
		is_weak = m_weak_callback;
	} else if (m_ref_count == 1 && m_weak_callback) {
		is_weak = true;
	}
	if (is_weak != IsWeak()) {
		if (is_weak) {
			m_flags |= kWEAK;  // add weak flags
			if (!GetIsolate()->HasDestroy()) { // 如果上下文在释放中,不能访问JSValueUnprotect
				JSValueUnprotect(JSC_CTX(m_isolate), m_handle);
			}
		} else {
			m_flags &= ~kWEAK; // cancel weak flags
			if (!GetIsolate()->HasDestroy()) { // 如果上下文在释放中,不能访问JSValueProtect
				JSValueProtect(JSC_CTX(m_isolate), m_handle);
			}
		}
	}
}

PrivateData* PrivateData::Private(Object* o) {
	if (!i::Wrap::IsWrap(o)) {
		auto isolate = ISOLATE();
		if (!isolate->HasDestroy()) {
			auto value = reinterpret_cast<JSObjectRef>(o);
			if (JSValueIsObject(JSC_CTX(isolate), value)) {
				return i::PrivateData::Private(value);
			}
		}
	}
	return nullptr;
}

PrivateData* PrivateData::EnsurePrivate(Isolate* isolate, JSObjectRef value) {
	auto priv = (PrivateData*)JSObjectGetPrivate(value);
	if (priv) {
		DCHECK(!priv->AsWrap());
	} else {
		DCHECK(!isolate->HasDestroy());
		auto ctx = JSC_CTX(isolate);
		
		JSObjectRef proto = value;
		do {
			proto = (JSObjectRef)JSObjectGetPrototype(ctx, proto);
			DCHECK(proto);
			if (JSValueIsObject(ctx, proto)) {
				priv = (PrivateData*)JSObjectGetPrivate(proto);
				if (priv && priv->Handle() == value) {
					return priv;
				}
			} else {
				break;
			}
		} while(true);
		
		priv = PrivateData::New(isolate, nullptr, nullptr);
		priv->SetHandleNotBindPrivate(value);
		proto = JSObjectMake(ctx, CInfo::ObjectClass, priv);
		JSObjectSetPrivate(proto, priv);
		JSObjectSetPrototype(ctx, proto, JSObjectGetPrototype(ctx, value));
		JSObjectSetPrototype(ctx, value, proto);
	}
	return priv;
}

PrivateData* PrivateData::New(Isolate* isolate,
															JSObjectRef handle,
															ObjectTemplate* instance_template) {
	size_t size = sizeof(PrivateData);
	if (instance_template) {
		size += instance_template->InternalFieldCount() * sizeof(EmbedderData);
		instance_template = Retain(instance_template);
	} else {
		instance_template = isolate->DefaultPlaceholderTemplate();
	}
	PrivateData* priv = (PrivateData*)malloc(size);
	memset((void*)priv, 0, size);
	new(priv) PrivateData(isolate, handle, instance_template);
	return priv;
}

int PrivateData::InternalFieldCount() const {
	return InstanceTemplate()->InternalFieldCount();
}

EmbedderData* PrivateData::InternalField(int index) {
	DCHECK(index < InstanceTemplate()->InternalFieldCount());
	return reinterpret_cast<EmbedderData*>((this + 1) + sizeof(EmbedderData) * index);
}

CInfo::~CallbackInfo() {
	auto isolate = GetIsolate();
	if (!isolate->HasDestroy()) {
		auto ctx = JSC_CTX(isolate);
		Release(m_receiver_signature);
		Release(ctx, m_data);
		Release(ctx, m_name);
		if (m_front_callback_style == CInfo::FUNCTION_TEMPLATE) {
			Release((FunctionTemplate*)m_front_callback);
		}
	}
}

void CInfo::ObjectDestructor(JSObjectRef object) {
	auto priv = (PrivateData*)JSObjectGetPrivate(object);
	DCHECK(priv);
	priv->Destroy();
}

void CInfo::CallbackDestructor(JSObjectRef object) {
	auto priv = (CInfo*)JSObjectGetPrivate(object);
	DCHECK(priv);
	priv->Destroy();
}

void CInfo::WrapDestructor(JSObjectRef object) {
	auto ptr = (Wrap*)JSObjectGetPrivate(object);
	DCHECK(ptr);
	ptr->m_handle = nullptr;
	ptr->Destroy();
	// delete ptr;
}

template<typename T>
class JSCFunctionCallbackInfo: public FunctionCallbackInfo<T> {
 public:
	JSCFunctionCallbackInfo(internal::Object** implicit_args,
													internal::Object** values, int length)
	: FunctionCallbackInfo<T>(implicit_args, values, length) { }
};

template<typename T>
class JSCPropertyCallbackInfo: public PropertyCallbackInfo<T> {
 public:
	JSCPropertyCallbackInfo(internal::Object** values)
	: PropertyCallbackInfo<T>(values) { }
};

bool CInfo::VerificationSignature(JSObjectRef holder) {
	if (m_receiver_signature) { // Verification signature
		return m_receiver_signature->HasInstance(holder);
	}
	return true;
}

JSValueRef CInfo::Function(JSContextRef context, JSObjectRef func, JSObjectRef self,
													 size_t argc, const JSValueRef argv[], JSValueRef* exception) {
	auto cinfo = (CInfo*)JSObjectGetPrivate(func);
	return Function(cinfo, func, self, argc, argv, exception);
}

JSValueRef CInfo::Function(CInfo* cinfo, JSObjectRef func, JSObjectRef self,
													 size_t argc, const JSValueRef argv[], JSValueRef* exception) {
	DCHECK(cinfo);
	ENV(cinfo->GetIsolate());
	
	#define ok(...) exception); do { if (*exception) { return __VA_ARGS__ ; } }while(0
	
	JSObjectRef constructor = (JSObjectRef)JSObjectGetProperty(ctx, self, constructor_s, ok(0));
	JSValueRef __native__ = JSObjectGetProperty(ctx, constructor, __native___s, ok(0));
	bool is_constructor_call = JSValueIsStrictEqual(ctx, func, __native__);
	
	#undef ok
	
	JSObjectRef instance = nullptr;
	FunctionTemplate* ft = nullptr;
	FunctionCallback f;
	
	if (cinfo->FrontCallbackStyle() == CInfo::FUNCTION_TEMPLATE) {
		ft = reinterpret_cast<FunctionTemplate*>(cinfo->FrontCallback());
		if (is_constructor_call) {
			if (ft->Behavior() == ConstructorBehavior::kThrow) {
				*exception = isolate->NewError("Not allow constructor call."); return nullptr;
			}
			instance = ft->InstanceTemplate()->NewInstance(true);
			DCHECK(instance);
			JSObjectSetPrototype(ctx, instance, JSObjectGetPrototype(ctx, self)); // setting __proto__
		}
		f = ft->FrontCallback();
	} else {
		if (is_constructor_call) {
			if (cinfo->FrontCallbackStyle() != CInfo::FUNCTION_CONSTRUCTOR) {
				*exception = isolate->NewError("Not allow constructor call."); return nullptr;
			}
			instance = JSObjectMake(ctx, CInfo::ObjectClass, 0);
			DCHECK(instance);
			auto priv = PrivateData::New(isolate, instance, nullptr); // default template
			JSObjectSetPrototype(ctx, instance, JSObjectGetPrototype(ctx, self)); // setting __proto__
		}
		f = (FunctionCallback)cinfo->FrontCallback();
	}
	
	if (!is_constructor_call) {
		if (!cinfo->VerificationSignature(self)) {
			*exception = isolate->NewError("Cannot call function signature invalid.");
			return nullptr;
		}
	}
	
	Object* implicit_args[9] = {
		reinterpret_cast<Object*>(self),        // kHolderIndex
		reinterpret_cast<Object*>(isolate),     // kIsolateIndex
		(Object*)isolate->Undefined(),          // kReturnValueDefaultValueIndex
		(Object*)isolate->Undefined(),          // kReturnValueIndex
		(Object*)(cinfo->Data()),               // kDataIndex
		reinterpret_cast<Object*>(func),        // kCalleeIndex
		(Object*)isolate->Undefined(),          // kContextSaveIndex
		(Object*)isolate->Undefined(),          // kNewTargetIndex
		reinterpret_cast<Object*>(self),        // This
	};
	
	if (is_constructor_call) { // constructor call
		implicit_args[0] = reinterpret_cast<Object*>(instance);
		implicit_args[2] = reinterpret_cast<Object*>(instance);
		implicit_args[3] = reinterpret_cast<Object*>(instance);
		implicit_args[7] = reinterpret_cast<Object*>(instance);
		implicit_args[8] = reinterpret_cast<Object*>(instance);
	}
	JSCFunctionCallbackInfo<Value> info(implicit_args, (Object**)argv, int(argc));
	
	isolate->m_call_stack++;
	f(info);
	isolate->m_call_stack--;
	DCHECK(isolate->m_call_stack >= 0);
	*exception = isolate->m_exception; // throw error
	isolate->m_exception = nullptr;
	
	return reinterpret_cast<JSValueRef>(implicit_args[3]);
}

JSValueRef CInfo::Getter(JSContextRef context, JSObjectRef func, JSObjectRef self,
												 size_t argc, const JSValueRef argv[], JSValueRef* exception) {
	DCHECK(argc == 0);
	auto cinfo = (CInfo*)JSObjectGetPrivate(func);
	DCHECK(cinfo);
	ENV(cinfo->GetIsolate());
	
	if (cinfo->FrontCallbackStyle() == CInfo::FUNCTION) {
		return CInfo::Function(cinfo, func, self, 0, argv, exception);
	}
	if (!cinfo->VerificationSignature(self)) {
		*exception = isolate->NewError("Inaccessible Getter signature invalid.");
		return nullptr;
	}
	auto f = (AccessorGetterCallback)cinfo->FrontCallback();
	
	Object* args[JSCPropertyCallbackInfo<Value>::kArgsLength] = {
		nullptr,                                // kShouldThrowOnErrorIndex
		reinterpret_cast<Object*>(self),        // kHolderIndex
		reinterpret_cast<Object*>(isolate),     // kIsolateIndex
		(Object*)isolate->m_undefined,          // kReturnValueDefaultValueIndex
		(Object*)isolate->m_undefined,          // kReturnValueIndex
		(Object*)(cinfo->Data()),               // kDataIndex
		reinterpret_cast<Object*>(self),        // kThisIndex
	};
	JSCPropertyCallbackInfo<Value> info(args);
	
	isolate->m_call_stack++;
	f(Cast<String>(cinfo->Name()), info);
	isolate->m_call_stack--;
	DCHECK(isolate->m_call_stack >= 0);
	*exception = isolate->m_exception; // throw error
	isolate->m_exception = nullptr;
	
	return reinterpret_cast<JSValueRef>(args[4]);
}

JSValueRef CInfo::Setter(JSContextRef context, JSObjectRef func, JSObjectRef self,
												 size_t argc, const JSValueRef argv[], JSValueRef* exception) {
	DCHECK(argc == 1);
	auto cinfo = (CInfo*)JSObjectGetPrivate(func);
	DCHECK(cinfo);
	ENV(cinfo->GetIsolate());
	
	if (cinfo->FrontCallbackStyle() == CInfo::FUNCTION) {
		return CInfo::Function(cinfo, func, self, 1, argv, exception);
	}
	if (!cinfo->VerificationSignature(self)) {
		*exception = isolate->NewError("Inaccessible Setter signature invalid.");
		return nullptr;
	}
	auto f = (AccessorSetterCallback)cinfo->FrontCallback();
	
	Object* args[JSCPropertyCallbackInfo<void>::kArgsLength] = {
		nullptr,                                // kShouldThrowOnErrorIndex
		reinterpret_cast<Object*>(self),        // kHolderIndex
		reinterpret_cast<Object*>(isolate),     // kIsolateIndex
		(Object*)isolate->m_the_hole_value,     // kReturnValueDefaultValueIndex
		(Object*)isolate->m_the_hole_value,     // kReturnValueIndex
		(Object*)(cinfo->Data()),               // kDataIndex
		reinterpret_cast<Object*>(self),        // kThisIndex
	};
	
	JSCPropertyCallbackInfo<void> info(args);
	
	isolate->m_call_stack++;
	f(Cast<String>(cinfo->Name()), Cast(argv[0]), info);
	isolate->m_call_stack--;
	DCHECK(isolate->m_call_stack >= 0);
	*exception = isolate->m_exception; // throw error
	isolate->m_exception = nullptr;
	
	return isolate->m_undefined;
}

#define PropertyHandleBegin(Type, defaultValue)\
auto ptr = (PrivateData*)JSObjectGetPrivate(self);\
DCHECK(ptr);\
auto config = ptr->InstanceTemplate()->m_configuration;\
auto indexed_config = ptr->InstanceTemplate()->m_indexed_configuration;\
DCHECK(config || indexed_config);\
auto isolate = ptr->InstanceTemplate()->GetIsolate();\
\
Object* args[JSCPropertyCallbackInfo<Type>::kArgsLength] = {\
nullptr,                                /* kShouldThrowOnErrorIndex */\
reinterpret_cast<Object*>(self),        /* kHolderIndex */\
reinterpret_cast<Object*>(isolate),     /* kIsolateIndex */\
defaultValue,                           /* kReturnValueDefaultValueIndex */\
defaultValue,                           /* kReturnValueIndex */\
(Object*)(Back(config->data)),          /* kDataIndex */\
reinterpret_cast<Object*>(self),        /* kThisIndex */\
};\
JSCPropertyCallbackInfo<Type> info(args);\
uint32_t indexed;

#define PropertyHandleEnd(black)\
isolate->m_call_stack++; \
black; \
isolate->m_call_stack--; \
DCHECK(isolate->m_call_stack >= 0); \
*exception = isolate->m_exception; /* throw error */\
isolate->m_exception = nullptr;

JSValueRef CInfo::GetProperty(JSContextRef context, JSObjectRef self,
															JSStringRef name, JSValueRef* exception) {
	PropertyHandleBegin(Value, (Object*)isolate->m_undefined);
	
	if (config && config->getter) {
		if (indexed_config && indexed_config->getter &&
				Isolate::PropertyNameToIndexed(name, indexed)) {
			goto indexed;
		}
		PropertyHandleEnd(config->getter(Cast<v8::Name>(JSValueMakeString(context, name)), info));
		return reinterpret_cast<JSValueRef>(args[4]);
	}
	DCHECK(indexed_config && indexed_config->getter);
	if (Isolate::PropertyNameToIndexed(name, indexed)) {
	indexed:
		PropertyHandleEnd(indexed_config->getter(indexed, info));
		return reinterpret_cast<JSValueRef>(args[4]);
	}
	return nullptr;
}

bool CInfo::SetProperty(JSContextRef context, JSObjectRef self,
												JSStringRef name, JSValueRef value, JSValueRef* exception) {
	PropertyHandleBegin(Value, (Object*)isolate->m_the_hole_value);
	
	if (config && config->setter) {
		if (indexed_config && indexed_config->setter &&
				Isolate::PropertyNameToIndexed(name, indexed)) {
			goto indexed;
		}
		PropertyHandleEnd
		(
		 config->setter(Cast<v8::Name>(JSValueMakeString(context, name)), Cast(value), info)
		 );
		return true;
	}
	DCHECK(indexed_config && indexed_config->setter);
	if (Isolate::PropertyNameToIndexed(name, indexed)) {
	indexed:
		PropertyHandleEnd(indexed_config->setter(indexed, Cast(value), info));
		return true;
	}
	return false;
}

bool CInfo::HasProperty(JSContextRef context, JSObjectRef self, JSStringRef name) {
	PropertyHandleBegin(Integer, (Object*)isolate->m_false);
	
	if (config && config->query) {
		if (indexed_config && indexed_config->query &&
				Isolate::PropertyNameToIndexed(name, indexed)) {
			goto indexed;
		}
		isolate->m_call_stack++;
		config->query(Cast<v8::Name>(JSValueMakeString(context, name)), info);
		isolate->m_call_stack--;
		DCHECK(isolate->m_call_stack >= 0);
		return JSValueToBoolean(context, reinterpret_cast<JSValueRef>(args[4]));
	}
	DCHECK(indexed_config && indexed_config->query);
	if (Isolate::PropertyNameToIndexed(name, indexed)) {
	indexed:
		isolate->m_call_stack++;
		indexed_config->query(indexed, info);
		isolate->m_call_stack--;
		DCHECK(isolate->m_call_stack >= 0);
		return JSValueToBoolean(context, reinterpret_cast<JSValueRef>(args[4]));
	}
	return false;
}

bool CInfo::DeleteProperty(JSContextRef context, JSObjectRef self,
													 JSStringRef name, JSValueRef* exception) {
	PropertyHandleBegin(Boolean, (Object*)isolate->m_false);
	
	if (config && config->deleter) {
		if (indexed_config && indexed_config->deleter &&
				Isolate::PropertyNameToIndexed(name, indexed)) {
			goto indexed;
		}
		PropertyHandleEnd(config->deleter(Cast<v8::Name>(JSValueMakeString(context, name)), info));
		return JSValueToBoolean(context, reinterpret_cast<JSValueRef>(args[4]));
	}
	DCHECK(indexed_config && indexed_config->deleter);
	if (Isolate::PropertyNameToIndexed(name, indexed)) {
	indexed:
		PropertyHandleEnd(indexed_config->deleter(indexed, info));
		return JSValueToBoolean(context, reinterpret_cast<JSValueRef>(args[4]));
	}
	return false;
}

void CInfo::GetPropertyNames(JSContextRef ctx, JSObjectRef self,
														 JSPropertyNameAccumulatorRef names) {
	PropertyHandleBegin(Array, (Object*)isolate->m_undefined);
	
	isolate->m_call_stack++;
	if (config && config->enumerator) {
		config->enumerator(info);
	} else {
		indexed_config->enumerator(info);
	}
	isolate->m_call_stack--;
	DCHECK(isolate->m_call_stack >= 0);
	
	auto r = reinterpret_cast<JSValueRef>(args[4]);
	
	if ( JSValueIsArray(ctx, r) ) { // each array
		JSObjectRef arr = (JSObjectRef)r;
		JSValueRef ex = nullptr;
		auto jlength = JSObjectGetProperty(ctx, arr, length_s, OK());
		int length = JSValueToNumber(ctx, jlength, OK());
		DCHECK(length>=0);
		for (int i = 0; i < length; i++) {
			auto jitem = JSObjectGetPropertyAtIndex(ctx, arr, i, OK());
			JSCStringPtr str = JSValueToStringCopy(ctx, jitem, OK());
			JSPropertyNameAccumulatorAddName(names, *str);
		}
	}
}

JSClassRef CInfo::ObjectClass = nullptr;
JSClassRef CInfo::WrapHandleClass = nullptr;
JSClassRef CInfo::FunctionCallbackClass = nullptr;
JSClassRef CInfo::AccessorGetterCallbackClass = nullptr;
JSClassRef CInfo::AccessorSetterCallbackClass = nullptr;

static void Initialize() {
	if ( !CInfo::ObjectClass ) {
		
		JSClassDefinition def = kJSClassDefinitionEmpty;
		def.finalize = CInfo::ObjectDestructor;
		CInfo::ObjectClass = JSClassCreate(&def);
		DCHECK(CInfo::ObjectClass);
		JSClassRetain(CInfo::ObjectClass);
		
		def = kJSClassDefinitionEmpty;
		def.finalize = CInfo::WrapDestructor;
		CInfo::WrapHandleClass = JSClassCreate(&def);
		DCHECK(CInfo::WrapHandleClass);
		JSClassRetain(CInfo::WrapHandleClass);
		
		// Function.__native__()
		def = kJSClassDefinitionEmpty;
		def.finalize = CInfo::CallbackDestructor;
		def.callAsFunction = CInfo::Function;
		CInfo::FunctionCallbackClass = JSClassCreate(&def);
		DCHECK(CInfo::FunctionCallbackClass);
		JSClassRetain(CInfo::FunctionCallbackClass);
		
		def.callAsFunction = CInfo::Getter;
		CInfo::AccessorGetterCallbackClass = JSClassCreate(&def);
		DCHECK(CInfo::AccessorGetterCallbackClass);
		JSClassRetain(CInfo::AccessorGetterCallbackClass);
		
		def.callAsFunction = CInfo::Setter;
		CInfo::AccessorSetterCallbackClass = JSClassCreate(&def);
		DCHECK(CInfo::AccessorSetterCallbackClass);
		JSClassRetain(CInfo::AccessorSetterCallbackClass);
	}
}

static void Dispose() {
	if ( CInfo::ObjectClass ) {
		JSClassRelease(CInfo::ObjectClass);
		CInfo::ObjectClass = nullptr;
		JSClassRelease(CInfo::WrapHandleClass);
		CInfo::WrapHandleClass = nullptr;
		JSClassRelease(CInfo::FunctionCallbackClass);
		CInfo::FunctionCallbackClass = nullptr;
		JSClassRelease(CInfo::AccessorGetterCallbackClass);
		CInfo::AccessorGetterCallbackClass = nullptr;
		JSClassRelease(CInfo::AccessorSetterCallbackClass);
		CInfo::AccessorSetterCallbackClass = nullptr;
	}
}

v8_ns_end

class Utils {
 public:
	static void ReportApiFailure(const char* location, const char* message) {
		i::Isolate* isolate = ISOLATE();
		isolate->Terminated();
		FatalErrorCallback callback = isolate->ExceptionBehavior();
		if (callback == nullptr) {
			fatal("\n#\n# Fatal error in %s\n# %s\n#\n\n", location, message);
		} else {
			callback(location, message);
		}
	}
	
	static inline bool ApiCheck(bool condition,
															const char* location,
															const char* message) {
		if (!condition) ReportApiFailure(location, message);
		return condition;
	}
};

bool v8::V8::Initialize() {
	i::Initialize(); return true;
}

bool v8::V8::Dispose() {
	i::Dispose(); return true;
}

// --- V 8 ---

void i::Internals::CheckInitializedImpl(v8::Isolate* isolate) {
}

void i::Internals::SetEmbedderData(v8::Isolate* isolate, uint32_t slot, void* data) {
	ISOLATE(isolate)->SetEmbedderData(slot, data);
}

void* i::Internals::GetEmbedderData(const v8::Isolate* isolate, uint32_t slot) {
	return ISOLATE(isolate)->GetEmbedderData(slot);
}

bool i::Internals::IsWeak(internal::Object* o) {
	auto priv = i::PrivateData::Private(o);
	if (priv) {
		return priv->IsWeak();
	}
	return true;
}

bool i::Internals::IsNearDeath(internal::Object* o) {
	auto priv = i::PrivateData::Private(o);
	if (priv) {
		return priv->IsNearDeath();
	}
	return false;
}

void i::Internals::MakeIndependent(internal::Object* o) {
	if (!i::Wrap::IsWrap(o)) {
		auto isolate = ISOLATE();
		if (!isolate->HasDestroy()) {
			auto ctx = JSC_CTX(isolate);
			auto value = reinterpret_cast<JSObjectRef>(o);
			if (JSValueIsObject(ctx, value)) {
				if (!JSObjectIsFunction(ctx, value)) {
					i::PrivateData::EnsurePrivate(isolate, value)->MarkIndependent();
				}
			}
		}
	}
}

void i::Internals::MarkActive(internal::Object* o) {}

i::Object** V8::GlobalizeReference(i::Isolate* isolate, i::Object** obj) {
	auto o = reinterpret_cast<i::Object*>(obj);
	if (!isolate->HasDestroy()) {
		if (i::Wrap::IsWrap(o)) {
			static_cast<i::Wrap*>(o)->retain();
		} else {
			auto ctx = JSC_CTX(isolate);
			auto value = reinterpret_cast<JSObjectRef>(o);
			if (JSValueIsObject(ctx, value) &&
					!JSObjectIsFunction(ctx, value)) {
				i::PrivateData::EnsurePrivate(isolate, value)->retain();
			} else {
				JSValueProtect(ctx, value);
			}
		}
	}
	return obj;
}

void V8::DisposeGlobal(i::Object** location) {
	auto o = reinterpret_cast<i::Object*>(location);
	if (i::Wrap::IsWrap(o)) {
		if (!static_cast<i::Wrap*>(o)->GetIsolate()->HasDestroy())
			static_cast<i::Wrap*>(o)->release();
	} else {
		auto isolate = ISOLATE();
		if (!isolate->HasDestroy()) {
			auto ctx = JSC_CTX(isolate);
			auto value = reinterpret_cast<JSObjectRef>(o);
			if (JSValueIsObject(ctx, value) &&
					!JSObjectIsFunction(ctx, value)) {
				i::PrivateData::EnsurePrivate(isolate, value)->release();
			} else {
				JSValueProtect(ctx, value);
			}
		}
	}
}

i::Object** V8::CopyPersistent(i::Object** obj) {
	return GlobalizeReference(ISOLATE(), obj);
}

void V8::RegisterExternallyReferencedObject(i::Object** object, i::Isolate* isolate) {
	UNIMPLEMENTED();
}

void V8::MakeWeak(i::Object** location, void* parameter,
									int embedder_field_index1, int embedder_field_index2,
									WeakCallbackInfo<void>::Callback weak_callback) {
	UNIMPLEMENTED();
}

void V8::MakeWeak(i::Object** location, void* parameter,
									WeakCallbackInfo<void>::Callback weak_callback,
									WeakCallbackType type) {
	auto o = reinterpret_cast<i::Object*>(location);
	if (i::Wrap::IsWrap(o)) {
		Utils::ReportApiFailure("v8::V8::MakeWeak", "Could not mark as weak");
	} else {
		auto isolate = ISOLATE();
		if (isolate->HasDestroy()) {
			/*
			 * 现在正直释放中不能访问JSObjectGetPrivate(),
			 * 如果这个句柄之前设置过弱回调,等到js句柄释放回调时可能会异常甚至程序奔溃
			 * 所以不要重复设置或替换弱回调,
			 */
			void* embedder_fields[kEmbedderFieldsInWeakCallback] = { nullptr, nullptr };
			WeakCallbackInfo<void> info(reinterpret_cast<v8::Isolate*>(isolate),
																	parameter, embedder_fields,
																	&weak_callback);
			weak_callback(info);
		} else {
			auto ctx = JSC_CTX(isolate);
			auto value = reinterpret_cast<JSObjectRef>(o);
			if (JSValueIsObject(ctx, value) &&
					!JSObjectIsFunction(ctx, value)) {
				i::PrivateData::EnsurePrivate(isolate, value)->SetWeak(parameter, weak_callback, type);
			} else {
				Utils::ReportApiFailure("v8::V8::MakeWeak", "Could not mark as weak");
			}
		}
	}
}

void V8::MakeWeak(i::Object*** location_addr) {
	i::Object** o = *location_addr;
	MakeWeak(o, nullptr, i::PrivateData::DefaultWeakCallback,
					 WeakCallbackType::kParameter);
}

void* V8::ClearWeak(i::Object** location) {
	auto o = reinterpret_cast<i::Object*>(location);
	if (!i::Wrap::IsWrap(o)) {
		auto isolate = ISOLATE();
		if (!isolate->HasDestroy()) {
			auto ctx = JSC_CTX(isolate);
			auto value = reinterpret_cast<JSObjectRef>(o);
			if (JSValueIsObject(ctx, value) &&
					!JSObjectIsFunction(ctx, value)) {
				i::PrivateData::EnsurePrivate(isolate, value)->ClearWeak();
			}
		}
	}
}

Value* V8::Eternalize(Isolate* v8_isolate, Value* value) {
	auto isolate = ISOLATE(v8_isolate);
	if (!isolate->HasDestroy()) {
		auto o = reinterpret_cast<i::Object*>(value);
		if (i::Wrap::IsWrap(o)) {
			reinterpret_cast<i::Wrap*>(o)->retain();
		} else {
			auto ctx = JSC_CTX(v8_isolate);
			auto value = reinterpret_cast<JSObjectRef>(o);
			if (JSValueIsObject(ctx, value) &&
					!JSObjectIsFunction(ctx, value)) {
				 i::PrivateData::EnsurePrivate(isolate, value)->Eternalize();
			} else {
				JSValueProtect(ctx, value);
			}
		}
	}
	return value;
}

const char* V8::GetVersion() {
	return i::v8_version_string;
}

void V8::FromJustIsNothing() {
	Utils::ReportApiFailure("v8::FromJust", "Maybe value is Nothing.");
}
void V8::ToLocalEmpty() {
	Utils::ReportApiFailure("v8::ToLocalChecked", "Empty MaybeLocal.");
}
void V8::InternalFieldOutOfBounds(int index) {
	Utils::ApiCheck(0 <= index && index < kInternalFieldsInWeakCallback,
									"WeakCallbackInfo::GetInternalField",
									"Internal field out of bounds.");
}

// --- H a n d l e s ---

HandleScope::HandleScope(Isolate* v8_isolate)
: isolate_(nullptr)
, prev_next_(nullptr), prev_limit_(nullptr) {
	Initialize(v8_isolate);
}

HandleScope::~HandleScope() {
	if (isolate_->CallStackCount() == 0) { // exec handle clear
		std::lock_guard<std::mutex> scope(isolate_->m_garbage_handle_mutex);
		if (isolate_->m_garbage_handle.size()) {
			std::vector<i::PrivateData*> handles(std::move(isolate_->m_garbage_handle));
			for (auto& i: handles) {
				delete i;
			}
		}
	}
	if (isolate_->m_microtask_policy != MicrotasksPolicy::kExplicit) {
		isolate_->RunMicrotasks(false);
	}
	
	auto ctx = isolate_->jscc();
	auto vec = reinterpret_cast<std::vector<JSValueRef>*>(prev_limit_);
	for (auto& i: (*vec)) {
		JSValueUnprotect(ctx, i);
	}
	isolate_->m_handle_scope = reinterpret_cast<HandleScope*>(prev_next_);
}

int HandleScope::NumberOfHandles(Isolate* v8_isolate) {
	auto isolate = ISOLATE(v8_isolate);
	auto handle_scope = isolate->m_handle_scope;
	DCHECK(handle_scope);
	int r = 0;
	while(handle_scope) {
		r += reinterpret_cast<std::vector<JSValueRef>*>(handle_scope->prev_limit_)->size();
		handle_scope = reinterpret_cast<HandleScope*>(handle_scope->prev_next_);
	}
	return r;
}

void HandleScope::Initialize(Isolate* v8_isolate) {
	DCHECK(v8_isolate);
	auto isolate = ISOLATE(v8_isolate);
	isolate_ = isolate;
	prev_next_ = reinterpret_cast<internal::Object**>(isolate->m_handle_scope);
	prev_limit_ = reinterpret_cast<internal::Object**>(new std::vector<JSValueRef>());
	isolate->m_handle_scope = this;
}

static i::Object** ScopeRetain(HandleScope* scope, i::Object* value) {
	DCHECK(value);
	if (i::Wrap::IsWrap(value)) {
		i::Isolate::ScopeRetain(scope, reinterpret_cast<i::Wrap*>(value)->Handle());
	} else {
		i::Isolate::ScopeRetain(scope, reinterpret_cast<JSValueRef>(value));
	}
	return reinterpret_cast<i::Object**>(value);
}

internal::Object** HandleScope::CreateHandle(
	i::Isolate* v8_isolate, internal::Object* value) {
	return ScopeRetain(ISOLATE(v8_isolate)->m_handle_scope, value);
}

internal::Object** HandleScope::CreateHandle(
	i::HeapObject* heap_object, internal::Object* value) {
	return reinterpret_cast<internal::Object**>(value);
}

EscapableHandleScope::EscapableHandleScope(Isolate* v8_isolate)
: escape_slot_(nullptr) {
	CHECK(reinterpret_cast<i::Isolate*>(v8_isolate)->m_handle_scope);
	Initialize(v8_isolate);
}

internal::Object** EscapableHandleScope::Escape(i::Object** escape_value) {
	struct HS {
		i::Isolate* isolate;
		HandleScope* priv;
		void* value;
	};
	auto hs = reinterpret_cast<HS*>(this);;
	return ScopeRetain(hs->priv, reinterpret_cast<i::Object*>(escape_value));
}

struct WrapHandleScope {
	HandleScope scope;
	inline WrapHandleScope(Isolate* isolate): scope(isolate) {}
};

SealHandleScope::SealHandleScope(Isolate* isolate)
: isolate_(reinterpret_cast<internal::Isolate*>(isolate))
, prev_limit_(nullptr), prev_sealed_level_(0) {
	prev_limit_ = reinterpret_cast<internal::Object**>(new WrapHandleScope(isolate));
}

SealHandleScope::~SealHandleScope() {
	reinterpret_cast<WrapHandleScope*>(prev_limit_)->~WrapHandleScope();
}
void* SealHandleScope::operator new(size_t) { UNREACHABLE(); }
void SealHandleScope::operator delete(void*, size_t) { UNREACHABLE(); }

#include "jsc-v8-serializer.cc.inl"
#include "jsc-v8-script.cc.inl"
#include "jsc-v8-exceptions.cc.inl"
#include "jsc-v8-property-descriptor.cc.inl"
#include "jsc-v8-check.cc.inl"
#include "jsc-v8-value.cc.inl"
#include "jsc-v8-object.cc.inl"
#include "jsc-v8-function.cc.inl"
#include "jsc-v8-string.cc.inl"
#include "jsc-v8-array-buffer.cc.inl"
#include "jsc-v8-promise.cc.inl"
#include "jsc-v8-unimplemented.cc.inl"

v8_ns_end
