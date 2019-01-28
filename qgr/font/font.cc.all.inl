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

XX_NS(qgr)

/**
 * @get font {Font*}
 */
inline Font* FontGlyph::font() {
	return m_container->font;
}

void Font::Inl::initialize(
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
	int underline_thickness,
	FT_Library lib
) {
	m_pool = pool;
	m_font_family = family;
	m_font_name = font_name;
	m_style = style;
	m_num_glyphs = num_glyphs;
	m_ft_glyph = nullptr;
	m_face_index = face_index;
	m_ft_lib = lib;
	m_ft_face = nullptr;
	m_descender = 0;
	m_height = 0;
	m_max_advance = 0;
	m_ascender = 0;
	m_containers = nullptr;
	m_flags = nullptr;
	m_height = height;
	m_max_advance = max_advance;
	m_ascender = ascender;
	m_descender = descender;
	m_underline_position = underline_position;
	m_underline_thickness = underline_thickness;
}

/**
 * @func move_to # 新的多边形开始
 */
int Font::Inl::move_to(const FT_Vector* to, void* user) {
	DecomposeData* data = static_cast<DecomposeData*>(user);
	if (data->length) { // 添加一个多边形
		tessAddContour(data->tess, 2, *data->vertex, sizeof(Vec2), data->length);
		data->length = 0;
	}
	//    LOG("move_to:%d,%d", to->x, to->y);
	
	Vec2 vertex = Vec2(to->x, -to->y);
	*data->push_vertex(1) = vertex;
	data->p0 = vertex;
	return FT_Err_Ok;
}

/**
 * @func line_to # 一次贝塞尔点（直线）
 */
int Font::Inl::line_to(const FT_Vector* to, void* user) {
	DecomposeData* data = static_cast<DecomposeData*>(user);
	Vec2 vertex = Vec2(to->x, -to->y);
	//  LOG("line_to:%d,%d", to->x, to->y);
	*data->push_vertex(1) = vertex;
	data->p0 = vertex;
	return FT_Err_Ok;
}

/**
 * @func line_to # 二次贝塞尔曲线点,转换到一次塞尔点
 */
int Font::Inl::conic_to(const FT_Vector* control, const FT_Vector* to, void* user) {
	DecomposeData* data = static_cast<DecomposeData*>(user);
	Vec2 p2 = Vec2(to->x, -to->y);
	//  LOG("conic_to:%d,%d|%d,%d", control->x, control->y, to->x, to->y);
	QuadraticBezier bezier(data->p0, Vec2(control->x, -control->y), p2);
	// 使用10点采样,采样越多越能还原曲线,但需要更多有存储空间
	bezier.sample_curve_points(data->sample, (float*)(data->push_vertex(data->sample - 1) - 1));
	data->p0 = p2;
	return FT_Err_Ok;
}

/**
 * @func line_to # 三次贝塞尔曲线点,转换到一次塞尔点
 */
int Font::Inl::cubic_to(const FT_Vector* control1,
												const FT_Vector* control2,
												const FT_Vector* to, void* user) 
{
	DecomposeData* data = static_cast<DecomposeData*>(user);
	Vec2 p3 = Vec2(to->x, -to->y);
	//  LOG("cubic_to:%d,%d|%d,%d|%d,%d",
	//        control1->x, control1->y, control2->x, control2->y, to->x, to->y);
	CubicBezier bezier(data->p0,
										 Vec2(control1->x, -control1->y),
										 Vec2(control2->x, -control2->y), p3);
	bezier.sample_curve_points(data->sample, (float*)(data->push_vertex(data->sample - 1) - 1));
	data->p0 = p3;
	return FT_Err_Ok;
}

/**
 * @func del_glyph_data
 */
void Font::Inl::del_glyph_data(GlyphContainer* container) {
	
	if ( container ) {
		FontGlyph* glyph = container->glyphs;
		auto ctx = m_pool->m_draw_ctx;
		
		for ( int i = 0; i < 128; i++, glyph++ ) {
			if ( glyph->m_vertex_value ) {
				ctx->del_buffer( glyph->m_vertex_value );
				glyph->m_vertex_value = 0;
				glyph->m_vertex_count = 0;
			}
			
			for ( int i = 1; i < 12; i++ ) {
				if ( glyph->m_textures[i] ) { // 删除纹理
					ctx->del_texture( glyph->m_textures[i] );
				}
			}
			memset(glyph->m_textures, 0, sizeof(uint) * 13);
		}
		m_pool->m_total_data_size -= container->data_size;
		container->use_count = 0;
		container->data_size = 0;
	}
}

/**
 * @func get_glyph
 */
FontGlyph* Font::Inl::get_glyph(uint16 unicode, uint region,
																uint index, FGTexureLevel level, bool vector) 
{
	XX_ASSERT(region < 512);
	XX_ASSERT(index < 128);
	
	load(); XX_ASSERT(m_ft_face);
	
	GlyphContainerFlag* flags = m_flags[region];
	
	if ( !flags ) {
		flags = new GlyphContainerFlag();
		m_flags[region] = flags;
		memset(flags, 0, sizeof(GlyphContainerFlag));
	}
	
	FontGlyph* glyph = nullptr;
	
	switch ( flags->flags[index] ) {
		default:
		{
			return nullptr;
		}
		case CF_NO_READY:
		{
			uint16 glyph_index = FT_Get_Char_Index( (FT_Face)m_ft_face, unicode );
			
			if (! glyph_index ) goto cf_none;
			
			GlyphContainer* container = m_containers[region];
			if ( !container ) {
				m_containers[region] = container = new GlyphContainer();
				memset(container, 0, sizeof(GlyphContainer));
				container->font = this;
			}
			
			FT_Error error = FT_Set_Char_Size( (FT_Face)m_ft_face, 0, 64 * 64, 72, 72);
			if (error) {
				XX_WARN("%s", "parse font glyph vbo data error"); goto cf_none;
			}
			
			error = FT_Load_Glyph( (FT_Face)m_ft_face, glyph_index, FT_LOAD_NO_HINTING);
			if (error) {
				XX_WARN("%s", "parse font glyph vbo data error"); goto cf_none;
			}
		
			FT_GlyphSlot ft_glyph = (FT_GlyphSlot)m_ft_glyph;
			
			glyph = container->glyphs + index;
			glyph->m_container = container;
			glyph->m_unicode = unicode;
			glyph->m_glyph_index = glyph_index;
			glyph->m_hori_bearing_x = ft_glyph->metrics.horiBearingX;
			glyph->m_hori_bearing_y = ft_glyph->metrics.horiBearingY;
			glyph->m_hori_advance = ft_glyph->metrics.horiAdvance;
			glyph->m_have_outline = ft_glyph->outline.points;
			
			if (vector) {
				if ( ! set_vertex_data(glyph) ) {
					goto cf_none;
				}
			} else if ( FontGlyph::LEVEL_NONE != level ) {
				if ( ! set_texture_data(glyph, level) ) {
					goto cf_none;
				}
			}
			
			flags->flags[index] = CF_READY;
		}
		case CF_READY:
		{
			glyph = m_containers[region]->glyphs + index;
			
			if (vector) {
				if ( ! glyph->vertex_data() ) {
					if ( ! set_vertex_data(glyph) ) {
						goto cf_none;
					}
				}
			} else if ( FontGlyph::LEVEL_NONE != level ) {
				if ( ! glyph->has_texure_level(level) ) {
					if ( ! set_texture_data(glyph, level) ) {
						goto cf_none;
					}
				}
			}
			break;
		}
		// switch end
	}
	
	return glyph;
	
 cf_none:
	flags->flags[index] = CF_NONE;
	return nullptr;
}

/**
 * @func clear
 * 在调用这个方法后此前通过任何途径获取到的字型数据将不能再使用
 * 包括从FontGlyphTable获取到的字型,调用FontGlyphTable::Inl.clear_table()可清理引用
 */
void Font::Inl::clear(bool full) {
	if ( m_ft_face ) {
		if ( full ) { // 完全清理
			
			for (int i = 0; i < 512; i++) {
				del_glyph_data( m_containers[i] );
				delete m_containers[i]; m_containers[i] = nullptr;
				delete m_flags[i]; m_flags[i] = nullptr;
			}
			delete m_containers; m_containers = nullptr;
			delete m_flags; m_flags = nullptr;
			
			FT_Done_Face((FT_Face)m_ft_face);
			m_ft_face = nullptr;
			m_ft_glyph = nullptr;
			
		} else {
			struct Container {
				GlyphContainer* container;
				int region;
				uint64 use_count;
			};
			uint64 total_data_size = 0;
			List<Container> containers_sort;
			// 按使用使用次数排序
			for (int regioni = 0; regioni < 512; regioni++) {
				GlyphContainer* con = m_containers[regioni];
				if ( con ) {
					auto it = containers_sort.end();
					uint64 use_count = con->use_count;
					
					for ( auto& j : containers_sort ) {
						if ( use_count <= j.value().use_count ) {
							it = j; break;
						}
					}
					if ( it.is_null() ) {
						containers_sort.push({ con, regioni, use_count });
					} else {
						containers_sort.before(it, { con, regioni, use_count });
					}
					total_data_size += con->data_size;
					con->use_count /= 2;
				} else { // 容器不存在,标志也不需要存在
					delete m_flags[regioni]; m_flags[regioni] = nullptr;
				}
			}
			
			if ( containers_sort.length() ) {
				uint64 total_data_size_1_3 = total_data_size / 3;
				uint64 del_data_size = 0;
				// 从排序列表顶部开始删除总容量的1/3,并置零容器使用次数
				auto last = --containers_sort.end();
				
				for ( auto it = containers_sort.begin(); it != last; it++ ) {
					if ( del_data_size < total_data_size_1_3 ) {
						int region = it.value().region;
						del_data_size += it.value().container->data_size;
						del_glyph_data( it.value().container );
						delete m_containers[region]; m_containers[region] = nullptr;
						delete m_flags[region]; m_flags[region] = nullptr;
					}
				}
				// 如果删除到最后一个容器还不足总容量的1/3,并且最后一个容器总容量超过512kb也一并删除
				if ( del_data_size < total_data_size_1_3 ) {
					if ( last.value().container->data_size > 512 * 1024 ) {
						int region = last.value().region;
						del_glyph_data( last.value().container );
						delete m_containers[region]; m_containers[region] = nullptr;
						delete m_flags[region]; m_flags[region] = nullptr;
					}
				}
				
				XX_DEBUG("Font memory clear, %ld", del_data_size);
			}
			// not full clear end
		}
	}
}

/**
 * @destructor
 */
Font::~Font() {
	_inl_font(this)->clear(true);
}

cString& Font::name() const {
	return m_font_name;
}

Font* Font::font(TextStyleEnum style) {
	return this;
}

/**
 * @func load
 */
bool Font::load() {
	if ( !m_ft_face ) {
		install();
		
		if ( !m_ft_face ) {
			XX_ERR("Unable to install font");
			return false;
		}
		
		m_ft_glyph = ((FT_Face)m_ft_face)->glyph;
		
		if ( ! m_containers ) { // 创建块容器
			m_containers = new GlyphContainer*[512];
			m_flags = new GlyphContainerFlag*[512];
			memset( m_containers, 0, sizeof(GlyphContainer*) * 512);
			memset( m_flags, 0, sizeof(GlyphContainerFlag*) * 512);
		}
	}
	return true;
}

/**
 * @func unload
 */
void Font::unload() {
	if ( m_ft_face ) {
		for (int i = 0; i < 512; i++) {
			_inl_font(this)->del_glyph_data( m_containers[i] );
		}
		FT_Done_Face((FT_Face)m_ft_face);
		m_ft_face = nullptr;
		m_ft_glyph = nullptr;
	}
}

/**
 * @class FontFromData
 */
class FontFromData: public Font {
 public:
	
	class Data: public Reference {
	 public:
		Data(Buffer& buff)
		: value((FT_Byte*)*buff)
		, length(buff.length()), storage(buff) {  }
		FT_Byte* value;
		uint  length;
		Buffer storage;
	};
	
	Data*  m_data;
	
	FontFromData(Data* data) : m_data(data) {
		m_data->retain();
	}
	
	virtual ~FontFromData() {
		m_data->release();
	}
	
	/**
	 * @overwrite
	 */
	void install() {
		XX_ASSERT(!m_ft_face);
		FT_New_Memory_Face((FT_Library)m_ft_lib,
											 m_data->value,
											 m_data->length,
											 m_face_index,
											 (FT_Face*)&m_ft_face);
	}
};

/**
 * @class FontFromFile
 */
class FontFromFile: public Font {
 public:
	String  m_font_path;    // 字体文件路径
	
	FontFromFile(cString& path) : m_font_path(path) { }
	
	/**
	 * @overwrite
	 */
	void install() {
		XX_ASSERT(!m_ft_face);
		FT_New_Face((FT_Library)m_ft_lib,
								Path::fallback_c(m_font_path),
								m_face_index,
								(FT_Face*)&m_ft_face);
	}
};

// ------------------------ font glyph ------------------------

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
		
		for ( uint i = 0; i < m_fonts.length(); i++ ) {
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

// ------------------------FontFamily::Inl------------------------

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
