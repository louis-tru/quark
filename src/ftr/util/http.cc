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

#include "ftr/util/http.h"
#include "ftr/util/http-cookie.h"
#include "ftr/util/fs.h"
#include "ftr/version.h"
#include "ftr/util/net.h"
#include <http_parser.h>
#include <zlib.h>
#include <list>

namespace ftr {

typedef HttpClientRequest::Delegate HttpDelegate;

extern String inl__get_http_user_agent();
extern String inl__get_http_cache_path();
extern String inl__uri_encode(const String& url, bool component = false, bool secondary = false);

static const String string_method[5] = { "GET", "POST", "HEAD", "DELETE", "PUT" };
static const String string_colon(": ");
static const String string_space(" ");
static const String string_header_end("\r\n");
static const String string_max_age("max-age=");
static const String content_type_form("application/x-www-form-urlencoded; charset=utf-8");
static const String content_type_multipart_form("multipart/form-data; "
																								"boundary=----FtrFormBoundaryrGKCBY7qhFd3TrwA");
static const String multipart_boundary_start("------FtrFormBoundaryrGKCBY7qhFd3TrwA\r\n");
static const String multipart_boundary_end  ("------FtrFormBoundaryrGKCBY7qhFd3TrwA--");

#define MAX_CONNECT_COUNT (5)
#define BUFFER_SIZE (65536)

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

/**
 * @class HttpClientRequest::Inl
 */
class HttpClientRequest::Inl: public Reference, public Delegate {
 public:
	typedef HttpClientRequest::Inl Client;
	virtual void trigger_http_error(HttpClientRequest* req, cError& error) {}
	virtual void trigger_http_write(HttpClientRequest* req) {}
	virtual void trigger_http_header(HttpClientRequest* req) {}
	virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) {}
	virtual void trigger_http_end(HttpClientRequest* req) {}
	virtual void trigger_http_readystate_change(HttpClientRequest* req) {}
	virtual void trigger_http_timeout(HttpClientRequest* req) {}
	virtual void trigger_http_abort(HttpClientRequest* req) {}
	
	Inl(HttpClientRequest* host, RunLoop* loop)
	: _host(host)
	, _keep(loop->keep_alive("HttpClientRequest::Inl", false))
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
	//, _disable_ssl_verify_host(0)
	, _keep_alive(true)
	, _sending(nullptr)
	, _timeout(0), _pause(false)
	, _url_no_cache_arg(false), _wait_connect_id(0)
	, _write_cache_flag(0)
	{
		HttpHelper::initialize();
	}
	
	virtual ~Inl() {
		ASSERT(!_sending);
		ASSERT(!_connect);
		ASSERT(!_cache_reader);
		ASSERT(!_file_writer);
		Release(_keep); _keep = nullptr;
	}
	
	inline RunLoop* loop() { return _keep->host(); }
	inline uv_loop_t* uv_loop() { return loop()->uv_loop(); }
	
	void set_delegate(HttpDelegate* delegate) {
		_delegate = delegate ? delegate : this;
	}
	
	class Sending {
	 public:
		typedef NonObjectTraits Traits;
		Sending(Inl* host)
		: _host(host), _ending(false) {
			Retain(host);
		}
		~Sending() { Release(_host); }
		void release() {
			ASSERT(_host);
			_host->_sending = nullptr;
			delete this;
		}
		Inl*  _host;
		bool  _ending;
	};
	
	class Reader {
	 public:
		virtual void read_advance() = 0;
		virtual void read_pause() = 0;
		virtual bool is_cache() = 0;
	};
	
	class ConnectPool;
	class Connect;
	typedef std::list<Connect*>::iterator ConnectID;

	/**
	 * @class HttpClientRequest::Inl::Connect
	 */
	class Connect: public Object
	, public Socket::Delegate
	, public Reader, public AsyncFile::Delegate {
	 public:
		
		typedef std::list<Connect*>::iterator ID;
		
		Connect(const String& hostname, uint16_t  port, bool ssl, RunLoop* loop)
		: _ssl(ssl)
		, _use(false)
		, _is_multipart_form_data(false)
		, _send_data(false)
		, _socket(nullptr)
		, _client(nullptr)
		, _upload_file(nullptr)
		, _z_gzip(0), _loop(loop) { //

			if ( _ssl ) {
				_socket = new SSLSocket(hostname, port, loop);
			} else {
				_socket = new Socket(hostname, port, loop);
			}
			
			ASSERT(_socket);
			_socket->set_delegate(this);
			
			_parser.data = this;
			http_parser_settings_init(&_settings);
			_settings.on_message_begin = &on_message_begin;
			_settings.on_status = &on_status;
			_settings.on_header_field = &on_header_field;
			_settings.on_header_value = &on_header_value;
			_settings.on_headers_complete = &on_headers_complete;
			_settings.on_body = &on_body;
			_settings.on_message_complete = &on_message_complete;
		}
		
		~Connect() {
			ASSERT(_id.is_null());
			Release(_socket);     _socket = nullptr;
			Release(_upload_file);_upload_file = nullptr;
		}
		
		inline RunLoop* loop() { return _loop; }
		
		void bind_client_and_send(Client* client) {
			ASSERT(client);
			ASSERT(!_client);
			
			_client = client;
			_socket->set_timeout(_client->_timeout); // set timeout
			
			if ( _ssl ) {
				static_cast<SSLSocket*>(_socket)->disable_ssl_verify(_client->_disable_ssl_verify);
			}
			
			if ( _socket->is_open() ) {
				send_http_request(); // send request
			} else {
				_socket->open();
			}
		}
		
		static int on_message_begin(http_parser* parser) {
			//g_debug("--http response parser on_message_begin");
			Connect* self = static_cast<Connect*>(parser->data);
			self->_client->trigger_http_readystate_change(HTTP_READY_STATE_RESPONSE);
			return 0;
		}
		
		static int on_status(http_parser* parser, const char *at, size_t length) {
			//g_debug("http response parser on_status, %s %s", String(at - 4, 3).c(), String(at, uint32_t(length)).c());
			Connect* self = static_cast<Connect*>(parser->data);
			int status_code = String(at - 4, 3).to_uint32_t();
			if (status_code == 200) {
				self->_client->_write_cache_flag = 2; // set write cache flag
			}
			ASSERT(status_code == parser->status_code);
			// LOG("http %d,%d", int(parser->http_major), int(parser->http_minor));
			self->_client->_status_code = status_code;
			self->_client->_http_response_version = 
				String(parser->http_major) + '.' + parser->http_minor;
			return 0;
		}
		
		static int on_header_field(http_parser* parser, const char *at, size_t length) {
			//g_debug("http response parser on_header_field, %s", String(at, uint32_t(length)).c());
			static_cast<Connect*>(parser->data)->_header_field = String(at, uint32_t(length)).lower_case();
			return 0;
		}
		
		static int on_header_value(http_parser* parser, const char *at, size_t length) {
			//g_debug("http response parser on_header_value, %s", String(at, uint32_t(length)).c());
			Connect* self = static_cast<Connect*>(parser->data);
			String value(at, uint32_t(length));
			
			if ( !self->_client->_disable_cookie ) {
				if ( self->_header_field == "set-cookie" ) {
					http_cookie_set_with_expression(self->_client->_uri.domain(), value);
				}
			}
			
			self->_header.set( std::move(self->_header_field), value );
			
			return 0;
		}
		
		static int on_headers_complete(http_parser* parser) {
			//g_debug("--http response parser on_headers_complete");
			Connect* self = static_cast<Connect*>(parser->data);
			Client* cli = self->_client;
			if ( self->_header.has("content-length") ) {
				cli->_download_total = self->_header.get("content-length").to_int64_t();
			}
			self->init_gzip_parser();
			cli->trigger_http_header(cli->_status_code, std::move(self->_header), 0);
			return 0;
		}
		
		void init_gzip_parser() {
			if ( _header.has("content-encoding") ) {

				_z_strm.zalloc = Z_NULL;
				_z_strm.zfree = Z_NULL;
				_z_strm.opaque = Z_NULL;
				_z_strm.next_in = Z_NULL;
				_z_strm.avail_in = 0;
				
				String encoding = _header.get("content-encoding");
				if ( encoding.index_of("gzip") != -1 ) {
					_z_gzip = 2;
					inflateInit2(&_z_strm, 47);
				} else if ( encoding.index_of("deflate") != -1 ) {
					_z_gzip = 1;
					inflateInit(&_z_strm);
				}
			}
		}
		
		int gzip_inflate(const char* data, uint32_t len, Buffer& out) {
			static Buffer _z_strm_buff(16384); // 16k
			
			int r = 0;
			
			_z_strm.next_in = (uint8_t*)data;
			_z_strm.avail_in = len;
			do {
				_z_strm.next_out = (uint8_t*)*_z_strm_buff;
				_z_strm.avail_out = _z_strm_buff.length();
				r = inflate(&_z_strm, Z_NO_FLUSH);
				out.write(_z_strm_buff, -1, _z_strm_buff.length() - _z_strm.avail_out);
			} while(_z_strm.avail_out == 0);
			
			if ( r == Z_STREAM_END ) {
				inflateEnd(&_z_strm);
			}
			
			return r;
		}
		
		static int on_body(http_parser* parser, const char *at, size_t length) {
			//g_debug("--http response parser on_body, %d", length);
			Connect* self = static_cast<Connect*>(parser->data);
			self->_client->_download_size += length;
			Buffer buff;
			if ( self->_z_gzip ) {
				int r = self->gzip_inflate(at, uint32_t(length), buff);
				if (r < 0) {
					FX_ERR("un gzip err, %d", r);
				}
			} else {
				buff = WeakBuffer(at, uint32_t(length)).copy();
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
			Release(_upload_file); _upload_file = nullptr;
			_is_multipart_form_data = false;
			_send_data = false;
			_multipart_form_data.clear();
			_z_gzip = 0;
			_header.clear();
			//
			
			Map<String, String> header = _client->_request_header;
			
			header.set("Host", _client->_uri.host());
			header.set("Connection", _client->_keep_alive ? "keep-alive" : "close");
			header.set("Accept-Encoding", "gzip, deflate");
			header.set("Date", gmt_time_string(sys::time_second()));
			
			if ( !header.has("Cache-Control") )   header.set("Cache-Control", "max-age=0");
			if ( !header.has("User-Agent") )      header.set("User-Agent", inl__get_http_user_agent());
			if ( !header.has("Accept-Charset") )  header.set("Accept-Charset", "utf-8");
			if ( !header.has("Accept") )          header.set("Accept", "*/*");
			if ( !header.has("DNT") )             header.set("DNT", "1");
			if ( !header.has("Accept-Language") ) header.set("Accept-Language", sys::languages());
			
			if ( !_client->_username.is_empty() && !_client->_password.is_empty() ) {
				String s = _client->_username + ':' + _client->_password;
				header.set("Authorization", Codec::encoding(Encoding::BASE64, s));
			}
			
			if ( !_client->_disable_cookie && !_client->_disable_send_cookie ) { // send cookies
				
				String cookies = http_cookie_get_all_string(_client->_uri.domain(),
																										_client->_uri.pathname(),
																										_client->_uri.type() == URI_HTTPS);
				if ( !cookies.is_empty() ) {
					header.set("Cookie", cookies);
				}
			}
			
			if ( _client->_cache_reader ) {
				String last_modified = _client->_cache_reader->header()["last-modified"];
				String etag = _client->_cache_reader->header()["etag"];
				if ( !last_modified.is_empty() )  {
					header.set("If-Modified-Since", std::move(last_modified) );
				}
				if ( !etag.is_empty() ) {
					header.set("If-None-Match", std::move(etag) );
				}
			}
			
			if ( _client->_method == HTTP_METHOD_POST ) {
				
				if ( _client->_post_data.length() ) { // ignore form data
					if ( _client->_post_form_data.length() ) {
						FX_WARN("Ignore form data");
					}
					_client->_upload_total = _client->_post_data.length();
					header.set("Content-Length", _client->_upload_total);
				}
				else if ( _client->_post_form_data.length() ) { // post form data
					
					for ( auto& i : _client->_post_form_data ) {
						if ( i.value().type == FORM_TYPE_FILE ) {
							_is_multipart_form_data = true; break;
						}
					}
					
					if (_is_multipart_form_data ) {
						
						uint32_t content_length = multipart_boundary_end.length();
						
						for ( auto& i : _client->_post_form_data ) {
							FormValue& form = i.value();
							MultipartFormValue _form = { form.type, form.data };
							
							if ( i.value().type == FORM_TYPE_FILE ) {
								FileStat stat = FileHelper::stat_sync(i.value().data);
								if ( stat.is_valid() && stat.is_file() ) {
									String basename = inl__uri_encode(Path::basename(form.data));
									_form.headers =
									String::format("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
																 "Content-Type: application/octet-stream\r\n\r\n",
																 *form.name, *basename);
									content_length += stat.size();
									_client->_upload_total += stat.size();
								} else {
									Error err(ERR_INVALID_PATH, "invalid upload path `%s`", i.value().data.c());
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
							content_length += 2; // end
							_multipart_form_data.push(_form);
						}
						
						header.set("Content-Length", content_length);
						header.set("Content-Type", content_type_multipart_form);
					} else {
						
						for ( auto& i : _client->_post_form_data ) {
							String value = inl__uri_encode(i.value().data);
							_client->_post_data.write(i.key().c(), -1, i.key().length());
							_client->_post_data.write("=", -1, 1);
							_client->_post_data.write(*value, -1, value.length());
							_client->_post_data.write("&", -1, 1);
							_client->_upload_total += i.key().length() + value.length() + 2;
						}
						header.set("Content-Length", _client->_upload_total);
						header.set("Content-Type", content_type_form);
					}
				}
			}
			
			StringBuilder header_str;
			String search = _client->_uri.search();

			if (_client->_url_no_cache_arg) {
				search = search.replace("__no_cache", "");
				if (search.length() == 1) {
					search = String();
				}
			}
			
			header_str.push(String::format
			(
			"%s %s%s HTTP/1.1\r\n"
			 , string_method[_client->_method].c()
			 , *inl__uri_encode(_client->_uri.pathname(), false, true)
			 , search.c()
			));
			
			for ( auto& i : header ) {
				header_str.push(i.key());       // name
				header_str.push(string_colon);  // :
				header_str.push(i.value());     // value
				header_str.push(string_header_end);    // \r\n
			}
			
			header_str.push(string_header_end); // \r\n
			
			_client->trigger_http_readystate_change(HTTP_READY_STATE_SENDING);
			
			_socket->resume();
			_socket->write(header_str.to_buffer()); // write header
		}
		
		virtual void trigger_socket_timeout(Socket* socket) {
			if ( _client ) {
				_client->trigger_http_timeout();
			}
		}
		
		virtual void trigger_socket_open(Socket* stream) {
			if ( _client ) {
				send_http_request();
			}
		}
		
		virtual void trigger_socket_close(Socket* stream) {
			if ( _client ) {
				Error err(ERR_CONNECT_UNEXPECTED_SHUTDOWN, "Connect unexpected shutdown");
				_client->report_error_and_abort(err);
			} else {
				_pool.release(this, true);
			}
		}
		
		virtual void trigger_socket_error(Socket* stream, cError& error) {
			if ( _client ) {
				_client->report_error_and_abort(error);
			}
		}
		
		virtual void trigger_socket_data(Socket* stream, Buffer& buffer) {
			if ( _client ) {
				http_parser_execute(&_parser, &_settings, buffer.value(), buffer.length());
			}
		}
		
		virtual void trigger_socket_write(Socket* stream, Buffer buffer, int mark) {
			if ( !_client ) return;
			if ( _send_data ) {
				if ( mark == 1 ) {
					_client->_upload_size += buffer.length();
					_client->trigger_http_write();
					
					if ( _is_multipart_form_data ) {
						_multipart_form_buffer = buffer.realloc(BUFFER_SIZE);
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
					if ( !_multipart_form_buffer.length() ) {
						_multipart_form_buffer = Buffer(BUFFER_SIZE);
					}
					send_multipart_form_data();
				}
			}
		}
		
		virtual void trigger_async_file_open(AsyncFile* file) {
			ASSERT( _is_multipart_form_data );
			send_multipart_form_data();
		}
		
		virtual void trigger_async_file_close(AsyncFile* file) {
			ASSERT( _is_multipart_form_data );
			Error err(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown");
			_client->report_error_and_abort(err);
		}
		
		virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
			ASSERT( _is_multipart_form_data );
			_client->report_error_and_abort(error);
		}
		
		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {
			ASSERT( _is_multipart_form_data );
			if ( buffer.length() ) {
				_socket->write(buffer, 1);
			} else {
				ASSERT(_multipart_form_data.length());
				ASSERT(_upload_file);
				_socket->write(string_header_end.copy_buffer()); // \r\n
				_upload_file->release(); // release file
				_upload_file = nullptr;
				_multipart_form_data.shift();
				_multipart_form_buffer = buffer.realloc(BUFFER_SIZE);
				send_multipart_form_data();
			}
		}
		
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) { }
		
		void send_multipart_form_data() {
			ASSERT( _multipart_form_buffer.length() == BUFFER_SIZE );
			
			if ( _upload_file ) { // upload file
				ASSERT( _upload_file->is_open() );
				_upload_file->read(_multipart_form_buffer);
			}
			else if ( _multipart_form_data.length() ) {
				MultipartFormValue& form = _multipart_form_data.begin().value();
				_socket->write(multipart_boundary_start.copy_buffer());
				_socket->write(form.headers.collapse_buffer());
				
				if ( form.type == FORM_TYPE_FILE ) {
					_upload_file = new AsyncFile(form.data, _loop);
					_upload_file->set_delegate(this);
					_upload_file->open();
				} else {
					_multipart_form_buffer.write( form.data.c(), 0, form.data.length() );
					_socket->write(_multipart_form_buffer.realloc(form.data.length()), 1);
					_socket->write(string_header_end.copy_buffer());
					_multipart_form_data.shift();
				}
			} else {
				_socket->write(multipart_boundary_end.copy_buffer()); // end send data, wait http response
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
		bool  _use;
		bool  _is_multipart_form_data;
		bool  _send_data;
		Socket*     _socket;
		Client*     _client;
		ID          _id;
		AsyncFile*  _upload_file;
		http_parser _parser;
		http_parser_settings _settings;
		List<MultipartFormValue> _multipart_form_data;
		Buffer  _multipart_form_buffer;
		String  _header_field;
		Map<String, String> _header;
		z_stream _z_strm;
		int      _z_gzip;
		RunLoop*  _loop;
	};

	/**
	 * @class HttpClientRequest::Inl::ConnectPool
	 */
	class ConnectPool {
	 public:
		
		struct connect_req {
			Client* client;
			Callback<> cb;
			uint32_t wait_id;
			String  hostname;
			uint16_t   port;
			URIType uri_type;
		};
		
		ConnectPool() {
			_pool_ptr = this;
		}
		
		~ConnectPool() {
			ScopeLock scope(_mutex);
			for (auto& i : _pool) {
				auto con = i.value();
				con->_id = ConnectID();
				con->_loop->post(Cb([con](CbD& e){
					Release(con);
				}));
			}
			_pool_ptr = nullptr;
		}
		
		void get_connect(Client* client, cCb& cb) {
			ASSERT(client);
			ASSERT(!client->_uri.is_null());
			ASSERT(!client->_uri.hostname().is_empty());
			ASSERT(client->_uri.type() == URI_HTTP || client->_uri.type() == URI_HTTPS);
			
			uint16_t  port = client->_uri.port();
			if (!port) {
				port = client->_uri.type() == URI_HTTP ? 80 : 443;
			}
			
			client->_wait_connect_id = iid32();
			
			connect_req req = {
				client,
				cb,
				client->_wait_connect_id,
				client->_uri.hostname(),
				port,
				client->_uri.type(),
			};
			
			Connect* conn = nullptr;
			{ //
				ScopeLock scope(_mutex);
				conn = get_connect2(req);
				if ( conn ) {
					conn->_use = true;
				} else {
					_connect_req.push(req); // wait
				}
			}
			if (conn) {
				CbD evt = { 0, conn };
				cb->call(evt);
			}
		}
		
		Connect* get_connect2(connect_req& req) {
			Connect* conn = nullptr;
			Connect* conn2 = nullptr;
			uint32_t connect_count = 0;
			
			for ( auto& i : _pool ) {
				if ( connect_count < MAX_CONNECT_COUNT ) {
					Connect* connect = i.value();
					if (connect->socket()->hostname() == req.hostname &&
							connect->socket()->port() == req.port &&
							connect->ssl() == (req.uri_type == URI_HTTPS)
					) {
						connect_count++;
						if ( !connect->_use ) {
							if (connect->loop() == req.client->loop()) {
								conn = connect; break;
							} else {
								conn2 = connect;
							}
						}
					}
				}
			}
			
			ASSERT(connect_count <= MAX_CONNECT_COUNT);
			
			if (!conn) {
				if (connect_count == MAX_CONNECT_COUNT) {
					if (conn2) {
						// 连接已达到最大限制然而这个连接虽然可用但不在同一个loop,所以删除
						connect_count--;
						_pool.del(conn2->_id);
						conn2->_id = ConnectID();
						conn2->loop()->post(Cb([conn2](CbD& e) {
							conn2->release();
						}));
					}
				}
				if (connect_count < MAX_CONNECT_COUNT) {
					conn = new Connect(req.hostname,
														 req.port,
														 req.uri_type == URI_HTTPS,
														 req.client->loop());
					conn->_id = _pool.push(conn);
				}
			}
			return conn;
		}
		
		void release(Connect* connect, bool immediately) {
			if (!connect) return;
			Lock lock(_mutex);
			if (connect->_id.is_null()) {
				return;
			}
			
			if ( !connect->socket()->is_open() || immediately ) { // immediately release
				_pool.del(connect->_id);
				connect->_id = ConnectID();
				connect->release();
			} else {
				if ( connect->_use ) {
					ASSERT( !connect->_id.is_null() );
					connect->_use = false;
					connect->_client = nullptr;
					connect->socket()->set_timeout(0);
					connect->socket()->resume();
				}
			}
			
			for ( auto& i : _connect_req ) {
				connect_req& req = i.value();
				if (req.client->_wait_connect_id == req.wait_id) {
					Connect* conn = get_connect2(req);
					if ( conn ) {
						conn->_use = true;
						Cb cb = req.cb;
						_connect_req.del(i);
						lock.unlock(); // unlock
						CbD evt = { 0, conn };
						cb->call( evt );
						break;
					}
				} else {
					_connect_req.del(i); // discard req
				}
			}
		}
		
	 private:
		Mutex _mutex;
		List<Connect*>  _pool;
		List<connect_req> _connect_req;
	};
	
	/**
	 * @class HttpClientRequest::Inl::FileCacheReader
	 */
	class FileCacheReader: public AsyncFile, 
		public AsyncFile::Delegate, public Reader 
	{
	 public:
		FileCacheReader(Client* client, int64_t size, RunLoop* loop)
		: AsyncFile(client->_cache_path, loop)
		, _read_count(0)
		, _client(client)
		, _parse_header(true), _offset(0), _size(size) {
			ASSERT(!_client->_cache_reader);
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
		
		virtual void trigger_async_file_open(AsyncFile* file) {
			read(Buffer(512));
		}

		virtual void trigger_async_file_close(AsyncFile* file) {
			if ( _parse_header ) { // unexpected shutdown
				continue_send_and_release();
			} else {
				// throw error to http client host
				_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
			}
		}

		virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
			if ( _parse_header ) {
				continue_send_and_release();
			} else {
				// throw error to http client host
				_client->report_error_and_abort(error);
			}
		}

		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {
			if ( _parse_header ) { // parse cache header
				if ( buffer.length() ) {
					
					String str(buffer.value(), buffer.length()), s("\r\n"), s2(':');
					/*
					 Expires: Thu, 27 Apr 2017 11:57:20 GMT
					 Last-Modified: Fri, 18 Nov 2016 12:08:17 GMT
					 
					 ... Body ...
					 */
					
					for ( int i = 0; ; ) {
						int j = str.index_of(s, i);
						if ( j != -1 && j != 0 ) {
							if ( j == i ) { // parse header end
								_parse_header = false;
								_offset += (j + 2);

								int64_t expires = parse_time(_header.get("expires"));
								if ( expires > sys::time() ) {
									_client->trigger_http_readystate_change(HTTP_READY_STATE_RESPONSE);
									_client->_download_total = FX_MAX(_size - _offset, 0);
									_client->trigger_http_header(200, std::move(_header), true);
									read_advance();
								} else {
									// LOG("Read -- %ld, %ld, %s", expires, sys::time(), *_header.get("expires"));
									if (parse_time(_header.get("last-modified")) > 0 ||
											!_header.get("etag").is_empty()
									) {
										_client->send_http();
									} else {
										continue_send_and_release(); // full invalid
									}
								}
								// parse header end
								break;
							} else {
								int k = str.index_of(s2, i);
								if ( k != -1 && k - i > 1 && j - k > 2 ) {
									// LOG("  %s:-> %s", str.substring(i, k).lower_case().c(), str.substring(k + 2, j).c());
									_header.set(str.substring(i, k).lower_case(), str.substring(k + 2, j));
								}
							}
						} else {
							if ( i == 0 ) { // invalid cache
								continue_send_and_release();
							} else { // read next
								_offset += i;
								read(buffer.realloc(512), _offset);
							}
							break;
						}
						i = j + 2;
					}
					
				} else {
					// no cache
					continue_send_and_release();
				}
			} else {
				// read cache
				_read_count--;
				ASSERT(_read_count == 0);
				
				if ( buffer.length() ) {
					_offset += buffer.length();
					_client->_download_size += buffer.length();
					_client->trigger_http_data(buffer);
				} else { // end
					_client->http_response_complete(true);
				}
			}
		}
		
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) {}
		
		Map<String, String>& header() {
			return _header;
		}
		
		virtual void read_advance() {
			if ( !_parse_header ) {
				if ( _read_count == 0 ) {
					_read_count++;
					read(Buffer(BUFFER_SIZE), _offset);
				}
			}
		}
		
		virtual void read_pause() {}
		
		virtual bool is_cache() {
			return true;
		}

	 private:
		int _read_count;
		Client* _client;
		Map<String, String> _header;
		bool _parse_header;
		uint32_t  _offset;
		int64_t _size;
	};

	Map<String, String>& response_header() {
		return _response_header;
	}

	static String convert_to_expires(const String& cache_control) {
		if ( !cache_control.is_empty() ) {
			int i = cache_control.index_of(string_max_age);
			if ( i != -1 && i + string_max_age.length() < cache_control.length() ) {
				int j = cache_control.index_of(',', i);
				String max_age = j != -1
				? cache_control.substring(i + string_max_age.length(), j)
				: cache_control.substring(i + string_max_age.length());
				
				int64_t num = max_age.trim().to_int64_t();
				if ( num > 0 ) {
					return gmt_time_string( sys::time_second() + num );
				}
			}
		}
		return String();
	}

	/**
	 * @class HttpClientRequest::Inl::FileWriter
	 */
	class FileWriter: public Object, public AsyncFile::Delegate {
	 public:
		FileWriter(Client* client, const String& path, int flag, RunLoop* loop)
		: _client(client)
		, _file(nullptr)
		, _write_flag(flag)
		, _write_count(0)
		, _ready(0), _completed_end(0) {
			// flag:
			// flag = 0 only write body 
			// flag = 1 only write header 
			// flag = 2 write header and body

			ASSERT(!_client->_file_writer);
			_client->_file_writer = this;

			// LOG("FileWriter _write_flag -- %i, %s", _write_flag, *path);
			
			if ( _write_flag ) { // verification cache is valid
				auto& r_header = _client->response_header(); 
				ASSERT(r_header.length());

				if ( r_header.has("cache-control") ) {
					String expires = convert_to_expires(r_header.get("cache-control"));
					// LOG("FileWriter -- %s", *expires);
					if ( !expires.is_empty() ) {
						r_header.set("expires", expires);
					}
				}

				if ( r_header.has("expires") ) {
					int64_t expires = parse_time(r_header.get("expires"));
					int64_t now = sys::time();
					if ( expires > now ) {
						_file = new AsyncFile(path, loop);
					}
				} else if ( r_header.has("last-modified") || r_header.has("etag") ) {
					_file = new AsyncFile(path, loop);
				}
			} else { // download save
				_file = new AsyncFile(path, loop);
			}
			
			if ( _file ) {
				_file->set_delegate(this);
				if (_write_flag == 1) { // only write header
					_file->open(FOPEN_WRONLY | FOPEN_CREAT);
				} else {
					_file->open(FOPEN_W);
				}
			}
		}
		
		~FileWriter() {
			Release(_file);
			_client->_file_writer = nullptr;
		}
		
		virtual void trigger_async_file_open(AsyncFile* file) {
			if ( _write_flag ) { // write header
				String header;
				auto& r_header = _client->response_header();

				for ( auto& i : r_header ) {
					if (!i.value().is_empty() && i.key() != "cache-control") {
						header += i.key();
						header += string_colon;
						if (i.key() == "expires") {
							// 写入一个固定长度的时间字符串,方便以后重写这个值
							String val = i.value();
							while (val.length() < 36) {
								val.push(' ');
							}
							header += val;
						} else {
							header += i.value();
						}
						header += string_header_end;
					}
				}
				header += string_header_end;
				_file->write( header.collapse_buffer(), _write_flag == 1 ? 0: -1, 2 ); // header write
			} else {
				_ready = true;
				_write_count++;
				_file->write(_buffer);
			}
		}
		
		virtual void trigger_async_file_close(AsyncFile* file) {
			// throw error to http client host
			_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
		}
		
		virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
			_client->report_error_and_abort(error);
		}
		
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) {
			if ( mark ) {
				if ( mark == 2 ) {
					_ready = true;
					if (_buffer.length()) {
						_write_count++;
						_file->write(_buffer);
					} else {
						goto advance;
					}
				}
			} else {
				_client->trigger_http_data2(buffer);
				_write_count--;
				ASSERT(_write_count >= 0);
			 advance:
				if ( _write_count == 0 ) {
					if ( _completed_end ) { // http已经结束
						_client->trigger_http_end();
					} else {
						_client->read_advance();
					}
				}
			}
		}
		
		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) { }
		
		bool is_write_complete() const {
			return _write_count == 0 && _buffer.length() == 0;
		}
		
		void write(Buffer& buffer) {
			if ( _file && _write_flag != 1 ) {
				if ( _ready ) {
					_write_count++;
					if ( _write_count > 32 ) {
						_client->read_pause();
					}
					_file->write(buffer);
				} else {
					_buffer.write(buffer);
					_client->read_pause();
				}
			} else { // no file write task
				_client->trigger_http_data2(buffer);
				_client->read_advance();
			}
		}

		void end() {
			_completed_end = true;
		}
		
	 private:
		Client* _client;
		Buffer  _buffer;
		AsyncFile*  _file;
		int	_write_flag, _write_count;
		bool	_ready, _completed_end;
	};
	
 private:
	
	Reader* reader() {
		return _connect ? (Reader*)_connect: (Reader*)_cache_reader;
	}
	
	void read_advance() {
		Reader* r = reader(); ASSERT(r);
		if ( _pause ) {
			r->read_pause();
		} else {
			r->read_advance();
		}
	}

	void read_pause() {
		Reader* r = reader(); ASSERT(r);
		r->read_pause();
	}
	
	bool is_disable_cache() {
		return _disable_cache || _url_no_cache_arg;
	}
	
	void trigger_http_readystate_change(HttpReadyState ready_state) {
		if ( ready_state != _ready_state ) {
			_ready_state = ready_state;
			_delegate->trigger_http_readystate_change(_host);
		}
	}
	
	void trigger_http_write() {
		_delegate->trigger_http_write(_host);
	}
	
	void trigger_http_header(uint32_t status_code, Map<String, String>&& header, bool fromCache) {
		_status_code = status_code;
		_response_header = std::move(header);
		_delegate->trigger_http_header(_host);
	}

	void trigger_http_data2(Buffer& buffer) {
		_delegate->trigger_http_data(_host, buffer);
	}
	
	/**
	 * @func trigger_http_data()
	 */
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

		if ( !_save_path.is_empty() ) {
			if ( !_file_writer ) {
				new FileWriter(this, _save_path, 0, loop());
			}
			_file_writer->write(buffer);
		} else if ( !is_disable_cache() && _write_cache_flag ) {
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
			ASSERT(_pool_ptr);
			ASSERT(_connect);
			_pool_ptr->release(_connect, false);
			_connect = nullptr;

			if ( _status_code == 304) {
				if (_cache_reader) {
					String expires = convert_to_expires(_response_header.get("cache-control"));
					if (expires.is_empty()) {
						expires = _response_header.get("expires");
					}
					_response_header = std::move(_cache_reader->header());

					if (!expires.is_empty() && expires != _response_header.get("expires")) {
						// 重新设置 expires
						_write_cache_flag = 1; // rewrite response header
						_response_header.set("expires", expires);
					}
					_cache_reader->read_advance();
					return;
				} else {
					FX_ERR("http response status code error, %d", _status_code);
				}
			}
		}
		
		if ( _file_writer ) {
			if ( _file_writer->is_write_complete() ) { // 缓存是否写入完成
				trigger_http_end();
			} else {
				_file_writer->end(); // 通知已经结束
			}
		} else {
			trigger_http_end();
		}
	}
	
	void report_error_and_abort(cError& error) {
		_delegate->trigger_http_error(_host, error);
		abort_();
	}
	
	void trigger_http_timeout() {
		_delegate->trigger_http_timeout(_host);
		abort_();
	}

	void send_http() {
		ASSERT(_sending);
		ASSERT(!_connect);
		ASSERT(_pool_ptr);
		_pool_ptr->get_connect(this, Cb([this](CbD& evt) {
			if ( _wait_connect_id ) {
				if ( evt.error ) {
					report_error_and_abort(*evt.error);
				} else {
					ASSERT( !_connect );
					_connect = static_cast<Connect*>(evt.data);
					_connect->bind_client_and_send(this);
				}
			}
		}, this));
	}
	
	void cache_file_stat_cb(CbD& evt) {
		if ( _sending ) {
			if ( evt.error ) { //
				send_http();
			} else {
				new FileCacheReader(this, static_cast<FileStat*>(evt.data)->size(), loop());
			}
		}
	}
	
	inline void trigger_http_end() {
		end_(false);
	}
	
	inline void abort_() {
		end_(true);
	}
	
	void end_(bool abort) {
		if ( _sending && !_sending->_ending ) {
			_sending->_ending = true;
			
			ASSERT(_pool_ptr);
			
			Release(_cache_reader); _cache_reader = nullptr;
			Release(_file_writer);  _file_writer = nullptr;
			_pool_ptr->release(_connect, abort);  _connect = nullptr;
			_pause = false;
			_wait_connect_id = 0;
			
			if ( abort ) {
				_sending->release();
				HttpReadyState state = _ready_state;
				_delegate->trigger_http_abort(_host);
				if (state == _ready_state)
					_ready_state = HTTP_READY_STATE_INITIAL;
			} else {
				ASSERT(_sending);
				_ready_state = HTTP_READY_STATE_COMPLETED;
				_delegate->trigger_http_readystate_change(_host);
				_sending->release();
				_delegate->trigger_http_end(_host);
				if (_ready_state == HTTP_READY_STATE_COMPLETED)
					_ready_state = HTTP_READY_STATE_INITIAL;
			}
		}
	}
	
 public:
	// public api
	
	void send(Buffer data) throw(Error) {
		FX_CHECK(!_sending, ERR_REPEAT_CALL, "Sending repeat call");
		FX_CHECK( !_uri.is_null(), ERR_INVALID_PATH, "Invalid path" );
		FX_CHECK(_uri.type() == URI_HTTP ||
						_uri.type() == URI_HTTPS, ERR_INVALID_PATH, "Invalid path `%s`", *_uri.href());
		_post_data = data;
		
		_sending = new Sending(this);
		_pause = false;
		_url_no_cache_arg = false;
		_cache_path = inl__get_http_cache_path() + '/' +
			hash_code(_uri.href().c(), _uri.href().length());
		
		int i = _uri.search().index_of("__no_cache");
		if ( i != -1 && _uri.search()[i+9] != '=' ) {
			_url_no_cache_arg = true;
		}
		
		trigger_http_readystate_change(HTTP_READY_STATE_READY); // ready
		
		if ( !_sending ) return; // abort
		_upload_total = 0; _upload_size = 0;
		_download_total = 0; _download_size = 0;
		_status_code = 0;
		_response_header.clear();
		_http_response_version = String();
		
		if ( is_disable_cache() ) { // check cache
			send_http();
		} else {
			FileHelper::stat(_cache_path, Cb(&Inl::cache_file_stat_cb, this));
		}
	}
	
	void abort() {
		if ( _sending ) {
			abort_();
		}
	}
	
	void check_is_can_modify() throw(Error) {
		FX_CHECK(!_sending, ERR_SENDIFX_CANNOT_MODIFY,
							"Http request sending cannot modify property");
	}
	
	void pause() {
		if ( _sending ) _pause = true;
	}
	
	void resume() {
		if ( _sending && _pause ) {
			_pause = false;
			Reader* r = reader();
			if ( r ) {
				r->read_advance();
			}
		}
	}
		
	// -----------------------------------attrs------------------------------------------
	
	HttpClientRequest* _host;
	KeepLoop*  _keep;
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
	Map<String, String> _request_header;
	Map<String, String> _response_header;
	Map<String, FormValue> _post_form_data;
	Buffer      _post_data;
	String      _username;
	String      _password;
	String      _cache_path;
	String      _http_response_version;
	bool        _disable_cache;
	bool        _disable_cookie;
	bool        _disable_send_cookie;
	bool        _disable_ssl_verify;
	//bool        _disable_ssl_verify_host; //
	bool        _keep_alive;
	Sending*    _sending;
	uint64_t      _timeout;
	bool        _pause;
	bool        _url_no_cache_arg;
	uint32_t        _wait_connect_id, _write_cache_flag;
	static      ConnectPool _pool;
	static      ConnectPool* _pool_ptr;
};

HttpClientRequest::Inl::ConnectPool HttpClientRequest::Inl::_pool;
HttpClientRequest::Inl::ConnectPool* HttpClientRequest::Inl::_pool_ptr = nullptr;

HttpClientRequest::HttpClientRequest(RunLoop* loop): _inl(NewRetain<Inl>(this, loop))
{
}

HttpClientRequest::~HttpClientRequest() {
	ASSERT(_inl->_keep->host() == RunLoop::current());
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

void HttpClientRequest::set_url(const String& path) throw(Error) {
	_inl->check_is_can_modify();
	_inl->_uri = URI(path);
}

void HttpClientRequest::set_save_path(const String& path) throw(Error) {
	_inl->check_is_can_modify();
	_inl->_save_path = path;
}

void HttpClientRequest::set_username(const String& username) throw(Error) {
	_inl->check_is_can_modify();
	_inl->_username = username;
}

void HttpClientRequest::set_password(const String& password) throw(Error) {
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

void HttpClientRequest::set_request_header(const String& name, const String& value) throw(Error) {
	_inl->check_is_can_modify();
	_inl->_request_header.set(name, value);
}

void HttpClientRequest::set_form(const String& form_name, const String& value) throw(Error) {
	_inl->check_is_can_modify();
	FX_CHECK( value.length() <= BUFFER_SIZE,
						ERR_HTTP_FORM_SIZE_LIMIT, "Http form field size limit <= %d", BUFFER_SIZE);
	_inl->_post_form_data.set(form_name, {
		FORM_TYPE_TEXT, value, inl__uri_encode(form_name)
	});
}

void HttpClientRequest::set_upload_file(const String& form_name, const String& path) throw(Error) {
	_inl->check_is_can_modify();
	_inl->_post_form_data.set(form_name, {
		FORM_TYPE_FILE, path, inl__uri_encode(form_name)
	});
}

void HttpClientRequest::clear_request_header() throw(Error) {
	_inl->check_is_can_modify();
	_inl->_request_header.clear();
}

void HttpClientRequest::clear_form_data() throw(Error) {
	_inl->check_is_can_modify();
	_inl->_post_form_data.clear();
}

String HttpClientRequest::get_response_header(const String& name) {
	auto i = _inl->_response_header.find(name);
	if ( i.is_null() ) return String();
	return i.value();
}

const Map<String, String>& HttpClientRequest::get_all_response_headers() {
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

void HttpClientRequest::send(Buffer data) throw(Error) { // thread safe
	_inl->send(data);
}

void HttpClientRequest::send(const String& data) throw(Error) { // thread safe
	_inl->send(data.copy_buffer());
}

void HttpClientRequest::pause() { // thread safe
	_inl->pause();
}

void HttpClientRequest::resume() { // thread safe
	_inl->resume();
}

void HttpClientRequest::abort() { // thread safe
	_inl->abort();
}

String HttpClientRequest::http_response_version() const {
	return _inl->_http_response_version;
}

}
