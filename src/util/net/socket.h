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

// @private head

#ifndef __quark__util__net__socket__
#define __quark__util__net__socket__

#include "../net.h"
#include "../uv.h"

namespace qk {

	struct SocketWriteReqData {
		Buffer  raw_buffer;
		int32_t flag;
	};
	struct SSLSocketWriteReqData {
		Buffer     raw_buffer;
		int32_t    flag;
		uint32_t   buffers_count;
		int32_t    error;
		Buffer     buffers[2];
	};
	typedef UVRequestWrap<uv_connect_t, Socket::Impl> SocketConReq;
	typedef UVRequestWrap<uv_shutdown_t, Socket::Impl> SocketShutdownReq;
	typedef UVRequestWrap<uv_write_t, Socket::Impl, SocketWriteReqData, Buffer> SocketWriteReq;
	typedef UVRequestWrap<uv_write_t, Socket::Impl, SSLSocketWriteReqData, Buffer> SSLSocketWriteReq;

	// ----------------------------------------------------------------------

	class Socket::Impl: public Reference, public Socket::Delegate {
	public:
		void trigger_socket_open(Socket* stream) override {}
		void trigger_socket_close(Socket* stream) override {}
		void trigger_socket_error(Socket* stream, cError& error) override {}
		void trigger_socket_data(Socket* stream, cBuffer& buffer) override {}
		void trigger_socket_write(Socket* stream, Buffer& buffer, int flag) override {}
		void trigger_socket_timeout(Socket* socket) override {}

		struct RetainRef {
			RetainRef(Impl* hold, uv_loop_t* loop): hold(hold) {
				Qk_ASSERT_EQ(0, uv_tcp_init(loop, &tcp));
				Qk_ASSERT_EQ(0, uv_timer_init(loop, &timer));
				tcp.data = this;
				timer.data = this;
			}
			Sp<Impl> hold;
			uv_tcp_t tcp;
			uv_timer_t timer;
		};

		Impl(Socket* sock, RunLoop* loop);
		~Impl();

		void set_hostname(cString& hostname, uint16_t port);
		inline RunLoop* loop() { return _loop; }
		inline uv_loop_t* uv_loop() { return _loop->uv_loop(); }
		static void report_err_async(Cb::Data& evt, Impl *self);
		void report_err(Error err, bool async = false);
		int report_uv_err(int code, bool async = false, Callback<Buffer> *cb = 0);
		void report_not_open_connect_err(bool async = false, Callback<Buffer> *cb = 0);
		void reset_timeout();
		// --------------------- public api ---------------------
		void set_delegate(Delegate* delegate);
		void close();
		bool ipv6();
		bool is_open();
		bool is_pause();
		void set_keep_alive(bool enable, uint64_t keep_idle);
		void set_no_delay(bool no_delay);
		void set_timeout(uint64_t timeout);
		void pause();
		void resume();
		void write(Buffer &buffer, int size, int flag, Callback<Buffer>& cb);
		void connect();
		void close_and_delete(bool async = false);
		virtual void trigger_socket_connect_open();
		virtual void shutdown();
		void start_read();
		virtual void trigger_socket_data_char(int nread, Char* buffer);
		virtual void write_data(Buffer& buffer, int size, int flag, Callback<Buffer>& cb);
		virtual void disable_ssl_verify(bool disable);
	private:
		Socket*     _shell;
		Delegate*   _delegate;
		RunLoop*    _loop;
		RetainRef*  _retain;
		bool        _is_open;
		bool        _is_connecting;
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
		friend class Socket;
		friend class SSL_Impl;
	};

	typedef Socket::Impl Impl;
	typedef Socket::Delegate Delegate;
}

#endif // __quark__util__net__socket__
