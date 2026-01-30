/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
		RepeatX, // only repeat x
		RepeatY, // only repeat y
		NoRepeat,
		MirrorRepeat, // mirror repeat
		MirrorRepeatX, // only mirror repeat x
		MirrorRepeatY, // only mirror repeat y
	};

	enum class FillPositionKind: uint8_t {
		Start,     /* 开始 start */
		Center,    /* 居中 center */
		End,       /* 结束 end */
		Value,     /* 明确值  rem */
		Ratio,     /* 百分比  % */
	};

	enum class FillSizeKind: uint8_t {
		Auto,      /* 自动值  auto */
		Value,     /* 明确值  rem */
		Ratio,     /* 百分比  % */
	};

	// ---------------- F l e x . F l o w ----------------

	// View direction
	enum class Direction: uint8_t {
		None,
		Row,
		RowReverse,
		Column,
		ColumnReverse,
		Left = Row,
		Right = RowReverse,
		Top = Column,
		Bottom = ColumnReverse,
		LeftTop,
		RightTop,
		RightBottom,
		LeftBottom,
	};

	// 项目在主轴上的对齐方式
	enum class ItemsAlign: uint8_t {
		Start, // 左对齐
		Center, // 居中
		End, // 右对齐
		SpaceBetween, // 两端对齐，项目之间的间隔都相等
		SpaceAround, // 每个项目两侧的间隔相等，项目之间的间隔比项目边界的间隔大一倍
		SpaceEvenly, // 每个项目两侧的间隔相等，这包括边框的间距
		CenterCenter, // 把除两端以外的所有项目尽可能的居中对齐
	};

	// 项目在交叉轴内如何对齐
	enum class CrossAlign: uint8_t {
		Start = 1, // 与交叉轴内的起点对齐 @Align.Start
		Center, // 与交叉轴内的中点对齐 @Align.Center
		End, // 与交叉轴内的终点对齐 @Align.End
		Both, // 与交叉轴内的两端对齐 @Align.Both
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
		Stretch = 7, // 轴线占满整个交叉轴，平均分配剩余的交叉轴空间给每一个交叉轴项目
	};

	// view align
	enum class Align: uint8_t {
		Normal,
		Start, // wrap line and left align
		Center, // wrap line and center align
		End, // wrap line and right align
		Both, //
		NewStart = Both, // New independent line and left align
		NewCenter, // New independent line and center align
		NewEnd, // New independent line and right align
		FloatStart, // Try not to wrap until the maximum limit and left align
		FloatCenter, // Try not to wrap until the maximum limit and center align
		FloatEnd, // Try not to wrap until the maximum limit and right align
		Baseline = Normal, // vertical align in text
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

	// Box size kind
	enum class BoxSizeKind: uint8_t {
		None,    /* Do not use value */
		Auto,    /* wrap content or auto value */
		Match,   /* match parent */
		Value,   /* Density-independent Pixel, dp = px * window.scale */
		Dp = Value,/* dp, alias of Value */
		// Pt,    /* pt, Points, pt = px * window.defaultScale */
		// Px,   /* px, Pixel */
		Ratio,   /* value % */
		Minus,   /* (parent-value) value ! */
	};
	typedef FillSizeKind BoxOriginKind;

	/**
	 * @enum LayoutType
	 * 
	 * The layout type of the View
	 */
	enum class LayoutType: uint8_t {
		Normal, /* Use Normal layout */
		Float, /* Float layout */
		Free, /* Free layout */
		Text, /* Text layout only Text and extended Text */
		Flex, /* Flex layout only Flex and extended Flex */
		Flow, /* Flow layout only Flow and extended Flow */
	};

	// ---------------- T e x t . F o n t ----------------

	// All Text* enums share the same semantic:
	// Inherit  -> resolve from parent
	// Default  -> resolve from application default value
	// Others   -> explicit values
	//
	// After style resolution, Inherit and Default
	// must not appear in layout/render stages.

	enum class TextValueKind: uint8_t {
		Inherit, Default, Value,
	};

	enum class TextAlign: uint8_t {
		Inherit,        /* inherit */
		Default,        /* use default of the application */
		Left,           /* 左对齐 */
		Center,         /* 居中 */
		Right,          /* 右对齐 */
	};

	enum class TextDecoration: uint8_t {
		Inherit,        /* inherit */
		Default,        /* use default of the application */
		None,           /* 没有 */
		Overline,       /* 上划线 */
		LineThrough,    /* 中划线 */
		Underline,      /* 下划线 */
	};

	enum class TextOverflow: uint8_t {
		Inherit,        /* inherit */
		Default,        /* use default of the application */
		Normal,         /* 不做任何处理 */
		Clip,           /* 剪切 */
		Ellipsis,       /* 剪切并显示省略号 */
		EllipsisCenter, /* 剪切并居中显示省略号 */
	};

	enum class WhiteSpace: uint8_t {
		Inherit,      /* inherit */
		Default,      /* use default of the application */
		Normal,       /* 使用自动wrap,  合并空白序列 */
		NoWrap,       /* 不使用自动wrap,合并空白序列 */
		Pre,          /* 不使用自动wrap,保留所有空白 */
		PreWrap,      /* 使用自动wrap, 保留所有空白 */
		PreLine,      /* 使用自动wrap, 合并空白符序列但保留换行符 */
	};

	enum class WordBreak: uint8_t {
		Inherit,  /* inherit */
		Default,  /* use default of the application */
		Normal,   /* 保持单词在同一行 */
		BreakWord,/* 保持单词在同一行,除非单词长度超过一行才截断 */
		BreakAll, /* 以字为单位行空间不足换行 */
		KeepAll,  /* 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符 */
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
		Text = Ibeam,
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

	/**
	 * Inherit Color from parent view
	 */
	enum class CascadeColor: uint8_t {
		None, // No inherit
		Alpha, // Inherit only Alpha from parent
		Color, // Inherit only RGB from parent
		Rgb = Color, // alias of Color
		Both, // Inherit RGB and Alpha from parent
	};

	// border value
	struct Border {
		float width;
		Color color;
		inline bool operator==(const Border& val) const {
			return val.width == width && val.color == color;
		}
		inline bool operator!=(const Border& val) const {
			return !operator==(val);
		}
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
		inline bool operator==(Kind val) const {
			return kind == val;
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
	typedef WrapTextValue<float>  FontSize;
	typedef FontSize              LineHeight;
	typedef WrapTextValue<Shadow> TextShadow;
	typedef WrapTextValue<FFID>   FontFamily;
	typedef WrapTextValue<Border> TextStroke;
}
#endif
