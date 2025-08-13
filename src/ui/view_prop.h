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
#include "../render/bezier.h"
#include "./types.h"

#define Qk_DEFINE_VIEW_ACCE_GET(...) Qk_DEFINE_ACCE_GET(__VA_ARGS__)
#define Qk_DEFINE_VIEW_ACCESSOR(type,name,...) \
	Qk_DEFINE_VIEW_ACCE_GET(type,name,##__VA_ARGS__); void set_##name (type val,bool isRt=false);

#define Qk_DEFINE_VIEW_PROP_GET(...) Qk_DEFINE_PROP_GET(__VA_ARGS__)
#define Qk_DEFINE_VIEW_PROPERTY(type,name,...) \
	Qk_DEFINE_VIEW_PROP_GET(type,name,##__VA_ARGS__) void set_##name (type val,bool isRt=false)

#define Qk_DEFINE_VIEW_PROP_GET_Atomic(...) Qk_DEFINE_PROP_GET_Atomic(__VA_ARGS__)
#define Qk_DEFINE_VIEW_PROPERTY_Atomic(type,name,...) \
	Qk_DEFINE_VIEW_PROP_GET_Atomic(type,name,##__VA_ARGS__) void set_##name (type val,bool isRt=false)

namespace qk {
	class   BoxFilter;
	class   BoxShadow;
	typedef BoxFilter* BoxFilterPtr;
	typedef BoxShadow* BoxShadowPtr;
	typedef Array<float> ArrayFloat;
	typedef Array<Color> ArrayColor;
	typedef Array<BoxOrigin> ArrayOrigin;
	typedef Array<BoxBorder> ArrayBorder;

#define Qk_View_Props(F) \
	F(OPACITY, float, opacity, View) /*view*/\
	F(CURSOR, CursorStyle, cursor, View) \
	F(VISIBLE, bool, visible, View) \
	F(RECEIVE, bool, receive, View) \
	F(CLIP, bool, clip, Box) /* box */ \
	F(ALIGN, Align, align, Box) \
	F(WIDTH, BoxSize, width, Box) \
	F(HEIGHT, BoxSize, height, Box) \
	F(MIN_WIDTH, BoxSize, min_width, Box) \
	F(MIN_HEIGHT, BoxSize, min_height, Box) \
	F(MAX_WIDTH, BoxSize, max_width, Box) \
	F(MAX_HEIGHT, BoxSize, max_height, Box) \
	F(MARGIN, ArrayFloat, margin, Box) /*margin*/\
	F(MARGIN_TOP, float, margin_top, Box) \
	F(MARGIN_RIGHT, float, margin_right, Box) \
	F(MARGIN_BOTTOM, float, margin_bottom, Box) \
	F(MARGIN_LEFT, float, margin_left, Box) \
	F(PADDING, ArrayFloat, padding, Box) /*padding*/\
	F(PADDING_TOP, float, padding_top, Box) \
	F(PADDING_RIGHT, float, padding_right, Box) \
	F(PADDING_BOTTOM, float, padding_bottom, Box) \
	F(PADDING_LEFT, float, padding_left, Box) \
	F(BORDER_RADIUS, ArrayFloat, border_radius, Box) /*border radius*/\
	F(BORDER_RADIUS_LEFT_TOP, float, border_radius_left_top, Box) \
	F(BORDER_RADIUS_RIGHT_TOP, float, border_radius_right_top, Box) \
	F(BORDER_RADIUS_RIGHT_BOTTOM, float, border_radius_right_bottom, Box) \
	F(BORDER_RADIUS_LEFT_BOTTOM, float, border_radius_left_bottom, Box) \
	F(BORDER, ArrayBorder, border, Box) /*border*/\
	F(BORDER_WIDTH, ArrayFloat, border_width, Box) \
	F(BORDER_COLOR, ArrayColor, border_color, Box) \
	F(BORDER_TOP, BoxBorder, border_top, Box) \
	F(BORDER_RIGHT, BoxBorder, border_right, Box) \
	F(BORDER_BOTTOM, BoxBorder, border_bottom, Box) \
	F(BORDER_LEFT, BoxBorder, border_left, Box) \
	F(BORDER_COLOR_TOP, Color, border_color_top, Box) \
	F(BORDER_COLOR_RIGHT, Color, border_color_right, Box) \
	F(BORDER_COLOR_BOTTOM, Color, border_color_bottom, Box) \
	F(BORDER_COLOR_LEFT, Color, border_color_left, Box) \
	F(BORDER_WIDTH_TOP, float, border_width_top, Box) \
	F(BORDER_WIDTH_RIGHT, float, border_width_right, Box) \
	F(BORDER_WIDTH_BOTTOM, float, border_width_bottom, Box) \
	F(BORDER_WIDTH_LEFT, float, border_width_left, Box) /*border end*/\
	F(BACKGROUND_COLOR, Color, background_color, Box) \
	F(BACKGROUND, BoxFilterPtr, background, Box) \
	F(BOX_SHADOW, BoxShadowPtr, box_shadow, Box) \
	F(WEIGHT, Vec2, weight, Box) \
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
	F(SCROLLBAR_COLOR, Color, scrollbar_color, ScrollView) /*scroll/textarea of ScrollView*/ \
	F(SCROLLBAR_WIDTH, float, scrollbar_width, ScrollView) \
	F(SCROLLBAR_MARGIN, float, scrollbar_margin, ScrollView) \
	F(TRANSLATE, Vec2, translate, MatrixView) /*matrix/sprite of MatrixView*/ \
	F(SCALE, Vec2, scale, MatrixView) \
	F(SKEW, Vec2, skew, MatrixView) \
	F(ORIGIN, Vec2, origin, MatrixView) \
	F(X, float, x, MatrixView) \
	F(Y, float, y, MatrixView) \
	F(SCALE_X, float, scale_x, MatrixView) \
	F(SCALE_Y, float, scale_y, MatrixView) \
	F(SKEW_X, float, skew_x, MatrixView) \
	F(SKEW_Y, float, skew_y, MatrixView) \
	F(ROTATE_Z, float, rotate_z, MatrixView) \
	F(ORIGIN_X, float, origin_x, MatrixView) \
	F(ORIGIN_Y, float, origin_y, MatrixView) \
	F(BOX_ORIGIN, ArrayOrigin, box_origin, Matrix) /*Matrix*/\
	F(BOX_ORIGIN_X, BoxOrigin, box_origin_x, Matrix) \
	F(BOX_ORIGIN_Y, BoxOrigin, box_origin_y, Matrix) \
	F(CURVE, Curve, curve, CSS) /*extends*//*****Large size data*****/\

	// Unsupported attributes for ScrollView:
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
		kSprite_ViewType,
		kLabel_ViewType, // textOpts
		kBox_ViewType,  // box
		kFlex_ViewType, // box flex
		kFlow_ViewType, // box flow
		kFree_ViewType, // box
		kImage_ViewType, // box
		kVideo_ViewType, // box
		kInput_ViewType, // box textOpts input
		kTextarea_ViewType, // box textOpts input
		kScroll_ViewType, // box
		kText_ViewType, // box textOpts
		kButton_ViewType, // box
		kMatrix_ViewType, // box
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

	Qk_EXPORT PropAccessor* get_props_accessor(ViewType type, ViewProp prop);
}

#endif
