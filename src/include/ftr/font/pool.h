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

#ifndef __ftr__font__pool__
#define __ftr__font__pool__

namespace ftr {

	class Draw;
	class BasicFont;
	class FontGlyphTable;
	class Font;

	/**
	* @class FontPool 加载与管理所有字体、字型、字型表, 并在系统内存不足时能自动清理使用频率不高的字型数据
	*/
	class FX_EXPORT FontPool: public Object {
		FX_HIDDEN_ALL_COPY(FontPool);
		public:
		
		FontPool(Draw* ctx);
		
		/**
		* @destructor
		*/
		virtual ~FontPool();
		
		/**
		* @func set_default_fonts 尝试设置默认字体
		* @arg first {const Array<String>*}  第一字体列表
		* @arg ... {const Array<String>*} 第2/3/4..字体列表
		*/
		void set_default_fonts(const Array<String>* first, ...);
		
		/**
		* @func set_default_fonts 在当前字体库找到字体名称,设置才能生效
		* @arg fonts {const Array<String>&} 要设置的默认字体的名称
		*/
		void set_default_fonts(const Array<String>& fonts);
		
		/**
		* @func default_font_names
		*/
		Array<String> default_font_names() const;
		
		/**
		* @func font_familys
		*/
		inline Array<String> family_names() const { return m_familys.keys(); }
		
		/**
		* @func font_names
		*/
		Array<String> font_names(cString& family_name) const;
		
		/**
		* @func get_font_family
		*/
		FontFamily* get_font_family(cString& family_name);
		
		/**
		* @fucn test 测试是否有字体或家族
		*/
		inline bool test(cString& font) { return m_blend_fonts.has(font); }
		
		/**
		* @func get_font 通过名称获得一个字体对像
		* @arg name {cString&} 字体名称或家族名称
		*/
		Font* get_font(cString& font, TextStyleEnum style = TextStyleEnum::REGULAR);
		
		/**
		* @func get_group 通过id获取字型集合表
		* @arg id {cFFID} 组id
		* @arg [style = fs_regular] {Font::TextStyle} # 使用的字体家族才生效
		*/
		FontGlyphTable* get_table(cFFID id, TextStyleEnum style = TextStyleEnum::REGULAR);
		
		/**
		* @func get_group 获取默认字型集合表
		*/
		FontGlyphTable* get_table(TextStyleEnum style = TextStyleEnum::REGULAR);
		
		/**
		* @func register_font 通过Buffer数据注册字体
		* @arg buff {Buffer} 字体数据
		* @arg [family_alias = String()] {cString&} 给所属家族添加一个别名
		* @ret {bool}
		*/
		bool register_font(Buffer buff, cString& family_alias = String());
		
		/**
		* @func register_font_file 注册本地字体文件
		* @arg path {cString&} 字体文件的本地路径
		* @arg [family_alias = String()] {cString&} # 给所属家族添加一个别名
		* @ret {bool}
		*/
		bool register_font_file(cString& path, cString& family_alias = String());
		
		/**
		* @func set_family_alias 设置家族别名
		*/
		void set_family_alias(cString& family, cString& alias);
		
		/**
		* @func clear 释放池中不用的字体数据,一般会由系统自动调用
		* @arg [full = false] {bool} 全面清理资源尽可能最大程度清理
		*/
		void clear(bool full = false);
		
		/**
		* @func get_glyph_texture_level 通过字体尺寸获取纹理等级,与纹理大小font_size
		*/
		FontGlyph::TexureLevel get_glyph_texture_level(float& font_size_out);
		
		/**
		* @func get_family_name(path) by font file path
		*/
		String get_family_name(cString& path) const;
		
		/**
		* @func get_glyph_texture_size 通过等级大小获取字型纹理大小
		*/
		static float get_glyph_texture_size(FontGlyph::TexureLevel level);
		
		/**
		* @func get_font_familys_id
		*/
		static cFFID get_font_familys_id(const Array<String> fonts);
		
		/**
		* @func get_font_familys_id
		*/
		static cFFID get_font_familys_id(cString fonts);
		
		struct FX_EXPORT SimpleFont {
			String  name;
			TextStyleEnum style;
			uint    num_glyphs;
			int     height;       /* text height in 26.6 frac. pixels       */
			int     max_advance;  /* max horizontal advance, in 26.6 pixels */
			int     ascender;     /* ascender in 26.6 frac. pixels          */
			int     descender;    /* descender in 26.6 frac. pixels         */
			int     underline_position;
			int     underline_thickness;
		};
		
		struct FX_EXPORT SimpleFontFamily {
			typedef NonObjectTraits Traits;
			String path;
			String family;
			Array<SimpleFont> fonts;
		};
		
		/**
		* @func read_font_file
		*/
		static Handle<SimpleFontFamily> read_font_file(cString& path);
		
		/**
		* @func system_font_family
		*/
		static const Array<SimpleFontFamily>& system_font_family();
		
		private:
		
		/**
		* @func set_display_port
		*/
		void set_display_port(DisplayPort* display_port);
		
		void*                       m_ft_lib;     /* FT_Library */
		Map<String, BasicFont*>     m_blend_fonts;/* 所有的家族与字体包括别名 */
		Map<String, FontFamily*>    m_familys;    /* 所有的字体家族 */
		Map<String, Font*>          m_fonts;      /* 所有的字体 */
		Map<uint, FontGlyphTable*>  m_tables;     /* 所有的字型表 */
		Map<String, String>         m_paths;      /* 所有的字体路径 */
		Array<BasicFont*>           m_default_fonts;
		FontFamily*                 m_spare_family;     /* 备用字体家族 */
		Draw*                       m_draw_ctx;
		DisplayPort*                m_display_port;
		uint64                      m_total_data_size; /* 总数据尺寸 */
		float                       m_max_glyph_texture_size; /* 纹理绘制的最大限制,超过这个size使用顶点进行绘制 */
		float                       m_display_port_scale;
		
		FX_DEFINE_INLINE_CLASS(Inl);
		
		friend class Font;
		friend class FontGlyphTable;
		friend class GUIApplication;
		friend class Draw;
		friend class GLDraw;
	};

}
#endif