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
var fs = require('./fs');
var event = require('./event');
var Router = require('./router').Router;
var service = require('./service');
var ws_conv = require('./ws_conv');
var http_heartbeat_proxy = require('./http_heartbeat_proxy');
var StaticService = require('./static_service').StaticService;
var http = require('http');
var incoming_form = require('./incoming_form');

var shared = null;
var mime_types = { };
var default_root = process.cwd();
var default_temp = incoming_form.temp_dir;

read_mime('mime.types');
read_mime('mime+.types');

function read_mime (filename) {
	var data = fs.readFileSync(  __dirname + '/' + filename ) + '';
	var ls = data.replace(/ *#.*\r?\n?/g, '').split(/\n/);
	
	for (var i = 0; i < ls.length; i++) {
		var item = ls[i].replace(/^\s|\s$|;\s*$/g, '')
			.replace(/\s+|\s*=\s*/, '*%*').split('*%*');
			
		var key = item[0];
		var value = item[1];
		if (value) {
			var values = value.split(/\s+/);
			var len2 = values.length;
			
			for(var j = 0; j < len2; j++){
				mime_types[values[j]] = key;
			}
		}
	}
}

//Handle http and websocket and http-heartbeat request
function initializ(self) {
  
	//http
	self.on('request', function (req, res) {
	  
    var url = decodeURI(req.url);       // 解码
		var info = self.router.find(url);   // 通过url查找目标服务信息
		var name = info.service;
		var cls = service.get(name);
		
	  if (self.print_log) 
	    console.log(url);
    
		if (cls) {
			if (!util.equals_class(StaticService, cls)) {
				console.error(name + ' not the correct type, http request');
				cls = StaticService;
			}
		} else {
			cls = StaticService;
		}
    
		var ser = new cls(req, res);

		req.pause();
		ser.auth(function (is) { // 认证请求的合法性
			if (!is) // 立即断开连接
				return req.socket.destroy();
				
			req.on('data', function () { });
			req.resume();
			ser.action(info);
		}, info.action);
	});
  
	// upgrade websocket
	self.on('upgrade', function (req, socket, upgradeHead) {
		// 创建新web socket连接
		ws_conv.create(req, upgradeHead);
	});
	
	self.on('error', function (err) {
	  console.log(err);
	  console.log('Server Error ---------------');
	});
}

/**
 * 设置服务器
 */
function config_server(self, config) {
  
  config = config || { };
  
	util.update(self, util.filter(config, [
		'host',
		'print_log', 
		'auto_index',
		'mime_types',
		'error_status',
		'agzip',
		'origins',
		'port',
		'file_cache_time',
		'expires',
		'timeout',
		'session',
		'max_file_size',
		'max_form_data_size',
		'max_upload_file_size',
		'text_encoding',
		'defaults',
	]));
  
	var disable   = config.disable;
	var root      = config.root;
	var temp      = config.temp;
	var virtual   = config.virtual;
	var gzip      = config.gzip;

	self.port     = parseInt(process.env.WEB_SERVER_PORT) || self.port;
	self.root     = root ? util.format(root) : self.root;
	self.temp     = temp ? util.format(temp) : self.temp;
	
	if (disable) {
	  if (util.is_array(disable)) 
	    disable = disable.join(' ');
	  disable = String(disable).trim().replace(/\s+/mg, '|');
	  self.disable = new RegExp('^\\/(' + disable + ')');
	}
	if (virtual) {
	  self.virtual = String(virtual).trim().replace(/^(\/|\\)*([^\/\\]+)/, '/$2');
	}

	if ('gzip' in config) {
	  if (gzip === false) {
	  	self.gzip = false;
	  } else {
      gzip = String(gzip).trim().replace(/\s+/, '|');
	    new RegExp('javascript|text|json|xml|' + gzip, 'i');
	  }
	}
	
	fs.mkdir_p_sync(self.temp);
	
	self.router.config({
		static_service: config.static_service,
		virtual: self.virtual,
		router: config.router,
	});
}

/**
  * @class Server
  */
var Server = util.class('Server', http.Server, {

	//public:
	/**
	 * 侦听主机IP
	 * @type {String}
	 */
	host: '',

	/**
	 * 侦听端口
	 * @type {Number}
	 */
	port: 0, // 自动端口
	
  /**
	  * 打印log
	  */
	print_log: false,

	/**
	 * session timeout default 15 minutes
	 * @type {Number}
	 */
	session: 15,
  
	/**
	 * 站点根目录
	 * @type {String}
	 */
	root: default_root,
  
	/**
	 * 临时目录
	 * @type {String}
	 */
	temp: default_temp,
  
	/**
	 * 站点虚拟目录
	 * @type {String}
	 */
	virtual: '',

	/**
	 * web socket conversation verify origins
	 * @type {String[]}
	 */
	origins: null,

	/**
	 * 是否浏览静态文件目录
	 * @type {Boolean}
	 */
	auto_index: false,

	/**
	 * 静态缓存文件过期时间,以分钟为单位,为默认为30天
	 * @type {Number}
	 */
	expires: 60 * 24 * 30,

	/**
	 * 静态文件缓存,该值可减低硬盘静态文件读取次数,但需要消耗内存,单位(秒)
	 * @type {Number}
	 */
	file_cache_time: 10,

	/**
	 * Download file size limit
	 * @type {Number}
	 */
	max_file_size: 5 * 1024 * 1024,

	/**
	 * Max form data size limit
	 */
	max_form_data_size: 5 * 1024 * 1024,

	/**
	 * Upload file size limit
	 * @type {Number}
	 */
	max_upload_file_size: 5 * 1024 * 1024,
	
	/**
	 * 文本文件编码,默认为utf-8
	 */
	text_encoding: 'utf-8',

	/**
	 * 请求超时时间(秒)
	 * @type {Number}
	 */
	timeout: 120,

	/**
	 * 静态gzip文件格式
	 * defaults javascript|text|json|xml
	 * @type {Regexp}
	 */
	gzip: null,

	/**
	 * 是否动态数据内容压缩
	 * @type {Boolean}
	 */
	agzip: true,
  
	/**
	 * 默认页
	 * @type {String[]}
	 */
	defaults: null,

	/**
	 * 设置禁止访问的目录
	 * @type {RegExp}
	 */
	disable: null,

	/**
	 * 错误状态页
	 * @type {Object}
	 */
	error_status: null,

	/**
	 * 配置的文件mime
	 * mime types
	 * @type {Object}
	 */
	mime_types: null,
  
	/**
	 * http请求路由器
	 * @type {Router}
	 */
	router: null,
  
	//private:
	/**
	 * 是否正在运行
	 */
	m_is_run: false,

	/**
	 * 构造函数
	 * @constructor
	 * @param {Object} opt (Optional) 配置项
	 */
	constructor: function (config) {
		http.Server.call(this);
		this.gzip = /javascript|text|json|xml/i;
		this.error_status = { };
		this.disable = /^\/server/i;
		this.defaults = [];
		this.mime_types = {};
		this.origins = ['*:*'];
		this.router = new Router();
		config_server(this, config);
		initializ(this);
	},
  
	/**
	 * MIME 获取类型
	 * @param {String}   ename  扩展名或文件名称
	 * @return {String}
	 */
	get_mime: function (name) {
	  
		var mat = name.match(/\.([^$\?\/\\\.]+)((#|\?).+)?$/);
		if (mat) {
			name = mat[1];
		}
		name = name.toLowerCase();
		return this.mime_types[name] || mime_types[name] || 'application/octet-stream';
	},
  
	/**
	 * 是否正在运行
	 */
	get is_run(){
		return this.m_is_run;
	},
  
	/**
	 * 启动服务
	 */
	start: function () {
	  var self = this;

		if (this.port) {
			this.listen(this.port, this.host);
		} else if ( this.host ) {
		  this.listen(String(this.host), function() {
				self.port = self.address().port;
		  });
		} else {
			this.listen(function() {
			  var addr = self.address();
				self.host = addr.address;
				self.port = addr.port;
		  });
		}
		this.m_is_run = true;
	},
  
	/**
	 * 停止服务
	 */
	stop: function () {
		this.close();
		this.m_is_run = false;
	},
	 
	/**
	 * 重新启动
	 */
	restart: function (){
	  this.stop();
	  this.start();
	},
	// @end
});

module.exports = {

	Server: Server,

	/**
	 * @func set_shared
	 */
	set_shared: function(server) {
		shared = server;
	},
  
	/**
	 * @get shared # default web server
	 */
	get shared () {
		return shared;
	}
};
