"use strict";
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
(function () {
	if (Date.formatTimeSpan !== undefined)
		return;
	if (typeof globalThis == 'undefined') {
		if (typeof global == 'object') {
			global.globalThis = global;
		}
		else if (typeof window == 'object') {
			window.globalThis = window;
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
	function definePropertys(obj, extd) {
		for (var i in extd) {
			var desc = Object.getOwnPropertyDescriptor(extd, i);
			desc.enumerable = false;
			Object.defineProperty(obj, i, desc);
		}
	}
	function hashCode(obj) {
		return obj === null ? -1354856 :
			obj === undefined ? -3387255 : obj.hashCode();
	}
	// index of
	function indexOf(str, str1) {
		var index = str.indexOf(str1);
		return index > -1 ? index : Infinity;
	}
	definePropertys(Object, {
		hashCode: hashCode,
	});
	definePropertys(Object.prototype, {
		hashCode() {
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
		hashCode() {
			if (!this.hasOwnProperty('M_hashCode')) {
				Object.defineProperty(this, 'M_hashCode', {
					enumerable: false, configurable: false, writable: false, value: G_hash_code_id++
				});
			}
			return this.M_hashCode;
		},
		setTimeout(time, ...args) {
			var fn = this;
			return setTimeout(function () {
				fn(...args);
			}, time);
		},
	});
	definePropertys(Array, {
		toArray(obj, index, end) {
			return G_slice.call(obj, index, end);
		},
	});
	definePropertys(Array.prototype, {
		hashCode() {
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
		deleteOf(value) {
			var i = this.indexOf(value);
			if (i != -1) {
				this.splice(i, 1);
			}
			return this;
		},
		indexReverse(index) {
			return this[this.length - 1 - index];
		},
	});
	definePropertys(String, {
		format(str, ...args) {
			var val = String(str);
			for (var i = 0, len = args.length; i < len; i++)
				val = val.replace(new RegExp('\\{' + i + '\\}', 'g'), args[i]);
			return val;
		}
	});
	definePropertys(String.prototype, {
		hashCode: function () {
			var _hash = 5381;
			var len = this.length;
			while (len--)
				_hash += (_hash << 5) + this.charCodeAt(len);
			return _hash;
		},
	});
	definePropertys(Number.prototype, {
		hashCode() {
			return this;
		},
		toFixedBefore(before, after) {
			if (!isFinite(this)) {
				return String(this);
			}
			else {
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
		hashCode() {
			return this == true ? -1186256 : -23547257;
		},
	});
	definePropertys(Date, {
		currentTimezone: currentTimezone,
		parseDate(date_str, format, /* = 'yyyyMMddhhmmssfff', */ timezone) {
			var s = String(date_str).replace(/[^0-9]/gm, '');
			var f = '';
			format = format || 'yyyyMMddhhmmssfff';
			format.replace(/(yyyy|MM|dd|hh|mm|ss|fff)/gm, e => {
				f += e;
				return '';
			});
			if (timezone === undefined)
				timezone = currentTimezone;
			var d = new Date();
			var diffTime = currentTimezone - timezone;
			return new Date(Number(s.substr(indexOf(f, 'yyyy'), 4)) || d.getFullYear(), Number(s.substr(indexOf(f, 'MM'), 2) || 1 /*(d.getMonth() + 1)*/) - 1, Number(s.substr(indexOf(f, 'dd'), 2)) || 1 /*d.getDate()*/, Number(s.substr(indexOf(f, 'hh'), 2) || 0 /*d.getHours()*/) - diffTime, Number(s.substr(indexOf(f, 'mm'), 2)) || 0 /*d.getMinutes()*/, Number(s.substr(indexOf(f, 'ss'), 2)) || 0 /*d.getSeconds()*/, Number(s.substr(indexOf(f, 'fff'), 3)) || 0);
		},
		formatTimeSpan(time_span, format = 'dd hh:mm:ss') {
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
				var reg = item[2];
				if (format.match(reg)) {
					start = true;
				}
				else if (start) {
					break;
				}
				time_span = time_span / item[0];
				data.push([time_span % item[1], time_span]);
			}
			if (!start) {
				return format;
			}
			data.indexReverse(0).reverse();
			data.forEach(function (item, index) {
				format =
					format.replace(items[index][2], Math.floor(item[0]).toFixedBefore(2));
			});
			return format;
		},
	});
	definePropertys(Date.prototype, {
		hashCode() {
			return this.valueOf();
		},
		add(ms) {
			this.setMilliseconds(this.getMilliseconds() + ms);
			return this;
		},
		toString(format, timezone) {
			if (format /*typeof format == 'string'*/) {
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
			}
			else {
				return dateToString.call(this);
			}
		},
	});
	const errors = {
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
			captureStackTrace(targetObject, constructorOpt) {
				// TODO ...
			}
		});
	}
	var stackTraceJSON = true;
	definePropertys(Error, {
		new(arg, ...child) {
			var err;
			if (typeof arg == 'object') { // ErrnoCode | Error | ErrorDescribe;
				if (arg instanceof Error) {
					err = arg;
				}
				if (Array.isArray(arg)) { // ErrnoCode
					var errnoCode = arg;
					err = new Error(errnoCode[1] || errnoCode[2] || 'Unknown error');
					err.errno = errnoCode[0];
					err.description = errnoCode[2] || '';
					Error.captureStackTrace(err, Error.new);
				}
				else { // ErrorDescribe
					var describe = arg;
					var Err = (errors[arg.name] || Error);
					var msg = describe.message || describe.error || 'Unknown error';
					err = Object.assign(new Err(msg), arg);
					Error.captureStackTrace(err, Error.new);
				}
			}
			else { // string
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
		toJSON(err) {
			return Error.new(err).toJSON();
		},
		setStackTraceJSON(enable) {
			stackTraceJSON = !!enable;
		},
	});
	definePropertys(Error.prototype, {
		hashCode() {
			var _hash = Object.prototype.hashCode.call(this);
			_hash += (_hash << 5) + this.message.hashCode();
			return _hash;
		},
		toJSON() {
			var err = this;
			var r = Object.assign({}, err);
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
})();
