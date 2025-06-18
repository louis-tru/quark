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
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, &exEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./jsc.h"
#include <uv.h>
#include <limits>

extern "C" {
	void JSGlobalContextSetUnhandledRejectionCallback(JSGlobalContextRef ctx, JSObjectRef func, JSValueRef* ex);// JSC_API_AVAILABLE(macos(10.15.4), ios(13.4));
}

namespace qk { namespace js {
	static uv_key_t th_key;

	#define _Fun(name) \
		JSStringRef name##_s = JsStringWithUTF8(#name).collapse();
	Js_Const_Strings(_Fun)
	#undef _Fun

#if Qk_ARCH_64BIT
		constexpr int64_t NumberTag = 0xfffe000000000000ll;
		constexpr size_t DoubleEncodeOffsetBitOld = 48;
		constexpr size_t DoubleEncodeOffsetBit = 49;
		static    int64_t DoubleEncodeOffset = 1ll << DoubleEncodeOffsetBit;  // 48 or 49?
		constexpr int32_t OtherTag       = 0x2;
		constexpr int32_t BoolTag        = 0x4;
		constexpr int32_t UndefinedTag   = 0x8;
		constexpr int32_t ValueFalse     = OtherTag | BoolTag | false;
		constexpr int32_t ValueTrue      = OtherTag | BoolTag | true;
		constexpr int32_t ValueUndefined = OtherTag | UndefinedTag;
		constexpr int32_t ValueNull      = OtherTag;
		constexpr int64_t NotCellMask    = NumberTag | OtherTag;
#else
		enum { Int32Tag =        0xffffffff };
		enum { BooleanTag =      0xfffffffe };
		enum { NullTag =         0xfffffffd };
		enum { UndefinedTag =    0xfffffffc };
		enum { CellTag =         0xfffffffb };
		enum { EmptyValueTag =   0xfffffffa };
		enum { DeletedValueTag = 0xfffffff9 };
		enum { LowestTag =  DeletedValueTag };
#endif

	enum JSType : uint8_t {
		CellType,
		StringTypeOld,
		StringTypeNew,
		APIValueWrapperTypeOld = 6,
		APIValueWrapperTypeNew,
		ObjectTypeOld = 22,
		ObjectTypeNew,
	};

	static uint8_t APIValueWrapperType_JSType = APIValueWrapperTypeNew;
	static uint8_t StringType_JSType = StringTypeNew;
	static uint8_t ObjectType_JSType = ObjectTypeNew;

	inline double asDouble(const JSValue* val);
	inline bool   isString(const JSValue* val);

	void jsFatal(JSContextRef ctx, JSValueRef ex_, cChar* msg) {
		if (ex_) { // err
			auto ex = (JSObjectRef)ex_;
			if (!JSValueIsObject(ctx, ex)) {
				auto Error = JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), Error_s, nullptr);
				DCHECK(Error);
				ex = JSObjectCallAsConstructor(ctx, (JSObjectRef)Error, 1, &ex_, nullptr);
				DCHECK(ex);
			}
			JSValueRef line = JSObjectGetProperty(ctx, ex, line_s, nullptr);
			JSValueRef column = JSObjectGetProperty(ctx, ex, column_s, nullptr);
			JSValueRef message = JSObjectGetProperty(ctx, ex, message_s, nullptr);
			JSValueRef stack = JSObjectGetProperty(ctx, ex, stack_s, nullptr);
			double l = JSValueToNumber(ctx, line, 0);
			double c = JSValueToNumber(ctx, column, 0);
			auto m = jsToString(ctx, message);
			auto s = jsToString(ctx, stack);
			qk::Fatal("", l, "", "%s\n\n%s\n\n%s", msg?msg:"", m.c_str(), s.c_str());
		}
	}

	String jsToString(JSStringRef value) {
		// if (!value) return "<string conversion failed>";
		DCHECK(value);
		size_t bufferSize = JSStringGetMaximumUTF8CStringSize(value);
		char* str = (char*)malloc(bufferSize);
		auto size = JSStringGetUTF8CString(value, str, bufferSize);
		return Buffer(str, size, bufferSize).collapseString();
	}

	String jsToString(JSContextRef ctx, JSValueRef value) {
		auto str = JsValueToStringCopy(ctx, value, nullptr);
		return jsToString(*str);
	}

	void WorkerData::initialize(JSGlobalContextRef ctx) {
		JSValueRef ex = nullptr;
		auto global = JSContextGetGlobalObject(ctx);
		#define _Init_Fun(name,from) from##_##name = (JSObjectRef) \
		JSObjectGetProperty(ctx, from, *JSCStringPtr(*JsStringWithUTF8(#name)), JsFatal()); \
		JSValueProtect(ctx, from##_##name);
		Js_Worker_Data_Each(_Init_Fun)
		#undef _Init_Fun

		Undefined = JSValueMakeUndefined(ctx);   JSValueProtect(ctx, Undefined);
		Null = JSValueMakeNull(ctx);             JSValueProtect(ctx, Null);
		True = JSValueMakeBoolean(ctx, true);    JSValueProtect(ctx, True);
		False = JSValueMakeBoolean(ctx, false);  JSValueProtect(ctx, False);
		EmptyString = JSValueMakeString(ctx, *JsStringWithUTF8(""));
		JSValueProtect(ctx, EmptyString);
		TypedArray = (JSObjectRef)JSObjectGetPrototype(ctx, global_Uint8Array);
		JSValueProtect(ctx, TypedArray);
		DCHECK(JSValueIsObject(ctx, TypedArray));
		JSCStringPtr args[] = {JsStringWithUTF8("name"), JsStringWithUTF8("make")};
		JSCStringPtr body = JsStringWithUTF8(
			"return Object.defineProperties("
				"function(...args){return make(this,...args)},"
				"{"
					"name: {value:name},"
					"toString: {value:function(){return `class ${name} { [native code] }`}}"
				"}"
			");"
		);
		MakeConstructor = JSObjectMakeFunction(ctx, 0, 2,
			reinterpret_cast<JSStringRef*>(args), body.get(), 0, 0, JsFatal());
		JSValueProtect(ctx, MakeConstructor);
	}

	void WorkerData::destroy(JSGlobalContextRef ctx) {
		#define _Des_Fun(name,from) JSValueUnprotect(ctx, from##_##name);
		Js_Worker_Data_Each(_Des_Fun)
		#undef _Des_Fun
		JSValueUnprotect(ctx, Undefined);
		JSValueUnprotect(ctx, Null);
		JSValueUnprotect(ctx, True);
		JSValueUnprotect(ctx, False);
		JSValueUnprotect(ctx, EmptyString);
		JSValueUnprotect(ctx, TypedArray);
		JSValueUnprotect(ctx, MakeConstructor);
	}

	void JscClassReleasep(JscClass *&cls);
	JscClass* JscClassNew(JscWorker *w);

	JscWorker::JscWorker()
		: _ex(nullptr)
		, _try(nullptr)
		, _scope(nullptr)
		, _base(nullptr)
		, _callStack(0)
	{
		uv_key_set(&th_key, this);

		_group = JSContextGroupCreate();
		_ctx = JSGlobalContextCreateInGroup(_group, nullptr);
		_global.reset(this, Cast<JSObject>(JSContextGetGlobalObject(_ctx)));
		_data.initialize(_ctx);

		auto testNum = JSValueMakeNumber(_ctx, 0xffffffff);
		if (asDouble(Cast(testNum)) != 0xffffffff) {
			DoubleEncodeOffset = 1ll << DoubleEncodeOffsetBitOld;
			Qk_ASSERT_RAW(asDouble(Cast(testNum)) == 0xffffffff);
		}
		auto testStr = JSValueMakeString(_ctx, prototype_s);
		if (!isString(Cast(testStr))) {
			StringType_JSType = StringTypeOld;
			ObjectType_JSType = ObjectTypeOld;
			APIValueWrapperType_JSType = APIValueWrapperTypeOld;
			Qk_ASSERT_RAW(isString(Cast(testStr)));
		}

		_base = JscClassNew(this);

		HandleScope handle(this);
		_rejectionListener = Back<JSObjectRef>(newFunction("rejectionListener", [](auto args) {
			DCHECK(args.length() == 2);
			defalutPromiseRejectListener(args.worker(), args[1], args[0]);
		}));
		DCHECK(_rejectionListener);
		ENV(this);
		JSGlobalContextSetUnhandledRejectionCallback(_ctx, _rejectionListener, JsFatal());
		JSValueProtect(_ctx, _rejectionListener);
	}

	void JscWorker::release() {
		Worker::release();
		JscClassReleasep(_base);
		JSValueUnprotect(_ctx, _rejectionListener);
		_ex = nullptr;
		_data.destroy(_ctx);
		JSGlobalContextRelease(_ctx);
		JSContextGroupRelease(_group);
		Releasep(_classes);
		uv_key_set(&th_key, nullptr);
	}

	JSObjectRef JscWorker::newErrorJsc(cChar* message) {
		auto str = JSValueMakeString(_ctx, *JsStringWithUTF8(message));
		auto error = JSObjectCallAsConstructor(_ctx, _data.global_Error, 1, &str, nullptr);
		DCHECK(error);
		return error;
	}

	// -----------------------------------------------------------------------------------------

	Worker* Worker::current() {
		return first_worker ? first_worker : reinterpret_cast<Worker*>(uv_key_get(&th_key));
	}

	Worker* Worker::Make() {
		auto o = new JscWorker();
		o->init();
		return o;
	}

	struct JSCell;

	struct JscValueImpl {
		union EncodedValueDescriptor {
			int64_t asInt64;
#if Qk_ARCH_64BIT
			JSCell* ptr;
#elif
			double asDouble;
#endif
			struct {
#if Qk_CPU_LENDIAN
				int32_t payload;
				int32_t tag;
#else
				int32_t tag;
				int32_t payload;
#endif
			} asBits;
		};
		EncodedValueDescriptor u;
	};

	struct JSCell {
		uint32_t m_structureID;
		uint8_t m_indexingTypeAndMisc;
		JSType m_type;
		uint8_t m_flags;
		uint8_t m_cellState;
	};
	struct JSAPIValueWrapper: JSCell {
		JscValueImpl m_value;
	};

	inline double reinterpretInt64ToDouble(int64_t value) {
		return bitwise_cast<double>(value);
	}

	static bool isUint32Range(double num) {
		constexpr const uint32_t max = std::numeric_limits<uint32_t>::max();
		return num >= 0 && num <= max;
	}

	static bool isInt32Range(double num) {
		constexpr const int32_t min = std::numeric_limits<int32_t>::min();
		constexpr const int32_t max = std::numeric_limits<int32_t>::max();
		return num >= min && num <= max;
	}

	/// ----------------------

	inline static JscValueImpl toJscValueImpl(const JSValue* val) {
		DCHECK(val);
#if Qk_ARCH_64BIT
		JscValueImpl result = bitwise_cast<JscValueImpl>(val);
#else
		JSCell* jsCell = bitwise_cast<JSCell*>(val);
		JscValueImpl result;
		if (jsCell->m_type == APIValueWrapperType_JSType)
			result = static_cast<JSAPIValueWrapper*>(jsCell)->m_value;
		else {
			result.u.asBits.tag = CellTag;
			result.u.asBits.payload = reinterpret_cast<int32_t>(jsCell);
		}
#endif
		//if (result.isCell())
		//		RELEASE_ASSERT(result.asCell()->methodTable(getVM(globalObject)));
		return result;
	}

	inline bool isCell(const JSValue* val) {
#if Qk_ARCH_64BIT
		return !(bitwise_cast<JscValueImpl>(val).u.asInt64 & NotCellMask);
#else
		return toJscValueImpl(val).u.asBits.tag == CellTag;
#endif
	}

	inline JSCell* asCell(const JSValue* val) {
		DCHECK(isCell(val));
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl>(val).u.ptr;
#else
		return reinterpret_cast<JSCell*>(toJscValueImpl(val).u.asBits.payload);
#endif
	}

	inline bool isInt32(const JSValue* val) {
#if Qk_ARCH_64BIT
		return (bitwise_cast<JscValueImpl>(val).u.asInt64 & NumberTag) == NumberTag;
#else
		return toJscValueImpl(val).u.asBits.tag == Int32Tag;
#endif
	}

	inline bool isDouble(const JSValue* val) {
#if Qk_ARCH_64BIT
		auto mask = bitwise_cast<JscValueImpl>(val).u.asInt64 & NumberTag;
		return mask && mask != NumberTag;
#else
		return toJscValueImpl(val).u.asBits.tag < LowestTag;
#endif
	}

	inline bool isNumber(const JSValue* val) {
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl>(val).u.asInt64 & NumberTag;
#else
		return js::isInt32(val) || isDouble(val);
#endif
	}

	inline bool isBoolean(const JSValue* val) {
#if Qk_ARCH_64BIT
		return (bitwise_cast<JscValueImpl>(val).u.asInt64 & ~1) == ValueFalse;
#else
		return toJscValueImpl(val).u.asBits.tag == BooleanTag;
#endif
	}

	inline bool isUndefined(const JSValue* val) {
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl>(val).u.asInt64 == ValueUndefined;
#else
		return toJscValueImpl(val).u.asBits.tag == UndefinedTag;
#endif
	}

	inline bool isNull(const JSValue* val) {
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl>(val).u.asInt64 == ValueNull;
#else
		return toJscValueImpl(val).u.asBits.tag == NullTag;
#endif
	}

	inline int32_t asInt32(const JSValue* val) {
		DCHECK(isInt32(val));
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl>(val).u.asInt64;
#else
		return toJscValueImpl(val).u.asBits.payload;
#endif
	}

	inline double asDouble(const JSValue* val) {
		DCHECK(isDouble(val));
#if Qk_ARCH_64BIT
		return reinterpretInt64ToDouble(bitwise_cast<JscValueImpl>(val).u.asInt64 - DoubleEncodeOffset);
#else
		return toJscValueImpl(val).u.asDouble;
#endif
	}

	inline bool asBoolean(const JSValue* val) {
		DCHECK(isBoolean(val));
#if Qk_ARCH_64BIT
		return  bitwise_cast<JscValueImpl>(val).u.asInt64 == ValueTrue;
#else
		return toJscValueImpl(val).u.asBits.payload;
#endif
	}

	inline bool isString(const JSValue* val) {
		return js::isCell(val) && js::asCell(val)->m_type == StringType_JSType;
	}

	inline bool isObject(const JSValue* val) {
		return js::isCell(val) && js::asCell(val)->m_type >= ObjectType_JSType;
	}

	/// ----------------------

	bool JSValue::isUndefined() const {
		return js::isUndefined(this); //JSValueIsUndefined
	}

	bool JSValue::isNull() const {
		return js::isNull(this); // JSValueIsNull
	}

	bool JSValue::isBoolean() const {
		return js::isBoolean(this);
	}

	bool JSValue::isString() const {
		// return JSValueIsString(JSC_CTX(), Back(this));
		return js::isString(this);
	}

	bool JSValue::isObject() const {
		//return JSValueIsObject(JSC_CTX(), Back(this));
		return js::isObject(this);
	}

	bool JSValue::isArray() const {
		return JSValueIsArray(JSC_CTX(), Back(this));
	}

	bool JSValue::isDate() const {
		return JSValueIsDate(JSC_CTX(), Back(this));
	}

	bool JSValue::isNumber() const {
		return js::isNumber(this);
	}

	bool JSValue::isInt32() const {
		return js::isInt32(this);
	}

	bool JSValue::isUint32() const {
		if (js::isNumber(this)) {
			if (js::isInt32(this)) {
				return js::asInt32(this) >= 0;
			} else {
				return isUint32Range(js::asDouble(this));
			}
		}
		return false;
	}

	bool JSValue::isFunction() const {
		ENV();
		//auto ok = JSObjectIsFunction(ctx, Back<JSObjectRef>(this));
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.global_Function, JsFatal("JSValue::isFunction"));
		return ok;
	}

	bool JSValue::isArrayBuffer() const {
		ENV();
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.global_ArrayBuffer, JsFatal("JSValue::isArrayBuffer"));
		return ok;
	}

	bool JSValue::isTypedArray() const {
		ENV();
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(this),
			(JSObjectRef)worker->_data.TypedArray, JsFatal("JSValue::isTypedArray")
		);
		return ok;
	}

	bool JSValue::isUint8Array() const {
		ENV();
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.global_Uint8Array, JsFatal("JSValue::isUint8Array"));
		return ok;
	}

	bool JSValue::isBuffer() const {
		ENV();
		auto ok = JSValueIsInstanceOfConstructor(
			ctx, Back(this), worker->_data.global_ArrayBuffer, JsFatal("JSValue::isBuffer")
		);
		if (ok)
			return true;
		ok = JSValueIsInstanceOfConstructor(
			ctx, Back(this), worker->_data.TypedArray, JsFatal("JSValue::isBuffer 1")
		);
		return ok;
	}

	bool JSValue::equals(Worker *w, JSValue* val) const {
		ENV(w);
		auto ok = JSValueIsEqual(ctx, Back(this), Back(val), OK(false));
		return ok;
	}

	bool JSValue::strictEquals(Worker *w, JSValue* val) const {
		ENV(w);
		return JSValueIsStrictEqual(ctx, Back(this), Back(val));
	}

	bool JSValue::instanceOf(Worker* w, JSObject* constructor) const {
		ENV(w);
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(this), Back<JSObjectRef>(constructor), JsFatal("JSValue::instanceOf"));
		return ok;
	}

	bool JSValue::toBoolean(Worker* w) const {
		if (js::isBoolean(this))
			return asBoolean(this);
		ENV(w);
		auto ret = JSValueToBoolean(ctx, Back(this)); // Force convert
		return ret;
	}

	JSString* JSValue::toString(Worker* w) const {
		ENV(w);
		if (JSValueIsString(ctx, Back(this))) {
			return static_cast<JSString*>(const_cast<JSValue*>(this));
		}
		auto str = JsValueToStringCopy(ctx, Back(this), OK(nullptr));
		auto val = JSValueMakeString(ctx, *str);
		return worker->addToScope<JSString>(val);
	}

	JSNumber* JSValue::toNumber(Worker* w) const {
		if (js::isNumber(this))
			return bitwise_cast<JSNumber*>(this);
		ENV(w);
		auto num = JSValueToNumber(ctx, Back(this), OK(nullptr)); // Force convert
		auto ret = JSValueMakeNumber(ctx, num);
		return worker->addToScope<JSNumber>(ret);
	}

	JSInt32* JSValue::toInt32(Worker* w) const {
		if (js::isInt32(this))
			return bitwise_cast<JSInt32*>(this);
		ENV(w);
		double num;
		if (js::isDouble(this)) {
			num = js::asDouble(this);
		} else {
			num = JSValueToNumber(ctx, Back(this), OK(nullptr)); // Force convert
		}
		if (!isInt32Range(num)) {
			return THROW_ERR("Invalid conversion toInt32, Range overflow"), nullptr;
		}
		auto ret = JSValueMakeNumber(ctx, int(num));
		return worker->addToScope<JSInt32>(ret);
	}

	JSUint32* JSValue::toUint32(Worker* w) const {
		ENV(w);
		if (js::isInt32(this)) {
			if (js::asInt32(this) >= 0) {
				return bitwise_cast<JSUint32*>(this);
			} else {
				return THROW_ERR("Invalid conversion toUint32, Can't be a negative number"), nullptr;
			}
		}
		double num;
		if (js::isDouble(this)) {
			num = js::asDouble(this);
		} else {
			num = JSValueToNumber(ctx, Back(this), OK(nullptr)); // Force convert
		}
		if (!isUint32Range(num)) {
			return THROW_ERR("Invalid conversion toUint32, Range overflow"), nullptr;
		}
		auto ret = JSValueMakeNumber(ctx, int(num));
		return worker->addToScope<JSUint32>(ret);
	}

	Maybe<String> JSValue::asString(Worker *w) const {
		ENV(w);
		if (JSValueIsString(ctx, Back(this))) {
			return static_cast<const JSString*>(this)->value(w);
		}
		return Maybe<String>();
	}

	Maybe<double> JSValue::asNumber(Worker* w) const {
		if (js::isNumber(this)) {
			if (js::isInt32(this))
				return js::asInt32(this);
			else
				return js::asDouble(this);
		} 
		return Maybe<double>();
	}

	Maybe<float> JSValue::asFloat32(Worker* w) const {
		if (js::isNumber(this)) {
			if (js::isInt32(this))
				return Maybe<float>(js::asInt32(this));
			else
				return Maybe<float>(js::asDouble(this));
		} 
		return Maybe<float>();
	}

	Maybe<int32_t> JSValue::asInt32(Worker* w) const {
		if (js::isInt32(this))
			return Maybe<int>(js::asInt32(this));
		if (js::isDouble(this)) {
			auto num = js::asDouble(this);
			if (isInt32Range(num)) {
				return Maybe<int>(num);
			}
		}
		return Maybe<int>();
	}

	Maybe<uint32_t> JSValue::asUint32(Worker* w) const {
		if (js::isInt32(this)) {
			int32_t num = js::asInt32(this);
			if (num >= 0) {
				return Maybe<uint32_t>(num);
			} else {
				return Maybe<uint32_t>();
			}
		}
		if (js::isDouble(this)) {
			auto num = js::asDouble(this);
			if (isUint32Range(num)) {
				return Maybe<uint32_t>(num);
			}
		}
		return Maybe<uint32_t>();
	}

	template<>
	JSValue* JSObject::get(Worker* w, JSValue* key) {
		DCHECK(isObject());
		ENV(w);
		auto ret = JSObjectGetPropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), OK(nullptr));
		// worker->addToScope(ret);
		return Cast(ret);
	}

	template<>
	JSValue* JSObject::get(Worker* w, uint32_t index) {
		DCHECK(isObject());
		ENV(w);
		auto ret = JSObjectGetPropertyAtIndex(ctx, Back<JSObjectRef>(this), index, OK(nullptr));
		// worker->addToScope(ret);
		return Cast(ret);
	}

	bool JSObject::set(Worker* w, JSValue* key, JSValue* val) {
		DCHECK(isObject());
		ENV(w);
		JSObjectSetPropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), Back(val), 0, OK(false));
		return true;
	}

	bool JSObject::set(Worker* w, uint32_t index, JSValue* val) {
		DCHECK(isObject());
		ENV(w);
		JSObjectSetPropertyAtIndex(ctx, Back<JSObjectRef>(this), index, Back(val), OK(false));
		return true;
	}

	bool JSObject::has(Worker* w, JSValue* key) {
		DCHECK(isObject());
		ENV(w);
		return JSObjectHasPropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), OK(false));
	}

	bool JSObject::has(Worker* w, uint32_t index) {
		DCHECK(isObject());
		ENV(w);
		auto ok = JSObjectHasPropertyForKey(ctx,
			Back<JSObjectRef>(this), JSValueMakeNumber(ctx, index), OK(false)
		);
		return ok;
	}

	bool JSObject::JSObject::deleteFor(Worker* w, JSValue* key) {
		DCHECK(isObject());
		ENV(w);
		auto ok = JSObjectDeletePropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), OK(false));
		return ok;
	}

	bool JSObject::deleteFor(Worker* w, uint32_t index) {
		DCHECK(isObject());
		ENV(w);
		auto ok = JSObjectDeletePropertyForKey(ctx,
			Back<JSObjectRef>(this), JSValueMakeNumber(ctx, index), OK(false)
		);
		return ok;
	}

	JSArray* JSObject::getPropertyNames(Worker* w) {
		DCHECK(isObject());
		ENV(w);
		JSCPropertyNameArrayPtr names = JSObjectCopyPropertyNames(ctx, Back<JSObjectRef>(this));
		auto ret = JSObjectMakeArray(ctx, 0, nullptr, OK(nullptr));
		auto count = JSPropertyNameArrayGetCount(*names);

		for (int i = 0; i < count; i++) {
			auto key = JSValueMakeString(ctx, JSPropertyNameArrayGetNameAtIndex(*names, i));
			JSObjectSetPropertyAtIndex(ctx, ret, i, key, OK(nullptr));
		}
		return worker->addToScope<JSArray>(ret);
	}

	Maybe<Array<String>> JSObject::getPropertyKeys(Worker* w) {
		DCHECK(isObject());
		ENV(w);
		JSCPropertyNameArrayPtr names = JSObjectCopyPropertyNames(ctx, Back<JSObjectRef>(this));
		auto count = JSPropertyNameArrayGetCount(*names);
		Array<String> ret;
		for (int i = 0; i < count; i++) {
			JSCStringPtr s = JSPropertyNameArrayGetNameAtIndex(*names, i);
			size_t len = JSStringGetLength(*s);
			const JSChar* ch = JSStringGetCharactersPtr(*s);
			auto buf = codec_utf16_to_utf8(ArrayWeak<uint16_t>(ch, len).buffer());
			ret.push(buf.collapseString());
		}
		Qk_ReturnLocal(ret);
	}

	JSFunction* JSObject::getConstructor(Worker* w) {
		DCHECK(isObject());
		ENV(w);
		auto ret = JSObjectGetProperty(ctx, Back<JSObjectRef>(this), constructor_s, OK(nullptr));
		auto jsret = Cast<JSFunction>(ret);
		// worker->addToScope(ret);
		DCHECK(jsret->isFunction());
		return jsret;
	}

	bool JSObject::defineOwnProperty(Worker *w, JSValue *key, JSValue *value, int flags) {
		ENV(w);
		JSPropertyAttributes attrs = flags << 1;
		JSObjectSetPropertyForKey(ctx,
			Back<JSObjectRef>(this), Back(key), Back(value), attrs, OK(false)
		);
		return true;
	}

	bool JSObject::setPrototype(Worker* w, JSObject* __proto__) {
		DCHECK(__proto__, "Bad argument");
		ENV(w);
		JSObjectSetPrototype(ctx, Back<JSObjectRef>(this), Back(__proto__));
		return true;
	}

	int JSString::length() const {
		DCHECK(isString());
		ENV();
		auto len = JSObjectGetProperty(ctx, Back<JSObjectRef>(this), length_s, &ex);
		CHECK(ex);
		return js::asInt32(Cast(len));
	}

	String JSString::value(Worker* w) const {
		DCHECK(isString());
		ENV(w);
		auto s = JsValueToStringCopy(ctx, Back(this), JsFatal("JSString::value()"));
		size_t len = JSStringGetLength(*s);
		const JSChar* ch = JSStringGetCharactersPtr(*s);
		auto buf = codec_utf16_to_utf8(ArrayWeak<uint16_t>(ch, len).buffer());
		return buf.collapseString();
	}

	String2 JSString::value2(Worker* w) const {
		DCHECK(isString());
		ENV(w);
		auto s = JsValueToStringCopy(ctx, Back(this), JsFatal("JSString::value2()"));
		size_t len = JSStringGetLength(*s);
		const JSChar* ch = JSStringGetCharactersPtr(*s);
		return String2(ch, len);
	}

	String4 JSString::value4(Worker* w) const {
		DCHECK(isString());
		ENV(w);
		auto s = JsValueToStringCopy(ctx, Back(this), JsFatal("JSString::value4()"));
		size_t len = JSStringGetLength(*s);
		const JSChar* ch = JSStringGetCharactersPtr(*s);
		auto str4 = codec_utf16_to_unicode(ArrayWeak<uint16_t>(ch, len).buffer());
		return str4.collapseString();
	}

	int JSArray::length() const {
		ENV();
		auto val = JSObjectGetProperty(ctx, Back<JSObjectRef>(this), length_s, JsFatal("JSArray::length()"));
		auto len = js::asInt32(Cast(val));
		return len;
	}

	double JSDate::valueOf() const {
		ENV();
		auto val = JSObjectCallAsFunction(ctx,
			worker->data().global_Date_prototype_valueOf,
			Back<JSObjectRef>(this), 0, nullptr, JsFatal("JSDate::valueOf()")
		);
		auto num = js::asDouble(Cast(val));
		return num;
	}

	double JSNumber::value() const {
		if (js::isInt32(this))
			return js::asInt32(this);
		else
			return js::asDouble(this);
	}

	float JSNumber::float32() const {
		return js::isInt32(this) ? js::asInt32(this): js::asDouble(this);
	}

	int JSInt32::value() const {
		return js::asInt32(this);
	}

	uint32_t JSUint32::value() const {
		if (js::isInt32(this))
			return js::asInt32(this);
		else
			return js::asDouble(this);
	}

	bool JSBoolean::value() const {
		return js::asBoolean(this);
	}

	JSValue* JSFunction::call(Worker* w, int argc, JSValue* argv[], JSValue* recv) {
		DCHECK(isFunction());
		ENV(w);
		auto rev = JSObjectCallAsFunction(ctx, Back<JSObjectRef>(this),
			Back<JSObjectRef>(recv), argc, reinterpret_cast<const JSValueRef*>(argv), OK(nullptr)
		);
		DCHECK(rev);
		return worker->addToScope(rev);
	}

	JSValue* JSFunction::call(Worker* w, JSValue* recv) {
		return call(w, 0, nullptr, recv);
	}

	JSObject* JSFunction::newInstance(Worker* w, int argc, JSValue* argv[]) {
		DCHECK(isFunction());
		ENV(w);
		auto rev = JSObjectCallAsConstructor(ctx, Back<JSObjectRef>(this),
			argc, reinterpret_cast<const JSValueRef*>(argv), OK(nullptr)
		);
		DCHECK(rev);
		return worker->addToScope<JSObject>(rev);
	}

	JSObject* JSFunction::getFunctionPrototype(Worker* w) {
		DCHECK(isFunction());
		ENV(w);
		auto rev = JSObjectGetProperty(ctx, Back<JSObjectRef>(this), prototype_s, OK(nullptr));
		DCHECK(rev);
		return worker->addToScope<JSObject>(rev);
	}

	uint32_t JSArrayBuffer::byteLength(Worker* w) const {
		DCHECK(isArrayBuffer());
		ENV(w);
		auto len = JSObjectGetArrayBufferByteLength(ctx, Back<JSObjectRef>(this), JsFatal("JSArrayBuffer::byteLength()"));
		return len;
	}

	Char* JSArrayBuffer::data(Worker* w) {
		DCHECK(isArrayBuffer());
		ENV(w);
		auto ptr = JSObjectGetArrayBufferBytesPtr(ctx, Back<JSObjectRef>(this), JsFatal("JSArrayBuffer::data()"));
		return static_cast<Char*>(ptr);
	}

	JSArrayBuffer* JSTypedArray::buffer(Worker* w) {
		DCHECK(isTypedArray());
		ENV(w);
		auto buff = JSObjectGetTypedArrayBuffer(ctx, Back<JSObjectRef>(this), JsFatal("JSArrayBuffer::buffer()"));
		DCHECK(buff);
		return Cast<JSArrayBuffer>(buff);
	}

	uint32_t JSTypedArray::byteLength(Worker* w) const {
		DCHECK(isTypedArray());
		ENV(w);
		auto len = JSObjectGetTypedArrayByteLength(ctx, Back<JSObjectRef>(this), JsFatal("JSTypedArray::byteLength()"));
		return len;
	}

	uint32_t JSTypedArray::byteOffset(Worker* w) const {
		DCHECK(isTypedArray());
		ENV(w);
		auto off = JSObjectGetTypedArrayByteOffset(ctx, Back<JSObjectRef>(this), JsFatal("JSTypedArray::byteOffset()"));
		return off;
	}

	inline bool isSet(Worker* w, JSValue* val) {
		ENV(w);
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(val), worker->data().global_Set, JsFatal("isSet"));
		return ok;
	}

	bool JSSet::add(Worker* w, JSValue* key) {
		ENV(w);
		DCHECK(isSet(w, this));
		auto argv = Back(key);
		JSObjectCallAsFunction(ctx,
			worker->data().global_Set_prototype_add, Back<JSObjectRef>(this), 1, &argv, OK(false)
		);
		return true;
	}

	bool JSSet::has(Worker* w, JSValue* key) {
		ENV(w);
		DCHECK(isSet(w, this));
		auto argv = Back(key);
		auto rev = JSObjectCallAsFunction(ctx,
			worker->data().global_Set_prototype_has, Back<JSObjectRef>(this), 1, &argv, OK(false)
		);
		return js::asBoolean(Cast(rev));
	}

	bool JSSet::deleteFor(Worker* w, JSValue* key) {
		ENV(w);
		DCHECK(isSet(w, this));
		auto argv = Back(key);
		auto rev = JSObjectCallAsFunction(ctx,
			worker->data().global_Set_prototype_delete, Back<JSObjectRef>(this), 1, &argv, OK(false)
		);
		return true;
	}

	template <> void Persistent<JSValue>::reset() {
		if (_val) {
			JSValueUnprotect(JSC_CTX(_worker), Back(_val));
			_val = nullptr;
		}
	}

	template <> template <>
	void Persistent<JSValue>::reset(Worker* w, JSValue* other) {
		reset();
		if (other) {
			DCHECK(w);
			JSValueProtect(JSC_CTX(w), Back(other));
			_val = other;
			_worker = w;
		}
	}

	template<> template<>
	void Persistent<JSValue>::copy(const Persistent<JSValue>& that) {
		reset(that._worker, that._val);
	}

	template<>
	JSValue* Persistent<JSValue>::operator*() const {
		// if (_val)
		// 	WORKER(_worker)->addToScope(_val);
		return _val;
	}

	JSNumber* Worker::newValue(float val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSNumber>(ret);
	}

	JSNumber* Worker::newValue(double val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSNumber>(ret);
	}

	JSBoolean* Worker::newBool(bool val) {
		ENV(this);
		return Cast<JSBoolean>(val ? worker->_data.True: worker->_data.False);
	}

	JSInt32* Worker::newValue(int32_t val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSInt32>(ret);
	}

	JSUint32* Worker::newValue(uint32_t val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSUint32>(ret);
	}

	JSNumber* Worker::newValue(int64_t val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSNumber>(ret);
	}

	JSNumber* Worker::newValue(uint64_t val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSNumber>(ret);
	}

	JSString* Worker::newString(cBuffer& val) {
		auto obj = JSValueMakeString(JSC_CTX(this), *JsStringWithUTF8(*val));
		DCHECK(obj);
		return WORKER(this)->addToScope<JSString>(obj);
	}

	JSString* Worker::newValue(cString& val) {
		auto obj = JSValueMakeString(JSC_CTX(this), *JsStringWithUTF8(*val));
		DCHECK(obj);
		return WORKER(this)->addToScope<JSString>(obj);
	}

	JSString* Worker::newValue(cString2& val) {
		auto str = JSStringCreateWithCharacters(*val, val.length());
		auto obj = JSValueMakeString(JSC_CTX(this), str);
		DCHECK(obj);
		return WORKER(this)->addToScope<JSString>(obj);
	}

	JSUint8Array* Worker::newValue(Buffer&& buff) {
		ENV(this);
		JSObjectRef arr;
		if (buff.length()) {
			arr = JSObjectMakeTypedArrayWithBytesNoCopy(
				ctx, kJSTypedArrayTypeUint8Array, *buff, buff.length(), [](void* bytes, void* data) {
				Allocator::free(bytes);
			}, nullptr, OK(nullptr));
		} else {
			arr = JSObjectMakeTypedArray(ctx, kJSTypedArrayTypeUint8Array, 0, OK(nullptr));
		}
		DCHECK(arr);
		buff.collapse();
		return worker->addToScope<JSUint8Array>(arr);
	}

	JSObject* Worker::newObject() {
		ENV(this);
		auto obj = JSObjectMake(ctx, nullptr, nullptr);
		DCHECK(obj);
		return worker->addToScope<JSObject>(obj);
	}

	JSArray* Worker::newArray() {
		ENV(this);
		auto obj = JSObjectMakeArray(ctx, 0, nullptr, OK(nullptr));
		DCHECK(obj);
		return worker->addToScope<JSArray>(obj);
	}

	JSValue* Worker::newNull() {
		ENV(this);
		return Cast(worker->_data.Null);
	}

	JSValue* Worker::newUndefined() {
		ENV(this);
		return Cast(worker->_data.Undefined);
	}

	JSSet* Worker::newSet() {
		ENV(this);
		auto obj = JSObjectCallAsConstructor(ctx, worker->_data.global_Set, 0, nullptr, OK(nullptr));
		DCHECK(obj);
		return worker->addToScope<JSSet>(obj);
	}

	JSString* Worker::newEmpty() {
		return Cast<JSString>(WORKER(this)->_data.EmptyString);
	}

	JSString* Worker::newStringOneByte(cString& data) {
		return newValue(data);
	}

	JSArrayBuffer* Worker::newArrayBuffer(char* use_buff, uint32_t len) {
		ENV(this);
		auto obj = JSObjectMakeArrayBufferWithBytesNoCopy(ctx, use_buff, len,
			[](void* bytes, void* data) { ::free(bytes); }, nullptr, OK(nullptr)
		);
		DCHECK(obj);
		return worker->addToScope<JSArrayBuffer>(obj);
	}

	JSArrayBuffer* Worker::newArrayBuffer(uint32_t len) {
		ENV(this);
		Buffer buff(len);
		auto obj = JSObjectMakeArrayBufferWithBytesNoCopy(ctx, *buff, len, [](void* bytes, void* data){
			Allocator::free(bytes);
		}, nullptr, OK(nullptr));
		DCHECK(obj);
		buff.collapse();
		return worker->addToScope<JSArrayBuffer>(obj);
	}

	JSUint8Array* Worker::newUint8Array(JSArrayBuffer* abuffer, uint32_t offset, uint32_t size) {
		DCHECK(abuffer->isArrayBuffer());
		ENV(this);
		auto obj = JSObjectMakeTypedArrayWithArrayBufferAndOffset(
			ctx, kJSTypedArrayTypeUint8Array, Back<JSObjectRef>(abuffer), offset, size, OK(nullptr)
		);
		DCHECK(obj);
		return worker->addToScope<JSUint8Array>(obj);
	}

	JSObject* Worker::newRangeError(cChar* errmsg, ...) {
		ENV(this);
		Js_Format_Str(errmsg);
		auto val = Back(newValue(str));
		auto obj = JSObjectCallAsConstructor(ctx, worker->_data.global_RangeError, 1, &val, OK(nullptr));
		DCHECK(obj);
		return worker->addToScope<JSObject>(obj);
	}

	JSObject* Worker::newReferenceError(cChar* errmsg, ...) {
		ENV(this);
		Js_Format_Str(errmsg);
		auto val = Back(newValue(str));
		auto obj = JSObjectCallAsConstructor(ctx, worker->_data.global_ReferenceError, 1, &val, OK(nullptr));
		DCHECK(obj);
		return worker->addToScope<JSObject>(obj);
	}

	JSObject* Worker::newSyntaxError(cChar* errmsg, ...) {
		ENV(this);
		Js_Format_Str(errmsg);
		auto val = Back(newValue(str));
		auto obj = JSObjectCallAsConstructor(ctx, worker->_data.global_SyntaxError, 1, &val, OK(nullptr));
		DCHECK(obj);
		return worker->addToScope<JSObject>(obj);
	}

	JSObject* Worker::newTypeError(cChar* errmsg, ...) {
		ENV(this);
		Js_Format_Str(errmsg);
		auto val = Back(newValue(str));
		auto obj = JSObjectCallAsConstructor(ctx, worker->_data.global_TypeError, 1, &val, OK(nullptr));
		DCHECK(obj);
		return worker->addToScope<JSObject>(obj);
	}

	JSObject* Worker::newValue(cError& err) {
		ENV(this);
		auto str = Back(newValue(err.message()));
		auto obj = JSObjectCallAsConstructor(ctx, worker->_data.global_Error, 1, &str, OK(nullptr));
		DCHECK(obj);
		JSObjectSetProperty(ctx, obj, errno_s, JSValueMakeNumber(ctx, err.code()), 0, OK(nullptr));
		return worker->addToScope<JSObject>(obj);
	}

	void Worker::throwError(JSValue* exception) {
		WORKER(this)->throwException(Back(exception));
	}

	JSValue* JscWorker::runScript(JSString* jsSource, cString& source, cString& name, JSObject* sandbox) {
		ENV(this);
		auto url = JsStringWithUTF8(*String::format("[%s]", *name));
		DCHECK(url);

		if (sandbox) {
			DCHECK(sandbox->isObject());
			String sandboxExpand, sandboxRet;
			for (auto &k: sandbox->getPropertyKeys(this).from(Array<String>())) {
				sandboxExpand += String::format("var %s=__sandbox.%s;", *k, *k);
				sandboxRet += k + ',';
			}
			auto body = String::format("(function(__sandbox){%s; (function(){%s})(); return {%s}})",
				*sandboxExpand, source.isEmpty() ? *jsSource->value(this): *source, *sandboxRet
			);
			auto script = JsStringWithUTF8(*body);
			auto func = JSEvaluateScript(ctx, *script, nullptr, *url, 1, OK(nullptr));
			auto argv = sandbox->cast();
			return Cast<JSFunction>(func)->call(this, 1, &argv);
		} else {
			JSCStringPtr script;
			if (jsSource) {
				script = JsValueToStringCopy(ctx, Back(jsSource), OK(nullptr));
			} else {
				DCHECK(!source.isEmpty());
				script = JsStringWithUTF8(*source);
			}
			DCHECK(script);
			auto val = JSEvaluateScript(ctx, *script, nullptr, *url, 1, OK(nullptr));
			return worker->addToScope(val);
		}
	}

	JSValue* Worker::runScript(cString& source, cString& name, JSObject* sandbox) {
		return WORKER(this)->runScript(nullptr, source, name, sandbox);
	}

	JSValue* Worker::runScript(JSString* source, JSString* name, JSObject* sandbox) {
		return WORKER(this)->runScript(source, String(), name->value(this), sandbox);
	}

	void Worker::garbageCollection() {
		JSGarbageCollect(JSC_CTX(this));
	}

	void runDebugger(Worker* w, const DebugOptions &opts) {
		// noop
	}

	void stopDebugger(Worker* w) {
		// noop
	}

	void debuggerBreakNextStatement(Worker* w) {
		// noop
	}

	void setFlagsFromCommandLine(const Arguments* args) {
		if (args->options.has("help")) {
			Qk_Log("Usage: quark [options] [ script.js ] [arguments]");
			Qk_Log("       quark --eval|-e [ script ] [arguments]");
		}
	}

	int startPlatform(int (*exec)(Worker*)) {
		Sp<Worker> worker = Worker::Make();
		return exec(*worker); // exec main script
	}

	Qk_Init_Func(jsc_th_key_init) {
		uv_key_create(&th_key);
		initFactories();
	};
} }
