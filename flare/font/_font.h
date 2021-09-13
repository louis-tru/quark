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

#ifndef __flare__font_1__
#define __flare__font_1__

#include "../util/fs.h"
#include "./font.h"
#include "../bezier.h"
// #include "../draw.h"
#include "./pool.h"
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>
#include <tesselator.h>
#include "../util/dict.h"

namespace flare {

	typedef FontGlyph::TexureLevel FGTexureLevel;

	enum ContainerFlag: Char {
		CF_NO_READY = 0,
		CF_READY,
		CF_NONE,
	};

	struct Font::GlyphContainerFlag {
		ContainerFlag flags[128];
	};

	struct Font::GlyphContainer {
		Font*       font;
		uint64_t    use_count;  /* 字型块使用次数,值越大使用越频繁 */
		uint64_t    data_size;  /* 数据容量 */
		FontGlyph   glyphs[128];
	};

	struct FontGlyphTable::GlyphsBlock {
		FontGlyph* glyphs[128];
	};

	struct FontGlyphTextureLevel {
		FGTexureLevel level;
		uint32_t min_font_size;
		uint32_t max_font_size;
	};

	// static data
	extern FontGlyphTextureLevel font_glyph_texture_levels_idx[513];

	/**
	 * @class FontFromData
	 */
	class FontFromData: public Font {
	public:
		class Data: public Reference {
		public:
			Data(Buffer& buff);
			FT_Byte*  value;
			uint32_t  length;
			private:
			Buffer    _storage;
		};
		FontFromData(Data* data);
		virtual ~FontFromData();
		void install();
	private:
		Data* _data;
	};

	/**
	* @class FontFromFile
	*/
	class FontFromFile: public Font {
	public:
		FontFromFile(cString& path);
		void install();
	private:
		String _font_path; // 字体文件路径
	};

	/**
	 * @class Font::Inl
	 */
	FX_DEFINE_INLINE_MEMBERS(Font, Inl) {
	public:
		#define _inl_font(self) static_cast<Font::Inl*>(self)
		
		void initialize(
			FontPool* pool,
			FontFamily* family,
			String font_name,
			TextStyleValue style,
			uint32_t num_glyphs,
			uint32_t face_index,
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
			Array<Vec2> vertex;
			int  sample;
			uint32_t length;
			uint32_t total;
			Vec2 p0;
			/**
			* @func push_vertex
			*/
			inline Vec2* push_vertex(uint32_t count) {
				uint32_t len = length + count;
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
		inline void mark_new_data_size(FontGlyph* glyph, uint32_t size) {
			glyph->_container->data_size += size;
			_pool->_total_data_size += size;
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
			// return _pool->_draw_ctx->set_font_glyph_vertex_data(this, glyph);
			// TODO ...
			return true;
		}
		
		/**
		* @func set_texture_data
		*/
		inline bool set_texture_data(FontGlyph* glyph, FGTexureLevel level) {
			// return _pool->_draw_ctx->set_font_glyph_texture_data(this, glyph, level);
			// TODO ...
			return true;
		}
		
		/**
		* @func get_glyph
		*/
		FontGlyph* get_glyph(uint16_t unicode, uint32_t region, uint32_t index, FGTexureLevel level, bool vector);
		
	};

	FX_DEFINE_INLINE_MEMBERS(FontFamilysID, Inl) {
	public:
		#define _inl_ff_id(self) static_cast<FontFamilysID::Inl*>(self)
		void initialize(const Array<String>& names);
	};

	FX_DEFINE_INLINE_MEMBERS(FontGlyphTable, Inl) {
	public:
		#define _inl_table(self) static_cast<FontGlyphTable::Inl*>(self)
		void clear_table();
		void initialize(FFID ffid, TextStyleValue style, FontPool* pool);
		void make();
		void set_glyph(uint32_t region, uint32_t index, FontGlyph* glyph);
		FontGlyph* get_glyph(uint16_t unicode);
		FontGlyph* find_glyph(uint16_t unicode, FGTexureLevel level, bool vector);
	};

	FX_DEFINE_INLINE_MEMBERS(FontFamily, Inl) {
	public:
		#define _inl_family(self) static_cast<FontFamily::Inl*>(self)
		int get_font_style_index(TextStyleValue style);
		void add_font(Font* font);
	};

	FX_DEFINE_INLINE_MEMBERS(FontPool, Inl) {
	public:
		#define _inl_pool(self) static_cast<FontPool::Inl*>(self)
		void initialize_default_fonts();
		bool register_font(
			cString& family_name,
			cString& font_name,
			TextStyleValue style,
			uint32_t num_glyphs,
			uint32_t face_index,
			int  height,       /* text height in 26.6 frac. pixels       */
			int  max_advance,  /* max horizontal advance, in 26.6 pixels */
			int  ascender,     /* ascender in 26.6 frac. pixels          */
			int  descender,    /* descender in 26.6 frac. pixels         */
			int  underline_position,
			int  underline_thickness,
			cString& path,
			FontFromData::Data* data
		);
		bool register_font(FontFromData::Data* font_data, cString& family_alias);
		void display_port_change_handle(Event<>& evt);
		static bool has_italic_style(cString& style_name);
		static TextStyleValue parse_style_flags(cString& name, cString& style_name);
		static Handle<FontPool::SimpleFontFamily> inl_read_font_file(cString& path, FT_Library lib);
	};

}
#endif
