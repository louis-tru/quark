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

#include "src/util/net.h"
#include <uv.h>
#include "../test.h"

using namespace qk;

class MySSLSocket: public SSLSocket, public Socket::Delegate {
 public:
	MySSLSocket(): SSLSocket("www.baidu.com", 443) {
		set_delegate(this);
		open();
		//set_timeout(2e6); // 2s
	}

	~MySSLSocket() {
		current_loop()->stop();
	}

	void send_http() {
		
		String header =
		"GET / HTTP/1.1\r\n"
		"Host: www.baidu.com\r\n"
		"_Connection: keep-alive\r\n"
		"Connection: close\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/5.0 AppleWebKit quark Net Test\r\n\r\n";
		
		write(header.collapse());
	}
	
	virtual void trigger_socket_open(Socket* stream) {
		Qk_Log("Open Socket");
		send_http();
	}
	virtual void trigger_socket_close(Socket* stream) {
		Qk_Log("Close Socket");
		Release(this);
	}
	virtual void trigger_socket_error(Socket* stream, cError& error) {
		Qk_Log("Error, %d, %s", error.code(), error.message().c_str());
	}
	virtual void trigger_socket_data(Socket* stream, cBuffer& buffer) {
		//Qk_Log( String(buffer.val(), buffer.length()) );
		Qk_Log("DATA.., %d", buffer.length());
	}
	virtual void trigger_socket_write(Socket* stream, Buffer& buffer, int mark) {
		Qk_Log("Write, OK");
	}
	virtual void trigger_socket_timeout(Socket* socket) {
		Qk_Log("Timeout Socket");
		close();
	}
};

Qk_TEST_Func(net_ssl) {
	New<MySSLSocket>();
	RunLoop::current()->run();
}
