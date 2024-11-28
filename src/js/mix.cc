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

	void* JsHeapAllocator::alloc(size_t size) {
		auto o = ::malloc(size + sizeof(MixObject));
		Qk_Assert(o);
		::memset(o, 0, sizeof(MixObject));
		return static_cast<MixObject*>(o) + 1;
	}

	void JsHeapAllocator::free(void *ptr) {
		auto o = static_cast<MixObject*>(ptr) - 1;
		::free(o);
	}

	void JsHeapAllocator::strong(Object* obj) {
		auto mix = reinterpret_cast<MixObject*>(obj) - 1;
		if ( mix->worker() ) {
			MixObject::clearWeak(mix);
		}
	}

	void JsHeapAllocator::weak(Object* obj) {
		auto mix = reinterpret_cast<MixObject*>(obj) - 1;
		auto worker = mix->worker();
		if ( worker ) {
			if (thread_self_id() == worker->thread_id()) {
				MixObject::setWeak(mix);
			} else if (worker->isValid()) {
				worker->loop()->post(Cb((Cb::Static<>)[](auto e, auto obj) {
					MixObject::setWeak(reinterpret_cast<MixObject*>(obj) - 1); // Must be called on the js worker thread
				}, obj));
			} else {
				obj->destroy();
			}
		} else {
			obj->destroy();
		}
	}

	// ------------------------- W r a p . O b j e c t -------------------------

	bool MixObject::addEventListener(cString& name, cString& func, int id) {
		return false;
	}

	bool MixObject::removeEventListener(cString& name, int id) {
		return false;
	}

	void MixObject::init() {
	}

	MixObject::~MixObject() {
	}

	static bool isSetWeak(Object *obj) {
		return !obj->isReference() || /* non reference */
			static_cast<Reference*>(obj)->refCount() <= 0;
	}

	MixObject* MixObject::newInit(FunctionArgs args) {
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

	MixObject* MixObject::attach(Worker *worker, JSObject* This) {
		_handle.reset(worker, This);
		Qk_Assert(This->setObjectPrivate(this));
		init();
		return this;
	}

	Object* MixObject::externalData() {
		auto worker = _handle.worker();
		auto data = _handle->get(worker, worker->strs()->_mix_external_data());
		if ( worker->instanceOf(data, Js_Typeid(Object)) )
			return mix(data)->self();
		return nullptr;
	}

	bool MixObject::setExternalData(Object* data) {
		Qk_Assert(data);
		auto p = mix(data, Js_Typeid(Object));
		if (p) {
			if (isSetWeak(data)) {
				setWeak(p);
			}
			auto worker = _handle.worker();
			//Qk_DLog("%i", p->handle().isWeak());
			Qk_Assert(_handle->set(worker, worker->strs()->_mix_external_data(), p->handle()));
			//Qk_DLog("%i", p->handle().isWeak());
			Qk_Assert(externalData());
		}
		return p;
	}

	JSValue* MixObject::call(JSValue* method, int argc, JSValue* argv[]) {
		auto recv = *_handle;
		auto func = recv->get(worker(), method);
		if ( func->isFunction() ) {
			return func->as<JSFunction>()->call(worker(), argc, argv, recv);
		} else {
			worker()->throwError("Function not found, \"%s\"", *method->toStringValue(worker()));
			return nullptr;
		}
	}

	JSValue* MixObject::call(cString& name, int argc, JSValue* argv[]) {
		return call(worker()->newStringOneByte(name), argc, argv);
	}

	MixObject* MixObject::unpack(JSValue* object) {
		Qk_Assert(object);
		return static_cast<MixObject*>(object->as<JSObject>()->getObjectPrivate());
	}

	MixObject* MixObject::pack(Object* object, uint64_t type_id) {
		MixObject* mix = reinterpret_cast<MixObject*>(object) - 1;
		if ( !mix->worker() ) {
			Js_Worker();
			return worker->classses()->attachObject(type_id, object);
		}
		return mix;
	}

} }
