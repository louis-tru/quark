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

	class JscPrivateData: SafeFlag {
	public:
		JscPrivateData() {
		}
	private:
		void *_data;
	};

	class JscClass: public JSClass {
	public:
		JscClass(Worker* worker, cString& name,
						FunctionCallback constructor,
						AttachCallback attach, JscClass* base, JSObjectRef baseFunc)
			: JSClass(constructor, attach)
			, _jsclass(nullptr)
			, _base(base)
			, _baseFunc(baseFunc)
			, _prototype(nullptr)
		{
			_worker = worker;
			auto ctx = CONTEXT(worker);
			if (_baseFunc) {
				JSValueProtect(ctx, _baseFunc);
			}
			JSClassDefinition def = kJSClassDefinitionEmpty;

			def.callAsFunction = [](JSContextRef ctx, JSObjectRef func, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
				auto priv = static_cast<JscPrivateData*>(JSObjectGetPrivate(thisObject));
				if (!priv->isValid()) {
					
				}
				return nullptr;
			};

			def.callAsConstructor = [](JSContextRef ctx, JSObjectRef constr, size_t argc, const JSValueRef argv[], JSValueRef* ex) {
				return nullptr;
			};

			def.hasProperty = [](JSContextRef ctx, JSObjectRef self, JSStringRef name) {
				return false;
			};

			def.getProperty = [](JSContextRef ctx, JSObjectRef self, JSStringRef name, JSValueRef* ex) {
				return nullptr;
			};

			def.setProperty = [](JSContextRef ctx, JSObjectRef self, JSStringRef name, JSValueRef value, JSValueRef* ex) {
				return false;
			};

			def.deleteProperty = [](JSContextRef ctx, JSObjectRef self, JSStringRef name, JSValueRef* ex) {
				return false;
			};

			def.getPropertyNames = [](JSContextRef ctx, JSObjectRef self, JSPropertyNameAccumulatorRef names) {
			};

			_jsclass = JSClassCreate(&def);
			_jsclass = JSClassRetain(_jsclass);
		}
		~JscClass() {
			ENV(_worker);
			DCHECK(worker->hasDestroy());

			if (_jsclass) {
				JSClassRelease(_jsclass);
			}
			if (_baseFunc) {
				JSValueUnprotect(ctx, _baseFunc);
			}
			if (_func) {
				JSValueUnprotect(ctx, _func);
			}
		}
	private:
		JSClassRef  _jsclass;
		JscClass   *_base;
		JSObjectRef _baseFunc;
		JSObjectRef _prototype;
		friend class JSClass;
	};

	JSFunction* JSClass::getFunction() {
		// ...
	}

	bool JSClass::hasInstance(JSValue* val) {
		// TODO ...
	}

	bool JSClass::setMethod(cString& name, FunctionCallback func) {
		// To prototype
	}

	bool JSClass::setAccessor(cString& name,
														AccessorGetterCallback get, AccessorSetterCallback set) {
		// To prototype
	}

	bool JSClass::setIndexedAccessor(IndexedAccessorGetterCallback get,
																	IndexedAccessorSetterCallback set) {
		// To prototype
	}

	template<>
	bool JSClass::setProperty<JSValue*>(cString& name, JSValue* value) {
		// To prototype
	}

	bool JSClass::setStaticMethod(cString& name, FunctionCallback func) {
		// To func
	}

	template<>
	bool JSClass::setStaticProperty<JSValue*>(cString& name, JSValue* value) {
		// To func
	}

	JSClass* Worker::newClass(cString& name, uint64_t alias,
															FunctionCallback constructor,
															AttachCallback attach, JSClass* base) {
		auto cls = new JscClass(this, name, constructor, attach, static_cast<JscClass*>(base), NULL);
		_classses->add(alias, cls);
		return cls;
	}

	JSClass* Worker::newClass(cString& name, uint64_t alias,
																	FunctionCallback constructor,
																	AttachCallback attach, uint64_t base) {
		return newClass(name, alias, constructor, attach, _classses->get(base));
	}

	JSClass* Worker::newClass(cString& name, uint64_t alias,
															FunctionCallback constructor,
															AttachCallback attach, JSFunction* base) {
		auto cls = new JscClass(this, name, constructor, attach, NULL, Back<JSObjectRef>(base));
		_classses->add(alias, cls);
		return cls;
	}

	JSFunction* Worker::newFunction(cString& name, FunctionCallback func) {
		// TODO ...
		return nullptr;
	}

}}