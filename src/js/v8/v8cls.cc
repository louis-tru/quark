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

#include "./v8js.h"

namespace qk { namespace js {

	void MixObject::clearWeak() {
		Qk_Assert_Ne(_handle, nullptr);
		auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(&_handle);
		h->ClearWeak();
	}

	void MixObject::setWeak() {
		Qk_Assert_Ne( _handle, nullptr);
		auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(&_handle);
		//h->MarkIndependent();
		h->SetWeak(this, [](const v8::WeakCallbackInfo<MixObject>& info) {
			auto ptr = info.GetParameter();
			ptr->~MixObject(); // destroy mix
			ptr->self()->destroy(); // destroy object
		}, v8::WeakCallbackType::kParameter);
	}

	void MixObject::bindObject(JSObject* handle) {
		Qk_Assert_Eq(_handle, nullptr);
		auto h = reinterpret_cast<v8::PersistentBase<v8::Object>*>(&_handle);
		auto v8h = Back<v8::Object>(handle);
		h->Reset(ISOLATE(worker()), v8h);
		v8h->SetAlignedPointerInInternalField(0, this);
	}

	MixObject* MixObject::unpack(JSValue* obj) {
		Qk_Assert_Ne(obj, nullptr);
		auto v8obj = reinterpret_cast<v8::Object*>(obj);
		Qk_Assert_Gt(v8obj->InternalFieldCount(), 0);
		return static_cast<MixObject*>(v8obj->GetAlignedPointerFromInternalField(0));
	}

	class V8JSClass: public JSClass {
	public:
		V8JSClass(Worker* worker, cString& name,
							FunctionCallback constructor, AttachCallback attach, V8JSClass* base,
							v8::Local<v8::Function> baseFunc = v8::Local<v8::Function>())
			: JSClass(constructor, attach), _base(base)
		{
			_worker = worker;
			auto data = External::New(ISOLATE(worker), this);
			auto cb = (v8::FunctionCallback)[](const v8::FunctionCallbackInfo<v8::Value>& info) {
				auto self = static_cast<V8JSClass*>(info.Data().As<External>()->Value());
				self->callConstructor(*reinterpret_cast<const FunctionCallbackInfo*>(&info));
			};
			v8::Local<v8::FunctionTemplate> ft = v8::FunctionTemplate::New(ISOLATE(worker), cb, data);
			v8::Local<v8::String> className = Back<v8::String>(worker->newStringOneByte(name));

			if ( base ) {
				ft->Inherit( base->Template() );
			}
			else if ( !baseFunc.IsEmpty() ) {
				_baseFunc.Reset(ISOLATE(worker), baseFunc);
			}

			ft->SetClassName(className);
			ft->InstanceTemplate()->SetInternalFieldCount(1);
			_funcTemplate.Reset(ISOLATE(worker), ft);
		}

		~V8JSClass() {
			_baseFunc.Reset();
			_funcTemplate.Reset();
		}

		v8::Local<v8::FunctionTemplate> Template() {
			auto _ = reinterpret_cast<v8::Local<v8::FunctionTemplate>*>(&_funcTemplate);
			return *_;
		}

		v8::Local<v8::Function> BaseFunction() {
			auto _ = reinterpret_cast<v8::Local<v8::Function>*>(&_baseFunc);
			return *_;
		}

		bool HasBaseFunction() {
			return !_baseFunc.IsEmpty();
		}

	private:
		V8JSClass*                   _base;
		v8::Persistent<v8::Function> _baseFunc; // base constructor function
		v8::Persistent<v8::FunctionTemplate> _funcTemplate; // v8 func template
	};

	JSFunction* JSClass::getFunction() {
		if (_func.isEmpty()) { // Gen constructor
			auto v8cls = static_cast<V8JSClass*>(this);
			auto f = v8cls->Template()->GetFunction(CONTEXT(_worker)).ToLocalChecked();
			Qk_Assert_Eq(f.IsEmpty(), false);
			if (v8cls->HasBaseFunction()) {
				auto str = Back(_worker->strs()->prototype());
				auto base = v8cls->BaseFunction();
				auto proto = f->Get(CONTEXT(_worker), str).ToLocalChecked().As<v8::Object>();
				auto baseProto = base->Get(CONTEXT(_worker), str).ToLocalChecked().As<v8::Object>();
				// function.__proto__ = base;
				// f->SetPrototype(v8cls->BaseFunction());
				// function.prototype.__proto__ = base.prototype;
				Qk_Assert_Eq(true, proto->SetPrototype(CONTEXT(_worker), baseProto).ToChecked());
			}
			auto func = Cast<JSFunction>(f);
			_func.reset(_worker, func);
		}
		return *_func;
	}

	bool JSClass::hasInstance(JSValue* val) {
		return reinterpret_cast<V8JSClass*>(this)->Template()->HasInstance(Back(val));
	}

	bool JSClass::setMethod(cString& name, FunctionCallback func) {
		v8::Local<v8::FunctionTemplate> ftemp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		v8::Local<Signature> sign = Signature::New(ISOLATE(_worker), ftemp);
		v8::Local<v8::FunctionTemplate> t =
			v8::FunctionTemplate::New(ISOLATE(_worker), func2, v8::Local<v8::Value>(), sign);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		t->SetClassName(fn_name);
		ftemp->PrototypeTemplate()->Set(fn_name, t);
		return true;
	}

	bool JSClass::setAccessor(cString& name,
																	AccessorGetterCallback get, AccessorSetterCallback set) {
		v8::Local<v8::FunctionTemplate> ftemp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		v8::Local<AccessorSignature> sign = AccessorSignature::New(ISOLATE(_worker), ftemp);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		ftemp->PrototypeTemplate()->SetAccessor(fn_name, get2, set2,
																					v8::Local<v8::Value>(), v8::DEFAULT, v8::None, sign);
		return true;
	}

	/*
	bool JSClass::setLazyDataProperty(cString& name, AccessorGetterCallback get) {
		v8::Local<v8::FunctionTemplate> ftemp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		// v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		v8::Local<AccessorSignature> s = AccessorSignature::New(ISOLATE(_worker), ftemp);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		ftemp->PrototypeTemplate()->SetAccessor(fn_name, get2, nullptr,
																					v8::Local<v8::Value>(), v8::DEFAULT, v8::None, s);
		return true;
	}*/

	bool JSClass::setIndexedAccessor(IndexedAccessorGetterCallback get,
																				IndexedAccessorSetterCallback set) {
		v8::IndexedPropertyGetterCallback get2 = reinterpret_cast<v8::IndexedPropertyGetterCallback>(get);
		v8::IndexedPropertySetterCallback set2 = reinterpret_cast<v8::IndexedPropertySetterCallback>(set);
		v8::IndexedPropertyHandlerConfiguration cfg(get2, set2);
		reinterpret_cast<V8JSClass*>(this)->Template()->PrototypeTemplate()->SetHandler(cfg);
		return true;
	}

	template<>
	bool JSClass::setProperty<JSValue*>(cString& name, JSValue* value) {
		reinterpret_cast<V8JSClass*>(this)->Template()->
						PrototypeTemplate()->Set(Back<v8::String>(_worker->newStringOneByte(name)), Back(value));
		return true;
	}

	bool JSClass::setStaticMethod(cString& name, FunctionCallback func) {
		v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(ISOLATE(_worker), func2);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		t->SetClassName(fn_name);
		reinterpret_cast<V8JSClass*>(this)->Template()->Set(fn_name, t);
		return true;
	}

	template<>
	bool JSClass::setStaticProperty<JSValue*>(cString& name, JSValue* value) {
		reinterpret_cast<V8JSClass*>(this)->Template()->
						Set(Back<v8::String>(_worker->newStringOneByte(name)), Back(value));
		return true;
	}

	JSClass* Worker::newClass(cString& name, uint64_t alias,
															FunctionCallback constructor,
															AttachCallback attach, JSClass* base) {
		auto cls = new V8JSClass(this, name, constructor, attach, static_cast<V8JSClass*>(base));
		_classes->add(alias, cls);
		return cls;
	}

	JSClass* Worker::newClass(cString& name, uint64_t alias,
																	FunctionCallback constructor,
																	AttachCallback attach, uint64_t base) {
		return newClass(name, alias, constructor, attach, _classes->get(base));
	}

	JSClass* Worker::newClass(cString& name, uint64_t alias,
															FunctionCallback constructor,
															AttachCallback attach, JSFunction* base) {
		auto cls = new V8JSClass(this, name, constructor, attach, nullptr, Back<v8::Function>(base));
		_classes->add(alias, cls);
		return cls;
	}

	JSFunction* Worker::newFunction(cString& name, FunctionCallback func) {
		v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(ISOLATE(this), func2);
		t->SetClassName(Back<v8::String>(newStringOneByte(name)));
		return Cast<JSFunction>(t->GetFunction(CONTEXT(this)));
	}

	// ------------------- F u n c t i o n . C a l l b a c k . I n f o -------------------

	void ReturnValue::set(bool value) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(value);
	}

	void ReturnValue::set(double i) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::set(int i) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::set(uint32_t i) {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::setNull() {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetNull();
	}

	void ReturnValue::setUndefined() {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetUndefined();
	}

	void ReturnValue::setEmptyString() {
		reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetEmptyString();
	}

	void ReturnValue::set(JSValue* value) {
		if (value)
			reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(Back(value));
		else
			setNull();
	}

	int FunctionCallbackInfo::length() const {
		return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->Length();
	}

	JSValue* FunctionCallbackInfo::operator[](int i) const {
		return Cast(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->operator[](i));
	}

	JSObject* FunctionCallbackInfo::thisObj() const {
		return Cast<JSObject>(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->This());
	}

	bool FunctionCallbackInfo::isConstructCall() const {
		return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->IsConstructCall();
	}

	ReturnValue FunctionCallbackInfo::returnValue() const {
		auto info = reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this);
		v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		auto _ = reinterpret_cast<ReturnValue*>(&rv);
		return *_;
	}

	JSObject* PropertyCallbackInfo::thisObj() const {
		return Cast<JSObject>(reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this)->This());
	}

	ReturnValue PropertyCallbackInfo::returnValue() const {
		auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
		v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		auto _ = reinterpret_cast<ReturnValue*>(&rv);
		return *_;
	}

	JSObject* PropertySetCallbackInfo::thisObj() const {
		return Cast<JSObject>(reinterpret_cast<const v8::PropertyCallbackInfo<void>*>(this)->This());
	}

	Worker* FunctionCallbackInfo::worker() const {
		if (first_worker)
			return first_worker;
		auto info = reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

	Worker* PropertyCallbackInfo::worker() const {
		if (first_worker)
			return first_worker;
		auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

	Worker* PropertySetCallbackInfo::worker() const {
		if (first_worker)
			return first_worker;
		auto info = reinterpret_cast<const v8::PropertyCallbackInfo<void>*>(this);
		return WorkerImpl::worker(info->GetIsolate());
	}

}}
