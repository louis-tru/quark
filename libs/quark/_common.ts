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

import {List} from './_event';
import errno from './errno';
import type {Uint,Float,ErrorNewArg} from './defs';

let id = 10;
let scopeLockQueue = new Map();

export const currentTimezone = new Date().getTimezoneOffset() / -60; //!< Current time zone

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
 * @method clone Clone an Object
 * @param  obj   The Object to be cloned
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
				return clone_object(Object.create(obj.constructor.prototype), obj);
		}
	}
	return obj;
}

/**
 * @method extend(obj:any,extd:any,top?:any)any
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
 * EXT class prototype objects
 * 
 * @method extendClass(cls:Function,extds:Function[]|Function,top?:any)Function
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
 * @method scopeLock(mutex:any,cb:Function)Promise
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
 * Get object value by name
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
 * Setting object value by name
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
 * Delete object value by name
 */
export function removeProp(name: string, self: any): void {
	let names = name.split('.');
	let last = names.pop()!;
	self = getProp(names.join('.'), self);
	if (self)
		delete self[last];
}

/**
 * @method random(start?,end?) Creating random numbers
 * @param start? Numbers of begin
 * @param end?   Numbers of end
 */
export function random(start: Uint = 0, end: Uint = 1E8): Uint {
	if (start == end)
		return start;
	let r = Math.random();
	start = start || 0;
	end = end || (end===0?0:1E8);
	// start = 0, end = 2
	// floor(r * 3) => 0 or 1 or 2, probability 1/3 of each
	return Math.floor(start + r * (end - start + 1));
}

/**
 * @method randomFloat(start?,end?) Creating random float numbers
 * @param start? Numbers of begin
 * @param end?   Numbers of end
 */
export function randomFloat(start: Float = 0, end: Float = 1): Float {
	if (start == end)
		return start;
	let r = Math.random();
	start = start || 0;
	end = end || 1;
	return start + r * (end - start);
}

/**
* @method fixRandom(arg0,...args)Uint
* 
* * Fixed random value, specified probability to return a constant
* 
* * Get a random number from `0` to the passed probability number `arguments.length` by probability
* 
* * The sum of the passed probabilities cannot be `zero`
* 
* @param arg0:number   Enter percentage
* @param args:number[] Enter percentage
* 
* Example:
* 
* ```js
* // Prints: 3 5 9
* console.log(util.random(0, 10))
* console.log(util.random(0, 10))
* console.log(util.random(0, 10))
* // Prints 0 3 2
* console.log(util.fixRandom(10, 20, 30, 40))
* console.log(util.fixRandom(10, 20, 30, 40))
* console.log(util.fixRandom(10, 20, 30, 40))
* ```
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
* @method filter(obj,exp,non?)any Filter object attrs
* @param obj:any
* @param exp:string[]|Function filter exp
* @param non?:boolean          take non
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
 * @method update(obj,extd)any Update object property value
 * @param obj:any  need to be updated for as
 * @param extd:any update object
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
 * @method select(default:any,value:any)any
 */
export function select<T>(default_: T, value: any): T {
	if ( typeof default_ == typeof value ) {
		return <T>value;
	} else {
		return default_;
	}
}

/**
 * Whether this type of sub-types
 * 
 * @method equalsClass(baseclass:any,subclass:any)boolean
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
 * @method assert(condition,arg?:number|ErrorNewArg,extMsg?:string)
 * Asserts that value is not null or undefined.
 * @param condition The value to check.
 * @param arg? Optional code or message for the error thrown.
 * @param extMsg?  Optional message for the error thrown.
 * @returns {void} nothing
 */
export function assert<T>(condition: T, arg?: number | ErrorNewArg, extMsg?: string): asserts condition is NonNullable<T> {
	if (condition)
		return;
	if (typeof arg == 'number') {
		throw Error.new([arg, extMsg || 'assert fail, unforeseen exceptions']);
	} else if (arg) {
		const err = Error.new(arg);
		if (extMsg)
			err.message += '\n' + extMsg;
		throw err;
	} else {
		throw Error.new([-30009, extMsg || 'ERR_ASSERT_ERROR']);
	}
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

/**
 * @method promise(executor:Function)Promise
 */
export function promise<T extends any>(executor: (resolve: (value: T)=>void, reject: (reason?: any)=>void, promise: Promise<T>)=>any) {
	let _resolve: (value: T | PromiseLike<T>) => void;
	let _reject:  (reason?: any) => void
	let p = new Promise<T>(function(r,j) {
		_resolve = r;
		_reject = j;
	});
	executor(_resolve!, _reject!, p);
	return p;
}
