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

#ifndef __langou__js__js__
#define __langou__js__js__

#include "langou/utils/util.h"
#include "langou/utils/string.h"
#include "langou/utils/buffer.h"
#include "langou/utils/map.h"
#include "langou/utils/error.h"
#include "langou/utils/fs.h"
#include "langou/utils/json.h"

// ------------- js common macro -------------

#define JS_BEGIN         XX_NS(langou) XX_NS(js)
#define JS_END           XX_END XX_END
#define JS_WORKER(...)   auto worker = Worker::worker(__VA_ARGS__)
#define JS_RETURN(rev)   return worker->result(args, (rev))
#define JS_RETURN_NULL() return worker->result(args, worker->NewNull())
#define JS_UNPACK(type)  auto wrap = langou::js::WrapObject::unpack<type>(args.This())
#define JS_SELF(type)    auto self = langou::js::WrapObject::unpack<type>(args.This())->self()
#define JS_HANDLE_SCOPE() HandleScope scope(worker)

#define JS_THROW_ERROR(Error, err, ...) \
	return worker->throw_err(worker->New##Error((err), ##__VA_ARGS__))

#define JS_THROW_ERR(err, ...) JS_THROW_ERROR(Error, err, ##__VA_ARGS__)
#define JS_TRY_CATCH(block, Error) try block catch(const Error& e) { JS_THROW_ERR(e); }

#define JS_REG_MODULE(name, cls) \
	XX_INIT_BLOCK(JS_REG_MODULE_##name) { \
		XX_DEBUG("%s", "JS_REG_MODULE "#name""); \
		langou::js::Worker::reg_module(#name, cls::binding, __FILE__); \
	}

#define JS_TYPEID(t) (typeid(t).hash_code())
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

namespace langou {
	class HttpError;
}

/**
 * @ns langou::js
 */

JS_BEGIN

class ValueProgram;
class Worker;
class V8WorkerIMPL;
class JSCWorker;
class WrapObject;
class Allocator;
class CommonStrings;
template<class T> class Maybe;
template<class T> class Local;
template<class T> class NonCopyablePersistentTraits;
template<class T> class PersistentBase;
template <class T, class M = NonCopyablePersistentTraits<T> >
class Persistent;
class JSValue;
class JSString;
class JSObject;
class JSArray;
class JSDate;
class JSNumber;
class JSInt32;
class JSInteger;
class JSUint32;
class JSBoolean;
class JSFunction;
class JSArrayBuffer;
class JSClass;

class XX_EXPORT NoCopy {
 public:
	XX_INLINE NoCopy() {}
	XX_HIDDEN_ALL_COPY(NoCopy);
	XX_HIDDEN_HEAP_ALLOC();
};

template<class T>
class XX_EXPORT Maybe {
 public:
	Maybe() : val_ok_(false) {}
	explicit Maybe(const T& t) : val_ok_(true), val_(t) {}
	explicit Maybe(T&& t) : val_ok_(true), val_(move(t)) {}
	XX_INLINE bool Ok() const { return val_ok_; }
	XX_INLINE bool To(T& out) {
		if ( val_ok_ ) {
			out = move(val_); return true;
		}
		return false;
	}
	XX_INLINE T FromMaybe(const T& default_value) {
		return val_ok_ ? move(val_) : default_value;
	}
 private:
	bool val_ok_;
	T val_;
};

template <class T>
class XX_EXPORT MaybeLocal {
 public:
	XX_INLINE MaybeLocal() : val_(nullptr) {}
	template <class S>
	XX_INLINE MaybeLocal(Local<S> that)
	: val_(reinterpret_cast<T*>(*that)) {
		JS_TYPE_CHECK(T, S);
	}
	XX_INLINE bool IsEmpty() const { return val_ == nullptr; }
	template <class S>
	XX_INLINE bool ToLocal(Local<S>* out) const {
		out->val_ = IsEmpty() ? nullptr : this->val_;
		return !IsEmpty();
	}
	XX_INLINE Local<T> ToLocalChecked();
	template <class S>
	XX_INLINE Local<S> FromMaybe(Local<S> default_value) const {
		return IsEmpty() ? default_value : Local<S>(val_);
	}
 private:
	T* val_;
};

template<class T>
class XX_EXPORT Local {
 public:
	XX_INLINE Local() : val_(0) {}
	template <class S>
	XX_INLINE Local(Local<S> that)
	: val_(reinterpret_cast<T*>(*that)) {
		JS_TYPE_CHECK(T, S);
	}
	XX_INLINE bool IsEmpty() const { return val_ == 0; }
	XX_INLINE void Clear() { val_ = 0; }
	XX_INLINE T* operator->() const { return val_; }
	XX_INLINE T* operator*() const { return val_; }
	template <class S> XX_INLINE static Local<T> Cast(Local<S> that) {
		return Local<T>( static_cast<T*>(*that) );
	}
	template <class S = JSObject> XX_INLINE Local<S> To() const {
		// unsafe conversion 
		return Local<S>::Cast(*this);
	}
 private:
	friend class JSValue;
	friend class JSFunction;
	friend class JSString;
	friend class JSClass;
	friend class Worker;
	explicit XX_INLINE Local(T* that) : val_(that) { }
	T* val_;
};

template<class T>
class XX_EXPORT PersistentBase: public NoCopy {
 public:
	typedef void (*WeakCallback)(void* ptr);
	XX_INLINE void Reset() {
		JS_TYPE_CHECK(JSValue, T);
		reinterpret_cast<PersistentBase<JSValue>*>(this)->Reset();
	}
	template <class S>
	XX_INLINE void Reset(Worker* worker, const Local<S>& other) {
		JS_TYPE_CHECK(T, S);
		JS_TYPE_CHECK(JSValue, T);
		reinterpret_cast<PersistentBase<JSValue>*>(this)->
		Reset(worker, *reinterpret_cast<const Local<JSValue>*>(&other));
	}
	template <class S>
	XX_INLINE void Reset(Worker* worker, const PersistentBase<S>& other) {
		JS_TYPE_CHECK(T, S);
		reinterpret_cast<PersistentBase<JSValue>*>(this)->Reset(worker, other.local());
	}
	XX_INLINE bool IsEmpty() const { return val_ == 0; }
	XX_INLINE Local<T> local() const {
		return *reinterpret_cast<Local<T>*>(const_cast<PersistentBase*>(this));
	}
	XX_INLINE Worker* worker() const { return worker_; }
 private:
	friend class WrapObject;
	friend class Worker;
	template<class F1, class F2> friend class Persistent;
	XX_INLINE PersistentBase(): val_(0), worker_(0) { }
	XX_INLINE void Empty() { val_ = 0; }
	template<class S> void Copy(const PersistentBase<S>& that);
	T* val_;
	Worker* worker_;
};

template<class T> class
XX_EXPORT NonCopyablePersistentTraits {
 public:
	static constexpr bool kResetInDestructor = true;
	XX_INLINE static void CopyCheck() { Uncompilable<Object>(); }
	template<class O> static void Uncompilable() {
		JS_TYPE_CHECK(O, JSValue);
	}
};

template<class T> class
XX_EXPORT CopyablePersistentTraits {
 public:
	typedef Persistent<T, CopyablePersistentTraits<T>> Handle;
	static constexpr bool kResetInDestructor = true;
	static XX_INLINE void CopyCheck() { }
};

template<class T, class M>
class XX_EXPORT Persistent: public PersistentBase<T> {
 public:
	XX_INLINE Persistent() { }
	
	~Persistent() { if(M::kResetInDestructor) this->Reset(); }
	
	template <class S>
	XX_INLINE Persistent(Worker* worker, Local<S> that) {
		this->Reset(worker, that);
	}
	
	template <class S, class M2>
	XX_INLINE Persistent(Worker* worker, const Persistent<S, M2>& that) {
		this->Reset(worker, that);
	}
	
	XX_INLINE Persistent(const Persistent& that) {
		Copy(that);
	}
	
	template<class S, class M2>
	XX_INLINE Persistent(const Persistent<S, M2>& that) {
		Copy(that);
	}
	
	XX_INLINE Persistent& operator=(const Persistent& that) {
		Copy(that);
		return *this;
	}
	
	template <class S, class M2>
	XX_INLINE Persistent& operator=(const Persistent<S, M2>& that) {
		Copy(that);
		return *this;
	}
	
 private:
	template<class F1, class F2> friend class Persistent;
	template<class S>
	XX_INLINE void Copy(const PersistentBase<S>& that) {
		M::CopyCheck();
		JS_TYPE_CHECK(T, S);
		JS_TYPE_CHECK(JSValue, T);
		this->Reset();
		if ( that.IsEmpty() ) return;
		reinterpret_cast<PersistentBase<JSValue>*>(this)->
			Copy(*reinterpret_cast<const PersistentBase<JSValue>*>(&that));
	}
};

class XX_EXPORT ReturnValue {
 public:
	template <class S>
	inline void Set(Local<S> value) {
		JS_TYPE_CHECK(JSValue, S);
		auto _ = reinterpret_cast<Local<JSValue>*>(&value);
		Set(*_);
	}
	void Set(bool value);
	void Set(double i);
	void Set(int i);
	void Set(uint i);
	void SetNull();
	void SetUndefined();
	void SetEmptyString();
 private:
	void* val_;
};

class XX_EXPORT FunctionCallbackInfo: public NoCopy {
 public:
	Worker* worker() const;
	int Length() const;
	Local<JSValue> operator[](int i) const;
	Local<JSObject> This() const;
	bool IsConstructCall() const;
	ReturnValue GetReturnValue() const;
};

class XX_EXPORT PropertyCallbackInfo: public NoCopy {
 public:
	Worker* worker() const;
	Local<JSObject> This() const;
	ReturnValue GetReturnValue() const;
};

class XX_EXPORT PropertySetCallbackInfo: public NoCopy {
 public:
	Worker* worker() const;
	Local<JSObject> This() const;
};

typedef const FunctionCallbackInfo& FunctionCall;
typedef const PropertyCallbackInfo& PropertyCall;
typedef const PropertySetCallbackInfo& PropertySetCall;
typedef void (*FunctionCallback)(FunctionCall args);
typedef void (*AccessorGetterCallback)(Local<JSString> name, PropertyCall args);
typedef void (*AccessorSetterCallback)(Local<JSString> name, Local<JSValue> value, PropertySetCall args);
typedef void (*IndexedPropertyGetterCallback)(uint index, PropertyCall info);
typedef void (*IndexedPropertySetterCallback)(uint index, Local<JSValue> value, PropertyCall info);

class XX_EXPORT HandleScope: public NoCopy {
 public:
	explicit HandleScope(Worker* worker);
	~HandleScope();
 private:
	void* val_[3];
};

class XX_EXPORT JSValue: public NoCopy {
 public:
	bool IsUndefined() const;
	bool IsUndefined(Worker* worker) const;
	bool IsNull() const;
	bool IsNull(Worker* worker) const;
	bool IsString() const;
	bool IsString(Worker* worker) const;
	bool IsBoolean() const;
	bool IsBoolean(Worker* worker) const;
	bool IsObject() const;
	bool IsObject(Worker* worker) const;
	bool IsArray() const;
	bool IsArray(Worker* worker) const;
	bool IsDate() const;
	bool IsDate(Worker* worker) const;
	bool IsNumber() const;
	bool IsNumber(Worker* worker) const;
	bool IsUint32() const;
	bool IsUint32(Worker* worker) const;
	bool IsInt32() const;
	bool IsInt32(Worker* worker) const;
	bool IsFunction() const;
	bool IsFunction(Worker* worker) const;
	bool IsArrayBuffer() const;
	bool IsArrayBuffer(Worker* worker) const;
	bool IsTypedArray() const;
	bool IsTypedArray(Worker* worker) const;
	bool Equals(Local<JSValue> val) const;
	bool Equals(Worker* worker, Local<JSValue> val) const;
	bool StrictEquals(Local<JSValue> val) const;
	bool StrictEquals(Worker* worker, Local<JSValue> val) const;
	Local<JSString> ToString(Worker* worker) const;
	Local<JSNumber> ToNumber(Worker* worker) const;
	Local<JSInt32> ToInt32(Worker* worker) const;
	Local<JSUint32> ToUint32(Worker* worker) const;
	Local<JSObject> ToObject(Worker* worker) const;
	Local<JSBoolean> ToBoolean(Worker* worker) const;
	String ToStringValue(Worker* worker, bool ascii = false) const;
	Ucs2String ToUcs2StringValue(Worker* worker) const;
	bool ToBooleanValue(Worker* worker) const;
	double ToNumberValue(Worker* worker) const;
	int ToInt32Value(Worker* worker) const;
	uint ToUint32Value(Worker* worker) const;
	Maybe<double> ToNumberMaybe(Worker* worker) const;
	Maybe<int> ToInt32Maybe(Worker* worker) const;
	Maybe<uint> ToUint32Maybe(Worker* worker) const;
	Buffer ToBuffer(Worker* worker, Encoding en) const;
	bool InstanceOf(Worker* worker, Local<JSObject> value);
};

class XX_EXPORT JSString: public JSValue {
 public:
	int Length(Worker* worker) const;
	String Value(Worker* worker, bool ascii = false) const;
	Ucs2String Ucs2Value(Worker* worker) const;
	static Local<JSString> Empty(Worker* worker);
};

class XX_EXPORT JSObject: public JSValue {
 public:
	Local<JSValue> Get(Worker* worker, Local<JSValue> key);
	Local<JSValue> Get(Worker* worker, uint index);
	bool Set(Worker* worker, Local<JSValue> key, Local<JSValue> val);
	bool Set(Worker* worker, uint index, Local<JSValue> val);
	bool Has(Worker* worker, Local<JSValue> key);
	bool Has(Worker* worker, uint index);
	bool Delete(Worker* worker, Local<JSValue> key);
	bool Delete(Worker* worker, uint index);
	Local<JSArray> GetPropertyNames(Worker* worker);
	Maybe<Map<String, int>> ToIntegerMap(Worker* worker);
	Maybe<Map<String, String>> ToStringMap(Worker* worker);
	Maybe<JSON> ToJSON(Worker* worker);
	Local<JSValue> GetProperty(Worker* worker, cString& name);
	Local<JSFunction> GetConstructor(Worker* worker);
	template<class T>
	bool SetProperty(Worker* worker, cString& name, T value);
	bool SetMethod(Worker* worker, cString& name, FunctionCallback func);
	bool SetAccessor(Worker* worker, cString& name,
									 AccessorGetterCallback get, AccessorSetterCallback set = nullptr);
};

class XX_EXPORT JSArray: public JSObject {
 public:
	int Length(Worker* worker) const;
	Maybe<Array<String>> ToStringArrayMaybe(Worker* worker);
	Maybe<Array<double>> ToNumberArrayMaybe(Worker* worker);
	Maybe<Buffer> ToBufferMaybe(Worker* worker);
};

class XX_EXPORT JSDate: public JSObject {
 public:
	double ValueOf(Worker* worker) const;
};

class XX_EXPORT JSNumber: public JSValue {
 public:
	double Value(Worker* worker) const;
};

class XX_EXPORT JSInt32: public JSNumber {
 public:
	int Value(Worker* worker) const;
};

class XX_EXPORT JSInteger: public JSNumber {
 public:
	int64 Value(Worker* worker) const;
};

class XX_EXPORT JSUint32: public JSNumber {
 public:
	uint Value(Worker* worker) const;
};

class XX_EXPORT JSBoolean: public JSValue {
 public:
	bool Value(Worker* worker) const;
};

class XX_EXPORT JSFunction: public JSObject {
 public:
	Local<JSValue> Call(Worker* worker, int argc = 0,
											Local<JSValue> argv[] = nullptr,
											Local<JSValue> recv = Local<JSValue>());
	Local<JSValue> Call(Worker* worker, Local<JSValue> recv);
	Local<JSObject> NewInstance(Worker* worker, int argc = 0,
															Local<JSValue> argv[] = nullptr);
};

class XX_EXPORT JSArrayBuffer: public JSObject {
 public:
	int ByteLength(Worker* worker) const;
	char* Data(Worker* worker);
	WeakBuffer weak_buffer(Worker* worker);
	static Local<JSArrayBuffer> New(Worker* worker, char* buff, uint len);
};

class XX_EXPORT JSTypedArray: public JSObject {
 public:
	Local<JSArrayBuffer> Buffer(Worker* worker);
	WeakBuffer weak_buffer(Worker* worker);
};

class XX_EXPORT JSClass: public NoCopy {
 public:
	uint64 ID() const;
	bool HasInstance(Worker* worker, Local<JSValue> val);
	Local<JSFunction> GetFunction(Worker* worker);
	Local<JSObject> NewInstance(uint argc = 0, Local<JSValue>* argv = nullptr);
	bool SetMemberMethod(Worker* worker, cString& name, FunctionCallback func);
	bool SetMemberAccessor(Worker* worker, cString& name,
												 AccessorGetterCallback get,
												 AccessorSetterCallback set = nullptr);
	bool SetMemberIndexedAccessor(Worker* worker,
																IndexedPropertyGetterCallback get,
																IndexedPropertySetterCallback set = nullptr);
	template<class T>
	bool SetMemberProperty(Worker* worker, cString& name, T value);
	template<class T>
	bool SetStaticProperty(Worker* worker, cString& name, T value);
	void Export(Worker* worker, cString& name, Local<JSObject> exports);
	void SetInstanceInternalFieldCount(int count);
	int InstanceInternalFieldCount();
};

class XX_EXPORT TryCatch: public NoCopy {
 public:
	TryCatch();
	~TryCatch();
	bool HasCaught() const;
	Local<JSValue> Exception() const;
 private:
	void* val_;
};

/**
 * @class Worker
 */
class XX_EXPORT Worker: public Object {
	XX_HIDDEN_ALL_COPY(Worker);
 public:
	typedef void (*BindingCallback)(Local<JSObject> exports, Worker* worker);
	typedef void (*WrapAttachCallback)(WrapObject* wrap);
	
	Worker* create();
	
	/**
	 * @destructor
	 */
	virtual ~Worker();

	/**
	 * @func worker() get current thread worker
	 */
	static Worker* worker();
	
	template<class T>
	inline static Worker* worker(T& args) {
		return args.worker();
	}
	
	/**
	 * @func reg_module
	 */
	static void reg_module(cString& name,
												 BindingCallback binding, cchar* file = nullptr);

	/**
	 * @func binding_module
	 */
	Local<JSValue> binding_module(cString& name);
	
	/**
	 * @func New()
	 */
	Local<JSNumber> New(float data);
	Local<JSNumber> New(double data);
	Local<JSBoolean>New(bool data);
	Local<JSInt32>  New(char data);
	Local<JSUint32> New(byte data);
	Local<JSInt32>  New(int16 data);
	Local<JSUint32> New(uint16 data);
	Local<JSInt32>  New(int data);
	Local<JSUint32> New(uint data);
	Local<JSNumber> New(int64 data);
	Local<JSNumber> New(uint64 data);
	Local<JSString> New(cchar* data);
	Local<JSString> New(cString& data, bool is_ascii = false);
	Local<JSString> New(cUcs2String& data);
	Local<JSObject> New(cError& data);
	Local<JSObject> New(const HttpError& err);
	Local<JSArray>  New(const Array<String>& data);
	Local<JSArray>  New(Array<FileStat>& data);
	Local<JSArray>  New(Array<FileStat>&& data);
	Local<JSObject> New(const Map<String, String>& data);
	Local<JSObject> New(Buffer& buff);
	Local<JSObject> New(Buffer&& buff);
	Local<JSObject> New(FileStat& stat);
	Local<JSObject> New(FileStat&& stat);
	Local<JSObject> New(const Dirent& dir);
	Local<JSArray>  New(Array<Dirent>& data);
	Local<JSArray>  New(Array<Dirent>&& data);
	
	template <class T>
	XX_INLINE Local<T> New(Local<T> val) { return val; }
	
	template <class T>
	XX_INLINE Local<T> New(const PersistentBase<T>& value) {
		auto r = New(*reinterpret_cast<const PersistentBase<JSValue>*>(&value));
		auto r_ = reinterpret_cast<Local<T>*>(&r);
		return *r_;
	}
	
	Local<JSValue> New(const PersistentBase<JSValue>& value);
	
	Local<JSObject> NewInstance(uint64 id, uint argc = 0, Local<JSValue>* argv = nullptr);
	Local<JSString> NewString(cBuffer& data);
	Local<JSString> NewString(cchar* str, uint len);
	Local<JSObject> NewBuffer(Local<JSString> str, Encoding enc = Encoding::utf8);
	Local<JSObject> NewRangeError(cchar* errmsg, ...);
	Local<JSObject> NewReferenceError(cchar* errmsg, ...);
	Local<JSObject> NewSyntaxError(cchar* errmsg, ...);
	Local<JSObject> NewTypeError(cchar* errmsg, ...);
	Local<JSObject> NewError(cchar* errmsg, ...);
	Local<JSObject> NewError(cError& err);
	Local<JSObject> NewError(const HttpError& err);
	Local<JSObject> NewError(Local<JSObject> value);
	Local<JSObject> NewObject();
	Local<JSArray>  NewArray(uint len = 0);
	Local<JSValue>  NewNull();
	Local<JSValue>  NewUndefined();
	
	inline Local<JSBoolean> New(const Bool& v) { return New(v.value); }
	inline Local<JSNumber>  New(const Float& v) { return New(v.value); }
	inline Local<JSNumber>  New(const Double& v) { return New(v.value); }
	inline Local<JSInt32>   New(const Char& v) { return New(v.value); }
	inline Local<JSUint32>  New(const Byte& v) { return New(v.value); }
	inline Local<JSInt32>   New(const Int16& v) { return New(v.value); }
	inline Local<JSUint32>  New(const Uint16& v) { return New(v.value); }
	inline Local<JSInt32>   New(const Int& v) { return New(v.value); }
	inline Local<JSUint32>  New(const Uint& v) { return New(v.value); }
	inline Local<JSNumber>  New(const Int64& v) { return New(v.value); }
	inline Local<JSNumber>  New(const Uint64& v) { return New(v.value); }
	
	template<class T>
	static inline Local<JSValue> New(const Object& obj, Worker* worker) {
		return worker->New( static_cast<const T*>(&obj) );
	}
	
	/**
	 * @func throw_err
	 */
	void throw_err(Local<JSValue> exception);
	void throw_err(cchar* errmsg, ...);
	
	/**
	 * @func has_instance
	 */
	bool has_instance(Local<JSValue> val, uint64 id);
	
	/**
	 * @func has_buffer has javascript ArrayBufferView or ArrayBuffer
	 */
	bool has_buffer(Local<JSValue> val);
	
	/**
	 * @func has_typed_buffer() has javascript TypedArray or ArrayBuffer
	 */
	bool has_typed_buffer(Local<JSValue> val);
	
	/**
	 * @func has_view() has View type
	 */
	bool has_view(Local<JSValue> val);

	/**
	 * @func as_buffer()
	 */
	WeakBuffer as_buffer(Local<JSValue> val);
	
	/**
	 * @func as_buffer TypedArray or ArrayBuffer to WeakBuffer
	 */
	WeakBuffer as_typed_buffer(Local<JSValue> val);
	
	/**
	 * @func has_instance
	 */
	template<class T> inline bool has_instance(Local<JSValue> val) {
		return has_instance(val, JS_TYPEID(T));
	}
	
	/**
	 * @func js_class(id) find class
	 */
	Local<JSClass> js_class(uint id);
	
	/**
	 * @func result
	 */
	template <class Args, class T>
	inline void result(const Args& args, Local<T> data) {
		args.GetReturnValue().Set( data );
	}
	
	/**
	 * @func result
	 */
	template <class Args, class T>
	inline void result(const Args& args, const T& data) {
		args.GetReturnValue().Set( New(data) );
	}
	
	/**
	 * @func result
	 */
	template <class Args, class T>
	inline void result(const Args& args, T&& data) {
		args.GetReturnValue().Set( New(move(data)) );
	}
	
	/**
	 * @func NewClass js class
	 */
	Local<JSClass> NewClass(uint64 id, cString& name,
													FunctionCallback constructor,
													WrapAttachCallback attach_callback,
													Local<JSClass> base = Local<JSClass>());
	/**
	 * @func NewClass js class
	 */
	Local<JSClass> NewClass(uint64 id, cString& name,
													FunctionCallback constructor,
													WrapAttachCallback attach_callback, uint64 base);
	/**
	 * @func NewClass js class
	 */
	Local<JSClass> NewClass(uint64 id, cString& name,
													FunctionCallback constructor,
													WrapAttachCallback attach_callback, Local<JSFunction> base);
	/**
	 * @func run_script
	 */
	Local<JSValue> run_script(cString& source,
														cString& name,
														Local<JSObject> sandbox = Local<JSObject>());
	/**
	 * @func run_script
	 */
	Local<JSValue> run_script(Local<JSString> source,
														Local<JSString> name,
														Local<JSObject> sandbox = Local<JSObject>());
	/**
	 * @func run_native_script
	 */
	Local<JSValue> run_native_script(
		cBuffer& source, cString& name, 
		Local<JSObject> exports = Local<JSObject>());

	/**
	 * @func values
	 */
	ValueProgram* values();
	
	/**
	 * @func strs
	 */
	CommonStrings* strs();
	
	/**
	 * @func thread_id
	 */
	ThreadID thread_id();
	
	/**
	 * @func global()
	 */
	Local<JSObject> global();
	
	/**
	 * @func report_exception
	 */
	void report_exception(TryCatch* try_catch);
	
	/**
	 * @func garbage_collection()
	 */
	void garbage_collection();
	
 private:

	friend class NativeValue;
	friend class WorkerIMPL;
	XX_DEFINE_INLINE_CLASS(IMPL);
	IMPL* m_inl;

	Worker(IMPL* inl);
};

// **********************************************************************

typedef CopyablePersistentTraits<JSClass>::Handle CopyablePersistentClass;
typedef CopyablePersistentTraits<JSFunction>::Handle CopyablePersistentFunc;
typedef CopyablePersistentTraits<JSObject>::Handle CopyablePersistentObject;
typedef CopyablePersistentTraits<JSValue>::Handle CopyablePersistentValue;

template <>
XX_EXPORT void PersistentBase<JSValue>::Reset();
template <>
XX_EXPORT void PersistentBase<JSClass>::Reset();
template <> template <>
XX_EXPORT void PersistentBase<JSValue>::Reset(Worker* worker, const Local<JSValue>& other);
template <> template <>
XX_EXPORT void PersistentBase<JSClass>::Reset(Worker* worker, const Local<JSClass>& other);
template<> template<>
XX_EXPORT void PersistentBase<JSValue>::Copy(const PersistentBase<JSValue>& that);
template<> template<>
XX_EXPORT void CopyablePersistentClass::Copy(const PersistentBase<JSClass>& that);

template<> XX_EXPORT void ReturnValue::Set<JSValue>(Local<JSValue> value);

template<class T>
bool JSObject::SetProperty(Worker* worker, cString& name, T value) {
	return Set(worker, worker->New(name, 1), worker->New(value));
}
template<class T>
bool JSClass::SetMemberProperty(Worker* worker, cString& name, T value) {
	return SetMemberProperty<Local<JSValue>>(worker, name, worker->New(value));
}
template<class T>
bool JSClass::SetStaticProperty(Worker* worker, cString& name, T value) {
	return SetStaticProperty<Local<JSValue>>(worker, name, worker->New(value));
}
template<> XX_EXPORT bool JSClass::SetMemberProperty<Local<JSValue>>
(
 Worker* worker, cString& name, Local<JSValue> value
 );
template<> XX_EXPORT bool JSClass::SetStaticProperty<Local<JSValue>>
(
 Worker* worker, cString& name, Local<JSValue> value
 );
template<class T>
Local<T> MaybeLocal<T>::ToLocalChecked() {
	reinterpret_cast<MaybeLocal<JSValue>*>(this)->ToLocalChecked();
	return Local<T>(val_);
}
template <> XX_EXPORT Local<JSValue> MaybeLocal<JSValue>::ToLocalChecked();

JS_END
#endif
