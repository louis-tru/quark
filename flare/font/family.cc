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

#include "./_font.h"

namespace flare {

	int FontFamily::Inl::get_font_style_index(TextStyleValue style) {
		// TODO ...
		int index = 0;
		// switch(style) {
		// 	case TextStyleValue::THIN: index = 0; break;
		// 	case TextStyleValue::ULTRALIGHT: index = 1; break;
		// 	case TextStyleValue::LIGHT: index = 2; break;
		// 	case TextStyleValue::NORMAL: index = 3; break;
		// 	case TextStyleValue::MEDIUM: index = 4; break;
		// 	case TextStyleValue::SEMIBOLD: index = 5; break;
		// 	case TextStyleValue::BOLD: index = 6; break;
		// 	case TextStyleValue::HEAVY: index = 7; break;
		// 	case TextStyleValue::BLACK: index = 8; break;
		// 	case TextStyleValue::BLACK_ITALIC: index = 9; break;
		// 	case TextStyleValue::HEAVY_ITALIC: index = 10; break;
		// 	case TextStyleValue::BOLD_ITALIC: index = 11; break;
		// 	case TextStyleValue::SEMIBOLD_ITALIC: index = 12; break;
		// 	case TextStyleValue::MEDIUM_ITALIC: index = 13; break;
		// 	case TextStyleValue::ITALIC: index = 14; break;
		// 	case TextStyleValue::LIGHT_ITALIC: index = 15; break;
		// 	case TextStyleValue::ULTRALIGHT_ITALIC: index = 16; break;
		// 	case TextStyleValue::THIN_ITALIC: index = 17; break;
		// 	default: index = 18; break;
		// }
		return index;
	}

	void FontFamily::Inl::add_font(Font* font) {
		int index = get_font_style_index(font->style());
		if ( !_fonts[index] || _fonts[index]->num_glyphs() < font->num_glyphs() ) {
			_fonts[index] = font;
		}
		_all_fonts.push(font);
	}

	FontFamily::FontFamily(cString& family_name)
		: _family_name(family_name)
		, _fonts()
	{
		memset(_fonts, 0, sizeof(_fonts));
	}

	/**
	* @func font_names
	*/
	Array<String> FontFamily::font_names() const {
		Array<String> rev;
		
		for (auto i = _all_fonts.begin(),
							e = _all_fonts.end(); i != e; i++) {
			rev.push((*i)->font_name());
		}
		return rev;
	}

	/**
	* @overwrite
	*/
	cString& FontFamily::name() const {
		return _family_name;
	}

	Font* FontFamily::font(TextStyleValue style, TextWeightValue weight) {
		int index = _inl_family(this)->get_font_style_index(style);
		Font* font = _fonts[index];
		
		if ( font ) {
			return font;
		}
		
		int big = index + 1;
		int small = index - 1;
		
		// 查找相邻字重
		while (big < 19 || small >= 0) {
			if ( small >= 0 ) {
				font = _fonts[small];
				if ( font ) break;
				small--;
			}
			if ( big < 19 ) {
				font = _fonts[big];
				if ( font ) break;
				big++;
			}
		}
		return font;
	}

}
