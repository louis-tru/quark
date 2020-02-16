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

import * as value from './value';

export enum Propery {
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
	// ext
	PROPERTY_TIME = -1,
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
	x?: string;
	y?: any;
	scaleX?: any;
	scaleY?: any;
	skewX?: any;
	skewY?: any;
	rotateZ?: any;
	originX?: any;
	originY?: any;
	opacity?: any;
	visible?: any;
	width?: any;
	height?: any;
	marginTop?: any;
	marginRight?: any;
	marginBottom?: any;
	marginLeft?: any;
	borderTopWidth?: any;
	borderRightWidth?: any;
	borderBottomWidth?: any;
	borderLeftWidth?: any;
	borderTopColor?: any;
	borderRIghtColor?: any;
	borderBottomColor?: any;
	borderLeftColor?: any;
	borderRadiusLeftTop?: any;
	borderRadiusRightTop?: any;
	borderRadiusRightBottom?: any;
	borderRadiusLeftBottom?: any;
	backgroundColor?: any;
	background?: any;
	newline?: any;
	clip?: any;
	contentAlign?: any;
	textAlign?: any;
	maxWidth?: any;
	maxHeight?: any;
	startX?: any;
	startY?: any;
	ratioX?: any;
	ratioY?: any;
	repeat?: any;
	textBackgroundColor?: any;
	textColor?: any;
	textSize?: any;
	textStyle?: any;
	textFamily?: any;
	textLineHeight?: any;
	textShadow?: any;
	textDecoration?: any;
	textOverflow?: any;
	textWhiteSpace?: any;
	alignX?: any;
	alignY?: any;
	shadow?: any;
	src?: any;
	// ext
	time?: any;
	translate?: any;
	scale?: any;
	skew?: any;
	origin?: any;
	margin?: any;
	border?: any;
	borderWidth?: any;
	borderColor?: any;
	borderRadius?: any;
	borderLeft?: any;
	borderTop?: any;
	borderRight?: any;
	borderBottom?: any;
	minWidth?: any;
	minHeight?: any;
	start?: any;
	ratio?: any;
	align?: any;
}