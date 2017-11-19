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
var EventNoticer = event.EventNoticer;
var Service = require('./service').Service;
var Session = require('./session').Session;

function message_handle (self, data) {
	var type = data.type;
	
	if (type == 'call') { // 调用函数
		var args = data.args;
		var fn = self[data.name];
		var cb = data.callback;
		
		if (!fn) {
		  return self.error('"{0}" no defined'.format(data.name));
		}
		
		if (typeof fn != 'function') {
		  return self.error('"{0}" no function'.format(data.name));
		}
		
		if (util.is_array(args.args)) {
		  args = args.args;
		} else {
		  args = [ args ];
		}
    
    var hasCallback = false;
		var rev = { type: 'callback', callback: cb, service: self.name };
		
		var callback = function (err, data) {
      if (hasCallback) {
      	return self.error(new Error('callback has been completed'));
      }
      hasCallback = true;
      
      if (cb) {
        if (self.conv.is_open) {  // 如果连接断开,将这个数据丢弃
          if (err) {
            rev.error = util.err_to(err);
          } else {
            rev.data = data;
          }
          self.conv.send(rev);
        }
      }
		};
		
    args.push(function (data) { callback(null, data) }.catch(callback));
    
		fn.apply(self, args);
	}
	else if (type == 'cancel_call') {
	  // TODO cancel call
	}
	else if (type == 'callback') { // 客户端返回的回调
	
    var err = data.error;
    var id = data.callback;
    var callbacks = self.m_callbacks;
    var callback = callbacks[id];
    
    if (!callback) {
      // 没有回调可能已经取消
      return;
    }
    
    delete callbacks[id];
    
    callback(err, data.data);
	} else {
	  // TODO Unknown
	}
}

function close_handle (self, evt) {
	var all = event.all_delegate(self);
  // 删除全部侦听器
	for (var i = 0; i < all.length; i++) {
		all[i].off();
	}
	
  var callbacks = self.m_callbacks;
  var error = new Error('Connection closed unexpectedly');
  self.m_callbacks = { };
  
  for (i in callbacks) {
    callbacks[i](error); // 处理连接异常
  }
}

// 
function get_callback (self, cb) {
  return function (err, data) {
    if (err) {
      cb.throw(util.err(err));
    } else {
      cb(data);
    }
  };
}

/**
 * @class SocketService
 * @bases service::Service
 */
var SocketService = util.class('SocketService', Service, {
  
// @private:
  m_callbacks: null,
  m_conv: null,
  
// @public:
	/**
	 * @event onerror
	 */
	'@event onerror': null,
	
	/**
	 * conv
	 * @type {conv}
	 */	
	get conv () {
	  return this.m_conv;
	},
  
	/**
	 * site session
	 * @type {Session}
	 */
	session: null,
	
	/**
	 * @arg conv {Conversation}
	 * @constructor
	 */
	constructor: function (conv) {
    Service.call(this, conv.request);
		this.m_conv = conv;
		this.m_callbacks = { };
		this.session = new Session(this);
		
		var self = this;
    
		function listen (evt) {
			if(self.conv.is_open) {  // 如果连接断开,将这个数据丢弃
				conv.send({
				  service: self.name, 
				  type: 'event', 
				  event: evt.type,
				  data: evt.type == 'error' ? util.err_to(evt.data) : evt.data,
				});
			}
		}
		
		var all = event.all_delegate(this);
		// 侦听事件发送到客户端
		for (var i = 0; i < all.length; i++) {
			all[i].on(listen);
		}
		conv.onclose.$on(close_handle, this);
	},
	
	/**
	 * @fun receive_message # 消息处理器
	 * @arg data {Object}
	 */
	receive_message: function (data) {
	  message_handle(this, data);
	},
	
	/**
	 * @fun call # 调用客户端 api
	 * @arg name {String} 
   * @arg [args] {Array} 
   * @arg [cb] {Function} # 如果不希望返回数据可以不传入回调
   * @ret {Number}
	 */
	call: function (name, args, cb) {
    
    if (!this.conv.is_open) { // 连接是否关闭
      if (cb) {
        util.next_tick(cb, new Error('error connection close status'));
      }
      return 0;
    }
    
    if (typeof args == 'function') {
      cb = args;
      args = [];
    }
    
    args = args ? util.is_array(args) ? args : [args] : [];
    
    var msg = {
      service: this.name,
      type: 'call', 
      name: name, 
      args: args,
    };
    
    var id = 0;
    if (cb) {
      msg.callback = id = util.id();
      this.m_callbacks[id] = get_callback(this, cb);
    }
    this.conv.send(msg);
    
    return id;
	},
	
  /**
   * @fun abort # 取消调用
   * @arg [id] {String}
   */
	abort: function (id) {
    // TODO 这个地方不能这样简单,最好能把取消的消息发到客服端
    // cancel call
    if (id) {
      delete this.m_callbacks[id];
    } else {
      this.m_callbacks = { };
    }
	},
	
	/**
	 * @fun error # trigger error event
	 * @arg err {Error} 
	 */
	error: function (err) {
		console.error(err);
		this.onerror.trigger(util.err(err));
	},
	// @end
});

exports.SocketService = SocketService;
