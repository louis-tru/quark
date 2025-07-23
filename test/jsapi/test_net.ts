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

import { LOG, Pv, Mv, Mvcb } from './tool'
import {Socket} from 'quark/net'
import {Event} from 'quark/event'
import {toString} from 'quark/buffer'

class MySocket extends Socket {
	constructor() {
		super("www.baidu.com", 443, true);
		this.onOpen.on(this.trigger_socket_open, this);
		this.onClose.on(this.trigger_socket_close, this);
		this.onError.on(this.trigger_socket_error, this);
		this.onData.on(this.trigger_socket_data, this);
		this.onWrite.on(this.trigger_socket_write, this);
		this.onTimeout.on(this.trigger_socket_timeout, this);

		this.connect();
	}

	send_http() {
		var header =
		"GET / HTTP/1.1\r\n"+
		"Host: www.baidu.com\r\n"+
		"_Connection: keep-alive\r\n"+
		"Connection: close\r\n"+
		"Accept: */*\r\n"+
		"User-Agent: Mozilla/5.0 AppleWebKit quark Net Test\r\n\r\n";

		this.write(header);
	}

	trigger_socket_open(e: Event<Socket>) {
		LOG("Open Socket");
		this.send_http();
	}
	trigger_socket_close(e: Event<Socket>) {
		LOG("Close Socket");
		// Release(this);
	}
	trigger_socket_error(e: Event<Socket, Error>) {
		LOG("Error, %d, %s", e.data.errno, e.data.error.message);
	}
	trigger_socket_data(e: Event<Socket, Uint8Array>) {
		LOG(toString(e.data));
		// LOG("DATA.., %d", e.data.length);
	}
	trigger_socket_write(e: Event<Socket, Int>) {
		LOG("Write, OK");
	}
	trigger_socket_timeout(e: Event<Socket>) {
		LOG("Timeout Socket");
		this.close();
	}
}

export default async function(_: any) {
	return new Promise<void>(function(r,j) {
		var so = new MySocket();
		so.onClose.on(()=>{
			r();
		});
		so.onError.on(e=>{
			j(e.data);
		});
	});
}
