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
var Conversation = require('./conv').Conversation;
var crypto = require('crypto');
var Buffer = require('buffer').Buffer;

//var TIMEOUT = 7e4;
var TIMEOUT2 = 5e4;

/**
 * @class Parser
 */
var Parser = util.class('Parser', {
  // @public:
  
	buffer: '',
	i: 0,
  
	constructor: function () {
		event.init_events(this, 'close', 'data', 'error');
	},
  
	add: function (data) {
		this.buffer += data;
		this.parse();
	},
  
	parse: function () {
		for (var i = this.i, chr; i < this.buffer.length; i++) {
			chr = this.buffer[i];
      
			if (this.buffer.length == 2 && this.buffer[1] == '\u0000') {
				this.onclose.trigger();
				this.buffer = '';
				this.i = 0;
				return;
			}
      
			if (i === 0) {
				if (chr != '\u0000')
					this.error('Bad framing. Expected null byte as first frame');
				else
					continue;
			}
      
			if (chr == '\ufffd') {
				var buffer = this.buffer.substr(1, i - 1);
				if (buffer[0] != '\ufffb' && buffer[1] != '\ubfff')
					this.ondata.trigger(buffer);
          
				this.buffer = this.buffer.substr(i + 1);
				this.i = 0;
				return this.parse();
			}
		}
	},
  
	error: function (reason) {
		this.buffer = '';
		this.i = 0;
		this.onerror.trigger(reason);
		return this;
	},
	// @end
});

// 初次握手
function handshakes (self, req, socket, head, parser) {
  
	var key1 = req.headers['sec-websocket-key1'];
	var key2 = req.headers['sec-websocket-key2'];
	var origin = req.headers.origin;
	var location = (socket.encrypted ? 'wss' : 'ws') + '://' + req.headers.host + req.url;
	var upgrade = req.headers.upgrade;
	var headers;
	var encoding;
  
	if (!upgrade || upgrade.toLowerCase() !== 'websocket') {
		console.error('connection invalid');
		self.close();
		return false;
	}
  
	if (!self.verify_origin(origin)) {
		console.error('connection invalid: origin mismatch');
		self.close();
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
				'Sec-WebSocket-Origin: ' + origin,
				'Sec-WebSocket-Location: ' + location
			];

			var protocol = req.headers['sec-websocket-protocol'];
			if (protocol)
				headers.push('Sec-WebSocket-Protocol: ' + protocol);

			headers.push('', md5.digest('binary'));
			encoding = 'binary';
		}
		else {
			self.close();
			return false;
		}
	}
	else {
		headers = [
			'HTTP/1.1 101 Web Socket Protocol Handshake',
			'Upgrade: WebSocket',
			'Connection: Upgrade',
			'WebSocket-Origin: ' + origin,
			'WebSocket-Location: ' + location,
			'',
			''
		];
		encoding = 'utf8';
	}
  
	try {
		socket.write(headers.join('\r\n'), encoding);
	}
	catch (e) {
		console.error(e);
		self.close();
		return false;
	}
	return true;
}

/**
 * @class Early
 * @bases conv::Conversation
 */
exports.Early = util.class('Early', Conversation, {
  // @private:
	m_socket: null,
	m_head: null,
	
  // @public:
	/**
	 * @constructor
	 * @arg req {http.ServerRequest} 
	 * @arg upgradeHead {Buffer}             
   * @arg bind_services_name {String}             
	 */
	constructor: function (req, upgradeHead, bind_services_name) {
		Conversation.call(this, req, bind_services_name);
		this.m_socket = req.socket;
		this.m_head = upgradeHead;
	},
  
  /**
   * @overwrite
   */
	init: function () {
    
		var self = this;
		var socket = this.m_socket;
		var parser = new Parser();
    
		if (!handshakes(this, this.request, socket, this.m_head, parser)){
			return;
		}
    
		socket.setTimeout(0);
		//socket.setNoDelay(true);
		socket.setKeepAlive(true, TIMEOUT2);
		socket.setEncoding('utf8');
    
		socket.on('timeout', function () {
			//console.log('websocket timeout close');
			self.close();
		});
    
		socket.on('end', function () {
			//console.log('websocket end close');
			self.close();
		});
    
		socket.on('close', function () {
			//console.log('websocket close');
			self.close();
		});
    
		socket.on('error', function (e) {
			console.error(e);
			self.onerror.trigger(e);
			self.close();
			self.m_socket.destroy();
		});
    
		socket.on('data', parser.add.bind(parser));
    
    parser.ondata.on(function (evt){
      self.parse(evt.data);
    });

		parser.onclose.on(function () {
			//console.log('websocket parser close');
			self.close();
		});
    
		parser.onerror.on(function (e) {
			var data = e.data;
			console.error(data);
			self.onerror.trigger(data);
			self.close();
		});
	},
  
  /**
   * @overwrite
   */
	close: function () {
	  
		if (this.is_open) {
			var socket = this.m_socket;
			socket.removeAllListeners('end');
			socket.removeAllListeners('close');
			socket.removeAllListeners('error');
			socket.removeAllListeners('data');
			socket.end();
			this.onclose.trigger();
		}
	},
	
  /**
   * @overwrite
   */
	send: function (msg) {
    
		if (!this.is_open) {
		  throw new Error('error connection close status');
		}
		
		msg = JSON.stringify(msg);

		var length = Buffer.byteLength(msg);
		var buffer = new Buffer(2 + length);

		buffer.write('\x00', 'binary');
		buffer.write(msg, 1, 'utf8');
		buffer.write('\xff', 1 + length, 'binary');

		try {
			this.m_socket.write(buffer);
		}
		catch (e) {
			this.close();
		}
	},
	// @end
});

