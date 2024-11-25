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

#include "./jsc.h"

namespace qk { namespace js {

	class JscClass: public JSClass {
	public:
		JscClass(Worker* worker, cString& name,
							FunctionCallback constructor, AttachCallback attach, V8JSClass* base,
							v8::Local<v8::Function> baseFunc = v8::Local<v8::Function>())
			: JSClass(constructor, attach), _base(base)
		{
			_worker = worker;
			auto data = External::New(ISOLATE(worker), this);
			auto cb = (v8::FunctionCallback)[](const v8::FunctionCallbackInfo<v8::Value>& info) {
				auto self = (V8JSClass*)info.Data().As<External>()->Value();
				if (!self->_worker->classses()->isAttachFlag()) {
					self->_constructor(*reinterpret_cast<const FunctionCallbackInfo*>(&info));
				}
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
		// if (_func.isEmpty()) {
		// 	// Gen constructor
		// 	auto v8cls = static_cast<V8JSClass*>(this);
		// 	auto f = v8cls->Template()->GetFunction(CONTEXT(_worker)).ToLocalChecked();
		// 	if (v8cls->HasBaseFunction()) {
		// 		bool ok;
		// 		// function.__proto__ = base;
		// 		// f->SetPrototype(v8cls->BaseFunction());
		// 		// function.prototype.__proto__ = base.prototype;
		// 		auto str = Back(_worker->strs()->prototype());
		// 		auto base = v8cls->BaseFunction();
		// 		auto proto = f->Get(CONTEXT(_worker), str).ToLocalChecked().As<v8::Object>();
		// 		auto baseProto = base->Get(CONTEXT(_worker), str).ToLocalChecked().As<v8::Object>();
		// 		ok = proto->SetPrototype(CONTEXT(_worker), baseProto).ToChecked();
		// 		Qk_Assert(ok);
		// 	}
		// 	auto func = Cast<JSFunction>(f);
		// 	_func.reset(_worker, func);
		// }
		// return *_func;
	}

	bool JSClass::hasInstance(JSValue* val) {
		// return reinterpret_cast<V8JSClass*>(this)->Template()->HasInstance(Back(val));
	}

	bool JSClass::setMemberMethod(cString& name, FunctionCallback func) {
		// v8::Local<v8::FunctionTemplate> ftemp = reinterpret_cast<V8JSClass*>(this)->Template();
		// v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		// v8::Local<Signature> sign = Signature::New(ISOLATE(_worker), ftemp);
		// v8::Local<v8::FunctionTemplate> t =
		// 	FunctionTemplate::New(ISOLATE(_worker), func2, v8::Local<v8::Value>(), sign);
		// v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		// t->SetClassName(fn_name);
		// ftemp->PrototypeTemplate()->Set(fn_name, t);
		// return true;
	}

	bool JSClass::setMemberAccessor(cString& name,
																	AccessorGetterCallback get, AccessorSetterCallback set) {
		// v8::Local<v8::FunctionTemplate> temp = reinterpret_cast<V8JSClass*>(this)->Template();
		// v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		// v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		// v8::Local<AccessorSignature> sign = AccessorSignature::New(ISOLATE(_worker), temp);
		// v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		// temp->PrototypeTemplate()->SetAccessor(fn_name, get2, set2,
		// 																			v8::Local<v8::Value>(), v8::DEFAULT, v8::None, sign);
		// return true;
	}

	/*
	bool JSClass::setLazyDataProperty(cString& name, AccessorGetterCallback get) {
		v8::Local<v8::FunctionTemplate> temp = reinterpret_cast<V8JSClass*>(this)->Template();
		v8::AccessorGetterCallback get2 = reinterpret_cast<v8::AccessorGetterCallback>(get);
		// v8::AccessorSetterCallback set2 = reinterpret_cast<v8::AccessorSetterCallback>(set);
		v8::Local<AccessorSignature> s = AccessorSignature::New(ISOLATE(_worker), temp);
		v8::Local<v8::String> fn_name = Back<v8::String>(_worker->newStringOneByte(name));
		temp->PrototypeTemplate()->SetAccessor(fn_name, get2, nullptr,
																					v8::Local<v8::Value>(), v8::DEFAULT, v8::None, s);
		return true;
	}*/

	bool JSClass::setMemberIndexedAccessor(IndexedAccessorGetterCallback get,
																				IndexedAccessorSetterCallback set) {
		// v8::IndexedPropertyGetterCallback get2 = reinterpret_cast<v8::IndexedPropertyGetterCallback>(get);
		// v8::IndexedPropertySetterCallback set2 = reinterpret_cast<v8::IndexedPropertySetterCallback>(set);
		// v8::IndexedPropertyHandlerConfiguration cfg(get2, set2);
		// reinterpret_cast<V8JSClass*>(this)->Template()->PrototypeTemplate()->SetHandler(cfg);
		// return true;
	}

	template<>
	bool JSClass::setMemberProperty<JSValue*>(cString& name, JSValue* value) {
		// reinterpret_cast<V8JSClass*>(this)->Template()->
		// 				PrototypeTemplate()->Set(Back<v8::String>(_worker->newStringOneByte(name)), Back(value));
		// return true;
	}

	template<>
	bool JSClass::setStaticProperty<JSValue*>(cString& name, JSValue* value) {
		// reinterpret_cast<V8JSClass*>(this)->Template()->
		// 				Set(Back<v8::String>(_worker->newStringOneByte(name)), Back(value));
		// return true;
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
															FunctionCallback constructor,
															AttachCallback attach, JSClass* base) {
		// auto cls = new V8JSClass(this, name, constructor, attach, static_cast<V8JSClass*>(base));
		// _classses->add(id, cls);
		// return cls;
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach, uint64_t base) {
		// return newClass(name, id, constructor, attach, _classses->get(base));
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
															FunctionCallback constructor,
															AttachCallback attach, JSFunction* base) {
		// auto cls = new V8JSClass(this, name, constructor, attach, nullptr, Back<v8::Function>(base));
		// _classses->add(id, cls);
		// return cls;
	}

	JSFunction* Worker::newFunction(cString& name, FunctionCallback func) {
		// TODO ...
		return nullptr;
	}

}}