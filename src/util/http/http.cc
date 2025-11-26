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

#include "./http.inl"

namespace qk {
	cString string_method[5] = { "GET", "POST", "HEAD", "DELETE", "PUT" };
	cString string_colon(": ");
	cString string_space(" ");
	cString string_header_end("\r\n");
	cString string_max_age("max-age=");
	cString content_type_form("application/x-www-form-urlencoded; Charset=utf-8");
	cString content_type_multipart_form("multipart/form-data; boundary=----QuarkFormBoundaryrGKCBY7qhFd3TrwA");
	cString multipart_boundary_start("------QuarkFormBoundaryrGKCBY7qhFd3TrwA\r\n");
	cString multipart_boundary_end  ("------QuarkFormBoundaryrGKCBY7qhFd3TrwA--");

	class DefaultDelegate: public HttpDelegate {
		void trigger_http_error(HttpClientRequest* req, cError& error) override {}
		void trigger_http_write(HttpClientRequest* req) override {}
		void trigger_http_header(HttpClientRequest* req) override {}
		void trigger_http_data(HttpClientRequest* req, Buffer &buffer) override {}
		void trigger_http_end(HttpClientRequest* req) override {}
		void trigger_http_readystate_change(HttpClientRequest* req) override {}
		void trigger_http_timeout(HttpClientRequest* req) override {}
		void trigger_http_abort(HttpClientRequest* req) override {}
	};

	static DefaultDelegate _default_delegate;

	Host::Impl(HttpClientRequest* cli, RunLoop* loop)
		: _cli(cli)
		, _loop(loop)
		, _delegate(&_default_delegate)
		, _upload_total(0)
		, _upload_size(0)
		, _download_total(0)
		, _download_size(0)
		, _ready_state(HTTP_READY_STATE_INITIAL)
		, _status_code(0)
		, _method(HTTP_METHOD_GET)
		, _handler(nullptr)
		, _cache_reader(nullptr)
		, _file_writer(nullptr)
		, _keep_alive(true)
		, _retain(nullptr)
		, _timeout(0)
		, _wait_connect_id(0)
		, _write_flag(kNone_WriteFlag)
		, _disable_cache(false)
		, _disable_cookie(false)
		, _disable_send_cookie(false)
		, _disable_ssl_verify(false)
		, _pause(false)
		, _url_no_cache_arg(false)
	{
		Qk_ASSERT(loop);
	}

	Host::~Impl() {
		abort();
		Qk_ASSERT(!_retain);
		Qk_ASSERT(!_handler);
		Qk_ASSERT(!_cache_reader);
		Qk_ASSERT(!_file_writer);
	}

	void Host::set_delegate(HttpDelegate* delegate) {
		_delegate = delegate ? delegate: &_default_delegate;
	}

	Reader* Host::reader() {
		return _handler ? HttpHandler_reader(_handler):
			FileCacheReader_reader(_cache_reader);
	}

	void Host::read_advance() {
		auto r = reader();
		Qk_ASSERT(r);
		if ( _pause ) {
			r->read_pause();
		} else {
			r->read_advance();
		}
	}

	bool Host::is_disable_cache() {
		return _disable_cache || _url_no_cache_arg || _method != HTTP_METHOD_GET;
	}

	void Host::read_pause() {
		auto r = reader();
		Qk_ASSERT(r);
		r->read_pause();
	}

	void Host::trigger_http_data(Buffer& buffer) {
		_delegate->trigger_http_data(_cli, buffer);
	}

	void Host::on_http_readystate_change(HttpReadyState ready_state) {
		_ready_state = ready_state;
		_delegate->trigger_http_readystate_change(_cli);
	}

	void Host::on_http_write() {
		_delegate->trigger_http_write(_cli);
	}

	void Host::on_http_header(uint32_t status_code, DictSS&& header, bool fromCache) {
		if (fromCache) {
			if ( _save_path == _cache_path )
				_canSave = false; // conflict path, do not save
		} else if (status_code == 200) {
			_write_flag = kAll_WriteFlag; // write header and body
			// no longer need cache reader, release it as data is from http response
			FileCacheReader_Releasep(_cache_reader); 
		}
		Qk_ASSERT_EQ(_status_code, 0); // check status code not set
		_status_code = status_code; // set status code
		_response_header = std::move(header); // move header
		_delegate->trigger_http_header(_cli); // trigger header event
	}

	void Host::on_http_data(Buffer& buffer, bool fromCache) {
		if ( _canSave ) { // Save file content to path, ignore cache
			if ( !_file_writer ) { // download file
				Qk_ASSERT(!_save_path.isEmpty(), "Save path is empty");
				FileWriter_new(this, _save_path, kBody_WriteFlag, loop()); // only write body to file
			}
			FileWriter_write(_file_writer, buffer); // pipeline write file
		} else if ( _write_flag && !is_disable_cache() ) {
			if ( !_file_writer ) {
				FileWriter_new(this, _cache_path, _write_flag, loop());
			}
			FileWriter_write(_file_writer, buffer); // pipeline write cache
		} else {
			trigger_http_data(buffer);
			read_advance(); // continue read data
		}
	}

	void Host::on_response_complete(bool fromCache) {
		if (!fromCache) {
			Qk_ASSERT(_handler);
			_pool->release(_handler, false); // release handler
			_handler = nullptr; // clear handler ptr

			if ( _status_code == 304) {
				if (_cache_reader) {
					// read http headers from new response
					auto expires = get_expires_from_header(_response_header);
					// use cache header if 304 status, only update expires header
					_response_header = std::move(FileCacheReader_header(_cache_reader));
					// Convert from 304 to cache 200
					if (!expires.isEmpty() && expires != _response_header["expires"]) {
						// update expires header if different
						_write_flag = kHeader_WriteFlag; // need write header
						_response_header["expires"] = expires;
					}
					FileCacheReader_read_advance(_cache_reader); // start read cache data
					return;
				} else {
					Qk_ELog("http response status code error, %d", _status_code);
					// return empty body response
				}
			}
		}

		if ( _file_writer ) {
			FileWriter_end(_file_writer); // pipeline end
		} else {
			on_http_end();
		}
	}

	void Host::on_error_and_abort(cError& error) {
		_delegate->trigger_http_error(_cli, error);
		abort();
	}

	void Host::on_http_timeout() {
		_delegate->trigger_http_timeout(_cli);
		abort();
	}

	void Host::send_http() {
		Qk_ASSERT(_retain);
		Qk_ASSERT_EQ(_handler, nullptr);
		// request http handler from pool and wait for callback
		_pool->request(this, Cb([this](Cb::Data& evt) {
			auto h = reinterpret_cast<HttpHandler*>(evt.data); // force cast to HttpHandler
			if ( _wait_connect_id ) { // still need connect if wait id exist
				if ( evt.error ) {
					on_error_and_abort(*evt.error);
				} else {
					Qk_ASSERT_EQ(_handler, nullptr);
					_handler = h;
					HttpHandler_bind_host_and_send(_handler, this);
				}
			} else {
				// release handler if no longer need connect
				// maybe aborted send request
				_pool->release(h, false);
			}
		}, this));
	}

	void Host::cache_file_stat_cb(Callback<FileStat>::Data& e) {
		if (_retain) {
			if (e.error || e.data->size() < 50) { // no cache or cache file is bad
				send_http();
			} else {
				FileCacheReader_new(this, e.data->size(), loop());
			}
		}
	}

	void Host::on_http_end() {
		end_(false); // normal end
	}

	void Host::end_(bool abort) {
		if ( _retain && !_retain->ending ) {
			_retain->ending = true;

			// release resources
			FileCacheReader_Releasep(_cache_reader);
			FileWriter_Releasep(_file_writer);
			_pool->release(_handler, abort);
			_handler = nullptr;
			_pause = false;
			_wait_connect_id = 0; // reset wait connect id

			if ( abort ) {
				on_http_readystate_change(HTTP_READY_STATE_INITIAL);
				delete _retain; // release retain reference
				_delegate->trigger_http_abort(_cli);
			} else {
				on_http_readystate_change(HTTP_READY_STATE_COMPLETED);
				delete _retain; // release retain reference
				_delegate->trigger_http_end(_cli);
			}
		}
	}

	void Host::send(Buffer data) throw(Error) {
		Qk_IfThrow(!_retain, ERR_REPEAT_CALL, "RetainRef repeat call");
		Qk_IfThrow(!_uri.is_null(), ERR_INVALID_FILE_PATH, "Invalid path" );
		Qk_IfThrow(_uri.type() == URI_HTTP ||
						_uri.type() == URI_HTTPS, ERR_INVALID_FILE_PATH, "Invalid path `%s`", *_uri.href());

		_post_data = data;
		_retain = new RetainRef(this); // Force to maintain reference until end
		_pause = false;
		_url_no_cache_arg = false;
		_canSave = !_save_path.isEmpty();
		_cache_path = http_cache_path() + '/' + hash_str(_uri.href());
		_write_flag = kNone_WriteFlag; // reset write cache flag

		int i = _uri.search().indexOf("__no_cache");
		if ( i != -1 && _uri.search()[i+9] != '=' ) {
			_url_no_cache_arg = true;
		}

		on_http_readystate_change(HTTP_READY_STATE_READY); // ready

		if ( !_retain )
			return; // abort

		// reset state
		_upload_total = 0; _upload_size = 0;
		_download_total = 0; _download_size = 0;
		_status_code = 0;
		_response_header.clear();
		_http_response_version = String();

		if ( is_disable_cache() ) { // check cache
			send_http();
		} else {
			fs_stat(_cache_path, Callback<FileStat>(&Impl::cache_file_stat_cb, this));
		}
	}

	void Host::abort() {
		end_(true); // abort end
	}

	void Host::check_is_can_modify() throw(Error) {
		Qk_IfThrow(!_retain, ERR_SENDIF_CANNOT_MODIFY, "Http request sending cannot modify property");
	}

	void Host::pause() {
		if ( _retain )
			_pause = true;
	}

	void Host::resume() {
		if ( _retain && _pause ) {
			_pause = false;
			auto r = reader();
			if ( r ) {
				r->read_advance();
			}
		}
	}

	//-----------------------------------------------------------------------

	HttpClientRequest::HttpClientRequest(RunLoop* loop)
		: _impl(NewRetain<Impl>(this, loop))
	{}

	HttpClientRequest::~HttpClientRequest() {
		_impl->set_delegate(nullptr);
		_impl->abort();
		Releasep(_impl);
	}

	void HttpClientRequest::set_delegate(HttpDelegate* delegate) throw(Error) {
		_impl->check_is_can_modify();
		_impl->set_delegate(delegate);
	}

	void HttpClientRequest::set_method(HttpMethod method) throw(Error) {
		_impl->check_is_can_modify();
		if ( method < HTTP_METHOD_GET || method > HTTP_METHOD_PUT ) {
			method = HTTP_METHOD_GET;
		}
		_impl->_method = method;
	}

	void HttpClientRequest::set_url(cString& path) throw(Error) {
		_impl->check_is_can_modify();
		_impl->_uri = URI(path);
	}

	void HttpClientRequest::set_save_path(cString& path) throw(Error) {
		_impl->check_is_can_modify();
		_impl->_save_path = path;
	}

	void HttpClientRequest::set_username(cString& username) throw(Error) {
		_impl->check_is_can_modify();
		_impl->_username = username;
	}

	void HttpClientRequest::set_password(cString& password) throw(Error) {
		_impl->check_is_can_modify();
		_impl->_password = password;
	}

	void HttpClientRequest::disable_cache(bool disable) throw(Error) {
		// _impl->check_is_can_modify();
		_impl->_disable_cache = disable;
	}

	void HttpClientRequest::disable_cookie(bool disable) throw(Error) {
		// _impl->check_is_can_modify();
		_impl->_disable_cookie = disable;
	}

	void HttpClientRequest::disable_send_cookie(bool disable) throw(Error) {
		// _impl->check_is_can_modify();
		_impl->_disable_send_cookie = disable;
	}

	void HttpClientRequest::disable_ssl_verify(bool disable) throw(Error) {
		// _impl->check_is_can_modify();
		_impl->_disable_ssl_verify = disable;
	}

	void HttpClientRequest::set_keep_alive(bool keep_alive) throw(Error) {
		// _impl->check_is_can_modify();
		_impl->_keep_alive = keep_alive;
	}

	void HttpClientRequest::set_timeout(uint64_t timeout) throw(Error) {
		// _impl->check_is_can_modify();
		_impl->_timeout = timeout;
	}

	void HttpClientRequest::set_request_header(cString& name, cString& value) throw(Error) {
		_impl->check_is_can_modify();
		_impl->_request_header[name] = value;
	}

	void HttpClientRequest::set_form(cString& form_name, cString& value) throw(Error) {
		_impl->check_is_can_modify();
		Qk_IfThrow( value.length() <= BUFFER_SIZE,
							ERR_HTTP_FORM_SIZE_LIMIT, "Http form field size limit <= %d", BUFFER_SIZE);
		_impl->_form_data[form_name] = {
			FORM_TYPE_TEXT, value, uri_encode(form_name)
		};
	}

	void HttpClientRequest::set_upload_file(cString& form_name, cString& path) throw(Error) {
		_impl->check_is_can_modify();
		_impl->_form_data[form_name] = {
			FORM_TYPE_FILE, path, uri_encode(form_name)
		};
	}

	void HttpClientRequest::clear_request_header() throw(Error) {
		_impl->check_is_can_modify();
		_impl->_request_header.clear();
	}

	void HttpClientRequest::clear_form_data() throw(Error) {
		_impl->check_is_can_modify();
		_impl->_form_data.clear();
	}

	String HttpClientRequest::get_response_header(cString& name) {
		auto i = _impl->_response_header.find(name);
		if ( i == _impl->_response_header.end() ) return String();
		return i->second;
	}

	const DictSS& HttpClientRequest::get_all_response_headers() const {
		return _impl->_response_header;
	}

	int64_t HttpClientRequest::upload_total() const {
		return _impl->_upload_total;
	}

	int64_t HttpClientRequest::upload_size() const {
		return _impl->_upload_size;
	}

	int64_t HttpClientRequest::download_total() const {
		return _impl->_download_total;
	}

	int64_t HttpClientRequest::download_size() const {
		return _impl->_download_size;
	}

	HttpReadyState HttpClientRequest::ready_state() const {
		return _impl->_ready_state;
	}

	int HttpClientRequest::status_code() const {
		return _impl->_status_code;
	}

	String HttpClientRequest::url() const {
		return _impl->_uri.href();
	}

	void HttpClientRequest::send(Buffer data) throw(Error) {
		_impl->send(data);
	}

	void HttpClientRequest::send(cString& data) throw(Error) {
		_impl->send(data.copy().collapse());
	}

	void HttpClientRequest::pause() {
		_impl->pause();
	}

	void HttpClientRequest::resume() {
		_impl->resume();
	}

	void HttpClientRequest::abort() {
		_impl->abort();
	}

	String HttpClientRequest::http_response_version() const {
		return _impl->_http_response_version;
	}

}
