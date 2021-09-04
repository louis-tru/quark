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

	Font* FontGlyph::font() {
		return _container->font;
	}

	void FontFamilysID::Inl::initialize(const Array<String>& names) {
		_names = names.copy();
		_name = _names.join(",");
		_code = (uint32_t)_name.hash_code();
	}

	void FontGlyphTable::Inl::clear_table() {
		for ( int i = 0; i < 512; i++ ) {
			delete _blocks[i]; _blocks[i] = nullptr;
		}
	}
	
	void FontGlyphTable::Inl::initialize(FFID ffid, TextStyleValue style, FontPool* pool) {
		_ffid = ffid;
		_style = style;
		_pool = pool;
		make();
	}
	
	void FontGlyphTable::Inl::make() {
		clear_table();
		_fonts.clear();
		
		Dict<String, bool> fonts_name;
		
		for ( auto& i : _ffid->names() ) {
			Font* font = _pool->get_font(i, _style);
			if ( font && !fonts_name.count(font->name()) ) {
				if ( font->load() ) { // 载入字体
					_fonts.push(font);
					fonts_name[font->name()] = true;
				}
			}
		}
		
		/* 如果使用icon图标字体时，加入默认字体会导致不同系统`ascender`与`descender`有所差异,
		* 从而导致不同系统的字体基线的位置不相同，无法使用`icon`精准排版，
		* 为了使用这个`icon`进行精准排版现在暂时做例外处理，在使用`icon`字体名称时不加入默认字体
		*/
		
		if (_ffid->code() != 2090550926) {
			for ( auto& i : _pool->_default_fonts ) {
				Font* font = i->font(_style);
				if ( ! fonts_name.count(font->name()) ) { // 避免重复的名称
					if ( font->load() ) { // 载入字体
						_fonts.push(font);
						fonts_name[font->name()] = true;
					}
				}
			}
		} else {
			ASSERT(_ffid->name() == "icon");
		}
		
		// 查找最大高度与行高度
		_height = 0;
		_ascender = 0;
		_descender = 0;
		
		for ( uint32_t i = 0; i < _fonts.length(); i++ ) {
			Font* font = _fonts[i];
			_height = FX_MAX(_height, font->height());
			_ascender = FX_MAX(_ascender, font->ascender());
			_descender = FX_MAX(_descender, font->descender());
		}
	}
	
	void FontGlyphTable::Inl::set_glyph(uint32_t region, uint32_t index, FontGlyph* glyph) {
		ASSERT( glyph );
		if ( !_blocks[region] ) {
			GlyphsBlock* block = new GlyphsBlock();
			memset(block, 0, sizeof(GlyphsBlock));
			_blocks[region] = block;
		}
		_blocks[region]->glyphs[index] = glyph;
	}
	
	FontGlyph* FontGlyphTable::Inl::get_glyph(uint16_t unicode) {
		uint32_t region = unicode / 128;
		uint32_t index = unicode % 128;
		GlyphsBlock* con = _blocks[region];
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
	FontGlyph* FontGlyphTable::Inl::find_glyph(uint16_t unicode, FGTexureLevel level, bool vector) {
		
		FontGlyph* glyph = nullptr;
		
		uint32_t region = unicode / 128;
		uint32_t index = unicode % 128;
		
		if ( _fonts.length() ) {
			
			Font** begin = &_fonts[0];
			Font** end = begin + _fonts.length();
			
			do {
				glyph = _inl_font(*begin)->get_glyph(unicode, region, index, level, vector);
				if ( glyph ) {
					set_glyph(region, index, glyph); return glyph;
				}
				begin++;
			} while (begin != end);
		}
		
		/* TODO 使用一个默认字形  � 65533 */
		Font* font = _pool->_spare_family->font(_style);
		
		glyph = _inl_font(font)->get_glyph(65533, 65533 / 128, 65533 % 128, level, vector);
		
		set_glyph(region, index, glyph);
		
		return glyph;
	}

	FontGlyphTable::~FontGlyphTable() {
		_inl_table(this)->clear_table();
	}

	/**
	* @func glyph
	*/
	FontGlyph* FontGlyphTable::glyph(uint16_t unicode) {
		FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
		if ( glyph ) {
			return glyph;
		}
		return _inl_table(this)->find_glyph(unicode, FontGlyph::LEVEL_NONE, false);
	}

	/**
	* @func use_texture_glyph 使用纹理字型
	*/
	FontGlyph* FontGlyphTable::use_texture_glyph(uint16_t unicode, FGTexureLevel level) {
		ASSERT(level < FontGlyph::LEVEL_NONE);
		
		FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
		
		if ( glyph ) {
			if ( glyph->has_texure_level(level) ) {
				glyph->_container->use_count++; return glyph;
			}
			else { // 先尝试使用最快的方法
				if ( _inl_font(glyph->font())->set_texture_data(glyph, level) ) { //
					glyph->_container->use_count++; return glyph;
				}
			}
		}
		glyph = _inl_table(this)->find_glyph(unicode, level, false);
		
		glyph->_container->use_count++; return glyph;
	}

	/**
	* @func use_vector_glyph # 使用字型,并且载入vbo矢量顶点数据
	*/
	FontGlyph* FontGlyphTable::use_vector_glyph(uint16_t unicode) {
		FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
		
		if ( glyph ) {
			if ( glyph->vertex_data() ) {
				glyph->_container->use_count++; return glyph;
			}
			else { // 先尝试使用最快的方法
				if ( _inl_font(glyph->font())->set_vertex_data(glyph) ) { //
					glyph->_container->use_count++; return glyph;
				}
			}
		}
		glyph = _inl_table(this)->find_glyph(unicode, FontGlyph::LEVEL_NONE, true);
		
		glyph->_container->use_count++; return glyph;
	}
}
