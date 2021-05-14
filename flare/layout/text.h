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

#ifndef __flare__layout__text__
#define __flare__layout__text__

#include "./box.h"

namespace flare {

	class FontFamilysID;

	typedef const FontFamilysID *FFID;

	class FX_EXPORT Text: public Box {
		FX_Define_View(Text);
		public:

		/**
		* @enum TextValueType
		*/
		enum TextValueType: uint8_t {
			DEFAULT = value::DEFAULT,
			INHERIT = value::INHERIT,
			VALUE = value::VALUE,
		};

		/**
		* @enum TextStyleValue
		*/
		enum TextStyleValue: uint8_t {
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
		};

		/**
		* @enum TextDecorationValue
		*/
		enum TextDecorationValue: uint8_t {
			NONE = value::NONE,           /* 没有 */
			OVERLINE = value::OVERLINE,       /* 上划线 */
			LINE_THROUGH = value::LINE_THROUGH,   /* 中划线 */
			UNDERLINE = value::UNDERLINE,      /* 下划线 */
		};

		/**
		* @enum TextOverflowValue
		*/
		enum TextOverflowValue: uint8_t {
			NORMAL = value::NORMAL,          /* 不做任何处理 */
			CLIP = value::CLIP,            /* 剪切 */
			ELLIPSIS = value::ELLIPSIS,        /* 剪切并显示省略号 */
			CENTER_ELLIPSIS = value::CENTER_ELLIPSIS, /* 剪切并居中显示省略号 */
		};
		
		/**
		 * @enum TextWhiteSpaceValue
		 */
		enum TextWhiteSpaceValue: uint8_t {
			NORMAL = value::NORMAL,           /* 保留所有空白,使用自动wrap */
			NO_WRAP = value::NO_WRAP,          /* 合并空白序列,不使用自动wrap */
			NO_SPACE = value::NO_SPACE,         /* 合并空白序列,使用自动wrap */
			PRE = value::PRE,              /* 保留所有空白,不使用自动wrap */
			PRE_LINE = value::PRE_LINE,         /* 合并空白符序列,但保留换行符,使用自动wrap */
			WRAP = value::WRAP,             /* 保留所有空白,强制使用自动wrap */
		};

		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, Color> TextColor;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, float> TextSize;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, TextStyleValue> TextStyle;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, Shadow> TextShadow;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, float> TextLineHeight;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, TextDecorationValue> TextDecoration;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, TextOverflowValue> TextOverflow;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, TextWhiteSpaceValue> TextWhiteSpace;
		typedef ValueTemplate<TextValueType, TextValueType::INHERIT, FFID> TextFamily;

		// TODO ...
		private:
		// TODO ...
	};

}

#endif