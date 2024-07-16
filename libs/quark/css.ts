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

import * as types from './types';

const _css = __binding__('_css');

export type CSSNameExp = `.${string}`;

export interface StyleSheet {
	time?: number; // keyframe time or css transition time
	curve?: types.CurveIn; // keyframe curve or css transition curve
	// Meta attribute
	opacity?: number;
	visible?: boolean;
	receive?: boolean;
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
	direction?: types.DirectionIn;
	itemsAlign?: types.ItemsAlignIn;
	crossAlign?: types.CrossAlignIn;
	wrap?: types.WrapIn;
	wrapAlign?: types.WrapAlignIn;
	src?: string;
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
	security?: boolean;
	readonly?: boolean;
	type?: types.KeyboardTypeIn;
	returnType?: types.KeyboardReturnTypeIn;
	placeholderColor?: types.ColorIn;
	cursorColor?: types.ColorIn;
	maxLength?: number;
	placeholder?: string;
	scrollbarColor?: types.ColorIn;
	scrollbarWidth?: number;
	scrollbarMargin?: number;
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
	originX?: number;
	originY?: number;
}

export declare class CStyleSheetsClass {
	set(name: string[] | string): void;
	add(name: string): void;
	remove(name: string): void;
	toggle(name: string): void;
}

export const createCss = _css.create as ((sheets: { [key: `.${string}`]: StyleSheet })=>void);
export default createCss;