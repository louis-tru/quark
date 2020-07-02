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
import * as value from './value';

const _css = __require__('_css');
const debug = utils.debug;

exports.create = _css.create;

export enum Propery {
	// Meta attribute
	PROPERTY_X,
	PROPERTY_Y,
	PROPERTY_SCALE_X,
	PROPERTY_SCALE_Y,
	PROPERTY_SKEW_X,
	PROPERTY_SKEW_Y,
	PROPERTY_ROTATE_Z,
	PROPERTY_ORIGIN_X,
	PROPERTY_ORIGIN_Y,
	PROPERTY_OPACITY,
	PROPERTY_VISIBLE,
	PROPERTY_WIDTH,
	PROPERTY_HEIGHT,
	PROPERTY_MARGIN_TOP,
	PROPERTY_MARGIN_RIGHT,
	PROPERTY_MARGIN_BOTTOM,
	PROPERTY_MARGIN_LEFT,
	PROPERTY_BORDER_TOP_WIDTH,
	PROPERTY_BORDER_RIGHT_WIDTH,
	PROPERTY_BORDER_BOTTOM_WIDTH,
	PROPERTY_BORDER_LEFT_WIDTH,
	PROPERTY_BORDER_TOP_COLOR,
	PROPERTY_BORDER_RIGHT_COLOR,
	PROPERTY_BORDER_BOTTOM_COLOR,
	PROPERTY_BORDER_LEFT_COLOR,
	PROPERTY_BORDER_RADIUS_LEFT_TOP,
	PROPERTY_BORDER_RADIUS_RIGHT_TOP,
	PROPERTY_BORDER_RADIUS_RIGHT_BOTTOM,
	PROPERTY_BORDER_RADIUS_LEFT_BOTTOM,
	PROPERTY_BACKGROUND_COLOR,
	PROPERTY_BACKGROUND,
	PROPERTY_NEWLINE,
	PROPERTY_CLIP,
	PROPERTY_CONTENT_ALIGN,
	PROPERTY_TEXT_ALIGN,
	PROPERTY_MAX_WIDTH,
	PROPERTY_MAX_HEIGHT,
	PROPERTY_START_X,
	PROPERTY_START_Y,
	PROPERTY_RATIO_X,
	PROPERTY_RATIO_Y,
	PROPERTY_REPEAT,
	PROPERTY_TEXT_BACKGROUND_COLOR,
	PROPERTY_TEXT_COLOR,
	PROPERTY_TEXT_SIZE,
	PROPERTY_TEXT_STYLE,
	PROPERTY_TEXT_FAMILY,
	PROPERTY_TEXT_LINE_HEIGHT,
	PROPERTY_TEXT_SHADOW,
	PROPERTY_TEXT_DECORATION,
	PROPERTY_TEXT_OVERFLOW,
	PROPERTY_TEXT_WHITE_SPACE,
	PROPERTY_ALIGN_X,
	PROPERTY_ALIGN_Y,
	PROPERTY_SHADOW,
	PROPERTY_SRC,
	PROPERTY_TIME = -1, // action time
	// Non meta attribute
	PROPERTY_TRANSLATE = -2,
	PROPERTY_SCALE = -3,
	PROPERTY_SKEW = -4,
	PROPERTY_ORIGIN = -5,
	PROPERTY_MARGIN = -6,
	PROPERTY_BORDER = -7,
	PROPERTY_BORDER_WIDTH = -8,
	PROPERTY_BORDER_COLOR = -9,
	PROPERTY_BORDER_RADIUS = -10,
	PROPERTY_BORDER_LEFT = -11,
	PROPERTY_BORDER_TOP = -12,
	PROPERTY_BORDER_RIGHT = -13,
	PROPERTY_BORDER_BOTTOM = -14,
	PROPERTY_MIN_WIDTH = -15,
	PROPERTY_MIN_HEIGHT = -16,
	PROPERTY_START = -17,
	PROPERTY_RATIO = -18,
	PROPERTY_ALIGN = -19,
}

export interface StyleSheet {
	// action time attribute
	time?: number;
	// Meta attribute
	x?: number;
	y?: number;
	scaleX?: number;
	scaleY?: number;
	skewX?: number;
	skewY?: number;
	rotateZ?: number;
	originX?: number;
	originY?: number;
	opacity?: number;
	visible?: boolean;
	width?: value.ValueIn;
	height?: value.ValueIn;
	marginTop?: value.ValueIn;
	marginRight?: value.ValueIn;
	marginBottom?: value.ValueIn;
	marginLeft?: value.ValueIn;
	borderTopWidth?: number;
	borderRightWidth?: number;
	borderBottomWidth?: number;
	borderLeftWidth?: number;
	borderTopColor?: value.ColorIn;
	borderRIghtColor?: value.ColorIn;
	borderBottomColor?: value.ColorIn;
	borderLeftColor?: value.ColorIn;
	borderRadiusLeftTop?: number;
	borderRadiusRightTop?: number;
	borderRadiusRightBottom?: number;
	borderRadiusLeftBottom?: number;
	backgroundColor?: value.ColorIn;
	background?: value.BackgroundIn;
	newline?: boolean;
	clip?: boolean;
	contentAlign?: value.ContentAlignIn;
	textAlign?: value.TextAlignIn;
	maxWidth?: value.ValueIn;
	maxHeight?: value.ValueIn;
	startX?: number;
	startY?: number;
	ratioX?: number;
	ratioY?: number;
	repeat?: value.RepeatIn;
	textBackgroundColor?: value.TextColorIn;
	textColor?: value.TextColorIn;
	textSize?: value.TextSizeIn;
	textStyle?: value.TextStyleIn;
	textFamily?: value.TextFamilyIn;
	textLineHeight?: value.TextLineHeightIn;
	textShadow?: value.TextShadowIn;
	textDecoration?: value.TextDecorationIn;
	textOverflow?: value.TextOverflowIn;
	textWhiteSpace?: value.TextWhiteSpaceIn;
	alignX?: value.AlignIn;
	alignY?: value.AlignIn;
	shadow?: value.ShadowIn;
	src?: string;
	// Non meta attribute
	translate?: value.Vec2In;
	scale?: value.Vec2In;
	skew?: value.Vec2In;
	origin?: value.Vec2In;
	margin?: value.ValuesIn;
	border?: value.BorderIn;
	borderWidth?: number;
	borderColor?: value.ColorIn;
	borderRadius?: number;
	borderLeft?: value.BorderIn;
	borderTop?: value.BorderIn;
	borderRight?: value.BorderIn;
	borderBottom?: value.BorderIn;
	minWidth?: value.ValueIn;
	minHeight?: value.ValueIn;
	start?: value.Vec2In;
	ratio?: value.Vec2In;
	align?: value.AlignsIn;
}

export declare function create(sheets: Dict<StyleSheet>): void;

/**
 * @func check(css_name)
 * @arg css_name {String}
 * @ret {bool}
 */
export function check(cssName: string, selected: string = '') {
	var name = cssName.replace(/([A-Z_]+)/g, '_$1');
	if ( !('PROPERTY_' + name.toUpperCase() in Propery) ) {
		console.warn( `---------- Invalid name "${cssName}" in CSS style sheet ${selected}` );
		return false;
	}
	return true;
}

/**
 * @func default(sheets)
 */
export default function(sheets: Dict<StyleSheet>) {
	if ( debug ) {
		for ( var cls in sheets ) {
			for ( var name in sheets[cls] ) {
				check(name, cls);
			}
		}
	}
	_css.create(sheets);
}
