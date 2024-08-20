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

#include "./net.h"
#include "./string.h"
#include "./uv.h"
#include "./fs.h"
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace qk {
	typedef Socket::Delegate Delegate;

	static int ssl_initializ = 0;
	static X509_STORE* ssl_x509_store = nullptr;
	static SSL_CTX* ssl_v23_client_ctx = nullptr;

	X509_STORE* NewRootCertStore();

	struct SocketWriteReqData {
		Buffer  raw_buffer;
		int32_t mark;
	};
	struct SSLSocketWriteReqData {
		Buffer     raw_buffer;
		int32_t    mark;
		uint32_t   buffers_count;
		int32_t    error;
		Buffer     buffers[2];
	};
	typedef UVRequestWrap<uv_connect_t, Socket::Inl> SocketConReq;
	typedef UVRequestWrap<uv_shutdown_t, Socket::Inl> SocketShutdownReq;
	typedef UVRequestWrap<uv_write_t, Socket::Inl, SocketWriteReqData> SocketWriteReq;
	typedef UVRequestWrap<uv_write_t, Socket::Inl, SSLSocketWriteReqData> SSLSocketWriteReq;

	// ----------------------------------------------------------------------

	class Socket::Inl: public Reference, public Socket::Delegate {
	public:
		typedef ReferenceTraits Traits;
		virtual void trigger_socket_open(Socket* stream) override {}
		virtual void trigger_socket_close(Socket* stream) override {}
		virtual void trigger_socket_error(Socket* stream, cError& error) override {}
		virtual void trigger_socket_data(Socket* stream, cBuffer& buffer) override {}
		virtual void trigger_socket_write(Socket* stream, Buffer buffer, int mark) override {}
		virtual void trigger_socket_timeout(Socket* socket) override {}

		Inl(Socket* host, RunLoop* loop) 
			: _host(host)
			, _delegate(this)
			, _loop(loop)
			, _retain(nullptr)
			, _is_open(false)
			, _is_opening(false)
			, _is_pause(false)
			, _enable_keep_alive(false)
			, _no_delay(false)
			, _keep_idle(7200)
			, _port(0)
			, _uv_tcp(nullptr)
			, _uv_timer(nullptr)
			, _timeout(0)
		{
			Qk_Assert(loop);
		}

		~Inl() {
			close_delete();
			Qk_Assert(!_is_open);
			Qk_Assert(!_retain);
		}

		struct RetainRef {
			typedef NonObjectTraits Traits;
			RetainRef(Inl* hold, uv_loop_t* loop): hold(hold) {
				Qk_Assert_Eq(0, uv_tcp_init(loop, &tcp));
				Qk_Assert_Eq(0, uv_timer_init(loop, &timer));
				tcp.data = this;
				timer.data = this;
			}
			Sp<Inl> hold;
			uv_tcp_t tcp;
			uv_timer_t timer;
		};

		// utils

		void initialize(cString& hostname, uint16_t port) {
			_hostname = hostname;
			_port = port;
		}

		inline RunLoop* loop() { return _loop; }
		inline uv_loop_t* uv_loop() { return _loop->uv_loop(); }

		static void report_err_async(Cb::Data& evt, Inl *self) {
			self->_delegate->trigger_socket_error(self->_host, *evt.error);
		}

		void report_err(Error err, bool async = false) {
			if (async)
				async_reject(Cb(report_err_async, this), std::move(err), _loop);
			else
				_delegate->trigger_socket_error(_host, err);
		}

		void report_err_and_close(Error err, bool async = false) {
			report_err(err, async);
			close();
		}

		int report_uv_err(int code, bool async = false) {
			if ( code != 0 ) {
				report_err( Error(code, "%s, %s", uv_err_name(code), uv_strerror(code)), async);
			}
			return code;
		}

		void report_not_open_connect_err(bool async = false) {
			// report_uv_err(ENOTCONN);
			report_err(Error(ERR_NOT_OPTN_TCP_CONNECT, "not tcp connect or open connecting"), async);
		}

		void reset_timeout() {
			if ( _is_open ) {
				uv_timer_stop(_uv_timer);
				if ( _timeout && !_is_pause ) {
					uv_timer_start(_uv_timer, [](uv_timer_t* handle) {
						Inl* self = *static_cast<RetainRef*>(handle->data)->hold;
						self->_delegate->trigger_socket_timeout(self->_host);
					}, _timeout / 1000, 0);
				}
			}
		}

		// -------------------------------- public api --------------------------------

		void set_delegate(Delegate* delegate) {
			_delegate = delegate ? delegate: this;
		}

		void open() {
			if ( _is_opening ) {
				report_err(Error(ERR_CONNECT_ALREADY_OPEN, "Connect opening or already open"), 1);
			} else {
				try_open();
			}
		}

		void close() {
			if (_is_open) {
				shutdown();
			} else {
				report_not_open_connect_err(1);
			}
		}

		bool ipv6() {
			return _address.sa_family == AF_INET6;
		}

		bool is_open() {
			return _is_open;
		}

		bool is_pause() {
			return _is_open && _is_pause;
		}

		void set_keep_alive(bool enable, uint64_t keep_idle) {
			/*
			keepalive默认是关闭的, 因为虽然流量极小, 毕竟是开销. 因此需要用户手动开启. 有两种方式开启.
			
			除了keepAlive 开关, 还有keepIdle, keepInterval, keepCount 3个属性, 使用简单, 如下:
			*/
			/*
			int keepAlive = 1;   // 开启keepalive属性. 缺省值: 0(关闭)
			int keepIdle = 60;   // 如果在60秒内没有任何数据交互,则进行探测. 缺省值:7200(s)
			int keepInterval = 5;// 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
			int keepCount = 2;   // 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
			
			int fd = _retain.io_watcher.fd;
			setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
			setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&keepIdle, sizeof(keepIdle));
			setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
			setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));
			*/
			_enable_keep_alive = enable;
			_keep_idle = keep_idle ? uint32_t(keep_idle / 1000000): 7200;
			if ( _uv_tcp && uv_is_active((uv_handle_t*)_uv_tcp) ) {
				uv_tcp_keepalive(_uv_tcp, _enable_keep_alive, _keep_idle);
			}
		}

		void set_no_delay(bool no_delay) {
			_no_delay = no_delay;
			if ( _uv_tcp && uv_is_active((uv_handle_t*)_uv_tcp) ) {
				uv_tcp_nodelay(_uv_tcp, _no_delay);
			}
		}

		void set_timeout(uint64_t timeout) {
			_timeout = timeout;
			reset_timeout();
		}

		void pause() {
			_is_pause = true;
			if ( _is_open ) {
				uv_read_stop((uv_stream_t*)_uv_tcp);
			}
		}

		void resume() {
			if ( _is_open ) {
				if ( _is_pause ) {
					_is_pause = false;
					start_read();
				}
			} else {
				_is_pause = false;
			}
		}

		void write(Buffer buffer, int64_t size, int mark) {
			if ( size < 0 ) {
				size = buffer.length();
			}
			if ( _is_open ) {
				if ( uv_is_writable((uv_stream_t*)_uv_tcp) ) {
					reset_timeout();
					write(buffer, mark);
				} else {
					report_err(Error(ERR_SOCKET_NOT_WRITABLE, "Socket not writable"), 1);
				}
			} else {
				report_not_open_connect_err(1);
			}
		}

		// ------------------------------------------------------------------------------------------
		void try_open() {
			Qk_Assert(_is_opening == false);
			Qk_Assert(_retain == nullptr);
			
			if ( _remote_ip.isEmpty() ) {
				sockaddr_in sockaddr;
				sockaddr_in6 sockaddr6;
				Char dst[64];
	
				if ( uv_ip4_addr(_hostname.c_str(), _port, &sockaddr) == 0 ) {
					_address = *((struct sockaddr*)&sockaddr);
					// uv_ip4_name(&sockaddr, dst, 64); _remote_ip = dst;
					_remote_ip = _hostname;
				}
				else if ( uv_ip6_addr(_hostname.c_str(), _port, &sockaddr6) == 0 ) {
					_address = *((struct sockaddr*)&sockaddr6);
					// uv_ip6_name(&sockaddr6, dst, 64); _remote_ip = dst;
					_remote_ip = _hostname;
				}
				else {
					hostent* host = gethostbyname(_hostname.c_str());
					if ( host ) {
						if ( host->h_addrtype == AF_INET ) { // ipv4
							memset(&sockaddr, 0, sizeof(sockaddr_in));
							//sockaddr.sin_len = host->h_length;
							sockaddr.sin_family = host->h_addrtype;
							sockaddr.sin_port = htons(_port);
							sockaddr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
							_address = *((struct sockaddr*)&sockaddr);
							uv_ip4_name(&sockaddr, dst, 64); _remote_ip = dst;
						} else if ( host->h_addrtype == AF_INET6 ) { // ipv6
							memset(&sockaddr6, 0, sizeof(sockaddr_in6));
							//sockaddr6.sin6_len = host->h_length;
							sockaddr6.sin6_family = host->h_addrtype;
							sockaddr6.sin6_port = htons(_port);
							sockaddr6.sin6_addr = *((struct in6_addr *)host->h_addr_list[0]);
							_address = *((struct sockaddr*)&sockaddr6);
							uv_ip6_name(&sockaddr6, dst, 64); _remote_ip = dst;
						} else {
							report_err(Error(ERR_PARSE_HOSTNAME_ERROR, "Parse hostname error `%s`", _hostname.c_str()),1);
							return;
						}
					} else {
						report_err(Error(ERR_PARSE_HOSTNAME_ERROR, "Parse hostname error `%s`", _hostname.c_str()),1);
						return;
					}
				}
			}
			Qk_Assert(!_remote_ip.isEmpty());

			_retain = new RetainRef(this, uv_loop());
			Qk_Assert(_uv_tcp == nullptr);
			Qk_Assert(_uv_timer == nullptr);
			_uv_tcp = &_retain->tcp;
			_uv_timer = &_retain->timer;
			auto req = new SocketConReq(this);

			int r = uv_tcp_connect(req->req(), _uv_tcp, &_address, [](uv_connect_t* uv_req, int status) {
				Handle<SocketConReq> req = SocketConReq::cast(uv_req);
				Inl* self = req->ctx();
				Qk_Assert(self->_is_opening);
				Qk_Assert(!self->_is_open);

				uv_tcp_keepalive(self->_uv_tcp, self->_enable_keep_alive, self->_keep_idle);
				uv_tcp_nodelay(self->_uv_tcp, self->_no_delay);

				if ( status ) {
					self->_is_opening = false;
					self->report_uv_err(status);
					self->close_delete();
				} else {
					self->trigger_socket_connect_open();
				}
			});

			if (report_uv_err(r)) {
				Release(req);
				close_delete();
			} else {
				_is_opening = true;
			}
		}

		void close_delete() {
			if (!_retain)
				return;
			uv_close((uv_handle_t*)_uv_tcp, [](uv_handle_t* handle) {
				Sp<RetainRef> h((RetainRef*)handle->data);
			});
			uv_timer_stop(_uv_timer);

			_retain = nullptr;
			_uv_tcp = nullptr;
			_uv_timer = nullptr;
			_is_pause = false;
			if (_is_open) {
				_is_opening = false;
				_is_open = false;
				_delegate->trigger_socket_close(_host);
			} else if (_is_opening) {
				_is_opening = false;
				Error err(ERR_CONNECT_UNEXPECTED_SHUTDOWN, "Connect unexpected shutdown");
				report_err(err);
			}
		}

		virtual void trigger_socket_connect_open() {
			_is_open = true;
			if ( !_is_pause ) {
				start_read(); // start receive data
			}
			reset_timeout();
			_delegate->trigger_socket_open(_host);
		}

		virtual void shutdown() {
			auto req = new SocketShutdownReq(this);

			int r = uv_shutdown(req->req(), (uv_stream_t*)_uv_tcp, [](uv_shutdown_t* uv_req, int status) {
				Handle<SocketShutdownReq> req(SocketShutdownReq::cast(uv_req));
				Inl* self = req->ctx();
				if ( status != 0 && status != UV_ECANCELED ) {
					// close status is send error
					self->report_uv_err(status);
				}
				self->close_delete();
			});

			if ( report_uv_err(r) ) {
				Release(req);
			}
		}

		void start_read() {
			uv_read_start(
				(uv_stream_t*)_uv_tcp,
				[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
					Inl* self = *static_cast<RetainRef*>(handle->data)->hold;
					if ( self->_read_buffer.isNull() ) {
						self->_read_buffer = Buffer::alloc( Qk_MIN(65536, uint32_t(suggested_size)) );
					}
					buf->base = *self->_read_buffer;
					buf->len = self->_read_buffer.length();
				},
				[](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
					static_cast<RetainRef*>(stream->data)->hold->trigger_socket_data_char(int(nread), buf->base);
				}
			);
		}

		virtual void trigger_socket_data_char(int nread, Char* buffer) {
			Qk_Assert( _is_open );
			if ( nread < 0 ) {
				if ( nread != UV_EOF ) { // 异常断开
					report_uv_err(int(nread));
				}
				close_delete();
			} else {
				reset_timeout();
				WeakBuffer buff(buffer, nread);
				_delegate->trigger_socket_data(_host, buff.buffer());
			}
		}

		virtual void write(Buffer& buffer, int mark) {
			auto req = new SocketWriteReq(this, 0, { buffer, mark });
			uv_buf_t buf;
			buf.base = *req->data().raw_buffer;
			buf.len = req->data().raw_buffer.length();

			int r = uv_write(req->req(), (uv_stream_t*)_uv_tcp, &buf, 1,
			[](uv_write_t* uv_req, int status) {
				SocketWriteReq* req = SocketWriteReq::cast(uv_req);
				Handle<SocketWriteReq> handle(req);
				if ( status < 0 ) {
					req->ctx()->report_uv_err(status);
				} else {
					req->ctx()->_delegate->trigger_socket_write(req->ctx()->_host,
																											req->data().raw_buffer,
																											req->data().mark);
				}
			});

			if ( report_uv_err(r) ) {
				Release(req);
			}
		}
		
	public:
		// ----------------------------------------------------------------------
		Socket*     _host;
		Delegate*   _delegate;
		RunLoop*    _loop;
		RetainRef*  _retain;
		bool        _is_open;
		bool        _is_opening;
		bool        _is_pause;
		bool        _enable_keep_alive;
		bool        _no_delay;
		uint32_t    _keep_idle;
		String      _hostname;
		uint16_t    _port;
		uv_tcp_t*   _uv_tcp;
		uv_timer_t* _uv_timer;
		sockaddr    _address;
		String      _remote_ip;
		Buffer      _read_buffer;
		uint64_t    _timeout;
	};

	// ----------------------------------------------------------------------

	class SSL_INL: public Socket::Inl {
	public:

		static void initializ_ssl() {
			if ( ! ssl_initializ++ ) {
				SSL_load_error_strings();
				SSL_library_init();
				//OpenSSL_add_all_algorithms();

				ssl_v23_client_ctx = SSL_CTX_new( SSLv23_client_method() );
				SSL_CTX_set_verify(ssl_v23_client_ctx, SSL_VERIFY_PEER, NULL);
				if (!ssl_x509_store) {
					ssl_x509_store = NewRootCertStore();
					// ssl_x509_store = NewRootCertStoreFromFile(fs_resources("cacert.pem"));
					SSL_CTX_set_cert_store(ssl_v23_client_ctx, ssl_x509_store);
				}
			}
		}

		static void ssl_info_callback(const SSL* ssl, int where, int ret) {
			if ( where & SSL_CB_HANDSHAKE_START ) { /*LOG("----------------start");*/ }
			if ( where & SSL_CB_HANDSHAKE_DONE ) { /* Qk_LOG("----------------done"); */ }
		}

		static int bio_puts(BIO *bp, cChar *str) {
			return bio_write(bp, str, int(strlen(str)));
		}

		static long bio_ctrl(BIO *b, int cmd, long num, void *ptr) {
			long ret = 1;
			int *ip;

			switch (cmd) {
				case BIO_C_SET_FD:
					// sock_close(b);
					b->num = *((int *)ptr);
					b->shutdown = (int)num;
					b->init = 1;
					break;
				case BIO_C_GET_FD:
					if (b->init) {
						ip = (int *)ptr;
						if (ip != NULL)
							*ip = b->num;
						ret = b->num;
					} else
						ret = -1;
					break;
				case BIO_CTRL_GET_CLOSE: ret = b->shutdown; break;
				case BIO_CTRL_SET_CLOSE: b->shutdown = (int)num; break;
				case BIO_CTRL_DUP:
				case BIO_CTRL_FLUSH: ret = 1; break;
				default: ret = 0; break;
			}
			return (ret);
		}

		static BIO_METHOD bio_method;

		// ---------------------------------------------------------

		SSL_INL(Socket* host, RunLoop* loop)
			: Inl(host, loop)
			, _bio_read_source_buffer(nullptr)
			, _bio_read_source_buffer_length(0)
			, _ssl_handshake(0), _ssl_write_req(nullptr) 
		{
			initializ_ssl();
	
			_ssl = SSL_new(ssl_v23_client_ctx);
			
			SSL_set_app_data(_ssl, this);
			SSL_set_info_callback(_ssl, &ssl_info_callback);
			SSL_set_verify(_ssl, SSL_VERIFY_PEER, nullptr);
			
			BIO* bio = BIO_new(&bio_method);
			bio->ptr = this;
			BIO_set_fd(bio, 0, BIO_NOCLOSE);
			SSL_set_bio(_ssl, bio, bio);
		}

		~SSL_INL() {
			SSL_free(_ssl);
		}

		void disable_ssl_verify(bool disable) {
			if ( disable ) {
				SSL_set_verify(_ssl, SSL_VERIFY_NONE, nullptr);
			} else {
				SSL_set_verify(_ssl, SSL_VERIFY_PEER, nullptr);
			}
		}

		virtual void shutdown() {
			SSL_shutdown(_ssl);
			Inl::shutdown();
		}

		static void ssl_write_cb(uv_write_t* req, int status) {
			SSLSocketWriteReq* req_ = SSLSocketWriteReq::cast(req);
			Qk_Assert(req_->data().buffers_count);

			req_->data().buffers_count--;

			auto self = req_->ctx();

			if ( status < 0 ) {
				req_->data().error++;
				if ( req_->data().buffers_count == 0 ) {
					Release(req_);
				}
				self->report_uv_err(status);
			} else {
				if ( req_->data().buffers_count == 0 ) {
					Sp<SSLSocketWriteReq> sp(req_);
					if ( req_->data().error == 0 ) {
						self->_delegate->trigger_socket_write(
							self->_host, req_->data().raw_buffer, req_->data().mark
						);
					} 
				}
			}
		}

		static void ssl_handshake_write_cb(uv_write_t* req, int status) {
			Handle<SocketWriteReq> req_(SocketWriteReq::cast(req));
			if ( status < 0 ) { // send handshake msg fail
				req_->ctx()->close();
			}
		}

		static void ssl_other_write_cb(uv_write_t* uv_req, int status) {
			Handle<SocketWriteReq> req(SocketWriteReq::cast(uv_req));
			// Do nothing
		}

		static int bio_write(BIO* b, cChar* in, int inl) {
			SSL_INL* self = ((SSL_INL*)b->ptr);
			Qk_Assert( self->_ssl_handshake );

			int r;
			Buffer buffer = WeakBuffer(in, inl)->copy();
			
			if ( self->_ssl_handshake == 1 ) { // handshake or SSL_shutdown
				
				auto req = new SocketWriteReq(self, 0, { buffer });
				uv_buf_t buf;
				buf.base = *req->data().raw_buffer;
				buf.len = req->data().raw_buffer.length();
				
				r = uv_write(req->req(), (uv_stream_t*)self->_uv_tcp, &buf, 1, &ssl_handshake_write_cb);
				
				if ( self->report_uv_err(r) ) {
					r = -1;
					Release(req);
					self->close(); // close connect
				} else {
					r = inl;
				}
				
				BIO_clear_retry_flags(b);
				
				return r;
				
			} else {
				
				if ( self->_ssl_write_req ) { // send msg
					
					auto req = self->_ssl_write_req;
					Qk_Assert( req->data().buffers_count < 2 );
					
					uv_buf_t buf;
					buf.base = *buffer;
					buf.len = inl;
					
					req->data().buffers[req->data().buffers_count] = buffer;
					
					r = uv_write(req->req(), (uv_stream_t*)self->_uv_tcp, &buf, 1, &ssl_write_cb);
					
					if ( self->report_uv_err(r) ) {
						// uv err
					} else {
						req->data().buffers_count++;
					}
					
				} else { // SSL_shutdown or ssl other
					
					auto req = new SocketWriteReq(self, 0, { buffer });
					uv_buf_t buf;
					buf.base = *req->data().raw_buffer;
					buf.len = req->data().raw_buffer.length();
					
					r = uv_write(req->req(), (uv_stream_t*)self->_uv_tcp, &buf, 1, &ssl_other_write_cb);
					
					if ( r != 0 ) {
						Release(req);
					}
				}
				
				return inl;
			}
		}
		
		static int bio_read(BIO *b, Char* out, int outl) {
			Qk_Assert(out);
			SSL_INL* self = ((SSL_INL*)b->ptr);
			
			int ret = Qk_MIN(outl, self->_bio_read_source_buffer_length);
			if ( ret > 0 ) {
				memcpy(out, self->_bio_read_source_buffer, ret);
				self->_bio_read_source_buffer += ret;
				self->_bio_read_source_buffer_length -= ret;
			}
			BIO_clear_retry_flags(b);
			
			return ret;
		}
		
		static int receive_ssl_err(cChar *str, size_t len, void *u) {
			SSL_INL* self = (SSL_INL*)u;
			self->_ssl_error_msg.append(str, uint32_t(len));
			return 1;
		}
		
		void report_ssl_err(int code) {
			_ssl_error_msg = String();
			ERR_print_errors_cb(&receive_ssl_err, this);
			report_err(Error(code, _ssl_error_msg));
		}
		
		void ssl_handshake_fail() {
			report_err(Error(ERR_SSL_HANDSHAKE_FAIL, "ssl handshake fail"));
			close();
		}
		
		static void ssl_handshake_timeout_cb(uv_timer_t* handle) {
			SSL_INL* self = static_cast<SSL_INL*>(*static_cast<RetainRef*>(handle->data)->hold);
			self->ssl_handshake_fail();
		}
		
		void set_ssl_handshake_timeout() {
			Qk_Assert(_retain);
			uv_timer_stop(_uv_timer);
			uv_timer_start(_uv_timer, &ssl_handshake_timeout_cb, 1e7, 0); // 10s handshake timeout
		}
		
		virtual void trigger_socket_connect_open() {
			Qk_Assert( !_ssl_handshake );
			set_ssl_handshake_timeout();
			_bio_read_source_buffer_length = 0;
			_ssl_handshake = 1;
			start_read();
			SSL_set_connect_state(_ssl);
			if ( SSL_connect(_ssl) < 0 ) {
				ssl_handshake_fail();
			}
		}

		virtual void trigger_socket_data_char(int nread, Char* buffer) {
			if ( nread < 0 ) {

				if ( _ssl_handshake == 0 ) { //
					report_err(Error(ERR_SSL_HANDSHAKE_FAIL, "ssl handshake fail"));
				} else {
					if ( nread != UV_EOF ) { // 异常断开
						report_uv_err(int(nread));
					}
				}
				close_delete();
			} else {
				Qk_Assert( _bio_read_source_buffer_length == 0 );

				_bio_read_source_buffer = buffer;
				_bio_read_source_buffer_length = nread;

				if ( !_ssl_read_buffer.length() ) {
					_ssl_read_buffer = Buffer::alloc(65536);
				}

				if ( _is_open ) {
					reset_timeout();

					while (1) {
						int i = SSL_read(_ssl, _ssl_read_buffer.val(), 65536);

						if ( i > 0 ) {
							WeakBuffer buff(_ssl_read_buffer.val(), i);
							_delegate->trigger_socket_data(_host, buff.buffer());
						} else {
							if ( i < 0 ) { // err
								report_ssl_err(ERR_SSL_UNKNOWN_ERROR); close(); // close connect
							}
							break;
						}
					}
				} else { // ssl handshake
					Qk_Assert(_ssl_handshake == 1);

					int r = SSL_connect(_ssl);

					if ( r < 0 ) {
						ssl_handshake_fail();
					}
					else if ( r == 1 ) {
						_ssl_handshake = 2; // ssl handshake done
						_is_open = true;
						
						if ( _is_pause ) {
							uv_read_stop((uv_stream_t*)_uv_tcp); // pause status
						}
						reset_timeout();
						_delegate->trigger_socket_open(_host);
						
						Qk_Assert( _bio_read_source_buffer_length == 0 );
					}
				}
				
			} // if ( nread < 0 ) end
		}

		virtual void write(Buffer& buffer, int mark) {
			Qk_Assert(!_ssl_write_req);

			auto req = new SSLSocketWriteReq(this, 0, { buffer, mark, 0, 0 });
			_ssl_write_req = req;

			int r = SSL_write(_ssl, req->data().raw_buffer.val(), req->data().raw_buffer.length());

			_ssl_write_req = nullptr;

			if ( r < 0 ) {
				report_ssl_err(ERR_SSL_UNKNOWN_ERROR);
			}
			if ( req->data().buffers_count == 0 ) {
				Release(req);
			}
		}

	private:
		SSL*    _ssl;
		cChar*  _bio_read_source_buffer;
		int     _bio_read_source_buffer_length;
		Buffer  _ssl_read_buffer;
		String  _ssl_error_msg;
		int     _ssl_handshake;
		SSLSocketWriteReq* _ssl_write_req;
	};

	BIO_METHOD SSL_INL::bio_method = {
		BIO_TYPE_MEM,
		"socket",
		bio_write,
		bio_read,
		bio_puts,
		nullptr,  /* sock_gets, */
		bio_ctrl,
		nullptr,
		nullptr,
		nullptr
	};

	// ----------------------------------------------------------------------

	Socket::Socket(): _inl(nullptr) {}

	Socket::Socket(cString& hostname, uint16_t port, RunLoop* loop)
	: _inl( NewRetain<Inl>(this, loop) ) {
		_inl->initialize(hostname, port);
	}

	Socket::~Socket() {
		_inl->set_delegate(nullptr);
		if (_inl->is_open())
			_inl->close();
		_inl->release();
		_inl = nullptr;
	}
	void Socket::open() {
		_inl->open();
	}
	String Socket::hostname() const {
		return _inl->_hostname;
	}
	uint16_t  Socket::port() const {
		return _inl->_port;
	}
	String Socket::ip() const {
		return _inl->_remote_ip;
	}
	bool Socket::ipv6() const {
		return _inl->ipv6();
	}
	void Socket::set_keep_alive(bool enable, uint64_t keep_idle) {
		_inl->set_keep_alive(enable, keep_idle);
	}
	void Socket::set_no_delay(bool no_delay) {
		_inl->set_no_delay(no_delay);
	}
	void Socket::set_timeout(uint64_t timeout) {
		_inl->set_timeout(timeout);
	}
	void Socket::set_delegate(Delegate* delegate) {
		_inl->set_delegate(delegate);
	}
	void Socket::close() {
		_inl->close();
	}
	bool Socket::is_open() {
		return _inl->is_open();
	}
	bool Socket::is_pause() {
		return _inl->is_pause();
	}
	void Socket::pause() {
		_inl->pause();
	}
	void Socket::resume() {
		_inl->resume();
	}
	void Socket::write(Buffer buffer, int mark) {
		uint32_t size = buffer.length();
		_inl->write(buffer, size, mark);
	}

	/*
	void set_ssl_cacert_file(cString& path) {
		try {
			NewRootCertStoreFromFile(fs_read_file_sync(path));
		} catch(cError& err) {
			Qk_ERR("SSL", "NewRootCertStoreFromFile() fail, %s", err.message().c_str());
		}
	}

	void set_ssl_client_key_file(cString& path) {
	// TODO
	}

	void set_ssl_client_keypasswd(cString& passwd) {
	// TODO
	}*/

	SSLSocket::SSLSocket(cString& hostname, uint16_t port, RunLoop* loop) {
		_inl = NewRetain<SSL_INL>(this, loop);
		_inl->initialize(hostname, port);
	}

	void SSLSocket::disable_ssl_verify(bool disable) {
		static_cast<SSL_INL*>(_inl)->disable_ssl_verify(disable);
	}

}
