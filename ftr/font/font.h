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

#ifndef __ftr__font__font__
#define __ftr__font__font__

#include "../draw.h"
#include "../value.h"

namespace ftr {

	class GUIApplication;
	class DisplayPort;
	class Texture;
	class FontFamilysID;
	class FontGlyph;
	class FontGlyphTable;
	class Font;
	class FontFamily;
	class FontPool;

	typedef const FontFamilysID* cFFID;

	/**
	* @class FontFamilysID
	*/
	class FX_EXPORT FontFamilysID {
		public:
		inline const Array<String>& names() const { return _names; }
		inline cString& name() const { return _name; }
		inline uint32_t code() const { return _code; }
		
		private:
		~FontFamilysID() {}
		
		Array<String> _names;
		String              _name;
		uint32_t            _code;
		
		FX_DEFINE_INLINE_CLASS(Inl);
		friend class FontPool;
	};

	/**
	* @class BaseFont
	*/
	class FX_EXPORT BaseFont: public Object {
		public:
		virtual cString& name() const = 0;
		virtual Font* font(TextStyleEnum style = TextStyleEnum::REGULAR) = 0;
	};

	/**
	* @class Font 矢量字体
	* 暂时只支持unicode编码中第0平面最常用的编码(0x0000-0xFFFF)
	*/
	class FX_EXPORT Font: public BaseFont {
		FX_HIDDEN_ALL_COPY(Font);
		protected:

		inline Font() {}
		
		/**
		* @destructor
		*/
		virtual ~Font();
		
		public:
		/**
		* @overwrite
		*/
		virtual cString& name() const;
		virtual Font* font(TextStyleEnum style = TextStyleEnum::REGULAR);
		
		/**
		* @func font_name
		*/
		inline cString font_name() const { return _font_name; }
		
		/**
		* @func num_glyphs
		*/
		inline uint32_t num_glyphs() const { return _num_glyphs; }
		
		/**
		* @func family
		*/
		inline FontFamily* family() { return _font_family; }
		
		/**
		* @func height
		*/
		inline int height() const { return _height; }
		
		/**
		* @func max_advance
		*/
		inline int max_advance() const { return _max_advance; }
		
		/**
		* @func ascender
		*/
		inline int ascender() const { return _ascender; }
		
		/**
		* @func descender
		*/
		inline int descender() const { return _descender; }
		
		/**
		* @func underline_position
		*/
		inline int underline_position() const { return _underline_position; }
		
		/**
		* @func underline_thickness
		*/
		inline int underline_thickness() const { return _underline_thickness; }
		
		/**
		* @func load
		*/
		bool load();
		
		/**
		* @func unload
		*/
		void unload();
		
		/**
		* @func style
		*/
		inline TextStyleEnum style() const { return _style; }
		
		private:
		
		/**
		* @func install
		*/
		virtual void install() = 0;
		
		/*
		* 一个块容器包含128个字符
		*/
		struct GlyphContainerFlag;
		struct GlyphContainer;
		
		FontPool*     _pool;
		FontFamily*   _font_family;    // 所属字体家族
		String        _font_name;      // 字体名称
		TextStyleEnum _style;    //
		uint32_t      _num_glyphs;     //
		void*         _ft_glyph;       /* FT_GlyphSlot */
		int           _height;         /* text height in 26.6 frac. pixels       */
		int           _max_advance;    /* max horizontal advance, in 26.6 pixels */
		int           _ascender;       /* ascender in 26.6 frac. pixels          */
		int           _descender;      /* descender in 26.6 frac. pixels         */
		int           _underline_position;
		int           _underline_thickness;
		GlyphContainer** _containers;  /* 字型描叙块指针,0x0000-0xFFFF共计512个区 */
		GlyphContainerFlag**  _flags;
		
		protected:
		uint32_t      _face_index;   // 当前字体在字体文件中的索引
		void*         _ft_lib;       /* FT_Library */
		void*         _ft_face;      /* FT_Face */
		
		FX_DEFINE_INLINE_CLASS(Inl);
		
		friend class FontGlyph;
		friend class FontGlyphTable;
		friend class FontPool;
		friend class GLDraw;
	};

	/**
	* @class FontGlyph # 字体中一个文字的字型轮廓
	*/
	class FX_EXPORT FontGlyph {
		public:
		
		/**
		* @enum TexureLevel
		* 字形纹理分为12个等级,绘制尺寸超过这个等级或超过屏幕的1/10使用顶点进行绘制
		* 10/12/14/16/18/20/26/32/64/128/256/512
		*/
		enum TexureLevel {
			LEVEL_0 = 0,   //  0-10
			LEVEL_1,       // 10-12
			LEVEL_2,       // 12-14
			LEVEL_3,       // 14-16
			LEVEL_4,       // 16-18
			LEVEL_5,       // 18-20
			LEVEL_6,       // 20-26
			LEVEL_7,       // 26-32
			LEVEL_8,       // 32-64
			LEVEL_9,       // 64-128
			LEVEL_10,      // 128-256
			LEVEL_11,      // 256-512
			LEVEL_NONE
		};
		
		struct TexSize {
			int16_t width;
			int16_t height;
			int16_t left;
			int16_t top;
		};
		
		/**
		* @func texture
		*/
		inline uint32_t texture_id(TexureLevel level) const { return _textures[level]; }
		/**
		* @func texture_size
		*/
		inline TexSize texture_size(TexureLevel level) const {
			return _texture_size[level];
		}
		/**
		* @func vertex_data
		*/
		inline uint32_t vertex_data() const { return _vertex_value; }
		/**
		* @func vertex_count
		*/
		inline uint16_t vertex_count() const { return _vertex_count; }
		/**
		* @func glyph_index
		*/
		inline uint16_t glyph_index() const { return _glyph_index; }
		/**
		* @func unicode
		*/
		inline uint16_t unicode() const { return _unicode; }
		/**
		* @func hori_bearing_x
		*/
		inline int16_t hori_bearing_x() const { return _hori_bearing_x; }
		/**
		* @func hori_bearing_y
		*/
		inline uint16_t hori_bearing_y() const { return _hori_bearing_y; }
		/**
		* @func hori_advance
		*/
		inline uint16_t hori_advance() const { return _hori_advance; }
		/**
		* @func have_outline
		*/
		inline bool have_outline() const { return _have_outline; }
		/**
		* @func has_tex_level
		*/
		inline bool has_texure_level(TexureLevel level) {
			ASSERT(level < LEVEL_NONE);
			return _textures[level];
		}
		
		/**
		* @func font
		*/
		Font* font();
		
		private:
		
		typedef Font::GlyphContainer Container;
		
		uint32_t        _textures[12];       /* 纹理集合 */
		TexSize     _texture_size[12];   /* 纹理尺寸集合 */
		uint32_t        _vertex_value;       /* vbo顶点数据 */
		uint16_t      _vertex_count;       /* 顶点数量 */
		uint16_t      _glyph_index;        /* 字型在字体文件中的索引 */
		uint16_t      _unicode;            /* 字型的unicode */
		int16_t       _hori_bearing_x;     /* 26.6 frac. 64pt hori_bearing_x */
		uint16_t      _hori_bearing_y;     /* 26.6 frac. 64pt hori_bearing_y */
		uint16_t      _hori_advance;       /* 26.6 frac. 64pt hori_advance */
		Container*  _container;          /* 所属容器 */
		bool        _have_outline;       /* 是否有轮廓 */
		
		friend class Font;
		friend class FontGlyphTable;
		friend class GLDraw;
	};

	/**
	* @class FontGlyphTable
	*/
	class FX_EXPORT FontGlyphTable: public Object {
		public:
		
		virtual ~FontGlyphTable();
		
		/**
		* @func id {cFFID}
		*/
		inline cFFID id() const { return _ffid; }
		
		/**
		* @func count {uint}
		*/
		inline uint32_t count() const { return (uint32_t)_fonts.size(); }
		
		/**
		* @func style {TextStyleEnum}
		*/
		inline TextStyleEnum style() const { return _style; }
		
		/**
		* @func text_height {int} # 字体列表中最大text-height
		*/
		inline int text_height() const { return _height; }
		
		/**
		* @func ascender {int} # 字体列表中最大ascender
		*/
		inline int max_ascender() const { return _ascender; }
		
		/**
		* @func descender {int} # 字体列表中最大descender
		*/
		inline int max_descender() const { return _descender; }
		
		/**
		* @func glyph
		*/
		FontGlyph* glyph(uint16_t unicode);
		
		/**
		* @func use_texture_glyph # 使用纹理字型
		*/
		FontGlyph* use_texture_glyph(uint16_t unicode, FontGlyph::TexureLevel level);
		
		/**
		* @func use_vector_glyph # 使用矢量顶点字型
		*/
		FontGlyph* use_vector_glyph(uint16_t unicode);
		
		private:
		
		struct GlyphsBlock;
		
		FontPool*     _pool;
		GlyphsBlock*  _blocks[512];
		Array<Font*>  _fonts;
		cFFID         _ffid;
		TextStyleEnum _style;
		int _height, _ascender, _descender;
		
		FX_DEFINE_INLINE_CLASS(Inl);
		
		friend class FontPool;
	};

	/**
	* @class FontFamily 字体家族
	*/
	class FX_EXPORT FontFamily: public BaseFont {
		FX_HIDDEN_ALL_COPY(FontFamily);
		public:
		
		/**
		* @overwrite
		*/
		virtual cString& name() const;
		virtual Font* font(TextStyleEnum style = TextStyleEnum::REGULAR);
		
		/**
		* @func family_name
		*/
		inline cString family_name() const { return _family_name; }
		
		/**
		* @func font_names
		*/
		Array<String> font_names() const;
		
		/**
		* @func num_fonts
		*/
		inline uint32_t num_fonts() const { return (uint32_t)_all_fonts.size(); }
		
		private:
		
		FontFamily(cString& family_name);
		
		String        _family_name;
		Font*         _fonts[19];
		Array<Font*>  _all_fonts;
		
		FX_DEFINE_INLINE_CLASS(Inl);
		friend class FontPool;
	};

}
#endif
