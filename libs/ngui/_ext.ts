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

import './_globals';

(function(_: any) {

if (Date.formatTimeSpan !== undefined)
	return;

if (typeof globalThis == 'undefined') {
	var globa = arguments[0]('(global)');
	if (typeof globa == 'object') {
		(globa as any).globalThis = globa;
	} else if (typeof window == 'object') {
		(window as any).globalThis = window;
	}
}

var currentTimezone = new Date().getTimezoneOffset() / -60;
var G_slice = Array.prototype.slice;
var G_hash_code_id = 1;
var G_hash_code_set = new WeakSet();
var dateToString = Date.prototype.toString;

/**
 * @fun ext_class #  EXT class prototype objects
 */
function definePropertys(obj: any, extd: any): void {
	for (var i in extd) {
		var desc = <PropertyDescriptor>Object.getOwnPropertyDescriptor(extd, i);
		desc.enumerable = false;
		Object.defineProperty(obj, i, desc);
	}
}

function hashCode(obj: any): number {
	return 	obj === null ? -1354856:
					obj === undefined ? -3387255: obj.hashCode();
}

// index of
function indexOf(str: string, str1: string): number {
	var index = str.indexOf(str1);
	return index > -1 ? index : Infinity;
}

definePropertys(Object, {
	hashCode: hashCode,
});

definePropertys(Object.prototype, {
	hashCode(): number {
		if (G_hash_code_set.has(this)) 
			return 0;
		G_hash_code_set.add(this);
		var _hash = 5381;
		for (var key in this) {
			_hash += (_hash << 5) + (key.hashCode() + hashCode(this[key]));
		}
		G_hash_code_set.delete(this);
		return _hash;
	},
});

definePropertys(Function.prototype, {
	
	hashCode(): number {
		if (!this.hasOwnProperty('M_hashCode')) {
			Object.defineProperty(this, 'M_hashCode', { 
				enumerable: false, configurable: false, writable: false, value: G_hash_code_id++
			});
		}
		return this.M_hashCode;
	},

	setTimeout(time: number, ...args: any[]): TimeoutResult {
		var fn = this;
		return setTimeout(function() {
			fn(...args);
		}, time);
	},

});

definePropertys(Array, {
	toArray(obj: any, index: number, end: number): any[] {
		return G_slice.call(obj, index, end);
	},
});

definePropertys(Array.prototype, {

	hashCode(): number {
		if (G_hash_code_set.has(this)) 
			return 0;
		G_hash_code_set.add(this);
		var _hash = 5381;
		for (var item of this) {
			if (item) {
				_hash += (_hash << 5) + item.hashCode();
			}
		}
		G_hash_code_set.delete(this);
		return _hash;
	},

	deleteOf(value: any): any[] {
		var i = this.indexOf(value);
		if (i != -1) {
			this.splice(i, 1);
		}
		return this;
	},

	indexReverse (index: number): any {
		return this[this.length - 1 - index];
	},

});

// ext TypedArray
definePropertys((Uint8Array as any).prototype.__proto__, {

	hashCode(): number {
		var _hash = 5381;
		var self = new Uint8Array(this.buffer, this.byteOffset, this.byteLength);
		for (var item of self) {
			_hash += (_hash << 5) + item;
		}
		return _hash;
	},
});

definePropertys(String, {
	format(str: string, ...args: any[]): string {
		var val = String(str);
		for (var i = 0, len = args.length; i < len; i++)
			val = val.replace(new RegExp('\\{' + i + '\\}', 'g'), args[i]);
		return val;
	}
});

definePropertys(String.prototype, {
	hashCode: function(): number {
		var _hash = 5381;
		var len = this.length;
		while (len--) 
			_hash += (_hash << 5) + this.charCodeAt(len);
		return _hash;
	},
});

definePropertys(Number.prototype, {

	hashCode(): number {
		return this;
	},

	toFixedBefore(before: number, after: number): string {
		if (!isFinite(this)) {
			return String(this);
		} else {
			var num = typeof after == 'number' ? this.toFixed(after) : String(this);
			var match = num.match(/^(\d+)(\.\d+)?$/);
			var integer = match[1];
			var len = before - integer.length;
			if (len > 0)
				num = new Array(len + 1).join('0') + num;
			return num;
		}
	},

});

definePropertys(Boolean.prototype, {
	hashCode(): number {
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
		var s = String(date_str).replace(/[^0-9]/gm, '');
		var f = '';

		format = format || 'yyyyMMddhhmmssfff';
		format.replace(/(yyyy|MM|dd|hh|mm|ss|fff)/gm, e=>{
			f += e;
			return '';
		});
		
		if (timezone === undefined)
			timezone = currentTimezone;

		var d = new Date();
		var diffTime = currentTimezone - timezone;

		return new Date(
			Number(s.substr(indexOf(f, 'yyyy'), 4)) || d.getFullYear(),
			Number(s.substr(indexOf(f, 'MM'), 2) || 1/*(d.getMonth() + 1)*/) - 1,
			Number(s.substr(indexOf(f, 'dd'), 2)) || 1/*d.getDate()*/,
			Number(s.substr(indexOf(f, 'hh'), 2) || 0/*d.getHours()*/) - diffTime,
			Number(s.substr(indexOf(f, 'mm'), 2)) || 0/*d.getMinutes()*/,
			Number(s.substr(indexOf(f, 'ss'), 2)) || 0/*d.getSeconds()*/,
			Number(s.substr(indexOf(f, 'fff'), 3)) || 0
		);
	},

	formatTimeSpan(time_span: number, format: string = 'dd hh:mm:ss'): string {

		var data = [];
		var items = [
			[1, 1000, /fff/g],
			[1000, 60, /ss/g],
			[60, 60, /mm/g],
			[60, 24, /hh/g],
			[24, 1, /dd/g]
		];
		
		var start = false;

		for (var i = 0; i < 5; i++) {
			var item = items[i];
			var reg = <RegExp>item[2];

			if (format.match(reg)) {
				start = true;
			}
			else if (start) {
				break;
			}
			time_span = time_span / <number>item[0];
			data.push([time_span % <number>item[1], time_span]);
		}

		if (!start) {
			return format;
		}

		data.indexReverse(0).reverse();
		data.forEach(function (item, index) {
			format =
				format.replace(<RegExp>items[index][2], Math.floor(<number>item[0]).toFixedBefore(2));
		});
		return format;
	},

});

definePropertys(Date.prototype, {

	hashCode(): number {
		return this.valueOf();
	},

	add(ms: number): Date {
		this.setMilliseconds(this.getMilliseconds() + ms);
		return this;
	},

	toString(format?: string, timezone?: number): string {
		if (format/*typeof format == 'string'*/) {
			var d = new Date(this.valueOf());
			if (typeof timezone == 'number') {
				var cur_time_zone = d.getTimezoneOffset() / -60;
				var offset = timezone - cur_time_zone;
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
			return dateToString.call(this);
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
		captureStackTrace(targetObject: Object, constructorOpt?: Function): void {
			// TODO ...
		}
	});
}

var stackTraceJSON = true;

definePropertys(Error, {

	new(arg: ErrorNewArg, ...child: ErrorNewArg[]): Error {
		var err: Error;
		if (typeof arg == 'object') { // ErrnoCode | Error | ErrorDescribe;
			if (arg instanceof Error) {
				err = <Error>arg;
			} if (Array.isArray(arg)) { // ErrnoCode
				var errnoCode = <ErrnoCode>arg;
				err = new Error(errnoCode[1] || errnoCode[2] || 'Unknown error');
				err.errno = errnoCode[0];
				err.description = errnoCode[2] || '';
				Error.captureStackTrace(err, Error.new);
			} else { // ErrorDescribe
				var describe = <ErrorDescribe>arg;
				var Err = <ErrorConstructor>(errors[(<Error>arg).name] || Error);
				var msg = describe.message || describe.error || 'Unknown error';
				err = <Error>Object.assign(new Err(msg), arg);
				Error.captureStackTrace(err, Error.new);
			}
		} else { // string
			err = new Error(String(arg));
			Error.captureStackTrace(err, Error.new);
		}
		err.errno = Number(err.errno) || -30000;

		if (child.length) {
			if (!Array.isArray(err.child))
				err.child = [];
			for (var ch of child) {
				err.child.push(Error.new(ch));
			}
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

	hashCode(): number {
		var _hash = Object.prototype.hashCode.call(this);
		_hash += (_hash << 5) + this.message.hashCode();
		return _hash;
	},

	toJSON(): any {
		var err: Error = this;
		var r: any = Object.assign({}, err);
		r.name = err.name || '';
		r.message = err.message || 'Unknown error';
		r.errno = Number(err.errno) || -30000;
		r.code = r.errno; // compatible old
		r.description = err.description || '';
		if (stackTraceJSON)
			r.stack = err.stack || '';
		return r;
	},
});

})((s:any)=>eval(s));