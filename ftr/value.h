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

#ifndef __ftr__value__
#define __ftr__value__

#include "./util/util.h"
#include "./math/math.h"

namespace ftr {

	class FontFamilysID;

	namespace value {

		#define FX_ENUM_VALUE(F) \
			F(AUTO,               auto) \
			F(FULL,               full) \
			F(PIXEL,              pixel) \
			F(PERCENT,            percent) \
			F(MINUS,              minus) \
			F(INHERIT,            inherit) \
			F(VALUE,              value) \
			F(THIN,               thin)              /*100*/ \
			F(ULTRALIGHT,         ultralight)        /*200*/ \
			F(LIGHT,              light)             /*300*/ \
			F(REGULAR,            regular)           /*400*/ \
			F(MEDIUM,             medium)            /*500*/ \
			F(SEMIBOLD,           semibold)          /*600*/ \
			F(BOLD,               bold)              /*700*/ \
			F(HEAVY,              heavy)             /*800*/ \
			F(BLACK,              black)             /*900*/ \
			F(THIN_ITALIC,        thin_italic)       /*100*/ \
			F(ULTRALIGHT_ITALIC,  ultralight_italic) /*200*/ \
			F(LIGHT_ITALIC,       light_italic)      /*300*/ \
			F(ITALIC,             italic)            /*400*/ \
			F(MEDIUM_ITALIC,      medium_italic)     /*500*/ \
			F(SEMIBOLD_ITALIC,    semibold_italic)   /*600*/ \
			F(BOLD_ITALIC,        bold_italic)       /*700*/ \
			F(HEAVY_ITALIC,       heavy_italic)      /*800*/ \
			F(BLACK_ITALIC,       black_italic)      /*900*/ \
			F(OTHER,              other) \
			F(NONE,               none) \
			F(OVERLINE,           overline) \
			F(LINE_THROUGH,       line_through) \
			F(UNDERLINE,          underline) \
			F(LEFT,               left) \
			F(CENTER,             center) \
			F(RIGHT,              right) \
			F(LEFT_REVERSE,       left_reverse) \
			F(CENTER_REVERSE,     center_reverse) \
			F(RIGHT_REVERSE,      right_reverse) \
			F(TOP,                top) \
			F(BOTTOM,             bottom) \
			F(MIDDLE,             middle) \
			F(REPEAT,             repeat) \
			F(REPEAT_X,           repeat_x) \
			F(REPEAT_Y,           repeat_y) \
			F(MIRRORED_REPEAT,    mirrored_repeat) \
			F(MIRRORED_REPEAT_X,  mirrored_repeat_x) \
			F(MIRRORED_REPEAT_Y,  mirrored_repeat_y) \
			F(NORMAL,             normal) \
			F(CLIP,               clip) \
			F(ELLIPSIS,           ellipsis) \
			F(CENTER_ELLIPSIS,    center_ellipsis) \
			F(NO_WRAP,            no_wrap) \
			F(NO_SPACE,           no_space) \
			F(PRE,                pre) \
			F(PRE_LINE,           pre_line) \
			F(WRAP,               wrap) \
			F(ASCII,              ascii) \
			F(NUMBER,             number) \
			F(URL,                url) \
			F(NUMBER_PAD,         number_pad) \
			F(PHONE,              phone) \
			F(NAME_PHONE,         name_phone) \
			F(EMAIL,              email) \
			F(DECIMAL,            decimal) \
			F(TWITTER,            twitter) \
			F(SEARCH,             search) \
			F(ASCII_NUMBER,       ascii_numner) \
			F(GO,                 go) \
			F(JOIN,               join) \
			F(NEXT,               next) \
			F(ROUTE,              route) \
			F(SEND,               send) \
			F(DONE,               done) \
			F(EMERGENCY,          emergency) \
			F(CONTINUE,           continue) \
			F(ROW,                row) \
			F(ROW_REVERSE,        row_reverse) \
			F(COLUMN,             column) \
			F(COLUMN_REVERSE,     column_reverse) \
			F(WRAP_REVERSE,       wrap_reverse) \
		
		enum Enum {
			#define DEF_ENUM_VALUE(NAME, NAME2) NAME,
			FX_ENUM_VALUE(DEF_ENUM_VALUE)
			#undef DEF_ENUM_VALUE
		};

		/**
		 * @enum KeyboardType
		 */
		enum class KeyboardType: uint16_t {
			NORMAL = value::NORMAL,
			ASCII = value::ASCII,
			NUMBER = value::NUMBER,
			URL = value::URL,
			NUMBER_PAD = value::NUMBER_PAD,
			PHONE = value::PHONE,
			NAME_PHONE = value::NAME_PHONE,
			EMAIL = value::EMAIL,
			DECIMAL = value::DECIMAL,
			TWITTER = value::TWITTER,
			SEARCH = value::SEARCH,
			ASCII_NUMBER = value::ASCII_NUMBER,
		};

		/**
		* @enum KeyboardReturnType
		*/		
		enum class KeyboardReturnType: uint16_t {
			NORMAL = value::NORMAL,
			GO = value::GO,
			JOIN = value::JOIN,
			NEXT = value::NEXT,
			ROUTE = value::ROUTE,
			SEARCH = value::SEARCH,
			SEND = value::SEND,
			DONE = value::DONE,
			EMERGENCY = value::EMERGENCY,
			CONTINUE = value::CONTINUE,
		};

		/**
		* @enum Direction
		*/
		enum class Direction: uint16_t {
			NONE = value::NONE,
			LEFT = value::LEFT,
			RIGHT = value::RIGHT,
			TOP = value::TOP,
			BOTTOM = value::BOTTOM,
		};

		/**
		* @enum ValueType
		*/
		enum class ValueType: uint16_t {
			AUTO = value::AUTO,    /* 自动值  auto */
			FULL = value::FULL,    /* 吸附到父视图(client边距与父视图重叠) full */
			PIXEL = value::PIXEL,   /* 像素值  px */
			PERCENT = value::PERCENT, /* 百分比  % */
			MINUS = value::MINUS,   /* 减法(parent-value) ! */
		};

		/**
		* @enum BackgroundPositionType
		*/
		enum class BackgroundPositionType: uint16_t {
			PIXEL = value::PIXEL,     /* 像素值  px */
			PERCENT = value::PERCENT,   /* 百分比  % */
			LEFT = value::LEFT,      /* 居左 */
			RIGHT = value::RIGHT,     /* 居右  % */
			CENTER = value::CENTER,    /* 居中 */
			TOP = value::TOP,       /* 居上 */
			BOTTOM = value::BOTTOM,    /* 居下 */
		};
		
		/**
		* @enum BackgroundSizeType
		*/
		enum class BackgroundSizeType: uint16_t {
			AUTO = value::AUTO,      /* 自动值  auto */
			PIXEL = value::PIXEL,     /* 像素值  px */
			PERCENT = value::PERCENT,   /* 百分比  % */
		};
		
		/**
		* @enum TextValueType
		*/
		enum class TextValueType: uint16_t {
			INHERIT = value::INHERIT,
			VALUE = value::VALUE,
		};

		/**
		* @enum TextStyleEnum
		*/
		enum class TextStyleEnum: uint16_t {
			THIN = value::THIN,
			ULTRALIGHT = value::ULTRALIGHT,
			LIGHT = value::LIGHT,
			REGULAR = value::REGULAR,
			MEDIUM = value::MEDIUM,
			SEMIBOLD = value::SEMIBOLD,
			BOLD = value::BOLD,
			HEAVY = value::HEAVY,
			BLACK = value::BLACK,
			THIN_ITALIC = value::THIN_ITALIC,
			ULTRALIGHT_ITALIC = value::ULTRALIGHT_ITALIC,
			LIGHT_ITALIC = value::LIGHT_ITALIC,
			ITALIC = value::ITALIC,
			MEDIUM_ITALIC = value::MEDIUM_ITALIC,
			SEMIBOLD_ITALIC = value::SEMIBOLD_ITALIC,
			BOLD_ITALIC = value::BOLD_ITALIC,
			HEAVY_ITALIC = value::HEAVY_ITALIC,
			BLACK_ITALIC = value::BLACK_ITALIC,
			OTHER = value::OTHER,
		};

		/**
		* @enum TextDecorationEnum
		*/
		enum class TextDecorationEnum: uint16_t {
			NONE = value::NONE,           /* 没有 */
			OVERLINE = value::OVERLINE,       /* 上划线 */
			LINE_THROUGH = value::LINE_THROUGH,   /* 中划线 */
			UNDERLINE = value::UNDERLINE,      /* 下划线 */
		};

		enum class TextOverflowEnum: uint16_t {
			NORMAL = value::NORMAL,          /* 不做任何处理 */
			CLIP = value::CLIP,            /* 剪切 */
			ELLIPSIS = value::ELLIPSIS,        /* 剪切并显示省略号 */
			CENTER_ELLIPSIS = value::CENTER_ELLIPSIS, /* 剪切并居中显示省略号 */
		};
		
		enum class TextWhiteSpaceEnum: uint16_t {
			NORMAL = value::NORMAL,           /* 保留所有空白,使用自动wrap */
			NO_WRAP = value::NO_WRAP,          /* 合并空白序列,不使用自动wrap */
			NO_SPACE = value::NO_SPACE,         /* 合并空白序列,使用自动wrap */
			PRE = value::PRE,              /* 保留所有空白,不使用自动wrap */
			PRE_LINE = value::PRE_LINE,         /* 合并空白符序列,但保留换行符,使用自动wrap */
			WRAP = value::WRAP,             /* 保留所有空白,强制使用自动wrap */
		};

		/**
		* @enum TextAlign 文本对齐方式
		*/
		enum class TextAlign: uint16_t {
			LEFT = value::LEFT,           /* 左对齐 */
			CENTER = value::CENTER,         /* 居中 */
			RIGHT = value::RIGHT,          /* 右对齐 */
			LEFT_REVERSE = value::LEFT_REVERSE,   /* 左对齐并反向 */
			CENTER_REVERSE = value::CENTER_REVERSE, /* 居中对齐并反向 */
			RIGHT_REVERSE = value::RIGHT_REVERSE,  /* 右对齐并反向 */
		};

		/**
		* @enum LayoutAlign 对齐方式
		*/
		enum class LayoutAlign: uint16_t {
			LEFT = value::LEFT,
			RIGHT = value::RIGHT,
			CENTER = value::CENTER,
			TOP = value::TOP,
			BOTTOM = value::BOTTOM,
		};

		/**
		* @enum ContentAlign div 内容对齐方式
		*/
		enum class ContentAlign: uint16_t {
			LEFT = value::LEFT,    /* 水平左对齐 */
			RIGHT = value::RIGHT,   /* 水平右对齐 */
			TOP = value::TOP,     /* 垂直上对齐 */
			BOTTOM = value::BOTTOM,  /* 垂直下对齐 */
		};

		/**
		* @enum Repeat 纹理重复方式
		*/
		enum class Repeat: uint16_t {
			NONE = Enum::NONE,
			REPEAT = Enum::REPEAT,
			REPEAT_X = Enum::REPEAT_X,
			REPEAT_Y = Enum::REPEAT_Y,
			MIRRORED_REPEAT = Enum::MIRRORED_REPEAT,
			MIRRORED_REPEAT_X = Enum::MIRRORED_REPEAT_X,
			MIRRORED_REPEAT_Y = Enum::MIRRORED_REPEAT_Y,
		};
		
		struct Rect {
			Vec2 origin;
			Vec2  size;
		};
		
		struct Region {
			float x, y;
			float x2, y2;
			float w, h;
		};
		
		/**
		* @struct Border
		*/
		struct FX_EXPORT Border {
			float width;
			Color color;
			inline Border(float w = 0,
										Color c = Color())
			: width(w), color(c) {}
		};
		
		/**
		* @struct Shadow
		*/
		struct FX_EXPORT Shadow {
			float   offset_x;
			float   offset_y;
			float   size;
			Color   color;
			bool operator==(const Shadow&) const;
			inline bool operator!=(const Shadow& value) const { return ! operator==(value); }
		};
		
		// Compound value
		
		/**
		* @struct ValueTemplate
		*/
		template<typename Type, Type TypeInit, typename Value = float>
		struct FX_EXPORT ValueTemplate {
			Type type;
			Value value;
			inline bool operator==(const ValueTemplate& val) const {
				return val.type == type && val.value == value;
			}
			inline bool operator!=(const ValueTemplate& value) const {
				return !operator==(value);
			}
			inline ValueTemplate(Type t = TypeInit, Value v = 0): type(t), value(v) {}
			inline ValueTemplate(Value v) : type(Type::PIXEL), value(v) {}
		};
		
		typedef ValueTemplate<ValueType, ValueType::AUTO> Value;
		typedef ValueTemplate<BackgroundPositionType, BackgroundPositionType::PIXEL> BackgroundPosition;
		typedef ValueTemplate<BackgroundSizeType, BackgroundSizeType::AUTO> BackgroundSize;
		
		struct BackgroundPositionCollection {
			BackgroundPosition x, y;
		};
		struct BackgroundSizeCollection {
			BackgroundSize x, y;
		};
		
		/**
		* @struct TextColor
		*/
		struct FX_EXPORT TextColor {
			TextValueType type;
			Color value;
		};
		
		/**
		* @struct TextSize
		*/
		struct FX_EXPORT TextSize {
			TextValueType type;
			float value;
		};
		
		/**
		* @struct TextFamily
		*/
		struct FX_EXPORT TextFamily {
			TextFamily(TextValueType type = TextValueType::INHERIT);
			TextFamily(TextValueType type, const FontFamilysID* cffid);
			TextValueType type;
			const FontFamilysID* value;
			cString& name() const;
			const Array<String>& names() const;
		};
		
		/**
		* @struct TextStyle
		*/
		struct FX_EXPORT TextStyle {
			TextValueType type;
			TextStyleEnum value;
		};
		
		/**
		* @struct TextShadow
		*/
		struct FX_EXPORT TextShadow {
			TextValueType type;
			Shadow  value;
		};
		
		/**
		* @struct TextLineHeightValue
		*/
		struct FX_EXPORT TextLineHeightValue {
			float height;
			inline void set_auto() { height = 0; }
			inline bool is_auto() const { return height <= 0; }
			inline bool operator!=(const TextLineHeightValue& value) const {
				return height != value.height;
			}
			inline bool operator==(const TextLineHeightValue& value) const {
				return height == value.height;
			}
		};
		
		/**
		* @struct TextLineHeight
		*/
		struct FX_EXPORT TextLineHeight {
			TextValueType  type;
			TextLineHeightValue value;
		};
		
		/**
		* @struct TextDecoration
		*/
		struct FX_EXPORT TextDecoration {
			TextValueType  type;
			TextDecorationEnum  value;
		};
		
		/**
		* @struct TextOverflow
		*/
		struct FX_EXPORT TextOverflow {
			TextValueType  type;
			TextOverflowEnum  value;
		};
		
		/**
		* @struct TextWhiteSpace
		*/
		struct FX_EXPORT TextWhiteSpace {
			TextValueType  type;
			TextWhiteSpaceEnum  value;
		};
		
	}

	using value::Direction;
	using value::ValueType;
	using value::BackgroundPositionType;
	using value::BackgroundSizeType;
	using value::TextValueType;
	using value::TextDecorationEnum;
	using value::TextOverflowEnum;
	using value::TextWhiteSpaceEnum;
	using value::TextAlign;
	using value::LayoutAlign;
	using value::ContentAlign;
	using value::Repeat;
	using value::Value;
	using value::BackgroundPosition;
	using value::BackgroundSize;
	using value::Border;
	using value::TextLineHeightValue;
	using value::TextStyleEnum;
	using value::Shadow;
	typedef value::Rect CGRect;
	using value::Region;
	using value::KeyboardType;
	using value::KeyboardReturnType;
	using value::TextColor;
	using value::TextSize;
	using value::TextStyle;
	using value::TextFamily;
	using value::TextLineHeight;
	using value::TextShadow;
	using value::TextDecoration;
	using value::TextOverflow;
	using value::TextWhiteSpace;

}
#endif
