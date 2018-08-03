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

#include "ngui/base/util.h"
#include "ngui/base/http.h"
#include "ngui/base/net.h"
#include "js.h"

#if XX_ANDROID
#include "ngui/base/os/android-log.h"
#define Console AndroidConsole
#else
#define Console Console
#endif

JS_BEGIN

#define TEST_HTTP 0

class RemoteConsole;
static RemoteConsole* r_console = nullptr;

#if TEST_HTTP

static int i = 0;

class MyHttp: public Socket, public Socket::Delegate {
public:
	
	Buffer buff;
	
	MyHttp(Buffer bf): Socket("192.168.1.111", 1026, NULL), buff(bf) {
		set_delegate(this);
		i++;
		open();
	}
	
	~MyHttp() {
		i--;
		// printf("release close, %d\n", i);
	}
	
	void send_http() {
		
		String req = String::format(
		"POST /$console/log/ HTTP/1.1\r\n"
		"Host: ngui.io\r\n"
		"Connection: Close\r\n"
		"Accept: */*\r\n"
		"Content-Length: %d\r\n"
		"User-Agent: Mozilla/5.0 AppleWebKit Avocado Net Test\r\n\r\n%s", buff.length(), *buff);
		
		write(req.copy_buffer());
	}
	
	virtual void trigger_socket_open(Socket* stream) {
		send_http();
	}
	virtual void trigger_socket_close(Socket* stream) {
		Release(this);
	}
	virtual void trigger_socket_error(Socket* stream, cError& error) {
		printf("%s\n", error.message().c());
		Release(this);
	}
	virtual void trigger_socket_data(Socket* stream, Buffer& buffer) {
		//printf("http done\n");
	}
	virtual void trigger_socket_write(Socket* stream, Buffer buffer, int mark) {
		//printf("stream_write ok\n");
	}
	virtual void trigger_socket_timeout(Socket* socket) {
		
	}
};

#endif

class RemoteConsole: public Console {
 public:
	
	RemoteConsole(cString& r_server)
	: m_remote_url(r_server) {
		XX_ASSERT(!r_console);
		r_console = this;
		m_remote_log_cb = Callback(remote_log_cb_func, this);
	}
	
	virtual ~RemoteConsole() {
		r_console = nullptr;
	}
	
	void send(cchar* tag, int type, bool line_feed, cString& log) {
		
		Buffer buff = String::format("tag=%s&type=%d&line_feed=%d&log=%s",
																 tag, type, line_feed, *URI::encode(log)).collapse_buffer();
#if TEST_HTTP
		New<MyHttp>(buff);
#else
		HttpHelper::RequestOptions opts = {
			String::format("%s/$console/log/", *m_remote_url),
			HTTP_METHOD_POST,
			Map<String, String>(),
			buff,
			String(),
			String(),
			false,
			true,
		};
		HttpHelper::request(opts, m_remote_log_cb);
#endif
	}
	
	virtual void log(cString& log) {
		Console::log(log);
		send("Log", 0, true, log);
	}
	virtual void warn(cString& log) {
		Console::warn(log);
		send("Warning", 1, true, log);
	}
	virtual void error(cString& log) {
		Console::error(log);
		send("Error", 2, true, log);
	}
	virtual void print(cString& log) {
		Console::print(log);
		send("Log", 0, false, log);
	}
	virtual void print_err(cString& log) {
		Console::print_err(log);
		send("Log", 2, false, log);
	}
	
	static void remote_log_cb_func(SimpleEvent& evt, RemoteConsole* ctx) {
		if ( evt.error ) {
			printf("remote_log, %s\n", evt.error->message().c());
		}
	}
	
	static void connect_remote_console(cString& server) {
		URI uri(server); // check
		if ( uri.type() == URI_HTTP || uri.type() == URI_HTTPS ) {
			String origin = uri.origin();
			New<RemoteConsole>(origin)->set_as_default();
		}
	}
	
 private:
	String    m_remote_url;
	Callback  m_remote_log_cb;
};

void open_rlog(cString& r_url) {
	if (!r_console) {
		RemoteConsole::connect_remote_console( r_url );
	}
}

void close_rlog() {
	if ( r_console ) {
		New<Console>()->set_as_default();
		delete r_console;
	}
}

JS_END
