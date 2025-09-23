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
import { StyleSheets, CStyleSheetsClass } from './css';
import { Window } from './window';
import { Action, KeyframeAction,createAction,KeyframeIn,ActionCb } from './action';
import * as action from './action';
import {ViewController} from './ctr';
import {Player,MediaType,MediaSourceStatus,Stream} from './media';

/**
 * @enum ViewType
*/
export enum ViewType {
	View, //!<
	Sprite, //!<
	Spine, //!<
	Label, //!<
	Box, //!<
	Flex, //!<
	Flow, //!<
	Free, //!<
	Image, //!<
	Video, //!<
	Input, //!<
	Textarea, //!<
	Scroll, //!<
	Text, //!<
	Button, //!<
	Matrix, //!<
	Root, //!<
	Enum_Counts, //!<
}

/**
 * @interface DOM
*/
export interface DOM {
	/** @get ref:string */
	readonly ref: string;
	/** @get metaView:View mount point for view controller */
	readonly metaView: View;
	/** @method appendTo(parent:View)View */
	appendTo(parent: View): View;
	/** @method afterTo(prev:View)View */
	afterTo(prev: View): View;
	/** @method destroy(owner:ViewController) destroy from owner */
	destroy(owner: ViewController): void;
}

type ChildDOM = DOM|null; //!<

/**
 * @class View
 * @extends Notification<UIEvent>
 * @implements DOM
*/
export declare class View extends Notification<UIEvent> implements DOM {
	/** JSX system specific */
	readonly childDoms: ChildDOM[];
	/** @event */
	readonly onClick: EventNoticer<ClickEvent>;
	/** @event */
	readonly onBack: EventNoticer<ClickEvent>;
	/** @event */
	readonly onKeyDown: EventNoticer<KeyEvent>;
	/** @event */
	readonly onKeyPress: EventNoticer<KeyEvent>;
	/** @event */
	readonly onKeyUp: EventNoticer<KeyEvent>;
	/** @event */
	readonly onKeyEnter: EventNoticer<KeyEvent>;
	/** @event */
	readonly onTouchStart: EventNoticer<TouchEvent>;
	/** @event */
	readonly onTouchMove: EventNoticer<TouchEvent>;
	/** @event */
	readonly onTouchEnd: EventNoticer<TouchEvent>;
	/** @event */
	readonly onTouchCancel: EventNoticer<TouchEvent>;
	/** @event */
	readonly onMouseOver: EventNoticer<MouseEvent>;
	/** @event */
	readonly onMouseOut: EventNoticer<MouseEvent>;
	/** @event */
	readonly onMouseLeave: EventNoticer<MouseEvent>;
	/** @event */
	readonly onMouseEnter: EventNoticer<MouseEvent>;
	/** @event */
	readonly onMouseMove: EventNoticer<MouseEvent>;
	/** @event */
	readonly onMouseDown: EventNoticer<MouseEvent>;
	/** @event */
	readonly onMouseUp: EventNoticer<MouseEvent>;
	/** @event */
	readonly onMouseWheel: EventNoticer<MouseEvent>;
	/** @event */
	readonly onFocus: EventNoticer<UIEvent>;
	/** @event */
	readonly onBlur: EventNoticer<UIEvent>;
	/** @event */
	readonly onHighlighted: EventNoticer<HighlightedEvent>;
	/** @event */
	readonly onActionKeyframe: EventNoticer<ActionEvent>;
	/** @event */
	readonly onActionLoop: EventNoticer<ActionEvent>;
	readonly cssclass: CStyleSheetsClass; //!<
	readonly parent: View | null; //!<
	readonly prev: View | null; //!<
	readonly next: View | null; //!<
	readonly first: View | null; //!<
	readonly last: View | null; //!<
	readonly window: Window; //!<
	readonly matrixView: MatrixView | null; //!< top matrix view
	readonly level: number; //!<
	readonly layoutWeight: types.Vec2; //!<
	readonly layoutAlign: types.Align; //!<
	readonly isClip: boolean; //!<
	readonly viewType: ViewType; //!<
	readonly position: types.Vec2; //!< @safe Rt
	readonly layoutOffset: types.Vec2; //!< @safe Rt
	readonly layoutSize: types.Vec2; //!< @safe Rt, For: Box = border + padding + content + margin
	readonly clientSize: types.Vec2; //!< @safe Rt, For: Box = border + padding + content
	readonly metaView: View; //!<
	readonly visibleRegion: boolean; //!<
	readonly ref: string; //!<
	style: StyleSheets; //!<
	action: Action | null; //!<
	class: string[]; //!< settingonly method, cssclass.set()
	color: types.Color; //!<
	cascadeColor: types.CascadeColor; //!<
	cursor: types.CursorStyle; //!<
	opacity: Float; //!< opacity 0.0 ~ 1.0, color.a alias, opacity = color.a / 255.0
	visible: boolean; //!<
	receive: boolean; //!<
	aa: boolean; //!< anti alias
	isFocus: boolean; //!<
	focus(): boolean; //!<
	blur(): boolean;
	show(): void; //!< Set visible = true
	hide(): void; //!< Set visible = false
	isSelfChild(child: View): boolean; //!<
	before(view: View): void; //!<
	after(view: View): void; //!<
	prepend(view: View): void; //!<
	append(view: View): void; //!<
	remove(): void; //!<
	removeAllChild(): void; //!<
	overlapTest(point: types.Vec2): boolean; //!<
	hashCode(): Int; //!<
	appendTo(parent: View): this; //!<
	afterTo(prev: View): this; //!<
	destroy(owner: ViewController): void; //!<
	transition(to: KeyframeIn, from?: KeyframeIn | ActionCb, cb?: ActionCb): KeyframeAction; //!<
	constructor(win: Window); //!<
	static readonly isViewController: boolean;
}

/**
 * @class Box
 * @extends View
*/
export declare class Box extends View {
	clip: boolean; //!<
	align: types.Align; //!<
	width: types.BoxSize; //!<
	height: types.BoxSize; //!<
	minWidth: types.BoxSize; //!<
	minHeight: types.BoxSize; //!<
	maxWidth: types.BoxSize; //!<
	maxHeight: types.BoxSize; //!<
	margin: number[]; //!<
	marginTop: number; //!<
	marginRight: number; //!<
	marginBottom: number; //!<
	marginLeft: number; //!<
	padding: number[]; //!<
	paddingTop: number; //!<
	paddingRight: number; //!<
	paddingBottom: number; //!<
	paddingLeft: number; //!<
	borderRadius: number[]; //!<
	borderRadiusLeftTop: number; //!<
	borderRadiusRightTop: number; //!<
	borderRadiusRightBottom: number; //!<
	borderRadiusLeftBottom: number; //!<
	border: types.Border[]; //!<
	borderTop: types.Border; //!<
	borderRight: types.Border; //!<
	borderBottom: types.Border; //!<
	borderLeft: types.Border; //!<
	borderWidth: number[]; //!<
	borderColor: types.Color[]; //!<
	borderWidthTop: number; //!<
	borderWidthRight: number; //!<
	borderWidthBottom: number; //!<
	borderWidthLeft: number; //!<
	borderColorTop: types.Color; //!<
	borderColorRight: types.Color; //!<
	borderColorBottom: types.Color; //!<
	borderColorLeft: types.Color; //!<
	backgroundColor: types.Color; //!<
	background: types.BoxFilter | null; //!<
	boxShadow: types.BoxShadow | null; //!<
	weight: types.Vec2; //!<
	readonly contentSize: types.Vec2; //!< width,height, no include padding
}

/**
 * @class Flex
 * @extends Box
*/
export declare class Flex extends Box {
	direction: types.Direction; //!<
	itemsAlign: types.ItemsAlign; //!<
	crossAlign: types.CrossAlign; //!<
}

/**
 * @class Flow
 * @extends Flex
*/
export declare class Flow extends Flex {
	wrap: types.Wrap; //!<
	wrapAlign: types.WrapAlign; //!<
}

/**
 * @class Free
 * @extends Box
*/
export declare class Free extends Box {
}

/**
 * @class Image
 * @extends Box
*/
export declare class Image extends Box {
	/** @event */
	readonly onLoad: EventNoticer<UIEvent>;
	/** @event */
	readonly onError: EventNoticer<UIEvent>;
	src: string; //!<
}

/**
 * @interface MatrixView
 * @extends View
*/
export interface MatrixView extends View {
	translate: types.Vec2; //!<
	scale: types.Vec2; //!<
	skew: types.Vec2; //!<
	origin: types.BoxOrigin[]; //!<
	originX: types.BoxOrigin; //!<
	originY: types.BoxOrigin; //!<
	x: number; //!<
	y: number; //!<
	scaleX: number; //!<
	scaleY: number; //!<
	skewX: number; //!<
	skewY: number; //!<
	rotateZ: number; //!<
	readonly originValue: number[]; //!<
	readonly matrix: types.Mat; //!<
}

/**
 * @class Matrix
 * @extends Box
 * @implements MatrixView
*/
export declare class Matrix extends Box implements MatrixView {
	translate: types.Vec2;
	scale: types.Vec2;
	skew: types.Vec2;
	origin: types.BoxOrigin[];
	originX: types.BoxOrigin;
	originY: types.BoxOrigin;
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	skewX: number;
	skewY: number;
	rotateZ: number;
	readonly originValue: number[];
	readonly matrix: types.Mat;
}

/**
 * @class Sprite
 * @extends View
 * @implements MatrixView
*/
export declare class Sprite extends View implements MatrixView {
	translate: types.Vec2;
	scale: types.Vec2;
	skew: types.Vec2;
	origin: types.BoxOrigin[];
	originX: types.BoxOrigin;
	originY: types.BoxOrigin;
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	skewX: number;
	skewY: number;
	rotateZ: number;
	readonly originValue: number[];
	readonly matrix: types.Mat;
	/** @event */
	readonly onLoad: EventNoticer<UIEvent>;
	/** @event */
	readonly onError: EventNoticer<UIEvent>;
	src: string; //!<
	width: number; //!<
	height: number; //!<
	frame: Uint16; //!<
	frames: Uint16; //!<
	item: Uint16; //!<
	items: Uint16; //!<
	gap: Uint8; //!<
	fsp: Uint8; //!<
	direction: types.Direction; //!<
	playing: boolean; //!<
	play(): void; //!<
	stop(): void; //!<
}

export declare class Spine extends View implements MatrixView {
	translate: types.Vec2;
	scale: types.Vec2;
	skew: types.Vec2;
	origin: types.BoxOrigin[];
	originX: types.BoxOrigin;
	originY: types.BoxOrigin;
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	skewX: number;
	skewY: number;
	rotateZ: number;
	readonly originValue: number[];
	readonly matrix: types.Mat;
	skeleton: types.SkeletonData | null; //!<
	skin: string; //!<
	speed: Float; //!<
	defaultMix: Float; //!<
	setToSetupPose(): void; //!<
	setBonesToSetupPose(): void; //!<
	setSlotsToSetupPose(): void; //!<
	setAttachment(slotName: string, attachmentName: string): void; //!<
	setMix(fromName: string, toName: string, duration: Float): void; //!<
	setAnimation(trackIndex: Uint, name: string, loop: boolean): void; //!<
	addAnimation(trackIndex: Uint, name: string, loop: boolean, delay?: Float): void; //!<
	setEmptyAnimation(trackIndex: Uint, mixDuration: Float): void; //!<
	setEmptyAnimations(mixDuration: Float): void; //!<
	addEmptyAnimation(trackIndex: Uint, mixDuration: Float, delay?: Float): void; //!<
	clearTracks(): void; //!<
	clearTrack(trackIndex?: Uint): void; //!<
}

/**
 * @class Root
 * @extends Matrix
*/
export declare class Root extends Matrix {
}

/**
 * @interface TextOptions
*/
export interface TextOptions {
	readonly fontStyle: number; //!<
	textAlign: types.TextAlign; //!<
	textWeight: types.TextWeight; //!<
	textSlant: types.TextSlant; //!<
	textDecoration: types.TextDecoration; //!<
	textOverflow: types.TextOverflow; //!<
	textWhiteSpace: types.TextWhiteSpace; //!<
	textWordBreak: types.TextWordBreak; //!<
	textSize: types.TextSize; //!<
	textBackgroundColor: types.TextColor; //!<
	textStroke: types.TextStroke; //!<
	textColor: types.TextColor; //!<
	textLineHeight: types.TextSize; //!<
	textShadow: types.TextShadow; //!<
	textFamily: types.TextFamily; //!<
	computeLayoutSize(text: string): types.Vec2; //!<
}

/**
 * @class Text
 * @extends Box
 * @implements TextOptions
*/
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
	textStroke: types.TextStroke;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;
	value: string; //!<
	computeLayoutSize(text: string): types.Vec2;
}

/**
 * @class Button
 * @extends Text
*/
export declare class Button extends Text {
	nextButton(dir: types.FindDirection): Button | null; //!<
}

/**
 * @class Label
 * @extends View
 * @implements TextOptions
*/
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
	textStroke: types.TextStroke;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;
	value: string; //!<
	computeLayoutSize(text: string): types.Vec2;
}

/**
 * @class Label
 * @extends Box
 * @implements TextOptions
*/
export declare class Input extends Box implements TextOptions {
	/** @event */
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
	textStroke: types.TextStroke;
	security: boolean; //!< input
	readonly: boolean; //!<
	type: types.KeyboardType; //!<
	returnType: types.KeyboardReturnType; //!<
	placeholderColor: types.Color; //!<
	cursorColor: types.Color; //!<
	maxLength: number; //!<
	value: string; //!<
	placeholder: string; //!<
	readonly textLength: number; //!<
	computeLayoutSize(text: string): types.Vec2;
}

/**
 * @interface ScrollView
 * @extends Box
*/
export interface ScrollView extends Box {
	scrollbar: boolean; //!<
	bounce: boolean; //!<
	bounceLock: boolean; //!<
	momentum: boolean; //!<
	lockDirection: boolean; //!<
	scrollX: number; //!<
	scrollY: number; //!<
	scroll: types.Vec2; //!<
	resistance: number; //!<
	catchPositionX: number; //!<
	catchPositionY: number; //!<
	scrollbarColor: types.Color; //!<
	scrollbarWidth: number; //!<
	scrollbarMargin: number; //!<
	scrollDuration: number; //!<
	defaultCurve: types.Curve; //!<
	readonly scrollbarH: boolean; //!<
	readonly scrollbarV: boolean; //!<
	readonly scrollSize: types.Vec2; //!<
	scrollTo(val: types.Vec2, duration?: number, curve?: types.Curve): void; //!<
	terminate(): void; //!<
}

/**
 * @class Textarea
 * @extends Input
 * @implements ScrollView
*/
export declare class Textarea extends Input implements ScrollView {
	/** @event */
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

/**
 * @class Scroll
 * @extends Box
 * @implements ScrollView
*/
export declare class Scroll extends Box implements ScrollView {
	/** @event */
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

/**
 * @class Video
 * @extends Image
 * @implements Player
*/
export declare class Video extends Image implements Player {
	/** @event */
	readonly onStop: EventNoticer<UIEvent>;
	/** @event */
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
	switchAudio(index: number): void;
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
	Sprite: _ui.Sprite,
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
			style?: StyleSheets;
			action?: action.ActionIn | null;
			class?: string | string[];
			color?: types.ColorIn;
			cascadeColor?: types.CascadeColorIn;
			cursor?: types.CursorStyleIn;
			opacity?: Float;
			visible?: boolean;
			receive?: boolean;
			aa?: boolean;
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
			border?: types.BorderIn[] | types.BorderIn; // border
			borderTop?: types.BorderIn;
			borderRight?: types.BorderIn;
			borderBottom?: types.BorderIn;
			borderLeft?: types.BorderIn;
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
			weight?: types.Vec2In;
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
			volume?: Float;
			mute?: boolean;
		}

		interface MatrixViewJSX {
			translate?: types.Vec2In;
			scale?: types.Vec2In;
			skew?: types.Vec2In;
			origin?: types.BoxOriginIn[] | types.BoxOriginIn
			originX?: types.BoxOriginIn;
			originY?: types.BoxOriginIn;
			x?: Float;
			y?: Float;
			scaleX?: Float;
			scaleY?: Float;
			skewX?: Float;
			skewY?: Float;
			rotateZ?: Float;
		}

		interface MatrixJSX extends BoxJSX, MatrixViewJSX {
		}

		interface SpriteJSX extends ViewJSX, MatrixViewJSX {
			onLoad?: Listen<UIEvent, Sprite> | null;
			onError?: Listen<UIEvent, Sprite> | null;
			src?: string;
			width?: Float;
			height?: Float;
			frame?: Uint16;
			frames?: Uint16;
			item?: Uint16;
			items?: Uint16;
			gap?: Uint8;
			fsp?: Uint8;
			direction?: types.DirectionIn;
			playing?: boolean;
		}

		interface SpineJSX extends ViewJSX, MatrixViewJSX {
			skeleton?: types.SkeletonDataIn;
			skin?: string;
			speed?: Float;
			defaultMix?: Float;
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
			textStroke?: types.TextStrokeIn;
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

		interface ScrollViewJSX {
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

		interface TextareaJSX extends InputJSX, ScrollViewJSX {
			onScroll?: Listen<UIEvent, Textarea> | null;
		}

		interface ScrollJSX extends BoxJSX, ScrollViewJSX {
			onScroll?: Listen<UIEvent, Scroll> | null;
		}

		interface IntrinsicElements {
			view: ViewJSX;
			box: BoxJSX;
			flex: FlexJSX;
			flow: FlowJSX;
			free: FreeJSX;
			image: ImageJSX;
			img: ImageJSX;
			matrix: MatrixJSX;
			sprite: SpriteJSX;
			spine: SpineJSX;
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

	readonly childDoms: (DOM|undefined)[]; // jsx children dom
	readonly ref: string;
	get metaView() { return this }
	get style() { return this as StyleSheets }
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

	set key(val: any) { /* ignore */ }

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
		for (let dom of this.childDoms) {
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

class _Sprite {
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
_ui.View.prototype.childDoms = [];
util.extendClass(_ui.View, _View);
util.extendClass(_ui.Scroll, _Scroll);
util.extendClass(_ui.Image, _Image);
util.extendClass(_ui.Sprite, _Sprite);
util.extendClass(_ui.Video, _Video);
util.extendClass(_ui.Input, _Input);
util.extendClass(_ui.Textarea, _Textarea);