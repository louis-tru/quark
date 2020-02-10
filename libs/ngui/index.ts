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
import event, {
	NativeNotification, Listen, EventNoticer, 
	HighlightedStatus,
	GUIEvent, GUIHighlightedEvent,
	GUIKeyEvent, GUIClickEvent,
	GUITouchEvent, GUIMouseEvent,
	GUIActionEvent,
} from './event';
import {View} from './_view';
import {ViewController} from './ctr';
import {Action, Options as ActionOpt} from './_action';
import * as action from './action';
import * as value from './value';
import app from  './app';
import display_port from './display_port';
import * as css from './css';
export {View} from './_view';
export {ViewController} from './ctr';
export {GUIApplication} from './app';

const _ngui = __requireNgui__('_ngui');

type Texture = any;

export declare class Sprite extends View {
	src: string;
	texture: Texture | null;
	startX: number;
	startY: number;
	width: number;
	height: number;
	start: number;
	ratioX: number;
	ratioY: number;
	ratio: value.Vec2;
	repeat: value.Repeat;
}

export interface TextFont {
	simpleLayoutWidth(): number;
	textBackgroundColor: value.TextColor;
	textColor: value.TextColor;
	textSize: value.TextSize;
	textStyle: value.TextStyle;
	textFamily: value.TextFamily;
	textShadow: value.TextShadow;
	textLineHeight: value.TextLineHeight;
	textDecoration: value.TextDecoration;
}

export interface TextLayout extends TextFont {
	textOverflow: value.TextOverflow;
	textWhiteSpace: value.TextWhiteSpace;
}

export declare abstract class Layout extends View {
	readonly clientWidth: number;
	readonly clientHeight: number;
}

export declare abstract class Box extends Layout {
	width: value.Value;
	height: value.Value;
	margin: value.Value[]; //
	marginLeft: value.Value;
	marginTop: value.Value;
	marginRight: value.Value;
	marginBottom: value.Value;
	border: value.Border[]; //
	borderLeft: value.Border;
	borderTop: value.Border;
	borderRight: value.Border;
	borderBottom: value.Border;
	borderWidth: number[]; //
	borderLeftWidth: number;
	borderTopWidth: number;
	borderRightWidth: number;
	borderBottomWidth: number;
	borderColor: value.Color[];
	borderLeftColor: value.Color;
	borderTopColor: value.Color;
	borderRightColor: value.Color;
	borderBottomColor: value.Color;
	borderRadius: number[]; //
	borderRadiusLeftTop: number;
	borderRadiusRightTop: number;
	borderRadiusRightBottom: number;
	borderRadiusLeftBottom: number;
	backgroundColor: value.Color;
	background: value.Background | null;
	newline: boolean;
	clip: boolean;
	readonly finalWidth: number;
	readonly finalHeight: number;
	readonly finalMarginLeft: number;
	readonly finalMarginTop: number;
	readonly finalMarginRight: number;
	readonly finalMarginBottom: number;
}

export declare class Span extends Layout implements TextLayout {
	simpleLayoutWidth(): number;
	textBackgroundColor: value.TextColor;
	textColor: value.TextColor;
	textSize: value.TextSize;
	textStyle: value.TextStyle;
	textFamily: value.TextFamily;
	textShadow: value.TextShadow;
	textLineHeight: value.TextLineHeight;
	textDecoration: value.TextDecoration;
	textOverflow: value.TextOverflow;
	textWhiteSpace: value.TextWhiteSpace;
}

export declare class TextNode extends Span {
	readonly length: number;
	value: string;
	readonly textHoriBearing: number;
	readonly textHeight: number;
}

export declare class Div extends Box {
	contentAlign: value.ContentAlign;
}

export declare class Image extends Div {
	readonly onLoad: EventNoticer<GUIEvent>;
	readonly onError: EventNoticer<GUIEvent>;
	src: string;
	readonly sourceWidth: number;
	readonly sourceHeight: number;
}

export declare class Panel extends Div {
	readonly onFocusMove: EventNoticer<GUIEvent>;
	allowLeave: boolean;
	allowEntry: boolean;
	intervalTime: number;
	enableSelect: boolean;
	readonly isActivity: boolean;
	readonly parentPanel: Panel;
}

export interface IScroll {
	readonly onScroll: EventNoticer<GUIEvent>;
	scrollTo(value: value.Vec2, duration?: number, curve?: value.Curve): void;
	terminate(): void;
	scroll: value.Vec2;
	scrollX: number;
	scrollY: number;
	readonly scrollWidth: number;
	readonly scrollHeight: number;
	scrollbar: boolean;
	resistance: number;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: value.Color;
	readonly hScrollbar: boolean;
	readonly vScrollbar: boolean;
	scrollbarWidth: number;
	scrollbarMargin: number;
	defaultScrollDuration: number;
	defaultScrollCurve: value.Curve;
}

export declare class Scroll extends Panel implements IScroll {
	focusMarginLeft: number;
	focusMarginRight: number;
	focusMarginTop: number;
	focusMarginBottom: number;
	focusAlignX: value.Align;
	focusAlignY: value.Align;
	enableFocusAlign: boolean;
	readonly isFixedScrollSize: boolean;
	setFixedScrollSize(size: value.Vec2): void;
	// implements IScroll
	readonly onScroll: EventNoticer<GUIEvent>;
	scrollTo(value: value.Vec2, duration?: number, curve?: value.Curve): void;
	terminate(): void;
	scroll: value.Vec2;
	scrollX: number;
	scrollY: number;
	readonly scrollWidth: number;
	readonly scrollHeight: number;
	scrollbar: boolean;
	resistance: number;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: value.Color;
	readonly hScrollbar: boolean;
	readonly vScrollbar: boolean;
	scrollbarWidth: number;
	scrollbarMargin: number;
	defaultScrollDuration: number;
	defaultScrollCurve: value.Curve;
}

export declare class Indep extends Div {
	alignX: value.Align;
	alignY: value.Align;
	align: value.Vec2[];
}

export interface ILimit {
	minWidth: value.Value;
	minHeight: value.Value;
	maxWidth: value.Value;
	maxHeight: value.Value;
}

export declare class Limit extends Div implements ILimit {
	// implements ILimit
	minWidth: value.Value;
	minHeight: value.Value;
	maxWidth: value.Value;
	maxHeight: value.Value;
}

export declare class LimitIndep extends Indep implements ILimit {
	// implements ILimit
	minWidth: value.Value;
	minHeight: value.Value;
	maxWidth: value.Value;
	maxHeight: value.Value;
}

export declare class Hybrid extends Box implements TextLayout {
	textAlign: value.TextAlign;
	// implements TextLayout
	simpleLayoutWidth(): number;
	textBackgroundColor: value.TextColor;
	textColor: value.TextColor;
	textSize: value.TextSize;
	textStyle: value.TextStyle;
	textFamily: value.TextFamily;
	textShadow: value.TextShadow;
	textLineHeight: value.TextLineHeight;
	textDecoration: value.TextDecoration;
	textOverflow: value.TextOverflow;
	textWhiteSpace: value.TextWhiteSpace;
}

export declare class Label extends View implements TextFont {
	readonly length: number;
	value: string;
	readonly textHoriBearing: number;
	readonly textHeight: number;
	textAlign: value.TextAlign;
	// implements TextFont
	simpleLayoutWidth(): number;
	textBackgroundColor: value.TextColor;
	textColor: value.TextColor;
	textSize: value.TextSize;
	textStyle: value.TextStyle;
	textFamily: value.TextFamily;
	textShadow: value.TextShadow;
	textLineHeight: value.TextLineHeight;
	textDecoration: value.TextDecoration;
}

export declare class Text extends Hybrid {
	readonly length: number;
	value: string;
	readonly textHoriBearing: number;
	readonly textHeight: number;
}

export declare class Input extends Text {
	readonly onChange: EventNoticer<GUIEvent>;
	type: value.KeyboardType;
	returnType: value.KeyboardReturnType;
	placeholder: string;
	placeholderColor: value.Color;
	security: boolean;
	textMargin: number;
}

export declare class Textarea extends Input implements IScroll {
	// implements IScroll
	readonly onScroll: EventNoticer<GUIEvent>;
	scrollTo(value: value.Vec2, duration?: number, curve?: value.Curve): void;
	terminate(): void;
	scroll: value.Vec2;
	scrollX: number;
	scrollY: number;
	readonly scrollWidth: number;
	readonly scrollHeight: number;
	scrollbar: boolean;
	resistance: number;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: value.Color;
	readonly hScrollbar: boolean;
	readonly vScrollbar: boolean;
	scrollbarWidth: number;
	scrollbarMargin: number;
	defaultScrollDuration: number;
	defaultScrollCurve: value.Curve;
}

export declare class Button extends Hybrid {
	findNextButton(): Button | null;
	panel(): Panel | null;
	defaultStyle: boolean;
	setHighlighted(status: HighlightedStatus): void;
	triggerHighlighted(evt: GUIHighlightedEvent): number;
}

export declare class Root extends Panel {}

export class Clip extends (_ngui.Div as typeof Div) {
	constructor() {
		super();
		this.clip = true;
	}
}

class _View extends NativeNotification {

	private m_id: string;
	private m_owner: ViewController | null;

	get __view__() { return this }

	@event readonly onKeyDown: EventNoticer<GUIKeyEvent>;
	@event readonly onKeyPress: EventNoticer<GUIKeyEvent>;
	@event readonly onKeyUp: EventNoticer<GUIKeyEvent>;
	@event readonly onKeyEnter: EventNoticer<GUIKeyEvent>;
	@event readonly onBack: EventNoticer<GUIClickEvent>;
	@event readonly onClick: EventNoticer<GUIClickEvent>;
	@event readonly onTouchStart: EventNoticer<GUITouchEvent>;
	@event readonly onTouchMove: EventNoticer<GUITouchEvent>;
	@event readonly onTouchEnd: EventNoticer<GUITouchEvent>;
	@event readonly onTouchCancel: EventNoticer<GUITouchEvent>;
	@event readonly onMouseOver: EventNoticer<GUIMouseEvent>;
	@event readonly onMouseOut: EventNoticer<GUIMouseEvent>;
	@event readonly onMouseLeave: EventNoticer<GUIMouseEvent>;
	@event readonly onMouseEnter: EventNoticer<GUIMouseEvent>;
	@event readonly onMouseMove: EventNoticer<GUIMouseEvent>;
	@event readonly onMouseDown: EventNoticer<GUIMouseEvent>;
	@event readonly onMouseUp: EventNoticer<GUIMouseEvent>;
	@event readonly onMouseWheel: EventNoticer<GUIMouseEvent>;
	@event readonly onFocus: EventNoticer<GUIEvent>;
	@event readonly onBlur: EventNoticer<GUIEvent>;
	@event readonly onHighlighted: EventNoticer<GUIHighlightedEvent>;
	@event readonly onActionKeyframe: EventNoticer<GUIActionEvent>;
	@event readonly onActionLoop: EventNoticer<GUIActionEvent>;

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
	addDefaultListener(name: string, listen: Listen<GUIEvent> | string) {
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

	setAction(opt: ActionOpt | null) {
		(this as any)._setAction(opt && action.create(opt));
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
		(this as unknown as View).setAction(value);
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
	transition(style: Dict, delay?: number, cb?: (e: GUIActionEvent)=>void) { // transition animate
		return action.transition(this as unknown as View, style, delay, cb);
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

class _Panel {
	@event readonly onFocusMove: EventNoticer<GUIEvent>;
}

class _Scroll {
	@event readonly onScroll: EventNoticer<GUIEvent>;
}

class _Image {
	@event readonly onLoad: EventNoticer<GUIEvent>;
	@event readonly onError: EventNoticer<GUIEvent>;
}

class _Input {
	@event readonly onChange: EventNoticer<GUIEvent>;
}

class _Textarea {
	@event readonly onScroll: EventNoticer<GUIEvent>;
}

class _Button {

	private m_defaultStyle: boolean; // = true;

	getNoticer(name: string) {
		var noticer = (this as any)['m_on' + name] as EventNoticer<GUIEvent>;
		if ( ! noticer ) {
			if ( name == 'Click' ) {
				View.prototype.getNoticer.call(this as unknown as View, 'Highlighted'); // bind highlighted
			}
			return View.prototype.getNoticer.call(this as unknown as View, name);
		}
		return noticer;
	}

	get defaultStyle() {
		return this.m_defaultStyle; 
	}

	set defaultStyle(value) {
		this.m_defaultStyle = !!value; 
	}

	setHighlighted(status: HighlightedStatus) {
		if ( this.m_defaultStyle ) {
			if ( status == HighlightedStatus.HIGHLIGHTED_HOVER ) {
				(this as unknown as View).transition({ opacity: 0.7, time: 80 });
			} else if ( status == HighlightedStatus.HIGHLIGHTED_DOWN ) {
				(this as unknown as View).transition({ opacity: 0.35, time: 50 });
			} else {
				(this as unknown as View).transition({ opacity: 1, time: 180 });
			}
		}
	}

	triggerHighlighted(evt: GUIHighlightedEvent) {
		this.setHighlighted(evt.status);
		return (this as unknown as View).triggerWithEvent('Highlighted', evt);
	}
}

utils.extendClass(_ngui.View, _View);
utils.extendClass(_ngui.Panel, _Panel);
utils.extendClass(_ngui.Scroll, _Scroll);
utils.extendClass(_ngui.Image, _Image);
utils.extendClass(_ngui.Input, _Input);
utils.extendClass(_ngui.Textarea, _Textarea);
utils.extendClass(_ngui.Button, _Button);

_ngui.View.prototype.m_id = '';
_ngui.View.prototype.m_owner = null;
_ngui.Button.prototype.m_defaultStyle = true;

Object.assign(exports, {
	Sprite: _ngui.Sprite,
	Label: _ngui.Label,
	Span: _ngui.Span,
	TextNode: _ngui.TextNode,
	Div: _ngui.Div,
	Image: _ngui.Image,
	Textarea: _ngui.Textarea,
	Panel: _ngui.Panel,
	Scroll: _ngui.Scroll,
	Indep: _ngui.Indep,
	Limit: _ngui.Limit,
	LimitIndep: _ngui.LimitIndep,
	Hybrid: _ngui.Hybrid,
	Text: _ngui.Text,
	Input: _ngui.Input,
	Button: _ngui.Button,
	Root: _ngui.Root,
});

export default {

	/**
	 * @func nextFrame(cb)
	 * @arg cb {Function}
	 */
	nextFrame: display_port.nextFrame,

	/**
	 * @func render(vdom[,parent])
	 * @arg vdom {VDOM}
	 * @arg [parent] {View}
	 * @ret {DOM}
	 */
	render: ViewController.render,

	/**
	 * @func CSS(sheets)
	 * @arg sheets {Object}
	 */
	CSS: css.CSS,

	/**
	 * @get app {GUIApplication} get current application object
	 */
	get app() { return app.current },

	/**
	 * @get root {Root} get current root view
	 */
	get root() { return app.root },

	/**
	 * @get rootCtr {ViewController} get current root view controller
	 */
	get rootCtr() { return app.rootCtr },

	/**
	 * @get displayPort {DisplayPort} get current display port
	 */
	get displayPort() { return app.current.displayPort },

	/**
	 * @get atomPixel {float}
	 */
	get atomPixel() { return display_port.atomPixel },

	/**
	 * @get statusBarHeight {float}
	 */
	get statusBarHeight() { return display_port.statusBarHeight },

};
