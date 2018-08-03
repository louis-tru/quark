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

static int wrap_mark_ptr = 0;
constexpr int kPointerSize = sizeof(void*);

/**
 * @class Wrap
 * 这个类的功能主要为了方便内存管理`Wrap->m_handle`被回收时`Wrap`也会被删除
 */
class Wrap: public PrivateDataBase {
 public:
	Wrap(Isolate* isolate): m_isolate(isolate) {
		m_mark = &wrap_mark_ptr;
		m_handle = JSObjectMake(isolate->jscc(), CInfo::WrapHandleClass, this);
		DCHECK(m_handle);
		isolate->ScopeRetain(m_handle);
	}
	virtual ~Wrap() {
		CHECK(Handle() == nullptr);
	}
	virtual Wrap* AsWrap() { return this; }
	
	inline void retain() {
		JSValueProtect(m_isolate->jscc(), Handle());
	}
	inline void release() {
		if (!m_isolate->HasDestroy())
			JSValueUnprotect(m_isolate->jscc(), Handle());
	}
	inline void Destroy() {
		delete this;
	}
	inline Isolate* GetIsolate() const {
		return m_isolate;
	}
	inline JSObjectRef Handle() const {
		return m_handle;
	}
	inline static bool IsWrap(Object* o) {
		DCHECK(o);
		auto w = reinterpret_cast<Wrap*>(o);
		if (kPointerSize == 8) {
			if (0xffff == (size_t(w) >> 48)) {
				return false;
			}
		}
		if (size_t(w) < 0xffff) {
			return false;
		}
		return w->m_mark == &wrap_mark_ptr;
	}
	inline static bool IsWrap(Local<v8::Data> o) {
		return IsWrap(reinterpret_cast<Object*>(*o));
	}
	inline void Reference(Wrap* child, JSStringRef name = nullptr) {
		if (child)
			Reference(child->Handle(), name);
	}
	void Reference(JSValueRef child, JSStringRef name = nullptr) {
		if (child) {
			JSValueRef ex = 0;
			if (name) {
				JSObjectSetProperty(m_isolate->jscc(), Handle(), name, child, 0, &ex);
			} else {
				static int id = 0;
				JSObjectSetPropertyAtIndex(m_isolate->jscc(), Handle(), id++, child, &ex);
			}
			DCHECK(!ex);
		}
	}
	JSValueRef GetReference(JSStringRef name) {
		JSValueRef ex = 0;
		auto r = JSObjectGetProperty(m_isolate->jscc(), Handle(), name, &ex);
		DCHECK(!ex);
		return r;
	}
	static Wrap* Unwrap(JSObjectRef obj) {
		auto priv = (PrivateDataBase*)JSObjectGetPrivate(obj);
		return priv ? priv->AsWrap() : nullptr;
	}
	
 private:
	int* m_mark;
	Isolate*  m_isolate;
	JSObjectRef m_handle;
	friend class CallbackInfo;
};

/**
 * @class Private
 */
class Private: public Wrap {
 public:
	
	Private(Isolate* iso, JSValueRef name)
	: Wrap(iso), m_name(nullptr), m_private_value(nullptr) {
		ENV(iso);
		m_name = JSValueToStringCopy(ctx, name, &ex);
		DCHECK(!ex);
		auto value = JSObjectCallAsFunction(ctx, isolate->newPrivateValue(), 0, 1, &name, &ex);
		DCHECK(!ex);
		m_private_value = JSValueToStringCopy(ctx, value, &ex);
		DCHECK(!ex);
		JSStringRetain(m_name);
		JSStringRetain(m_private_value);
	}
	virtual ~Private() {
		JSStringRelease(m_name);
		JSStringRelease(m_private_value);
	}
	inline JSStringRef Name() const { return m_name; }
	inline JSStringRef Value() const { return m_private_value; }
	static JSStringRef PrivateValue(Local<v8::Private> priv) {
		auto p = reinterpret_cast<i::Private*>(*priv);
		return p->m_private_value;
	}
	
 private:
	JSStringRef m_name;
	JSStringRef m_private_value;
};

v8_ns_end
