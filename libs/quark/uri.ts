/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

const PREFIX = 'file:///';
const isQuark: boolean = !!globalThis.__binding__;
const isNode: boolean = !!(globalThis as any).process;
const isWeb: boolean = !!globalThis.document;

function unrealized(): any {
	throw new Error('Unrealized function');
}

let _win32: boolean = false;
let _executable: ()=>string = unrealized;
let _documents: (path?: string)=>string = unrealized;
let _temp: (path?: string)=>string = unrealized;
let _resources: (path?: string)=>string = unrealized;
let _cwd: ()=>string = unrealized;
let _chdir: (path: string)=>boolean = unrealized;

if (isQuark) {
	const _fs = __binding__('_fs');
	_win32 = __binding__('_init').platform == 'win32';
	_executable = _fs.executable;
	_documents = _fs.documents;
	_temp = _fs.temp;
	_resources = _fs.resources;
	_cwd = _fs.cwd;
	_chdir = _fs.chdir;
} else if (isNode) {
	const process = (globalThis as any).process;
	_win32 = process.platform == 'win32';
	_cwd = _win32 ? function() {
		return PREFIX + process.cwd().replace(/\\/g, '/');
	}: function() {
		return PREFIX + process.cwd().substring(1);
	};
	_chdir = function(path) {
		path = classicPath(path);
		process.chdir(path);
		return process.cwd() == path;
	};
} else if (isWeb) {
	let dirname = location.pathname.substring(0, location.pathname.lastIndexOf('/'));
	let cwdPath = location.origin + dirname;
	_cwd = function() { return cwdPath };
	_chdir = function() { return false };
} else {
	throw new Error('no support');
}

const win32 = {
	classicPath: function(url: string) {
		return url.replace(/^file:\/\//i, '').replace(/^\/([a-z]:)?/i, '$1').replace(/\//g, '\\');
	},
	joinPath: function(args: any[]) {
		let ls = [];
		for (let i = 0; i < args.length; i++) {
			let item = args[i];
			if (item)
				ls.push(item.replace(/\\/g, '/'));
		}
		return ls.join('/');
	},
	resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	isAbsolute: /^([\/\\]|[a-z]:|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	isLocal: /^([\/\\]|[a-z]:|(file|zip):\/\/\/)/i,
	delimiter: ';',
};

const posix = {
	classicPath: function(url: string) {
		return url.replace(/^file:\/\//i, '');
	},
	joinPath: function(args: any[]) {
		let ls = [];
		for (let i = 0; i < args.length; i++) {
			let item = args[i];
			if (item)
				ls.push(item);
		}
		return ls.join('/');
	},
	resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	isAbsolute: /^(\/|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	isLocal: /^(\/|(file|zip):\/\/\/)/i,
	delimiter: ':',
};

const paths = _win32 ? win32: posix;

/**
 * get current working directory
 * @method cwd()string
*/
export const cwd = _cwd;

/**
 * convert to classic path, for: file:///d:/test/path.js => d:\test\path.js
 * @method classicPath(path:string)string
 */
export const classicPath = paths.classicPath;

/**
 * path delimiter
 * @const delimiter:string
*/
export const delimiter = paths.delimiter;

/**
 * normalizePath
 * @member normalizePath(path:string,retainUp?:boolean)string
 */
export function normalizePath(path: string, retainUp: boolean = false): string {
	var ls = path.split('/');
	var rev = [];
	var up = 0;
	for (var i = ls.length - 1; i > -1; i--) {
		var v = ls[i];
		if (v && v != '.') {
			if (v == '..') // set up
				up++;
			else if (up === 0) // no up item
				rev.push(v);
			else // un up
				up--;
		}
	}
	path = rev.reverse().join('/');

	return (retainUp ? new Array(up + 1).join('../') + path : path);
}

/**
 * file:///home/louis/test.txt
 * file:///d:/home/louis/test.txt
 * http://google.com/test.txt
 * return format path
 * @method resolve(path:string,partPath?string)string
 */
export function resolve(...args: string[]): string {
	let path = paths.joinPath(args);
	let prefix = '';
	// Find absolute path
	let mat = path.match(paths.resolve);
	let slash = '';
	// resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	// resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	if (mat) {
		if (mat[2]) { // local absolute path /
			if (_win32 && mat[2] != '/') { // windows d:\
				prefix = PREFIX + mat[2] + '/';
				path = path.substring(2);
			} else if (isWeb) {
				prefix = origin + '/';
			} else {
				prefix = PREFIX; //'file:///';
			}
		} else {
			if (mat[4]) { // local file protocol
				prefix = mat[4];
			} else { // network protocol
				prefix = mat[0];
				slash = '/';
			}
			// if (prefix == path.length)
			if (prefix == path) // file:///
				return prefix;
			path = path.substring(prefix.length);
		}
	} else { // Relative path, no network protocol
		let cwd = _cwd();
		if (_win32) {
			prefix += cwd.substring(0,10) + '/'; // 'file:///d:/';
			path = cwd.substring(11) + '/' + path;
		} else if (isWeb) {
			prefix = origin + '/';
			path = cwd.substring(prefix.length) + '/' + path;
		} else {
			prefix = PREFIX; // 'file:///';
			path = cwd.substring(8) + '/' + path;
		}
	}
	path = normalizePath(path);
	return path ? prefix + slash + path : prefix;
}

/**
 * Is it an absolute path?
 */
export function isAbsolute(path: string): boolean {
	return paths.isAbsolute.test(path);
}

/**
 * Is it a local path?
 */
export function isLocal(path: string): boolean {
	return paths.isLocal.test(path);
}

export function isLocalZip(path: string): boolean { //!<
	return /^zip:\/\/\//i.test(path);
}

export function isHttp(path: string): boolean { //!<
	return /^(https?):\/\/[^\/]+/i.test(path);
}

function init_uri(self: any) {
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
	self._value = resolve(val);
}

function parse_path(self: any) {
	if (self._is_parse) return;
	self._is_parse = true;
	init_uri(self);

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
	if (self._basename != -1)
		return;
	init_uri(self);
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
	private _origin: string = '';
	private _filename: string = '';
	private _dirname: string = '';
	private _basename: string | -1 = -1;
	private _extname: string | -1 = -1;
	private _search: string = '';
	private _hash: string = '';
	private _hostname: string = '';
	private _port: string = '';
	private _protocol: string = '';
	private _params: Dict<string> | null = null;
	private _hashParams: Dict<string> | null = null;

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
		init_uri(this);
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
		init_uri(this);
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
		return this._basename as string;
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
		return this._extname as string;
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
		init_uri(this);
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
		init_uri(this);
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
		init_uri(this);
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
		init_uri(this);
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
		this._search = querystringStringify('?', this.params);
		return this;
	}

	/**
	 * Remove URL query parameters by name
	 */
	deleteParam(name: string): this {
		delete this.params[name];
		this._search = querystringStringify('?', this.params);
		return this;
	}

	/**
	 * Delete all of params in the URL
	 */
	clearParams(): this {
		init_uri(this);
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
		this._hash = querystringStringify('#', this.hashParams);
		return this;
	}

	/**
	 * Delete hash param by the name
	 */
	deleteHash(name: string): this {
		delete this.hashParams[name];
		this._hash = querystringStringify('#', this.hashParams);
		return this;
	}

	/**
	 * Delete all of hash params in the URL
	 */
	clearHashs(): this {
		init_uri(this);
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

function get_path(path?: string): URL {
	return new URL(path);
}

/**
 * @default
*/
export default {
	URL: URL,

	/**
	 * Get the executable file path
	 * @method executable()string
	*/
	executable: _executable,

	/**
	 * Get the documents directory path
	 * @method documents(path?:string)string
	*/
	documents: _documents,

	/**
	 * Get the temporary directory path
	 * @method temp(path?:string)string
	*/
	temp: _temp,

	/**
	 * Get the resources directory path
	 * @method resources(path?:string)string
	*/
	resources: _resources,

	/**
	 * Get the current working directory
	 * @method cwd()string
	*/
	cwd: _cwd,

	/**
	 * Set the current working directory and return `true` if successful
	 * @method chdir(path:string)boolean
	*/
	chdir: _chdir,

	/**
	 * Format part path to normalize path
	 * @method normalizePath(path:string,retain_up?:boolean)string
	*/
	normalizePath: normalizePath,

	/**
	 * Path delimiter
	 * @get delimiter:string
	*/
	delimiter: delimiter,

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
	classicPath: classicPath,

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
	resolve: resolve, // func

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
	isAbsolute: isAbsolute, // func

	/**
	 * full filename
	 */
	basename(path?: string) {
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
