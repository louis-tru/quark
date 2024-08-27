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

#include "./js_.h"

namespace qk { namespace js {

	static void clearWeak(WrapObject *wrap) {
		wrap->handle().clearWeak();
	}

	static void setWeak(WrapObject *wrap) {
		wrap->handle().setWeak(wrap, [](const WeakCallbackInfo& info) {
			auto self = static_cast<WrapObject*>(info.getParameter());
			self->~WrapObject(); // destroy wrap
			self->self()->destroy(); // destroy object
		});
	}

	void* JsHeapAllocator::alloc(size_t size) {
		auto o = ::malloc(size + sizeof(WrapObject));
		Qk_Assert(o);
		::memset(o, 0, sizeof(WrapObject));
		return static_cast<WrapObject*>(o) + 1;
	}

	void JsHeapAllocator::free(void *ptr) {
		auto o = static_cast<WrapObject*>(ptr) - 1;
		::free(o);
	}

	void JsHeapAllocator::strong(Object* obj) {
		auto wrap = reinterpret_cast<WrapObject*>(obj) - 1;
		if ( wrap->worker() ) {
			clearWeak(wrap);
		}
	}

	void JsHeapAllocator::weak(Object* obj) {
		auto wrap = reinterpret_cast<WrapObject*>(obj) - 1;
		auto worker = wrap->worker();
		if ( worker ) {
			if (thread_self_id() == worker->thread_id()) {
				setWeak(wrap);
			} else if (worker->isValid()) {
				worker->loop()->post(Cb((Cb::Static<>)[](auto e, auto obj) {
					setWeak(reinterpret_cast<WrapObject*>(obj) - 1); // Must be called on the js worker thread
				}, obj));
			} else {
				obj->destroy();
			}
		} else {
			obj->destroy();
		}
	}

	// ------------------------- W r a p . O b j e c t -------------------------

	bool WrapObject::addEventListener(cString& name, cString& func, int id) {
		return false;
	}

	bool WrapObject::removeEventListener(cString& name, int id) {
		return false;
	}

	void WrapObject::init() {
	}

	WrapObject::~WrapObject() {
	}

	static bool isSetWeak(Object *obj) {
		return !obj->isReference() || /* non reference */
			static_cast<Reference*>(obj)->refCount() <= 0;
	}

	WrapObject* WrapObject::newInit(FunctionArgs args) {
		Qk_Assert(_handle.isEmpty());
		Qk_Assert(args.isConstructCall());
		_handle.reset(args.worker(), args.This());
		Qk_Assert(args.This()->setObjectPrivate(this));
		if (isSetWeak(self())) {
			setWeak(this);
		}
		init();
		return this;
	}

	WrapObject* WrapObject::attach(Worker *worker, JSObject* This) {
		_handle.reset(worker, This);
		Qk_Assert(This->setObjectPrivate(this));
		init();
		return this;
	}

	Object* WrapObject::externalData() {
		auto data = get(worker()->strs()->_wrap_external_data());
		if ( worker()->instanceOf(data, Js_Typeid(Object)) )
			return wrap<Object>(data)->self();
		return nullptr;
	}

	bool WrapObject::setExternalData(Object* data) {
		Qk_Assert(data);
		auto p = wrap(data, Js_Typeid(Object));
		if (p) {
			if (isSetWeak(data)) {
				setWeak(p);
			}
			//Qk_DEBUG("%i", p->handle().isWeak());
			Qk_Assert(set(worker()->strs()->_wrap_external_data(), p->that()));
			//Qk_DEBUG("%i", p->handle().isWeak());
			Qk_Assert(externalData());
		}
		return p;
	}

	JSValue* WrapObject::call(JSValue* method, int argc, JSValue* argv[]) {
		auto recv = that();
		auto func = recv->get(worker(), method);
		if ( func->isFunction() ) {
			return func->as<JSFunction>()->call(worker(), argc, argv, recv);
		} else {
			worker()->throwError("Function not found, \"%s\"", *method->toStringValue(worker()));
			return nullptr;
		}
	}

	JSValue* WrapObject::call(cString& name, int argc, JSValue* argv[]) {
		return call(worker()->newStringOneByte(name), argc, argv);
	}

	WrapObject* WrapObject::unpack(JSValue* object) {
		Qk_Assert(object);
		return static_cast<WrapObject*>(object->as<JSObject>()->objectPrivate());
	}

	WrapObject* WrapObject::pack(Object* object, uint64_t type_id) {
		WrapObject* wrap = reinterpret_cast<WrapObject*>(object) - 1;
		if ( !wrap->worker() ) {
			Js_Worker();
			return worker->classsinfo()->attachObject(type_id, object);
		}
		return wrap;
	}

} }
