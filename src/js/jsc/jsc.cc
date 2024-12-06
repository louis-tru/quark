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
#include <uv.h>
#include <limits>
// #include <cmath>

namespace qk { namespace js {
	static uv_key_t th_key;

	#define _Fun(name) \
		JSStringRef name##_s = JsStringWithUTF8(name);
	Js_Const_Strings(_Fun)
	// JSStringRef name##_s = JSStringRetain(JsStringWithUTF8(name));
	#undef _Fun

	void jsFatal(JSContextRef ctx, JSValueRef ex_) {
		if (ex_) { // err
			auto ex = (JSObjectRef)ex_;
			if (!JSValueIsObject(ctx, ex)) {
				auto Error = JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), Error_s, nullptr);
				DCHECK(Error);
				ex = JSObjectCallAsConstructor(ctx, Error, 1, &ex, nullptr);
				DCHECK(ex);
			}
			auto ex = (JSObjectRef)ex_;
			JSValueRef line = JSObjectGetProperty(ctx, ex, line_s, 0);
			JSValueRef column = JSObjectGetProperty(ctx, ex, column_s, 0);
			JSValueRef message = JSObjectGetProperty(ctx, ex, message_s, 0);
			JSValueRef stack = JSObjectGetProperty(ctx, ex, stack_s, 0);
			double l = JSValueToNumber(ctx, line, 0);
			double c = JSValueToNumber(ctx, column, 0);
			auto m = jsToString(ctx, message);
			auto s = jsToString(ctx, stack);
			qk::Fatal("", l, "", "%s\n\n%s", m.c_str(), s.c_str());
		}
	}

	String jsToString(JSStringRef value) {
		DCHECK(value);
		size_t bufferSize = JSStringGetMaximumUTF8CStringSize(value);
		char* str = (char*)malloc(bufferSize);
		auto size = JSStringGetUTF8CString(value, str, bufferSize);
		return Buffer(str, size, bufferSize).collapseString();
	}

	JSObjectRef runNativeScript(JSGlobalContextRef ctx, cChar* script, cChar* name) {
		JSValueRef ex = nullptr;
		JSObjectRef global = (JSObjectRef)JSContextGetGlobalObject(ctx);
		JSCStringPtr jsc_v8_js = JsStringWithUTF8(name);
		JSCStringPtr script2 = JsStringWithUTF8(script);
		auto fn = (JSObjectRef)JSEvaluateScript(ctx, *script2, nullptr, *jsc_v8_js, 1, JsFatal(ctx));
		auto exports = JSObjectMake(ctx, 0, 0);
		JSValueProtect(ctx, exports); // Protect
		JSValueRef argv[2] = { exports, global };
		JSObjectCallAsFunction(ctx, fn, 0, 2, argv, JsFatal(ctx));
		JSValueUnprotect(ctx, exports); // Unprotect
		return exports;
	}

	void WorkerData::initialize(JSGlobalContextRef ctx) {
		JSValueRef ex = nullptr;
		auto global = JSContextGetGlobalObject(ctx);
		#define _Init_Fun(name,from) from##_##name = (JSObjectRef) \
		JSObjectGetProperty(ctx, from, *JSCStringPtr(JsStringWithUTF8(name)), JsFatal(ctx)); \
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

	Worker* Worker::current() {
		return first_worker ? first_worker : reinterpret_cast<Worker*>(uv_key_get(&th_key));
	}

	Worker* Worker::Make() {
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

	inline int32_t asInt32(JSValue* val) {
		DCHECK(isInt32(val));
#if Qk_ARCH_64BIT
		return bitwise_cast<JscValueImpl*>(val)->u.asInt64;
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asBits.payload;
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

	inline double asDouble(JSValue* val) {
		DCHECK(isDouble(val));
#if 1
		return reinterpretInt64ToDouble(bitwise_cast<JscValueImpl*>(val)->u.asInt64 - DoubleEncodeOffset);
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asDouble;
#endif
	}

	inline bool isBoolean(JSValue* val) {
#if Qk_ARCH_64BIT
		return (bitwise_cast<JscValueImpl*>(val)->u.asInt64 & ~1) == ValueFalse;
#else
		return bitwise_cast<JscValueImpl*>(val)->u.asBits.tag == BooleanTag;
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

// 	inline JSCell* asCell(JSValue* val) {
// #if Qk_ARCH_64BIT
// 		return bitwise_cast<JscValueImpl*>(val)->u.ptr;
// #else
// 		return reinterpret_cast<JSCell*>(bitwise_cast<JscValueImpl*>(val)->u.asBits.payload);
// #endif
// 	}

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
		// return JSValueIsBoolean(test_type_jsc_ctx, Back(this));
		return isBoolean(this);
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
		// return JSValueIsNumber(test_type_jsc_ctx, Back(this));
#if Qk_ARCH_64BIT
		bitwise_cast<JscValueImpl*>(this)->asInt64 & NumberTag;
#else
		return js::isInt32(this) || isDouble(this);
#endif
	}

	bool JSValue::isInt32() const {
		return js::isInt32(this);
	}

	bool JSValue::isUint32() const {
		return isInt32() && asInt32(this) >= 0;
	}

	bool JSValue::isFunction() const {
		ENV();
		return JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.global_Function, ex. OK(false));
	}

	bool JSValue::isArrayBuffer() const {
		ENV();
		return JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.global_ArrayBuffer, ex. OK(false));
	}

	bool JSValue::isTypedArray() const {
		ENV();
		return JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.TypedArray, ex. OK(false));
	}

	bool JSValue::isUint8Array() const {
		ENV();
		return JSValueIsInstanceOfConstructor(ctx, Back(this), worker->_data.global_Uint8Array, ex. OK(false));
	}

	bool JSValue::equals(Worker *worker, JSValue* val) const {
		ENV(worker);
		return JSValueIsEqual(ctx, Back(this), Back(val), ex, OK(false));
	}

	bool JSValue::strictEquals(JSValue* val) const {
		ENV(worker);
		return JSValueIsStrictEqual(ctx, Back(this), Back(val));
	}

	bool JSValue::instanceOf(Worker* worker, JSObject* constructor) {
		ENV(worker);
		return JSValueIsInstanceOfConstructor(ctx, Back(this), Back<JSObjectRef>(constructor), ex. OK(false));
	}

	JSNumber* JSValue::toNumber(Worker* w) const {
		if (isNumber()) {
			return bitwise_cast<JSNumber*>(this);
		}
		ENV(w);
		auto num = JSValueToNumber(ctx, Back(this), ex, OK(nullptr)); // Force convert
		auto ret = JSValueMakeNumber(ctx, num);
		return worker->addToScope<JSNumber>(ret);
	}

	inline bool isInt32Range(double value) {
		constexpr double mix = std::numeric_limits<int32_t>::min();
		constexpr double max = std::numeric_limits<int32_t>::max();
		if (num < mix || max < num) {
			return false;
		} else {
			return true;
		}
	}

	inline bool isUint32Range(double value) {
		constexpr double mix = 0;
		// The jsc number storage structure only uses half a uint32,
		// which makes the conversion more efficient
		constexpr double max = std::numeric_limits<int32_t>::max(); // jsc 
		if (num < mix || max < num) {
			return false;
		} else {
			return true;
		}
	}

	JSInt32* JSValue::toInt32(Worker* w) const {
		if (isInt32()) {
			return bitwise_cast<JSInt32*>(this);
		}
		ENV(w);
		double num;
		if (isDouble(this)) {
			num = asDouble(this);
		} else {
			num = JSValueToNumber(ctx, Back(this), ex, OK(nullptr)); // Force convert
		}
		if (!isInt32Range(num)) {
			return THROW_ERR("Invalid conversion toInt32, Range overflow"), Maybe<int>;
		}
		auto ret = JSValueMakeNumber(ctx, int(num));
		return worker->addToScope<JSInt32>(ret);
	}

	JSUint32* JSValue::toUint32(Worker* worker) const {
		ENV(w);
		if (isInt32()) {
			if (asInt32(this) >= 0) {
				return bitwise_cast<JSUint32*>(this);
			} else {
				return THROW_ERR("Invalid conversion toUint32, Can't be a negative number"), nullptr;
			}
		}
		double num;
		if (isDouble(this)) {
			num = asDouble(this);
		} else {
			num = JSValueToNumber(ctx, Back(this), ex, OK(nullptr)); // Force convert
		}
		if (isUint32Range(num)) {
			return THROW_ERR("Invalid conversion toUint32, Range overflow"), Maybe<int>;
		}
		auto ret = JSValueMakeNumber(ctx, int(num));
		return worker->addToScope<JSUint32>(ret);
	}

	JSBoolean* JSValue::toBoolean(Worker* w) const {
		if (isBoolean())
			return bitwise_cast<JSBoolean*>(this);
		ENV(w);
		auto ret = JSValueMakeBoolean(ctx, JSValueToBoolean(ctx, Back(this))); // Force convert
		return worker->addToScope<JSBoolean>(ret);
	}

	JSString* JSValue::toString(Worker* worker) const {
		ENV(worker);
		// TODO ...
		return nullptr;
	}

	String JSValue::toStringValue(Worker* worker, bool oneByte) const {
		// v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		// if (!str->Length()) return String();
		// if ( oneByte ) {
		// 	Buffer buffer(str->Length());
		// 	str->WriteOneByte(ISOLATE(worker), (uint8_t*)*buffer, 0, buffer.capacity());
		// 	return buffer.collapseString();
		// } else {
		// 	uint16_t source[128];
		// 	int start = 0, count;
		// 	auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;
		// 	Array<String> rev;

		// 	while ( (count = str->Write(ISOLATE(worker), source, start, 128, opts)) ) {
		// 		auto unicode = codec_decode_form_utf16(ArrayWeak<uint16_t>(source, count).buffer());
		// 		rev.push(codec_encode(kUTF8_Encoding, unicode).collapseString());
		// 		start += count;
		// 	}

		// 	return rev.length() == 0 ? String():
		// 				 rev.length() == 1 ? rev[0]:
		// 				 rev.join(String());
		// }
	}

	String2 JSValue::toStringValue2(Worker* worker) const {
		// v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		// if (!str->Length()) return String2();
		// ArrayBuffer<uint16_t> source(str->Length());
		// str->Write(ISOLATE(worker), *source, 0, str->Length());
		// return source.collapseString();
	}

	String4 JSValue::toStringValue4(Worker* worker) const {
		// v8::Local<v8::String> str = ((v8::Value*)this)->ToString(CONTEXT(worker)).ToLocalChecked();
		// if (!str->Length()) return String4();

		// uint16_t source[128];
		// int start = 0, count, revOffset = 0;
		// auto opts = v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION;
		// Array<uint32_t> rev(str->Length());

		// while ( (count = str->Write(ISOLATE(worker), source, start, 128, opts)) ) {
		// 	auto unicode = codec_decode_form_utf16(ArrayWeak<uint16_t>(source, count).buffer());
		// 	rev.write(*unicode, unicode.length(), revOffset);
		// 	revOffset += unicode.length();
		// 	start += count;
		// }
		// rev.reset(revOffset);
		// return rev.collapseString();
	}

	Maybe<float> JSValue::toFloatValue(Worker* w) const {
		ENV(w);
		auto num = JSValueToNumber(ctx, Back(this), ex, OK(Maybe<float>())); // Force convert
		return Maybe<float>(num);
	}

	Maybe<double> JSValue::toNumberValue(Worker* w) const {
		ENV(w);
		auto num = JSValueToNumber(ctx, Back(this), ex, OK(Maybe<double>())); // Force convert
		return Maybe<double>(num);
	}

	Maybe<int> JSValue::toInt32Value(Worker* w) const {
		if (isInt32()) {
			return Maybe<int>(asInt32(this));
		}
		ENV(w);
		double num;
		if (isDouble(this)) {
			num = asDouble(this);
		} else {
			num = JSValueToNumber(ctx, Back(this), ex, OK(Maybe<int>())); // Force convert
		}
		if (!isInt32Range(num)) {
			return THROW_ERR("Invalid conversion toInt32Value, Range overflow"), Maybe<int>();
		}
		return Maybe<int>(num);
	}

	Maybe<uint32_t> JSValue::toUint32Value(Worker* w) const {
		ENV(w);
		if (isInt32()) {
			int32_t out = asInt32(this);
			if (out >= 0) {
				return Maybe<uint32_t>(out);
			} else {
				return THROW_ERR("Invalid conversion toUint32Value, Can't be a negative number"), Maybe<uint32_t>;
			}
		}
		double num;
		if (isDouble(this)) {
			num = asDouble(this);
		} else {
			num = JSValueToNumber(ctx, Back(this), ex, OK(Maybe<uint32_t>())); // Force convert
		}
		if (isUint32Range(num)) {
			return THROW_ERR("Invalid conversion toUint32Value, Range overflow"), Maybe<uint32_t>();
		}
		return Maybe<uint32_t>(num);
	}

	bool JSValue::toBooleanValue(Worker* w) const {
		return JSValueToBoolean(JSC_CTX(w), Back(this));
	}

	JSValue* JSObject::get(Worker* w, JSValue* key) {
		ENV(w);
		auto ret = JSObjectGetPropertyForKey(ctx,
			Back<JSObjectRef>(this), Back(key), ex, OK(nullptr)
		);
		// worker->addToScope(ret);
		return Cast(ret);
	}

	JSValue* JSObject::get(Worker* w, uint32_t index) {
		ENV(w);
		auto ret = JSObjectGetPropertyAtIndex(ctx, Back<JSObjectRef>(this), index, ex, OK(nullptr));
		// worker->addToScope(ret);
		return Cast(ret);
	}

	bool JSObject::set(Worker* w, JSValue* key, JSValue* val) {
		ENV(w);
		JSObjectSetPropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), Back(val), 0, ex, OK(false));
		return true;
	}

	bool JSObject::set(Worker* w, uint32_t index, JSValue* val) {
		ENV(w);
		JSObjectSetPropertyAtIndex(ctx, Back<JSObjectRef>(this), index, Back(val), 0, ex, OK(false));
		return true;
	}

	bool JSObject::has(Worker* w, JSValue* key) {
		ENV(worker);
		return JSObjectHasPropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), ex, OK(false));
	}

	bool JSObject::has(Worker* worker, uint32_t index) {
		ENV(worker);
		return JSObjectHasPropertyForKey(ctx,
			Back<JSObjectRef>(this), JSValueMakeNumber(ctx, index), ex, OK(false)
		);
	}

	bool JSObject::JSObject::deleteFor(Worker* w, JSValue* key) {
		ENV(w);
		return JSObjectDeletePropertyForKey(ctx, Back<JSObjectRef>(this), Back(key), ex, OK(false));
	}

	bool JSObject::deleteFor(Worker* worker, uint32_t index) {
		ENV(w);
		return JSObjectDeletePropertyForKey(ctx,
			Back<JSObjectRef>(this), JSValueMakeNumber(ctx, index), ex, OK(false)
		);
	}

	JSArray* JSObject::getPropertyNames(Worker* w) {
		ENV(w);
		JSCPropertyNameArrayPtr names = JSObjectCopyPropertyNames(ctx, Back<JSObjectRef>(this));
		auto ret = JSObjectMakeArray(ctx, 0, nullptr, ex, OK(nullptr));
		auto count = JSPropertyNameArrayGetCount(*names);

		for (int i = 0; i < count; i++) {
			auto key = JSValueMakeString(ctx, JSPropertyNameArrayGetNameAtIndex(*names, i));
			JSObjectSetPropertyAtIndex(ctx, ret, i, key, ex, OK(nullptr));
		}
		return worker->addToScope<JSBoolean>(ret);
	}

	JSFunction* JSObject::getConstructor(Worker* w) {
		ENV(w);
		auto ret = JSObjectGetProperty(ctx, Back<JSObjectRef>(this), constructor_s, ex, OK(nullptr));
		auto jsret = Cast<JSFunction>(ret);
		// worker->addToScope(ret);
		DCHECK(jsret->isFunction());
		return jsret;
	}

	bool JSObject::defineOwnProperty(Worker *w, JSValue *key, JSValue *value, int flags) {
		ENV(w);
		JSPropertyAttributes attrs = flags << 1;
		JSObjectSetPropertyForKey(ctx,
			Back<JSObjectRef>(this), Back(key), Back(value), attrs, ex, OK(false)
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
		// return reinterpret_cast<const v8::String*>(this)->Length();
		//JSStringGetLength()
		// JSValueToStringCopy(0, 0, 0);
		//JSStringGetCharactersPtr()
	}

	JSString* JSString::Empty(Worker* w) {
		return Cast<JSString>(WORKER(w)->_data.EmptyString);
	}

	int JSArray::length() const {
		ENV();
		auto val = JSObjectGetProperty(ctx, Back<JSObjectRef>(this), length_s, ex, OK(0));
		auto len = asInt32(val);
		return len;
	}

	double JSDate::valueOf() const {
		return 0;
	}

	double JSNumber::value() const {
		if (isInt32(this))
			return asInt32(this)
		else
			return asDouble(this);
	}

	int JSInt32::value() const {
		return asInt32(this);
	}

	uint32_t JSUint32::value() const {
		return asInt32(this);
	}

	bool JSBoolean::value() const {
		return asBoolean(this);
	}

	JSValue* JSFunction::call(Worker* worker, int argc, JSValue* argv[], JSValue* recv) {
		// if ( !recv ) {
		// 	recv = worker->newUndefined();
		// }
		// auto fn = reinterpret_cast<v8::Function*>(this);
		// v8::MaybeLocal<v8::Value> r = fn->Call(CONTEXT(worker), Back(recv), argc,
		// 																				reinterpret_cast<v8::Local<v8::Value>*>(argv));
		// Local<v8::Value> out;
		// return r.ToLocal(&out) ? Cast(out): nullptr;
	}

	JSValue* JSFunction::call(Worker* worker, JSValue* recv) {
		// return call(worker, 0, nullptr, recv);
	}

	JSObject* JSFunction::newInstance(Worker* worker, int argc, JSValue* argv[]) {
		// auto fn = reinterpret_cast<v8::Function*>(this);
		// v8::MaybeLocal<v8::Object> r = fn->NewInstance(CONTEXT(worker), argc,
		// 																								reinterpret_cast<v8::Local<v8::Value>*>(argv));
		// return Cast<JSObject>(r);
	}

	JSObject* JSFunction::getFunctionPrototype(Worker* worker) {
		// auto fn = reinterpret_cast<v8::Function*>(this);
		// auto str = Back(worker->strs()->prototype());
		// auto r = fn->Get(CONTEXT(worker), str);
		// return Cast<JSObject>(r);
	}

	uint32_t JSArrayBuffer::byteLength(Worker* worker) const {
		// return (uint32_t)reinterpret_cast<const v8::ArrayBuffer*>(this)->ByteLength();
	}

	Char* JSArrayBuffer::data(Worker* worker) {
		// return (Char*)reinterpret_cast<v8::ArrayBuffer*>(this)->GetContents().Data();
	}

	JSArrayBuffer* JSTypedArray::buffer(Worker* worker) {
		// auto typedArray = reinterpret_cast<v8::TypedArray*>(this);
		// v8::Local<v8::ArrayBuffer> abuff = typedArray->Buffer();
		// return Cast<JSArrayBuffer>(abuff);
	}

	uint32_t JSTypedArray::byteLength(Worker* worker) {
		// return (uint32_t)reinterpret_cast<v8::TypedArray*>(this)->ByteLength();
	}

	uint32_t JSTypedArray::byteOffset(Worker* worker) {
		// return (uint32_t)reinterpret_cast<v8::TypedArray*>(this)->ByteOffset();
	}

	bool JSSet::add(Worker* worker, JSValue* key) {
		// auto set = reinterpret_cast<v8::Set*>(this);
		// return !set->Add(CONTEXT(worker), Back(key)).IsEmpty();
	}

	bool JSSet::has(Worker* worker, JSValue* key) {
		// auto set = reinterpret_cast<v8::Set*>(this);
		// return set->Has(CONTEXT(worker), Back(key)).ToChecked();
	}

	bool JSSet::deleteFor(Worker* worker, JSValue* key) {
		// auto set = reinterpret_cast<v8::Set*>(this);
		// return set->Delete(CONTEXT(worker), Back(key)).ToChecked();
	}

	template <> void Persistent<JSValue>::reset() {
		// reinterpret_cast<v8::Persistent<v8::Value>*>(this)->Reset();
	}

	template <> template <>
	void Persistent<JSValue>::reset(Worker* worker, JSValue* other) {
		// Qk_Assert(worker);
		// reinterpret_cast<v8::Persistent<v8::Value>*>(this)->
		// 	Reset(ISOLATE(worker), *reinterpret_cast<const v8::Local<v8::Value>*>(&other));
		// _worker = worker;
	}

	template<> template<>
	void Persistent<JSValue>::copy(const Persistent<JSValue>& that) {
		// reset();
		// if (that.isEmpty())
		// 	return;
		// Qk_Assert(that._worker);
		// typedef v8::CopyablePersistentTraits<v8::Value>::CopyablePersistent Handle;
		// reinterpret_cast<Handle*>(this)->operator=(*reinterpret_cast<const Handle*>(&that));
		// _worker = that._worker;
	}

	template<>
	JSValue* Persistent<JSValue>::operator*() const {
		// // return Cast(Local<Value>::New(ISOLATE(_worker), Back(_val)));
		// add to scope ... ?
		// return _val;
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
		// TODO ...
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
		// return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									// v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newValue(cString& val) {
		// return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									// v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newValue(cString2& val) {
		// return Cast<JSString>(v8::String::NewExternalTwoByte(
			// ISOLATE(this),
			// new V8ExternalStringResource(data)
		// ));
	}

	JSUint8Array* Worker::newValue(Buffer&& buff) {
		// size_t offset = 0;
		// size_t len = buff.length();
		// v8::Local<v8::ArrayBuffer> ab;
		// if (buff.length()) {
		// 	size_t len = buff.length();
		// 	Char* data = buff.collapse();
		// 	ab = v8::ArrayBuffer::New(ISOLATE(this), data, len, ArrayBufferCreationMode::kInternalized);
		// } else {
		// 	ab = v8::ArrayBuffer::New(ISOLATE(this), 0);
		// }
		// return Cast<JSUint8Array>(v8::Uint8Array::New(ab, offset, len));
	}

	JSObject* Worker::newObject() {
		// return Cast<JSObject>(v8::Object::New(ISOLATE(this)));
	}

	JSArray* Worker::newArray(uint32_t len) {
		// return Cast<JSArray>(v8::Array::New(ISOLATE(this), len));
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
		// return Cast<JSSet>(v8::Set::New(ISOLATE(this)));
	}

	JSString* Worker::newStringOneByte(cString& data) {
		// return Cast<JSString>(v8::String::NewExternal(ISOLATE(this),
																									// new V8ExternalOneByteStringResource(data)));
	}

	JSArrayBuffer* Worker::newArrayBuffer(Char* use_buff, uint32_t len) {
		// return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), use_buff, len));
	}

	JSArrayBuffer* Worker::newArrayBuffer(uint32_t len) {
		// return Cast<JSArrayBuffer>(v8::ArrayBuffer::New(ISOLATE(this), len));
	}

	JSUint8Array* Worker::newUint8Array(JSArrayBuffer* abuffer, uint32_t offset, uint32_t size) {
		// auto ab2 = Back<v8::ArrayBuffer>(abuffer);
		// offset = Qk_Min((uint)ab2->ByteLength(), offset);
		// if (size + offset > ab2->ByteLength()) {
		// 	size = (uint)ab2->ByteLength() - offset;
		// }
		// return Cast<JSUint8Array>(v8::Uint8Array::New(ab2, offset, size));
	}

	JSObject* Worker::newRangeError(cChar* errmsg, ...) {
		// va_list arg;
		// va_start(arg, errmsg);
		// auto str = _Str::printfv(errmsg, arg);
		// va_end(arg);
		// return Cast<JSObject>(v8::Exception::RangeError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newReferenceError(cChar* errmsg, ...) {
		// va_list arg;
		// va_start(arg, errmsg);
		// auto str = _Str::printfv(errmsg, arg);
		// va_end(arg);
		// return Cast<JSObject>(v8::Exception::ReferenceError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newSyntaxError(cChar* errmsg, ...) {
		// va_list arg;
		// va_start(arg, errmsg);
		// auto str = _Str::printfv(errmsg, arg);
		// va_end(arg);
		// return Cast<JSObject>(v8::Exception::SyntaxError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newTypeError(cChar* errmsg, ...) {
		// va_list arg;
		// va_start(arg, errmsg);
		// auto str = _Str::printfv(errmsg, arg);
		// va_end(arg);
		// return Cast<JSObject>(v8::Exception::TypeError(Back<v8::String>(newValue(str))));
	}

	JSObject* Worker::newValue(cError& err) {
		// v8::Local<v8::Object> e =
		// 	v8::Exception::Error(Back<v8::String>(newValue(err.message()))).As<v8::Object>();
		// e->Set(Back(strs()->Errno()), Back(newValue(err.code())));
		// return Cast<JSObject>(e);
	}

	void Worker::throwError(JSValue* exception) {
		// ISOLATE(this)->ThrowException(Back(exception));
	}

	JSValue* Worker::runScript(JSString* source, JSString* name, JSObject* sandbox) {
		// v8::MaybeLocal<v8::Value> r = WORKER(this)->runScript(Back<v8::String>(source),
		// 																											Back<v8::String>(name),
		// 																											Back<v8::Object>(sandbox));
		// return Cast(r.FromMaybe(v8::Local<v8::Value>()));
	}

	JSValue* Worker::runScript(cString& source, cString& name, JSObject* sandbox) {
		// return runScript(newValue(source), newValue(name), sandbox);
	}

	JSValue* Worker::runNativeScript(cBuffer& source, cString& name, JSObject* exports) {
		// return WORKER(this)->runNativeScript(source, name, exports);
	}

	void Worker::garbageCollection() {
	}

	void runDebugger(Worker* worker, const DebugOptions &opts) {
	}

	void stopDebugger(Worker* worker) {
	}

	void debuggerBreakNextStatement(Worker* worker) {
	}

	int platformStart(int argc, Char** argv, int (*exec)(Worker *worker)) {
		Sp<Worker> worker = Worker::Make();
		// v8::SealHandleScope sealhandle(ISOLATE(*worker));
		// v8::HandleScope handle(ISOLATE(*worker));
		int rc = exec(*worker); // exec main script
		return rc;
	}

	Qk_Init_Func(jsc_th_key_init) {
		uv_key_create(&th_key);
		initFactorys();
	};
} }
