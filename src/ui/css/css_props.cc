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

#include "./css_props.h"
#include "../view/view.h"
#include "../view/box.h"
#include "../view/flex.h"
#include "../view/flow.h"
#include "../view/text.h"
#include "../view/input.h"
#include "../view/image.h"
#include "../view/scroll.h"
#include "../view/matrix.h"
#include "../view/sprite.h"

namespace qk {
	typedef void (Object::*Func)();

	#define __Qk_Modifier_ const
	#define __Qk_Modifier_NoConst
	#define Qk_Set_Accessor(View, Prop, Name, Type, ...) \
		accessors[k##View##_ViewType].value[k##Prop##_CssProp] =\
			{(Func)static_cast<Type (View::*)()__Qk_Modifier_##__VA_ARGS__>(&View::Name),\
				(Func)static_cast<void (View::*)(Type,bool)>(&View::set_##Name)};

	#define Qk_Copy_Accessor(From, Dest, Index, Count) \
		accessors[k##From##_ViewType].copy(k##Index##_CssProp, Count, accessors[k##Dest##_ViewType])

	struct Accessors {
		void copy(uint32_t index, uint32_t count, Accessors &dest) {
			auto end = index + count;
			do {
				dest.value[index] = value[index];
			} while (++index < end);
		}
		CssPropAccessor value[kEnum_Counts_CssProp] = {0};
	};

	static Accessors *accessors = nullptr;

	void view_prop_acc_init() {
		if (accessors)
			return;
		accessors = new Accessors[kEnum_Counts_ViewType];

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
				Type Name() const { return const_cast<Text*>(this)->asTextOptions()->Name(); } \
				void set_##Name(Type v, bool isRt) { asTextOptions()->set_##Name(v, isRt); }
			_Func_TextOptions_Props(_Func)
			#undef _Func
			#undef _Func_TextOptions_Props
		};

		struct Scroll: View {
			#define _Func(Type, Name) \
				Type Name() const { return const_cast<Scroll*>(this)->asScrollView()->Name(); } \
				void set_##Name(Type v, bool isRt) { asScrollView()->set_##Name(v, isRt); }
			_Func_ScrollView_Props(_Func)
			#undef _Func
			#undef _Func_ScrollView_Props
		};

		struct Matrix: qk::Matrix {
			#define _Func(Type, Name) \
				Type Name() const { return const_cast<Matrix*>(this)->asMatrixView()->Name(); } \
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
		Qk_Set_Accessor(View, OPACITY, opacity, float);
		Qk_Set_Accessor(View, CURSOR, cursor, CursorStyle);
		Qk_Set_Accessor(View, VISIBLE, visible, bool);
		Qk_Set_Accessor(View, RECEIVE, receive, bool);
		accessors[kLabel_ViewType] = accessors[kView_ViewType]; // copy view props to label
		accessors[kBox_ViewType] = accessors[kView_ViewType]; // copy view props to box
		accessors[kFlex_ViewType] = accessors[kView_ViewType]; // copy view props to flex
		accessors[kFlow_ViewType] = accessors[kView_ViewType]; // copy view props to flow
		accessors[kFree_ViewType] = accessors[kView_ViewType]; // copy view props to free
		accessors[kImage_ViewType] = accessors[kView_ViewType]; // copy view props to image
		accessors[kVideo_ViewType] = accessors[kView_ViewType]; // copy view props to video
		accessors[kInput_ViewType] = accessors[kView_ViewType]; // copy view props to input
		accessors[kTextarea_ViewType] = accessors[kView_ViewType]; // copy view props to textarea
		accessors[kScroll_ViewType] = accessors[kView_ViewType]; // copy view props to scroll
		accessors[kText_ViewType] = accessors[kView_ViewType]; // copy view props to text
		accessors[kButton_ViewType] = accessors[kView_ViewType]; // copy view props to button
		accessors[kMatrix_ViewType] = accessors[kView_ViewType]; // copy view props to matrix
		accessors[kSprite_ViewType] = accessors[kView_ViewType]; // copy view props to sprite
		accessors[kSpine_ViewType] = accessors[kView_ViewType]; // copy view props to spine
		accessors[kRoot_ViewType] = accessors[kView_ViewType]; // copy view props to root
		// Box
		Qk_Set_Accessor(Box, CLIP, clip, bool);
		Qk_Set_Accessor(Box, ALIGN, align, Align);
		Qk_Set_Accessor(Box, WIDTH, width, BoxSize);
		Qk_Set_Accessor(Box, HEIGHT, height, BoxSize);
		Qk_Set_Accessor(Box, MIN_WIDTH, min_width, BoxSize);
		Qk_Set_Accessor(Box, MIN_HEIGHT, min_height, BoxSize);
		Qk_Set_Accessor(Box, MAX_WIDTH, max_width, BoxSize);
		Qk_Set_Accessor(Box, MAX_HEIGHT, max_height, BoxSize);
		Qk_Set_Accessor(Box, MARGIN, margin, ArrayFloat); // margin
		Qk_Set_Accessor(Box, MARGIN_TOP, margin_top, float);
		Qk_Set_Accessor(Box, MARGIN_RIGHT, margin_right, float);
		Qk_Set_Accessor(Box, MARGIN_BOTTOM, margin_bottom, float);
		Qk_Set_Accessor(Box, MARGIN_LEFT, margin_left, float);
		Qk_Set_Accessor(Box, PADDING, padding, ArrayFloat); // padding
		Qk_Set_Accessor(Box, PADDING_TOP, padding_top, float);
		Qk_Set_Accessor(Box, PADDING_RIGHT, padding_right, float);
		Qk_Set_Accessor(Box, PADDING_BOTTOM, padding_bottom, float);
		Qk_Set_Accessor(Box, PADDING_LEFT, padding_left, float);
		Qk_Set_Accessor(Box, BORDER_RADIUS, border_radius, ArrayFloat); // border radius
		Qk_Set_Accessor(Box, BORDER_RADIUS_LEFT_TOP, border_radius_left_top, float);
		Qk_Set_Accessor(Box, BORDER_RADIUS_RIGHT_TOP, border_radius_right_top, float);
		Qk_Set_Accessor(Box, BORDER_RADIUS_RIGHT_BOTTOM, border_radius_right_bottom, float);
		Qk_Set_Accessor(Box, BORDER_RADIUS_LEFT_BOTTOM, border_radius_left_bottom, float);
		Qk_Set_Accessor(Box, BORDER, border, ArrayBorder); // border
		Qk_Set_Accessor(Box, BORDER_WIDTH, border_width, ArrayFloat);
		Qk_Set_Accessor(Box, BORDER_COLOR, border_color, ArrayColor);
		Qk_Set_Accessor(Box, BORDER_TOP, border_top, BoxBorder);
		Qk_Set_Accessor(Box, BORDER_RIGHT, border_right, BoxBorder);
		Qk_Set_Accessor(Box, BORDER_BOTTOM, border_bottom, BoxBorder);
		Qk_Set_Accessor(Box, BORDER_LEFT, border_left, BoxBorder);
		Qk_Set_Accessor(Box, BORDER_WIDTH_TOP, border_width_top, float); // border width
		Qk_Set_Accessor(Box, BORDER_WIDTH_RIGHT, border_width_right, float);
		Qk_Set_Accessor(Box, BORDER_WIDTH_BOTTOM, border_width_bottom, float);
		Qk_Set_Accessor(Box, BORDER_WIDTH_LEFT, border_width_left, float);
		Qk_Set_Accessor(Box, BORDER_COLOR_TOP, border_color_top, Color); // border color
		Qk_Set_Accessor(Box, BORDER_COLOR_RIGHT, border_color_right, Color);
		Qk_Set_Accessor(Box, BORDER_COLOR_BOTTOM, border_color_bottom, Color);
		Qk_Set_Accessor(Box, BORDER_COLOR_LEFT, border_color_left, Color); // border end
		Qk_Set_Accessor(Box, BACKGROUND_COLOR, background_color, Color);
		Qk_Set_Accessor(Box, BACKGROUND, background, BoxFilterPtr, NoConst);
		Qk_Set_Accessor(Box, BOX_SHADOW, box_shadow, BoxShadowPtr, NoConst);
		Qk_Set_Accessor(Box, WEIGHT, weight, Vec2);
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
		Qk_Set_Accessor(Flex, DIRECTION, direction, Direction);
		Qk_Set_Accessor(Flex, ITEMS_ALIGN, items_align, ItemsAlign);
		Qk_Set_Accessor(Flex, CROSS_ALIGN, cross_align, CrossAlign);
		Qk_Copy_Accessor(Flex, Flow, DIRECTION, 3); // copy flex props to flow
		// Flow
		Qk_Set_Accessor(Flow, WRAP, wrap, Wrap);
		Qk_Set_Accessor(Flow, WRAP_ALIGN, wrap_align, WrapAlign);
		// Image
		Qk_Set_Accessor(Image, SRC, src, String);
		Qk_Copy_Accessor(Image, Video, SRC, 1); // copy image props to video
		// Text/Input/Label of TextOptions
		Qk_Set_Accessor(Text, TEXT_ALIGN, text_align, TextAlign);
		Qk_Set_Accessor(Text, TEXT_WEIGHT, text_weight, TextWeight);
		Qk_Set_Accessor(Text, TEXT_SLANT, text_slant, TextSlant);
		Qk_Set_Accessor(Text, TEXT_DECORATION, text_decoration, TextDecoration);
		Qk_Set_Accessor(Text, TEXT_OVERFLOW, text_overflow, TextOverflow);
		Qk_Set_Accessor(Text, TEXT_WHITE_SPACE, text_white_space, TextWhiteSpace);
		Qk_Set_Accessor(Text, TEXT_WORD_BREAK, text_word_break, TextWordBreak);
		Qk_Set_Accessor(Text, TEXT_SIZE, text_size, TextSize);
		Qk_Set_Accessor(Text, TEXT_BACKGROUND_COLOR, text_background_color, TextColor);
		Qk_Set_Accessor(Text, TEXT_COLOR, text_color, TextColor);
		Qk_Set_Accessor(Text, TEXT_LINE_HEIGHT, text_line_height, TextLineHeight);
		Qk_Set_Accessor(Text, TEXT_SHADOW, text_shadow, TextShadow);
		Qk_Set_Accessor(Text, TEXT_FAMILY, text_family, TextFamily);
		Qk_Copy_Accessor(Text, Input, TEXT_ALIGN, 13); // copy text props to input
		Qk_Copy_Accessor(Text, Textarea, TEXT_ALIGN, 13); // copy text props to textarea
		Qk_Copy_Accessor(Text, Label, TEXT_ALIGN, 13); // copy text props to label
		Qk_Copy_Accessor(Text, Button, TEXT_ALIGN, 13); // copy text props to button
		// Input/Textarea
		Qk_Set_Accessor(Input, SECURITY, security, bool);
		Qk_Set_Accessor(Input, READONLY, readonly, bool);
		Qk_Set_Accessor(Input, KEYBOARD_TYPE, type, KeyboardType);
		Qk_Set_Accessor(Input, KEYBOARD_RETURN_TYPE, return_type, KeyboardReturnType);
		Qk_Set_Accessor(Input, PLACEHOLDER_COLOR, placeholder_color, Color);
		Qk_Set_Accessor(Input, CURSOR_COLOR, cursor_color, Color);
		Qk_Set_Accessor(Input, MAX_LENGTH, max_length, uint32_t);
		Qk_Set_Accessor(Input, PLACEHOLDER, placeholder, String);
		Qk_Copy_Accessor(Input, Textarea, SECURITY, 8); // copy input props to textarea
		// Scroll/Textarea of ScrollView
		Qk_Set_Accessor(Scroll, SCROLLBAR_COLOR, scrollbar_color, Color);
		Qk_Set_Accessor(Scroll, SCROLLBAR_WIDTH, scrollbar_width, float);
		Qk_Set_Accessor(Scroll, SCROLLBAR_MARGIN, scrollbar_margin, float);
		Qk_Copy_Accessor(Scroll, Textarea, SCROLLBAR_COLOR, 3); // copy scroll props to textarea
		// Matrix/Sprite of MatrixView
		Qk_Set_Accessor(Matrix, TRANSLATE, translate, Vec2);
		Qk_Set_Accessor(Matrix, SCALE, scale, Vec2);
		Qk_Set_Accessor(Matrix, SKEW, skew, Vec2);
		Qk_Set_Accessor(Matrix, ORIGIN, origin, ArrayOrigin);
		Qk_Set_Accessor(Matrix, ORIGIN_X, origin_x, BoxOrigin);
		Qk_Set_Accessor(Matrix, ORIGIN_Y, origin_y, BoxOrigin);
		Qk_Set_Accessor(Matrix, X, x, float);
		Qk_Set_Accessor(Matrix, Y, y, float);
		Qk_Set_Accessor(Matrix, SCALE_X, scale_x, float);
		Qk_Set_Accessor(Matrix, SCALE_Y, scale_y, float);
		Qk_Set_Accessor(Matrix, SKEW_X, skew_x, float);
		Qk_Set_Accessor(Matrix, SKEW_Y, skew_y, float);
		Qk_Set_Accessor(Matrix, ROTATE_Z, rotate_z, float);
		Qk_Copy_Accessor(Matrix, Sprite, TRANSLATE, 13); // copy matrix props to sprite
		Qk_Copy_Accessor(Matrix, Spine, TRANSLATE, 13); // copy matrix props to spine
		Qk_Copy_Accessor(Matrix, Root, TRANSLATE, 13); // copy matrix props to root
		// Sprite
		Qk_Set_Accessor(Sprite, SRC, src, String);
		Qk_Set_Accessor(Sprite, WIDTH, width, BoxSize);
		Qk_Set_Accessor(Sprite, HEIGHT, height, BoxSize);
		Qk_Set_Accessor(Sprite, FRAME, frame, uint16_t);
		Qk_Set_Accessor(Sprite, DIRECTION, direction, Direction);
	}

	CssPropAccessor* get_props_accessor(ViewType type, CssProp prop) {
		return accessors[type].value + prop;
	}
}
