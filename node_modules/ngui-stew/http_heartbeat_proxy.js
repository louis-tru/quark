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
var conv = require('./conv');
var HttpHeartbeatconv = require('./http_heartbeat_conv').HttpHeartbeatconv;
var HttpService = require('./http_service').HttpService;
var service = require('./service');

exports.HttpHeartbeatProxy = util.class('HttpHeartbeatProxy', HttpService, {
  
	/**
	 * complete handshakes, return client
	 * @param {Number} token             conv token
	 * @param {Object} password          verify the password
	 */
	handshakes_complete: function (token, password) {
		this.result({ type: 'handshakes_complete', token: token, password: password });
	},
  
	/**
	 * send message to client
	 * @param {String[]} msg
	 */
	send: function (msg) {
		this.result({ type: 'message', data: msg });
	},

	/**
	 * complete receive client data, return client
	 */
	receive_complete: function () {
		this.result({ type: 'receive_complete' });
	},

	/**
	 * close the connection
	 */
	close: function () {
		this.result({ type: 'close' });
	},

	/**
	 * http heartbeat proxy handshakes
	 * @type {String} bind_services_name  bind services name
	 */
	handshakes: function (bind_services_name) {
		// 创建连接
		new HttpHeartbeatconv(this, bind_services_name);
	},

	/**
	 * listen conv change
	 * @param {Number} token        conv token
	 * @param {String} password     verify the password
	 */
	listen: function (token, password) {

		/*
		Note:
		The network fault tolerance,
		the browser will cause strange the second request,
		this error only occurs on the server restart,
		the BUG caused by the request can not respond to
		*/
		var conv = conv.get(token);
		if (conv)
			conv.listen(this, password);
		else
			this.close();
	},
  
	/**
	 * receive client data
	 * @param {Number} token        conv token
	 * @param {String} password     verify the password
	 * @param {String} data         get data
	 */
	receive: function (token, password, data) {
		var conv = conv.get(token);
		if (conv)
			conv.receive(this, password, data);
		else
			this.close();
	},
	// @end
});

service.set('HttpHeartbeatProxy', exports.HttpHeartbeatProxy);