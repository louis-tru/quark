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

var keys = require('./_keys');
var path = require('path');
var fs = require('fs');
var options = {};  // start options
var ignore_local_package;var ignore_all_local_package;
var config = null;
var win32 = process.platform == 'win32';

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
	is_absolute: /^([\/\\]|[a-z]:|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	is_local: /^([\/\\]|[a-z]:|(file|zip):\/\/\/)/i,
} : {
	resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	is_absolute: /^(\/|[a-z]{2,}:\/\/[^\/]+|(file|zip):\/\/\/)/i,
	is_local: /^(\/|(file|zip):\/\/\/)/i,
};

/** 
 * format part 
 */
function resolve_path_level(path, retain_up) {
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

const PREFIX = (win32 ? process.cwd().match(/^([a-z]:)/i)[1] : '') + '/';

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
				prefix = /*PREFIX+*/ mat[2] + '/';
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
		var cwd = process.cwd();
		prefix = PREFIX; // 'file:///';
		path = (win32 ? cwd.substr(3) : cwd) + '/' + path;
	}

	path = resolve_path_level(path);

	return path ? prefix + slash + path : prefix;
}

/**
 * @func is_absolute # 是否为绝对路径
 */
function is_absolute(path) {
	return matchs.is_absolute.test(path);
}

/**
 * @func is_local # 是否为本地路径
 */
function is_local(path) {
	return matchs.is_local.test(path);
}

function is_local_zip(path) {
	return /^zip:\/\/\//i.test(path);
}

function is_network(path) {
	return /^(https?):\/\/[^\/]+/i.test(path);
}

function extendEntries(obj, extd) {
	for (var item of Object.entries(extd)) {
		obj[item[0]] = item[1];
	}
	return obj;
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

require('module').Module._extensions['.keys'] = function(module, filename) {
	var content = fs.readFileSync(filename, 'utf8');
	try {
		module.exports = keys.parse(stripBOM(content));
	} catch (err) {
		err.message = filename + ': ' + err.message;
		throw err;
	}
};

function parse_argv() {
	var args = process.argv.slice(2);
	
	for (var i = 0; i < args.length; i++) {
		var item = args[i];
		var mat = item.match(/^-{1,2}([^=]+)(?:=(.*))?$/);
		if (mat) {
			var name = mat[1].replace(/-/gm, '_');
			var val = mat[2] || 1;
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

	// options.dev = _util.dev;

	if (process.execArgv.some(s=>(s+'').indexOf('--inspect') == 0) || 
			process.argv.some(s=>(s+'').indexOf('--inspect') == 0)) {
		options.dev = 1;
	}

	/*_pkgutil.dev = */options.dev = !!options.dev;
	
	if ( !('url_arg' in options) ) {
		options.url_arg = '';
	}

	if ('no_cache' in options || options.dev) {
		if (options.url_arg) {
			options.url_arg += '&__nocache';
		} else {
			options.url_arg = '__nocache';
		}
	}

	ignore_local_package = options.ignore_local || [];
	ignore_all_local_package = false;
	if ( typeof ignore_local_package == 'string' ) {
		if ( ignore_local_package == '' || ignore_local_package == '*' ) {
			ignore_all_local_package = true;
		} else {
			ignore_local_package = [ ignore_local_package ];
		}
	} else {
		ignore_all_local_package = ignore_local_package.indexOf('*') != -1;
	}
}

parse_argv();

function read_config(pathname) {
	try {
		return require(pathname);
	} catch(e) {}
}

module.exports = {
	resolve: resolve, 				// func pkg
	isAbsolute: is_absolute, 	// func pkg
	isLocal: is_local,				// 
	isLocalZip: is_local_zip,
	isNetwork: is_network,
	extendEntries: extendEntries,
	get options() {
		return options;
	},
	get config() {
		if (!config) {
			config = {};
			var mainModule = process.mainModule;
			if (mainModule) {
				config = 
					read_config(path.dirname(mainModule.filename) + '/config') ||
					read_config(path.dirname(mainModule.filename) + '/.config') ||
					read_config(process.cwd() + '/config') ||
					read_config(process.cwd() + '/.config') || {};
			} else {
				config = 
					read_config(process.cwd() + '/config') || 
					read_config(process.cwd() + '/.config') || {};
			}
		}
		return config;
	},
};
