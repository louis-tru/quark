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

#ifndef __langou__font__
#define __langou__font__

#include "langou/utils/array.h"
#include "langou/utils/map.h"
#include "langou/draw.h"
#include "langou/value.h"

/**
 * @ns langou
 */

XX_NS(langou)

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
class XX_EXPORT FontFamilysID {
 public:
	inline const Array<String>& names() const { return m_names; }
	inline cString& name() const { return m_name; }
	inline uint code() const { return m_code; }
	
 private:
	~FontFamilysID() { }
	
	Array<String>   m_names;
	String          m_name;
	uint            m_code;
	
	XX_DEFINE_INLINE_CLASS(Inl);
	friend class FontPool;
};

/**
 * @class BasicFont
 */
class XX_EXPORT BasicFont: public Object {
 public:
	virtual cString& name() const = 0;
	virtual Font* font(TextStyleEnum style= TextStyleEnum::REGULAR) = 0;
};

/**
 * @class Font 矢量字体
 * 暂时只支持unicode编码中第0平面最常用的编码(0x0000-0xFFFF)
 */
class XX_EXPORT Font: public BasicFont {
	XX_HIDDEN_ALL_COPY(Font);
 protected:

	inline Font() { }
	
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
	inline cString font_name() const { return m_font_name; }
	
	/**
	 * @func num_glyphs
	 */
	inline uint num_glyphs() const { return m_num_glyphs; }
	
	/**
	 * @func family
	 */
	inline FontFamily* family() { return m_font_family; }
	
	/**
	 * @func height
	 */
	inline int height() const { return m_height; }
	
	/**
	 * @func max_advance
	 */
	inline int max_advance() const { return m_max_advance; }
	
	/**
	 * @func ascender
	 */
	inline int ascender() const { return m_ascender; }
	
	/**
	 * @func descender
	 */
	inline int descender() const { return m_descender; }
	
	/**
	 * @func underline_position
	 */
	inline int underline_position() const { return m_underline_position; }
	
	/**
	 * @func underline_thickness
	 */
	inline int underline_thickness() const { return m_underline_thickness; }
	
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
	inline TextStyleEnum style() const { return m_style; }
	
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
	
	FontPool*     m_pool;
	FontFamily*   m_font_family;    // 所属字体家族
	String        m_font_name;      // 字体名称
	TextStyleEnum m_style;    //
	uint          m_num_glyphs;     //
	void*         m_ft_glyph;       /* FT_GlyphSlot */
	int           m_height;         /* text height in 26.6 frac. pixels       */
	int           m_max_advance;    /* max horizontal advance, in 26.6 pixels */
	int           m_ascender;       /* ascender in 26.6 frac. pixels          */
	int           m_descender;      /* descender in 26.6 frac. pixels         */
	int           m_underline_position;
	int           m_underline_thickness;
	GlyphContainer** m_containers;  /* 字型描叙块指针,0x0000-0xFFFF共计512个区 */
	GlyphContainerFlag**  m_flags;
	
 protected:
	uint          m_face_index;   // 当前字体在字体文件中的索引
	void*         m_ft_lib;       /* FT_Library */
	void*         m_ft_face;      /* FT_Face */
	
	XX_DEFINE_INLINE_CLASS(Inl);
	
	friend class FontGlyph;
	friend class FontGlyphTable;
	friend class FontPool;
	friend class GLDraw;
};

/**
 * @class FontGlyph # 字体中一个文字的字型轮廓
 */
class XX_EXPORT FontGlyph {
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
		int16 width;
		int16 height;
		int16 left;
		int16 top;
	};
	
	/**
	 * @func texture
	 */
	inline uint texture_id(TexureLevel level) const { return m_textures[level]; }
	/**
	 * @func texture_size
	 */
	inline TexSize texture_size(TexureLevel level) const {
		return m_texture_size[level];
	}
	/**
	 * @func vertex_data
	 */
	inline uint vertex_data() const { return m_vertex_value; }
	/**
	 * @func vertex_count
	 */
	inline uint16 vertex_count() const { return m_vertex_count; }
	/**
	 * @func glyph_index
	 */
	inline uint16 glyph_index() const { return m_glyph_index; }
	/**
	 * @func unicode
	 */
	inline uint16 unicode() const { return m_unicode; }
	/**
	 * @func hori_bearing_x
	 */
	inline int16 hori_bearing_x() const { return m_hori_bearing_x; }
	/**
	 * @func hori_bearing_y
	 */
	inline uint16 hori_bearing_y() const { return m_hori_bearing_y; }
	/**
	 * @func hori_advance
	 */
	inline uint16 hori_advance() const { return m_hori_advance; }
	/**
	 * @func have_outline
	 */
	inline bool have_outline() const { return m_have_outline; }
	/**
	 * @func has_tex_level
	 */
	inline bool has_texure_level(TexureLevel level) {
		XX_ASSERT(level < LEVEL_NONE);
		return m_textures[level];
	}
	
	/**
	 * @func font
	 */
	Font* font();
	
 private:
	
	typedef Font::GlyphContainer Container;
	
	uint        m_textures[12];       /* 纹理集合 */
	TexSize     m_texture_size[12];   /* 纹理尺寸集合 */
	uint        m_vertex_value;       /* vbo顶点数据 */
	uint16      m_vertex_count;       /* 顶点数量 */
	uint16      m_glyph_index;        /* 字型在字体文件中的索引 */
	uint16      m_unicode;            /* 字型的unicode */
	int16       m_hori_bearing_x;     /* 26.6 frac. 64pt hori_bearing_x */
	uint16      m_hori_bearing_y;     /* 26.6 frac. 64pt hori_bearing_y */
	uint16      m_hori_advance;       /* 26.6 frac. 64pt hori_advance */
	Container*  m_container;          /* 所属容器 */
	bool        m_have_outline;       /* 是否有轮廓 */
	
	friend class Font;
	friend class FontGlyphTable;
	friend class GLDraw;
};

/**
 * @class FontGlyphTable
 */
class XX_EXPORT FontGlyphTable: public Object {
 public:
	
	virtual ~FontGlyphTable();
	
	/**
	 * @func id {cFFID}
	 */
	inline cFFID id() const { return m_ffid; }
	
	/**
	 * @func count {uint}
	 */
	inline uint count() const { return m_fonts.length(); }
	
	/**
	 * @func style {TextStyleEnum}
	 */
	inline TextStyleEnum style() const { return m_style; }
	
	/**
	 * @func text_height {int} # 字体列表中最大text-height
	 */
	inline int text_height() const { return m_height; }
	
	/**
	 * @func ascender {int} # 字体列表中最大ascender
	 */
	inline int max_ascender() const { return m_ascender; }
	
	/**
	 * @func descender {int} # 字体列表中最大descender
	 */
	inline int max_descender() const { return m_descender; }
	
	/**
	 * @func glyph
	 */
	FontGlyph* glyph(uint16 unicode);
	
	/**
	 * @func use_texture_glyph # 使用纹理字型
	 */
	FontGlyph* use_texture_glyph(uint16 unicode, FontGlyph::TexureLevel level);
	
	/**
	 * @func use_vector_glyph # 使用矢量顶点字型
	 */
	FontGlyph* use_vector_glyph(uint16 unicode);
	
 private:
	
	struct GlyphsBlock;
	
	FontPool*     m_pool;
	GlyphsBlock*  m_blocks[512];
	Array<Font*>  m_fonts;
	cFFID         m_ffid;
	TextStyleEnum m_style;
	int           m_height;
	int           m_ascender;
	int           m_descender;
	
	XX_DEFINE_INLINE_CLASS(Inl);
	
	friend class FontPool;
};

/**
 * @class FontFamily 字体家族
 */
class XX_EXPORT FontFamily: public BasicFont {
	XX_HIDDEN_ALL_COPY(FontFamily);
 public:
	
	/**
	 * @overwrite
	 */
	virtual cString& name() const;
	virtual Font* font(TextStyleEnum style = TextStyleEnum::REGULAR);
	
	/**
	 * @func family_name
	 */
	inline cString family_name() const { return m_family_name; }
	
	/**
	 * @func font_names
	 */
	Array<String> font_names() const;
	
	/**
	 * @func num_fonts
	 */
	inline uint num_fonts() const { return m_all_fonts.length(); }
	
 private:
	
	FontFamily(cString& family_name);
	
	String        m_family_name;
	Font*         m_fonts[19];
	Array<Font*>  m_all_fonts;
	
	XX_DEFINE_INLINE_CLASS(Inl);
	friend class FontPool;
};

/**
 * @class FontPool 加载与管理所有字体、字型、字型表, 并在系统内存不足时能自动清理使用频率不高的字型数据
 */
class XX_EXPORT FontPool: public Object {
	XX_HIDDEN_ALL_COPY(FontPool);
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
	
	struct XX_EXPORT SimpleFont {
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
	
	struct XX_EXPORT SimpleFontFamily {
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
	
	XX_DEFINE_INLINE_CLASS(Inl);
	
	friend class Font;
	friend class FontGlyphTable;
	friend class GUIApplication;
	friend class Draw;
	friend class GLDraw;
};

XX_END
#endif
