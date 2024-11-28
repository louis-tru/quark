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

#include "../js_.h"

namespace qk { namespace js {

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

	struct MixHttpClientRequest: MixObject {
		typedef HttpClientRequest Type;

		struct Delegate: Object, HttpClientRequest::Delegate {
			MixHttpClientRequest* _host;
			String _error;
			String _write;
			String _header;
			String _data;
			String _end;
			String _readystate_change;
			String _timeout;
			String _abort;

			Worker* worker() { return _host->worker(); }

			virtual void trigger_http_error(HttpClientRequest* req, cError& error) {
				if ( !_error.isEmpty() ) {
					HandleScope scope(worker());
					JSValue* arg = worker()->newValue( error );
					_host->call( worker()->newStringOneByte(_error), 1, &arg );
				}
			}
			virtual void trigger_http_write(HttpClientRequest* req) {
				if ( !_write.isEmpty() ) {
					HandleScope scope(worker());
					_host->call( worker()->newStringOneByte(_write) );
				}
			}
			virtual void trigger_http_header(HttpClientRequest* req) {
				if ( !_header.isEmpty() ) {
					HandleScope scope(worker());
					_host->call( worker()->newStringOneByte(_header) );
				}
			}
			virtual void trigger_http_data(HttpClientRequest* req, Buffer &buffer) {
				if ( !_data.isEmpty() ) {
					HandleScope scope(_host->worker());
					JSValue* arg = worker()->newValue( std::move(buffer) );
					_host->call( worker()->newStringOneByte(_data), 1, &arg );
				}
			}
			virtual void trigger_http_readystate_change(HttpClientRequest* req) {
				if (req->ready_state() == HTTP_READY_STATE_READY) {
					_host->self()->retain(); // TODO: js handle keep active
				}
				if ( !_readystate_change.isEmpty() ) {
					HandleScope scope(worker());
					_host->call( worker()->newStringOneByte(_readystate_change) );
				}
			}
			virtual void trigger_http_timeout(HttpClientRequest* req) {
				if ( !_timeout.isEmpty() ) {
					HandleScope scope(worker());
					_host->call( worker()->newStringOneByte(_timeout) );
				}
			}
			virtual void trigger_http_end(HttpClientRequest* req) {
				if ( !_end.isEmpty() ) {
					HandleScope scope(worker());
					_host->call( worker()->newStringOneByte(_end) );
				}
				if (req->ready_state() > HTTP_READY_STATE_SENDING) {
					req->release(); // TODO: js handle set weak object
				}
			}
			virtual void trigger_http_abort(HttpClientRequest* req) {
				if ( !_abort.isEmpty() ) {
					HandleScope scope(worker());
					_host->call( worker()->newStringOneByte(_abort) );
				}
				if (req->ready_state() > HTTP_READY_STATE_SENDING) {
					req->release(); // v8 handle set weak object
				}
			}
		};

		Delegate* getDelegate() {
			return static_cast<Delegate*>(externalData());
		}

		virtual bool addEventListener(cString& name, cString& func, int id) {
			Delegate* _del = getDelegate();
			if (!_del) {
				_del = new Delegate();
				_del->_host = this;
				self<HttpClientRequest>()->set_delegate(_del);
				setExternalData(_del);
			}
			if ( id != -1 ) return 0; // 只接收id==-1的监听器

			if ( name == "Error" ) {
				_del->_error = func;
			} else if ( name == "Write" ) {
				_del->_write = func;
			} else if ( name == "Header" ) {
				_del->_header = func;
			} else if ( name == "Data" ) {
				_del->_data = func;
			} else if ( name == "End" ) {
				_del->_end = func;
			} else if ( name == "ReadystateChange" ) {
				_del->_readystate_change = func;
			} else if ( name == "Timeout" ) {
				_del->_timeout = func;
			} else if ( name == "Abort" ) {
				_del->_abort = func;
			} else {
				return false;
			}
			return true;
		}

		virtual bool removeEventListener(cString& name, int id) {
			auto _del = getDelegate();
			if ( id != -1 || !_del ) return 0;

			if ( name == "Error" ) {
				_del->_error = String();
			} else if ( name == "Write" ) {
				_del->_write = String();
			} else if ( name == "Header" ) {
				_del->_header = String();
			} else if ( name == "Data" ) {
				_del->_data = String();
			} else if ( name == "End" ) {
				_del->_end = String();
			} else if ( name == "ReadystateChange" ) {
				_del->_readystate_change = String();
			} else if ( name == "Timeout" ) {
				_del->_timeout = String();
			} else if ( name == "Abort" ) {
				_del->_abort = String();
			} else {
				return false;
			}
			return true;
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(HttpClientRequest, 0, {
				New<MixHttpClientRequest>(args, new HttpClientRequest());
			});

			Js_Class_Method(setMethod, {
				if (args.length() < 1 || !args[0]->isUint32()) {
					Js_Throw(
						"@method setMethod(method)\n"
						"@param method {HttpMethod}\n"
					);
				}
				auto arg = args[0]->toUint32Value(worker).unsafe();
				HttpMethod method = arg > 4 ? HTTP_METHOD_GET: (HttpMethod)arg;
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->set_method(method); }, Error);
			});
			

			Js_Class_Method(setUrl, {
				if (args.length() < 1 || !args[0]->isString()) {
					Js_Throw(
						"@method setUrl(url)\n"
						"@param url {String}\n"
					);
				}
				String arg = args[0]->toStringValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->set_url(arg); }, Error);
			});

			Js_Class_Method(setSavePath, {
				if (args.length() < 1 || !args[0]->isString()) {
					Js_Throw(
						"@method setSavePath(path)\n"
						"@param path {String}\n"
					);
				}
				String arg = args[0]->toStringValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->set_save_path(arg); }, Error);
			});

			Js_Class_Method(setUsername, {
				if (args.length() < 1 || !args[0]->isString()) {
					Js_Throw(
						"@method setUsername(username)\n"
						"@param username {String}\n"
					);
				}
				String arg = args[0]->toStringValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->set_username(arg); }, Error);
			});

			Js_Class_Method(setPassword, {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"@method setPassword(password)\n"
						"@param password {String}\n"
					);
				}
				String arg = args[0]->toStringValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->set_password(arg); }, Error);
			});

			Js_Class_Method(disableCache, {
				if (args.length() < 1) {
					Js_Throw(
						"@method disableCache(disable)\n"
						"@param disable {bool}\n"
					);
				}
				bool arg = args[0]->toBooleanValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->disable_cache(arg); }, Error);
			});

			Js_Class_Method(disableCookie, {
				if (args.length() < 1) {
					Js_Throw(
						"@method disableCookie(disable)\n"
						"@param disable {bool}\n"
					);
				}
				bool arg = args[0]->toBooleanValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->disable_cookie(arg); }, Error);
			});

			Js_Class_Method(disableSendCookie, {
				if (args.length() < 1) {
					Js_Throw(
						"@method disableSendCookie(disable)\n"
						"@param disable {bool}\n"
					);
				}
				auto arg = args[0]->toBooleanValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->disable_send_cookie(arg); }, Error);
			});

			Js_Class_Method(disableSslVerify, {
				if (args.length() < 1) {
					Js_Throw(
						"@method disableSslVerify(disable)\n"
						"@param disable {bool}\n"
					);
				}
				Js_Self(HttpClientRequest);
				Js_Try_Catch({
					self->disable_ssl_verify(args[0]->toBooleanValue(worker));
				}, Error);
			});

			Js_Class_Method(setRequestHeader, {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
					Js_Throw(
						"@method setRequestHeader(header_name, value)\n"
						"@param header_name {String} ascii string\n"
						"@param value {String}\n"
					);
				}
				Js_Self(HttpClientRequest);
				Js_Try_Catch({
					self->set_request_header( args[0]->toStringValue(worker,1), args[1]->toStringValue(worker));
				}, Error);
			});

			Js_Class_Method(setForm, {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isString() ) {
					Js_Throw(
						"@method setForm(form_name, value)\n"
						"@param form_name {String}\n"
						"@param value {String}\n"
					);
				}
				String form_name = args[0]->toStringValue(worker);
				String value = args[1]->toStringValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({
					self->set_form(form_name, value);
				}, Error);
			});

			Js_Class_Method(setUploadFile, {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isString() ) {
					Js_Throw(
						"@method setUploadFile(form_name, local_path)\n"
						"@param form_name {String}\n"
						"@param local_path {String}\n"
					);
				}
				String form_name = args[0]->toStringValue(worker);
				String local_path = args[1]->toStringValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({
					self->set_upload_file(form_name, local_path);
				}, Error);
			});

			Js_Class_Method(clearRequestHeader, {
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->clear_request_header(); }, Error);
			});

			Js_Class_Method(clearFormData, {
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->clear_form_data(); }, Error);
			});

			Js_Class_Method(getResponseHeader, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_Throw(
						"@method getResponseHeader(header_name)\n"
						"@param header_name {String}\n"
						"@return {String}\n"
					);
				}
				Js_Self(HttpClientRequest);
				auto rv = self->get_response_header(args[0]->toStringValue(worker,1));
				Js_Return( rv );
			});

			typedef cDict<String, String> cDictSS;

			Js_Class_Method(getAllResponseHeaders, {
				Js_Self(HttpClientRequest);
				cDictSS &rv = self->get_all_response_headers();
				Js_Return( rv );
			});

			Js_Class_Method(setKeepAlive, {
				if (args.length() == 0) {
					Js_Throw(
						"@method setKeepAlive(keep_alive)\n"
						"@param keep_alive {bool}\n"
					);
				}
				bool enable = args[0]->toBooleanValue(worker);
				Js_Self(HttpClientRequest);
				Js_Try_Catch({ self->set_keep_alive(enable); }, Error);
			});

			Js_Class_Method(setTimeout, {
				if (args.length() == 0 || !args[0]->isNumber()) {
					Js_Throw(
						"@method setTimeout(time)\n"
						"@param time {uint} ms\n"
					);
				}
				Js_Self(HttpClientRequest);

				uint64_t time = args[0]->toUint32Value(worker).unsafe() * 1000;

				Js_Try_Catch({ self->set_timeout(time); }, Error);
			});

			Js_Class_Method(send, {
				Js_Self(HttpClientRequest);
				if (args.length() == 0) {
					Js_Try_Catch({ self->send(); }, Error);
				} else {
					if (args[0]->isString()) {
						Js_Try_Catch({
							self->send( args[0]->toStringValue(worker).collapse() );
						}, Error);
					}
					else if ( args[0]->isBuffer() ) {
						auto buff = args[0]->toBufferValue(worker);
						Js_Try_Catch({ self->send(buff.buffer().copy()); }, Error);
					}
					else {
						Js_Try_Catch({ self->send(); }, Error );
					}
				}
			});

			Js_Class_Method(pause, {
				Js_Self(HttpClientRequest);
				self->pause();
			});

			Js_Class_Method(resume, {
				Js_Self(HttpClientRequest);
				self->resume();
			});

			Js_Class_Method(abort, {
				Js_Self(HttpClientRequest);
				self->abort();
			});

			Js_Class_Accessor_Get(uploadTotal, {
				Js_Self(HttpClientRequest);
				Js_Return( self->upload_total() );
			});

			Js_Class_Accessor_Get(uploadSize, {
				Js_Self(HttpClientRequest);
				Js_Return( self->upload_size() );
			});

			Js_Class_Accessor_Get(downloadTotal, {
				Js_Self(HttpClientRequest);
				Js_Return( self->download_total() );
			});

			Js_Class_Accessor_Get(downloadSize, {
				Js_Self(HttpClientRequest);
				Js_Return( self->download_size() );
			});

			Js_Class_Accessor_Get(readyState, {
				Js_Self(HttpClientRequest);
				Js_Return( self->ready_state() );
			});

			Js_Class_Accessor_Get(statusCode, {
				Js_Self(HttpClientRequest);
				Js_Return( self->status_code() );
			});

			Js_Class_Accessor_Get(httpResponseVersion, {
				Js_Self(HttpClientRequest);
				Js_Return( self->http_response_version() );
			});

			Js_Class_Accessor_Get(url, {
				Js_Self(HttpClientRequest);
				Js_Return( self->url() );
			});

			cls->exports("HttpClientRequest", exports);
		}
	};

	struct NativeHttp {
		static bool get_options(Worker* worker, JSValue* arg, RequestOptions& opt) {
			Js_Handle_Scope();
			JSObject* obj = arg->as<JSObject>();

			opt = {
				String(),
				HTTP_METHOD_GET,
				Dict<String, String>(),
				Buffer(),
				String(),
				String(),
				false,
				false,
				false,
			};
			JSValue* value;

			value = obj->get(worker, worker->newStringOneByte(const_url));
			if ( !value ) return false;
			if ( value->isString() ) opt.url = value->toStringValue(worker);
			
			value = obj->get(worker, worker->newStringOneByte(const_method));
			if ( !value ) return false;
			if ( value->isUint32() ) {
				auto arg = value->toUint32Value(worker).unsafe();
				opt.method = arg > 4 ? HTTP_METHOD_GET: (HttpMethod)arg;
			}

			value = obj->get(worker, worker->newStringOneByte(const_headers));
			if ( !value ) return false;
			if (!value->as<JSObject>()->toStringDict(worker).to(opt.headers)) return false;
			
			value = obj->get(worker, worker->newStringOneByte(const_post_data));
			if ( !value ) return false;
			if ( value->isString() ) {
				opt.post_data = value->toStringValue(worker).collapse();
			}
			else if ( value->isBuffer() ) {
				opt.post_data = value->toBufferValue(worker).buffer().copy();
			}

			value = obj->get(worker, worker->newStringOneByte(const_save));
			if ( !value ) return false;
			if ( value->isString() ) {
				opt.save = value->toStringValue(worker);
			}

			value = obj->get(worker, worker->newStringOneByte(const_upload));
			if ( !value ) return false;
			if ( value->isString() ) {
				opt.upload = value->toStringValue(worker);
			}

			value = obj->get(worker, worker->newStringOneByte(const_timeout));
			if ( !value ) return false;
			if ( value->isUint32() ) {
				opt.timeout = value->toUint32Value(worker).unsafe() * 1e3;
			}

			value = obj->get(worker, worker->newStringOneByte(const_disable_ssl_verify));
			if ( !value ) return false;
			opt.disable_ssl_verify = value->toBooleanValue(worker);

			value = obj->get(worker, worker->newStringOneByte(const_disable_cache));
			if ( !value ) return false;
			opt.disable_cache = value->toBooleanValue(worker);

			value = obj->get(worker, worker->newStringOneByte(const_disable_cache));
			if ( !value ) return false;
			opt.disable_cookie = value->toBooleanValue(worker);

			return true;
		}

		static void request(FunctionArgs args, cChar* argument, bool stream) {
			Js_Worker(args);
			if (args.length() == 0 || ! args[0]->isObject()) {
				Js_Throw(argument);
			}
			Js_Handle_Scope();
			uint32_t rev = 0;
			RequestOptions opt;
			if (!get_options(worker, args[0], opt))
				return;

			Callback<StreamResponse> cb;
			Callback<ResponseData> cb1;

			if ( args.length() > 1 ) {
				if (stream)
					cb = get_callback_for_io_stream_http_error(worker, args[1]);
				else
					cb1 = get_callback_for_response_data_http_error(worker, args[1]);
			}

			Js_Try_Catch({
				if ( stream ) {
					rev = http_request_stream(opt, cb);
				} else {
					rev = http_request(opt, cb1);
				}
			}, HttpError);
			
			Js_Return( rev );
		}

		static void binding(JSObject* exports, Worker* worker) {
			worker->bindingModule("_buffer");
			MixHttpClientRequest::binding(exports, worker);

			Js_Method(request, {
				request(args,
					"@method request(options[,cb])\n"
					"@param options {RequestOptions}\n"
					"@param [cb] {Function}\n"
					"@return {uint} return req id\n", false
				);
			});

			Js_Method(requestStream, {
				request(args, 
					"@method requestStream(options[,cb])\n"
					"@param options {RequestOptions}\n"
					"@param [cb] {Function}\n"
					"@return {uint} return req id\n", true
				);
			});

			Js_Method(requestSync, {
				if (args.length() == 0 || !args[0]->isObject()) {
					Js_Throw(
						"@method requestSync(url)\n"
						"@param url {String}\n"
						"@return {Buffer}\n"
					);
				}
				RequestOptions opt;
				if (!get_options(worker, args[0], opt))
					return;

				Js_Try_Catch({
					Js_Return( http_request_sync(opt) );
				}, HttpError);
			});

			Js_Method(abort, {
				if ( args.length() == 0 || !args[0]->isUint32() ) {
					Js_Throw(
						"@method abort(id)\n"
						"@param id {uint} abort id\n"
					);
				}
				http_abort( args[0]->toUint32Value(worker).unsafe() );
			});

			Js_Method(userAgent, {
				Js_Return( http_user_agent() );
			});

			Js_Method(setUserAgent, {
				if (args.length() == 0 || ! args[0]->isString()) {
					Js_Throw("Bad argument");
				}
				http_set_user_agent( args[0]->toStringValue(worker) );
			});

			Js_Method(cachePath, {
				Js_Return( http_cache_path() );
			});

			Js_Method(setCachePath, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_Throw(
						"@method setCachePath(path)\n"
						"@param path {String}\n"
					);
				}
				http_set_cache_path( args[0]->toStringValue(worker) );
			});

			Js_Method(maxConnectPoolSize, {
				Js_Return( http_max_connect_pool_size() );
			});

			Js_Method(setMaxConnectPoolSize, {
				if (args.length() == 0 || !args[0]->isUint32()) {
					Js_Throw(
						"@method setMaxConnectPoolSize(size)\n"
						"@param size {number}\n"
					);
				}
				http_set_max_connect_pool_size( args[0]->toUint32Value(worker).unsafe() );
			});

			Js_Method(clearCache, {
				http_clear_cache();
			});

			Js_Method(clearCookie, {
				http_clear_cookie();
			});
		}
	};

	Js_Module(_http, NativeHttp);
} }
