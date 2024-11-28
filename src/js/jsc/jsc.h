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

#include "./js_.h"
#include "./macros.h"
#include <JavaScriptCore/JavaScript.h>

namespace qk {
	template<>
	struct object_traits<OpaqueJSString> {
		inline static void Retain(JSStringRef obj) { if (obj) JSStringRetain(obj); }
		inline static void Release(JSStringRef obj) { if (obj) JSStringRelease(obj); }
		inline static void Releasep(JSStringRef& obj) { Release(obj); obj = nullptr; }
	};
	template<>
	struct object_traits<OpaqueJSPropertyNameArray> {
		inline static void Retain(JSPropertyNameArrayRef obj) { if (obj) JSPropertyNameArrayRetain(obj); }
		inline static void Release(JSPropertyNameArrayRef obj) { if (obj) JSPropertyNameArrayRelease(obj); }
		inline static void Releasep(JSPropertyNameArrayRef& obj) { Release(obj); obj = nullptr; }
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
		worker->ThrowException(ex); \
		return __VA_ARGS__ ;\
	}}while(0

	#define JsFatal(ctx) &ex); do { qj::js::checkFatal(ctx, ex); }while(0
	#define DCHECK Qk_Assert

	class StackFrame;
	class StackTrace;
	class Message;
	class JscHandleScope;
	class JscTryCatch;
	class JscWorker;
	struct GlobalData;
	struct ContextData;
	struct Templates;

	typedef void (*MessageCallback)(Message* message, JSValue* exception);
	typedef Handle<OpaqueJSString> JSCStringPtr;
	typedef Handle<OpaqueJSPropertyNameArray> JSCPropertyNameArrayPtr;

	template<class T = JSValue>
	inline T* Cast(JSValueRef o) {
		return reinterpret_cast<T*>(o);
	}

	template<class T = JSValueRef>
	inline T Back(JSValue* o) {
		return reinterpret_cast<T>(o);
	}

	void        checkFatal     (JSContextRef ctx, JSValueRef exception);
	JSObjectRef runNativeScript(JSGlobalContextRef ctx, cChar* script, cChar* name);

	struct GlobalData {
		void initialize(JSGlobalContextRef ctx);
		void destroy(JSGlobalContextRef ctx);
		JSValueRef Undefined, Null, True, False, Empty;
		#define _Attr_Fun(name) JSObjectRef name;
		Js_Worker_Data_List(_Attr_Fun)
	};

	struct ContextData {
		void initialize(JSGlobalContextRef ctx);
		void destroy(JSGlobalContextRef ctx);
		Js_Context_Data_List(_Attr_Fun)
		#undef _Attr_Fun
	};

	struct Templates {
		void initialize(JSGlobalContextRef ctx) {}
		void destroy(JSGlobalContextRef ctx) {}
	};

	class JscWorker: public Worker {
		JscWorker();
		void release();
		inline JSGlobalContextRef jscc() { return _ctx; }
		inline bool hasTerminated() { return _hasTerminated; }
		inline bool hasDestroy() { return _hasDestroy; }
		inline void addMessageListener(MessageCallback that) { _messageListener = that; }
		JSObjectRef newError(cChar* message);
		void throwException(JSValueRef exception);
		void reportException(JSObjectRef exception);
		void addToScope(JSValueRef ref);
		inline static JscWorker* worker() {
			return static_cast<JscWorker*>(Worker::current());
		}
		template<class Args>
		inline static JscWorker* worker(Args args) {
			// return static_cast<JscWorker*>( args.GetIsolate()->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
			return nullptr;
		}
	private:
		JSContextGroupRef _group;
		JSGlobalContextRef _ctx; // default ctx for jsc global
		GlobalData  _globalData;
		ContextData _ctxData; // default ctx data
		Templates   _templates;
		JSValueRef  _exception;
		JscTryCatch *_try;
		JscHandleScope *_scope;
		MessageCallback _messageListener;
		bool _hasTerminated, _hasDestroy;
		friend class StackFrame;
		friend class StackTrace;
		friend class Message;
		friend class JscHandleScope;
		friend class JscTryCatch;
	};

	template<>
	inline JscWorker* JscWorker::worker<Worker*>(Worker* arg) {
		return static_cast<JscWorker*>(arg);
	}

	template<>
	inline JscWorker* JscWorker::worker<Worker*>(JscWorker* arg) {
		return arg;
	}

} }


#endif