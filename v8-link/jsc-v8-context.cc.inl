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

/**
 * @class Context
 */
class Context: public Wrap {
 public:

	inline Context(Isolate* external_isolate,
								 ObjectTemplate* global_template,
								 JSObjectRef global_object)
	: Wrap(external_isolate)
	, m_jsc_ctx(nullptr)
	, m_external_global_template(global_template)
	, m_external_global_object(global_object)
	, m_global(nullptr)
	, m_parent(nullptr)
	, m_is_enter(0) {
		Reference(reinterpret_cast<Wrap*>(global_template));
		Reference(global_object);
	}
	
	virtual ~Context() {
		CHECK(!m_parent);
		auto isolate = GetIsolate();
		for (int i = 0; i < 64; i++) {
			if (m_data[i].type && m_data[i].value) {
				JSValueUnprotect(isolate->jscc(), (JSValueRef)m_data[i].value);
			}
		}
	}
	
	inline Local<v8::Object> Global() {
		return Cast<v8::Object>(m_global);
	}
	
	void Enter() {
		if (m_is_enter == 0) {
			auto isolate = GetIsolate();
			m_parent = isolate->m_cur_ctx;
			bool is_top = m_parent == nullptr;
			isolate->m_cur_ctx = this;
			if (is_top) {
				m_jsc_ctx = isolate->m_default_jsc_ctx;
				m_global = (JSObjectRef)JSContextGetGlobalObject(m_jsc_ctx);
				m_context_data = isolate->m_default_context_data;
			} else {
				m_jsc_ctx = JSGlobalContextCreateInGroup(isolate->m_group, nullptr);
				m_global = (JSObjectRef)JSContextGetGlobalObject(m_jsc_ctx);
				m_context_data.Initialize(m_jsc_ctx);
			}
			
			if (m_external_global_object) {
				JSObjectSetPrototype(m_jsc_ctx, m_global, m_external_global_object);
			} else if (m_external_global_template) {
				auto global = m_external_global_template->NewInstance();
				DCHECK(global);
				JSObjectSetPrototype(m_jsc_ctx, m_global, global);
			}
			retain();
		}
		m_is_enter++;
	}
	
	void Exit() {
		CHECK(m_is_enter > 0);
		m_is_enter--;
		if (m_is_enter == 0) {
			auto isolate = GetIsolate();
			bool is_top = m_parent == nullptr;
			isolate->m_cur_ctx = m_parent;
			release();
			if (!is_top) {
				JSGlobalContextRelease(m_jsc_ctx);
				m_context_data.Destroy();
			}
			m_jsc_ctx = nullptr;
			m_parent = nullptr;
			m_global = nullptr;
			m_context_data.Reset();
		}
	}
	
	inline bool IsEnter() const { return m_jsc_ctx; }
	
	v8::Local<v8::Value> SlowGetEmbedderData(int index) {
		CHECK(index < 64);
		return m_data[index].GetEmbedderData();
	}
	
	void* SlowGetAlignedPointerFromEmbedderData(int index) {
		CHECK(index < 64);
		return m_data[index].GetAlignedPointer();
	}
	
	void SetEmbedderData(int index, v8::Local<Value> value) {
		CHECK(index < 64);
		m_data[index].SetEmbedderData(GetIsolate()->jscc(), value);
	}
	
	void SetAlignedPointerInEmbedderData(int index, void* value) {
		CHECK(index < 64);
		m_data[index].SetAlignedPointer(GetIsolate()->jscc(), value);
	}
	
	inline JSGlobalContextRef jscc() const { return m_jsc_ctx; };
	
	Context* Root() {
		return m_parent ? m_parent->Root(): this;
	}
	
	inline static Context* RootContext(Context* ctx = ISOLATE()->GetContext()) {
		return ctx ? ctx->Root() : nullptr;
	}
	
 private:
	JSGlobalContextRef m_jsc_ctx;
	ObjectTemplate* m_external_global_template;
	JSObjectRef m_external_global_object;
	JSObjectRef m_global;
	Context* m_parent;
	ContextData m_context_data;
	EmbedderData m_data[64];
	int m_is_enter;
	friend class Isolate;
	friend class v8::Isolate;
	friend class v8::Context;
};

inline JSGlobalContextRef Isolate::jscc() const {
	return m_cur_ctx ? m_cur_ctx->m_jsc_ctx: m_default_jsc_ctx;
}

#define GET_ISOLATE_ARRTIBUTES(NAME) \
JSObjectRef Isolate::NAME() { \
	return m_cur_ctx ? \
		m_cur_ctx->m_context_data.NAME(): m_default_context_data.NAME(); \
}
JS_CONTEXT_DATA(GET_ISOLATE_ARRTIBUTES)
#undef GET_ISOLATE_ARRTIBUTES


v8_ns_end

// --- C o n t e x t ---

Local<Context> v8::Context::New(v8::Isolate* external_isolate,
																v8::ExtensionConfiguration* extensions,
																v8::MaybeLocal<ObjectTemplate> global_template,
																v8::MaybeLocal<Value> global_object) {
	auto t = reinterpret_cast<i::ObjectTemplate*>
		(*global_template.FromMaybe(Local<ObjectTemplate>()));
	auto g = i::Back<JSObjectRef>(global_object.FromMaybe(Local<Value>()));
	auto ctx = new i::Context(ISOLATE(external_isolate), t, g);
	return i::Cast<Context>(ctx);
}

void Context::Enter() {
	reinterpret_cast<i::Context*>(this)->Enter();
}

void Context::Exit() {
	reinterpret_cast<i::Context*>(this)->Exit();
}

v8::Isolate* Context::GetIsolate() {
	return reinterpret_cast<v8::Isolate*>(reinterpret_cast<i::Context*>(this)->GetIsolate());
}

v8::Local<v8::Object> Context::Global() {
	return reinterpret_cast<i::Context*>(this)->Global();
}

v8::Local<v8::Value> Context::SlowGetEmbedderData(int index) {
	return reinterpret_cast<i::Context*>(this)->SlowGetEmbedderData(index);
}

void v8::Context::SetEmbedderData(int index, v8::Local<Value> value) {
	reinterpret_cast<i::Context*>(this)->SetEmbedderData(index, value);
}

void* v8::Context::SlowGetAlignedPointerFromEmbedderData(int index) {
	return reinterpret_cast<i::Context*>(this)->SlowGetAlignedPointerFromEmbedderData(index);
}

void v8::Context::SetAlignedPointerInEmbedderData(int index, void* value) {
	reinterpret_cast<i::Context*>(this)->SetAlignedPointerInEmbedderData(index, value);
}

