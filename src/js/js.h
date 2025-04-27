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
#include "../util/dict.h"
#include "../util/http.h"
#include "../util/codec.h"

namespace qk { namespace js {
	#define Js_Worker(...)   auto worker = Worker::worker(__VA_ARGS__)
	#define Js_Return(v)     return args.returnValue().set(worker->newValue(v))
	#define Js_ReturnBool(v) return args.returnValue().set(bool(v))
	#define Js_Return_Null() return args.returnValue().setNull()
	#define Js_Mix(type)     auto mix = qk::js::MixObject::mix<type>(args.thisObj())
	#define Js_Self(type)    auto self = qk::js::MixObject::mix<type>(args.thisObj())->self()
	#define Js_Handle_Scope() qk::js::HandleScope scope(worker)

	#define Js_Throw_Error(Error, err, ...) \
		return worker->throwError(worker->new##Error((err), ##__VA_ARGS__))
	#define Js_Throw(err, ...) Js_Throw_Error(Error, err, ##__VA_ARGS__)
	#define Js_Try_Catch(block, Error) try block catch(const Error& e) { Js_Throw(e); }

	#define Js_Module(name, cls) \
		Qk_Init_Func(Js_Module_##name) {\
			qk::js::Worker::setModule(#name, cls::binding, __FILE__); \
		}
	#define Js_Typeid(t) (typeid(t).hash_code())
	#define Js_Type_Check(T, S) Qk_Type_Check(T, S)

	// define class macro
	#define Js_New_Class(name, alias, base, constructor) \
		static_assert(sizeof(MixObject)==sizeof(Mix##name), \
			"Derived mix class pairs cannot declare data members"); \
		auto cls = worker->newClass(#name,alias,constructor,([](auto o){new(o) Mix##name();}),base)

	#define Js_Define_Class(name, base, constructor) \
		Js_New_Class(name,Js_Typeid(name),Js_Typeid(base),_Js_Fun(constructor))

	#define _Js_Fun(f)([](auto args){auto worker=args.worker();f})
	#define _Js_Get(f)([](auto key,auto args){auto worker=args.worker();f})
	#define _Js_Set(f)([](auto key,auto val,auto args){auto worker=args.worker();f})
	// object
	#define Js_Method(name,func)             exports->setMethod(worker,#name,_Js_Fun(func))
	#define Js_Accessor(name,get,set)        exports->setAccessor(worker,#name,_Js_Get(get),_Js_Set(set))
	#define Js_Accessor_Get(name,get)        exports->getAccessor(worker,#name,_Js_Get(get))
	#define Js_Accessor_Set(name,set)        exports->setAccessor(worker,#name,0,_Js_Set(set))
	#define Js_Property(name, value)         exports->setProperty(worker,#name,value)
	// class
	#define Js_Class_Accessor(name,get,set)  cls->setAccessor(#name,_Js_Get(get),_Js_Set(set))
	#define Js_Class_Accessor_Get(name,get)  cls->setAccessor(#name,_Js_Get(get))
	#define Js_Class_Accessor_Set(name,set)  cls->setAccessor(#name,0,_Js_Set(set))
	#define Js_Class_Method(name,func)       cls->setMethod(#name,_Js_Fun(func))
	#define Js_Class_Indexed(get,set)        cls->setIndexedAccessor(_Js_Get(get),_Js_Set(set))
	#define Js_Class_Indexed_Get(get)        cls->setIndexedAccessor(_Js_Get(get),0)
	#define Js_Class_Indexed_Set(set)        cls->setIndexedAccessor(0,_Js_Set(set))
	#define Js_Class_Property(name,value)    cls->setProperty(#name,value)
	#define Js_Class_Static_Property(name,value) cls->setStaticProperty(#name,value)
	#define Js_Class_Static_Method(name,value) cls->setStaticMethod(#name,_Js_Fun(func))

	// -------------------------------------------------------------------

	class Worker;
	class MixObject;
	class JSValue;
	class JSObject;
	class TypesParser;
	class Strings;
	class JsClasses;
	class JSString;
	class JSNumber;
	class JSInt32;
	class JSUint32;
	class JSBoolean;
	class JSArray;
	class JSFunction;
	class JSClass;

	class NoCopy {
	public:
		inline NoCopy() {}
		Qk_HIDDEN_ALL_COPY(NoCopy);
		Qk_HIDDEN_HEAP_ALLOC();
	};

	template<class T> class Persistent: public NoCopy {
		T      *_val;
		Worker *_worker;
	public:
		inline Persistent(): _val(0) , _worker(0) {
			Js_Type_Check(JSValue, T);
		}
		template <class S>
		inline Persistent(Worker* worker, S* that): _val(0), _worker(0) {
			Js_Type_Check(JSValue, T);
			reset(worker, that);
		}
		inline ~Persistent() {
			reset();
		}
		inline void reset() {
			reinterpret_cast<Persistent<JSValue>*>(this)->reset();
		}
		template <class S>
		inline void reset(Worker* worker, S* other) {
			reinterpret_cast<Persistent<JSValue>*>(this)->reset(worker, static_cast<JSValue*>(other));
		}
		template <class S>
		inline void reset(const Persistent<S>& other) {
			reinterpret_cast<Persistent<JSValue>*>(this)->reset(other._worker, static_cast<JSValue*>(*other));
		}
		template<class S>
		void copy(const Persistent<S>& from) {
			Js_Type_Check(T, S);
			reinterpret_cast<Persistent<JSValue>*>(this)->
				copy(*reinterpret_cast<const Persistent<JSValue>*>(&from));
		}
		inline bool isEmpty() const { return _val == nullptr; }
		inline T* operator->() const { return operator*(); }
		inline T* operator*() const {
			return static_cast<T*>(reinterpret_cast<const Persistent<JSValue>*>(this)->operator*());
		}
		inline operator bool() const { return _val; }
		inline Worker* worker() const { return _worker; }

		friend class MixObject;
	};

	template<class T> class Maybe {
		T  _val;
		Qk_DEFINE_PROP_GET(bool, ok);
		Maybe(): _ok(false) {}
		Maybe(const T& t): _val(t),_ok(true) {}
		Maybe(T&& t): _val(std::move(t)), _ok(true) {}
		inline operator bool() const { return _ok; }
		inline bool to(T& out) { return _ok ? (out = std::move(_val), true): false; }
		inline T from(const T& defaultValue) { return _ok ? std::move(_val) : defaultValue; }
	};

	class Qk_EXPORT HandleScope: public NoCopy {
	public:
		explicit HandleScope(Worker* worker);
		~HandleScope();
	protected:
		inline HandleScope() = default;
		void *_val[4];
	};

	class Qk_EXPORT EscapableHandleScope: public HandleScope {
	public:
		explicit EscapableHandleScope(Worker* worker);
		template<class T> inline T* escape(T* val) {
			return static_cast<T*>(escape(static_cast<JSValue*>(val)));
		}
	};

	class Qk_EXPORT TryCatch: public NoCopy {
	public:
		TryCatch(Worker *worker);
		~TryCatch();
		bool hasCaught() const;
		JSValue* exception() const;
		void reThrow();
		void print() const;
	private:
		void *_val;
	};

	class Qk_EXPORT ReturnValue {
	public:
		Worker* worker();
		void set(bool value);
		void set(float val);
		void set(double i);
		void set(int32_t i);
		void set(uint32_t i);
		void set(JSValue *value);
		void setNull();
		void setUndefined();
		void setEmptyString();
	private:
		void *_val;
	};

	class Qk_EXPORT FunctionCallbackInfo {
	public:
		Worker* worker() const;
		JSObject* thisObj() const;
		ReturnValue returnValue() const;
		JSValue* operator[](int i) const;
		int length() const;
		bool isConstructCall() const;
	};

	class Qk_EXPORT PropertyCallbackInfo {
	public:
		Worker* worker() const;
		JSObject* thisObj() const;
		ReturnValue returnValue() const;
	};

	class Qk_EXPORT PropertySetCallbackInfo {
	public:
		Worker* worker() const;
		JSObject* thisObj() const;
	};

	typedef const FunctionCallbackInfo& FunctionArgs;
	typedef const PropertyCallbackInfo& PropertyArgs;
	typedef const PropertySetCallbackInfo& PropertySetArgs;
	typedef void (*FunctionCallback)(FunctionArgs args);
	typedef void (*AccessorGetterCallback)(JSValue* name, PropertyArgs args);
	typedef void (*AccessorSetterCallback)(JSValue* name, JSValue* value, PropertySetArgs args);
	typedef void (*IndexedAccessorGetterCallback)(uint32_t index, PropertyArgs args);
	typedef void (*IndexedAccessorSetterCallback)(uint32_t index, JSValue* value, PropertySetArgs args);
	typedef void (*BindingCallback)(JSObject* exports, Worker* worker);
	typedef void (*AttachCallback)(MixObject* mix);

	class Qk_EXPORT JSValue {
	public:
		bool isUndefined() const;
		bool isNull() const;
		bool isString() const;
		bool isBoolean() const;
		bool isObject() const;
		bool isArray() const;
		bool isDate() const;
		bool isNumber() const;
		bool isUint32() const;
		bool isInt32() const;
		bool isFunction() const;
		bool isArrayBuffer() const;
		bool isTypedArray() const;
		bool isUint8Array() const;
		bool isBuffer() const; // IsTypedArray or IsArrayBuffer
		bool equals(Worker *worker, JSValue* val) const;
		bool strictEquals(Worker *worker, JSValue* val) const;
		bool instanceOf(Worker* worker, JSObject* constructor) const; // this instanceOf constructor
		// Force convert to type xxx
		bool      toBoolean(Worker* worker) const;
		JSString* toString(Worker* worker) const;
		JSNumber* toNumber(Worker* worker) const;
		JSInt32*  toInt32(Worker* worker) const;
		JSUint32* toUint32(Worker* worker) const;

		// Convert to type xxx
		Maybe<String> asString(Worker* worker) const;
		Maybe<double> asNumber(Worker* worker) const;
		Maybe<float> asFloat32(Worker* worker) const;
		Maybe<int32_t> asInt32(Worker* worker) const;
		Maybe<uint32_t> asUint32(Worker* worker) const;
		Maybe<WeakBuffer> asBuffer(Worker* worker) const;
		// Unsafe static convert
		template<class T = JSValue>
		inline T* cast() { return static_cast<T*>(this); }
	};

	class Qk_EXPORT JSString: public JSValue {
	public:
		int length() const; // utf16 length
		String value(Worker* worker) const; // utf8 string value
		String2 value2(Worker* worker) const;
		String4 value4(Worker* worker) const;
		Buffer toBuffer(Worker* worker, Encoding en = kUTF16_Encoding) const; // encode to en
	};

	class Qk_EXPORT JSObject: public JSValue {
	public:
		enum PropertyFlags {
			None = 0,
			ReadOnly = 1 << 0,
			DontEnum = 1 << 1,
			DontDelete = 1 << 2
		};
		template<class T = JSValue>
		T* get(Worker* worker, JSValue* key);
		template<class T = JSValue>
		T* get(Worker* worker, uint32_t index);
		template<class T = JSValue>
		T* get(Worker* worker, cString& key);
		bool set(Worker* worker, JSValue* key, JSValue* val);
		bool set(Worker* worker, uint32_t index, JSValue* val);
		template<class T>
		bool setProperty(Worker* worker, cString& name, T value);
		bool has(Worker* worker, JSValue* key);
		bool has(Worker* worker, uint32_t index);
		bool deleteFor(Worker* worker, JSValue* key);
		bool deleteFor(Worker* worker, uint32_t index);
		JSArray* getPropertyNames(Worker* worker);
		JSFunction* getConstructor(Worker* worker);
		bool setMethod(Worker* worker, cString& name, FunctionCallback func);
		bool setAccessor(Worker* worker, cString& name, AccessorGetterCallback get,
			AccessorSetterCallback set = nullptr);
		bool defineOwnProperty(Worker *worker, JSValue *key, JSValue *value, int flags = None);
		bool setPrototype(Worker* worker, JSObject* proto); // set obj.__proto__
		Maybe<Dict<String, int>>    toIntegerDict(Worker* worker);
		Maybe<Dict<String, String>> toStringDict(Worker* worker);
		Maybe<Array<String>>        getPropertyKeys(Worker* worker);
	};

	class Qk_EXPORT JSArray: public JSObject {
	public:
		int length() const;
		Maybe<Buffer> toBuffer(Worker* worker);
		Maybe<Array<String>> toStringArray(Worker* worker);
		Maybe<Array<double>> toNumberArray(Worker* worker);
	};

	class Qk_EXPORT JSDate: public JSObject {
	public:
		double valueOf() const;
	};

	class Qk_EXPORT JSNumber: public JSValue {
	public:
		double value() const;
		float float32() const;
	};

	class Qk_EXPORT JSInt32: public JSNumber {
	public:
		int32_t value() const;
	};

	class Qk_EXPORT JSUint32: public JSNumber {
	public:
		uint32_t value() const;
	};

	class Qk_EXPORT JSBoolean: public JSValue {
	public:
		bool value() const;
	};

	class Qk_EXPORT JSFunction: public JSObject {
	public:
		JSValue*  call(Worker* worker, JSValue* recv);
		JSValue*  call(Worker* worker, int argc = 0, JSValue* argv[] = 0, JSValue* recv = 0);
		JSObject* newInstance(Worker* worker, int argc = 0, JSValue* argv[] = 0); // call as constructor
		JSObject* getFunctionPrototype(Worker* worker); // funciton.prototype
	};

	class Qk_EXPORT JSArrayBuffer: public JSObject {
	public:
		uint32_t   byteLength(Worker* worker) const;
		char*      data(Worker* worker);
		WeakBuffer value(Worker* worker) const;
	};

	class Qk_EXPORT JSTypedArray: public JSObject {
	public:
		JSArrayBuffer* buffer(Worker* worker);
		WeakBuffer value(Worker* worker) const;
		uint32_t byteLength(Worker* worker) const;
		uint32_t byteOffset(Worker* worker) const;
	};

	class Qk_EXPORT JSUint8Array: public JSTypedArray {
	};

	class Qk_EXPORT JSSet: public JSObject {
	public:
		bool add(Worker* worker, JSValue* key);
		bool has(Worker* worker, JSValue* key);
		bool deleteFor(Worker* worker, JSValue* key);
	};

	class Qk_EXPORT JSClass {
		Qk_HIDDEN_ALL_COPY(JSClass);
	public:
		Qk_DEFINE_PROP_GET(Worker*, worker, Protected);
		Qk_DEFINE_PROP_GET(uint64_t, alias, Protected);
		void exports(cString& name, JSObject* exports);
		bool hasInstance(JSValue* val);
		JSFunction* getFunction(); // constructor function
		JSObject* newInstance(uint32_t argc = 0, JSValue* argv[] = nullptr);
		bool setMethod(cString& name, FunctionCallback func);
		bool setAccessor(cString& name, AccessorGetterCallback get,
			AccessorSetterCallback set = nullptr);
		bool setIndexedAccessor(IndexedAccessorGetterCallback get,
			IndexedAccessorSetterCallback set = nullptr);
		bool setStaticMethod(cString& name, FunctionCallback func);
		template<class T>
		bool setProperty(cString& name, T value);
		template<class T>
		bool setStaticProperty(cString& name, T value);
	protected:
		virtual void destroy() = 0;
		void callConstructor(FunctionArgs args);
		JSClass(FunctionCallback constructor, AttachCallback attach);
		FunctionCallback _constructor;
		AttachCallback _attachConstructor;
		friend class JsClasses;
		friend class MixObject;
	};

	template<class T> class Mix;

	class Qk_EXPORT MixObject {
		Qk_HIDDEN_ALL_COPY(MixObject);
	public:
		inline Worker* worker() {
			return _class->worker();
		}
		inline JSObject* handle() {
			return _handle;
		}
		inline JSClass* jsclass() {
			return _class;
		}
		inline uint32_t flags() {
			return _flags;
		}
		template<class Self = Object>
		inline Self* self() {
			return static_cast<Self*>(reinterpret_cast<Object*>(this + 1));
		}
		inline MixObject() {}
		virtual ~MixObject();
		virtual void initialize();
		virtual bool addEventListener(cString& name, cString& func, int id);
		virtual bool removeEventListener(cString& name, int id);

		// Calling member function
		JSValue* call(JSValue* method, int argc = 0, JSValue* argv[] = 0);
		JSValue* call(cString& method, int argc = 0, JSValue* argv[] = 0);

		/**
		* Note: value must instance of MixObject else causes the program to crash
		*/
		template<class T = MixObject>
		static inline T* mixObject(JSValue *value) {
			Js_Type_Check(MixObject, T);
			return static_cast<T*>(unpack(value));
		}
		template<class Self = Object>
		static inline Mix<Self>* mix(JSValue *value) {
			static_assert(object_traits<Self>::is::obj, "Must be object");
			return static_cast<Mix<Self>*>(unpack(value));
		}
		template<class Self>
		static inline Mix<Self>* mix(Self *object) {
			return mix(object, Js_Typeid(*object));
		}
		template<class Self>
		static inline Mix<Self>* mix(Self *object, uint64_t classAlias) {
			static_assert(object_traits<Self>::is::obj, "Must be object");
			return static_cast<js::Mix<Self>*>(pack(object, classAlias));
		}

	protected:
		template<class M, class Self>
		static Mix<Self>* New(FunctionArgs args, Self *self) {
			static_assert(sizeof(M) == sizeof(MixObject),
										"Derived mix class pairs cannot declare data members");
			static_assert(object_traits<Self>::is::obj, "Must be object");
			auto mix = (new(reinterpret_cast<MixObject*>(self) - 1) M())->newInit(args);
			return static_cast<Mix<Self>*>(static_cast<MixObject*>(mix));
		}

	private:
		static MixObject* pack(Object* obj, uint64_t classAlias);
		static MixObject* unpack(JSValue* obj);
		void clearWeak();
		void setWeak();
		void bindObject(JSObject* handle);
		MixObject* newInit(FunctionArgs args);

		JSObject *_handle;
		JSClass *_class;
		uint32_t _flags;
		friend class JsHeapAllocator;
	};

	template<class Self = Object>
	class Mix: public MixObject {
	public:
		inline Self* self() {
			return reinterpret_cast<Self*>(this + 1);
		}
	};

	class Qk_EXPORT Worker: public Object, public SafeFlag {
		Qk_HIDDEN_ALL_COPY(Worker);
	public:
		Qk_DEFINE_PROP_GET(TypesParser*, types, Protected);
		Qk_DEFINE_PROP_GET(Strings*, strs, Protected);
		Qk_DEFINE_PROP_GET(JsClasses*, classes, Protected);
		Qk_DEFINE_PROP_GET(ThreadID, thread_id, Protected);
		Qk_DEFINE_PROP_GET(RunLoop*, loop);
		Qk_DEFINE_ACCE_GET(JSObject*, global);

		static Worker* Make();
		static Worker* current();
		inline
		static Worker* worker() { return current(); }
		template<class T>
		static Worker* worker(T& args) { return args.worker(); }
		static void setModule(cString& name, BindingCallback binding, cChar* pathname);

		void release() override;
		void garbageCollection();

		// new instance
		JSValue*  newValue(Object *val);
		JSNumber* newValue(float val);
		JSNumber* newValue(double val);
		JSInt32*  newValue(int32_t val);
		JSUint32* newValue(uint32_t val);
		JSNumber* newValue(int64_t val);
		JSNumber* newValue(uint64_t val);
		JSString* newValue(cString& val);
		JSString* newValue(cString2& val);
		JSString* newValue(cString4& val);
		JSObject* newValue(cError& val);
		JSObject* newValue(cHttpError& val);
		JSObject* newValue(cDictSS& val);
		JSUint8Array* newValue(Buffer& val);
		JSUint8Array* newValue(Buffer&& val);
		JSArray*  newValue(cArray<String>& val);
		JSArray*  newValue(cArray<uint32_t>& val);
		template <class S>
		inline S* newValue(const Persistent<S>& val) { return *val; }
		inline JSValue* newValue(JSValue* val) { return val; }
		inline JSBoolean* newValue(bool val) { return newBool(val); }

		JSBoolean*    newBool(bool val);
		JSValue*      newNull();
		JSValue*      newUndefined();
		JSObject*     newObject();
		JSArray*      newArray();
		JSSet*        newSet();
		JSString*     newEmpty();
		JSString*     newString(cBuffer& val);
		JSString*     newStringOneByte(cString& val);
		JSArrayBuffer* newArrayBuffer(char* useBuffer, uint32_t len);
		JSArrayBuffer* newArrayBuffer(uint32_t len);
		JSUint8Array* newUint8Array(JSString* str, Encoding en = kUTF8_Encoding); // encode to en
		JSUint8Array* newUint8Array(int size, char fill = 0);
		JSUint8Array* newUint8Array(JSArrayBuffer* abuff);
		JSUint8Array* newUint8Array(JSArrayBuffer* abuff, uint32_t offset, uint32_t size);
		JSObject*     newRangeError(cChar* errmsg, ...);
		JSObject*     newReferenceError(cChar* errmsg, ...);
		JSObject*     newSyntaxError(cChar* errmsg, ...);
		JSObject*     newTypeError(cChar* errmsg, ...);
		JSObject*     newError(cChar* errmsg, ...);
		JSObject*     newError(cError& err);
		JSObject*     newError(JSObject* value);

		void throwError(JSValue* exception);
		void throwError(cChar* errmsg, ...);

		template<class T>
		inline bool instanceOf(JSValue* val) { // val instanceOf Js_Typeid(T)
			return instanceOf(val, Js_Typeid(T));
		}
		bool instanceOf(JSValue* val, uint64_t alias); // val instanceOf alias

		JSClass* jsclass(uint64_t alias); // Get js class by alias
		JSClass* newClass(cString& name, uint64_t alias,
											FunctionCallback constructor,
											AttachCallback attachConstructor, JSClass* base = 0);
		JSClass* newClass(cString& name, uint64_t alias,
											FunctionCallback constructor,
											AttachCallback attachConstructor, uint64_t base);
		JSClass* newClass(cString& name, uint64_t alias,
											FunctionCallback constructor,
											AttachCallback attachConstructor, JSFunction* base);
		JSFunction* newFunction(cString& name, FunctionCallback func); // new native function
		JSValue* runScript(cString& source, cString& name, JSObject* sandbox = 0);
		JSValue* runScript(JSString* source, JSString* name, JSObject* sandbox = 0);
		JSValue* runNativeScript(cChar* source, int sLen, cString& name, JSObject* exports = 0);
		JSValue* bindingModule(cString& name);

	protected:
		explicit Worker();
		void init();
		// props
		Persistent<JSObject> _global, _console;
		Persistent<JSObject> _nativeModules;
	private:
		template <class S> JSObject* newValue(cArray<S>& val); // Access Denied
	};

	Qk_EXPORT int Start(cString &cmd, cArray<String> &argv = {});
	Qk_EXPORT int Start(int argc, char** argv);

	// **********************************************************************

	template<class T>
	inline T* JSObject::get(Worker* worker, JSValue* key) {
		return static_cast<T*>(get<JSValue>(worker, key));
	}
	template<class T>
	inline T* JSObject::get(Worker* worker, uint32_t index) {
		return static_cast<T*>(get<JSValue>(worker, index));
	}
	template<class T>
	inline T* JSObject::get(Worker* worker, cString& key) {
		return static_cast<T*>(get<JSValue>(worker, key));
	}
	template<class T>
	inline bool JSObject::setProperty(Worker* worker, cString& key, T value) {
		return setProperty<JSValue*>(worker, key, worker->newValue(value));
	}
	template<class T>
	inline bool JSClass::setProperty(cString& name, T value) {
		return setProperty<JSValue*>(name, _worker->newValue(value));
	}
	template<class T>
	inline bool JSClass::setStaticProperty(cString& name, T value) {
		return setStaticProperty<JSValue*>(name, _worker->newValue(value));
	}

	///

	template<>
	Qk_EXPORT void Persistent<JSValue>::reset();
	template<>
	template<>
	Qk_EXPORT void Persistent<JSValue>::reset(Worker* worker, JSValue* other);
	template<>
	template<>
	Qk_EXPORT void Persistent<JSValue>::copy(const Persistent<JSValue>& that);
	template<>
	Qk_EXPORT JSValue* Persistent<JSValue>::operator*() const;
	template<>
	Qk_EXPORT JSValue* EscapableHandleScope::escape(JSValue* val);

	template<>
	Qk_EXPORT JSValue* JSObject::get(Worker* worker, JSValue* key);
	template<>
	Qk_EXPORT JSValue* JSObject::get(Worker* worker, uint32_t index);
	template<>
	Qk_EXPORT JSValue* JSObject::get(Worker* worker, cString& key);
	template<>
	Qk_EXPORT bool JSObject::setProperty(Worker* worker, cString& key, JSValue* value);
	template<>
	Qk_EXPORT bool JSClass::setProperty<JSValue*>(cString& name, JSValue* value);
	template<>
	Qk_EXPORT bool JSClass::setStaticProperty<JSValue*>(cString& name, JSValue* value);
} }
#endif
