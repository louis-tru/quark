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

#include "./wrap.h"
#include "./_js.h"
#include <v8.h>

JS_BEGIN

#if FX_MEMORY_TRACE_MARK
static int record_wrap_count = 0;
static int record_strong_count = 0;

# define print_wrap(s) \
	LOG("record_wrap_count: %d, strong: %d, %s", record_wrap_count, record_strong_count, s)
#else
# define print_wrap(s)
#endif

/**
 * @class WrapObject::Inl
 */
class WrapObject::Inl: public WrapObject {
 public:
 #define _inl_wrap(self) static_cast<WrapObject::Inl*>(self)
	
	void clear_weak() {
#if FX_MEMORY_TRACE_MARK
		if (IMPL::current(worker())->IsWeak(handle_)) {
			record_strong_count++;
			print_wrap("mark_strong");
		}
#endif
		IMPL::current(worker())->ClearWeak(handle_, this);
	}
	
	void make_weak() {
#if FX_MEMORY_TRACE_MARK
		if (!IMPL::current(worker())->IsWeak(handle_)) {
			record_strong_count--;
			print_wrap("make_weak");
		}
#endif
		IMPL::current(worker())->SetWeak(handle_, this, [](const WeakCallbackInfo& info) {
			auto self = _inl_wrap(info.GetParameter());
			self->handle_.V8_DEATH_RESET();
			self->destroy();
		});
	}
};

void WrapObject::initialize() {}
void WrapObject::destroy() {
	delete this;
}

void WrapObject::init2(FunctionCall args) {
	ASSERT(args.IsConstructCall());
	Worker* worker_ = args.worker();
	auto classs = IMPL::current(worker_)->js_class();
	ASSERT( !classs->current_attach_object_ );
	handle_.Reset(worker_, args.This());
	bool ok = IMPL::SetObjectPrivate(args.This(), this); ASSERT(ok);
#if FX_MEMORY_TRACE_MARK
	record_wrap_count++; 
	record_strong_count++;
#endif 
	if (!self()->is_reference() || /* non reference */
			static_cast<Reference*>(self())->ref_count() <= 0) {
		_inl_wrap(this)->make_weak();
	}
	initialize();
}

WrapObject* WrapObject::attach(FunctionCall args) {
	JS_WORKER(args);
	auto classs = IMPL::current(worker)->js_class();
	if ( classs->current_attach_object_ ) {
		WrapObject* wrap = classs->current_attach_object_;
		ASSERT(!wrap->worker());
		ASSERT(args.IsConstructCall());
		wrap->handle_.Reset(worker, args.This());
		bool ok = IMPL::SetObjectPrivate(args.This(), wrap); ASSERT(ok);
		classs->current_attach_object_ = nullptr;
		wrap->initialize();
#if FX_MEMORY_TRACE_MARK
		record_wrap_count++; 
		record_strong_count++;
		print_wrap("External");
#endif
		return wrap;
	}
	return nullptr;
}

WrapObject::~WrapObject() {
	ASSERT(handle_.IsEmpty());

#if FX_MEMORY_TRACE_MARK
	record_wrap_count--;
	print_wrap("~WrapObject");
#endif 
	self()->~Object();
}

Object* WrapObject::privateData() {
	Local<JSValue> data = get(worker()->strs()->__native_private_data());
	if ( worker()->hasInstance(data, JS_TYPEID(Object)) ) {
		return unpack<Object>(data.To<JSObject>())->self();
	}
	return nullptr;
}

bool WrapObject::setPrivateData(Object* data, bool trusteeship) {
	ASSERT(data);
	auto p = pack(data, JS_TYPEID(Object));
	if (p) {
		set(worker()->strs()->__native_private_data(), p->that());
		if (trusteeship) {
			if (!data->is_reference() || /* non reference */
					static_cast<Reference*>(data)->ref_count() <= 0) {
				_inl_wrap(static_cast<WrapObject*>(p))->make_weak();
			}
		}
		ASSERT(privateData());
	}
	return p;
}

Local<JSValue> WrapObject::call(Local<JSValue> name, int argc, Local<JSValue> argv[]) {
	Local<JSObject> o = that();
	Local<JSValue> func = o->Get(worker(), name);
	if ( func->IsFunction(worker()) ) {
		return func.To<JSFunction>()->Call(worker(), argc, argv, o);
	} else {
		worker()->throwError("Function not found, \"%s\"", *name->ToStringValue(worker()));
		return Local<JSValue>();
	}
}

Local<JSValue> WrapObject::call(cString& name, int argc, Local<JSValue> argv[]) {
	return call(worker()->New(name), argc, argv);
}

bool WrapObject::isPack(Local<JSObject> object) {
	ASSERT(!object.IsEmpty());
	return IMPL::GetObjectPrivate(object);
}

WrapObject* WrapObject::unpack2(Local<JSObject> object) {
	ASSERT(!object.IsEmpty());
	return static_cast<WrapObject*>(IMPL::GetObjectPrivate(object));
}

WrapObject* WrapObject::pack2(Object* object, uint64 type_id) {
	WrapObject* wrap = reinterpret_cast<WrapObject*>(object) - 1;
	if ( !wrap->worker() ) { // uninitialized
		JS_WORKER();
		return IMPL::js_class(worker)->attach(type_id, object);
	}
	return wrap;
}

void* object_allocator_alloc(size_t size) {
	WrapObject* o = (WrapObject*)::malloc(size + sizeof(WrapObject));
	ASSERT(o);
	memset((void*)o, 0, sizeof(WrapObject));
	return o + 1;
}

void object_allocator_release(Object* obj) {
	WrapObject* wrap = reinterpret_cast<WrapObject*>(obj) - 1;
	if ( wrap->worker() ) {
		_inl_wrap(wrap)->make_weak();
	}  else { // uninitialized
		obj->~Object();
		::free(wrap);
	}
}

void object_allocator_retain(Object* obj) {
	WrapObject* wrap = reinterpret_cast<WrapObject*>(obj) - 1;
	if ( wrap->worker() ) {
		_inl_wrap(wrap)->clear_weak();
	} // else // uninitialized
}

JS_END
