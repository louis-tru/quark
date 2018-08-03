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


// --- V a l u e ---

bool Value::FullIsUndefined() const {
	return JSValueIsUndefined(JSC_CTX(), i::Back(this));
}

bool Value::FullIsNull() const {
	return JSValueIsNull(JSC_CTX(), i::Back(this));
}

bool Value::IsTrue() const {
	return JSValueToBoolean(JSC_CTX(), i::Back(this));
}

bool Value::IsFalse() const {
	return !JSValueToBoolean(JSC_CTX(), i::Back(this));
}

bool Value::IsFunction() const {
	return JSValueIsObject(JSC_CTX(), i::Back(this)) &&
				 JSObjectIsFunction(JSC_CTX(), i::Back<JSObjectRef>(this));
}

bool Value::IsName() const {
	return JSValueIsString(JSC_CTX(), i::Back(this));
}

bool Value::FullIsString() const {
	return JSValueIsString(JSC_CTX(), i::Back(this));
}

bool Value::IsArray() const {
	return JSValueIsArray(JSC_CTX(), i::Back(this));
}

bool Value::IsObject() const {
	return JSValueIsObject(JSC_CTX(), i::Back(this));
}

bool Value::IsNumber() const {
	return JSValueIsNumber(JSC_CTX(), i::Back(this));
}

bool Value::IsDate() const {
	return JSValueIsDate(JSC_CTX(), i::Back(this));
}

bool Value::IsBoolean() const {
	return JSValueIsBoolean(JSC_CTX(), i::Back(this));
}

bool Value::IsInt32() const {
	ENV();
	JSValueRef val = i::Back(this);
	if ( JSValueIsNumber(ctx, val) ) {
		double num = JSValueToNumber(ctx, val, OK(false));
		return num >= std::numeric_limits<int>::min() &&
					 num <= std::numeric_limits<int>::max();
	} else {
		return false;
	}
}

bool Value::IsUint32() const {
	ENV();
	JSValueRef val = i::Back(this);
	if ( JSValueIsNumber(ctx, val) ) {
		double num = JSValueToNumber(ctx, val, OK(false));
		return num >= std::numeric_limits<uint32_t>::min() &&
					 num <= std::numeric_limits<uint32_t>::max();
	} else {
		return false;
	}
}

#define JS_TYPE_TABLE(F) \
F(ArrayBuffer) \
/*F(SharedArrayBuffer)*/ \
F(DataView) \
F(Proxy) \
F(Map) \
F(Set) \
F(WeakMap) \
F(WeakSet) \
F(RegExp) \
F(AsyncFunction) \
F(MapIterator) \
F(SetIterator) \
F(Promise) \
F(TypedArray) \
F(Uint8Array) \
F(Int8Array) \
F(Uint16Array) \
F(Int16Array) \
F(Uint32Array) \
F(Int32Array) \
F(Float32Array) \
F(Float64Array) \
F(Uint8ClampedArray) \

#define VALUE_IS_TYPE(Type) \
bool Value::Is##Type () const { \
	ENV(); \
	bool ok = JSValueIsInstanceOfConstructor(ctx, \
		i::Back(this), isolate->Type(), OK(false)); \
	return ok; \
}
JS_TYPE_TABLE(VALUE_IS_TYPE)
#undef VALUE_IS_TYPE

bool Value::IsSharedArrayBuffer() const {
	// ENV();
	// bool ok = JSValueIsInstanceOfConstructor(ctx,
	//   i::Back(this), isolate->Type(), OK(false));
	// return ok;
	//
	// TODO..
	// Discarded SharedArrayBuffer
	UNIMPLEMENTED();
	return false;
}

bool Value::IsExternal() const {
	ENV();
	auto value = i::Back<JSObjectRef>(this);
	if (JSValueIsObject(ctx, value)) {
		auto priv = i::PrivateData::Private(value);
		if (priv) {
			if (priv->InstanceTemplate() == isolate->ExternalTemplate()) {
				return true;
			} else {
				DLOG("No IsExternal");
			}
		} else {
			DLOG("No IsExternal, not priv");
		}
	} else {
		DLOG("No IsExternal, JSValueIsObject, %ld", size_t(value));
	}
	return false;
}

bool Value::IsBooleanObject() const {
	ENV();
	bool ok = JSValueIsInstanceOfConstructor(ctx, i::Back(this),
																					 isolate->Boolean(), OK(false));
	return ok;
}
bool Value::IsNumberObject() const {
	ENV();
	bool ok = JSValueIsInstanceOfConstructor(ctx, i::Back(this),
																					 isolate->Number(), OK(false));
	return ok;
}
bool Value::IsStringObject() const {
	ENV();
	bool ok = JSValueIsInstanceOfConstructor(ctx, i::Back(this),
																					 isolate->String(), OK(false));
	return ok;
}
bool Value::IsArgumentsObject() const { return false; }
bool Value::IsSymbolObject() const { return false; }
bool Value::IsWebAssemblyCompiledModule() const { return false; }
bool Value::IsNativeError() const { return false; }
bool Value::IsGeneratorFunction() const { return false; }
bool Value::IsGeneratorObject() const { return false; }

bool Value::IsSymbol() const {
	ENV();
	JSValueRef argv[1] = { i::Back(this) };
	auto ok = JSObjectCallAsFunction(ctx, isolate->isSymbol(), 0, 1, argv, OK(false));
	return JSValueToBoolean(ctx, ok);
}

bool Value::IsArrayBufferView() const {
	return IsTypedArray();
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
	ENV(context->GetIsolate());
	if (JSValueIsString(ctx, i::Back(this))) {
		return i::Cast<String>((JSValueRef)this);
	} else {
		auto s = JSValueToStringCopy(ctx, i::Back(this), OK(MaybeLocal<String>()));
		return i::Cast<String>(JSValueMakeString(ctx, s));
	}
}

Local<String> Value::ToString(Isolate* isolate) const {
	RETURN_TO_LOCAL_UNCHECKED(ToString(isolate->GetCurrentContext()), String);
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
	return ToString(context);
}

Local<String> Value::ToDetailString(Isolate* isolate) const {
	RETURN_TO_LOCAL_UNCHECKED(ToDetailString(isolate->GetCurrentContext()), String);
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
	ENV(context->GetIsolate());
	auto r = JSValueToObject(ctx, i::Back(this), OK(MaybeLocal<Object>()));
	return i::Cast<Object>(r);
}

Local<v8::Object> Value::ToObject(Isolate* isolate) const {
	RETURN_TO_LOCAL_UNCHECKED(ToObject(isolate->GetCurrentContext()), Object);
}

MaybeLocal<Boolean> Value::ToBoolean(Local<Context> context) const {
	ENV(context->GetIsolate());
	return JSValueToBoolean(ctx, i::Back(this)) ?
				 True(reinterpret_cast<Isolate*>(isolate)) :
				 False(reinterpret_cast<Isolate*>(isolate));
}

Local<Boolean> Value::ToBoolean(Isolate* v8_isolate) const {
	return ToBoolean(v8_isolate->GetCurrentContext()).ToLocalChecked();
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
	ENV(context->GetIsolate());
	if ( JSValueIsNumber(ctx, i::Back(this)) ) {
		return i::Cast<Number>(i::Back(this));
	} else {
		double v = JSValueToNumber(ctx, i::Back(this), OK(MaybeLocal<Number>()));
		return i::Cast<Number>(JSValueMakeNumber(ctx, v));
	}
}

Local<Number> Value::ToNumber(Isolate* isolate) const {
	RETURN_TO_LOCAL_UNCHECKED(ToNumber(isolate->GetCurrentContext()), Number);
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
	ENV(context->GetIsolate());
	double num = JSValueToNumber(ctx, i::Back(this), OK(Local<Integer>()));
	return i::Cast<Integer>(JSValueMakeNumber(ctx, (int64_t)num));
}

Local<Integer> Value::ToInteger(Isolate* isolate) const {
	RETURN_TO_LOCAL_UNCHECKED(ToInteger(isolate->GetCurrentContext()), Integer);
}

MaybeLocal<Int32> Value::ToInt32(Local<Context> context) const {
	ENV(context->GetIsolate());
	double num = JSValueToNumber(ctx, i::Back(this), OK(Local<Int32>()));
	return i::Cast<Int32>(JSValueMakeNumber(ctx, (int)num));
}

Local<Int32> Value::ToInt32(Isolate* isolate) const {
	RETURN_TO_LOCAL_UNCHECKED(ToInt32(isolate->GetCurrentContext()), Int32);
}

MaybeLocal<Uint32> Value::ToUint32(Local<Context> context) const {
	ENV(context->GetIsolate());
	double num = JSValueToNumber(ctx, i::Back(this), OK(Local<Uint32>()));
	return i::Cast<Uint32>(JSValueMakeNumber(ctx, (uint32_t)num));
}

Local<Uint32> Value::ToUint32(Isolate* isolate) const {
	RETURN_TO_LOCAL_UNCHECKED(ToUint32(isolate->GetCurrentContext()), Uint32);
}

Maybe<bool> Value::BooleanValue(Local<Context> context) const {
	return Just(JSValueToBoolean(JSC_CTX(context->GetIsolate()), i::Back(this)));
}

bool Value::BooleanValue() const {
	return BooleanValue(Isolate::GetCurrent()->GetCurrentContext()).FromJust();
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
	ENV(context->GetIsolate());
	double r = JSValueToNumber(ctx, i::Back(this), OK(Nothing<double>()));
	return Just(r);
}

double Value::NumberValue() const {
	return NumberValue(Isolate::GetCurrent()->GetCurrentContext())
				.FromMaybe(std::numeric_limits<double>::quiet_NaN());
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
	ENV(context->GetIsolate());
	int64_t r = JSValueToNumber(ctx, i::Back(this), OK(Nothing<int64_t>()));
	return Just(r);
}

int64_t Value::IntegerValue() const {
	return IntegerValue(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(0);
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
	ENV(context->GetIsolate());
	int32_t r = JSValueToNumber(ctx, i::Back(this), OK(Nothing<int32_t>()));
	return Just(r);
}

int32_t Value::Int32Value() const {
	return Int32Value(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(0);
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
	ENV(context->GetIsolate());
	uint32_t r = JSValueToNumber(ctx, i::Back(this), OK(Nothing<uint32_t>()));
	return Just(r);
}

uint32_t Value::Uint32Value() const {
	return Uint32Value(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(0);
}

MaybeLocal<Uint32> Value::ToArrayIndex(Local<Context> context) const {
	ENV(context->GetIsolate());
	MaybeLocal<Uint32> err;
	i::JSCStringPtr key = JSValueToStringCopy(ctx, i::Back(this), OK(err));
	uint32_t out;
	if (i::Isolate::PropertyNameToIndexed(*key, out)) {
		auto num = JSValueMakeNumber(ctx, out);
		return i::Cast<Uint32>(num);
	}
	return err;
}

Local<Uint32> Value::ToArrayIndex() const {
	RETURN_TO_LOCAL_UNCHECKED(ToArrayIndex(Isolate::GetCurrent()->GetCurrentContext()), Uint32);
}

Maybe<bool> Value::Equals(Local<Context> context, Local<Value> that) const {
	ENV(context->GetIsolate());
	bool r = JSValueIsEqual(ctx, i::Back(this), i::Back(that), OK(Nothing<bool>()));
	return Just(r);
}

bool Value::Equals(Local<Value> that) const {
	return Equals(Isolate::GetCurrent()->GetCurrentContext(), that).FromMaybe(false);
}

bool Value::StrictEquals(Local<Value> that) const {
	ENV();
	return JSValueIsStrictEqual(ctx, i::Back(this), i::Back(that));
}

bool Value::SameValue(Local<Value> that) const {
	ENV();
	// UNIMPLEMENTED();
	return JSValueIsStrictEqual(ctx, i::Back(this), i::Back(that));
}

Local<String> Value::TypeOf(v8::Isolate* external_isolate) {
	ENV(external_isolate);
	switch(JSValueGetType(ctx, i::Back(this))) {
		case kJSTypeUndefined:  return i::Cast<String>(JSValueMakeString(ctx, i::undefined_s));
		case kJSTypeNull:       return i::Cast<String>(JSValueMakeString(ctx, i::null_s));
		case kJSTypeBoolean:    return i::Cast<String>(JSValueMakeString(ctx, i::boolean_s));
		case kJSTypeNumber:     return i::Cast<String>(JSValueMakeString(ctx, i::number_s));
		case kJSTypeString:     return i::Cast<String>(JSValueMakeString(ctx, i::string_s));
		case kJSTypeObject:     return i::Cast<String>(JSValueMakeString(ctx, i::object_s));
		default: {
			JSValueRef argv[1] = { i::Back(this) };
			auto s = JSObjectCallAsFunction(ctx, isolate->typeOf(), nullptr, 1, argv,
																			OK(Local<String>()));
			return i::Cast<String>(s);
		}
	}
}

Maybe<bool> Value::InstanceOf(v8::Local<v8::Context> context,
															v8::Local<v8::Object> object) {
	ENV(context->GetIsolate());
	bool ok = JSValueIsInstanceOfConstructor(ctx,
																					 i::Back(this),
																					 i::Back<JSObjectRef>(object),
																					 OK(Nothing<bool>())
																				 );
	return Just(ok);
}

// --- D e r i v e   V a l u e ---

static int get_hash_code(const uint16_t* data, int len) {
	int hash = 5381;
	while (len--) hash += (hash << 5) + data[len];
}

Local<External> v8::External::New(Isolate* iso, void* value) {
	ENV(iso);
	auto r = isolate->ExternalTemplate()->NewInstance();
	auto priv = (i::PrivateData*)JSObjectGetPrivate(r);
	DCHECK(priv);
	i::EmbedderData* data = priv->InternalField(0);
	data->SetAlignedPointer(ctx, value);
	return i::Cast<External>(r);
}

void* v8::External::Value() const {
	ENV();
	auto self = i::Back<JSObjectRef>(this);
	auto priv = (i::PrivateData*)JSObjectGetPrivate(self);
	DCHECK(priv);
	return priv->InternalField(0)->GetAlignedPointer();
}

int Name::GetIdentityHash() {
	ENV();
	i::JSCStringPtr str = JSValueToStringCopy(ctx, i::Back(this), OK(0));
	return get_hash_code(JSStringGetCharactersPtr(*str), (int)JSStringGetLength(*str));
}

Local<Value> Symbol::Name() const {
	ENV();
	JSValueRef argv[1] = { i::Back(this) };
	auto r = JSObjectCallAsFunction(ctx, isolate->symbolName(), 0, 1, argv, OK(Local<Value>()));
	return i::Cast(r);
}

Local<Value> Private::Name() const {
	auto priv = reinterpret_cast<const i::Private*>(this);
	ENV(priv->GetIsolate());
	auto r = JSValueMakeString(ctx, priv->Name());
	return i::Cast(r);
}

double Number::Value() const {
	return NumberValue();
}

bool Boolean::Value() const {
	return BooleanValue();
}

int64_t Integer::Value() const {
	return IntegerValue();
}

int32_t Int32::Value() const {
	return Int32Value();
}

uint32_t Uint32::Value() const {
	return Uint32Value();
}

MaybeLocal<v8::Value> v8::Date::New(Local<Context> context, double time) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { JSValueMakeNumber(ctx, time) };
	auto r = JSObjectCallAsConstructor(ctx, isolate->Date(), 1, argv, OK(MaybeLocal<v8::Value>()));
	return i::Cast(r);
}

Local<v8::Value> v8::Date::New(Isolate* isolate, double time) {
	auto context = isolate->GetCurrentContext();
	RETURN_TO_LOCAL_UNCHECKED(New(context, time), Value);
}

double v8::Date::ValueOf() const {
	ENV();
	double err = std::numeric_limits<double>::quiet_NaN();
	JSValueRef argv[1] = { i::Back(this) };
	auto r = JSObjectCallAsFunction(ctx, isolate->valueOf(), nullptr, 1, argv, OK(err));
	auto num = JSValueToNumber(ctx, r, OK(err));
	return num;
}

void v8::Date::DateTimeConfigurationChangeNotification(Isolate* isolate) {
}

MaybeLocal<v8::RegExp> v8::RegExp::New(Local<Context> context,
																			 Local<String> pattern, Flags flags) {
	ENV(context->GetIsolate());
	MaybeLocal<v8::RegExp> err;
	JSValueRef argv[2] = { i::Back(pattern), JSValueMakeNumber(ctx, flags) };
	auto r = JSObjectCallAsFunction(ctx, isolate->newRegExp(), nullptr, 2, argv, OK(err));
	return i::Cast<RegExp>(r);
}

Local<v8::RegExp> v8::RegExp::New(Local<String> pattern, Flags flags) {
	auto context = ISOLATE()->GetCurrentContext();
	RETURN_TO_LOCAL_UNCHECKED(New(context, pattern, flags), RegExp);
}

Local<v8::String> v8::RegExp::GetSource() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::source_s,
															 OK(Local<v8::String>()));
	return i::Cast<String>(r);
}

v8::RegExp::Flags v8::RegExp::GetFlags() const {
	ENV();
	int r = kNone;
	auto str = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::flags_s, OK(kNone));
	i::JSCStringPtr flags = JSValueToStringCopy(ctx, str, OK(kNone));
	char flags_s[8];
	auto size = JSStringGetUTF8CString(*flags, flags_s, 8);
	for (int i = 0; i < size; i++) {
		switch(flags_s[i]) {
			case 'i': r |= kIgnoreCase; break;
			case 'm': r |= kMultiline; break;
			case 'g': r |= kGlobal; break;
			case 'y': r |= kSticky; break;
			case 'u': r |= kUnicode; break;
		}
	}
	return Flags(r);
}

Local<v8::Array> v8::Array::New(Isolate* iso, int length) {
	ENV(iso);
	JSValueRef argv[1] = { JSValueMakeNumber(ctx, length) };
	auto r = JSObjectMakeArray(ctx, 1, argv, OK(Local<v8::Array>()));
	return i::Cast<Array>(r);
}

uint32_t v8::Array::Length() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::length_s, OK(0));
	auto num = JSValueToNumber(ctx, r, OK(0));
	return num;
}

MaybeLocal<Object> Array::CloneElementAt(Local<Context> context, uint32_t index) {
	UNIMPLEMENTED();
}

Local<Object> Array::CloneElementAt(uint32_t index) {
	UNIMPLEMENTED();
}

Local<v8::Map> v8::Map::New(Isolate* iso) {
	ENV(iso);
	auto r = JSObjectCallAsConstructor(ctx, isolate->Map(), 0, 0, OK(Local<v8::Map>()));
	return i::Cast<Map>(r);
}

size_t v8::Map::Size() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::size_s, OK(0));
	auto num = JSValueToNumber(ctx, r, OK(0));
	return num;
}

void Map::Clear() {
	ENV();
	JSObjectCallAsFunction(ctx, isolate->mapClear(), i::Back<JSObjectRef>(this), 0, 0, OK());
}

MaybeLocal<Value> Map::Get(Local<Context> context, Local<Value> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->mapGet(), i::Back<JSObjectRef>(this), 1, argv,
																	OK(MaybeLocal<Value>()));
	return i::Cast(r);
}

MaybeLocal<Map> Map::Set(Local<Context> context, Local<Value> key,
												 Local<Value> value) {
	ENV(context->GetIsolate());
	JSValueRef argv[2] = { i::Back(key), i::Back(value) };
	auto r = JSObjectCallAsFunction(ctx, isolate->mapSet(), i::Back<JSObjectRef>(this), 2, argv,
																	OK(MaybeLocal<Map>()));
	return i::Cast<Map>(r);
}

Maybe<bool> Map::Has(Local<Context> context, Local<Value> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->mapHas(), i::Back<JSObjectRef>(this), 1, argv,
																	NOTHING);
	auto has = JSValueToBoolean(ctx, r);
	return Just(has);
}

Maybe<bool> Map::Delete(Local<Context> context, Local<Value> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->mapDelete(), i::Back<JSObjectRef>(this), 1, argv,
																	NOTHING);
	auto has = JSValueToBoolean(ctx, r);
	return Just(has);
}

Local<Array> Map::AsArray() const {
	ENV();
	JSValueRef argv[1] = { i::Back(this) };
	auto r = JSObjectCallAsFunction(ctx, isolate->mapAsArray(), 0, 1, argv, OK(Local<Array>()));
	return i::Cast<Array>(r);
}

Local<v8::Set> v8::Set::New(Isolate* iso) {
	ENV(iso);
	auto r = JSObjectCallAsConstructor(ctx, isolate->Set(), 0, 0, OK(Local<Set>()));
	return i::Cast<Set>(r);
}

size_t v8::Set::Size() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::size_s, OK(0));
	auto num = JSValueToNumber(ctx, r, OK(0));
	return num;
}

void Set::Clear() {
	ENV();
	JSObjectCallAsFunction(ctx, isolate->setClear(), i::Back<JSObjectRef>(this), 0, 0, OK());
}

MaybeLocal<Set> Set::Add(Local<Context> context, Local<Value> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->setAdd(), i::Back<JSObjectRef>(this), 1, argv,
																	OK(MaybeLocal<Set>()));
	return i::Cast<Set>(r);
}

Maybe<bool> Set::Has(Local<Context> context, Local<Value> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->setHas(), i::Back<JSObjectRef>(this), 1, argv,
																	NOTHING);
	auto has = JSValueToBoolean(ctx, r);
	return Just(has);
}

Maybe<bool> Set::Delete(Local<Context> context, Local<Value> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->setDelete(), i::Back<JSObjectRef>(this), 1, argv,
																	NOTHING);
	auto has = JSValueToBoolean(ctx, r);
	return Just(has);
}

Local<Array> Set::AsArray() const {
	ENV();
	JSValueRef argv[1] = { i::Back(this) };
	auto r = JSObjectCallAsFunction(ctx, isolate->setAsArray(), 0, 1, argv, OK(Local<Array>()));
	return i::Cast<Array>(r);
}

struct JSProxy {
	void* p0;
	void* p1;
	JSObjectRef target;
	JSObjectRef handle;
};

Local<Object> Proxy::GetTarget() {
	auto self = reinterpret_cast<JSProxy*>(this);
	return i::Cast<Object>(self->target);
}

Local<Value> Proxy::GetHandler() {
	auto self = reinterpret_cast<JSProxy*>(this);
	return i::Cast(self->handle);
}

bool Proxy::IsRevoked() {
	ENV();
	auto self = reinterpret_cast<JSProxy*>(this);
	return self->handle == isolate->Null(); // isNull
}

void Proxy::Revoke() {
	UNIMPLEMENTED();
}

MaybeLocal<Proxy> Proxy::New(Local<Context> context,
														 Local<Object> local_target,
														 Local<Object> local_handler) {
	ENV(context->GetIsolate());
	JSValueRef argv[2] = { i::Back(local_target), i::Back(local_handler) };
	auto r = JSObjectCallAsConstructor(ctx, isolate->Proxy(), 2, argv, OK(MaybeLocal<Proxy>()));
	return i::Cast<Proxy>(r);
}

Local<Symbol> v8::Symbol::New(Isolate* iso, Local<String> name) {
	ENV(iso);
	JSValueRef argv[1] = { i::Back(name) };
	auto r = JSObjectCallAsFunction(ctx, isolate->Symbol(), 0, 1, argv, OK(Local<Symbol>()));
	return i::Cast<Symbol>(r);
}

Local<Symbol> v8::Symbol::For(Isolate* iso, Local<String> name) {
	ENV(iso);
	JSValueRef argv[1] = { i::Back(name) };
	auto r = JSObjectCallAsFunction(ctx, isolate->symbolFor(), 0, 1, argv, OK(Local<Symbol>()));
	return i::Cast<Symbol>(r);
}

Local<Private> v8::Private::New(Isolate* iso, Local<String> name) {
	ENV(iso);
	auto priv = new i::Private(isolate, i::Back(name));
	auto p2 = i::Cast<Private>(priv);
	return p2;
}

Local<Symbol> v8::Symbol::ForApi(Isolate* iso, Local<String> name) {
	ENV(iso);
	Local<Symbol> err;
	i::JSCStringPtr key = i::Isolate::ToJSString(isolate, name);
	if (JSObjectHasProperty(ctx, isolate->symbol_for_api(), *key)) {
		auto r = JSObjectGetProperty(ctx, isolate->symbol_for_api(), *key, OK(err));
		return i::Cast<Symbol>(r);
	} else {
		JSValueRef argv[1] = { i::Back(name) };
		auto r = JSObjectCallAsFunction(ctx, isolate->Symbol(), 0, 1, argv, OK(err));
		JSObjectSetProperty(ctx, isolate->symbol_for_api(), *key, r, 0, OK(err));
		return i::Cast<Symbol>(r);
	}
}

Local<Private> v8::Private::ForApi(Isolate* iso, Local<String> name) {
	ENV(iso);
	Local<Private> err;
	JSObjectRef obj = nullptr;
	i::JSCStringPtr key = i::Isolate::ToJSString(isolate, name);
	if (JSObjectHasProperty(ctx, isolate->private_for_api(), *key)) {
		auto r = (JSObjectRef)JSObjectGetProperty(ctx, isolate->private_for_api(), *key, OK(err));
		DCHECK(JSValueIsObject(ctx, obj));
		auto priv = (i::Private*)JSObjectGetPrivate(r);
		DCHECK(priv);
		return i::Cast<Private>(priv);
	} else {
		JSValueRef argv[1] = { i::Back(name) };
		auto priv = new i::Private(isolate, i::Back(name));
		JSObjectSetProperty(ctx, isolate->private_for_api(), *key, priv->Handle(), 0, OK(err));
		return i::Cast<Private>(priv);
	}
}

Local<Number> v8::Number::New(Isolate* isolate, double value) {
	return i::Cast<Integer>(JSValueMakeNumber(JSC_CTX(isolate), value));
}

Local<Integer> v8::Integer::New(Isolate* isolate, int32_t value) {
	return i::Cast<Integer>(JSValueMakeNumber(JSC_CTX(isolate), value));
}

Local<Integer> v8::Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
	return i::Cast<Integer>(JSValueMakeNumber(JSC_CTX(isolate), value));
}

