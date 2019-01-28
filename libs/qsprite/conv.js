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
var ClientService = require('./cli_service').ClientService;
var url = require('url');
var service = require('./service');
var { Buffer } = require('buffer');
var errno = require('./errno');

// 绑定服务
function bind_services(self, services, cb) {
	
	if (!services.length) {
		cb(true);
		return;
	}
	
	var name = services.shift();
	var cls = service.get(name);
	
	if (name in self.m_services) {
		console.error('Service no need to repeat binding');
		cb(false);
		return;
	}

	if (!cls || !util.equalsClass(ClientService, cls)) {
		console.error(name + ' Service type is not correct');
		cb(false);
		return;
	}
	
	var ser = new cls(self);
	ser.name = name;
	self.m_services[name] = ser;

	function auth_done(is) { // 认证合法性
		if (is) {
			bind_services(self, services, cb); // 继续绑定
		} else {
			cb(false);
		}
	}
	
	if (util.isAsync(ser.requestAuth)) {
		ser.requestAuth(null).then(auth_done);
	} else {
		auth_done(ser.requestAuth(null));
	}
}

//Init
function initialize(self, bind_services_name) {
	var services = bind_services_name.split(',');

	if (!services[0]) { // 没有服务,这种连接没有意义
		self.socket.destroy();
		return;
	}

	bind_services(self, services, function(is) {

		if (!is) {
			self.onError.trigger(Error.new(errno.ERR_REQUEST_AUTH_FAIL));
		}

		if (is && self.initialize()) {
			util.assert(!self.m_isOpen);
			self.server.m_ws_conversations[self.token] = self;
			self.m_isOpen = true;

			self.onClose.once(e=>{
				util.assert(self.m_isOpen);
				delete self.server.m_ws_conversations[self.token];
				self.m_isOpen = false;
				self.request = null;
				self.socket = null;
				self.token = '';
				self.onOpen.off();
				self.onMessage.off();
				self.onError.off();
				self.server.emit('WSConversationClose', self);
				self.server = null;
				util.nextTick(e=>self.onClose.off());
			});

			self.onOpen.trigger();
			self.server.emit('WSConversationOpen', self);
		} else { // 绑定失败
			self.socket.destroy();  // 关闭连接
		}
	});
}

/**
 * @class Conversation
 */
var Conversation = util.class('Conversation', {

	m_isOpen: false,
	m_services: null,
	
	/**
	 * @field server {Server}
	 */
	server: null,
	
	/**
	 * @field request {http.ServerRequest}
	 */
	request: null,
	
	/**
	 * @field socket 
	 */
	socket: null,
	
	/**
	 * @field token {Number}
	 */
	token: '',
	
	// @event:
	onError: null,
	onMessage: null,
	onClose: null,
	onOpen: null,
	
	/**
	 * @param {http.ServerRequest}   req
	 * @param {String}   bind_services_name
	 * @constructor
	 */
	constructor: function(req, bind_services_name) {
		event.initEvents(this, 'Open', 'Message', 'Error', 'Close');
		
		this.server = req.socket.server;
		this.request = req;
		this.socket = req.socket;
		this.token = util.hash(util.id + this.server.host + '');
		this.m_services = {};
		
		util.nextTick(initialize, this, bind_services_name);
	},
	
	/**
	 * 是否已经打开
	 */
	get isOpen() {
		return this.m_isOpen;
	},
	
	/**
	 * verifies the origin of a request.
	 * @param  {String} origin
	 * @return {Boolean}
	 */
	verifyOrigin: function(origin) {
		
		var origins = this.server.origins;
		
		if (origin == 'null') {
			origin = '*';
		}
		
		if (origins.indexOf('*:*') != -1) {
			return true;
		}
		
		if (origin) {
			try {
				var parts = url.parse(origin);
				var ok =
					~origins.indexOf(parts.hostname + ':' + parts.port) ||
					~origins.indexOf(parts.hostname + ':*') ||
					~origins.indexOf('*:' + parts.port);
				if (!ok) {
					console.warn('illegal origin: ' + origin);
				}
				return ok;
			}
			catch (ex) {
				console.warn('error parsing origin');
			}
		} else {
			console.warn('origin missing from websocket call, yet required by config');
		}
		return false;
	},
	
	/**
	 * 获取绑定的服务
	 */
	get clientServices() {
		return this.m_services;
	},
	
	/**
	 * @func handlePacket() 进一步解析数据
	 * @arg {Number} type    0:String|1:Buffer
	 * @arg {String|Buffer} packet
	 */
	handlePacket: function(type, packet) {
		this.onMessage.trigger({ type, data: packet });

		if (type === 0 && packet[0] == '\ufffe') { // json text
			try {
				var data = JSON.parse(packet.substr(1));
			} catch(err) {
				console.error(err); return;
			}
			if (data.type == 'bind_client_service') { // 绑定服务消息
				bind_services(this, [data.name], (is)=>{ 
					if (!is) {
						this.onError.trigger(Error.new('Bindings service failure'));
					}
				});
			} else {
				var service = this.m_services[data.service];
				if (service) {
					service.receiveMessage(data);
				} else {
					console.error('Could not find the message handler, '+
												'discarding the message, ' + data.service);
				}
			}
		}
	},

	/**
	 * open Conversation
	 */
	initialize: function() {},

	/**
	 * send message to client
	 * @arg {Object} data
	 */
	send: function(data) {},

	/**
	 * @func ping()
	 */
	ping: function() {},
	
	/**
	 * close the connection
	 */
	close: function () {}
	
	// @end
});

exports.Conversation = Conversation;
