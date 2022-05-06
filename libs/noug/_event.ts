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

export class ListItem<T> {
	private _host: List<T> | null;
	private _prev: ListItem<T> | null; 
	private _next: ListItem<T> | null;
	private _value: T;
	constructor(host: List<T>, prev: ListItem<T> | null, next: ListItem<T> | null, value: T) {
		this._host = host;
		this._prev = prev;
		this._next = next;
		this._value = value;
	}
	get host() { return this._host }
	get prev() { return this._prev }
	get next() { return this._next }
	get value(): T { return this._value }
	set value(value: T) { this._value = value }
}

/**
 * @class List linked 
 */
export class List<T> {

	private _first: ListItem<T> | null = null;
	private _last: ListItem<T> | null = null;
	private _length: number = 0;

	get first() {
		return this._first;
	}

	get last() {
		return this._last;
	}

	get length() {
		return this._length;
	}

	del(item: ListItem<T>) {
		if ( item.host === this ) {
			var prev = item.prev;
			var next = item.next;
			if (prev) {
				(<any>prev)._next = next;
			} else {
				this._first = next;
			}
			if (next) {
				(<any>next)._prev = prev;
			} else {
				this._last = prev;
			}
			(<any>item)._host = null;
			(<any>item)._prev = null;
			(<any>item)._next = null;
			this._length--;
			return next;
		}
		return null;
	}

	unshift(value: T): ListItem<T> {
		var item: ListItem<T>;
		if ( this._first ) {
			item = new ListItem(this, null, this._first, value);
			(<any>this._first)._prev = item;
			this._first = item;
		} else {
			item = new ListItem(this, null, null, value);
			this._last = item;
			this._first = item;
		}
		this._length++;
		return item;
	}

	push(value: T): ListItem<T> {
		var item: ListItem<T>;
		if ( this._last ) {
			item = new ListItem(this, this._last, null, value);
			(<any>this._last)._next = item;
			this._last = item;
		} else {
			item = new ListItem(this, null, null, value);
			this._last = item;
			this._first = item;
		}
		this._length++;
		return item;
	}

	pop(): T | null {
		if ( this._length ) {
			var r = <ListItem<T>>this._last;
			if ( this._length > 1 ) {
				(<any>r.prev)._next = null;
				this._last = r.prev;
			} else {
				this._first = null;
				this._last = null;
			}
			this._length--;
			(<any>r)._host = null;
			(<any>r)._prev = null;
			(<any>r)._next = null;
			return r.value;
		}
		return null;
	}

	shift(): T | null {
		if ( this._length ) {
			var r= <ListItem<T>>this._first;
			if ( this._length > 1 ) {
				(<any>r.next)._prev = null;
				this._first = r.next;
			} else {
				this._first = null;
				this._last = null;
			}
			this._length--;
			(<any>r)._host = null;
			(<any>r)._prev = null;
			(<any>r)._next = null;
			return r.value;
		}
		return null;
	}

	insert(prev: ListItem<T>, value: T) {
		if (prev.host !== this)
			throw 'Bad argument.';

		var _prev = prev as any;
		var item: ListItem<T>;

		if (_prev._next) {
			item = new ListItem(this, _prev, _prev._next, value);
			_prev._next._prev = item;
			_prev._next = item;
		} else {
			item = new ListItem(this, _prev, null, value);
			_prev._next = item;
			this._last = item;
		}

		this._length++;
		return item;
	}

	clear() {
		this._first = null;
		this._last = null;
		this._length = 0;
	}

}

/**
	* @class Event
	*/
export class Event<Sender = any, Data = any, Origin = any> {
	private _data: Data;
	protected _noticer: any; //EventNoticer<Event<Data, Sender>> | null; // = null;
	private _origin: any; // = null;

	returnValue: number = 0;

	get name() {
		return (this._noticer as EventNoticer<Event<Sender, Data, Origin>>).name;
	}

	get data () {
		return this._data;
	}

	get sender(): Sender {
		return (this._noticer as EventNoticer<Event<Sender, Data, Origin>>).sender as Sender;
	}

	get origin () {
		return this._origin;
	}

	get noticer () {
		return this._noticer as EventNoticer<Event<Sender, Data, Origin>> | null;
	}

	constructor(data: Data, origin?: Origin) {
		this._data = data;
		this._origin = origin;
	}
}

(Event as any).prototype._noticer = null;
(Event as any).prototype._origin = null;

type DefaultEvent = Event;

export interface Listen<Event = DefaultEvent, Scope extends object = object> {
	(this: Scope, evt: Event): any;
}

export interface Listen2<Event = DefaultEvent, Scope extends object = object> {
	(self: Scope, evt: Event): any;
}

interface ListenItem {
	origin: any,
	listen: Function | null,
	scope: any,
	id: string,
}

function check_noticer(noticer: any) {
	if ( !(noticer as EventNoticer) )
		throw new Error('Event listener function type is incorrect ');
}

function check_fun(origin: any) {
	if ( typeof origin != 'function' ) {
		throw new Error('Event listener function type is incorrect ');
	}
}

function forwardNoticeNoticer<E>(forward_noticer: EventNoticer<E>, evt: E) {
	try {
		var noticer = (evt as any)._noticer;
		forward_noticer.triggerWithEvent(evt);
	} finally {
		(evt as any)._noticer = noticer;
	}
}

// export interface EventNoticer<E = DefaultEvent> extends Listen<E> {}

export class EventNoticer<E = DefaultEvent> {

	private m_name: string;
	private m_sender: any;
	private m_listens: List<ListenItem> | null = null;
	private m_listens_map: Map<string, ListItem<ListenItem>> | null = null
	private m_length: number = 0
	private m_enable: boolean = true

	/* @fun add # Add event listen */
	private _add(origin_listen: any, listen: any, scope: any, id?: string): string {
		var self = this;

		var listens_map = self.m_listens_map;
		if ( !listens_map ) {
			self.m_listens = new List();
			self.m_listens_map = listens_map = new Map();
		}

		if (typeof scope != 'object') {
			id = String(scope || ++_id);
			scope = self.m_sender;
		} else {
			scope = scope || self.m_sender;
			id = String(id || ++_id);
		}

		id = String(id);

		var value: ListenItem = {
			origin: origin_listen,
			listen: listen,
			scope: scope,
			id: id,
		};
		var item = listens_map.get(id);

		if ( item ) { // replace
			item.value = value;
		} else { // add
			listens_map.set(id, (<List<ListenItem>>self.m_listens).push(value));
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
	set enable(value: boolean) {
		this.m_enable = value;
	}
	
	/**
	 * @get name {String} # 事件名称
	 */
	get name(): string {
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
	get length () {
		return this.m_length;
	}
	
	/**
	 * @constructor
	 * @arg name   {String} # 事件名称
	 * @arg sender {Object} # 事件发起者
	 */
	constructor (name: string, sender: object) {
		this.m_name = name;
		this.m_sender = sender;
	}

	/**
	 * @fun on # 绑定一个事件侦听器(函数)
	 * @arg  listen {Function} #  侦听函数
	 * @arg [scope] {Object}   # 重新指定侦听函数this
	 * @arg [id]  {String}     # 侦听器别名,可通过id删除
	 */
	on<Scope extends object>(listen: Listen<E, Scope>, scopeOrId?: Scope | string, id?: string): string {
		check_fun(listen);
		return this._add(listen, listen, scopeOrId, id);
	}

	/**
	 * @fun once # 绑定一个侦听器(函数),且只侦听一次就立即删除
	 * @arg listen {Function} #         侦听函数
	 * @arg [scope] {Object}  #         重新指定侦听函数this
	 * @arg [id] {String}     #         侦听器别名,可通过id删除
	 */
	once<Scope extends object>(listen: Listen<E, Scope>, scopeOrId?: Scope | string, id?: string): string {
		check_fun(listen);
		var self = this;
		var _id = this._add(listen, {
			call: function (scope: Scope, evt: E) {
				self.off(_id);
				listen.call(scope, evt);
			}
		}, scopeOrId, id);
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
	on2<Scope extends object>(listen: Listen2<E, Scope>, scopeOrId?: Scope | string, id?: string): string {
		check_fun(listen);
		return this._add(listen, { call: listen }, scopeOrId, id);
	}

	/**
	 * Bind an event listener (function), And to listen only once and immediately remove
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @fun once2
	 * @arg listen {Function}     #           侦听函数
	 * @arg [scope] {Object}      # 重新指定侦听函数this
	 * @arg [id] {String}         # 侦听器id,可通过id删除
	 */
	once2<Scope extends object>(listen: Listen2<E, Scope>, scopeOrId?: Scope | string, id?: string): string {
		check_fun(listen);
		var self = this;
		var _id = this._add(listen, {
			call: function (scope: Scope, evt: E) {
				self.off(_id);
				listen(scope, evt);
			}
		}, scopeOrId, id);
		return _id;
	}

	forward(noticer: EventNoticer<E>, id?: string): string {
		check_noticer(noticer);
		return this._add(noticer, { call: forwardNoticeNoticer }, noticer, id);
	}

	forwardOnce(noticer: EventNoticer<E>, id?: string): string {
		check_noticer(noticer);
		var self = this;
		var _id = this._add(noticer, function(evt: E) {
			self.off(_id);
			forwardNoticeNoticer(noticer, evt);
		}, noticer, id);
		return _id;
	}

	/**
	 * @fun trigger # 通知所有观察者
	 * @arg data {Object} # 要发送的数据
	 */
	trigger(data?: any) {
		this.triggerWithEvent(new Event(data) as unknown as E);
	}

	/**
	 * @fun triggerWithEvent # 通知所有观察者
	 * @arg evt {Object} 要发送的event
	 */
	triggerWithEvent(evt: E) {
		if ( this.m_enable && this.m_length ) {
			(evt as any)._noticer = this;
			var listens = this.m_listens as List<ListenItem>;
			var item = listens.first;
			while ( item ) {
				var value = item.value;
				if ( value.listen ) {
					value.listen.call(value.scope, evt);
					item = item.next;
				} else {
					item = listens.del(item);
				}
			}
			(evt as any)._noticer = null;
		}
	}

	/**
	 * @fun off # 卸载侦听器(函数)
	 * @arg [func] {Object}   # 可以是侦听函数,id,如果不传入参数卸载所有侦听器
	 * @arg [scope] {Object}  # scope
	 */
	off(listen?: string | Function | object, scope?: object): number {
		if ( !this.m_length ) {
			return 0;
		}
		var r = 0;
		if (listen) {
			if ( typeof listen == 'string' ) { // by id delete 
				var name = String(listen);
				let listens_map = <Map<string, ListItem<ListenItem>>>this.m_listens_map;
				let item = listens_map.get(name);
				if ( item ) {
					this.m_length--;
					listens_map.delete(name);
					item.value.listen = null; // clear
					r++;
				}
			} else if ( listen instanceof Function ) { // 要卸载是一个函数
				let listens = <List<ListenItem>>this.m_listens;
				let listens_map = <Map<string, ListItem<ListenItem>>>this.m_listens_map;
				let item = listens.first;
				if (scope) { // 需比较范围
					while ( item ) {
						let value = item.value;
						if ( value.listen ) {
							if ( value.origin === listen && value.scope === scope ) {
								this.m_length--;
								listens_map.delete(value.id);
								item.value.listen = null;
								r++;
								break; // clear
							}
						}
						item = item.next;
					}
				} else { // 与这个函数有关系的
					let listens_map = <Map<string, ListItem<ListenItem>>>this.m_listens_map;
					while ( item ) {
						let value = item.value;
						if ( value.listen ) {
							if ( value.origin === listen ) {
								this.m_length--;
								listens_map.delete(value.id);
								item.value.listen = null;
								r++;
								break; // clear
							}
						}
						item = item.next;
					}
				}
			} else if ( listen instanceof Object ) { //
				let listens = <List<ListenItem>>this.m_listens;
				let listens_map = <Map<string, ListItem<ListenItem>>>this.m_listens_map;
				let item = listens.first;
				// 要卸载这个范围上相关的侦听器,包括`EventNoticer`代理
				while ( item ) {
					var value = item.value;
					if ( value.listen ) {
						if ( value.scope === listen ) {
							this.m_length--;
							listens_map.delete(value.id);
							item.value.listen = null; // break; // clear
							r++;
						}
					}
					item = item.next;
				}
			} else { //
				throw new Error('Bad argument.');
			}
		} else { // 全部删除
			let listens = <List<ListenItem>>this.m_listens;
			let item = listens.first;
			while ( item ) {
				item.value.listen = null; // clear
				item = item.next;
				r++;
			}
			this.m_length = 0;
			this.m_listens_map = new Map<string, ListItem<ListenItem>>();
		}
		return r;
	}

}

export const VOID = {} as any;

const PREFIX = 'm_on';
const FIND_REG = new RegExp('^' + PREFIX);

/**
 * @class Notification
 */
export class Notification<E = DefaultEvent> {

	/**
	 * @func getNoticer
	 */
	getNoticer(name: string): EventNoticer<E> {
		var noticer = (<any>this)[PREFIX + name];
		if ( ! noticer ) {
			noticer = new EventNoticer<E>(name, this as any);
			(<any>this)[PREFIX + name] = noticer;
		}
		return noticer;
	}

	/**
	 * @func hasNoticer
	 */
	hasNoticer(name: string) {
		return (PREFIX + name) in this;
	}

	/**
	 * @func addDefaultListener
	 */
	addDefaultListener(name: string, listen: Listen<E> | null) {
		if (listen) {
			this.addEventListener(name, listen, '0'); // default id 0
		} else { // delete default listener
			this.removeEventListener(name, '0');
		}
	}

	/**
	 * @func addEventListener(name, listen[,scope[,id]])
	 */
	addEventListener<Scope extends object>(name: string, listen: Listen<E, Scope>, scopeOrId?: Scope | string, id?: string) {
		var del = this.getNoticer(name);
		var r = del.on(listen, scopeOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * @func addEventListenerOnce(name, listen[,scope[,id]])
	 */
	addEventListenerOnce<Scope extends object>(name: string, listen: Listen<E, Scope>, scopeOrId?: Scope | string, id?: string) {
		var del = this.getNoticer(name);
		var r = del.once(listen, scopeOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * @func addEventListener2(name, listen[,scope[,id]])
	 */
	addEventListener2<Scope extends object>(name: string, listen: Listen2<E, Scope>, scopeOrId?: Scope | string, id?: string) {
		var del = this.getNoticer(name);
		var r = del.on2(listen, scopeOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * @func addEventListenerOnce2(name, listen[,scope[,id]])
	 */
	addEventListenerOnce2<Scope extends object>(name: string, listen: Listen2<E, Scope>, scopeOrId?: Scope | string, id?: string) {
		var del = this.getNoticer(name);
		var r = del.once2(listen, scopeOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	addEventForward(name: string, noticer: EventNoticer<E>, id?: string) {
		var del = this.getNoticer(name);
		var r = del.forward(noticer, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	addEventForwardOnce(noticer: EventNoticer<E>, id?: string) {
		var del = this.getNoticer(noticer.name);
		var r = del.forwardOnce(noticer, id);
		this.triggerListenerChange(noticer.name, del.length, 1);
		return r;
	}

	/**
	* @func trigger 通知事监听器
	* @arg name {String}       事件名称
	* @arg data {Object}       要发送的消数据
	*/
	trigger(name: string, data?: any) {
		this.triggerWithEvent(name, new Event(data) as unknown as E);
	}

	/**
	* @func triggerWithEvent 通知事监听器
	* @arg name {String}       事件名称
	* @arg event {Event}       Event 
	*/
	triggerWithEvent(name: string, event: E) {
		var noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if (noticer) {
			noticer.triggerWithEvent(event);
		}
	}

	/**
	 * @func removeEventListener(name,[func[,scope]])
	 */
	removeEventListener(name: string, listen?: string | Function | object, scope?: object) {
		var noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if (noticer) {
			noticer.off(listen, scope);
			this.triggerListenerChange(name, noticer.length, -1);
		}
	}

	/**
	 * @func removeEventListenerWithScope(scope) 卸载notification上所有与scope相关的侦听器
	 * @arg scope {Object}
	 */
	removeEventListenerWithScope(scope: object) {
		for ( let noticer of this.allNoticers() ) {
			noticer.off(scope);
			this.triggerListenerChange(noticer.name, noticer.length, -1);
		}
	}

	/**
	 * @func allNoticers() # Get all event noticer
	 * @ret {Array}
	 */
	allNoticers() {
		var result: EventNoticer<E>[] = [];
		for ( var i in this ) {
			if ( FIND_REG.test(i) ) {
				var noticer = this[i];
				if ( noticer instanceof EventNoticer ) {
					result.push(noticer);
				}
			}
		}
		return result;
	}

	/**
	 * @func triggerListenerChange
	 */
	triggerListenerChange(name: string, count: number, change: number) {/*NOOP*/}

}

export function event(target: any, name: string) {
	if (name.substr(0, 2) !== 'on') {
		throw new Error(`event name incorrect format`);
	}
	var event = name.substr(2);
	Object.defineProperty(target, name, {
		configurable: false,
		enumerable: true,
		get() { return this.getNoticer(event) },
		set(listen: Function | null) {
			if (listen !== VOID)
				this.addDefaultListener(event, listen);
		},
	});
}