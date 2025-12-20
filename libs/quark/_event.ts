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

import type {Uint} from './defs';

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
	private _length: Uint;
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
 * @class Event The event data
*/
export class Event<Sender = any, SendData = any> {
	private _sender: any;
	private _data: SendData;
	/** The return value */
	returnValue: Uint = 0;
	/** The Data */
	get data(): SendData { return this._data; }
	/** The sender */
	get sender(): Sender { return this._sender; }
	/** */
	constructor(data: SendData) { this._data = data; }
}

/**
 * @template E,Ctx
 * @callback Listen(this:Ctx,evt:E)any
*/
export interface Listen<E = Event, Ctx extends object = object> {
	(this: Ctx, evt: E): any;
}

/**
 * @template E,Ctx
 * @callback Listen2(self:Ctx,evt:E)any
*/
export interface Listen2<E = Event, Ctx extends object = object> {
	(self: Ctx, evt: E): any;
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

function forwardNoticeNoticer<E extends Event<any, any> = Event>(noticer: EventNoticer<E>, evt: E) {
	let oldSender = (evt as any)._sender;
	try {
		noticer.triggerWithEvent(evt);
	} finally {
		(evt as any)._sender = oldSender;
	}
}

interface ListenItem {
	origin: any,
	listen: Function | null,
	ctx: any,
	id: string,
}

export type DataOf<T> = T extends Event<any, infer D> ? D : never;
export type SenderOf<T> = T extends Event<infer S, any> ? S : never;

/**
 * @class EventNoticer
 * 
 * Event notifier, the core of event listener adding, deleting, triggering and notification
*/
export class EventNoticer<E extends Event = Event> {
	private _name: string;
	private _sender: SenderOf<E>;
	private _listens?: List<ListenItem>;
	private _listens_map?: Map<string, ListIterator<ListenItem>>;
	private _length: Uint = 0

	/* Add event listen */
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

	/** Event name */
	get name(): string {
		return this._name;
	}

	/**
	 * @get sender:any Event sender
	 */
	get sender() {
		return this._sender;
	}

	/**
	 * @get length Number of event listeners
	 */
	get length(): Uint {
		return this._length;
	}
	
	/**
	 * @param name   Event name
	 * @param sender Event sender
	 */
	constructor(name: string, sender: SenderOf<E>) {
		this._name = name;
		this._sender = sender;
	}

	/**
	 * Add an event listener (function)
	 * @param  listen    Listening Function
	 * @param  ctxOrId?  Specify the listener function this or id alias
	 * @param  id?       Listener alias, can be deleted by id
	 * @return Returns the passed `id` or the automatically generated `id`
	 * @example
	 *	```ts
	 *	var ctx = { a:100 }
	 *	var id = screen.onChange.on(function(ev) {
	 *	// Prints: 100
	 *		console.log(this.a)
	 *	}, ctx)
	 *	// Replace Listener
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
	 * Add an event listener (function),
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @param  listen    Listening Function
	 * @param  ctxOrId?  Specify the listener function this or id alias
	 * @param  id?       Listener alias, can be deleted by id
	 * @return Returns the passed `id` or the automatically generated `id`
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

	/** Forward the event to another noticer */ 
	forward(noticer: EventNoticer<E>, id?: string): string {
		check_noticer(noticer);
		return this._add(noticer, { call: forwardNoticeNoticer }, noticer, id);
	}

	/**
	 * Set the lifespan of the listener function with the specified `id`
	 * @param id Listener id
	 * @param lifespan Lifespan, the number of times the listener is valid, default is 1
	 */
	setLifespan(id: string, lifespan: Uint = 1) {
		if (this._length) {
			let item = this._listens_map!.get(id);
			if (item) {
				let last = item.value.listen;
				if (last) {
					lifespan = Math.max(1, Number(lifespan) || 1);
					item.value.listen = (ctx: any, evt: any)=>{
						if (--lifespan <= 0) {
							this.off(id);
						}
						last.call(ctx, evt);
					};
				}
			}
		}
	}

	/**
	 * Notify all observers
	 */
	trigger(data: DataOf<E>) {
		this.triggerWithEvent(new Event(data) as E);
	}

	/**
	 * Notify all observers
	 */
	triggerWithEvent(e: E) {
		if ( this._length ) {
			(e as any)._sender = this._sender;
			let listens = this._listens!;
			let begin = listens.begin, end = listens.end;
			while (begin !== end) {
				let value = begin.value;
				if (value.listen) {
					value.listen.call(value.ctx, e);
					begin = begin.next;
				} else {
					begin = listens.remove(begin);
				}
			}
		}
	}

	/**
	 * Remove listener function
	 * @param listen?
	 * 	It can be a listener function/id alias.
	 * 	If no parameter is passed, all listeners will be uninstalled.
	 * @param ctx? Context object, only valid when `listen` is a function
	 * @return {Uint} Returns the number of deleted listeners
	 */
	off(listen?: Function | string, ctx?: object): Uint {
		if ( !this._length )
			return 0;
		let r = 0;
		if ( !listen ) { // Delete all
			let listens = this._listens!;
			let begin = listens.begin, end = listens.end;
			while ( begin !== end ) {
				begin.value.listen = null; // clear
				begin = begin.next;
				r++;
			}
			this._length = 0;
			this._listens_map = new Map<string, ListIterator<ListenItem>>();
		}
		else if ( listen instanceof Function ) { // 卸载一个监听函数
			let listens = this._listens!;
			let listens_map = this._listens_map!;
			let begin = listens.begin, end = listens.end;
			while ( begin !== end ) {
				let value = begin.value;
				if ( value.listen ) {
					if ( value.origin === listen && (!ctx || value.ctx === ctx) ) {
						this._length--;
						listens_map.delete(value.id);
						begin.value.listen = null;
						r++;
						break;
					}
				}
				begin = begin.next;
			}
		}
		else if ( typeof listen == 'string' ) { // by id delete 
			let id = String(listen);
			let listens_map = this._listens_map!;
			let item = listens_map.get(id);
			if ( item ) {
				this._length--;
				listens_map.delete(id);
				item.value.listen = null; // clear
				r++;
			}
		}
		else { //
			throw new Error('Bad argument.');
		}
		return r;
	}

	/**
	 * Remove all listeners related to `ctx` on this noticer
	*/
	offByCtx(ctx: object): Uint {
		let r = 0;
		let listens = this._listens!;
		let listens_map = this._listens_map!;
		let begin = listens.begin;
		let end = listens.end;
		// 要卸载这个范围上相关的侦听器,包括`EventNoticer`代理
		while ( begin !== end ) {
			let value = begin.value;
			if ( value.listen ) {
				if ( value.ctx === ctx ) {
					this._length--;
					listens_map.delete(value.id);
					begin.value.listen = null; // break; // clear
					r++;
				}
			}
			begin = begin.next;
		}
		return r;
	}
}

const PREFIX = '_on';
const FIND_REG = new RegExp('^' + PREFIX);

/**
 * @class Notification
 *
 * This is a collection of events `EventNoticer`, event triggering and response center
 *
 * Derived types inherited from it can use the `@event` keyword to declare member events
 *
 */
export class Notification<E extends Event = Event> {
	/**
	 * @method getNoticer(name)
	 */
	getNoticer(name: string): EventNoticer<E> {
		let key = PREFIX + name;
		let noticer = (this as any)[key];
		if ( ! noticer ) {
			noticer = new EventNoticer<E>(name, this as SenderOf<E>);
			(this as any)[key] = noticer;
		}
		return noticer;
	}

	/**
	 * @method hasNoticer(name)bool
	 */
	hasNoticer(name: string) {
		return (PREFIX + name) in this;
	}

	/**
	 * @method addDefaultListener(name,listen)
	 */
	addDefaultListener(name: string, listen: Listen<E> | null) {
		if (listen) {
			this.addEventListener(name, listen, '0'); // default id 0
		} else { // delete default listener
			this.removeEventListener(name, '0');
		}
	}

	/**
	 * call: [`EventNoticer.on(listen,ctxOrId?,id?)`]
	 */
	addEventListener<Ctx extends object>(name: string, listen: Listen<E, Ctx>, ctxOrId?: Ctx | string, id?: string): string {
		let del = this.getNoticer(name);
		let r = del.on(listen, ctxOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * call: [`EventNoticer.on2(listen,ctxOrId?,id?)`]
	 */
	addEventListener2<Ctx extends object>(name: string, listen: Listen2<E, Ctx>, ctxOrId?: Ctx | string, id?: string): string {
		let del = this.getNoticer(name);
		let r = del.on2(listen, ctxOrId, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/** 
	 * call: [`EventNoticer.forward(noticer,id?)`]
	*/
	addEventForward(name: string, noticer: EventNoticer<E>, id?: string): string {
		let del = this.getNoticer(name);
		let r = del.forward(noticer, id);
		this.triggerListenerChange(name, del.length, 1);
		return r;
	}

	/**
	 * call: [`EventNoticer.setLifespan(id,lifespan)`]
	 */
	setEventListenerLifespan(name: string, id: string, lifespan: Uint = 1) {
		let noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if (noticer)
			noticer.setLifespan(id, lifespan);
	}

	/**
	* Trigger an event by event name --> [`EventNoticer.trigger(data)`]
	*/
	trigger(name: string, data: DataOf<E>) {
		this.triggerWithEvent(name, new Event(data) as unknown as E);
	}

	/**
	* Trigger an event by name and [`Event`] --> [`EventNoticer.triggerWithEvent(event)`]
	*/
	triggerWithEvent(name: string, event: E) {
		let noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if (noticer) {
			noticer.triggerWithEvent(event);
		}
	}

	/**
	 * Remove listener function
	 * @param name      Event name
	 * @param listen?   It can be a listener function/id alias.
	 *                   If no parameter is passed, all listeners will be uninstalled.
	 * @param ctx?      Context object, only valid when `listen` is a function
	*/
	removeEventListener(name: string, listen?: Function | string, ctx?: object) {
		let noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if (noticer) {
			noticer.off(listen, ctx);
			this.triggerListenerChange(name, noticer.length, -1);
		}
	}

	/**
	 * Delete all listeners related to `ctx` on `notification`
	 *
	 * Actually traverse and call the [`EventNoticer.offByCtx(ctx)`] method
	 *
	 * @example
	 *
	 * ```ts
	 * import event from 'quark/event';
	 * 
	 * class TestNotification extends Notification {
	 * 	\@event readonly onChange;
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
	removeEventListenerByCtx(ctx: object) {
		for ( let noticer of this.allNoticers() ) {
			noticer.offByCtx(ctx);
			this.triggerListenerChange(noticer.name, noticer.length, -1);
		}
	}

	/**
	 * Get all of [`EventNoticer`]
	 */
	allNoticers(): EventNoticer<E>[] {
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
	*/
	triggerListenerChange(name: string, count: Uint, change: Uint) {}
}

/**
 * Typescript decorator
 * @decorator
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