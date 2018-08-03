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

v8_ns(internal)

class Signature: public Wrap {
 public:
	inline Signature(Isolate* isolate, FunctionTemplate* receiver);
	FunctionTemplate* receiver() const { return m_receiver; }
 private:
	FunctionTemplate* m_receiver;
};

class AccessorSignature: public Signature {
 public:
	inline AccessorSignature(Isolate* isolate, FunctionTemplate* receiver)
	: Signature(isolate, receiver) { };
};

/**
 * @class Template
 */
class Template: public Wrap {
 public:
	
	inline Template(Isolate* iso)
		: Wrap(iso), m_template_handle(nullptr) {}
	
	JSObjectRef TemplateHandle() {
		if (!m_template_handle) {
			m_template_handle = JSObjectMake(JSC_CTX(GetIsolate()), 0, 0);
			Reference(m_template_handle);
		}
		return m_template_handle;
	}
	
	inline bool HasTemplateHandle() const {
		return m_template_handle;
	}
	
	void Set(JSStringRef name, Local<v8::Data> value, v8::PropertyAttribute attributes);
	void Set(Local<v8::Name> name, Local<v8::Data> value,
					 v8::PropertyAttribute attributes = None) {
		// TODO Symbol 不能被转换为字符串，
		JSCStringPtr s = Isolate::ToJSString(GetIsolate(), name);
		DCHECK(*s);
		Set(*s, value, attributes);
	}
	
	void SetPrivate(Local<v8::Private> name, Local<v8::Data> value,
									v8::PropertyAttribute attributes = None) {
		Set(i::Private::PrivateValue(name), value, attributes);
	}
	
	void SetAccessorProperty(Local<v8::Name> name,
													 FunctionTemplate* getter,
													 FunctionTemplate* setter,
													 v8::PropertyAttribute attribute = None,
													 AccessControl settings = DEFAULT);
	
	static bool SetAccessorProperty(Isolate* isolate, JSObjectRef target,
																	Local<v8::Name> name,
																	UniquePtr<CInfo> getter, UniquePtr<CInfo> setter,
																	v8::PropertyAttribute attribute,
																	AccessControl settings);
	
	JSObjectRef NewInstanceStep(JSObjectRef instance) {
		DCHECK(instance);
		if (HasTemplateHandle()) {
			ENV(GetIsolate());
			JSObjectRef descriptors = (JSObjectRef)
			JSObjectCallAsFunction(ctx, isolate->getOwnPropertyDescriptors(),
														 0, 1, &m_template_handle, OK(0));
			DCHECK(descriptors);
			JSValueRef args[2] = { instance, descriptors };
			JSObjectCallAsFunction(ctx, isolate->defineProperties(), 0, 2, args, &ex);
			DCHECK(!ex);
		}
		return instance;
	}
	
 private:
	JSObjectRef m_template_handle;
};

/**
 * @class ObjectTemplate
 */
class ObjectTemplate: public Template {
 public:
	ObjectTemplate(Isolate* isolate, FunctionTemplate* constructor);
	
	virtual ~ObjectTemplate() {
		auto isolate = GetIsolate();
		if (!isolate->HasDestroy()) {
			auto ctx = JSC_CTX(isolate);
			if (m_configuration && !m_configuration->data.IsEmpty()) {
				JSValueUnprotect(ctx, Back(m_configuration->data));
			}
			if (m_indexed_configuration && !m_indexed_configuration->data.IsEmpty()) {
				JSValueUnprotect(ctx, Back(m_indexed_configuration->data));
			}
			if (m_class)
				JSClassRelease(m_class);
		}
		free(m_configuration);
		free(m_indexed_configuration);
	}
	
	JSClassRef GetObjectClass(bool ignore_property_handler = false);
	
	JSObjectRef NewInstance(bool hidden_prototype = false, bool ignore_property_handler = false);
	
	void SetAccessor(Local<Name> name, AccessorGetterCallback getter,
									 AccessorSetterCallback setter, Local<Value> data,
									 AccessControl settings, PropertyAttribute attribute,
									 AccessorSignature* signature);
	
	void SetHandler(const NamedPropertyHandlerConfiguration& configuration) {
		CHECK(m_instance_count == 0);
		if (!m_configuration) {
			m_configuration = (NamedPropertyHandlerConfiguration*)
			malloc(sizeof(NamedPropertyHandlerConfiguration));
			memset(m_configuration, 0, sizeof(NamedPropertyHandlerConfiguration));
		}
		if (!m_configuration->data.IsEmpty()) {
			JSValueUnprotect(GetIsolate()->jscc(), Back(m_configuration->data));
		}
		*m_configuration = configuration;
		if (!m_configuration->data.IsEmpty()) {
			JSValueProtect(GetIsolate()->jscc(), Back(m_configuration->data));
		}
		if (m_class) {
			JSClassRelease(m_class); m_class = nullptr;
		}
	}
	
	void SetHandler(const IndexedPropertyHandlerConfiguration& configuration) {
		CHECK(m_instance_count == 0);
		if (!m_indexed_configuration) {
			m_indexed_configuration = (IndexedPropertyHandlerConfiguration*)
			malloc(sizeof(IndexedPropertyHandlerConfiguration));
			memset(m_indexed_configuration, 0, sizeof(IndexedPropertyHandlerConfiguration));
		}
		if (!m_indexed_configuration->data.IsEmpty()) {
			JSValueUnprotect(GetIsolate()->jscc(), Back(m_indexed_configuration->data));
		}
		*m_indexed_configuration = configuration;
		if (!m_indexed_configuration->data.IsEmpty()) {
			JSValueProtect(GetIsolate()->jscc(), Back(m_indexed_configuration->data));
		}
	}
	
	void InheritHandler(ObjectTemplate* parent);
	
	inline int InternalFieldCount() {
		return m_internal_field_count;
	}
	
	void SetInternalFieldCount(int value) {
		CHECK(m_instance_count == 0);
		m_internal_field_count = value;
	}
	
	inline FunctionTemplate* Constructor() const { return m_constructor; }
	
 private:
	JSClassRef m_class;
	FunctionTemplate* m_constructor;
	int m_internal_field_count;
	int m_instance_count;
	NamedPropertyHandlerConfiguration* m_configuration;
	IndexedPropertyHandlerConfiguration* m_indexed_configuration;
	friend struct CallbackInfo;
};

/**
 * @class FunctionTemplate
 */
class FunctionTemplate: public Template {
 public:
	FunctionTemplate(Isolate* isolate,
									 FunctionCallback callback = nullptr,
									 Value* data = nullptr,
									 Signature* signature = nullptr,
									 ConstructorBehavior behavior = v8::ConstructorBehavior::kAllow)
	: Template(isolate)
	, m_instance_template(nullptr)
	, m_prototype_template(nullptr)
	, m_parent(nullptr)
	, m_callback(callback ? callback : DefaultFunctionCallback)
	, m_function_instance(nullptr)
	, m_prototype_instance(nullptr)
	, m_data(reinterpret_cast<JSValueRef>(data))
	, m_receiver(nullptr)
	, m_id(++id)
	, m_name(nullptr)
	, m_behavior(behavior)
	{
		Reference(m_data);
		if (signature) {
			m_receiver = signature->receiver();
			Reference(m_receiver);
		}
	}
	
	static void DefaultFunctionCallback(const FunctionCallbackInfo<Value>& info) {}
	
	virtual ~FunctionTemplate() {
		if (m_name) {
			JSStringRelease(m_name);
		}
	}
	
	inline FunctionTemplate* Receiver() const {
		return m_receiver;
	}
	
	Local<v8::Function> GetFunction() {
		return Cast<v8::Function>(FunctionInstance());
	}
	
	ObjectTemplate* InstanceTemplate() {
		if (!m_instance_template) {
			m_instance_template = new ObjectTemplate(GetIsolate(), this);
			Reference(m_instance_template, instance_template_s);
		}
		return m_instance_template;
	}
	
	ObjectTemplate* PrototypeTemplate() {
		if (!m_prototype_template) {
			m_prototype_template = new ObjectTemplate(GetIsolate(), nullptr);
			Reference(m_prototype_template, prototype_template_s);
		}
		return m_prototype_template;
	}
	
	JSObjectRef FunctionInstance() {
		if (!m_function_instance) {
			ENV(GetIsolate());
			m_function_instance =
				JSObjectMakeFunction(ctx, m_name, 0, 0, __native__body_s , 0, 0, &ex);
			CHECK(m_function_instance);
			
			bool is_constructor = ConstructorBehavior::kAllow == m_behavior;
			bool retain_template = false;
			
			if (is_constructor) {
				if (m_parent) {
					retain_template = true;
				} else if (m_prototype_template && m_prototype_template->HasTemplateHandle()) {
					retain_template = true;
				} else if (m_instance_template && (m_instance_template->HasTemplateHandle() ||
																					 m_instance_template->InternalFieldCount())) {
					retain_template = true;
				}
			}
			JSObjectRef native = JSObjectMake(ctx, CInfo::FunctionCallbackClass, 0);
			CHECK(native);
			
			auto cb = new CInfo(
				isolate,
				native,
				Retain(m_receiver),
				Retain(ctx, m_data),
				nullptr,
				retain_template ? (void*)Retain(this) : (void*)m_callback,
				retain_template ? CInfo::FUNCTION_TEMPLATE :
													(is_constructor ? CInfo::FUNCTION_CONSTRUCTOR : CInfo::FUNCTION)
			);
			JSObjectSetPrototype(ctx, native, isolate->Function_prototype()); // inherit Function
			
			JSPropertyAttributes attrs = kJSPropertyAttributeReadOnly |
				kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
			JSObjectSetProperty(ctx, m_function_instance,
													toString_s, isolate->NativeToString(), attrs, &ex);
			CHECK(!ex);
			NewInstanceStep(m_function_instance);
			JSObjectSetProperty(ctx, m_function_instance, __native___s, native, attrs, &ex);
			CHECK(!ex);
			attrs = kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
			// constructor.prototype = prototype
			JSObjectSetProperty(isolate->jscc(),
													m_function_instance,
													prototype_s, PrototypeInstance(), attrs, &ex);
			DCHECK(!ex);
			Reference(m_function_instance, function_instance_s); // Retain
		}
		return m_function_instance;
	}
	
	JSObjectRef PrototypeInstance() {
		if (!m_prototype_instance) {
			m_prototype_instance = PrototypeTemplate()->NewInstance(false, true);
			if (m_parent) { // Inherit
				// prototype.__proto__ = parent.prototype
				JSObjectSetPrototype(GetIsolate()->jscc(),
														 m_prototype_instance, m_parent->PrototypeInstance());
				PrototypeTemplate()->InheritHandler(m_parent->PrototypeTemplate());
			}
			JSPropertyAttributes attrs = kJSPropertyAttributeReadOnly |
			kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
			JSValueRef exception = nullptr;
			
			// prototype.constructor = constructor
			JSObjectSetProperty(GetIsolate()->jscc(),
													m_prototype_instance, constructor_s,
													FunctionInstance(), attrs, &exception);
			DCHECK(!exception);
			Reference(m_prototype_instance, prototype_instance_s); // Retain
		}
		return m_prototype_instance;
	}
	
	void SetCallHandler(FunctionCallback callback, v8::Local<v8::Value> data) {
		m_callback = callback;
		m_data = Back(data);
		Reference(m_data, data_s);
	}
	
	void Inherit(Local<v8::FunctionTemplate> value) {
		DCHECK(!value.IsEmpty());
		CHECK(m_function_instance == nullptr);
		m_parent = reinterpret_cast<FunctionTemplate*>(*value);
		Reference(m_parent, parent_s);
	}
	
	void SetClassName(Local<v8::String> name) {
		if (m_name)
			JSStringRelease(m_name);
		JSValueRef ex = 0;
		m_name = JSValueToStringCopy(GetIsolate()->jscc(), Back(name), &ex);
		DCHECK(!ex);
		JSStringRetain(m_name);
		if (m_function_instance) {
			Cast<Function>(m_function_instance)->SetName(name);
		}
	}
	
	void SetHiddenPrototype(bool value) {}
	
	bool HasInstance(JSObjectRef value) {
		auto priv = PrivateData::Private(value);
		if (priv) {
			DCHECK(priv->InstanceTemplate());
			auto constructor = priv->InstanceTemplate()->Constructor();
			if (constructor) {
				do {
					if (constructor == this)
						return true;
					constructor = constructor->m_parent;
				} while(constructor);
			} else {
				if (m_function_instance) {
					ENV(GetIsolate());
					return JSValueIsInstanceOfConstructor(ctx, value, m_function_instance, OK(false));
				}
			}
		}
		return false;
	}
	
	inline FunctionCallback FrontCallback() const { return m_callback; }
	inline JSValueRef Data() const{ return m_data; }
	inline FunctionTemplate* Parent() const{ return m_parent; }
	inline ConstructorBehavior Behavior() const{ return m_behavior; }
	
 private:
	ObjectTemplate* m_instance_template;
	ObjectTemplate* m_prototype_template;
	FunctionTemplate* m_parent;
	FunctionCallback m_callback;
	JSObjectRef m_function_instance;
	JSObjectRef m_prototype_instance;
	JSValueRef m_data;
	FunctionTemplate* m_receiver;
	int m_id;
	JSStringRef m_name;
	ConstructorBehavior m_behavior;
};

void Isolate::InitializeTemplate() {
	m_default_placeholder_template = Retain(new ObjectTemplate(this, nullptr));
	m_cinfo_placeholder_template = Retain(new ObjectTemplate(this, nullptr));
	auto ft = new FunctionTemplate(this);
	m_external_template = ft->InstanceTemplate();
	m_external_template->SetInternalFieldCount(1);
	m_external_template->retain(); // Retain ref
	m_External = ft->FunctionInstance();
}

void Isolate::DisposeTemplate() {
	m_default_placeholder_template->release(); // release template
	m_cinfo_placeholder_template->release(); // release template
	m_external_template->release(); // release template
}

Signature::Signature(Isolate* isolate, FunctionTemplate* receiver)
: Wrap(isolate), m_receiver(receiver) {
	DCHECK(receiver);
	Reference(receiver);
}

void Template::Set(JSStringRef name, Local<v8::Data> value, v8::PropertyAttribute attributes) {
	JSValueRef e = nullptr;
	JSPropertyAttributes attrs = attributes << 1;
	JSValueRef val = Back(value);
	if (IsWrap(value)) {
		auto w = reinterpret_cast<Wrap*>(*value);
		auto ft = dynamic_cast<FunctionTemplate*>(w);
		if (ft) {
			val = Back(ft->GetFunction());
		} else {
			auto ot = dynamic_cast<ObjectTemplate*>(w);
			if (ot) {
				val = ot->NewInstance();
			} else {
				FATAL();
			}
		}
	}
	JSObjectSetProperty(JSC_CTX(GetIsolate()), TemplateHandle(), name, val, attrs, &e);
	CHECK(!e);
}

void Template::SetAccessorProperty(Local<v8::Name> name,
																	 FunctionTemplate* getter,
																	 FunctionTemplate* setter,
																	 v8::PropertyAttribute attribute,
																	 AccessControl settings) {
	ENV(GetIsolate());
	Template::SetAccessorProperty(isolate, TemplateHandle(), name, new CInfo(
		isolate,
		nullptr,
		getter->Receiver(),
		getter->Data(),
		Back(name),
		(void*)getter->FrontCallback(),
		CallbackInfo::FUNCTION
	), new CInfo(
		isolate,
		nullptr,
		setter->Receiver(),
		setter->Data(),
		Back(name),
		(void*)setter->FrontCallback(),
		CallbackInfo::FUNCTION
	), attribute, settings);
}

ObjectTemplate::ObjectTemplate(Isolate* isolate, FunctionTemplate* constructor)
: m_class(nullptr)
, Template(isolate)
, m_constructor(constructor)
, m_internal_field_count(0)
, m_instance_count(0)
, m_configuration(nullptr)
, m_indexed_configuration(nullptr) {
	Reference(constructor);
}

void ObjectTemplate::SetAccessor(Local<Name> name,
																 AccessorGetterCallback getter,
																 AccessorSetterCallback setter,
																 Local<Value> data,
																 AccessControl settings,
																 PropertyAttribute attribute,
																 AccessorSignature* signature) {
	ENV(GetIsolate());
	Template::SetAccessorProperty(isolate, TemplateHandle(), name, new CInfo(
		isolate,
		nullptr,
		signature ? signature->receiver(): nullptr,
		Back(data),
		Back(name),
		(void*)getter,
		CallbackInfo::ACCESSOR
	), new CInfo(
		isolate,
		nullptr,
		signature ? signature->receiver(): nullptr,
		Back(data),
		Back(name),
		(void*)setter,
		CallbackInfo::ACCESSOR
	), attribute, settings);
}

bool Template::SetAccessorProperty(Isolate* iso, JSObjectRef target,
																	 Local<v8::Name> name,
																	 UniquePtr<CInfo> getter, UniquePtr<CInfo> setter,
																	 v8::PropertyAttribute attribute,
																	 AccessControl settings) {
	DCHECK(*getter || *setter);
	ENV(iso);
	JSObjectRef desc = JSObjectMake(ctx, 0, OK(false));
	if (*getter) {
		JSObjectRef f = JSObjectMake(ctx, CInfo::AccessorGetterCallbackClass, *getter);
		DCHECK(f);
		getter->SetHandle(f);
		JSObjectSetPrototype(ctx, f, isolate->Function_prototype());
		JSObjectSetProperty(ctx, desc, get_s, f, 0, OK(false));
	}
	if (*setter) {
		JSObjectRef f = JSObjectMake(ctx, CInfo::AccessorSetterCallbackClass, *setter);
		DCHECK(f);
		setter->SetHandle(f);
		JSObjectSetPrototype(ctx, f, isolate->Function_prototype());
		JSObjectSetProperty(ctx, desc, set_s, f, 0, OK(false));
	}
	
	JSObjectSetProperty(ctx, desc, enumerable_s,
											JSValueMakeBoolean(ctx, !(attribute | DontEnum)), 0, OK(false));
	JSObjectSetProperty(ctx, desc, configurable_s,
											JSValueMakeBoolean(ctx, !(attribute | DontDelete)), 0, OK(false));
	JSValueRef args[3] = {
		target, Back(name), desc,
	};
	JSObjectCallAsFunction(ctx, isolate->defineProperty(), 0, 3, args, OK(false));
	getter.collapse();
	setter.collapse();
	return true;
}

JSClassRef ObjectTemplate::GetObjectClass(bool ignore_property_handler) {
	if (ignore_property_handler) {
		return CInfo::ObjectClass;
	}
	if (!m_class) {
		JSClassDefinition def = kJSClassDefinitionEmpty;
		def.finalize = CInfo::ObjectDestructor;
		if (m_constructor) {
			m_constructor->PrototypeInstance(); // Inherit recursion
			InheritHandler(m_constructor->PrototypeTemplate());
		}
#define GET_CONFIG(config, id) (config ? config->id : nullptr)
#define SET_CLASS_ATTRIBUTE(id, name, value) \
if (GET_CONFIG(m_configuration, id) || GET_CONFIG(m_indexed_configuration, id)) { \
def.name = CInfo::value; \
characteristic++; \
}
		int characteristic = 0;
		SET_CLASS_ATTRIBUTE(getter, getProperty, GetProperty)
		SET_CLASS_ATTRIBUTE(setter, setProperty, SetProperty)
		SET_CLASS_ATTRIBUTE(query, hasProperty, HasProperty)
		SET_CLASS_ATTRIBUTE(deleter, deleteProperty, DeleteProperty)
		SET_CLASS_ATTRIBUTE(enumerator, getPropertyNames, GetPropertyNames)
		if (characteristic == 0) {
			m_class = CInfo::ObjectClass;
		} else {
			m_class = JSClassCreate(&def);
		}
		m_class = JSClassRetain(m_class);
		DCHECK(m_class);
	}
	return m_class;
}

void ObjectTemplate::InheritHandler(ObjectTemplate* parent) {
#define CONFIG_ATTRIBUTES(F, ...) \
F(getter, ##__VA_ARGS__) \
F(setter, ##__VA_ARGS__) \
F(query, ##__VA_ARGS__) \
F(deleter, ##__VA_ARGS__) \
F(enumerator, ##__VA_ARGS__) \
F(definer, ##__VA_ARGS__) \
F(descriptor, ##__VA_ARGS__)
#define SET_CONFIG(id, config, config1) \
if (!config1->id) config1->id = config->id;
	
	NamedPropertyHandlerConfiguration* configuration[4] = {
		parent->m_configuration,
		m_configuration,
		(NamedPropertyHandlerConfiguration*)parent->m_indexed_configuration,
		(NamedPropertyHandlerConfiguration*)m_indexed_configuration,
	};
	
	for (int i = 0; i < 4; i+=2) {
		auto config = configuration[i];
		auto config1 = configuration[i+1];
		if (config) {
			if (config1) {
				CONFIG_ATTRIBUTES(SET_CONFIG, config, config1)
				if (!config->data.IsEmpty() && config1->data.IsEmpty()) {
					config1->data = config->data;
					JSValueProtect(GetIsolate()->jscc(), Back(config1->data));
				}
			} else {
				if (i == 0) {
					SetHandler(*config);
				} else {
					SetHandler(*(IndexedPropertyHandlerConfiguration*)config);
				}
			}
		}
	}
}

JSObjectRef ObjectTemplate::NewInstance(bool hidden_prototype, bool ignore_property_handler) {
	auto instance = JSObjectMake(GetIsolate()->jscc(), GetObjectClass(ignore_property_handler), 0);
	auto priv = PrivateData::New(GetIsolate(), instance, this);
	NewInstanceStep(instance);
	if (!hidden_prototype && m_constructor) { // setting __proto__
		JSObjectSetPrototype(GetIsolate()->jscc(), instance, m_constructor->PrototypeInstance());
	}
	m_instance_count++;
	return instance;
}

v8_ns_end

// --- S i g n a t u r e ---

Local<Signature> Signature::New(Isolate* isolate,
																Local<FunctionTemplate> receiver) {
	auto sig =
	new i::Signature(reinterpret_cast<i::Isolate*>(isolate),
									 reinterpret_cast<i::FunctionTemplate*>(*receiver));
	return i::Cast<Signature>(sig);
}

Local<AccessorSignature> AccessorSignature::New(Isolate* isolate,
																								Local<FunctionTemplate> receiver) {
	auto sig =
	new i::AccessorSignature(reinterpret_cast<i::Isolate*>(isolate),
													 reinterpret_cast<i::FunctionTemplate*>(*receiver));
	return i::Cast<AccessorSignature>(sig);
}

// --- T e m p l a t e ---

void Template::Set(v8::Local<Name> name, v8::Local<Data> value,
									 v8::PropertyAttribute attribute) {
	reinterpret_cast<i::Template*>(this)->Set(name, value, attribute);
}

void Template::SetPrivate(v8::Local<Private> name, v8::Local<Data> value,
													v8::PropertyAttribute attribute) {
	reinterpret_cast<i::Template*>(this)->SetPrivate(name, value, attribute);
}

void Template::SetAccessorProperty(v8::Local<v8::Name> name,
																	 v8::Local<FunctionTemplate> getter,
																	 v8::Local<FunctionTemplate> setter,
																	 v8::PropertyAttribute attribute,
																	 v8::AccessControl access_control) {
	reinterpret_cast<i::Template*>(this)->
	SetAccessorProperty(name,
											reinterpret_cast<i::FunctionTemplate*>(*getter),
											reinterpret_cast<i::FunctionTemplate*>(*setter),
											attribute, access_control);
}

// --- F u n c t i o n   T e m p l a t e ---

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
	auto t = reinterpret_cast<i::FunctionTemplate*>(this)->PrototypeTemplate();
	return *reinterpret_cast<Local<ObjectTemplate>*>(&t);
}

void FunctionTemplate::SetPrototypeProviderTemplate(Local<FunctionTemplate> prototype_provider) {
	UNIMPLEMENTED();
}

void FunctionTemplate::Inherit(v8::Local<FunctionTemplate> value) {
	reinterpret_cast<i::FunctionTemplate*>(this)->Inherit(value);
}

Local<FunctionTemplate> FunctionTemplate::New(Isolate* isolate,
																							FunctionCallback callback,
																							v8::Local<Value> data,
																							v8::Local<Signature> signature,
																							int length, ConstructorBehavior behavior) {
	auto t = new i::FunctionTemplate(ISOLATE(isolate),
																	 callback,
																	 *data,
																	 reinterpret_cast<i::Signature*>(*signature),
																	 behavior);
	return *reinterpret_cast<Local<FunctionTemplate>*>(&t);
}

MaybeLocal<FunctionTemplate> FunctionTemplate::FromSnapshot(Isolate* isolate, size_t index) {
	UNIMPLEMENTED();
}

Local<FunctionTemplate> FunctionTemplate::NewWithCache(Isolate* isolate,
																											 FunctionCallback callback,
																											 Local<Private> cache_property,
																											 Local<Value> data,
																											 Local<Signature> signature, int length) {
	UNIMPLEMENTED();
}

void FunctionTemplate::SetCallHandler(FunctionCallback callback,
																			v8::Local<Value> data) {
	reinterpret_cast<i::FunctionTemplate*>(this)->SetCallHandler(callback, data);
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
	auto t = reinterpret_cast<i::FunctionTemplate*>(this)->InstanceTemplate();
	return i::Cast<ObjectTemplate>(t);
}

void FunctionTemplate::SetLength(int length) {
}

void FunctionTemplate::SetClassName(Local<String> name) {
	reinterpret_cast<i::FunctionTemplate*>(this)->SetClassName(name);
}

void FunctionTemplate::SetAcceptAnyReceiver(bool value) {
	UNIMPLEMENTED();
}

void FunctionTemplate::SetHiddenPrototype(bool value) {
	reinterpret_cast<i::FunctionTemplate*>(this)->SetHiddenPrototype(value);
}

void FunctionTemplate::ReadOnlyPrototype() {
	UNIMPLEMENTED();
}

void FunctionTemplate::RemovePrototype() {
	UNIMPLEMENTED();
}

MaybeLocal<v8::Function> FunctionTemplate::GetFunction(Local<Context> context) {
	return reinterpret_cast<i::FunctionTemplate*>(this)->GetFunction();
}

Local<v8::Function> FunctionTemplate::GetFunction() {
	return reinterpret_cast<i::FunctionTemplate*>(this)->GetFunction();
}

MaybeLocal<v8::Object> FunctionTemplate::NewRemoteInstance() {
	UNIMPLEMENTED();
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
	if (value.IsEmpty()) return false;
	JSObjectRef o = i::Back<JSObjectRef>(value);
	auto ft = reinterpret_cast<i::FunctionTemplate*>(this);
	if ( JSValueIsObject(ft->GetIsolate()->jscc(), o) ) {
		return reinterpret_cast<i::FunctionTemplate*>(this)->HasInstance(o);
	}
	return false;
}

// --- O b j e c t T e m p l a t e ---

Local<v8::ObjectTemplate> ObjectTemplate::New(Isolate* isolate,
																							v8::Local<FunctionTemplate> constructor) {
	return New(ISOLATE(isolate), constructor);
}

Local<v8::ObjectTemplate> ObjectTemplate::New() {
	return New(Isolate::GetCurrent());
}

Local<v8::ObjectTemplate> ObjectTemplate::New(i::Isolate* isolate,
																							v8::Local<FunctionTemplate> constructor) {
	auto r = new i::ObjectTemplate(isolate, reinterpret_cast<i::FunctionTemplate*>(*constructor));
	return *reinterpret_cast<Local<ObjectTemplate>*>(&r);
}

MaybeLocal<ObjectTemplate> ObjectTemplate::FromSnapshot(Isolate* isolate, size_t index) {
	UNIMPLEMENTED();
}

void Template::SetNativeDataProperty(v8::Local<String> name,
																		 AccessorGetterCallback getter,
																		 AccessorSetterCallback setter,
																		 v8::Local<Value> data,
																		 PropertyAttribute attribute,
																		 v8::Local<AccessorSignature> signature,
																		 AccessControl settings) {
	UNIMPLEMENTED();
}

void Template::SetNativeDataProperty(v8::Local<Name> name,
																		 AccessorNameGetterCallback getter,
																		 AccessorNameSetterCallback setter,
																		 v8::Local<Value> data,
																		 PropertyAttribute attribute,
																		 v8::Local<AccessorSignature> signature,
																		 AccessControl settings) {
	UNIMPLEMENTED();
}

void Template::SetLazyDataProperty(v8::Local<Name> name,
																	 AccessorNameGetterCallback getter,
																	 v8::Local<Value> data,
																	 PropertyAttribute attribute) {
	UNIMPLEMENTED();
}

void Template::SetIntrinsicDataProperty(Local<Name> name, Intrinsic intrinsic,
																				PropertyAttribute attribute) {
	UNIMPLEMENTED();
}

void ObjectTemplate::SetAccessor(v8::Local<String> name,
																 AccessorGetterCallback getter,
																 AccessorSetterCallback setter,
																 v8::Local<Value> data, AccessControl settings,
																 PropertyAttribute attribute,
																 v8::Local<v8::AccessorSignature> signature) {
	reinterpret_cast<i::ObjectTemplate*>(this)->
	SetAccessor(name, getter,
							setter, data,
							settings, attribute,
							reinterpret_cast<i::AccessorSignature*>(*signature));
}

void ObjectTemplate::SetAccessor(v8::Local<Name> name,
																 AccessorNameGetterCallback getter,
																 AccessorNameSetterCallback setter,
																 v8::Local<Value> data, AccessControl settings,
																 PropertyAttribute attribute,
																 v8::Local<v8::AccessorSignature> signature) {
	reinterpret_cast<i::ObjectTemplate*>(this)->
	SetAccessor(name,
							(AccessorGetterCallback)getter,
							(AccessorSetterCallback)setter,
							data,
							settings, attribute,
							reinterpret_cast<i::AccessorSignature*>(*signature));
}

void ObjectTemplate::SetNamedPropertyHandler(NamedPropertyGetterCallback getter,
																						 NamedPropertySetterCallback setter,
																						 NamedPropertyQueryCallback query,
																						 NamedPropertyDeleterCallback remover,
																						 NamedPropertyEnumeratorCallback enumerator,
																						 Local<Value> data) {
	SetHandler(NamedPropertyHandlerConfiguration(GenericNamedPropertyGetterCallback(getter),
																							 GenericNamedPropertySetterCallback(setter),
																							 GenericNamedPropertyQueryCallback(query),
																							 GenericNamedPropertyDeleterCallback(remover),
																							 GenericNamedPropertyEnumeratorCallback(enumerator),
																							 data));
}

void ObjectTemplate::MarkAsUndetectable() {
	UNIMPLEMENTED();
}

void ObjectTemplate::SetAccessCheckCallback(AccessCheckCallback callback,
																						Local<Value> data) {
	UNIMPLEMENTED();
}

void ObjectTemplate::SetAccessCheckCallbackAndHandler(
	AccessCheckCallback callback,
	const NamedPropertyHandlerConfiguration& named_handler,
	const IndexedPropertyHandlerConfiguration& indexed_handler,
Local<Value> data) {
	UNIMPLEMENTED();
}

void ObjectTemplate::SetHandler(const NamedPropertyHandlerConfiguration& config) {
	reinterpret_cast<i::ObjectTemplate*>(this)->SetHandler(config);
}

void ObjectTemplate::SetHandler(const IndexedPropertyHandlerConfiguration& config) {
	reinterpret_cast<i::ObjectTemplate*>(this)->SetHandler(config);
}

void ObjectTemplate::SetCallAsFunctionHandler(FunctionCallback callback, Local<Value> data) {
	UNIMPLEMENTED();
}

int ObjectTemplate::InternalFieldCount() {
	return reinterpret_cast<i::ObjectTemplate*>(this)->InternalFieldCount();
}

void ObjectTemplate::SetInternalFieldCount(int value) {
	reinterpret_cast<i::ObjectTemplate*>(this)->SetInternalFieldCount(value);
}

bool ObjectTemplate::IsImmutableProto() {
	return false;
}

void ObjectTemplate::SetImmutableProto() {
}

MaybeLocal<v8::Object> ObjectTemplate::NewInstance(Local<Context> context) {
	auto r = reinterpret_cast<i::ObjectTemplate*>(this)->NewInstance();
	return i::Cast<v8::Object>(r);
}

Local<v8::Object> ObjectTemplate::NewInstance() {
	RETURN_TO_LOCAL_UNCHECKED(NewInstance(Isolate::GetCurrent()->GetCurrentContext()), Object);
}
