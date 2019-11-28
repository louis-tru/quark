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

var _id = 0;

/**
	* @class Event
	*/
class Event {
	
	// m_data: null;
	// m_noticer: null;
	// m_return_value: 0;
	// __has_event: 1;

	get name () {
		return this.m_noticer.name;
	}

	get data () {
		return this.m_data;
	}

	get sender () {
		return this.m_noticer.sender;
	}

	get origin () {
		return this.m_origin;
	}

	set origin(value) {
		this.m_origin = value;
	}

	get noticer () {
		return this.m_noticer;
	}

	get returnValue() {
		return this.m_return_value;
	}

	set returnValue(value) {
		if ( typeof value != 'number' ) {
			throw new TypeError('Bad argument.');
		}
		this.m_return_value = value;
	}

	/**
	 * @constructor
	 */
	constructor(data) {
		this.m_data = data;
	}
	// @end
}

Event.prototype.m_data = null;
Event.prototype.m_noticer = null;
Event.prototype.m_return_value = 0;
Event.prototype.__has_event = 1;
Event.prototype.m_origin = null;

class LiteItem {
	constructor(host, prev, next, value) {
		this._host = host;
		this._prev = prev;
		this._next = next;
		this._value = value;
	}
	get host() { return this._host }
	get prev() { return this._prev }
	get next() { return this._next }
	get value() { return this._value }
}

/**
 * @class List linked 
 */
class List {

	// _first: null
	// _last: null
	// _length: 0

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
		if ( item._host === this ) {
			var prev = item._prev;
			var next = item._next;
			
			if (prev) {
				prev._next = next;
			} else {
				this._first = next;
			}
			if (next) {
				next._prev = prev;
			} else {
				this._last = prev;
			}
			
			item._host = null;
			this._length--;
			return next;
		}
		return null;
	}

	unshift(value) {
		var item;
		if ( this._first ) {
			this._first._prev = item = new LiteItem(this, null, this._first, value);
			this._first = item;
		} else {
			item = new LiteItem(this, null, null, value);
			this._last = item;
			this._first = item;
		}
		this._length++;
		return item;
	}

	push(value) {
		var item;
		if ( this._last ) {
			this._last._next = item = new LiteItem(this, this._last, null, value);
			this._last = item;
		} else {
			item = new LiteItem(this, null, null, value);
			this._last = item;
			this._first = item;
		}
		this._length++;
		return item;
	}

	pop() {
		if ( this._length ) {
			var r = this._last;
			if ( this._length > 1 ) {
				this._last._prev._next = null;
				this._last = this._last._prev;
			} else {
				this._first = null;
				this._last = null;
			}
			this._length--;
			r._host = null;
			return r._value;
		}
	}

	shift() {
		if ( this._length ) {
			var r = this._first;
			if ( this._length > 1 ) {
				this._first._next._prev = null;
				this._first = this._first._next;
			} else {
				this._first = null;
				this._last = null;
			}
			this._length--;
			r._host = null;
			return r._value;
		}
	}

	clear() {
		this._first = null;
		this._last = null;
		this._length = 0;
	}

}

List.prototype._first = null;
List.prototype._last = null;
List.prototype._length = 0;

/* @fun add # Add event listen */
function add(self, origin, listen, scope, id) {

	var listens_map = self.m_listens_map;
	if ( !listens_map ) {
		self.m_listens = new List();
		self.m_listens_map = listens_map = { };
	}

	if (typeof scope != 'object') {
		id = scope || ++_id;
		scope = self.m_sender;
	} else {
		scope = scope || self.m_sender;
		id = id || ++_id;
	}

	id = String(id);

	var value = {
		origin: origin,
		listen: listen,
		scope: scope,
		id: id,
	};
	var item = listens_map[id];

	if ( item ) { // replace
		item._value = value;
	} else { // add
		listens_map[id] = self.m_listens.push(value);
		self.m_length++;
	}
	return id;
}

function check_add(self, origin, listen, scope, id) {
	if ( typeof origin == 'function' ) {
		return add(self, origin, listen, scope, id);
	} else {
		throw new Error('Event listener function type is incorrect ');
	}
}

function notice_proxy_noticer(proxy_noticer, evt) {
	var noticer = evt.m_noticer;
	proxy_noticer.triggerWithEvent(evt);
	evt.m_noticer = noticer;
}

function add_on_noticer(self, noticer, id) {
	return add(self, noticer, { call : notice_proxy_noticer }, noticer, id);
}

function add_once_noticer(self, noticer, id) {
	var _id = add(self, noticer, function (evt) {
		self.off(_id);
		notice_proxy_noticer(noticer, evt);
	}, noticer, id);
	return _id;
}

/**
 * @class EventNoticer
 */
class EventNoticer {

	// m_name: ''
	// m_sender: null
	// m_listens: null
	// m_listens_map: null
	// m_length: 0
	// m_enable: true
	
	/**
	 * @get enable {bool} # 获取是否已经启用
	 */
	get enable () {
		return this.m_enable;
	}
	
	/**
	 * @set enable {bool} # 设置, 启用/禁用
	 */
	set enable (value) {
		this.m_enable = true;
	}
	
	/**
	 * @get name {String} # 事件名称
	 */
	get name () {
		return this.m_name;
	}
	
	/**
	 * @get {Object} # 事件发送者
	 */
	get sender () {
		return this.m_sender;
	}

	/**
	 * 
	 * @get {int} # 添加的事件侦听数量
	 */
	get length () {
		return this.m_length;
	}
	
	/**
	 * @constructor
	 * @arg name   {String} # 事件名称
	 * @arg sender {Object} # 事件发起者
	 */
	constructor (name, sender) {
		this.m_name = name;
		this.m_sender = sender;
		this.m_listens = null;
		this.m_listens_map = null;
		this.m_length = 0;
		this.m_enable = true;
	}
	
	/**
	 * @fun on # 绑定一个事件侦听器(函数)
	 * @arg  listen {Function} #  侦听函数
	 * @arg [scope] {Object}   # 重新指定侦听函数this
	 * @arg [id]  {String}     # 侦听器别名,可通过id删除
	 */
	on(listen, scope, id) {
		if (listen instanceof EventNoticer) {
			return add_on_noticer(this, listen, scope);
		}
		return check_add(this, listen, listen, scope, id);
	}
	
	/**
	 * @fun once # 绑定一个侦听器(函数),且只侦听一次就立即删除
	 * @arg listen {Function} #         侦听函数
	 * @arg [scope] {Object}  #         重新指定侦听函数this
	 * @arg [id] {String}     #         侦听器别名,可通过id删除
	 */
	once(listen, scope, id) {
		if(listen instanceof EventNoticer){
			return add_once_noticer(this, listen);
		}
		var self = this;
		var _id = check_add(this, listen, {
			call: function (scope, evt) {
				self.off(_id); listen.call(scope, evt);
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
	on2(listen, scope, id) {
		if(listen instanceof EventNoticer){
			return add_on_noticer(this, listen);
		}
		return check_add(this, listen, { call: listen }, scope, id);
	}

	/**
	 * Bind an event listener (function), And to listen only once and immediately remove
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @fun once2
	 * @arg listen {Function}     #           侦听函数
	 * @arg [scope] {Object}      # 重新指定侦听函数this
	 * @arg [id] {String}         # 侦听器id,可通过id删除
	 */
	once2(listen, scope, id) {
		if ( listen instanceof EventNoticer ) {
			return add_once_noticer(this, listen, scope);
		}
		var self = this;
		var _id = check_add(this, listen, {
			call: function (scope, evt) {
				self.off(_id); listen(scope, evt);
			}
		},
		scope, id);
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
		if ( this.m_enable && this.m_length ) {
			evt.m_noticer = this;
			var item = this.m_listens._first;
			while ( item ) {
				var value = item._value;
				if ( value ) {
					value.listen.call(value.scope, evt);
					item = item._next;
				} else {
					item = this.m_listens.del(item);
				}
			}
		}
		evt.m_noticer = null;
		return evt.returnValue;
	}

	/**
	 * @fun off # 卸载侦听器(函数)
	 * @arg [func] {Object}   # 可以是侦听函数,id,如果不传入参数卸载所有侦听器
	 * @arg [scope] {Object}  # scope
	 */
	off(func, scope) {
		if ( !this.m_length ) { return }
		if (func) {

			if ( typeof func == 'string' || typeof func == 'number' ) { // by id delete 
				var item = this.m_listens_map[func];
				if ( item ) {
					this.m_length--;
					delete this.m_listens_map[func];
					item._value = null; // clear
				}
			} else if ( func instanceof Function ) { // 要卸载是一个函数
				var item = this.m_listens._first;
				if (scope) { // 需比较范围
					while ( item ) {
						var value = item._value;
						if ( value ) {
							if ( value.origin === func && value.scope === scope ) {
								this.m_length--;
								delete this.m_listens_map[value.id];
								item._value = null; break; // clear
							}
						}
						item = item._next;
					}
				} else { // 与这个函数有关系的
					while ( item ) {
						var value = item._value;
						if ( value ) {
							if ( value.origin === func ) {
								this.m_length--;
								delete this.m_listens_map[value.id];
								item._value = null; break; // clear
							}
						}
						item = item._next;
					}
				}
			} else if ( func instanceof Object ) { //
				var item = this.m_listens._first;
				// 要卸载这个范围上相关的侦听器,包括`EventNoticer`代理
				while ( item ) {
					var value = item._value;
					if ( value ) {
						if ( value.scope === func ) {
							this.m_length--;
							delete this.m_listens_map[value.id];
							item._value = null; // break; // clear
						}
					}
					item = item._next;
				}
			} else { //
				throw new Error('Param err');
			}
		} else { // 全部删除
			var item = this.m_listens._first;
			while ( item ) {
				item._value = null; // clear
				item = item._next;
			}
			this.m_length = 0;
			this.m_listens_map = {};
		}
	}

	// @end
}

exports.List = List;
exports.Event = Event;
exports.EventNoticer = EventNoticer;
