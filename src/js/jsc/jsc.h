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

// @private head

#ifndef __quark__js__jsc__jsc__
#define __quark__js__jsc__jsc__

#include "../js_.h"
#include "./macros.h"
#include <JavaScriptCore/JavaScript.h>

namespace qk {
	template<>
	struct object_traits<OpaqueJSString> {
		struct is { static constexpr bool ref = true; };
		inline static void Retain(JSStringRef obj) { if (obj) JSStringRetain(obj); }
		inline static void Release(JSStringRef obj) { if (obj) JSStringRelease(obj); }
	};
	template<>
	struct object_traits<OpaqueJSPropertyNameArray> {
		struct is { static constexpr bool ref = true; };
		inline static void Retain(JSPropertyNameArrayRef obj) { if (obj) JSPropertyNameArrayRetain(obj); }
		inline static void Release(JSPropertyNameArrayRef obj) { if (obj) JSPropertyNameArrayRelease(obj); }
	};
}

namespace qk { namespace js {
	#define ENV(...) \
	auto worker = WORKER(__VA_ARGS__); \
	auto ctx = worker->jscc(); \
	JSValueRef ex = nullptr
	#define WORKER(...) qk::js::JscWorker::worker(__VA_ARGS__)
	#define JSC_CTX(...) WORKER(__VA_ARGS__)->jscc()
	#define OK(...) &ex); do { \
	if (ex) { \
		worker->throwException(ex); \
		return __VA_ARGS__ ;\
	}} while (0
	#define THROW_ERR(msg) worker->throwException(worker->newErrorJsc(msg))

	#define JsStringWithUTF8(S) JSCStringPtr::lazy(JSStringCreateWithUTF8CString(S))
	#define JsFatal(...) &ex); do { qk::js::jsFatal(ctx,ex, ##__VA_ARGS__); } while (0
	#define DCHECK Qk_ASSERT
	#define CHECK  Qk_ASSERT_RAW

	#define _Fun(name) extern JSStringRef name##_s;
	Js_Const_Strings(_Fun)
	#undef _Fun

	class StackFrame;
	class StackTrace;
	class Message;
	class JscHandleScope;
	class JscTryCatch;
	class JscWorker;
	class JscClass;

	typedef Sp<OpaqueJSString> JSCStringPtr;
	typedef Sp<OpaqueJSPropertyNameArray> JSCPropertyNameArrayPtr;

	inline JSCStringPtr JsValueToStringCopy(JSContextRef ctx, JSValueRef val, JSValueRef* ex) {
		return JSCStringPtr::lazy(JSValueToStringCopy(ctx,val,ex));
	}

	template<class T = JSValue>
	inline T* Cast(JSValueRef o) {
		return reinterpret_cast<T*>( const_cast<JSObjectRef>(o) );
	}

	template<class T = JSValueRef>
	inline T Back(const JSValue* o) {
		return reinterpret_cast<T>(const_cast<JSValue*>(o));
	}

	template<typename TO, typename FROM>
	inline TO bitwise_cast(FROM in) {
		DCHECK(sizeof(TO) == sizeof(FROM), "WTF_wtf_reinterpret_cast_sizeof_types_is_equal");
		union {
			FROM from;
			TO to;
		} u;
		u.from = in;
		return u.to;
	}

	void initFactories();
	void jsFatal(JSContextRef ctx, JSValueRef ex, cChar* msg = 0);
	String jsToString(JSStringRef value);
	String jsToString(JSContextRef ctx, JSValueRef value);
	void defalutMessageListener(Worker* worker, JSValue* exception);
	void defalutPromiseRejectListener(Worker* worker, JSValue* reason, JSValue* promise);

	struct WorkerData {
		void initialize(JSGlobalContextRef ctx);
		void destroy(JSGlobalContextRef ctx);
		JSValueRef Undefined, Null, True, False, EmptyString;
		JSObjectRef TypedArray, MakeConstructor;
		#define _Attr_Fun(name,from) JSObjectRef from##_##name;
		Js_Worker_Data_Each(_Attr_Fun)
		#undef _Attr_Fun
	};

	class JscWorker: public Worker {
	public:
		JscWorker();
		void release();
		inline JSGlobalContextRef jscc() { return _ctx; }
		JSObjectRef newErrorJsc(cChar* message);
		void throwException(JSValueRef exception);
		JSValue* runScript(JSString* jsSource, cString& source, cString& name, JSObject* sandbox);
		template<typename T = JSValue>
		inline T* addToScope(JSValueRef ref) {
			return reinterpret_cast<T*>(addToScope<JSValue>(ref));
		}
		inline static JscWorker* worker() {
			return static_cast<JscWorker*>(Worker::current());
		}
		template<class Args>
		inline static JscWorker* worker(Args args) { return nullptr; }
		inline WorkerData& data() { return _data; }

	private:
		JSContextGroupRef _group;
		JSGlobalContextRef _ctx;
		WorkerData  _data;
		JSValueRef  _ex;
		JscTryCatch *_try;
		JscHandleScope *_scope;
		JscClass *_base;
		JSObjectRef _rejectionListener;
		int _callStack;

		friend class StackFrame;
		friend class StackTrace;
		friend class Message;
		friend class JscHandleScope;
		friend class JscTryCatch;
		friend class JscClass;
		friend class Worker;
		friend class ReturnValue;
		friend class JSValue;
	};

	template<>
	JSValue* JscWorker::addToScope(JSValueRef ref);

	template<>
	inline JscWorker* JscWorker::worker<Worker*>(Worker* arg) {
		return static_cast<JscWorker*>(arg);
	}

	template<>
	inline JscWorker* JscWorker::worker<JscWorker*>(JscWorker* arg) {
		return arg;
	}

	struct ReturnValueImpl {
		JscWorker*  _worker;
		JSObjectRef _this;
		JSValueRef  _return;
	};

	struct FunctionCallbackInfoImpl {
		JscWorker* _worker;
		JSObjectRef _this;
		JSValueRef  _return;
		const JSValueRef *argv;
		int argc;
		bool isConstructCall;
	};

	struct PropertyCallbackInfoImpl {
		JscWorker* _worker;
		JSObjectRef _this;
		JSValueRef _return;
	};

	struct PropertySetCallbackInfoImpl {
		Worker* _worker;
		JSObjectRef _this;
	};

} }


#endif
