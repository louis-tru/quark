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

// @private head

#ifndef __quark__js___js__
#define __quark__js___js__

#include "./js.h"
#include "./types.h"

#define Js_On( name, block, id, ...) \
	Qk_On(name, [this,##__VA_ARGS__]( auto & evt) { HandleScope scope(worker()); block }, id)

#define Js_Native_On(name, func, id) \
	Js_On(name, { call(worker()->newInstance(func,true)); }, id, func)

namespace qk { namespace js {

	int  platformStart(int argc, Char** argv);
	int  triggerExit(Worker* worker, int code);
	int  triggerBeforeExit(Worker* worker, int code);
	bool triggerUncaughtException(Worker* worker, Local<JSValue> err);
	bool triggerUnhandledRejection(Worker* worker, Local<JSValue> reason, Local<JSValue> promise);

	struct BindingModule: public Worker {
		Local<JSValue> binding(Local<JSValue> name);
	};

	class JsClassInfo {
	public:
		JsClassInfo(Worker* worker);
		~JsClassInfo();
		void add(uint64_t id, JSClass *cls, AttachCallback cb) throw(Error);
		Local<JSClass> get(uint64_t id);
		Local<JSFunction> getFunction(uint64_t id);
		WrapObject* attach(uint64_t id, Object* object);
		bool instanceOf(Local<JSValue> val, uint64_t id);
	private:
		Worker *_worker;
		Dict<uint64_t, JSClass*> _jsclass;
		Persistent<JSFunction> _jsAttachConstructorEmpty;
	};

	struct JsConverter { // convert data to js value
		template<class T>
		static inline Local<JSValue> Cast(Worker* worker, const Object& obj) {
			return worker->newInstance( *static_cast<const T*>(&obj) ).cast();
		}
		inline Local<JSValue> cast(Worker* worker, const Object& object) {
			return callback(worker, object);
		}
		template<class T>
		static JsConverter* Instance() {
			static JsConverter value{&Cast<T>};
			return &value;
		}
	private:
		Local<JSValue> (*callback)(Worker* worker, const Object& object);
	};

} }
#endif
