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
Object.defineProperty(exports, "__esModule", { value: true });
var _id = 0;
class LiteItem {
	constructor(host, prev, next, value) {
		this._host = host;
		this._prev = prev;
		this._next = next;
		this._value = value;
	}
	get host() { return this._host; }
	get prev() { return this._prev; }
	get next() { return this._next; }
	get value() { return this._value; }
	set value(value) { this._value = value; }
}
exports.LiteItem = LiteItem;
/**
 * @class List linked
 */
class List {
	constructor() {
		this._first = null;
		this._last = null;
		this._length = 0;
	}
	get first() {
		return this._first;
	}
	get last() {
		return this._last;
	}
	get length() {
		return this._length;
	}
	del(item) {
		if (item.host === this) {
			var prev = item.prev;
			var next = item.next;
			if (prev) {
				prev._next = next;
			}
			else {
				this._first = next;
			}
			if (next) {
				next._prev = prev;
			}
			else {
				this._last = prev;
			}
			item._host = null;
			item._prev = null;
			item._next = null;
			this._length--;
			return next;
		}
		return null;
	}
	unshift(value) {
		var item;
		if (this._first) {
			item = new LiteItem(this, null, this._first, value);
			this._first._prev = item;
			this._first = item;
		}
		else {
			item = new LiteItem(this, null, null, value);
			this._last = item;
			this._first = item;
		}
		this._length++;
		return item;
	}
	push(value) {
		var item;
		if (this._last) {
			item = new LiteItem(this, this._last, null, value);
			this._last._next = item;
			this._last = item;
		}
		else {
			item = new LiteItem(this, null, null, value);
			this._last = item;
			this._first = item;
		}
		this._length++;
		return item;
	}
	pop() {
		if (this._length) {
			var r = this._last;
			if (this._length > 1) {
				r.prev._next = null;
				this._last = r.prev;
			}
			else {
				this._first = null;
				this._last = null;
			}
			this._length--;
			r._host = null;
			r._prev = null;
			r._next = null;
			return r.value;
		}
		return null;
	}
	shift() {
		if (this._length) {
			var r = this._first;
			if (this._length > 1) {
				r.next._prev = null;
				this._first = r.next;
			}
			else {
				this._first = null;
				this._last = null;
			}
			this._length--;
			r._host = null;
			r._prev = null;
			r._next = null;
			return r.value;
		}
		return null;
	}
	clear() {
		this._first = null;
		this._last = null;
		this._length = 0;
	}
}
exports.List = List;
/**
	* @class Event
	*/
class Event {
	/**
	 * @constructor
	 */
	constructor(data) {
		this.m_noticer = null;
		this.m_return_value = null;
		this.m_origin = null;
		this.m_data = data;
	}
	get name() {
		return this.m_noticer.name;
	}
	get data() {
		return this.m_data;
	}
	get sender() {
		return this.m_noticer.sender;
	}
	get origin() {
		return this.m_origin;
	}
	set origin(value) {
		this.m_origin = value;
	}
	get noticer() {
		return this.m_noticer;
	}
	get returnValue() {
		return this.m_return_value;
	}
	set returnValue(value) {
		if (!value)
			throw new TypeError('Bad argument.');
		this.m_return_value = value;
	}
}
exports.Event = Event;
function check_noticer(noticer) {
	if (!noticer)
		throw new Error('Event listener function type is incorrect ');
}
function check_fun(origin) {
	if (typeof origin != 'function') {
		throw new Error('Event listener function type is incorrect ');
	}
}
function forwardNoticeNoticer(forward_noticer, evt) {
	var noticer = evt.m_noticer;
	forward_noticer.triggerWithEvent(evt);
	evt.m_noticer = noticer;
}
/**
 * @class EventNoticer
 */
class EventNoticer {
	/**
	 * @constructor
	 * @arg name   {String} # 事件名称
	 * @arg sender {Object} # 事件发起者
	 */
	constructor(name, sender) {
		this.m_listens = null;
		this.m_listens_map = null;
		this.m_length = 0;
		this.m_enable = true;
		this.m_name = name;
		this.m_sender = sender;
	}
	/* @fun add # Add event listen */
	_add(origin_listen, listen, scope, id) {
		var self = this;
		var listens_map = self.m_listens_map;
		if (!listens_map) {
			self.m_listens = new List();
			self.m_listens_map = listens_map = new Map();
		}
		if (typeof scope != 'object') {
			id = String(scope || ++_id);
			scope = self.m_sender;
		}
		else {
			scope = scope || self.m_sender;
			id = String(id || ++_id);
		}
		id = String(id);
		var value = {
			origin: origin_listen,
			listen: listen,
			scope: scope,
			id: id,
		};
		var item = listens_map.get(id);
		if (item) { // replace
			item.value = value;
		}
		else { // add
			listens_map.set(id, self.m_listens.push(value));
			self.m_length++;
		}
		return id;
	}
	/**
	 * @get enable {bool} # 获取是否已经启用
	 */
	get enable() {
		return this.m_enable;
	}
	/**
	 * @set enable {bool} # 设置, 启用/禁用
	 */
	set enable(value) {
		this.m_enable = value;
	}
	/**
	 * @get name {String} # 事件名称
	 */
	get name() {
		return this.m_name;
	}
	/**
	 * @get {Object} # 事件发送者
	 */
	get sender() {
		return this.m_sender;
	}
	/**
	 *
	 * @get {int} # 添加的事件侦听数量
	 */
	get length() {
		return this.m_length;
	}
	/**
	 * @fun on # 绑定一个事件侦听器(函数)
	 * @arg  listen {Function} #  侦听函数
	 * @arg [scope] {Object}   # 重新指定侦听函数this
	 * @arg [id]  {String}     # 侦听器别名,可通过id删除
	 */
	on(listen, scope = this.m_sender, id) {
		check_fun(listen);
		return this._add(listen, listen, scope, id);
	}
	/**
	 * @fun once # 绑定一个侦听器(函数),且只侦听一次就立即删除
	 * @arg listen {Function} #         侦听函数
	 * @arg [scope] {Object}  #         重新指定侦听函数this
	 * @arg [id] {String}     #         侦听器别名,可通过id删除
	 */
	once(listen, scope = this.m_sender, id) {
		check_fun(listen);
		var self = this;
		var _id = this._add(listen, {
			call: function (scope, evt) {
				self.off(_id);
				listen.call(scope, evt);
			}
		}, scope, id);
		return _id;
	}
	/**
	 * Bind an event listener (function),
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @fun on2
	 * @arg listen {Function}  #              侦听函数
	 * @arg [scope] {Object}   #      重新指定侦听函数this
	 * @arg [id] {String}     #     侦听器别名,可通过id删除
	 */
	on2(listen, scope = this.m_sender, id) {
		check_fun(listen);
		return this._add(listen, { call: listen }, scope, id);
	}
	/**
	 * Bind an event listener (function), And to listen only once and immediately remove
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @fun once2
	 * @arg listen {Function}     #           侦听函数
	 * @arg [scope] {Object}      # 重新指定侦听函数this
	 * @arg [id] {String}         # 侦听器id,可通过id删除
	 */
	once2(listen, scope = this.m_sender, id) {
		check_fun(listen);
		var self = this;
		var _id = this._add(listen, {
			call: function (scope, evt) {
				self.off(_id);
				listen(scope, evt);
			}
		}, scope, id);
		return _id;
	}
	forward(noticer, id) {
		check_noticer(noticer);
		return this._add(noticer, { call: forwardNoticeNoticer }, noticer, id);
	}
	forwardOnce(noticer, id) {
		check_noticer(noticer);
		var self = this;
		var _id = this._add(noticer, function (evt) {
			self.off(_id);
			forwardNoticeNoticer(noticer, evt);
		}, noticer, id);
		return _id;
	}
	/**
	 * @fun trigger # 通知所有观察者
	 * @arg data {Object} # 要发送的数据
	 * @ret {Object}
	 */
	trigger(data) {
		return this.triggerWithEvent(new Event(data));
	}
	/**
	 * @fun triggerWithEvent # 通知所有观察者
	 * @arg data {Object} 要发送的event
	 * @ret {Object}
	 */
	triggerWithEvent(evt) {
		if (this.m_enable && this.m_length) {
			evt.m_noticer = this;
			var listens = this.m_listens;
			var item = listens.first;
			while (item) {
				var value = item.value;
				if (value) {
					value.listen.call(value.scope, evt);
					item = item.next;
				}
				else {
					item = listens.del(item);
				}
			}
			evt.m_noticer = null;
		}
		return evt.returnValue;
	}
	/**
	 * @fun off # 卸载侦听器(函数)
	 * @arg [func] {Object}   # 可以是侦听函数,id,如果不传入参数卸载所有侦听器
	 * @arg [scope] {Object}  # scope
	 */
	off(listen, scope) {
		if (!this.m_length) {
			return 0;
		}
		var r = 0;
		if (listen) {
			if (typeof listen == 'string' || typeof listen == 'number') { // by id delete 
				var name = String(listen);
				let listens_map = this.m_listens_map;
				let item = listens_map.get(name);
				if (item) {
					this.m_length--;
					listens_map.delete(name);
					item.value = null; // clear
					r++;
				}
			}
			else if (listen instanceof Function) { // 要卸载是一个函数
				let listens = this.m_listens;
				let listens_map = this.m_listens_map;
				let item = listens.first;
				if (scope) { // 需比较范围
					while (item) {
						let value = item.value;
						if (value) {
							if (value.origin === listen && value.scope === scope) {
								this.m_length--;
								listens_map.delete(value.id);
								item.value = null;
								r++;
								break; // clear
							}
						}
						item = item.next;
					}
				}
				else { // 与这个函数有关系的
					let listens_map = this.m_listens_map;
					while (item) {
						let value = item.value;
						if (value) {
							if (value.origin === listen) {
								this.m_length--;
								listens_map.delete(value.id);
								item.value = null;
								r++;
								break; // clear
							}
						}
						item = item.next;
					}
				}
			}
			else if (listen instanceof Object) { //
				let listens = this.m_listens;
				let listens_map = this.m_listens_map;
				let item = listens.first;
				// 要卸载这个范围上相关的侦听器,包括`EventNoticer`代理
				while (item) {
					var value = item.value;
					if (value) {
						if (value.scope === listen) {
							this.m_length--;
							listens_map.delete(value.id);
							item.value = null; // break; // clear
							r++;
						}
					}
					item = item.next;
				}
			}
			else { //
				throw new Error('Param err');
			}
		}
		else { // 全部删除
			let listens = this.m_listens;
			let item = listens.first;
			while (item) {
				item.value = null; // clear
				item = item.next;
				r++;
			}
			this.m_length = 0;
			this.m_listens_map = new Map();
		}
		return r;
	}
}
exports.EventNoticer = EventNoticer;
const PREFIX = 'm_on';
const FIND_REG = new RegExp('^' + PREFIX);
/**
 * @class Notification
 */
class Notification {
	/**
	 * @func getNoticer
	 */
	getNoticer(name) {
		var noticer = this[PREFIX + name];
		if (!noticer) {
			noticer = new EventNoticer(name, this);
			this[PREFIX + name] = noticer;
		}
		return noticer;
	}
	/**
	 * @func hasNoticer
	 */
	hasNoticer(name) {
		return (PREFIX + name) in this;
	}
	/**
	 * @func addDefaultListener
	 */
	addDefaultListener(name, listen) {
		if (typeof listen == 'string') {
			var func = this[listen]; // find func 
			if (typeof func == 'function') {
				return this.addEventListener(name, func, '0'); // default id 0
			}
			else {
				throw Error.new(`Cannot find a function named "${listen}"`);
			}
		}
		else {
			if (listen) {
				return this.addEventListener(name, listen, '0'); // default id 0
			}
			else { // delete default listener
				this.removeEventListener(name, '0');
			}
		}
	}
	/**
	 * @func addEventListener(name, listen[,scope[,id]])
	 */
	addEventListener(name, listen, scope, id) {
		var del = this.getNoticer(name);
		var r = del.on(listen, scope, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}
	/**
	 * @func addEventListenerOnce(name, listen[,scope[,id]])
	 */
	addEventListenerOnce(name, listen, scope, id) {
		var del = this.getNoticer(name);
		var r = del.once(listen, scope, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}
	/**
	 * @func addEventListener2(name, listen[,scope[,id]])
	 */
	addEventListener2(name, listen, scope, id) {
		var del = this.getNoticer(name);
		var r = del.on2(listen, scope, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}
	/**
	 * @func addEventListenerOnce2(name, listen[,scope[,id]])
	 */
	addEventListenerOnce2(name, listen, scope, id) {
		var del = this.getNoticer(name);
		var r = del.once2(listen, scope, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}
	addEventForward(name, noticer, id) {
		var del = this.getNoticer(name);
		var r = del.forward(noticer, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}
	addEventForwardOnce(noticer, id) {
		var del = this.getNoticer(name);
		var r = del.forwardOnce(noticer, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}
	/**
	* @func trigger 通知事监听器
	* @arg name {String}       事件名称
	* @arg data {Object}       要发送的消数据
	*/
	trigger(name, data) {
		return this.triggerWithEvent(name, new Event(data));
	}
	/**
	* @func triggerWithEvent 通知事监听器
	* @arg name {String}       事件名称
	* @arg event {Event}       Event
	*/
	triggerWithEvent(name, event) {
		var noticer = this[PREFIX + name];
		if (noticer) {
			return noticer.triggerWithEvent(event);
		}
		return event.returnValue;
	}
	/**
	 * @func removeEventListener(name,[func[,scope]])
	 */
	removeEventListener(name, listen, scope) {
		var noticer = this[PREFIX + name];
		if (noticer) {
			noticer.off(listen, scope);
			this.triggerListenerChange(name, noticer.length, -1);
		}
	}
	/**
	 * @func removeEventListenerWithScope(scope) 卸载notification上所有与scope相关的侦听器
	 * @arg scope {Object}
	 */
	removeEventListenerWithScope(scope) {
		for (let noticer of this.allNoticers()) {
			noticer.off(scope);
			this.triggerListenerChange(name, noticer.length, -1);
		}
	}
	/**
	 * @func allNoticers() # Get all event noticer
	 * @ret {Array}
	 */
	allNoticers() {
		var result = [];
		for (var i in this) {
			if (FIND_REG.test(i)) {
				var noticer = this[i];
				if (noticer instanceof EventNoticer) {
					result.push(noticer);
				}
			}
		}
		return result;
	}
	/**
	 * @func triggerListenerChange
	 */
	triggerListenerChange(name, count, change) { }
}
exports.Notification = Notification;
function event(target, name) {
	if (name.substr(0, 2) !== 'on') {
		throw new Error(`event name incorrect format`);
	}
	var event = name.substr(2);
	Object.defineProperty(target, name, {
		configurable: false,
		enumerable: false,
		get() { return this.getNoticer(event); },
		set(listen) { this.addDefaultListener(event, listen); },
	});
}
exports.event = event;
