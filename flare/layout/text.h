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
		* @enum TextWeightValue
		*/
		enum TextWeightValue: uint8_t {
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
		enum TextStyleValue: unit8_t {
			NORMAL = value::NORMAL, // 正常
			ITALIC = value::ITALIC, // 斜体
			OBLIQUE = value::OBLIQUE,  // 倾斜
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
			NORMAL = value::NORMAL,          /* 保留所有空白,使用自动wrap */
			NO_WRAP = value::NO_WRAP,        /* 合并空白序列,不使用自动wrap */
			NO_SPACE = value::NO_SPACE,      /* 合并空白序列,使用自动wrap */
			PRE = value::PRE,                /* 保留所有空白,不使用自动wrap */
			PRE_LINE = value::PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */
			WRAP = value::WRAP,              /* 保留所有空白,强制使用自动wrap */
		};

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

		// TODO ...
		private:
		// TODO ...
	};

	class DefaultTextSettings: public Object {
		public:
		DefaultTextSettings();

		// get default text attrs
		inline TextColor text_background_color() { return _text_background_color; }
		inline TextColor text_color() { return _text_color; }
		inline TextSize text_size() { return _text_size; }
		inline TextStyle text_style() { return _text_style; }
		inline TextFamily text_family() { return _text_family; }
		inline TextShadow text_shadow() { return _text_shadow; }
		inline TextLineHeight text_line_height() { return _text_line_height; }
		inline TextDecoration text_decoration() { return _text_decoration; }
		inline TextOverflow text_overflow() { return _text_overflow; }
		inline TextWhiteSpace text_white_space() { return _text_white_space; }
		// set default text attrs
		void set_text_background_color(TextColor value);
		void set_text_color(TextColor value);
		void set_text_size(TextSize value);
		void set_text_style(TextStyle value);
		void set_text_family(TextFamily value);
		void set_text_shadow(TextShadow value);
		void set_text_line_height(TextLineHeight value);
		void set_text_decoration(TextDecoration value);
		void set_text_overflow(TextOverflow value);
		void set_text_white_space(TextWhiteSpace value);
		
		private:
		TextColor       _text_background_color; // default text attrs
		TextColor       _text_color;
		TextSize        _text_size;
		TextStyle       _text_style;
		TextFamily      _text_family;
		TextShadow      _text_shadow;
		TextLineHeight  _text_line_height;
		TextDecoration  _text_decoration;
		TextOverflow    _text_overflow;
		TextWhiteSpace  _text_white_space; // text
	}

}

#endif