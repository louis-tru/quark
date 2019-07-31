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

/**************************************************************************/

import { Event, Notification } from 'langou/event';
var _util = requireNative('_util');
var _pkg = requireNative('_pkg');
var { haveNode } = _util;

function next_tick(cb, ...args) {
	if (typeof cb != 'function')
		throw new Error('callback must be a function');
	_util.nextTick(function() {
		cb(...args);
	});
}

var _exiting = false;

var listeners = {
	BeforeExit: function(noticer, code = 0) {
		return noticer.triggerWithEvent(new Event(code, code));
	},
	Exit: function(noticer, code = 0) {
		_exiting = true;
		return noticer.triggerWithEvent(new Event(code, code));
	},
	UncaughtException: function(noticer, err) {
		return noticer.length && noticer.trigger(err) === 0;
	},
	UnhandledRejection: function(noticer, reason, promise) {
		return noticer.length && noticer.trigger({ reason, promise }) === 0;
	},
}

class Utils extends Notification {

	getNoticer(name) {
		var noticer = this['__on' + name];
		if ( ! noticer ) {
			var listener = listeners[name];
			if (listener) {
				if (haveNode) {
					var event = name.substr(0, 1).toLowerCase() + name.substr(1);
					process.on(event, function(...args) {
						return listener(noticer, ...args);
					});
				}
				_util[`__on${name}_native`] = function(...args) {
					return listener(noticer, ...args);
				};
			} else {
				// bind native event
				_util[`__on${name}_native`] = function(data) {
					return noticer.trigger(data);
				};
			}
			this['__on' + name] = noticer = new event.EventNoticer(name, this);
		}
		return noticer;
	}

	exit(code) {
		if (!_exiting) {
			_exiting = true;
			delete Utils.prototype.exit;
			if (haveNode) {
				process._exiting = true;
				if (code || code === 0)
					process.exitCode = code;
				try {
					process.emit('exit', process.exitCode || 0);
				} catch(err) {
					console.error(err);
				}
			}
			_util._exit(code || 0);
		}
	}
}

var utils = new Utils();

/**************************************************************************/

var currentTimezone = new Date().getTimezoneOffset() / -60; // 当前时区
var default_throw = Function.prototype.throw;
var id = 10;
var extendObject = _pkg.extendObject;
var assign = Object.assign;
var AsyncFunctionConstructor = (async function() {}).constructor;

function is_async(func) {
	return func && func.constructor === AsyncFunctionConstructor;
}

//
// util
// ======
//
function obj_constructor() { }

function clone_object(new_obj, obj) {
	var names = Object.getOwnPropertyNames(obj);
	for (var i = 0, len = names.length; i < len; i++) {
		var name = names[i];
		var property = Object.getOwnPropertyDescriptor(obj, name);
		if (property.writable) {
			new_obj[name] = clone(property.value);
		}
		//else {
			// Object.defineProperty(new_obj, name, property);
		//}
	}
	return new_obj;
}

function clone(obj) {
	if (obj && typeof obj == 'object') {
		var new_obj = null, i;
		
		switch (obj.constructor) {
			case Object:
				new_obj = { };
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

function extend(obj, extd) {
	if (extd.__proto__ && extd.__proto__ !== Object.prototype) {
		extend(obj, extd.__proto__);
	}
	for (var i of Object.getOwnPropertyNames(extd)) {
		if (i != 'constructor') {
			var desc = Object.getOwnPropertyDescriptor(extd, i);
			desc.enumerable = false;
			Object.defineProperty(obj, i, desc);
		}
	}
	return obj;
}

function extendClass(cls, ...extds) {
	var proto = cls.prototype;
	for (var extd of extds) {
		if (extd instanceof Function) {
			extd = extd.prototype;
		}
		extend(proto, extd);
	}
}

module.exports = exports = extend(extend(utils, _util), {

	// @func hashCode()
	// @func hash()
	// @func version()
	// @func addNativeEventListener()
	// @func removeNativeEventListener()
	// @func garbageCollection()
	// @func runScript()
	// @func transformJsx()
	// @func transformJs()

	/**
	 * @current timezone
	 */
	timezone: currentTimezone,

	/**
	 * @has dev mode
	 */
	dev: !!_pkg.options.dev,
	
	/**
	 * @start argv options
	 */
	options: _pkg.options,
	
	/**
	 * @func resolve(...args)
	 */
	resolve: _pkg.resolve,

	/**
	 * @func isAbsolute(path)
	 */
	isAbsolute: _pkg.isAbsolute,

	/**
	 * Empty function
	 */
	noop: function() { },

	/**
	 * @func isAsync(func)
	 */
	isAsync: function(func) {
		return is_async(func);
	},

	/**
	 * @func nextTick(cb)
	 */
	nextTick: next_tick,
	
	/**
	 * 
	 * @fun assign(ext, ...) `Object.assign` Extended attribute from obj to extd
	 * @arg obj   {Object} 
	 * @arg extd  {Object}
	 * @ret       {Object}
	 */
	assign: assign,

	/**
	 * @func extend(obj, extd)
	 */
	extend: extend,

	/**
	 * @func extendObject(obj, extd)
	 */
	extendObject: extendObject,
	
	/**
	 * @get id
	 */
	get id() {
		return id++;
	},

	/**
	 * @fun err # create error object
	 * @arg e {Object}
	 * @arg [code] {Number}
	 * @ret {Error}
	 */
	err: function(e, code) {
		return Error.new(e, code);
	},

	/**
	 * @fun cb # return default callback
	 * @ret {Function}
	 */
	cb: function(cb) {
		return cb || function () { };
	},

	/**
	 * @fun throw # 抛出异常
	 * @arg err {Object}
	 * @arg [cb] {Function} # 异步回调
	 */
	throw: function(err, cb) {
		exports.cb(cb).throw(exports.err(err));
	},
	
	/**
	 * @func isDefaultThrow
	 */
	isDefaultThrow: function(func) {
		return default_throw === func.throw;
	},
	
	/**
	 * @fun get(name[,self]) # get object value by name
	 * @arg name {String} 
	 * @arg [self] {Object}
	 * @ret {Object}
	 */
	get: function(name, self) {
		var names = name.split('.');
		var item;
		self = self || global;

		while ( (item = names.shift()) ) {
			self = self[item];
			if (!self)
				return self;
		}
		return self;
	},

 /**
	* @fun set(name,value[,self]) # Setting object value by name
	* @arg name {String} 
	* @arg value {Object} 
	* @arg [self] {Object}
	* @ret {Object}
	*/
	set: function(name, value, self) {
		self = self || global;
		var item = null;
		var names = name.split('.');
		name = names.pop();
		
		while ( (item = names.shift()) ){
			self = self[item] || (self[item] = {});
		}
		self[name] = value;
		return self;
	},

	/**
	 * @fun def(name[,self]) # Delete object value by name
	 * @arg name {String} 
	 * @arg [self] {Object}
	 */
	del: function(name, self) {
		var names = name.split('.');
		name = names.pop();
		self = exports.get(names.join('.'), self || global);
		if (self)
			delete self[name];
	},
	
	/**
	 * @fun random # 创建随机数字
	 * @arg [start] {Number} # 开始位置
	 * @arg [end] {Number}   # 结束位置
	 * @ret {Number}
	 */
	random: function(start, end) {
		var r = Math.random();
		start = start || 0;
		end = end || 1E8;
		return Math.floor(start + r * (end - start + 1));
	},

	/**
	* @fun fixRandom # 固定随机值,指定几率返回常数
	* @arg args.. {Number} # 输入百分比
	* @ret {Number}
	*/
	fixRandom: function() {
		var total = 0;
		var argus = [];
		var i = 0;
		
		var len = arguments.length;
		for (; (i < len); i++) {
			var e = arguments[i];
			total += e;
			argus.push(total);
		}

		var r = exports.random(0, total - 1);
		for (i = 0; (i < len); i++) {
			if (r < argus[i])
				return i;
		}
	},

	/**
	 * @fun clone # 克隆一个Object对像
	 * @arg obj {Object} # 要复制的Object对像
	 * @arg {Object}
	 */
	clone: clone,

	/**
	 * @fun wrap
	 * @ret {Object}
	 */
	wrap: function(o) { return { __proto__: o } },
	
	/**
		* @fun filter # object filter
		* @arg obj {Object}  
		* @arg exp {Object}  #   filter exp
		* @arg non {Boolean} #   take non
		* @ret {Object}
		*/
	filter: function(obj, exp, non) {
		var rev = { };
		var isfn = (typeof exp == 'function');
		
		if (isfn || non) {
				for (var key in obj) {
					var value = obj[key];
					var b = isfn ? exp(key, value) : (exp.indexOf(key) != -1);
					if (non ? !b : b)
						rev[key] = value;
				}
		} else {
			exp.forEach(function (item) {
				item = String(item);
				if (item in obj)
					rev[item] = obj[item];
			});
		}
		return rev;
	},
	
	/**
	 * @fun update # update object property value
	 * @arg obj {Object}      #        need to be updated for as
	 * @arg extd {Object}    #         update object
	 * @arg {Object}
	 */
	update: function(obj, extd) {
		for (var key in extd) {
			if (key in obj) {
				obj[key] = exports.select(obj[key], extd[key]);
			}
		}
		return obj;
	},
	
	/**
	 * @fun select
	 * @arg default {Object} 
	 * @arg value   {Object} 
	 * @reg {Object}
	 */
	select: function(default_, value) {
		if ( typeof default_ == typeof value ) {
			return value;
		} else {
			return default_;
		}
	},
	
	/**
	 * @fun extendClass #  EXT class prototype objects
	 */
	extendClass: extendClass,
	
	/**
	 * @fun equalsClass  # Whether this type of sub-types
	 * @arg baseclass {class}
	 * @arg subclass {class}
	 */
	equalsClass: function(baseclass, subclass) {
		if (!baseclass || !subclass) return false;
		if (baseclass === subclass) return true;
		
		var prototype = baseclass.prototype;
		var obj = subclass.prototype.__proto__;
		
		while (obj) {
			if (prototype === obj)
				return true;
			obj = obj.__proto__;
		}
		return false;
	},
	
	/**
	 * @fun assert
	 */
	assert: function(is, code) {
		if (is) {
			return;
		}
		if (Array.isArray(code)) {
			throw Error.new(code);
		} else {
			var args = Array.toArray(arguments);
			if (typeof code == 'number') {
				args = args.slice(2);
			} else {
				args = args.slice(1);
				code = -2;
			}
			if (args.length) {
				throw Error.new(String.format.apply(null, args), code);
			} else {
				throw Error.new('assert fail, unforeseen exceptions', code);
			}
		}
	},

	/**
	 * @get config
	 */ 
	get config() {
		return _pkg.config;
	},

	/**
	 * @func sleep()
	 */
	sleep: function(time, defaultValue) {
		return new Promise((ok, err)=>setTimeout(e=>ok(defaultValue), time));
	},
	
	// @end
});
