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

'use strict';

/// <reference path="./_ext.d.ts" />

const _init = __binding__('_init');
const _fs = __binding__('_fs');
let   config: Dict | null = null;
const options: Optopns = _init.options;  // start options
const debug = ['debug', 'inspect', 'inspect_brk'].some(e=>e in options);
export const mainPath = options.__main__ || '';

import type {Uint,Int} from './defs';

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
globalThis.queueMicrotask = _init.nextTick;

export function setTimer<A extends any[]>(cb: (...args: A)=>void, timeout?: number, repeat?: number, ...args: A): any {
	return setTimer_(args.length ? ()=>cb(...args): cb, timeout, repeat);
}
export const clearTimer = (globalThis as any).clearTimer as (id?: any)=>void;

/**
 * Remove byte order marker. This catches EF BB BF (the UTF-8 BOM)
 * because the buffer-to-string conversion in `fs.readFileSync()`
 * translates it to FEFF, the UTF-16 BOM.
 */
export function stripBOM(content: string): string {
	if (content.charCodeAt(0) === 0xFEFF)
		content = content.slice(1);
	return content;
}

// without error
function requireWithoutErr(pathname: string) {
	let Module = __binding__('_pkg').Module;
	try { return Module.load(pathname) } catch(e) {}
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
		let mod = __binding__('_pkg').default.mainModule;
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
export function stripShebang(content: string): string {
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

/**
 * Asserts that value is not null or undefined.
*/
export function assert(value: any, message?: string) {
	if (!value) {
		throw new Error('assert fail, ' + (message || ''));
	}
}

export function debugLog(TAG = 'PKG') {
	return debug ? function(...args: any[]) {
		console.log(TAG, ...args);
	}: function() {};
}

/***/
export declare class Hash5381 {
	hashCode(): Int; //!<
	update(data: string | Uint8Array): void; //!<
	updatefv2(f1: number, f2: number): void; //!<
	equals(other: Hash5381): boolean; //!<
	digest(): string; //!<
	clear(): void; //!<
}

exports.Hash5381 = _init.Hash5381;

// ------------------------------------------------------------------------------------------------

type Platform = 'darwin' | 'android' | 'linux' | 'win32'; //!<

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

// Set runtime event listener, 'UncaughtException'|'UnhandledRejection'|'BeforeExit'|'Exit'
export function __setListenerHook__(name: string, handle: any) {
	if (name == 'Exit') {
		_init[`__on${name}_native`] = function(...args: any[]) {
			_exiting = true;
			return handle(...args);
		};
	} else {
		_init[`__on${name}_native`] = handle;
	}
}

const String_hashCode = String.prototype.hashCode;
const Buffer_hashCode = (Uint8Array as any).prototype.__proto__.hashCode;
if (!(globalThis as any)._jscRunning) {
	// Extend Object prototype
	String.prototype.hashCode = function() {
		if (this.length > 256) {
			// Use native hashCode for long strings to improve performance
			return _init.hashCode(this);
		} else {
			return String_hashCode.call(this);
		}
	};
}
// Extend Uint8Array prototype
(Uint8Array as any).prototype.__proto__.hashCode = function() {
	if (this.length > 256) {
		// Use native hashCode for long buffers to improve performance
		return _init.hashCode(this);
	} else {
		return Buffer_hashCode.call(this);
	}
}

/**
 * @default
*/
export default {

	/**
	 * @get debug:boolean
	 * 
	 * It is on debug status, specify via command line parameters
	 */
	debug,

	/**
	 * @method version()string
	 * 
	 * Get `quark` framework version
	*/
	version: _init.version as ()=>string,

	/**
	 * @get platform:Platform
	 * 
	 * Get platform type
	*/
	platform: _init.platform as Platform,

	isQuark: true,
	isNode: false,
	isWeb: false,
	webFlags: null,

	/**
	 * @get argv:string[]
	 * 
	 * Command line startup parameter
	*/
	argv: _init.argv as string[],

	/**
	 * @get options:object
	 * 
	 * Command line startup parameter parsing results
	*/
	options,

	/**
	 * Wait for the next tick of the message loop to call the callback function
	 * @method nextTick(cb:Function)
	*/
	nextTick, unrealized, exit,

	/**
	 * Asynchronous sleep, equivalent to calling `setTimeout`
	 * @method sleep(time:Uint):Promise
	*/
	sleep,

	/**
	 * @method gc()
	 * 
	 * Manually let the JavaScript engine perform `GC` actions
	*/
	gc: _init.garbageCollection as ()=>void,

	/**
	 * @method runScript(source,name?,sandbox?)any
	 * 
	 * Compile and run a piece of javascript code and return the running result.
	 * 	You can specify a name and a running context object.
	 *
	 * The name is very useful when debugging code or program exceptions. It is usually a file name.
	 *
	 * If you do not pass this sandbox `sandbox` context, the default is to use the `global` object.
	 *
	 * @param source:string javascript source code
	 * @param name?:string
	 * @param sandbox?:object
	*/
	runScript: _init.runScript as (source: string, name?: string, sandbox?: any)=>any,

	/**
	 * @method hashCode(obj:any)Uint
	 * 
	 * Read the hash value of a data object
	*/
	hashCode: _init.hashCode as (obj: any)=>Uint,

	/**
	 * @method hash(obj:any)string
	 * 
	 * Read the hash value of a data object, and convert the value to a string
	*/
	hash: _init.hash as (obj: any)=>string,

	/**
	 * @method debugLog(TAG?:string)Function
	*/
	debugLog: debugLog,
};