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

namespace qk { namespace js {
	static uv_key_t th_key;

	#define _Fun(name) \
		JSStringRef name##_s = JsStringWithUTF8(name);
	Js_Const_Strings(_Fun)
	#undef _Fun

	void jsFatal(JSContextRef ctx, JSValueRef ex_, cCher* msg = "") {
		if (ex_) { // err
			auto ex = (JSObjectRef)ex_;
			if (!JSValueIsObject(ctx, &ex)) {
				auto Error = JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), Error_s, nullptr);
				DCHECK(Error);
				ex = JSObjectCallAsConstructor(ctx, Error, 1, &ex, nullptr);
				DCHECK(ex);
			}
			auto ex = (JSObjectRef)ex_;
			JSValueRef line = JSObjectGetProperty(ctx, &ex, line_s, 0);
			JSValueRef column = JSObjectGetProperty(ctx, &ex, column_s, 0);
			JSValueRef message = JSObjectGetProperty(ctx, &ex, message_s, 0);
			JSValueRef stack = JSObjectGetProperty(ctx, &ex, stack_s, 0);
			double l = JSValueToNumber(ctx, line, 0);
			double c = JSValueToNumber(ctx, column, 0);
			auto m = jsToString(ctx, message);
			auto s = jsToString(ctx, stack);
			qk::Fatal("", l, "", "%s\n\n%s\n\n%s", msg?msg:"", m.c_str(), s.c_str());
		}
	}

	String jsToString(JSStringRef value) {
		DCHECK(value);
		size_t bufferSize = JSStringGetMaximumUTF8CStringSize(value);
		char* str = (char*)malloc(bufferSize);
		auto size = JSStringGetUTF8CString(value, str, bufferSize);
		return Buffer(str, size, bufferSize).collapseString();
	}

	void WorkerData::initialize(JSGlobalContextRef ctx) {
		JSValueRef ex = nullptr;
		auto global = JSContextGetGlobalObject(ctx);
		#define _Init_Fun(name,from) from##_##name = (JSObjectRef) \
		JSObjectGetProperty(ctx, from, *JSCStringPtr(JsStringWithUTF8(name)), JsFatal()); \
		JSValueProtect(ctx, from##_##name);
		Js_Worker_Data_Each(_Init_Fun)
		#undef _Init_Fun

		Undefined = JSValueMakeUndefined(ctx);   JSValueProtect(ctx, Undefined);
		Null = JSValueMakeNull(ctx);             JSValueProtect(ctx, Null);
		True = JSValueMakeBoolean(ctx, true);    JSValueProtect(ctx, true);
		False = JSValueMakeBoolean(ctx, false);  JSValueProtect(ctx, false);
		EmptyString = JSValueMakeString(ctx, JsStringWithUTF8("")); JSValueProtect(ctx, EmptyString);
		TypedArray = JSObjectGetPrototype(global_Uint8Array); JSValueProtect(ctx, TypedArray);
		DCHECK(JSValueIsObject(ctx, TypedArray));

		JSCStringPtr sourceName = JsStringWithUTF8("source");
		JSCStringPtr sandboxName = JsStringWithUTF8("sandbox");
		JSCStringPtr body = JsStringWithUTF8("return arguments.callee.__native__.apply(this,arguments)");
		JSStringRef argvNames[] = { *sourceName, *sandboxName };
		WrapSandboxScriptFunc = JSObjectMakeFunction(ctx, nullptr, 2, argvNames, body, nullptr, 1, JsFatal());
		JSValueProtect(ctx, WrapSandboxScriptFunc);
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
		JSValueUnprotect(ctx, WrapSandboxScript);
	}

	JscWorker::JscWorker()
		: _ex(nullptr)
		, _try(nullptr)
		, _scope(nullptr)
		, _base(nullptr)
		, _messageListener(nullptr)
		, _callStack(0)
		, _hasTerminated(false)
		, _hasDestroy(false)
	{
		uv_key_set(&th_key, this);

		_group = JSContextGroupCreate();
		_ctx = JSGlobalContextCreateInGroup(_group, nullptr);
		_global.reset(this, Cast<JSObject>(JSContextGetGlobalObject(_ctx)));
		_data.initialize(_ctx);

		initBase();
	}

	void JscWorker::release() {
		Worker::release();
		_hasTerminated = true;
		Releasep(_base);
		_ex = nullptr;
		_data.destroy(_ctx);
		_hasDestroy = true;
		JSGlobalContextRelease(_ctx);
		JSContextGroupRelease(_group);
		uv_key_set(&th_key, nullptr);
	}

	JSObjectRef JscWorker::newErrorJsc(cChar* message) {
		auto str = JSValueMakeString(_ctx, JsStringWithUTF8(message));
		auto error = JSObjectCallAsConstructor(_ctx, _ctxData.Error, 1, &str, nullptr);
		DCHECK(error);
		return error;
	}

	// -----------------------------------------------------------------------------------------

	Worker* w::current() {
		return first_worker ? first_worker : reinterpret_cast<Worker*>(uv_key_get(&th_key));
	}

	Worker* w::Make() {
		auto o = new JscWorker();
		o->init();
		return o;
	}

#if Qk_ARCH_64BIT
	// Because some types don't require a context object on x64 os
	JSGlobalContextRef test_type_jsc_ctx = reinterpret_cast<JSGlobalContextRef>(1);
#else
	#define test_type_jsc_ctx (JSC_CTX())
#endif

	struct JSCell;

	struct JscValueImpl {
		union EncodedValueDescriptor {
			int64_t asInt64;
#if Qk_ARCH_64BIT
			JSCell* ptr;
#elif
			double asDouble;
#endif

#if Qk_CPU_LENDIAN
			struct {
				int32_t payload;
				int32_t tag;
			} asBits;
#else
			struct {
				int32_t tag;
				int32_t payload;
			} asBits;
#endif
		};
		EncodedValueDescriptor u;
	};

	#if Qk_ARCH_64BIT
		constexpr int64_t NumberTag = 0xfffe000000000000ll;
		constexpr size_t DoubleEncodeOffsetBit = 49;
		constexpr int64_t DoubleEncodeOffset = 1ll << DoubleEncodeOffsetBit;
		constexpr int32_t OtherTag       = 0x2;
		constexpr int32_t BoolTag        = 0x4;
		constexpr int32_t ValueFalse     = OtherTag | BoolTag | false;
		constexpr int32_t ValueTrue      = OtherTag | BoolTag | true;
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

	inline double reinterpretInt64ToDouble(int64_t value) {
		return bitwise_cast<double>(value);
	}

	inline bool isInt32(JSValue* val) {
#if Qk_ARCH_64BIT
		return (bitwise_cast<JscValueImpl*>(val)->u.asInt64 & NumberTag) == NumberTag;
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asBits.tag == Int32Tag;
#endif
	}

	inline bool isDouble(JSValue* val) {
#if Qk_ARCH_64BIT
		auto mask = bitwise_cast<JscValueImpl*>(val)->u.asInt64 & NumberTag;
		return mask && mask != NumberTag;
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asBits.tag < LowestTag;
#endif
	}

	inline bool isNumber(JSValue* val) {
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl*>(val)->asInt64 & NumberTag;
#else
		return js::isInt32(val) || isDouble(val);
#endif
	}

	inline bool isBoolean(JSValue* val) {
#if Qk_ARCH_64BIT
		return (bitwise_cast<JscValueImpl*>(val)->u.asInt64 & ~1) == ValueFalse;
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asBits.tag == BooleanTag;
#endif
	}

	inline int32_t asInt32(JSValue* val) {
		DCHECK(isInt32(val));
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl*>(val)->u.asInt64;
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asBits.payload;
#endif
	}

	inline double asDouble(JSValue* val) {
		DCHECK(isDouble(val));
#if 1
		return reinterpretInt64ToDouble(bitwise_cast<JscValueImpl*>(val)->u.asInt64 - DoubleEncodeOffset);
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asDouble;
#endif
	}

	inline bool asBoolean(JSValue* val) {
		DCHECK(isBoolean(val));
#if Qk_ARCH_64BIT
		return  bitwise_cast<JscValueImpl*>(val)->u.asInt64 == ValueTrue;
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asBits.payload;
#endif
	}

	bool JSValue::isUndefined() const {
		return JSValueIsUndefined(test_type_jsc_ctx, Back(this));
	}

	bool JSValue::isNull() const {
		return JSValueIsNull(test_type_jsc_ctx, Back(this));
	}

	bool JSValue::isString() const {
		return JSValueIsString(test_type_jsc_ctx, Back(this));
	}

	bool JSValue::isBoolean() const {
		return js::isBoolean(this);
	}

	bool JSValue::isObject() const {
		return JSValueIsObject(test_type_jsc_ctx, Back(this));
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
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.TypedArray, JsFatal("JSValue::isTypedArray"));
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

	JSString* JSValue::toString(Worker* w) const {
		ENV(w);
		if (JSValueIsString(ctx, Back(this))) {
			return static_cast<JSString*>(this);
		}
		auto str = JSValueToStringCopy(ctx, Back(this), OK(nullptr));
		auto val = JSValueMakeString(ctx, str);
		return worker->addToScope<JSString>(val);
	}

	JSBoolean* JSValue::toBoolean(Worker* w) const {
		if (js::isBoolean(this))
			return bitwise_cast<JSBoolean*>(this);
		ENV(w);
		auto ret = JSValueMakeBoolean(ctx, JSValueToBoolean(ctx, Back(this))); // Force convert
		return worker->addToScope<JSBoolean>(ret);
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
			return THROW_ERR("Invalid conversion toInt32, Range overflow"), Maybe<int>;
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
		if (isUint32Range(num)) {
			return THROW_ERR("Invalid conversion toUint32, Range overflow"), Maybe<int>;
		}
		auto ret = JSValueMakeNumber(ctx, int(num));
		return worker->addToScope<JSUint32>(ret);
	}

	Maybe<String> JSValue::asString(Worker *w) const {
		ENV(w);
		if (JSValueIsString(ctx, Back(this))) {
			return static_cast<JSString*>(this)->value(w);
		}
		return Maybe<String>();
	}

	Maybe<bool> JSValue::asBoolean(Worker* w) const {
		return JSValueToBoolean(JSC_CTX(w), Back(this));
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

	JSValue* JSObject::get(Worker* w, JSValue* key) {
		DCHECK(isObject());
		ENV(w);
		auto ret = JSObjectGetPropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), OK(nullptr));
		// worker->addToScope(ret);
		return Cast(ret);
	}

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
		JSObjectSetPropertyAtIndex(ctx, Back<JSObjectRef>(this), index, Back(val), 0, OK(false));
		return true;
	}

	bool JSObject::has(Worker* w, JSValue* key) {
		DCHECK(isObject());
		ENV(worker);
		return JSObjectHasPropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), OK(false));
	}

	bool JSObject::has(Worker* w, uint32_t index) {
		DCHECK(isObject());
		ENV(worker);
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
		return worker->addToScope<JSBoolean>(ret);
	}

	Maybe<Array<String>> JSObject::getPropertyKeys(Worker* w) {
		DCHECK(isObject());
		ENV(w);
		JSCPropertyNameArrayPtr names = JSObjectCopyPropertyNames(ctx, Back<JSObjectRef>(this));
		auto count = JSPropertyNameArrayGetCount(*names);
		Array<String> ret;
		for (int i = 0; i < count; i++) {
			auto s = JSPropertyNameArrayGetNameAtIndex(*names, i)
			size_t len = JSStringGetLength(*s);
			const JSChar* ch = JSStringGetCharactersPtr(*s);
			auto buf = codec_utf16_to_utf8(ArrayWeak(ch, len).buffer());
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
		return js::asInt32(len);
	}

	String JSString::value(Worker* w) const {
		DCHECK(isString());
		ENV(w);
		JSCStringPtr s = JSValueToStringCopy(ctx, Back(this), JsFatal("JSString::value()"));
		size_t len = JSStringGetLength(*s);
		const JSChar* ch = JSStringGetCharactersPtr(*s);
		auto buf = codec_utf16_to_utf8(ArrayWeak(ch, len).buffer());
		return buf.collapseString();
	}

	String2 JSString::value2(Worker* w) const {
		DCHECK(isString());
		ENV(w);
		JSCStringPtr s = JSValueToStringCopy(ctx, Back(this), JsFatal("JSString::value2()"));
		size_t len = JSStringGetLength(*s);
		const JSChar* ch = JSStringGetCharactersPtr(*s);
		return String2(ch, len);
	}

	String4 JSString::value4(Worker* w) const {
		DCHECK(isString());
		ENV(w);
		JSCStringPtr s = JSValueToStringCopy(ctx, Back(this), JsFatal("JSString::value4()"));
		size_t len = JSStringGetLength(*s);
		const JSChar* ch = JSStringGetCharactersPtr(*s);
		auto str4 = codec_utf16_to_unicode(ArrayWeak(ch, len).buffer());
		return str4.collapseString();
	}

	int JSArray::length() const {
		ENV();
		auto val = JSObjectGetProperty(ctx, Back<JSObjectRef>(this), length_s, JsFatal("JSArray::length()"));
		auto len = js::asInt32(val);
		return len;
	}

	double JSDate::valueOf() const {
		ENV();
		auto val = JSObjectCallAsFunction(ctx,
			worker->_data.global_Date_prototype_valueOf,
			Back<JSObjectRef>(this), 0. nullptr, JsFatal("JSDate::valueOf()")
		);
		auto val = js::asDouble(val);
		return val;
	}

	double JSNumber::value() const {
		if (js::isInt32(this))
			return js::asInt32(this)
		else
			return js::asDouble(this);
	}

	int JSInt32::value() const {
		return js::asInt32(this);
	}

	uint32_t JSUint32::value() const {
		if (js::isInt32(this))
			return js::asInt32(this)
		else
			return js::asDouble(this);
	}

	bool JSBoolean::value() const {
		return js::asBoolean(this);
	}

	JSValue* JSFunction::call(Worker* w, int argc, JSValue* argv[], JSValue* recv) {
		DCHECK(isFunction());
		ENV(w);
		auto rev = JSObjectCallAsFunction(ctx, Cast<JSObjectRef>(this),
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
		auto rev = JSObjectCallAsConstructor(ctx, Cast<JSObjectRef>(this),
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
		DCHECK(isArrayBuffer());
		ENV(w);
		auto buff = JSObjectGetTypedArrayBuffer(ctx, Back<JSObjectRef>(this), JsFatal("JSArrayBuffer::buffer()"));
		DCHECK(buff);
		return Cast<JSArrayBuffer*>(buff);
	}

	uint32_t JSTypedArray::byteLength(Worker* w) {
		DCHECK(isTypedArray());
		ENV(w);
		auto len = JSObjectGetTypedArrayByteLength(ctx, Back<JSObjectRef>(this), JsFatal("JSTypedArray::byteLength()"));
		return len;
	}

	uint32_t JSTypedArray::byteOffset(Worker* w) {
		DCHECK(isTypedArray());
		ENV(w);
		auto off = JSObjectGetTypedArrayByteOffset(ctx, Back<JSObjectRef>(this), JsFatal("JSTypedArray::byteOffset()"));
		return off;
	}

	inline bool isSet(Worker* w, JSValue* val) {
		ENV(w);
		auto ok = JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.global_Set, JsFatal("isSet"));
		return ok;
	}

	bool JSSet::add(Worker* w, JSValue* key) {
		ENV(w);
		DCHECK(isSet(w, this));
		auto argv = Back(key);
		JSObjectCallAsFunction(ctx,
			worker->_data.global_Set_prototype_add, Back<JSObjectRef>(this), 1, &argv, OK(false)
		);
		return true;
	}

	bool JSSet::has(Worker* w, JSValue* key) {
		ENV(w);
		DCHECK(isSet(w, this));
		auto argv = Back(key);
		auto rev = JSObjectCallAsFunction(ctx,
			worker->_data.global_Set_prototype_has, Back<JSObjectRef>(this), 1, &argv, OK(false)
		);
		return js::asBoolean(rev);
	}

	bool JSSet::deleteFor(Worker* w, JSValue* key) {
		ENV(w);
		DCHECK(isSet(w, this));
		auto argv = Back(key);
		auto rev = JSObjectCallAsFunction(ctx,
			worker->_data.global_Set_prototype_delete, Back<JSObjectRef>(this), 1, &argv, OK(false)
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

	JSInt32* Worker::newValue(Char val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSInt32>(ret);
	}

	JSUint32* Worker::newValue(uint8_t val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSInt32>(ret);
	}

	JSInt32* Worker::newValue(int16_t val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSInt32>(ret);
	}

	JSUint32* Worker::newValue(uint16_t val) {
		ENV(this);
		auto ret = JSValueMakeNumber(ctx, val);
		return worker->addToScope<JSUint32>(ret);
	}

	JSInt32* Worker::newValue(int val) {
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
		auto str = JSStringCreateWithUTF8CString(*val);
		auto obj = JSValueMakeString(JSX_CTX(this), str);
		DCHECK(obj);
		return WORKER(this)->addToScope<JSString>(obj);
	}

	JSString* Worker::newValue(cString& val) {
		auto str = JSStringCreateWithUTF8CString(*val);
		auto obj = JSValueMakeString(JSX_CTX(this), str);
		DCHECK(obj);
		return WORKER(this)->addToScope<JSString>(str);
	}

	JSString* Worker::newValue(cString2& val) {
		auto str = JSStringCreateWithCharacters(*val, val.length());
		auto obj = JSValueMakeString(JSX_CTX(this), str);
		DCHECK(obj);
		return WORKER(this)->addToScope<JSString>(obj);
	}

	JSUint8Array* Worker::newValue(Buffer&& buff) {
		ENV(this);
		auto arr = JSObjectMakeTypedArrayWithBytesNoCopy(
			ctx, kJSTypedArrayTypeUint8Array, *buff, buff.length(), [](void* bytes, void* data) {
			Allocator::free(bytes);
		}, nullptr, OK(nullptr));
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

	JSValue* JscWorker::runScript(JSString* jsSource, cString& source, JSString* name, JSObject* sandbox) {
		ENV(this);
		auto url = JSValueToStringCopy(ctx, Back(name), OK(nullptr));
		DCHECK(url);

		if (sandbox) {
			DCHECK(sandbox->isObject());
			String sandboxExpand;
			for (auto &k: sandbox->getPropertyKeys(this).from(Array<String>())) {
				sandboxExpand += String::format("var %s=sandbox.%s;", *k, *k);
			}
			auto body = String::format("(function(sandbox){%s%s;return sandbox})",
				*sandboxExpand, source.isEmpty() ? *jsSource->value(this): *source
			);
			auto script = JSStringCreateWithUTF8CString(*body);
			auto func = JSEvaluateScript(ctx, body, nullptr, url, 1, OK(nullptr));
			auto argv = sandbox->cast();
			return Cast<JSFunction>(func)->call(this, 1, &argv);
		} else {
			JSStringRef script;
			if (jsSource) {
				script = JSValueToStringCopy(ctx, Back(jsSource), OK(nullptr));
			} else {
				DCHECK(!source.isEmpty());
				script = JSStringCreateWithUTF8CString(*source);
			}
			DCHECK(script);
			auto val = JSEvaluateScript(ctx, script, nullptr, url, 1, OK(nullptr));
			return worker->addToScope(val);
		}
	}

	JSValue* Worker::runScript(cString& source, cString& name, JSObject* sandbox) {
		return WORKER(this)->runScript(nullptr, source, newValue(name), sandbox);
	}

	JSValue* Worker::runScript(JSString* source, JSString* name, JSObject* sandbox) {
		return WORKER(this)->runScript(source, String(), name, sandbox);
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

	int StartPlatform(int argc, Char** argv, int (*exec)(Worker *worker)) {
		Sp<Worker> worker = Worker::Make();
		return exec(*worker); // exec main script
	}

	Qk_Init_Func(jsc_th_key_init) {
		uv_key_create(&th_key);
		initFactorys();
	};
} }
