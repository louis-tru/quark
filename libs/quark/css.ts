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

exports.CStyleSheetsClass = _css.CStyleSheetsClass;

/**
 * @type CSSNameExp:'.className'
*/
export type CSSNameExp = `.${string}`;

/**
 * @class StyleSheets
*/
export declare abstract class StyleSheets {
	time?: number; //!< keyframe time or css transition time
	curve?: types.CurveIn; //!< keyframe curve or css transition curve
	// Meta attribute
	color?: types.ColorIn; //!< view color
	cascadeColor?: types.CascadeColorIn; //!< cascade color
	cursor?: types.CursorStyleIn; //!<
	opacity?: Float; //!< opacity 0.0 ~ 1.0, color.a alias, opacity = color.a / 255.0
	visible?: boolean; //!<
	receive?: boolean; //!<
	clip?: boolean; //!<
	align?: types.AlignIn; //!<
	width?: types.BoxSizeIn; //!<
	height?: types.BoxSizeIn; //!<
	minWidth?: types.BoxSizeIn; //!<
	minHeight?: types.BoxSizeIn; //!<
	maxWidth?: types.BoxSizeIn; //!<
	maxHeight?: types.BoxSizeIn; //!<
	margin?: number[] | number; //!<
	marginTop?: number; //!<
	marginRight?: number; //!<
	marginBottom?: number; //!<
	marginLeft?: number; //!<
	padding?: number[] | number; //!<
	paddingTop?: number; //!<
	paddingRight?: number; //!<
	paddingBottom?: number; //!<
	paddingLeft?: number; //!<
	borderRadius?: number[] | number; //!<
	borderRadiusLeftTop?: number; //!<
	borderRadiusRightTop?: number; //!<
	borderRadiusRightBottom?: number; //!<
	borderRadiusLeftBottom?: number; //!<
	border?: types.BoxBorderIn[] | types.BoxBorderIn; //!< border
	borderTop?: types.BoxBorderIn; //!<
	borderRight?: types.BoxBorderIn; //!<
	borderBottom?: types.BoxBorderIn; //!<
	borderLeft?: types.BoxBorderIn; //!<
	borderWidth?: number[] | number; //!<
	borderColor?: types.ColorIn[] | types.ColorIn; //!<
	borderWidthTop?: number; //!< border width
	borderWidthRight?: number; //!<
	borderWidthBottom?: number; //!<
	borderWidthLeft?: number; //!<
	borderColorTop?: types.ColorIn; //!< border color
	borderColorRight?: types.ColorIn; //!<
	borderColorBottom?: types.ColorIn; //!<
	borderColorLeft?: types.ColorIn; //!<
	backgroundColor?: types.ColorIn; //!<
	background?: types.BoxFilterIn; //!<
	boxShadow?: types.BoxShadowIn; //!<
	weight?: number[] | number; //!<
	direction?: types.DirectionIn; //!<
	itemsAlign?: types.ItemsAlignIn; //!<
	crossAlign?: types.CrossAlignIn; //!<
	wrap?: types.WrapIn; //!<
	wrapAlign?: types.WrapAlignIn; //!<
	src?: string; //!<
	textAlign?: types.TextAlignIn; //!<
	textWeight?: types.TextWeightIn; //!<
	textSlant?: types.TextSlantIn; //!<
	textDecoration?: types.TextDecorationIn; //!<
	textOverflow?: types.TextOverflowIn; //!<
	textWhiteSpace?: types.TextWhiteSpaceIn; //!<
	textWordBreak?: types.TextWordBreakIn; //!<
	textSize?: types.TextSizeIn; //!<
	textBackgroundColor?: types.TextColorIn; //!<
	textColor?: types.TextColorIn; //!<
	textLineHeight?: types.TextSizeIn; //!<
	textShadow?: types.TextShadowIn; //!<
	textFamily?: types.TextFamilyIn; //!<
	security?: boolean; //!<
	readonly?: boolean; //!<
	type?: types.KeyboardTypeIn; //!<
	returnType?: types.KeyboardReturnTypeIn; //!<
	placeholderColor?: types.ColorIn; //!<
	cursorColor?: types.ColorIn; //!<
	maxLength?: number; //!<
	placeholder?: string; //!<
	scrollbarColor?: types.ColorIn; //!<
	scrollbarWidth?: number; //!<
	scrollbarMargin?: number; //!<
	translate?: types.Vec2In; //!<
	scale?: types.Vec2In; //!<
	skew?: types.Vec2In; //!<
	origin?: types.BoxOriginIn[] | types.BoxOriginIn //!<
	originX?: types.BoxOriginIn; //!<
	originY?: types.BoxOriginIn; //!<
	x?: number; //!<
	y?: number; //!<
	scaleX?: number; //!<
	scaleY?: number; //!<
	skewX?: number; //!<
	skewY?: number; //!<
	rotateZ?: number; //!<
}

/**
 * A collection of stylesheet names to apply to the view
*/
export declare class CStyleSheetsClass {
	/**
	 * Set Style Collection
	*/
	set(name: string[] | string): void;
	/**
	 * Add a stylesheet class selector name
	*/
	add(name: string): void;
	/**
	 * Delete a stylesheet class selector name
	*/
	remove(name: string): void;
	/**
	 * Toggle between Add and Delete
	*/
	toggle(name: string): void;
}
/** @end */

/**
 *
 * @method createCss(sheets)
 *
 * * `css` stylesheet is similar to html `css` stylesheet, supports multi-level stylesheet, but only supports `class` type
 *
 * * Supports `3` pseudo-types `normal`, `hover`, `action`
 *
 * 	Corresponds to [`HighlightedStatus.Normal`], [`HighlightedStatus.Hover`], [`HighlightedStatus.Active`] in [`View.onHighlighted`] event
 *
 * * When calling this method to create a stylesheet, pause the rendering threads of all windows
 *
 * @param sheets:object
 * 
 * @example
 * 
 *	```ts
 *	createCss({
 *		'.test': {
 *			width: '50%',
 *			height: '50%',
 *			backgroundColor: '#00f',
 *		},
 *		'.test .a': {
 *			width: 50,
 *			height: 50,
 *		},
 *		'.test .a.b': { // This selector will have a higher priority
 *			height: 60,
 *		},
 *		// Apply these pseudo-classes to the target.
 *		// To make them effective on the target, the target view must be able to receive events.
 *		'.test:normal .a': {
 *			backgroundColor: '#0000',
 *			time: 1000, // Use transition time
 *		},
 *		'.test:hover': {
 *			backgroundColor: '#f0f',
 *		},
 *		'.test:hover .a': {
 *			backgroundColor: '#f00',
 *			time: 1000,  // Use transition time
 *		},
 *	})
 *	```
 */
export const createCss = _css.create as ((sheets: { [key: `.${string}`]: StyleSheets })=>void);
export default createCss;
