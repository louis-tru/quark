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

import utils from './util';
import event, { EventNoticer, NativeNotification, Event, Listen } from './event';
import action from './action';

const _ngui = __requireNgui__('_ngui');

type Matrix = any;

/**
 * @class View
 */
export declare class View {

	onKeyDown: EventNoticer;
	onKeyPress: EventNoticer;
	onKeyUp: EventNoticer;
	onKeyEnter: EventNoticer;
	onBack: EventNoticer;
	onClick: EventNoticer;
	onTouchStart: EventNoticer;
	onTouchMove: EventNoticer;
	onTouchEnd: EventNoticer;
	onTouchCancel: EventNoticer;
	onMouseOver: EventNoticer;
	onMouseOut: EventNoticer;
	onMouseLeave: EventNoticer;
	onMouseEnter: EventNoticer;
	onMouseMove: EventNoticer;
	onMouseDown: EventNoticer;
	onMouseUp: EventNoticer;
	onMouseWheel: EventNoticer;
	onFocus: EventNoticer;
	onBlur: EventNoticer;
	onHighlighted: EventNoticer;
	onActionKeyframe: EventNoticer;
	onActionLoop: EventNoticer;

	prepend(view: View): void;
	append(view: View): void;
	append_text(str: string): void;
	before(view: View): void;
	after(view: View): void;
	remove(): void;
	removeAllChild(): void;
	focus(): void;
	blur(): void;
	layoutOffset(): void;
	layoutOffsetFrom(): void;
	getAction(): void;
	setAction(): void;
	screenRect(): void;
	finalMatrix(): void;
	finalOpacity(): void;
	position(): void;
	overlapTest(): void;
	addClass(cls: string): void;
	removeClass(): void;
	toggleClass(): void;
	firstButton(): void;
	hasChild(): boolean;

	id?: string;

	readonly innerText: string;
	readonly parent: View | null;
	readonly prev: View | null;
	readonly next: View | null;
	readonly first: View | null;
	readonly last: View | null;
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	rotateZ: number;
	skewX: number;
	skewY: number;
	opacity: number;
	visible: boolean;
	readonly finalVisible: boolean;
	readonly drawVisible: boolean;
	readonly translate: any; //, translate, set_translate);
	readonly scale: any; //, scale, set_scale);
	readonly skew: any; //, skew, set_skew);
	originX: number;
	originY: number;
	readonly origin: any; //, origin, set_origin);
	readonly matrix: Matrix;
	readonly level: number;
	needDraw: boolean;
	receive: boolean;
	isFocus: boolean;
	readonly viewType: number; //, view_type);
	readonly class: any; //, classs, set_class);
}

class ViewX extends NativeNotification {

	// @private:
	m_id?: string;
	m_owner = null;
	
	get __view__() { return this }

	@event onKeyDown: EventNoticer;
	@event onKeyPress: EventNoticer;
	@event onKeyUp: EventNoticer;
	@event onKeyEnter: EventNoticer;
	@event onBack: EventNoticer;
	@event onClick: EventNoticer;
	@event onTouchStart: EventNoticer;
	@event onTouchMove: EventNoticer;
	@event onTouchEnd: EventNoticer;
	@event onTouchCancel: EventNoticer;
	@event onMouseOver: EventNoticer;
	@event onMouseOut: EventNoticer;
	@event onMouseLeave: EventNoticer;
	@event onMouseEnter: EventNoticer;
	@event onMouseMove: EventNoticer;
	@event onMouseDown: EventNoticer;
	@event onMouseUp: EventNoticer;
	@event onMouseWheel: EventNoticer;
	@event onFocus: EventNoticer;
	@event onBlur: EventNoticer;
	@event onHighlighted: EventNoticer;
	@event onActionKeyframe: EventNoticer;
	@event onActionLoop: EventNoticer;

	get id() {
		return this.m_id;
	}

	set id(value) {
		ViewController.setID(this, value);
	}

	get owner() {
		return this.m_owner;
	}

	/**
	 * @overwrite
	 */
	addDefaultListener(name: string, listen: Listen<Event<any, number, View>> | string) {
		if ( typeof listen == 'string' ) { // find func 
			var owner: any = this;
			do {
				var func2 = owner[listen];
				if ( typeof func2 == 'function' ) {
					return this.addEventListener(name, func2, owner, '0'); // default id 0
				}
				owner = owner.m_owner;
			} while(owner);
			throw Error.new(`Cannot find a function named "${listen}"`);
		} else {
			return NativeNotification.prototype.addDefaultListener.call(this, name, listen);
		}
	}
	
	/**
	 * @overwrite
	 */
	hashCode() {
		return (this as unknown as View).viewType + 18766898;
	}

	appendTo(parentView: View) {
		(parentView as View).append(this as unknown as View);
		return this;
	}

	afterTo(prevView: View) {
		(prevView as View).after(this as unknown as View);
		return this;
	}

	/**
	 * @get action {Action}
	 */
	get action() { // get action object
		return (this as unknown as View).getAction(); 
	}

	/**
	 * @set action {Action}
	 */
	set action(value) { // set action
		(this as unknown as View).setAction(action.create(value));
	}

	/**
	 * @get style {Object}
	 */	
	get style() {
		return this as unknown as View;
	}

	/**
	 * @set style {Object}
	 */
	set style(value: Dict) {
		for (var key in value) {
			(this as any)[key] = value[key];
		}
	}

	/**
	 * @func transition(style[,delay[,cb]][,cb])
	 * @arg style {Object}
	 * @arg [delay] {uint} ms
	 * @arg [cb] {Funcion}
	 * @ret {KeyframeAction}
	 */
	transition(style: Dict, delay?: number, cb?: ()=>void) { // transition animate
		return action.transition(this, style, delay, cb);
	}
	
	/**
	 * @func show()
	 */
	show() {
		(this as unknown as View).visible = true;
	}

	/**
	 * @func hide()
	 */
	hide() {
		(this as unknown as View).visible = false; 
	}

}

utils.extendClass(_ngui.View, ViewX);

exports.View = _ngui.View;