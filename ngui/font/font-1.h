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

#ifndef __ngui__font_1__
#define __ngui__font_1__

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>
#include <tesselator.h>
#include "nxutils/fs.h"
#include "ngui/font.h"
#include "ngui/bezier.h"
#include "ngui/draw.h"

XX_NS(ngui)

typedef FontGlyph::TexureLevel FGTexureLevel;

enum ContainerFlag: char {
	CF_NO_READY = 0,
	CF_READY,
	CF_NONE,
};

struct Font::GlyphContainerFlag {
	ContainerFlag flags[128];
};

struct Font::GlyphContainer {
	Font*       font;
	uint64      use_count;  /* 字型块使用次数,值越大使用越频繁 */
	uint64      data_size;  /* 数据容量 */
	FontGlyph   glyphs[128];
};

struct FontGlyphTable::GlyphsBlock {
	FontGlyph* glyphs[128];
};

struct FontGlyphTextureLevel {
	FGTexureLevel level;
	uint min_font_size;
	uint max_font_size;
};

// static data
extern FontGlyphTextureLevel font_glyph_texture_levels_idx[513];

/**
 * @class Font::Inl
 */
class Font::Inl : public Font {
public:
#define _inl_font(self) static_cast<Font::Inl*>(self)
	
	void initialize(
		FontPool* pool,
		FontFamily* family,
		String font_name,
		TextStyleEnum style,
		uint num_glyphs,
		uint face_index,
		int height,       /* text height in 26.6 frac. pixels       */
		int max_advance,  /* max horizontal advance, in 26.6 pixels */
		int ascender,     /* ascender in 26.6 frac. pixels          */
		int descender,    /* descender in 26.6 frac. pixels         */
		int underline_position,
		int underline_thickness, FT_Library lib
	);

	/**
	 * @struct DecomposeData
	 */
	struct DecomposeData {
		TESStesselator* tess;
		Container<Vec2> vertex;
		int  sample;
		uint length;
		uint total;
		Vec2 p0;
		/**
		 * @func push_vertex
		 */
		inline Vec2* push_vertex(uint count) {
			uint len = length + count;
			if (len > vertex.capacity())
				vertex.realloc(len);
			Vec2* rev = *vertex + length;
			length = len;
			total += count;
			return rev;
		}
		// @end
	};
	
	/**
	 * @func move_to # 新的多边形开始
	 */
	static int move_to(const FT_Vector* to, void* user);
	
	/**
	 * @func line_to # 一次贝塞尔点（直线）
	 */
	static int line_to(const FT_Vector* to, void* user);
	
	/**
	 * @func line_to # 二次贝塞尔曲线点,转换到一次塞尔点
	 */
	static int conic_to(const FT_Vector* control, const FT_Vector* to, void* user);
	
	/**
	 * @func line_to # 三次贝塞尔曲线点,转换到一次塞尔点
	 */
	static int cubic_to(const FT_Vector* control1,
											const FT_Vector* control2,
											const FT_Vector* to, void* user);
	
	/**
	 * @func mark_new_data_size
	 */
	inline void mark_new_data_size(FontGlyph* glyph, uint size) {
		glyph->m_container->data_size += size;
		m_pool->m_total_data_size += size;
	}
	
	/**
	 * @func del_glyph_data 仅只删除纹理数据与顶点数据,
	 */
	void del_glyph_data(FontGlyph::Container* block);
	
	/**
	 * @func clear 
	 * 在调用这个方法后此前通过任何途径获取到的字型数据将不能再使用
	 * 包括从FontGlyphTable获取到的字型,调用FontGlyphTable::Inl.clear_table()可清理引用
	 */
	void clear(bool full);
	
	/**
	 * @func set_vertex_data
	 */
	inline bool set_vertex_data( FontGlyph* glyph ) {
		return m_pool->m_draw_ctx->set_font_glyph_vertex_data(this, glyph);
	}
	
	/**
	 * @func set_texture_data
	 */
	inline bool set_texture_data(FontGlyph* glyph, FGTexureLevel level) {
		return m_pool->m_draw_ctx->set_font_glyph_texture_data(this, glyph, level);
	}
	
	/**
	 * @func get_glyph
	 */
	FontGlyph* get_glyph(uint16 unicode, uint region, uint index, FGTexureLevel level, bool vector);
	
};

XX_END
#endif
