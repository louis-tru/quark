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

if (typeof window == 'object') { // web
	if (typeof global != 'object') {
		window.global = window;
	}
}

var currentTimezone = new Date().getTimezoneOffset() / -60;
var G_slice = Array.prototype.slice;
var G_hash_code_id = 1;
var G_hash_code_set = new WeakSet();

function illegal_operation() {
	throw new Error('Illegal operation');
}

/**
 * @fun ext_class #  EXT class prototype objects
 */
function extend(obj, extd) {
	for (var i in extd) {
		var desc = Object.getOwnPropertyDescriptor(extd, i);
		desc.enumerable = false;
		Object.defineProperty(obj, i, desc);
	}
}

function hashCode(obj) {
	return 	obj === null ? -1354856:
					obj === undefined ? -3387255: obj.hashCode();
}

extend(Object, {
	/**
	 * @func hashCode(obj)
	*/
	hashCode: hashCode,
});

extend(Object.prototype, {

	/**
	 * @func hashCode()
	 */
	hashCode: function() {
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

extend(Function.prototype, {
	
	/**
	 * @func hashCode()
	 */
	hashCode: function() {
		if (!this.hasOwnProperty('M_hashCode')) {
			Object.defineProperty(this, 'M_hashCode', { 
				enumerable: false, configurable: false, writable: false, value: G_hash_code_id++
			});
		}
		return this.M_hashCode;
	},

	catch: function(catch_func) {
		this.throw = catch_func;
		this.catch = illegal_operation;
		return this;
	},

	/**
	 * @fun err # 捕获回调异常
	 * @arg cb {Function}
	 */
	err: function(cb) {
		if (cb)
			return this.catch(cb.throw);
		return this;
	},
	
	/**
	 * @fun throw # 抛出异常到给回调函数
	 * @arg e {Object}
	 */
	throw: function(e) {
		throw e;
	},

	/**
		* @func setTimeout 延迟执行函数单位毫秒
		* @arg time {Number}  要延迟时间长度单位(毫秒)
		* @arg ...args        提前传入的参数1
		*/
	setTimeout: function(time/*, ...args*/) {
		var self = this;
		var args = G_slice.call(arguments, 1);
		return setTimeout(function() {
			self.apply(null, args);
		}, time);
	},

});

extend(Array, {
	toArray: function (obj, index, end) {
		return G_slice.call(obj, index, end);
	},
});

extend(Array.prototype, {

	/**
	 * @func hashCode()
	 */
	hashCode: function() {
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

	/**
	 * @func deleteValue(val) 移除指定值元素
	 * @arg value {Object}
	 */
	deleteValue: function(value) {
		var i = this.indexOf(value);
		if (i != -1) {
			this.splice(i, 1);
		}
		return this;
	},

	/**
	 * 倒叙索引数组元素
	 */
	last: function (index) {
		return this[this.length - 1 - index];
	},

});

extend(String, {
	format: function(str) {
		return String.prototype.format.apply(str, G_slice.call(arguments, 1));
	}
});

extend(String.prototype, {

	hashCode: function() {
		var _hash = 5381;
		var len = this.length;
		while (len--) 
			_hash += (_hash << 5) + this.charCodeAt(len);
		return _hash;
	},

	/**
	 * var str = 'xxxxxx{0}xxxxx{1}xxxx{2},xxx{0}xxxxx{2}';
	 * var newStr = str.format('A', 'B', 'C');
	 * @ret : xxxxxxAxxxxxBxxxxC,xxxAxxxxxB
	 */
	format: function() {
		var val = String(this);
		for (var i = 0, len = arguments.length; i < len; i++)
			val = val.replace(new RegExp('\\{' + i + '\\}', 'g'), arguments[i]);
		return val;
	}
});

extend(Number.prototype, {

	/**
	 * @func hashCode()
	 */
	hashCode: function() {
		return this;
	},

	/**
	* 转换为前后固定位数的字符串
	* @arg before {Number}  小数点前固定位数
	* @arg [after] {Number} 小数点后固定位数
	*/
	toFixedBefore: function(before, after) {
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
	}

});

extend(Boolean.prototype, {

	/**
	 * @func hashCode()
	 */
	hashCode: function() {
		return this == true ? -1186256: -23547257;
	},
});

// index of
function index_of(str, str1) {
	var index = str.indexOf(str1);
	return index > -1 ? index : Infinity;
}

extend(Date, {

	currentTimezone: currentTimezone,

	/**
	 * 解析字符串为时间
	 * <pre><code>
	 * var i = '2008-02-13 01:12:13';
	 * var date = Date.parseDate(i); //返回的新时间
	 * </code></pre>
	 * @func parseDate(str[,format[,timezone]])
	 * @arg str {String}        要解析的字符串
	 * @arg [format] {String}   date format   default yyyyMMddhhmmssfff
	 * @arg [timezone] {Number} 要解析的时间所在时区,默认为当前时区
	 * @ret {Date}              返回新时间
	 */
	parseDate: function(date_str, format, timezone) {
		var s = String(date_str).replace(/[^0-9]/gm, '');
		var f = '';

		format = format || 'yyyyMMddhhmmssfff';
		format.replace(/(yyyy|MM|dd|hh|mm|ss|fff)/gm, e=>{
			f += e;
		});
		
		if (timezone === undefined)
			timezone = currentTimezone;

		var d = new Date();
		var diffTime = currentTimezone - timezone;

		return new Date(
			Number(s.substr(index_of(f, 'yyyy'), 4)) || d.getFullYear(),
			Number(s.substr(index_of(f, 'MM'), 2) || 1/*(d.getMonth() + 1)*/) - 1,
			Number(s.substr(index_of(f, 'dd'), 2)) || 1/*d.getDate()*/,
			Number(s.substr(index_of(f, 'hh'), 2) || 0/*d.getHours()*/) - diffTime,
			Number(s.substr(index_of(f, 'mm'), 2)) || 0/*d.getMinutes()*/,
			Number(s.substr(index_of(f, 'ss'), 2)) || 0/*d.getSeconds()*/,
			Number(s.substr(index_of(f, 'fff'), 3)) || 0
		);
	},

	 /**
		* 格式化时间戳(单位:毫秒)
		* <pre><code>
		* var time_span = 10002100;
		* var format = 'dd hh:mm:ss';
		* var str = Date.formatTimeSpan(time_span, format); // str = '0 2:46:42'
		* var format = 'dd天hh时mm分ss秒';
		* var str = Date.formatTimeSpan(time_span, format); // str = '0天2时46分42秒'
		* format = 'hh时mm分ss秒';
		* str = Date.formatTimeSpan(time_span, format); // str = '2时46分42秒'
		* format = 'mm分ss秒';
		* str = Date.formatTimeSpan(time_span, format); // str = '166分42秒'
		* </code></pre>
		* @func formatTimeSpan(ts[,format])
		* @arg ts {Number} 要格式化的时间戳
		* @arg [format]  {String} 要格式化的时间戳格式
		* @ret {String} 返回的格式化后的时间戳
		*/
	formatTimeSpan: function(time_span, format) {
		
		format = format || 'dd hh:mm:ss';

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

		data.last(0).reverse();
		data.forEach(function (item, index) {
			format =
				format.replace(items[index][2], Math.floor(item[0]).toFixedBefore(2));
		});
		return format;
	},

});

extend(Date.prototype, {

	/**
	 * @func hashCode()
	 */
	hashCode: function() {
		return this.valueOf();
	},

	/**
	 * @func addMs 给当前Date时间追加毫秒,改变时间值
	 * @arg ms {Number}  要添追加的毫秒值
	 * @ret {Date}
	 */
	addMs: function(ms) {
		this.setMilliseconds(this.getMilliseconds() + ms);
		return this;
	},

});

if (!Date.prototype.rawToString) {
	var rawDateToString = Date.prototype.toString;

	extend(Date.prototype, {
		/**
		* 给定日期格式返回日期字符串
		* <pre><code>
		* var date = new Date();
		* var format = 'yyyy-MM-dd hh:mm:ss.fff';
		* var dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10：32：23'
		* format = 'yyyy-MM-dd hh:mm:ss';
		* dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10：32：23'
		* format = 'yyyy/MM/dd';
		* dateStr = date.toString(format); // dateStr的值为 '2008/12/10'
		* format = 'yyyy-MM-dd hh';
		* dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10'
		* </code></pre>
		* @func date_to_string(date[,foramt])
		* @arg date {Date}
		* @arg [format] {String} 要转换的字符串格式
		* @ret {String} 返回格式化后的时间字符串
		*/
		toString: function(format, timezone) {
			if (typeof format == 'string') {
				var d = new Date(this.valueOf());
				
				if (typeof timezone == 'number') {
					var cur_time_zone = d.getTimezoneOffset() / -60;
					var offset = timezone - cur_time_zone;
					d.setHours(d.getHours() + offset);
				}
				return format.replace('yyyy', d.getFullYear())
					.replace('MM', (d.getMonth() + 1).toFixedBefore(2))
					.replace('dd', d.getDate().toFixedBefore(2))
					.replace('hh', d.getHours().toFixedBefore(2))
					.replace('HH', d.getHours().toFixedBefore(2))
					.replace('mm', d.getMinutes().toFixedBefore(2))
					.replace('ss', d.getSeconds().toFixedBefore(2))
					.replace('fff', d.getMilliseconds().toFixedBefore(3))
			} else {
				return rawDateToString.call(this);
			}
		},
		rawToString: rawDateToString,
	});
}

extend(Error, {

	toJSON: function(err) {
		if ( err ) {
			if ( typeof err == 'string' ) {
				return { message: err || 'unknown error', code: -1, name: '', description: '' };
			} else if ( typeof err == 'number' ) {
				return { message: 'unknown error', code: err, name: '', description: '' };
			} else {
				var r = Object.assign(Object.create(err), err);
				if (typeof r.code == 'string') {
					r.rawCode = r.code;
					r.code = -1;
				}
				r.code = Number(r.code) || -1;
				r.name = r.name || '';
				r.description = r.description || '';
				r.message = r.message || 'unknown error';
				r.stack = r.stack || '';
				return r;
			}
		} else {
			return { message: 'unknown error', code: 0, name: '', description: '' };
		}
		return err;
	},
	
	new: function(e, code) {
		if (! (e instanceof Error)) {
			if (typeof e == 'object') {
				if (Array.isArray(e)) {
					code = e[0];
					var description = e.slice(2).join() || '';
					e = new Error(e[1] || 'Unknown error');
					e.description = description;
				} else {
					var Err = global[e.name] || Error;
					var msg = e.message || e.error || 'Unknown error';
					e = Object.assign(new Err(msg), e);
				}
			} else {
				e = new Error(e);
			}
		}
		e.rawCode = code || e.code;
		e.code = Number(e.rawCode) || -1;
		return e;
	},

	furl: function(err) {
		err = Error.new(err);
		var Err = global[err.name] || Error;
		var msg = err.message +'\n' + Object.entries(err).map(([k,v])=>{
			return k + ': ' + (typeof v == 'object' ? JSON.stringify(v, null, 2): v);
		}).join('\n');
		var r = new Err(msg);
		r.stack = err.stack;
		return r;
	},

});

extend(Error.prototype, {

	/**
	 * @func hashCode()
	 */
	hashCode: function() {
		var _hash = Object.prototype.hashCode.call(this);
		_hash += (_hash << 5) + this.message.hashCode();
		return _hash;
	},

	toJSON: function() {
		return Error.toJSON(this);
	},
});
