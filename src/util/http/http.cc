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

	Client::Impl(HttpClientRequest* host, RunLoop* loop)
		: _host(host)
		, _loop(loop)
		, _delegate(&_default_delegate)
		, _upload_total(0)
		, _upload_size(0)
		, _download_total(0)
		, _download_size(0)
		, _ready_state(HTTP_READY_STATE_INITIAL)
		, _status_code(0)
		, _method(HTTP_METHOD_GET)
		, _connect(nullptr)
		, _cache_reader(nullptr)
		, _file_writer(nullptr)
		, _keep_alive(true)
		, _retain(nullptr)
		, _timeout(0)
		, _wait_connect_id(0)
		, _write_cache_flag(kBody_WriteCacheFlag)
		, _disable_cache(false)
		, _disable_cookie(false)
		, _disable_send_cookie(false)
		, _disable_ssl_verify(false)
		, _pause(false)
		, _url_no_cache_arg(false)
	{
		Qk_ASSERT(loop);
	}

	Client::~Impl() {
		abort();
		Qk_ASSERT(!_retain);
		Qk_ASSERT(!_connect);
		Qk_ASSERT(!_cache_reader);
		Qk_ASSERT(!_file_writer);
	}

	void Client::set_delegate(HttpDelegate* delegate) {
		_delegate = delegate ? delegate: &_default_delegate;
	}

	Reader* Client::reader() {
		return _connect ? Connect_reader(_connect):
			FileCacheReader_reader(_cache_reader);
	}

	void Client::read_advance() {
		auto r = reader();
		Qk_ASSERT(r);
		if ( _pause ) {
			r->read_pause();
		} else {
			r->read_advance();
		}
	}

	bool Client::is_disable_cache() {
		return _disable_cache || _url_no_cache_arg || _method != HTTP_METHOD_GET;
	}

	void Client::read_pause() {
		auto r = reader();
		Qk_ASSERT(r);
		r->read_pause();
	}

	void Client::trigger_http_readystate_change(HttpReadyState ready_state) {
		_ready_state = ready_state;
		_delegate->trigger_http_readystate_change(_host);
	}

	void Client::trigger_http_write() {
		_delegate->trigger_http_write(_host);
	}

	void Client::trigger_http_header(uint32_t status_code, DictSS&& header, bool fromCache) {
		_status_code = status_code;
		_response_header = std::move(header);
		_delegate->trigger_http_header(_host);
	}

	void Client::trigger_http_data2(Buffer& buffer) {
		_delegate->trigger_http_data(_host, buffer);
	}

	void Client::trigger_http_data(Buffer& buffer) {
		// _write_cache_flag:
		// _write_cache_flag = 0 not write cache
		// _write_cache_flag = 1 write response header
		// _write_cache_flag = 2 write response header and body

		if ( _write_cache_flag == kAll_WriteCacheFlag ) { // http status == 200
			// `_write_cache_flag==2` 写入头与主体缓存时,
			// 一定是由`Connect`发起的调用,所以已不再需要`_cache_reader`了
			FileCacheReader_Releasep(_cache_reader);
		}

		if ( !_save_path.isEmpty() ) { // Save file content to path, ignore cache
			if ( !_file_writer ) {
				FileWriter_new(this, _save_path, kNone_WriteCacheFlag, loop());
			}
			FileWriter_write(_file_writer, buffer);
		} else if ( _write_cache_flag && !is_disable_cache() ) {
			if ( !_file_writer ) {
				FileWriter_new(this, _cache_path, _write_cache_flag, loop());
			}
			FileWriter_write(_file_writer, buffer);
		} else {
			trigger_http_data2(buffer);
			read_advance();
		}
	}

	void Client::http_response_complete(bool fromCache) {
		if (!fromCache) {
			Qk_ASSERT(_connect);
			_pool->recovery(_connect, false);
			_connect = nullptr;

			if ( _status_code == 304) {
				if (_cache_reader) {
					auto expires = to_expires_from_cache_content(_response_header["cache-control"]);
					if (expires.isEmpty()) {
						expires = _response_header["expires"];
					}
					_response_header = std::move(FileCacheReader_header(_cache_reader)); // use local cache headers

					if (!expires.isEmpty() && expires != _response_header["expires"]) {
						// set expires value
						_write_cache_flag = kHeader_WriteCacheFlag; // only write response header
						_response_header["expires"] = expires;
					}
					FileCacheReader_read_advance(_cache_reader);
					return;
				} else {
					Qk_ELog("http response status code error, %d", _status_code);
				}
			}
		}

		if ( _file_writer ) {
			FileWriter_end(_file_writer);  // 通知已经结束
		} else {
			trigger_http_end();
		}
	}

	void Client::report_error_and_abort(cError& error) {
		_delegate->trigger_http_error(_host, error);
		abort();
	}

	void Client::trigger_http_timeout() {
		_delegate->trigger_http_timeout(_host);
		abort();
	}

	void Client::send_http() {
		Qk_ASSERT(_retain);
		Qk_ASSERT_EQ(_connect, nullptr);
		_pool->request(this, Cb([this](Cb::Data& evt) {
			auto c = reinterpret_cast<Connect*>(evt.data);
			if ( _wait_connect_id ) {
				if ( evt.error ) {
					report_error_and_abort(*evt.error);
				} else {
					Qk_ASSERT_EQ(_connect, nullptr);
					_connect = c;
					Connect_bind_client_and_send(_connect, this);
				}
			} else {
				_pool->recovery(c, false);
			}
		}, this));
	}

	void Client::cache_file_stat_cb(Callback<FileStat>::Data& e) {
		if (_retain) {
			if (e.error || e.data->size() < 50) { // no cache or cache is bad
				send_http();
			} else {
				FileCacheReader_new(this, e.data->size(), loop());
			}
		}
	}

	void Client::trigger_http_end() {
		end_(false);
	}

	void Client::end_(bool abort) {
		if ( _retain && !_retain->ending ) {
			_retain->ending = true;

			FileCacheReader_Releasep(_cache_reader);
			FileWriter_Releasep(_file_writer);
			_pool->recovery(_connect, abort);
			_connect = nullptr;
			_pause = false;
			_wait_connect_id = 0;

			if ( abort ) {
				trigger_http_readystate_change(HTTP_READY_STATE_INITIAL);
				delete _retain;
				_delegate->trigger_http_abort(_host);
			} else {
				trigger_http_readystate_change(HTTP_READY_STATE_COMPLETED);
				delete _retain;
				_delegate->trigger_http_end(_host);
			}
		}
	}

	void Client::send(Buffer data) throw(Error) {
		Qk_IfThrow(!_retain, ERR_REPEAT_CALL, "RetainRef repeat call");
		Qk_IfThrow(!_uri.is_null(), ERR_INVALID_FILE_PATH, "Invalid path" );
		Qk_IfThrow(_uri.type() == URI_HTTP ||
						_uri.type() == URI_HTTPS, ERR_INVALID_FILE_PATH, "Invalid path `%s`", *_uri.href());

		_post_data = data;
		_retain = new RetainRef(this);
		_pause = false;
		_url_no_cache_arg = false;
		_cache_path = http_cache_path() + '/' + hash_str(_uri.href());
		_write_cache_flag = kNone_WriteCacheFlag; // reset write cache flag

		int i = _uri.search().indexOf("__no_cache");
		if ( i != -1 && _uri.search()[i+9] != '=' ) {
			_url_no_cache_arg = true;
		}

		trigger_http_readystate_change(HTTP_READY_STATE_READY); // ready

		if ( !_retain )
			return; // abort

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

	void Client::abort() {
		end_(true);
	}

	void Client::check_is_can_modify() throw(Error) {
		Qk_IfThrow(!_retain, ERR_SENDIF_CANNOT_MODIFY, "Http request sending cannot modify property");
	}

	void Client::pause() {
		if ( _retain )
			_pause = true;
	}

	void Client::resume() {
		if ( _retain && _pause ) {
			_pause = false;
			auto r = reader();
			if ( r ) {
				r->read_advance();
			}
		}
	}

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
