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

export declare class LiteItem<T> {
	private _host;
	private _prev;
	private _next;
	private _value;
	constructor(host: List<T>, prev: LiteItem<T> | null, next: LiteItem<T> | null, value: T);
	get host(): List<T> | null;
	get prev(): LiteItem<T> | null;
	get next(): LiteItem<T> | null;
	get value(): T | null;
	set value(value: T | null);
}

/**
* @class List linked
*/
export declare class List<T> {
	private _first;
	private _last;
	private _length;
	get first(): LiteItem<T> | null;
	get last(): LiteItem<T> | null;
	get length(): number;
	del(item: LiteItem<T>): LiteItem<T> | null;
	unshift(value: T): LiteItem<T>;
	push(value: T): LiteItem<T>;
	pop(): T | null;
	shift(): T | null;
	clear(): void;
}

/**
	* @class Event
	*/
export declare class Event<Data = any, Return = number, Sender = any> {
	private m_data;
	protected m_noticer: EventNoticer<Data, Return, Sender> | null;
	private m_return_value;
	private m_origin;
	get name(): string;
	get data(): Data;
	get sender(): Sender;
	get origin(): any;
	set origin(value: any);
	get noticer(): EventNoticer<Data, Return, Sender> | null;
	get returnValue(): Return | undefined;
	set returnValue(value: Return | undefined);
	/**
	 * @constructor
	 */
	constructor(data: Data, returnValue?: Return);
}

declare type DefaultEvent = Event;

export interface Listen<Event = DefaultEvent, Scope = any> {
	(evt: Event): any;
}

export interface Listen2<Event = DefaultEvent, Scope = any> {
	(scope: Scope, evt: Event): any;
}

/**
* @class EventNoticer
*/
export declare class EventNoticer<Data = any, Return = number, Sender = any> {
	private m_name;
	private m_sender;
	private m_listens;
	private m_listens_map;
	private m_length;
	private m_enable;
	private _add;
	/**
	 * @get enable {bool} # 获取是否已经启用
	 */
	get enable(): boolean;
	/**
	 * @set enable {bool} # 设置, 启用/禁用
	 */
	set enable(value: boolean);
	/**
	 * @get name {String} # 事件名称
	 */
	get name(): string;
	/**
	 * @get {Object} # 事件发送者
	 */
	get sender(): any;
	/**
	 *
	 * @get {int} # 添加的事件侦听数量
	 */
	get length(): number;
	/**
	 * @constructor
	 * @arg name   {String} # 事件名称
	 * @arg sender {Object} # 事件发起者
	 */
	constructor(name: string, sender: Sender);
	/**
	 * @fun on # 绑定一个事件侦听器(函数)
	 * @arg  listen {Function} #  侦听函数
	 * @arg [scope] {Object}   # 重新指定侦听函数this
	 * @arg [id]  {String}     # 侦听器别名,可通过id删除
	 */
	on<Scope>(listen: Listen<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	/**
	 * @fun once # 绑定一个侦听器(函数),且只侦听一次就立即删除
	 * @arg listen {Function} #         侦听函数
	 * @arg [scope] {Object}  #         重新指定侦听函数this
	 * @arg [id] {String}     #         侦听器别名,可通过id删除
	 */
	once<Scope>(listen: Listen<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	/**
	 * Bind an event listener (function),
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @fun on2
	 * @arg listen {Function}  #              侦听函数
	 * @arg [scope] {Object}   #      重新指定侦听函数this
	 * @arg [id] {String}     #     侦听器别名,可通过id删除
	 */
	on2<Scope>(listen: Listen2<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	/**
	 * Bind an event listener (function), And to listen only once and immediately remove
	 * and "on" the same processor of the method to add the event trigger to receive two parameters
	 * @fun once2
	 * @arg listen {Function}     #           侦听函数
	 * @arg [scope] {Object}      # 重新指定侦听函数this
	 * @arg [id] {String}         # 侦听器id,可通过id删除
	 */
	once2<Scope>(listen: Listen2<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	forward(noticer: EventNoticer<Data, Return, Sender>, id?: string): string;
	forwardOnce(noticer: EventNoticer<Data, Return, Sender>, id?: string): string;
	/**
	 * @fun trigger # 通知所有观察者
	 * @arg data {Object} # 要发送的数据
	 * @ret {Object}
	 */
	trigger(data: Data): Return | undefined;
	/**
	 * @fun triggerWithEvent # 通知所有观察者
	 * @arg data {Object} 要发送的event
	 * @ret {Object}
	 */
	triggerWithEvent(evt: Event<Data, Return, Sender>): Return | undefined;
	/**
	 * @fun off # 卸载侦听器(函数)
	 * @arg [func] {Object}   # 可以是侦听函数,id,如果不传入参数卸载所有侦听器
	 * @arg [scope] {Object}  # scope
	 */
	off(listen?: string | Function | Object, scope?: any): number;
}

/**
* @class Notification
*/
export declare class Notification<Data = any, Return = number, Sender = any> {
	/**
	 * @func getNoticer
	 */
	getNoticer(name: string): EventNoticer<Data, Return, Sender>;
	/**
	 * @func hasNoticer
	 */
	hasNoticer(name: string): boolean;
	/**
	 * @func addDefaultListener
	 */
	addDefaultListener(name: string, listen: Listen<Event<Data, Return, Sender>> | string): string | undefined;
	/**
	 * @func addEventListener(name, listen[,scope[,id]])
	 */
	addEventListener<Scope>(name: string, listen: Listen<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	/**
	 * @func addEventListenerOnce(name, listen[,scope[,id]])
	 */
	addEventListenerOnce<Scope>(name: string, listen: Listen<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	/**
	 * @func addEventListener2(name, listen[,scope[,id]])
	 */
	addEventListener2<Scope>(name: string, listen: Listen2<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	/**
	 * @func addEventListenerOnce2(name, listen[,scope[,id]])
	 */
	addEventListenerOnce2<Scope>(name: string, listen: Listen2<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	addEventForward(name: string, noticer: EventNoticer<Data, Return, Sender>, id?: string): string;
	addEventForwardOnce(noticer: EventNoticer<Data, Return, Sender>, id?: string): string;
	/**
	* @func trigger 通知事监听器
	* @arg name {String}       事件名称
	* @arg data {Object}       要发送的消数据
	*/
	trigger(name: string, data: Data): Return | undefined;
	/**
	* @func triggerWithEvent 通知事监听器
	* @arg name {String}       事件名称
	* @arg event {Event}       Event
	*/
	triggerWithEvent(name: string, event: Event<Data, Return, Sender>): Return | undefined;
	/**
	 * @func removeEventListener(name,[func[,scope]])
	 */
	removeEventListener(name: string, listen?: string | Function | Object, scope?: any): void;
	/**
	 * @func removeEventListenerWithScope(scope) 卸载notification上所有与scope相关的侦听器
	 * @arg scope {Object}
	 */
	removeEventListenerWithScope(scope: any): void;
	/**
	 * @func allNoticers() # Get all event noticer
	 * @ret {Array}
	 */
	allNoticers(): EventNoticer<Data, Return, Sender>[];
	/**
	 * @func triggerListenerChange
	 */
	triggerListenerChange(name: string, count: number, change: number): void;
}

// ======================== interface ========================

export interface Notification<Data = any, Return = number, Sender = any> {
	getNoticer(name: string): EventNoticer<Data, Return, Sender>;
	hasNoticer(name: string): boolean;
	addDefaultListener(name: string, listen: Listen<Event<Data, Return, Sender>> | string): string | undefined;
	addEventListener<Scope>(name: string, listen: Listen<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	addEventListenerOnce<Scope>(name: string, listen: Listen<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	addEventListener2<Scope>(name: string, listen: Listen2<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	addEventListenerOnce2<Scope>(name: string, listen: Listen2<Event<Data, Return, Sender>, Scope>, scope?: Scope | string, id?: string): string;
	addEventForward(name: string, noticer: EventNoticer<Data, Return, Sender>, id?: string): string;
	addEventForwardOnce(noticer: EventNoticer<Data, Return, Sender>, id?: string): string;
	trigger(name: string, data: Data): Return | undefined;
	triggerWithEvent(name: string, event: Event<Data, Return, Sender>): Return | undefined;
	removeEventListener(name: string, listen?: string | Function | Object, scope?: any): void;
	removeEventListenerWithScope(scope: any): void;
	allNoticers(): EventNoticer<Data, Return, Sender>[];
	triggerListenerChange(name: string, count: number, change: number): void;
}

// ======================== IMPL ========================

const _event = __requireNgui__('_event');
const _util = __requireNgui__('_util');
const PREFIX = 'm_on';

Object.assign(exports, _event);

export default (exports.event as (target: any, name: string)=>void);

// ======================================================

/**
 * @class NativeNotification
 */
export class NativeNotification<Data = any, Return = number, Sender = any> extends Notification<Data, Return, Sender>  {

	getNoticer(name: string) {
		var noticer = (this as any)[PREFIX + name] as EventNoticer<Data, Return, Sender>;
		if ( ! noticer ) {
			// bind native event
			var func = (this as any)['trigger' + name];
			// bind native
			_util.addNativeEventListener(this, name, (event?: any, isEvent?: boolean) => {
				//console.log('_util.addNativeEventListener', name);
				var evt = event && isEvent ? event: new _event.Event(event);
				var ok = func ? func.call(this, evt): this.triggerWithEvent(name, evt);
				//console.log('_util.addNativeEventListener', name, ok, String(trigger));
				return ok;
			}, -1);
			(this as any)[PREFIX + name] = noticer = new EventNoticer<Data, Return, Sender>(name, this as any);
		}
		return noticer;
	}
	
}
