/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./socket.h"
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace qk {
	static int ssl_initializ = 0;
	static X509_STORE* ssl_x509_store = nullptr;
	static SSL_CTX* ssl_v23_client_ctx = nullptr;

	X509_STORE* NewRootCertStore();

	class SSL_Impl: public Impl {
	public:
		static void initializ_ssl();
		static void ssl_info_callback(const SSL* ssl, int where, int ret);
		static int bio_puts(BIO *bp, cChar *str);
		static long bio_ctrl(BIO *b, int cmd, long num, void *ptr);
		// ------------------------------------------------------------------------

		SSL_Impl(Socket* host, RunLoop* loop)
			: Impl(host, loop)
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

		~SSL_Impl() {
			SSL_free(_ssl);
		}

		void disable_ssl_verify(bool disable) override {
			if ( disable ) {
				SSL_set_verify(_ssl, SSL_VERIFY_NONE, nullptr);
			} else {
				SSL_set_verify(_ssl, SSL_VERIFY_PEER, nullptr);
			}
		}

		void shutdown() override {
			SSL_shutdown(_ssl);
			Impl::shutdown();
		}

		static void ssl_write_cb(uv_write_t* req, int status) {
			SSLSocketWriteReq* req_ = SSLSocketWriteReq::cast(req);
			SSLSocketWriteReqData &data = req_->data();
			Qk_ASSERT(data.buffers_count);

			data.buffers_count--;

			auto self = req_->ctx();

			if ( status < 0 ) {
				data.error++;
				self->report_uv_err(status, false, data.error == 1 ? &req_->cb(): 0);

				if (data.buffers_count == 0)
					Release(req_);
			} else {
				if (data.buffers_count == 0) {
					Sp<SSLSocketWriteReq> sp(req_);
					if ( data.error == 0 ) {
						req_->cb()->resolve(&data.raw_buffer);
						self->_delegate->trigger_socket_write(
							self->_shell, data.raw_buffer, data.flag
						);
					} 
				}
			}
		}

		static void ssl_handshake_write_cb(uv_write_t* req, int status) {
			Sp<SocketWriteReq> req_(SocketWriteReq::cast(req));
			if ( status < 0 ) { // send handshake msg fail
				req_->ctx()->close();
			}
		}

		static void ssl_other_write_cb(uv_write_t* uv_req, int status) {
			Sp<SocketWriteReq> req(SocketWriteReq::cast(uv_req));
			// Do nothing
		}

		static int bio_write(BIO* b, cChar* in, int inl) {
			SSL_Impl* self = ((SSL_Impl*)b->ptr);
			Qk_ASSERT( self->_ssl_handshake );

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
					Qk_ASSERT( req->data().buffers_count < 2 );

					uv_buf_t buf;
					buf.base = *buffer;
					buf.len = inl;

					req->data().buffers[req->data().buffers_count] = buffer;

					r = uv_write(req->req(), (uv_stream_t*)self->_uv_tcp, &buf, 1, &ssl_write_cb);

					if ( self->report_uv_err(r, true, &req->cb()) ) {
						return r; // uv err
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
			Qk_ASSERT(out);
			SSL_Impl* self = ((SSL_Impl*)b->ptr);
			
			int ret = Qk_Min(outl, self->_bio_read_source_buffer_length);
			if ( ret > 0 ) {
				memcpy(out, self->_bio_read_source_buffer, ret);
				self->_bio_read_source_buffer += ret;
				self->_bio_read_source_buffer_length -= ret;
			}
			BIO_clear_retry_flags(b);
			
			return ret;
		}
		
		static int receive_ssl_err(cChar *str, size_t len, void *u) {
			SSL_Impl* self = (SSL_Impl*)u;
			self->_ssl_error_msg.append(str, uint32_t(len));
			return 1;
		}
		
		void report_ssl_err(int code, bool async = false, Callback<Buffer> *cb = 0) {
			_ssl_error_msg = String();
			ERR_print_errors_cb(&receive_ssl_err, this);
			Error err(code, _ssl_error_msg);
			if (cb)
				(*cb)->reject(&err);
			report_err(err, async);
		}

		void ssl_handshake_fail() {
			report_err(Error(ERR_SSL_HANDSHAKE_FAIL, "ssl handshake fail"));
			close();
		}
		
		static void ssl_handshake_timeout_cb(uv_timer_t* handle) {
			SSL_Impl* self = static_cast<SSL_Impl*>(*static_cast<RetainRef*>(handle->data)->hold);
			self->ssl_handshake_fail();
		}
		
		void set_ssl_handshake_timeout() {
			Qk_ASSERT(_retain);
			uv_timer_stop(_uv_timer);
			uv_timer_start(_uv_timer, &ssl_handshake_timeout_cb, 1e7, 0); // 10s handshake timeout
		}
		
		void trigger_socket_connect_open() override {
			Qk_ASSERT( !_ssl_handshake );
			set_ssl_handshake_timeout();
			_bio_read_source_buffer_length = 0;
			_ssl_handshake = 1;
			start_read();
			SSL_set_connect_state(_ssl);
			if ( SSL_connect(_ssl) < 0 ) {
				ssl_handshake_fail();
			}
		}

		void trigger_socket_data_char(int nread, Char* buffer) override {
			if ( nread < 0 ) {
				if ( _ssl_handshake == 0 ) { //
					report_err(Error(ERR_SSL_HANDSHAKE_FAIL, "ssl handshake fail"));
				} else {
					if ( nread != UV_EOF ) { // 异常断开
						report_uv_err(int(nread));
					}
				}
				close_and_delete();
			} else {
				Qk_ASSERT( _bio_read_source_buffer_length == 0 );

				_bio_read_source_buffer = buffer;
				_bio_read_source_buffer_length = nread;

				if ( _ssl_read_buffer.length() == 0 ) {
					_ssl_read_buffer = Buffer(65535);
				}

				if ( _is_open ) {
					reset_timeout();

					while (1) {
						int i = SSL_read(_ssl, _ssl_read_buffer.val(), 65535);

						if ( i > 0 ) {
							WeakBuffer buff(_ssl_read_buffer.val(), i);
							_delegate->trigger_socket_data(_shell, buff.buffer());
						} else {
							if ( i < 0 ) { // err
								report_ssl_err(ERR_SSL_UNKNOWN_ERROR);
								close(); // close connect
							}
							break;
						}
					}
				} else { // ssl handshake
					Qk_ASSERT(_ssl_handshake == 1);

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
						_delegate->trigger_socket_open(_shell);

						Qk_ASSERT( _bio_read_source_buffer_length == 0 );
					}
				}
				
			} // if ( nread < 0 ) end
		}

		void write_data(Buffer& buffer, int size, int flag, Callback<Buffer>& cb) override {
			Qk_ASSERT(!_ssl_write_req);

			auto req = new SSLSocketWriteReq(this, cb, { buffer, flag, 0, 0 });
			_ssl_write_req = req;

			int r = SSL_write(_ssl, req->data().raw_buffer.val(), size/*req->data().raw_buffer.length()*/);
 
			_ssl_write_req = nullptr;

			if ( r < 0 ) {
				report_ssl_err(ERR_SSL_UNKNOWN_ERROR, true, &cb);
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
		static BIO_METHOD bio_method;
	};

	BIO_METHOD SSL_Impl::bio_method = {
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

	void SSL_Impl::initializ_ssl() {
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

	void SSL_Impl::ssl_info_callback(const SSL* ssl, int where, int ret) {
		if ( where & SSL_CB_HANDSHAKE_START ) { /*LOG("----------------start");*/ }
		if ( where & SSL_CB_HANDSHAKE_DONE ) { /* Qk_Log("----------------done"); */ }
	}

	int SSL_Impl::bio_puts(BIO *bp, cChar *str) {
		return bio_write(bp, str, int(strlen(str)));
	}

	long SSL_Impl::bio_ctrl(BIO *b, int cmd, long num, void *ptr) {
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

	Socket::Socket(cString& hostname, uint16_t port, bool isSSL, RunLoop* loop)
		: _impl(isSSL ? NewRetain<SSL_Impl>(this, loop): NewRetain<Impl>(this, loop))
	{
		_impl->set_hostname(hostname, port);
	}
}
