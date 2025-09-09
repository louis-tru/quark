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

#ifndef __quark__util__net__
#define __quark__util__net__

#include "./util.h"
#include "./string.h"
#include "./loop.h"
#include "./stream.h"

namespace qk {

	/**
	* @calss Socket
	*/
	class Qk_EXPORT Socket: public Object {
		Qk_DISABLE_COPY(Socket);
		Qk_DEFINE_INLINE_CLASS(Inl);
	public:

		class Qk_EXPORT Delegate {
		public:
			virtual void trigger_socket_open(Socket* socket) = 0;
			virtual void trigger_socket_close(Socket* socket) = 0;
			virtual void trigger_socket_error(Socket* socket, cError& error) = 0;
			virtual void trigger_socket_data(Socket* socket, cBuffer& buffer) = 0;
			virtual void trigger_socket_write(Socket* socket, Buffer& buffer, int flag) = 0; // write ok
			virtual void trigger_socket_timeout(Socket* socket) = 0;
		};

		/**
		 * @constructor(hostname,port,isSSL,loop)
		*/
		Socket(cString& hostname, uint16_t port, bool isSSL, RunLoop* loop = RunLoop::current());

		/**
		 * @destructor
		*/
		~Socket();

		String    hostname() const; //!<
		uint16_t  port() const; //!<
		String    ip() const; //!<
		bool      ipv6() const; //!<

		/**
		* 如果在指定的时间(微秒)内没有任何数据交互,则进行探测
		* @param enable?:bool
		* @param keep_idle?:uint64_t 空闲的时间(微秒),0使用系统默认值一般为7200秒 7200 * 10e6 毫秒
		*/
		void set_keep_alive(bool enable = true, uint64_t keep_idle = 0);

		/**
		* 禁止Nagele算法,设置为有数据立即发送
		*/
		void set_no_delay(bool no_delay = true);

		/**
		* 超过指定(微妙)时间内不发送数据也没有收到数据触发事件,并不关闭连接. 0为不超时
		*/
		void set_timeout(uint64_t timeout_us);

		/**
		* set_delegate()
		*/
		void set_delegate(Delegate* delegate);

		void connect(); //!<
		void close(); //!<
		bool is_open(); //!<
		bool is_connecting(); //!<
		bool is_pause(); //!<
		void pause(); //!<
		void resume(); //!<
		void write(Buffer buffer, int flag = 0, Callback<Buffer> cb = 0); //!<

		/**
		* @method disable_ssl_verify(disable)
		*/
		void disable_ssl_verify(bool disable);

	private:
		Socket(Inl* inl);
		Inl* _inl;
	};
}
#endif
