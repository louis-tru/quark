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
 * 
 * URL Processing Tool Type
 * 
 * @example
 * 
 * ```ts
 * // cwd: file:///var/data
 * // Prints: file:///var/data/index.js
 * var uri = new URL('index.js');
 * console.log(uri.href);
 * // Prints: http://quarks.cc/index.html?args=0
 * var uri2 = new URL('http://quarks.cc/home/../index.html?args=0')
 * console.log(uri2.href);
 * // Prints: 
 * // Error: Parse uri error, Illegal URL
 * new URL('http://quarks.cc:').href
 * ```
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
	 * @method constructor(path?:string)
	 */
	constructor(path: string = '') {
		if (!path && isWeb) {
			path = location.href;
		}
		this._value = path;
	}

	/**
	 * Get the complete URL, including parameters
	 * 
	 * href: "http://xxxx.xxx:81/v1.1.0/quark/path.js?sasasas&asasasa#sdsdsd"
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: http://quarks.cc/
	 * console.log(new URL('http://quarks.cc/').href);
	 * ```
	 */
	get href(): string {
		parse_path(this);
		return this._origin + this._filename + this._search + this._hash;
	}

	/**
	 * Get full path name
	 * 
	 * filename: "/D:/Documents/test.js"
	 * 
	 * @example
	 *
	 * ```ts
	 * // Prints: /aaa/bbbb/ccc/test.js
	 * console.log(new URL('http://quarks.cc/aaa/bbbb/ccc/test.js').filename);
	 * ```
	*/
	get filename(): string {
		parse_path(this);
		return this._filename;
	}

	/**
	 * @get path:string /a/b/s/test.html?aaaaa=100
	 * 
	 * Get the path and include the parameter part
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: /aaa/bbbb/ccc/test.js?asas=asas
	 * console.log(new URL('http://quarks.cc/aaa/bbbb/ccc/test.js?asas=asas').path);
	 * ```
	 */
	get path(): string {
		parse_path(this);
		return this._filename + this._search;
	}

	/**
	 * Full path dir
	 * 
	 * dirname: "/D:/Documents"
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: /aaa/bbbb/ccc
	 * console.log(new URL('http://quarks.cc/aaa/bbbb/ccc/test.js').dirname);
	 * ```
	 */
	get dirname(): string {
		parse_path(this);
		return this._dirname;
	}

	/**
	 * Get url query parameters
	 * 
	 * search: "?sasasas&asasasa"
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: ?a=A&b=B
	 * console.log(new URL('http://quarks.cc/?a=A&b=B').search);
	 * ```
	*/
	get search(): string {
		init(this);
		return this._search;
	}

	/**
	 * hash: "#sdsdsd"
	 * 
	 * Get hash parameters
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: #c=C&d=D
	 * console.log(new URL('http://quarks.cc/?a=A&b=B#c=C&d=D').hash);
	 * ```
	 */
	get hash(): string {
		init(this);
		return this._hash;
	}

	/**
	 * host: "quarks.cc:81"
	 * 
	 * Gets the host name and returns the host name with the port number
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: quarks.cc:80
	 * console.log(new URL('http://quarks.cc:81/').host);
	 * ```
	 */
	get host(): string {
		parse_path(this);
		return this._hostname + (this._port ? ':' + this._port : '');
	}

	/**
	 * hostname: "quarks.cc"
	 * 
	 * Gets the host name, but does not return the port number
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: quarks.cc
	 * console.log(new URL('http://quarks.cc:81/').host);
	 * ```
	 */
	get hostname(): string {
		parse_path(this);
		return this._hostname;
	}

	/**
	 * origin: "http://quarks.cc:81"
	 * 
	 * Get the uri origin, protocol+host
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: http://quarks.cc:81
	 * console.log(new URL('http://quarks.cc:81/host/index.html').host);
	 * // Prints: file://
	 * console.log(new URL('file:///var/data/index.html').host);
	 * ```
	 */
	get origin(): string {
		parse_path(this);
		return this._origin;
	}

	/**
	 * Get path base name
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: index.html
	 * console.log(new URL('file:///var/data/index.html').basename);
	 * ```
	 */
	get basename(): string {
		parse_basename(this);
		return this._basename;
	}
	
	/**
	 * Get path extname
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: .html
	 * console.log(new URL('file:///var/data/index.html').extname);
	 * ```
	 */
	get extname(): string {
		parse_basename(this);
		return this._extname;
	}

	/**
	 * port: "81"
	 * 
	 * Get the host port number.
	 * If the port number is not defined in the URL, an empty string is returned.
	 * 
	 * @example
	 *
	 * ```ts
	 * // Prints: 81
	 * console.log(new URL('http://quarks.cc:81').port);
	 * // Prints If there is no port number, an empty string will be returned: ""
	 * console.log(new URL('http://quarks.cc').port);
	 * ```
	 */
	get port(): string {
		parse_path(this);
		return this._port;
	}

	/**
	 * Get the protocol type string of the URL, For: `'http:'`|`'https'`|`'ftp:'`
	 */
	get protocol(): string {
		parse_path(this);
		return this._protocol;
	}

	/**
	 * Returns a collection of query parameters as an object, or set
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints:
	 * // {
	 * //   a: "100",
	 * //   b: "test"
	 * // }
	 * console.log(new URL('http://quarks.cc/?a=100&b=test').params);
	 * ```
	*/
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

	set params(value: Dict<string>) {
		init(this);
		this._params = {...value};
		this._search = querystringStringify('?', this._params);
	}

	/**
	 * Returns a Hash parameter set as an object, or set
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints:
	 * // {
	 * //   a: "200",
	 * //   b: "300"
	 * // }
	 * console.log(new URL('http://quarks.cc/#a=200&b=300').hashParams);
	 * ```
	*/
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

	set hashParams(value: Dict<string>) {
		init(this);
		this._hashParams = {...value};
		this._hash = querystringStringify('#', this._hashParams);
	}

	/**
	 * Get path param
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: ok
	 * console.log(new URL('http://quarks.cc/?args=ok').getParam('args'));
	 * ```
	 */
	getParam(name: string): string {
		return this.params[name];
	}

	/**
	 * Set the URL query parameter key-value pair and return self
	 */
	setParam(name: string, value: string): this {
		this.params[name] = value || '';
		this._search = querystringStringify('?', this._params);
		return this;
	}

	/**
	 * Remove URL query parameters by name
	 */
	deleteParam(name: string): this {
		delete this.params[name];
		this._search = querystringStringify('?', this._params);
		return this;
	}

	/**
	 * Delete all of params in the URL
	 */
	clearParams(): this {
		init(this);
		this._params = {};
		this._search = '';
		return this;
	}

	/**
	 * Get hash param by name
	 */
	getHash(name: string): string {
		return this.hashParams[name];
	}

	/**
	 * Set hash param by the key/value
	*/
	setHash(name: string, value: string): this {
		this.hashParams[name] = value || '';
		this._hash = querystringStringify('#', this._hashParams);
		return this;
	}

	/**
	 * Delete hash param by the name
	 */
	deleteHash(name: string): this {
		delete this.hashParams[name];
		this._hash = querystringStringify('#', this._hashParams);
		return this;
	}

	/**
	 * Delete all of hash params in the URL
	 */
	clearHashs(): this {
		init(this);
		this._hashParams = {};
		this._hash = '';
		return this;
	}

	/**
	 * Get relative path from the fromPath to self
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Prints: ../A/B/C/test.js
	 * var url = new URL('http://quarks.cc/A/B/C/test.js');
	 * console.log(url.relative('http://quarks.cc/home/'));
	 * // Prints: file:///var/data/A/B/C/test.js
	 * var url2 = new URL('file:///var/data/A/B/C/test.js');
	 * console.log(url2.relative('http://quarks.cc/home/'));
	 * ```
	 */
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

	/**
	*/
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

/**
 * @default
*/
export default {
	URL: URL,

	/**
	 * Get the binary executable file path of the current application
	 * @method executable()string
	 * @example
	 * 
	 * ```ts
	 * // Prints:
	 * // file:///var/containers/Bundle/Application/4F1BD659-601D-4932-8484-D0D1F978F0BE/test.app/test
	 * console.log(path.executable());
	 * ```
	*/
	executable: _util.executable,

	/**
	 * Get the document storage path of the current application
	 * @method documents(path?:string)string
	 * @param path? Append to the document path
	 * @example
	 * 
	 * ```ts
	 * // Prints:
	 * // file:///var/mobile/Containers/Data/Application/89A576FE-7BB9-4F26-A456-E9D7F8AD053D/Documents
	 * console.log(path.documents());
	 * // Prints Setting the result of appending path parameters:
	 * // file:///var/mobile/Containers/Data/Application/89A576FE-7BB9-4F26-A456-E9D7F8AD053D/Documents/aa.jpeg
	 * console.log(path.documents('aa.jpeg'));
	 * ```
	*/
	documents: _util.documents,

	/**
	 * Get the application temporary directory
	 * @method temp(path?:string)string
	*/
	temp: _util.temp,

	/**
	 * Get the application resource directory
	 * @method resources(path?:string)string
	*/
	resources: _util.resources,

	/**
	 * Get the current working directory
	 * @method cwd()string
	*/
	cwd: _util.cwd,

	/**
	 * Set the current working directory and return `true` if successful
	 * @method chdir(path:string)boolean
	*/
	chdir: _util.chdir,

	/**
	 * Format part path to normalize path
	 * @method normalizePath(path:string,retain_up?:boolean)string
	*/
	normalizePath: _util.normalizePath,
	delimiter: _util.delimiter,

	/**
	 * Restore the path to a path that the operating system can recognize.
	 * Generally, you do not need to call this function unless you directly call
	 * a Native/C/C++ function that is not provided by `Quark`
	 * @method classicPath(path:string)string
	 * @example
	 * 
	 * ```ts
	 * // Prints: /var/data/test.js
	 * console.log(path.normalizePath('file:///var/data/test.js'));
	 * ```
	 */
	classicPath: _util.classicPath,

	/**
	 * Format path to standard absolute path
	 * @method resolve(path:string,partPath?string)string
	 * @example
	 * 
	 * ```ts
	 * // Prints: http://quarks.cc/A/C/test.js
	 * console.log(path.resolve('http://quarks.cc/home', "..", "A", "B", "..", "C", "test.js"));
	 * // Prints: 
	 * // true
	 * // file:///var/data/aaa/cc/ddd/kk.jpg
	 * console.log(path.chdir('/var/data'));
	 * console.log(path.resolve('aaa/bbb/../cc/.//!<ddd/kk.jpg'));
	 * ```
	 */
	resolve: _util.formatPath, // func

	/**
	 * Test whether it is an absolute path
	 * @method isAbsolute(path:string)boolean
	 * @example
	 * 
	 * ```ts
	 * // Prints:
	 * // true
	 * // true
	 * // false
	 * console.log(path.isAbsolute('/var/kk'));
	 * console.log(path.isAbsolute('http://quarks.cc/'));
	 * console.log(path.isAbsolute('index.jsx'));
	 * ```
	 */
	isAbsolute: _util.isAbsolute, // func

	/**
	 * Get basename
	 */
	basename(path?: string): string {
		return get_path(path).basename;
	},

	/**
	 * Get dirname
	 */
	dirname(path?: string): string {
		return get_path(path).dirname;
	},

	/**
	 * Get extname
	 */
	extname(path?: string): string {
		return get_path(path).extname;
	},

	/**
	 * Get filename
	 */
	filename(path?: string): string {
		return get_path(path).filename;
	},

	/**
	 * Get path
	 */
	path(path?: string): string {
		return get_path(path).path;
	},

	/** Get search */
	search(path?: string): string {
		return get_path(path).search;
	},

	/** Get hash */
	hash(path?: string): string {
		return get_path(path).hash;
	},

	/** Get host */
	host(path?: string): string {
		return get_path(path).host;
	},

	/** Get hostname */
	hostname(path?: string): string {
		return get_path(path).hostname;
	},

	/** Get origin */
	origin(path?: string): string {
		return get_path(path).origin;
	},

	/** Get port: "81" */
	port(path?: string): string {
		return get_path(path).port;
	},
	
	/** Get protocol: "http:" */
	protocol(path?: string): string {
		return get_path(path).protocol;
	},

	/** Get params */
	params(path?: string): Dict<string> {
		return get_path(path).params;
	},

	/** Get params  */
	hashParams(path?: string): Dict<string> {
		return get_path(path).hashParams;
	},

	/** Get path param */
	getParam(name: string, path?: string): string {
		return get_path(path).getParam(name);
	},

	/** Set path param */
	setParam(name: string, value: string, path?: string): string {
		return get_path(path).setParam(name, value).href;
	},

	/** Delete path param */
	deleteParam(name: string, path?: string): string {
		return get_path(path).deleteParam(name).href;
	},

	/** Delete all of hash params */
	clearParams(path?: string): string {
		return get_path(path).clearParams().href;
	},

	/** Get hash param */
	getHash(name: string, path?: string): string {
		return get_path(path).getHash(name);
	},

	/** Set hash param */
	setHash(name: string, value: string, path?: string): string {
		return get_path(path).setHash(name, value).href;
	},

	/** Delete hash param */
	deleteHash(name: string, path?: string): string {
		return get_path(path).deleteHash(name).href;
	},

	/** Delete all of hash params */
	clearHashs(path?: string): string {
		return get_path(path).clearHashs().href;
	},

	/** Get relative path */
	relative(from: string, target?: string): string {
		return get_path(target).relative(from);
	},

}
