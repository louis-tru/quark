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

#include "niutils/net.h"
#include <uv.h>

using namespace ngui;

void echo_ipv6(hostent* host) {
	LOG("addrtype, IPV6, %d", host->h_addrtype);
	
	char dst[64];
	char dst2[64];
	sockaddr_in6 addr6;
	sockaddr_in6 addr6_2;
	
	for ( int i = 0; host->h_addr_list[i]; i++ ) {
		{ //
			memset(&addr6, 0, sizeof(sockaddr_in6));
			//addr6.sin6_len = host->h_length;
			addr6.sin6_family = host->h_addrtype;
			addr6.sin6_port = htons(80);
			addr6.sin6_addr = *((struct in6_addr *)host->h_addr_list[i]);
			uv_ip6_name(&addr6, dst, 64);
		}
		{ //
			uv_inet_ntop(AF_INET6, host->h_addr_list[i], dst2, 64);
			uv_ip6_addr(dst2, 80, &addr6_2);
			uv_ip6_name(&addr6_2, dst2, 64);
		}
		LOG("address, %s, port, %d, address, %s, port, %d", dst, addr6.sin6_port, dst2, addr6_2.sin6_port);
	}
}

void echo_ipv4(hostent* host) {
	LOG("addrtype, IPV4, %d", host->h_addrtype);
	
	char dst[64];
	char dst2[64];
	sockaddr_in addr4;
	sockaddr_in addr4_2;
	
	for ( int i = 0; host->h_addr_list[i]; i++ ) {
		{ //
			memset(&addr4, 0, sizeof(sockaddr_in));
			// addr4.sin_len = host->h_length;
			addr4.sin_family = host->h_addrtype;
			addr4.sin_port = htons(80);
			addr4.sin_addr = *((struct in_addr *)host->h_addr_list[i]);
			uv_ip4_name(&addr4, dst, 64);
		}
		{ //
			uv_inet_ntop(AF_INET, host->h_addr_list[i], dst2, 64);
			uv_ip4_addr(dst2, 80, &addr4_2);
			uv_ip4_name(&addr4_2, dst2, 64);
			
		}
		LOG("address, %s, port, %d, address, %s, port, %d", dst, addr4.sin_port, dst2, addr4_2.sin_port);
	}
}

void test_net_parse_host(cString& host_str, int af) {
	hostent* host = (af == -1) ? gethostbyname(*host_str) : gethostbyname2( *host_str, af );
	
	if ( host ) {
		LOG("name, %s", host->h_name);
		if ( host->h_addrtype == AF_INET ) {
			echo_ipv4(host);
		} else if ( host->h_addrtype == AF_INET6 ) {
			echo_ipv6(host);
		} else {
			LOG("ERR");
		}
	}
}

class MySocket: public Socket, public Socket::Delegate {
 public:
	MySocket(): Socket("www.iqiyi.com", 80) {
		set_delegate(this);
		open();
		set_timeout(2e6); // 2s
	}
	
	void send_http() {
		
		String header =
		"GET / HTTP/1.1\r\n"
		"Host: www.iqiyi.com\r\n"
		"Connection: keep-alive\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/5.0 AppleWebKit ngui Net Test\r\n\r\n";
		
		write(header.collapse_buffer());
	}
	
	virtual void trigger_socket_open(Socket* stream) {
		LOG("Open Socket");
		send_http();
	}
	virtual void trigger_socket_close(Socket* stream) {
		LOG("Close Socket");
		Release(this);
		//RunLoop::current()->stop();
	}
	virtual void trigger_socket_error(Socket* stream, cError& error) {
		LOG("Error, %d, %s", error.code(), error.message().c());
	}
	virtual void trigger_socket_data(Socket* stream, Buffer& buffer) {
		// LOG( String(buffer.value(), buffer.length()) );
		LOG("DATA.., %d", buffer.length());
	}
	virtual void trigger_socket_write(Socket* stream, Buffer buffer, int mark) {
		LOG("Write, OK");
	}
	virtual void trigger_socket_timeout(Socket* socket) {
		LOG("Timeout Socket");
		close();
	}
};

void test_net(int argc, char **argv) {
	test_net_parse_host("v.qq.com", AF_INET);
	test_net_parse_host("google.com", AF_INET6);
	test_net_parse_host("google.com", -1);
	
	char dst[64];
	sockaddr_in sockaddr;
	sockaddr_in6 sockaddr6;
	
	LOG(uv_ip4_addr("192.168.1.", 80, &sockaddr));
	LOG(uv_ip4_addr("192.168.1.1", 80, &sockaddr));
	
	New<MySocket>();
	RunLoop::current()->run();
}
