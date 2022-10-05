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

import _path from './_path';
import _pkgutil from './_pkgutil';

const haveWeb = typeof globalThis.window == 'object';

function split_path(self: any) {
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

function parse_base_ext_name(self: any) {
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

function parse_path(self: any) {
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

function parse_params(self: any) {
	if (self._params) 
		return;
	split_path(self);
	
	var params: Dict = self._params = {};
	
	if (self._search[0] != '?') 
		return;
		
	var ls = self._search.substr(1).split('&');
	
	for (var i = 0; i < ls.length; i++) {
		var o = ls[i].split('=');
		params[ o[0] ] = decodeURIComponent(o[1] || '');
	}
}

function parse_hash_params(self: any) {
	if (self._hash_params) 
		return;
	split_path(self);
	
	var params: Dict = self._hash_params = {};
	if (self._hash[0] != '#') 
		return;
		
	var ls = self._hash.substr(1).split('&');
	
	for (var i = 0; i < ls.length; i++) {
		var o = ls[i].split('=');
		params[ o[0] ] = decodeURIComponent(o[1] || '');
	}
}

function querystringStringify(prefix: string, params: Dict) {
	var rev = [];
	for (var i in params) {
		rev.push(i + '=' + encodeURIComponent(params[i]));
	}
	return rev.length ? prefix + rev.join('&') : '';
}

/**
 * @class URL
 */
export class URL {
	
	/**
		* @arg [path] {String}
		* @constructor
		*/
	constructor(path: string = '') {
		if (!path && haveWeb) {
			path = location.href;
		}
		(<any>this)._value = path;
	}
	
	// href: "http://xxxx.xxx:81/v1.1.0/quark/path.js?sasasas&asasasa#sdsdsd"
	get href(): string {
		parse_path(this);
		return (<any>this)._origin + (<any>this)._filename + (<any>this)._search + (<any>this)._hash;
	}
	
	/**
		* full path
		* filename: "/D:/Documents/test.js"
		*/
	get filename(): string {
		parse_path(this);
		return  (<any>this)._filename;
	}
	
	/**
	 * @get path /a/b/s/test.html?aaaaa=100
	 */
	get path(): string {
		parse_path(this);
		return (<any>this)._filename + (<any>this)._search;
	}
	
	/**
		* full path dir
		* dirname: "/D:/Documents"
		*/
	get dirname(): string {
		parse_path(this);
		return (<any>this)._dirname;
	}
	
	// search: "?sasasas&asasasa"
	get search(): string {
		split_path(this);
		return (<any>this)._search;
	}
	
	// hash: "#sdsdsd"
	get hash(): string {
		split_path(this);
		return (<any>this)._hash;
	}
	
	// host: "quarks.cc:81"
	get host(): string {
		parse_path(this);
		return (<any>this)._hostname + ((<any>this)._port ? ':' + (<any>this)._port : '');
	}
	
	// hostname: "quarks.cc"
	get hostname(): string {
		parse_path(this);
		return (<any>this)._hostname;
	}
	
	// origin: "http://quarks.cc:81"
	get origin(): string {
		parse_path(this);
		return (<any>this)._origin;
	}

	// get path base name 
	get basename(): string {
		parse_base_ext_name(this);
		return (<any>this)._basename;
	}
	
	// path extname
	get extname(): string {
		parse_base_ext_name(this);
		return (<any>this)._extname;
	}
	
	// port: "81"
	get port(): string {
		parse_path(this);
		return (<any>this)._port;
	}
	
	// protocol: "http:"
	get protocol(): string {
		parse_path(this);
		return (<any>this)._protocol;
	}

	get params(): Dict<string> {
		parse_params(this);
		return (<any>this)._params;
	}

	set params(value: Dict<string>) {
		(<any>this)._params = {...value};
		(<any>this)._search = querystringStringify('?', (<any>this)._params);
	}
	
	get hashParams(): Dict<string> {
		parse_hash_params(this);
		return (<any>this)._hash_params;
	}

	set hashParams(value: Dict<string>){
		(<any>this)._hash_params = {...value};
		(<any>this)._hash = querystringStringify('#', (<any>this)._hash_params);
	}

	// get path param
	getParam(name: string): string {
		return (<any>this).params[name];
	}

	// set path param
	setParam(name: string, value: string): URL {
		this.params[name] = value || '';
		(<any>this)._search = querystringStringify('?', (<any>this)._params);
		return this;
	}
	
	// del path param
	deleteParam(name: string): URL {
		delete this.params[name];
		(<any>this)._search = querystringStringify('?', (<any>this)._params);
		return this;
	}
	
	// del all prams
	clearParam(): URL {
		(<any>this)._params = {};
		(<any>this)._search = '';
		return this;
	}
	
	// get hash param
	getHash(name: string): string {
		return this.hashParams[name];
	}
	
	// set hash param
	setHash(name: string, value: string): URL {
		this.hashParams[name] = value || '';
		(<any>this)._hash = querystringStringify('#', (<any>this)._hash_params);
		return this;
	}
	
	// del hash param
	deleteHash(name: string): URL {
		delete this.hashParams[name];
		(<any>this)._hash = querystringStringify('#', (<any>this)._hash_params);
		return this;
	}
	
	// del hash all params
	clearHash(): URL {
		(this as any)._hash_params = {};
		(this as any)._hash = '';
		return this;
	}
	
	// relative path
	relative(targetPath: string): string {
		var target = new URL(targetPath);
		if ( this.origin != target.origin )
			return (this as any)._origin + (this as any)._filename;
		var ls: string[]  = (this as any)._filename == '/' ? [] : (this as any)._filename.split('/');
		var ls2: string[] = (target as any)._filename == '/' ? [] : (target as any)._filename.split('/');
		var len = Math.max(ls.length, ls2.length);
		
		for (var i = 1; i < len; i++) {
			if (ls[i] != ls2[i]) {
				len = ls.length - i;
				if (len > 0) {
					ls = [];
					for (var j = 0; j < len; j++)
						ls.push('..');
					return ls.join('/') + '/' + ls2.splice(i).join('/');
				}
				return ls2.splice(i).join('/');
			}
		}
		return '.';
	}

	toJSON(): string {
		return this.href;
	}
}

(URL as any).prototype._is_split = false;
(URL as any).prototype._is_parse = false;
(URL as any).prototype._value = '';
(URL as any).prototype._hostname = '';
(URL as any).prototype._port = '';
(URL as any).prototype._protocol = '';
(URL as any).prototype._search = '';
(URL as any).prototype._hash = '';
(URL as any).prototype._origin = '';
(URL as any).prototype._filename = '';
(URL as any).prototype._dirname = '';
(URL as any).prototype._basename = -1;
(URL as any).prototype._extname = -1;
(URL as any).prototype._params = null;
(URL as any).prototype._hash_params = null;

function get_path(path?: string): URL {
	return new URL(path);
}

export default {

	..._path,

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
	 * @func fallbackPath()
	 */
	fallbackPath: _pkgutil.fallbackPath,

	/**
	 * full filename
	 */
	basename(path?: string) {
		return get_path(path).basename;
	},

	/**
	 * full filename
	 */
	dirname(path?: string) {
		return get_path(path).dirname;
	},

	/**
	 * full filename
	 */
	extname(path?: string) {
		return get_path(path).extname;
	},

	/**
	 * full filename
	 */
	filename(path?: string) {
		return get_path(path).filename;
	},

	/**
	 * full path
	 */
	path(path?: string) {
		return get_path(path).path;
	},
	
	search(path?: string) {
		return get_path(path).search;
	},
	
	hash(path?: string) {
		return get_path(path).hash;
	},
	
	host(path?: string) {
		return get_path(path).host;
	},
	
	hostname(path?: string) {
		return get_path(path).hostname;
	},
	
	// href origin
	origin(path?: string) {
		return get_path(path).origin;
	},
	
	// port: "81"
	port(path?: string) {
		return get_path(path).port;
	},
	
	// protocol: "http:"
	protocol(path?: string) {
		return get_path(path).protocol;
	},
	
	// href params
	params(path?: string) {
		return get_path(path).params;
	},
	
	// hash params 
	hashParams(path?: string) {
		return get_path(path).hashParams;
	},
	
	// get path param
	getParam(name: string, path?: string) {
		return get_path(path).getParam(name);
	},
	
	// set path param
	setParam(name: string, value: string, path?: string) {
		return get_path(path).setParam(name, value).href;
	},
	
	// del path param
	deleteParam(name: string, path?: string) {
		return get_path(path).deleteParam(name).href;
	},
	
	// del all hash param
	clearParam(path?: string) {
		return get_path(path).clearParam().href;
	},
	
	// get hash param
	getHash(name: string, path?: string) {
		return get_path(path).getHash(name);
	},
	
	// set hash param
	setHash(name: string, value: string, path?: string) {
		return get_path(path).setHash(name, value).href;
	},
	
	// del hash param
	deleteHash(name: string, path?: string) {
		return get_path(path).deleteHash(name).href;
	},
	
	// del all hash param
	clearHash(path?: string) {
		return get_path(path).clearHash().href;
	},
	
	// relative path
	relative(path: string, target: string) {
		if (arguments.length > 1) 
			return get_path(path).relative(target);
		else 
			return get_path(path).relative(path);
	},
	
}
