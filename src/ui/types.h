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

#define Qk_DEFINE_VIEW_PROP_ACC_GET(...) Qk_DEFINE_PROP_ACC_GET(__VA_ARGS__)
#define Qk_DEFINE_VIEW_PROP_ACC(type,name,...) \
	Qk_DEFINE_VIEW_PROP_ACC_GET(type,name,##__VA_ARGS__); void set_##name (type val,bool isRt=false);
#define Qk_DEFINE_VIEW_PROP_GET(...) Qk_DEFINE_PROP_GET(__VA_ARGS__)
#define Qk_DEFINE_VIEW_PROP(type,name,...) \
	Qk_DEFINE_VIEW_PROP_GET(type,name,##__VA_ARGS__) void set_##name (type val,bool isRt=false)

namespace qk {

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

	struct Shadow {
		inline bool operator==(const Shadow& val) const {
			return (
				val.offset_x == offset_x && val.offset_y == offset_y && 
				val.size     == size     && val.color    == color
			);
		}
		inline bool operator!=(const Shadow& val) const {
			return ! operator==(val);
		}
		float offset_x, offset_y, size;
		Color color;
	};

	// ---------------- F i l l ----------------

	enum class Repeat: uint8_t {
		kRepeat,
		kRepeatX,
		kRepeatY,
		kRepeatNo,
	};

	/**
	* @enum FillPositionKind
	*/
	enum class FillPositionKind: uint8_t {
		kPixel,     /* 像素值  px */
		kRatio,     /* 百分比  % */
		kStart,     /* 开始 start */
		kEnd,       /* 结束 end */
		kCenter,    /* 居中 center */
	};

	/**
	* @enum FillSizeKind
	*/
	enum class FillSizeKind: uint8_t {
		kAuto,      /* 自动值  auto */
		kPixel,     /* 像素值  px */
		kRatio,     /* 百分比  % */
	};

	typedef WrapValue<FillPositionKind, FillPositionKind::kPixel, float> FillPosition;
	typedef WrapValue<FillSizeKind, FillSizeKind::kAuto, float> FillSize;
	struct FillSizes { FillSize x, y;};
	struct FillPositions { FillPosition x, y; };

	// ---------------- F l e x . F l o w ----------------

	// view direction
	enum class Direction: uint8_t {
		kRow,
		kRowReverse,
		kColumn,
		kColumnReverse,
	};

	// 项目在主轴上的对齐方式
	enum class ItemsAlign: uint8_t {
		kStart, // 左对齐
		kCenter, // 居中
		kEnd, // 右对齐
		kSpaceBetween, // 两端对齐，项目之间的间隔都相等
		kSpaceAround, // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
		kSpaceEvenly, // 每个项目两侧的间隔相等,这包括边框的间距
	};

	// 项目在交叉轴内如何对齐
	enum class CrossAlign: uint8_t {
		kStart, // 与交叉轴内的起点对齐
		kCenter, // 与交叉轴内的中点对齐
		kEnd, // 与交叉轴内的终点对齐
	};

	// 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
	enum class Wrap: uint8_t {
		kNoWrap, // 只有一根交叉轴线
		kWrap, // 溢出后会有多根交叉轴线
		kWrapReverse, // 多根交叉轴线反向排列
	};

	// 多根交叉轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
	enum class WrapAlign: uint8_t {
		kStart, // 与交叉轴的起点对齐
		kCenter, // 与交叉轴的中点对齐
		kEnd, // 与交叉轴的终点对齐
		kSpaceBetween, // 与交叉轴两端对齐,轴线之间的间隔平均分布
		kSpaceAround, // 每根轴线两侧的间隔都相等,所以轴线之间的间隔比轴线与边框的间隔大一倍
		kSpaceEvenly, // 每根轴线两侧的间隔都相等,这包括边框的间距
		kStretch, // 轴线占满整个交叉轴，平均分配剩余的交叉轴空间
	};

	// view align
	enum class Align: uint8_t {
		kAuto,
		kStart,
		kCenter,
		kEnd,
		kLeftTop = kStart,
		kCenterTop,
		kRightTop,
		kLeftCenter,
		kCenterCenter,
		kRightCenter,
		kLeftBottom,
		kCenterBottom,
		kRightBottom,
	};

	/**
	* @enum BoxSizeKind
	*/
	enum class BoxSizeKind: uint8_t {
		kNone,    /* none default wrap content */
		kWrap,    /* 包裹内容 wrap content */
		kMatch,   /* 匹配父视图 match parent */
		kPixel,   /* 明确值 value px */
		kRatio,   /* 百分比 value % */
		kMinus,   /* 减法(parent-value) value ! */
	};
	typedef FillSizeKind BoxOriginKind;

	typedef WrapValue<BoxSizeKind, BoxSizeKind::kPixel> BoxSize;
	typedef WrapValue<BoxOriginKind, BoxOriginKind::kPixel> BoxOrigin;

	// ---------------- T e x t . F o n t ----------------

	enum class TextAlign: uint8_t {
		kLeft,           /* 左对齐 */
		kCenter,         /* 居中 */
		kRight,          /* 右对齐 */
		kDefault = kLeft,
	};

	enum class TextDecoration: uint8_t {
		kInherit, // inherit
		kNone,           /* 没有 */
		kOverline,       /* 上划线 */
		kLineThrough,   /* 中划线 */
		kUnderline,      /* 下划线 */
		kDefault = kNone,
	};

	enum class TextOverflow: uint8_t {
		kInherit, // inherit
		kNormal,          /* 不做任何处理 */
		kClip,            /* 剪切 */
		kEllipsis,        /* 剪切并显示省略号 */
		kEllipsisCenter, /* 剪切并居中显示省略号 */
		kDefault = kNormal,
	};

	enum class TextWhiteSpace: uint8_t {
		kInherit,       // inherit
		kNormal,       /* 合并空白序列,使用自动wrap */
		kNoWrap,       /* 合并空白序列,不使用自动wrap */
		kPre,          /* 保留所有空白,不使用自动wrap */
		kPreWrap,      /* 保留所有空白,使用自动wrap */
		kPreLine,      /* 合并空白符序列,但保留换行符,使用自动wrap */
		kDefault = kNormal,
	};

	enum class TextWordBreak: uint8_t {
		kInherit,  // inherit
		kNormal,   /* 保持单词在同一行 */
		kBreakWord,/* 保持单词在同一行,除非单词长度超过一行才截断 */
		kBreakAll, /* 以字为单位行空间不足换行 */
		kKeepAll,  /* 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符 */
		kDefault = kNormal,
	};

	/**
	* @enum TextValueKind
	*/
	enum class TextValueKind: uint8_t {
		kInherit, kDefault, kValue,
	};

	// text value template
	template<typename Value> struct TextValueWrap {
		typedef NonObjectTraits Traits;
		inline bool operator!=(const TextValueWrap& val) const {
			return kind != val.kind || (kind == TextValueKind::kValue && value != val.value);
		}
		inline bool operator==(const TextValueWrap& val) const {
			return !operator!=(val);
		}
		Value value = Value();
		TextValueKind kind = TextValueKind::kValue;
	};

	typedef TextValueWrap<Color> TextColor; // inherit / default / value
	typedef TextValueWrap<float> TextSize;
	typedef TextSize             TextLineHeight;
	typedef TextValueWrap<Shadow> TextShadow;
	typedef TextValueWrap<FFID> TextFamily;

	// ---------------- K e y b o a r d . T y p e ----------------

	/**
		* @enum KeyboardType
		*/
	enum class KeyboardType: uint8_t {
		kNormal,
		kAscii,
		kNumber,
		kUrl,
		kNumberPad,
		kPhone,
		kNamePhone,
		kEmail,
		kDecimal,
		kSearch,
		kAsciiNumber,
	};

	/**
	* @enum KeyboardReturnType
	*/
	enum class KeyboardReturnType: uint8_t {
		kNormal,
		kGo,
		kJoin,
		kNext,
		kRoute,
		kSearch,
		kSend,
		kDone,
		kEmergency,
		kContinue,
	};

	// ---------------- C u r s o r . S t y l e ----------------

	/**
	 * @enum CursorStyle
	*/
	enum class CursorStyle: uint8_t {
		kNormal,
		kNone,
		kNoneUntilMouseMoves,
		kArrow,
		kIBeam,
		kPointingHand,
		kClosedHand,
		kOpenHand,
		kResizeLeft,
		kResizeRight,
		kResizeLeftRight,
		kResizeUp,
		kResizeDown,
		kResizeUpDown,
		kCrosshair,
		kDisappearingItem,
		kOperationNotAllowed,
		kDragLink,
		kDragCopy,
		kContextualMenu,
		kIBeamForVertical,
	};

	// ----------------------------------------------------------------

	enum class FindDirection: uint8_t {
		kNone,
		kLeft,
		kTop,
		kRight,
		kBottom,
	};

	// box border value
	struct BoxBorder {
		Color color[4]; // top/right/bottom/left
		float width[4];
	};

}
#endif
