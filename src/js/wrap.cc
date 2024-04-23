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

#include "./_js.h"

namespace qk { namespace js {

#if Qk_MEMORY_TRACE_MARK
	static int record_wrap_count = 0;
	static int record_strong_count = 0;
# define print_wrap(s) \
	Qk_LOG("record_wrap_count: %d, strong: %d, %s", record_wrap_count, record_strong_count, s)
#else
# define print_wrap(s)
#endif

	static void WrapObject_clearWeak(WrapObject *wrap) {
#if Qk_MEMORY_TRACE_MARK
		if (wrap->handle().isWeak()) {
			record_strong_count++;
			print_wrap("WrapObject_clearWeak");
		}
#endif
		wrap->handle().clearWeak();
	}

	static void WrapObject_setWeak(WrapObject *wrap) {
#if Qk_MEMORY_TRACE_MARK
		if (!wrap->handle().isWeak()) {
			record_strong_count--;
			print_wrap("WrapObject_setWeak");
		}
#endif
		wrap->handle().setWeak(wrap, [](const WeakCallbackInfo& info) {
			auto self = static_cast<WrapObject*>(info.getParameter());
			self->handle().reset(); // TODO DEATH RESET ..
			self->~WrapObject(); // destroy
		});
	}

	void* object_allocator_alloc(size_t size) {
		auto o = (WrapObject*)::malloc(size + sizeof(WrapObject));
		Qk_ASSERT(o);
		::memset((void*)o, 0, sizeof(WrapObject));
		return o + 1;
	}

	void  object_allocator_free(void *ptr) {
		auto o = static_cast<WrapObject*>(ptr) - 1;
		::free(o);
	}

	void object_allocator_weak(Object* obj) {
		auto wrap = reinterpret_cast<WrapObject*>(obj) - 1;
		if ( wrap->worker() ) {
			WrapObject_setWeak(wrap);
		}  else {
			obj->destroy();
		}
	}

	void object_allocator_strong(Object* obj) {
		auto wrap = reinterpret_cast<WrapObject*>(obj) - 1;
		if ( wrap->worker() ) {
			WrapObject_clearWeak(wrap);
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
		Qk_ASSERT(_handle.isEmpty());
#if Qk_MEMORY_TRACE_MARK
		record_wrap_count--;
		print_wrap("WrapObject::~WrapObject()");
#endif
		self()->destroy();
	}

	WrapObject* WrapObject::newInit(FunctionArgs args) {
		Qk_ASSERT(args.isConstructCall());
		_handle.reset(args.worker(), args.This());
		auto ok = args.This()->setObjectPrivate(this);
		Qk_ASSERT(ok);
#if Qk_MEMORY_TRACE_MARK
		record_wrap_count++;
		record_strong_count++;
#endif
		if (!self()->isReference() || /* non reference */
				static_cast<Reference*>(self())->refCount() <= 0) {
			WrapObject_setWeak(this);
		}
		init();
		return this;
	}

	WrapObject* WrapObject::attach(Worker *worker, Local<JSObject> This) {
		_handle.reset(worker, This);
		bool ok = This->setObjectPrivate(this); Qk_ASSERT(ok);
		init();
#if Qk_MEMORY_TRACE_MARK
		record_wrap_count++;
		record_strong_count++;
		print_wrap("WrapObject::attach()");
#endif
		return this;
	}

	Object* WrapObject::externalData() {
		Local<JSValue> data = get(worker()->strs()->__native_external_data());
		if ( worker()->instanceOf(data, Js_Typeid(Object)) ) {
			return wrap<Object>(data)->self();
		}
		return nullptr;
	}

	bool WrapObject::setExternalData(Object* data) {
		Qk_ASSERT(data);
		auto p = wrap(data, Js_Typeid(Object));
		if (p) {
			set(worker()->strs()->__native_external_data(), p->that());
			if (!data->isReference() || /* non reference */
					static_cast<Reference*>(data)->refCount() <= 0) {
				WrapObject_setWeak(p);
			}
			Qk_ASSERT(externalData());
		}
		return p;
	}

	Local<JSValue> WrapObject::call(Local<JSValue> method, int argc, Local<JSValue> argv[]) {
		Local<JSObject> recv = that();
		Local<JSValue> func = recv->get(worker(), method);
		if ( func->isFunction(worker()) ) {
			return func.cast<JSFunction>()->call(worker(), argc, argv, recv.cast());
		} else {
			worker()->throwError("Function not found, \"%s\"", *method->toStringValue(worker()));
			return Local<JSValue>();
		}
	}

	Local<JSValue> WrapObject::call(cString& name, int argc, Local<JSValue> argv[]) {
		return call(worker()->newInstance(name), argc, argv);
	}

	WrapObject* WrapObject::unpack(Local<JSObject> object) {
		Qk_ASSERT(!object.isEmpty());
		return static_cast<WrapObject*>(object->objectPrivate());
	}

	WrapObject* WrapObject::pack(Object* object, uint64_t type_id) {
		WrapObject* wrap = reinterpret_cast<WrapObject*>(object) - 1;
		if ( !wrap->worker() ) {
			Js_Worker();
			return worker->classsinfo()->attach(type_id, object);
		}
		return wrap;
	}

} }