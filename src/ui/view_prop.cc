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

#include "./view_prop.h"
#include "./view/view.h"
#include "./view/box.h"
#include "./view/flex.h"
#include "./view/flow.h"
#include "./view/text.h"
#include "./view/input.h"
#include "./view/image.h"
#include "./view/scroll.h"
#include "./view/matrix.h"

namespace qk {
	typedef View ViewView;
	typedef void (Object::*Func)();

	#define Qk_Set_Accessor(View, Prop, Name) \
		prop_accessors[k##View##_ViewType].value[k##Prop##_ViewProp] =\
			{(Func)&View::Name,(Func)&View::set_##Name};

	#define Qk_Copy_Accessor(From, Dest, Index, Count) \
		prop_accessors[k##From##_ViewType].copy(k##Index##_ViewProp, Count, prop_accessors[k##Dest##_ViewType])

	struct PropAccessors {
		void copy(uint32_t index, uint32_t count, PropAccessors &dest) {
			auto end = index + count;
			do {
				dest.value[index] = value[index];
			} while (++index < end);
		}
		PropAccessor value[kEnum_Counts_ViewProp] = {0};
	};

	static PropAccessors *prop_accessors = nullptr;

	void view_prop_acc_init() {
		if (prop_accessors)
			return;
		prop_accessors = new PropAccessors[kEnum_Counts_ViewType];

		#define _Func_TextOptions_Props(_Func) \
			_Func(TextAlign, text_align) \
			_Func(TextSize, text_size) \
			_Func(TextColor, text_color) \
			_Func(TextLineHeight, text_line_height) \
			_Func(TextFamily, text_family) \
			_Func(TextShadow, text_shadow) \
			_Func(TextColor, text_background_color) \
			_Func(TextWeight, text_weight) \
			_Func(TextSlant, text_slant) \
			_Func(TextDecoration, text_decoration) \
			_Func(TextOverflow, text_overflow) \
			_Func(TextWhiteSpace, text_white_space) \
			_Func(TextWordBreak, text_word_break) \

		#define _Func_ScrollBase_Props(_Func) \
			_Func(Color, scrollbar_color) \
			_Func(float, scrollbar_width) \
			_Func(float, scrollbar_margin) \

		struct Text: View {
			#define _Func(Type, Name) Type Name() { return asTextOptions()->Name(); } \
				void set_##Name(Type v) { asTextOptions()->set_##Name(v); }
			_Func_TextOptions_Props(_Func)
			#undef _Func
			#undef _Func_TextOptions_Props
		};
		struct Scroll: View {
			#define _Func(Type, Name) Type Name() { return asScrollBase()->Name(); } \
				void set_##Name(Type v) { asScrollBase()->set_##Name(v); }
			_Func_ScrollBase_Props(_Func)
			#undef _Func
			#undef _Func_ScrollBase_Props
		};

		// view
		Qk_Set_Accessor(View, OPACITY, opacity);
		Qk_Set_Accessor(View, CURSOR, cursor);
		Qk_Set_Accessor(View, VISIBLE, visible);
		Qk_Set_Accessor(View, RECEIVE, receive);
		prop_accessors[kBox_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kFlex_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kFlow_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kFree_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kImage_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kVideo_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kInput_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kTextarea_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kLabel_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kScroll_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kText_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kButton_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kMatrix_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kRoot_ViewType] = prop_accessors[kView_ViewType];
		// box
		Qk_Set_Accessor(Box, CLIP, clip);
		Qk_Set_Accessor(Box, ALIGN, align);
		Qk_Set_Accessor(Box, WIDTH, width);
		Qk_Set_Accessor(Box, HEIGHT, height);
		Qk_Set_Accessor(Box, MIN_WIDTH, min_width);
		Qk_Set_Accessor(Box, MIN_HEIGHT, min_height);
		Qk_Set_Accessor(Box, MAX_WIDTH, max_width);
		Qk_Set_Accessor(Box, MAX_HEIGHT, max_height);
		Qk_Set_Accessor(Box, MARGIN, margin); // margin
		Qk_Set_Accessor(Box, MARGIN_TOP, margin_top);
		Qk_Set_Accessor(Box, MARGIN_RIGHT, margin_right);
		Qk_Set_Accessor(Box, MARGIN_BOTTOM, margin_bottom);
		Qk_Set_Accessor(Box, MARGIN_LEFT, margin_left);
		Qk_Set_Accessor(Box, MARGIN, padding); // padding
		Qk_Set_Accessor(Box, MARGIN_TOP, padding_top);
		Qk_Set_Accessor(Box, MARGIN_RIGHT, padding_right);
		Qk_Set_Accessor(Box, MARGIN_BOTTOM, padding_bottom);
		Qk_Set_Accessor(Box, MARGIN_LEFT, padding_left);
		Qk_Set_Accessor(Box, BORDER_RADIUS, border_radius); // border radius
		Qk_Set_Accessor(Box, BORDER_RADIUS_LEFT_TOP, border_radius_left_top);
		Qk_Set_Accessor(Box, BORDER_RADIUS_RIGHT_TOP, border_radius_right_top);
		Qk_Set_Accessor(Box, BORDER_RADIUS_RIGHT_BOTTOM, border_radius_right_bottom);
		Qk_Set_Accessor(Box, BORDER_RADIUS_LEFT_BOTTOM, border_radius_left_bottom);
		Qk_Set_Accessor(Box, BORDER, border); // border
		Qk_Set_Accessor(Box, BORDER_WIDTH, border_width);
		Qk_Set_Accessor(Box, BORDER_COLOR, border_color);
		Qk_Set_Accessor(Box, BORDER_TOP, border_top);
		Qk_Set_Accessor(Box, BORDER_RIGHT, border_right);
		Qk_Set_Accessor(Box, BORDER_BOTTOM, border_bottom);
		Qk_Set_Accessor(Box, BORDER_LEFT, border_left);
		Qk_Set_Accessor(Box, BORDER_WIDTH_TOP, border_width_top); // border width
		Qk_Set_Accessor(Box, BORDER_WIDTH_RIGHT, border_width_right);
		Qk_Set_Accessor(Box, BORDER_WIDTH_BOTTOM, border_width_bottom);
		Qk_Set_Accessor(Box, BORDER_WIDTH_LEFT, border_width_left);
		Qk_Set_Accessor(Box, BORDER_COLOR_TOP, border_color_top); // border color
		Qk_Set_Accessor(Box, BORDER_COLOR_RIGHT, border_color_right);
		Qk_Set_Accessor(Box, BORDER_COLOR_BOTTOM, border_color_bottom);
		Qk_Set_Accessor(Box, BORDER_COLOR_LEFT, border_color_left); // border end
		Qk_Set_Accessor(Box, BACKGROUND_COLOR, background_color);
		Qk_Set_Accessor(Box, BACKGROUND, background);
		Qk_Set_Accessor(Box, BOX_SHADOW, box_shadow);
		Qk_Set_Accessor(Box, WEIGHT, weight);
		Qk_Copy_Accessor(Box, Flex, CLIP, 42);
		Qk_Copy_Accessor(Box, Flow, CLIP, 42);
		Qk_Copy_Accessor(Box, Free, CLIP, 42);
		Qk_Copy_Accessor(Box, Image, CLIP, 42);
		Qk_Copy_Accessor(Box, Video, CLIP, 42);
		Qk_Copy_Accessor(Box, Input, CLIP, 42);
		Qk_Copy_Accessor(Box, Textarea, CLIP, 42);
		Qk_Copy_Accessor(Box, Scroll, CLIP, 42);
		Qk_Copy_Accessor(Box, Text, CLIP, 42);
		Qk_Copy_Accessor(Box, Button, CLIP, 42);
		Qk_Copy_Accessor(Box, Matrix, CLIP, 42);
		Qk_Copy_Accessor(Box, Root, CLIP, 42);
		// flex
		Qk_Set_Accessor(Flex, DIRECTION, direction);
		Qk_Set_Accessor(Flex, ITEMS_ALIGN, items_align);
		Qk_Set_Accessor(Flex, CROSS_ALIGN, cross_align);
		Qk_Copy_Accessor(Flex, Flow, DIRECTION, 3);
		// flow
		Qk_Set_Accessor(Flow, WRAP, wrap);
		Qk_Set_Accessor(Flow, WRAP_ALIGN, wrap_align);
		// image
		Qk_Set_Accessor(Image, SRC, src);
		Qk_Copy_Accessor(Image, Video, SRC, 1);
		// text/input/label of TextOptions
		Qk_Set_Accessor(Text, TEXT_ALIGN, text_align);
		Qk_Set_Accessor(Text, TEXT_WEIGHT, text_weight);
		Qk_Set_Accessor(Text, TEXT_SLANT, text_slant);
		Qk_Set_Accessor(Text, TEXT_DECORATION, text_decoration);
		Qk_Set_Accessor(Text, TEXT_OVERFLOW, text_overflow);
		Qk_Set_Accessor(Text, TEXT_WHITE_SPACE, text_white_space);
		Qk_Set_Accessor(Text, TEXT_WORD_BREAK, text_word_break);
		Qk_Set_Accessor(Text, TEXT_SIZE, text_size);
		Qk_Set_Accessor(Text, TEXT_BACKGROUND_COLOR, text_background_color);
		Qk_Set_Accessor(Text, TEXT_COLOR, text_color);
		Qk_Set_Accessor(Text, TEXT_LINE_HEIGHT, text_line_height);
		Qk_Set_Accessor(Text, TEXT_SHADOW, text_shadow);
		Qk_Set_Accessor(Text, TEXT_FAMILY, text_family);
		Qk_Copy_Accessor(Text, Input, TEXT_ALIGN, 13);
		Qk_Copy_Accessor(Text, Textarea, TEXT_ALIGN, 13);
		Qk_Copy_Accessor(Text, Label, TEXT_ALIGN, 13);
		// input/textarea
		Qk_Set_Accessor(Input, SECURITY, security);
		Qk_Set_Accessor(Input, READONLY, readonly);
		Qk_Set_Accessor(Input, KEYBOARD_TYPE, type);
		Qk_Set_Accessor(Input, KEYBOARD_RETURN_TYPE, return_type);
		Qk_Set_Accessor(Input, PLACEHOLDER_COLOR, placeholder_color);
		Qk_Set_Accessor(Input, CURSOR_COLOR, cursor_color);
		Qk_Set_Accessor(Input, MAX_LENGTH, max_length);
		Qk_Set_Accessor(Input, PLACEHOLDER, placeholder);
		Qk_Copy_Accessor(Input, Textarea, SECURITY, 8);
		// scroll/textarea of ScrollViewBase
		Qk_Set_Accessor(Scroll, SCROLLBAR_COLOR, scrollbar_color);
		Qk_Set_Accessor(Scroll, SCROLLBAR_WIDTH, scrollbar_width);
		Qk_Set_Accessor(Scroll, SCROLLBAR_MARGIN, scrollbar_margin);
		Qk_Copy_Accessor(Scroll, Textarea, SCROLLBAR_COLOR, 3);
		// matrix
		Qk_Set_Accessor(Matrix, TRANSLATE, translate);
		Qk_Set_Accessor(Matrix, SCALE, scale);
		Qk_Set_Accessor(Matrix, SKEW, skew);
		Qk_Set_Accessor(Matrix, ORIGIN, origin);
		Qk_Set_Accessor(Matrix, X, x);
		Qk_Set_Accessor(Matrix, Y, y);
		Qk_Set_Accessor(Matrix, SCALE_X, scale_x);
		Qk_Set_Accessor(Matrix, SCALE_Y, scale_y);
		Qk_Set_Accessor(Matrix, SKEW_X, skew_x);
		Qk_Set_Accessor(Matrix, SKEW_Y, skew_y);
		Qk_Set_Accessor(Matrix, ROTATE_Z, rotate_z);
		Qk_Set_Accessor(Matrix, ORIGIN_X, origin_x);
		Qk_Set_Accessor(Matrix, ORIGIN_Y, origin_y);
	}

	PropAccessor* prop_accessor_at_view(ViewType type, ViewProp prop) {
		return prop_accessors[type].value + prop;
	}
}
