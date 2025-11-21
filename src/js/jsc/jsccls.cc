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

	struct factories {
		JSClassRef constructor;
		JSClassRef object;
		JSClassRef objectWithIndexed;
		JSClassRef function;
		JSClassRef accessorGet;
		JSClassRef accessorSet;
	} static factories = {0};

	constexpr int FunctionPrivateMark = 125894334;

	template<typename Func>
	struct FunctionPrivate {
		Func f;
		JSCStringPtr name;
		JscWorker *worker;
		JscClass *signature;
		int _mark = FunctionPrivateMark;
	};

	enum Flags {
		kWeak_Flags = (1 << 0),
	};

	void MixObject::clearWeak() {
		Qk_ASSERT_NE(_handle, nullptr);
		if (_flags & kWeak_Flags) {
			_flags &= ~kWeak_Flags; // delete kWeak_Flags
			JSValueProtect(JSC_CTX(_class->worker()), Back<JSObjectRef>(_handle));
		}
	}

	void MixObject::setWeak() {
		Qk_ASSERT_NE(_handle, nullptr);
		if (!(_flags & kWeak_Flags)) {
			_flags |= kWeak_Flags; // add kWeak_Flags
			JSValueUnprotect(JSC_CTX(_class->worker()), Back<JSObjectRef>(_handle));
		}
	}

	void MixObject::bindObject(JSObject* handle) {
		Qk_ASSERT_EQ(_handle, nullptr);
		_handle = handle;
		auto val = Back<JSObjectRef>(_handle);
		JSObjectSetPrivate(val, this);
		JSValueProtect(JSC_CTX(_class->worker()), val);
	}

	MixObject* MixObject::unpack(JSValue* obj) {
		Qk_ASSERT_NE(obj, nullptr);
		auto mix = static_cast<MixObject*>(JSObjectGetPrivate(Back<JSObjectRef>(obj)));
		return mix;
	}

	static MixObject* getMix(JSObjectRef obj) {
		auto priv = JSObjectGetPrivate(obj);
		if (priv && reinterpret_cast<FunctionPrivate<void*>*>(priv)->_mark != FunctionPrivateMark) {
			return static_cast<MixObject*>(priv);
		}
		return nullptr;
	}

	class JscClass: public JSClass {
	public:
		#define _jscclass(v) static_cast<JscClass*>(v)

		static void DestructorObject(JSObjectRef object) {
			auto mix = static_cast<MixObject*>(JSObjectGetPrivate(object));
			DCHECK(mix);
			if (mix->flags() & kWeak_Flags) {
				mix->~MixObject(); // destroy mix
				mix->self()->destroy(); // destroy object
			}
		}

		static void DestructorFunction(JSObjectRef object) {
			delete static_cast<FunctionPrivate<void*>*>(JSObjectGetPrivate(object));
		}
		
		static JSValueRef ConstructorFunc(JSContextRef ctx, JSObjectRef f, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			DCHECK(argc > 0);
			thisObj = (JSObjectRef)argv[0];
			DCHECK(HasInstanceOf(ctx, f, thisObj, 0));
			auto cls = static_cast<JscClass*>(JSObjectGetPrivate(f));
			auto indexed = cls->_indexedGet || cls->_indexedSet;
			auto obj = JSObjectMake(ctx, indexed ? factories.objectWithIndexed: factories.object, nullptr);
			JSObjectSetPrototype(ctx, obj, JSObjectGetPrototype(ctx, thisObj)); // set object __proto__

			auto worker = WORKER(cls->worker());
			FunctionCallbackInfoImpl args{ worker, obj, worker->_data.Undefined, argv+1, int(argc-1), true };
			worker->_callStack++;
			cls->callConstructor(*reinterpret_cast<FunctionCallbackInfo*>(&args)); // class native constructor
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return obj;
		}

		static bool HasInstanceOf(JSContextRef ctx, JSObjectRef fun, JSValueRef val, JSValueRef* ex) {
			if (JSValueIsObject(ctx, val)) {
				auto cls = static_cast<JscClass*>(JSObjectGetPrivate(fun));
				DCHECK(cls);
				auto to = cls->_prototype;
				auto obj = (JSObjectRef)val;
				do {
					obj = (JSObjectRef)JSObjectGetPrototype(ctx, obj); // obj.__proto__
					if (obj == to)
						return true;
				} while (JSValueIsObject(ctx, obj));
			}
			return false;
		}

		static JscClass* checkIndexedCall(JSObjectRef thisObj, JSStringRef name, bool hasGet, uint32_t &idx, JSValueRef* ex) {
			auto mix = getMix(thisObj);
			if (mix) {
				auto cls = static_cast<JscClass*>(mix->jsclass());
				Qk_ASSERT_NE(cls, nullptr);

				if (hasGet ? (void*)cls->_indexedGet: (void*)cls->_indexedSet) {
					char buffer[10];
					int size = JSStringGetUTF8CString(name, buffer, 10);
					if (_Str::toNumber(buffer, size, 1, &idx)) {
						return cls;
					}
				}
			}
			return nullptr;
		}

		static JSValueRef IndexedGet(JSContextRef ctx, JSObjectRef thisObj, JSStringRef name, JSValueRef* ex) {
			//Qk_DLog("IndexedGet,%s", *jsToString(name));
			uint32_t idx;
			auto cls = checkIndexedCall(thisObj, name, true, idx, ex);
			if (!cls) return nullptr;

			auto worker = WORKER(cls->worker());
			PropertyCallbackInfoImpl args{ worker, thisObj, worker->_data.Undefined };

			worker->_callStack++;
			cls->_indexedGet(idx, *reinterpret_cast<PropertyCallbackInfo*>(&args));
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return args._return;
		}

		static bool IndexedSet(JSContextRef ctx, JSObjectRef thisObj, JSStringRef name, JSValueRef value, JSValueRef* ex) {
			//Qk_DLog("IndexedSet,%s", *jsToString(name));
			uint32_t idx;
			auto cls = checkIndexedCall(thisObj, name, false, idx, ex);
			if (!cls) return false;

			auto worker = WORKER(cls->worker());
			PropertySetCallbackInfoImpl args{ worker, thisObj };

			worker->_callStack++;
			cls->_indexedSet(idx, Cast(value), *reinterpret_cast<PropertySetCallbackInfo*>(&args));
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return true;
		}

		static JSValueRef FunctionToString(JSContextRef ctx, JSObjectRef f, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			return JSValueMakeString(ctx, JsStringWithUTF8("function() { [native code] }").get());
		}

		static bool checkCallSign(JSObjectRef thisObj, JscClass *signature, JSValueRef* ex) {
			auto mix = getMix(thisObj);
			if (mix) {
				auto cls = static_cast<JscClass*>(mix->jsclass());
				DCHECK(cls);
				if (cls->isEqual(signature))
					return true;
			}
			auto worker = WORKER(signature->worker());
			*ex = worker->newErrorJsc("Calling signature is illegal");
			return false;
		}

		static JSValueRef Function(JSContextRef ctx, JSObjectRef f, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			auto func = static_cast<FunctionPrivate<FunctionCallback>*>(JSObjectGetPrivate(f));
			if (func->signature && !checkCallSign(thisObj, func->signature, ex))
				return nullptr;

			JscWorker* worker = func->worker;
			FunctionCallbackInfoImpl args{ worker, thisObj, worker->_data.Undefined, argv, int(argc), false };

			worker->_callStack++;
			func->f(*reinterpret_cast<FunctionCallbackInfo*>(&args));
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return args._return;
		}

		static JSValueRef FunctionGet(JSContextRef ctx, JSObjectRef f, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			auto func = static_cast<FunctionPrivate<AccessorGetterCallback>*>(JSObjectGetPrivate(f));
			if (func->signature && !checkCallSign(thisObj, func->signature, ex)) {
				//Qk_DLog("FunctionGet, %s", *jsToString(*func->name));
				return nullptr;
			}
			JscWorker* worker = func->worker;
			PropertyCallbackInfoImpl args{ worker, thisObj, worker->_data.Undefined };

			worker->_callStack++;
			func->f(Cast(JSValueMakeString(ctx, *func->name)), *reinterpret_cast<PropertyCallbackInfo*>(&args));
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return args._return;
		}

		static JSValueRef FunctionSet(JSContextRef ctx, JSObjectRef f, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			auto func = static_cast<FunctionPrivate<AccessorSetterCallback>*>(JSObjectGetPrivate(f));
			if (func->signature && !checkCallSign(thisObj, func->signature, ex)) {
				//Qk_DLog("FunctionSet, %s", *jsToString(*func->name));
				return nullptr;
			}
			Qk_ASSERT_EQ(argc, 1);

			JscWorker* worker = func->worker;
			PropertySetCallbackInfoImpl args{ worker, thisObj };

			worker->_callStack++;
			func->f(Cast(JSValueMakeString(ctx, *func->name)), Cast(argv[0]), *reinterpret_cast<PropertySetCallbackInfo*>(&args));
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return worker->_data.Undefined;
		}

		JscClass(Worker* w, cString& name,
						FunctionCallback constructor,
						AttachCallback attach, JscClass* base, JSObjectRef baseFunc)
			: JSClass(constructor, attach)
			, _name(name)
			, _base(base)
			, _baseFunc(baseFunc)
			, _constructor(nullptr)
			, _prototype(nullptr), _indexedGet(nullptr), _indexedSet(nullptr)
		{
			ENV(w);
			_worker = worker;

			JSValueRef args[] = {
				JSValueMakeString(ctx, JsStringWithUTF8(name.c_str()).get()),
				JSObjectMake(ctx, factories.constructor, this),
			};
			_constructor = (JSObjectRef)JSObjectCallAsFunction(ctx, worker->_data.MakeConstructor, 0, 2, args, 0);
			DCHECK(JSValueIsObject(ctx, _constructor));
			_prototype = (JSObjectRef)JSObjectGetProperty(ctx, _constructor, prototype_s, 0);
			DCHECK(JSValueIsObject(ctx, _prototype));

			if (!_base) {
				if (_baseFunc && JSObjectIsConstructor(ctx, _baseFunc)) {
					_base = static_cast<JscClass*>(JSObjectGetPrivate(_baseFunc));
				}
			}

			if (_base) {
				_indexedGet = _base->_indexedGet;
				_indexedSet = _base->_indexedSet;
				_parents = _base->_parents;
				_parents.add(_base);
				DCHECK(JSValueIsObject(ctx, _base->_prototype));
				JSObjectSetPrototype(ctx, _prototype, _base->_prototype); // set __proto__
				JSObjectSetPrototype(ctx, _constructor, _base->_constructor); // set __proto__
			} else if (_baseFunc) {
				auto baseProrotype = JSObjectGetProperty(ctx, _baseFunc, prototype_s, &ex);
				DCHECK(!ex);
				DCHECK(JSValueIsObject(ctx, baseProrotype));
				JSObjectSetPrototype(ctx, _prototype, baseProrotype); // set __proto__
				JSObjectSetPrototype(ctx, _constructor, _baseFunc); // set __proto__
			}
			//Qk_DLog(jsToString(ctx, _prototype));
			//Qk_DLog(jsToString(ctx, _constructor));

			if (_baseFunc)
				JSValueProtect(ctx, _baseFunc);
			JSValueProtect(ctx, _prototype);
			JSValueProtect(ctx, _constructor);
		}

		bool isEqual(JscClass* cls) {
			if (cls == this)
			 	return true;
			return _parents.has(cls);
		}

		void destroy() override {
			ENV(_worker);
			if (_baseFunc)
				JSValueUnprotect(ctx, _baseFunc);
			JSValueUnprotect(ctx, _prototype);
			JSValueUnprotect(ctx, _constructor);
		}

	private:
		String _name;
		JscClass*   _base;
		JSObjectRef _baseFunc;
		JSObjectRef _constructor;
		JSObjectRef _prototype;
		IndexedAccessorGetterCallback _indexedGet;
		IndexedAccessorSetterCallback _indexedSet;
		Set<JscClass*> _parents;
		friend class JSClass;
	};

	void JscClassReleasep(JscClass *&cls) {
		Releasep(cls);
	}

	JscClass* JscClassNew(JscWorker *worker) {
		return new JscClass(worker, "base", [](auto e){}, [](auto e){}, nullptr, nullptr);
	}

	JSFunction* JSClass::getFunction() {
		return Cast<JSFunction>(_jscclass(this)->_constructor);
	}

	bool JSClass::hasInstance(JSValue* val) {
		Qk_ASSERT_NE(val, nullptr);
		ENV(_worker);
		auto self = _jscclass(this);
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(val), self->_constructor, OK(false));
		return ok;
	}

	static bool setMethodFunction(
		Worker* w, JSObjectRef target, cString& name, FunctionCallback func, JSClass* sign
	) {
		Qk_ASSERT_NE(func, nullptr);
		ENV(w);
		auto s = JsStringWithUTF8(*name);
		auto fp = new FunctionPrivate<FunctionCallback>{func, s, worker, static_cast<JscClass*>(sign)};
		auto f = JSObjectMake(ctx, factories.function, fp);
		DCHECK(f);
		// Qk_DLog(jsToString(ctx, f));
		JSObjectSetProperty(ctx, target, *s, f, 0, OK(false));
		return true;
	}

	static bool setAccessorFunction(
		Worker* w, JSObjectRef target, cString& name, AccessorGetterCallback get, AccessorSetterCallback set, JSClass* sign
	) {
		Qk_ASSERT(get || set);
		ENV(w);

		if (!get) {
			get = [](auto name, auto args){};
		}
		if (!set) {
			set = [](auto name, auto value, auto args){};
		}

		auto info = JSObjectMake(ctx, 0, OK(false));
		auto s = JsStringWithUTF8(*name);

		auto fpg = new FunctionPrivate<AccessorGetterCallback>{get, s, worker, static_cast<JscClass*>(sign)};
		auto getf = JSObjectMake(ctx, factories.accessorGet, fpg);
		DCHECK(getf);
		JSObjectSetProperty(ctx, info, get_s, getf, 0, OK(false));

		auto fps = new FunctionPrivate<AccessorSetterCallback>{set, s, worker, static_cast<JscClass*>(sign)};
		auto setf = JSObjectMake(ctx, factories.accessorSet, fps);
		DCHECK(setf);
		JSObjectSetProperty(ctx, info, set_s, setf, 0, OK(false));

		//JSObjectSetProperty(ctx, info, enumerable_s, worker->data().True, 0, OK(false));
		//JSObjectSetProperty(ctx, info, configurable_s, worker->data().True, 0, OK(false));

		JSValueRef args[3] = { target, JSValueMakeString(ctx, *s), info };
		JSObjectCallAsFunction(ctx, worker->data().global_Object_defineProperty, nullptr, 3, args, OK(false));

		return true;
	}

	bool JSObject::setMethod(Worker* w, cString& name, FunctionCallback func) {
		return setMethodFunction(w, Back<JSObjectRef>(this), name, func, nullptr);
	}

	bool JSClass::setMethod(cString& name, FunctionCallback func) {
		return setMethodFunction(_worker, _jscclass(this)->_prototype, name, func, this);
	}

	bool JSObject::setAccessor(Worker* w, cString& name, AccessorGetterCallback get, AccessorSetterCallback set) {
		return setAccessorFunction(w, Back<JSObjectRef>(this), name, get, set, nullptr);
	}

	bool JSClass::setAccessor(cString& name, AccessorGetterCallback get, AccessorSetterCallback set) {
		return setAccessorFunction(_worker, _jscclass(this)->_prototype, name, get, set, this);
	}

	bool JSClass::setIndexedAccessor(IndexedAccessorGetterCallback get, IndexedAccessorSetterCallback set) {
		_jscclass(this)->_indexedGet = get;
		_jscclass(this)->_indexedSet = set;
		return true;
	}

	template<>
	bool JSClass::setProperty<JSValue*>(cString& name, JSValue* value) {
		return Cast<JSObject>(_jscclass(this)->_prototype)->set(_worker, name, value);
	}

	bool JSClass::setStaticMethod(cString& name, FunctionCallback func) {
		return Cast<JSObject>(_jscclass(this)->_constructor)->setMethod(_worker, name, func);
	}

	template<>
	bool JSClass::setStaticProperty<JSValue*>(cString& name, JSValue* value) {
		return Cast<JSObject>(_jscclass(this)->_constructor)->set(_worker, name, value);
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
															FunctionCallback constructor,
															AttachCallback attach, JSClass* base) {
		auto cls = new JscClass(this, name, constructor, attach, static_cast<JscClass*>(base), nullptr);
		_classes->add(id, cls);
		return cls;
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
																	FunctionCallback constructor,
																	AttachCallback attach, uint64_t base) {
		return newClass(name, id, constructor, attach, _classes->get(base));
	}

	JSClass* Worker::newClass(cString& name, uint64_t id,
															FunctionCallback constructor,
															AttachCallback attach, JSFunction* base) {
		auto cls = new JscClass(this, name, constructor, attach, nullptr, Back<JSObjectRef>(base));
		_classes->add(id, cls);
		return cls;
	}

	JSFunction* Worker::newFunction(cString& name, FunctionCallback func) {
		Qk_ASSERT_NE(func, nullptr);
		ENV(this);
		auto f = JSObjectMake(ctx, factories.function,
			new FunctionPrivate<FunctionCallback>{func, JsStringWithUTF8(*name), worker, nullptr}
		);
		DCHECK(f);
		return f ? worker->addToScope<JSFunction>(f): nullptr;
	}

	void initFactories() {
		Qk_ASSERT_EQ(factories.constructor, nullptr);
		JSClassDefinition def;

		def = kJSClassDefinitionEmpty;
		// def.hasInstance = JscClass::HasInstanceOf;
		def.callAsFunction = JscClass::ConstructorFunc;
		factories.constructor = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.finalize = JscClass::DestructorObject;
		factories.object = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.finalize = JscClass::DestructorObject;
		def.getProperty = JscClass::IndexedGet;
		def.setProperty = JscClass::IndexedSet;
		factories.objectWithIndexed = JSClassCreate(&def);

		JSStaticFunction staticFunctions[2] = {
			{"toString", JscClass::FunctionToString, kJSPropertyAttributeDontEnum},{0}
		};
		def = kJSClassDefinitionEmpty;
		def.finalize = JscClass::DestructorFunction;
		def.staticFunctions = staticFunctions;
		def.callAsFunction = JscClass::Function;
		factories.function = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.callAsFunction = JscClass::FunctionGet;
		factories.accessorGet = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.callAsFunction = JscClass::FunctionSet;
		factories.accessorSet = JSClassCreate(&def);
	}

	// ------------------- F u n c t i o n . C a l l b a c k . I n f o -------------------

	Worker* ReturnValue::worker() {
		return reinterpret_cast<ReturnValueImpl*>(_val)->_worker;
	}

	void ReturnValue::set(JSValue* value) {
		if (value)
			reinterpret_cast<ReturnValueImpl*>(_val)->_return = Back(value);
		else
			setNull();
	}

	void ReturnValue::set(bool value) {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = value ? impl->_worker->_data.True: impl->_worker->_data.False;
	}

	void ReturnValue::set(float i) {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = JSValueMakeNumber(JSC_CTX(impl->_worker), i);
	}

	void ReturnValue::set(double i) {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = JSValueMakeNumber(JSC_CTX(impl->_worker), i);
	}

	void ReturnValue::set(int32_t i) {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = JSValueMakeNumber(JSC_CTX(impl->_worker), i);
	}

	void ReturnValue::set(uint32_t i) {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = JSValueMakeNumber(JSC_CTX(impl->_worker), i);
	}

	void ReturnValue::setNull() {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = impl->_worker->_data.Null;
	}

	void ReturnValue::setUndefined() {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = impl->_worker->_data.Undefined;
	}

	void ReturnValue::setEmptyString() {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = impl->_worker->_data.EmptyString;
	}

	int FunctionCallbackInfo::length() const {
		return reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->argc;
	}

	JSValue* FunctionCallbackInfo::operator[](int i) const {
		Qk_ASSERT_GT(reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->argc, i,
			"argument index out of bounds");
		return Cast(reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->argv[i]);
	}

	JSObject* FunctionCallbackInfo::thisObj() const {
		return Cast<JSObject>(reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->_this);
	}

	bool FunctionCallbackInfo::isConstructCall() const {
		return reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->isConstructCall;
	}

	ReturnValue FunctionCallbackInfo::returnValue() const {
		return bitwise_cast<ReturnValue>(this);
	}

	JSObject* PropertyCallbackInfo::thisObj() const {
		return Cast<JSObject>(reinterpret_cast<const PropertyCallbackInfoImpl*>(this)->_this);
	}

	ReturnValue PropertyCallbackInfo::returnValue() const {
		return bitwise_cast<ReturnValue>(this);
	}

	JSObject* PropertySetCallbackInfo::thisObj() const {
		return Cast<JSObject>(reinterpret_cast<const PropertySetCallbackInfoImpl*>(this)->_this);
	}

	Worker* FunctionCallbackInfo::worker() const {
		return reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->_worker;
	}

	Worker* PropertyCallbackInfo::worker() const {
		return reinterpret_cast<const PropertyCallbackInfoImpl*>(this)->_worker;
	}

	Worker* PropertySetCallbackInfo::worker() const {
		return reinterpret_cast<const PropertySetCallbackInfoImpl*>(this)->_worker;
	}

}}
