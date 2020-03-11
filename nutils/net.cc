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

#include "nutils/net.h"
#include "nutils/string.h"
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "nutils/http.h"
#include "nutils/uv-1.h"

XX_NS(ngui)

typedef Socket::Delegate Delegate;

static int ssl_initializ = 0;
static String* ssl_cacert_file_path = new String();
static String* ssl_client_key_file_path = new String();
static String* ssl_client_keypasswd = new String();
static X509_STORE* ssl_x509_store = nullptr;
static SSL_CTX* ssl_v23_client_ctx = nullptr;
static X509_STORE* (*new_root_cert_store)() = nullptr;

struct SocketWriteReqData {
	Buffer raw_buffer;
	int    mark;
};
struct SSLSocketWriteReqData {
	Buffer raw_buffer;
	int    mark;
	uint   buffers_count;
	int    error;
	Buffer buffers[2];
};
typedef UVRequestWrap<uv_connect_t, Socket::Inl> SocketConReq;
typedef UVRequestWrap<uv_shutdown_t, Socket::Inl> SocketShutdownReq;
typedef UVRequestWrap<uv_write_t, Socket::Inl, SocketWriteReqData> SocketWriteReq;
typedef UVRequestWrap<uv_write_t, Socket::Inl, SSLSocketWriteReqData> SSLSocketWriteReq;

/**
 * @class Socket::Inl
 */
class Socket::Inl: public Reference, public Socket::Delegate {
 public:
	typedef ReferenceTraits Traits;
	virtual void trigger_socket_open(Socket* stream) {}
	virtual void trigger_socket_close(Socket* stream) {}
	virtual void trigger_socket_error(Socket* stream, cError& error) {}
	virtual void trigger_socket_data(Socket* stream, Buffer& buffer) {}
	virtual void trigger_socket_write(Socket* stream, Buffer buffer, int mark) {}
	virtual void trigger_socket_timeout(Socket* socket) {}

	Inl(Socket* host, RunLoop* loop) 
	: m_host(host)
	, m_delegate(this)
	, m_keep(loop->keep_alive("Socket::Inl", false))
	, m_uv_handle(nullptr)
	, m_is_open(false)
	, m_is_opening(false)
	, m_is_pause(false)
	, m_enable_keep_alive(false)
	, m_no_delay(false)
	, m_keep_idle(7200)
	, m_port(0)
	, m_uv_tcp(nullptr)
	, m_uv_timer(nullptr)
	, m_timeout(0)
	{ //
		XX_CHECK(m_keep);
	}
	
	virtual ~Inl() {
		XX_CHECK(!m_is_open);
		XX_CHECK(!m_uv_handle);
		Release(m_keep); m_keep = nullptr;
	}
	
	class UVHandle {
	 public:
		typedef NonObjectTraits Traits;
		inline UVHandle(Inl* host, uv_loop_t* loop): host(host) {
			host->retain();
			int r;
			r = uv_tcp_init(loop, &uv_tcp); XX_ASSERT( r == 0 );
			r = uv_timer_init(loop, &uv_timer); XX_ASSERT( r == 0 );
			uv_tcp.data = this;
			uv_timer.data = this;
		}
		inline ~UVHandle() {
			host->release();
		}
		Inl* host;
		uv_tcp_t  uv_tcp;
		uv_timer_t  uv_timer;
	};
	
	// utils
	
	void initialize(cString& hostname, uint16 port) {
		m_hostname = hostname;
		m_port = port;
	}
	
	inline RunLoop* loop() { return m_keep->host(); }
	inline uv_loop_t* uv_loop() { return loop()->uv_loop(); }
	
	void report_err_from_loop(CbD& evt) {
		m_delegate->trigger_socket_error(m_host, *evt.error);
	}
	
	void report_err(Error err, bool async = false) {
		if (async)
			async_err_callback(Cb(&Inl::report_err_from_loop, this), move(err), m_keep);
		else
			m_delegate->trigger_socket_error(m_host, err);
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
	
	void timeout_cb2(CbD& evt) {
		m_delegate->trigger_socket_timeout(m_host);
	}
	
	static void timeout_cb(uv_timer_t* handle) {
		Inl* self = static_cast<Inl*>(handle->data);
		async_callback(Cb(&Inl::timeout_cb2, self));
	}
	
	void reset_timeout() {
		if ( m_is_open ) {
			uv_timer_stop(m_uv_timer);
			if ( m_timeout && !m_is_pause ) {
				uv_timer_start(m_uv_timer, &timeout_cb, m_timeout / 1000, 0);
			}
		}
	}
	
	// ---------------------------------------- public api ----------------------------------------
	
	void set_delegate(Delegate* delegate) {
		if ( delegate ) {
			m_delegate = delegate;
		} else {
			m_delegate = this;
		}
	}
	
	void open() {
		if ( m_is_opening ) {
			report_err(Error(ERR_CONNECT_ALREADY_OPEN, "Connect opening or already open"), 1);
		} else {
			open2();
		}
	}
	
	void close() {
		if (m_is_open) {
			shutdown();
		} else {
			report_not_open_connect_err(1);
		}
	}
	
	bool ipv6() {
		return m_address.sa_family == AF_INET6;
	}
	
	bool is_open() {
		return m_is_open;
	}
	
	bool is_pause() {
		return m_is_open && m_is_pause;
	}
	
	void set_keep_alive(bool enable, uint64 keep_idle) {
		/*
		 keepalive默认是关闭的, 因为虽然流量极小, 毕竟是开销. 因此需要用户手动开启. 有两种方式开启.
		 
		 除了keepAlive 开关, 还有keepIdle, keepInterval, keepCount 3个属性, 使用简单, 如下:
		 */
		/*
		 int keepAlive = 1;   // 开启keepalive属性. 缺省值: 0(关闭)
		 int keepIdle = 10;   // 如果在60秒内没有任何数据交互,则进行探测. 缺省值:7200(s)
		 int keepInterval = 5;// 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
		 int keepCount = 2;   // 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
		 
		 int fd = m_uv_handle.io_watcher.fd;
		 setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
		 setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&keepIdle, sizeof(keepIdle));
		 setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
		 setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));
		 */
		m_enable_keep_alive = enable;
		m_keep_idle = keep_idle ? uint(keep_idle / 1000000): 7200;
		if ( m_uv_tcp && uv_is_active((uv_handle_t*)m_uv_tcp) ) {
			uv_tcp_keepalive(m_uv_tcp, m_enable_keep_alive, m_keep_idle);
		}
	}
	
	void set_no_delay(bool no_delay) {
		m_no_delay = no_delay;
		if ( m_uv_tcp && uv_is_active((uv_handle_t*)m_uv_tcp) ) {
			uv_tcp_nodelay(m_uv_tcp, m_no_delay);
		}
	}
	
	void set_timeout(uint64 timeout) {
		m_timeout = timeout;
		reset_timeout();
	}
	
	void pause() {
		m_is_pause = true;
		if ( m_is_open ) {
			uv_read_stop((uv_stream_t*)m_uv_tcp);
		}
	}
	
	void resume() {
		if ( m_is_open ) {
			if ( m_is_pause ) {
				m_is_pause = false;
				start_read();
			}
		} else {
			m_is_pause = false;
		}
	}
	
	void write(Buffer buffer, int64 size, int mark) {
		if ( size < 0 ) {
			size = buffer.length();
		}
		if ( m_is_open ) {
			if ( uv_is_writable((uv_stream_t*)m_uv_tcp) ) {
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
	
	void open2() {
		XX_ASSERT(m_is_opening == false);
		XX_ASSERT(m_uv_handle == nullptr);
		
		if ( m_remote_ip.is_empty() ) {
			sockaddr_in sockaddr;
			sockaddr_in6 sockaddr6;
			char dst[64];
			
			if ( uv_ip4_addr(*m_hostname, m_port, &sockaddr) == 0 ) {
				m_address = *((struct sockaddr*)&sockaddr);
				// uv_ip4_name(&sockaddr, dst, 64); m_remote_ip = dst;
				m_remote_ip = m_hostname;
			}
			
			else if ( uv_ip6_addr(*m_hostname, m_port, &sockaddr6) == 0 ) {
				m_address = *((struct sockaddr*)&sockaddr6);
				// uv_ip6_name(&sockaddr6, dst, 64); m_remote_ip = dst;
				m_remote_ip = m_hostname;
			}
			
			else {
				hostent* host = gethostbyname(*m_hostname);
				if ( host ) {
					if ( host->h_addrtype == AF_INET ) { // ipv4
						memset(&sockaddr, 0, sizeof(sockaddr_in));
						//sockaddr.sin_len = host->h_length;
						sockaddr.sin_family = host->h_addrtype;
						sockaddr.sin_port = htons(m_port);
						sockaddr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
						m_address = *((struct sockaddr*)&sockaddr);
						uv_ip4_name(&sockaddr, dst, 64); m_remote_ip = dst;
					} else if ( host->h_addrtype == AF_INET6 ) { // ipv6
						memset(&sockaddr6, 0, sizeof(sockaddr_in6));
						//sockaddr6.sin6_len = host->h_length;
						sockaddr6.sin6_family = host->h_addrtype;
						sockaddr6.sin6_port = htons(m_port);
						sockaddr6.sin6_addr = *((struct in6_addr *)host->h_addr_list[0]);
						m_address = *((struct sockaddr*)&sockaddr6);
						uv_ip6_name(&sockaddr6, dst, 64); m_remote_ip = dst;
					} else {
						report_err(Error(ERR_PARSE_HOSTNAME_ERROR, "Parse hostname error `%s`", *m_hostname),1);
						return;
					}
				} else {
					report_err(Error(ERR_PARSE_HOSTNAME_ERROR, "Parse hostname error `%s`", *m_hostname),1);
					return;
				}
			}
		}
		
		if ( !m_remote_ip.is_empty() ) {
			m_uv_handle = new UVHandle(this, uv_loop());
			XX_ASSERT(m_uv_tcp == nullptr);
			XX_ASSERT(m_uv_timer == nullptr);
			m_uv_tcp = &m_uv_handle->uv_tcp;
			m_uv_timer = &m_uv_handle->uv_timer;
			auto req = new SocketConReq(this);
			int r = uv_tcp_connect(req->req(), m_uv_tcp, &m_address, &open_cb);
			if (report_uv_err(r)) {
				Release(req);
				close2();
			} else {
				m_is_opening = true;
			}
		}
	}
	
	void close2() {
		XX_ASSERT(m_uv_handle);
		uv_close((uv_handle_t*)m_uv_tcp, [](uv_handle_t* handle){
			Handle<UVHandle> h((UVHandle*)handle->data);
		});
		uv_timer_stop(m_uv_timer);
		uv_close((uv_handle_t*)m_uv_timer, nullptr);
		
		m_uv_handle = nullptr;
		m_uv_tcp = nullptr;
		m_uv_timer = nullptr;
		m_is_pause = false;
		if (m_is_open) {
			m_is_opening = false;
			m_is_open = false;
			m_delegate->trigger_socket_close(m_host);
		} else if (m_is_opening) {
			m_is_opening = false;
			Error err(ERR_CONNECT_UNEXPECTED_SHUTDOWN, "Connect unexpected shutdown");
			report_err(err);
		}
	}
	
	virtual void trigger_socket_connect_open() {
		m_is_open = true;
		if ( !m_is_pause ) {
			start_read(); // start receive data
		}
		reset_timeout();
		m_delegate->trigger_socket_open(m_host);
	}
	
	static void open_cb(uv_connect_t* uv_req, int status) {
		Handle<SocketConReq> req = SocketConReq::cast(uv_req);
		Inl* self = req->ctx();
		XX_ASSERT(self->m_is_opening);
		XX_ASSERT(!self->m_is_open);
		
		uv_tcp_keepalive(self->m_uv_tcp, self->m_enable_keep_alive, self->m_keep_idle);
		uv_tcp_nodelay(self->m_uv_tcp, self->m_no_delay);
		
		if ( status ) {
			self->m_is_opening = false;
			self->report_uv_err(status);
			self->close2();
		} else {
			self->trigger_socket_connect_open();
		}
	}
	
	static void shutdown_cb(uv_shutdown_t* uv_req, int status) {
		Handle<SocketShutdownReq> req(SocketShutdownReq::cast(uv_req));
		Inl* self = req->ctx();
		if ( status != 0 && status != UV_ECANCELED ) {
			// close status is send error
			self->report_uv_err(status);
		}
		self->close2();
	}
	
	virtual void shutdown() {
		auto req = new SocketShutdownReq(this);
		int r = uv_shutdown(req->req(), (uv_stream_t*)m_uv_tcp, &shutdown_cb);
		if ( report_uv_err(r) ) {
			Release(req);
		}
	}
	
	static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
		(static_cast<UVHandle*>(stream->data)->host)->trigger_socket_data(int(nread), buf->base);
	}
	
	void start_read() {
		uv_read_start((uv_stream_t*)m_uv_tcp, &read_alloc_cb, &read_cb);
	}
	
	static void read_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
		Inl* self = static_cast<UVHandle*>(handle->data)->host;
		if ( self->m_read_buffer.is_null() ) {
			self->m_read_buffer = Buffer( XX_MIN(65536, uint(suggested_size)) );
		}
		buf->base = *self->m_read_buffer;
		buf->len = self->m_read_buffer.length();
	}
	
	virtual void trigger_socket_data(int nread, char* buffer) {
		XX_ASSERT( m_is_open );
		if ( nread < 0 ) {
			if ( nread != UV_EOF ) { // 异常断开
				report_uv_err(int(nread));
			}
			close2();
		} else {
			reset_timeout();
			WeakBuffer buff(buffer, nread);
			m_delegate->trigger_socket_data(m_host, buff);
		}
	}
	
	static void write_cb(uv_write_t* uv_req, int status) {
		SocketWriteReq* req = SocketWriteReq::cast(uv_req);
		Handle<SocketWriteReq> handle(req);
		if ( status < 0 ) {
			req->ctx()->report_uv_err(status);
		} else {
			req->ctx()->m_delegate->trigger_socket_write(req->ctx()->m_host,
																									 req->data().raw_buffer,
																									 req->data().mark);
		}
	}
	
	virtual void write(Buffer& buffer, int mark) {
		auto req = new SocketWriteReq(this, 0, { buffer, mark });
		uv_buf_t buf;
		buf.base = req->data().raw_buffer.value();
		buf.len = req->data().raw_buffer.length();
		int r = uv_write(req->req(), (uv_stream_t*)m_uv_tcp, &buf, 1, &write_cb);
		if ( report_uv_err(r) ) {
			Release(req);
		}
	}
	
	// ----------------------------------------------------------------------
	Socket*     m_host;
	Delegate*   m_delegate;
	KeepLoop*   m_keep;
	UVHandle*   m_uv_handle;
	bool        m_is_open;
	bool        m_is_opening;
	bool        m_is_pause;
	bool        m_enable_keep_alive;
	bool        m_no_delay;
	uint        m_keep_idle;
	String      m_hostname;
	uint16      m_port;
	uv_tcp_t*   m_uv_tcp;
	uv_timer_t* m_uv_timer;
	sockaddr    m_address;
	String      m_remote_ip;
	Buffer      m_read_buffer;
	uint64      m_timeout;
};

/**
 * @class SSL_INL
 */
class SSL_INL: public Socket::Inl {
 public:
	
	static bool copy_ssl_cert_file(cString& path, cString& copy, String* out, cchar* err_msg) {
		String str = f_reader()->format(path);
		if ( Path::is_local_zip(str) ) {
			String copy_path = Path::temp(copy);
			if ( !FileHelper::exists_sync(copy_path) ) {
				try {
					if ( FileHelper::write_file_sync(copy_path, f_reader()->read_file_sync(str)) <= 0 ) {
						return false;
					}
				} catch (Error& err) {
					XX_ERR("%s, %s", err_msg, *err.message()); return false;
				}
			}
			*out = copy_path;
		} else {
			if ( FileHelper::exists_sync(str) ) {
				*out = str;
			} else {
				XX_ERR("%s, path does not exist", err_msg); return false;
			}
		}
		return true;
	}
	
	static void set_ssl_cacert_file(cString& path) {
		if ( copy_ssl_cert_file(path, "cacert.pem", ssl_cacert_file_path, "Set cacert file fail") ) {
			
			if ( !ssl_x509_store ) {
				ssl_x509_store = X509_STORE_new();
			}
			
			cchar* ca = Path::fallback_c(*ssl_cacert_file_path);
			int r = X509_STORE_load_locations(ssl_x509_store, ca, nullptr);
			
			// XX_DEBUG("ssl load x509 store, %s"r);
			
			if ( ssl_v23_client_ctx ) {
				SSL_CTX_set_cert_store(ssl_v23_client_ctx, ssl_x509_store);
			}
		}
	}
	
	static void set_ssl_client_key_file(cString& path) {
		if ( copy_ssl_cert_file(path, "key.pem", ssl_client_key_file_path, "Set ssl client key file fail") ) { }
	}

	static void initializ_ssl() {
		if ( ! ssl_initializ++ ) {
			// Initialize ssl libraries and error messages
			SSL_load_error_strings();
			SSL_library_init();
			//OpenSSL_add_all_algorithms();
			
			ssl_v23_client_ctx = SSL_CTX_new( SSLv23_client_method() );
			SSL_CTX_set_verify(ssl_v23_client_ctx, SSL_VERIFY_PEER, NULL);
			if (!ssl_x509_store) {
				if (new_root_cert_store) {
					ssl_x509_store = new_root_cert_store();
					SSL_CTX_set_cert_store(ssl_v23_client_ctx, ssl_x509_store);
				} else {
					set_ssl_cacert_file(Path::resources("cacert.pem"));
				}
			}
		}
	}
	
	static void ssl_info_callback(const SSL* ssl, int where, int ret) {
		if ( where & SSL_CB_HANDSHAKE_START ) { /*LOG("----------------start");*/ }
		if ( where & SSL_CB_HANDSHAKE_DONE ) { /* LOG("----------------done"); */ }
	}
	
	static int bio_puts(BIO *bp, const char *str) {
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
	
	/**
	 * @constructor
	 */
	SSL_INL(Socket* host, RunLoop* loop)
	: Inl(host, loop)
	, m_bio_read_source_buffer(nullptr)
	, m_bio_read_source_buffer_length(0)
	, m_ssl_handshake(0), m_ssl_write_req(nullptr) {
		
		initializ_ssl();
		
		m_ssl = SSL_new(ssl_v23_client_ctx);
		
		SSL_set_app_data(m_ssl, this);
		SSL_set_info_callback(m_ssl, &ssl_info_callback);
		SSL_set_verify(m_ssl, SSL_VERIFY_PEER, nullptr);
		
		BIO* bio = BIO_new(&bio_method);
		bio->ptr = this;
		BIO_set_fd(bio, 0, BIO_NOCLOSE);
		SSL_set_bio(m_ssl, bio, bio);
	}
	
	virtual ~SSL_INL() {
		SSL_free(m_ssl);
	}
	
	void disable_ssl_verify(bool disable) {
		if ( disable ) {
			SSL_set_verify(m_ssl, SSL_VERIFY_NONE, nullptr);
		} else {
			SSL_set_verify(m_ssl, SSL_VERIFY_PEER, nullptr);
		}
	}
	
	virtual void shutdown() {
		SSL_shutdown(m_ssl);
		Inl::shutdown();
	}
	
	static void trigger_socket_write_from_loop(CbD& evt, SSLSocketWriteReq* req) {
		Handle<SSLSocketWriteReq> req_(req);
		req->ctx()->m_delegate->trigger_socket_write(req->ctx()->m_host, req->data().raw_buffer,
																								 req->data().mark);
	}
	
	static void ssl_write_cb(uv_write_t* req, int status) {
		SSLSocketWriteReq* req_ = SSLSocketWriteReq::cast(req);
		XX_ASSERT(req_->data().buffers_count);
		
		req_->data().buffers_count--;
		
		if ( status < 0 ) {
			req_->data().error++;
			if ( req_->data().buffers_count == 0 ) {
				Release(req_);
			}
			req_->ctx()->report_uv_err(status);
			
		} else {
			if ( req_->data().buffers_count == 0 ) {
				if ( req_->data().error == 0 ) {
					async_callback(Cb(&SSL_INL::trigger_socket_write_from_loop, req_));
				} else {
					Release(req_);
				}
			}
		}
	}
	
	static void ssl_handshake_write_cb(uv_write_t* req, int status) {
		Handle<SocketWriteReq> req_(SocketWriteReq::cast(req));
		if ( status < 0 ) {
			// send handshake msg fail
			req_->ctx()->close();
		}
	}
	
	static void ssl_other_write_cb(uv_write_t* uv_req, int status) {
		Handle<SocketWriteReq> req(SocketWriteReq::cast(uv_req));
		// Do nothing
	}
	
	static int bio_write(BIO* b, cchar* in, int inl) {
		SSL_INL* self = ((SSL_INL*)b->ptr);
		XX_ASSERT( self->m_ssl_handshake );
		
		int r;
		
		Buffer buffer = WeakBuffer(in, inl).copy();
		
		if ( self->m_ssl_handshake == 1 ) { // handshake or SSL_shutdown
			
			auto req = new SocketWriteReq(self, 0, { buffer });
			uv_buf_t buf;
			buf.base = req->data().raw_buffer.value();
			buf.len = req->data().raw_buffer.length();
			
			r = uv_write(req->req(), (uv_stream_t*)self->m_uv_tcp, &buf, 1, &ssl_handshake_write_cb);
			
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
			
			if ( self->m_ssl_write_req ) { // send msg
				
				auto req = self->m_ssl_write_req;
				XX_ASSERT( req->data().buffers_count < 2 );
				
				uv_buf_t buf;
				buf.base = buffer.value();
				buf.len = inl;
				
				req->data().buffers[req->data().buffers_count] = buffer;
				
				r = uv_write(req->req(), (uv_stream_t*)self->m_uv_tcp, &buf, 1, &ssl_write_cb);
				
				if ( self->report_uv_err(r) ) {
					// uv err
				} else {
					req->data().buffers_count++;
				}
				
			} else { // SSL_shutdown or ssl other
				
				auto req = new SocketWriteReq(self, 0, { buffer });
				uv_buf_t buf;
				buf.base = req->data().raw_buffer.value();
				buf.len = req->data().raw_buffer.length();
				
				r = uv_write(req->req(), (uv_stream_t*)self->m_uv_tcp, &buf, 1, &ssl_other_write_cb);
				
				if ( r != 0 ) {
					Release(req);
				}
			}
			
			return inl;
		}
	}
	
	static int bio_read(BIO *b, char* out, int outl) {
		XX_ASSERT(out);
		SSL_INL* self = ((SSL_INL*)b->ptr);
		
		int ret = XX_MIN(outl, self->m_bio_read_source_buffer_length);
		if ( ret > 0 ) {
			memcpy(out, self->m_bio_read_source_buffer, ret);
			self->m_bio_read_source_buffer += ret;
			self->m_bio_read_source_buffer_length -= ret;
		}
		BIO_clear_retry_flags(b);
		
		return ret;
	}
	
	static int receive_ssl_err(const char *str, size_t len, void *u) {
		SSL_INL* self = (SSL_INL*)u;
		self->m_ssl_error_msg.push(str, uint(len));
		return 1;
	}
	
	void report_ssl_err(int code) {
		m_ssl_error_msg = String();
		ERR_print_errors_cb(&receive_ssl_err, this);
		report_err(Error(code, m_ssl_error_msg));
	}
	
	void ssl_handshake_fail() {
		report_err(Error(ERR_SSL_HANDSHAKE_FAIL, "ssl handshake fail"));
		close();
	}
	
	static void ssl_handshake_timeout_cb(uv_timer_t* handle) {
		SSL_INL* self = static_cast<SSL_INL*>(static_cast<UVHandle*>(handle->data)->host);
		self->ssl_handshake_fail();
	}
	
	void set_ssl_handshake_timeout() {
		XX_ASSERT(m_uv_handle);
		uv_timer_stop(m_uv_timer);
		uv_timer_start(m_uv_timer, &ssl_handshake_timeout_cb, 1e7, 0); // 10s handshake timeout
	}
	
	virtual void trigger_socket_connect_open() {
		XX_ASSERT( !m_ssl_handshake );
		set_ssl_handshake_timeout();
		m_bio_read_source_buffer_length = 0;
		m_ssl_handshake = 1;
		start_read();
		SSL_set_connect_state(m_ssl);
		if ( SSL_connect(m_ssl) < 0 ) {
			ssl_handshake_fail();
		}
	}
	
	virtual void trigger_socket_data(int nread, char* buffer) {
		
		if ( nread < 0 ) {
			
			if ( m_ssl_handshake == 0 ) { //
				report_err(Error(ERR_SSL_HANDSHAKE_FAIL, "ssl handshake fail"));
			} else {
				if ( nread != UV_EOF ) { // 异常断开
					report_uv_err(int(nread));
				}
			}
			close2();
		} else {
			
			XX_ASSERT( m_bio_read_source_buffer_length == 0 );
			
			m_bio_read_source_buffer = buffer;
			m_bio_read_source_buffer_length = nread;
			
			if ( !m_ssl_read_buffer.length() ) {
				m_ssl_read_buffer = Buffer(65536);
			}
			
			if ( m_is_open ) {
				reset_timeout();
				
				while (1) {
					int i = SSL_read(m_ssl, m_ssl_read_buffer.value(), 65536);
					
					if ( i > 0 ) {
						WeakBuffer buff(m_ssl_read_buffer.value(), i);
						m_delegate->trigger_socket_data(m_host, buff);
					} else {
						if ( i < 0 ) { // err
							report_ssl_err(ERR_SSL_UNKNOWN_ERROR); close(); // close connect
						}
						break;
					}
				}
			} else { // ssl handshake
				XX_ASSERT(m_ssl_handshake == 1);
				
				int r = SSL_connect(m_ssl);
				
				if ( r < 0 ) {
					ssl_handshake_fail();
				}
				else if ( r == 1 ) {
					m_ssl_handshake = 2; // ssl handshake done
					m_is_open = true;
					
					if ( m_is_pause ) {
						uv_read_stop((uv_stream_t*)m_uv_tcp); // pause status
					}
					reset_timeout();
					m_delegate->trigger_socket_open(m_host);
					
					XX_ASSERT( m_bio_read_source_buffer_length == 0 );
				}
			}
			
		} // if ( nread < 0 ) end
	}
	
	virtual void write(Buffer& buffer, int mark) {
		XX_ASSERT(!m_ssl_write_req);
		
		auto req = new SSLSocketWriteReq(this, 0, { buffer, mark, 0, 0 });
		m_ssl_write_req = req;
		
		int r = SSL_write(m_ssl, req->data().raw_buffer.value(), req->data().raw_buffer.length());
		
		m_ssl_write_req = nullptr;
		
		if ( r < 0 ) {
			report_ssl_err(ERR_SSL_UNKNOWN_ERROR);
		}
		if ( req->data().buffers_count == 0 ) {
			Release(req);
		}
	}
	
 private:
	
	SSL*    m_ssl;
	cchar*  m_bio_read_source_buffer;
	int     m_bio_read_source_buffer_length;
	Buffer  m_ssl_read_buffer;
	String  m_ssl_error_msg;
	int     m_ssl_handshake;
	SSLSocketWriteReq* m_ssl_write_req;
};

BIO_METHOD SSL_INL::bio_method = {
	BIO_TYPE_MEM,
	"socket",
	bio_write,
	bio_read,
	bio_puts,
	nullptr,    /* sock_gets, */
	bio_ctrl,
	nullptr,
	nullptr,
	nullptr
};

Socket::Socket(): m_inl(nullptr) {}

Socket::Socket(cString& hostname, uint16 port, RunLoop* loop)
: m_inl( NewRetain<Inl>(this, loop) ) {
	m_inl->initialize(hostname, port);
}

Socket::~Socket() {
	XX_CHECK(m_inl->m_keep->host() == RunLoop::current());
	m_inl->set_delegate(nullptr);
	if (m_inl->is_open())
		m_inl->close();
	Release(m_inl);
	m_inl = nullptr;
}
void Socket::open() {
	m_inl->open();
}
String Socket::hostname() const {
	return m_inl->m_hostname;
}
uint16 Socket::port() const {
	return m_inl->m_port;
}
String Socket::ip() const {
	return m_inl->m_remote_ip;
}
bool Socket::ipv6() const {
	return m_inl->ipv6();
}
void Socket::set_keep_alive(bool enable, uint64 keep_idle) {
	m_inl->set_keep_alive(enable, keep_idle);
}
void Socket::set_no_delay(bool no_delay) {
	m_inl->set_no_delay(no_delay);
}
void Socket::set_timeout(uint64 timeout) {
	m_inl->set_timeout(timeout);
}
void Socket::set_delegate(Delegate* delegate) {
	m_inl->set_delegate(delegate);
}
void Socket::close() {
	m_inl->close();
}
bool Socket::is_open() {
	return m_inl->is_open();
}
bool Socket::is_pause() {
	return m_inl->is_pause();
}
void Socket::pause() {
	m_inl->pause();
}
void Socket::resume() {
	m_inl->resume();
}
void Socket::write(Buffer buffer, int mark) {
	uint size = buffer.length();
	m_inl->write(buffer, size, mark);
}

XX_EXPORT void set_ssl_root_x509_store_function(X509_STORE* (*func)()) {
	XX_CHECK(func);
	new_root_cert_store = func;
}

/*
static void set_ssl_cacert_file(cString& path) {
	SSL_INL::set_ssl_cacert_file(path);
}

static void set_ssl_client_key_file(cString& path) {
	SSL_INL::set_ssl_client_key_file(path);
}

static void set_ssl_client_keypasswd(cString& passwd) {
	*ssl_client_keypasswd = passwd;
}*/

SSLSocket::SSLSocket(cString& hostname, uint16 port, RunLoop* loop) {
	m_inl = NewRetain<SSL_INL>(this, loop);
	m_inl->initialize(hostname, port);
}

void SSLSocket::disable_ssl_verify(bool disable) {
	static_cast<SSL_INL*>(m_inl)->disable_ssl_verify(disable);
}

XX_END
