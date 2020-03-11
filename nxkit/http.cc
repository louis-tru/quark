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

#include "nxkit/http.h"
#include "nxkit/http-cookie.h"
#include "nxkit/fs.h"
#include "ngui/version.h"
#include "nxkit/string-builder.h"
#include "nxkit/net.h"
#include <http_parser.h>
#include <zlib.h>

NX_NS(ngui)

typedef HttpClientRequest::Delegate HttpDelegate;

extern String inl__get_http_user_agent();
extern String inl__get_http_cache_path();
extern String inl__uri_encode(cString& url, bool component = false, bool secondary = false);

static const String string_method[5] = { "GET", "POST", "HEAD", "DELETE", "PUT" };
static const String string_colon(": ");
static const String string_space(" ");
static const String string_header_end("\r\n");
static const String string_max_age("max-age=");
static const String content_type_form("application/x-www-form-urlencoded; charset=utf-8");
static const String content_type_multipart_form("multipart/form-data; "
																								"boundary=----NguiFormBoundaryrGKCBY7qhFd3TrwA");
static const String multipart_boundary_start("------NguiFormBoundaryrGKCBY7qhFd3TrwA\r\n");
static const String multipart_boundary_end  ("------NguiFormBoundaryrGKCBY7qhFd3TrwA--");

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
	: m_host(host)
	, m_keep(loop->keep_alive("HttpClientRequest::Inl", false))
	, m_delegate(this)
	, m_upload_total(0)
	, m_upload_size(0)
	, m_download_total(0)
	, m_download_size(0)
	, m_ready_state(HTTP_READY_STATE_INITIAL)
	, m_status_code(0)
	, m_method(HTTP_METHOD_GET)
	, m_connect(nullptr)
	, m_cache_reader(nullptr)
	, m_file_writer(nullptr)
	, m_disable_cache(false)
	, m_disable_cookie(false)
	, m_disable_send_cookie(false)
	, m_disable_ssl_verify(false)
	//, _disable_ssl_verify_host(0)
	, m_keep_alive(true)
	, m_sending(nullptr)
	, m_timeout(0), m_pause(false)
	, m_url_no_cache_arg(false), m_wait_connect_id(0)
	, m_write_cache_flag(0)
	{
		HttpHelper::initialize();
	}
	
	virtual ~Inl() {
		NX_CHECK(!m_sending);
		NX_CHECK(!m_connect);
		NX_CHECK(!m_cache_reader);
		NX_CHECK(!m_file_writer);
		Release(m_keep); m_keep = nullptr;
	}
	
	inline RunLoop* loop() { return m_keep->host(); }
	inline uv_loop_t* uv_loop() { return loop()->uv_loop(); }
	
	void set_delegate(HttpDelegate* delegate) {
		m_delegate = delegate ? delegate : this;
	}
	
	class Sending {
	 public:
		typedef NonObjectTraits Traits;
		Sending(Inl* host)
		: m_host(host), m_ending(false) {
			Retain(host);
		}
		~Sending() { Release(m_host); }
		void release() {
			NX_ASSERT(m_host);
			m_host->m_sending = nullptr;
			delete this;
		}
		Inl*  m_host;
		bool  m_ending;
	};
	
	class Reader {
	 public:
		virtual void read_advance() = 0;
		virtual void read_pause() = 0;
		virtual bool is_cache() = 0;
	};
	
	class ConnectPool;
	class Connect;
	typedef List<Connect*>::Iterator ConnectID;

	/**
	 * @class HttpClientRequest::Inl::Connect
	 */
	class Connect: public Object
	, public Socket::Delegate
	, public Reader, public AsyncFile::Delegate {
	 public:
		
		typedef List<Connect*>::Iterator ID;
		
		Connect(cString& hostname, uint16 port, bool ssl, RunLoop* loop)
		: m_ssl(ssl)
		, m_use(false)
		, m_is_multipart_form_data(false)
		, m_send_data(false)
		, m_socket(nullptr)
		, m_client(nullptr)
		, m_upload_file(nullptr)
		, m_z_gzip(0), m_loop(loop) { //

			if ( m_ssl ) {
				m_socket = new SSLSocket(hostname, port, loop);
			} else {
				m_socket = new Socket(hostname, port, loop);
			}
			
			NX_ASSERT(m_socket);
			m_socket->set_delegate(this);
			
			m_parser.data = this;
			http_parser_settings_init(&m_settings);
			m_settings.on_message_begin = &on_message_begin;
			m_settings.on_status = &on_status;
			m_settings.on_header_field = &on_header_field;
			m_settings.on_header_value = &on_header_value;
			m_settings.on_headers_complete = &on_headers_complete;
			m_settings.on_body = &on_body;
			m_settings.on_message_complete = &on_message_complete;
		}
		
		~Connect() {
			NX_ASSERT(m_id.is_null());
			Release(m_socket);     m_socket = nullptr;
			Release(m_upload_file);m_upload_file = nullptr;
		}
		
		inline RunLoop* loop() { return m_loop; }
		
		void bind_client_and_send(Client* client) {
			NX_ASSERT(client);
			NX_ASSERT(!m_client);
			
			m_client = client;
			m_socket->set_timeout(m_client->m_timeout); // set timeout
			
			if ( m_ssl ) {
				static_cast<SSLSocket*>(m_socket)->disable_ssl_verify(m_client->m_disable_ssl_verify);
			}
			
			if ( m_socket->is_open() ) {
				send_http_request(); // send request
			} else {
				m_socket->open();
			}
		}
		
		static int on_message_begin(http_parser* parser) {
			//g_debug("--http response parser on_message_begin");
			Connect* self = static_cast<Connect*>(parser->data);
			self->m_client->trigger_http_readystate_change(HTTP_READY_STATE_RESPONSE);
			return 0;
		}
		
		static int on_status(http_parser* parser, const char *at, size_t length) {
			//g_debug("http response parser on_status, %s %s", String(at - 4, 3).c(), String(at, uint(length)).c());
			Connect* self = static_cast<Connect*>(parser->data);
			int status_code = String(at - 4, 3).to_uint();
			if (status_code == 200) {
				self->m_client->m_write_cache_flag = 2; // set write cache flag
			}
			NX_ASSERT(status_code == parser->status_code);
			// LOG("http %d,%d", int(parser->http_major), int(parser->http_minor));
			self->m_client->m_status_code = status_code;
			self->m_client->m_http_response_version = 
				String(parser->http_major) + '.' + parser->http_minor;
			return 0;
		}
		
		static int on_header_field(http_parser* parser, const char *at, size_t length) {
			//g_debug("http response parser on_header_field, %s", String(at, uint(length)).c());
			static_cast<Connect*>(parser->data)->m_header_field = String(at, uint(length)).lower_case();
			return 0;
		}
		
		static int on_header_value(http_parser* parser, const char *at, size_t length) {
			//g_debug("http response parser on_header_value, %s", String(at, uint(length)).c());
			Connect* self = static_cast<Connect*>(parser->data);
			String value(at, uint(length));
			
			if ( !self->m_client->m_disable_cookie ) {
				if ( self->m_header_field == "set-cookie" ) {
					http_cookie_set_with_expression(self->m_client->m_uri.domain(), value);
				}
			}
			
			self->m_header.set( move(self->m_header_field), value );
			
			return 0;
		}
		
		static int on_headers_complete(http_parser* parser) {
			//g_debug("--http response parser on_headers_complete");
			Connect* self = static_cast<Connect*>(parser->data);
			Client* cli = self->m_client;
			if ( self->m_header.has("content-length") ) {
				cli->m_download_total = self->m_header.get("content-length").to_int64();
			}
			self->init_gzip_parser();
			cli->trigger_http_header(cli->m_status_code, move(self->m_header), 0);
			return 0;
		}
		
		void init_gzip_parser() {
			if ( m_header.has("content-encoding") ) {

				m_z_strm.zalloc = Z_NULL;
				m_z_strm.zfree = Z_NULL;
				m_z_strm.opaque = Z_NULL;
				m_z_strm.next_in = Z_NULL;
				m_z_strm.avail_in = 0;
				
				String encoding = m_header.get("content-encoding");
				if ( encoding.index_of("gzip") != -1 ) {
					m_z_gzip = 2;
					inflateInit2(&m_z_strm, 47);
				} else if ( encoding.index_of("deflate") != -1 ) {
					m_z_gzip = 1;
					inflateInit(&m_z_strm);
				}
			}
		}
		
		int gzip_inflate(cchar* data, uint len, Buffer& out) {
			static Buffer _z_strm_buff(16384); // 16k
			
			int r = 0;
			
			m_z_strm.next_in = (byte*)data;
			m_z_strm.avail_in = len;
			do {
				m_z_strm.next_out = (byte*)*_z_strm_buff;
				m_z_strm.avail_out = _z_strm_buff.length();
				r = inflate(&m_z_strm, Z_NO_FLUSH);
				out.write(_z_strm_buff, -1, _z_strm_buff.length() - m_z_strm.avail_out);
			} while(m_z_strm.avail_out == 0);
			
			if ( r == Z_STREAM_END ) {
				inflateEnd(&m_z_strm);
			}
			
			return r;
		}
		
		static int on_body(http_parser* parser, const char *at, size_t length) {
			//g_debug("--http response parser on_body, %d", length);
			Connect* self = static_cast<Connect*>(parser->data);
			self->m_client->m_download_size += length;
			Buffer buff;
			if ( self->m_z_gzip ) {
				int r = self->gzip_inflate(at, uint(length), buff);
				if (r < 0) {
					NX_ERR("un gzip err, %d", r);
				}
			} else {
				buff = WeakBuffer(at, uint(length)).copy();
			}
			if ( buff.length() ) {
				self->m_client->trigger_http_data(buff);
			}
			return 0;
		}
		
		static int on_message_complete(http_parser* parser) {
			//g_debug("--http response parser on_message_complete");
			static_cast<Connect*>(parser->data)->m_client->http_response_complete(false);
			return 0;
		}
		
		void send_http_request() {
			
			http_parser_init(&m_parser, HTTP_RESPONSE);
			Release(m_upload_file); m_upload_file = nullptr;
			m_is_multipart_form_data = false;
			m_send_data = false;
			m_multipart_form_data.clear();
			m_z_gzip = 0;
			m_header.clear();
			//
			
			Map<String, String> header = m_client->m_request_header;
			
			header.set("Host", m_client->m_uri.host());
			header.set("Connection", m_client->m_keep_alive ? "keep-alive" : "close");
			header.set("Accept-Encoding", "gzip, deflate");
			header.set("Date", gmt_time_string(sys::time_second()));
			
			if ( !header.has("Cache-Control") )   header.set("Cache-Control", "max-age=0");
			if ( !header.has("User-Agent") )      header.set("User-Agent", inl__get_http_user_agent());
			if ( !header.has("Accept-Charset") )  header.set("Accept-Charset", "utf-8");
			if ( !header.has("Accept") )          header.set("Accept", "*/*");
			if ( !header.has("DNT") )             header.set("DNT", "1");
			if ( !header.has("Accept-Language") ) header.set("Accept-Language", sys::languages());
			
			if ( !m_client->m_username.is_empty() && !m_client->m_password.is_empty() ) {
				String s = m_client->m_username + ':' + m_client->m_password;
				header.set("Authorization", Codec::encoding(Encoding::BASE64, s));
			}
			
			if ( !m_client->m_disable_cookie && !m_client->m_disable_send_cookie ) { // send cookies
				
				String cookies = http_cookie_get_all_string(m_client->m_uri.domain(),
																										m_client->m_uri.pathname(),
																										m_client->m_uri.type() == URI_HTTPS);
				if ( !cookies.is_empty() ) {
					header.set("Cookie", cookies);
				}
			}
			
			if ( m_client->m_cache_reader ) {
				String last_modified = m_client->m_cache_reader->header()["last-modified"];
				String etag = m_client->m_cache_reader->header()["etag"];
				if ( !last_modified.is_empty() )  {
					header.set("If-Modified-Since", move(last_modified) );
				}
				if ( !etag.is_empty() ) {
					header.set("If-None-Match", move(etag) );
				}
			}
			
			if ( m_client->m_method == HTTP_METHOD_POST ) {
				
				if ( m_client->m_post_data.length() ) { // ignore form data
					if ( m_client->m_post_form_data.length() ) {
						NX_WARN("Ignore form data");
					}
					m_client->m_upload_total = m_client->m_post_data.length();
					header.set("Content-Length", m_client->m_upload_total);
				}
				else if ( m_client->m_post_form_data.length() ) { // post form data
					
					for ( auto& i : m_client->m_post_form_data ) {
						if ( i.value().type == FORM_TYPE_FILE ) {
							m_is_multipart_form_data = true; break;
						}
					}
					
					if (m_is_multipart_form_data ) {
						
						uint content_length = multipart_boundary_end.length();
						
						for ( auto& i : m_client->m_post_form_data ) {
							FormValue& form = i.value();
							MultipartFormValue m_form = { form.type, form.data };
							
							if ( i.value().type == FORM_TYPE_FILE ) {
								FileStat stat = FileHelper::stat_sync(i.value().data);
								if ( stat.is_valid() && stat.is_file() ) {
									String basename = inl__uri_encode(Path::basename(form.data));
									m_form.headers =
									String::format("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
																 "Content-Type: application/octet-stream\r\n\r\n",
																 *form.name, *basename);
									content_length += stat.size();
									m_client->m_upload_total += stat.size();
								} else {
									Error err(ERR_INVALID_PATH, "invalid upload path `%s`", i.value().data.c());
									m_client->report_error_and_abort(err);
									return;
								}
							} else {
								m_form.headers = String::format("Content-Disposition: form-data;"
																								"name=\"%s\"\r\n\r\n", *form.name);
								content_length += form.data.length();
								m_client->m_upload_total += form.data.length();
							}
						
							content_length += multipart_boundary_start.length();
							content_length += m_form.headers.length();
							content_length += 2; // end
							m_multipart_form_data.push(m_form);
						}
						
						header.set("Content-Length", content_length);
						header.set("Content-Type", content_type_multipart_form);
					} else {
						
						for ( auto& i : m_client->m_post_form_data ) {
							String value = inl__uri_encode(i.value().data);
							m_client->m_post_data.write(i.key().c(), -1, i.key().length());
							m_client->m_post_data.write("=", -1, 1);
							m_client->m_post_data.write(*value, -1, value.length());
							m_client->m_post_data.write("&", -1, 1);
							m_client->m_upload_total += i.key().length() + value.length() + 2;
						}
						header.set("Content-Length", m_client->m_upload_total);
						header.set("Content-Type", content_type_form);
					}
				}
			}
			
			StringBuilder header_str;
			String search = m_client->m_uri.search();

			if (m_client->m_url_no_cache_arg) {
				search = search.replace("__no_cache", "");
				if (search.length() == 1) {
					search = String();
				}
			}
			
			header_str.push(String::format
			(
			"%s %s%s HTTP/1.1\r\n"
			 , string_method[m_client->m_method].c()
			 , *inl__uri_encode(m_client->m_uri.pathname(), false, true)
			 , search.c()
			));
			
			for ( auto& i : header ) {
				header_str.push(i.key());       // name
				header_str.push(string_colon);  // :
				header_str.push(i.value());     // value
				header_str.push(string_header_end);    // \r\n
			}
			
			header_str.push(string_header_end); // \r\n
			
			m_client->trigger_http_readystate_change(HTTP_READY_STATE_SENDING);
			
			m_socket->resume();
			m_socket->write(header_str.to_buffer()); // write header
		}
		
		virtual void trigger_socket_timeout(Socket* socket) {
			if ( m_client ) {
				m_client->trigger_http_timeout();
			}
		}
		
		virtual void trigger_socket_open(Socket* stream) {
			if ( m_client ) {
				send_http_request();
			}
		}
		
		virtual void trigger_socket_close(Socket* stream) {
			if ( m_client ) {
				Error err(ERR_CONNECT_UNEXPECTED_SHUTDOWN, "Connect unexpected shutdown");
				m_client->report_error_and_abort(err);
			} else {
				m_pool.release(this, true);
			}
		}
		
		virtual void trigger_socket_error(Socket* stream, cError& error) {
			if ( m_client ) {
				m_client->report_error_and_abort(error);
			}
		}
		
		virtual void trigger_socket_data(Socket* stream, Buffer& buffer) {
			if ( m_client ) {
				http_parser_execute(&m_parser, &m_settings, buffer.value(), buffer.length());
			}
		}
		
		virtual void trigger_socket_write(Socket* stream, Buffer buffer, int mark) {
			if ( !m_client ) return;
			if ( m_send_data ) {
				if ( mark == 1 ) {
					m_client->m_upload_size += buffer.length();
					m_client->trigger_http_write();
					
					if ( m_is_multipart_form_data ) {
						m_multipart_form_buffer = buffer.realloc(BUFFER_SIZE);
						send_multipart_form_data();
					}
				}
			}
			else if ( m_client->m_method == HTTP_METHOD_POST ) { // post data
				m_send_data = true;
				if ( m_client->m_post_data.length() ) {
					m_socket->write(m_client->m_post_data, 1);
				}
				else if ( m_is_multipart_form_data ) { // send multipart/form-data
					if ( !m_multipart_form_buffer.length() ) {
						m_multipart_form_buffer = Buffer(BUFFER_SIZE);
					}
					send_multipart_form_data();
				}
			}
		}
		
		virtual void trigger_async_file_open(AsyncFile* file) {
			NX_ASSERT( m_is_multipart_form_data );
			send_multipart_form_data();
		}
		
		virtual void trigger_async_file_close(AsyncFile* file) {
			NX_ASSERT( m_is_multipart_form_data );
			Error err(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown");
			m_client->report_error_and_abort(err);
		}
		
		virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
			NX_ASSERT( m_is_multipart_form_data );
			m_client->report_error_and_abort(error);
		}
		
		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {
			NX_ASSERT( m_is_multipart_form_data );
			if ( buffer.length() ) {
				m_socket->write(buffer, 1);
			} else {
				NX_ASSERT(m_multipart_form_data.length());
				NX_ASSERT(m_upload_file);
				m_socket->write(string_header_end.copy_buffer()); // \r\n
				m_upload_file->release(); // release file
				m_upload_file = nullptr;
				m_multipart_form_data.shift();
				m_multipart_form_buffer = buffer.realloc(BUFFER_SIZE);
				send_multipart_form_data();
			}
		}
		
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) { }
		
		void send_multipart_form_data() {
			NX_ASSERT( m_multipart_form_buffer.length() == BUFFER_SIZE );
			
			if ( m_upload_file ) { // upload file
				NX_ASSERT( m_upload_file->is_open() );
				m_upload_file->read(m_multipart_form_buffer);
			}
			else if ( m_multipart_form_data.length() ) {
				MultipartFormValue& form = m_multipart_form_data.begin().value();
				m_socket->write(multipart_boundary_start.copy_buffer());
				m_socket->write(form.headers.collapse_buffer());
				
				if ( form.type == FORM_TYPE_FILE ) {
					m_upload_file = new AsyncFile(form.data, m_loop);
					m_upload_file->set_delegate(this);
					m_upload_file->open();
				} else {
					m_multipart_form_buffer.write( form.data.c(), 0, form.data.length() );
					m_socket->write(m_multipart_form_buffer.realloc(form.data.length()), 1);
					m_socket->write(string_header_end.copy_buffer());
					m_multipart_form_data.shift();
				}
			} else {
				m_socket->write(multipart_boundary_end.copy_buffer()); // end send data, wait http response
			}
		}
		
		bool ssl() { return m_ssl; }
		
		Socket* socket() { return m_socket; }
		
		virtual void read_advance() {
			m_socket->resume();
		}
		
		virtual void read_pause() {
			m_socket->pause();
		}
		
		virtual bool is_cache() {
			return false;
		}
		
	 private:
		friend class ConnectPool;
		
		bool  m_ssl;
		bool  m_use;
		bool  m_is_multipart_form_data;
		bool  m_send_data;
		Socket*     m_socket;
		Client*     m_client;
		ID          m_id;
		AsyncFile*  m_upload_file;
		http_parser m_parser;
		http_parser_settings m_settings;
		List<MultipartFormValue> m_multipart_form_data;
		Buffer  m_multipart_form_buffer;
		String  m_header_field;
		Map<String, String> m_header;
		z_stream m_z_strm;
		int      m_z_gzip;
		RunLoop*  m_loop;
	};

	/**
	 * @class HttpClientRequest::Inl::ConnectPool
	 */
	class ConnectPool {
	 public:
		
		struct connect_req {
			Client* client;
			Callback<> cb;
			uint wait_id;
			String  hostname;
			uint16  port;
			URIType uri_type;
		};
		
		ConnectPool() {
			m_pool_ptr = this;
		}
		
		~ConnectPool() {
			ScopeLock scope(m_mutex);
			for (auto& i : m_pool) {
				auto con = i.value();
				con->m_id = ConnectID();
				con->m_loop->post(Cb([con](CbD& e){
					Release(con);
				}));
			}
			m_pool_ptr = nullptr;
		}
		
		void get_connect(Client* client, cCb& cb) {
			NX_ASSERT(client);
			NX_ASSERT(!client->m_uri.is_null());
			NX_ASSERT(!client->m_uri.hostname().is_empty());
			NX_ASSERT(client->m_uri.type() == URI_HTTP || client->m_uri.type() == URI_HTTPS);
			
			uint16 port = client->m_uri.port();
			if (!port) {
				port = client->m_uri.type() == URI_HTTP ? 80 : 443;
			}
			
			client->m_wait_connect_id = iid32();
			
			connect_req req = {
				client,
				cb,
				client->m_wait_connect_id,
				client->m_uri.hostname(),
				port,
				client->m_uri.type(),
			};
			
			Connect* conn = nullptr;
			{ //
				ScopeLock scope(m_mutex);
				conn = get_connect2(req);
				if ( conn ) {
					conn->m_use = true;
				} else {
					m_connect_req.push(req); // wait
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
			uint connect_count = 0;
			
			for ( auto& i : m_pool ) {
				if ( connect_count < MAX_CONNECT_COUNT ) {
					Connect* connect = i.value();
					if (connect->socket()->hostname() == req.hostname &&
							connect->socket()->port() == req.port &&
							connect->ssl() == (req.uri_type == URI_HTTPS)
					) {
						connect_count++;
						if ( !connect->m_use ) {
							if (connect->loop() == req.client->loop()) {
								conn = connect; break;
							} else {
								conn2 = connect;
							}
						}
					}
				}
			}
			
			NX_CHECK(connect_count <= MAX_CONNECT_COUNT);
			
			if (!conn) {
				if (connect_count == MAX_CONNECT_COUNT) {
					if (conn2) {
						// 连接已达到最大限制然而这个连接虽然可用但不在同一个loop,所以删除
						connect_count--;
						m_pool.del(conn2->m_id);
						conn2->m_id = ConnectID();
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
					conn->m_id = m_pool.push(conn);
				}
			}
			return conn;
		}
		
		void release(Connect* connect, bool immediately) {
			if (!connect) return;
			Lock lock(m_mutex);
			if (connect->m_id.is_null()) {
				return;
			}
			
			if ( !connect->socket()->is_open() || immediately ) { // immediately release
				m_pool.del(connect->m_id);
				connect->m_id = ConnectID();
				connect->release();
			} else {
				if ( connect->m_use ) {
					NX_ASSERT( !connect->m_id.is_null() );
					connect->m_use = false;
					connect->m_client = nullptr;
					connect->socket()->set_timeout(0);
					connect->socket()->resume();
				}
			}
			
			for ( auto& i : m_connect_req ) {
				connect_req& req = i.value();
				if (req.client->m_wait_connect_id == req.wait_id) {
					Connect* conn = get_connect2(req);
					if ( conn ) {
						conn->m_use = true;
						Cb cb = req.cb;
						m_connect_req.del(i);
						lock.unlock(); // unlock
						CbD evt = { 0, conn };
						cb->call( evt );
						break;
					}
				} else {
					m_connect_req.del(i); // discard req
				}
			}
		}
		
	 private:
		Mutex m_mutex;
		List<Connect*>  m_pool;
		List<connect_req> m_connect_req;
	};
	
	/**
	 * @class HttpClientRequest::Inl::FileCacheReader
	 */
	class FileCacheReader: public AsyncFile, 
		public AsyncFile::Delegate, public Reader 
	{
	 public:
		FileCacheReader(Client* client, int64 size, RunLoop* loop)
		: AsyncFile(client->m_cache_path, loop)
		, m_read_count(0)
		, m_client(client)
		, m_parse_header(true), m_offset(0), m_size(size) {
			NX_ASSERT(!m_client->m_cache_reader);
			m_client->m_cache_reader = this;
			set_delegate(this);
			open();
		}
		
		~FileCacheReader() {
			m_client->m_cache_reader = nullptr;
		}
		
		void continue_send_and_release() {
			set_delegate(nullptr);
			m_client->m_cache_reader = nullptr;
			m_client->send_http();
			release();
		}
		
		virtual void trigger_async_file_open(AsyncFile* file) {
			read(Buffer(512));
		}

		virtual void trigger_async_file_close(AsyncFile* file) {
			if ( m_parse_header ) { // unexpected shutdown
				continue_send_and_release();
			} else {
				// throw error to http client host
				m_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
			}
		}

		virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
			if ( m_parse_header ) {
				continue_send_and_release();
			} else {
				// throw error to http client host
				m_client->report_error_and_abort(error);
			}
		}

		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {
			if ( m_parse_header ) { // parse cache header
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
								m_parse_header = false;
								m_offset += (j + 2);

								int64 expires = parse_time(m_header.get("expires"));
								if ( expires > sys::time() ) {
									m_client->trigger_http_readystate_change(HTTP_READY_STATE_RESPONSE);
									m_client->m_download_total = NX_MAX(m_size - m_offset, 0);
									m_client->trigger_http_header(200, move(m_header), true);
									read_advance();
								} else {
									// LOG("Read -- %ld, %ld, %s", expires, sys::time(), *m_header.get("expires"));
									if (parse_time(m_header.get("last-modified")) > 0 ||
											!m_header.get("etag").is_empty()
									) {
										m_client->send_http();
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
									m_header.set(str.substring(i, k).lower_case(), str.substring(k + 2, j));
								}
							}
						} else {
							if ( i == 0 ) { // invalid cache
								continue_send_and_release();
							} else { // read next
								m_offset += i;
								read(buffer.realloc(512), m_offset);
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
				m_read_count--;
				NX_ASSERT(m_read_count == 0);
				
				if ( buffer.length() ) {
					m_offset += buffer.length();
					m_client->m_download_size += buffer.length();
					m_client->trigger_http_data(buffer);
				} else { // end
					m_client->http_response_complete(true);
				}
			}
		}
		
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) {}
		
		Map<String, String>& header() {
			return m_header;
		}
		
		virtual void read_advance() {
			if ( !m_parse_header ) {
				if ( m_read_count == 0 ) {
					m_read_count++;
					read(Buffer(BUFFER_SIZE), m_offset);
				}
			}
		}
		
		virtual void read_pause() {}
		
		virtual bool is_cache() {
			return true;
		}

	 private:
		int m_read_count;
		Client* m_client;
		Map<String, String> m_header;
		bool m_parse_header;
		uint  m_offset;
		int64 m_size;
	};

	Map<String, String>& response_header() {
		return m_response_header;
	}

	static String convert_to_expires(cString& cache_control) {
		if ( !cache_control.is_empty() ) {
			int i = cache_control.index_of(string_max_age);
			if ( i != -1 && i + string_max_age.length() < cache_control.length() ) {
				int j = cache_control.index_of(',', i);
				String max_age = j != -1
				? cache_control.substring(i + string_max_age.length(), j)
				: cache_control.substring(i + string_max_age.length());
				
				int64 num = max_age.trim().to_int64();
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
		FileWriter(Client* client, cString& path, int flag, RunLoop* loop)
		: m_client(client)
		, m_file(nullptr)
		, m_write_flag(flag)
		, m_write_count(0)
		, m_ready(0), m_completed_end(0) {
			// flag:
			// flag = 0 only write body 
			// flag = 1 only write header 
			// flag = 2 write header and body

			NX_ASSERT(!m_client->m_file_writer);
			m_client->m_file_writer = this;

			// LOG("FileWriter m_write_flag -- %i, %s", m_write_flag, *path);
			
			if ( m_write_flag ) { // verification cache is valid
				auto& r_header = m_client->response_header(); 
				NX_ASSERT(r_header.length());

				if ( r_header.has("cache-control") ) {
					String expires = convert_to_expires(r_header.get("cache-control"));
					// LOG("FileWriter -- %s", *expires);
					if ( !expires.is_empty() ) {
						r_header.set("expires", expires);
					}
				}

				if ( r_header.has("expires") ) {
					int64 expires = parse_time(r_header.get("expires"));
					int64 now = sys::time();
					if ( expires > now ) {
						m_file = new AsyncFile(path, loop);
					}
				} else if ( r_header.has("last-modified") || r_header.has("etag") ) {
					m_file = new AsyncFile(path, loop);
				}
			} else { // download save
				m_file = new AsyncFile(path, loop);
			}
			
			if ( m_file ) {
				m_file->set_delegate(this);
				if (m_write_flag == 1) { // only write header
					m_file->open(FOPEN_WRONLY | FOPEN_CREAT);
				} else {
					m_file->open(FOPEN_W);
				}
			}
		}
		
		~FileWriter() {
			Release(m_file);
			m_client->m_file_writer = nullptr;
		}
		
		virtual void trigger_async_file_open(AsyncFile* file) {
			if ( m_write_flag ) { // write header
				String header;
				auto& r_header = m_client->response_header();

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
				m_file->write( header.collapse_buffer(), m_write_flag == 1 ? 0: -1, 2 ); // header write
			} else {
				m_ready = true;
				m_write_count++;
				m_file->write(m_buffer);
			}
		}
		
		virtual void trigger_async_file_close(AsyncFile* file) {
			// throw error to http client host
			m_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
		}
		
		virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
			m_client->report_error_and_abort(error);
		}
		
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) {
			if ( mark ) {
				if ( mark == 2 ) {
					m_ready = true;
					if (m_buffer.length()) {
						m_write_count++;
						m_file->write(m_buffer);
					} else {
						goto advance;
					}
				}
			} else {
				m_client->trigger_http_data2(buffer);
				m_write_count--;
				NX_ASSERT(m_write_count >= 0);
			 advance:
				if ( m_write_count == 0 ) {
					if ( m_completed_end ) { // http已经结束
						m_client->trigger_http_end();
					} else {
						m_client->read_advance();
					}
				}
			}
		}
		
		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) { }
		
		bool is_write_complete() const {
			return m_write_count == 0 && m_buffer.length() == 0;
		}
		
		void write(Buffer& buffer) {
			if ( m_file && m_write_flag != 1 ) {
				if ( m_ready ) {
					m_write_count++;
					if ( m_write_count > 32 ) {
						m_client->read_pause();
					}
					m_file->write(buffer);
				} else {
					m_buffer.write(buffer);
					m_client->read_pause();
				}
			} else { // no file write task
				m_client->trigger_http_data2(buffer);
				m_client->read_advance();
			}
		}

		void end() {
			m_completed_end = true;
		}
		
	 private:
		Client* m_client;
		Buffer  m_buffer;
		AsyncFile*  m_file;
		int	m_write_flag, m_write_count;
		bool	m_ready, m_completed_end;
	};
	
 private:
	
	Reader* reader() {
		return m_connect ? (Reader*)m_connect: (Reader*)m_cache_reader;
	}
	
	void read_advance() {
		Reader* r = reader(); NX_ASSERT(r);
		if ( m_pause ) {
			r->read_pause();
		} else {
			r->read_advance();
		}
	}

	void read_pause() {
		Reader* r = reader(); NX_ASSERT(r);
		r->read_pause();
	}
	
	bool is_disable_cache() {
		return m_disable_cache || m_url_no_cache_arg;
	}
	
	void trigger_http_readystate_change(HttpReadyState ready_state) {
		if ( ready_state != m_ready_state ) {
			m_ready_state = ready_state;
			m_delegate->trigger_http_readystate_change(m_host);
		}
	}
	
	void trigger_http_write() {
		m_delegate->trigger_http_write(m_host);
	}
	
	void trigger_http_header(uint status_code, Map<String, String>&& header, bool fromCache) {
		m_status_code = status_code;
		m_response_header = move(header);
		m_delegate->trigger_http_header(m_host);
	}

	void trigger_http_data2(Buffer& buffer) {
		m_delegate->trigger_http_data(m_host, buffer);
	}
	
	/**
	 * @func trigger_http_data()
	 */
	void trigger_http_data(Buffer& buffer) {

		// m_write_cache_flag:
		// m_write_cache_flag = 0 not write cache 
		// m_write_cache_flag = 1 write response header 
		// m_write_cache_flag = 2 write response header and body

		if ( m_write_cache_flag == 2 ) {
			// `m_write_cache_flag==2` 写入头与主体缓存时,
			// 一定是由`Connect`发起的调用,所以已不再需要`m_cache_reader`了
			if ( m_cache_reader ) {
				m_cache_reader->release();
				m_cache_reader = nullptr;
			}
		}

		if ( !m_save_path.is_empty() ) {
			if ( !m_file_writer ) {
				new FileWriter(this, m_save_path, 0, loop());
			}
			m_file_writer->write(buffer);
		} else if ( !is_disable_cache() && m_write_cache_flag ) {
			if ( !m_file_writer ) {
				new FileWriter(this, m_cache_path, m_write_cache_flag, loop());
			}
			m_file_writer->write(buffer);
		} else {
			trigger_http_data2(buffer);
			read_advance();
		}
	}
	
	void http_response_complete(bool fromCache) {

		if (!fromCache) {
			NX_ASSERT(m_pool_ptr);
			NX_ASSERT(m_connect);
			m_pool_ptr->release(m_connect, false);
			m_connect = nullptr;

			if ( m_status_code == 304) {
				if (m_cache_reader) {
					String expires = convert_to_expires(m_response_header.get("cache-control"));
					if (expires.is_empty()) {
						expires = m_response_header.get("expires");
					}
					m_response_header = move(m_cache_reader->header());

					if (!expires.is_empty() && expires != m_response_header.get("expires")) {
						// 重新设置 expires
						m_write_cache_flag = 1; // rewrite response header
						m_response_header.set("expires", expires);
					}
					m_cache_reader->read_advance();
					return;
				} else {
					NX_ERR("http response status code error, %d", m_status_code);
				}
			}
		}
		
		if ( m_file_writer ) {
			if ( m_file_writer->is_write_complete() ) { // 缓存是否写入完成
				trigger_http_end();
			} else {
				m_file_writer->end(); // 通知已经结束
			}
		} else {
			trigger_http_end();
		}
	}
	
	void report_error_and_abort(cError& error) {
		m_delegate->trigger_http_error(m_host, error);
		abort_();
	}
	
	void trigger_http_timeout() {
		m_delegate->trigger_http_timeout(m_host);
		abort_();
	}

	void send_http() {
		NX_ASSERT(m_sending);
		NX_ASSERT(!m_connect);
		NX_ASSERT(m_pool_ptr);
		m_pool_ptr->get_connect(this, Cb([this](CbD& evt) {
			if ( m_wait_connect_id ) {
				if ( evt.error ) {
					report_error_and_abort(*evt.error);
				} else {
					NX_ASSERT( !m_connect );
					m_connect = static_cast<Connect*>(evt.data);
					m_connect->bind_client_and_send(this);
				}
			}
		}, this));
	}
	
	void cache_file_stat_cb(CbD& evt) {
		if ( m_sending ) {
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
		if ( m_sending && !m_sending->m_ending ) {
			m_sending->m_ending = true;
			
			NX_ASSERT(m_pool_ptr);
			
			Release(m_cache_reader); m_cache_reader = nullptr;
			Release(m_file_writer);  m_file_writer = nullptr;
			m_pool_ptr->release(m_connect, abort);  m_connect = nullptr;
			m_pause = false;
			m_wait_connect_id = 0;
			
			if ( abort ) {
				m_sending->release();
				HttpReadyState state = m_ready_state;
				m_delegate->trigger_http_abort(m_host);
				if (state == m_ready_state)
					m_ready_state = HTTP_READY_STATE_INITIAL;
			} else {
				NX_ASSERT(m_sending);
				m_ready_state = HTTP_READY_STATE_COMPLETED;
				m_delegate->trigger_http_readystate_change(m_host);
				m_sending->release();
				m_delegate->trigger_http_end(m_host);
				if (m_ready_state == HTTP_READY_STATE_COMPLETED)
					m_ready_state = HTTP_READY_STATE_INITIAL;
			}
		}
	}
	
 public:
	// public api
	
	void send(Buffer data) throw(Error) {
		NX_ASSERT_ERR(!m_sending, ERR_REPEAT_CALL, "Sending repeat call");
		NX_ASSERT_ERR( !m_uri.is_null(), ERR_INVALID_PATH, "Invalid path" );
		NX_ASSERT_ERR(m_uri.type() == URI_HTTP ||
									m_uri.type() == URI_HTTPS, ERR_INVALID_PATH, "Invalid path `%s`", *m_uri.href());
		m_post_data = data;
		
		m_sending = new Sending(this);
		m_pause = false;
		m_url_no_cache_arg = false;
		m_cache_path = inl__get_http_cache_path() + '/' +
			hash_code(m_uri.href().c(), m_uri.href().length());
		
		int i = m_uri.search().index_of("__no_cache");
		if ( i != -1 && m_uri.search()[i+9] != '=' ) {
			m_url_no_cache_arg = true;
		}
		
		trigger_http_readystate_change(HTTP_READY_STATE_READY); // ready
		
		if ( !m_sending ) return; // abort
		m_upload_total = 0; m_upload_size = 0;
		m_download_total = 0; m_download_size = 0;
		m_status_code = 0;
		m_response_header.clear();
		m_http_response_version = String();
		
		if ( is_disable_cache() ) { // check cache
			send_http();
		} else {
			FileHelper::stat(m_cache_path, Cb(&Inl::cache_file_stat_cb, this));
		}
	}
	
	void abort() {
		if ( m_sending ) {
			abort_();
		}
	}
	
	void check_is_can_modify() throw(Error) {
		NX_ASSERT_ERR(!m_sending, ERR_SENDINX_CANNOT_MODIFY,
									"Http request sending cannot modify property");
	}
	
	void pause() {
		if ( m_sending ) m_pause = true;
	}
	
	void resume() {
		if ( m_sending && m_pause ) {
			m_pause = false;
			Reader* r = reader();
			if ( r ) {
				r->read_advance();
			}
		}
	}
		
	// -----------------------------------attrs------------------------------------------
	
	HttpClientRequest* m_host;
	KeepLoop*  m_keep;
	HttpDelegate* m_delegate;
	int64      m_upload_total;    /* 需上传到服务器数据总量 */
	int64      m_upload_size;     /* 已写上传到服务器数据尺寸 */
	int64      m_download_total;  /* 需下载数据总量 */
	int64      m_download_size;   /* 已下载数据量 */
	HttpReadyState m_ready_state; /* 请求状态 */
	int         m_status_code;    /* 服务器响应http状态码 */
	HttpMethod  m_method;
	URI         m_uri;
	String      m_save_path;
	Connect*    m_connect;
	FileCacheReader* m_cache_reader;
	FileWriter* m_file_writer;
	Map<String, String> m_request_header;
	Map<String, String> m_response_header;
	Map<String, FormValue> m_post_form_data;
	Buffer      m_post_data;
	String      m_username;
	String      m_password;
	String      m_cache_path;
	String      m_http_response_version;
	bool        m_disable_cache;
	bool        m_disable_cookie;
	bool        m_disable_send_cookie;
	bool        m_disable_ssl_verify;
	//bool        _disable_ssl_verify_host; //
	bool        m_keep_alive;
	Sending*    m_sending;
	uint64      m_timeout;
	bool        m_pause;
	bool        m_url_no_cache_arg;
	uint        m_wait_connect_id, m_write_cache_flag;
	static      ConnectPool m_pool;
	static      ConnectPool* m_pool_ptr;
};

HttpClientRequest::Inl::ConnectPool HttpClientRequest::Inl::m_pool;
HttpClientRequest::Inl::ConnectPool* HttpClientRequest::Inl::m_pool_ptr = nullptr;

HttpClientRequest::HttpClientRequest(RunLoop* loop): m_inl(NewRetain<Inl>(this, loop))
{
}

HttpClientRequest::~HttpClientRequest() {
	NX_CHECK(m_inl->m_keep->host() == RunLoop::current());
	m_inl->set_delegate(nullptr);
	m_inl->abort();
	m_inl->release();
	m_inl = nullptr;
}

void HttpClientRequest::set_delegate(HttpDelegate* delegate) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->set_delegate(delegate);
}

void HttpClientRequest::set_method(HttpMethod method) throw(Error) {
	m_inl->check_is_can_modify();
	if ( method < HTTP_METHOD_GET || method > HTTP_METHOD_PUT ) {
		method = HTTP_METHOD_GET;
	}
	m_inl->m_method = method;
}

void HttpClientRequest::set_url(cString& path) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_uri = URI(path);
}

void HttpClientRequest::set_save_path(cString& path) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_save_path = path;
}

void HttpClientRequest::set_username(cString& username) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_username = username;
}

void HttpClientRequest::set_password(cString& password) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_password = password;
}

void HttpClientRequest::disable_cache(bool disable) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_disable_cache = disable;
}

void HttpClientRequest::disable_cookie(bool disable) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_disable_cookie = disable;
}

void HttpClientRequest::disable_send_cookie(bool disable) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_disable_send_cookie = disable;
}

void HttpClientRequest::disable_ssl_verify(bool disable) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_disable_ssl_verify = disable;
}

void HttpClientRequest::set_keep_alive(bool keep_alive) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_keep_alive = keep_alive;
}

void HttpClientRequest::set_timeout(uint64 timeout) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_timeout = timeout;
}

void HttpClientRequest::set_request_header(cString& name, cString& value) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_request_header.set(name, value);
}

void HttpClientRequest::set_form(cString& form_name, cString& value) throw(Error) {
	m_inl->check_is_can_modify();
	NX_ASSERT_ERR( value.length() <= BUFFER_SIZE,
								ERR_HTTP_FORM_SIZE_LIMIT, "Http form field size limit <= %d", BUFFER_SIZE);
	m_inl->m_post_form_data.set(form_name, {
		FORM_TYPE_TEXT, value, inl__uri_encode(form_name)
	});
}

void HttpClientRequest::set_upload_file(cString& form_name, cString& path) throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_post_form_data.set(form_name, {
		FORM_TYPE_FILE, path, inl__uri_encode(form_name)
	});
}

void HttpClientRequest::clear_request_header() throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_request_header.clear();
}

void HttpClientRequest::clear_form_data() throw(Error) {
	m_inl->check_is_can_modify();
	m_inl->m_post_form_data.clear();
}

String HttpClientRequest::get_response_header(cString& name) {
	auto i = m_inl->m_response_header.find(name);
	if ( i.is_null() ) return String();
	return i.value();
}

const Map<String, String>& HttpClientRequest::get_all_response_headers() {
	return m_inl->m_response_header;
}

int64 HttpClientRequest::upload_total() const {
	return m_inl->m_upload_total;
}

int64 HttpClientRequest::upload_size() const {
	return m_inl->m_upload_size;
}

int64 HttpClientRequest::download_total() const {
	return m_inl->m_download_total;
}

int64 HttpClientRequest::download_size() const {
	return m_inl->m_download_size;
}

HttpReadyState HttpClientRequest::ready_state() const {
	return m_inl->m_ready_state;
}

int HttpClientRequest::status_code() const {
	return m_inl->m_status_code;
}

String HttpClientRequest::url() const {
	return m_inl->m_uri.href();
}

void HttpClientRequest::send(Buffer data) throw(Error) { // thread safe
	m_inl->send(data);
}

void HttpClientRequest::send(cString& data) throw(Error) { // thread safe
	m_inl->send(data.copy_buffer());
}

void HttpClientRequest::pause() { // thread safe
	m_inl->pause();
}

void HttpClientRequest::resume() { // thread safe
	m_inl->resume();
}

void HttpClientRequest::abort() { // thread safe
	m_inl->abort();
}

String HttpClientRequest::http_response_version() const {
	return m_inl->m_http_response_version;
}

NX_END
