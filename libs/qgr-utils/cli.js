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
var { userAgent } = require('./request');
var net = require('net');
var http = require('http');
var https = require('https');
var { PacketParser, Hybi } = require('./hybi');
var { Notification } = require('./event');
var url = require('./url');
var { Buffer } = require('buffer');
var errno = require('./errno');
var crypto = require('crypto');

var KEEP_ALIVE_TIME = 5e4; // 50s
var METHOD_CALL_TIMEOUT = 12e4; // 120s

/**
 * @class Conversation
 */
var Conversation = util.class('Conversation', {

	// @private:
	m_connect: false, // 是否尝试连接中
	m_is_open: false, // open status
	m_clients: null, // client list
	m_socket: null, // web socket connection
	m_response: null,
	m_token: '',
	
	/**
	 * @get response
	 */
	get response() { return this.m_response },

	/**
	 * @get socket
	 */
	get socket() { return this.m_socket },

	/**
	 * @get token
	 */
	get token() { return this.m_token },

	onOpen: null,
	onMessage: null,
	onError: null,
	onClose: null,

	/**
	 * @constructor
	 */
	constructor: function() {
		event.initEvents(this, 'Open', 'Message', 'Error', 'Close');

		this.onClose.on(()=>{
			this.m_is_open = false;
			this.m_connect = false;
			this.m_socket = null;
			this.m_response = null;
			this.m_token = '';
		});
		this.onError.on((e)=>this.m_connect = false);

		this.m_clients = {};
	},
	
	/**
	 * @get isOpen # 获取是否打开连接
	 */
	get isOpen() {
		return this.m_is_open;
	},
	
	/**
	 * @fun bindClient # 绑定
	 * @arg client {Client}
	 */
	bindClient: function(client) {
		var name = client.name;
		var clients = this.m_clients;
		if (name in clients) {
			throw new Error('No need to repeat binding');
		} else {
			clients[name] = client;
			if (this.m_is_open) {
				this.send({ type: 'bind_client_service', name: name });
			}
			else {
				util.nextTick(e=>this.connect()); // 还没有打开连接,下一帧开始尝试连接
			}
		}
	},
	
	/**
	 * @get clients # 获取绑定的Client列表
	 */
	get clients() {
		return this.m_clients;
	},
	
	/**
	 * @fun connect # connercion server
	 */
	connect: function() {
		if (!this.m_is_open && !this.m_connect) {
			for (var i in this.m_clients) {
				this.m_connect = true;
				this.initialize();
				return;
			}
			// 连接必需要绑定服务才能使用
			throw new Error('connection must bind service');
		}
	},
	
	/**
	 * @fun parse # parser message
	 * @arg {Number} type    0:String|1:Buffer
	 * @arg packet {String|Buffer}
	 */
	handlePacket: function(type, packet) {
		this.onMessage.trigger({ type, data: packet });

		if (type === 0 && packet[0] == '\ufffe') { // json text
			try {
				var data = JSON.parse(packet.substr(1));
			} catch(err) {
				console.log(err); return;
			}
			var client = this.m_clients[data.service];
			if (client) {
				client.receiveMessage(data);
			} else {
				console.error('Could not find the message handler, '+
											'discarding the message, ' + data.service);
			}
		}
	},
	
	/**
	 * @fun init # init conversation
	 */
	initialize: function() {},
	
	/**
	 * @fun close # close conversation connection
	 */
	close: function() {},
	
	/**
	 * @fun send # send message to server
	 * @arg [data] {Object}
	 */
	send: function(data) {},

	/**
	 * @func ping()
	 */
	ping: function() { },

	// @end

});

// private:
function handshakes(self, res, key) {
	var accept = res.headers['sec-websocket-accept'];
	if (accept) {
		var shasum = crypto.createHash('sha1');
		shasum.update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11');
		key = shasum.digest('base64');
		return key == accept;
	}
	return false;
}

/**
 * @class WSConversation
 */
var WSConversation = util.class('WSConversation', Conversation, {

	// @private:
	m_req: null,
	m_url: null,
	m_message: null,

	/**
	 * @get url
	 */
	get url() { return this.m_url },

	/**
	 * @get token
	 */
	get token() { return this.m_token },
	
	// @public:
	/**
	 * @constructor
	 * @arg path {String} ws://192.168.1.101:8091/
	 */
	constructor: function(path) {
		Conversation.call(this);
		this.m_message = [];
		path = path || util.config.web_service || 'ws://localhost';
		util.assert(path, 'Server path is not correct');
		path = url.resolve(path);
		this.m_url = new url.URL(path.replace(/^http/, 'ws'));
	},
	
	/** 
	 * @ovrewrite 
	 */
	initialize: function() {
		util.assert(!this.m_req, 'No need to repeat open');
		// self.m_socket
		var self = this;
		var url = this.m_url;
		var bind_client_services = Object.keys(this.clients).join(',');

		url.setParam('bind_client_services', bind_client_services);

		var isSSL = url.protocol == 'wss:';
		var port = url.port || (isSSL ? 443: 80);
		var lib = isSSL ? https: http;
		var path = url.path;
		var origin = '127.0.0.1:' + port;
		var key = Date.now();

		var options = {
			hostname: url.hostname,
			port: port,
			path: path,
			headers: {
				'User-Agent': userAgent,
				'Connection': 'Upgrade',
				'Upgrade': 'websocket',
				'Origin': origin,
				'Sec-Websocket-Origin': origin,
				'Sec-Websocket-Version': 13,
				'Sec-Websocket-Key': key,
			},
			rejectUnauthorized: false
		};

		if (isSSL) {
			options.agent = new https.Agent(options);
		}

		var req = this.m_req = lib.request(options);

		req.on('upgrade', function(res, socket, upgradeHead) {
			self.m_response = res;
			self.m_socket = socket;
			self.m_token = res.headers['session-token'] || '';

			if (!handshakes(self, res, key)) {
				socket.end(); return;
			}

			var parser = new PacketParser();
			
			socket.setTimeout(0);
			socket.setKeepAlive(true, KEEP_ALIVE_TIME);
			
			socket.on('timeout', e=>self.close());
			socket.on('end', e=>self.close());
			socket.on('close', e=>self.close());
			socket.on('data', d=>parser.add(d));

			socket.on('error', function(e) {
				var socket = self.m_socket;
				self.onError.trigger(e);
				self.close();
				if (socket)
					socket.destroy();
			});

			parser.onText.on(e=>self.handlePacket(0, e.data));
			parser.onData.on(e=>self.handlePacket(1, e.data));
			parser.onClose.on(e=>self.close());

			parser.onError.on(function(e) {
				self.onError.trigger(e.data);
				self.close();
			});

			self.m_is_open = true;
			self.m_connect = false;

			var message = self.m_message;

			self.m_message = [];
			self.onOpen.trigger();

			message.forEach(m=>self.send(m));
		});

		req.on('error', function(e) {
			self.m_req = null;
			self.onError.trigger(e);
			self.close();
		});

		req.end();
	},
	
	/** 
	 * @ovrewrite 
	 */
	close: function() {
		var socket = this.m_socket;
		if (socket) {
			this.m_req = null;
			this.m_socket = null;
			socket.removeAllListeners('end');
			socket.removeAllListeners('close');
			socket.removeAllListeners('error');
			socket.removeAllListeners('data');
			if (socket.writable)
				socket.end();
			if (!this.isOpen) {
				this.onError.trigger(Error.new(errno.ERR_REQUEST_AUTH_FAIL));
			}
		}
		if (this.isOpen) {
			this.onClose.trigger();
		}
	},
	
	/**
	 * @ovrewrite
	 */
	send: function(msg) {
		if (this.isOpen) {
			try {
				if (this.m_socket) {
					Hybi.sendDataPacket(this.m_socket, msg);
				} else {
					console.error('cannot find function `this.m_socket`');
				}
			} catch (e) {
				console.error(e);
				this.close();
			}
		} else {
			this.m_message.push(msg);
			this.connect(); // 尝试连接
		}
	},

	/**
	 * @ovrewrite 
	 */
	ping: function() {
		if (this.isOpen) {
			if (this.m_socket) {
				Hybi.sendPingPacket(this.m_socket);
			} else {
				console.error('cannot find function `this.m_socket`');
			}
		} else {
			this.connect(); // 尝试连接
		}
	},
	
});

/**
 * @class Client
 */
var Client = util.class('Client', Notification, {
	// @private:
	m_callbacks: null,
	m_service_name: '',
	m_conv: null,   // conversation

	// @public:
	/**
	 * @get name
	 */
	get name() {
		return this.m_service_name;
	},

	/**
	 * @get conv
	 */	
	get conv() {
		return this.m_conv;
	},
	
	/**
	 * @constructor constructor(service_name, conv)
	 */
	constructor: function(service_name, conv) {
		this.m_callbacks = {};
		this.m_service_name = service_name;
		this.m_conv = conv || new WSConversation();

		util.assert(service_name);
		util.assert(this.m_conv);

		conv.onClose.on(e=>{
			var callbacks = this.m_callbacks;
			this.m_callbacks = {};
			var err = Error.new(errno.ERR_CONNECTION_DISCONNECTION);
			for (var i in callbacks) {
				callbacks[i].err(err);
			}
		});

		this.m_conv.bindClient(this);
	},

	/**
	 * @func receiveMessage(data)
	 */
	receiveMessage: function(data) {
		if (data.type == 'callback') {
			var cb = this.m_callbacks[data.callback];
			delete this.m_callbacks[data.callback];
			if (cb) {
				if (data.error) { // throw error
					cb.err(Error.new(data.error));
				} else {
					cb.ok(data.data);
				}
			} else {
				console.error('Unable to callback, no callback context can be found');
			}
		} else if (data.type == 'event') {
			this.trigger(data.name, data.data);
		} else {
			// TODO ...
		}
	},

	/**
	 * @func call(name, data)
	 */
	call: function(name, data, timeout) {
		timeout = typeof timeout == 'number' ? timeout : exports.METHOD_CALL_TIMEOUT;
		
		return new Promise((resolve, reject)=>{
			var id = util.id;
			var timeid = 0;
			if (timeout) {
				timeid = setTimeout(e=>{
					// console.error(`method call timeout, ${this.name}/${name}`);
					reject(Error.new([...errno.ERR_METHOD_CALL_TIMEOUT,
						`method call timeout, ${this.name}/${name}`]));
				}, timeout);
			}

			this.m_callbacks[id] = { 
				id: id, 
				ok: (e)=>{
					if (timeid)
						clearTimeout(timeid);
					resolve(e);
				},
				err: (e)=>{
					if (timeid)
						clearTimeout(timeid);
					reject(e);
				},
			};

			this.m_conv.send({ 
				service: this.name, 
				type: 'call', 
				name: name, 
				data: data, 
				callback: id,
			});
		});
	},

	/**
	 * @func send(name, data) no callback, no return data
	 */
	send: function(name, data) {
		this.m_conv.send({ 
			service: this.name,
			type: 'call', 
			name: name, 
			data: data,
		});
	},

});

exports = module.exports = {
	METHOD_CALL_TIMEOUT: METHOD_CALL_TIMEOUT,
	Conversation: Conversation,
	WSConversation: WSConversation,
	Client: Client,
};
