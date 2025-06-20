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

let _id = 0;

function assert(value: any, message?: string) {
	if (!value) {
		throw new Error('assert fail, ' + (message || ''));
	}
}

type RemoveReadonly<T> = {
	-readonly [P in keyof T]: T[P];
};

export interface ListIterator<T> {
	readonly prev: ListIterator<T>;
	readonly next: ListIterator<T>;
	readonly host: List<T> | null; // safe host
	value: T;
}

function List_link<T>(prev: ListIterator<T>, next: ListIterator<T>) {
	(prev as RemoveReadonly<ListIterator<T>>).next = next;
	(next as RemoveReadonly<ListIterator<T>>).prev = prev;
	return next;
}

export class List<T = any> {
	private _length: number;
	private _end: ListIterator<T>;

	get begin() {return this._end.next}
	get end() {return this._end}
	get length() {return this._length}
	get front(): T | undefined {return this._end.next.value}
	get back(): T | undefined {return this._end.prev.value}

	constructor() {
		let end = { host: this as List<T> } as RemoveReadonly<ListIterator<T>>;
		end.prev = end;
		end.next = end;
		this._end = end;
		this._length = 0;
	}

	toArray(): T[] {
		const arr = [] as T[];
		let node = this.begin, end = this._end;
		while (node !== end) {
			arr.push(node.value);
			node = node.next;
		}
		return arr;
	}

	remove(it: ListIterator<T>): ListIterator<T> {
		assert(it.host === this, 'List.erase() it host no match');
		let node = it as RemoveReadonly<ListIterator<T>>;
		if (node !== this._end) {
			let next = List_link(node.prev, node.next);
			node.host = null;
			this._length--;
			return next;
		} else {
			return this._end;
		}
	}

	erase(begin: ListIterator<T>, end: ListIterator<T>) {
		assert(begin.host === this, 'List.erase() begin host no match');
		assert(end.host === this, 'List.erase() end host no match');
		let node = begin as RemoveReadonly<ListIterator<T>>;
		let prev = node.prev;
		while (node !== end) {
			node.host = null;
			node = node.next;
			this._length--;
		}
		List_link(prev, end);
	}

	insert(after: ListIterator<T>, value: T): ListIterator<T> {
		assert(after.host === this, 'List.insert() host no match');
		let node = {host: this as List<T>,value} as ListIterator<T>;
		let next = after as RemoveReadonly<ListIterator<T>>;
		List_link(next.prev, node);
		List_link(node, next);
		this._length++;
		return node;
	}

	splice(it: ListIterator<T>, from_begin: ListIterator<T>, from_end: ListIterator<T>) {
		assert(it.host === this, 'List.splice() assert it.host === this');
		assert(from_begin.host, 'List.splice() assert from_begin.host');
		assert(from_begin.host === from_end.host, 'List.splice() assert from_begin.host === from_end.host');
		assert(from_begin.host !== this, 'List.splice() assert from_begin.host !== this');

		let from = from_begin.host!;
		let start = from_begin as RemoveReadonly<ListIterator<T>>;
		let end = from_end as RemoveReadonly<ListIterator<T>>;
		let cur = it as RemoveReadonly<ListIterator<T>>;
		let start_prev = start.prev;

		List_link(cur.prev, start);
		while (start != end) {
			this._length++;
			from._length--;
			start.host = this;
			start = start.next;
		}
		List_link(end.prev, cur);
		List_link(start_prev, end); // link
	}

	spliceAll(it: ListIterator<T>, from: List<T>) {
		this.splice(it, from._end.next, from._end);
	}

	pushBack(value: T): ListIterator<T> {
		return this.insert(this._end, value);
	}

	pushFront(value: T): ListIterator<T> {
		return this.insert(this._end.next, value);
	}

	popBack() {
		if (this._length)
			this.remove(this._end.prev);
	}

	popFront() {
		if (this._length)
			this.remove(this._end.next);
	}

	clear() {
		this.erase(this._end.next, this._end);
	}
}

// -------------------------------------------------------------------------------------------

/**
 * @class Event 事件数据
*/
export class Event<Sender = any, SendData = any> {
	private _sender: Sender;
	private _data: SendData;
	/** 返回值 */
	returnValue: number = 0;
	/** 数据 */
	get data() { return this._data; }
	/** 发送者 */
	get sender() { return this._sender; }
	constructor(data: SendData) { this._data = data; }
}

/**
 * @template Ctx,E
 * @function Listen = (this: Ctx, evt: E): any
*/
export interface Listen<E = Event, Ctx extends object = object> {
	(this: Ctx, evt: E): any;
}

/**
 * @template Ctx,E
 * @function Listen2 = (self: Ctx, evt: E): any
*/
export interface Listen2<E = Event, Ctx extends object = object> {
	(self: Ctx, evt: E): any;
}

interface ListenItem {
	origin: any,
	listen: Function | null,
	ctx: any,
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

function forwardNoticeNoticer<E>(noticer: EventNoticer<E>, evt: E) {
	let oldSender = (evt as any)._sender;
	try {
		noticer.triggerWithEvent(evt);
	} finally {
		(evt as any)._sender = oldSender;
	}
}

/**
 * @template E
 * @class EventNoticer
 * 
 * 事件通知者,事件监听器添加、删除、触发与通知的核心
*/
export class EventNoticer<E = Event> {
	private _name: string;
	private _sender: any;
	private _listens: List<ListenItem> | null = null;
	private _listens_map: Map<string, ListIterator<ListenItem>> | null = null
	private _length: number = 0

	/* @method add  Add event listen */
	private _add(origin_listen: any, listen: any, ctx: any, id?: string): string {
		let self = this;

		let listens_map = self._listens_map;
		if ( !listens_map ) {
			self._listens = new List();
			self._listens_map = listens_map = new Map();
		}

		if (typeof ctx != 'object') {
			id = String(ctx || ++_id);
			ctx = self._sender;
		} else {
			ctx = ctx || self._sender;
			id = String(id || ++_id);
		}
		id = String(id);

		let value: ListenItem = {
			origin: origin_listen,
			listen: listen,
			ctx: ctx,
			id: id,
		};
		let item = listens_map.get(id);

		if ( item ) { // replace
			item.value = value;
		} else { // add
			listens_map.set(id, self._listens!.pushBack(value));
			self._length++;
		}
		return id;
	}

	/**
	 * @get name {string}  事件名称
	 */
	get name(): string {
		return this._name;
	}
	
	/**
	 * @get {object}  事件发送者
	 */
	get sender() {
		return this._sender;
	}

	/**
	 * 
	 * @get {int}  事件侦听数量
	 */
	get length () {
		return this._length;
	}
	
	/**
	 * @constructor
	 * @param name   {string}  事件名称
	 * @param sender {object}  事件发起者
	 */
	constructor (name: string, sender: object) {
		this._name = name;
		this._sender = sender;
	}

	/**
	 * @template Ctx
	 * @method on(listen[,ctxOrId[,id]]) 绑定一个事件侦听器(函数)
	 * @param  listen    {function}   侦听函数
	 * @param  ctxOrId?  {Ctx|string}    重新指定侦听函数this
	 * @param  id?       {string}      侦听器别名,可通过id删除
	 * @return {string} 返回传入的`id`或自动生成的`id`
	 * 
	 * 
	 *	Example:
	 *
	 *	```ts
	 *	var ctx = { a:100 }
	 *	var id = screen.onChange.on(function(ev) {
	 *	// Prints: 100
	 *		console.log(this.a)
	 *	}, ctx)
	 *	// 替换监听器
	 *	screen.onChange.on(function(ev) {
	 *	// Prints: replace 100
	 *		console.log('replace', this.a)
	 *	}, ctx, id)
	 *	```
	 */
	on<Ctx extends object>(listen: Listen<E, Ctx>, ctxOrId?: Ctx | string, id?: string): string {
		check_fun(listen);
		return this._add(listen, listen, ctxOrId, id);
	}

	/**
	 * @template Ctx
	 * @method once(listen[,ctxOrId[,id]])  绑定一个侦听器(函数),且只侦听一次就立即删除
	 * @param listen    {function}          侦听函数
	 * @param ctxOrId?  {Ctx|string}        重新指定侦听函数this
	 * @param id?       {string}            侦听器别名,可通过id删除
	 * @return {string} 返回传入的`id`或自动生成的`id`
	 */
	once<Ctx extends object>(listen: Listen<E, Ctx>, ctxOrId?: Ctx | string, id?: string): string {
		check_fun(listen);
		let self = this;
		let _id = this._add(listen, {
			call: function (ctx: Ctx, evt: E) {
				self.off(_id);
				listen.call(ctx, evt);
			}
		}, ctxOrId, id);
		return _id;
	}

	/**
	 * Bind an event listener (function),
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @template Ctx
	 * @method on2
	 * @param listen {function}              侦听函数
	 * @param ctxOrId? {Ctx|string}         重新指定侦听函数this
	 * @param id? {string}            侦听器别名,可通过id删除
	 * @return {string} 返回传入的`id`或自动生成的`id`
	 * 
	 * 
	 * Example:
	 * 
	 * ```js
	 * var ctx = { a:100 }
	 * var id = display_port.onChange.on2(function(ctx, ev) {
	 * 	// Prints: 100
	 * 	console.log(ctx.a)
	 * }, ctx)
	 * ```
	 */
	on2<Ctx extends object>(listen: Listen2<E, Ctx>, ctxOrId?: Ctx | string, id?: string): string {
		check_fun(listen);
		return this._add(listen, { call: listen }, ctxOrId, id);
	}

	/**
	 * Bind an event listener (function), And to listen only once and immediately remove
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @template Ctx
	 * @method once2
	 * @param listen {function}      侦听函数
	 * @param ctxOrId? {Ctx|string} 重新指定侦听函数this
	 * @param id? {string}        侦听器id,可通过id删除
	 * @return {string} 返回传入的`id`或自动生成的`id`
	 */
	once2<Ctx extends object>(listen: Listen2<E, Ctx>, ctxOrId?: Ctx | string, id?: string): string {
		check_fun(listen);
		let self = this;
		let _id = this._add(listen, {
			call: function (ctx: Ctx, evt: E) {
				self.off(_id);
				listen(ctx, evt);
			}
		}, ctxOrId, id);
		return _id;
	}

	forward(noticer: EventNoticer<E>, id?: string): string {
		check_noticer(noticer);
		return this._add(noticer, { call: forwardNoticeNoticer }, noticer, id);
	}

	forwardOnce(noticer: EventNoticer<E>, id?: string): string {
		check_noticer(noticer);
		let self = this;
		let _id = this._add(noticer, function(evt: E) {
			self.off(_id);
			forwardNoticeNoticer(noticer, evt);
		}, noticer, id);
		return _id;
	}

	/**
	 * @method trigger(data)     通知所有观察者
	 * @param data {object}  要发送的数据
	 */
	trigger<T>(data: T) {
		this.triggerWithEvent(new Event<any, T>(data) as E);
	}

	/**
	 * @method triggerWithEvent(event) 通知所有观察者
	 * @param event {object} 要发送的event
	 */
	triggerWithEvent(event: E) {
		if ( this._length ) {
			(event as any)._sender = this._sender;
			let listens = this._listens!;
			let begin = listens.begin;
			let end = listens.end;
			while ( begin !== end ) {
				let value = begin.value;
				if ( value.listen ) {
					value.listen.call(value.ctx, event);
					begin = begin.next;
				} else {
					begin = listens.remove(begin);
				}
			}
		}
	}

	/**
	 * @method off([listen[,ctx]])  卸载侦听器(函数)
	 * @param listen? {object|function|object}    可以是侦听函数,id,如果不传入参数卸载所有侦听器
	 * @param ctx?  {object}   ctx
	 * @return {int} 返回删除监听器数量
	 */
	off(listen?: string | Function | object, ctx?: object): number {
		if ( !this._length )
			return 0;
		let r = 0;
		if (listen) {
			if ( typeof listen == 'string' ) { // by id delete 
				let name = String(listen);
				let listens_map = this._listens_map!;
				let item = listens_map.get(name);
				if ( item ) {
					this._length--;
					listens_map.delete(name);
					item.value.listen = null; // clear
					r++;
				}
			} else if ( listen instanceof Function ) { // 要卸载是一个函数
				let listens = this._listens!;
				let listens_map = this._listens_map!;
				let begin = listens.begin;
				let end = listens.end;
				if (ctx) { // 需比较范围
					while ( begin !== end ) {
						let value = begin.value;
						if ( value.listen ) {
							if ( value.origin === listen && value.ctx === ctx ) {
								this._length--;
								listens_map.delete(value.id);
								begin.value.listen = null;
								r++;
								break;
							}
						}
						begin = begin.next;
					}
				} else { // 与这个函数有关系的
					let listens_map = this._listens_map!;
					while ( begin !== end ) {
						let value = begin.value;
						if ( value.listen ) {
							if ( value.origin === listen ) {
								this._length--;
								listens_map.delete(value.id);
								begin.value.listen = null;
								r++;
								break; // clear
							}
						}
						begin = begin.next;
					}
				}
			} else if ( listen instanceof Object ) { // by id ctx
				let listens = this._listens!;
				let listens_map = this._listens_map!;
				let begin = listens.begin;
				let end = listens.end;
				// 要卸载这个范围上相关的侦听器,包括`EventNoticer`代理
				while ( begin !== end ) {
					let value = begin.value;
					if ( value.listen ) {
						if ( value.ctx === listen ) {
							this._length--;
							listens_map.delete(value.id);
							begin.value.listen = null; // break; // clear
							r++;
						}
					}
					begin = begin.next;
				}
			} else { //
				throw new Error('Bad argument.');
			}
		} else { // 全部删除
			let listens = this._listens!;
			let begin = listens.begin;
			let end = listens.end;
			while ( begin !== end ) {
				begin.value.listen = null; // clear
				begin = begin.next;
				r++;
			}
			this._length = 0;
			this._listens_map = new Map<string, ListIterator<ListenItem>>();
		}
		return r;
	}
}

const PREFIX = '_on';
const FIND_REG = new RegExp('^' + PREFIX);

/**
 * @template E
 * @class Notification
 *
 * 这是事件`EventNoticer`的集合，事件触发与响应中心
 *
 * 继承于它的派生类型可以使用`@event`关键字来声明成员事件
 *
 */
export class Notification<E = Event> {
	/**
	 * @method getNoticer(name)
	 */
	getNoticer(name: string): EventNoticer<E> {
		let key = PREFIX + name;
		let noticer = (this as any)[key];
		if ( ! noticer ) {
			noticer = new EventNoticer<E>(name, this as any);
			(this as any)[key] = noticer;
		}
		return noticer;
	}

	/**
	 * @method hasNoticer(name)
	 */
	hasNoticer(name: string) {
		return (PREFIX + name) in this;
	}

	/**
	 * @method addDefaultListener(name,listen)
	 * @param name {string}
	 * @param listen {Listen|null}
	 */
	addDefaultListener(name: string, listen: Listen<E> | null) {
		if (listen) {
			this.addEventListener(name, listen, '0'); // default id 0
		} else { // delete default listener
			this.removeEventListener(name, '0');
		}
	}

	/**
	 * @template Ctx
	 * @method addEventListener(name,listen[,ctxOrId[,id]])
	 * 
	 * call: [`EventNoticer.on(listen[,ctxOrId[,id]])`]
	 * 
	 * @param name {string}
	 * @param listen {Listen}
	 * @param ctxOrId? {Ctx|string}
	 * @param id? {string}
	 * @return {string}
	 */
	addEventListener<Ctx extends object>(name: string, listen: Listen<E, Ctx>, ctxOrId?: Ctx | string, id?: string) {
		let del = this.getNoticer(name);
		let r = del.on(listen, ctxOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * @template Ctx
	 * @method addEventListenerOnce(name,listen[,ctxOrId[,id]])
	 * 
	 * call: [`EventNoticer.once(listen[,ctxOrId[,id]])`]
	 * 
	 * @param name {string}
	 * @param listen {Listen}
	 * @param ctxOrId? {Ctx|string}
	 * @param id? {string}
	 * @return {string}
	 */
	addEventListenerOnce<Ctx extends object>(name: string, listen: Listen<E, Ctx>, ctxOrId?: Ctx | string, id?: string) {
		let del = this.getNoticer(name);
		let r = del.once(listen, ctxOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * @template Ctx
	 * @method addEventListener2(name,listen[,ctxOrId[,id]])
	 * 
	 * call: [`EventNoticer.on2(listen[,ctxOrId[,id]])`]
	 * 
	 * @param name {string}
	 * @param Listen2 {Listen2}
	 * @param ctxOrId? {Ctx|string}
	 * @param id? {string}
	 * @return {string}
	 */
	addEventListener2<Ctx extends object>(name: string, listen: Listen2<E, Ctx>, ctxOrId?: Ctx | string, id?: string) {
		let del = this.getNoticer(name);
		let r = del.on2(listen, ctxOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * @template Ctx
	 * @method addEventListenerOnce2(name,listen[,ctx[,id]])
	 * 
	 * call: [`EventNoticer.once2(listen[,ctxOrId[,id]])`]
	 * 
	 * @param name {string}
	 * @param Listen2 {Listen2}
	 * @param ctxOrId? {Ctx|string}
	 * @param id? {string}
	 * @return {string}
	 */
	addEventListenerOnce2<Ctx extends object>(name: string, listen: Listen2<E, Ctx>, ctxOrId?: Ctx | string, id?: string) {
		let del = this.getNoticer(name);
		let r = del.once2(listen, ctxOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	addEventForward(name: string, noticer: EventNoticer<E>, id?: string) {
		let del = this.getNoticer(name);
		let r = del.forward(noticer, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	addEventForwardOnce(noticer: EventNoticer<E>, id?: string) {
		let del = this.getNoticer(noticer.name);
		let r = del.forwardOnce(noticer, id);
		this.triggerListenerChange(noticer.name, del.length, 1);
		return r;
	}

	/**
	* @method trigger(name[,data])
	* 
	* 通过事件名称触发事件 --> [`EventNoticer.trigger(data)`]
	* 
	* @param name {string}       事件名称
	* @param data {object}       要发送的消数据
	*/
	trigger(name: string, data?: any) {
		this.triggerWithEvent(name, new Event(data) as unknown as E);
	}

	/**
	* @method triggerWithEvent(name[,event])
	* 
	* 通过名称与[`Event`]触发事件 --> [`EventNoticer.triggerWithEvent(event)`]
	* 
	* @param name {string}       事件名称
	* @param event {Event}       Event 
	*/
	triggerWithEvent(name: string, event: E) {
		let noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if (noticer) {
			noticer.triggerWithEvent(event);
		}
	}

	/**
	 * @method removeEventListener(name[,func[,ctx]])
	 */
	removeEventListener(name: string, listen?: string | Function | object, ctx?: object) {
		let noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if (noticer) {
			noticer.off(listen, ctx);
			this.triggerListenerChange(name, noticer.length, -1);
		}
	}

	/**
	 * @method removeEventListenerWithCtx(ctx) 卸载notification上所有与ctx相关的侦听器
	 * 
	 * 卸载`notification`上所有与`ctx`相关的侦听器
	 *
	 * 实际上遍历调用了[`EventNoticer.off(ctx)`]方法
	 *
	 * @param ctx {object}
	 *
	 * Example:
	 *
	 * ```ts
	 * import event from 'quark/event';
	 * 
	 * class TestNotification extends Notification {
	 * 	\@event readonly onChange; // 这里必须以on开始以;分号结束
	 * }
	 *
	 * var notification = new TestNotification();
	 * // Prints: responseonChange 0 100
	 * notification.onChange = function(ev) { // add default listener
	 * 	console.log('responseonChange 0', ev.data)
	 * }
	 * notification.triggerChange(100);
	 *
	 * // Prints: 
	 * // responseonChange 0 200
	 * // responseonChange 1
	 * notification.onChange.on(function(ev) {
	 * 	console.log('responseonChange 1')	
	 * })
	 * notification.triggerWithEvent('change', new Event(200));
	 *
	 * var noticer = notification.onChange;
	 * noticer.off(0) // delete default listener
	 * // Prints: responseonChange 1
	 * notification.triggerChange();
	 *
	 * ```
	 */
	removeEventListenerWithCtx(ctx: object) {
		for ( let noticer of this.allNoticers() ) {
			noticer.off(ctx);
			this.triggerListenerChange(noticer.name, noticer.length, -1);
		}
	}

	/**
	 * @method allNoticers()
	 * 
	 * Get all of [`EventNoticer`]
	 *
	 * @return {[`EventNoticer`][]}
	 */
	allNoticers() {
		let result: EventNoticer<E>[] = [];
		for ( let i in this ) {
			if ( FIND_REG.test(i) ) {
				let noticer = this[i];
				if ( noticer instanceof EventNoticer ) {
					result.push(noticer);
				}
			}
		}
		return result;
	}

	/**
	 * @method triggerListenerChange
	 */
	triggerListenerChange(name: string, count: number, change: number) {}
}

/**
 * @decorator typescript decorator
*/
export function event(target: any, name: string) {
	if (name.substring(0, 2) !== 'on') {
		throw new Error(`event name incorrect format`);
	}
	let event = name.substring(2);
	Object.defineProperty(target, name, {
		configurable: false,
		enumerable: true,
		get() { return this.getNoticer(event) },
		set(listen: Function | null) {
			this.addDefaultListener(event, listen);
		},
	});
}