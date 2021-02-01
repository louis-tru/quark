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

#ifndef __ftr__js__js_1__
#define __ftr__js__js_1__

#include "js.h"
#include "string.h"

#define js_bind_native_event( name, type, block) \
	FX_ON(name, [this, func]( type & evt) { HandleScope scope(worker()); block }, id)

#define js_unbind_native_event(name) FX_OFF(name, id);

#define js_bind_common_native_event(name) \
	js_bind_native_event(name, Event<>, { call(worker()->New(func,1)); })

/**
 * @ns ftr::js
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
	virtual Worker* initialize();
	virtual void release();
	static Worker* create();
	static inline IMPL* inl(Worker* worker) { return worker->_inl; }
	template<class T = IMPL>
	inline static T* current(Worker* worker = Worker::worker()) { return static_cast<T*>(worker->_inl); }
	inline static JSClassStore* js_class(Worker* worker) { return worker->_inl->_classs; }
	inline JSClassStore* js_class() { return _classs; }
	inline Worker* host() { return _host; }
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
	
	inline int is_node() const { return _is_node; }

	static int start(int argc, Char** argv);


 protected:
	friend class Worker;
	friend class NativeValue;
	Worker*         _host;
	ThreadID        _thread_id;
	ValueProgram*   _values;
	CommonStrings*  _strs;
	JSClassStore*   _classs;
	Persistent<JSObject> _global;
	Persistent<JSObject> _native_modules;
	int _is_node;
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
		ASSERT(ref_ >= 0);
		ref_++;
	}
	
	inline void release() {
		ASSERT(ref_ >= 0);
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
