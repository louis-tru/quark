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

#include "./_js.h"

namespace qk { namespace js {

	JSClassImpl::JSClassImpl(Worker* worker, uint64_t id, cString &name)
		: _worker(worker)
		, _attachCallback(nullptr)
		, _id(id), _name(name), _ref(0)
	{}

	JSClassImpl::~JSClassImpl() {
		_func.Reset();
	}

	void JSClassImpl::retain() {
		Qk_ASSERT(_ref >= 0);
		_ref++;
	}

	void JSClassImpl::release() {
		Qk_ASSERT(_ref >= 0);
		if ( --_ref <= 0 ) {
			delete this;
		}
	}

	void JSClassImpl::resetFunc() {
		_func.Reset();
	}

	// --------------------------- J S . C l a s s ---------------------------

	void JSClass::Export(cString& name, Local<JSObject> exports) {
		auto impl = static_cast<JSClassImpl*>(this);
		impl->resetFunc(); // reset func
		exports->setProperty(worker, name, getFunction());
	}

	Local<JSFunction> JSClass::getFunction() {
		return static_cast<JSClassImpl*>(this)->func();
	}

	uint64_t JSClass::id() const {
		return static_cast<const JSClassImpl*>(this)->_id;
	}

	Local<JSObject> JSClass::newInstance(uint32_t argc, Local<JSValue>* argv) {
		auto impl = static_cast<JSClassImpl*>(this);
		auto func = impl->func();
		Qk_ASSERT( !func.IsEmpty() );
		return func->newInstance(impl->_worker, argc, argv);
	}

	template<> void PersistentBase<JSClass>::Reset() {
		if ( val_ ) {
			static_cast<JSClassImpl*>(val_)->release();
			val_ = nullptr;
		}
	}

	template<>
	template<>
	void PersistentBase<JSClass>::Reset(Worker* worker, const Local<JSClass>& other) {
		if ( val_ ) {
			static_cast<JSClassImpl*>(val_)->release();
			val_ = nullptr;
			worker_ = nullptr;
		}
		if ( !other.IsEmpty() ) {
			Qk_ASSERT(worker);
			val_ = *other;
			worker_ = worker;
			static_cast<JSClassImpl*>(val_)->retain();
		}
	}

	template<>
	template<>
	void CopyablePersistentClass::Copy(const PersistentBase<JSClass>& that) {
		Reset(that.worker_, that.local());
	}

	// --------------------------- J S . C l a s s . I n f o ---------------------------

	JSClassInfo::JSClassInfo(Worker* worker)
		: _worker(worker)
		, _currentAttachObject(nullptr)
	{
	}

	JSClassInfo::~JSClassInfo() {
		for ( auto i : _jsclass ) {
			i->release();
		}
	}

	Local<JSClass> JSClassInfo::get(uint64_t id) {
		JSClass *out;
		if ( _alias.get(id, out) ) {
			return *reinterpret_cast<Local<JSClass>*>(&out);
		}
		return Local<JSClass>();
	}

	void JSClassInfo::add(uint64_t id, JSClass *cls,
												AttachCallback callback, uint64_t alias) throw(Error) {
		Qk_Check( ! _alias.has(id), "Set native Constructors ID repeat");
		cls->retain();
		cls->_attachCallback = callback;
		_jsclass.push(cls);
		_alias.set(id, cls);

		if (alias) {
			Qk_Check( !_alias.has(alias), "Set native Constructors alias repeat");
			_alias.set(alias, cls);
		}
	}

	WrapObject* JSClassInfo::attach(uint64_t id, Object* object) {
		auto wrap = reinterpret_cast<WrapObject*>(object) - 1;
		Qk_ASSERT( !wrap->worker() );

		JSClass *out;
		if ( _alias.get(id, out) ) {
			out->_attachCallback(wrap);

			Qk_ASSERT( !_currentAttachObject );

			auto func = out->func();
			_currentAttachObject = wrap;
			func->newInstance(_worker);
			_currentAttachObject = nullptr;

			if ( !wrap->handle().IsEmpty() ) {
				return wrap;
			}
		}
		return nullptr;
	}

	bool JSClassInfo::instanceOf(Local<JSValue> val, uint64_t id) {
		JSClass *out;
		if ( _alias.get(id, out) )
			return out->hasInstance(val);
		return false;
	}

} }