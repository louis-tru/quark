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

#include "qgr/utils/http.h"
#include "qgr/js/wrap.h"
#include "cb-1.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

static cString const_url("url");
static cString const_method("method");
static cString const_headers("headers");
static cString const_post_data("postData");
static cString const_save("save");
static cString const_upload("upload");
static cString const_disable_ssl_verify("disableSslVerify");
static cString const_disable_cache("disableCache");
static cString const_disable_cookie("disableCookie");

typedef HttpClientRequest NativeHttpClientRequest;

/**
 * @class WrapNativeHttpClientRequest
 */
class WrapNativeHttpClientRequest: public WrapObject {
 public: 
	typedef HttpClientRequest Type;
	
	class Delegate: public Object, public HttpClientRequest::Delegate {
	 public:
		
		WrapNativeHttpClientRequest* _host;
	
		String trigger_error;
		String trigger_write;
		String trigger_header;
		String trigger_data;
		String trigger_end;
		String trigger_readystate_change;
		String trigger_timeout;
		String trigger_abort;
		
		WrapNativeHttpClientRequest* host() {
			return _host;
		}
		
		Worker* worker() {
			return _host->worker();
		}
		
		virtual void trigger_http_error(HttpClientRequest* req, cError& error) {
			if ( !trigger_error.is_empty() ) {
				HandleScope scope(worker());
				Local<JSValue> arg = worker()->New( error );
				host()->call( worker()->New(trigger_error, 1), 1, &arg );
			}
		}
		virtual void trigger_http_write(HttpClientRequest* req) {
			if ( !trigger_write.is_empty() ) {
				HandleScope scope(worker());
				host()->call( worker()->New(trigger_write, 1) );
			}
		}
		virtual void trigger_http_header(HttpClientRequest* req) {
			if ( !trigger_header.is_empty() ) {
				HandleScope scope(worker());
				host()->call( worker()->New(trigger_header, 1) );
			}
		}
		virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) {
			if ( !trigger_data.is_empty() ) {
				HandleScope scope(host()->worker());
				Local<JSValue> arg = worker()->New( move(buffer) );
				host()->call( worker()->New(trigger_data, 1), 1, &arg );
			}
		}
		virtual void trigger_http_end(HttpClientRequest* req) {
			if ( !trigger_end.is_empty() ) {
				HandleScope scope(worker());
				host()->call( worker()->New(trigger_end, 1) );
			}
		}
		virtual void trigger_http_readystate_change(HttpClientRequest* req) {
			if ( !trigger_readystate_change.is_empty() ) {
				HandleScope scope(worker());
				host()->call( worker()->New(trigger_readystate_change, 1) );
			}
		}
		virtual void trigger_http_timeout(HttpClientRequest* req) {
			if ( !trigger_timeout.is_empty() ) {
				HandleScope scope(worker());
				host()->call( worker()->New(trigger_timeout, 1) );
			}
		}
		virtual void trigger_http_abort(HttpClientRequest* req) {
			if ( !trigger_abort.is_empty() ) {
				HandleScope scope(worker());
				host()->call( worker()->New(trigger_abort, 1) );
			}
		}
		
	};
	
	Delegate* del() {
		return static_cast<Delegate*>(private_data());
	}
	
	virtual bool add_event_listener(cString& name, cString& func, int id) {
		
		Delegate* _del = del();
		if (!_del) {
			_del = new Delegate();
			_del->_host = this;
			self<Type>()->set_delegate(_del);
			set_private_data(_del, true);
		}
		
		if ( id != -1 ) return 0; // 只接收id==-1的监听器
		
		if ( name == "Error" ) {
			_del->trigger_error = func;
		} else if ( name == "Write" ) {
			_del->trigger_write = func;
		} else if ( name == "Header" ) {
			_del->trigger_header = func;
		} else if ( name == "Data" ) {
			_del->trigger_data = func;
		} else if ( name == "End" ) {
			_del->trigger_end = func;
		} else if ( name == "ReadystateChange" ) {
			_del->trigger_readystate_change = func;
		} else if ( name == "Timeout" ) {
			_del->trigger_timeout = func;
		} else if ( name == "Abort" ) {
			_del->trigger_abort = func;
		} else {
			return false;
		}
		return true;
	}
	
	virtual bool remove_event_listener(cString& name, int id) {
		
		Delegate* _del = del();
		
		if ( id != -1 || !_del ) return 0;
		
		if ( name == "Error" ) {
			_del->trigger_error = Ucs2String();
		} else if ( name == "Write" ) {
			_del->trigger_write = Ucs2String();
		} else if ( name == "Header" ) {
			_del->trigger_header = Ucs2String();
		} else if ( name == "Data" ) {
			_del->trigger_data = Ucs2String();
		} else if ( name == "End" ) {
			_del->trigger_end = Ucs2String();
		} else if ( name == "ReadystateChange" ) {
			_del->trigger_readystate_change = Ucs2String();
		} else if ( name == "Timeout" ) {
			_del->trigger_timeout = Ucs2String();
		} else if ( name == "Abort" ) {
			_del->trigger_abort = Ucs2String();
		} else {
			return false;
		}
		return true;
	}
	
	static void constructor(FunctionCall args) {
		New<WrapNativeHttpClientRequest>(args, new HttpClientRequest());
	}
	
	/**
	 * @func set_method(method)
	 * @arg method {HttpMethod}
	 */
	static void set_method(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsUint32(worker)) {
			JS_THROW_ERR(
				"* @func setMethod(method)\n"
				"* @arg method {HttpMethod}\n"
			);
		}
		uint32 arg = args[0]->ToUint32Value(worker);
		HttpMethod method = arg > 4 ? HTTP_METHOD_GET: (HttpMethod)arg;
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->set_method(method); }, Error);
	}
	
	/**
	 * @func set_url(url)
	 * @arg url {String}
	 */
	static void set_url(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func setUrl(url)\n"
				"* @arg url {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->set_url(arg); }, Error);
	}
	
	/**
	 * @func set_save_path(path)
	 * @arg path {String}
	 */
	static void set_save_path(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func setSavePath(path)\n"
				"* @arg path {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->set_save_path(arg); }, Error);
	}
	
	/**
	 * @func set_username(username)
	 * @arg username {String}
	 */
	static void set_username(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func setUsername(username)\n"
				"* @arg username {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->set_username(arg); }, Error);
	}
	
	/**
	 * @func set_password(password)
	 * @arg password {String}
	 */
	static void set_password(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func setPassword(password)\n"
				"* @arg password {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->set_password(arg); }, Error);
	}
	
	/**
	 * @func disable_cache(disable)
	 * @arg disable {bool}
	 */
	static void disable_cache(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1) {
			JS_THROW_ERR(
				"* @func disableCache(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		bool arg = args[0]->ToBooleanValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->disable_cache(arg); }, Error);
	}
	
	/**
	 * @func disable_cookie(disable)
	 * @arg disable {bool}
	 */
	static void disable_cookie(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1) {
			JS_THROW_ERR(
				"* @func disableCookie(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		bool arg = args[0]->ToBooleanValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->disable_cookie(arg); }, Error);
	}
	
	/**
	 * @func disable_send_cookie(disable)
	 * @arg disable {bool}
	 */
	static void disable_send_cookie(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1) {
			JS_THROW_ERR(
				"* @func disableSendCookie(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		uint32 arg = args[0]->ToBooleanValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->disable_send_cookie(arg); }, Error);
	}
	
	/**
	 * @func disable_ssl_verify(disable)
	 * @arg disable {bool}
	 */
	static void disable_ssl_verify(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1) {
			JS_THROW_ERR(
				"* @func disableSslVerify(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({
			self->disable_ssl_verify(args[0]->ToBooleanValue(worker));
		}, Error);
	}
	
	/**
	 * @func set_request_header(header_name, value)
	 * @arg header_name {String} ascii string
	 * @arg value {String}
	 */
	static void set_request_header(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func setRequestHeader(header_name, value)\n"
				"* @arg header_name {String} ascii string\n"
				"* @arg value {String}\n"
			);
		}
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({
			self->set_request_header( args[0]->ToStringValue(worker,1), args[1]->ToStringValue(worker));
		}, Error);
	}
	
	/**
	 * @func set_form(form_name, value)
	 * @arg form_name {String}
	 * @arg value {String}
	 */
	static void set_form(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker) ) {
			JS_THROW_ERR(
				"* @func setForm(form_name, value)\n"
				"* @arg form_name {String}\n"
				"* @arg value {String}\n"
			);
		}
		String form_name = args[0]->ToStringValue(worker);
		String value = args[1]->ToStringValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({
			self->set_form(form_name, value);
		}, Error);
	}
	
	/**
	 * @func set_upload_file(form_name, local_path)
	 * @arg form_name {String}
	 * @arg local_path {String}
	 */
	static void set_upload_file(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker) ) {
			JS_THROW_ERR(
				"* @func setUploadFile(form_name, local_path)\n"
				"* @arg form_name {String}\n"
				"* @arg local_path {String}\n"
			);
		}
		String form_name = args[0]->ToStringValue(worker);
		String local_path = args[1]->ToStringValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({
			self->set_upload_file(form_name, local_path);
		}, Error);
	}
	
	/**
	 * @func clear_request_header()
	 */
	static void clear_request_header(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->clear_request_header(); }, Error);
	}
	
	/**
	 * @func clear_form_data()
	 */
	static void clear_form_data(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->clear_form_data(); }, Error);
	}
	
	/**
	 * @func get_response_header(header_name)
	 * @arg header_name {String}
	 * @ret {String}
	 */
	static void get_response_header(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func getResponseHeader(header_name)\n"
				"* @arg header_name {String}\n"
				"* @ret {String}\n"
			);
		}
		JS_SELF(HttpClientRequest);
		String rv;
		JS_TRY_CATCH({
			rv = self->get_response_header(args[0]->ToStringValue(worker,1));
		}, Error);
		JS_RETURN( rv );
	}
	
	/**
	 * @func get_all_response_headers()
	 * @ret {Object}
	 */
	static void get_all_response_headers(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		const Map<String, String>* rv;
		JS_TRY_CATCH({ rv = &self->get_all_response_headers(); }, Error);
		JS_RETURN( *rv );
	}
	
	/**
	 * @func set_keep_alive(keep_alive)
	 * @arg keep_alive {bool}
	 */
	static void set_keep_alive(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0) {
			JS_THROW_ERR(
				"* @func setKeepAlive(keep_alive)\n"
				"* @arg keep_alive {bool}\n"
			);
		}
		bool enable = args[0]->ToBooleanValue(worker);
		JS_SELF(HttpClientRequest);
		JS_TRY_CATCH({ self->set_keep_alive(enable); }, Error);
	}

	/**
	 * @func set_timeout(time)
	 * @arg time {uint} ms
	 */
	static void set_timeout(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsNumber(worker)) {
			JS_THROW_ERR(
				"* @func setTimeout(time)\n"
				"* @arg time {uint} ms\n"
			);
		}
		JS_SELF(HttpClientRequest);
		
		uint64 time = args[0]->ToUint32Value(worker) * 1000;
		
		JS_TRY_CATCH({ self->set_timeout(time); }, Error);
	}

	/**
	 * @get upload_total {uint}
	 */
	static void upload_total(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_RETURN( self->upload_total() );
	}

	/**
	 * @get upload_size {uint}
	 */
	static void upload_size(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_RETURN( self->upload_size() );
	}

	/**
	 * @get download_total {uint}
	 */
	static void download_total(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_RETURN( self->download_total() );
	}

	/**
	 * @get download_size {uint}
	 */
	static void download_size(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_RETURN( self->download_size() );
	}

	/**
	 * @get download_size {HttpReadyState}
	 */
	static void ready_state(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_RETURN( self->ready_state() );
	}

	/**
	 * @get status_code {int}
	 */
	static void status_code(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_RETURN( self->status_code() );
	}

	/**
	 * @get url {String}
	 */
	static void url(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		JS_RETURN( self->url() );
	}

	/**
	 * @func send([data])
	 * @arg [data] {String|ArrayBuffer|Buffer}
	 */
	static void send(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(HttpClientRequest);
		if (args.Length() == 0) {
			JS_TRY_CATCH({ self->send(); }, Error);
		} else {
			if (args[0]->IsString(worker)) {
				JS_TRY_CATCH({
					self->send( args[0]->ToStringValue(worker).collapse_buffer() );
				}, Error);
			}
			else if (worker->has_buffer(args[0])) {
				WeakBuffer buff = worker->as_buffer(args[0]);
				JS_TRY_CATCH({ self->send(buff.copy()); }, Error);
			}
			else {
				JS_TRY_CATCH({ self->send(); }, Error );
			}
		}
	}

	/**
	 * @func pause()
	 */
	static void pause(FunctionCall args) {
		JS_SELF(HttpClientRequest);
		self->pause();
	}

	/**
	 * @func resume()
	 */
	static void resume(FunctionCall args) {
		JS_SELF(HttpClientRequest);
		self->resume();
	}

	/**
	 * @func abort()
	 */
	static void abort(FunctionCall args) {
		JS_SELF(HttpClientRequest);
		self->abort();
	}
	
	/**
	 * @func binding
	 */
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(NativeHttpClientRequest, constructor, {
			JS_SET_CLASS_METHOD(setMethod, set_method);
			JS_SET_CLASS_METHOD(setUrl, set_url);
			JS_SET_CLASS_METHOD(setSavePath, set_save_path);
			JS_SET_CLASS_METHOD(setUsername, set_username);
			JS_SET_CLASS_METHOD(setPassword, set_password);
			JS_SET_CLASS_METHOD(disableCache, disable_cache);
			JS_SET_CLASS_METHOD(disableCookie, disable_cookie);
			JS_SET_CLASS_METHOD(disableSendCookie, disable_send_cookie);
			JS_SET_CLASS_METHOD(disableSslVerify, disable_ssl_verify);
			JS_SET_CLASS_METHOD(setKeepAlive, set_keep_alive);
			JS_SET_CLASS_METHOD(setTimeout, set_timeout);
			JS_SET_CLASS_METHOD(setRequestHeader, set_request_header);
			JS_SET_CLASS_METHOD(setForm, set_form);
			JS_SET_CLASS_METHOD(setUploadFile, set_upload_file);
			JS_SET_CLASS_METHOD(clearRequestHeader, clear_request_header);
			JS_SET_CLASS_METHOD(clearFormData, clear_form_data);
			JS_SET_CLASS_METHOD(getResponseHeader, get_response_header);
			JS_SET_CLASS_METHOD(getAllResponseHeaders, get_all_response_headers);
			JS_SET_CLASS_ACCESSOR(uploadTotal, upload_total);
			JS_SET_CLASS_ACCESSOR(uploadSize, upload_size);
			JS_SET_CLASS_ACCESSOR(downloadTotal, download_total);
			JS_SET_CLASS_ACCESSOR(downloadSize, download_size);
			JS_SET_CLASS_ACCESSOR(readyState, ready_state);
			JS_SET_CLASS_ACCESSOR(statusCode, status_code);
			JS_SET_CLASS_ACCESSOR(url, url);
			JS_SET_CLASS_METHOD(send, send);
			JS_SET_CLASS_METHOD(pause, pause);
			JS_SET_CLASS_METHOD(resume, resume);
			JS_SET_CLASS_METHOD(abort, abort);
		}, nullptr);
	}
};

/**
 * @class NativeHttp
 */
class NativeHttp {
public:
	typedef HttpHelper::RequestOptions RequestOptions;

	static bool get_options(Worker* worker, Local<JSValue> arg, RequestOptions& opt) {
		Local<JSObject> obj = arg.To<JSObject>();
		
		opt = {
			String(),
			HTTP_METHOD_GET,
			Map<String, String>(),
			Buffer(),
			String(),
			String(),
			false,
			false,
			false,
		};
		Local<JSValue> value;
		
		value = obj->Get(worker, worker->New(const_url,1));
		if ( value.IsEmpty() ) return false;
		if ( value->IsString(worker) ) opt.url = value->ToStringValue(worker);
		
		value = obj->Get(worker, worker->New(const_method,1));
		if ( value.IsEmpty() ) return false;
		if ( value->IsUint32(worker) ) {
			uint32 arg = value->ToUint32Value(worker);
			opt.method = arg > 4 ? HTTP_METHOD_GET: (HttpMethod)arg;
		}
		
		value = obj->Get(worker, worker->New(const_headers,1));
		if ( value.IsEmpty() ) return false;
		if (!value.To<JSObject>()->ToStringMap(worker).To(opt.headers)) return false;
		
		value = obj->Get(worker, worker->New(const_post_data,1));
		if ( value.IsEmpty() ) return false;
		if ( value->IsString(worker) ) {
			opt.post_data = value->ToStringValue(worker).collapse_buffer();
		}
		else if (worker->has_buffer(value)) {
			opt.post_data = worker->as_buffer(value).copy();
		}
		
		value = obj->Get(worker, worker->New(const_save,1));
		if ( value.IsEmpty() ) return false;
		if ( value->IsString(worker) ) {
			opt.save = value->ToStringValue(worker);
		}
		
		value = obj->Get(worker, worker->New(const_upload,1));
		if ( value.IsEmpty() ) return false;
		if ( value->IsString(worker) ) {
			opt.upload = value->ToStringValue(worker);
		}
		
		value = obj->Get(worker, worker->New(const_disable_ssl_verify,1));
		if ( value.IsEmpty() ) return false;
		opt.disable_ssl_verify = value->ToBooleanValue(worker);
		
		value = obj->Get(worker, worker->New(const_disable_cache,1));
		if ( value.IsEmpty() ) return false;
		opt.disable_cache = value->ToBooleanValue(worker);
		
		value = obj->Get(worker, worker->New(const_disable_cache,1));
		if ( value.IsEmpty() ) return false;
		opt.disable_cookie = value->ToBooleanValue(worker);
		
		return true;
	}

	template<bool stream> static void request(FunctionCall args, cchar* argument) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsObject(worker)) {
			JS_THROW_ERR(argument);
		}
		JS_HANDLE_SCOPE();
		uint rev = 0;
		RequestOptions opt;
		if (!get_options(worker, args[0], opt))
			return;
		
		Callback cb;
		
		if ( args.Length() > 1 ) {
			cb = stream ? get_callback_for_io_stream_http_error(worker, args[1]) :
										get_callback_for_buffer_http_error(worker, args[1]);
		}

		JS_TRY_CATCH({
			if ( stream ) {
				rev = HttpHelper::request_stream(opt, cb);
			} else {
				rev = HttpHelper::request(opt, cb);
			}
		}, HttpError);
		
		JS_RETURN( rev );
	}
	
	/**
	 * @func request(options[,cb])
	 * @arg options {RequestOptions}
	 * @arg [cb] {Function}
	 * @ret {uint} return req id
	 */
	static void request(FunctionCall args) {
		request<false>(args,
			"* @func request(options[,cb])\n"
			"* @arg options {RequestOptions}\n"
			"* @arg [cb] {Function}\n"
			"* @ret {uint} return req id\n"
		);
	}
	
	/**
	 * @func request_stream(options[,cb])
	 * @arg options {RequestOptions}
	 * @arg [cb] {Function}
	 * @ret {uint} return req id
	 */
	static void request_stream(FunctionCall args) {
		request<true>(args, 
			"* @func requestStream(options[,cb])\n"
			"* @arg options {RequestOptions}\n"
			"* @arg [cb] {Function}\n"
			"* @ret {uint} return req id\n"
		);
	}
	
	/**
	 * @func request_sync(url)
	 * @arg url {String}
	 * @ret {Buffer}
	 */
	static void request_sync(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsObject(worker)) {
			JS_THROW_ERR(
				"* @func requestSync(url)\n"
				"* @arg url {String}\n"
				"* @ret {Buffer}\n"
			);
		}
		
		RequestOptions opt;
		if (!get_options(worker, args[0], opt))
			return;
		
		JS_TRY_CATCH({
			JS_RETURN( HttpHelper::request_sync(opt) );
		}, HttpError);
	}
	
	/**
	 * @func download(url,save[,cb])
	 * @arg url {String}
	 * @arg save {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return req id
	 */
	static void download(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func download(url,save[,cb])\n"
				"* @arg url {String}\n"
				"* @arg save {String}\n"
				"* @arg [cb] {Function}\n"
				"* @ret {uint} return req id\n"
			);
		}
		uint rev = 0;
		String url = args[0]->ToStringValue(worker);
		String save = args[1]->ToStringValue(worker);
		Callback cb;
		
		if ( args.Length() > 2 ) {
			cb = get_callback_for_buffer_http_error(worker, args[2]);
		}
		JS_TRY_CATCH({
			rev = HttpHelper::download(url, save, cb);
		}, HttpError);
		JS_RETURN( rev );
	}
	
	/**
	 * @func download_sync(url,save)
	 * @arg url {String}
	 * @arg save {String}
	 */
	static void download_sync(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func downloadSync(url,save)\n"
				"* @arg url {String}\n"
				"* @arg save {String}\n"
			);
		}
		String url = args[0]->ToStringValue(worker);
		String save = args[1]->ToStringValue(worker);
		JS_TRY_CATCH({
			HttpHelper::download_sync(url, save);
		}, HttpError);
	}
		
	/**
	 * @func upload(url,local_path[,cb])
	 * @arg url {String}
	 * @arg local_path {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return req id
	 */
	static void upload(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func upload(url,local_path[,cb])\n"
				"* @arg url {String}\n"
				"* @arg local_path {String}\n"
				"* @arg [cb] {Function}\n"
				"* @ret {uint} return req id\n"
			);
		}
		uint rev = 0;
		String url = args[0]->ToStringValue(worker);
		String file = args[1]->ToStringValue(worker);
		Callback cb;
		
		if ( args.Length() > 2 ) {
			cb = get_callback_for_buffer_http_error(worker, args[2]);
		}
		JS_TRY_CATCH({
			rev = HttpHelper::upload(url, file, cb);
		}, HttpError);
		JS_RETURN( rev );
	}
	
	/**
	 * @func upload_sync(url,local_path)
	 * @arg url {String}
	 * @arg local_path {String}
	 * @ret {Buffer}
	 */
	static void upload_sync(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func uploadSync(url,local_path)\n"
				"* @arg url {String}\n"
				"* @arg local_path {String}\n"
				"* @ret {Buffer}\n"
			);
		}
		String url = args[0]->ToStringValue(worker);
		String file = args[1]->ToStringValue(worker);
		JS_TRY_CATCH({
			JS_RETURN( HttpHelper::upload_sync(url, file) );
		}, HttpError);
	}
	
	template<bool stream> static void get(FunctionCall args, cchar* argument) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(argument);
		}
		uint rev = 0;
		String url = args[0]->ToStringValue(worker);
		Callback cb;
		
		if ( args.Length() > 1 ) {
			cb = stream ? get_callback_for_io_stream_http_error(worker, args[1]) :
										get_callback_for_buffer_http_error(worker, args[1]);
		}
		
		if ( stream ) {
			JS_TRY_CATCH({ rev = HttpHelper::get_stream(url, cb); }, HttpError);
		} else {
			JS_TRY_CATCH({ rev = HttpHelper::get(url, cb); }, HttpError);
		}
		JS_RETURN( rev );
	}

	/**
	 * @func get(url[,cb])
	 * @arg url {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return req id
	 */
	static void get(FunctionCall args) {
		get<false>(args, 
			"* @func get(url[,cb])\n"
			"* @arg url {String}\n"
			"* @arg [cb] {Function}\n"
			"* @ret {uint} return req id\n"
							 );
	}
	
	/**
	 * @func get_stream(url[,cb])
	 * @arg url {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return req id
	 */
	static void get_stream(FunctionCall args) {
		get<true>(args, 
			"* @func getStream(url[,cb])\n"
			"* @arg url {String}\n"
			"* @arg [cb] {Function}\n"
			"* @ret {uint} return req id\n"
							);
	}

	/**
	 * @func post(url,data[,cb])
	 * @arg url {String}
	 * @arg data {String|ArrayBuffer|Buffer}
	 * @arg [cb] {Function}
	 * @ret {uint} return req id
	 */
	static void post(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() < 2 || ! args[0]->IsString(worker) ||
				!(args[1]->IsString(worker) || worker->has_buffer(args[1]))
		) {
			JS_THROW_ERR(
				"* @func post(url,data[,cb])\n"
				"* @arg url {String}\n"
				"* @arg data {String|ArrayBuffer|Buffer}\n"
				"* @arg [cb] {Function}\n"
				"* @ret {uint} return req id\n"
			);
		}
		uint rev = 0;
		String url = args[0]->ToStringValue(worker);
		Callback cb;
		
		if ( args.Length() > 2 ) {
			cb = get_callback_for_buffer_http_error(worker, args[2]);
		}
		
		JS_TRY_CATCH({
			if (args[1]->IsString(worker)) {
				rev = HttpHelper::post(url, args[1]->ToStringValue(worker).collapse_buffer(), cb);
			} 
			else {
				WeakBuffer buff = worker->as_buffer(args[1]);
				rev = HttpHelper::post(url, buff.copy(), cb);
			}
		}, HttpError);
		JS_RETURN( rev );
	}

	/**
	 * @func get_sync(url)
	 * @arg url {String}
	 * @ret {Buffer}
	 */
	static void get_sync(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func getSync(url)\n"
				"* @arg url {String}\n"
				"* @ret {Buffer}\n"
			);
		}
		String url = args[0]->ToStringValue(worker);
		JS_TRY_CATCH({ JS_RETURN( HttpHelper::get_sync(url) ); }, HttpError);
	}
	
	/**
	 * @func post_sync(url,data)
	 * @arg url {String}
	 * @arg data {String|ArrayBuffer|Buffer}
	 * @ret {Buffer}
	 */
	static void post_sync(FunctionCall args) {
		JS_WORKER(args);
		if (  args.Length() < 2 || !args[0]->IsString(worker) ||
				!(args[1]->IsString(worker) || worker->has_buffer(args[1])
				)
		) {
			JS_THROW_ERR(
				"* @func postSync(url,data)\n"
				"* @arg url {String}\n"
				"* @arg data {String|ArrayBuffer|Buffer}\n"
				"* @ret {Buffer}\n"
			);
		}
		
		String url = args[0]->ToStringValue(worker);
		Buffer rev;
		
		JS_TRY_CATCH({
			if (args[1]->IsString(worker)) {
				rev = HttpHelper::post_sync(url, args[1]->ToStringValue(worker).collapse_buffer());
			}
			else {
				WeakBuffer buff = worker->as_buffer(args[1]);
				rev = HttpHelper::post_sync(url, buff.copy());
			}
		}, HttpError);
		JS_RETURN( move(rev) );
	}

	/**
	 * @func abort(id)
	 * @arg id {uint} abort id
	 */
	static void abort(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || !args[0]->IsUint32(worker) ) {
			JS_THROW_ERR(
				"* @func abort(id)\n"
				"* @arg id {uint} abort id\n"
			);
		}
		HttpHelper::abort( args[0]->ToUint32Value(worker) );
	}

	/**
	 * @func user_agent()
	 * @ret {String}
	 */
	static void user_agent(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( HttpHelper::user_agent() );
	}

	/**
	 * @func set_user_agent(user_agent)
	 * @arg user_agent {String}
	 */
	static void set_user_agent(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsString(worker)) {
			JS_THROW_ERR("Bad argument");
		}
		HttpHelper::set_user_agent( args[0]->ToStringValue(worker) );
	}

	/**
	 * @func ssl_cacert_file()
	 * @ret {String} return cacert file path
	 */
	static void ssl_cacert_file(FunctionCall args) {
		JS_WORKER(args);
		// JS_RETURN( HttpHelper::ssl_cacert_file() );
	}

//  /**
//   * @func ssl_cacert_file(path)
//   * @arg path {String}
//   */
//  static void set_ssl_cacert_file(FunctionCall args) {
//    JS_WORKER(args);
//    if (args.Length() == 0 || !args[0]->IsString(worker)) {
//      JS_THROW_ERR(
//        "* @func sslCacertFile(path)\n"
//        "* @arg path {String}\n"
//      );
//    }
//    HttpHelper::set_ssl_cacert_file( args[0]->ToStringValue(worker) );
//  }
//
//  /**
//   * @func set_ssl_client_key_file(path)
//   * @arg path {String}
//   */
//  static void set_ssl_client_key_file(FunctionCall args) {
//    JS_WORKER(args);
//    if (args.Length() == 0 || !args[0]->IsString(worker)) {
//      JS_THROW_ERR(
//        "* @func setSslClientKeyfile(path)\n"
//        "* @arg path {String}\n"
//      );
//    }
//    HttpHelper::set_ssl_client_key_file( args[0]->ToStringValue(worker) );
//  }
//
//  /**
//   * @func set_ssl_client_keypasswd(password)
//   * @arg password {String}
//   */
//  static void set_ssl_client_keypasswd(FunctionCall args) {
//    JS_WORKER(args);
//    if (args.Length() == 0 || !args[0]->IsString(worker)) {
//      JS_THROW_ERR(
//        "* @func setSslClientKeypasswd(password)\n"
//        "* @arg password {String}\n"
//      );
//    }
//    HttpHelper::set_ssl_client_keypasswd( args[0]->ToStringValue(worker) );
//  }

	/**
	 * @func cache_path()
	 * @ret {String}
	 */
	static void cache_path(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( HttpHelper::cache_path() );
	}

	/**
	 * @func set_cache_path(path)
	 * @arg path {String}
	 */
	static void set_cache_path(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func setCachePath(path)\n"
				"* @arg path {String}\n"
			);
		}
		HttpHelper::set_cache_path( args[0]->ToStringValue(worker) );
	}

	/**
	 * @func clear_cache()
	 */
	static void clear_cache(FunctionCall args) {
		HttpHelper::clear_cache();
	}

	/**
	 * @func clear_cookie()
	 */
	static void clear_cookie(FunctionCall args) {
		HttpHelper::clear_cookie();
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		WrapNativeHttpClientRequest::binding(exports, worker);
		//
		JS_SET_PROPERTY(HTTP_METHOD_GET, HTTP_METHOD_GET);
		JS_SET_PROPERTY(HTTP_METHOD_POST, HTTP_METHOD_POST);
		JS_SET_PROPERTY(HTTP_METHOD_HEAD, HTTP_METHOD_HEAD);
		JS_SET_PROPERTY(HTTP_METHOD_DELETE, HTTP_METHOD_DELETE);
		JS_SET_PROPERTY(HTTP_METHOD_PUT, HTTP_METHOD_PUT);
		//
		JS_SET_PROPERTY(HTTP_READY_STATE_INITIAL, HTTP_READY_STATE_INITIAL);
		JS_SET_PROPERTY(HTTP_READY_STATE_READY, HTTP_READY_STATE_READY);
		JS_SET_PROPERTY(HTTP_READY_STATE_SENDING, HTTP_READY_STATE_SENDING);
		JS_SET_PROPERTY(HTTP_READY_STATE_RESPONSE, HTTP_READY_STATE_RESPONSE);
		JS_SET_PROPERTY(HTTP_READY_STATE_COMPLETED, HTTP_READY_STATE_COMPLETED);
		//
		JS_SET_METHOD(request, request);
		JS_SET_METHOD(requestStream, request_stream);
		JS_SET_METHOD(requestSync, request_sync);
		JS_SET_METHOD(download, download);
		JS_SET_METHOD(upload, upload);
		JS_SET_METHOD(get, get);
		JS_SET_METHOD(post, post);
		JS_SET_METHOD(getSync, get_sync);
		JS_SET_METHOD(postSync, post_sync);
		JS_SET_METHOD(abort, abort);
		JS_SET_METHOD(userAgent, user_agent);
		JS_SET_METHOD(setUserAgent, set_user_agent);
		JS_SET_METHOD(cachePath, cache_path);
		JS_SET_METHOD(setCachePath, set_cache_path);
		JS_SET_METHOD(clearCache, clear_cache);
		JS_SET_METHOD(clearCookie, clear_cookie);
		JS_SET_METHOD(downloadSync, download_sync);
		JS_SET_METHOD(uploadSync, upload_sync);
		
		//JS_SET_METHOD(get_stream, get_stream);
		//JS_SET_METHOD(sslCacertFile, ssl_cacert_file);
		//JS_SET_METHOD(setSslCacertFile, set_ssl_cacert_file);
		//JS_SET_METHOD(set_ssl_client_key_file, set_ssl_client_key_file);
		//JS_SET_METHOD(set_ssl_client_keypasswd, set_ssl_client_keypasswd);
	}
};

JS_REG_MODULE(_http, NativeHttp);
JS_END
