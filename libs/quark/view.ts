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

import util from './util';
import event, {
	Listen, NativeNotification, Notification, EventNoticer,
	UIEvent, HighlightedEvent, KeyEvent,
	ClickEvent, TouchEvent, MouseEvent, ActionEvent } from './event';
import * as types from './types';
import { StyleSheet, CStyleSheetsClass } from './css';
import { Window } from './window';
import { Action, KeyframeAction,createAction,KeyframeIn,ActionCb } from './action';
import * as action from './action';
import {ViewController} from './ctr';
import {Player,MediaType,MediaSourceStatus,Stream} from './media';

export enum ViewType {
	View,
	Box,
	Flex,
	Flow,
	Free,
	Image,
	Video,
	Input,
	Textarea,
	Label,
	Scroll,
	Text,
	Button,
	Matrix,
	Root,
	Enum_Counts,
}

export interface DOM {
	readonly ref: string;
	readonly metaView: View; // mount point for view controller
	appendTo(parent: View): View;
	afterTo(prev: View): View;
	destroy(owner: ViewController): void; // destroy from owner
}

export declare class View extends Notification<UIEvent> implements DOM {
	private _children: (DOM|null)[]; // View | ViewController | null, JSX system specific
	readonly onClick: EventNoticer<ClickEvent>;
	readonly onBack: EventNoticer<ClickEvent>;
	readonly onKeyDown: EventNoticer<KeyEvent>;
	readonly onKeyPress: EventNoticer<KeyEvent>;
	readonly onKeyUp: EventNoticer<KeyEvent>;
	readonly onKeyEnter: EventNoticer<KeyEvent>;
	readonly onTouchStart: EventNoticer<TouchEvent>;
	readonly onTouchMove: EventNoticer<TouchEvent>;
	readonly onTouchEnd: EventNoticer<TouchEvent>;
	readonly onTouchCancel: EventNoticer<TouchEvent>;
	readonly onMouseOver: EventNoticer<MouseEvent>;
	readonly onMouseOut: EventNoticer<MouseEvent>;
	readonly onMouseLeave: EventNoticer<MouseEvent>;
	readonly onMouseEnter: EventNoticer<MouseEvent>;
	readonly onMouseMove: EventNoticer<MouseEvent>;
	readonly onMouseDown: EventNoticer<MouseEvent>;
	readonly onMouseUp: EventNoticer<MouseEvent>;
	readonly onMouseWheel: EventNoticer<MouseEvent>;
	readonly onFocus: EventNoticer<UIEvent>;
	readonly onBlur: EventNoticer<UIEvent>;
	readonly onHighlighted: EventNoticer<HighlightedEvent>;
	readonly onActionKeyframe: EventNoticer<ActionEvent>;
	readonly onActionLoop: EventNoticer<ActionEvent>;
	readonly cssclass: CStyleSheetsClass;
	readonly parent: View | null;
	readonly prev: View | null;
	readonly next: View | null;
	readonly first: View | null;
	readonly last: View | null;
	readonly window: Window;
	readonly matrix: Matrix | null; // top matrix view
	readonly level: number;
	readonly layoutWeight: number;
	readonly layoutAlign: types.Align;
	readonly isClip: boolean;
	readonly viewType: ViewType;
	readonly position: types.Vec2; // @safe Rt
	readonly layoutOffset: types.Vec2; // @safe Rt
	readonly center: types.Vec2; // @safe Rt
	readonly metaView: View;
	readonly visibleRegion: boolean;
	readonly ref: string;
	style: StyleSheet;
	action: Action | null;
	class: string[]; // settingonly method, cssclass.set()
	opacity: number;
	cursor: types.CursorStyle;
	visible: boolean;
	receive: boolean;
	isFocus: boolean;
	focus(): boolean;
	blur(): boolean;
	show(): void; // visible = true
	hide(): void; // visible = false
	isSelfChild(child: View): boolean;
	before(view: View): void;
	after(view: View): void;
	prepend(view: View): void;
	append(view: View): void;
	remove(): void;
	removeAllChild(): void;
	overlapTest(point: types.Vec2): boolean; // @safe Rt
	hashCode(): number;
	appendTo(parent: View): this;
	afterTo(prev: View): this;
	destroy(owner: ViewController): void;
	transition(to: KeyframeIn, from?: KeyframeIn | ActionCb, cb?: ActionCb): KeyframeAction;
	constructor(win: Window);
	static readonly isViewController: boolean;
}

export declare class Box extends View {
	clip: boolean;
	align: types.Align;
	width: types.BoxSize;
	height: types.BoxSize;
	minWidth: types.BoxSize;
	minHeight: types.BoxSize;
	maxWidth: types.BoxSize;
	maxHeight: types.BoxSize;
	margin: number[];
	marginTop: number;
	marginRight: number;
	marginBottom: number;
	marginLeft: number;
	padding: number[];
	paddingTop: number;
	paddingRight: number;
	paddingBottom: number;
	paddingLeft: number;
	borderRadius: number[];
	borderRadiusLeftTop: number;
	borderRadiusRightTop: number;
	borderRadiusRightBottom: number;
	borderRadiusLeftBottom: number;
	border: types.BoxBorder[];
	borderTop: types.BoxBorder;
	borderRight: types.BoxBorder;
	borderBottom: types.BoxBorder;
	borderLeft: types.BoxBorder;
	borderWidth: number[];
	borderColor: types.Color[];
	borderWidthTop: number;
	borderWidthRight: number;
	borderWidthBottom: number;
	borderWidthLeft: number;
	borderColorTop: types.Color;
	borderColorRight: types.Color;
	borderColorBottom: types.Color;
	borderColorLeft: types.Color;
	backgroundColor: types.Color;
	background: types.BoxFilter | null;
	boxShadow: types.BoxShadow | null;
	weight: number;
	readonly wrapX: boolean; // @safe Rt, Returns the x-axis is wrap content, use internal extrusion size
	readonly wrapY: boolean; // @safe Rt, Returns the y-axis is wrap content, use internal extrusion size
	readonly contentSize: types.Vec2; // @safe Rt, width,height, no include padding
	readonly clientSize: types.Vec2; // @safe Rt, border + padding + content
}

export declare class Flex extends Box {
	direction: types.Direction;
	itemsAlign: types.ItemsAlign;
	crossAlign: types.CrossAlign;
}

export declare class Flow extends Flex {
	wrap: types.Wrap;
	wrapAlign: types.WrapAlign;
}

export declare class Free extends Box {
}

export declare class Image extends Box {
	readonly onLoad: EventNoticer<UIEvent>;
	readonly onError: EventNoticer<UIEvent>;
	src: string;
}

export declare class Matrix extends Box {
	translate: types.Vec2;
	scale: types.Vec2;
	skew: types.Vec2;
	rotateZ: number;
	originX: types.BoxOrigin;
	originY: types.BoxOrigin;
	readonly originValue: types.Vec2;
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	skewX: number;
	skewY: number;
	origin: types.BoxOrigin[];
	readonly mat: types.Mat;
}

export declare class Root extends Matrix {
}

export interface TextOptions {
	readonly fontStyle: number;
	textAlign: types.TextAlign;
	textWeight: types.TextWeight;
	textSlant: types.TextSlant;
	textDecoration: types.TextDecoration;
	textOverflow: types.TextOverflow;
	textWhiteSpace: types.TextWhiteSpace;
	textWordBreak: types.TextWordBreak;
	textSize: types.TextSize;
	textBackgroundColor: types.TextColor;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;
	computeLayoutSize(text: string): types.Vec2;
}

export declare class Text extends Box implements TextOptions {
	readonly fontStyle: number;
	textAlign: types.TextAlign;
	textWeight: types.TextWeight;
	textSlant: types.TextSlant;
	textDecoration: types.TextDecoration;
	textOverflow: types.TextOverflow;
	textWhiteSpace: types.TextWhiteSpace;
	textWordBreak: types.TextWordBreak;
	textSize: types.TextSize;
	textBackgroundColor: types.TextColor;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;
	value: string;
	computeLayoutSize(text: string): types.Vec2;
}

export declare class Button extends Text {
	nextButton(dir: types.FindDirection): Button | null;
}

export declare class Label extends View implements TextOptions {
	readonly fontStyle: number;
	textAlign: types.TextAlign;
	textWeight: types.TextWeight;
	textSlant: types.TextSlant;
	textDecoration: types.TextDecoration;
	textOverflow: types.TextOverflow;
	textWhiteSpace: types.TextWhiteSpace;
	textWordBreak: types.TextWordBreak;
	textSize: types.TextSize;
	textBackgroundColor: types.TextColor;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;
	value: string;
	computeLayoutSize(text: string): types.Vec2;
}

export declare class Input extends Box implements TextOptions {
	readonly onChange: EventNoticer<UIEvent>;
	readonly fontStyle: number;
	textAlign: types.TextAlign;
	textWeight: types.TextWeight;
	textSlant: types.TextSlant;
	textDecoration: types.TextDecoration;
	textOverflow: types.TextOverflow;
	textWhiteSpace: types.TextWhiteSpace;
	textWordBreak: types.TextWordBreak;
	textSize: types.TextSize;
	textBackgroundColor: types.TextColor;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;
	security: boolean; // input
	readonly: boolean;
	type: types.KeyboardType;
	returnType: types.KeyboardReturnType;
	placeholderColor: types.Color;
	cursorColor: types.Color;
	maxLength: number;
	value: string;
	placeholder: string;
	readonly textLength: number;
	computeLayoutSize(text: string): types.Vec2;
}

export interface ScrollBase {
	scrollbar: boolean;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	scrollX: number;
	scrollY: number;
	scroll: types.Vec2;
	resistance: number;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: types.Color;
	scrollbarWidth: number;
	scrollbarMargin: number;
	scrollDuration: number;
	defaultCurve: types.Curve;
	readonly scrollbarH: boolean;
	readonly scrollbarV: boolean;
	readonly scrollSize: types.Vec2;
	scrollTo(val: types.Vec2, duration?: number, curve?: types.Curve): void;
	terminate(): void;
}

export declare class Textarea extends Input implements ScrollBase {
	readonly onScroll: EventNoticer<UIEvent>;
	scrollbar: boolean;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	scrollX: number;
	scrollY: number;
	scroll: types.Vec2;
	resistance: number;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: types.Color;
	scrollbarWidth: number;
	scrollbarMargin: number;
	scrollDuration: number;
	defaultCurve: types.Curve;
	readonly scrollbarH: boolean;
	readonly scrollbarV: boolean;
	readonly scrollSize: types.Vec2;
	scrollTo(val: types.Vec2, duration?: number, curve?: types.Curve): void;
	terminate(): void;
}

export declare class Scroll extends Box implements ScrollBase {
	readonly onScroll: EventNoticer<UIEvent>;
	scrollbar: boolean;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	scrollX: number;
	scrollY: number;
	scroll: types.Vec2;
	resistance: number;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: types.Color;
	scrollbarWidth: number;
	scrollbarMargin: number;
	scrollDuration: number;
	defaultCurve: types.Curve;
	readonly scrollbarH: boolean;
	readonly scrollbarV: boolean;
	readonly scrollSize: types.Vec2;
	scrollTo(val: types.Vec2, duration?: number, curve?: types.Curve): void;
	terminate(): void;
}

export declare class Video extends Image implements Player {
	readonly onStop: EventNoticer<UIEvent>;
	readonly onBuffering: EventNoticer<UIEvent>;
	readonly pts: number;
	volume: number;
	mute: boolean;
	readonly isPause: boolean;
	readonly type: MediaType;
	readonly duration: number;
	readonly status: MediaSourceStatus;
	readonly video: Stream | null;
	readonly audio: Stream | null;
	readonly audioStreams: number;
	play(): void;
	pause(): void;
	stop(): void;
	seek(timeMs: number): void;
	switch_audio(index: number): void;
}

const _ui = __binding__('_ui');

Object.assign(exports, {
	View: _ui.View,
	Box: _ui.Box,
	Flex: _ui.Flex,
	Flow: _ui.Flow,
	Free: _ui.Free,
	Image: _ui.Image,
	Video: _ui.Video,
	Input: _ui.Input,
	Textarea: _ui.Textarea,
	Label: _ui.Label,
	Scroll: _ui.Scroll,
	Text: _ui.Text,
	Button: _ui.Button,
	Matrix: _ui.Matrix,
	Root: _ui.Root,
});

// JSX IntrinsicElements
// -------------------------------------------------------------------------------
declare global {
	namespace JSX {
		interface ViewJSX {
			onClick?: Listen<ClickEvent, View> | null;
			onBack?: Listen<ClickEvent, View> | null;
			onKeyDown?: Listen<KeyEvent, View> | null;
			onKeyPress?: Listen<KeyEvent, View> | null;
			onKeyUp?: Listen<KeyEvent, View> | null;
			onKeyEnter?: Listen<KeyEvent, View> | null;
			onTouchStart?: Listen<TouchEvent, View> | null;
			onTouchMove?: Listen<TouchEvent, View> | null;
			onTouchEnd?: Listen<TouchEvent, View> | null;
			onTouchCancel?: Listen<TouchEvent, View> | null;
			onMouseOver?: Listen<MouseEvent, View> | null;
			onMouseOut?: Listen<MouseEvent, View> | null;
			onMouseLeave?: Listen<MouseEvent, View> | null;
			onMouseEnter?: Listen<MouseEvent, View> | null;
			onMouseMove?: Listen<MouseEvent, View> | null;
			onMouseDown?: Listen<MouseEvent, View> | null;
			onMouseUp?: Listen<MouseEvent, View> | null;
			onMouseWheel?: Listen<MouseEvent, View> | null;
			onFocus?: Listen<UIEvent, View> | null;
			onBlur?: Listen<UIEvent, View> | null;
			onHighlighted?: Listen<HighlightedEvent, View> | null;
			onActionKeyframe?: Listen<ActionEvent, View> | null;
			onActionLoop?: Listen<ActionEvent, View> | null;
			ref?: string;
			key?: string|number;
			style?: StyleSheet;
			action?: action.ActionIn | null;
			class?: string | string[];
			opacity?: number;
			cursor?: types.CursorStyleIn;
			visible?: boolean;
			receive?: boolean;
			isFocus?: boolean;
		}

		interface BoxJSX extends ViewJSX {
			clip?: boolean;
			align?: types.AlignIn;
			width?: types.BoxSizeIn;
			height?: types.BoxSizeIn;
			minWidth?: types.BoxSizeIn;
			minHeight?: types.BoxSizeIn;
			maxWidth?: types.BoxSizeIn;
			maxHeight?: types.BoxSizeIn;
			margin?: number[] | number;
			marginTop?: number;
			marginRight?: number;
			marginBottom?: number;
			marginLeft?: number;
			padding?: number[] | number;
			paddingTop?: number;
			paddingRight?: number;
			paddingBottom?: number;
			paddingLeft?: number;
			borderRadius?: number[] | number;
			borderRadiusLeftTop?: number;
			borderRadiusRightTop?: number;
			borderRadiusRightBottom?: number;
			borderRadiusLeftBottom?: number;
			border?: types.BoxBorderIn[] | types.BoxBorderIn; // border
			borderTop?: types.BoxBorderIn;
			borderRight?: types.BoxBorderIn;
			borderBottom?: types.BoxBorderIn;
			borderLeft?: types.BoxBorderIn;
			borderWidth?: number[] | number;
			borderColor?: types.ColorIn[] | types.ColorIn;
			borderWidthTop?: number; // border width
			borderWidthRight?: number;
			borderWidthBottom?: number;
			borderWidthLeft?: number;
			borderColorTop?: types.ColorIn; // border color
			borderColorRight?: types.ColorIn;
			borderColorBottom?: types.ColorIn;
			borderColorLeft?: types.ColorIn;
			backgroundColor?: types.ColorIn;
			background?: types.BoxFilterIn;
			boxShadow?: types.BoxShadowIn;
			weight?: number;
		}

		interface FlexJSX extends BoxJSX {
			direction?: types.DirectionIn;
			itemsAlign?: types.ItemsAlignIn;
			crossAlign?: types.CrossAlignIn;
		}

		interface FlowJSX extends FlexJSX {
			wrap?: types.WrapIn;
			wrapAlign?: types.WrapAlignIn;
		}

		interface FreeJSX extends BoxJSX {
		}

		interface ImageJSX extends BoxJSX {
			onLoad?: Listen<UIEvent, Image> | null;
			onError?: Listen<UIEvent, Image> | null;
			src?: string;
		}

		interface VideoJSX extends ImageJSX {
			onStop?: Listen<UIEvent, Image> | null;
			onLoading?: Listen<UIEvent, Image> | null;
			volume?: number;
			mute?: boolean;
		}

		interface MatrixJSX extends BoxJSX {
			translate?: types.Vec2In;
			scale?: types.Vec2In;
			skew?: types.Vec2In;
			origin?: types.BoxOriginIn[] | types.BoxOriginIn
			x?: number;
			y?: number;
			scaleX?: number;
			scaleY?: number;
			skewX?: number;
			skewY?: number;
			rotateZ?: number;
			originX?: types.BoxOriginIn;
			originY?: types.BoxOriginIn;
		}

		interface TextOptionsJSX {
			textAlign?: types.TextAlignIn;
			textWeight?: types.TextWeightIn;
			textSlant?: types.TextSlantIn;
			textDecoration?: types.TextDecorationIn;
			textOverflow?: types.TextOverflowIn;
			textWhiteSpace?: types.TextWhiteSpaceIn;
			textWordBreak?: types.TextWordBreakIn;
			textSize?: types.TextSizeIn;
			textBackgroundColor?: types.TextColorIn;
			textColor?: types.TextColorIn;
			textLineHeight?: types.TextSizeIn;
			textShadow?: types.TextShadowIn;
			textFamily?: types.TextFamilyIn;
		}

		interface TextJSX extends BoxJSX, TextOptionsJSX {
			value?: string;
		}

		interface ButtonJSX extends TextJSX {
		}

		interface LabelJSX extends ViewJSX, TextOptionsJSX {
			value?: string;
		}

		interface InputJSX extends BoxJSX, TextOptionsJSX {
			onChange?: Listen<UIEvent, Input> | null;
			security?: boolean;
			readonly?: boolean;
			type?: types.KeyboardTypeIn;
			returnType?: types.KeyboardReturnTypeIn;
			placeholderColor?: types.ColorIn;
			cursorColor?: types.ColorIn;
			maxLength?: number;
			placeholder?: string;
			value?: string;
		}

		interface ScrollBaseJSX {
			scrollbar?: boolean;
			bounce?: boolean;
			bounceLock?: boolean;
			momentum?: boolean;
			lockDirection?: boolean;
			scrollX?: number;
			scrollY?: number;
			scroll?: types.Vec2In;
			resistance?: number;
			catchPositionX?: number;
			catchPositionY?: number;
			scrollbarColor?: types.ColorIn;
			scrollbarWidth?: number;
			scrollbarMargin?: number;
			scrollDuration?: number;
			defaultCurve?: types.CurveIn;
		}

		interface TextareaJSX extends InputJSX, ScrollBaseJSX {
			onScroll?: Listen<UIEvent, Textarea> | null;
		}

		interface ScrollJSX extends BoxJSX, ScrollBaseJSX {
			onScroll?: Listen<UIEvent, Scroll> | null;
		}

		interface IntrinsicElements {
			view: ViewJSX;
			box: BoxJSX;
			flex: FlexJSX;
			flow: FlexJSX;
			free: FreeJSX;
			image: ImageJSX;
			img: ImageJSX;
			matrix: MatrixJSX;
			text: TextJSX;
			button: ButtonJSX;
			label: LabelJSX;
			input: InputJSX;
			textarea: TextareaJSX;
			scroll: ScrollJSX;
			video: VideoJSX;
		}

		type IntrinsicElementsName = keyof IntrinsicElements;
	}
}

// extend view impl
// ----------------------------------------------------------------------------

class _View extends NativeNotification {
	@event readonly onClick: EventNoticer<ClickEvent>;
	@event readonly onBack: EventNoticer<ClickEvent>;
	@event readonly onKeyDown: EventNoticer<KeyEvent>;
	@event readonly onKeyPress: EventNoticer<KeyEvent>;
	@event readonly onKeyUp: EventNoticer<KeyEvent>;
	@event readonly onKeyEnter: EventNoticer<KeyEvent>;
	@event readonly onTouchStart: EventNoticer<TouchEvent>;
	@event readonly onTouchMove: EventNoticer<TouchEvent>;
	@event readonly onTouchEnd: EventNoticer<TouchEvent>;
	@event readonly onTouchCancel: EventNoticer<TouchEvent>;
	@event readonly onMouseOver: EventNoticer<MouseEvent>;
	@event readonly onMouseOut: EventNoticer<MouseEvent>;
	@event readonly onMouseLeave: EventNoticer<MouseEvent>;
	@event readonly onMouseEnter: EventNoticer<MouseEvent>;
	@event readonly onMouseMove: EventNoticer<MouseEvent>;
	@event readonly onMouseDown: EventNoticer<MouseEvent>;
	@event readonly onMouseUp: EventNoticer<MouseEvent>;
	@event readonly onMouseWheel: EventNoticer<MouseEvent>;
	@event readonly onFocus: EventNoticer<UIEvent>;
	@event readonly onBlur: EventNoticer<UIEvent>;
	@event readonly onHighlighted: EventNoticer<HighlightedEvent>;
	@event readonly onActionKeyframe: EventNoticer<ActionEvent>;
	@event readonly onActionLoop: EventNoticer<ActionEvent>;

	private _children: (DOM|undefined)[]; // jsx children dom
	readonly ref: string;
	get metaView() { return this }
	get style() { return this as StyleSheet }
	set style(value) { Object.assign(this, value) }
	get class() { return [] }
	set class(value: string[]) {
		(this as unknown as View).cssclass.set(value);
	}

	get action() { // get action object
		return (this as any).action_ as Action | null;
	}

	set action(value) { // set action
		if (value)
			(this as any).action_ = createAction((this as unknown as View).window, value);
		else
			(this as any).action_ = null;
	}

	show() {
		(this as unknown as View).visible = true;
	}

	hide() {
		(this as unknown as View).visible = false;
	}

	hashCode() {
		return (this as unknown as View).viewType + 18766898;
	}

	appendTo(parent: View) {
		parent.append(this as unknown as View);
		return this;
	}

	afterTo(prev: View) {
		prev.after(this as unknown as View);
		return this;
	}

	destroy(owner: ViewController): void {
		for (let dom of this._children) {
			if (dom)
				dom.destroy(owner);
		}
		let ref = this.ref;
		if (ref) {
			if (owner.refs[ref] === this as unknown as View) {
				delete (owner.refs as Dict<DOM>)[ref];
			}
		}
		(this as unknown as View).remove(); // remove from parent view
	}

	transition(to: KeyframeIn, from?: KeyframeIn | ActionCb, cb?: ActionCb) { // transition animate
		return action.transition(this as unknown as View, to, from, cb);
	}

	toStringStyled(indent?: number) {
		let _rv = [] as string[];
		_rv.push('{');
		let kv = Object.entries(this);
		let _indent = (indent || 0) + 2;
		let push_indent = ()=>_rv.push(new Array(_indent + 1).join(' '));

		for (let i = 0; i < kv.length; i++) {
			let [k,v] = kv[i];
			_rv.push(i ? ',\n': '\n'); push_indent();
			_rv.push(k);
			_rv.push(':'); _rv.push(' ');

			if (typeof v == 'object') {
				if ('toStringStyled' in v) {
					_rv.push(v['toStringStyled'](_indent));
				} else if (Array.isArray(v)) {
					_rv.push('[Array]');
				} else {
					_rv.push('[Object]');
				}
			} else {
				_rv.push(v + '');
			}
		}
		_indent -= 2;
		_rv.push('\n'); push_indent();
		_rv.push('}');

		return _rv.join('');
	}

	toString() {
		return '[object view]';
	}
}

class _Image {
	@event readonly onLoad: EventNoticer<UIEvent>;
	@event readonly onError: EventNoticer<UIEvent>;
}

class _Video {
	@event readonly onStop: EventNoticer<UIEvent>;
	@event readonly onBuffering: EventNoticer<UIEvent>;
}

class _Input {
	@event readonly onChange: EventNoticer<UIEvent>;
}

class _Textarea {
	@event readonly onScroll: EventNoticer<UIEvent>;
}

class _Scroll {
	@event readonly onScroll: EventNoticer<UIEvent>;
}

_ui.View.isViewController = false;
_ui.View.prototype.ref = '';
_ui.View.prototype.owner = null;
_ui.View.prototype._children = [];
util.extendClass(_ui.View, _View);
util.extendClass(_ui.Scroll, _Scroll);
util.extendClass(_ui.Image, _Image);
util.extendClass(_ui.Video, _Video);
util.extendClass(_ui.Input, _Input);
util.extendClass(_ui.Textarea, _Textarea);