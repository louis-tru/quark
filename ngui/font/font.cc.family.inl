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

XX_NS(ngui)

class FontFamily::Inl: public FontFamily {
	public:
#define _inl_family(self) static_cast<FontFamily::Inl*>(self)
	
	int get_font_style_index(TextStyleEnum style) {
		int index;
		switch(style) {
			case TextStyleEnum::THIN: index = 0; break;
			case TextStyleEnum::ULTRALIGHT: index = 1; break;
			case TextStyleEnum::LIGHT: index = 2; break;
			case TextStyleEnum::REGULAR: index = 3; break;
			case TextStyleEnum::MEDIUM: index = 4; break;
			case TextStyleEnum::SEMIBOLD: index = 5; break;
			case TextStyleEnum::BOLD: index = 6; break;
			case TextStyleEnum::HEAVY: index = 7; break;
			case TextStyleEnum::BLACK: index = 8; break;
			case TextStyleEnum::BLACK_ITALIC: index = 9; break;
			case TextStyleEnum::HEAVY_ITALIC: index = 10; break;
			case TextStyleEnum::BOLD_ITALIC: index = 11; break;
			case TextStyleEnum::SEMIBOLD_ITALIC: index = 12; break;
			case TextStyleEnum::MEDIUM_ITALIC: index = 13; break;
			case TextStyleEnum::ITALIC: index = 14; break;
			case TextStyleEnum::LIGHT_ITALIC: index = 15; break;
			case TextStyleEnum::ULTRALIGHT_ITALIC: index = 16; break;
			case TextStyleEnum::THIN_ITALIC: index = 17; break;
			default: index = 18; break;
		}
		return index;
	}
	
	/**
	 * @func add_font
	 */
	void add_font(Font* font) {
		int index = get_font_style_index(font->style());
		if ( !m_fonts[index] || m_fonts[index]->num_glyphs() < font->num_glyphs() ) {
			m_fonts[index] = font;
		}
		m_all_fonts.push(font);
	}
};

/**
 * @constructor
 */
FontFamily::FontFamily(cString& family_name)
: m_family_name(family_name)
, m_fonts()
{
	memset(m_fonts, 0, sizeof(m_fonts));
}

/**
 * @func font_names
 */
Array<String> FontFamily::font_names() const {
	Array<String> rev;
	
	for (auto i = m_all_fonts.begin(),
						e = m_all_fonts.end(); i != e; i++) {
		rev.push(i.value()->font_name());
	}
	return rev;
}

/**
 * @overwrite
 */
cString& FontFamily::name() const {
	return m_family_name;
}

Font* FontFamily::font(TextStyleEnum style) {
	int index = _inl_family(this)->get_font_style_index(style);
	Font* font = m_fonts[index];
	
	if ( font ) {
		return font;
	}
	
	int big = index + 1;
	int small = index - 1;
	
	// 查找相邻字重
	while (big < 19 || small >= 0) {
		if ( small >= 0 ) {
			font = m_fonts[small];
			if ( font ) break;
			small--;
		}
		if ( big < 19 ) {
			font = m_fonts[big];
			if ( font ) break;
			big++;
		}
	}
	return font;
}

XX_END
