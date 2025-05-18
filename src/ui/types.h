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

#ifndef __quark__types__
#define __quark__types__

#include "../util/util.h"
#include "../render/math.h"
#include "../render/font/style.h"

namespace qk {
	// ---------------- F i l l ----------------

	enum class Repeat: uint8_t {
		Repeat,
		RepeatX,
		RepeatY,
		RepeatNo,
	};

	enum class FillPositionKind: uint8_t {
		Value,     /* 明确值  rem */
		Ratio,     /* 百分比  % */
		Start,     /* 开始 start */
		End,       /* 结束 end */
		Center,    /* 居中 center */
	};

	enum class FillSizeKind: uint8_t {
		Auto,      /* 自动值  auto */
		Value,     /* 明确值  rem */
		Ratio,     /* 百分比  % */
	};

	// ---------------- F l e x . F l o w ----------------

	// view direction
	enum class Direction: uint8_t {
		Row,
		RowReverse,
		Column,
		ColumnReverse,
	};

	// 项目在主轴上的对齐方式
	enum class ItemsAlign: uint8_t {
		Start, // 左对齐
		Center, // 居中
		End, // 右对齐
		SpaceBetween, // 两端对齐，项目之间的间隔都相等
		SpaceAround, // 每个项目两侧的间隔相等，项目之间的间隔比项目边界的间隔大一倍
		SpaceEvenly, // 每个项目两侧的间隔相等，这包括边框的间距
		CenterPart, // 把除两端以外的所有项目尽可能的居中对齐
	};

	// 项目在交叉轴内如何对齐
	enum class CrossAlign: uint8_t {
		Start, // 与交叉轴内的起点对齐
		Center, // 与交叉轴内的中点对齐
		End, // 与交叉轴内的终点对齐
	};

	// 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
	enum class Wrap: uint8_t {
		NoWrap, // 只有一根交叉轴线
		Wrap, // 溢出后会有多根交叉轴线
		WrapReverse, // 多根交叉轴线反向排列
	};

	// 多根交叉轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
	enum class WrapAlign: uint8_t {
		Start, // 与交叉轴的起点对齐
		Center, // 与交叉轴的中点对齐
		End, // 与交叉轴的终点对齐
		SpaceBetween, // 与交叉轴两端对齐,轴线之间的间隔平均分布
		SpaceAround, // 每根轴线两侧的间隔都相等,所以轴线之间的间隔比轴线与边框的间隔大一倍
		SpaceEvenly, // 每根轴线两侧的间隔都相等,这包括边框的间距
		Stretch, // 轴线占满整个交叉轴，平均分配剩余的交叉轴空间给每一个交叉轴项目
	};

	// view align
	enum class Align: uint8_t {
		Auto,
		Start,
		Center,
		End,
		Baseline = Auto, // vertical align in text
		Top, // Start, vertical align in text
		Middle, // Center, vertical align in text
		Bottom, // End, vertical align in text
		LeftTop = Start,
		CenterTop, // Center
		RightTop, // End
		LeftMiddle,
		CenterMiddle,
		RightMiddle,
		LeftBottom,
		CenterBottom,
		RightBottom,
	};

	enum class BoxSizeKind: uint8_t {
		None,    /* Do not use value */
		Auto,    /* 包裹内容 wrap content or auto value */
		Match,   /* 匹配父视图 match parent */
		Value,   /* 明确值 dp */
		// Pixel,   /* 明确值 px */
		Ratio,   /* 百分比 value % */
		Minus,   /* 减法(parent-value) value ! */
	};
	typedef FillSizeKind BoxOriginKind;

	// ---------------- T e x t . F o n t ----------------

	enum class TextValueKind: uint8_t {
		Inherit, Default, Value,
	};

	enum class TextAlign: uint8_t {
		Inherit, // inherit
		Left,           /* 左对齐 */
		Center,         /* 居中 */
		Right,          /* 右对齐 */
		Default = Left,
	};

	enum class TextDecoration: uint8_t {
		Inherit, // inherit
		None,           /* 没有 */
		Overline,       /* 上划线 */
		LineThrough,   /* 中划线 */
		Underline,      /* 下划线 */
		Default = None,
	};

	enum class TextOverflow: uint8_t {
		Inherit, // inherit
		Normal,          /* 不做任何处理 */
		Clip,            /* 剪切 */
		Ellipsis,        /* 剪切并显示省略号 */
		EllipsisCenter, /* 剪切并居中显示省略号 */
		Default = Normal,
	};

	enum class TextWhiteSpace: uint8_t {
		Inherit,       // inherit
		Normal,       /* 合并空白序列,使用自动wrap */
		NoWrap,       /* 合并空白序列,不使用自动wrap */
		Pre,          /* 保留所有空白,不使用自动wrap */
		PreWrap,      /* 保留所有空白,使用自动wrap */
		PreLine,      /* 合并空白符序列,但保留换行符,使用自动wrap */
		Default = Normal,
	};

	enum class TextWordBreak: uint8_t {
		Inherit,  // inherit
		Normal,   /* 保持单词在同一行 */
		BreakWord,/* 保持单词在同一行,除非单词长度超过一行才截断 */
		BreakAll, /* 以字为单位行空间不足换行 */
		KeepAll,  /* 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符 */
		Default = Normal,
	};

	// ---------------- K e y b o a r d . T y p e ----------------

	enum class KeyboardType: uint8_t {
		Normal,
		Ascii,
		Number,
		Url,
		NumberPad,
		Phone,
		NamePhone,
		Email,
		Decimal,
		Search,
		AsciiNumber,
	};

	enum class KeyboardReturnType: uint8_t {
		Normal,
		Go,
		Join,
		Next,
		Route,
		Search,
		Send,
		Done,
		Emergency,
		Continue,
	};

	// ---------------- C u r s o r . S t y l e ----------------

	enum class CursorStyle: uint8_t {
		Normal,
		None,
		NoneUntilMouseMoves,
		Arrow,
		Ibeam,
		PointingHand,
		ClosedHand,
		OpenHand,
		ResizeLeft,
		ResizeRight,
		ResizeLeftRight,
		ResizeUp,
		ResizeDown,
		ResizeUpDown,
		Crosshair,
		DisappearingItem,
		OperationNotAllowed,
		DragLink,
		DragCopy,
		ContextualMenu,
		IbeamForVertical,
		Cross,
	};

	// ----------------------------------------------------------------

	enum class FindDirection: uint8_t {
		None,
		Left,
		Top,
		Right,
		Bottom,
	};

	// ----------------------------------------------------------------

	// box border value
	struct BoxBorder {
		float width;
		Color color;
	};

	struct Shadow {
		inline bool operator==(const Shadow& val) const {
			return val.x == x && val.y == y && val.size == size && val.color == color;
		}
		inline bool operator!=(const Shadow& val) const {
			return !operator==(val);
		}
		float x, y, size;
		Color color;
	};

	template<typename Kind, Kind KindInit, typename Value = float>
	struct WrapValue {
		inline bool operator!=(const WrapValue& val) const {
			return val.kind != kind || val.value != value;
		}
		inline bool operator==(const WrapValue& val) const {
			return !operator!=(val);
		}
		Value value = Value();
		Kind kind = KindInit;
	};

	typedef WrapValue<FillPositionKind, FillPositionKind::Value> FillPosition;
	typedef WrapValue<FillSizeKind, FillSizeKind::Auto> FillSize;

	typedef WrapValue<BoxSizeKind, BoxSizeKind::Value> BoxSize;
	typedef WrapValue<BoxOriginKind, BoxOriginKind::Value> BoxOrigin;

	template <typename T>
	using WrapTextValue = WrapValue<TextValueKind, TextValueKind::Value, T>;

	// inherit / default / value
	typedef WrapTextValue<Color>  TextColor;
	typedef WrapTextValue<float>  TextSize;
	typedef TextSize              TextLineHeight;
	typedef WrapTextValue<Shadow> TextShadow;
	typedef WrapTextValue<FFID>   TextFamily;

}
#endif
