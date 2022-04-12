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

#ifndef __flare__property__
#define __flare__property__

#include "./util/util.h"
#include "./util/dict.h"

namespace flare {

	class View;

#define F_EACH_PROPERTY_TABLE(F) \
	F(PROPERTY_X, float, x) /*view*/ \
	F(PROPERTY_Y, float, y) \
	F(PROPERTY_SCALE_X, float, scale_x) \
	F(PROPERTY_SCALE_Y, float, scale_y) \
	F(PROPERTY_SKEW_X, float, skew_x) \
	F(PROPERTY_SKEW_Y, float, skew_y) \
	F(PROPERTY_ROTATE_Z, float, rotate_z) \
	F(PROPERTY_ORIGIN_X, float, origin_x) \
	F(PROPERTY_ORIGIN_Y, float, origin_y) \
	F(PROPERTY_OPACITY, float, opacity) \
	F(PROPERTY_VISIBLE, bool, visible) \
	F(PROPERTY_WIDTH, Value, width)  /* box / Value */ \
	F(PROPERTY_HEIGHT, Value, height) /* Value */ \
	F(PROPERTY_MARGIN_TOP, Value, margin_top) \
	F(PROPERTY_MARGIN_RIGHT, Value, margin_right) \
	F(PROPERTY_MARGIN_BOTTOM, Value, margin_bottom) \
	F(PROPERTY_MARGIN_LEFT, Value, margin_left) \
	F(PROPERTY_BORDER_TOP_WIDTH, float, border_top_width) \
	F(PROPERTY_BORDER_RIGHT_WIDTH, float, border_right_width) \
	F(PROPERTY_BORDER_BOTTOM_WIDTH, float, border_bottom_width) \
	F(PROPERTY_BORDER_LEFT_WIDTH, float, border_left_width) \
	F(PROPERTY_BORDER_TOP_COLOR, Color, border_top_color) \
	F(PROPERTY_BORDER_RIGHT_COLOR, Color, border_right_color) \
	F(PROPERTY_BORDER_BOTTOM_COLOR, Color, border_bottom_color) \
	F(PROPERTY_BORDER_LEFT_COLOR, Color, border_left_color) \
	F(PROPERTY_BORDER_RADIUS_LEFT_TOP, float, border_radius_left_top) \
	F(PROPERTY_BORDER_RADIUS_RIGHT_TOP, float, border_radius_right_top) \
	F(PROPERTY_BORDER_RADIUS_RIGHT_BOTTOM, float, border_radius_right_bottom) \
	F(PROPERTY_BORDER_RADIUS_LEFT_BOTTOM, float, border_radius_left_bottom) \
	F(PROPERTY_BACKGROUND_COLOR, Color, background_color) \
	F(PROPERTY_BACKGROUND, BackgroundPtr, background) \
	F(PROPERTY_NEWLINE, bool, newline) \
	F(PROPERTY_CLIP, bool, clip) \
	F(PROPERTY_CONTENT_ALIGN, ContentAlign, content_align) /* div*/ \
	F(PROPERTY_TEXT_ALIGN, TextAlign, text_align) /* text / label */ \
	F(PROPERTY_MAX_WIDTH, Value, max_width) /* limit / limit-indep */ \
	F(PROPERTY_MAX_HEIGHT, Value, max_height) \
	F(PROPERTY_START_X, float, start_x) /* sprite */ \
	F(PROPERTY_START_Y, float, start_y) \
	F(PROPERTY_RATIO_X, float, ratio_x) \
	F(PROPERTY_RATIO_Y, float, ratio_y) \
	F(PROPERTY_REPEAT, Repeat, repeat) \
	F(PROPERTY_TEXT_BACKGROUND_COLOR, TextColor, text_background_color) /* text-font */ \
	F(PROPERTY_TEXT_COLOR, TextColor, text_color) \
	F(PROPERTY_TEXT_SIZE, TextSize, text_size) \
	F(PROPERTY_TEXT_STYLE, TextStyle, text_style) \
	F(PROPERTY_TEXT_FAMILY, TextFamily, text_family) \
	F(PROPERTY_TEXT_LINE_HEIGHT, TextLineHeight, text_line_height) \
	F(PROPERTY_TEXT_SHADOW, TextShadow, text_shadow) \
	F(PROPERTY_TEXT_DECORATION, TextDecoration, text_decoration) \
	F(PROPERTY_TEXT_OVERFLOW, TextOverflow, text_overflow) \
	F(PROPERTY_TEXT_WHITE_SPACE, TextWhiteSpace, text_white_space) \
	F(PROPERTY_ALIGN_X, Align, align_x) /* indep */ \
	F(PROPERTY_ALIGN_Y, Align, align_y) \
	F(PROPERTY_SHADOW, Shadow, shadow) /* box-shadow */ \
	F(PROPERTY_SRC, String, src)    /* image */ \

	/**
	 * @func PropertyName
	 */
	enum PropertyName: uint32_t {
#define fx_def_enum(ENUM, TYPE, NAME) ENUM,
		F_EACH_PROPERTY_TABLE(fx_def_enum)
#undef fx_def_enum
	};

	template<> uint64_t Compare<PropertyName>::hash_code(const PropertyName& key) { return key; }

	/**
	 * @class PropertysAccessor
	 */
	class F_EXPORT PropertysAccessor: public Object {
	public:
		
		typedef void (View::*Func)();
		typedef int ViewType;
		
		struct F_EXPORT Accessor {
			inline Accessor(): get_accessor(nullptr), set_accessor(nullptr) {}

			template<typename T, typename T2>
			inline Accessor(T get, T2 set): get_accessor(Func(get)), set_accessor(Func(set)) {}

			Func get_accessor;
			Func set_accessor;
		};
		
		PropertysAccessor();
		
		/**
		 * @func accessor
		 */
		Accessor accessor(ViewType type, PropertyName name);
		
		/**
		 * @func has_accessor
		 */
		bool has_accessor(ViewType type, PropertyName name);
		
		/**
		 * @func shared
		 */
		static PropertysAccessor* shared();
		
	private:
		
		typedef Dict<PropertyName, Accessor> Accessors;
		
		Dict<ViewType, Accessors> _property_func_table;
	};

}

#endif
