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

v8_ns(internal)

static pthread_key_t  specific_key;
static pthread_once_t specific_init_done = PTHREAD_ONCE_INIT;
static std::string unknown("[Unknown]");
static std::mutex* global_isolate_mutex = nullptr;
static int global_isolate_count = 0;
static Isolate* global_isolate = nullptr;
static int id = 0;
static const char* v8_version_string = V8_VERSION_STRING;

struct ThreadSpecific {
	Isolate* isolate;
	
	static void init() {
		int err = pthread_key_create(&specific_key, destructor);
		global_isolate_mutex = new std::mutex;
	}
	static void destructor(void * ptr) {
		delete reinterpret_cast<ThreadSpecific*>(ptr);
	}
	static ThreadSpecific* get_data() {
		auto ptr = reinterpret_cast<ThreadSpecific*>(pthread_getspecific(specific_key));
		if ( !ptr ) {
			ptr = new ThreadSpecific();
			memset(ptr, 0, sizeof(ThreadSpecific));
			pthread_setspecific(specific_key, ptr);
		}
		return ptr;
	}
};

// -------------------------------------------------------------

static const int64_t DECIMAL_MULTIPLE[] = {
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000,
};

struct JSCStringTraits: public DefaultTraits {
	inline static bool Retain(JSStringRef str) {
		if ( str ) return JSStringRetain(str); else return false;
	}
	inline static void Release(JSStringRef str) {
		if ( str ) JSStringRelease(str);
	}
};

struct JSCPropertyNameArrayTraits: public DefaultTraits {
	inline static bool Retain(JSPropertyNameArrayRef arr) {
		if ( arr ) return JSPropertyNameArrayRetain(arr); else return false;
	}
	inline static void Release(JSPropertyNameArrayRef arr) {
		if ( arr ) JSPropertyNameArrayRelease(arr);
	}
};

typedef UniquePtr<OpaqueJSString, JSCStringTraits> JSCStringPtr;
typedef UniquePtr<OpaqueJSPropertyNameArray, JSCPropertyNameArrayTraits> JSCPropertyNameArrayPtr;

template<class T = Value>
inline Local<T> Cast(JSValueRef o) {
	return *reinterpret_cast<Local<T>*>(&o);
}

template<class T = Value>
inline Local<T> Cast(const Object* o) {
	return *reinterpret_cast<Local<T>*>(const_cast<Object**>(&o));
}

template<class T = JSValueRef, class S>
inline T Back(Local<S> o) {
	return reinterpret_cast<T>(*o);
}

template<class T = JSValueRef>
inline T Back(const Value* v) {
	return reinterpret_cast<T>(const_cast<Value*>(v));
}

template<class T>
static T* Retain(T* t) {
	if (t) {
		t->retain(); return t;
	}
	return nullptr;
}

static JSValueRef Retain(JSContextRef ctx, JSValueRef t) {
	if (t) {
		JSValueProtect(ctx, t);
		return t;
	}
	return nullptr;
}

inline JSValueRef ScopeRetain(Isolate* isolate, JSValueRef value);

template<class T>
static T* Release(T* t) {
	if (t) {
		t->release(); return t;
	}
	return nullptr;
}

static JSValueRef Release(JSContextRef ctx, JSValueRef t) {
	if (t) {
		JSValueUnprotect(ctx, t);
		return t;
	}
	return nullptr;
}

static void CheckException(JSContextRef ctx, JSValueRef exception);

class Microtask {
 public:
	enum CallbackStyle {
		Callback,
		JSFunction,
	};
	Microtask(const Microtask& micro) = delete;
	inline Microtask(Microtask&& micro)
	: m_style(micro.m_style)
	, m_microtask(micro.m_microtask)
	, m_data(micro.m_data) {
		micro.m_microtask = nullptr;
		micro.m_data = nullptr;
	}
	inline Microtask(MicrotaskCallback microtask, void* data = nullptr)
	: m_style(Callback)
	, m_microtask((void*)microtask), m_data(data) {
	}
	inline Microtask(Isolate* isolate, Local<Function> microtask);
	inline ~Microtask() { Reset(); }
	inline void Reset();
	inline bool Run(Isolate* isolate);
 private:
	CallbackStyle m_style = Callback;
	void* m_microtask = nullptr;
	void* m_data = nullptr;
};

class IsolateData {
 public:
	void Initialize(JSGlobalContextRef ctx) {
#define __ok() &ex); do { CheckException(ctx, ex); }while(0
#define SET_ARRTIBUTES(name) m_##name = (JSObjectRef) \
JSObjectGetProperty(ctx, exports, i::name##_s, __ok()); \
JSValueProtect(ctx, m_##name);
		m_ctx = ctx;
		JSValueRef ex = nullptr;
		auto exports = run_native_script(ctx, (const char*)
																		 native_js::JSC_native_js_code_jsc_v8_isolate_,
																		 "[jsc-v8-isolate.js]");
		JS_ISOLATE_DATA(SET_ARRTIBUTES)
	}
	void Destroy() {
		#define DEL_ARRTIBUTES(name) JSValueUnprotect(m_ctx, m_##name);
		JS_ISOLATE_DATA(DEL_ARRTIBUTES)
	}
	static JSObjectRef run_native_script(JSGlobalContextRef ctx,
																			 const char* script, const char* name) {
		JSValueRef ex = nullptr;
		JSObjectRef global = (JSObjectRef)JSContextGetGlobalObject(ctx);
		JSCStringPtr jsc_v8_js = JSStringCreateWithUTF8CString(name);
		JSCStringPtr script2 = JSStringCreateWithUTF8CString(script);
		auto fn = (JSObjectRef)JSEvaluateScript(ctx, *script2, 0, *jsc_v8_js, 1, __ok());
		auto exports = JSObjectMake(ctx, 0, 0);
		JSValueRef argv[2] = { exports, global };
		JSObjectCallAsFunction(ctx, fn, 0, 2, argv, __ok());
		return exports;
	}
	
 private:
	JSGlobalContextRef m_ctx;
#define DEF_ARRTIBUTES(name) \
public: JSObjectRef name() { return m_##name; } \
private: JSObjectRef m_##name;
	JS_ISOLATE_DATA(DEF_ARRTIBUTES)
};

class ContextData {
 public:
	void Initialize(JSGlobalContextRef ctx) {
		m_ctx = ctx;
		JSValueRef ex = nullptr;
		auto exports = IsolateData::run_native_script(ctx, (const char*)
																									native_js::JSC_native_js_code_jsc_v8_context_,
																									"[jsc-v8-context.js]");
		JS_CONTEXT_DATA(SET_ARRTIBUTES)
	}
	void Destroy() {
		JS_CONTEXT_DATA(DEL_ARRTIBUTES)
	}
	void Reset() {
		#define RESET_ARRTIBUTES(name) JSValueUnprotect(m_ctx, m_##name);
		JS_CONTEXT_DATA(RESET_ARRTIBUTES)
	}
 private:
	JSGlobalContextRef m_ctx;
	JS_CONTEXT_DATA(DEF_ARRTIBUTES)
#undef SET_ARRTIBUTES
#undef DEF_ARRTIBUTES
#undef DEL_ARRTIBUTES
#undef RESET_ARRTIBUTES
};

/**
 * @class i::Isloate
 */
class Isolate {
 private:
	
	void add_global_isolate() {
		ThreadSpecific::get_data()->isolate = this;
		if (global_isolate_count) {
			global_isolate = nullptr;
		} else {
			global_isolate = this;
		}
		global_isolate_count++;
	}
	
	void clear_global_isolate() {
		ThreadSpecific::get_data()->isolate = nullptr;
		if (global_isolate == this) {
			global_isolate = nullptr;
		}
		global_isolate_count--;
	}
	
	Isolate() {
		pthread_once(&specific_init_done, ThreadSpecific::init);
		std::lock_guard<std::mutex> scope(*global_isolate_mutex);
		CHECK(ThreadSpecific::get_data()->isolate == nullptr);
		add_global_isolate();
		m_group = JSContextGroupCreate();
		m_default_jsc_ctx = JSGlobalContextCreateInGroup(m_group, nullptr);
		ScopeClear clear([this]() {
			JSGlobalContextRelease(m_default_jsc_ctx);
			JSContextGroupRelease(m_group);
			clear_global_isolate();
		});
		Initialize();
		clear.cancel();
	}
	
	void Initialize() {
		#define ok() &ex); do { CHECK(ex==nullptr); }while(0
		
		m_thread_id = std::this_thread::get_id();
		m_microtask_policy = MicrotasksPolicy::kAuto;
		auto ctx = m_default_jsc_ctx;
		JSObjectRef global = (JSObjectRef)JSContextGetGlobalObject(ctx);
		JSValueRef ex = nullptr;
		Isolate* isolate = this;
		
		v8::HandleScope scope(reinterpret_cast<v8::Isolate*>(this));
		
		m_undefined = JSValueMakeUndefined(ctx); JSValueProtect(ctx, m_undefined);
		m_null = JSValueMakeNull(ctx); JSValueProtect(ctx, m_null);
		m_true = JSValueMakeBoolean(ctx, true); JSValueProtect(ctx, m_true);
		m_false = JSValueMakeBoolean(ctx, false); JSValueProtect(ctx, m_false);
		m_empty_s = JSValueMakeString(ctx, empty_s); JSValueProtect(ctx, m_empty_s);
		
		m_isolate_data.Initialize(ctx);
		m_default_context_data.Initialize(ctx);
		InitializeTemplate();
		
		// LOG("%d", v8::HandleScope::NumberOfHandles(reinterpret_cast<v8::Isolate*>(this)));
		#undef ok
	}
	
	~Isolate() {
		std::lock_guard<std::mutex> scope(*global_isolate_mutex);
		CHECK(!m_cur_ctx);
		m_has_terminated = true;
		m_exception = nullptr;
		auto ctx = m_default_jsc_ctx;
		JSValueUnprotect(ctx, m_undefined);
		JSValueUnprotect(ctx, m_null);
		JSValueUnprotect(ctx, m_true);
		JSValueUnprotect(ctx, m_false);
		JSValueUnprotect(ctx, m_empty_s);
		if (m_message_listener_data) {
			JSValueUnprotect(ctx, m_message_listener_data);
		}
		DisposeTemplate();
		m_isolate_data.Destroy();
		m_default_context_data.Destroy();
		m_has_destroy = true;
		JSGlobalContextRelease(m_default_jsc_ctx);
		JSContextGroupRelease(m_group);
		clear_global_isolate();
	}

	void InitializeTemplate();
	void DisposeTemplate();
	
 public:
	inline static v8::Isolate* New(const v8::Isolate::CreateParams& params) {
		auto isolate = (i::Isolate*)malloc(sizeof(i::Isolate));
		memset(isolate, 0, sizeof(i::Isolate));
		new(isolate) Isolate();
		return reinterpret_cast<v8::Isolate*>(isolate);
	}
	
	inline JSGlobalContextRef jscc() const;
	
	Local<String> New(const char* str, bool ascii = false) {
		if (ascii) {
			return String::NewFromOneByte(reinterpret_cast<v8::Isolate*>(this), (const uint8_t*)str);
		} else {
			return String::NewFromUtf8(reinterpret_cast<v8::Isolate*>(this), str);
		}
	}
	
	inline JSObjectRef NewError(const char* message) {
		auto str = JSValueMakeString(jscc(), JSStringCreateWithUTF8CString(message));
		auto error = JSObjectCallAsConstructor(jscc(), Error(), 1, &str, 0);
		DCHECK(error);
		return error;
	}
	
	void ThrowException(JSValueRef exception, i::Message* message) {
		CHECK(m_exception == nullptr ||
					exception == m_exception, "Throw too many exceptions");
		if (!JSValueIsObject(jscc(), exception)) {
			exception = JSObjectCallAsConstructor(jscc(), Error(), 1, &exception, 0);
			DCHECK(exception);
		}
		m_exception = exception;
		
		if (m_try_catch || m_call_stack == 0) {
			i::Message* m = message;
			if (!m) {
				auto msg = Exception::CreateMessage(reinterpret_cast<v8::Isolate*>(this),
																						i::Cast(exception));
				m = reinterpret_cast<i::Message*>(*msg);
			}
			if (m_try_catch) {
				m_try_catch->message_obj_ = m;
			} else { // top stack throw
				auto data = m_message_listener_data ? m_message_listener_data : exception;
				if (m_message_listener) {
					m_message_listener(Cast<v8::Message>(reinterpret_cast<class Object*>(m)),
														 Cast(data));
				} else {
					PrintMessage(m);
				}
			}
		}
	}
	
	void ThrowException(JSValueRef exception) {
		ThrowException(exception, nullptr);
	}
	
	static inline void PrintMessage(i::Message* message);
	
	bool AddMessageListener(MessageCallback that, Local<Value> data) {
		m_message_listener = that;
		if (m_message_listener_data) {
			JSValueUnprotect(jscc(), m_message_listener_data);
			m_message_listener_data = nullptr;
		}
		if (!data.IsEmpty()) {
			m_message_listener_data = Back(data);
			JSValueProtect(jscc(), m_message_listener_data);
		}
		return true;
	}
	
	static JSValueRef ScopeRetain(HandleScope* scope, JSValueRef value) {
		struct HS {
			Isolate* isolate;
			HandleScope* prev;
			std::vector<JSValueRef>* values;
		};
		auto hs = reinterpret_cast<HS*>(scope);
		if (!hs->isolate->HasDestroy()) {
			JSValueProtect(hs->isolate->jscc(), value);
			hs->values->push_back(value);
		}
		return value;
	}
	
	inline JSValueRef ScopeRetain(JSValueRef value) {
		return ScopeRetain(m_handle_scope, value);
	}
	
	inline static Isolate* Current() {
		if (global_isolate)
			return global_isolate;
		auto isolate = ThreadSpecific::get_data()->isolate;
		DCHECK(isolate);
		return isolate;
	}
	inline static Isolate* Current(const v8::Isolate* isolate) {
		DCHECK(isolate);
		return reinterpret_cast<Isolate*>(const_cast<v8::Isolate*>(isolate));
	}
	inline static Isolate* Current(Isolate* isolate) {
		DCHECK(isolate);
		return isolate;
	}
	inline static JSStringRef ToJSString(Isolate* isolate, Local<Value> value) {
		return ToJSString(isolate, i::Back(value));
	}
	inline static JSStringRef ToJSString(Isolate* isolate, JSValueRef value) {
		JSStringRef r = JSValueToStringCopy(JSC_CTX(isolate), value, 0);
		CHECK(r); return r;
	}
	inline static std::string ToSTDString(Isolate* isolate, Local<Value> value) {
		v8::String::Utf8Value utf8(value);
		return *utf8;
	}
	inline static std::string ToSTDString(Isolate* isolate, JSValueRef value) {
		return ToSTDString(isolate, Cast(value));
	}
	inline static std::string ToSTDString(JSContextRef ctx, JSValueRef value) {
		i::JSCStringPtr s = JSValueToStringCopy(ctx, value, 0);
		return ToSTDString(*s);
	}
	static std::string ToSTDString(JSStringRef value) {
		size_t bufferSize = JSStringGetMaximumUTF8CStringSize(value);
		char* str = (char*)malloc(bufferSize);
		JSStringGetUTF8CString(value, str, bufferSize);
		std::string r = str;
		free(str);
		return r;
	}
	inline static JSStringRef IndexedToPropertyName(Isolate* isolate, uint32_t index) {
		char s[11];
		sprintf(s, "%u", index);
		JSStringRef r = JSStringCreateWithUTF8CString(s);
		CHECK(r); return r;
	}
	
	static bool PropertyNameToIndexed(JSStringRef propertyName, uint32_t& out) {
		out = 0;
		uint len = (uint)JSStringGetLength(propertyName);
		if ( len > 10 ) { // 32无符号最长表示为`4294967295`
			return false;
		}
		const JSChar* chars = JSStringGetCharactersPtr(propertyName);
		for ( int i = 0; i < len; i++ ) {
			JSChar num = chars[i] - '0';
			if ( 0 <= num && num <= 9 ) {
				out += (num * DECIMAL_MULTIPLE[len - 1 - i]);
			} else {
				return false; // not int number
			}
		}
		return true;
	}
	
	inline void Dispose() {
		delete this;
	}
	inline void SetEmbedderData(uint32_t slot, void* data) {
		CHECK(slot < Internals::kNumIsolateDataSlots);
		m_data[slot] = data;
	}
	inline void* GetEmbedderData(uint32_t slot) const {
		CHECK(slot < Internals::kNumIsolateDataSlots);
		return m_data[slot];
	}
	inline Local<v8::Context> GetCurrentContext() {
		return *reinterpret_cast<Local<v8::Context>*>(&m_cur_ctx);
	}
	inline Context* GetContext() { return m_cur_ctx; }
	
	void RunMicrotasks(bool Explicit);
	
	inline JSValueRef Undefined() { return m_undefined; }
	inline JSValueRef TheHoleValue() { return m_the_hole_value; }
	inline JSValueRef Null() { return m_null; }
	inline JSValueRef True() { return m_true; }
	inline JSValueRef False() { return m_true; }
	inline JSValueRef Empty() { return m_empty_s; }
	inline JSObjectRef External() { return m_External; }
	inline ObjectTemplate* ExternalTemplate() { return m_external_template; }
	inline ObjectTemplate* DefaultPlaceholderTemplate() { return m_default_placeholder_template; }
	inline ObjectTemplate* CInfoPlaceholderTemplate() { return m_cinfo_placeholder_template; }
	inline FatalErrorCallback ExceptionBehavior() const { return m_exception_behavior; }
	inline bool HasTerminated() const { return m_has_terminated; }
	inline bool HasDestroy() const { return m_has_destroy; }
	inline std::thread::id ThreadID() const { return m_thread_id; }
	inline int CallStackCount() const { return m_call_stack; }
	void Terminated() {}
	void CancelTerminated() {}
	
#define DEF_ARRTIBUTES(NAME) \
	public: JSObjectRef NAME() { return m_isolate_data.NAME(); }
	JS_ISOLATE_DATA(DEF_ARRTIBUTES)
#undef DEF_ARRTIBUTES
	
#define DEF_ARRTIBUTES(NAME) \
public: JSObjectRef NAME();
JS_CONTEXT_DATA(DEF_ARRTIBUTES)
#undef DEF_ARRTIBUTES

 private:
	JSContextGroupRef m_group;
	JSGlobalContextRef m_default_jsc_ctx;
	JSValueRef  m_exception;
	Context*  m_cur_ctx;
	JSValueRef m_undefined;     // kUndefinedValueRootIndex = 4;
	JSValueRef m_the_hole_value;// kTheHoleValueRootIndex = 5;
	JSValueRef m_null;          // kNullValueRootIndex = 6;
	JSValueRef m_true;          // kTrueValueRootIndex = 7;
	JSValueRef m_false;         // kFalseValueRootIndex = 8;
	JSValueRef m_empty_s;       // kEmptyStringRootIndex = 9;
	JSObjectRef m_External;
	ObjectTemplate* m_external_template;
	ObjectTemplate* m_default_placeholder_template;
	ObjectTemplate* m_cinfo_placeholder_template;
	TryCatch* m_try_catch;
	HandleScope* m_handle_scope;
	MessageCallback m_message_listener;
	JSValueRef m_message_listener_data;
	FatalErrorCallback m_exception_behavior;
	v8::Isolate::AbortOnUncaughtExceptionCallback m_abort_on_uncaught_exception_callback;
	std::map<size_t, MicrotasksCompletedCallback> m_microtasks_completed_callback;
	std::vector<PrivateData*> m_garbage_handle;
	std::mutex m_garbage_handle_mutex;
	std::thread::id m_thread_id;
	std::vector<Microtask> m_microtask;
	MicrotasksPolicy m_microtask_policy;
	IsolateData m_isolate_data;
	ContextData m_default_context_data;
	bool m_has_terminated;
	bool m_has_destroy;
	int m_call_stack;
	void* m_data[Internals::kNumIsolateDataSlots];
	friend class Context;
	friend class v8::Isolate;
	friend class v8::Context;
	friend class v8::TryCatch;
	friend struct CallbackInfo;
	friend class v8::Utils;
	friend class v8::Private;
	friend class v8::Symbol;
	friend class PrivateData;
	friend class v8::HandleScope;
	friend class v8::EscapableHandleScope;
};

JSValueRef ScopeRetain(Isolate* isolate, JSValueRef value) {
	return isolate->ScopeRetain(value);
}

void CheckException(JSContextRef ctx, JSValueRef exception) {
	if (exception) { // err
		JSValueRef line = JSObjectGetProperty(ctx, (JSObjectRef)exception, line_s, 0);
		JSValueRef column = JSObjectGetProperty(ctx, (JSObjectRef)exception, column_s, 0);
		JSValueRef message = JSObjectGetProperty(ctx, (JSObjectRef)exception, message_s, 0);
		JSValueRef stack = JSObjectGetProperty(ctx, (JSObjectRef)exception, stack_s, 0);
		double l = JSValueToNumber(ctx, line, 0);
		double c = JSValueToNumber(ctx, column, 0);
		std::string m = Isolate::ToSTDString(ctx, message);
		std::string s = Isolate::ToSTDString(ctx, stack);
		v8::fatal("", l, "", "%s\n\n%s", m.c_str(), s.c_str());
	}
}

Microtask::Microtask(Isolate* isolate, Local<Function> microtask)
: m_style(JSFunction)
, m_microtask(*microtask)
, m_data(isolate) {
	DCHECK(m_microtask);
	JSValueProtect(JSC_CTX(isolate), reinterpret_cast<JSValueRef>(m_microtask));
}

void Microtask::Reset() {
	if (m_microtask) {
		if (m_style == JSFunction) {
			DCHECK(m_data);
			auto isolate = reinterpret_cast<Isolate*>(m_data);
			JSValueUnprotect(JSC_CTX(isolate), reinterpret_cast<JSValueRef>(m_microtask));
		}
		m_microtask = nullptr;
	}
}

bool Microtask::Run(Isolate* iso) {
	if (m_style == Callback) {
		reinterpret_cast<MicrotaskCallback>(m_microtask)(m_data);
	} else {
		ENV(iso);
		auto f = reinterpret_cast<JSObjectRef>(m_microtask);
		JSObjectCallAsFunction(ctx, f, 0, 0, 0, OK(false));
	}
	return true;
}

void Isolate::RunMicrotasks(bool Explicit) {
	if (m_microtask.size()) {
		std::vector<Microtask> microtask = std::move(m_microtask);
		for (auto& i: microtask) {
			if (!i.Run(this)) return;
		}
		if (Explicit) {
			for (auto& i: m_microtasks_completed_callback) {
				i.second(reinterpret_cast<v8::Isolate*>(this));
			}
		}
	}
}

v8_ns_end

// --- I s o l a t e ---

V8_EXPORT JSGlobalContextRef javascriptcore_context(v8::Isolate* isolate) {
	return ISOLATE(isolate)->jscc();
}

HeapProfiler* Isolate::GetHeapProfiler() {
	static uint64_t ptr = 0;
	return reinterpret_cast<HeapProfiler*>(&ptr);
}

CpuProfiler* Isolate::GetCpuProfiler() {
	static uint64_t ptr = 0;
	return reinterpret_cast<CpuProfiler*>(&ptr);
}

bool Isolate::InContext() {
	return reinterpret_cast<v8::Isolate*>(ISOLATE()) == this;
}

v8::Local<v8::Context> Isolate::GetCurrentContext() {
	return ISOLATE(this)->GetCurrentContext();
}

v8::Local<Value> Isolate::ThrowException(v8::Local<v8::Value> value) {
	ENV(this);
	isolate->ThrowException(i::Back(value));
	return i::Cast(isolate->Undefined());
}

Isolate* Isolate::GetCurrent() {
	return reinterpret_cast<v8::Isolate*>(ISOLATE());
}

Isolate* Isolate::New(const Isolate::CreateParams& params) {
	return i::Isolate::New(params);
}

void Isolate::Dispose() {
	ISOLATE(this)->Dispose();
}

void Isolate::TerminateExecution() {
	return ISOLATE(this)->Terminated();
}

bool Isolate::IsExecutionTerminating() {
	return ISOLATE(this)->HasTerminated();
}

void Isolate::CancelTerminateExecution() {
	ISOLATE(this)->CancelTerminated();
}

void Isolate::LowMemoryNotification() {
	JSGarbageCollect(JSC_CTX(this));
}

bool Isolate::AddMessageListener(MessageCallback that, Local<Value> data) {
	return ISOLATE(this)->AddMessageListener(that, data);
}

void Isolate::SetFatalErrorHandler(FatalErrorCallback that) {
	ISOLATE(this)->m_exception_behavior = that;
}

void Isolate::SetAbortOnUncaughtExceptionCallback(AbortOnUncaughtExceptionCallback callback) {
	ISOLATE(this)->m_abort_on_uncaught_exception_callback = callback;
}

// --- Isolate Microtasks ---

void Isolate::AddMicrotasksCompletedCallback(MicrotasksCompletedCallback callback) {
	ISOLATE(this)->m_microtasks_completed_callback[size_t(callback)] = callback;
}

void Isolate::RemoveMicrotasksCompletedCallback(MicrotasksCompletedCallback callback) {
	ISOLATE(this)->m_microtasks_completed_callback.erase(size_t(callback));
}

void Isolate::RunMicrotasks() {
	ISOLATE(this)->RunMicrotasks(true);
}

void Isolate::EnqueueMicrotask(Local<Function> microtask) {
	ISOLATE(this)->m_microtask.push_back(i::Microtask(ISOLATE(this), microtask));
}

void Isolate::EnqueueMicrotask(MicrotaskCallback microtask, void* data) {
	ISOLATE(this)->m_microtask.push_back(i::Microtask(microtask, data));
}

void Isolate::SetAutorunMicrotasks(bool autorun) {
	ISOLATE(this)->m_microtask_policy =
		autorun ? MicrotasksPolicy::kAuto : MicrotasksPolicy::kExplicit;
}

bool Isolate::WillAutorunMicrotasks() const {
	return false;
}

void Isolate::SetMicrotasksPolicy(MicrotasksPolicy policy) {
	ISOLATE(this)->m_microtask_policy = policy;
}

MicrotasksPolicy Isolate::GetMicrotasksPolicy() const {
	return ISOLATE(this)->m_microtask_policy;
}
