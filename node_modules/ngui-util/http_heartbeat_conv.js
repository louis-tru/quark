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

var TIMEOUT = 2E4;     //wait listen timeout default as 20s
var TIMEOUT2 = 5E4;

/**
 * @class Parser
 * @private
 */
var Parser = util.class('Parser', {
  
	buffer: '',
	
	i: 0,
  
  /**
   * @constructor
   */
	constructor: function () {
		event.init_events(this, 'close', 'data', 'error');
	},
  
	add: function (data) {
		this.buffer += data;
		this.parse();
	},
  
	parse: function () {
	  
	  var len = this.buffer.length;
    
		for (var i = this.i, chr; i < len; i++) {
      
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
				this.ondata.trigger(this.buffer.substr(1, i - 1));
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


function reset (self) {
	self.m_proxy = null;
	self.m_timeout = function () {
		//console.log('http heartbeat timeout close');
		self.close();
	} .delay(TIMEOUT);
}

function handshakes_complete (self) {
	self.m_proxy.handshakes_complete(self.token, self.m_password);
	reset(self);
}

function send (self) {
	util.clear_delay(self.m_no_timeout);
	self.m_proxy.send(self.m_message);
	self.m_message = [];
	reset(self);
}

/**
 * Send a test signal
 */
function sendTest (self) {
	self.send('\ufffb\ubfff');
}

/**
 * @class HttpHeartbeatConversation
 * @bases conv::Conversation
 */
var HttpHeartbeatConversation = util.class('HttpHeartbeatConversation', Conversation, {
	// @private:
	m_proxy: null,
	m_parser: null,
	m_password: null,
	m_timeout: 0,
	m_no_timeout: 0,
	m_message: null,
	
	// @public:
	/**
	 * @constructor
	 * @arg proxy {HttpHeartbeatProxy} 
	 * @arg bind_services_name {String}
	 */
	constructor: function (proxy, bind_services_name) {
		Conversation.call(this, proxy.request, bind_services_name);
		this.m_proxy = proxy;
		this.m_password = { main: util.random(), aid: util.random() };
		this.m_message = [];
	},
  
	/**
	 * @fun listen # listen conversation change
	 * @arg {HttpHeartbeatProxy} proxy
	 * @arg {String} password     verify the password
	 */
	listen: function (proxy, password) {
	  
		var self = this;
		
		if (this.is_open && !this.m_proxy && this.m_password.main == password) {
      
			util.clear_delay(this.m_timeout);
			this.m_proxy = proxy;
			this.m_no_timeout = sendTest.delay(TIMEOUT2, self);

			var req = proxy.request;

			req.on('close', function () {
				//console.log('http heartbeat close');
				self.close();
			});

			req.on('error', function (err) {
				console.log('http heartbeat error close');
				self.close();
			});

			req.on('aborted', function () {
				//console.log('http heartbeat aborted close');
				self.close();
			});

			if (this.m_message.length)
			  send(this);
		} else 
			proxy.close();
	},

	/**
	 * @fun receive # receive client data
	 * @param {HttpHeartbeatProxy} proxy
	 * @param {String} password     verify the password
	 * @param {String} data         get data
	 */
	receive: function (proxy, password, data) {
		if(this.is_open && this.m_password.aid == password) {
			this.m_parser.add(data);
			proxy.receive_complete();
		} else {
			proxy.close();
		}
	},
  
  /**
   * @overwrite
   */
	init: function () {
		var self = this;
		this.m_parser = new Parser();
    
		handshakes_complete(this);
    
		this.m_parser.ondata.on(function (evt) {
			self.parse(evt.data);
		});
    
		this.m_parser.onclose.on(function () {
			//console.log('http heartbeat parser close');
			self.close();
		});
    
		this.m_parser.onerror.on(function (e) {
			console.error(e.data + '\nhttp heartbeat parser error close');
			self.onerror.trigger(e.data);
			self.close();
		});
	},
  
  /**
   * @overwrite
   */
	send: function (msg) {
		if (this.is_open) {
			this.m_message.push(JSON.stringify(msg));
			if (this.m_proxy)
			  send(this);
		} else 
			throw new Error('error connection close status');
	},
	
  /**
   * @overwrite
   */
	close: function () {
		if (this.is_open) {
			util.clear_delay(this.m_timeout);
			if (this.m_proxy)
			  this.m_proxy.close();
			this.onclose.trigger();
		}
	},
  // @end
});

exports.HttpHeartbeatConversation = HttpHeartbeatConversation;
