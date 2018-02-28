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
var Cookie = require('./cookie').Cookie;
var StaticService = require('./static_service').StaticService;
var IncomingForm = require('./incoming_form').IncomingForm;
var session = require('./session');
var zlib = require('zlib');
var Buffer = require('buffer').Buffer;

var StaticService_action = StaticService.members.action;

/**
 * @private
 */
function ret(self, data) {
	var type = self.server.get_mime(self.jsonp_cb_arg ? 'js' : 'json');
	var rev = JSON.stringify(data);
	
	if (self.jsonp_cb_arg)
		data = self.jsonp_cb_arg + '(' + rev + ')';
	self.ret_str(type , rev);
}

function action_multiple_calls(self, calls, index, cb) {
	var funcs = { };
	var result = { };
	var count = 0; var done_count = 0;
	var done = 0;

	for ( var name in calls ) {
		count++;
		var func = self[name];
		if ( typeof func == 'function' ) {
			funcs[name] = func;
		} else {
			var err = new Error('could not find function ' + name);
			err.code = -1;
			cb ( err, index ); return;
		}
	}

	function cb2(name, err, data) {
		if ( result[name] ) {
			throw new Error('Already completed callback');
		}
		if ( done ) { // Already end
			return;
		}

		done_count++;

		if ( err ) { //
			done = true;
			err = util.err_to(err);
			err.api = name;
			err.name = name;
			if ( !err.code ) err.code = -1;
			cb ( err, index );
			result[name] = err;
			return;
		} else {
			result[name] = data;
		}

		if ( done_count == count ) {
			done = true;
			cb( { data: result }, index ); // done
		}
	}

	for ( var name in calls ) {
		(function(name) {
			funcs[name].call(self, calls[name], function(data) { 
				cb2(name, null, data);
			}.catch(function(err) { cb2(name, err) }));
		}(name));
	}
}

function action_multiple(self) {
	
	var post_buffs = [];
	var post_total = 0;
	
	if ( self.request.method == 'POST' ) {
	  self.request.on('data', function(buff) {
			post_buffs.push(buff);
			post_total += buff.length;
	  });
	}

	self.request.on('end', function() { 
		var data = null;
		if ( post_buffs.length ) {
			data = Buffer.concat(post_buffs, post_total).toString('utf-8');
		} else {
			data = self.params.data;
		}
		if ( data ) {
			try {
				data = JSON.parse(data);
				if ( !Array.isArray(data) ) {
					self.ret_err(new Error('multiple call data error')); return;
				}
			} catch(err) {
				self.ret_err(err); return;
			}

			var count = data.length;
			var done_count = 0;
			var result = Array(count);

			for ( var i = 0; i < count; i++ ) {
				action_multiple_calls(self, data[i], i, function(data, i) {
					result[i] = data; 
					done_count++;
					if ( done_count == count ) { //done
						self.ret(result);
					}
				});
			}
		} else {
			self.ret_err(new Error('multiple call data error'));
		}
	});
}

/** 
 * @class HttpService
 * @bases static_service::StaticService
 */
var HttpService = util.class('HttpService', StaticService, {
// @public:
	/**
	 * site cookie
	 * @type {Cookie}
	 */
	cookie: null,
	
	/**
	 * site session
	 * @type {Session}
	 */
	session: null,

	/**
	 * ajax jsonp callback name
	 * @tpye {String}
	 */
	jsonp_cb_arg: '',

	/**
	 * post form
	 * @type {IncomingForm}
	 */
	form: null,

	/**
	 * post form data
	 * @type {Object}
	 */
	data: null,
  
	/**
	 * @constructor
	 * @arg req {http.ServerRequest}
	 * @arg res {http.ServerResponse}
	 */
	constructor: function (req, res) {
		StaticService.call(this, req, res);
		this.cookie = new Cookie(req, res);
		this.session = new session.Session(this);
		this.jsonp_cb_arg = this.params.callback || '';
		this.data = { };
	},
	
  /** 
   * @overwrite
   */
	action: function (info) {
		/*
		 * Note:
		 * The network fault tolerance,
		 * the browser will cause strange the second request,
		 * this error only occurs on the server restart,
		 * the BUG caused by the request can not respond to
		 */

		var self = this;
		var action = info.action;

		if ( action == 'multiple' ) {
			return action_multiple(this);
		}

		//Filter private function
		if (/^_/.test(action))
			return StaticService_action.call(this);
			
		var fn = this[action];
		if (!fn || typeof fn != 'function')
		  return StaticService_action.call(this);
		
		delete info.service;
		delete info.action;
		
		var has_callback = false;
		
		var callback = function (err, data) {
			if (has_callback) {
				throw new Error('callback has been completed');
			}
			has_callback = true;
			
			if (err) {
			  self.ret_err(err);
			} else {
			  self.ret(data);
			}
		};
    
		var end = function () {
			var args = self.data.args || self.params.args;
			if (args) {
				try { args = JSON.parse(args) } catch (e) { }
			}
			args = util.values(info).concat(util.is_array(args) ? args : []);
			args.push(function (data) { callback (null, data) }.catch(callback));
			fn.apply(self, args);
		};

		if (this.request.method == 'POST') {
		  var self = this;
		  var form = this.form = new IncomingForm(this);
			form.upload_dir = this.server.temp;
			
			form.onend.on(function () {
				util.ext(self.data, form.fields);
				util.ext(self.data, form.files);
				end();
			});
			form.parse();
		} else {
			this.request.on('end', end);
		}
	},
  
	/**
	 * @fun ret_data   # return data to browser
	 * @arg type {String} #    MIME type
	 * @arg data {Object} #    data
	 */
	ret_data: function (type, data) {
		var res = this.response;
		var ae = this.request.headers['accept-encoding'];
    
		this.set_default_header(0);
		res.setHeader('Content-Type', type);
    
		if (typeof data == 'string' && 
		    this.server.agzip && ae && ae.match(/gzip/i)) {
			zlib.gzip(data, function (err, data) {
				res.setHeader('Content-Encoding', 'gzip');
				res.writeHead(200);
				res.end(data);
			});
		} else {
			res.writeHead(200);
			res.end(data);
		}
	},
	
	/**
	 * @fun ret_str # return string to browser
	 * @arg type {String} #    MIME type
	 * @arg str {String}
	 */
	ret_str: function (type, str) {
	  this.ret_data(type + ';charset=utf-8', str);
	},
	
	/**
	 * @fun ret_html # return html to browser
	 * @arg html {String}  
	 */
	ret_html: function (html) {
		var type = this.server.get_mime('html');
		this.ret_str(type, html);
	},
	
	/**
	 * @fun ret_err # return error to browser
	 * @arg [err] {Error} 
	 */
	ret_err: function (err) {
		err = util.err_to(err);
		err.st = new Date().valueOf();
		if ( !err.code ) err.code = -1;
		ret(this, err);
	},
	
	/**
	 * @fun rev # return data to browser
	 * @arg data {JSON}
	 */
	ret: function (data) {
	  ret(this, { data: data, st: new Date().valueOf() });
	},
	// @end
});

exports.HttpService = HttpService;