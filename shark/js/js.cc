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

#include "shark/utils/string.h"
#include "js-1.h"
#include "node-1.h"
#include "native-core-js.h"
#include "shark.h"
#include "shark/utils/http.h"
#if XX_ANDROID
# include "android/android.h"
#endif
#include "node.h"
#include "shark/utils/codec.h"
#include <openssl/ssl.h>

namespace shark {
	extern void set_ssl_root_x509_store_function(X509_STORE* (*)());
}

namespace node {
	namespace crypto {
		extern X509_STORE* NewRootCertStore();
	}
}

/**
 * @ns shark::js
 */

#ifndef WORKER_DATA_INDEX
# define WORKER_DATA_INDEX (-1)
#endif

extern int (*__xx_default_gui_main)(int, char**);

JS_BEGIN

void open_rlog(cString& r_url);
void close_rlog();

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

Worker::Worker(node::Environment* env)
: m_thread_id(SimpleThread::current_id())
, m_value_program( nullptr )
, m_strs( nullptr )
, m_inl(nullptr)
, m_env(env)
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
	return m_inl->classs_->instanceof(val, shark::View::VIEW);
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

//#undef XX_MEMORY_TRACE_MARK
//#define XX_MEMORY_TRACE_MARK 1

#if XX_MEMORY_TRACE_MARK
static int record_wrap_count = 0;
static int record_strong_count = 0;

# define print_wrap(s) \
	LOG("record_wrap_count: %d, strong: %d, %s", record_wrap_count, record_strong_count, s)
#else
# define print_wrap(s)
#endif

/**
 * @class WrapObject::Inl
 */
class WrapObject::Inl: public WrapObject {
public:
#define _inl_wrap(self) static_cast<WrapObject::Inl*>(self)
	
	void clear_weak() {
#if XX_MEMORY_TRACE_MARK
		if (IMPL::current(worker())->IsWeak(handle_)) {
			record_strong_count++;
			print_wrap("mark_strong");
		}
#endif
		IMPL::current(worker())->ClearWeak(handle_, this);
	}
	
	void make_weak() {
#if XX_MEMORY_TRACE_MARK
		if (!IMPL::current(worker())->IsWeak(handle_)) {
			record_strong_count--;
			print_wrap("make_weak");
		}
#endif
		IMPL::current(worker())->SetWeak(handle_, this, [](const WeakCallbackInfo& info) {
			auto self = _inl_wrap(info.GetParameter());
			self->handle_.V8_DEATH_RESET();
			self->destroy();
		});
	}
};

void WrapObject::initialize() {}
void WrapObject::destroy() { 
	delete this;
}

void WrapObject::init2(FunctionCall args) {
	XX_ASSERT(args.IsConstructCall());
	Worker* worker_ = args.worker();
	auto classs = IMPL::current(worker_)->js_class();
	XX_ASSERT( !classs->current_attach_object_ );
	handle_.Reset(worker_, args.This());
	bool ok = IMPL::SetObjectPrivate(args.This(), this); XX_ASSERT(ok);
#if XX_MEMORY_TRACE_MARK
	record_wrap_count++; 
	record_strong_count++;
#endif
	if (!self()->is_reference() || /* non reference */
			static_cast<Reference*>(self())->ref_count() <= 0) {
		_inl_wrap(this)->make_weak();
	}
	initialize();
}

WrapObject* WrapObject::attach(FunctionCall args) {
	JS_WORKER(args);
	auto classs = IMPL::current(worker)->js_class();
	if ( classs->current_attach_object_ ) {
		WrapObject* wrap = classs->current_attach_object_;
		XX_ASSERT(!wrap->worker());
		XX_ASSERT(args.IsConstructCall());
		wrap->handle_.Reset(worker, args.This());
		bool ok = IMPL::SetObjectPrivate(args.This(), wrap); XX_ASSERT(ok);
		classs->current_attach_object_ = nullptr;
		wrap->initialize();
#if XX_MEMORY_TRACE_MARK
		record_wrap_count++; 
		record_strong_count++;
		print_wrap("External");
#endif
		return wrap;
	}
	return nullptr;
}

WrapObject::~WrapObject() {
	XX_ASSERT(handle_.IsEmpty());

 #if XX_MEMORY_TRACE_MARK
	record_wrap_count--;
	print_wrap("~WrapObject");
 #endif
	self()->~Object();
}

Object* WrapObject::private_data() {
	Local<JSValue> data = get(worker()->strs()->__native_private_data());
	if ( worker()->has_instance(data, JS_TYPEID(Object)) ) {
		return unpack<Object>(data.To<JSObject>())->self();
	}
	return nullptr;
}

bool WrapObject::set_private_data(Object* data, bool trusteeship) {
	XX_ASSERT(data);
	auto p = pack(data, JS_TYPEID(Object));
	if (p) {
		set(worker()->strs()->__native_private_data(), p->that());
		if (trusteeship) {
			if (!data->is_reference() || /* non reference */
					static_cast<Reference*>(data)->ref_count() <= 0) {
				_inl_wrap(static_cast<WrapObject*>(p))->make_weak();
			}
		}
		XX_ASSERT(private_data());
	}
	return p;
}

Local<JSValue> WrapObject::call(Local<JSValue> name, int argc, Local<JSValue> argv[]) {
	Local<JSObject> o = that();
	Local<JSValue> func = o->Get(worker(), name);
	if ( func->IsFunction(worker()) ) {
		return func.To<JSFunction>()->Call(worker(), argc, argv, o);
	} else {
		worker()->throw_err("Function not found, \"%s\"", *name->ToStringValue(worker()));
		return Local<JSValue>();
	}
}

Local<JSValue> WrapObject::call(cString& name, int argc, Local<JSValue> argv[]) {
	return call(worker()->New(name), argc, argv);
}

bool WrapObject::is_pack(Local<JSObject> object) {
	XX_ASSERT(!object.IsEmpty());
	return IMPL::GetObjectPrivate(object);
}

WrapObject* WrapObject::unpack2(Local<JSObject> object) {
	XX_ASSERT(!object.IsEmpty());
	return static_cast<WrapObject*>(IMPL::GetObjectPrivate(object));
}

WrapObject* WrapObject::pack2(Object* object, uint64 type_id) {
	WrapObject* wrap = reinterpret_cast<WrapObject*>(object) - 1;
	if ( !wrap->worker() ) { // uninitialized
		JS_WORKER();
		return IMPL::js_class(worker)->attach(type_id, object);
	}
	return wrap;
}

struct ObjectAllocatorImpl {

	static void* alloc(size_t size) {
		WrapObject* o = (WrapObject*)::malloc(size + sizeof(WrapObject));
		XX_ASSERT(o);
		memset((void*)o, 0, sizeof(WrapObject));
		return o + 1;
	}
	
	static void release(Object* obj) {
		WrapObject* wrap = reinterpret_cast<WrapObject*>(obj) - 1;
		if ( wrap->worker() ) {
			_inl_wrap(wrap)->make_weak();
		}  else { // uninitialized
			obj->~Object();
			::free(wrap);
		}
	}
	
	static void retain(Object* obj) {
		WrapObject* wrap = reinterpret_cast<WrapObject*>(obj) - 1;
		if ( wrap->worker() ) {
			_inl_wrap(wrap)->clear_weak();
		} // else // uninitialized
	}
};

struct SharkApiImplementation {
	static Worker* create_shark_js_worker(node::Environment* env,
																			 bool is_inspector,
																			 int argc, const char* const* argv) {
		if (argc > 1) {
			Map<String, String> opts;
			for (int i = 2; i < argc; i++) {
				String arg = argv[i];
				if ( arg[0] == '-' ) {
					Array<String> ls = arg.split('=');
					opts.set( ls[0].substr(arg[1] == '-' ? 2: 1), ls.length() > 1 ? ls[1] : String() );
				}
			}
			if (opts.has("rlog")) {
				open_rlog(opts["rlog"]);
			} else if (is_inspector || opts.has("dev")) {
				open_rlog(argv[1]);
			}
		}
		return new Worker(env);
	}
	static void delete_shark_js_worker(shark::js::Worker* worker) {
		Release(worker);
	}
	static RunLoop* shark_main_loop() {
		return shark::RunLoop::main_loop();
	}
	static void run_shark_loop(RunLoop* loop) {
		loop->run();
	}
	static char* encoding_to_utf8(const uint16_t* src, int length, int* out_len) {
		auto buff = Codec::encoding(Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}
	static uint16_t* decoding_utf8_to_uint16(const char* src, int length, int* out_len) {
		auto buff = Codec::decoding_to_uint16(Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}
	static void print(const char* msg, ...) {
		XX_STRING_FORMAT(msg, str);
		LOG(str);
	}
	static bool is_process_exit() {
		return RunLoop::is_process_exit();
	}
};

int start(cString& argv_str) {
	
	static int is_initializ = 0;
	if ( is_initializ++ == 0 ) {
		HttpHelper::initialize();
		node::set_shark_api({
			SharkApiImplementation::create_shark_js_worker,
			SharkApiImplementation::delete_shark_js_worker,
			SharkApiImplementation::shark_main_loop,
			SharkApiImplementation::run_shark_loop,
			SharkApiImplementation::encoding_to_utf8,
			SharkApiImplementation::decoding_utf8_to_uint16,
			SharkApiImplementation::print,
			SharkApiImplementation::is_process_exit,
		});
		ObjectAllocator allocator = {
			ObjectAllocatorImpl::alloc,
			ObjectAllocatorImpl::release,
			ObjectAllocatorImpl::retain,
		};
		set_object_allocator(&allocator);
		shark::set_ssl_root_x509_store_function(node::crypto::NewRootCertStore);
	}
	
	String str = argv_str.trim();
	// add prefix
	if (argv_str.index_of("node") != 0 && argv_str.index_of("shark") != 0) {
		str = String("node ") + str;
	}
	
	char* str2 = const_cast<char*>(*str);
	Array<char*> argv = { str2 };
	
	int index = 0;
	while ((index = str.index_of(" ", index)) != -1) {
		str2[index] = '\0';
		index++;
		argv.push(str2 + index);
	}
	
	return node::Start(argv.length(), const_cast<char**>(&argv[0]));
}

int start(const Array<String>& argv) {
	String argv_str;
	for ( auto& i : argv  ) {
		if (!i.value().is_blank()) {
			if ( !argv_str.is_empty() ) {
				argv_str += " ";
			}
			argv_str += i.value();
		}
	}
	return start(*argv_str);
}

/**
 * @func __default_main
 */
int __default_main(int argc, char** argv) {
	String path;

 #if XX_ANDROID
	path = Android::start_path();
	if ( path.is_empty() )
 #endif
	{
		FileReader* reader = FileReader::shared();
		path = Path::resources("index");
		Array<String> ls = String(reader->read_file_sync( path )).split('\n');
		path = String();
	
		for ( uint i = 0; i < ls.length(); i++ ) {
			String s = ls[i].trim();
			if ( s[0] != '#' ) {
				path = s;
				break;
			}
		}
	}
	if ( ! path.is_empty() ) {
		return start(path);
	}

	return 0;
}

XX_INIT_BLOCK(__default_main) {
	__xx_default_gui_main = __default_main;
}

JS_END
