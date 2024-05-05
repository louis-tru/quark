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

#ifndef __quark__ui__view_prop__
#define __quark__ui__view_prop__

#include "../util/util.h"

namespace qk {
	class BoxFilter;
	class BoxShadow;

#define Qk_View_Props(F) \
	F(OPACITY, float, opacity, View) /*view*/\
	F(VISIBLE, bool, visible, View) \
	F(RECEIVE, bool, receive, View) \
	F(CLIP, bool, clip, Box) /* box */ \
	F(WIDTH, BoxSize, width, Box) \
	F(HEIGHT, BoxSize, height, Box) \
	F(WIDTH_LIMIT, BoxSize, width_limit, Box) \
	F(HEIGHT_LIMIT, BoxSize, height_limit, Box) \
	F(MARGIN_TOP, float, margin_top, Box) \
	F(MARGIN_RIGHT, float, margin_right, Box) \
	F(MARGIN_BOTTOM, float, margin_bottom, Box) \
	F(MARGIN_LEFT, float, margin_left, Box) \
	F(PADDING_TOP, float, padding_top, Box) \
	F(PADDING_RIGHT, float, padding_right, Box) \
	F(PADDING_BOTTOM, float, padding_bottom, Box) \
	F(PADDING_LEFT, float, padding_left, Box) \
	F(BORDER_RADIUS_LEFT_TOP, float, border_radius_left_top, Box) \
	F(BORDER_RADIUS_RIGHT_TOP, float, border_radius_right_top, Box) \
	F(BORDER_RADIUS_RIGHT_BOTTOM, float, border_radius_right_bottom, Box) \
	F(BORDER_RADIUS_LEFT_BOTTOM, float, border_radius_left_bottom, Box) \
	F(BORDER_COLOR_TOP, Color, border_color_top, Box) \
	F(BORDER_COLOR_RIGHT, Color, border_color_right, Box) \
	F(BORDER_COLOR_BOTTOM, Color, border_color_bottom, Box) \
	F(BORDER_COLOR_LEFT, Color, border_color_left, Box) \
	F(BORDER_WIDTH_TOP, float, border_width_top, Box) \
	F(BORDER_WIDTH_RIGHT, float, border_width_right, Box) \
	F(BORDER_WIDTH_BOTTOM, float, border_width_bottom, Box) \
	F(BORDER_WIDTH_LEFT, float, border_width_left, Box) \
	F(BACKGROUND_COLOR, Color, background_color, Box) \
	F(BACKGROUND, BoxFilter*, background, Box) \
	F(BOX_SHADOW, BoxShadow*, box_shadow, Box) \
	F(WEIGHT, float, weight, Box) \
	F(ALIGN, Align, align, Box) \
	F(DIRECTION, Direction, direction, Flex) /*flex*/\
	F(ITEMS_ALIGN, ItemsAlign, items_align, Flex) \
	F(CROSS_ALIGN, CrossAlign, cross_align, Flex) \
	F(WRAP, Wrap, wrap, Flow) /*flow*/ \
	F(WRAP_ALIGN, WrapAlign, wrap_align, Flow) \
	F(SRC, String, src, Image) /*image/video*//*****Large size data*****/\
	F(TEXT_ALIGN, TextAlign, text_align, TextOptions) /*text/input/label of TextOptions*/\
	F(TEXT_WEIGHT, TextWeight, text_weight, TextOptions) \
	F(TEXT_SLANT, TextSlant, text_slant, TextOptions) \
	F(TEXT_DECORATION, TextDecoration, text_decoration, TextOptions) \
	F(TEXT_OVERFLOW, TextOverflow, text_overflow, TextOptions) \
	F(TEXT_WHITE_SPACE, TextWhiteSpace, text_white_space, TextOptions) \
	F(TEXT_WORD_BREAK, TextWordBreak, text_word_break, TextOptions) \
	F(TEXT_SIZE, TextSize, text_size, TextOptions) \
	F(TEXT_BACKGROUND_COLOR, TextColor, text_background_color, TextOptions) \
	F(TEXT_COLOR, TextColor, text_color, TextOptions) \
	F(TEXT_LINE_HEIGHT, TextLineHeight, text_line_height, TextOptions) \
	F(TEXT_SHADOW, TextShadow, text_shadow, TextOptions) /*****Large size data*****/\
	F(TEXT_FAMILY, TextFamily, text_family, TextOptions) /*****Large size data*****/\
	F(SECURITY, bool, security, Input) /*input*/\
	F(READONLY, bool, readonly, Input) \
	F(KEYBOARD_TYPE, KeyboardType, type, Input) \
	F(KEYBOARD_RETURN_TYPE, KeyboardReturnType, return_type, Input) \
	F(PLACEHOLDER_COLOR, Color, placeholder_color, Input) \
	F(CURSOR_COLOR, Color, cursor_color, Input) \
	F(MAX_LENGTH, uint32_t, max_length, Input) \
	F(PLACEHOLDER, String, placeholder, Input) /*****Large size data*****/\
	F(SCROLLBAR_COLOR, Color, scrollbar_color, ScrollBase) /*scroll/textarea of ScrollBase*/ \
	F(SCROLLBAR_WIDTH, float, scrollbar_width, ScrollBase) \
	F(SCROLLBAR_MARGIN, float, scrollbar_margin, ScrollBase) \
	F(X, float, x, Transform) /*transform*/ \
	F(Y, float, y, Transform) \
	F(SCALE_X, float, scale_x, Transform) \
	F(SCALE_Y, float, scale_y, Transform) \
	F(SKEW_X, float, skew_x, Transform) \
	F(SKEW_Y, float, skew_y, Transform) \
	F(ROTATE_Z, float, rotate_z, Transform) \
	F(ORIGIN_X, float, origin_x, Transform) \
	F(ORIGIN_Y, float, origin_y, Transform) \

	// Unsupported attributes for ScrollBase:
	// (ScrollBase, bool, scrollbar, scrollbar)
	// (ScrollBase, bool, bounce, bounce)
	// (ScrollBase, bool, bounce_lock, bounceLock)
	// (ScrollBase, bool, momentum, momentum)
	// (ScrollBase, bool, lock_direction, lockDirection)
	// (ScrollBase, float, scroll_x, scrollX)
	// (ScrollBase, float, scroll_y, scrollY)
	// (ScrollBase, Vec2, scroll, scroll)
	// (ScrollBase, float, resistance, resistance)
	// (ScrollBase, float, catch_position_x, catchPositionX)
	// (ScrollBase, float, catch_position_y, catchPositionY)
	// (ScrollBase, uint32_t, scroll_duration, scrollDuration)
	// (ScrollBase, Curve, default_curve, defaultCurve)

	enum ViewType {
		kView_ViewType,
		kBox_ViewType,
		kFlex_ViewType, // box flex
		kFlow_ViewType, // box flow
		kFloat_ViewType, // box
		kImage_ViewType, // box
		kVideo_ViewType, // box
		kInput_ViewType, // box textOpts input
		kTextarea_ViewType, // box textOpts input
		kLabel_ViewType, // textOpts
		kScroll_ViewType, // box
		kText_ViewType, // box textOpts
		kButton_ViewType, // box
		kTransform_ViewType, // box
		kRoot_ViewType, // box
		kEnum_Counts_ViewType,
	};

	enum ViewProp {
		#define _Fun(Enum, Type, Name, From) k##Enum##_ViewProp,
			Qk_View_Props(_Fun)
			kEnum_Counts_ViewProp,
		#undef _Fun
	};

	struct PropAccessor {
		void (Object::*get)();
		void (Object::*set)();
	};

	Qk_EXPORT PropAccessor* prop_accessor_at_view(ViewType type, ViewProp prop);
}

#endif
