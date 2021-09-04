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

	Font::~Font() {
		_inl_font(this)->clear(true);
	}

	cString& Font::name() const {
		return _font_name;
	}

	Font* Font::font(TextStyleValue style, TextWeightValue weight) {
		return this;
	}

	bool Font::load() {
		if ( !_ft_face ) {
			install();
			
			if ( !_ft_face ) {
				FX_ERR("Unable to install font");
				return false;
			}
			
			_ft_glyph = ((FT_Face)_ft_face)->glyph;
			
			if ( ! _containers ) { // 创建块容器
				_containers = new GlyphContainer*[512];
				_flags = new GlyphContainerFlag*[512];
				memset( _containers, 0, sizeof(GlyphContainer*) * 512);
				memset( _flags, 0, sizeof(GlyphContainerFlag*) * 512);
			}
		}
		return true;
	}

	void Font::unload() {
		if ( _ft_face ) {
			for (int i = 0; i < 512; i++) {
				_inl_font(this)->del_glyph_data( _containers[i] );
			}
			FT_Done_Face((FT_Face)_ft_face);
			_ft_face = nullptr;
			_ft_glyph = nullptr;
		}
	}

	FontFromData::Data::Data(Buffer& buff)
		: value((FT_Byte*)*buff), length(buff.length()), _storage(buff)
	{}

	FontFromData::FontFromData(Data* data) : _data(data) {
		_data->retain();
	}
	
	FontFromData::~FontFromData() {
		_data->release();
	}
	
	void FontFromData::install() {
		ASSERT(!_ft_face);
		FT_New_Memory_Face((FT_Library)_ft_lib,
											_data->value, _data->length,
											_face_index, (FT_Face*)&_ft_face);
	}

	FontFromFile::FontFromFile(cString& path): _font_path(path) { }
	
	void FontFromFile::install() {
		ASSERT(!_ft_face);
		FT_New_Face((FT_Library)_ft_lib,
								Path::fallback_c(_font_path),
								_face_index, (FT_Face*)&_ft_face);
	}

	void Font::Inl::initialize(
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
		int underline_thickness,
		FT_Library lib
	) {
		_pool = pool;
		_font_family = family;
		_font_name = font_name;
		_style = style;
		_num_glyphs = num_glyphs;
		_ft_glyph = nullptr;
		_face_index = face_index;
		_ft_lib = lib;
		_ft_face = nullptr;
		_descender = 0;
		_height = 0;
		_max_advance = 0;
		_ascender = 0;
		_containers = nullptr;
		_flags = nullptr;
		_height = height;
		_max_advance = max_advance;
		_ascender = ascender;
		_descender = descender;
		_underline_position = underline_position;
		_underline_thickness = underline_thickness;
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
		// LOG("move_to:%d,%d", to->x, to->y);
		
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
			auto ctx = _pool->_render_ctx;
			
			for ( int i = 0; i < 128; i++, glyph++ ) {
				if ( glyph->_vertex_value ) {
					// TODO ...
					// ctx->del_buffer( glyph->_vertex_value );
					glyph->_vertex_value = 0;
					glyph->_vertex_count = 0;
				}
				
				for ( int i = 1; i < 12; i++ ) {
					if ( glyph->_textures[i] ) { // 删除纹理
						// TODO ...
						// ctx->del_texture( glyph->_textures[i] );
					}
				}
				memset(glyph->_textures, 0, sizeof(uint32_t) * 13);
			}
			_pool->_total_data_size -= container->data_size;
			container->use_count = 0;
			container->data_size = 0;
		}
	}

	/**
	* @func get_glyph
	*/
	FontGlyph* Font::Inl::get_glyph(uint16_t unicode, uint32_t region,
																	uint32_t index, FGTexureLevel level, bool vector) 
	{
		ASSERT(region < 512);
		ASSERT(index < 128);
		
		load(); ASSERT(_ft_face);
		
		GlyphContainerFlag* flags = _flags[region];
		
		if ( !flags ) {
			flags = new GlyphContainerFlag();
			_flags[region] = flags;
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
				uint16_t glyph_index = FT_Get_Char_Index( (FT_Face)_ft_face, unicode );
				
				if (! glyph_index ) goto cf_none;
				
				GlyphContainer* container = _containers[region];
				if ( !container ) {
					_containers[region] = container = new GlyphContainer();
					memset(container, 0, sizeof(GlyphContainer));
					container->font = this;
				}
				
				FT_Error error = FT_Set_Char_Size( (FT_Face)_ft_face, 0, 64 * 64, 72, 72);
				if (error) {
					FX_WARN("%s", "parse font glyph vbo data error"); goto cf_none;
				}
				
				error = FT_Load_Glyph( (FT_Face)_ft_face, glyph_index, FT_LOAD_NO_HINTING);
				if (error) {
					FX_WARN("%s", "parse font glyph vbo data error"); goto cf_none;
				}
			
				FT_GlyphSlot ft_glyph = (FT_GlyphSlot)_ft_glyph;
				
				glyph = container->glyphs + index;
				glyph->_container = container;
				glyph->_unicode = unicode;
				glyph->_glyph_index = glyph_index;
				glyph->_hori_bearing_x = ft_glyph->metrics.horiBearingX;
				glyph->_hori_bearing_y = ft_glyph->metrics.horiBearingY;
				glyph->_hori_advance = ft_glyph->metrics.horiAdvance;
				glyph->_have_outline = ft_glyph->outline.points;
				
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
				glyph = _containers[region]->glyphs + index;
				
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
		if ( _ft_face ) {
			if ( full ) { // 完全清理
				
				for (int i = 0; i < 512; i++) {
					del_glyph_data( _containers[i] );
					delete _containers[i]; _containers[i] = nullptr;
					delete _flags[i]; _flags[i] = nullptr;
				}
				delete _containers; _containers = nullptr;
				delete _flags; _flags = nullptr;
				
				FT_Done_Face((FT_Face)_ft_face);
				_ft_face = nullptr;
				_ft_glyph = nullptr;
				
			} else {
				struct Container {
					GlyphContainer* container;
					int region;
					uint64_t use_count;
				};
				uint64_t total_data_size = 0;
				List<Container> containers_sort;
				// 按使用使用次数排序
				for (int regioni = 0; regioni < 512; regioni++) {
					GlyphContainer* con = _containers[regioni];
					if ( con ) {
						auto it = containers_sort.end();
						uint64_t use_count = con->use_count;
						
						for ( auto j = containers_sort.begin(),
							e = containers_sort.end(); j != e; j++ ) {
							if ( use_count <= j->use_count ) {
								it = j; break;
							}
						}
						if ( it.is_null() ) {
							containers_sort.push_back({ con, regioni, use_count });
						} else {
							containers_sort.insert(it, { con, regioni, use_count });
						}
						total_data_size += con->data_size;
						con->use_count /= 2;
					} else { // 容器不存在,标志也不需要存在
						delete _flags[regioni]; _flags[regioni] = nullptr;
					}
				}
				
				if ( containers_sort.length() ) {
					uint64_t total_data_size_1_3 = total_data_size / 3;
					uint64_t del_data_size = 0;
					// 从排序列表顶部开始删除总容量的1/3,并置零容器使用次数
					auto last = --containers_sort.end();
					
					for ( auto it = containers_sort.begin(); it != last; it++ ) {
						if ( del_data_size < total_data_size_1_3 ) {
							int region = it->region;
							del_data_size += it->container->data_size;
							del_glyph_data( it->container );
							delete _containers[region]; _containers[region] = nullptr;
							delete _flags[region]; _flags[region] = nullptr;
						}
					}
					// 如果删除到最后一个容器还不足总容量的1/3,并且最后一个容器总容量超过512kb也一并删除
					if ( del_data_size < total_data_size_1_3 ) {
						if ( last->container->data_size > 512 * 1024 ) {
							int region = last->region;
							del_glyph_data( last->container );
							delete _containers[region]; _containers[region] = nullptr;
							delete _flags[region]; _flags[region] = nullptr;
						}
					}
					
					FX_DEBUG("Font memory clear, %ld", del_data_size);
				}
				// not full clear end
			}
		}
	}

}
