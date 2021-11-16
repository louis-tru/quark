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

'use strict';

const _path = __require__('_path');
const _util = __require__('_util');
const { haveNode } = _util;
const PREFIX = 'file:///';
const _cwd = _path.cwd;
const _debug = _util.debug as boolean;
const options: Optopns = {};  // start options
var config: Dict | null = null;
var mainPath: string = '';

export type Optopns = Dict<string|string[]>;

if (!haveNode) {
	const _console = __require__('_console');
	const _timer = __require__('_timer');

	globalThis.console = _console;

	function setTimeout<A extends any[]>(cb: (...args: A)=>void, timeout?: number, ...args: A): any {
		if (typeof cb != 'function') {
			throw TypeError('"callback" argument must be a function');
		}
		if (args.length) {
			return _timer.setTimeout(()=>cb(...args), timeout || 0);
		} else {
			return _timer.setTimeout(cb, timeout || 0);
		}
	}

	function setInterval<A extends any[]>(cb: (...args: A)=>void, timeout?: number, ...args: A): any {
		if (typeof cb != 'function') {
			throw TypeError('"callback" argument must be a function');
		}
		if (args.length) {
			return _timer.setInterval(()=>cb(...args), timeout || 0);
		} else {
			return _timer.setInterval(cb, timeout || 0);
		}
	}

	globalThis.setTimeout = setTimeout as any;
	globalThis.setInterval = setInterval as any;
	globalThis.clearTimeout = _timer.clearTimeout;
	globalThis.clearInterval = _timer.clearInterval;
	globalThis.setImmediate = <A extends any[]>(cb: (...args: A)=>void, ...args: A)=>setTimeout(cb, 0, ...args);
	globalThis.clearImmediate = _timer.clearTimeout;
}

const win32 = {
	fallbackPath: function(url: string) {
		return url.replace(/^file:\/\//i, '').replace(/^\/([a-z]:)?/i, '$1').replace(/\//g, '\\');
	},
	joinPath: function(args: any[]) {
		for (var i = 0, ls = []; i < args.length; i++) {
			var item = args[i];
			if (item) ls.push(item.replace(/\\/g, '/'));
		}
		return ls.join('/');
	},
	resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	isAbsolute: /^([\/\\]|[a-z]:|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	isLocal: /^([\/\\]|[a-z]:|(file|zip):\/\/\/)/i,
	delimiter: ';',
};

const posix = {
	fallbackPath: function(url: string) {
		return url.replace(/^file:\/\//i, '');
	},
	joinPath: function(args: any[]) {
		for (var i = 0, ls = []; i < args.length; i++) {
			var item = args[i];
			if (item) ls.push(item);
		}
		return ls.join('/');
	},
	resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	isAbsolute: /^(\/|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	isLocal: /^(\/|(file|zip):\/\/\/)/i,
	delimiter: ':',
};

const isWin32 = _util.platform == 'win32';
const utils = isWin32 ? win32: posix;

/** 
 * format part 
 */
function resolvePathLevel(path: string, retain_up?: boolean) {
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

	return (retain_up ? new Array(up + 1).join('../') + path : path);
}

/**
 * return format path
 */
function resolve(...args: string[]) {
	var path = utils.joinPath(args);
	var prefix = '';
	// Find absolute path
	var mat = path.match(utils.resolve);
	var slash = '';

	// resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	// resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,

	if (mat) {
		if (mat[2]) { // local absolute path /
			if (isWin32 && mat[2] != '/') { // windows d:\
				prefix = PREFIX + mat[2] + '/';
				path = path.substr(2);
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
			path = path.substr(prefix.length);
		}
	} else { // Relative path, no network protocol
		var cwd = _cwd();
		if (isWin32) {
			prefix += cwd.substr(0,10) + '/'; // 'file:///d:/';
			path = cwd.substr(11) + '/' + path;
		} else {
			prefix = PREFIX; // 'file:///';
			path = cwd.substr(8) + '/' + path;
		}
	}

	path = resolvePathLevel(path);

	return path ? prefix + slash + path : prefix;
}

/**
 * @func isAbsolute # 是否为绝对路径
 */
function isAbsolute(path: string) {
	return utils.isAbsolute.test(path);
}

/**
 * @func isLocal # 是否为本地路径
 */
function isLocal(path: string) {
	return utils.isLocal.test(path);
}

function isLocalZip(path: string) {
	return /^zip:\/\/\//i.test(path);
}

function isNetwork(path: string) {
	return /^(https?):\/\/[^\/]+/i.test(path);
}

/**
 * Remove byte order marker. This catches EF BB BF (the UTF-8 BOM)
 * because the buffer-to-string conversion in `fs.readFileSync()`
 * translates it to FEFF, the UTF-16 BOM.
 */
function stripBOM(content: string) {
	if (content.charCodeAt(0) === 0xFEFF) {
		content = content.slice(1);
	}
	return content;
}

function parseOptions(args: string[], options: Optopns) {
	for (var i = 0; i < args.length; i++) {
		var item = args[i];
		var mat = item.match(/^-{1,2}([^=]+)(?:=(.*))?$/);
		if (mat) {
			var name = mat[1].replace(/-/gm, '_');
			var val = mat[2] || 'true';
			var raw_val = options[name];
			if ( raw_val ) {
				if ( Array.isArray(raw_val) ) {
					raw_val.push(val);
				} else {
					options[name] = [raw_val, val];
				}
			} else {
				options[name] = val;
			}
		}
	}
}

// without error
function requireWithoutErr(pathname: string) {
	var _pkg = __require__('_pkg');
	try { return _pkg._load(pathname) } catch(e) {}
}

function readConfigFile(pathname: string, pathname2: string) {
	var c = requireWithoutErr(pathname);
	var c2 = requireWithoutErr(pathname2);
	if (c || c2) {
		return Object.assign({}, c, c2);
	}
}

function getConfig(): Dict {
	if (!config) {
		var cfg: Dict | null = null;
		var mod = __require__('_pkg').mainModule;
		if (mod) {
			var pkg = mod.package;
			if (pkg) {
				cfg = readConfigFile(pkg.path + '/.config', pkg.path + '/config');
			} else {
				cfg = readConfigFile(mod.dirname + '/.config', mod.dirname + '/config');
			}
		}
		config = cfg || readConfigFile(_path.cwd() + '/.config', _path.cwd() + '/config') || {};
	}
	return config as Dict;
}

function initArgv() {
	var args: string[] = [];
	if (_util.argv.length > 1) {
		mainPath = String(_util.argv[1] || '');
		args = _util.argv.slice(2);
	}
	parseOptions(args, options); // parse options

	if ( 'url_arg' in options ) {
		if (Array.isArray(options.url_arg))
			options.url_arg = options.url_arg.join('&');
	} else {
		options.url_arg = '';
	}
	if ('no_cache' in options || _debug) {
		if (options.url_arg) {
			options.url_arg += '&__no_cache';
		} else {
			options.url_arg = '__no_cache';
		}
	}
}

function makeRequireFunction(mod: any, main: any): NodeRequire {
	const Module = mod.constructor;

	function require(path: string) {
		return mod.require(path);
	}

	function resolve(request: string, options?: { paths?: string[]; }) {
		return Module._resolveFilename(request, mod, false, options);
	}

	require.resolve = resolve;

	function paths(request: string) {
		return Module._resolveLookupPaths(request, mod, true);
	}

	resolve.paths = paths;

	require.main = main;

	// Enable support to add extra extension types.
	require.extensions = Module._extensions;

	require.cache = Module._cache;

	return require;
}

/**
 * Find end of shebang line and slice it off
 */
function stripShebang(content: string) {
	// Remove shebang
	var contLen = content.length;
	if (contLen >= 2) {
		if (content.charCodeAt(0) === 35/*#*/ &&
				content.charCodeAt(1) === 33/*!*/) {
			if (contLen === 2) {
				// Exact match
				content = '';
			} else {
				// Find end of shebang line and slice it off
				var i = 2;
				for (; i < contLen; ++i) {
					var code = content.charCodeAt(i);
					if (code === 10/*\n*/ || code === 13/*\r*/)
						break;
				}
				if (i === contLen)
					content = '';
				else {
					// Note that this actually includes the newline character(s) in the
					// new output. This duplicates the behavior of the regular expression
					// that was previously used to replace the shebang line
					content = content.slice(i);
				}
			}
		}
	}
	return content;
}

function assert(value: any, message?: string) {
	if (!value) {
		throw new Error('assert fail, ' + (message || ''));
	}
}

function debugLog(TAG = 'PKG') {
	return function(...args: any[]) {
		if (_util.debug) {
			if (args.length > 1) {
				var str = args.shift();
				for (var arg of args) {
					str = str.replace(/\%(j|s|d)/, arg);
				}
				console.log(TAG, str);
			}
		}
	}
}

initArgv();

export default {
	fallbackPath: utils.fallbackPath,
	resolvePathLevel,
	resolve,
	isAbsolute,
	isLocal,
	isLocalZip,
	isNetwork,
	get options() { return options },
	get config() { return getConfig() },
	debug: _debug,
	//
	stripBOM,
	makeRequireFunction,
	stripShebang,
	assert,
	debugLog,
	mainPath,
	delimiter: utils.delimiter,
};