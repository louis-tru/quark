// @private head
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

#ifndef __quark__js___js__
#define __quark__js___js__

#include "./js.h"
#include "./types.h"

#define js_bind_native_event( name, type, block) \
	Qk_On(name, [this, func]( type & evt) { HandleScope scope(worker()); block }, id)

#define js_unbind_native_event(name) Qk_Off(name, id);

#define js_bind_common_native_event(name) \
	js_bind_native_event(name, Event<>, { call(worker()->New(func,1)); })

namespace qk { namespace js {

	class WorkerImpl: public Worker {
	public:
		// @static
		static inline WorkerImpl* Impl(Worker* worker) {
			return static_cast<WorkerImpl*>(worker);
		}

		template<class T = WorkerImpl>
		static inline T* Current(Worker* worker = Worker::worker()) {
			return static_cast<T*>(worker);
		}

		static WrapObject* GetObjectPrivate(Local<JSObject> object);
		static bool        SetObjectPrivate(Local<JSObject> object, WrapObject* value);

		static int Start(int argc, Char** argv);

		// @member
		WorkerImpl();
		virtual ~WorkerImpl();
		virtual void initialize();

		int  triggerExit(int code);
		int  triggerBeforeExit(int code);
		bool triggerUncaughtException(Local<JSValue> err);
		bool triggerUnhandledRejection(Local<JSValue> reason, Local<JSValue> promise);

		friend class Worker;
		friend class NativeValue;
	};

	class JSClassImpl: public JSClass {
	public:
		Qk_DEFINE_PROP_GET(Worker*, worker, Protected);
		Qk_DEFINE_PROP_GET(AttachCallback, attachCallback, Protected);
		Qk_DEFINE_PROP_GET(uint64_t, id, Protected);
		Qk_DEFINE_PROP_GET(String, name, Protected);
		Qk_DEFINE_PROP_GET(int, ref, Protected);
		Qk_DEFINE_PROP_ACC_GET(Local<JSFunction>, func); // constructor function

		JSClassImpl(Worker* worker, uint64_t id, cString& name);
		virtual ~JSClassImpl();
		void retain();
		void release();
		void resetFunc();

	private:
		Persistent<JSFunction> _func; // constructor function

		friend class JSClass;
		friend class JSClassInfo;
	};

	class JSClassInfo {
	public:
		JSClassInfo(Worker* worker);
		~JSClassInfo();
		void add(uint64_t id, JSClass *cls, AttachCallback callback, uint64_t alias = 0) throw(Error);
		Local<JSClass> get(uint64_t id);
		WrapObject* attach(uint64_t id, Object* object);
		bool instanceOf(Local<JSValue> val, uint64_t id);
	private:
		Worker* _worker;
		Array<JSClass*> _jsclass;
		Dict<uint64_t, JSClass*> _alias;
		WrapObject* _currentAttachObject;

		friend class WrapObject;
	};

} }
#endif
