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

#ifndef __quark__js__v8__v8js__
#define __quark__js__v8__v8js__

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

	#define DCHECK Qk_Assert
	#define CHECK  Qk_Fatal_Assert

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

	class WorkerImpl: public Worker {
	public:
		struct HandleScopeMix {
			v8::HandleScope value;
			inline HandleScopeMix(Isolate* isolate): value(isolate) {}
		};
		Isolate*  _isolate;
		Locker*   _locker;
		HandleScopeMix* _handle_scope;
		v8::Local<v8::Context> _context;
		inspector::Agent* _inspector;
		Isolate::CreateParams _params;

		WorkerImpl();
		void release() override;

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
		void printException(v8::Local<v8::Message> message, v8::Local<v8::Value> error);
		void runDebugger(const DebugOptions &opts);
		void stopDebugger();
		void debuggerBreakNextStatement();
	};

	template<>
	inline WorkerImpl* WorkerImpl::worker<Worker*>(Worker* worker) {
		return static_cast<WorkerImpl*>(worker);
	}

	template<>
	inline WorkerImpl* WorkerImpl::worker<Isolate*>(Isolate* isolate) {
		return static_cast<WorkerImpl*>( isolate->GetData(ISOLATE_INL_WORKER_DATA_INDEX) );
	}

}}
#endif