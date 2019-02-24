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

const _path = requireNative('_path');
const _util = requireNative('_util');
const win32 = _util.platform == 'win32';
const { readFileSync, isFileSync } = requireNative('_reader');
const { haveNode } = _util;

const fallbackPath = win32 ? function(url) {
	return url.replace(/^file:\/\/(\/([a-z]:))?/i, '$3').replace(/\//g, '\\');
} : function(url) {
	return url.replace(/^file:\/\//i, '');
};

const join_path = win32 ? function(args) {
	for (var i = 0, ls = []; i < args.length; i++) {
		var item = args[i];
		if (item) ls.push(item.replace(/\\/g, '/'));
	}
	return ls.join('/');
}: function(args) {
	for (var i = 0, ls = []; i < args.length; i++) {
		var item = args[i];
		if (item) ls.push(item);
	}
	return ls.join('/');
};

const matchs = win32 ? {
	resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	isAbsolute: /^([\/\\]|[a-z]:|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	isLocal: /^([\/\\]|[a-z]:|(file|zip):\/\/\/)/i,
} : {
	resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	isAbsolute: /^(\/|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	isLocal: /^(\/|(file|zip):\/\/\/)/i,
};

if (!haveNode) {
	const _console = requireNative('_console');
	const _timer = requireNative('_timer');
	global.console = _console;

	function setTimeout(cb, timeout, ...args) {
		if (typeof cb != 'function') {
			throw TypeError('"callback" argument must be a function');
		}
		if (args.length) {
			return _timer.setTimeout(e=>cb(...args), timeout);
		} else {
			return _timer.setTimeout(cb, timeout);
		}
	}

	function setInterval(cb, timeout, ...args) {
		if (typeof cb != 'function') {
			throw TypeError('"callback" argument must be a function');
		}
		if (args.length) {
			return _timer.setInterval(e=>cb(...args), timeout);
		} else {
			return _timer.setInterval(cb, timeout);
		}
	}

	global.setTimeout = setTimeout;
	global.setInterval = setInterval;
	global.clearTimeout = _timer.clearTimeout;
	global.clearInterval = _timer.clearInterval;
	global.setImmediate = (cb, ...args)=>setTimeout(cb, 0, ...args);
	global.clearImmediate = _timer.clearTimeout;
}

/** 
 * format part 
 */
function resolvePathLevel(path, retain_up) {
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

const PREFIX = 'file:///';

/**
 * return format path
 */
function resolve() {
	var path = join_path(arguments);
	var prefix = '';
	// Find absolute path
	var mat = path.match(matchs.resolve);
	var slash = '';
	
	// resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	// resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	
	if (mat) {
		if (mat[2]) { // local absolute path /
			if (win32 && mat[2] != '/') { // windows d:\
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
			if (prefix == path.length) // file:///
				return prefix;
			path = path.substr(prefix.length);
		}
	} else { // Relative path, no network protocol
		var cwd = _path.cwd();
		if (win32) {
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
function isAbsolute(path) {
	return matchs.isAbsolute.test(path);
}

/**
 * @func isLocal # 是否为本地路径
 */
function isLocal(path) {
	return matchs.isLocal.test(path);
}

function isLocalZip(path) {
	return /^zip:\/\/\//i.test(path);
}

function isNetwork(path) {
	return /^(https?):\/\/[^\/]+/i.test(path);
}

function resolveMainPath(path) {
	if (path) {
		if ( !isAbsolute(path) ) {
			// 非绝对路径,优先查找资源路径
			if (isFileSync(_path.resources(path + '/package.json'))) {
				// 如果在资源中找到`package.json`文件
				path = _path.resources(path);
			}
		}
		path = fallbackPath(resolve(path));
	}
	return path;
}

function makeRequireFunction(mod) {
	const Module = mod.constructor;

	function require(path) {
		return mod.require(path);
	}

	function resolve(request, options) {
		return Module._resolveFilename(request, mod, false, options);
	}

	require.resolve = resolve;

	function paths(request) {
		return Module._resolveLookupPaths(request, mod, true);
	}

	resolve.paths = paths;

	require.main = mod.package.mainModule;

	// Enable support to add extra extension types.
	require.extensions = Module._extensions;

	require.cache = Module._cache;

	return require;
}

/**
 * Find end of shebang line and slice it off
 */
function stripShebang(content) {
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

function assert(value, message) {
	if (!value) {
		throw new Error('assert fail, ' + (message || ''));
	}
}

function debug(TAG = 'PKG') {
	return function(...args) {
		if (exports.__dev) {
			if (args.length > 1) {
				var str = args.shift();
				for (var arg of args) {
					str = str.replace(/\%(j|s)/, arg);
				}
				console.log(TAG, str);
			}
		}
	}
}

/**
 * Remove byte order marker. This catches EF BB BF (the UTF-8 BOM)
 * because the buffer-to-string conversion in `fs.readFileSync()`
 * translates it to FEFF, the UTF-16 BOM.
 */
function stripBOM(content) {
	if (content.charCodeAt(0) === 0xFEFF) {
		content = content.slice(1);
	}
	return content;
}

Object.assign(exports, {
	fallbackPath,
	resolvePathLevel,
	resolve,
	isAbsolute,
	isLocal,
	isLocalZip,
	isNetwork,
	resolveMainPath,
	readFileSync,
	makeRequireFunction,
	stripShebang,
	stripBOM,
	assert, debug,
});