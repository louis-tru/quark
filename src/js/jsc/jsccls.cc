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

	struct Factorys {
		static JSClassRef constructor;
		static JSClassRef object;
		static JSClassRef prototype;
		static JSClassRef function;
		static JSClassRef accessorGet;
		static JSClassRef accessorGet;
	} static factorys = {0};

	constexpr int FunctionPrivateMark = 125894334;

	template<typename Func>
	struct FunctionPrivate {
		Func f;
		JSCStringPtr name;
		JscWorker *worker;
		JscClass *sign;
		int _mark = FunctionPrivateMark;
	};

#if DEBUG
	struct DebugPrivate {
		MixObject* mix;
		bool isWeak;
	};
#endif

	void MixObject::clearWeak() {
		Qk_Assert_Ne(_handle, nullptr);
#if DEBUG
		auto priv = static_cast<DebugPrivate*>(JSObjectGetPrivate(_handle));
		Qk_Assert_Eq(priv->isWeak, true);
		priv->isWeak = false;
#endif
		JSValueProtect(JSC_CTX(_class->worker()), Back<JSObjectRef>(_handle));
	}

	void MixObject::setWeak() {
		Qk_Assert_Ne(_handle, nullptr);
#if DEBUG
		auto priv = static_cast<DebugPrivate*>(JSObjectGetPrivate(_handle));
		Qk_Assert_Eq(priv->isWeak, false);
		priv->isWeak = true;
#endif
		JSValueUnprotect(JSC_CTX(_class->worker()), Back<JSObjectRef>(_handle));
	}

	void MixObject::bindObject(JSObject* handle) {
		Qk_Assert_Eq(_handle, nullptr);
		_handle = handle;
		auto val = Back<JSObjectRef>(_handle);
		void* priv = this;
#if DEBUG
		priv = new DebugPrivate{this,false};
#endif
		JSObjectSetPrivate(val, priv);
		JSValueProtect(JSC_CTX(_class->worker()), val);
	}

	class JscClass: public JSClass {
	public:
		#define _jscclass(v) static_cast<JscClass*>(v)

		static void DestructorObject(JSObjectRef object) {
			auto mix = static_cast<MixObject*>(JSObjectGetPrivate(object));
			Qk_Assert_Ne(mix, nullptr);
#if DEBUG
			auto priv = reinterpret_cast<DebugPrivate*>(mix);
			Qk_Assert_Eq(priv->isWeak, true);
			mix = priv->mix;
			delete priv;
#endif
			mix->~MixObject(); // destroy mix
			mix->self()->destroy(); // destroy object
		}

		static void DestructorFunction(JSObjectRef object) {
			delete static_cast<FunctionPrivate<void>*>(JSObjectGetPrivate(object));
		}

		static JSObjectRef Constructor(JSContextRef ctx, JSObjectRef constructor, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			auto cls = static_cast<JscClass*>(JSObjectGetPrivate(constructor));
			auto obj = JSObjectMake(ctx, objectFactory, nullptr);
			JSObjectSetPrototype(ctx, obj, cls->_prototype); // set object __proto__

			auto worker = WORKER(cls->worker());
			FunctionCallbackInfoImpl args{ worker, obj, worker->_data.Undefined, argv, argc, true };
			worker->_callStack++;
			cls->callConstructor(*reinterpret_cast<FunctionCallbackInfo*>(&args)); // class native constructor
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return obj;
		}

		static JSClass* checkIndexedCall(JSObjectRef thisObj, JSStringRef name, bool hasGet, uint32_t &idx, JSValueRef* ex) {
			auto mix = static_cast<MixObject*>(JSObjectGetPrivate(thisObj));
			if (mix && reinterpret_cast<FunctionPrivate<void>*>(mix)->_mark != FunctionPrivateMark) {
				auto cls = mix->jsclass();
				Qk_Assert_Ne(cls, nullptr);

				if (hasGet ? cls->_indexedGet: cls->_indexedSet) {
					char buffer[10];
					auto size = JSStringGetUTF8CString(name, buffer, 10);
					uint32_t idx;
					if (_Str::toNumber(buffer, size, 1, &idx)) {
						return cls;
					}
				}
			}
			return nullptr;
		}

		static JSValueRef IndexedGet(JSContextRef ctx, JSObjectRef thisObj, JSStringRef name, JSValueRef* ex) {
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

		static bool checkCallSign(JSObjectRef thisObj, JSClass *sign, JSValueRef* ex) {
			auto mix = static_cast<MixObject*>(JSObjectGetPrivate(thisObj));
			auto worker = WORKER(mix->worker());
			// TODO ...
			worker->throwException(worker->newErrorJsc("Illegal call"));
			return false;
		}

		static JSValueRef Function(JSContextRef ctx, JSObjectRef func, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			auto func = static_cast<FunctionPrivate<FunctionCallback>*>(JSObjectGetPrivate(func));
			if (func->sign && !checkCallSign(thisObj, func->sign, ex))
				return nullptr;

			JscWorker* worker = func->worker;
			FunctionCallbackInfoImpl args{ worker, thisObj, worker->_data.Undefined, argv, argc, false };

			worker->_callStack++;
			func->f(*_info_func(&args));
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return args._return;
		}

		static JSValueRef FunctionGet(JSContextRef ctx, JSObjectRef func, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			auto func = static_cast<FunctionPrivate<AccessorGetterCallback>*>(JSObjectGetPrivate(func));
			if (func->sign && !checkCallSign(thisObj, func->sign, ex))
				return nullptr;

			JscWorker* worker = func->worker;
			PropertyCallbackInfoImpl args{ worker, thisObj, worker->_data.Undefined };

			worker->_callStack++;
			func->f(Cast(JSValueMakeString(ctx, *func->name)), *_info_get(&args));
			worker->_callStack--;
			DCHECK(worker->_callStack >= 0);
			*ex = worker->_ex; // maybe have throw error on sub call
			worker->_ex = nullptr;

			return args._return;
		}

		static JSValueRef FunctionSet(JSContextRef ctx, JSObjectRef func, JSObjectRef thisObj, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
			auto func = static_cast<FunctionPrivate<AccessorSetterCallback>*>(JSObjectGetPrivate(func));
			if (func->sign && !checkCallSign(thisObj, func->sign, ex))
				return nullptr;

			Qk_Assert_Eq(argc, 1);

			JscWorker* worker = func->worker;
			PropertySetCallbackInfoImpl args{ worker, thisObj };

			worker->_callStack++;
			func->f(Cast(JSValueMakeString(ctx, *func->name)), Cast(argv[0]), *_info_set(&args));
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
			, _base(base)
			, _baseFunc(baseFunc)
			, _constructor(nullptr)
			, _prototype(nullptr), _indexedGet(nullptr), _indexedSet(nullptr)
		{
			ENV(w);
			_worker = worker;

			if (_base) {
			} else if (_baseFunc && JSObjectIsConstructor(ctx, _baseFunc)) {
				_base = static_cast<JscClass*>(JSObjectGetPrivate(_baseFunc));
				if (!_base)
					JSValueProtect(ctx, _baseFunc);
			} {
				_base = worker->_base;
			}

			_constructor = JSObjectMake(ctx, factorys.constructor, this);
			_prototype = JSObjectGetProperty(ctx, _constructor, prototype_s, ex);
			Qk_Assert_Eq(ex, nullptr);

			auto s = JSValueMakeString(ctx, JsStringWithUTF8(*name));
			JSObjectSetProperty(ctx, _constructor, name_s, s, ex);
			Qk_Assert_Eq(ex, nullptr);

			// Inherit
			if (_base) {
				_indexedGet = _base->_indexedGet;
				_indexedSet = _base->_indexedSe;
				JSObjectSetPrototype(ctx, _prototype, _base->_prototype);
				JSObjectSetPrototype(ctx, _constructor, _base->_constructor);
			} else if (_baseFunc) {
				auto baseProrotype = JSObjectGetProperty(ctx, _baseFunc, prototype_s, ex);
				Qk_Assert_Eq(ex, nullptr);
				JSObjectSetPrototype(ctx, _prototype, baseProrotype);
				JSObjectSetPrototype(ctx, _constructor, _baseFunc);
			} else { // base object
				_prototype = JSObjectMake(ctx, factorys.prototype, nullptr); // new prototype
				JSObjectSetPrototype(ctx, _prototype, worker->_data.Object_prototype); // Inherit from Object
				JSObjectSetProperty(ctx, _prototype, constructor_s, _constructor, ex);
				Qk_Assert_Eq(ex, nullptr);
			}

			JSValueProtect(ctx, _constructor);
		}

		~JscClass() {
			ENV(_worker);
			DCHECK(worker->hasDestroy());
			if (_baseFunc) {
				JSValueUnprotect(ctx, _baseFunc);
			}
			JSValueUnprotect(ctx, _constructor);
		}

	private:
		JscClass   *_base;
		JSObjectRef _baseFunc, _constructor, _prototype;
		IndexedAccessorGetterCallback _indexedGet;
		IndexedAccessorSetterCallback _indexedSet;
		friend class JSClass;
	};

	void JscWorker::initBase() {
		_base = new JscClass(this, "base", [](auto e){}, [](auto e){}, nullptr, nullptr);
	}

	JSFunction* JSClass::getFunction() {
		return Cast<JSFunction>(_jscclass(this)->_constructor);
	}

	bool JSClass::hasInstance(JSValue* val) {
		Qk_Assert_Ne(val, nullptr);
		ENV(_worker);
		auto ok = JSValueIsInstanceOfConstructor(ctx, val, _jscclass(this)->_constructor, OK(false));
		return ok;
	}

	static bool setMethodFunction(
		Worker* w, JSObjectRef target, cString& name, FunctionCallback func, JSClass* sign
	) {
		Qk_Assert_Ne(func, nullptr);
		ENV(w);
		auto s = JsStringWithUTF8(*name);
		auto f = JSObjectMake(ctx, factorys.function, new FunctionPrivate<FunctionCallback>{func, s, worker, sign});
		DCHECK(f);
		auto ok = JSObjectSetProperty(ctx, target, JSValueMakeString(ctx, s), f, nullptr, OK(false));
		return ok;
	}

	static bool setAccessorFunction(
		Worker* w, JSObjectRef target, cString& name,
		AccessorGetterCallback get, AccessorSetterCallback set, JSClass* sign
	) {
		Qk_Assert(get || set);
		ENV(w);

		auto info = JSObjectMake(ctx, 0, OK(false));
		auto s = JsStringWithUTF8(*name);

		if (get) {
			auto f = new FunctionPrivate<AccessorGetterCallback>{get, s, worker, sign};
			auto getf = JSObjectMake(ctx, factorys.accessorGet, f);
			DCHECK(getf);
			JSObjectSetProperty(ctx, info, get_s, getf, nullptr, OK(false));
		}
		if (set) {
			auto f = new FunctionPrivate<AccessorSetterCallback>{set, s, worker, sign};
			auto setf = JSObjectMake(ctx, factorys.accessorSet, f);
			DCHECK(setf);
			JSObjectSetProperty(ctx, info, set_s, setf, nullptr, OK(false));
		}
		//JSObjectSetProperty(ctx, info, enumerable_s, worker->_data.True, nullptr, OK(false));
		//JSObjectSetProperty(ctx, info, configurable_s, worker->_data.True, nullptr, OK(false));

		JSValueRef args[3] = { target, JSValueMakeString(ctx, s), info };
		JSObjectCallAsFunction(ctx, worker->_data.Object_defineProperty, nullptr, 3, args, OK(false));

		return true;
	}

	bool JSObject::setMethod(Worker* w, cString& name, FunctionCallback func) {
		return setMethodFunction(w, Back<JSObjectRef>(this), name, func, nullptr);
	}

	bool JSObject::setAccessor(Worker* w, cString& name, AccessorGetterCallback get, AccessorSetterCallback set) {
		return setAccessorFunction(w, Back<JSObjectRef>(this), name, get, set, nullptr);
	}

	bool JSClass::setMethod(cString& name, FunctionCallback func) {
		return setMethodFunction(w, _jscclass(this)->_prototype, name, func, this);
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
		return Cast<JSObject>(_jscclass(this)->_prototype)->setProperty(_worker, name, value);
	}

	bool JSClass::setStaticMethod(cString& name, FunctionCallback func) {
		return Cast<JSObject>(_jscclass(this)->_constructor)->setMethod(_worker, name, func);
	}

	template<>
	bool JSClass::setStaticProperty<JSValue*>(cString& name, JSValue* value) {
		return Cast<JSObject>(_jscclass(this)->_constructor)->setProperty(_worker, name, value);
	}

	JSClass* Worker::newClass(cString& name, uint64_t alias,
															FunctionCallback constructor,
															AttachCallback attach, JSClass* base) {
		auto cls = new JscClass(this, name, constructor, attach, static_cast<JscClass*>(base), nullptr);
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
		auto cls = new JscClass(this, name, constructor, attach, nullptr, Back<JSObjectRef>(base));
		_classes->add(alias, cls);
		return cls;
	}

	JSFunction* Worker::newFunction(cString& name, FunctionCallback func) {
		Qk_Assert_Ne(func, nullptr);
		auto f = JSObjectMake(JSC_CTX(this), factorys.function, func);
		DCHECK(f);
		worker->addToScope(f);
		return Cast<JSFunction>(f);
	}

	void initFactorys() {
		Qk_Assert_Eq(factorys.constructor, nullptr);
		JSClassDefinition def;

		def = kJSClassDefinitionEmpty;
		def.callAsConstructor = JscClass::Constructor;
		factorys.constructor = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.finalize = JscClass::DestructorObject;
		factorys.object = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.getProperty = JscClass::IndexedGet;
		def.setProperty = JscClass::IndexedSet;
		factorys.prototype = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.finalize = JscClass::DestructorFunction;
		def.callAsFunction = JscClass::Function;
		factorys.function = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.callAsFunction = JscClass::FunctionGet;
		factorys.accessorGet = JSClassCreate(&def);

		def = kJSClassDefinitionEmpty;
		def.callAsFunction = JscClass::FunctionSet;
		factorys.accessorSet = JSClassCreate(&def);
	}

	// ------------------- F u n c t i o n . C a l l b a c k . I n f o -------------------

	void ReturnValue::set(bool value) {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = value ? impl->_worker->_data.True: impl->_worker->_data.False;
	}

	void ReturnValue::set(double i) {
		auto impl = reinterpret_cast<ReturnValueImpl*>(_val);
		impl->_return = JSValueMakeNumber(JSC_CTX(impl->_worker), i);
	}

	void ReturnValue::set(int i) {
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

	void ReturnValue::set(JSValue* value) {
		if (value)
			reinterpret_cast<ReturnValueImpl*>(_val)->_return = Back(value);
		else
			setNull();
	}

	int FunctionCallbackInfo::length() const {
		return reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->argc;
	}

	JSValue* FunctionCallbackInfo::operator[](int i) const {
		Qk_Assert_Lt(i, reinterpret_cast<const FunctionCallbackInfoImpl*>(this)->argc,
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
		return reinterpret_cast<ReturnValue>(this);
	}

	JSObject* PropertyCallbackInfo::thisObj() const {
		return Cast<JSObject>(reinterpret_cast<const PropertyCallbackInfoImpl*>(this)->_this);
	}

	ReturnValue PropertyCallbackInfo::returnValue() const {
		return reinterpret_cast<ReturnValue>(this);
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