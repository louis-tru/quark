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

// ------------- js common macro -------------

#define JS_WORKER(...)   auto worker = Worker::worker(__VA_ARGS__)
#define JS_RETURN(rev)   return worker->result(args, (rev))
#define JS_RETURN_NULL() return worker->result(args, worker->NewNull())
#define JS_UNPACK(type)  auto wrap = qk::js::WrapObject::unpack<type>(args.This())
#define JS_SELF(type)    auto self = qk::js::WrapObject::unpack<type>(args.This())->self()
#define JS_HANDLE_SCOPE() HandleScope scope(worker)
#define JS_CALLBACK_SCOPE() CallbackScope cscope(worker)

#define JS_THROW_ERROR(Error, err, ...) \
	return worker->throwError(worker->New##Error((err), ##__VA_ARGS__))

#define JS_THROW_ERR(err, ...) JS_THROW_ERROR(Error, err, ##__VA_ARGS__)
#define JS_TRY_CATCH(block, Error) try block catch(const Error& e) { JS_THROW_ERR(e); }

#define JS_REG_MODULE(name, cls) \
	Qk_INIT_BLOCK(JS_REG_MODULE_##name) { \
		Qk_DEBUG("%s", "JS_REG_MODULE "#name""); \
		qk::js::Worker::registerModule(#name, cls::binding, __FILE__); \
	}

#define JS_TYPEID(t) (typeid(t).hashCode())
#define JS_ATTACH(args) if (WrapObject::attach(args)) return

#define JS_TYPE_CHECK(T, S)  \
	while (false) { \
		*(static_cast<T* volatile*>(0)) = static_cast<S*>(0); \
	}

// define class macro
#define JS_NEW_CLASS_FROM_ID(name, id, constructor, block, base) \
	struct Attach { static void Callback(WrapObject* o) { \
	static_assert(sizeof(WrapObject)==sizeof(Wrap##name), \
		"Derived wrap class pairs cannot declare data members"); new(o) Wrap##name(); \
	}}; \
	auto cls = worker->NewClass(id, #name, \
	constructor, &Attach::Callback, base); \
	cls->SetInstanceInternalFieldCount(1); \
	block; ((void)0)

#define JS_NEW_CLASS(name, constructor, block, base) \
	JS_NEW_CLASS_FROM_ID(name, JS_TYPEID(name), constructor, block, base)

#define JS_DEFINE_CLASS(name, constructor, block, base) \
	JS_NEW_CLASS(name, constructor, block, JS_TYPEID(base)); \
	cls->Export(worker, #name, exports)

#define JS_DEFINE_CLASS_NO_EXPORTS(name, constructor, block, base) \
	JS_NEW_CLASS(name, constructor, block, JS_TYPEID(base))

#define JS_SET_CLASS_METHOD(name, func)      cls->SetMemberMethod(worker, #name, func)
#define JS_SET_CLASS_ACCESSOR(name, get, ...)cls->SetMemberAccessor(worker, #name, get, ##__VA_ARGS__)
#define JS_SET_CLASS_INDEXED(get, ...)       cls->SetMemberIndexedAccessor(worker, get, ##__VA_ARGS__)
#define JS_SET_CLASS_PROPERTY(name, value)   cls->SetMemberProperty(worker, #name, value)
#define JS_SET_CLASS_STATIC_PROPERTY(name, value)cls->SetStaticProperty(worker, #name, value)
#define JS_SET_METHOD(name, func)          exports->SetMethod(worker, #name, func)
#define JS_SET_ACCESSOR(name, get, ...)    exports->SetAccessor(worker, #name, get, ##__VA_ARGS__)
#define JS_SET_PROPERTY(name, value)       exports->SetProperty(worker, #name, value)

namespace qk { namespace js {
	class Worker;
	class WrapObject;
	class JSValue;

	class NoCopy {
	public:
		inline NoCopy() {}
		Qk_HIDDEN_ALL_COPY(NoCopy);
		Qk_HIDDEN_HEAP_ALLOC();
	};

	template<class T>
	class Qk_EXPORT MaybeLocal {
	public:
		inline MaybeLocal(): _val(nullptr) {}
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

	template<class T>
	class Local {
	public:
		inline Local(): _val(0) {}
		template <class S>
		inline Local(Local<S> that): _val(that->_val) {}
		inline bool isEmpty() const { return _val == 0; }
		inline T* operator->() const { return _val; }
		inline T* operator*() const { return _val; }
		template <class S = JSObject>
		inline Local<S> to() const { return Local<S>(_val); } /* unsafe conversion */
	private:
		inline Local(T *val): _val(val) {}
		T        *_val;
		friend class Worker;
		friend class JSValue;
		friend class JSFunction;
		friend class JSString;
		template<class S> friend class MaybeLocal;
		template<class S> friend class Local;
	};

	class WeakCallbackInfo {
	public:
		Worker* worker() const;
		void*   getParameter() const;
	};
	typedef void (*WeakCallback)(const WeakCallbackInfo& info);

	template<class T>
	class PersistentBase: public NoCopy {
	public:
		inline void reset() {
			reinterpret_cast<PersistentBase<JSValue>*>(this)->reset();
		}
		template <class S>
		inline void reset(Worker* worker, const Local<S>& other) {
			JS_TYPE_CHECK(T, S);
			reinterpret_cast<PersistentBase<JSValue>*>(this)->
				reset(worker, *reinterpret_cast<const Local<JSValue>*>(&other));
		}
		template <class S>
		inline void reset(Worker* worker, const PersistentBase<S>& other) {
			JS_TYPE_CHECK(T, S);
			reinterpret_cast<PersistentBase<JSValue>*>(this)->reset(worker, other.local());
		}
		inline bool isEmpty() const { return _val == 0; }
		inline Local<T> toLocal() const {
			auto l = reinterpret_cast<PersistentBase<JSValue>*>(this)->toLocal();
			return reinterpret_cast<Local<T>>(l);
		}
		inline T* operator->() const { return _val; }
		inline T* operator*() const { return _val; }
		inline Worker* worker() const { return _worker; }
		inline bool isWeak() {
			return reinterpret_cast<PersistentBase<JSValue>*>(this)->isWeak();
		}
		inline void clearWeak() {
			reinterpret_cast<PersistentBase<JSValue>*>(this)->clearWeak();
		}
		inline void setWeak(void *ptr, WeakCallback cb) {
			reinterpret_cast<PersistentBase<JSValue>*>(this)->setWeak(ptr, cb);
		}
	private:
		inline PersistentBase(): _val(0), _worker(0) {
			JS_TYPE_CHECK(JSValue, T);
		}
		template<class S>
		void copy(const PersistentBase<S>& that);
		T      *_val;
		Worker *_worker;
		friend class WrapObject;
		friend class Worker;
		template<class F1, class F2> friend class Persistent;
	};

	template<class T>
	class NonCopyablePersistentTraits {
	public:
		static constexpr bool kResetInDestructor = true;
		inline static void CopyCheck() { Uncompilable<Object>(); }
		template<class O> static void Uncompilable() {
			JS_TYPE_CHECK(O, JSValue);
		}
	};

	template<class T>
	class CopyablePersistentTraits {
	public:
		typedef Persistent<T, CopyablePersistentTraits<T>> Handle;
		static constexpr bool kResetInDestructor = true;
		static inline void CopyCheck() {}
	};

	template<class T, class M = NonCopyablePersistentTraits<T>>
	class Persistent: public PersistentBase<T> {
	public:
		Persistent() {}
		~Persistent() {
			if (M::kResetInDestructor)
				this->reset();
		}
		template <class S>
		inline Persistent(Worker* worker, Local<S> that) {
			this->reset(worker, that);
		}
		template <class S, class M2>
		inline Persistent(Worker* worker, const Persistent<S, M2>& that) {
			this->reset(worker, that);
		}
		inline Persistent(const Persistent& that) {
			copy(that);
		}
		template<class S, class M2>
		inline Persistent(const Persistent<S, M2>& that) {
			copy(that);
		}
		inline Persistent& operator=(const Persistent& that) {
			copy(that);
			return *this;
		}
		template <class S, class M2>
		inline Persistent& operator=(const Persistent<S, M2>& that) {
			copy(that);
			return *this;
		}
	private:
		template<class S>
		inline void copy(const PersistentBase<S>& that) {
			JS_TYPE_CHECK(T, S);
			M::CopyCheck();
			this->reset();
			if ( that.isEmpty() )
				return;
			reinterpret_cast<PersistentBase<JSValue>*>(this)->
				copy(*reinterpret_cast<const PersistentBase<JSValue>*>(&that));
		}
		template<class F1, class F2> friend class Persistent;
	};

	class Qk_EXPORT ReturnValue {
	public:
		template <class S>
		inline void set(Local<S> value) {
			JS_TYPE_CHECK(JSValue, S);
			set(*reinterpret_cast<Local<JSValue>*>(&value));
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

	class Qk_EXPORT HandleScope: public NoCopy {
	public:
		explicit
		HandleScope(Worker* worker);
		~HandleScope();
	private:
		void* val_[3];
	};

	class Qk_EXPORT CallbackScope: public NoCopy {
	public:
		explicit
		CallbackScope(Worker* worker);
		~CallbackScope();
	private:
		void* val_;
	};

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

	class Qk_EXPORT JSClass: public NoCopy {
	public:
		uint64_t id() const;
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
		void setInstanceInternalFieldCount(int count);
		int  instanceInternalFieldCount();
		void Export(cString& name, Local<JSObject> exports);
	};

	class Qk_EXPORT TryCatch: public NoCopy {
	public:
		TryCatch();
		~TryCatch();
		bool hasCaught() const;
		Local<JSValue> exception() const;
	private:
		void* val_;
	};

	class TypesProgram;
	class CommonStrings;
	class JSClassInfo;

	class Qk_EXPORT Worker: public Object {
		Qk_HIDDEN_ALL_COPY(Worker);
	public:

		static Worker* worker();
		template<class T>
		static Worker* worker(T& args) {
			return args.worker();
		}
		static void RegisterModule(
			cString& name, BindingCallback binding, cChar* file = nullptr
		);
		static Worker* Make();

		// @prop
		Qk_DEFINE_PROP_GET(TypesProgram*, types, Protected);
		Qk_DEFINE_PROP_GET(CommonStrings*, strs, Protected);
		Qk_DEFINE_PROP_GET(JSClassInfo*, classsinfo, Protected);
		Qk_DEFINE_PROP_GET(ThreadID, thread_id, Protected);

		~Worker();

		Local<JSValue> bindingModule(cString& name);
		Local<JSNumber> New(float data);
		Local<JSNumber> New(double data);
		Local<JSBoolean>New(bool data);
		Local<JSInt32>  New(Char data);
		Local<JSUint32> New(uint8_t data);
		Local<JSInt32>  New(int16_t data);
		Local<JSUint32> New(uint16_t data);
		Local<JSInt32>  New(int data);
		Local<JSUint32> New(uint32_t data);
		Local<JSNumber> New(int64_t data);
		Local<JSNumber> New(uint64_t data);
		Local<JSString> New(cChar* data, int len = -1);
		Local<JSString> New(cString& data, bool is_ascii = false);
		Local<JSString> New(cString2& data);
		Local<JSObject> New(cError& data);
		Local<JSObject> New(const HttpError& err);
		Local<JSArray>  New(cArray<String>& data);
		Local<JSArray>  New(Array<FileStat>& data);
		Local<JSArray>  New(Array<FileStat>&& data);
		Local<JSObject> New(cDict<String, String>& data);
		Local<JSUint8Array> New(Buffer& buff);
		Local<JSUint8Array> New(Buffer&& buff);
		Local<JSObject> New(FileStat& stat);
		Local<JSObject> New(FileStat&& stat);
		Local<JSObject> New(const Dirent& dir);
		Local<JSArray>  New(Array<Dirent>& data);
		Local<JSArray>  New(Array<Dirent>&& data);
		//
		inline
		Local<JSBoolean> New(const Bool& v) { return New(v.value); }
		inline
		Local<JSNumber>  New(const Float32& v) { return New(v.value); }
		inline
		Local<JSNumber>  New(const Float64& v) { return New(v.value); }
		inline
		Local<JSInt32>   New(const Int8& v) { return New(v.value); }
		inline
		Local<JSUint32>  New(const Uint8& v) { return New(v.value); }
		inline
		Local<JSInt32>   New(const Int16& v) { return New(v.value); }
		inline
		Local<JSUint32>  New(const Uint16& v) { return New(v.value); }
		inline
		Local<JSInt32>   New(const Int32& v) { return New(v.value); }
		inline
		Local<JSUint32>  New(const Uint32& v) { return New(v.value); }
		inline
		Local<JSNumber>  New(const Int64& v) { return New(v.value); }
		inline
		Local<JSNumber>  New(const Uint64& v) { return New(v.value); }

		template <class T>
		inline Local<T>  New(Local<T> val) { return val; }
		
		template <class T>
		inline Local<T> New(const PersistentBase<T>& value) {
			auto r = New(*reinterpret_cast<const PersistentBase<JSValue>*>(&value));
			return *reinterpret_cast<Local<T>*>(&r);
		}

		Local<JSValue>  New(const PersistentBase<JSValue>& value);

		Local<JSObject> NewInstance(uint64_t id, uint32_t argc = 0, Local<JSValue>* argv = nullptr);
		Local<JSString> NewString(cBuffer& data);
		Local<JSString> NewAscii(cChar* str);
		Local<JSArrayBuffer> NewArrayBuffer(Char* use_buff, uint32_t len);
		Local<JSArrayBuffer> NewArrayBuffer(uint32_t len);
		Local<JSUint8Array> NewUint8Array(Local<JSString> str, Encoding enc = Encoding::kUTF8_Encoding);
		Local<JSUint8Array> NewUint8Array(int size, Char fill = 0);
		Local<JSUint8Array> NewUint8Array(Local<JSArrayBuffer> ab);
		Local<JSUint8Array> NewUint8Array(Local<JSArrayBuffer> ab, uint32_t offset, uint32_t size);
		Local<JSObject> NewRangeError(cChar* errmsg, ...);
		Local<JSObject> NewReferenceError(cChar* errmsg, ...);
		Local<JSObject> NewSyntaxError(cChar* errmsg, ...);
		Local<JSObject> NewTypeError(cChar* errmsg, ...);
		Local<JSObject> NewError(cChar* errmsg, ...);
		Local<JSObject> NewError(cError& err);
		Local<JSObject> NewError(const HttpError& err);
		Local<JSObject> NewError(Local<JSObject> value);
		Local<JSObject> NewObject();
		Local<JSArray>  NewArray(uint32_t len = 0);
		Local<JSValue>  NewNull();
		Local<JSValue>  NewUndefined();
		Local<JSSet>    NewSet();

		template<class T>
		inline static Local<JSValue> New(const Object& obj, Worker* worker) {
			return worker->New( *static_cast<const T*>(&obj) );
		}

		void throwError(Local<JSValue> exception);
		void throwError(cChar* errmsg, ...);
		bool hasInstance(Local<JSValue> val, uint64_t id);
		bool hasView(Local<JSValue> val);

		template<class T>
		inline bool hasInstance(Local<JSValue> val) {
			return hasInstance(val, JS_TYPEID(T));
		}

		/**
		 * @method jsclass(id) find class
		 */
		Local<JSClass> jsclass(uint32_t id);

		/**
		 * @method result
		 */
		template <class Args, class T>
		inline void result(const Args& args, Local<T> data) {
			args.GetReturnValue().Set( data );
		}

		/**
		 * @method result
		 */
		template <class Args, class T>
		inline void result(const Args& args, const T& data) {
			args.GetReturnValue().Set( New(data) );
		}

		/**
		 * @method result
		 */
		template <class Args, class T>
		inline void result(const Args& args, T&& data) {
			args.GetReturnValue().Set( New(std::move(data)) );
		}

		/**
		 * @method NewClass js class
		 */
		Local<JSClass> NewClass(uint64_t id, cString& name,
														FunctionCallback constructor,
														AttachCallback attach_callback,
														Local<JSClass> base = Local<JSClass>());
		/**
		 * @method NewClass js class
		 */
		Local<JSClass> NewClass(uint64_t id, cString& name,
														FunctionCallback constructor,
														AttachCallback attach_callback, uint64_t base);
		/**
		 * @method NewClass js class
		 */
		Local<JSClass> NewClass(uint64_t id, cString& name,
														FunctionCallback constructor,
														AttachCallback attach_callback, Local<JSFunction> base);
		/**
		 * @method runScript
		 */
		Local<JSValue> runScript(cString& source,
														cString& name,
														Local<JSObject> sandbox = Local<JSObject>());
		/**
		 * @method runScript
		 */
		Local<JSValue> runScript(Local<JSString> source,
														Local<JSString> name,
														Local<JSObject> sandbox = Local<JSObject>());
		/**
		 * @method runNativeScript
		 */
		Local<JSValue> runNativeScript(
			cBuffer& source, cString& name, Local<JSObject> exports = Local<JSObject>()
		);

		/**
		 * @method global()
		 */
		Local<JSObject> global();

		/**
		 * @method reportException
		 */
		void reportException(TryCatch* try_catch);

		/**
		 * @method garbageCollection()
		 */
		void garbageCollection();

	protected:
		Worker();
		Persistent<JSObject> _global;
		Persistent<JSObject> _nativeModules;
	};

	class Qk_EXPORT WrapObject {
		Qk_HIDDEN_ALL_COPY(WrapObject);
	protected:
		~WrapObject();

		virtual void initialize();
		virtual void destroy();

		template<class W, class O>
		static Wrap<O>* New(FunctionCall args, O* object) {
			static_assert(sizeof(W) == sizeof(WrapObject),
										"Derived wrap class pairs cannot declare data members");
			static_assert(O::Traits::isObject, "Must be object");
			auto wrap = new(reinterpret_cast<WrapObject*>(object) - 1) W();
			wrap->init2(args);
			return static_cast<js::Wrap<O>*>(static_cast<WrapObject*>(wrap));
		}

		static WrapObject* Attach(FunctionCall args);

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
			return handle_.local()->get(worker(), key);
		}
		inline bool set(Local<JSValue> key, Local<JSValue> value) {
			return handle_.local()->set(worker(), key, value);
		}
		inline bool del(Local<JSValue> key) {
			return handle_.local()->deleteOf(worker(), key);
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
		static inline Wrap<T>* pack(T* object, uint64_t type_id) {
			return static_cast<js::Wrap<T>*>(pack2(object, type_id));
		}

	private:
		static WrapObject* unpack2(Local<JSObject> object);
		static WrapObject* pack2(Object* object, uint64_t type_id);
		void init2(FunctionCall args);

	protected:
		Persistent<JSObject> handle_;
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

	template<class T = Object>
	class Qk_EXPORT Wrap: public WrapObject {
		Wrap() = delete;
	public:
		inline static Wrap<T>* unpack(Local<JSObject> value) {
			return WrapObject::unpack<T>(value);
		}
		inline T* self() {
			return reinterpret_cast<T*>(this + 1);
		}
	};

	Qk_EXPORT int Start(cArray<String>& argv);

	// **********************************************************************

	typedef CopyablePersistentTraits<JSClass>::Handle CopyablePersistentClass;
	typedef CopyablePersistentTraits<JSFunction>::Handle CopyablePersistentFunc;
	typedef CopyablePersistentTraits<JSObject>::Handle CopyablePersistentObject;
	typedef CopyablePersistentTraits<JSValue>::Handle CopyablePersistentValue;

	template<>
	Qk_EXPORT void PersistentBase<JSValue>::reset();
	template<>
	Qk_EXPORT void PersistentBase<JSClass>::reset();
	template<> template<>
	Qk_EXPORT void PersistentBase<JSValue>::reset(Worker* worker, const Local<JSValue>& other);
	template<> template<>
	Qk_EXPORT void PersistentBase<JSClass>::reset(Worker* worker, const Local<JSClass>& other);
	template<>
	Qk_EXPORT Local<JSValue> PersistentBase<JSValue>::toLocal() const;
	template<>
	Qk_EXPORT bool PersistentBase<JSValue>::isWeak();
	template<>
	Qk_EXPORT void PersistentBase<JSValue>::clearWeak();
	template<>
	Qk_EXPORT void PersistentBase<JSValue>::setWeak(void *ptr, WeakCallback cb);
	template<> template<>
	Qk_EXPORT void PersistentBase<JSValue>::copy(const PersistentBase<JSValue>& that);
	template<> template<>
	Qk_EXPORT void CopyablePersistentClass::copy(const PersistentBase<JSClass>& that);

	template<> Qk_EXPORT void ReturnValue::set<JSValue>(Local<JSValue> value);

	template<class T>
	bool JSObject::setProperty(Worker* worker, cString& name, T value) {
		return Set(worker, worker->New(name, 1), worker->New(value));
	}
	template<class T>
	bool JSClass::setMemberProperty(Worker* worker, cString& name, T value) {
		return SetMemberProperty<Local<JSValue>>(worker, name, worker->New(value));
	}
	template<class T>
	bool JSClass::setStaticProperty(Worker* worker, cString& name, T value) {
		return SetStaticProperty<Local<JSValue>>(worker, name, worker->New(value));
	}
	template<> Qk_EXPORT bool JSClass::setMemberProperty<Local<JSValue>>(
		Worker* worker, cString& name, Local<JSValue> value
	);
	template<> Qk_EXPORT bool JSClass::setStaticProperty<Local<JSValue>>(
		Worker* worker, cString& name, Local<JSValue> value
	);
	template<class T>
	Local<T> MaybeLocal<T>::toLocalChecked() {
		reinterpret_cast<MaybeLocal<JSValue>*>(this)->toLocalChecked();
		return Local<T>(_val);
	}
	template <> Qk_EXPORT Local<JSValue> MaybeLocal<JSValue>::toLocalChecked();

} }
#endif