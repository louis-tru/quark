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

#include "js-1.h"
#include "wrap.h"

JS_BEGIN

JSClassStore::JSClassStore(Worker* worker)
: current_attach_object_(nullptr)
, worker_(worker)
{
}

JSClassStore::~JSClassStore() {
	for ( auto& i : desc_ ) {
		i.value()->jsclass.Reset();
		i.value()->function.Reset();
		delete i.value();
	}
}

Local<JSClass> JSClassStore::get_class(uint64 id) {
	auto i = values_.find(id);
	if ( !i.is_null() ) {
		return i.value()->jsclass.local();
	}
	return Local<JSClass>();
}

uint64 JSClassStore::set_class(uint64 id, Local<JSClass> cls,
														 WrapAttachCallback attach_callback) throw(Error) {
	FX_CHECK( ! values_.has(id), "Set native Constructors ID repeat");
	Desc* desc = new Desc();
	desc_.push(desc);
	desc->jsclass.Reset(worker_, cls);
	desc->attach_callback = attach_callback;
	values_.set(id, {desc});
	return id;
}

uint64 JSClassStore::set_class_alias(uint64 id, uint64 alias) throw(Error) {
	FX_CHECK( values_.has(id), "No Constructors ID");
	FX_CHECK( !values_.has(alias), "Set native Constructors ID repeat");
	values_.set(alias, values_.get(id));
	return alias;
}

/**
 * @func get_constructor
 */
Local<JSFunction> JSClassStore::get_constructor(uint64 id) {
	auto it = values_.find(id);
	if ( !it.is_null() ) {
		if (it.value()->function.IsEmpty()) {
			Local<JSFunction> func =
				IMPL::current(worker_)->GenConstructor(it.value()->jsclass.local());
			it.value()->function.Reset(worker_, func);
		}
		return it.value()->function.local();
	}
	return Local<JSFunction>();
}

/**
 * @func reset()
 */
void JSClassStore::reset_constructor(uint64 id) {
	auto it = values_.find(id);
	if ( !it.is_null() ) {
		it.value()->function.Reset();
	}
}

WrapObject* JSClassStore::attach(uint64 id, Object* object) {
	WrapObject* wrap = reinterpret_cast<WrapObject*>(object) - 1;
	ASSERT( !wrap->worker() );
	
	auto it = values_.find(id);
	if ( !it.is_null() ) {
		it.value()->attach_callback(wrap);
		
		ASSERT( !current_attach_object_ );
		
		Local<JSFunction> func = it.value()->function.local();
		if ( func.IsEmpty() ) {
			func = get_constructor(id);
		}
		
		current_attach_object_ = wrap;
		func->NewInstance(worker_);
		current_attach_object_ = nullptr;
		
		if ( !wrap->handle().IsEmpty() ) {
			return wrap;
		}
	}
	return nullptr;
}

bool JSClassStore::instanceof(Local<JSValue> val, uint64 id) {
	if ( values_.has(id) ) {
		Local<JSClass> cls = values_.get(id)->jsclass.local();
		return cls->HasInstance(worker_, val);
	}
	return false;
}

JS_END
