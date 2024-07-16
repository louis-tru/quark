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

import {List} from './_event';
import errno from './errno';

let id = 10;
let AsyncFunctionConstructor = (async function() {}).constructor;
let scopeLockQueue = new Map();

export let currentTimezone = new Date().getTimezoneOffset() / -60; // 当前时区

export function isAsync(func: any): boolean {
	return func && func.constructor === AsyncFunctionConstructor;
}

class obj_constructor {}

function clone_object(new_obj: any, obj: any): any {
	for (let name of Object.getOwnPropertyNames(obj)) {
		let property = Object.getOwnPropertyDescriptor(obj, name)!;
		if (property.writable) {
			new_obj[name] = clone(property.value);
		}
	}
	return new_obj;
}

export function getId() {
	return id++;
}

/**
 * @method clone # 克隆一个Object对像
 * @param obj {Object} # 要复制的Object对像
 * @param {Object}
 */
export function clone(obj: any): any {
	if (obj && typeof obj == 'object') {
		let new_obj: any = null, i;

		switch (obj.constructor) {
			case Object:
				new_obj = {};
				for(i in obj) {
					new_obj[i] = clone(obj[i]);
				}
				return new_obj;
			case Array:
				new_obj = [ ];
				for (i = 0; i < obj.length; i++) {
					new_obj[i] = clone(obj[i]);
				}
				return new_obj;
			case Date:
				return new Date(obj.valueOf());
			default:
				obj_constructor.prototype = obj.constructor.prototype;
				new_obj = new obj_constructor();
				return clone_object(new_obj, obj);
		}
	}
	return obj;
}

/**
 * @method extend (obj, extd, top?)
 */
export function extend(obj: any, extd: any, top: any = Object.prototype): any {
	if (extd.__proto__ && extd.__proto__ !== top)
		extend(obj, extd.__proto__, top);
	for (let i of Object.getOwnPropertyNames(extd)) {
		if (i != 'constructor') {
			let descriptor = Object.getOwnPropertyDescriptor(extd, i)!;
			descriptor.enumerable = false;
			Object.defineProperty(obj, i, descriptor);
		}
	}
	return obj;
}

/**
 * Empty function
 */
export function noop() {}

/**
 * @method isNull(value)
 */
export function isNull(value: any): boolean {
	return value === null || value === undefined
}

/**
 * @method extendClass #  EXT class prototype objects
 */
export function extendClass(cls: Function, extds: Function[] | Function, top = Object.prototype) {
	for (let extd of Array.isArray(extds)? extds: [extds]) {
		if (extd instanceof Function) {
			extd = extd.prototype;
		}
		extend(cls.prototype, extd, top);
	}
	return cls;
}

async function scopeLockDequeue(mutex: any): Promise<void> {
	let item, queue = scopeLockQueue.get(mutex);
	while( item = queue.shift() ) {
		try {
			item.resolve(await item.cb());
		} catch(err) {
			item.reject(err);
		}
	}
	scopeLockQueue.delete(mutex);
}

/**
 * @method scopeLock (mutex, cb)
 */
export function scopeLock<R>(mutex: any, cb: ()=>Promise<R>|R): Promise<R> {
	assert(mutex, 'Bad argument');
	assert(typeof cb == 'function', 'Bad argument');
	return new Promise<R>((resolve, reject)=>{
		if (scopeLockQueue.has(mutex)) {
			scopeLockQueue.get(mutex).push({resolve, reject, cb});
		} else {
			scopeLockQueue.set(mutex, new List().pushBack({resolve, reject, cb}).host);
			scopeLockDequeue(mutex); // dequeue
		}
	})
}

/**
 * @method get(name[,self]) # get object value by name
 * @param name {String} 
 * @param [self] {Object}
 * @return {Object}
 */
export function getProp(name: string, self: any): any {
	let names = name.split('.');
	for ( let item of names ) {
		self = self[item];
		if (!self)
			return self;
	}
	return self;
}

/**
* @method set(name,value[,self]) # Setting object value by name
* @param name {String} 
* @param value {Object} 
* @param [self] {Object}
* @return {Object}
*/
export function setProp(name: string, value: any, self: any): any {
	let names = name.split('.');
	let last = names.pop()!;
	for ( let item of names ) {
		self = self[item];
		if (!self)
			self = self[item] = {};
	}
	self[last] = value;
	return self;
}

/**
 * @method def(name[,self]) # Delete object value by name
 * @param name {String} 
 * @param [self] {Object}
 */
export function removeProp(name: string, self: any): void {
	let names = name.split('.');
	let last = names.pop()!;
	self = getProp(names.join('.'), self);
	if (self)
		delete self[last];
}

/**
 * @method random # 创建随机数字
 * @param [start] {Number} # 开始位置
 * @param [end] {Number}   # 结束位置
 * @return {Number}
 */
export function random(start: number = 0, end: number = 1E8): number {
	if (start == end)
		return start;
	let r = Math.random();
	start = start || 0;
	end = end || (end===0?0:1E8);
	return Math.floor(start + r * (end - start + 1));
}

/**
* @method fixRandom # 固定随机值,指定几率返回常数
* @param args.. {Number} # 输入百分比
* @return {Number}
*/
export function fixRandom(arg: number, ...args: number[]): number {
	if (!args.length)
		return 0;
	let total = arg;
	let argus = [arg];
	let len = args.length;
	for (let i = 0; i < len; i++) {
		total += args[i];
		argus.push(total);
	}
	let r = random(0, total - 1);
	for (let i = 0; (i < len); i++) {
		if (r < argus[i])
			return i;
	}
	return 0;
}

/**
* @method filter # object filter
* @param obj {Object}  
* @param exp {Object}  #   filter exp
* @param non {Boolean} #   take non
* @return {Object}
*/
export function filter(obj: any, exp: string[] | ((key: string, value: any)=>boolean), non: boolean = false): any {
	let rev: any = {};
	let isfn = (typeof exp == 'function');
	
	if (isfn || non) {
		for (let key in obj) {
			let value = obj[key];
			let b: boolean = isfn ? (<any>exp)(key, value) : ((<string[]>exp).indexOf(key) != -1);
			if (non ? !b : b)
				rev[key] = value;
		}
	} else {
		for (let item of <string[]>exp) {
			item = String(item);
			if (item in obj)
				rev[item] = obj[item];
		}
	}
	return rev;
}

/**
 * @method update # update object property value
 * @param obj {Object}      #        need to be updated for as
 * @param extd {Object}    #         update object
 * @param {Object}
 */
export function update<T extends object>(obj: T, extd: any): T {
	for (let key in extd) {
		if (key in obj) {
			(<any>obj)[key] = select((<any>obj)[key], extd[key]);
		}
	}
	return obj;
}

/**
 * @method select
 * @param default {Object} 
 * @param value   {Object} 
 * @reg {Object}
 */
export function select<T>(default_: T, value: any): T {
	if ( typeof default_ == typeof value ) {
		return <T>value;
	} else {
		return default_;
	}
}

/**
 * @method equalsClass  # Whether this type of sub-types
 * @param baseclass {class}
 * @param subclass {class}
 */
export function equalsClass(baseclass: any, subclass: any): boolean {
	if (!baseclass || !subclass || !subclass.prototype)
		return false;
	if (baseclass === subclass)
		return true;
	
	let prototype = baseclass.prototype;
	let subprototype = subclass.prototype;
	if (!subprototype) return false;
	let obj = subprototype.__proto__;
	
	while (obj) {
		if (prototype === obj)
			return true;
		obj = obj.__proto__;
	}
	return false;
}

/**
 * @method assert
 */
export function assert(condition: any, code?: number | ErrorNewArg): void {
	if (condition)
		return;
	if (typeof code == 'number') {
		throw Error.new([code, 'assert fail, unforeseen exceptions']);
	} else {
		throw Error.new(code || [-30009, 'ERR_ASSERT_ERROR']);
	}
}

/**
 * @method sleep()
 */
export function sleep<T = number>(time: number, defaultValue?: T): Promise<T> {
	return new Promise((ok, err)=>setTimeout(()=>ok((defaultValue || 0) as any), time));
}

export function timeout<T>(promise: Promise<T> | T, time: number): Promise<T> {
	if (promise instanceof Promise) {
		return new Promise(function(_resolve, _reject) {
			let id: any = setTimeout(function() {
				id = 0;
				_reject(Error.new(errno.ERR_EXECUTE_TIMEOUT));
			}, time);

			let ok = (err: any, r?: any)=>{
				if (id) {
					clearTimeout(id);
					id = 0;
					if (err)
						_reject(err);
					else
						_resolve(r);
				}
			};

			promise.then(e=>ok(null, e)).catch(ok);
		});
	} else {
		return Promise.resolve(promise);
	}
}

interface PromiseExecutor<T> {
	(resolve: (value?: T)=>void, reject: (reason?: any)=>void, promise: Promise<T>): Promise<void> | void;
}

export class PromiseNx<T extends any> extends Promise<T> {
	protected _executor?: PromiseExecutor<T>;
	constructor(executor?: (resolve: (value?: T)=>void, reject: (reason?: any)=>void, promise: Promise<T>)=>any) {
		let _resolve: any;
		let _reject: any;

		super(function(resolve: (value: T)=>void, reject: (reason?: any)=>void) {
			_resolve = resolve;
			_reject = reject;
		});

		this._executor = executor;

		try {
			let r = this.executor(_resolve, _reject);
			if (r instanceof Promise) {
				r.catch(_reject);
			}
		} catch(err) {
			_reject(err);
		}
	}

	executor(resolve: (value?: T)=>void, reject: (reason?: any)=>void) {
		if (this._executor) {
			return this._executor(resolve, reject, this);
		} else {
			throw Error.new('executor undefined');
		}
	}

}

/**
 * @method promise(executor)
 */
export function promise<T extends any>(executor: (resolve: (value?: T)=>void, reject: (reason?: any)=>void, promise: Promise<T>)=>any) {
	return new PromiseNx<T>(executor) as Promise<T>;
}
