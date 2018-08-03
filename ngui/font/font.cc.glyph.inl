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

class FontFamilysID::Inl: public FontFamilysID {
public:
#define _inl_ff_id(self) static_cast<FontFamilysID::Inl*>(self)
	
	void initialize(const Array<String>& names) {
		m_names = names;
		m_name = names.join(',');
		m_code = m_name.hash_code();
	}
};

class FontGlyphTable::Inl: public FontGlyphTable {
public:
	#define _inl_table(self) static_cast<FontGlyphTable::Inl*>(self)
	
	void clear_table() {
		for ( int i = 0; i < 512; i++ ) {
			delete m_blocks[i]; m_blocks[i] = nullptr;
		}
	}
	
	void initialize(cFFID ffid, TextStyleEnum style, FontPool* pool) {
		m_ffid = ffid;
		m_style = style;
		m_pool = pool;
		make();
	}
	
	void make() {
		clear_table();
		m_fonts.clear();
		
		Map<String, bool> fonts_name;
		
		for ( auto& i : m_ffid->names() ) {
			Font* font = m_pool->get_font(i.value(), m_style);
			if ( font && !fonts_name.has(font->name()) ) {
				if ( font->load() ) { // 载入字体
					m_fonts.push(font);
					fonts_name.set(font->name(), true);
				}
			}
		}
		
		/* 如果使用icon图标字体时，加入默认字体会导致不同系统`ascender`与`descender`有所差异,
		 * 从而导致不同系统的字体基线的位置不相同，无法使用`icon`精准排版，
		 * 为了使用这个`icon`进行精准排版现在暂时做例外处理，在使用`icon`字体名称时不加入默认字体
		 */
		
		if (m_ffid->code() != 2090550926) {
			for ( auto& i : m_pool->m_default_fonts ) {
				Font* font = i.value()->font(m_style);
				if ( ! fonts_name.has(font->name()) ) { // 避免重复的名称
					if ( font->load() ) { // 载入字体
						m_fonts.push(font);
						fonts_name.set(font->name(), true);
					}
				}
			}
		} else {
			XX_ASSERT(m_ffid->name() == "icon");
		}
		
		// 查找最大高度与行高度
		m_height = 0;
		m_ascender = 0;
		m_descender = 0;
		
		for ( int i = 0; i < m_fonts.length(); i++ ) {
			Font* font = m_fonts[i];
			m_height = XX_MAX(m_height, font->height());
			m_ascender = XX_MAX(m_ascender, font->ascender());
			m_descender = XX_MAX(m_descender, font->descender());
		}
	}
	
	void set_glyph(uint region, uint index, FontGlyph* glyph) {
		XX_ASSERT( glyph );
		if ( !m_blocks[region] ) {
			GlyphsBlock* block = new GlyphsBlock();
			memset(block, 0, sizeof(GlyphsBlock));
			m_blocks[region] = block;
		}
		m_blocks[region]->glyphs[index] = glyph;
	}
	
	FontGlyph* get_glyph(uint16 unicode) {
		uint region = unicode / 128;
		uint index = unicode % 128;
		GlyphsBlock* con = m_blocks[region];
		if ( con ) {
			FontGlyph* glyph = con->glyphs[index];
			if ( glyph ) {
				return glyph;
			}
		}
		return nullptr;
	}
	
	/**
	 * @func find_glyph
	 */
	FontGlyph* find_glyph(uint16 unicode, FGTexureLevel level, bool vector) {
		
		FontGlyph* glyph = nullptr;
		
		uint region = unicode / 128;
		uint index = unicode % 128;
		
		if ( m_fonts.length() ) {
			
			Font** begin = &m_fonts[0];
			Font** end = begin + m_fonts.length();
			
			do {
				glyph = _inl_font(*begin)->get_glyph(unicode, region, index, level, vector);
				if ( glyph ) {
					set_glyph(region, index, glyph); return glyph;
				}
				begin++;
			} while (begin != end);
		}
		
		/* TODO 使用一个默认字形  � 65533 */
		Font* font = m_pool->m_spare_family->font(m_style);
		
		glyph = _inl_font(font)->get_glyph(65533, 65533 / 128, 65533 % 128, level, vector);
		
		set_glyph(region, index, glyph);
		
		return glyph;
	}
	
};

FontGlyphTable::~FontGlyphTable() {
	_inl_table(this)->clear_table();
}

/**
 * @func glyph
 */
FontGlyph* FontGlyphTable::glyph(uint16 unicode) {
	FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
	if ( glyph ) {
		return glyph;
	}
	return _inl_table(this)->find_glyph(unicode, FontGlyph::LEVEL_NONE, false);
}

/**
 * @func use_texture_glyph 使用纹理字型
 */
FontGlyph* FontGlyphTable::use_texture_glyph(uint16 unicode, FGTexureLevel level) {
	XX_ASSERT(level < FontGlyph::LEVEL_NONE);
	
	FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
	
	if ( glyph ) {
		if ( glyph->has_texure_level(level) ) {
			glyph->m_container->use_count++; return glyph;
		}
		else { // 先尝试使用最快的方法
			if ( _inl_font(glyph->font())->set_texture_data(glyph, level) ) { //
				glyph->m_container->use_count++; return glyph;
			}
		}
	}
	glyph = _inl_table(this)->find_glyph(unicode, level, false);
	
	glyph->m_container->use_count++; return glyph;
}

/**
 * @func use_vector_glyph # 使用字型,并且载入vbo矢量顶点数据
 */
FontGlyph* FontGlyphTable::use_vector_glyph(uint16 unicode) {
	FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
	
	if ( glyph ) {
		if ( glyph->vertex_data() ) {
			glyph->m_container->use_count++; return glyph;
		}
		else { // 先尝试使用最快的方法
			if ( _inl_font(glyph->font())->set_vertex_data(glyph) ) { //
				glyph->m_container->use_count++; return glyph;
			}
		}
	}
	glyph = _inl_table(this)->find_glyph(unicode, FontGlyph::LEVEL_NONE, true);
	
	glyph->m_container->use_count++; return glyph;
}

XX_END
