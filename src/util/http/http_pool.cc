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

	class Connect: public Reference
		, public Socket::Delegate
		, public Reader, public File::Delegate
	{
	public:
		Connect(cString& hostname, uint16_t  port, bool ssl, RunLoop* loop)
			: _ssl(ssl)
			, _busy(false)
			, _is_multipart_form_data(false)
			, _send_data(false)
			, _socket(nullptr)
			, _client(nullptr)
			, _upload_file(nullptr)
			, _loop(loop)
			, _recovery_time(0)
		{
			_socket = new Socket(hostname, port, _ssl, loop);
			_socket->set_delegate(this);

			memset(&_z_strm, 0, sizeof(_z_strm));

			_parser.data = this;
			http_parser_settings_init(&_settings); // init
			_settings.on_message_begin = &on_message_begin;
			_settings.on_status = &on_status;
			_settings.on_header_field = &on_header_field;
			_settings.on_header_value = &on_header_value;
			_settings.on_headers_complete = &on_headers_complete;
			_settings.on_body = &on_body;
			_settings.on_message_complete = &on_message_complete;
		}

		~Connect() {
			Qk_ASSERT(_id == ConnectID());
			Releasep(_socket);
			Releasep(_upload_file);
			gzip_inflate_end();
			Qk_DLog("Connect::~Connect()");
		}

		inline RunLoop* loop() { return _loop; }

		void bind_client_and_send(Client* client) {
			Qk_ASSERT(client);
			Qk_ASSERT(!_client);

			_client = client;
			_socket->set_timeout(_client->_timeout); // set timeout
			_socket->disable_ssl_verify(_client->_disable_ssl_verify);

			if ( _socket->is_open() ) {
				send_http_request(); // send request
			} else {
				_socket->connect();
			}
		}

		static int on_message_begin(http_parser* parser) {
			//Qk_DLog("--http response parser on_message_begin");
			auto self = static_cast<Connect*>(parser->data);
			self->_client->trigger_http_readystate_change(HTTP_READY_STATE_RESPONSE);
			return 0;
		}
		
		static int on_status(http_parser* parser, cChar *at, size_t length) {
			//Qk_DLog("http response parser on_status, %s %s", String(at - 4, 3).c(), String(at, uint32_t(length)).c());
			auto self = static_cast<Connect*>(parser->data);
			int status_code = String(at - 4, 3).toNumber<uint32_t>();
			if (status_code == 200) {
				self->_client->_write_cache_flag = kAll_WriteCacheFlag; // set write cache flag
			}
			Qk_ASSERT(status_code == parser->status_code);
			// Qk_Log("http %d,%d", int(parser->http_major), int(parser->http_minor));
			self->_client->_status_code = status_code;
			self->_client->_http_response_version =
				String::format("%d.%d", parser->http_major, parser->http_minor);
			return 0;
		}

		static int on_header_field(http_parser* parser, cChar *at, size_t length) {
			auto self = static_cast<Connect*>(parser->data);
			self->header_field_complete();
			self->_header_field.append(at, uint32_t(length));
			// Qk_DLog("on_header_field, %s", self->_header_field.c_str());
			return 0;
		}

		static int on_header_value(http_parser* parser, cChar *at, size_t length) {
			auto self = static_cast<Connect*>(parser->data);
			self->_header_value.append(at, uint32_t(length));
			return 0;
		}

		static int on_headers_complete(http_parser* parser) {
			//Qk_DLog("--http response parser on_headers_complete");
			auto self = static_cast<Connect*>(parser->data);
			auto cli = self->_client;
			self->header_field_complete();
			if ( self->_header.has("content-length") ) {
				cli->_download_total = self->_header["content-length"].toNumber<int64_t>();
			}
			self->gzip_inflate_init();
			cli->trigger_http_header(cli->_status_code, std::move(self->_header), 0);
			return 0;
		}

		void header_field_complete() {
			if (_header_value.length()) {
				_header_field.lowerCase();
				if ( !_client->_disable_cookie ) {
					if ( _header_field == "set-cookie" ) {
						http_set_cookie_with_expression(_client->_uri.domain(), _header_value);
					}
				}
				_header.set(_header_field, _header_value);
				_header_field = _header_value = String(); // clear field and value
			}
		}

		void gzip_inflate_init() {
			if ( _header.has("content-encoding") ) {
				String encoding = _header["content-encoding"];
				if ( encoding.indexOf("gzip") != -1 ) {
					Qk_ASSERT_EQ(0, inflateInit2(&_z_strm, 47));
				} else if ( encoding.indexOf("deflate") != -1 ) {
					Qk_ASSERT_EQ(0, inflateInit(&_z_strm));
				}
			}
		}

		void gzip_inflate_end() {
			if (_z_strm.state) {
				inflateEnd(&_z_strm);
			}
		}

		int gzip_inflate(cChar* data, uint32_t len, Buffer& out) {
			Buffer _z_strm_buff(16383); // 16k
			int r = 0;
			_z_strm.next_in = (uint8_t*)data;
			_z_strm.avail_in = len;
			do {
				_z_strm.next_out = (uint8_t*)*_z_strm_buff;
				_z_strm.avail_out = _z_strm_buff.length();
				r = inflate(&_z_strm, Z_NO_FLUSH);
				out.write(_z_strm_buff.val(), _z_strm_buff.length() - _z_strm.avail_out);
			} while(_z_strm.avail_out == 0);

			if ( r == Z_STREAM_END ) {
				gzip_inflate_end();
			}
			return r;
		}

		static int on_body(http_parser* parser, cChar *at, size_t length) {
			// Qk_DLog("Http response parser on_body, %d", length);
			auto self = static_cast<Connect*>(parser->data);
			self->_client->_download_size += length;
			Buffer buff;
			if ( self->_z_strm.state ) {
				int r = self->gzip_inflate(at, uint32_t(length), buff);
				if (r < 0)
					Qk_ELog("Http gzip inflate error, %d", r);
			} else {
				buff = WeakBuffer(at, uint32_t(length))->copy();
			}
			if ( buff.length() ) {
				self->_client->trigger_http_data(buff);
			}
			return 0;
		}

		static int on_message_complete(http_parser* parser) {
			// Qk_DLog("Http response parser on_message_complete");
			static_cast<Connect*>(parser->data)->_client->http_response_complete(false);
			return 0;
		}

		void send_http_request() {
			http_parser_init(&_parser, HTTP_RESPONSE);
			Releasep(_upload_file);
			_is_multipart_form_data = false;
			_send_data = false;
			_multipart_form_data.clear();
			_header.clear();
			gzip_inflate_end();

			DictSS header = _client->_request_header;

			header["Host"] = _client->_uri.host();
			header["Connection"] = _client->_keep_alive ? "keep-alive" : "close";
			header["Accept-Encoding"] = "gzip, deflate";
			header["Date"] = gmt_time_string(time_second());

			if ( !header.has("Cache-Control") )   header["Cache-Control"] = "max-age=0";
			if ( !header.has("User-Agent") )      header["User-Agent"] = http_user_agent();
			if ( !header.has("Accept-Charset") )  header["Accept-Charset"] = "utf-8";
			if ( !header.has("Accept") )          header["Accept"] = "*/*";
			if ( !header.has("DNT") )             header["DNT"] = "1";
			// if ( !header.has("Accept-Language") ) header["Accept-Language"] = languages();

			if ( !_client->_username.isEmpty() && !_client->_password.isEmpty() ) {
				String s = _client->_username + ':' + _client->_password;
				header["Authorization"] = codec_encode(kBase64_Encoding, s);
			}

			if ( !_client->_disable_cookie && !_client->_disable_send_cookie ) { // send cookies

				String cookies = http_get_all_cookie_string(_client->_uri.domain(),
																													_client->_uri.pathname(),
																													_client->_uri.type() == URI_HTTPS);
				if ( !cookies.isEmpty() ) {
					header["Cookie"] = cookies;
				}
			}

			if ( _client->_cache_reader ) {
				auto last_modified = FileCacheReader_header(_client->_cache_reader)["last-modified"];
				auto etag = FileCacheReader_header(_client->_cache_reader)["etag"];
				if ( !last_modified.isEmpty() )  {
					header["If-Modified-Since"] = std::move(last_modified);
				}
				if ( !etag.isEmpty() ) {
					header["If-None-Match"] = std::move(etag);
				}
			}

			if ( _client->_method == HTTP_METHOD_POST ) {

				if ( _client->_post_data.length() ) { // ignore form data
					if ( _client->_form_data.length() ) {
						Qk_Warn("Ignore form data");
					}
					_client->_upload_total = _client->_post_data.length();
					header["Content-Length"] = _client->_upload_total;
				}
				else if ( _client->_form_data.length() ) { // post form data

					for ( auto& i : _client->_form_data ) {
						if ( i.second.type == FORM_TYPE_FILE ) {
							_is_multipart_form_data = true;
							break;
						}
					}

					if (_is_multipart_form_data ) {
						uint32_t content_length = multipart_boundary_end.length();

						for ( auto& i : _client->_form_data ) {
							FormValue& form = i.second;
							MultipartFormValue _form = { form.type, form.data };

							if ( i.second.type == FORM_TYPE_FILE ) {
								FileStat stat = fs_stat_sync(i.second.data);
								if ( stat.is_valid() && stat.is_file() ) {
									String basename = uri_encode(fs_basename(form.data));
									_form.headers =
									String::format("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
																	"Content-Type: application/octet-stream\r\n\r\n",
																	*form.name, *basename);
									content_length += stat.size();
									_client->_upload_total += stat.size();
								} else {
									Error err(ERR_INVALID_FILE_PATH, "invalid upload path `%s`", i.second.data.c_str());
									_client->report_error_and_abort(err);
									return;
								}
							} else {
								_form.headers = String::format("Content-Disposition: form-data;"
																								"name=\"%s\"\r\n\r\n", *form.name);
								content_length += form.data.length();
								_client->_upload_total += form.data.length();
							}

							content_length += multipart_boundary_start.length();
							content_length += _form.headers.length();
							content_length += 2; // end \r\n
							_multipart_form_data.pushBack(_form);
						}

						header["Content-Length"] = content_length;
						header["Content-Type"] = content_type_multipart_form;
					} else {

						for ( auto& i : _client->_form_data ) {
							String value = uri_encode(i.second.data);
							_client->_post_data.write(i.first.c_str(), i.first.length());
							_client->_post_data.write("=", 1);
							_client->_post_data.write(*value, value.length());
							_client->_post_data.write("&", 1);
							_client->_upload_total += i.first.length() + value.length() + 2;
						}
						header["Content-Length"] = _client->_upload_total;
						header["Content-Type"] = content_type_form;
					}
				}
			}

			Array<String> header_str;
			auto search = _client->_uri.search();

			if (_client->_url_no_cache_arg) {
				search = search.replace("__no_cache", "");
				if (search.length() == 1) {
					search = String();
				}
			}

			header_str.push(
				String::format
				(
					"%s %s%s HTTP/1.1\r\n"
					, string_method[_client->_method].c_str()
					, *uri_encode(_client->_uri.pathname(), false, true)
					, search.c_str()
				)
			);

			for ( auto& i : header ) {
				header_str.push(i.first);       // name
				header_str.push(string_colon);  // :
				header_str.push(i.second);     // value
				header_str.push(string_header_end); // \r\n
			}
			header_str.push(string_header_end); // \r\n

			_client->trigger_http_readystate_change(HTTP_READY_STATE_SENDING);
			_socket->resume();
			_socket->write(header_str.join(String()).collapse()); // write header
		}

		void trigger_socket_timeout(Socket* socket) override {
			if ( _client ) {
				_client->trigger_http_timeout();
			}
		}

		void trigger_socket_open(Socket* stream) override {
			if ( _client ) {
				send_http_request();
			}
		}

		void trigger_socket_close(Socket* stream) override {
			if ( _client ) {
				_client->report_error_and_abort(
					Error(ERR_CONNECTING_UNEXPECTED_SHUTDOWN, "Connecting unexpected shutdown")
				);
			} else {
				Client::_pool->recovery(this, true);
			}
		}

		void trigger_socket_error(Socket* stream, cError& error) override {
			if ( _client ) {
				_client->report_error_and_abort(error);
			} else {
				Client::_pool->recovery(this, true);
			}
		}

		void trigger_socket_data(Socket* stream, cBuffer& buffer) override {
			if ( _client ) {
				// Avoid releasing connections during data parsing
				Sp<Connect> sp(this); // retain Connect
				http_parser_execute(&_parser, &_settings, buffer.val(), buffer.length());
			}
		}

		void trigger_socket_write(Socket* stream, Buffer& buffer, int flag) override {
			if ( !_client ) return;
			if ( _send_data ) {
				if ( flag == 1 ) {
					_client->_upload_size += buffer.length();
					_client->trigger_http_write();

					if ( _is_multipart_form_data ) {
						buffer.reset(BUFFER_SIZE);
						_multipart_form_tmp_buffer = buffer;
						send_multipart_form_data();
					}
				}
			}
			else if ( _client->_method == HTTP_METHOD_POST ) { // post data
				_send_data = true;
				if ( _client->_post_data.length() ) {
					_socket->write(_client->_post_data, 1);
				}
				else if ( _is_multipart_form_data ) { // send multipart/form-data
					if ( !_multipart_form_tmp_buffer.length() ) {
						_multipart_form_tmp_buffer = Buffer(BUFFER_SIZE);
					}
					send_multipart_form_data();
				}
			}
		}

		void trigger_file_open(File* file) override {
			Qk_ASSERT( _is_multipart_form_data );
			send_multipart_form_data();
		}

		void trigger_file_close(File* file) override {
			Qk_ASSERT( _is_multipart_form_data );
			Error err(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown");
			_client->report_error_and_abort(err);
		}

		void trigger_file_error(File* file, cError& error) override {
			Qk_ASSERT( _is_multipart_form_data );
			_client->report_error_and_abort(error);
		}

		void trigger_file_read(File* file, Buffer& buffer, int flag) override {
			Qk_ASSERT( _is_multipart_form_data );
			if ( buffer.length() ) {
				_socket->write(buffer, 1);
			} else { // read end
				Qk_ASSERT(_multipart_form_data.length());
				Qk_ASSERT(_upload_file);
				_socket->write(string_header_end.copy().collapse()); // \r\n
				_upload_file->release(); // release file
				_upload_file = nullptr;
				_multipart_form_data.popFront();
				buffer.reset(BUFFER_SIZE);
				_multipart_form_tmp_buffer = buffer;
				send_multipart_form_data();
			}
		}

		void trigger_file_write(File* file, Buffer& buffer, int flag) override {}

		void send_multipart_form_data() {
			Qk_ASSERT( _multipart_form_tmp_buffer.length() == BUFFER_SIZE );

			if ( _upload_file ) { // upload file
				Qk_ASSERT( _upload_file->is_open() );
				_upload_file->read(_multipart_form_tmp_buffer);
			}
			else if ( _multipart_form_data.length() ) {
				MultipartFormValue& form = *_multipart_form_data.begin();
				_socket->write(multipart_boundary_start.copy().collapse());
				_socket->write(form.headers.collapse());

				if ( form.type == FORM_TYPE_FILE ) { // file
					_upload_file = new File(form.data, _loop);
					_upload_file->set_delegate(this);
					_upload_file->open();
				} else { // text
					_multipart_form_tmp_buffer.write(form.data.c_str(), form.data.length(), 0);
					_multipart_form_tmp_buffer.reset(form.data.length());
					_socket->write(_multipart_form_tmp_buffer, 1);
					_socket->write(string_header_end.copy().collapse());
					_multipart_form_data.popFront();
				}
			} else {
				_socket->write(multipart_boundary_end.copy().collapse()); // end send data, wait http response
			}
		}

		bool ssl() { return _ssl; }

		Socket* socket() { return _socket; }

		void read_advance() override {
			_socket->resume();
		}

		void read_pause() override {
			_socket->pause();
		}

		bool is_cache() override {
			return false;
		}

	private:
		friend class ConnectPool;
		bool  _ssl;
		bool  _busy;
		bool  _is_multipart_form_data;
		bool  _send_data;
		Socket*     _socket;
		Client*     _client;
		ConnectID   _id;
		File*       _upload_file;
		http_parser _parser;
		http_parser_settings _settings;
		List<MultipartFormValue> _multipart_form_data;
		Buffer  _multipart_form_tmp_buffer;
		String  _header_field, _header_value;
		DictSS _header;
		z_stream _z_strm;
		RunLoop*  _loop;
		int64_t _recovery_time;
	};

	ConnectPool::ConnectPool() {
	}

	ConnectPool::~ConnectPool() {
		ScopeLock scope(_mutex);
		for (auto i : _conns) {
			i->_id = ConnectID();
			Release(i); // no send to loop, immediately release
		}
	}

	void ConnectPool::request(Client* cli, Cb cb) {
		Qk_ASSERT(cli);
		Qk_ASSERT(!cli->_uri.is_null());
		Qk_ASSERT(!cli->_uri.hostname().isEmpty());
		Qk_ASSERT(cli->_uri.type() == URI_HTTP || cli->_uri.type() == URI_HTTPS);

		uint16_t port = cli->_uri.port();
		if (!port) {
			port = cli->_uri.type() == URI_HTTP ? 80 : 443;
		}
		cli->_wait_connect_id = 1;

		Lock lock(_mutex);
		auto conn = get_connect(cli, port);
		if ( conn ) {
			conn->_busy = true;
			lock.unlock();
			cb->resolve(conn);
		} else {
			cli->_wait_connect_id = cli->_loop->timer(Cb([](auto e, auto ctx){ // delay call task
				Lock lock(ctx->_mutex);
				ctx->call_req_tasks(&lock);
			}, this), 1e2, -1); // after 100ms retry
			_reqs.pushBack({ cli, cb, cli->_wait_connect_id, port }); // wait
		}
	}

	void ConnectPool::recovery(Connect* c, bool immediatelyRelease) {
		if (!c)
			return;
		Lock lock(_mutex);

		if (c->_id == ConnectID())
			return;

		if ( !c->socket()->is_open() || immediatelyRelease ) { // immediately release
			_conns.erase(c->_id);
			c->_id = ConnectID();
			c->release();
		} else {
			if ( c->_busy ) {
				Qk_ASSERT_NE(c->_id, ConnectID());
				c->_busy = false;
				c->_client = nullptr;
				c->_recovery_time = time_microsecond();
				c->socket()->set_timeout(0);
				c->socket()->resume();
			}
		}
		call_req_tasks(&lock);
	}

	void ConnectPool::call_req_tasks(Lock *lock) {
		for ( auto i = _reqs.begin(); i != _reqs.end(); ) {
			auto j = i++;
			auto& req = *j;
			if (req.client->_wait_connect_id == req.wait_id) { // Not cancelled yet
				auto conn = get_connect(req.client, req.port);
				if (conn) {
					conn->_busy = true;
					auto cli = req.client;
					Cb cb = req.cb;
					cli->loop()->timer_stop(req.wait_id);
					_reqs.erase(j); // remove req
					lock->unlock(); // unlock
					cli->loop()->post(Cb([conn,cb](auto e) {
						cb->resolve(conn);
					},conn)); // async call
					break;
				}
			} else {
				req.client->loop()->timer_stop(req.wait_id);
				_reqs.erase(j); // remove req
			}
		}
	}

	Connect* ConnectPool::get_connect(Client* cli, uint16_t port) {
		Connect *conn = nullptr, *tryDel= nullptr; // try delete one connect
		uint32_t poolSize = 0;
		auto now = time_microsecond();
		auto max_pool_size = http_max_connect_pool_size();

		for ( auto i: _conns ) {
			if ( poolSize < max_pool_size ) {
				if (i->socket()->hostname() == cli->_uri.hostname() &&
						i->socket()->port() == port &&
						i->ssl() == (cli->_uri.type() == URI_HTTPS)
				) {
					if ( !i->_busy && now - i->_recovery_time > 8e4/*wait 80ms*/) { // It's after 80ms available
						if (i->loop() == cli->loop()) {
							conn = i; break;
						} else {
							tryDel = i;
						}
					}
					poolSize++;
				}
			}
		}
		Qk_ASSERT(poolSize <= max_pool_size);

		if (!conn) {
			if (poolSize == max_pool_size) {
				if (tryDel) {
					// The connection has reached its maximum limit,
					// but although it is available, it is not in the same loop, so it will be deleted
					_conns.erase(tryDel->_id);
					tryDel->_id = ConnectID();
					tryDel->loop()->post(Cb([tryDel](auto&e) {
						tryDel->release();
					}));
					poolSize--;
				}
			}
			if (poolSize < max_pool_size) {
				auto ssl = cli->_uri.type() == URI_HTTPS;
				conn = NewRetain<Connect>(cli->_uri.hostname(), port, ssl, cli->loop());
				conn->_id = _conns.pushBack(conn);
			}
		}

		return conn;
	}

	void Connect_bind_client_and_send(Connect *conn, Client* client) {
		conn->bind_client_and_send(client);
	}

	Reader* Connect_reader(Connect* self) {
		return self;
	}

	ConnectPool* Client::_pool = new ConnectPool();
}
