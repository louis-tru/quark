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

var util = require('./util');
var event = require('./event');
var crypto = require('crypto');
var { Conversation } = require('./conv');
var { Buffer } = require('buffer');
var errno = require('./errno');

var KEEP_ALIVE_TIME = 5e4;

/**
 * @class PacketParser
 */
class PacketParser {

	constructor() {
		event.initEvents(this, 'Close', 'Text', 'Error');
		this.buffer = '';
		this.i = 0;
	}
	
	add(data) {
		this.buffer += data;
		this.parse();
	}
	
	parse() {
		var len = this.buffer.length;

		for (var i = this.i; i < len; i++) {
			this.i = i;
			var ch = this.buffer[i];

			// \u0000\u0000   // close
			// \u0000\ufffd		// data
			// \u0000\ufffb\ubfff\ufffd		// ping
			
			if (len == 2 && this.buffer[1] == '\u0000') {
				this.onClose.trigger();
				this.buffer = '';
				this.i = 0;
				return;
			}
			if (i === 0) {
				if (ch != '\u0000')
					this.error('Bad framing. Expected null byte as first frame');
				else 
					continue;
			}
			if (ch == '\ufffd') {
				var buffer = this.buffer.substr(1, i - 1);
				this.onText.trigger(nbuffer);
				this.buffer = this.buffer.substr(i + 1);
				this.i = 0;
				this.parse();
				return;
			}
		}
	}
	
	error(reason) {
		this.buffer = '';
		this.i = 0;
		this.onError.trigger(reason);
		return this;
	}
	
	// @end
}

// 初次握手
function handshakes(self, req, socket, head, parser) {
	
	var key1 = req.headers['sec-websocket-key1'];
	var key2 = req.headers['sec-websocket-key2'];
	var origin = req.headers.origin || '';
	var location = (socket.encrypted ? 'wss' : 'ws') + '://' + req.headers.host + req.url;
	var upgrade = req.headers.upgrade;
	var headers;
	var encoding;
	
	if (!upgrade || upgrade.toLowerCase() !== 'websocket') {
		console.error('connection invalid');
		return false;
	}
	
	if (!self.verifyOrigin(origin)) {
		console.error('connection invalid: origin mismatch');
		return false;
	}
	
	if (key1 && key2) {
		
		if (head.length >= 8) {
			if (head.length > 8)
				parser.add(head.slice(8, head.length) + '');
				
			var num1 = parseInt(key1.match(/\d/g).join('')) / (key1.match(/\s/g).length);
			var num2 = parseInt(key2.match(/\d/g).join('')) / (key2.match(/\s/g).length);
			var md5 = crypto.createHash('md5');

			md5.update(String.fromCharCode(num1 >> 24 & 0xFF, num1 >> 16 & 0xFF, num1 >> 8 & 0xFF, num1 & 0xFF));
			md5.update(String.fromCharCode(num2 >> 24 & 0xFF, num2 >> 16 & 0xFF, num2 >> 8 & 0xFF, num2 & 0xFF));
			md5.update(head.slice(0, 8).toString('binary'));

			headers = [
				'HTTP/1.1 101 WebSocket Protocol Handshake',
				'Upgrade: WebSocket',
				'Connection: Upgrade',
				'Session-Token: ' + self.token,
				'Sec-WebSocket-Origin: ' + origin,
				'Sec-WebSocket-Location: ' + location,
			];

			var protocol = req.headers['sec-websocket-protocol'];
			if (protocol) {
				headers.push('Sec-WebSocket-Protocol: ' + protocol);
			}

			headers.push('', md5.digest('binary'));
			encoding = 'binary';
		} else {
			return false;
		}

	} else {
		headers = [
			'HTTP/1.1 101 Web Socket Protocol Handshake',
			'Upgrade: WebSocket',
			'Connection: Upgrade',
			'Session-Token: ' + self.token,
			'WebSocket-Origin: ' + origin,
			'WebSocket-Location: ' + location,
			'',
			''
		];
		encoding = 'utf8';
	}
	
	try {
		socket.write(headers.join('\r\n'), encoding);
	} catch (e) {
		console.error(e);
		return false;
	}

	return true;
}

/**
 * @class Early
 * @bases conv::Conversation
 */
class Early extends Conversation {

	constructor(req, upgradeHead, bind_services_name) {
		Conversation.call(this, req, bind_services_name);
		this.m_head = upgradeHead;
	}

	/**
	 * @overwrite
	 */
	initialize() {
		if (!handshakes(this, this.request, this.socket, this.m_head, parser)) {
			return;
		}
		var self = this;
		var socket = this.socket;
		var parser = new PacketParser();

		socket.setTimeout(0);
		socket.setKeepAlive(true, KEEP_ALIVE_TIME);
		socket.setEncoding('utf8');

		socket.on('timeout', e=>self.close());
		socket.on('end', e=>self.close());
		socket.on('close', e=>self.close());
		socket.on('data', e=>parser.add(e));
		
		socket.on('error', function(e) {
			var socket = self.socket;
			self.onError.trigger(e);
			self.close();
			if (socket)
				socket.destroy();
		});

		parser.onText.on(e=>self.handlePacket(0, e.data));
		parser.onClose.on(e=>self.close());
		
		parser.onError.on(function(e) {
			console.error(e.data);
			self.onError.trigger(e.data);
			self.close();
		});

		return true;
	}

	/**
	 * @overwrite
	 */
	send(msg) {
		// Text only.
		if (this.isOpen) {
			msg = '\ufffe' + JSON.stringify(msg);
			
			var length = Buffer.byteLength(msg);
			var buffer = new Buffer(2 + length);
			
			buffer.write('\x00', 'binary');
			buffer.write(msg, 1, 'utf8');
			buffer.write('\xff', 1 + length, 'binary');
			
			try {
				this.socket.write(buffer);
			} catch(e) {
				this.close();
			}
		} else {
			throw Error.new(errno.ERR_CONNECTION_CLOSE_STATUS);
		}
	}

	/**
	 * @overwrite
	 */
	close () {
		if (this.isOpen) {
			var socket = this.socket;
			socket.removeAllListeners('end');
			socket.removeAllListeners('close');
			socket.removeAllListeners('error');
			socket.removeAllListeners('data');
			socket.end();
			this.onClose.trigger();
		}
	}
	
}

exports.Early = Early;