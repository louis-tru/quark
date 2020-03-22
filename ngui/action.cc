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

NX_NS(ngui)

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
 * @func move_to # 鏂扮殑澶氳竟褰㈠紑濮�
 */
int Font::Inl::move_to(const FT_Vector* to, void* user) {
	DecomposeData* data = static_cast<DecomposeData*>(user);
	if (data->length) { // 娣诲姞涓€涓杈瑰舰
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
 * @func line_to # 涓€娆¤礉濉炲皵鐐癸紙鐩寸嚎锛�
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
 * @func line_to # 浜屾璐濆灏旀洸绾跨偣,杞崲鍒颁竴娆″灏旂偣
 */
int Font::Inl::conic_to(const FT_Vector* control, const FT_Vector* to, void* user) {
	DecomposeData* data = static_cast<DecomposeData*>(user);
	Vec2 p2 = Vec2(to->x, -to->y);
	//  LOG("conic_to:%d,%d|%d,%d", control->x, control->y, to->x, to->y);
	QuadraticBezier bezier(data->p0, Vec2(control->x, -control->y), p2);
	// 浣跨敤10鐐归噰鏍�,閲囨牱瓒婂瓒婅兘杩樺師鏇茬嚎,浣嗛渶瑕佹洿澶氭湁瀛樺偍绌洪棿
	bezier.sample_curve_points(data->sample, (float*)(data->push_vertex(data->sample - 1) - 1));
	data->p0 = p2;
	return FT_Err_Ok;
}

/**
 * @func line_to # 涓夋璐濆灏旀洸绾跨偣,杞崲鍒颁竴娆″灏旂偣
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
				if ( glyph->m_textures[i] ) { // 鍒犻櫎绾圭悊
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
	ASSERT(region < 512);
	ASSERT(index < 128);
	
	load(); ASSERT(m_ft_face);
	
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
				NX_WARN("%s", "parse font glyph vbo data error"); goto cf_none;
			}
			
			error = FT_Load_Glyph( (FT_Face)m_ft_face, glyph_index, FT_LOAD_NO_HINTING);
			if (error) {
				NX_WARN("%s", "parse font glyph vbo data error"); goto cf_none;
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
 * 鍦ㄨ皟鐢ㄨ繖涓柟娉曞悗姝ゅ墠閫氳繃浠讳綍閫斿緞鑾峰彇鍒扮殑瀛楀瀷鏁版嵁灏嗕笉鑳藉啀浣跨敤
 * 鍖呮嫭浠嶧ontGlyphTable鑾峰彇鍒扮殑瀛楀瀷,璋冪敤FontGlyphTable::Inl.clear_table()鍙竻鐞嗗紩鐢�
 */
void Font::Inl::clear(bool full) {
	if ( m_ft_face ) {
		if ( full ) { // 瀹屽叏娓呯悊
			
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
			// 鎸変娇鐢ㄤ娇鐢ㄦ鏁版帓搴�
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
				} else { // 瀹瑰櫒涓嶅瓨鍦�,鏍囧織涔熶笉闇€瑕佸瓨鍦�
					delete m_flags[regioni]; m_flags[regioni] = nullptr;
				}
			}
			
			if ( containers_sort.length() ) {
				uint64 total_data_size_1_3 = total_data_size / 3;
				uint64 del_data_size = 0;
				// 浠庢帓搴忓垪琛ㄩ《閮ㄥ紑濮嬪垹闄ゆ€诲閲忕殑1/3,骞剁疆闆跺鍣ㄤ娇鐢ㄦ鏁�
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
				// 濡傛灉鍒犻櫎鍒版渶鍚庝竴涓鍣ㄨ繕涓嶈冻鎬诲閲忕殑1/3,骞朵笖鏈€鍚庝竴涓鍣ㄦ€诲閲忚秴杩�512kb涔熶竴骞跺垹闄�
				if ( del_data_size < total_data_size_1_3 ) {
					if ( last.value().container->data_size > 512 * 1024 ) {
						int region = last.value().region;
						del_glyph_data( last.value().container );
						delete m_containers[region]; m_containers[region] = nullptr;
						delete m_flags[region]; m_flags[region] = nullptr;
					}
				}
				
				NX_DEBUG("Font memory clear, %ld", del_data_size);
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
			NX_ERR("Unable to install font");
			return false;
		}
		
		m_ft_glyph = ((FT_Face)m_ft_face)->glyph;
		
		if ( ! m_containers ) { // 鍒涘缓鍧楀鍣�
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
		, length(buff.length()), storage(buff) {}
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
		ASSERT(!m_ft_face);
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
	String  m_font_path;    // 瀛椾綋鏂囦欢璺緞
	
	FontFromFile(cString& path) : m_font_path(path) { }
	
	/**
	 * @overwrite
	 */
	void install() {
		ASSERT(!m_ft_face);
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
				if ( font->load() ) { // 杞藉叆瀛椾綋
					m_fonts.push(font);
					fonts_name.set(font->name(), true);
				}
			}
		}
		
		/* 濡傛灉浣跨敤icon鍥炬爣瀛椾綋鏃讹紝鍔犲叆榛樿瀛椾綋浼氬鑷翠笉鍚岀郴缁焋ascender`涓巂descender`鏈夋墍宸紓,
		 * 浠庤€屽鑷翠笉鍚岀郴缁熺殑瀛椾綋鍩虹嚎鐨勪綅缃笉鐩稿悓锛屾棤娉曚娇鐢╜icon`绮惧噯鎺掔増锛�
		 * 涓轰簡浣跨敤杩欎釜`icon`杩涜绮惧噯鎺掔増鐜板湪鏆傛椂鍋氫緥澶栧鐞嗭紝鍦ㄤ娇鐢╜icon`瀛椾綋鍚嶇О鏃朵笉鍔犲叆榛樿瀛椾綋
		 */
		
		if (m_ffid->code() != 2090550926) {
			for ( auto& i : m_pool->m_default_fonts ) {
				Font* font = i.value()->font(m_style);
				if ( ! fonts_name.has(font->name()) ) { // 閬垮厤閲嶅鐨勫悕绉�
					if ( font->load() ) { // 杞藉叆瀛椾綋
						m_fonts.push(font);
						fonts_name.set(font->name(), true);
					}
				}
			}
		} else {
			ASSERT(m_ffid->name() == "icon");
		}
		
		// 鏌ユ壘鏈€澶ч珮搴︿笌琛岄珮搴�
		m_height = 0;
		m_ascender = 0;
		m_descender = 0;
		
		for ( uint i = 0; i < m_fonts.length(); i++ ) {
			Font* font = m_fonts[i];
			m_height = NX_MAX(m_height, font->height());
			m_ascender = NX_MAX(m_ascender, font->ascender());
			m_descender = NX_MAX(m_descender, font->descender());
		}
	}
	
	void set_glyph(uint region, uint index, FontGlyph* glyph) {
		ASSERT( glyph );
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
		
		/* TODO 浣跨敤涓€涓粯璁ゅ瓧褰�  锟� 65533 */
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
 * @func use_texture_glyph 浣跨敤绾圭悊瀛楀瀷
 */
FontGlyph* FontGlyphTable::use_texture_glyph(uint16 unicode, FGTexureLevel level) {
	ASSERT(level < FontGlyph::LEVEL_NONE);
	
	FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
	
	if ( glyph ) {
		if ( glyph->has_texure_level(level) ) {
			glyph->m_container->use_count++; return glyph;
		}
		else { // 鍏堝皾璇曚娇鐢ㄦ渶蹇殑鏂规硶
			if ( _inl_font(glyph->font())->set_texture_data(glyph, level) ) { //
				glyph->m_container->use_count++; return glyph;
			}
		}
	}
	glyph = _inl_table(this)->find_glyph(unicode, level, false);
	
	glyph->m_container->use_count++; return glyph;
}

/**
 * @func use_vector_glyph # 浣跨敤瀛楀瀷,骞朵笖杞藉叆vbo鐭㈤噺椤剁偣鏁版嵁
 */
FontGlyph* FontGlyphTable::use_vector_glyph(uint16 unicode) {
	FontGlyph* glyph = _inl_table(this)->get_glyph(unicode);
	
	if ( glyph ) {
		if ( glyph->vertex_data() ) {
			glyph->m_container->use_count++; return glyph;
		}
		else { // 鍏堝皾璇曚娇鐢ㄦ渶蹇殑鏂规硶
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
	
	// 鏌ユ壘鐩搁偦瀛楅噸
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

NX_END
	time += m_delay;
	if (m_parent) {
		m_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void SequenceAction::seek_before(int64 time, Action* child) {
	time += m_delay;
	for ( auto& i : m_actions ) {
		if ( child == i.value() ) {
			break;
		} else {
			time += i.value()->m_full_duration;
		}
	}
	if (m_parent) {
		m_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void KeyframeAction::seek_before(int64 time, Action* child) {
	NX_UNIMPLEMENTED();
}

void SpawnAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - m_delay;
	if ( t < 0 ) {
		m_delayd = time;
		return;
	} else {
		m_delayd = m_delay;
		time = t;
	}
	
	m_loopd = 0;// 重置循环
	
	for ( auto& i : m_actions ) {
		i.value()->seek_time(time, root);
	}
}

void SequenceAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - m_delay;
	if ( t < 0 ) {
		m_delayd = time;
		m_action = Iterator();
		return;
	} else {
		m_delayd = m_delay;
		time = t;
	}
	
	m_loopd = 0;// 重置循环
	
	uint64 duration = 0;
	
	for ( auto& i : m_actions ) {
		uint64 du = duration + i.value()->m_full_duration;
		if ( du > time ) {
			m_action = i;
			i.value()->seek_time(time - duration, root);
			return;
		}
		duration = du;
	}
	
	if ( length() ) {
		m_actions.last()->seek_time(time - duration, root);
	}
}

void KeyframeAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - m_delay;
	if ( t < 0 ) {
		m_delayd = time;
		m_frame = -1;
		m_time = 0; return;
	} else {
		m_delayd = m_delay;
		time = t;
	}
	
	m_loopd = 0;// 重置循环
	
	if ( length() ) {
		Frame* frame = nullptr;
		
		for ( auto& i: m_frames ) {
			if ( time < i.value()->time() ) {
				break;
			}
			frame = i.value();
		}
		
		m_frame = frame->index();
		m_time = NX_MIN(int64(time), m_full_duration - m_delay);
		
		uint f1 = m_frame;
		uint f2 = f1 + 1;
		
		if ( f2 < length() ) {
			int64 time1 = frame->time();
			int64 time2 = m_frames[f2]->time();
			float x = (m_time - time1) / float(time2 - time1);
			float t = frame->curve().solve(x, 0.001);
			_inl_key_action(this)->transition(f1, f2, x, t, root);
		} else { // last frame
			_inl_key_action(this)->transition(f1, root);
		}
		
		if ( m_time == int64(frame->time()) ) {
			_inl_action(this)->trigger_action_key_frame(0, m_frame, root);
		}
	}
}

uint64 SpawnAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= m_speed; // Amplification time
	
	if ( restart ) { // restart
		m_delayd = 0;
		m_loopd = 0;
	}
	
	if ( m_delay > m_delayd ) { // 需要延时
		int64 time = m_delay - m_delayd - time_span;
		if ( time >= 0 ) {
			m_delayd += time_span;
			return 0;
		} else {
			m_delayd = m_delay;
			time_span = -time;
		}
	}
	
	uint64 surplus_time = time_span;
	
 advance:
	
	for ( auto& i : m_actions ) {
		uint64 time = i.value()->advance(time_span, restart, root);
		surplus_time = NX_MIN(surplus_time, time);
	}
	
	if ( surplus_time ) {
		if ( m_loop && m_full_duration > m_delay ) {
			
			if ( m_loop > 0 ) {
				if ( m_loopd < m_loop ) { // 可经继续循环
					m_loopd++;
				} else { //
					goto end;
				}
			}
			
			restart = true;
			time_span = surplus_time;
			
			_inl_action(this)->trigger_action_loop(time_span, root);
			
			if ( _inl_action(root)->is_playing() ) {
				goto advance;
			}
			
			return 0; // end
		}
	}
	
 end:
	return surplus_time / m_speed;
}

uint64 SequenceAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= m_speed; // Amplification time
	
	if ( m_action.is_null() || restart ) { // no start play
		
		if ( restart ) { // restart
			m_delayd = 0;
			m_loopd = 0;
			m_action = Iterator();
		}
		
		if ( m_delay > m_delayd ) { // 需要延时
			int64 time = m_delay - m_delayd - time_span;
			if ( time >= 0 ) {
				m_delayd += time_span;
				return 0;
			} else {
				m_delayd = m_delay;
				time_span = -time;
			}
		}
		
		if ( length() ) {
			restart = true;
			m_action = m_actions.begin();
		} else {
			return time_span / m_speed;
		}
	}
	
 advance:
	
	time_span = m_action.value()->advance(time_span, restart, root);
	
	if ( time_span ) {
		
		if ( m_action.is_null() ) { // May have been deleted
			if ( length() ) { // Restart
				restart = true;
				m_action = m_actions.begin();
				goto advance;
			}
		} else {
			if ( m_action.value() == m_actions.last() ) { // last action
				if ( m_loop && m_full_duration > m_delay ) {
					
					if ( m_loop > 0 ) {
						if ( m_loopd < m_loop ) { // 可经继续循环
							m_loopd++;
						} else { //
							goto end;
						}
					}
					
					restart = true;
					
					_inl_action(this)->trigger_action_loop(time_span, root); // trigger event
					m_action = m_actions.begin();
					
					if ( m_action.is_null() ) { // 可能在触发`action_loop`事件时被删除
						// 没有child action 无效,所以这里结束
						goto end;
					}
					
					if ( _inl_action(root)->is_playing() ) {
						goto advance;
					}
					return 0; // end
				}
			} else {
				restart = true;
				m_action++;
				goto advance;
			}
		}
	}
	
 end:
	return time_span / m_speed;
}

KeyframeAction::~KeyframeAction() {
	clear();
}

uint64 KeyframeAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= m_speed;
	
	if ( m_frame == -1 || restart ) { // no start play
		
		if ( restart ) { // restart
			m_delayd = 0;
			m_loopd = 0;
			m_frame = -1;
			m_time = 0;
		}

		if ( m_delay > m_delayd ) { // 需要延时
			int64 time = m_delay - m_delayd - time_span;
			if ( time >= 0 ) {
				m_delayd += time_span;
				return 0;
			} else {
				m_delayd = m_delay;
				time_span = -time;
			}
		}
		
		if ( length() ) {
			m_frame = 0;
			m_time = 0;
			_inl_key_action(this)->transition(0, root);
			_inl_action(this)->trigger_action_key_frame(time_span, 0, root);
			
			if ( time_span == 0 ) {
				return 0;
			}
			
			if ( length() == 1 ) {
				return time_span / m_speed;
			}
		} else {
			return time_span / m_speed;
		}
	}
	
	return _inl_key_action(this)->advance(time_span, root) / m_speed;
}

void GroupAction::bind_view(View* view) {
	for ( auto& i : m_actions ) {
		i.value()->bind_view(view);
	}
}

void KeyframeAction::bind_view(View* view) {
	int view_type = view->view_type();
	if ( view_type != m_bind_view_type ) {
		m_bind_view_type = view_type;
		for ( auto& i : m_property ) {
			i.value()->bind_view(view_type);
		}
	}
}

/**
 * @func add new frame
 */
Frame* KeyframeAction::add(uint64 time, const FixedCubicBezier& curve) {
	
	if ( length() ) {
		Frame* frame = last();
		int64 duration = time - frame->time();
		if ( duration <= 0 ) {
			time = frame->time();
		} else {
			_inl_action(this)->update_duration(duration);
		}
	} else {
		time = 0;
	}
	
	Frame* frame = new Frame(this, m_frames.length(), curve);
	m_frames.push(frame);
	frame->m_time = time;
	
	for ( auto& i : m_property ) {
		i.value()->add_frame();
	}
	
	return frame;
}

/**
 * @func clear all frame and property
 */
void KeyframeAction::clear() {
	
	for (auto& i : m_frames) {
		i.value()->m_host = nullptr;
		Release(i.value());
	}
	for (auto& i : m_property) {
		delete i.value();
	}
	m_frames.clear();
	m_property.clear();
	
	if ( m_full_duration ) {
		_inl_action(this)->update_duration( m_delay - m_full_duration );
	}
}

bool KeyframeAction::has_property(PropertyName name) {
	return m_property.has(name);
}

/**
 * @func match_property
 */
bool KeyframeAction::match_property(PropertyName name) {
	return PropertysAccessor::shared()->has_accessor(m_bind_view_type, name);
}
// ----------------------- ActionCenter -----------------------

static ActionCenter* action_center_shared = nullptr;

ActionCenter::ActionCenter()
: m_prev_sys_time(0) {
	ASSERT(!action_center_shared);
	action_center_shared = this;
}

ActionCenter::~ActionCenter() {
	action_center_shared = nullptr;
}

/**
 * @func advance
 */
void ActionCenter::advance(int64 now_time) {
	/*
	static int len = 0;
	if (len != m_actions.length()) {
		len = m_actions.length();
		LOG("ActionCenter::advance,length, %d", len);
	}*/
	
	if ( m_actions.length() ) { // run task
		int64 time_span = 0;
		if (m_prev_sys_time) {  // 0表示还没开始
			time_span = now_time - m_prev_sys_time;
			if ( time_span > 200000 ) {   // 距离上一帧超过200ms重新记时(如应用程序从休眠中恢复)
				time_span = 200000; // 100ms
			}
		}
		for ( auto i = m_actions.begin(); !i.is_null(); ) {
			Action::Wrap& wrap = i.value();
			if ( wrap.value ) {
				if (wrap.play) {
					if ( wrap.value->advance(time_span, false, wrap.value) ) {
						// 不能消耗所有时间表示动作已经结束
						// end action play
						_inl_action_center(this)->del(wrap.value);
					}
				} else {
					wrap.play = true;
					wrap.value->advance(0, false, wrap.value);
				}
				i++;
			} else {
				m_actions.del(i++);
			}
		}
		m_prev_sys_time = now_time;
	}
}

ActionCenter* ActionCenter::shared() {
	return action_center_shared;
}

NX_END
