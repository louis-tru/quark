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

#include "./socket.h"

namespace qk {

	Impl::Impl(Socket* shell, RunLoop* loop) 
		: _shell(shell)
		, _delegate(this)
		, _loop(loop)
		, _retain(nullptr)
		, _is_open(false)
		, _is_connecting(false)
		, _is_pause(false)
		, _enable_keep_alive(false)
		, _no_delay(false)
		, _keep_idle(7200)
		, _port(0)
		, _uv_tcp(nullptr)
		, _uv_timer(nullptr)
		, _timeout(0)
	{
		Qk_ASSERT(loop);
	}

	Impl::~Impl() {
		close_and_delete();
		Qk_ASSERT(!_is_open);
		Qk_ASSERT(!_retain);
	}

	void Impl::set_hostname(cString& hostname, uint16_t port) {
		_hostname = hostname;
		_port = port;
	}

	void Impl::report_err_async(Cb::Data& evt, Impl *self) {
		self->_delegate->trigger_socket_error(self->_shell, *evt.error);
	}

	void Impl::report_err(Error err, bool async) {
		if (async)
			async_reject(Cb(report_err_async, this), std::move(err), _loop);
		else
			_delegate->trigger_socket_error(_shell, err);
	}

	int Impl::report_uv_err(int code, bool async, Callback<Buffer> *cb) {
		if ( code != 0 ) {
			Error err(code, "%s, %s", uv_err_name(code), uv_strerror(code));
			if (cb)
				(*cb)->reject(&err);
			report_err(err, async);
		}
		return code;
	}

	void Impl::report_not_open_connect_err(bool async, Callback<Buffer> *cb) {
		Error err(ERR_NOT_OPTN_TCP_CONNECT, "not tcp connect or open connecting");
		if (cb)
			(*cb)->reject(&err);
		report_err(err, async);
	}

	void Impl::reset_timeout() {
		if ( _is_open ) {
			uv_timer_stop(_uv_timer);
			if ( _timeout && !_is_pause ) {
				uv_timer_start(_uv_timer, [](uv_timer_t* handle) {
					Impl* self = *static_cast<RetainRef*>(handle->data)->hold;
					self->_delegate->trigger_socket_timeout(self->_shell);
				}, _timeout / 1000, 0);
			}
		}
	}

	// -------------------------------- public api --------------------------------

	void Impl::set_delegate(Delegate* delegate) {
		_delegate = delegate ? delegate: this;
	}

	void Impl::close() {
		if (_is_open) {
			shutdown();
		} else if (_is_connecting) {
			_is_connecting = false;
		} else {
			report_not_open_connect_err(true);
		}
	}

	bool Impl::ipv6() {
		return _address.sa_family == AF_INET6;
	}

	bool Impl::is_open() {
		return _is_open;
	}

	bool Impl::is_pause() {
		return _is_open && _is_pause;
	}

	void Impl::set_keep_alive(bool enable, uint64_t keep_idle) {
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

	void Impl::set_no_delay(bool no_delay) {
		_no_delay = no_delay;
		if ( _uv_tcp && uv_is_active((uv_handle_t*)_uv_tcp) ) {
			uv_tcp_nodelay(_uv_tcp, _no_delay);
		}
	}

	void Impl::set_timeout(uint64_t timeout) {
		_timeout = timeout;
		reset_timeout();
	}

	void Impl::pause() {
		_is_pause = true;
		if ( _is_open ) {
			uv_read_stop((uv_stream_t*)_uv_tcp);
		}
	}

	void Impl::resume() {
		if ( _is_open ) {
			if ( _is_pause ) {
				_is_pause = false;
				start_read();
			}
		} else {
			_is_pause = false;
		}
	}

	void Impl::write(Buffer &buffer, int size, int flag, Callback<Buffer>& cb) {
		if ( _is_open ) {
			if ( uv_is_writable((uv_stream_t*)_uv_tcp) ) {
				size = size < 0 ?
					buffer.length(): Qk_Min(size, buffer.length());
				reset_timeout();
				write_data(buffer, size, flag, cb);
			} else {
				Error err(ERR_SOCKET_NOT_WRITABLE, "Socket not writable");
				cb->reject(&err);
				report_err(err, true);
			}
		} else {
			report_not_open_connect_err(true, &cb);
		}
	}

	void Impl::connect() {
		if ( _is_connecting ) {
			report_err(Error(ERR_CONNECT_ALREADY_OPEN, "Connecting opening or already opened"), true);
			return;
		}
		if ( _hostname.isEmpty() ) {
			report_err(Error(ERR_CONNECTING_HOSTNAME_INVALID, "Connecting hostname invalid"), true);
			return;
		}

		Qk_ASSERT(_retain == nullptr);

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
		Qk_ASSERT(!_remote_ip.isEmpty());

		_retain = new RetainRef(this, uv_loop());
		Qk_ASSERT(_uv_tcp == nullptr);
		Qk_ASSERT(_uv_timer == nullptr);
		_uv_tcp = &_retain->tcp;
		_uv_timer = &_retain->timer;
		auto req = new SocketConReq(this);

		int r = uv_tcp_connect(req->req(), _uv_tcp, &_address, [](uv_connect_t* uv_req, int status) {
			Sp<SocketConReq> req = SocketConReq::cast(uv_req);
			Impl* self = req->ctx();
			Qk_ASSERT(!self->_is_open);

			if (status) {
				self->_is_connecting = false;
				self->report_uv_err(status);
				self->close_and_delete();
			}
			else if (self->_is_connecting == false) {
				self->report_err(Error(ERR_CONNECTING_ALREADY_CLOSED, "Connecting already closed"));
				self->close_and_delete();
			} else {
				uv_tcp_keepalive(self->_uv_tcp, self->_enable_keep_alive, self->_keep_idle);
				uv_tcp_nodelay(self->_uv_tcp, self->_no_delay);
				self->trigger_socket_connect_open();
			}
		});

		if (report_uv_err(r, true)) {
			Release(req);
			close_and_delete(true);
		} else {
			_is_connecting = true;
		}
	}
	// ------------------------------------------------------------------------------------------

	void Impl::close_and_delete(bool async) {
		if (!_retain) {
			return;
		}
		uv_close((uv_handle_t*)_uv_tcp, [](uv_handle_t* h) {
			Sp<RetainRef> sp((RetainRef*)h->data);
		});
		uv_close((uv_handle_t*)_uv_timer, nullptr); // last call first execute
		_retain = nullptr;
		_uv_tcp = nullptr;
		_uv_timer = nullptr;
		_is_pause = false;

		if (_is_open) {
			_is_connecting = false;
			_is_open = false;
			_delegate->trigger_socket_close(_shell);
		} else if (_is_connecting) {
			_is_connecting = false;
			report_err(
				Error(ERR_CONNECTING_UNEXPECTED_SHUTDOWN, "Connecting unexpected shutdown"),
				async
			);
		}
	}

	void Impl::trigger_socket_connect_open() {
		_is_open = true;
		if ( !_is_pause ) {
			start_read(); // start receive data
		}
		reset_timeout();
		_delegate->trigger_socket_open(_shell);
	}

	void Impl::shutdown() {
		auto req = new SocketShutdownReq(this);

		int r = uv_shutdown(req->req(), (uv_stream_t*)_uv_tcp, [](uv_shutdown_t* uv_req, int status) {
			Sp<SocketShutdownReq> req(SocketShutdownReq::cast(uv_req));
			Impl* self = req->ctx();
			if ( status != 0 && status != UV_ECANCELED ) {
				// close status is send error
				self->report_uv_err(status);
			}
			self->close_and_delete();
		});

		if ( report_uv_err(r) ) {
			Release(req);
		}
	}

	void Impl::start_read() {
		uv_read_start(
			(uv_stream_t*)_uv_tcp,
			[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
				Impl* self = *static_cast<RetainRef*>(handle->data)->hold;
				if ( self->_read_buffer.isNull() ) {
					self->_read_buffer = Buffer( Qk_Min(65535, uint32_t(suggested_size)) );
				}
				buf->base = *self->_read_buffer;
				buf->len = self->_read_buffer.length();
			},
			[](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
				static_cast<RetainRef*>(stream->data)->hold->trigger_socket_data_char(int(nread), buf->base);
			}
		);
	}

	void Impl::trigger_socket_data_char(int nread, Char* buffer) {
		Qk_ASSERT( _is_open );
		if ( nread < 0 ) {
			if ( nread != UV_EOF ) { // 异常断开
				report_uv_err(int(nread));
			}
			close_and_delete();
		} else {
			reset_timeout();
			WeakBuffer buff(buffer, nread);
			_delegate->trigger_socket_data(_shell, buff.buffer());
		}
	}

	void Impl::write_data(Buffer& buffer, int size, int flag, Callback<Buffer>& cb) {
		auto req = new SocketWriteReq(this, cb, { buffer, flag });
		uv_buf_t buf;
		buf.base = *req->data().raw_buffer;
		buf.len = size;//req->data().raw_buffer.length();

		int r = uv_write(req->req(), (uv_stream_t*)_uv_tcp, &buf, 1,
		[](uv_write_t* uv_req, int status) {
			SocketWriteReq* req = SocketWriteReq::cast(uv_req);
			Sp<SocketWriteReq> handle(req);
			if ( status < 0 ) {
				req->ctx()->report_uv_err(status, false, &req->cb());
			} else {
				req->cb()->resolve(&req->data().raw_buffer);
				req->ctx()->_delegate->trigger_socket_write(req->ctx()->_shell,
																										req->data().raw_buffer,
																										req->data().flag);
			}
		});

		if ( report_uv_err(r, true, &cb) ) {
			Release(req);
		}
	}

	void Impl::disable_ssl_verify(bool disable) {}

	// ----------------------------------------------------------------------

	Socket::Socket(Impl* impl): _impl(impl) {
	}

	Socket::~Socket() {
		_impl->set_delegate(nullptr);
		if (_impl->is_open())
			_impl->close();
		_impl->release();
		_impl = nullptr;
	}
	String Socket::hostname() const {
		return _impl->_hostname;
	}
	uint16_t Socket::port() const {
		return _impl->_port;
	}
	String Socket::ip() const {
		return _impl->_remote_ip;
	}
	bool Socket::ipv6() const {
		return _impl->ipv6();
	}
	void Socket::set_keep_alive(bool enable, uint64_t keep_idle) {
		_impl->set_keep_alive(enable, keep_idle);
	}
	void Socket::set_no_delay(bool no_delay) {
		_impl->set_no_delay(no_delay);
	}
	void Socket::set_timeout(uint64_t timeout) {
		_impl->set_timeout(timeout);
	}
	void Socket::set_delegate(Delegate* delegate) {
		_impl->set_delegate(delegate);
	}
	void Socket::connect() {
		_impl->connect();
	}
	void Socket::close() {
		_impl->close();
	}
	bool Socket::is_open() {
		return _impl->is_open();
	}
	bool Socket::is_pause() {
		return _impl->is_pause();
	}
	bool Socket::is_connecting() {
		return _impl->_is_connecting;
	}
	void Socket::pause() {
		_impl->pause();
	}
	void Socket::resume() {
		_impl->resume();
	}
	void Socket::write(Buffer buffer, int flag, Callback<Buffer> cb) {
		_impl->write(buffer, -1, flag, cb);
	}

	/*
	void set_ssl_cacert_file(cString& path) {
		try {
			NewRootCertStoreFromFile(fs_read_file_sync(path));
		} catch(cError& err) {
			Qk_ELog("SSL", "NewRootCertStoreFromFile() fail, %s", err.message().c_str());
		}
	}

	void set_ssl_client_key_file(cString& path) {
	// TODO
	}

	void set_ssl_client_keypasswd(cString& passwd) {
	// TODO
	}*/

	void Socket::disable_ssl_verify(bool disable) {
		_impl->disable_ssl_verify(disable);
	}
}
