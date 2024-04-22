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

#ifndef __quark__js__js__
#define __quark__js__js__

#include "../util/util.h"
#include "../util/error.h"
#include "../util/json.h"
#include "../util/dict.h"
#include "../util/codec.h"
#include "../util/fs.h"
#include "../util/http.h"

// ------------- Js common macro -------------

#define Js_Worker(...)   auto worker = Worker::worker(__VA_ARGS__)
#define Js_Return(v)     return args.returnValue().set(worker->newInstance(v))
#define Js_Return_Null() return args.returnValue().setNull()
#define Js_Wrap(type)    auto wrap = qk::js::WrapObject::wrap<type>(args.This())
#define Js_Self(type)    auto self = qk::js::WrapObject::wrap<type>(args.This())->self()
#define Js_Handle_Scope() HandleScope scope(worker)
#define Js_Callback_Scope() CallbackScope cscope(worker)

#define Js_Throw_Error(Error, err, ...) \
	return worker->throwError(worker->new##Error((err), ##__VA_ARGS__))
#define Js_Throw(err, ...) Js_Throw_Error(Error, err, ##__VA_ARGS__)
#define Js_Try_Catch(block, Error) try block catch(const Error& e) { Js_Throw(e); }

#define Js_Set_Module(name, cls) \
	Qk_INIT_BLOCK(Js_Set_Module_##name) { \
		Qk_DEBUG("%s", "Worker_Set_MODULE "#name""); \
		qk::js::Worker::setModule(#name, cls::binding, __FILE__); \
	}
#define Js_Typeid(t) (typeid(t).hash_code())
#define Js_Type_Check(T, S)  \
	while (false) { \
		*(static_cast<T* volatile*>(0)) = static_cast<S*>(0); \
	}
// define class macro
#define Js_Define_Class_From(name, id, base, constructor, block) \
	struct Attach { static void Callback(WrapObject* o) { \
	static_assert(sizeof(WrapObject)==sizeof(Wrap##name), \
		"Derived wrap class pairs cannot declare data members"); new(o) Wrap##name(); \
	}}; \
	auto cls = worker->newClass(#name, id, constructor, &Attach::Callback, base); \
	block; ((void)0)

#define Js_Define_Class(name, base, constructor, block) \
	Js_Define_Class_From(name, Js_Typeid(name), constructor, block, base)

#define Js_Set_Class_Method(name, func)      cls->SetMemberMethod(worker, #name, func)
#define Js_Set_Class_Accessor(name, get, ...)cls->SetMemberAccessor(worker, #name, get, ##__VA_ARGS__)
#define Js_Set_Class_Indexed(get, ...)       cls->SetMemberIndexedAccessor(worker, get, ##__VA_ARGS__)
#define Js_Set_Class_Property(name, value)   cls->SetMemberProperty(worker, #name, value)
#define Js_Set_Class_Static_Property(name, value)cls->SetStaticProperty(worker, #name, value)
#define Js_Set_Method(name, func)          exports->SetMethod(worker, #name, func)
#define Js_Set_Accessor(name, get, ...)    exports->SetAccessor(worker, #name, get, ##__VA_ARGS__)
#define Js_Set_Property(name, value)       exports->SetProperty(worker, #name, value)

namespace qk { namespace js {
	class Worker;
	class WrapObject;
	class JSValue;
	class JSObject;
	class TypesParser;
	class CommonStrings;
	class JsClassInfo;

	template<class T>
	class Local {
	public:
		Qk_DEFINE_PROP_GET(T*, val, Const);
		inline Local(): _val(0) {}
		template <class S>
		inline Local(Local<S> that): _val(that->_val) {}
		inline bool isEmpty() const { return _val == 0; }
		inline T* operator->() const { return _val; }
		inline operator bool() const { return _val; }
		template <class S = JSValue>
		inline Local<S> cast() const {
			return Local<S>(static_cast<S*>(_val)); /* unsafe conversion */
		}
		inline Local<JSObject> castObject() const { return cast<JSObject>(); }
	private:
		inline Local(T *val): _val(val) {}
		template<class S> friend class MaybeLocal;
		template<class S> friend class Local;
		template<class S> friend class Persistent;
	};

	template<class T>
	class MaybeLocal {
	public:
		inline MaybeLocal(): _val(0) {}
		template <class S>
		inline MaybeLocal(Local<S> that): _val(that->_val) {}
		inline bool isEmpty() const { return _val == nullptr; }
		template <class S>
		inline bool toLocal(Local<S>* out) const {
			return _val ? (out->_val = _val, true): (out->_val = nullptr, false);
		}
		template <class S>
		inline Local<S> from(Local<S> defaultValue) const {
			return _val ? Local<S>(_val): defaultValue;
		}
		inline Local<T> toLocalChecked();
	private:
		T* _val;
	};

	class NoCopy {
	public:
		inline NoCopy() {}
		Qk_HIDDEN_ALL_COPY(NoCopy);
		Qk_HIDDEN_HEAP_ALLOC();
	};

	class WeakCallbackInfo {
	public:
		Worker* worker() const;
		void*   getParameter() const;
	};
	typedef void (*WeakCallback)(const WeakCallbackInfo& info);

	template<class T>
	class Persistent: public NoCopy {
	public:
		// @props
		Qk_DEFINE_PROP_GET(T*, val, Const);
		Qk_DEFINE_PROP_GET(Worker*, worker, Const);
		// @members
		inline Persistent(): _val(0), _worker(0) {
			Js_Type_Check(JSValue, T);
		}
		template <class S>
		inline Persistent(Worker* worker, Local<S> that) {
			reset(worker, that);
		}
		template <class S>
		inline Persistent(Worker* worker, const Persistent<S>& that) {
			reset(worker, that);
		}
		inline ~Persistent() {
			reset();
		}
		inline void reset() {
			reinterpret_cast<Persistent<JSValue>*>(this)->reset();
		}
		template <class S>
		inline void reset(Worker* worker, const Local<S>& other) {
			Js_Type_Check(T, S);
			reinterpret_cast<Persistent<JSValue>*>(this)->
				reset(worker, *reinterpret_cast<const Local<JSValue>*>(&other));
		}
		template <class S>
		inline void reset(Worker* worker, const Persistent<S>& other) {
			Js_Type_Check(T, S);
			reinterpret_cast<Persistent<JSValue>*>(this)->reset(worker, other.toLocal());
		}
		template<class S>
		void copy(const Persistent<S>& that) {
			Js_Type_Check(T, S);
			reinterpret_cast<Persistent<JSValue>*>(this)->
				copy(*reinterpret_cast<const Persistent<JSValue>*>(&that));
		}
		inline bool isEmpty() const { return _val == 0; }
		inline Local<T> toLocal() const {
			return Local<T>(static_cast<T*>(reinterpret_cast<Persistent<JSValue>*>(this)->toLocal().val()));
		}
		inline T* operator->() const { return _val; }
		inline operator bool() const { return _val; }
		inline bool isWeak() {
			return reinterpret_cast<Persistent<JSValue>*>(this)->isWeak();
		}
		inline void clearWeak() {
			reinterpret_cast<Persistent<JSValue>*>(this)->clearWeak();
		}
		inline void setWeak(void *ptr, WeakCallback cb) {
			reinterpret_cast<Persistent<JSValue>*>(this)->setWeak(ptr, cb);
		}
		friend class WrapObject;
		friend class Worker;
	};

	class Qk_EXPORT HandleScope: public NoCopy {
	public:
		explicit
		HandleScope(Worker* worker);
		~HandleScope();
	private:
		void *_val[3];
	};

	class Qk_EXPORT CallbackScope: public NoCopy {
	public:
		explicit
		CallbackScope(Worker* worker);
		~CallbackScope();
	private:
		void *_val;
	};

	class Qk_EXPORT TryCatch: public NoCopy {
	public:
		TryCatch();
		~TryCatch();
		bool hasCaught() const;
		Local<JSValue> exception() const;
	private:
		void *_val;
	};

	class Qk_EXPORT ReturnValue {
	public:
		template <class S>
		inline void set(Local<S> value) {
			Js_Type_Check(JSValue, S);
			set(value.cast<JSValue>());
		}
		void set(bool value);
		void set(double i);
		void set(int i);
		void set(uint32_t i);
		void setNull();
		void setUndefined();
		void setEmptyString();
	private:
		void *_val;
	};

	class Qk_EXPORT FunctionCallbackInfo: public NoCopy {
	public:
		Worker* worker() const;
		Local<JSObject> This() const;
		ReturnValue returnValue() const;
		Local<JSValue> operator[](int i) const;
		int length() const;
		bool isConstructCall() const;
	};

	class Qk_EXPORT PropertyCallbackInfo: public NoCopy {
	public:
		Worker* worker() const;
		Local<JSObject> This() const;
		ReturnValue returnValue() const;
	};

	class Qk_EXPORT PropertySetCallbackInfo: public NoCopy {
	public:
		Worker* worker() const;
		Local<JSObject> This() const;
	};

	typedef const FunctionCallbackInfo& FunctionArgs;
	typedef const PropertyCallbackInfo& PropertyArgs;
	typedef const PropertySetCallbackInfo& PropertySetArgs;
	typedef void (*FunctionCallback)(FunctionArgs args);
	typedef void (*AccessorGetterCallback)(Local<JSString> name, PropertyArgs args);
	typedef void (*AccessorSetterCallback)(Local<JSString> name, Local<JSValue> value, PropertySetArgs args);
	typedef void (*IndexedAccessorGetterCallback)(uint32_t index, PropertyArgs args);
	typedef void (*IndexedAccessorSetterCallback)(uint32_t index, Local<JSValue> value, PropertyArgs args);
	typedef void (*BindingCallback)(Local<JSObject> exports, Worker* worker);
	typedef void (*AttachCallback)(WrapObject* wrap);

	class Qk_EXPORT JSValue: public NoCopy {
	public:
		bool isUndefined(Worker* worker) const;
		bool isNull(Worker* worker) const;
		bool isString(Worker* worker) const;
		bool isBoolean(Worker* worker) const;
		bool isObject(Worker* worker) const;
		bool isArray(Worker* worker) const;
		bool isDate(Worker* worker) const;
		bool isNumber(Worker* worker) const;
		bool isUint32(Worker* worker) const;
		bool isInt32(Worker* worker) const;
		bool isFunction(Worker* worker) const;
		bool isArrayBuffer(Worker* worker) const;
		bool isTypedArray(Worker* worker) const;
		bool isUint8Array(Worker* worker) const;
		bool isBuffer(Worker* worker) const; // IsTypedArray or IsArrayBuffer
		bool equals(Local<JSValue> val) const;
		bool equals(Worker* worker, Local<JSValue> val) const;
		bool strictEquals(Local<JSValue> val) const;
		bool strictEquals(Worker* worker, Local<JSValue> val) const;
		Local<JSString> toString(Worker* worker) const; // to string local
		Local<JSNumber> toNumber(Worker* worker) const;
		Local<JSInt32> toInt32(Worker* worker) const;
		Local<JSUint32> toUint32(Worker* worker) const;
		Local<JSObject> toObject(Worker* worker) const;
		Local<JSBoolean> toBoolean(Worker* worker) const;
		String toStringValue(Worker* worker, bool ascii = false) const; // to string value
		String2 toStringValue2(Worker* worker) const; // to utf16 string
		bool toBooleanValue(Worker* worker) const;
		double toNumberValue(Worker* worker) const;
		int toInt32Value(Worker* worker) const;
		uint32_t toUint32Value(Worker* worker) const;
		Maybe<double> toNumberMaybe(Worker* worker) const;
		Maybe<int> toInt32Maybe(Worker* worker) const;
		Maybe<uint32_t> toUint32Maybe(Worker* worker) const;
		bool instanceOf(Worker* worker, Local<JSObject> value);
		Buffer toBuffer(Worker* worker, Encoding en) const;
		WeakBuffer asBuffer(Worker* worker); // TypedArray or ArrayBuffer to WeakBuffer
	};

	class Qk_EXPORT JSString: public JSValue {
	public:
		int length(Worker* worker) const; // utf16 length
		String value(Worker* worker, bool ascii = false) const; // utf8 string value
		String2 value2(Worker* worker) const; // utf16 string value
		static Local<JSString> Empty(Worker* worker);
	};

	class Qk_EXPORT JSObject: public JSValue {
	public:
		Local<JSValue> get(Worker* worker, Local<JSValue> key);
		Local<JSValue> get(Worker* worker, uint32_t index);
		bool set(Worker* worker, Local<JSValue> key, Local<JSValue> val);
		bool set(Worker* worker, uint32_t index, Local<JSValue> val);
		bool has(Worker* worker, Local<JSValue> key);
		bool has(Worker* worker, uint32_t index);
		bool deleteOf(Worker* worker, Local<JSValue> key);
		bool deleteOf(Worker* worker, uint32_t index);
		Local<JSArray> getPropertyNames(Worker* worker);
		Maybe<Dict<String, int>> toIntegerMap(Worker* worker);
		Maybe<Dict<String, String>> toStringMap(Worker* worker);
		Maybe<JSON> toJSON(Worker* worker);
		Local<JSValue> getProperty(Worker* worker, cString& name);
		Local<JSFunction> getConstructor(Worker* worker);
		template<class T>
		bool setProperty(Worker* worker, cString& name, T value);
		bool setMethod(Worker* worker, cString& name, FunctionCallback func);
		bool setAccessor(Worker* worker, cString& name,
										AccessorGetterCallback get, AccessorSetterCallback set = nullptr);
		void* objectPrivate();
		bool setObjectPrivate(void* value);
		bool set__Proto__(Worker* worker, Local<JSObject> __proto__); // set __proto__
	};

	class Qk_EXPORT JSArray: public JSObject {
	public:
		int length(Worker* worker) const;
		Maybe<Array<String>> toStringArrayMaybe(Worker* worker);
		Maybe<Array<double>> toNumberArrayMaybe(Worker* worker);
		Maybe<Buffer> toBufferMaybe(Worker* worker);
	};

	class Qk_EXPORT JSDate: public JSObject {
	public:
		double valueOf(Worker* worker) const;
	};

	class Qk_EXPORT JSNumber: public JSValue {
	public:
		double value(Worker* worker) const;
	};

	class Qk_EXPORT JSInt32: public JSNumber {
	public:
		int value(Worker* worker) const;
	};

	class Qk_EXPORT JSInteger: public JSNumber {
	public:
		int64_t value(Worker* worker) const;
	};

	class Qk_EXPORT JSUint32: public JSNumber {
	public:
		uint32_t value(Worker* worker) const;
	};

	class Qk_EXPORT JSBoolean: public JSValue {
	public:
		bool value(Worker* worker) const;
	};

	class Qk_EXPORT JSFunction: public JSObject {
	public:
		Local<JSValue> call(Worker* worker, int argc = 0,
												Local<JSValue> argv[] = nullptr,
												Local<JSValue> recv = Local<JSValue>());
		Local<JSValue> call(Worker* worker, Local<JSValue> recv);
		Local<JSObject> newInstance(Worker* worker, int argc = 0,
																Local<JSValue> argv[] = nullptr);
		Local<JSObject> getPrototype(Worker* worker); // funciton.prototype prop
	};

	class Qk_EXPORT JSArrayBuffer: public JSObject {
	public:
		int byteLength(Worker* worker) const;
		Char* data(Worker* worker);
		WeakBuffer weakBuffer(Worker* worker);
	};

	class Qk_EXPORT JSTypedArray: public JSObject {
	public:
		Local<JSArrayBuffer> buffer(Worker* worker);
		WeakBuffer weakBuffer(Worker* worker);
		int byteLength(Worker* worker);
		int byteOffset(Worker* worker);
	};

	class Qk_EXPORT JSUint8Array: public JSTypedArray {
	};

	class Qk_EXPORT JSSet: public JSObject {
	public:
		MaybeLocal<JSSet> add(Worker* worker, Local<JSValue> key);
		Maybe<bool> has(Worker* worker, Local<JSValue> key);
		Maybe<bool> deleteOf(Worker* worker, Local<JSValue> key);
	};

	class Qk_EXPORT JSClass {
		Qk_HIDDEN_ALL_COPY(JSClass);
	public:
		Qk_DEFINE_PROP_GET(Worker*, worker, Protected);
		Qk_DEFINE_PROP_GET(uint64_t, id, Protected);
		// @members
		virtual ~JSClass() = default;
		void exports(cString& name, Local<JSObject> exports);
		bool hasInstance(Local<JSValue> val);
		Local<JSFunction> getFunction(); // constructor function
		Local<JSObject> newInstance(uint32_t argc = 0, Local<JSValue>* argv = nullptr);
		bool setMemberMethod(cString& name, FunctionCallback func);
		bool setMemberAccessor(cString& name,
													AccessorGetterCallback get,
													AccessorSetterCallback set = nullptr);
		bool setMemberIndexedAccessor(IndexedAccessorGetterCallback get,
																	IndexedAccessorSetterCallback set = nullptr);
		template<class T>
		bool setMemberProperty(cString& name, T value);
		template<class T>
		bool setStaticProperty(cString& name, T value);
	protected:
		inline JSClass() {}
		Persistent<JSFunction> _func; // constructor function
		AttachCallback _attachConstructor;
		friend class JsClassInfo;
	};

	class Qk_EXPORT Worker: public Object {
		Qk_HIDDEN_ALL_COPY(Worker);
	public:
		static Worker* Make();
		static Worker* current();

		static inline Worker* worker() {
			return current();
		}
		template<class T>
		static Worker* worker(T& args) {
			return args.worker();
		}
		static void setModule(cString& name, BindingCallback binding, cChar* file = 0);

		// @prop
		Qk_DEFINE_PROP_GET(TypesParser*, types, Protected);
		Qk_DEFINE_PROP_GET(CommonStrings*, strs, Protected);
		Qk_DEFINE_PROP_GET(JsClassInfo*, classsinfo, Protected);
		Qk_DEFINE_PROP_GET(ThreadID, thread_id, Protected);
		Qk_DEFINE_PROP_ACC_GET(Local<JSObject>, global);

		void release() override;
		void reportException(TryCatch* try_catch);
		void garbageCollection();

		Local<JSValue> bindingModule(cString& name);
		// new instance
		Local<JSNumber> newInstance(float data);
		Local<JSNumber> newInstance(double data);
		Local<JSBoolean>newInstance(bool data);
		Local<JSInt32>  newInstance(Char data);
		Local<JSUint32> newInstance(uint8_t data);
		Local<JSInt32>  newInstance(int16_t data);
		Local<JSUint32> newInstance(uint16_t data);
		Local<JSInt32>  newInstance(int data);
		Local<JSUint32> newInstance(uint32_t data);
		Local<JSNumber> newInstance(int64_t data);
		Local<JSNumber> newInstance(uint64_t data);
		Local<JSString> newInstance(cString& data, bool oneByte = false);
		Local<JSString> newInstance(cString2& data);
		Local<JSObject> newInstance(cError& data);
		Local<JSObject> newInstance(const HttpError& err);
		Local<JSArray>  newInstance(cArray<String>& data);
		Local<JSArray>  newInstance(Array<FileStat>& data);
		Local<JSArray>  newInstance(Array<FileStat>&& data);
		Local<JSObject> newInstance(cDict<String, String>& data);
		Local<JSUint8Array> newInstance(Buffer& buff);
		Local<JSUint8Array> newInstance(Buffer&& buff);
		Local<JSObject> newInstance(FileStat& stat);
		Local<JSObject> newInstance(FileStat&& stat);
		Local<JSObject> newInstance(const Dirent& dir);
		Local<JSArray>  newInstance(Array<Dirent>& data);
		Local<JSArray>  newInstance(Array<Dirent>&& data);
		inline
		Local<JSBoolean> newInstance(const Bool& v) { return newInstance(v.value); }
		inline
		Local<JSNumber>  newInstance(const Float32& v) { return newInstance(v.value); }
		inline
		Local<JSNumber>  newInstance(const Float64& v) { return newInstance(v.value); }
		inline
		Local<JSInt32>   newInstance(const Int8& v) { return newInstance(v.value); }
		inline
		Local<JSUint32>  newInstance(const Uint8& v) { return newInstance(v.value); }
		inline
		Local<JSInt32>   newInstance(const Int16& v) { return newInstance(v.value); }
		inline
		Local<JSUint32>  newInstance(const Uint16& v) { return newInstance(v.value); }
		inline
		Local<JSInt32>   newInstance(const Int32& v) { return newInstance(v.value); }
		inline
		Local<JSUint32>  newInstance(const Uint32& v) { return newInstance(v.value); }
		inline
		Local<JSNumber>  newInstance(const Int64& v) { return newInstance(v.value); }
		inline
		Local<JSNumber>  newInstance(const Uint64& v) { return newInstance(v.value); }

		template <class S>
		inline Local<S> newInstance(Local<S> val) { return val; }

		template <class S>
		inline Local<S> newInstance(const Persistent<S>& value) {
			return value->toLocal();
		}

		Local<JSObject> newObject();
		Local<JSObject> newObject(uint64_t classid, uint32_t argc = 0, Local<JSValue>* argv = 0);
		Local<JSString> newString(cBuffer& data);
		Local<JSString> newAscii(cChar* str);
		Local<JSArrayBuffer> newArrayBuffer(Char* use_buff, uint32_t len);
		Local<JSArrayBuffer> newArrayBuffer(uint32_t len);
		Local<JSUint8Array> newUint8Array(Local<JSString> str, Encoding enc = Encoding::kUTF8_Encoding);
		Local<JSUint8Array> newUint8Array(int size, Char fill = 0);
		Local<JSUint8Array> newUint8Array(Local<JSArrayBuffer> ab);
		Local<JSUint8Array> newUint8Array(Local<JSArrayBuffer> ab, uint32_t offset, uint32_t size);
		Local<JSObject> newRangeError(cChar* errmsg, ...);
		Local<JSObject> newReferenceError(cChar* errmsg, ...);
		Local<JSObject> newSyntaxError(cChar* errmsg, ...);
		Local<JSObject> newTypeError(cChar* errmsg, ...);
		Local<JSObject> newError(cChar* errmsg, ...);
		Local<JSObject> newError(cError& err);
		Local<JSObject> newError(const HttpError& err);
		Local<JSObject> newError(Local<JSObject> value);
		Local<JSArray>  newArray(uint32_t len = 0);
		Local<JSValue>  newNull();
		Local<JSValue>  newUndefined();
		Local<JSSet>    newSet();

		void throwError(Local<JSValue> exception);
		void throwError(cChar* errmsg, ...);
		template<class T>
		inline bool instanceOf(Local<JSValue> val) {
			return instanceOf(val, Js_Typeid(T));
		}
		bool instanceOf(Local<JSValue> val, uint64_t id);

		Local<JSClass> jsclass(uint64_t id);
		Local<JSClass> newClass(cString& name, uint64_t id,
														FunctionCallback constructor,
														AttachCallback attachConstructor,
														Local<JSClass> base = Local<JSClass>());
		Local<JSClass> newClass(cString& name, uint64_t id,
														FunctionCallback constructor,
														AttachCallback attachConstructor, uint64_t base);
		Local<JSClass> newClass(cString& name, uint64_t id,
														FunctionCallback constructor,
														AttachCallback attachConstructor, Local<JSFunction> base);

		Local<JSValue> runScript(cString& source,
														cString& name,
														Local<JSObject> sandbox = Local<JSObject>());
		Local<JSValue> runScript(Local<JSString> source,
														Local<JSString> name,
														Local<JSObject> sandbox = Local<JSObject>());
		Local<JSValue> runNativeScript(
			cBuffer& source, cString& name, Local<JSObject> exports = Local<JSObject>()
		);

	protected:
		Persistent<JSObject> _global, _nativeModules;
		Worker();
		virtual void init();
	};

	class Qk_EXPORT WrapObject {
		Qk_HIDDEN_ALL_COPY(WrapObject);
	public:
		inline Worker* worker() {
			return _handle._worker;
		}
		template<class T = Object>
		inline T* self() {
			return static_cast<T*>(reinterpret_cast<Object*>(this + 1));
		}
		inline Persistent<JSObject>& handle() {
			return _handle;
		}
		inline Local<JSObject> that() {
			return _handle.toLocal();
		}
		inline Local<JSValue> get(Local<JSValue> key) {
			return _handle->get(worker(), key);
		}
		inline bool set(Local<JSValue> key, Local<JSValue> value) {
			return _handle->set(worker(), key, value);
		}
		inline bool del(Local<JSValue> key) {
			return _handle->deleteOf(worker(), key);
		}

		virtual void init();
		virtual ~WrapObject();
		virtual bool addEventListener(cString& name, cString& func, int id);
		virtual bool removeEventListener(cString& name, int id);

		Object* externalData();
		bool    setExternalData(Object* data);

		// call member func
		Local<JSValue> call(Local<JSValue> name, int argc = 0, Local<JSValue> argv[] = nullptr);
		Local<JSValue> call(cString& name, int argc = 0, Local<JSValue> argv[] = nullptr);

		template<class T = Object, class S>
		static inline Wrap<T>* wrap(Local<S> value) {
			return static_cast<Wrap<T>*>(unpack_(value.castObject()));
		}
		template<class T>
		static inline Wrap<T>* wrap(T* object) {
			return wrap(object, Js_Typeid(T));
		}
		template<class T>
		static inline Wrap<T>* wrap(T* object, uint64_t type_id) {
			return static_cast<js::Wrap<T>*>(pack_(object, type_id));
		}

		template<class W, class O>
		static Wrap<O>* New(FunctionArgs args, O *o) {
			static_assert(sizeof(W) == sizeof(WrapObject),
										"Derived wrap class pairs cannot declare data members");
			static_assert(typename O::Traits::isObject, "Must be object");
			auto wrap = (new(reinterpret_cast<WrapObject*>(o) - 1) W())->newInit(args);
			return static_cast<Wrap<O>*>(static_cast<WrapObject*>(wrap));
		}

	private:
		static WrapObject* unpack_(Local<JSObject> object);
		static WrapObject* pack_(Object* object, uint64_t type_id);
		WrapObject* newInit(FunctionArgs args);
		WrapObject* attach(Worker *worker, Local<JSObject> This);

		Persistent<JSObject> _handle;

		friend class JsClassInfo;
	};

	template<class T = Object>
	class Wrap: public WrapObject {
	public:
		inline T* self() {
			return reinterpret_cast<T*>(this + 1);
		}
	};

	Qk_EXPORT int Start(cArray<String>& argv);

	// **********************************************************************

	template<>
	Qk_EXPORT void Persistent<JSValue>::reset();
	template<> template<>
	Qk_EXPORT void Persistent<JSValue>::reset(Worker* worker, const Local<JSValue>& other);
	template<>
	Qk_EXPORT Local<JSValue> Persistent<JSValue>::toLocal() const;
	template<>
	Qk_EXPORT bool Persistent<JSValue>::isWeak();
	template<>
	Qk_EXPORT void Persistent<JSValue>::clearWeak();
	template<>
	Qk_EXPORT void Persistent<JSValue>::setWeak(void *ptr, WeakCallback cb);
	template<> template<>
	Qk_EXPORT void Persistent<JSValue>::copy(const Persistent<JSValue>& that);
	template<>
	Qk_EXPORT void ReturnValue::set<JSValue>(Local<JSValue> value);

	template<class T>
	bool JSObject::setProperty(Worker* worker, cString& name, T value) {
		return Set(worker, worker->newInstance(name, 1), worker->newInstance(value));
	}
	template<class T>
	bool JSClass::setMemberProperty(cString& name, T value) {
		return setMemberProperty<Local<JSValue>>(name, worker->newInstance(value));
	}
	template<class T>
	bool JSClass::setStaticProperty(cString& name, T value) {
		return setStaticProperty<Local<JSValue>>(name, worker->newInstance(value));
	}
	template<>
	Qk_EXPORT bool JSClass::setMemberProperty<Local<JSValue>>(cString& name, Local<JSValue> value);
	template<>
	Qk_EXPORT bool JSClass::setStaticProperty<Local<JSValue>>(cString& name, Local<JSValue> value);

	template<class T>
	inline Local<T> MaybeLocal<T>::toLocalChecked() {
		Js_Type_Check(JSValue, T);
		reinterpret_cast<MaybeLocal<JSValue>*>(this)->toLocalChecked();
		return Local<T>(_val);
	}
	template<>
	Qk_EXPORT Local<JSValue> MaybeLocal<JSValue>::toLocalChecked();

} }
#endif