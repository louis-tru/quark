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

F_NAMESPACE_START

template<typename Type, Type TypeInit, typename Value = float>
struct TemplateValue {
	Value value = Value();
	Type type = TypeInit;
	// @members method
	inline bool operator==(const TemplateValue& val) const {
		return val.type == type && val.value == value;
	}
	inline bool operator!=(const TemplateValue& val) const {
		return val.type != type || val.value != value;
	}
};

// shadow
struct Shadow {
	float offset_x, offset_y, size;
	Color color;
	// @members method
	inline bool operator==(const Shadow& val) const {
		return (
			val.offset_x == offset_x && val.offset_y == offset_y && 
			val.size     == size     && val.color    == color
		);
	}
	inline bool operator!=(const Shadow& val) const {
		return ! operator==(val);
	}
};

// ---------------- F i l l ----------------

enum class Repeat: uint8_t {
	REPEAT,
	REPEAT_X,
	REPEAT_Y,
	NO_REPEAT,
	//MIRRORED_REPEAT,
	//MIRRORED_REPEAT_X,
	//MIRRORED_REPEAT_Y,
};

/**
* @enum FillPositionType
*/
enum class FillPositionType: uint8_t {
	PIXEL,     /* 像素值  px */
	RATIO,     /* 百分比  % */
	START,      /* 开始 start */
	END,       /* 结束 end */
	CENTER,    /* 居中 center */
};

/**
* @enum FillSizeType
*/
enum class FillSizeType: uint8_t {
	AUTO,      /* 自动值  auto */
	PIXEL,     /* 像素值  px */
	RATIO,   /* 百分比  % */
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
	ROW,
	ROW_REVERSE,
	COLUMN,
	COLUMN_REVERSE,
};

// 项目在主轴上的对齐方式
enum class ItemsAlign: uint8_t {
	START, // 左对齐
	CENTER, // 居中
	END, // 右对齐
	SPACE_BETWEEN, // 两端对齐，项目之间的间隔都相等
	SPACE_AROUND, // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
	SPACE_EVENLY, // 每个项目两侧的间隔相等,这包括边框的间距
};

// 项目在交叉轴内如何对齐
enum class CrossAlign: uint8_t {
	START, // 与交叉轴内的起点对齐
	CENTER, // 与交叉轴内的中点对齐
	END, // 与交叉轴内的终点对齐
};

// 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
enum class Wrap: uint8_t {
	NO_WRAP, // 只有一根交叉轴线
	WRAP, // 溢出后会有多根交叉轴线
	WRAP_REVERSE, // 多根交叉轴线反向排列
};

// 多根交叉轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
enum class WrapAlign: uint8_t {
	START, // 与交叉轴的起点对齐
	CENTER, // 与交叉轴的中点对齐
	END, // 与交叉轴的终点对齐
	SPACE_BETWEEN, // 与交叉轴两端对齐,轴线之间的间隔平均分布
	SPACE_AROUND, // 每根轴线两侧的间隔都相等,所以轴线之间的间隔比轴线与边框的间隔大一倍
	SPACE_EVENLY, // 每根轴线两侧的间隔都相等,这包括边框的间距
	STRETCH, // 轴线占满整个交叉轴，平均分配剩余的交叉轴空间
};

// layout align
enum class Align: uint8_t {
	// flow/flex
	AUTO,
	START,
	CENTER,
	END,
	// default
	LEFT_TOP,
	CENTER_TOP,
	RIGHT_TOP,
	LEFT_CENTER,
	CENTER_CENTER,
	RIGHT_CENTER,
	LEFT_BOTTOM,
	CENTER_BOTTOM,
	RIGHT_BOTTOM,
};

/**
* @enum BoxSizeType
*/
enum class BoxSizeType: uint8_t {
	NONE,    /* none default wrap content */
	WRAP,    /* 包裹内容 wrap content */
	MATCH,   /* 匹配父视图 match parent */
	PIXEL,   /* 明确值 value px */
	RATIO,   /* 百分比 value % */
	MINUS,   /* 减法(parent-value) value ! */
};

typedef TemplateValue<BoxSizeType, BoxSizeType::WRAP> BoxSize;

// ---------------- T e x t . F o n t ----------------

/**
* @enum TextAlign 文本对齐方式
*/
enum class TextAlign: uint8_t {
	LEFT,           /* 左对齐 */
	CENTER,         /* 居中 */
	RIGHT,          /* 右对齐 */
	LEFT_REVERSE,   /* 左对齐并反向 */
	CENTER_REVERSE, /* 居中对齐并反向 */
	RIGHT_REVERSE,  /* 右对齐并反向 */
};

/**
* @enum TextValueType
*/
enum class TextValueType: uint8_t {
	DEFAULT,
	INHERIT,
	VALUE,
};

/**
* @enum TextWeightValue
*/
enum class TextWeightValue: uint8_t {
	THIN,
	ULTRALIGHT,
	LIGHT,
	REGULAR,
	MEDIUM,
	SEMIBOLD,
	BOLD,
	HEAVY,
	BLACK,
};

/**
	* @enum TextStyleValue
	*/
enum class TextStyleValue: uint8_t {
	NORMAL, // 正常
	ITALIC, // 斜体
	OBLIQUE,  // 倾斜
};

/**
* @enum TextDecorationValue
*/
enum class TextDecorationValue: uint8_t {
	NONE,           /* 没有 */
	OVERLINE,       /* 上划线 */
	LINE_THROUGH,   /* 中划线 */
	UNDERLINE,      /* 下划线 */
};

/**
* @enum TextOverflowValue
*/
enum class TextOverflowValue: uint8_t {
	NORMAL,          /* 不做任何处理 */
	CLIP,            /* 剪切 */
	ELLIPSIS,        /* 剪切并显示省略号 */
	CENTER_ELLIPSIS, /* 剪切并居中显示省略号 */
};

/**
	* @enum TextWhiteSpaceValue
	*/
enum class TextWhiteSpaceValue: uint8_t {
	NORMAL,          /* 保留所有空白,使用自动wrap */
	NO_WRAP,        /* 合并空白序列,不使用自动wrap */
	NO_SPACE,      /* 合并空白序列,使用自动wrap */
	PRE,                /* 保留所有空白,不使用自动wrap */
	PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */
	WRAP,              /* 保留所有空白,强制使用自动wrap */
};

class FontFamilysID;

typedef FontFamilysID* FFID;

typedef TemplateValue<TextValueType, TextValueType::INHERIT, Color> TextColor;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, float> TextSize;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextWeightValue> TextWeight;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextStyleValue> TextStyle;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, Shadow> TextShadow;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, float> TextLineHeight;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextDecorationValue> TextDecoration;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextOverflowValue> TextOverflow;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, TextWhiteSpaceValue> TextWhiteSpace;
typedef TemplateValue<TextValueType, TextValueType::INHERIT, FFID> TextFamily;

// ---------------- K e y b o a r d . T y p e ----------------

/**
	* @enum KeyboardType
	*/
enum class KeyboardType: uint8_t {
	NORMAL,
	ASCII,
	NUMBER,
	URL,
	NUMBER_PAD,
	PHONE,
	NAME_PHONE,
	EMAIL,
	DECIMAL,
	TWITTER,
	SEARCH,
	ASCII_NUMBER,
};

/**
* @enum KeyboardReturnType
*/		
enum class KeyboardReturnType: uint8_t {
	NORMAL,
	GO,
	JOIN,
	NEXT,
	ROUTE,
	SEARCH,
	SEND,
	DONE,
	EMERGENCY,
	CONTINUE,
};

/**
* @enum BorderStyle
*/
enum class BorderStyle: uint8_t {
	SOLID,
	DASHED,
	DOTTED,
	DOUBLE,
	GROOVE,
	INSET,
	OUTSET,
	RIDGE,
};


F_NAMESPACE_END
#endif
