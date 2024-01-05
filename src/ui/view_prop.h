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
	F(OPACITY, float, opacity) /*view*/\
	F(VISIBLE, bool, visible) \
	F(RECEIVE, bool, receive) \
	F(CLIP, bool, clip) /* box */ \
	F(WIDTH, BoxSize, width) \
	F(HEIGHT, BoxSize, height) \
	F(WIDTH, BoxSize, width_limit) \
	F(HEIGHT, BoxSize, height_limit) \
	F(MARGIN_TOP, float, margin_top) \
	F(MARGIN_RIGHT, float, margin_right) \
	F(MARGIN_BOTTOM, float, margin_bottom) \
	F(MARGIN_LEFT, float, margin_left) \
	F(MARGIN_TOP, float, padding_top)\
	F(MARGIN_RIGHT, float, padding_right) \
	F(MARGIN_BOTTOM, float, padding_bottom) \
	F(MARGIN_LEFT, float, padding_left) \
	F(BORDER_RADIUS_LEFT_TOP, float, border_radius_left_top) \
	F(BORDER_RADIUS_RIGHT_TOP, float, border_radius_right_top) \
	F(BORDER_RADIUS_RIGHT_BOTTOM, float, border_radius_right_bottom) \
	F(BORDER_RADIUS_LEFT_BOTTOM, float, border_radius_left_bottom) \
	F(BORDER_COLOR_TOP, Color, border_color_top) \
	F(BORDER_COLOR_RIGHT, Color, border_color_right) \
	F(BORDER_COLOR_BOTTOM, Color, border_color_bottom) \
	F(BORDER_COLOR_LEFT, Color, border_color_left) \
	F(BORDER_WIDTH_TOP, float, border_width_top) \
	F(BORDER_WIDTH_RIGHT, float, border_width_right) \
	F(BORDER_WIDTH_BOTTOM, float, border_width_bottom) \
	F(BORDER_WIDTH_LEFT, float, border_width_left) \
	F(BACKGROUND_COLOR, Color, background_color) \
	F(BACKGROUND, BoxFill*, background) \
	F(BOX_SHADOW, BoxShadow*, box_shadow) \
	F(WEIGHT, float, weight) \
	F(ALIGN, Align, align) \
	F(DIRECTION, Direction, direction) /*flex*/\
	F(ITEMS_ALIGN, ItemsAlign, items_align) \
	F(CROSS_ALIGN, CrossAlign, cross_align) \
	F(WRAP, Wrap, wrap) /*flow*/ \
	F(WRAP_ALIGN, WrapAlign, wrap_align) \
	F(SRC, String, src) /*image/video*//*****Large size data*****/\
	F(TEXT_ALIGN, TextAlign, text_align) /*text/input/label of TextOptions*/\
	F(TEXT_WEIGHT, TextWeight, text_weight) \
	F(TEXT_SLANT, TextSlant, text_slant) \
	F(TEXT_DECORATION, TextDecoration, text_decoration) \
	F(TEXT_OVERFLOW, TextOverflow, text_overflow) \
	F(TEXT_WHITE_SPACE, TextWhiteSpace, text_white_space) \
	F(TEXT_WORD_BREAK, TextWordBreak, text_word_break) \
	F(TEXT_SIZE, TextSize, text_size) \
	F(TEXT_BACKGROUND_COLOR, TextColor, text_background_color) \
	F(TEXT_COLOR, TextColor, text_color) \
	F(TEXT_LINE_HEIGHT, TextLineHeight, text_line_height) \
	F(TEXT_SHADOW, TextShadow, text_shadow) /*****Large size data*****/\
	F(TEXT_FAMILY, TextFamily, text_family) /*****Large size data*****/\
	F(SECURITY, bool, security) /*input*/\
	F(READONLY, bool, readonly) \
	F(KEYBOARD_TYPE, KeyboardType, type) \
	F(KEYBOARD_RETURN_TYPE, KeyboardReturnType, return_type) \
	F(PLACEHOLDER_COLOR, Color, placeholder_color) \
	F(CURSOR_COLOR, Color, cursor_color) \
	F(MAX_LENGTH, uint32_t, max_length) \
	F(PLACEHOLDER, String, placeholder) /*****Large size data*****/\
	F(SCROLLBAR_COLOR, Color, scrollbar_color) /*scroll/textarea of ScrollLayoutBase*/ \
	F(SCROLLBAR_WIDTH, float, scrollbar_width) \
	F(SCROLLBAR_MARGIN, float, scrollbar_margin) \
	F(X, float, x) /*transform*/ \
	F(Y, float, y) \
	F(SCALE_X, float, scale_x) \
	F(SCALE_Y, float, scale_y) \
	F(SKEW_X, float, skew_x) \
	F(SKEW_Y, float, skew_y) \
	F(ROTATE_Z, float, rotate_z) \
	F(ORIGIN_X, float, origin_x) \
	F(ORIGIN_Y, float, origin_y) \

	enum ViewType {
		kView_ViewType,
		kBox_ViewType,
		kFlex_ViewType, // box flex
		kFlow_ViewType, // box flex
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
		#define _Fun(Enum, Type, Name) k##Enum##_ViewProp,
			Qk_View_Props(_Fun)
			kEnum_Counts_ViewProp,
		#undef _Fun
	};

	struct ViewPropAccessor {
		void *layout_get, *layout_set, *get, *set;
	};

	Qk_EXPORT ViewPropAccessor* view_prop_accessor(ViewType type, ViewProp prop);
}

#endif