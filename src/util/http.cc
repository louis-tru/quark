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

#include "./http.h"
#include "./net.h"
#include "./fs.h"
#include "./codec.h"
#include "./errno.h"
#include "../version.h"
#include "./list.h"
#include "./dict.h"
#include <http_parser.h>
#include <zlib.h>

namespace qk {
	#define BUFFER_SIZE (65535)

	typedef HttpClientRequest::Delegate HttpDelegate;

	String inl__uri_encode(cString& url, bool component = false, bool secondary = false);

	static cString string_method[5] = { "GET", "POST", "HEAD", "DELETE", "PUT" };
	static cString string_colon(": ");
	static cString string_space(" ");
	static cString string_header_end("\r\n");
	static cString string_max_age("max-age=");
	static cString content_type_form("application/x-www-form-urlencoded; Charset=utf-8");
	static cString content_type_multipart_form("multipart/form-data; boundary=----QuarkFormBoundaryrGKCBY7qhFd3TrwA");
	static cString multipart_boundary_start("------QuarkFormBoundaryrGKCBY7qhFd3TrwA\r\n");
	static cString multipart_boundary_end  ("------QuarkFormBoundaryrGKCBY7qhFd3TrwA--");

	// cache-control: max-age=100000
	// return: expires str, Sat Aug 10 2024 13:26:02 GMT+0800
	static String to_expires_from_cache_content(cString& cache_control) {
		if ( !cache_control.isEmpty() ) {
			int i = cache_control.indexOf(string_max_age);
			if ( i != -1 && i + string_max_age.length() < cache_control.length() ) {
				int j = cache_control.indexOf(',', i);
				String max_age = j != -1
				? cache_control.substring(i + string_max_age.length(), j)
				: cache_control.substring(i + string_max_age.length());
				
				int64_t num = max_age.trim().toNumber<int64_t>();
				if ( num > 0 ) {
					return gmt_time_string( time_second() + num ); // Thu, 30 Mar 2017 06:16:55 GMT
				}
			}
		}
		return String();
	}

	enum FormType {
		FORM_TYPE_TEXT,
		FORM_TYPE_FILE,
	};

	struct FormValue {
		FormType type;
		String   data;
		String   name;
	};

	struct MultipartFormValue {
		FormType type;
		String   data;
		String   headers;
	};

	class HttpClientRequest::Inl: public Reference, public Delegate {
	public:
		class Connect;
		class ConnectPool;
		typedef List<Connect*>::Iterator ConnectID;
		typedef HttpClientRequest::Inl Client;
		// trigger
		virtual void trigger_http_error(HttpClientRequest* req, cError& error) override {}
		virtual void trigger_http_write(HttpClientRequest* req) override {}
		virtual void trigger_http_header(HttpClientRequest* req) override {}
		virtual void trigger_http_data(HttpClientRequest* req, Buffer &buffer) override {}
		virtual void trigger_http_end(HttpClientRequest* req) override {}
		virtual void trigger_http_readystate_change(HttpClientRequest* req) override {}
		virtual void trigger_http_timeout(HttpClientRequest* req) override {}
		virtual void trigger_http_abort(HttpClientRequest* req) override {}

		Inl(HttpClientRequest* host, RunLoop* loop)
			: _host(host)
			, _loop(loop)
			, _delegate(this)
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
			, _disable_cache(false)
			, _disable_cookie(false)
			, _disable_send_cookie(false)
			, _disable_ssl_verify(false)
			, _keep_alive(true)
			, _retain(nullptr)
			, _timeout(0), _pause(false)
			, _url_no_cache_arg(false), _wait_connect_id(0)
			, _write_cache_flag(0)
		{
			Qk_ASSERT(loop);
		}

		~Inl() {
			abort();
			Qk_ASSERT(!_retain);
			Qk_ASSERT(!_connect);
			Qk_ASSERT(!_cache_reader);
			Qk_ASSERT(!_file_writer);
		}

		void set_delegate(HttpDelegate* delegate) {
			_delegate = delegate ? delegate : this;
		}
		RunLoop* loop() { return _loop; }
		DictSS& response_header() { return _response_header; }

		struct RetainRef {
			RetainRef(Inl* hold): hold(hold) {}
			~RetainRef() {
				hold->_retain = nullptr;
			}
			Sp<Inl> hold;
			bool    ending = false;
		};

		class Reader {
		public:
			virtual void read_advance() = 0;
			virtual void read_pause() = 0;
			virtual bool is_cache() = 0;
		};

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
				Qk_ASSERT(_socket);
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
					self->_client->_write_cache_flag = 2; // set write cache flag
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
				//g_debug("--http response parser on_body, %d", length);
				Connect* self = static_cast<Connect*>(parser->data);
				self->_client->_download_size += length;
				Buffer buff;
				if ( self->_z_strm.state ) {
					int r = self->gzip_inflate(at, uint32_t(length), buff);
					if (r < 0)
						Qk_ELog("un gzip err, %d", r);
				} else {
					buff = WeakBuffer(at, uint32_t(length))->copy();
				}
				if ( buff.length() ) {
					self->_client->trigger_http_data(buff);
				}
				return 0;
			}

			static int on_message_complete(http_parser* parser) {
				//g_debug("--http response parser on_message_complete");
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
					String last_modified = _client->_cache_reader->header()["last-modified"];
					String etag = _client->_cache_reader->header()["etag"];
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
							if ( i.value.type == FORM_TYPE_FILE ) {
								_is_multipart_form_data = true;
								break;
							}
						}

						if (_is_multipart_form_data ) {
							uint32_t content_length = multipart_boundary_end.length();

							for ( auto& i : _client->_form_data ) {
								FormValue& form = i.value;
								MultipartFormValue _form = { form.type, form.data };

								if ( i.value.type == FORM_TYPE_FILE ) {
									FileStat stat = fs_stat_sync(i.value.data);
									if ( stat.is_valid() && stat.is_file() ) {
										String basename = inl__uri_encode(fs_basename(form.data));
										_form.headers =
										String::format("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
																	 "Content-Type: application/octet-stream\r\n\r\n",
																	 *form.name, *basename);
										content_length += stat.size();
										_client->_upload_total += stat.size();
									} else {
										Error err(ERR_INVALID_FILE_PATH, "invalid upload path `%s`", i.value.data.c_str());
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
								String value = inl__uri_encode(i.value.data);
								_client->_post_data.write(i.key.c_str(), i.key.length());
								_client->_post_data.write("=", 1);
								_client->_post_data.write(*value, value.length());
								_client->_post_data.write("&", 1);
								_client->_upload_total += i.key.length() + value.length() + 2;
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
						, *inl__uri_encode(_client->_uri.pathname(), false, true)
						, search.c_str()
					)
				);

				for ( auto& i : header ) {
					header_str.push(i.key);       // name
					header_str.push(string_colon);  // :
					header_str.push(i.value);     // value
					header_str.push(string_header_end);    // \r\n
				}

				header_str.push(string_header_end); // \r\n

				_client->trigger_http_readystate_change(HTTP_READY_STATE_SENDING);
				_socket->resume();
				_socket->write(header_str.join(String()).collapse()); // write header
			}

			virtual void trigger_socket_timeout(Socket* socket) override {
				if ( _client ) {
					_client->trigger_http_timeout();
				}
			}

			virtual void trigger_socket_open(Socket* stream) override {
				if ( _client ) {
					send_http_request();
				}
			}

			virtual void trigger_socket_close(Socket* stream) override {
				if ( _client ) {
					_client->report_error_and_abort(
						Error(ERR_CONNECTING_UNEXPECTED_SHUTDOWN, "Connecting unexpected shutdown")
					);
				} else {
					_pool->recovery(this, true);
				}
			}

			virtual void trigger_socket_error(Socket* stream, cError& error) override {
				if ( _client ) {
					_client->report_error_and_abort(error);
				} else {
					_pool->recovery(this, true);
				}
			}

			virtual void trigger_socket_data(Socket* stream, cBuffer& buffer) override {
				if ( _client ) {
					// avoid releasing connections during data parsing
					Sp<Connect> sp(this); // retain Connect
					http_parser_execute(&_parser, &_settings, buffer.val(), buffer.length());
				}
			}

			virtual void trigger_socket_write(Socket* stream, Buffer& buffer, int flag) override {
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

			virtual void trigger_file_open(File* file) override {
				Qk_ASSERT( _is_multipart_form_data );
				send_multipart_form_data();
			}

			virtual void trigger_file_close(File* file) override {
				Qk_ASSERT( _is_multipart_form_data );
				Error err(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown");
				_client->report_error_and_abort(err);
			}

			virtual void trigger_file_error(File* file, cError& error) override {
				Qk_ASSERT( _is_multipart_form_data );
				_client->report_error_and_abort(error);
			}

			virtual void trigger_file_read(File* file, Buffer& buffer, int flag) override {
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

			virtual void trigger_file_write(File* file, Buffer& buffer, int flag) override {}

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

			virtual void read_advance() {
				_socket->resume();
			}

			virtual void read_pause() {
				_socket->pause();
			}

			virtual bool is_cache() {
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

		class ConnectPool {
		public:
			struct connect_req {
				Client* client;
				Cb cb;
				uint32_t wait_id;
				uint16_t port;
			};

			~ConnectPool() {
				ScopeLock scope(_mutex);
				for (auto i : _conns) {
					i->_id = ConnectID();
					Release(i); // no send to loop, immediately release
				}
			}

			void request(Client* cli, Cb cb) {
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
					cli->_wait_connect_id = cli->_loop->timer(Cb([this](auto&e){ // delay call task
						Lock lock(_mutex);
						call_req_tasks(&lock);
					}),1e5, -1); // 100ms
					_reqs.pushBack({ cli, cb, cli->_wait_connect_id, port }); // wait
				}
			}

			void recovery(Connect* c, bool immediatelyRelease) {
				if (!c) return;
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
						c->_recovery_time = time_micro();
						c->socket()->set_timeout(0);
						c->socket()->resume();
					}
				}
				call_req_tasks(&lock);
			}

		private:

			void call_req_tasks(Lock *lock) {
				for ( auto i = _reqs.begin(); i != _reqs.end(); ) {
					auto& req = *i;
					if (req.client->_wait_connect_id == req.wait_id) {
						auto conn = get_connect(req.client, req.port);
						if ( conn ) {
							conn->_busy = true;
							auto cli = req.client;
							Cb cb = req.cb;
							cli->loop()->timer_stop(req.wait_id);
							_reqs.erase(i++);
							lock->unlock(); // unlock
							cli->loop()->post(Cb([conn,cb](auto&e){
								cb->resolve(conn);
							},conn)); // async call
							break;
						}
					} else {
						req.client->loop()->timer_stop(req.wait_id);
						_reqs.erase(i++); // discard req
					}
				}
			}

			Connect* get_connect(Client* cli, uint16_t port) {
				Connect *conn = nullptr, *tryDel= nullptr; // try delete one connect
				uint32_t poolSize = 0;
				auto now = time_micro();
				auto max_pool_size = http_max_connect_pool_size();

				for ( auto i: _conns ) {
					if ( poolSize < max_pool_size ) {
						if (i->socket()->hostname() == cli->_uri.hostname() &&
								i->socket()->port() == port &&
								i->ssl() == (cli->_uri.type() == URI_HTTPS)
						) {
							if ( !i->_busy && now - i->_recovery_time > 8e4/*wait 80ms*/) { // available
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

			Mutex _mutex;
			List<Connect*> _conns;
			List<connect_req> _reqs;
		};

		class FileCacheReader: public File, public File::Delegate, public Reader {
		public:
			FileCacheReader(Client* client, int64_t size, RunLoop* loop)
				: File(client->_cache_path, loop)
				, _read_count(0)
				, _client(client)
				, _parse_header(true), _offset(0), _size(size)
			{
				Qk_ASSERT_EQ(_client->_cache_reader, nullptr);
				_client->_cache_reader = this;
				set_delegate(this);
				open();
			}

			~FileCacheReader() {
				_client->_cache_reader = nullptr;
			}
			
			void continue_send_and_release() {
				set_delegate(nullptr);
				_client->_cache_reader = nullptr;
				_client->send_http();
				release();
			}
			
			virtual void trigger_file_open(File* file) override {
				read(Buffer(511));
			}

			virtual void trigger_file_close(File* file) override {
				if ( _parse_header ) { // unexpected shutdown
					continue_send_and_release();
				} else {
					// throw error to http client host
					_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
				}
			}

			virtual void trigger_file_error(File* file, cError& error) override {
				if ( _parse_header ) {
					continue_send_and_release();
				} else {
					// throw error to http client host
					_client->report_error_and_abort(error);
				}
			}

			virtual void trigger_file_read(File* file, Buffer& buffer, int flag) override {
				if ( _parse_header ) { // parse cache header
					if ( buffer.length() ) {
						
						String str(buffer.val(), buffer.length()), s("\r\n"), s2(':');
						/*
						 Expires: Thu, 27 Apr 2017 11:57:20 GMT
						 Last-Modified: Fri, 18 Nov 2016 12:08:17 GMT
						 
						 ... Body ...
						 */

						for ( int i = 0; ; ) {
							int j = str.indexOf(s, i);
							if ( j != -1 && j != 0 ) {
								if ( j == i ) { // parse header end
									_parse_header = false;
									_offset += (j + 2); // save read offset
									_header["expires"] = _header["expires"].trim();

									int64_t expires = parse_time(_header["expires"]);
									if ( expires > time_micro() ) {
										_client->trigger_http_readystate_change(HTTP_READY_STATE_RESPONSE);
										_client->_download_total = Int64::max(_size - _offset, 0);
										_client->trigger_http_header(200, std::move(_header), true);
										read_advance();
									} else {
										// Qk_Log("Read -- %ld, %ld, %s", expires, time(), *_header.get("expires"));
										if (parse_time(_header["last-modified"]) > 0 ||
												!_header["etag"].isEmpty()
										) {
											_client->send_http();
										} else {
											continue_send_and_release(); // invalid cache
										}
									}
									// parse header end
									break;
								} else {
									int k = str.indexOf(s2, i);
									if ( k != -1 && k - i > 1 && j - k > 2 ) {
										// Qk_DLog("%s: %s", *str.substring(i, k), *str.substring(k + 2, j));
										_header[str.substring(i, k).lowerCase()] = str.substring(k + 2, j);
									}
								}
							} else {
								if ( i == 0 ) { // invalid cache
									continue_send_and_release();
								} else { // read next
									_offset += i;
									buffer.reset(511);
									read(buffer, _offset);
								}
								break;
							}
							i = j + 2;
						}
						
					} else { // no cache
						continue_send_and_release();
					}
				} else {
					// read cache
					_read_count--;
					Qk_ASSERT_EQ(_read_count, 0);
					
					if ( buffer.length() ) {
						_offset += buffer.length();
						_client->_download_size += buffer.length();
						_client->trigger_http_data(buffer);
					} else { // end
						_client->http_response_complete(true);
					}
				}
			}
			
			virtual void trigger_file_write(File* file, Buffer& buffer, int flag) override {}

			DictSS& header() {
				return _header;
			}

			virtual void read_advance() override /*Reader*/ {
				if ( !_parse_header ) {
					if ( _read_count == 0 ) {
						_read_count++;
						read(Buffer(BUFFER_SIZE), _offset);
					}
				}
			}

			virtual void read_pause() override /*Reader*/ {}

			virtual bool is_cache() override /*Reader*/ {
				return true;
			}

		private:
			int _read_count;
			Client* _client;
			DictSS _header;
			bool _parse_header;
			uint32_t  _offset;
			int64_t _size;
		};

		class FileWriter: public Object, public File::Delegate {
		public:
			FileWriter(Client* client, cString& path, int type, RunLoop* loop)
				: _client(client)
				, _file(nullptr)
				, _write_type(type)
				, _write_count(0), _offset(0)
				, _completed_end(false)
			{
				// type:
				// type = 0 only write body
				// type = 1 only write header
				// type = 2 write header and body

				Qk_ASSERT_EQ(_client->_file_writer, nullptr);
				_client->_file_writer = this;

				// Qk_Log("FileWriter _write_flag -- %i, %s", _write_flag, *path);

				if ( _write_type ) { // verification cache is valid
					auto headers = _client->response_header();
					Qk_ASSERT(headers.length());

					if ( headers.has("cache-control") ) {
						auto expires = to_expires_from_cache_content(headers["cache-control"]);
						// Qk_Log("FileWriter -- %s", *expires);
						if ( !expires.isEmpty() ) {
							headers["expires"] = expires;
						}
					}

					if ( headers.has("expires") ) {
						int64_t expires = parse_time(headers["expires"]);
						int64_t now = time_micro();
						if ( expires > now ) {
							_file = new File(path, loop);
						}
					} else if ( headers.has("last-modified") || headers.has("etag") ) {
						_file = new File(path, loop);
					}
				} else { // download save
					_file = new File(path, loop);
				}

				if ( _file ) {
					_file->set_delegate(this);
					if (_write_type == 1) { // only write header
						_file->open(FOPEN_WRONLY | FOPEN_CREAT); // keep old content
					} else {
						_file->open(FOPEN_W); // clear old content
					}
				}
			}

			~FileWriter() {
				Releasep(_file);
				_client->_file_writer = nullptr;
			}

			virtual void trigger_file_open(File* file) {
				if ( _write_type ) { // write header
					String header_str;
					auto& header = _client->response_header();

					for ( auto& i : header ) {
						if (!i.value.isEmpty() && i.key != "cache-control") {
							header_str += i.key;
							header_str += string_colon;
							if (i.key == "expires") {
								// 写入一个固定长度的时间字符串,方便以后重写这个值
								auto val = i.value;
								while (val.length() < 36)
									val.append(' ');
								header_str += val;
							} else {
								header_str += i.value;
							}
							header_str += string_header_end;
						}
					}
					header_str += string_header_end;
					_write_count++;
					_offset = header_str.length();
					_file->write(header_str.collapse(), 0, 1); // header write
				}
				for (auto &i: _buffer) {
					auto off = _offset;
					_write_count++;
					_offset += i.length();
					_file->write(i, off);
				}
				_buffer.clear();
			}

			virtual void trigger_file_close(File* file) {
				// throw error to http client host
				_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
			}

			virtual void trigger_file_error(File* file, cError& error) {
				_client->report_error_and_abort(error);
			}

			virtual void trigger_file_write(File* file, Buffer& buffer, int flag) {
				_write_count--; Qk_ASSERT(_write_count >= 0);
				if ( flag == 0 ) {
					_client->trigger_http_data2(buffer);
				}
				if ( _write_count == 0 ) {
					if ( _completed_end ) { // http end
						_client->trigger_http_end();
					} else {
						_client->read_advance();
					}
				}
			}

			virtual void trigger_file_read(File* file, Buffer& buffer, int flag) {}

			void write(Buffer& buffer) {
				Qk_ASSERT_EQ(_completed_end, false);
				if ( _file && _write_type != 1 ) {
					if ( _file->is_open() ) {
						if ( ++_write_count > 32 )
							_client->read_pause();
						auto off = _offset;
						_offset += buffer.length();
						_file->write(buffer, off);
					} else {
						_buffer.pushBack(std::move(buffer));
						_client->read_pause();
					}
				} else { // no file write task
					_client->trigger_http_data2(buffer);
					_client->read_advance();
				}
			}

			void end() {
				_completed_end = true;
				if ( _write_count == 0 && _buffer.length() == 0 ) { // file is write complete
					_client->trigger_http_end();
				}
			}

		private:
			Client* _client;
			File*  _file;
			List<Buffer> _buffer;
			int	 _write_type, _write_count, _offset;
			bool _completed_end;
		};

	private:
		Reader* reader() {
			return _connect ? (Reader*)_connect: (Reader*)_cache_reader;
		}

		void read_advance() {
			auto r = reader();
			Qk_ASSERT(r);
			if ( _pause ) {
				r->read_pause();
			} else {
				r->read_advance();
			}
		}

		void read_pause() {
			auto r = reader();
			Qk_ASSERT(r);
			r->read_pause();
		}

		bool is_disable_cache() {
			return _disable_cache || _url_no_cache_arg || _method != HTTP_METHOD_GET;
		}

		void trigger_http_readystate_change(HttpReadyState ready_state) {
			_ready_state = ready_state;
			_delegate->trigger_http_readystate_change(_host);
		}

		void trigger_http_write() {
			_delegate->trigger_http_write(_host);
		}

		void trigger_http_header(uint32_t status_code, DictSS&& header, bool fromCache) {
			_status_code = status_code;
			_response_header = std::move(header);
			_delegate->trigger_http_header(_host);
		}

		void trigger_http_data2(Buffer& buffer) {
			_delegate->trigger_http_data(_host, buffer);
		}

		void trigger_http_data(Buffer& buffer) {
			// _write_cache_flag:
			// _write_cache_flag = 0 not write cache
			// _write_cache_flag = 1 write response header
			// _write_cache_flag = 2 write response header and body

			if ( _write_cache_flag == 2 ) {
				// `_write_cache_flag==2` 写入头与主体缓存时,
				// 一定是由`Connect`发起的调用,所以已不再需要`_cache_reader`了
				if ( _cache_reader ) {
					_cache_reader->release();
					_cache_reader = nullptr;
				}
			}

			if ( !_save_path.isEmpty() ) {
				if ( !_file_writer ) {
					new FileWriter(this, _save_path, 0, loop());
				}
				_file_writer->write(buffer);
			} else if ( _write_cache_flag && !is_disable_cache() ) {
				if ( !_file_writer ) {
					new FileWriter(this, _cache_path, _write_cache_flag, loop());
				}
				_file_writer->write(buffer);
			} else {
				trigger_http_data2(buffer);
				read_advance();
			}
		}

		void http_response_complete(bool fromCache) {
			if (!fromCache) {
				Qk_ASSERT(_pool);
				Qk_ASSERT(_connect);
				_pool->recovery(_connect, false);
				_connect = nullptr;

				if ( _status_code == 304) {
					if (_cache_reader) {
						auto expires = to_expires_from_cache_content(_response_header["cache-control"]);
						if (expires.isEmpty()) {
							expires = _response_header["expires"];
						}
						_response_header = std::move(_cache_reader->header()); // use local cache headers

						if (!expires.isEmpty() && expires != _response_header["expires"]) {
							// set expires value
							_write_cache_flag = 1; // only write response header
							_response_header["expires"] = expires;
						}
						_cache_reader->read_advance();
						return;
					} else {
						Qk_ELog("http response status code error, %d", _status_code);
					}
				}
			}

			if ( _file_writer ) {
				_file_writer->end(); // 通知已经结束
			} else {
				trigger_http_end();
			}
		}

		void report_error_and_abort(cError& error) {
			_delegate->trigger_http_error(_host, error);
			abort();
		}

		void trigger_http_timeout() {
			_delegate->trigger_http_timeout(_host);
			abort();
		}

		void send_http() {
			Qk_ASSERT(_retain);
			Qk_ASSERT(_pool);
			Qk_ASSERT_EQ(_connect, nullptr);
			_pool->request(this, Cb([this](Cb::Data& evt) {
				auto c = static_cast<Connect*>(evt.data);
				if ( _wait_connect_id ) {
					if ( evt.error ) {
						report_error_and_abort(*evt.error);
					} else {
						Qk_ASSERT_EQ(_connect, nullptr);
						_connect = c;
						_connect->bind_client_and_send(this);
					}
				} else {
					_pool->recovery(c, false);
				}
			}, this));
		}

		void cache_file_stat_cb(Callback<FileStat>::Data& evt) {
			if ( _retain ) {
				if ( evt.error ) { //
					send_http();
				} else {
					new FileCacheReader(this, evt.data->size(), loop());
				}
			}
		}

		void trigger_http_end() {
			end_(false);
		}

		void end_(bool abort) {
			if ( _retain && !_retain->ending ) {
				_retain->ending = true;

				Qk_ASSERT(_pool);
				Releasep(_cache_reader);
				Releasep(_file_writer);
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

	public:
		void send(Buffer data) throw(Error) {
			Qk_Check(!_retain, ERR_REPEAT_CALL, "RetainRef repeat call");
			Qk_Check(!_uri.is_null(), ERR_INVALID_FILE_PATH, "Invalid path" );
			Qk_Check(_uri.type() == URI_HTTP ||
							_uri.type() == URI_HTTPS, ERR_INVALID_FILE_PATH, "Invalid path `%s`", *_uri.href());

			_post_data = data;
			_retain = new RetainRef(this);
			_pause = false;
			_url_no_cache_arg = false;
			_cache_path = http_cache_path() + '/' + hash(_uri.href());
			_write_cache_flag = 0;

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
				fs_stat(_cache_path, Callback<FileStat>(&Inl::cache_file_stat_cb, this));
			}
		}

		void abort() {
			end_(true);
		}

		void check_is_can_modify() throw(Error) {
			Qk_Check(!_retain, ERR_SENDIF_CANNOT_MODIFY, "Http request sending cannot modify property");
		}

		void pause() {
			if ( _retain )
				_pause = true;
		}

		void resume() {
			if ( _retain && _pause ) {
				_pause = false;
				auto r = reader();
				if ( r ) {
					r->read_advance();
				}
			}
		}

	private:
		HttpClientRequest* _host;
		RunLoop*      _loop;
		HttpDelegate* _delegate;
		int64_t      _upload_total;    /* 需上传到服务器数据总量 */
		int64_t      _upload_size;     /* 已写上传到服务器数据尺寸 */
		int64_t      _download_total;  /* 需下载数据总量 */
		int64_t      _download_size;   /* 已下载数据量 */
		HttpReadyState _ready_state; /* 请求状态 */
		int         _status_code;    /* 服务器响应http状态码 */
		HttpMethod  _method;
		URI         _uri;
		String      _save_path;
		Connect*    _connect;
		FileCacheReader* _cache_reader;
		FileWriter* _file_writer;
		DictSS      _request_header;
		DictSS      _response_header;
		Dict<String, FormValue> _form_data;
		Buffer      _post_data;
		String      _username;
		String      _password;
		String      _cache_path;
		String      _http_response_version;
		bool        _disable_cache;
		bool        _disable_cookie;
		bool        _disable_send_cookie;
		bool        _disable_ssl_verify;
		bool        _keep_alive;
		RetainRef*  _retain;
		uint64_t    _timeout;
		bool        _pause;
		bool        _url_no_cache_arg;
		uint32_t    _wait_connect_id, _write_cache_flag;
		static       ConnectPool* _pool;
		friend class HttpClientRequest;
	};

	static
	HttpClientRequest::Inl::ConnectPool connectPool;
	HttpClientRequest::Inl::ConnectPool* HttpClientRequest::Inl::_pool(&connectPool);

	HttpClientRequest::HttpClientRequest(RunLoop* loop): _inl(NewRetain<Inl>(this, loop))
	{}

	HttpClientRequest::~HttpClientRequest() {
		_inl->set_delegate(nullptr);
		_inl->abort();
		_inl->release();
		_inl = nullptr;
	}

	void HttpClientRequest::set_delegate(HttpDelegate* delegate) throw(Error) {
		_inl->check_is_can_modify();
		_inl->set_delegate(delegate);
	}

	void HttpClientRequest::set_method(HttpMethod method) throw(Error) {
		_inl->check_is_can_modify();
		if ( method < HTTP_METHOD_GET || method > HTTP_METHOD_PUT ) {
			method = HTTP_METHOD_GET;
		}
		_inl->_method = method;
	}

	void HttpClientRequest::set_url(cString& path) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_uri = URI(path);
	}

	void HttpClientRequest::set_save_path(cString& path) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_save_path = path;
	}

	void HttpClientRequest::set_username(cString& username) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_username = username;
	}

	void HttpClientRequest::set_password(cString& password) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_password = password;
	}

	void HttpClientRequest::disable_cache(bool disable) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_disable_cache = disable;
	}

	void HttpClientRequest::disable_cookie(bool disable) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_disable_cookie = disable;
	}

	void HttpClientRequest::disable_send_cookie(bool disable) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_disable_send_cookie = disable;
	}

	void HttpClientRequest::disable_ssl_verify(bool disable) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_disable_ssl_verify = disable;
	}

	void HttpClientRequest::set_keep_alive(bool keep_alive) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_keep_alive = keep_alive;
	}

	void HttpClientRequest::set_timeout(uint64_t timeout) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_timeout = timeout;
	}

	void HttpClientRequest::set_request_header(cString& name, cString& value) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_request_header[name] = value;
	}

	void HttpClientRequest::set_form(cString& form_name, cString& value) throw(Error) {
		_inl->check_is_can_modify();
		Qk_Check( value.length() <= BUFFER_SIZE,
							ERR_HTTP_FORM_SIZE_LIMIT, "Http form field size limit <= %d", BUFFER_SIZE);
		_inl->_form_data[form_name] = {
			FORM_TYPE_TEXT, value, inl__uri_encode(form_name)
		};
	}

	void HttpClientRequest::set_upload_file(cString& form_name, cString& path) throw(Error) {
		_inl->check_is_can_modify();
		_inl->_form_data[form_name] = {
			FORM_TYPE_FILE, path, inl__uri_encode(form_name)
		};
	}

	void HttpClientRequest::clear_request_header() throw(Error) {
		_inl->check_is_can_modify();
		_inl->_request_header.clear();
	}

	void HttpClientRequest::clear_form_data() throw(Error) {
		_inl->check_is_can_modify();
		_inl->_form_data.clear();
	}

	String HttpClientRequest::get_response_header(cString& name) {
		auto i = _inl->_response_header.find(name);
		if ( i == _inl->_response_header.end() ) return String();
		return i->value;
	}

	const DictSS& HttpClientRequest::get_all_response_headers() const {
		return _inl->_response_header;
	}

	int64_t HttpClientRequest::upload_total() const {
		return _inl->_upload_total;
	}

	int64_t HttpClientRequest::upload_size() const {
		return _inl->_upload_size;
	}

	int64_t HttpClientRequest::download_total() const {
		return _inl->_download_total;
	}

	int64_t HttpClientRequest::download_size() const {
		return _inl->_download_size;
	}

	HttpReadyState HttpClientRequest::ready_state() const {
		return _inl->_ready_state;
	}

	int HttpClientRequest::status_code() const {
		return _inl->_status_code;
	}

	String HttpClientRequest::url() const {
		return _inl->_uri.href();
	}

	void HttpClientRequest::send(Buffer data) throw(Error) {
		_inl->send(data);
	}

	void HttpClientRequest::send(cString& data) throw(Error) {
		_inl->send(data.copy().collapse());
	}

	void HttpClientRequest::pause() {
		_inl->pause();
	}

	void HttpClientRequest::resume() {
		_inl->resume();
	}

	void HttpClientRequest::abort() {
		_inl->abort();
	}

	String HttpClientRequest::http_response_version() const {
		return _inl->_http_response_version;
	}

}
