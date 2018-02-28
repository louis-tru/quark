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
var SocketService = require('./ws_service').SocketService;
var url = require('url');
var service = require('./service');

var all_conversations = { };

// 绑定服务
function bind_services (self, services, cb) {
  
  if (!services.length) {
    cb (true);
    return;
  }
  
  var name = services.shift();
  var cls = service.get(name);
  
  if (name in self.m_services) {
    console.error('Service no need to repeat binding');
    cb(false);
    return;
  }

	if (!cls || !util.equals_class(SocketService, cls)) {
		console.error(name + ' Service type is not correct');
		cb(false);
		return;
	}
	
	var ser = new cls(self);
	self.m_services[name] = ser;
	
	ser.auth(function (is) { // 认证合法性
	  if (is) {
	    bind_services(self, services, cb); // 继续绑定
	  } else {
	    cb(false);
	  }
	});
}

//Init
function init (self, bind_services_name) {
  
	all_conversations[self.token] = self;
	self.m_is_open = true;
	
	var services = bind_services_name.split(',');
	
	if (!services[0]) { // 没有服务,这种连接没有意义
	  self.close();     // 关闭连接
	  return;
	}
	
	bind_services(self, services, function (is) {
	  if (is) { 
    	self.init(); // 初始化连接/握手
    	if (self.m_is_open) {
      	self.onopen.trigger();
      	exports.onopen.trigger(self);
    	}
	  } else { // 绑定失败
	    self.close(); // 关闭连接
	  }
	});
}

/**
 * 连接
 */
var Conversation = util.class('Conversation', {
  
  /**
   * 是否打开
   * @private
   */
  m_is_open: false,
  
	/**
	 * server
	 * @type {Server}
	 */
	server: null,
  
	/**
	 * request
	 * @type {http.ServerRequest}
	 */
	request: null,
  
	/**
	 * service
	 * @type {Object}
	 */
	m_services: null,
  
	/**
	 * Conversation token
	 * @type {Number}
	 */
	token: 0,
    
	/**
	 * @event onerror
	 */
	onerror: null,
  
	/**
	 * @event onmessage
	 */
	onmessage: null,
  
	/**
	 * @event onclose
	 */
	onclose: null,
  
	/**
	 * @event onclose
	 */
	onopen: null,
  
	/**
	 * @param {http.ServerRequest}   req
	 * @param {String}   bind_services_name
	 * @constructor
	 */
	constructor: function (req, bind_services_name) {
	  event.init_events(this, 'open', 'message', 'error', 'close');
	  
		this.server = req.socket.server;
		this.request = req;
    this.token = util.hash(util.id() + this.server.host + '');
		this.m_services = { };
		
		var self = this;
    
		this.onclose.once(function () {
			delete all_conversations[self.token];
			self.m_is_open = false;
			self.onopen.off();
			self.onmessage.off();
			self.onerror.off();
			util.next_tick(self.onclose, self.onclose.off);
			exports.onclose.trigger(self);
		});
		
		util.next_tick(init, this, bind_services_name);
	},
	
	/**
	 * 是否已经打开
	 */
	get is_open () {
	  return this.m_is_open;
	},
  
	/**
	 * verifies the origin of a request.
	 * @param  {String} origin
	 * @return {Boolean}
	 */
	verify_origin: function (origin) {
	  
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
	get services () {
    return this.m_services;
	},
	
	/**
	 * 进一步解析数据
	 * @param {String} msg
	 */
	parse: function (msg) {
    var data = JSON.parse(msg);
    if (data.type == 'bind_service') { // 绑定服务消息
      bind_services(this, [data.name], function (is) { 
        if (!is) {        // 绑定失败
          // TODO 是否要关闭连接
          // self.close(); // 关闭连接
        }
      });
    } else {
		  var service = this.m_services[data.service];
		  if (service) {
		    service.receive_message(data);
		  } else {
		    console.error('Could not find the message handler, '+
		                  'discarding the message, ' + data.service);
		  }
      this.onmessage.trigger(data);
    }
	},
  
	/**
	 * open Conversation
	 */
	init: function () { },
  
	/**
	 * close the connection
	 */
	close: function () { },
  
	/**
	 * send message to client
	 * @param {Object} data
	 */
	send: function (data) { },
  // @end
});

module.exports = {

	Conversation: Conversation,
  
	/**
	 * @event onopen
	 * @static
	 */
	onopen: new event.EventNoticer('open'),
  
	/**
	 * @event onclose
	 * @static
	 */
	onclose: new event.EventNoticer('close'),
  
	/**
	 * Get Conversation by token
	 * @param {Number} token
	 * @return {Conversation}
	 */
	get: function (token) {
		return all_conversations[token];
	},
  
	/**
	 * Get all conversation by token
	 * @return {Object}
	 */
	get all () {
		return all_conversations;
	},
};

