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

#include "qgr/utils/string.h"
#include "js-1.h"
#include "native-core-js.h"
#include "qgr.h"
#include "qgr/utils/http.h"
#include "qgr/utils/codec.h"
#include "value.h"

/**
 * @ns qgr::js
 */

#ifndef WORKER_DATA_INDEX
# define WORKER_DATA_INDEX (-1)
#endif


JS_BEGIN

IMPL::IMPL(Worker* host)
: host_(host)
, classs_(new JSClassStore(host)) {
}

IMPL::~IMPL() {
	XX_ASSERT( !classs_ );
}

Buffer JSValue::ToBuffer(Worker* worker, Encoding en) const {
	switch (en) {
		case Encoding::hex: // 解码 hex and base64
		case Encoding::base64:
			return Coder::decoding_to_byte(en, ToStringValue(worker,1));
		case Encoding::utf8:// ucs2编码成utf8
			return ToStringValue(worker).collapse_buffer();
		case Encoding::ucs2:
		case Encoding::utf16: {
			Ucs2String str = ToUcs2StringValue(worker);
			uint len = str.length() * 2;
			return Buffer((char*)str.collapse(), len);
		}
		default: // 编码
			return Coder::encoding(en, ToStringValue(worker));
	}
}

Maybe<Map<String, int>> JSObject::ToIntegerMap(Worker* worker) {
	Map<String, int> r;
	
	if ( IsObject(worker) ) {
	
		Local<JSArray> names = GetPropertyNames(worker);
		if ( names.IsEmpty() ) return Maybe<Map<String, int>>();
		
		for ( uint i = 0, len = names->Length(worker); i < len; i++ ) {
			Local<JSValue> key = names->Get(worker, i);
			if ( names.IsEmpty() ) return Maybe<Map<String, int>>();
			Local<JSValue> val = Get(worker, key);
			if ( val.IsEmpty() ) return Maybe<Map<String, int>>();
			if ( val->IsNumber(worker) ) {
				r.set( key->ToStringValue(worker), val->ToNumberValue(worker) );
			} else {
				r.set( key->ToStringValue(worker), val->ToBooleanValue(worker) );
			}
		}
	}
	return Maybe<Map<String, int>>(move(r));
}

Maybe<Map<String, String>> JSObject::ToStringMap(Worker* worker) {
	Map<String, String> r;
	
	if ( IsObject(worker) ) {
		Local<JSArray> names = GetPropertyNames(worker);
		if ( names.IsEmpty() ) return Maybe<Map<String, String>>();
		
		for ( uint i = 0, len = names->Length(worker); i < len; i++ ) {
			Local<JSValue> key = names->Get(worker, i);
			if ( names.IsEmpty() ) return Maybe<Map<String, String>>();
			Local<JSValue> val = Get(worker, key);
			if ( val.IsEmpty() ) return Maybe<Map<String, String>>();
			r.set( key->ToStringValue(worker), val->ToStringValue(worker) );
		}
	}
	return Maybe<Map<String, String>>(move(r));
}

Maybe<JSON> JSObject::ToJSON(Worker* worker) {
	JSON r = JSON::object();
	
	if ( IsObject(worker) ) {
		Local<JSArray> names = GetPropertyNames(worker);
		if ( names.IsEmpty() ) return Maybe<JSON>();
		
		for ( uint i = 0, len = names->Length(worker); i < len; i++ ) {
			Local<JSValue> key = names->Get(worker, i);
			if ( names.IsEmpty() ) return Maybe<JSON>();
			Local<JSValue> val = Get(worker, key);
			if ( val.IsEmpty() ) return Maybe<JSON>();
			String key_s = key->ToStringValue(worker);
			if (val->IsUint32(worker)) {
				r[key_s] = val->ToUint32Value(worker);
			} else if (val->IsInt32(worker)) {
				r[key_s] = val->ToInt32Value(worker);
			} else if (val->IsNumber(worker)) {
				r[key_s] = val->ToInt32Value(worker);
			} else if (val->IsBoolean(worker)) {
				r[key_s] = val->ToBooleanValue(worker);
			} else if (val->IsNull(worker)) {
				r[key_s] = JSON::null();
			} else {
				r[key_s] = val->ToStringValue(worker);
			}
		}
	}
	return Maybe<JSON>(move(r));
}

Local<JSValue> JSObject::GetProperty(Worker* worker, cString& name) {
	return Get(worker, worker->New(name, 1));
}

Maybe<Array<String>> JSArray::ToStringArrayMaybe(Worker* worker) {
	Array<String> rv;
	if ( IsArray(worker) ) {
		for ( uint i = 0, len = Length(worker); i < len; i++ ) {
			Local<JSValue> val = Get(worker, i);
			if ( val.IsEmpty() )
				return Maybe<Array<String>>();
			rv.push( val->ToStringValue(worker) );
		}
	}
	return Maybe<Array<String>>(move(rv));
}

Maybe<Array<double>> JSArray::ToNumberArrayMaybe(Worker* worker) {
	Array<double> rv;
	if ( IsArray(worker) ) {
		double out;
		for ( uint i = 0, len = Length(worker); i < len; i++ ) {
			Local<JSValue> val = Get(worker, i);
			if ( val.IsEmpty() || !val->ToNumberMaybe(worker).To(out) )
				return Maybe<Array<double>>();
			rv.push( out );
		}
	}
	return Maybe<Array<double>>(move(rv));
}

Maybe<Buffer> JSArray::ToBufferMaybe(Worker* worker) {
	Buffer rv;
	if ( IsArray(worker) ) {
		double out;
		for ( uint i = 0, len = Length(worker); i < len; i++ ) {
			Local<JSValue> val = Get(worker, i);
			if ( val.IsEmpty() || !val->ToNumberMaybe(worker).To(out) )
				return Maybe<Buffer>();
			rv.push( out );
		}
	}
	return Maybe<Buffer>(move(rv));
}

WeakBuffer JSArrayBuffer::weak_buffer(Worker* worker) {
	int size = ByteLength(worker);
	char* data = Data(worker);
	return WeakBuffer(data, size);
}

WeakBuffer JSTypedArray::weak_buffer(Worker* worker) {
	return Buffer(worker)->weak_buffer(worker);
}

void JSClass::Export(Worker* worker, cString& name, Local<JSObject> exports) {
	uint64 id = ID();
	auto js_class = IMPL::js_class(worker);
	js_class->reset_constructor(id);
	Local<JSFunction> func = js_class->get_constructor(ID());
	exports->SetProperty(worker, name, func);
}

Local<JSFunction> JSClass::GetFunction(Worker* worker) {
	uint64 id = ID();
	auto js_class = IMPL::js_class(worker);
	js_class->reset_constructor(id);
	return js_class->get_constructor(id);
}

uint64 JSClass::ID() const {
	return reinterpret_cast<const JSClassIMPL*>(this)->id();
}

Local<JSObject> JSClass::NewInstance(uint argc, Local<JSValue>* argv) {
	auto cls = reinterpret_cast<JSClassIMPL*>(this);
	Local<JSFunction> func = IMPL::js_class(cls->worker())->get_constructor(cls->id());
	XX_ASSERT( !func.IsEmpty() );
	return func->NewInstance(cls->worker(), argc, argv);
}

template <> void PersistentBase<JSClass>::Reset() {
	if ( val_ ) {
		reinterpret_cast<JSClassIMPL*>(val_)->release();
		val_ = nullptr;
	}
}

template <> template <>
void PersistentBase<JSClass>::Reset(Worker* worker, const Local<JSClass>& other) {
	if ( val_ ) {
		reinterpret_cast<JSClassIMPL*>(val_)->release();
		val_ = nullptr;
		worker_ = nullptr;
	}
	if ( !other.IsEmpty() ) {
		XX_ASSERT(worker);
		val_ = *other;
		worker_ = worker;
		reinterpret_cast<JSClassIMPL*>(val_)->retain();
	}
}

template<> template<>
void CopyablePersistentClass::Copy(const PersistentBase<JSClass>& that) {
	Reset(that.worker_, that.strong());
}

// -------------------------------------------------------------------------------------

Worker::Worker()
: m_thread_id(SimpleThread::current_id())
, m_value_program( nullptr )
, m_strs( nullptr )
, m_inl(nullptr)
{
	XX_CHECK(worker() == nullptr);
	
	m_inl = IMPL::create(this);
	m_global = m_inl->initialize();

	if (m_global.IsEmpty()) {
		XX_FATAL("Cannot complete initialization by Worker !");
	}
	
	m_strs = new CommonStrings(this);
	
	WeakBuffer bf((char*) native_js::CORE_native_js_code_ext_,
								native_js::CORE_native_js_code_ext_count_);
	if ( !run_native_script(m_global, bf, "ext.js")) {
		XX_FATAL("Not initialize");
	}
}

Worker::~Worker() {
	Release(m_value_program); m_value_program = nullptr;
	Release(m_strs); m_strs = nullptr;
	
	delete m_inl->classs_; m_inl->classs_ = nullptr;
	delete m_inl; m_inl = nullptr;
}

/**
 * @func fatal exit worker
 */
void Worker::fatal(Local<JSValue> err) {
	// TODO ..
	throw_err(err);
}

Local<JSObject> Worker::NewError(cchar* errmsg, ...) {
	XX_STRING_FORMAT(errmsg, str);
	Error err(ERR_UNKNOWN_ERROR, str);
	return New(err);
}

Local<JSObject> Worker::New(const HttpError& err) {
	Local<JSObject> rv = New(*static_cast<cError*>(&err));
	if ( !rv.IsEmpty() ) {
		if (!rv->Set(this, strs()->status(), New(err.status()))) return Local<JSObject>();
		if (!rv->Set(this, strs()->url(), New(err.url()))) return Local<JSObject>();
		if (!rv->Set(this, strs()->code(), New(err.code()))) return Local<JSObject>();
	}
	return rv;
}

Local<JSArray> Worker::New(Array<Dirent>& ls) { return New(move(ls)); }
Local<JSArray> Worker::New(Array<FileStat>& ls) { return New(move(ls)); }
Local<JSTypedArray> Worker::New(Buffer& buff) { return New(move(buff)); }
Local<JSObject> Worker::New(FileStat& stat) { return New(move(stat)); }
Local<JSObject> Worker::NewError(cError& err) { return New(err); }
Local<JSObject> Worker::NewError(const HttpError& err) { return New(err); }

Local<JSObject> Worker::New(FileStat&& stat) {
	Local<JSFunction> func = m_inl->classs_->get_constructor(JS_TYPEID(FileStat));
	XX_ASSERT( !func.IsEmpty() );
	Local<JSObject> r = func->NewInstance(this);
	*Wrap<FileStat>::unpack(r)->self() = move(stat);
	return r;
}

Local<JSObject> Worker::NewInstance(uint64 id, uint argc, Local<JSValue>* argv) {
	Local<JSFunction> func = m_inl->classs_->get_constructor(id);
	XX_ASSERT( !func.IsEmpty() );
	return func->NewInstance(this, argc, argv);
}

Local<JSTypedArray> Worker::NewBuffer(Local<JSString> str, Encoding en) {
	Buffer buff = str->ToBuffer(this, en);
	return New(buff);
}

void Worker::throw_err(cchar* errmsg, ...) {
	XX_STRING_FORMAT(errmsg, str);
	throw_err(NewError(*str));
}

bool Worker::has_buffer(Local<JSValue> val) {
	return val->IsTypedArray(this) || val->IsArrayBuffer(this);
}

bool Worker::has_view(Local<JSValue> val) {
	return m_inl->classs_->instanceof(val, qgr::View::VIEW);
}

bool Worker::has_instance(Local<JSValue> val, uint64 id) {
	return m_inl->classs_->instanceof(val, id);
}

/**
 * @func as_buffer
 */
WeakBuffer Worker::as_buffer(Local<JSValue> val) {
	if (val->IsTypedArray(this)) {
		return val.To<JSTypedArray>()->weak_buffer(this);
	} else if (val->IsArrayBuffer()) {
		return val.To<JSArrayBuffer>()->weak_buffer(this);
	}
	return WeakBuffer();
}

void Worker::reg_module(cString& name, BindingCallback binding, cchar* file) {
	
//	struct node_module2: node::node_module {
//		static void Func(v8::Local<v8::Object> exports,
//										 v8::Local<v8::Value> module,
//										 v8::Local<v8::Context> context, void* priv) {
//			auto data = (node_module2*)priv; XX_ASSERT(data);
//			data->binding(Cast<JSObject>(exports), Worker::worker());
//		}
//		BindingCallback binding;
//		String name;
//	};
//
//	auto m = new node_module2();
//
//	m->binding = binding;
//	m->name = name;
//	m->nm_version = NODE_MODULE_VERSION;
//	m->nm_flags = NM_F_BUILTIN;
//	m->nm_dso_handle = NULL;
//	m->nm_filename = file ? file : __FILE__;
//	m->nm_register_func = NULL;
//	m->nm_context_register_func = node_module2::Func;
//	m->nm_modname = *m->name;
//	m->nm_priv = m;
//	m->nm_link = NULL;
//
//	//XX_DEBUG("node::node_module_register: %s", *name);
//
//	node::node_module_register(m);
}

Local<JSObject> Worker::binding_module(cString& name) {
//	Local<JSValue> argv = New(name);
//	Local<JSValue> binding = Cast<JSObject>(m_env->process_object())->GetProperty(this, "binding");
//	return binding.To<JSFunction>()->Call(this, 1, &argv).To();
}

JS_END
