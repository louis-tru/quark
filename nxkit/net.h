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

#ifndef __ngui__utils__net__
#define __ngui__utils__net__

#include "nxkit/util.h"
#include "nxkit/string.h"
#include "nxkit/buffer.h"
#include "nxkit/loop.h"

/**
 * @ns ngui
 */

NX_NS(ngui)

/**
 * @calss Socket
 */
class NX_EXPORT Socket: public Object {
	NX_HIDDEN_ALL_COPY(Socket);
 public:
	class NX_EXPORT Delegate {
	 public:
		virtual void trigger_socket_open(Socket* socket) = 0;
		virtual void trigger_socket_close(Socket* socket) = 0;
		virtual void trigger_socket_error(Socket* socket, cError& error) = 0;
		virtual void trigger_socket_data(Socket* socket, Buffer& buffer) = 0;
		virtual void trigger_socket_write(Socket* socket, Buffer buffer, int mark) = 0;
		virtual void trigger_socket_timeout(Socket* socket) = 0;
	};
	
	Socket(cString& hostname, uint16 port, RunLoop* loop = RunLoop::current());
	
	/**
	 * @destructor
	 */
	virtual ~Socket();
	
	/**
	 * @func open
	 */
	void open();
	
	String  hostname() const;
	uint16  port() const;
	String  ip() const;
	bool    ipv6() const;
	
	/**
	 * @func set_keep_alive 如果在指定的时间(微秒)内没有任何数据交互,则进行探测
	 * @arg [enable = true] {bool}
	 * @arg [keep_idle = 0] {uint} 空闲的时间(微秒),0使用系统默认值一般为7200秒 7200 * 10e6 毫秒
	 */
	void set_keep_alive(bool enable = true, uint64 keep_idle = 0);
	
	/**
	 * @func set_no_delay 禁止Nagele算法,设置为有数据立即发送
	 */
	void set_no_delay(bool no_delay = true);
	
	/**
	 * @func set_timeout 超过指定(微妙)时间内不发送数据也没有收到数据触发事件,并不关闭连接. 0为不超时
	 */
	void set_timeout(uint64 timeout_us);
	
	/**
	 * @func set_delegate()
	 */
	virtual void set_delegate(Delegate* delegate);
	virtual void close();
	virtual bool is_open();
	virtual bool is_pause();
	virtual void pause();
	virtual void resume();
	virtual void write(Buffer buffer, int mark = 0);

	NX_DEFINE_INLINE_CLASS(Inl);

 protected:
	
	Socket();
	
	Inl* m_inl;
};

/**
 * @class SSLSocket
 */
class NX_EXPORT SSLSocket: public Socket {
 public:
	
	SSLSocket(cString& hostname, uint16 port, RunLoop* loop = RunLoop::current());
	
	/**
	 * @func disable_ssl_verify
	 */
	void disable_ssl_verify(bool disable);
	
};

NX_END
#endif
