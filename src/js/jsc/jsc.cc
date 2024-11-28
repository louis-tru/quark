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

namespace qk { namespace js {
	static uv_key_t th_key;
	static Worker* first_worker = nullptr;
	static std::atomic_int workers = 0;

	static JSStringRef __native__body_s = JsStringWithUTF8(
		"return arguments.callee.__native__.apply(this,arguments)");
	static JSStringRef anonymous_s = JsStringWithUTF8("<anonymous>");
	static JSStringRef eval_s = JsStringWithUTF8("<eval>");
	static JSStringRef empty_s = JsStringWithUTF8("");

	#define _Fun(name) \
		static JSStringRef name##_s = JsStringWithUTF8(name);
	Js_Const_List(_Fun)
	Js_Worker_Data_List(_Fun)
	Js_Context_Data_List(_Fun)
	#undef _Fun

	void checkFatal(JSContextRef ctx, JSValueRef exception) {
		if (exception) { // err
			JSValueRef line = JSObjectGetProperty(ctx, (JSObjectRef)exception, line_s, 0);
			JSValueRef column = JSObjectGetProperty(ctx, (JSObjectRef)exception, column_s, 0);
			JSValueRef message = JSObjectGetProperty(ctx, (JSObjectRef)exception, message_s, 0);
			JSValueRef stack = JSObjectGetProperty(ctx, (JSObjectRef)exception, stack_s, 0);
			double l = JSValueToNumber(ctx, line, 0);
			double c = JSValueToNumber(ctx, column, 0);
			// std::string m = Isolate::ToSTDString(ctx, message);
			// std::string s = Isolate::ToSTDString(ctx, stack);
			// v8::fatal("", l, "", "%s\n\n%s", m.c_str(), s.c_str());
		}
	}

	JSObjectRef runNativeScript(JSGlobalContextRef ctx, cChar* script, cChar* name) {
		JSValueRef ex = nullptr;
		JSObjectRef global = (JSObjectRef)JSContextGetGlobalObject(ctx);
		JSCStringPtr jsc_v8_js = JSStringCreateWithUTF8CString(name);
		JSCStringPtr script2 = JSStringCreateWithUTF8CString(script);
		auto fn = (JSObjectRef)JSEvaluateScript(ctx, *script2, nullptr, *jsc_v8_js, 1, JsFatal(ctx));
		auto exports = JSObjectMake(ctx, 0, 0);
		JSValueProtect(ctx, exports); // Protect
		JSValueRef argv[2] = { exports, global };
		JSObjectCallAsFunction(ctx, fn, 0, 2, argv, JsFatal(ctx));
		JSValueUnprotect(ctx, exports); // UnProtect
		return exports;
	}

	void GlobalData::initialize(JSGlobalContextRef ctx) {
		JSValueRef ex = nullptr;
		Undefined = JSValueMakeUndefined(ctx);   JSValueProtect(ctx, Undefined);
		Null = JSValueMakeNull(ctx);             JSValueProtect(ctx, Null);
		True = JSValueMakeBoolean(ctx, true);    JSValueProtect(ctx, true);
		False = JSValueMakeBoolean(ctx, false);  JSValueProtect(ctx, false);
		Empty = JSValueMakeString(ctx, empty_s); JSValueProtect(ctx, Empty);
		// auto exports = runNativeScript(ctx, (const char*)
		// 																native_js::JSC_native_js_code_jsc_v8_isolate_,
		// 																"[jsc-v8-isolate.js]");
		#define _Init_Fun(name) name = (JSObjectRef) \
		JSObjectGetProperty(ctx, exports, name##_s, JsFatal(ctx)); \
		JSValueProtect(ctx, name);
		Js_Worker_Data_List(_Fun)
		#undef _Fun
	}

	void GlobalData::destroy(JSGlobalContextRef ctx) {
		JSValueUnprotect(ctx, Undefined);
		JSValueUnprotect(ctx, Null);
		JSValueUnprotect(ctx, True);
		JSValueUnprotect(ctx, False);
		JSValueUnprotect(ctx, Empty_s);
		#define _Des_Fun(name) JSValueUnprotect(ctx, name);
		Js_Worker_Data_List(_Des_Fun)
	}

	void ContextData::initialize(JSGlobalContextRef ctx) {
		JSValueRef ex = nullptr;
		// auto exports = runNativeScript(ctx, (const char*)
		// 																native_js::JSC_native_js_code_jsc_v8_context_,
		// 																"[jsc-v8-context.js]");
		Js_Context_Data_List(_Init_Fun)
	}
	void ContextData::destroy(JSGlobalContextRef ctx) {
		Js_Context_Data_List(_Des_Fun)
	}
	#undef _Init_Fun
	#undef _Des_Fun

	JscWorker::JscWorker()
		: _exception(nullptr)
		, _try(nullptr)
		, _scope(nullptr)
		, _messageListener(nullptr)
		, _hasTerminated(false)
		, _hasDestroy(false)
	{
		uv_key_set(&th_key, this);
		first_worker = workers++ ? nullptr: this;

		_group = JSContextGroupCreate();
		_ctx = JSGlobalContextCreateInGroup(_group, nullptr);

		// JSObjectRef global = (JSObjectRef)JSContextGetGlobalObject(_ctx);
		// v8::HandleScope scope(reinterpret_cast<v8::Isolate*>(this));
		_globalData.initialize(_ctx);
		_ctxData.initialize(_ctx);
		_templates.initialize(_ctx);
	}

	void JscWorker::release() {
		Worker::release();
		_hasTerminated = true;
		_exception = nullptr;
		_templates.destroy(_ctx);
		_globalData.destroy(_ctx);
		_ctxData.destroy(_ctx);
		_hasDestroy = true;
		JSGlobalContextRelease(_ctx);
		JSContextGroupRelease(_group);

		if (first_worker == this)
			first_worker = nullptr;
		workers--;
		uv_key_set(&th_key, nullptr);
	}

	JSObjectRef JscWorker::newError(cChar* message) {
		auto str = JSValueMakeString(_ctx, JSStringCreateWithUTF8CString(message));
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

	bool JSValue::isUndefined() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsUndefined();
	}
	bool JSValue::isNull() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsNull();
	}
	bool JSValue::isString() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsString();
	}
	bool JSValue::isBoolean() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsBoolean();
	}
	bool JSValue::isObject() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsObject();
	}
	bool JSValue::isArray() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsArray();
	}
	bool JSValue::isDate() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsDate();
	}
	bool JSValue::isNumber() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsNumber();
	}
	bool JSValue::isUint32() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsUint32();
	}
	bool JSValue::isInt32() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsInt32();
	}
	bool JSValue::isFunction() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsFunction();
	}
	bool JSValue::isArrayBuffer() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsArrayBuffer();
	}
	bool JSValue::isTypedArray() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsTypedArray();
	}
	bool JSValue::isUint8Array() const {
		// return reinterpret_cast<const v8::Value*>(this)->IsUint8Array();
	}
	bool JSValue::equals(Worker *worker, JSValue* val) const {
		// return reinterpret_cast<const v8::Value*>(this)->Equals(CONTEXT(worker), Back(val)).ToChecked();
	}
	bool JSValue::strictEquals(JSValue* val) const {
		// return reinterpret_cast<const v8::Value*>(this)->StrictEquals(Back(val));
	}

	bool JSValue::instanceOf(Worker* worker, JSObject* value) {
		// return reinterpret_cast<v8::Value*>(this)->
		// 	InstanceOf(CONTEXT(worker), Back<v8::Object>(value)).FromMaybe(false);
	}

	JSString* JSValue::toString(Worker* worker) const {
		// return Cast<JSString>(reinterpret_cast<const v8::Value*>(this)->ToString(CONTEXT(worker)));
	}

	JSNumber* JSValue::toNumber(Worker* worker) const {
		// Cast<JSNumber>(reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker)));
	}

	JSInt32* JSValue::toInt32(Worker* worker) const {
		// return Cast<JSInt32>(reinterpret_cast<const v8::Value*>(this)->ToInt32(CONTEXT(worker)));
	}

	JSUint32* JSValue::toUint32(Worker* worker) const {
		// return Cast<JSUint32>(reinterpret_cast<const v8::Value*>(this)->ToUint32(CONTEXT(worker)));
	}

	// JSObject* JSValue::asObject(Worker* worker) const {
		// return Cast<JSObject>(reinterpret_cast<const v8::Value*>(this)->ToObject(CONTEXT(worker)));
	// }

	JSBoolean* JSValue::toBoolean(Worker* worker) const {
		//return Cast<JSBoolean>(reinterpret_cast<const v8::Value*>(this)->ToBoolean(CONTEXT(worker)));
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

	Maybe<float> JSValue::toFloatValue(Worker* worker) const {
		// auto v = reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker));
		// return v.IsEmpty() ? Maybe<float>(): Maybe<float>(Cast<JSNumber>(v)->value());
	}

	Maybe<double> JSValue::toNumberValue(Worker* worker) const {
		// auto v = reinterpret_cast<const v8::Value*>(this)->ToNumber(CONTEXT(worker));
		// return v.IsEmpty() ? Maybe<double>(): Maybe<double>(Cast<JSNumber>(v)->value());
	}

	Maybe<int> JSValue::toInt32Value(Worker* worker) const {
		// auto v = reinterpret_cast<const v8::Value*>(this)->ToInt32(CONTEXT(worker));
		// return v.IsEmpty() ? Maybe<int>(): Maybe<int>(Cast<JSInt32>(v)->value());
	}

	Maybe<uint32_t> JSValue::toUint32Value(Worker* worker) const {
		// auto v = reinterpret_cast<const v8::Value*>(this)->ToUint32(CONTEXT(worker));
		// return v.IsEmpty() ? Maybe<uint32_t>(): Maybe<uint32_t>(Cast<JSUint32>(v)->value());
	}

	bool JSValue::toBooleanValue(Worker* worker) const {
		// return reinterpret_cast<const v8::Value*>(this)->ToBoolean(CONTEXT(worker)).ToLocalChecked()->Value();
	}

	JSValue* JSObject::get(Worker* worker, JSValue* key) {
		// return Cast(reinterpret_cast<v8::Object*>(this)->Get(CONTEXT(worker), Back(key)));
	}

	JSValue* JSObject::get(Worker* worker, uint32_t index) {
		// return Cast(reinterpret_cast<v8::Object*>(this)->Get(CONTEXT(worker), index));
	}

	bool JSObject::set(Worker* worker, JSValue* key, JSValue* val) {
		// return reinterpret_cast<v8::Object*>(this)->
		// 	Set(CONTEXT(worker), Back(key), Back(val)).FromMaybe(false);
	}

	bool JSObject::set(Worker* worker, uint32_t index, JSValue* val) {
		// return reinterpret_cast<v8::Object*>(this)->
		// 	Set(CONTEXT(worker), index, Back(val)).FromMaybe(false);
	}

	bool JSObject::has(Worker* worker, JSValue* key) {
		// return reinterpret_cast<v8::Object*>(this)->
		// 	Has(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::has(Worker* worker, uint32_t index) {
		// return reinterpret_cast<v8::Object*>(this)->
		// 	Has(CONTEXT(worker), index).FromMaybe(false);
	}

	bool JSObject::JSObject::deleteFor(Worker* worker, JSValue* key) {
		// return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), Back(key)).FromMaybe(false);
	}

	bool JSObject::deleteFor(Worker* worker, uint32_t index) {
		// return reinterpret_cast<v8::Object*>(this)->Delete(CONTEXT(worker), index).FromMaybe(false);
	}

	JSArray* JSObject::getPropertyNames(Worker* worker) {
		// return Cast<JSArray>(reinterpret_cast<v8::Object*>(this)->
		// 										GetPropertyNames(CONTEXT(worker)).FromMaybe(v8::Local<v8::Array>()));
	}

	JSFunction* JSObject::getConstructor(Worker* worker) {
		// auto rv = reinterpret_cast<v8::Object*>(this)->
		// 	Get(CONTEXT(worker), Back(worker->strs()->constructor()));
		// return Cast<JSFunction>(rv.FromMaybe(v8::Local<v8::Value>()));
	}

	bool JSObject::setMethod(Worker* worker, cString& name, FunctionCallback func) {
		// v8::FunctionCallback func2 = reinterpret_cast<v8::FunctionCallback>(func);
		// v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(ISOLATE(worker), func2);
		// v8::Local<v8::Function> fn = t->GetFunction(CONTEXT(worker)).ToLocalChecked();
		// v8::Local<v8::String> fn_name = Back<v8::String>(worker->newStringOneByte(name));
		// fn->SetName(fn_name);
		// return reinterpret_cast<v8::Object*>(this)->
		// 	Set(CONTEXT(worker), fn_name, fn).FromMaybe(false);
	}

	bool JSObject::setAccessor(Worker* worker, cString& name,
														AccessorGetterCallback get, AccessorSetterCallback set) {
		// auto get2 = reinterpret_cast<v8::AccessorNameGetterCallback>(get);
		// auto set2 = reinterpret_cast<v8::AccessorNameSetterCallback>(set);
		// v8::Local<v8::String> fn_name = Back<v8::String>(worker->newStringOneByte(name));
		// return reinterpret_cast<v8::Object*>(this)->SetAccessor(CONTEXT(worker), fn_name, get2, set2).ToChecked();
	}

	bool JSObject::defineOwnProperty(Worker *worker, JSValue *key, JSValue *value, int flags) {
		// auto name = v8::Name::Cast(reinterpret_cast<v8::Value*>(key));
		// auto name_ = *reinterpret_cast<v8::Local<v8::Name>*>(&name);
		// return reinterpret_cast<v8::Object*>(this)->
		// 	DefineOwnProperty(CONTEXT(worker), name_, Back(value), v8::PropertyAttribute(flags))
		// 	.FromMaybe(false);
	}

	void* JSObject::getObjectPrivate() {
		// auto self = reinterpret_cast<v8::Object*>(this);
		// if (self->InternalFieldCount() > 0) {
		// 	return self->GetAlignedPointerFromInternalField(0);
		// }
		// return nullptr;
	}

	bool JSObject::setObjectPrivate(void *value) {
		// auto self = reinterpret_cast<v8::Object*>(this);
		// if (self->InternalFieldCount() > 0) {
		// 	self->SetAlignedPointerInInternalField(0, value);
		// 	return true;
		// }
		// return false;
	}

	bool JSObject::setPrototype(Worker* worker, JSObject* __proto__) {
		// return reinterpret_cast<v8::Object*>(this)->
		// 	SetPrototype(CONTEXT(worker), Back(__proto__)).FromMaybe(false);
	}

	int JSString::length() const {
		// return reinterpret_cast<const v8::String*>(this)->Length();
	}
	JSString* JSString::Empty(Worker* worker) {
		// return Cast<JSString>(v8::String::Empty(ISOLATE(worker)));
	}
	int JSArray::length() const {
		// return reinterpret_cast<const v8::Array*>(this)->Length();
	}
	double JSDate::valueOf() const {
		// return reinterpret_cast<const v8::Date*>(this)->ValueOf();
	}
	double JSNumber::value() const {
		// return reinterpret_cast<const v8::Number*>(this)->Value();
	}
	int JSInt32::value() const {
		// return reinterpret_cast<const v8::Int32*>(this)->Value();
	}
	int64_t JSInteger::value() const {
		// return reinterpret_cast<const v8::Integer*>(this)->Value();
	}
	uint32_t JSUint32::value() const {
		// return reinterpret_cast<const v8::Uint32*>(this)->Value();
	}
	bool JSBoolean::value() const {
		// return reinterpret_cast<const v8::Boolean*>(this)->Value();
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

	void ReturnValue::set(bool value) {
		// reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(value);
	}

	void ReturnValue::set(double i) {
		// reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::set(int i) {
		// reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::set(uint32_t i) {
		// reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(i);
	}

	void ReturnValue::setNull() {
		// reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetNull();
	}

	void ReturnValue::setUndefined() {
		// reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetUndefined();
	}

	void ReturnValue::setEmptyString() {
		// reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->SetEmptyString();
	}

	void ReturnValue::set(JSValue* value) {
		// if (value)
		// 	reinterpret_cast<v8::ReturnValue<v8::Value>*>(this)->Set(Back(value));
		// else
		// 	setNull();
	}

	int FunctionCallbackInfo::length() const {
		// return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->Length();
	}

	JSValue* FunctionCallbackInfo::operator[](int i) const {
		// return Cast(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->operator[](i));
	}

	JSObject* FunctionCallbackInfo::This() const {
		// return Cast<JSObject>(reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->This());
	}

	bool FunctionCallbackInfo::isConstructCall() const {
		// return reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this)->IsConstructCall();
	}

	ReturnValue FunctionCallbackInfo::returnValue() const {
		// auto info = reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this);
		// v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		// auto _ = reinterpret_cast<ReturnValue*>(&rv);
		// return *_;
	}

	JSObject* PropertyCallbackInfo::This() const {
		// return Cast<JSObject>(reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this)->This());
	}

	ReturnValue PropertyCallbackInfo::returnValue() const {
		// auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
		// v8::ReturnValue<v8::Value> rv = info->GetReturnValue();
		// auto _ = reinterpret_cast<ReturnValue*>(&rv);
		// return *_;
	}

	JSObject* PropertySetCallbackInfo::This() const {
		// return Cast<JSObject>(reinterpret_cast<const v8::PropertyCallbackInfo<void>*>(this)->This());
	}

	Worker* FunctionCallbackInfo::worker() const {
		// auto info = reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(this);
		// return WorkerImpl::worker(info->GetIsolate());
	}

	Worker* PropertyCallbackInfo::worker() const {
		// auto info = reinterpret_cast<const v8::PropertyCallbackInfo<v8::Value>*>(this);
		// return WorkerImpl::worker(info->GetIsolate());
	}

	Worker* PropertySetCallbackInfo::worker() const {
		// auto info = reinterpret_cast<const v8::PropertyCallbackInfo<void>*>(this);
		// return WorkerImpl::worker(info->GetIsolate());
	}

	TryCatch::TryCatch(Worker *worker) {
		// _val = new TryCatchMix(worker);
	}

	TryCatch::~TryCatch() {
		// delete reinterpret_cast<TryCatchMix*>(_val);
		// _val = nullptr;
	}

	bool TryCatch::hasCaught() const {
		// return reinterpret_cast<TryCatchMix*>(_val)->_try.HasCaught();
	}

	Worker* WeakCallbackInfo::worker() const {
		// auto info = reinterpret_cast<const v8::WeakCallbackInfo<Object>*>(this);
		// return WorkerImpl::worker(info->GetIsolate());
	}

	void* WeakCallbackInfo::getParameter() const {
		// return reinterpret_cast<const v8::WeakCallbackInfo<void>*>(this)->GetParameter();
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

	template <>
	bool Persistent<JSValue>::isWeak() {
		// Qk_Assert( _val );
		// auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(this);
		// return h->IsWeak();
	}

	template <>
	void Persistent<JSValue>::setWeak(void* ptr, WeakCallback callback) {
		// Qk_Assert( !isEmpty() );
		// auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(this);
		// h->MarkIndependent();
		// h->SetWeak(ptr, reinterpret_cast<v8::WeakCallbackInfo<void>::Callback>(callback),
		// 					v8::WeakCallbackType::kParameter);
	}

	template <>
	void Persistent<JSValue>::clearWeak() {
		// Qk_Assert( !isEmpty() );
		// auto h = reinterpret_cast<v8::PersistentBase<v8::Value>*>(this);
		// h->ClearWeak();
	}

	template<>
	JSValue* Persistent<JSValue>::operator*() const {
		// // return Cast(Local<Value>::New(ISOLATE(_worker), Back(_val)));
		// return _val;
	}

	JSNumber* Worker::newValue(float data) {
		// return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newValue(double data) {
		// return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSBoolean* Worker::newBool(bool data) {
		// return Cast<JSBoolean>(v8::Boolean::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newValue(Char data) {
		// return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newValue(uint8_t data) {
		// return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newValue(int16_t data) {
		// return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newValue(uint16_t data) {
		// return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSInt32* Worker::newValue(int data) {
		// return Cast<JSInt32>(v8::Int32::New(ISOLATE(this), data));
	}

	JSUint32* Worker::newValue(uint32_t data) {
		// return Cast<JSUint32>(v8::Uint32::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newValue(int64_t data) {
		// return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSNumber* Worker::newValue(uint64_t data) {
		// return Cast<JSNumber>(v8::Number::New(ISOLATE(this), data));
	}

	JSString* Worker::newString(cBuffer& data) {
		// return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									// v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newValue(cString& data) {
		// return Cast<JSString>(v8::String::NewFromUtf8(ISOLATE(this), *data,
																									// v8::String::kNormalString, data.length()));
	}

	JSString* Worker::newValue(cString2& data) {
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
		// return Cast(v8::Null(ISOLATE(this)));
	}

	JSValue* Worker::newUndefined() {
		// return Cast(v8::Undefined(ISOLATE(this)));
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
	};
} }
