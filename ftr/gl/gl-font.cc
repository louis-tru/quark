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

#include "../gl.h"
#include "../font/_font.h"
#include "../display-port.h"

namespace ftr {

	#if DEBUG
	void test__TESS_CONNECTED_POLYGONS() {
		// TESS_CONNECTED_POLYGONS
		//  Each element in the element array is polygon defined as 'polySize' number of vertex indices,
		//  followed by 'polySize' indices to neighour polygons, that is each element is 'polySize' * 2 indices.
		//  If a polygon has than 'polySize' vertices, the remaining indices are stored as TESS_UNDEF.
		//  If a polygon edge is a boundary, that is, not connected to another polygon, the neighbour index is TESS_UNDEF.
		//  Example, flood fill based on seed polygon:
		//
		// Data visited(new Char[nelems] { 0 }, nelems);
		// TESSindex stack[50];
		// TESSindex start_poly = 0;
		// uint32_t count = 0;

		// while (start_poly < nelems) {
		// 	if ( !visited[start_poly] ) {
		// 		int nstack = 0;
		// 		stack[nstack++] = start_poly;
		// 		visited[start_poly++] = 1;
		// 		while (nstack > 0) {
		// 			TESSindex idx = stack[--nstack];
		// 			const TESSindex* poly = &elems[idx * poly_size * 2];
		// 			const TESSindex* nei = &poly[poly_size];
		// 			LOG("--begin:%d", idx);
		// 			for (int i = 0; i < poly_size; i++) {
		// 				TESSindex vidx = poly[i], eidx = nei[i];
		// 				if (vidx == TESS_UNDEF) break;
		// 				LOG("i:%d,poly:%d", i, poly[i]);
		// 				if (eidx != TESS_UNDEF && !visited[eidx]) {
		// 					LOG("i:%d,nei:%d--", i, eidx);
		// 					stack[nstack++] = eidx;
		// 					visited[eidx] = 1;
		// 				}
		// 			}
		// 		}
		// 		count++;
		// 		LOG("------------------------------count:%d", count);
		// 	} else {
		// 		start_poly++;
		// 	}
		// }
	}
	#endif

	bool GLDraw::set_font_glyph_vertex_data(Font* font, FontGlyph* glyph) {
		
		struct ShortVec2 { int16 x; int16 y; };
		
		if ( ! glyph->_have_outline ) { // 没有轮廓,使用一个空白轮廓
			
			ShortVec2 vertex[3] = { { 0,0 }, { 0,0 }, { 0,0 } };
			
			glGenBuffers(1, &glyph->_vertex_value);
			glBindBuffer(GL_ARRAY_BUFFER, glyph->_vertex_value);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ShortVec2) * 3, vertex, GL_STATIC_DRAW);
			// mark_new_data_size(glyph, sizeof(ShortVec2) * 3);
			
			return true;
		}
		
		// TODO parse vbo data
		FT_Error error = FT_Set_Char_Size((FT_Face)font->_ft_face, 0, 64 * 64, 72, 72);
		ASSERT(!error);
		
		if ( error ) {
			FX_WARN("%s", "parse font glyph vbo data error"); return false;
		}
		
		error = FT_Load_Glyph((FT_Face)font->_ft_face, glyph->glyph_index(),
													FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
		ASSERT( ! error );
		
		if ( error ) {
			FX_WARN("%s", "parse font glyph vbo data error"); return false;
		}
		
		// 字的轮廓信息完全存储在outline中，下面的代码就是分解高阶曲线
		
		TESStesselator* tess = tessNewTess(nullptr);
		
		ScopeClear clear([tess]() { tessDeleteTess(tess); });
		
		FT_Outline_Funcs funcs = {
			Font::Inl::move_to,
			Font::Inl::line_to, Font::Inl::conic_to, Font::Inl::cubic_to, 0, 0
		};
		
		Font::Inl::DecomposeData data = {
			tess,
			Container<Vec2>(256),
			10, // 14, // 曲线采样率,采样率越高所表现的曲线越圆滑
			0,
			0
		};
		
		error = FT_Outline_Decompose(&((FT_GlyphSlot)font->_ft_glyph)->outline, &funcs, &data);
		if ( error ) {
			FX_WARN("%s", "parse font glyph vbo data error"); return false;
		}
		
		tessAddContour(tess, 2, *data.vertex, sizeof(Vec2), data.length);
		
		int poly_size = 3;
		// 转换成gl可以绘制的凸轮廓顶点数据
		if ( ! tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_CONNECTED_POLYGONS, poly_size, 2, 0) ) {
			return false;
		}
		
		const int nelems = tessGetElementCount(tess);
		const TESSindex* elems = tessGetElements(tess);
		const TESSreal* verts = tessGetVertices(tess);
		ArrayBuffer<ShortVec2> vertex(nelems * poly_size);
		
		int vertex_count = 0;
		for (int i = 0; i < nelems; i++) {
			const TESSindex* poly = &elems[i * poly_size * 2];
			for (int j = 0; j < poly_size; j++) {
				int16 x = roundf(*(verts + poly[j] * 2));
				int16 y = roundf(*(verts + poly[j] * 2 + 1));
				vertex[vertex_count] = { x, y };
				vertex_count++;
			}
		}
		
		//  LOG("vertex_length:%d", data.total);
		//  LOG("tess vertex_length:%d, element_count:%d", tessGetVertexCount(*tess), nelems);
		//  LOG("tess total vertex length:%d", vertex_count);
		
		glGenBuffers(1, &glyph->_vertex_value);
		glBindBuffer(GL_ARRAY_BUFFER, glyph->_vertex_value);
		glyph->_vertex_count = vertex_count;
		
		uint32_t size = sizeof(ShortVec2) * vertex_count;
		
		glBufferData(GL_ARRAY_BUFFER, size, *vertex, GL_STATIC_DRAW);
		_inl_font(font)->mark_new_data_size(glyph, size);
		
		return true;
	}

	bool GLDraw::set_font_glyph_texture_data(Font* font, FontGlyph* glyph, int level) {
		
		// @texture_levels_size
		const float texture_levels_size[13] = {
			10, 12, 14, 16, 18, 20, 25, 32, 64, 128, 256, 512
		};
		
		float font_size = texture_levels_size[level] * font->_pool->_display_port_scale;
		
		FT_Error error = FT_Set_Char_Size((FT_Face)font->_ft_face, 0, font_size * 64, 72, 72);
		if (error) {
			FX_WARN("%s", "parse font glyph vbo data error"); return false;
		}
		
		error = FT_Load_Glyph((FT_Face)font->_ft_face, glyph->glyph_index(), FT_LOAD_DEFAULT);
		if (error) {
			FX_WARN("%s", "parse font glyph vbo data error"); return false;
		}
		
		if ( ((FT_GlyphSlot)font->_ft_glyph)->format != FT_GLYPH_FORMAT_BITMAP ) {
			error = FT_Render_Glyph((FT_GlyphSlot)font->_ft_glyph, FT_RENDER_MODE_NORMAL);
			if (error) {
				FX_WARN("%s", "parse font glyph vbo data error"); return false;
			}
		}
		
		FT_GlyphSlot slot = (FT_GlyphSlot)font->_ft_glyph;
		FT_Bitmap bit = slot->bitmap;
		
		GLuint32_t texture_handle;
		glGenTextures(1, &texture_handle);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_handle);
		
		if ( !glIsTexture(texture_handle) ) { return false; }
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		// 纹理重复方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glyph->_textures[level] = texture_handle;
		
		if ( glyph->_have_outline ) { // 有轮廓
			
			glTexImage2D(GL_TEXTURE_2D, 0,
									GL_ALPHA, bit.width, bit.rows, 0,
									GL_ALPHA, GL_UNSIGNED_BYTE, bit.buffer);
			glyph->_texture_size[level] = {
				int16(bit.width),
				int16(bit.rows),
				int16(slot->bitmap_left),
				int16(slot->bitmap_top)
			};
			
			//uint16_t unicode = glyph->unicode();
			//FX_DEBUG("%s, level:%d, width:%d, height:%d, top:%d, left:%d",
			//        &unicode, level, bit.width, bit.rows, (int)slot->bitmap_top, (int)slot->bitmap_left);
			
			_inl_font(font)->mark_new_data_size(glyph, bit.width * bit.rows);
		}
		else { // 没有轮廓, 使用一个透明的空白像素
			uint8_t empty_pixel = 0;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &empty_pixel);
			glyph->_texture_size[level] = { 1, 1, 0, 0 };
			_inl_font(font)->mark_new_data_size(glyph, 1);
		}
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		return true;
	}

}