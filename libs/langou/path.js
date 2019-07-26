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

/**************************************************************************/

const _path = requireNative('_path');
const _pkgutil = requireNative('_pkgutil');

/**************************************************************************/

const haveWeb = typeof global.window == 'object';

function split_path(self) {
	if (self._is_split) return;
	self._is_split = true;
	
	var value = self._value;
	var val = '';
	var i = value.indexOf('?');
	
	if (i != -1) {
		val = value.substr(0, i);
		var search = self._search = value.substr(i);
		i = search.indexOf('#');
		if (i != -1) {
			self._search = search.substr(0, i);
			self._hash = search.substr(i);
		}
	} else {
		i = value.indexOf('#');
		if (i != -1) {
			val = value.substr(0, i);
			self._hash = value.substr(i);
		} else {
			val = value;
		}
	}
	self._value = _pkgutil.resolve(val);
}

function parse_base_ext_name(self) {
	if (self._basename == -1) {
		split_path(self);
		var mat = self._value.match(/([^\/\\]+)?(\.[^\/\\\.]+)$|[^\/\\]+$/);
		if (mat) {
			self._basename = mat[0];
			self._extname = mat[2] || '';
		} else {
			self._extname = self._basename = '';
		}
	}
}

function parse_path(self) {
	if (self._is_parse) return;
	self._is_parse = true;
	
	split_path(self);
	
	var mat = self._value.match(/^(([a-z]+:)\/\/)?(([^\/:]+)(?::(\d+))?)?(\/.*)?/);
	if (mat) {
		self._protocol = mat[2] || '';
		self._origin = mat[1] || '';

		if ( mat[3] ) { // http:// or ftp:// or lib://
			self._origin += mat[3];
			self._hostname = mat[4];
			self._port = mat[5] ? mat[5] : '';
		}

		var path = self._filename = mat[6] || '/';
		var i = path.lastIndexOf('/');
		if (i > 0) {
			self._dirname = path.substr(0, i);
		} else {
			self._dirname = '/';
		}
	} else {
		throw new Error(`Parse url error, Illegal URL ${self._value}`);
	}
}

function parse_params(self) {
	if (self._params) 
		return;
	split_path(self);
	
	var params = self._params = { };
	
	if (self._search[0] != '?') 
		return;
		
	var ls = self._search.substr(1).split('&');
	
	for (var i = 0; i < ls.length; i++) {
		var o = ls[i].split('=');
		params[ o[0] ] = o[1] || '';
	}
}

function parse_hash_params(self) {
	if (self._hash_params) 
		return;
	split_path(self);
	
	var params = self._hash_params = { };
	if (self._hash[0] != '#') 
		return;
		
	var ls = self._hash.substr(1).split('&');
	
	for (var i = 0; i < ls.length; i++) {
		var o = ls[i].split('=');
		params[ o[0] ] = o[1] || '';
	}
}

function stringify_params(prefix, params) {
	var rev = [];
	for (var i in params) {
		rev.push(i + '=' + params[i]);
	}
	return rev.length ? prefix + rev.join('&') : '';
}

/**
 * @class URL
 */
class URL {
	
	/**
		* @arg [path] {String}
		* @constructor
		*/
	constructor(path = '') {
		if (!path && haveWeb) {
			path = location.href;
		}
		this._value = path;
	}
	
	// href: "http://xxxx.xxx:81/v1.1.0/./path.js?sasasas&asasasa#sdsdsd"
	get href() {
		parse_path(this);
		return this._origin + this._filename + this._search + this._hash;
	}
	
	/**
		* full path
		* filename: "/D:/Documents/test.js"
		*/
	get filename() {
		parse_path(this);
		return  this._filename;
	}
	
	/**
	 * @get path /a/b/s/test.html?aaaaa=100
	 */
	get path() {
		return this._filename + this._search;
	}
	
	/**
		* full path dir
		* dirname: "/D:/Documents"
		*/
	get dirname() {
		parse_path(this);
		return this._dirname;
	}
	
	// search: "?sasasas&asasasa"
	get search() {
		split_path(this);
		return this._search;
	}
	
	// hash: "#sdsdsd"
	get hash() {
		split_path(this);
		return this._hash;
	}
	
	// host: "langou.org:81"
	get host() {
		parse_path(this);
		return this._hostname + (this._port ? ':' + this._port : '');
	}
	
	// hostname: "langou.org"
	get hostname() {
		parse_path(this);
		return this._hostname;
	}
	
	// origin: "http://langou.org:81"
	get origin() {
		parse_path(this);
		return this._origin;
	}

	// get path base name 
	get basename() {
		parse_base_ext_name(this);
		return this._basename;
	}
	
	// path extname
	get extname() {
		parse_base_ext_name(this);
		return this._extname;
	}
	
	// port: "81"
	get port() {
		parse_path(this);
		return this._port;
	}
	
	// protocol: "http:"
	get protocol() {
		parse_path(this);
		return this._protocol;
	}
	
	get params() {
		parse_params(this);
		return this._params;
	}
	
	get hashParams() {
		parse_hash_params(this);
		return this._hash_params;
	}
	
	// get path param
	getParam(name) {
		return this.params[name];
	}
	
	// set path param
	setParam(name, value) {
		this.params[name] = value || '';
		this._search = stringify_params('?', this._params);
		return this;
	}
	
	// del path param
	deleteParam(name) {
		delete this.params[name];
		this._search = stringify_params('?', this._params);
		return this;
	}
	
	// del all prams
	clearParam() {
		this._params = { };
		this._search = '';
		return this;
	}
	
	// get hash param
	getHash(name) {
		return this.hashParams[name];
	}
	
	// set hash param
	setHash(name, value) {
		this.hashParams[name] = value || '';
		this._hash = stringify_params('#', this._hash_params);
		return this;
	}
	
	// del hash param
	deleteHash(name) {
		delete this.hashParams[name];
		this._hash = stringify_params('#', this._hash_params);
		return this;
	}
	
	// del hash all params
	clearHash(){
		this._hash_params = { };
		this._hash = '';
		return this;
	}
	
	// relative path
	relative(target) {
		target = new URL(target);
		if ( this.origin != target.origin )
			return this._origin + this._filename;
		var ls  = this._filename == '/' ? [] : this._filename.split('/');
		var ls2 = target._filename == '/' ? [] : target._filename.split('/');
		var len = Math.max(ls.length, ls2.length);
		
		for (var i = 1; i < len; i++) {
			if (ls[i] != ls2[i]) {
				len = ls.length - i;
				if (len > 0) {
					for (var j = 0, ls = []; j < len; j++)
						ls.push('..');
					return ls.join('/') + '/' + ls2.splice(i).join('/');
				}
				return ls2.splice(i).join('/');
			}
		}
		return '';
	}
	// @end
}

URL.prototype._is_split = false;
URL.prototype._is_parse = false;
URL.prototype._value = '';
URL.prototype._hostname = '';
URL.prototype._port = '';
URL.prototype._protocol = '';
URL.prototype._search = '';
URL.prototype._hash = '';
URL.prototype._origin = '';
URL.prototype._filename = '';
URL.prototype._dirname = '';
URL.prototype._basename = -1;
URL.prototype._extname = -1;
URL.prototype._params = null;
URL.prototype._hash_params = null;

function get_path(path) {
	return new URL(path);
}

function resolveLocal(...args) {
	return _path.fallbackPath(_pkgutil.resolve(...args));
}

Object.assign(exports, _path);

Object.assign(exports, {

	URL: URL,

	/** 
	 * @func isAbsolute(path) is absolute path
	 */
	isAbsolute: _pkgutil.isAbsolute, // func
	
	/**
	 * @func resolve(path) resolve path 
	 */
	resolve: _pkgutil.resolve, // func

	/**
	 * @func resolveLocal()
	 */
	resolveLocal: resolveLocal,

	/**
	 * full filename
	 */
	basename: function (path) {
		return get_path(path).basename;
	},

	/**
	 * full filename
	 */
	dirname: function (path) {
		return get_path(path).dirname;
	},

	/**
	 * full filename
	 */
	extname: function (path) {
		return get_path(path).extname;
	},

	/**
	 * full filename
	 */
	filename: function (path) {
		return get_path(path).filename;
	},

	/**
	 * full path
	 */
	path: function (path) {
		return get_path(path).path;
	},
	
	search: function (path) {
		return get_path(path).search;
	},
	
	hash: function (path) {
		return get_path(path).hash;
	},
	
	host: function (path) {
		return get_path(path).host;
	},
	
	hostname: function (path) {
		return get_path(path).hostname;
	},
	
	// href origin
	origin: function (path) {
		return get_path(path).origin;
	},
	
	// port: "81"
	port: function (path) {
		return get_path(path).port;
	},
	
	// protocol: "http:"
	protocol: function (path) {
		return get_path(path).protocol;
	},
	
	// href params
	params: function (path) {
		return get_path(path).params;
	},
	
	// hash params 
	hashParams: function (path) {
		return get_path(path).hashParams;
	},
	
	// get path param
	getParam: function (name, path) {
		return get_path(path).getParam(name);
	},
	
	// set path param
	setParam: function (name, value, path) {
		return get_path(path).setParam(name, value).href;
	},
	
	// del path param
	deleteParam: function (name, path) {
		return get_path(path).deleteParam(name).href;
	},
	
	// del all hash param
	clearParam: function (path) {
		return get_path(path).clearParam().href;
	},
	
	// get hash param
	getHash: function (name, path) {
		return get_path(path).getHash(name);
	},
	
	// set hash param
	setHash: function (name, value, path) {
		return get_path(path).setHash(name, value).href;
	},
	
	// del hash param
	deleteHash: function (name, path) {
		return get_path(path).deleteHash(name).href;
	},
	
	// del all hash param
	clearHash: function (path) {
		return get_path(path).clearHash().href;
	},
	
	// relative path
	relative: function (path, target) {
		if (arguments.length > 1) 
			return get_path(path).relative(target);
		else 
			return get_path().relative(path);
	},
	
});
