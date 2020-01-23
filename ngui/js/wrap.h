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


#ifndef __ngui__js__wrap__
#define __ngui__js__wrap__

#include "ngui/js/js.h"

JS_BEGIN

template<class T> class Wrap;

/**
 * @class WrapObject
 */
class XX_EXPORT WrapObject {
	XX_HIDDEN_ALL_COPY(WrapObject);
 protected:
	
	inline WrapObject() {}
	
	/**
	 * @destructor
	 */
	~WrapObject();
	
	virtual void initialize();
	virtual void destroy();
	
	/**
	 * @func New()
	 */
	template<class W, class O>
	static Wrap<O>* New(FunctionCall args, O* object) {
		static_assert(sizeof(W) == sizeof(WrapObject),
									"Derived wrap class pairs cannot declare data members");
		static_assert(O::Traits::is_object, "Must be object");
		auto wrap = new(reinterpret_cast<WrapObject*>(object) - 1) W();
		wrap->init2(args);
		return static_cast<js::Wrap<O>*>(static_cast<WrapObject*>(wrap));
	}
	
	static WrapObject* attach(FunctionCall args);

 public:
	virtual bool addEventListener(cString& name, cString& func, int id) {
		return false;
	}
	virtual bool removeEventListener(cString& name, int id) {
		return false;
	}
  
	inline Worker* worker() {
		return handle_.worker_;
	}
	Object* privateData();
	bool setPrivateData(Object* data, bool trusteeship = false);
	
	inline Persistent<JSObject>& handle() {
		return handle_;
	}
	inline Local<JSObject> that() {
		return worker()->New(handle_);
	}
	inline Local<JSValue> get(Local<JSValue> key) {
		return handle_.local()->Get(worker(), key);
	}
	inline bool set(Local<JSValue> key, Local<JSValue> value) {
		return handle_.local()->Set(worker(), key, value);
	}
	inline bool del(Local<JSValue> key) {
		return handle_.local()->Delete(worker(), key);
	}
	
  // call member func
	Local<JSValue> call(Local<JSValue> name, int argc = 0, Local<JSValue> argv[] = nullptr);
	Local<JSValue> call(cString& name, int argc = 0, Local<JSValue> argv[] = nullptr);
	
	template<class T = Object>
	inline T* self() {
		return static_cast<T*>(reinterpret_cast<Object*>(this + 1));
	}
	
	// static
	
	static bool isPack(Local<JSObject> object);
	
	template<class T = Object>
	static inline Wrap<T>* unpack(Local<JSObject> value) {
		return static_cast<Wrap<T>*>(unpack2(value));
	}
	template<class T>
	static inline Wrap<T>* pack(T* object) {
		return static_cast<js::Wrap<T>*>(pack2(object, JS_TYPEID(*object)));
	}
	template<class T>
	static inline Wrap<T>* pack(T* object, uint64 type_id) {
		return static_cast<js::Wrap<T>*>(pack2(object, type_id));
	}
	
 private:
	static WrapObject* unpack2(Local<JSObject> object);
	static WrapObject* pack2(Object* object, uint64 type_id);
	void init2(FunctionCall args);

 protected:
	Persistent<JSObject> handle_;
	XX_DEFINE_INLINE_CLASS(Inl);
	friend class Allocator;
};

template<class T = Object>
class XX_EXPORT Wrap: public WrapObject {
	Wrap() = delete;
 public:
	inline static Wrap<T>* unpack(Local<JSObject> value) {
		return WrapObject::unpack<T>(value);
	}
	inline T* self() {
		return reinterpret_cast<T*>(this + 1);
	}
};

JS_END
#endif

