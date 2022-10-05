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

import './_ext';
import {Event, Notification, EventNoticer} from './event';

const _util = __require__('_util');

type Platform = 'darwin' | 'linux' | 'win32' | 'android';

function nextTick<A extends any[], R>(cb: (...args: A) => R, ...args: A): void {
	if (typeof cb != 'function')
		throw new Error('callback must be a function');
	_util.nextTick(function(){ cb(...args) });
}

function unrealized() {
	throw new Error('Unrealized function');
}

export declare class SimpleHash {
	hashCode(): number;
	update(data: string | Uint8Array): void;
	digest(): string;
	clear(): void;
}

exports.SimpleHash = _util.SimpleHash;

const _nodeProcess = (globalThis as any).process

class Process extends Notification {

	private _exiting = false;

	private _handles = {
		BeforeExit: (noticer: EventNoticer, code = 0)=>{
			noticer.trigger(code);
			return code;
		},
		Exit: (noticer: EventNoticer, code = 0)=>{
			this._exiting = true;
			noticer.trigger(code);
			return code;
		},
		UncaughtException: (noticer: EventNoticer, err: Error)=>{
			return noticer.length ? (noticer.trigger(err), true): false;
		},
		UnhandledRejection: (noticer: EventNoticer, reason: Error, promise: Promise<any>)=>{
			return noticer.length ? (noticer.trigger({ reason, promise }), true): false;
		},
	};

	getNoticer(name: 'BeforeExit'|'Exit'|'UncaughtException'|'UnhandledRejection') {
		if (!this.hasNoticer(name)) {
			var noticer = super.getNoticer(name);
			var handle = (this._handles as any)[name];
			if (handle) {
				if (_util.haveNode) {
					_nodeProcess.on(name.substr(0, 1).toLowerCase() + name.substr(1), function(...args: any[]) {
						return handle(noticer, ...args);
					});
				}
				_util[`__on${name}_native`] = function(...args: any[]) {
					return handle(noticer, ...args);
				};
			}
		}
		return super.getNoticer(name);
	}

	exit(code?: number) {
		if (!this._exiting) {
			this._exiting = true;
			if (_util.haveNode) {
				_nodeProcess._exiting = true;
				if (code || code === 0)
					_nodeProcess.exitCode = code;
				try {
					_nodeProcess.emit('exit', _nodeProcess.exitCode || 0);
				} catch(err) {
					console.error(err);
				}
			}
			_util._exit(code || 0);
		}
	}

}

export const _process = new Process();

export default {
	version: _util.version as ()=>string,
	addNativeEventListener: _util.addNativeEventListener as (target: Object, event: string, fn: (event: Event<any>)=>void, id?: number)=>boolean,
	removeNativeEventListener: _util.removeNativeEventListener as (target: Object, event: string, id?: number)=>boolean,
	gc: _util.garbageCollection as ()=>void,
	runScript: _util.runScript as (source: string, name?: string, sandbox?: any)=>any,
	hashCode: _util.hashCode as (obj: any)=>number,
	hash: _util.hash as (obj: any)=>string,
	nextTick: nextTick,
	platform: _util.platform as Platform,
	haveNode: _util.haveNode as boolean,
	haveQuark: true,
	haveWeb: false,
	argv: _util.argv as string[],
	webFlags: null,
	exit: (code?: number)=>{ _process.exit(code) },
	unrealized: unrealized,
	// hash
	SimpleHash: _util.SimpleHash as typeof SimpleHash,
};
