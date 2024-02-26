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
#include "./layout/layout.h"
#include "./layout/box.h"
#include "./layout/flex.h"
#include "./layout/flow.h"
#include "./layout/text.h"
#include "./layout/input.h"
#include "./layout/image.h"
#include "./layout/scroll.h"
#include "./layout/transform.h"

namespace qk {
	typedef Layout ViewLayout;
	typedef void (Object::*Func)();

	#define Qk_Set_Accessor(View, Prop, Name) \
		prop_accessors[k##View##_ViewType].value[k##Prop##_ViewProp] = {\
			(Func)&View##Layout::Name,(Func)&View##Layout::set_##Name};
	// prop_accessors[k##View##_ViewType].value[k##Prop##_ViewProp+kEnum_Counts_ViewProp] = {\
	// 	(Func)&View::Name,(Func)&View::set_##Name}\

	#define Qk_Copy_Accessor(From, Dest, Index, Count) \
		prop_accessors[k##From##_ViewType].copy(k##Index##_ViewProp, Count, prop_accessors[k##Dest##_ViewType])

	struct PropAccessors {
		void copy(uint32_t index, uint32_t count, PropAccessors &dest) {
			// auto index2 = index + kEnum_Counts_ViewProp;
			auto end = index + count;
			do {
				dest.value[index] = value[index];
				// dest.value[index2] = value[index2];
			} while (++index < end);
		}
		PropAccessor value[kEnum_Counts_ViewProp/*+kEnum_Counts_ViewProp*/] = {0};
	};

	static PropAccessors *prop_accessors = nullptr;

	void view_prop_acc_init() {
		if (prop_accessors)
			return;
		prop_accessors = new PropAccessors[kEnum_Counts_ViewType];

		#define _Func_TextOptions_Props(_Func) \
			_Func(TextAlign, text_align) \
			_Func(TextWeight, text_weight) \
			_Func(TextSlant, text_slant) \
			_Func(TextDecoration, text_decoration) \
			_Func(TextOverflow, text_overflow) \
			_Func(TextWhiteSpace, text_white_space) \
			_Func(TextWordBreak, text_word_break) \
			_Func(TextSize, text_size) \
			_Func(TextColor, text_background_color) \
			_Func(TextColor, text_color) \
			_Func(TextLineHeight, text_line_height) \
			_Func(TextShadow, text_shadow) \
			_Func(TextFamily, text_family) \

		#define _Func_ScrollLayoutBase_Props(_Func) \
			_Func(Color, scrollbar_color) \
			_Func(float, scrollbar_width) \
			_Func(float, scrollbar_margin) \

		struct TextLayout: public Layout {
			#define _Func(Type, Name) Type Name() { asTextOptions()->Name(); } \
				void set_##Name(Type v) { asTextOptions()->set_##Name(v); }
			_Func_TextOptions_Props(_Func)
			#undef _Func
			#undef _Func_TextOptions_Props
		};
		struct ScrollLayout: public Layout {
			#define _Func(Type, Name) Type Name() { asScrollLayoutBase()->Name(); } \
				void set_##Name(Type v) { asScrollLayoutBase()->set_##Name(v); }
			_Func_ScrollLayoutBase_Props(_Func)
			#undef _Func
			#undef _Func_ScrollLayoutBase_Props
		};

		// view
		Qk_Set_Accessor(View, OPACITY, opacity);
		Qk_Set_Accessor(View, VISIBLE, visible);
		Qk_Set_Accessor(View, RECEIVE, receive);
		prop_accessors[kBox_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kFlex_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kFlow_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kFloat_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kImage_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kVideo_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kInput_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kTextarea_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kLabel_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kScroll_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kText_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kButton_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kTransform_ViewType] = prop_accessors[kView_ViewType];
		prop_accessors[kRoot_ViewType] = prop_accessors[kView_ViewType];
		// box
		Qk_Set_Accessor(Box, CLIP, clip);
		Qk_Set_Accessor(Box, WIDTH, width);
		Qk_Set_Accessor(Box, HEIGHT, height);
		Qk_Set_Accessor(Box, WIDTH, width_limit);
		Qk_Set_Accessor(Box, HEIGHT, height_limit);
		Qk_Set_Accessor(Box, MARGIN_TOP, margin_top);
		Qk_Set_Accessor(Box, MARGIN_RIGHT, margin_right);
		Qk_Set_Accessor(Box, MARGIN_BOTTOM, margin_bottom);
		Qk_Set_Accessor(Box, MARGIN_LEFT, margin_left);
		Qk_Set_Accessor(Box, MARGIN_TOP, padding_top);
		Qk_Set_Accessor(Box, MARGIN_RIGHT, padding_right);
		Qk_Set_Accessor(Box, MARGIN_BOTTOM, padding_bottom);
		Qk_Set_Accessor(Box, MARGIN_LEFT, padding_left);
		Qk_Set_Accessor(Box, BORDER_RADIUS_LEFT_TOP, border_radius_left_top);
		Qk_Set_Accessor(Box, BORDER_RADIUS_RIGHT_TOP, border_radius_right_top);
		Qk_Set_Accessor(Box, BORDER_RADIUS_RIGHT_BOTTOM, border_radius_right_bottom);
		Qk_Set_Accessor(Box, BORDER_RADIUS_LEFT_BOTTOM, border_radius_left_bottom);
		Qk_Set_Accessor(Box, BORDER_COLOR_TOP, border_color_top);
		Qk_Set_Accessor(Box, BORDER_COLOR_RIGHT, border_color_right);
		Qk_Set_Accessor(Box, BORDER_COLOR_BOTTOM, border_color_bottom);
		Qk_Set_Accessor(Box, BORDER_COLOR_LEFT, border_color_left);
		Qk_Set_Accessor(Box, BORDER_WIDTH_TOP, border_width_top);
		Qk_Set_Accessor(Box, BORDER_WIDTH_RIGHT, border_width_right);
		Qk_Set_Accessor(Box, BORDER_WIDTH_BOTTOM, border_width_bottom);
		Qk_Set_Accessor(Box, BORDER_WIDTH_LEFT, border_width_left);
		Qk_Set_Accessor(Box, BACKGROUND_COLOR, background_color);
		Qk_Set_Accessor(Box, BACKGROUND, background);
		Qk_Set_Accessor(Box, BOX_SHADOW, box_shadow);
		Qk_Set_Accessor(Box, WEIGHT, weight);
		Qk_Set_Accessor(Box, ALIGN, align);
		Qk_Copy_Accessor(Box, Flex, CLIP, 30);
		Qk_Copy_Accessor(Box, Flow, CLIP, 30);
		Qk_Copy_Accessor(Box, Float, CLIP, 30);
		Qk_Copy_Accessor(Box, Image, CLIP, 30);
		Qk_Copy_Accessor(Box, Video, CLIP, 30);
		Qk_Copy_Accessor(Box, Input, CLIP, 30);
		Qk_Copy_Accessor(Box, Textarea, CLIP, 30);
		Qk_Copy_Accessor(Box, Scroll, CLIP, 30);
		Qk_Copy_Accessor(Box, Text, CLIP, 30);
		Qk_Copy_Accessor(Box, Button, CLIP, 30);
		Qk_Copy_Accessor(Box, Transform, CLIP, 30);
		Qk_Copy_Accessor(Box, Root, CLIP, 30);
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
		// scroll/textarea of ScrollLayoutBase
		Qk_Set_Accessor(Scroll, SCROLLBAR_COLOR, scrollbar_color);
		Qk_Set_Accessor(Scroll, SCROLLBAR_WIDTH, scrollbar_width);
		Qk_Set_Accessor(Scroll, SCROLLBAR_MARGIN, scrollbar_margin);
		Qk_Copy_Accessor(Scroll, Textarea, SCROLLBAR_COLOR, 3);
		// transform
		Qk_Set_Accessor(Transform, X, x);
		Qk_Set_Accessor(Transform, Y, y);
		Qk_Set_Accessor(Transform, SCALE_X, scale_x);
		Qk_Set_Accessor(Transform, SCALE_Y, scale_y);
		Qk_Set_Accessor(Transform, SKEW_X, skew_x);
		Qk_Set_Accessor(Transform, SKEW_Y, skew_y);
		Qk_Set_Accessor(Transform, ROTATE_Z, rotate_z);
		Qk_Set_Accessor(Transform, ORIGIN_X, origin_x);
		Qk_Set_Accessor(Transform, ORIGIN_Y, origin_y);
	}

	PropAccessor* prop_accessor_at_layout(ViewType type, ViewProp prop) {
		return prop_accessors[type].value + prop;
	}

}
