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

#include "../../util/http.h"
#include "../js.h"
#include "./_cb.h"

/**
 * @ns qk::js
 */

Js_BEGIN

static cString const_url("url");
static cString const_method("method");
static cString const_headers("headers");
static cString const_post_data("postData");
static cString const_save("save");
static cString const_upload("upload");
static cString const_timeout("timeout");
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
				CallbackScope cscope(worker());
				Local<JSValue> arg = worker()->New( error );
				host()->call( worker()->New(trigger_error, 1), 1, &arg );
			}
		}
		virtual void trigger_http_write(HttpClientRequest* req) {
			if ( !trigger_write.is_empty() ) {
				HandleScope scope(worker());
				CallbackScope cscope(worker());
				host()->call( worker()->New(trigger_write, 1) );
			}
		}
		virtual void trigger_http_header(HttpClientRequest* req) {
			if ( !trigger_header.is_empty() ) {
				HandleScope scope(worker());
				CallbackScope cscope(worker());
				host()->call( worker()->New(trigger_header, 1) );
			}
		}
		virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) {
			if ( !trigger_data.is_empty() ) {
				HandleScope scope(host()->worker());
				CallbackScope cscope(worker());
				Local<JSValue> arg = worker()->New( move(buffer) );
				host()->call( worker()->New(trigger_data, 1), 1, &arg );
			}
		}
		virtual void trigger_http_end(HttpClientRequest* req) {
			if ( !trigger_end.is_empty() ) {
				HandleScope scope(worker());
				CallbackScope cscope(worker());
				host()->call( worker()->New(trigger_end, 1) );
			}
		}
		virtual void trigger_http_readystate_change(HttpClientRequest* req) {
			if ( !trigger_readystate_change.is_empty() ) {
				HandleScope scope(worker());
				CallbackScope cscope(worker());
				host()->call( worker()->New(trigger_readystate_change, 1) );
			}
		}
		virtual void trigger_http_timeout(HttpClientRequest* req) {
			if ( !trigger_timeout.is_empty() ) {
				HandleScope scope(worker());
				CallbackScope cscope(worker());
				host()->call( worker()->New(trigger_timeout, 1) );
			}
		}
		virtual void trigger_http_abort(HttpClientRequest* req) {
			if ( !trigger_abort.is_empty() ) {
				HandleScope scope(worker());
				CallbackScope cscope(worker());
				host()->call( worker()->New(trigger_abort, 1) );
			}
		}
		
	};
	
	Delegate* del() {
		return static_cast<Delegate*>(privateData());
	}
	
	virtual bool addEventListener(cString& name, cString& func, int id) {
		
		Delegate* _del = del();
		if (!_del) {
			_del = new Delegate();
			_del->_host = this;
			self<Type>()->set_delegate(_del);
			setPrivateData(_del, true);
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
	
	virtual bool removeEventListener(cString& name, int id) {
		
		Delegate* _del = del();
		
		if ( id != -1 || !_del ) return 0;
		
		if ( name == "Error" ) {
			_del->trigger_error = String2();
		} else if ( name == "Write" ) {
			_del->trigger_write = String2();
		} else if ( name == "Header" ) {
			_del->trigger_header = String2();
		} else if ( name == "Data" ) {
			_del->trigger_data = String2();
		} else if ( name == "End" ) {
			_del->trigger_end = String2();
		} else if ( name == "ReadystateChange" ) {
			_del->trigger_readystate_change = String2();
		} else if ( name == "Timeout" ) {
			_del->trigger_timeout = String2();
		} else if ( name == "Abort" ) {
			_del->trigger_abort = String2();
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
		Js_Worker(args);
		if (args.Length() < 1 || !args[0]->IsUint32(worker)) {
			Js_Throw(
				"* @func setMethod(method)\n"
				"* @arg method {HttpMethod}\n"
			);
		}
		uint32 arg = args[0]->ToUint32Value(worker);
		HttpMethod method = arg > 4 ? HTTP_METHOD_GET: (HttpMethod)arg;
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->set_method(method); }, Error);
	}
	
	/**
	 * @func set_url(url)
	 * @arg url {String}
	 */
	static void set_url(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			Js_Throw(
				"* @func setUrl(url)\n"
				"* @arg url {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->set_url(arg); }, Error);
	}
	
	/**
	 * @func set_save_path(path)
	 * @arg path {String}
	 */
	static void set_save_path(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			Js_Throw(
				"* @func setSavePath(path)\n"
				"* @arg path {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->set_save_path(arg); }, Error);
	}
	
	/**
	 * @func set_username(username)
	 * @arg username {String}
	 */
	static void set_username(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			Js_Throw(
				"* @func setUsername(username)\n"
				"* @arg username {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->set_username(arg); }, Error);
	}
	
	/**
	 * @func set_password(password)
	 * @arg password {String}
	 */
	static void set_password(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			Js_Throw(
				"* @func setPassword(password)\n"
				"* @arg password {String}\n"
			);
		}
		String arg = args[0]->ToStringValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->set_password(arg); }, Error);
	}
	
	/**
	 * @func disable_cache(disable)
	 * @arg disable {bool}
	 */
	static void disable_cache(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1) {
			Js_Throw(
				"* @func disableCache(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		bool arg = args[0]->ToBooleanValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->disable_cache(arg); }, Error);
	}
	
	/**
	 * @func disable_cookie(disable)
	 * @arg disable {bool}
	 */
	static void disable_cookie(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1) {
			Js_Throw(
				"* @func disableCookie(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		bool arg = args[0]->ToBooleanValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->disable_cookie(arg); }, Error);
	}
	
	/**
	 * @func disable_send_cookie(disable)
	 * @arg disable {bool}
	 */
	static void disable_send_cookie(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1) {
			Js_Throw(
				"* @func disableSendCookie(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		uint32 arg = args[0]->ToBooleanValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->disable_send_cookie(arg); }, Error);
	}
	
	/**
	 * @func disable_ssl_verify(disable)
	 * @arg disable {bool}
	 */
	static void disable_ssl_verify(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1) {
			Js_Throw(
				"* @func disableSslVerify(disable)\n"
				"* @arg disable {bool}\n"
			);
		}
		Js_Self(HttpClientRequest);
		Js_Try_Catch({
			self->disable_ssl_verify(args[0]->ToBooleanValue(worker));
		}, Error);
	}
	
	/**
	 * @func set_request_header(header_name, value)
	 * @arg header_name {String} ascii string
	 * @arg value {String}
	 */
	static void set_request_header(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			Js_Throw(
				"* @func setRequestHeader(header_name, value)\n"
				"* @arg header_name {String} ascii string\n"
				"* @arg value {String}\n"
			);
		}
		Js_Self(HttpClientRequest);
		Js_Try_Catch({
			self->set_request_header( args[0]->ToStringValue(worker,1), args[1]->ToStringValue(worker));
		}, Error);
	}
	
	/**
	 * @func set_form(form_name, value)
	 * @arg form_name {String}
	 * @arg value {String}
	 */
	static void set_form(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker) ) {
			Js_Throw(
				"* @func setForm(form_name, value)\n"
				"* @arg form_name {String}\n"
				"* @arg value {String}\n"
			);
		}
		String form_name = args[0]->ToStringValue(worker);
		String value = args[1]->ToStringValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({
			self->set_form(form_name, value);
		}, Error);
	}
	
	/**
	 * @func set_upload_file(form_name, local_path)
	 * @arg form_name {String}
	 * @arg local_path {String}
	 */
	static void set_upload_file(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker) ) {
			Js_Throw(
				"* @func setUploadFile(form_name, local_path)\n"
				"* @arg form_name {String}\n"
				"* @arg local_path {String}\n"
			);
		}
		String form_name = args[0]->ToStringValue(worker);
		String local_path = args[1]->ToStringValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({
			self->set_upload_file(form_name, local_path);
		}, Error);
	}
	
	/**
	 * @func clear_request_header()
	 */
	static void clear_request_header(FunctionCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->clear_request_header(); }, Error);
	}
	
	/**
	 * @func clear_form_data()
	 */
	static void clear_form_data(FunctionCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->clear_form_data(); }, Error);
	}
	
	/**
	 * @func get_response_header(header_name)
	 * @arg header_name {String}
	 * @ret {String}
	 */
	static void get_response_header(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			Js_Throw(
				"* @func getResponseHeader(header_name)\n"
				"* @arg header_name {String}\n"
				"* @ret {String}\n"
			);
		}
		Js_Self(HttpClientRequest);
		String rv;
		Js_Try_Catch({
			rv = self->get_response_header(args[0]->ToStringValue(worker,1));
		}, Error);
		Js_Return( rv );
	}
	
	/**
	 * @func get_all_response_headers()
	 * @ret {Object}
	 */
	static void get_all_response_headers(FunctionCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		const Map<String, String>* rv;
		Js_Try_Catch({ rv = &self->get_all_response_headers(); }, Error);
		Js_Return( *rv );
	}
	
	/**
	 * @func set_keep_alive(keep_alive)
	 * @arg keep_alive {bool}
	 */
	static void set_keep_alive(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0) {
			Js_Throw(
				"* @func setKeepAlive(keep_alive)\n"
				"* @arg keep_alive {bool}\n"
			);
		}
		bool enable = args[0]->ToBooleanValue(worker);
		Js_Self(HttpClientRequest);
		Js_Try_Catch({ self->set_keep_alive(enable); }, Error);
	}

	/**
	 * @func set_timeout(time)
	 * @arg time {uint} ms
	 */
	static void set_timeout(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsNumber(worker)) {
			Js_Throw(
				"* @func setTimeout(time)\n"
				"* @arg time {uint} ms\n"
			);
		}
		Js_Self(HttpClientRequest);
		
		uint64_t time = args[0]->ToUint32Value(worker) * 1000;
		
		Js_Try_Catch({ self->set_timeout(time); }, Error);
	}

	/**
	 * @get upload_total {uint}
	 */
	static void upload_total(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->upload_total() );
	}

	/**
	 * @get upload_size {uint}
	 */
	static void upload_size(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->upload_size() );
	}

	/**
	 * @get download_total {uint}
	 */
	static void download_total(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->download_total() );
	}

	/**
	 * @get download_size {uint}
	 */
	static void download_size(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->download_size() );
	}

	/**
	 * @get download_size {HttpReadyState}
	 */
	static void ready_state(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->ready_state() );
	}

	/**
	 * @get status_code {int}
	 */
	static void status_code(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->status_code() );
	}

	static void http_response_version(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->http_response_version() );
	}

	/**
	 * @get url {String}
	 */
	static void url(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		Js_Return( self->url() );
	}

	/**
	 * @func send([data])
	 * @arg [data] {String|ArrayBuffer|Buffer}
	 */
	static void send(FunctionCall args) {
		Js_Worker(args);
		Js_Self(HttpClientRequest);
		if (args.Length() == 0) {
			Js_Try_Catch({ self->send(); }, Error);
		} else {
			if (args[0]->IsString(worker)) {
				Js_Try_Catch({
					self->send( args[0]->ToStringValue(worker).collapse_buffer() );
				}, Error);
			}
			else if ( args[0]->IsBuffer() ) {
				WeakBuffer buff = args[0]->AsBuffer(worker);
				Js_Try_Catch({ self->send(buff.copy()); }, Error);
			}
			else {
				Js_Try_Catch({ self->send(); }, Error );
			}
		}
	}

	/**
	 * @func pause()
	 */
	static void pause(FunctionCall args) {
		Js_Self(HttpClientRequest);
		self->pause();
	}

	/**
	 * @func resume()
	 */
	static void resume(FunctionCall args) {
		Js_Self(HttpClientRequest);
		self->resume();
	}

	/**
	 * @func abort()
	 */
	static void abort(FunctionCall args) {
		Js_Self(HttpClientRequest);
		self->abort();
	}

	/**
	 * @func binding
	 */
	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Define_Class(NativeHttpClientRequest, constructor, {
			Js_Set_Class_Method(setMethod, set_method);
			Js_Set_Class_Method(setUrl, set_url);
			Js_Set_Class_Method(setSavePath, set_save_path);
			Js_Set_Class_Method(setUsername, set_username);
			Js_Set_Class_Method(setPassword, set_password);
			Js_Set_Class_Method(disableCache, disable_cache);
			Js_Set_Class_Method(disableCookie, disable_cookie);
			Js_Set_Class_Method(disableSendCookie, disable_send_cookie);
			Js_Set_Class_Method(disableSslVerify, disable_ssl_verify);
			Js_Set_Class_Method(setKeepAlive, set_keep_alive);
			Js_Set_Class_Method(setTimeout, set_timeout);
			Js_Set_Class_Method(setRequestHeader, set_request_header);
			Js_Set_Class_Method(setForm, set_form);
			Js_Set_Class_Method(setUploadFile, set_upload_file);
			Js_Set_Class_Method(clearRequestHeader, clear_request_header);
			Js_Set_Class_Method(clearFormData, clear_form_data);
			Js_Set_Class_Method(getResponseHeader, get_response_header);
			Js_Set_Class_Method(getAllResponseHeaders, get_all_response_headers);
			Js_Set_Class_Accessor(uploadTotal, upload_total);
			Js_Set_Class_Accessor(uploadSize, upload_size);
			Js_Set_Class_Accessor(downloadTotal, download_total);
			Js_Set_Class_Accessor(downloadSize, download_size);
			Js_Set_Class_Accessor(readyState, ready_state);
			Js_Set_Class_Accessor(statusCode, status_code);
			Js_Set_Class_Accessor(url, url);
			Js_Set_Class_Accessor(httpResponseVersion, http_response_version);
			Js_Set_Class_Method(send, send);
			Js_Set_Class_Method(pause, pause);
			Js_Set_Class_Method(resume, resume);
			Js_Set_Class_Method(abort, abort);
		}, nullptr);
	}
};

/**
 * @class NativeHttp
 */
class NativeHttp {
	public:
	typedef http_RequestOptions RequestOptions;

	static bool get_options(Worker* worker, Local<JSValue> arg, RequestOptions& opt) {
		Js_Handle_Scope();
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
		else if ( value->IsBuffer() ) {
			opt.post_data = value->AsBuffer(worker).copy();
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

		value = obj->Get(worker, worker->New(const_timeout,1));
		if ( value.IsEmpty() ) return false;
		if ( value->IsUint32(worker) ) {
			opt.timeout = value->ToUint32Value(worker) * 1e3;
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

	template<bool stream> static void request(FunctionCall args, cChar* argument) {
		Js_Worker(args);
		if (args.Length() == 0 || ! args[0]->IsObject(worker)) {
			Js_Throw(argument);
		}
		Js_Handle_Scope();
		uint32_t rev = 0;
		RequestOptions opt;
		if (!get_options(worker, args[0], opt))
			return;
		
		Cb cb;
		
		if ( args.Length() > 1 ) {
			cb = stream ? get_callback_for_io_stream_http_error(worker, args[1]) :
										get_callback_for_response_data_http_error(worker, args[1]);
		}

		Js_Try_Catch({
			if ( stream ) {
				rev = http_request_stream(opt, cb);
			} else {
				rev = http_request(opt, cb);
			}
		}, HttpError);
		
		Js_Return( rev );
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
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsObject(worker)) {
			Js_Throw(
				"* @func requestSync(url)\n"
				"* @arg url {String}\n"
				"* @ret {Buffer}\n"
			);
		}
		
		RequestOptions opt;
		if (!get_options(worker, args[0], opt))
			return;
		
		Js_Try_Catch({
			Js_Return( http_request_sync(opt) );
		}, HttpError);
	}

	/**
	 * @func abort(id)
	 * @arg id {uint} abort id
	 */
	static void abort(FunctionCall args) {
		Js_Worker(args);
		if ( args.Length() == 0 || !args[0]->IsUint32(worker) ) {
			Js_Throw(
				"* @func abort(id)\n"
				"* @arg id {uint} abort id\n"
			);
		}
		http_abort( args[0]->ToUint32Value(worker) );
	}

	/**
	 * @func user_agent()
	 * @ret {String}
	 */
	static void user_agent(FunctionCall args) {
		Js_Worker(args);
		Js_Return( http_user_agent() );
	}

	/**
	 * @func set_user_agent(user_agent)
	 * @arg user_agent {String}
	 */
	static void set_user_agent(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || ! args[0]->IsString(worker)) {
			Js_Throw("Bad argument");
		}
		http_set_user_agent( args[0]->ToStringValue(worker) );
	}

	/**
	 * @func cache_path()
	 * @ret {String}
	 */
	static void cache_path(FunctionCall args) {
		Js_Worker(args);
		Js_Return( http_cache_path() );
	}

	/**
	 * @func set_cache_path(path)
	 * @arg path {String}
	 */
	static void set_cache_path(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			Js_Throw(
				"* @func setCachePath(path)\n"
				"* @arg path {String}\n"
			);
		}
		http_set_cache_path( args[0]->ToStringValue(worker) );
	}

	/**
	 * @func clear_cache()
	 */
	static void clear_cache(FunctionCall args) {
		http_clear_cache();
	}

	/**
	 * @func clear_cookie()
	 */
	static void clear_cookie(FunctionCall args) {
		http_clear_cookie();
	}

	// /**
	//  * @func ssl_cacert_file()
	//  * @ret {String} return cacert file path
	//  */
	// static void ssl_cacert_file(FunctionCall args) {
	// 	Js_Worker(args);
	// 	Js_Return( http_ssl_cacert_file() );
	// }
	// 
	//  /**
	//   * @func ssl_cacert_file(path)
	//   * @arg path {String}
	//   */
	//  static void set_ssl_cacert_file(FunctionCall args) {
	//    Js_Worker(args);
	//    if (args.Length() == 0 || !args[0]->IsString(worker)) {
	//      Js_Throw(
	//        "* @func sslCacertFile(path)\n"
	//        "* @arg path {String}\n"
	//      );
	//    }
	//    http_set_ssl_cacert_file( args[0]->ToStringValue(worker) );
	//  }
	//
	//  /**
	//   * @func set_ssl_client_key_file(path)
	//   * @arg path {String}
	//   */
	//  static void set_ssl_client_key_file(FunctionCall args) {
	//    Js_Worker(args);
	//    if (args.Length() == 0 || !args[0]->IsString(worker)) {
	//      Js_Throw(
	//        "* @func setSslClientKeyfile(path)\n"
	//        "* @arg path {String}\n"
	//      );
	//    }
	//    http_set_ssl_client_key_file( args[0]->ToStringValue(worker) );
	//  }
	//
	//  /**
	//   * @func set_ssl_client_keypasswd(password)
	//   * @arg password {String}
	//   */
	//  static void set_ssl_client_keypasswd(FunctionCall args) {
	//    Js_Worker(args);
	//    if (args.Length() == 0 || !args[0]->IsString(worker)) {
	//      Js_Throw(
	//        "* @func setSslClientKeypasswd(password)\n"
	//        "* @arg password {String}\n"
	//      );
	//    }
	//    http_set_ssl_client_keypasswd( args[0]->ToStringValue(worker) );
	//  }

	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->bindingModule("_buffer");
		WrapNativeHttpClientRequest::binding(exports, worker);
		// FUNC
		Js_Set_Method(request, request);
		Js_Set_Method(requestStream, request_stream);
		Js_Set_Method(requestSync, request_sync);
		Js_Set_Method(abort, abort);
		Js_Set_Method(userAgent, user_agent);
		Js_Set_Method(setUserAgent, set_user_agent);
		Js_Set_Method(cachePath, cache_path);
		Js_Set_Method(setCachePath, set_cache_path);
		Js_Set_Method(clearCache, clear_cache);
		Js_Set_Method(clearCookie, clear_cookie);

		//Js_Set_Method(sslCacertFile, ssl_cacert_file);
		//Js_Set_Method(setSslCacertFile, set_ssl_cacert_file);
		//Js_Set_Method(setSslClientKeyFile, set_ssl_client_key_file);
		//Js_Set_Method(setSslClientKeypasswd, set_ssl_client_keypasswd);
	}
};

Js_REG_MODULE(_http, NativeHttp);
Js_END
