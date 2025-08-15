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
#include "./view/sprite.h"

namespace qk {
	typedef View ViewView;
	typedef void (Object::*Func)();

	#define Qk_Set_Accessor(View, Prop, Name) \
		props_accessors[k##View##_ViewType].value[k##Prop##_ViewProp] =\
			{(Func)&View::Name,(Func)&View::set_##Name};

	#define Qk_Copy_Accessor(From, Dest, Index, Count) \
		props_accessors[k##From##_ViewType].copy(k##Index##_ViewProp, Count, props_accessors[k##Dest##_ViewType])

	struct PropAccessors {
		void copy(uint32_t index, uint32_t count, PropAccessors &dest) {
			auto end = index + count;
			do {
				dest.value[index] = value[index];
			} while (++index < end);
		}
		PropAccessor value[kEnum_Counts_ViewProp] = {0};
	};

	static PropAccessors *props_accessors = nullptr;

	void view_prop_acc_init() {
		if (props_accessors)
			return;
		props_accessors = new PropAccessors[kEnum_Counts_ViewType];

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

		#define _Func_ScrollView_Props(_Func) \
			_Func(Color, scrollbar_color) \
			_Func(float, scrollbar_width) \
			_Func(float, scrollbar_margin) \

		#define _Func_MatrixView_Props(_Func) \
			_Func(Vec2, translate) \
			_Func(Vec2, scale) \
			_Func(Vec2, skew) \
			_Func(ArrayOrigin, origin) \
			_Func(BoxOrigin, origin_x) \
			_Func(BoxOrigin, origin_y) \
			_Func(float, x) \
			_Func(float, y) \
			_Func(float, scale_x) \
			_Func(float, scale_y) \
			_Func(float, skew_x) \
			_Func(float, skew_y) \
			_Func(float, rotate_z) \

		struct Text: View {
			#define _Func(Type, Name) \
				Type Name() { return asTextOptions()->Name(); } \
				void set_##Name(Type v, bool isRt) { asTextOptions()->set_##Name(v, isRt); }
			_Func_TextOptions_Props(_Func)
			#undef _Func
			#undef _Func_TextOptions_Props
		};

		struct Scroll: View {
			#define _Func(Type, Name) \
				Type Name() { return asScrollView()->Name(); } \
				void set_##Name(Type v, bool isRt) { asScrollView()->set_##Name(v, isRt); }
			_Func_ScrollView_Props(_Func)
			#undef _Func
			#undef _Func_ScrollView_Props
		};

		struct Matrix: qk::Matrix {
			#define _Func(Type, Name) \
				Type Name() { return asMatrixView()->Name(); } \
				void set_##Name(Type v, bool isRt) { asMatrixView()->set_##Name(v, isRt); }
			_Func_MatrixView_Props(_Func)
			#undef _Func
			#undef _Func_MatrixView_Props
		};

		struct Sprite: qk::Sprite {
			BoxSize width() const { return BoxSize{qk::Sprite::width(), BoxSizeKind::Value}; }
			void set_width(BoxSize v, bool isRt) { qk::Sprite::set_width(v.value, isRt); }
			BoxSize height() const { return BoxSize{qk::Sprite::height(), BoxSizeKind::Value}; }
			void set_height(BoxSize v, bool isRt) { qk::Sprite::set_height(v.value, isRt); }
		};

		// View
		Qk_Set_Accessor(View, OPACITY, opacity);
		Qk_Set_Accessor(View, CURSOR, cursor);
		Qk_Set_Accessor(View, VISIBLE, visible);
		Qk_Set_Accessor(View, RECEIVE, receive);
		props_accessors[kLabel_ViewType] = props_accessors[kView_ViewType]; // copy view props to label
		props_accessors[kBox_ViewType] = props_accessors[kView_ViewType]; // copy view props to box
		props_accessors[kFlex_ViewType] = props_accessors[kView_ViewType]; // copy view props to flex
		props_accessors[kFlow_ViewType] = props_accessors[kView_ViewType]; // copy view props to flow
		props_accessors[kFree_ViewType] = props_accessors[kView_ViewType]; // copy view props to free
		props_accessors[kImage_ViewType] = props_accessors[kView_ViewType]; // copy view props to image
		props_accessors[kVideo_ViewType] = props_accessors[kView_ViewType]; // copy view props to video
		props_accessors[kInput_ViewType] = props_accessors[kView_ViewType]; // copy view props to input
		props_accessors[kTextarea_ViewType] = props_accessors[kView_ViewType]; // copy view props to textarea
		props_accessors[kScroll_ViewType] = props_accessors[kView_ViewType]; // copy view props to scroll
		props_accessors[kText_ViewType] = props_accessors[kView_ViewType]; // copy view props to text
		props_accessors[kButton_ViewType] = props_accessors[kView_ViewType]; // copy view props to button
		props_accessors[kMatrix_ViewType] = props_accessors[kView_ViewType]; // copy view props to matrix
		props_accessors[kSprite_ViewType] = props_accessors[kView_ViewType]; // copy view props to sprite
		props_accessors[kRoot_ViewType] = props_accessors[kView_ViewType]; // copy view props to root
		// Box
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
		Qk_Set_Accessor(Box, PADDING, padding); // padding
		Qk_Set_Accessor(Box, PADDING_TOP, padding_top);
		Qk_Set_Accessor(Box, PADDING_RIGHT, padding_right);
		Qk_Set_Accessor(Box, PADDING_BOTTOM, padding_bottom);
		Qk_Set_Accessor(Box, PADDING_LEFT, padding_left);
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
		Qk_Copy_Accessor(Box, Flex, CLIP, 42); // copy box props to flex
		Qk_Copy_Accessor(Box, Flow, CLIP, 42); // copy box props to flow
		Qk_Copy_Accessor(Box, Free, CLIP, 42); // copy box props to free
		Qk_Copy_Accessor(Box, Image, CLIP, 42); // copy box props to image
		Qk_Copy_Accessor(Box, Video, CLIP, 42); // copy box props to video
		Qk_Copy_Accessor(Box, Input, CLIP, 42); // copy box props to input
		Qk_Copy_Accessor(Box, Textarea, CLIP, 42); // copy box props to textarea
		Qk_Copy_Accessor(Box, Scroll, CLIP, 42); // copy box props to scroll
		Qk_Copy_Accessor(Box, Text, CLIP, 42); // copy box props to text
		Qk_Copy_Accessor(Box, Button, CLIP, 42); // copy box props to button
		Qk_Copy_Accessor(Box, Matrix, CLIP, 42); // copy box props to matrix
		Qk_Copy_Accessor(Box, Root, CLIP, 42); // copy box props to root
		// Flex
		Qk_Set_Accessor(Flex, DIRECTION, direction);
		Qk_Set_Accessor(Flex, ITEMS_ALIGN, items_align);
		Qk_Set_Accessor(Flex, CROSS_ALIGN, cross_align);
		Qk_Copy_Accessor(Flex, Flow, DIRECTION, 3); // copy flex props to flow
		// Flow
		Qk_Set_Accessor(Flow, WRAP, wrap);
		Qk_Set_Accessor(Flow, WRAP_ALIGN, wrap_align);
		// Image
		Qk_Set_Accessor(Image, SRC, src);
		Qk_Copy_Accessor(Image, Video, SRC, 1); // copy image props to video
		// Text/Input/Label of TextOptions
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
		Qk_Copy_Accessor(Text, Input, TEXT_ALIGN, 13); // copy text props to input
		Qk_Copy_Accessor(Text, Textarea, TEXT_ALIGN, 13); // copy text props to textarea
		Qk_Copy_Accessor(Text, Label, TEXT_ALIGN, 13); // copy text props to label
		Qk_Copy_Accessor(Text, Button, TEXT_ALIGN, 13); // copy text props to button
		// Input/Textarea
		Qk_Set_Accessor(Input, SECURITY, security);
		Qk_Set_Accessor(Input, READONLY, readonly);
		Qk_Set_Accessor(Input, KEYBOARD_TYPE, type);
		Qk_Set_Accessor(Input, KEYBOARD_RETURN_TYPE, return_type);
		Qk_Set_Accessor(Input, PLACEHOLDER_COLOR, placeholder_color);
		Qk_Set_Accessor(Input, CURSOR_COLOR, cursor_color);
		Qk_Set_Accessor(Input, MAX_LENGTH, max_length);
		Qk_Set_Accessor(Input, PLACEHOLDER, placeholder);
		Qk_Copy_Accessor(Input, Textarea, SECURITY, 8); // copy input props to textarea
		// Scroll/Textarea of ScrollView
		Qk_Set_Accessor(Scroll, SCROLLBAR_COLOR, scrollbar_color);
		Qk_Set_Accessor(Scroll, SCROLLBAR_WIDTH, scrollbar_width);
		Qk_Set_Accessor(Scroll, SCROLLBAR_MARGIN, scrollbar_margin);
		Qk_Copy_Accessor(Scroll, Textarea, SCROLLBAR_COLOR, 3); // copy scroll props to textarea
		// Matrix/Sprite of MatrixView
		Qk_Set_Accessor(Matrix, TRANSLATE, translate);
		Qk_Set_Accessor(Matrix, SCALE, scale);
		Qk_Set_Accessor(Matrix, SKEW, skew);
		Qk_Set_Accessor(Matrix, ORIGIN, origin);
		Qk_Set_Accessor(Matrix, ORIGIN_X, origin_x);
		Qk_Set_Accessor(Matrix, ORIGIN_Y, origin_y);
		Qk_Set_Accessor(Matrix, X, x);
		Qk_Set_Accessor(Matrix, Y, y);
		Qk_Set_Accessor(Matrix, SCALE_X, scale_x);
		Qk_Set_Accessor(Matrix, SCALE_Y, scale_y);
		Qk_Set_Accessor(Matrix, SKEW_X, skew_x);
		Qk_Set_Accessor(Matrix, SKEW_Y, skew_y);
		Qk_Set_Accessor(Matrix, ROTATE_Z, rotate_z);
		// Sprite
		Qk_Copy_Accessor(Matrix, Sprite, TRANSLATE, 13); // copy matrix props to sprite
		Qk_Set_Accessor(Sprite, SRC, src);
		Qk_Set_Accessor(Sprite, WIDTH, width);
		Qk_Set_Accessor(Sprite, HEIGHT, height);
		Qk_Set_Accessor(Sprite, DIRECTION, direction);
	}

	PropAccessor* get_props_accessor(ViewType type, ViewProp prop) {
		return props_accessors[type].value + prop;
	}
}
