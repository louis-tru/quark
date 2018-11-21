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
var querystring = require('querystring');
var Path = require('path');

var service_cls = { };

/**
 * base service abstract class
 * @class Service
 */
var Service = util.class('Service', {
	// @private:
	m_pathname: null,
	m_dirname: null,
	m_extname: null,
	m_params: null,
	
	// @public:
	/**
	 * @type {String} 服务名称
	 */
	name: '',
	
	/**
	 * server
	 * @type {Server}
	 */
	server: null,

	/**
	 * request of server
	 * @type {http.ServerRequest}
	 */
	request: null,

	/**
	 * @type {net.Stream}
	 */
	socket: null,

	/**
	 * request host
	 * @type {String}
	 */
	host: '',

	/**
	 * request path
	 * @type {String}
	 */
	url: '',
	
	/**
	 * @get headers
	 */
	get headers() {
		return this.request.headers;
	},
	
	/**
	 * no param url
	 * @type {String}
	 */
	get pathname() {
		if (!this.m_pathname) {
			var mat = this.url.match(/^\/[^\?\#]*/);
			this.m_pathname = mat ? mat[0] : this.url;
		}
		return this.m_pathname;
	},
	
	/**
	 * request path directory
	 * @type {String}
	 */
	get dirname() {
		if (!this.m_dirname) {
			this.m_dirname = Path.dirname(this.pathname);
		}
		return this.m_dirname;
	},
	
	/**
	 * request extended name
	 * @type {String}
	 */
	get extname() {
		if(this.m_extname === null) {
			var mat = this.pathname.match(/\.[^\.]+$/);
			this.m_extname = mat ? mat[0] : '';
		}
		return this._extname;
	},
	
	/**
	 * url param list
	 * @type {Object}
	 */
	get params() {
		if (!this.m_params) {
			var mat = this.url.match(/\?(.+)/);
			this.m_params = querystring.parse(mat ? mat[1] : '');
		}
		return this.m_params;
	},
	
	/**
	 * @func setTimeout(value)
	 */
	setTimeout: function(value) {
		this.request.setTimeout(value);
	},
	
	/**
	 * @constructor
	 * @arg req {http.ServerRequest}
	 */
	constructor: function(req) {
		this.server = req.socket.server;
		this.request = req;
		this.socket = req.socket;
		this.host = req.headers.host;
		this.url = decodeURI(req.url);
	},
	
	/**
	 * authentication by default all, subclasses override
	 * @param {Function} cb
	 * @param {Object}   info
	 */
	requestAuth: function(info) {
		return true;
	},
	
	/**
	 * call function virtual function
	 * @param {Object} info service info
	 */
	action: function(info) { 
		
	},
	// @end
});

module.exports = {

	Service: Service,
	
	/**
	 * 获取所有的服务名称列表
	 */
	get services() { return Object.keys(service_cls) },
	
	/**
	 * 通过名称获取服务class
	 */
	get: function(name) {
		return service_cls[name];
	},

	/**
	 * @func getServiceDescriptors()
	 */
	getServiceDescriptors: function() {
		var r = {};
		Object.entries(service_cls).forEach(([key, value])=>{
			if (!/^(StaticService|HttpHeartbeatProxy)$/.test(key)) {

				var type = 0;
				var methods = [], events = [];
				var item = { type: item, methods, events, };
				var self = value.prototype;

				Object.entries(Object.getOwnPropertyDescriptors(self)).forEach(([k, v])=>{
					if (!/(^(constructor|auth|requestAuth)$)|(^(_|\$|m_))/.test(k)) {
						if (/^on[a-zA-Z]/.test(k)) { // event
							events.push(k.substr(2));
						} else {
							if (typeof v.value == 'function') {
								methods.push(k);
							}
						}
					}
				});

				self = self.__proto__;

				while (self !== Service.prototype) {
					Object.entries(Object.getOwnPropertyDescriptors(self)).forEach(([k, v])=>{
						if (/^on[a-zA-Z]/.test(k)) { // event
							events.push(k.substr(2));
						}
					});
					self = self.__proto__;
				}
				
				r[key] = item;
			}
		});
		return r;
	},

	set: function(name, cls) {
		util.assert(util.equalsClass(Service, cls), '"{0}" is not a "Service" type', name);
		util.assert(!(name in service_cls), 'service repeat definition, "{0}"', name);
		cls.prototype.name = name; // 设置服务名称
		service_cls[name] = cls;
	},

	del: function(name) {
		delete service_cls[ name ];
	}
};
