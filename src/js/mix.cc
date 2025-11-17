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
		Qk_ASSERT(o);
		::memset(o, 0, sizeof(MixObject));
		return static_cast<MixObject*>(o) + 1;
	}

	void JsHeapAllocator::free(void *ptr) {
		auto o = static_cast<MixObject*>(ptr) - 1;
		::free(o);
	}

	void JsHeapAllocator::strong(Object* obj) {
		auto mix = reinterpret_cast<MixObject*>(obj) - 1;
		if ( mix->_class ) {
			mix->clearWeak();
		}
	}

	void JsHeapAllocator::weak(Object* obj) {
		auto mix = reinterpret_cast<MixObject*>(obj) - 1;
		if ( mix->_class ) {
			auto worker = mix->worker();
			if (thread_self_id() == worker->thread_id()) {
				mix->setWeak();
			} else if (worker->isValid()) {
				worker->loop()->post(Cb((Cb::Static<>)[](auto e, auto obj) {
					// Must be called on the js worker thread
					(reinterpret_cast<MixObject*>(obj) - 1)->setWeak();
				}, obj));
			} else {
				obj->destroy();
			}
		} else {
			obj->destroy();
		}
	}

	// ------------------------- W r a p . O b j e c t -------------------------

	bool MixObject::addEventListener(cString& name, cString& func, uint32_t id) {
		return false;
	}

	bool MixObject::removeEventListener(cString& name, uint32_t id) {
		return false;
	}

	MixObject::~MixObject() {
		_handle = nullptr;
		_class = nullptr;
	}

	void MixObject::initialize() {
	}

	JSValue* MixObject::call(JSValue* method, int argc, JSValue* argv[]) {
		auto recv = _handle;
		auto func = recv->get(worker(), method);
		if ( func->isFunction() ) {
			return func->cast<JSFunction>()->call(worker(), argc, argv, recv);
		} else {
			auto w = worker();
			w->throwError("Function not found, \"%s\"", *method->toString(w)->value(w));
			return nullptr;
		}
	}

	JSValue* MixObject::call(cString& name, int argc, JSValue* argv[]) {
		return call(worker()->newStringOneByte(name), argc, argv);
	}

	MixObject* MixObject::newInit(FunctionArgs args) {
		Qk_ASSERT_EQ(_handle, nullptr);
		Qk_ASSERT_EQ(args.isConstructCall(), true);
		auto worker = args.worker();
		_class = worker->classes()->_runClass;
		Qk_ASSERT_NE(_class, nullptr);
		bindObject(args.thisObj());
		auto obj = self();
		if (!obj->isReference() || static_cast<Reference*>(obj)->refCount() <= 0)
			setWeak();
		initialize();
		return this;
	}

	MixObject* MixObject::pack(Object* obj, uint64_t classAlias) {
		auto mix = reinterpret_cast<MixObject*>(obj) - 1;
		if (!mix->_class) {
			Js_Worker();
			auto classes = worker->classes();
			auto cls = classes->get(classAlias);
			if (!cls)
				return nullptr;
			Qk_ASSERT_EQ(classes->_attachObject, nullptr);
			classes->_attachObject = mix;
			auto handle = cls->newInstance();
			classes->_attachObject = nullptr;
			mix->_class = cls;
			mix->bindObject(handle);
			mix->initialize();
		}
		return mix;
	}

} }
