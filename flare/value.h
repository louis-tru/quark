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

#ifndef __flare__value__
#define __flare__value__

#include "./util/util.h"
#include "./math.h"

namespace flare {

	namespace value {

		#define FX_Enum_Value(F) \
			\
			F(NONE,               none) \
			F(DEFAULT,            default) \
			F(INHERIT,            inherit) \
			F(VALUE,              value) \
			F(AUTO,               auto) \
			F(NORMAL,             normal) \
			\
			F(WRAP,               wrap) \
			F(MATCH,              match) \
			F(PIXEL,              pixel) \
			F(RATIO,              ratio) \
			F(MINUS,              minus) \
			\
			F(ROW,                row) \
			F(ROW_REVERSE,        row_reverse) \
			F(COLUMN,             column) \
			F(COLUMN_REVERSE,     column_reverse) \
			\
			F(LEFT,               left) \
			F(RIGHT,              right) \
			F(TOP,                top) \
			F(BOTTOM,             bottom) \
			\
			F(LEFT_TOP,           left_top) \
			F(CENTER_TOP,         center_top) \
			F(RIGHT_TOP,          right_top) \
			F(LEFT_CENTER,        left_center) \
			F(CENTER_CENTER,      center_center) \
			F(RIGHT_CENTER,       right_center) \
			F(LEFT_BOTTOM,        left_bottom) \
			F(CENTER_BOTTOM,      center_bottom) \
			F(RIGHT_BOTTOM,       right_bottom) \
			\
			F(LEFT_REVERSE,       left_reverse) \
			F(CENTER_REVERSE,     center_reverse) \
			F(RIGHT_REVERSE,      right_reverse) \
			\
			F(START,              start) \
			F(CENTER,             center) \
			F(END,                end) \
			F(BASELINE,           baseline) \
			F(STRETCH,            stretch) \
			F(SPACE_BETWEEN,      space_between) \
			F(SPACE_AROUND,       space_around) \
			F(SPACE_EVENLY,       space_evenly) \
			\
			F(MIDDLE,             middle) \
			F(REPEAT,             repeat) \
			F(REPEAT_X,           repeat_x) \
			F(REPEAT_Y,           repeat_y) \
			F(MIRRORED_REPEAT,    mirrored_repeat) \
			F(MIRRORED_REPEAT_X,  mirrored_repeat_x) \
			F(MIRRORED_REPEAT_Y,  mirrored_repeat_y) \
			\
			F(CLIP,               clip) \
			F(ELLIPSIS,           ellipsis) \
			F(CENTER_ELLIPSIS,    center_ellipsis) \
			\
			/* text white space */ \
			/*F(NORMAL,           normal)*/ \
			F(NO_WRAP,            no_wrap) \
			F(NO_SPACE,           no_space) \
			F(PRE,                pre) \
			F(PRE_LINE,           pre_line) \
			/*F(WRAP,               wrap)*/ \
			F(WRAP_REVERSE,       wrap_reverse) \
			\
			/* text weight */ \
			F(THIN,               thin)              /*100*/ \
			F(ULTRALIGHT,         ultralight)        /*200*/ \
			F(LIGHT,              light)             /*300*/ \
			F(REGULAR,            regular)           /*400*/ \
			F(MEDIUM,             medium)            /*500*/ \
			F(SEMIBOLD,           semibold)          /*600*/ \
			F(BOLD,               bold)              /*700*/ \
			F(HEAVY,              heavy)             /*800*/ \
			F(BLACK,              black)             /*900*/ \
			\
			/* text style */ \
			/*F(NORMAL,       normal)*/ \
			F(ITALIC,         italic) \
			F(OBLIQUE,        oblique) \
			\
			/* text decoration */ \
			F(OVERLINE,           overline) \
			F(LINE_THROUGH,       line_through) \
			F(UNDERLINE,          underline) \
			\
			/* soft keyboard style */\
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
			\
			/* enter key style in soft keyboard */ \
			F(GO,                 go) \
			F(JOIN,               join) \
			F(NEXT,               next) \
			F(ROUTE,              route) \
			F(SEND,               send) \
			F(DONE,               done) \
			F(EMERGENCY,          emergency) \
			F(CONTINUE,           continue) \

		enum Enum {
			# define FX_Value_Name(NAME, NAME2) NAME,
				FX_Enum_Value(FX_Value_Name)
			# undef FX_Value_Name
		};
	}

	template<typename Type, Type TypeInit, typename Value = float>
	struct TemplateValue {
		Value value;
		Type type;
		inline bool operator==(const TemplateValue& val) const {
			return val.type == type && val.value == value;
		}
		inline bool operator!=(const TemplateValue& val) const {
			return !operator==(val);
		}
		inline TemplateValue(Value v, Type t = TypeInit): value(v), type(t) {}
	};

	// rect
	struct Rect {
		Vec2 origin, size;
	};

	// react region
	struct Region {
		float x, y, x2, y2, w, h;
	};

	// rect
	struct Shadow {
		float offset_x, offset_y, size;
		Color color;
		inline bool operator==(const Shadow& val) const {
			return (
				val.offset_x == offset_x && val.offset_y == offset_y && 
				val.size == size && val.color == color);
		}
		inline bool operator!=(const Shadow& val) const {
			return ! operator==(val);
		}
	};

	// ---------------- F i l l ----------------

	enum class Repeat: uint8_t {
		NONE = value::NONE,
		REPEAT = value::REPEAT,
		REPEAT_X = value::REPEAT_X,
		REPEAT_Y = value::REPEAT_Y,
		MIRRORED_REPEAT = value::MIRRORED_REPEAT,
		MIRRORED_REPEAT_X = value::MIRRORED_REPEAT_X,
		MIRRORED_REPEAT_Y = value::MIRRORED_REPEAT_Y,
	};
	
	/**
	* @enum FillPositionType
	*/
	enum class FillPositionType: uint8_t {
		PIXEL = value::PIXEL,     /* 像素值  px */
		RATIO = value::RATIO,   /* 百分比  % */
		LEFT = value::LEFT,      /* 居左 */
		RIGHT = value::RIGHT,     /* 居右  % */
		CENTER = value::CENTER,    /* 居中 */
		TOP = value::TOP,       /* 居上 */
		BOTTOM = value::BOTTOM,    /* 居下 */
	};
	
	/**
	* @enum FillSizeType
	*/
	enum class FillSizeType: uint8_t {
		AUTO = value::AUTO,      /* 自动值  auto */
		PIXEL = value::PIXEL,     /* 像素值  px */
		RATIO = value::RATIO,   /* 百分比  % */
	};

	typedef TemplateValue<FillSizeType, FillSizeType::AUTO, float> FillSize;
	typedef TemplateValue<FillPositionType, FillPositionType::PIXEL, float> FillPosition;
	
	struct FillSizeCollection {
		FillSize x, y;
	};

	struct FillPositionCollection {
		FillPosition x, y;
	};

	// ---------------- F l e x . F l o w ----------------

	// layout direction
	enum class Direction: uint8_t {
		ROW = value::ROW,
		ROW_REVERSE = value::ROW_REVERSE,
		COLUMN = value::COLUMN,
		COLUMN_REVERSE = value::COLUMN_REVERSE,
	};

	// 项目在主轴上的对齐方式
	enum class ItemsAlign: uint8_t {
		START = value::START, // 左对齐
		CENTER = value::CENTER, // 居中
		END = value::END, // 右对齐
		SPACE_BETWEEN = value::SPACE_BETWEEN, // 两端对齐，项目之间的间隔都相等
		SPACE_AROUND = value::SPACE_AROUND, // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
		SPACE_EVENLY = value::SPACE_EVENLY, // 每个项目两侧的间隔相等,这包括边框的间距
	};

	// 项目在交叉轴内如何对齐
	enum class CrossAlign: uint8_t {
		START = value::START, // 与交叉轴内的起点对齐
		CENTER = value::CENTER, // 与交叉轴内的中点对齐
		END = value::END, // 与交叉轴内的终点对齐
	};

	// 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
	enum class Wrap: uint8_t {
		NO_WRAP = value::NO_WRAP, // 只有一根交叉轴线
		WRAP = value::WRAP, // 溢出后会有多根交叉轴线
		WRAP_REVERSE = value::WRAP_REVERSE, // 多根交叉轴线反向排列
	};

	// 多根交叉轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
	enum class WrapAlign: uint8_t {
		START = value::START, // 与交叉轴的起点对齐
		CENTER = value::CENTER, // 与交叉轴的中点对齐
		END = value::END, // 与交叉轴的终点对齐
		SPACE_BETWEEN = value::SPACE_BETWEEN, // 与交叉轴两端对齐,轴线之间的间隔平均分布
		SPACE_AROUND = value::SPACE_AROUND, // 每根轴线两侧的间隔都相等,所以轴线之间的间隔比轴线与边框的间隔大一倍
		SPACE_EVENLY = value::SPACE_EVENLY, // 每根轴线两侧的间隔都相等,这包括边框的间距
		STRETCH = value::STRETCH, // 轴线占满整个交叉轴，平均分配剩余的交叉轴空间
	};

	// layout align
	enum class Align: uint8_t {
		// flow/flex
		AUTO = value::AUTO,
		START = value::START,
		CENTER = value::CENTER,
		END = value::END,
		// default
		LEFT_TOP = value::LEFT_TOP,
		CENTER_TOP = value::CENTER_TOP,
		RIGHT_TOP = value::RIGHT_TOP,
		LEFT_CENTER = value::LEFT_CENTER,
		CENTER_CENTER = value::CENTER_CENTER,
		RIGHT_CENTER = value::RIGHT_CENTER,
		LEFT_BOTTOM = value::LEFT_BOTTOM,
		CENTER_BOTTOM = value::CENTER_BOTTOM,
		RIGHT_BOTTOM = value::RIGHT_BOTTOM,
	};

	/**
	* @enum SizeType
	*/
	enum class SizeType: uint8_t {
		NONE  = value::NONE,    /* none default wrap content */
		WRAP  = value::WRAP,    /* 包裹内容 wrap content */
		MATCH = value::MATCH,   /* 匹配父视图 match parent */
		PIXEL = value::PIXEL,   /* 明确值 value px */
		RATIO = value::RATIO,   /* 百分比 value % */
		MINUS = value::MINUS,   /* 减法(parent-value) value ! */
	};

	typedef TemplateValue<SizeType, SizeType::WRAP> SizeValue;

	// ---------------- T e x t . F o n t ----------------

	/**
	* @enum TextAlign 文本对齐方式
	*/
	enum class TextAlign: uint8_t {
		LEFT = value::LEFT,           /* 左对齐 */
		CENTER = value::CENTER,         /* 居中 */
		RIGHT = value::RIGHT,          /* 右对齐 */
		LEFT_REVERSE = value::LEFT_REVERSE,   /* 左对齐并反向 */
		CENTER_REVERSE = value::CENTER_REVERSE, /* 居中对齐并反向 */
		RIGHT_REVERSE = value::RIGHT_REVERSE,  /* 右对齐并反向 */
	};

	/**
	* @enum TextValueType
	*/
	enum class TextValueType: uint8_t {
		DEFAULT = value::DEFAULT,
		INHERIT = value::INHERIT,
		VALUE = value::VALUE,
	};

	/**
	* @enum TextWeightValue
	*/
	enum class TextWeightValue: uint8_t {
		THIN = value::THIN,
		ULTRALIGHT = value::ULTRALIGHT,
		LIGHT = value::LIGHT,
		REGULAR = value::REGULAR,
		MEDIUM = value::MEDIUM,
		SEMIBOLD = value::SEMIBOLD,
		BOLD = value::BOLD,
		HEAVY = value::HEAVY,
		BLACK = value::BLACK,
	};

	/**
		* @enum TextStyleValue
		*/
	enum class TextStyleValue: uint8_t {
		NORMAL = value::NORMAL, // 正常
		ITALIC = value::ITALIC, // 斜体
		OBLIQUE = value::OBLIQUE,  // 倾斜
	};

	/**
	* @enum TextDecorationValue
	*/
	enum class TextDecorationValue: uint8_t {
		NONE = value::NONE,           /* 没有 */
		OVERLINE = value::OVERLINE,       /* 上划线 */
		LINE_THROUGH = value::LINE_THROUGH,   /* 中划线 */
		UNDERLINE = value::UNDERLINE,      /* 下划线 */
	};

	/**
	* @enum TextOverflowValue
	*/
	enum class TextOverflowValue: uint8_t {
		NORMAL = value::NORMAL,          /* 不做任何处理 */
		CLIP = value::CLIP,            /* 剪切 */
		ELLIPSIS = value::ELLIPSIS,        /* 剪切并显示省略号 */
		CENTER_ELLIPSIS = value::CENTER_ELLIPSIS, /* 剪切并居中显示省略号 */
	};
	
	/**
		* @enum TextWhiteSpaceValue
		*/
	enum class TextWhiteSpaceValue: uint8_t {
		NORMAL = value::NORMAL,          /* 保留所有空白,使用自动wrap */
		NO_WRAP = value::NO_WRAP,        /* 合并空白序列,不使用自动wrap */
		NO_SPACE = value::NO_SPACE,      /* 合并空白序列,使用自动wrap */
		PRE = value::PRE,                /* 保留所有空白,不使用自动wrap */
		PRE_LINE = value::PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */
		WRAP = value::WRAP,              /* 保留所有空白,强制使用自动wrap */
	};

	class FontFamilysID;

	typedef FontFamilysID* FFID;
	typedef const FFID cFFID;

	typedef TemplateValue<TextValueType, TextValueType::INHERIT, Color> TextColor;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, float> TextSize;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextWeightValue> TextWeight;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextStyleValue> TextStyle;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, Shadow> TextShadow;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, float> TextLineHeight;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextDecorationValue> TextDecoration;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextOverflowValue> TextOverflow;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextWhiteSpaceValue> TextWhiteSpace;
	typedef TemplateValue<TextValueType, TextValueType::INHERIT, cFFID> TextFamily;

	// ---------------- K e y b o a r d . T y p e ----------------

	/**
		* @enum KeyboardType
		*/
	enum class KeyboardType: uint8_t {
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
	enum class KeyboardReturnType: uint8_t {
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

}

#endif
