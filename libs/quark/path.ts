/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

import * as _util from './_util';

const isWeb = _util.default.isWeb;

function init(self: any) {
	if (self._is_init) return;
	self._is_init = true;

	let value = self._value;
	let val = '';
	let i = value.indexOf('?');

	if (i != -1) {
		val = value.substr(0, i);
		let search = self._search = value.substr(i);
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
	self._value = _util.formatPath(val);
}

function parse_path(self: any) {
	if (self._is_parse) return;
	self._is_parse = true;
	init(self);

	let mat = self._value.match(/^(([a-z]+:)\/\/)?(([^\/:]+)(?::(\d+))?)?(\/.*)?/);
	if (mat) {
		self._protocol = mat[2] || '';
		self._origin = mat[1] || ''; // file://

		if ( mat[3] ) { // http:// or ftp:// or lib://
			self._origin += mat[3]; // http://quarks.cc
			self._hostname = mat[4];
			self._port = mat[5] || '';
		}

		let path = self._filename = mat[6] || '/';
		let i = path.lastIndexOf('/');
		if (i > 0) {
			self._dirname = path.substr(0, i);
		} else {
			self._dirname = '/';
		}
	} else {
		throw new Error(`Parse url error, Illegal URL ${self._value}`);
	}
}

function parse_basename(self: any) {
	if (self._basename != -1) return;
	init(self);
	let mat = self._value.match(/([^\/\\]+)?(\.[^\/\\\.]+)$|[^\/\\]+$/);
	if (mat) {
		self._basename = mat[0];
		self._extname = mat[2] || '';
	} else {
		self._extname = self._basename = '';
	}
}

function querystringStringify(prefix: string, params: Dict) {
	let rev = [];
	for (let i in params) {
		rev.push(i + '=' + encodeURIComponent(params[i]));
	}
	return rev.length ? prefix + rev.join('&') : '';
}

/**
 * @class URL
 */
export class URL {
	private _value: string;
	private _origin: string;
	private _filename: string;
	private _dirname: string;
	private _basename: string;
	private _extname: string;
	private _search: string;
	private _hash: string;
	private _hostname: string;
	private _port: string;
	private _protocol: string;
	private _params: Dict<string>;
	private _hashParams: Dict<string>;

	/**
		* @param [path] {String}
		* @constructor
		*/
	constructor(path: string = '') {
		if (!path && isWeb) {
			path = location.href;
		}
		this._value = path;
	}

	// href: "http://xxxx.xxx:81/v1.1.0/quark/path.js?sasasas&asasasa#sdsdsd"
	get href(): string {
		parse_path(this);
		return this._origin + this._filename + this._search + this._hash;
	}

	/**
		* full path
		* filename: "/D:/Documents/test.js"
		*/
	get filename(): string {
		parse_path(this);
		return this._filename;
	}

	/**
	 * @get path /a/b/s/test.html?aaaaa=100
	 */
	get path(): string {
		parse_path(this);
		return this._filename + this._search;
	}

	/**
		* full path dir
		* dirname: "/D:/Documents"
		*/
	get dirname(): string {
		parse_path(this);
		return this._dirname;
	}

	// search: "?sasasas&asasasa"
	get search(): string {
		init(this);
		return this._search;
	}

	// hash: "#sdsdsd"
	get hash(): string {
		init(this);
		return this._hash;
	}

	// host: "quarks.cc:81"
	get host(): string {
		parse_path(this);
		return this._hostname + (this._port ? ':' + this._port : '');
	}

	// hostname: "quarks.cc"
	get hostname(): string {
		parse_path(this);
		return this._hostname;
	}

	// origin: "http://quarks.cc:81"
	get origin(): string {
		parse_path(this);
		return this._origin;
	}

	// get path base name 
	get basename(): string {
		parse_basename(this);
		return this._basename;
	}
	
	// path extname
	get extname(): string {
		parse_basename(this);
		return this._extname;
	}
	
	// port: "81"
	get port(): string {
		parse_path(this);
		return this._port;
	}
	
	// protocol: "http:"
	get protocol(): string {
		parse_path(this);
		return this._protocol;
	}

	get params(): Dict<string> {
		if (this._params)
			return this._params;
		init(this);
		this._params = {};
		if (this._search[0] != '?') {
			return this._params;
		}

		let ls = this._search.substring(1).split('&');

		for (let i = 0; i < ls.length; i++) {
			let o = ls[i].split('=');
			this._params[ o[0] ] = decodeURIComponent(o[1] || '');
		}
		return this._params;
	}

	get hashParams(): Dict<string> {
		if (this._hashParams)
			return this._hashParams;
		init(this);
		this._hashParams = {};
		if (this._hash[0] != '#') {
			return this._hashParams;
		}

		let ls = this._hash.substring(1).split('&');

		for (let i = 0; i < ls.length; i++) {
			let o = ls[i].split('=');
			this._hashParams[ o[0] ] = decodeURIComponent(o[1] || '');
		}
		return this._hashParams;
	}

	set params(value: Dict<string>) {
		init(this);
		this._params = {...value};
		this._search = querystringStringify('?', this._params);
	}

	set hashParams(value: Dict<string>) {
		init(this);
		this._hashParams = {...value};
		this._hash = querystringStringify('#', this._hashParams);
	}

	// get path param
	getParam(name: string): string {
		return this.params[name];
	}

	// set path param
	setParam(name: string, value: string): URL {
		this.params[name] = value || '';
		this._search = querystringStringify('?', this._params);
		return this;
	}

	// del path param
	deleteParam(name: string): URL {
		delete this.params[name];
		this._search = querystringStringify('?', this._params);
		return this;
	}

	// del all prams
	clearParam(): URL {
		init(this);
		this._params = {};
		this._search = '';
		return this;
	}

	// get hash param
	getHash(name: string): string {
		return this.hashParams[name];
	}

	// set hash param
	setHash(name: string, value: string): URL {
		this.hashParams[name] = value || '';
		this._hash = querystringStringify('#', this._hashParams);
		return this;
	}

	// del hash param
	deleteHash(name: string): URL {
		delete this.hashParams[name];
		this._hash = querystringStringify('#', this._hashParams);
		return this;
	}

	// del hash all params
	clearHash(): URL {
		init(this);
		this._hashParams = {};
		this._hash = '';
		return this;
	}

	// relative path
	relative(fromPath: string): string {
		let from = new URL(fromPath);
		if ( this.origin != from.origin ) {
			return this._origin + this._filename;
		}

		let fr: string[] = from._filename == '/' ? [] : from._filename.split('/').slice(0,-1);
		let to: string[] = this._filename == '/' ? [] : this._filename.split('/'); // to
		let len = Math.max(fr.length, to.length);

		for (let i = 1; i < len; i++) {
			if (fr[i] != to[i]) {
				len = fr.length - i;
				if (len > 0) {
					fr = [];
					for (let j = 0; j < len; j++)
						fr.push('..');
					return fr.join('/') + (i < to.length ? '/'+to.slice(i).join('/'): '');
				}
				return to.slice(i).join('/');
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
(URL as any).prototype._hashParams = null;

function get_path(path?: string): URL {
	return new URL(path);
}

export default {
	URL: URL,

	executable: _util.executable,
	documents: _util.documents,
	temp: _util.temp,
	resources: _util.resources,
	cwd: _util.cwd,
	chdir: _util.chdir,
	normalizePath: _util.normalizePath,
	delimiter: _util.delimiter,

	/**
	 * @method classicPath()
	 */
	classicPath: _util.classicPath,

	/**
	 * @method resolve(path) resolve path 
	 */
	resolve: _util.formatPath, // func

	/** 
	 * @method isAbsolute(path) is absolute path
	 */
	isAbsolute: _util.isAbsolute, // func

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
	relative(from: string, target?: string) {
		return get_path(target).relative(from);
	},

}
