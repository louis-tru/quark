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

#ifndef __ngui__js__js_1__
#define __ngui__js__js_1__

#include "js.h"
#include "str.h"

#define js_bind_native_event( name, type, block) \
	NX_ON(name, [this, func]( type & evt) { HandleScope scope(worker()); block }, id)

#define js_unbind_native_event(name) NX_OFF(name, id);

#define js_bind_common_native_event(name) \
	js_bind_native_event(name, Event<>, { call(worker()->New(func,1)); })

/**
 * @ns ngui::js
 */

JS_BEGIN

class JSClassStore;

class WeakCallbackInfo {
 public:
	typedef void (*Callback)(const WeakCallbackInfo& info);
	Worker* worker() const;
	void* GetParameter() const;
};

/**
 * @class Worker::IMPL
 */
class Worker::IMPL {
 public:

	IMPL();
	virtual ~IMPL();
	virtual void initialize();
	virtual void release();

	static Worker* create();

	template<class T = IMPL>
	inline static T* current(Worker* worker = Worker::worker()) {
		return static_cast<T*>(worker->m_inl);
	}
	inline static JSClassStore* js_class(Worker* worker) {
		return worker->m_inl->m_classs;
	}
	inline Worker* host() { return m_host; }
	inline JSClassStore* js_class() { return m_classs; }

	static WrapObject* GetObjectPrivate(Local<JSObject> object);
	static bool SetObjectPrivate(Local<JSObject> object, WrapObject* value);

	bool IsWeak(PersistentBase<JSObject>& handle);

	void SetWeak(PersistentBase<JSObject>& handle,
							 WrapObject* ptr, WeakCallbackInfo::Callback callback);
	void ClearWeak(PersistentBase<JSObject>& handle, WrapObject* ptr);

	Local<JSFunction> GenConstructor(Local<JSClass> cls);
	Local<JSValue> binding_node_module(cString& name);

	int  TriggerExit(int code);
	int  TriggerBeforeExit(int code);
	bool TriggerUncaughtException(Local<JSValue> err);
	bool TriggerUnhandledRejection(Local<JSValue> reason, Local<JSValue> promise);

	static int start(int argc, char** argv);

	static inline IMPL* inl(Worker* worker) {
		return worker->m_inl;
	}

 protected:
	friend class Worker;
	friend class NativeValue;
	Worker*         m_host;
	ThreadID        m_thread_id;
	ValueProgram*   m_values;
	CommonStrings*  m_strs;
	JSClassStore*   m_classs;
	Persistent<JSObject> m_global;
	Persistent<JSObject> m_native_modules;
	int m_is_node;
};

typedef Worker::IMPL IMPL;

/**
 * @class JSClassIMPL
 */
class JSClassIMPL {
 public:
	inline JSClassIMPL(Worker* worker, uint64 id, cString& name)
	: worker_(worker)
	, id_(id), name_(name), ref_(0) {
	}
	
	virtual ~JSClassIMPL() { }
	
	inline uint64 id() const { return id_; }
	
	inline void retain() {
		NX_ASSERT(ref_ >= 0);
		ref_++;
	}
	
	inline void release() {
		NX_ASSERT(ref_ >= 0);
		if ( --ref_ <= 0 ) {
			delete this;
		}
	}
	
	inline Worker* worker() const { return worker_; }
	
 protected:
	Worker* worker_;
	uint64 id_;
	String name_;
	int ref_;
};

/**
 * @class JSClassStore
 */
class JSClassStore {
 public: 
	typedef Worker::WrapAttachCallback WrapAttachCallback;
	
	JSClassStore(Worker* worker);
	
	/**
	 * @destructor
	 */
	virtual ~JSClassStore();
	
	/**
	 * @func set_class
	 */
	uint64 set_class(uint64 id, Local<JSClass> cls, WrapAttachCallback attach) throw(Error);
	
	/**
	 * @func get_class
	 */
	Local<JSClass> get_class(uint64 id);
	
	/**
	 * @func set_class_alias
	 */
	uint64 set_class_alias(uint64 id, uint64 alias) throw(Error);
	
	/**
	 * @func get_constructor
	 */
	Local<JSFunction> get_constructor(uint64 id);
	
	/**
	 * @func reset_constructor()
	 */
	void reset_constructor(uint64 id);
	
	/**
	 * @func attach
	 */
	WrapObject* attach(uint64 id, Object* object);
	
	/**
	 * @func instanceof
	 */
	bool instanceof(Local<JSValue> val, uint64 id);
	
	/**
	 * @func has
	 */
	inline bool has(uint64 id) {
		return values_.has(id);
	}
	
 private:
	
	struct Desc {
		Persistent<JSClass> jsclass;
		Persistent<JSFunction> function;
		WrapAttachCallback  attach_callback;
	};
	Array<Desc*> desc_;
	Map<uint64, Desc*> values_;
	WrapObject* current_attach_object_;
	Worker* worker_;
	
	friend class WrapObject;
};

JS_END
#endif
