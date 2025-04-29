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

'use strict';

/// <reference path="./_ext.d.ts" />

const _init = __binding__('_init');
const _fs = __binding__('_fs');
const PREFIX = 'file:///';
let   config: Dict | null = null;
const options: Optopns = _init.options;  // start options
const debug = ['debug', 'inspect', 'inspect_brk'].some(e=>e in options);
export const mainPath = options.__main__ || '';

if ('url_arg' in options) {
	options.url_arg = options.url_arg.replace(/\s/g, '&');
} else {
	options.url_arg = '';
}
if ('no_cache' in options) {
	if (options.url_arg) {
		options.url_arg += '&__no_cache';
	} else {
		options.url_arg = '__no_cache';
	}
}

export const executable = _fs.executable as ()=>string;
export const documents = _fs.documents as (path?: string)=>string;
export const temp = _fs.temp as (path?: string)=>string;
export const resources = _fs.resources as (path?: string)=>string;
export const cwd = _fs.cwd as ()=>string;
export const chdir = _fs.chdir as (path: string)=>boolean;
export type Optopns = Dict<string>;
export const timeMonotonic = _init.timeMonotonic as ()=>number;

let setTimer_ = (globalThis as any).setTimer;
let setTimeout_ = globalThis.setTimeout;
let setInterval_ = globalThis.setInterval;
let setImmediate_ = globalThis.setImmediate;

function setTimeout<A extends any[]>(cb: (...args: A)=>void, timeout?: number, ...args: A): any {
	return setTimeout_(args.length ? ()=>cb(...args): cb, timeout);
}
function setInterval<A extends any[]>(cb: (...args: A)=>void, timeout?: number, ...args: A): any {
	return setInterval_(args.length ? ()=>cb(...args): cb, timeout);
}
function setImmediate<A extends any[]>(cb: (...args: A)=>void, ...args: A): any {
	return setImmediate_(args.length ? ()=>cb(...args): cb as any);
}
globalThis.setTimeout = setTimeout as any;
globalThis.setInterval = setInterval as any;
globalThis.setImmediate = setImmediate as any;

export function setTimer<A extends any[]>(cb: (...args: A)=>void, timeout?: number, repeat?: number, ...args: A): any {
	return setTimer_(args.length ? ()=>cb(...args): cb, timeout, repeat);
}
export const clearTimer = (globalThis as any).clearTimer as (id?: any)=>void;

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

const isWin32 = _init.platform == 'win32';
const paths = isWin32 ? win32: posix;

export const classicPath = paths.classicPath;
export const delimiter = paths.delimiter;

/** 
 * format part
 */
export function normalizePath(path: string, retain_up?: boolean) {
	let ls = path.split('/');
	let rev = [];
	let up = 0;
	for (let i = ls.length - 1; i > -1; i--) {
		let v = ls[i];
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
 * file:///home/louis/test.txt
 * file:///d:/home/louis/test.txt
 * http://google.com/test.txt
 * return format path
 */
export function formatPath(...args: string[]) {
	let path = paths.joinPath(args);
	let prefix = '';
	// Find absolute path
	let mat = path.match(paths.resolve);
	let slash = '';
	// resolve: /^((\/|[a-z]:)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	// resolve: /^((\/)|([a-z]{2,}:\/\/[^\/]+)|((file|zip):\/\/\/))/i,
	if (mat) {
		if (mat[2]) { // local absolute path /
			if (isWin32 && mat[2] != '/') { // windows d:\
				prefix = PREFIX + mat[2] + '/';
				path = path.substring(2);
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
		let _cwd = cwd();
		if (isWin32) {
			prefix += _cwd.substring(0,10) + '/'; // 'file:///d:/';
			path = _cwd.substring(11) + '/' + path;
		} else {
			prefix = PREFIX; // 'file:///';
			path = _cwd.substring(8) + '/' + path;
		}
	}
	path = normalizePath(path);
	return path ? prefix + slash + path : prefix;
}

/**
 * @method isAbsolute # 是否为绝对路径
 */
export function isAbsolute(path: string) {
	return paths.isAbsolute.test(path);
}

/**
 * @method isLocal # 是否为本地路径
 */
export function isLocal(path: string) {
	return paths.isLocal.test(path);
}

export function isLocalZip(path: string) {
	return /^zip:\/\/\//i.test(path);
}

export function isHttp(path: string) {
	return /^(https?):\/\/[^\/]+/i.test(path);
}

/**
 * Remove byte order marker. This catches EF BB BF (the UTF-8 BOM)
 * because the buffer-to-string conversion in `fs.readFileSync()`
 * translates it to FEFF, the UTF-16 BOM.
 */
export function stripBOM(content: string) {
	if (content.charCodeAt(0) === 0xFEFF)
		content = content.slice(1);
	return content;
}

// without error
function requireWithoutErr(pathname: string) {
	let _pkg = __binding__('_pkg');
	try { return _pkg._load(pathname) } catch(e) {}
}

function readConfigFile(pathname: string, pathname2: string) {
	let c = requireWithoutErr(pathname);
	let c2 = requireWithoutErr(pathname2);
	if (c || c2) {
		return Object.assign({}, c, c2);
	}
}

export function getConfig(): Dict {
	if (!config) {
		let cfg: Dict | null = null;
		let mod = __binding__('_pkg').mainModule;
		if (mod) {
			let pkg = mod.package;
			if (pkg) {
				cfg = readConfigFile(pkg.path + '/.config', pkg.path + '/config');
			} else {
				cfg = readConfigFile(mod.dirname + '/.config', mod.dirname + '/config');
			}
		}
		config = cfg || readConfigFile(_fs.cwd() + '/.config', _fs.cwd() + '/config') || {};
	}
	return config as Dict;
}

/**
 * Find end of shebang line and slice it off
 */
export function stripShebang(content: string) {
	// Remove shebang
	let contLen = content.length;
	if (contLen >= 2) {
		if (content.charCodeAt(0) === 35/*#*/ &&
				content.charCodeAt(1) === 33/*!*/) {
			if (contLen === 2) {
				// Exact match
				content = '';
			} else {
				// Find end of shebang line and slice it off
				let i = 2;
				for (; i < contLen; ++i) {
					let code = content.charCodeAt(i);
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

export function assert(value: any, message?: string) {
	if (!value) {
		throw new Error('assert fail, ' + (message || ''));
	}
}

export function debugLog(TAG = 'PKG') {
	return debug ? function(...args: any[]) {
		if (args.length > 1) {
			let str = args.shift();
			for (let arg of args) {
				str = str.replace(/\%(j|s|d)/, arg);
			}
			console.log(TAG, str);
		}
	}: function() {};
}

export declare class Hash5381 {
	hashCode(): number;
	update(data: string | Uint8Array): void;
	digest(): string;
	clear(): void;
}

exports.Hash5381 = _init.Hash5381;

// ------------------------------------------------------------------------------------------------

type Platform = 'darwin' | 'android' | 'linux' | 'win32';

let _exiting = false;

function nextTick<A extends any[], R>(cb: (...args: A) => R, ...args: A): void {
	_init.nextTick(function(){ cb(...args) });
}

function sleep<T = number>(time: number, defaultValue?: T): Promise<T> {
	return new Promise((ok, err)=>setTimeout(()=>ok((defaultValue || 0) as any), time));
}

function unrealized() {
	throw new Error('Unrealized function');
}

function exit(code?: number) {
	if (!_exiting) {
		_exiting = true;
		_init.exit(code || 0);
	}
}

// set runtime event listener, 'UncaughtException'|'UnhandledRejection'|'BeforeExit'|'Exit'
export function __listener__(name: string, handle: any) {
	if (name == 'Exit') {
		_init[`__on${name}_native`] = function(...args: any[]) {
			_exiting = true;
			handle(...args);
		};
	} else {
		_init[`__on${name}_native`] = handle;
	}
}

export default {
	debug,
	version: _init.version as ()=>string,
	platform: _init.platform as Platform,
	isQuark: true, isNode: false, isWeb: false,
	webFlags: null,
	argv: _init.argv as string[],
	options,
	nextTick, unrealized, exit, sleep,
	gc: _init.garbageCollection as ()=>void,
	runScript: _init.runScript as (source: string, name?: string, sandbox?: any)=>any,
	hashCode: _init.hashCode as (obj: any)=>number,
	hash: _init.hash as (obj: any)=>string,
};