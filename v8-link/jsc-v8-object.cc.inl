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


// --- O b j e c t ---

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
														v8::Local<Value> key, v8::Local<Value> value) {
	ENV(context->GetIsolate());
	i::JSCStringPtr name = JSValueToStringCopy(ctx, i::Back(key), &ex);
	if (ex) { // 不能被转换为JSString
		ex = nullptr;
		JSValueRef argv[3] = { i::Back(this), i::Back(key), i::Back(value),  };
		JSObjectCallAsFunction(ctx, isolate->setProperty(), nullptr, 3, argv, NOTHING);
	} else {
		JSObjectSetProperty(ctx, i::Back<JSObjectRef>(this),
												*name, i::Back(value), 0, NOTHING);
	}
	return Just(true);
}

bool v8::Object::Set(v8::Local<Value> key, v8::Local<Value> value) {
	return Set(CONTEXT(), key, value).FromMaybe(false);
}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context, uint32_t index,
														v8::Local<Value> value) {
	ENV(context->GetIsolate());
	JSObjectSetPropertyAtIndex(ctx, i::Back<JSObjectRef>(this),
														 index, i::Back(value), NOTHING);
	return Just(true);
}

bool v8::Object::Set(uint32_t index, v8::Local<Value> value) {
	return Set(CONTEXT(), index, value).FromMaybe(false);
}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
																					 v8::Local<Name> key,
																					 v8::Local<Value> value) {
	return DefineOwnProperty(context, key, value);
}

Maybe<bool> v8::Object::CreateDataProperty(v8::Local<v8::Context> context,
																					 uint32_t index,
																					 v8::Local<Value> value) {
	ENV(context->GetIsolate());
	JSObjectSetPropertyAtIndex(ctx, i::Back<JSObjectRef>(this),
														 index, i::Back(value), NOTHING);
	return Just(true);
}

Maybe<bool> v8::Object::DefineOwnProperty(v8::Local<v8::Context> context,
																					v8::Local<Name> key,
																					v8::Local<Value> value,
																					v8::PropertyAttribute attributes) {
	ENV(context->GetIsolate());
	i::JSCStringPtr jkey = JSValueToStringCopy(ctx, i::Back(key), &ex);
	// TODO key 如果不能被转换为JSString暂时抛出异常
	CHECK(!ex);
	JSPropertyAttributes attrs = attributes << 1;
	JSObjectSetProperty(ctx, i::Back<JSObjectRef>(this), *jkey,
											i::Back(value), attrs, NOTHING);
	return Just(true);
}

Maybe<bool> v8::Object::DefineProperty(v8::Local<v8::Context> context,
																			 v8::Local<Name> key,
																			 PropertyDescriptor& descriptor) {
	ENV(context->GetIsolate());
	JSObjectRef desc = JSObjectMake(ctx, 0, NOTHING);
	JSObjectSetProperty(ctx, desc, i::enumerable_s,
											JSValueMakeBoolean(ctx, descriptor.enumerable()), 0, NOTHING);
	JSObjectSetProperty(ctx, desc, i::configurable_s,
											JSValueMakeBoolean(ctx, descriptor.configurable()), 0, NOTHING);
	if (!descriptor.value().IsEmpty()) {
		JSObjectSetProperty(ctx, desc, i::value_s, i::Back(descriptor.value()), 0, NOTHING);
	} else {
		JSObjectSetProperty(ctx, desc, i::writable_s,
												JSValueMakeBoolean(ctx, descriptor.writable()), 0, NOTHING);
		if (!descriptor.get().IsEmpty()) {
			JSObjectSetProperty(ctx, desc, i::get_s, i::Back(descriptor.get()), 0, NOTHING);
		}
		if (!descriptor.set().IsEmpty()) {
			JSObjectSetProperty(ctx, desc, i::get_s, i::Back(descriptor.set()), 0, NOTHING);
		}
	}
	JSValueRef args[3] = { i::Back(this), i::Back(key), desc, };
	JSObjectCallAsFunction(ctx, isolate->defineProperty(), 0, 3, args, NOTHING);
	return Just(true);
}

Maybe<bool> v8::Object::ForceSet(v8::Local<v8::Context> context,
																 v8::Local<Value> key, v8::Local<Value> value,
																 v8::PropertyAttribute attribs) {
	ENV(context->GetIsolate());
	i::JSCStringPtr jkey = JSValueToStringCopy(ctx, i::Back(key), &ex);
	// TODO key 如果不能被转换为JSString暂时抛出异常
	CHECK(!ex);
	JSPropertyAttributes attrs = attribs << 1;
	JSObjectSetProperty(ctx, i::Back<JSObjectRef>(this), *jkey,
											i::Back(value), attrs, NOTHING);
	return Just(true);
}

bool v8::Object::ForceSet(v8::Local<Value> key, v8::Local<Value> value,
													v8::PropertyAttribute attribs) {
	return ForceSet(CONTEXT(),
									key, value, attribs).FromMaybe(false);
}

Maybe<bool> v8::Object::SetPrivate(Local<Context> context,
																	 Local<Private> key, Local<Value> value) {
	ENV(context->GetIsolate());
	auto jkey = i::Private::PrivateValue(key);
	
	JSPropertyAttributes attrs = kJSPropertyAttributeReadOnly |
		kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
	JSObjectSetProperty(ctx, i::Back<JSObjectRef>(this), jkey,
											i::Back(value), attrs, NOTHING);
	return Just(true);
}

MaybeLocal<Value> v8::Object::Get(Local<v8::Context> context,
																	Local<Value> key) {
	ENV(context->GetIsolate());
	i::JSCStringPtr jkey = JSValueToStringCopy(ctx, i::Back(key), &ex);
	if (ex) {
		ex = nullptr;
		JSValueRef argv[2] = { i::Back(this), i::Back(key) };
		auto r = JSObjectCallAsFunction(ctx, isolate->getProperty(), nullptr, 2, argv,
																		OK(MaybeLocal<Value>()));
		return i::Cast(r);
	} else {
		auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), *jkey,
																 OK(MaybeLocal<Value>()));
		return i::Cast(r);
	}
}

Local<Value> v8::Object::Get(v8::Local<Value> key) {
	RETURN_TO_LOCAL_UNCHECKED(Get(CONTEXT(), key), Value);
}

MaybeLocal<Value> v8::Object::Get(Local<Context> context, uint32_t index) {
	ENV(context->GetIsolate());
	auto r = JSObjectGetPropertyAtIndex(ctx, i::Back<JSObjectRef>(this), index,
																			OK(MaybeLocal<Value>()));
	return i::Cast(r);
}

Local<Value> v8::Object::Get(uint32_t index) {
	RETURN_TO_LOCAL_UNCHECKED(Get(CONTEXT(), index), Value);
}

MaybeLocal<Value> v8::Object::GetPrivate(Local<Context> context,
																				 Local<Private> key) {
	ENV(context->GetIsolate());
	auto name = i::Private::PrivateValue(key);
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this),
															 name, OK(MaybeLocal<Value>()));
	return i::Cast(r);
}

Maybe<PropertyAttribute>
v8::Object::GetPropertyAttributes(Local<Context> context, Local<Value> key) {
#define VERIFY OK(Nothing<PropertyAttribute>())
	ENV(context->GetIsolate());
	JSValueRef argv[2] = { i::Back(this), i::Back(key) };
	auto r = (JSObjectRef)
	JSObjectCallAsFunction(ctx, isolate->getOwnPropertyDescriptor(), 0,
												 2, argv, OK(Nothing<PropertyAttribute>()));
	auto configurable = JSObjectGetProperty(ctx, r, i::configurable_s, VERIFY);
	auto enumerable = JSObjectGetProperty(ctx, r, i::enumerable_s, VERIFY);
	auto writable = JSObjectGetProperty(ctx, r, i::writable_s, VERIFY);
	
	int attrs = None;
	
	if (!JSValueToBoolean(ctx, writable)) {
		attrs |= ReadOnly;
	}
	if (!JSValueToBoolean(ctx, enumerable)) {
		attrs |= DontEnum;
	}
	if (!JSValueToBoolean(ctx, configurable)) {
		attrs |= DontDelete;
	}
	return Just(PropertyAttribute(attrs));
#undef VERIFY
}

PropertyAttribute v8::Object::GetPropertyAttributes(v8::Local<Value> key) {
	return GetPropertyAttributes(CONTEXT(), key).FromMaybe(None);
}

MaybeLocal<Value> v8::Object::GetOwnPropertyDescriptor(Local<Context> context,
																											 Local<Name> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[2] = { i::Back(this), i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->getOwnPropertyDescriptor(), 0,
												 2, argv, OK(MaybeLocal<Value>()));
	return i::Cast(r);
}

Local<Value> v8::Object::GetOwnPropertyDescriptor(Local<Name> key) {
	RETURN_TO_LOCAL_UNCHECKED(GetOwnPropertyDescriptor(CONTEXT(), key), Value);
}

Local<Value> v8::Object::GetPrototype() {
	auto r = JSObjectGetPrototype(JSC_CTX(), i::Back<JSObjectRef>(this));
	DCHECK(r);
	return i::Cast(r);
}

Maybe<bool> v8::Object::SetPrototype(Local<Context> context,
																		 Local<Value> value) {
	JSObjectSetPrototype(JSC_CTX(context->GetIsolate()),
											 i::Back<JSObjectRef>(this), i::Back(value));
	return Just(true);
}

bool v8::Object::SetPrototype(Local<Value> value) {
	return SetPrototype(CONTEXT(), value).FromMaybe(false);
}

Local<Object> v8::Object::FindInstanceInPrototypeChain(v8::Local<FunctionTemplate> tmpl) {
	UNIMPLEMENTED();
}

MaybeLocal<Array> v8::Object::GetPropertyNames(Local<Context> context) {
	return GetPropertyNames(context, v8::KeyCollectionMode::kIncludePrototypes,
													static_cast<v8::PropertyFilter>(ONLY_ENUMERABLE | SKIP_SYMBOLS),
													v8::IndexFilter::kIncludeIndices);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(Local<Context> context,
																							 KeyCollectionMode mode,
																							 PropertyFilter property_filter,
																							 IndexFilter index_filter) {
	ENV(context->GetIsolate());
	JSValueRef argv[4] = {
		i::Back(this),
		JSValueMakeBoolean(ctx, mode == KeyCollectionMode::kOwnOnly),
		JSValueMakeNumber(ctx, property_filter),
		JSValueMakeBoolean(ctx, index_filter == IndexFilter::kIncludeIndices),
	};
	auto r = JSObjectCallAsFunction(ctx, isolate->getPropertyNames(), 0,
																	4, argv, OK(MaybeLocal<Array>()));
	return i::Cast<Array>(r);
}

Local<Array> v8::Object::GetPropertyNames() {
	RETURN_TO_LOCAL_UNCHECKED(GetPropertyNames(CONTEXT()), Array);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context) {
	return GetOwnPropertyNames(context,
														 static_cast<v8::PropertyFilter>(ONLY_ENUMERABLE | SKIP_SYMBOLS));
}

Local<Array> v8::Object::GetOwnPropertyNames() {
	RETURN_TO_LOCAL_UNCHECKED(GetOwnPropertyNames(CONTEXT()), Array);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context,
																									PropertyFilter filter) {
	ENV(context->GetIsolate());
	JSValueRef argv[2] = {
		i::Back(this),
		JSValueMakeNumber(ctx, filter),
	};
	auto r = JSObjectCallAsFunction(ctx, isolate->getOwnPropertyNames2(), 0,
																	1, argv, OK(MaybeLocal<Array>()));
	return i::Cast<Array>(r);
}

MaybeLocal<String> v8::Object::ObjectProtoToString(Local<Context> context) {
	ENV(context->GetIsolate());
	auto r = (JSObjectRef)JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this),
																						i::prototype_s, OK(MaybeLocal<String>()));
	return i::Cast(r)->ToString(context);
}

Local<String> v8::Object::ObjectProtoToString() {
	RETURN_TO_LOCAL_UNCHECKED(ObjectProtoToString(CONTEXT()), String);
}

Local<String> v8::Object::GetConstructorName() {
	ENV();
	auto r = (JSObjectRef)JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this),
																						i::constructor_s, OK(Local<String>()));
	if (JSValueIsObject(ctx, r)) {
		auto name = JSObjectGetProperty(ctx, r, i::name_s, OK(Local<String>()));
		if (JSValueIsString(ctx, name)) {
			return i::Cast<String>(name);
		}
	}
	return Local<String>();
}

Maybe<bool> v8::Object::SetIntegrityLevel(Local<Context> context,
																					IntegrityLevel level) {
	return Just(false);
}

Maybe<bool> v8::Object::Delete(Local<Context> context, Local<Value> key) {
	ENV(context->GetIsolate());
	i::JSCStringPtr jkey = JSValueToStringCopy(ctx, i::Back(key), &ex);
	if (ex) {
		ex = nullptr;
		JSValueRef argv[2] = { i::Back(this), i::Back(key) };
		auto r = JSObjectCallAsFunction(ctx, isolate->deleteProperty(), nullptr, 2, argv,
																		NOTHING);
		return Just(true);
	} else {
		bool r = JSObjectDeleteProperty(ctx, i::Back<JSObjectRef>(this), *jkey, NOTHING);
		return Just(r);
	}
}

bool v8::Object::Delete(v8::Local<Value> key) {
	return Delete(CONTEXT(), key).FromMaybe(false);
}

Maybe<bool> v8::Object::DeletePrivate(Local<Context> context,
																			Local<Private> key) {
	ENV(context->GetIsolate());
	auto jkey = i::Private::PrivateValue(key);
	bool r = JSObjectDeleteProperty(ctx, i::Back<JSObjectRef>(this), jkey, NOTHING);
	return Just(r);
}

Maybe<bool> v8::Object::Has(Local<Context> context,
														Local<Value> key) {
	ENV(context->GetIsolate());
	i::JSCStringPtr jkey = JSValueToStringCopy(ctx, i::Back(key), NOTHING);
	if (ex) {
		ex = nullptr;
		JSValueRef argv[2] = { i::Back(this), i::Back(key) };
		auto r = JSObjectCallAsFunction(ctx, isolate->hasProperty(), nullptr, 2, argv,
																		NOTHING);
		return Just(JSValueToBoolean(ctx, r));
	} else {
		bool r = JSObjectHasProperty(ctx, i::Back<JSObjectRef>(this), *jkey);
		return Just(r);
	}
}

bool v8::Object::Has(v8::Local<Value> key) {
	return Has(CONTEXT(), key).FromMaybe(false);
}

Maybe<bool> v8::Object::HasPrivate(Local<Context> context,
																	 Local<Private> key) {
	ENV(context->GetIsolate());
	auto jkey = i::Private::PrivateValue(key);
	bool r = JSObjectHasProperty(ctx, i::Back<JSObjectRef>(this), jkey);
	return Just(r);
}

Maybe<bool> v8::Object::Delete(Local<Context> context,
															 uint32_t index) {
	ENV(context->GetIsolate());
	i::JSCStringPtr jkey = i::Isolate::IndexedToPropertyName(isolate, index);
	bool r = JSObjectDeleteProperty(ctx, i::Back<JSObjectRef>(this), *jkey, NOTHING);
	return Just(r);
}

bool v8::Object::Delete(uint32_t index) {
	return Delete(CONTEXT(), index).FromMaybe(false);
}

Maybe<bool> v8::Object::Has(Local<Context> context,
														uint32_t index) {
	ENV(context->GetIsolate());
	i::JSCStringPtr jkey = i::Isolate::IndexedToPropertyName(isolate, index);
	bool r = JSObjectHasProperty(ctx, i::Back<JSObjectRef>(this), *jkey);
	return Just(r);
}

bool v8::Object::Has(uint32_t index) {
	return Has(CONTEXT(), index).FromMaybe(false);
}

Maybe<bool> Object::SetAccessor(Local<Context> context, Local<Name> name,
																AccessorNameGetterCallback getter,
																AccessorNameSetterCallback setter,
																MaybeLocal<Value> data, AccessControl settings,
																PropertyAttribute attribute) {
	ENV(context->GetIsolate());
	bool ok = i::Template::SetAccessorProperty(isolate, i::Back<JSObjectRef>(this), name,
	getter ? new i::CInfo(
		isolate,
		nullptr,
		nullptr,
		i::Back(data.FromMaybe(Local<Value>())),
		i::Back(name),
		(void*)getter,
		i::CInfo::ACCESSOR
	) : nullptr,
	setter ? new i::CInfo(
		isolate,
		nullptr,
		nullptr,
		i::Back(data.FromMaybe(Local<Value>())),
		i::Back(name),
		(void*)getter,
		i::CInfo::ACCESSOR
	) : nullptr, attribute, settings);
	return Just(ok);
}

bool Object::SetAccessor(Local<String> name,
												 AccessorGetterCallback getter,
												 AccessorSetterCallback setter, v8::Local<Value> data,
												 AccessControl settings, PropertyAttribute attributes) {
	return SetAccessor(CONTEXT(), name,
										 (AccessorNameGetterCallback)getter,
										 (AccessorNameSetterCallback)setter,
										 data, settings, attributes).FromMaybe(false);
}

bool Object::SetAccessor(Local<Name> name,
												 AccessorNameGetterCallback getter,
												 AccessorNameSetterCallback setter,
												 v8::Local<Value> data, AccessControl settings,
												 PropertyAttribute attributes) {
	return SetAccessor(CONTEXT(), name, getter, setter,
										 data, settings, attributes).FromMaybe(false);
}

void Object::SetAccessorProperty(Local<Name> name,
																 Local<Function> getter,
																 Local<Function> setter,
																 PropertyAttribute attribute, AccessControl settings) {
	ENV();
	DCHECK(!getter.IsEmpty() || !setter.IsEmpty());
	JSObjectRef desc = JSObjectMake(ctx, 0, OK());
	if (!getter.IsEmpty()) {
		JSObjectSetProperty(ctx, desc, i::get_s, i::Back(getter), 0, OK());
	}
	if (!setter.IsEmpty()) {
		JSObjectSetProperty(ctx, desc, i::get_s, i::Back(setter), 0, OK());
	}
	JSObjectSetProperty(ctx, desc, i::enumerable_s,
											JSValueMakeBoolean(ctx, !(DontEnum | attribute)), 0, OK());
	JSObjectSetProperty(ctx, desc, i::configurable_s,
											JSValueMakeBoolean(ctx, !(DontDelete | attribute)), 0, OK());
	JSValueRef args[3] = { i::Back(this), i::Back(name), desc, };
	JSObjectCallAsFunction(ctx, isolate->defineProperty(), 0, 3, args, OK());
}

Maybe<bool> Object::SetNativeDataProperty(v8::Local<v8::Context> context,
																					v8::Local<Name> name,
																					AccessorNameGetterCallback getter,
																					AccessorNameSetterCallback setter,
																					v8::Local<Value> data,
																					PropertyAttribute attributes) {
	return Just(false);
}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context,
																			 Local<Name> key) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(key) };
	auto r = JSObjectCallAsFunction(ctx, isolate->hasOwnProperty(),
																	i::Back<JSObjectRef>(this), 1, argv, NOTHING);
	return Just(JSValueToBoolean(ctx, r));
}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context,
																			 uint32_t index) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { JSValueMakeNumber(ctx, index) };
	auto r = JSObjectCallAsFunction(ctx, isolate->hasOwnProperty(),
																	i::Back<JSObjectRef>(this), 1, argv, NOTHING);
	return Just(JSValueToBoolean(ctx, r));
}

bool v8::Object::HasOwnProperty(Local<String> key) {
	return HasOwnProperty(CONTEXT(), key).FromMaybe(false);
}

Maybe<bool> v8::Object::HasRealNamedProperty(Local<Context> context,
																						 Local<Name> key) {
	return HasOwnProperty(context, key.As<String>());
}

bool v8::Object::HasRealNamedProperty(Local<String> key) {
	return HasRealNamedProperty(CONTEXT(), key.As<String>()).FromMaybe(false);
}

Maybe<bool> v8::Object::HasRealIndexedProperty(Local<Context> context,
																							 uint32_t index) {
	return HasOwnProperty(context, index);
}

bool v8::Object::HasRealIndexedProperty(uint32_t index) {
	return HasRealIndexedProperty(CONTEXT(), index).FromMaybe(false);
}

Maybe<bool> v8::Object::HasRealNamedCallbackProperty(Local<Context> context,
																										 Local<Name> key) {
	return Just(false);
}

bool v8::Object::HasRealNamedCallbackProperty(Local<String> key) {
	return false;
}

bool v8::Object::HasNamedLookupInterceptor() {
	return false;
}

bool v8::Object::HasIndexedLookupInterceptor() {
	return false;
}

MaybeLocal<Value> v8::Object::GetRealNamedPropertyInPrototypeChain(
Local<Context> context, Local<Name> key) {
	return Get(context, key);
}

Local<Value> v8::Object::GetRealNamedPropertyInPrototypeChain(Local<String> key) {
	RETURN_TO_LOCAL_UNCHECKED(GetRealNamedPropertyInPrototypeChain(CONTEXT(), key), Value);
}

Maybe<PropertyAttribute>
v8::Object::GetRealNamedPropertyAttributesInPrototypeChain(
Local<Context> context, Local<Name> key) {
	return GetPropertyAttributes(context, key);
}

Maybe<PropertyAttribute>
v8::Object::GetRealNamedPropertyAttributesInPrototypeChain(Local<String> key) {
	return GetPropertyAttributes(CONTEXT(), key);
}

MaybeLocal<Value> v8::Object::GetRealNamedProperty(Local<Context> context,
																									 Local<Name> key) {
	return Get(context, key);
}

Local<Value> v8::Object::GetRealNamedProperty(Local<String> key) {
	RETURN_TO_LOCAL_UNCHECKED(GetRealNamedProperty(CONTEXT(), key), Value);
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(
Local<Context> context, Local<Name> key) {
	return GetPropertyAttributes(context, key);
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(Local<String> key) {
	return GetRealNamedPropertyAttributes(CONTEXT(), key);
}

Local<v8::Object> v8::Object::Clone() {
	UNIMPLEMENTED();
}

Local<v8::Context> v8::Object::CreationContext() {
	return i::Cast<Context>(i::Context::RootContext());
}

int v8::Object::GetIdentityHash() {
	return 0;
}

bool v8::Object::IsCallable() {
	return false;
}

bool v8::Object::IsConstructor() {
	ENV();
	if (JSObjectIsFunction(ctx, i::Back<JSObjectRef>(this))) {
		auto native = (JSObjectRef)
			JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::__native___s, OK(false));
		if (JSValueIsObject(ctx, native)) {
			auto cinfo = i::CInfo::Private(native);
			if (cinfo) {
				return cinfo->FrontCallbackStyle() == i::CInfo::FUNCTION_CONSTRUCTOR;
			}
		}
		return true;
	}
	return false;
}

MaybeLocal<Value> Object::CallAsFunction(Local<Context> context,
																				 Local<Value> recv, int argc,
																				 Local<Value> argv[]) {
	ENV(context->GetIsolate());
	if (!recv.IsEmpty()) {
		if (!recv->IsObject()) {
			recv = Local<Value>();
		}
	}
	auto r = JSObjectCallAsFunction(ctx, i::Back<JSObjectRef>(this),
																	i::Back<JSObjectRef>(recv),
																	argc, reinterpret_cast<JSValueRef*>(argv),
																	OK(MaybeLocal<Value>()));
	
	return i::Cast(r);
}

Local<v8::Value> Object::CallAsFunction(v8::Local<v8::Value> recv, int argc,
																				v8::Local<v8::Value> argv[]) {
	RETURN_TO_LOCAL_UNCHECKED(CallAsFunction(CONTEXT(), recv, argc, argv), Value);
}

MaybeLocal<Value> Object::CallAsConstructor(Local<Context> context, int argc,
																						Local<Value> argv[]) {
	ENV(context->GetIsolate());
	auto r = JSObjectCallAsConstructor(ctx, i::Back<JSObjectRef>(this),
																		 argc, reinterpret_cast<JSValueRef*>(argv),
																		 OK(MaybeLocal<Value>()));
	return i::Cast(r);
}

Local<v8::Value> Object::CallAsConstructor(int argc, v8::Local<v8::Value> argv[]) {
	RETURN_TO_LOCAL_UNCHECKED(CallAsConstructor(CONTEXT(), argc, argv), Value);
}

int v8::Object::InternalFieldCount() {
	auto priv = (i::PrivateData*)JSObjectGetPrivate(i::Back<JSObjectRef>(this));
	return priv ? priv->InternalFieldCount() : 0;
}

Local<Value> v8::Object::SlowGetInternalField(int index) {
	ENV();
	auto priv = (i::PrivateData*)JSObjectGetPrivate(i::Back<JSObjectRef>(this));
	return priv ? priv->InternalField(index)->GetEmbedderData(): Local<Value>();
}

void* v8::Object::SlowGetAlignedPointerFromInternalField(int index) {
	auto priv = (i::PrivateData*)JSObjectGetPrivate(i::Back<JSObjectRef>(this));
	return priv ? priv->InternalField(index)->GetAlignedPointer(): nullptr;
}

void v8::Object::SetInternalField(int index, v8::Local<Value> value) {
	auto priv = i::PrivateData::Private(i::Back<JSObjectRef>(this));
	DCHECK(priv);
	auto ctx = JSC_CTX(priv->GetIsolate());
	priv->InternalField(index)->SetEmbedderData(ctx, value);
}

void v8::Object::SetAlignedPointerInInternalField(int index, void* value) {
	auto priv = i::PrivateData::Private(i::Back<JSObjectRef>(this));
	DCHECK(priv);
	auto ctx = JSC_CTX(priv->GetIsolate());
	priv->InternalField(index)->SetAlignedPointer(ctx, value);
}

void v8::Object::SetAlignedPointerInInternalFields(int argc,
																									 int indices[], void* values[]) {
	for (int i = 0; i < argc; i++) {
		SetAlignedPointerInInternalField(indices[i], values[i]);
	}
}

// --- O b j e c t   V a l u e ---

Isolate* v8::Object::GetIsolate() {
	return Isolate::GetCurrent();
}

Local<v8::Object> v8::Object::New(Isolate* isolate) {
	JSObjectRef obj = JSObjectMake(JSC_CTX(isolate), nullptr, nullptr);
	DCHECK(obj);
	return i::Cast<Object>(obj);
}

Local<v8::Value> v8::NumberObject::New(Isolate* iso, double value) {
	ENV(iso);
	auto num = JSValueMakeNumber(ctx, value);
	auto r = JSObjectCallAsConstructor(ctx, isolate->Number(), 1, &num, OK(Local<v8::Value>()));
	return i::Cast(r);
}

double v8::NumberObject::ValueOf() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::valueOf_s, OK(0));
	return i::Cast(r)->NumberValue(CONTEXT(isolate)).FromMaybe(0);
}

Local<v8::Value> v8::BooleanObject::New(Isolate* iso, bool value) {
	ENV(iso);
	auto num = JSValueMakeBoolean(ctx, value);
	auto r = JSObjectCallAsConstructor(ctx, isolate->Boolean(), 1, &num, OK(Local<v8::Value>()));
	return i::Cast(r);
}

Local<v8::Value> v8::BooleanObject::New(bool value) {
	return New(Isolate::GetCurrent(), value);
}

bool v8::BooleanObject::ValueOf() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::valueOf_s, OK(0));
	return JSValueToBoolean(ctx, r);
}

Local<v8::Value> v8::StringObject::New(Local<String> value) {
	ENV();
	JSValueRef argv[1] = { i::Back(value) };
	auto r = JSObjectCallAsConstructor(ctx, isolate->Boolean(), 1, argv, OK(Local<v8::Value>()));
	return i::Cast(r);
}

Local<v8::String> v8::StringObject::ValueOf() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this),
															 i::valueOf_s, OK(Local<v8::String>()));
	return i::Cast<String>(r);
}

Local<v8::Value> v8::SymbolObject::New(Isolate* iso, Local<Symbol> value) {
	ENV(iso);
	isolate->ThrowException(isolate->NewError("UNIMPLEMENTED"));
	return Local<Value>();
}

Local<v8::Symbol> v8::SymbolObject::ValueOf() const {
	ENV();
	isolate->ThrowException(isolate->NewError("UNIMPLEMENTED"));
	return Local<Symbol>();
}
