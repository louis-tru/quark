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

import {
	Notification, EventNoticer, GUIEvent,
	GUIHighlightedEvent, GUIKeyEvent,
	GUIClickEvent, GUITouchEvent,
	GUIMouseEvent, GUIActionEvent,
} from './event';
import {Mat,Vec2,Rect} from './value';
import ViewController from './ctr';
import {Action, Options as ActionOpt} from './_action';

/**
 * @class View
 */
export declare class View extends Notification<GUIEvent> {
	// events
	readonly onKeyDown: EventNoticer<GUIKeyEvent>;
	readonly onKeyPress: EventNoticer<GUIKeyEvent>;
	readonly onKeyUp: EventNoticer<GUIKeyEvent>;
	onKeyEnter: EventNoticer<GUIKeyEvent>;
	onBack: EventNoticer<GUIClickEvent>;
	onClick: EventNoticer<GUIClickEvent>;
	onTouchStart: EventNoticer<GUITouchEvent>;
	onTouchMove: EventNoticer<GUITouchEvent>;
	onTouchEnd: EventNoticer<GUITouchEvent>;
	onTouchCancel: EventNoticer<GUITouchEvent>;
	onMouseOver: EventNoticer<GUIMouseEvent>;
	onMouseOut: EventNoticer<GUIMouseEvent>;
	onMouseLeave: EventNoticer<GUIMouseEvent>;
	onMouseEnter: EventNoticer<GUIMouseEvent>;
	onMouseMove: EventNoticer<GUIMouseEvent>;
	onMouseDown: EventNoticer<GUIMouseEvent>;
	onMouseUp: EventNoticer<GUIMouseEvent>;
	onMouseWheel: EventNoticer<GUIMouseEvent>;
	onFocus: EventNoticer<GUIEvent>;
	onBlur: EventNoticer<GUIEvent>;
	onHighlighted: EventNoticer<GUIHighlightedEvent>;
	onActionKeyframe: EventNoticer<GUIActionEvent>;
	onActionLoop: EventNoticer<GUIActionEvent>;
	// methods
	prepend(view: View): void;
	append(view: View): void;
	appendText(str: string): void;
	before(view: View): void;
	after(view: View): void;
	remove(): void;
	removeAllChild(): void;
	focus(): boolean;
	blur(): boolean;
	layoutOffset(): Vec2;
	layoutOffsetFrom(parents: View): Vec2;
	getAction(): Action | null;
	private _setAction(action: Action | null): void;
	screenRect(): Rect;
	finalMatrix(): Mat;
	finalOpacity(): number;
	position(): Vec2;
	overlapTest(): boolean;
	addClass(cls: string): void;
	removeClass(): void;
	toggleClass(): void;
	firstButton(): View | null;
	hasChild(): boolean;
	// props
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
	translate: Vec2;
	scale: Vec2;
	skew: Vec2;
	originX: number;
	originY: number;
	origin: Vec2;
	readonly matrix: Mat;
	readonly level: number;
	needDraw: boolean;
	receive: boolean;
	isFocus: boolean;
	readonly viewType: number;
	class: string;
	// ext
	id: string;
	readonly owner: ViewController | null;
	action: Action | null;
	style: Dict;
	setAction(action: ActionOpt | null): void;
	hashCode(): number;
	appendTo(parentView: View): this;
	afterTo(prevView: View): this;
	transition(style: Dict, delay?: number, cb?: (e: GUIActionEvent)=>void): Action;
	show(): void;
	hide(): void;
}

exports.View = __requireNgui__('_ngui').View;