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

declare namespace qk {
	interface RequireResolve {
		(id: string): string;
		paths: (request: string)=>string[] | null;
	}
	interface Require {
		(id: string): any;
		resolve: RequireResolve;
	}
	interface Module {
		id: string;
		exports: any;
		filename: string;
		loaded: boolean;
		children: Module[];
		paths: string[];
	}
}

declare const __binding__: (id: string)=>any; // binding native module
declare const __filename: string;
declare const __dirname: string;
declare const require: qk.Require;
declare const module: qk.Module;
declare const exports: any; // Same as module.exports

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number
 * @type number:Number
 * @global
*/

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean
 * @type boolean:Boolean
 * @global
*/

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
 * @type object:Object
 * @global
*/

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
 * @type string:String
 * @global
*/

/**
 * 32-bit signed integer, Range: -2147483648 to 2147483647
 * @global
*/
type Int = number;

/**
 * 32-bit signed integer, Range: 0 to 4294967295
 * @global
*/
type Uint = number;

/**
 * 32-bit floating point
 * @global
*/
type Float = number;

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
 * @interface Object
 * @global
*/
interface ObjectConstructor {
	hashCode(obj: any): Int;
}

interface Object {
	hashCode(): Int;
}

/**
 * @interface Dict Dictionaries
 * @global
 */
interface Dict<T = any> {
	[key: string]: T;
}

type TimeoutResult = any;

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function
 * @interface Function
 * @global
*/
interface Function {
	hashCode(): Int;
	setTimeout(this: Function, time: Uint, ...argArray: any[]): TimeoutResult;
}

interface CallableFunction extends Function {
	setTimeout<A extends any[], R>(this: (...args: A) => R, time: Uint, ...args: A): TimeoutResult;
}

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array
 * @interface Array
 * @global
*/
interface Array<T> {
	hashCode(): Int;
	deleteOf(value: T): T[];
	indexReverse(index: Uint): T;
	indexAt(index: Uint): T;
}

interface ArrayConstructor {
	toArray(obj: any, index?: Uint, end?: Uint): any[];
}

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
 * @interface String
 * @global
*/
interface String {
	hashCode(): Int;
}

interface StringConstructor {
	format(str: string, ...args: any[]): string;
}

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number
 * @interface Number
 * @global
*/
interface Number {

	hashCode(): Int;

	/**
	* Convert to a string with a fixed number of digits before and after the decimal point
	* 
	* @param before Fixed number of digits before the decimal point
	* @param after? Fixed number of digits after the decimal point
	*/
	toFixedBefore(before: Uint, after?: Uint): string;

	/**
	 * Fixed the number of digits before the decimal point and used symbol separation
	 * 
	 * @param after  Fixed number of digits before the decimal point
	 * @param split?  Split unit length
	 * @param symbol?  Split characters
	 * 
	 * @example
	 * 
	 * ```ts
	 * // Print: 1,000,000.03
	 * console.log((1000000.03).toFixedVariable(8,3,','))
	 * ```
	*/
	toFixedVariable(after: Uint, split?: Uint, symbol?: string): string;
}

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean
 * @interface Boolean
 * @global
*/
interface Boolean {
	hashCode(): Int;
}

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date
 * @interface Date
 * @global
*/
interface DateConstructor {
	/**
	 * current timezone
	 * @static
	 */
	currentTimezone: Uint;

	/**
	 * Parse a string as a time
	 * @param str       The string to parse
	 * @param format?   date format default yyyyMMddhhmmssfff
	 * @param timezone? The time zone of the time to be parsed, the default is the current time zone
	 * @return          Retruu Date object
	 * @static
	 * 
	 * @example
	 * 
	 * ```ts
	 * let i = '2008-02-13 01:12:13';
	 * let date = Date.parseDate(i); // The new time returned
	 * ```
	 */
	parseDate(date_str: string, format?: string, timezone?: Uint): Date;

	/**
		* Formatting timestamps
		* 
		* @method formatTimeSpan(ts,format?)
		* @param time_span The timestamp to format
		* @param format? The timestamp format to be formatted
		* @return The returned formatted timestamp
		* @static
		* 
		* @example
		* 
		* ```ts
		* // Format timestamp (unit: milliseconds)
		* let time_span = 10002100;
		* let format = 'dd hh:mm:ss';
		* let str = Date.formatTimeSpan(time_span, format); // str = '0 2:46:42'
		* let format = 'dd天hh时mm分ss秒';
		* let str = Date.formatTimeSpan(time_span, format); // str = '0天2时46分42秒'
		* format = 'hh时mm分ss秒';
		* str = Date.formatTimeSpan(time_span, format); // str = '2时46分42秒'
		* format = 'mm分ss秒';
		* str = Date.formatTimeSpan(time_span, format); // str = '166分42秒'
		* ```
		*/
	formatTimeSpan(time_span: Uint, format?: string): string;
}

interface Date {
	hashCode(): Int;

	/**
	 * Add milliseconds to the current Date time and change the time value
	 * @param ms The millisecond value to append
	 */
	add(ms: Uint): Date;

	/**
		* Returns a date string given a date format
		* @param format? The format of the string to be converted
		* @return Returns the formatted time string
		* 
		* @exmples
		* 
		* ```ts
		* let date = new Date();
		* let format = 'yyyy-MM-dd hh:mm:ss.fff';
		* let dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10：32：23'
		* format = 'yyyy-MM-dd hh:mm:ss';
		* dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10：32：23'
		* format = 'yyyy/MM/dd';
		* dateStr = date.toString(format); // dateStr的值为 '2008/12/10'
		* format = 'yyyy-MM-dd hh';
		* dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10'
		* ```
		*/
	toString(format?: string, timezone?: number): string;
}

/**
 * @interface ErrorDescribe
 * @global
*/
interface ErrorDescribe {
	name?: string;
	message?: string;
	error?: string;
	description?: string;
	errno?: number | string;
	child?: Error | Error[];
	[prop: string]: any;
}

/**
 * The [errno,message,description] of Array
 * @type ErrnoCode:[number,string,string?]
 * @global
*/
type ErrnoCode = [number, string, string?];

/**
 * @global
*/
type ErrorNewArg = ErrnoCode | Error | string | ErrorDescribe;

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Error
 * @interface Error
 * @global
*/
interface ErrorConstructor {
	'new'(err: ErrorNewArg, ...child: ErrorNewArg[]): Error;
	toJSON(err: Error): any;
	setStackTraceJSON(enable: boolean): void;
	/**
	 * Create .stack property on a target object
	 * @static
	 */
	captureStackTrace(targetObject: Object, constructorOpt?: Function): void;
}

interface Error {
	errno?: number | string;
	description?: string;
	child?: Error[];
	[prop: string]: any;
	extend: (desc: ErrorDescribe)=>this;
}

declare function setTimeout<A extends any[]>(cb: (...args: A)=>void, timeout?: number, ...args: A): TimeoutResult;
declare function setInterval<A extends any[]>(cb: (...args: A)=>void, timeout?: number, ...args: A): TimeoutResult;
declare function setImmediate<A extends any[]>(cb: (...args: A)=>void, ...args: A): TimeoutResult;
declare function clearTimeout(id?: TimeoutResult): void;
declare function clearInterval(id?: TimeoutResult): void;
declare function clearImmediate(id?: TimeoutResult): void;

(function(_: any) {
if (Date.formatTimeSpan !== undefined)
	return;

if (typeof globalThis == 'undefined') {
	let globa = arguments[0]('(typeof global == "object" ? global: 0)');
	if (globa) {
		(globa as any).globalThis = globa;
	} else if (typeof window == 'object') {
		(window as any).globalThis = window;
	}
}

let currentTimezone = new Date().getTimezoneOffset() / -60;
let _slice = Array.prototype.slice;
let _hash_code_id = 1;
let _hash_code_set = new WeakSet();
let DateToString = (Date.prototype as any).__toStrin__ || Date.prototype.toString;
let ErrorToString = (Error.prototype as any).__toStrin__ || Error.prototype.toString;

function definePropertys(obj: any, extd: any): void {
	for (let i in extd) {
		let desc = Object.getOwnPropertyDescriptor(extd, i)!;
		desc.enumerable = false;
		Object.defineProperty(obj, i, desc);
	}
}

function hashCode(obj: any): number {
	return obj ? obj.hashCode():
		obj === null ? -1354856:
		obj === undefined ? -3387255: obj.hashCode();
}

// index of
function indexOf(str: string, str1: string): number {
	let index = str.indexOf(str1);
	return index > -1 ? index : Infinity;
}

definePropertys(Date.prototype, {__toStrin__: Date.prototype.toString});
definePropertys(Date.prototype, {__toStrin__: Error.prototype.toString});

definePropertys(Object, { hashCode: hashCode });

definePropertys(Object.prototype, {
	hashCode(): Int {
		if (_hash_code_set.has(this))
			return 0;
		_hash_code_set.add(this);
		let _hash = 5381;
		for (let key in this) {
			_hash += (_hash << 5) + (key.hashCode() + hashCode(this[key]));
		}
		_hash_code_set.delete(this);
		return _hash;
	},
});

definePropertys(Function.prototype, {
	hashCode(): Int {
		if (!this.hasOwnProperty('__hashCode')) {
			Object.defineProperty(this, '__hashCode', {
				enumerable: false, configurable: false, writable: false, value: _hash_code_id++
			});
		}
		return this.__hashCode;
	},
	setTimeout(time: number, ...args: any[]): TimeoutResult {
		let fn = this;
		return setTimeout(function() {
			fn(...args);
		}, time);
	},
});

definePropertys(Array, {
	toArray(obj: any, index: number, end: number): any[] {
		return _slice.call(obj, index, end);
	},
});

definePropertys(Array.prototype, {
	hashCode(): Int {
		if (_hash_code_set.has(this))
			return 0;
		_hash_code_set.add(this);
		let _hash = 5381;
		for (let item of this) {
			if (item) {
				_hash += (_hash << 5) + item.hashCode();
			}
		}
		_hash_code_set.delete(this);
		return _hash;
	},

	deleteOf(value: any): any[] {
		let i = this.indexOf(value);
		if (i != -1) {
			this.splice(i, 1);
		}
		return this;
	},

	indexReverse (index: number): any {
		return this[this.length - 1 - index];
	},

	indexAt(index: number): any {
		return this[index];
	}
});

// ext TypedArray
definePropertys((Uint8Array as any).prototype.__proto__, {
	hashCode(): Int {
		let _hash = 5381;
		let self = new Uint8Array(this.buffer, this.byteOffset, this.byteLength);
		for (let item of self) {
			_hash += (_hash << 5) + item;
		}
		return _hash;
	},
});

definePropertys(String, {
	format(str: string, ...args: any[]): string {
		let val = String(str);
		for (let i = 0, len = args.length; i < len; i++)
			val = val.replace(new RegExp('\\{' + i + '\\}', 'g'), args[i]);
		return val;
	}
});

definePropertys(String.prototype, {
	hashCode: function(): number {
		let _hash = 5381;
		let len = this.length;
		while (len) {
			len--;
			_hash += (_hash << 5) + this.charCodeAt(len);
		}
		return _hash;
	},
});

definePropertys(Number.prototype, {
	hashCode(): Int {
		return this;
	},

	toFixedBefore(this: number, before: number, after: number): string {
		if (!isFinite(this)) {
			return String(this);
		} else {
			let num = typeof after == 'number' ? this.toFixed(after) : String(this);
			let match = num.match(/^(\d+)(\.\d+)?$/)!;
			let integer = match[1];
			let len = before - integer.length;
			if (len > 0)
				num = new Array(len + 1).join('0') + num;
			return num;
		}
	},

	toFixedVariable(this: number, after: number, split?: number, symbol?: string): string {
		let num = this.toFixed(after);
		let str = num.replace(/\.(\d*?)0+$/, function(_,b){ return b?'.'+b:'' });
		if (!str)
			return '0';
		if (!split)
			return str;
		symbol = symbol || ',';
		let l = str.split('.');
		let l0 = l[0];
		let l0_ = '';
		for (let i = l0.length; i > 0; i-=split) {
			let j = i - split;
			if (j > 0) {
				l0_ = symbol + l0.substring(j, i) + l0_;
			} else {
				l0_ = l0.substring(0, i) + l0_;
			}
		}
		l[0] = l0_;
		return l.join('.');
	},
});

definePropertys(Boolean.prototype, {
	hashCode(): Int {
		return this == true ? -1186256: -23547257;
	},
});

definePropertys(Date, {
	currentTimezone: currentTimezone,

	parseDate(
		date_str: string, 
		format?: string, /* = 'yyyyMMddhhmmssfff', */
		timezone?: number, /* = currentTimezone*/
	): Date
	{
		let s = String(date_str).replace(/[^0-9]/gm, '');
		let f = '';
		format = format || 'yyyyMMddhhmmssfff';
		format.replace(/(yyyy|MM|dd|hh|mm|ss|fff)/gm, e=>{
			f += e;
			return '';
		});
		if (timezone === undefined)
			timezone = currentTimezone;

		let d = new Date();
		let diffTime = currentTimezone - timezone;
		return new Date(
			Number(s.substring(indexOf(f, 'yyyy'), 4)) || d.getFullYear(),
			Number(s.substring(indexOf(f, 'MM'), 2) || 1/*(d.getMonth() + 1)*/) - 1,
			Number(s.substring(indexOf(f, 'dd'), 2)) || 1/*d.getDate()*/,
			Number(s.substring(indexOf(f, 'hh'), 2) || 0/*d.getHours()*/) - diffTime,
			Number(s.substring(indexOf(f, 'mm'), 2)) || 0/*d.getMinutes()*/,
			Number(s.substring(indexOf(f, 'ss'), 2)) || 0/*d.getSeconds()*/,
			Number(s.substring(indexOf(f, 'fff'), 3)) || 0
		);
	},

	formatTimeSpan(time_span: number, format: string = '{(dd) }?{(hh):}?{(mm):}{(ss)}', isFixedBefore = false): string {
		let data = [];
		let items = [
			[1, 1000, /\{([^\}]*?)\(fff\)([^\}]*?)\}/g, /\{([^\}]*?)\(fff\)([^\}]*?)\}\?/g],
			[1000, 60, /\{([^\}]*?)\(ss\)([^\}]*?)\}/g, /\{([^\}]*?)\(ss\)([^\}]*?)\}\?/g],
			[60, 60, /\{([^\}]*?)\(mm\)([^\}]*?)\}/g, /\{([^\}]*?)\(mm\)([^\}]*?)\}\?/g],
			[60, 24, /\{([^\}]*?)\(hh\)([^\}]*?)\}/g, /\{([^\}]*?)\(hh\)([^\}]*?)\}\?/g],
			[24, 1, /\{([^\}]*?)\(dd\)([^\}]*?)\}/g, /\{([^\}]*?)\(dd\)([^\}]*?)\}\?/g]
		];
		let start = false;

		for (let i = 0; i < 5; i++) {
			let item = items[i];
			let reg = <RegExp>item[2];

			if (format.match(reg)) {
				start = true;
			}
			else if (start) {
				break;
			}
			time_span = time_span / <number>item[0];
			data.push([time_span % <number>item[1], time_span]);
		}

		if (!start)
			return format;
		data.indexReverse(0).reverse();
		data.forEach(function (item, index) {
			let val = Math.floor(item[0] as number);
			let val2 = isFixedBefore ? val.toFixedBefore(2): String(val);
			if (val) { // != 0
				format = format.replace(items[index][3] as RegExp, `$1${val2}$2`);
				format = format.replace(items[index][2] as RegExp, `$1${val2}$2`);
			} else {
				format = format.replace(items[index][3] as RegExp, '');
				format = format.replace(items[index][2] as RegExp, `$1${val2}$2`);
			}
		});
		return format;
	},
});

definePropertys(Date.prototype, {

	hashCode(): Int {
		return this.valueOf();
	},

	add(ms: number): Date {
		this.setMilliseconds(this.getMilliseconds() + ms);
		return this;
	},

	toString(format?: string, timezone?: number): string {
		if (format/*typeof format == 'string'*/) {
			let d = new Date(this.valueOf());
			if (typeof timezone == 'number') {
				let cur_time_zone = d.getTimezoneOffset() / -60;
				let offset = timezone - cur_time_zone;
				d.setHours(d.getHours() + offset);
			}
			return format.replace('yyyy', String(d.getFullYear()))
				.replace('MM', (d.getMonth() + 1).toFixedBefore(2))
				.replace('dd', d.getDate().toFixedBefore(2))
				.replace('hh', d.getHours().toFixedBefore(2))
				.replace('HH', d.getHours().toFixedBefore(2))
				.replace('mm', d.getMinutes().toFixedBefore(2))
				.replace('ss', d.getSeconds().toFixedBefore(2))
				.replace('fff', d.getMilliseconds().toFixedBefore(3));
		} else {
			return DateToString.call(this);
		}
	},
});

const errors: Dict<Function> = {
	Error,
	SyntaxError,
	ReferenceError,
	TypeError,
	RangeError,
	EvalError,
	URIError,
};

if (!Error.captureStackTrace) {
	definePropertys(Error, {
		captureStackTrace(targetObject: Object, constructorOpt?: Function): void {}
	});
}

let stackTraceJSON = true;

definePropertys(Error, {
	new(arg: ErrorNewArg, ...child: ErrorNewArg[]): Error {
		let err: Error;
		if (typeof arg == 'object') { // ErrnoCode | Error | ErrorDescribe;
			if (arg instanceof Error) {
				err = <Error>arg;
			} if (Array.isArray(arg)) { // ErrnoCode
				let errnoCode = <ErrnoCode>arg;
				err = new Error(errnoCode[1] || errnoCode[2] || 'Unknown error');
				err.errno = errnoCode[0];
				err.description = errnoCode[2] || '';
				Error.captureStackTrace(err, Error.new);
			} else { // ErrorDescribe
				let describe = <ErrorDescribe>arg;
				let Err = <ErrorConstructor>(errors[(<Error>arg).name] || Error);
				let msg = describe.message || describe.error || 'Unknown error';
				err = <Error>Object.assign(new Err(msg), arg);
				Error.captureStackTrace(err, Error.new);
			}
		} else { // string
			err = new Error(String(arg));
			Error.captureStackTrace(err, Error.new);
		}
		err.errno = Number(err.errno) || Number(err.code) || -30000;
		if (child.length) {
			if (!Array.isArray(err.child))
				err.child = [];
			for (let ch of child)
				err.child.push(Error.new(ch));
		}
		return err;
	},
	toJSON(err: any): Error {
		return Error.new(err).toJSON()
	},
	setStackTraceJSON(enable: boolean) {
		stackTraceJSON = !!enable;
	},
});

definePropertys(Error.prototype, {
	extend(desc: ErrorDescribe) {
		return Object.assign(this, desc);
	},
	hashCode(): Int {
		let _hash = Object.prototype.hashCode.call(this);
		_hash += (_hash << 5) + this.message.hashCode();
		return _hash;
	},
	toJSON(): any {
		let err: Error = this;
		let r: any = Object.assign({}, err);
		r.name = err.name || '';
		r.message = err.message || 'Unknown error';
		r.errno = Number(err.errno) || -30000;
		r.code = r.errno; // compatible old
		r.description = err.description || '';
		if (stackTraceJSON)
			r.stack = err.stack || '';
		return r;
	},
	toStringStyled() {
		return this.toString();
	},
	toString(this: Error) {
		return ErrorToString.call(this) + '\n' +
			(this.description ? `Description: ${this.description}\n`: '') +
			(this.child ? `Childs: [${this.child.map(e=>e.toString()).join(',')}]`: '');
	},
});

})((s:any)=>eval(s));