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

#include "../util/os.h"
#include "../util/array.h"
#include "./_font.h"
#include "../bezier.h"
#include "../texture.h"
#include "../display.h"
// #include "../draw.h"
#include "../_app.h"
#include <native-font.h>

#ifndef FX_SUPPORT_MAX_TEXTURE_FONT_SIZE
#define FX_SUPPORT_MAX_TEXTURE_FONT_SIZE 512
#endif

namespace flare {

	static String THIN_("thin");
	static String ULTRALIGHT_("ultralight"); static String BOOK_("book");
	static String LIGHT_("light");
	static String REGULAR_("regular"); static String NORMAL_("normal");
	static String MEDIUM_("medium"); static String DEMI_("demi");
	static String SEMIBOLD_("semibold"); static String ROMAN_("roman");
	static String BOLD_("bold"); static String CONDENSED_("condensed");
	static String HEAVY_("heavy");
	static String BLACK_("black");
	static String ITALIC_("italic"); static String OBLIQUE_("oblique");

	bool FontPool::Inl::register_font(
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
	) {
		if ( ! path.is_empty() ) { // 文件
			
			if ( ! FileHelper::is_file_sync(path) )
				return false;
			_paths[path] = family_name;
			
		} else if ( !data || !data->value ) {
			return false;
		}
		
		String font_name_ = font_name;
		
		/*
		FX_DEBUG("family_name:%s, font_name:%s, %s, ------%dkb%s", *family_name, *font_name, *path,
						uint(FileHelper::stat_sync(path).size() / 1024),
						_fonts.has(font_name) ? "+++++++++++": "");
		*/
		
		for (int i = 1; _fonts.count(font_name_); i++ ) { // 重复的字体名称
			font_name_ = font_name + "_" + i;
		}
		
		FontFamily* family = nullptr;
		
		auto i = _familys.find(family_name);
		
		if ( i != _familys.end() ) {
			family = i->value;
		} else {
			family = new FontFamily(family_name);
			_familys[family_name] = family;
			_blend_fonts[family_name] = family; // 替换别名
		}
		
		Font* font = nullptr;
		
		if ( path.is_empty() ) {
			font = new FontFromData(data);
		} else {
			font = new FontFromFile(path);
		}
		
		_inl_font(font)->initialize(this, family,
															font_name_, style, num_glyphs, face_index,
															height, max_advance, ascender, descender,
															underline_position, underline_thickness, (FT_Library)_ft_lib);
		
		_fonts[font_name_] = font;
		if ( font_name_ != family_name ) { // 与家族名称相同忽略
			_blend_fonts[font_name_] = font;
		}
		
		_inl_family(family)->add_font(font);
		
		return true;
	}
	
	bool FontPool::Inl::register_font(FontFromData::Data* font_data, cString& family_alias) {
		Handle<FontFromData::Data> _font_data = font_data;
		
		const FT_Byte* data = font_data->value;
		
		FT_Face face;
		FT_Error err = FT_New_Memory_Face((FT_Library)_ft_lib, data, font_data->length, 0, &face);
		
		if (err) {
			FX_ERR("Unable to load font, Freetype2 error code: %d", err);
		} else if (!face->family_name) {
			FX_ERR("Unable to load font, not family name");
		} else {
			
			FT_Long num_faces = face->num_faces;
			String  family_name = face->family_name;
			int     face_index = 0;
			
			while (1) {
				
				if (face->charmap &&
						face->charmap->encoding == FT_ENCODING_UNICODE && // 必须要有unicode编码表
						FT_IS_SCALABLE(face)                              // 必须为矢量字体
						) {
					// 以64 pem 为标准
					float ratio = face->units_per_EM / 4096.0 /*( 64 * 64 = 4096 )*/;
					int height = face->height / ratio;
					int max_advance_width = face->max_advance_width / ratio;
					int ascender = face->ascender / ratio;
					int descender = -face->descender / ratio;
					int underline_position	= face->underline_position;
					int underline_thickness = face->underline_thickness;
					String name = FT_Get_Postscript_Name(face);
					
					if (!
							register_font(family_name,
														name,
														parse_style_flags(name, face->style_name),
														(uint32_t)face->num_glyphs,
														face_index, height,
														max_advance_width, ascender,
														descender, underline_position,
														underline_thickness, String(), font_data)
							) {
						return false;
					}
				}
				face_index++;
				
				FT_Done_Face(face);
				
				if (face_index < num_faces) {
					err = FT_New_Memory_Face((FT_Library)_ft_lib, data, font_data->length, face_index, &face);
					if (err) {
						FX_ERR("Unable to load font, Freetype2 error code: %d", err);
						return false;
					}
				} else {
					break;
				}
			}
			
			// set family_alias
			set_family_alias(family_name, family_alias);
			
			return true;
		}
		
		return false;
	}
	
	void FontPool::Inl::display_port_change_handle(Event<>& evt) {

		Vec2 scale_value = _host->display()->scale();
		float scale = FX_MAX(scale_value[0], scale_value[1]);
		
		if ( scale != _display_port_scale ) {
			
			if ( _display_port_scale != 0.0 ) { // 缩放改变影响字型纹理,所有全部清理
				clear(true);
			}
			_display_port_scale = scale;
			
			// TODO ...
			// _draw_ctx->host()->render_loop()->post(Cb([this](CbData& e) {
			// 	_draw_ctx->refresh_font_pool(this);
			// 	_inl_app(_draw_ctx->host())->refresh_display();
			// }));
			
			Vec2 size = _host->display()->size();
			uint32_t font_size = sqrtf(size.width() * size.height()) / 10;
			
			// 最大纹理字体不能超过上下文支持的大小
			if (font_size >= FX_SUPPORT_MAX_TEXTURE_FONT_SIZE) {
				_max_glyph_texture_size = FX_SUPPORT_MAX_TEXTURE_FONT_SIZE;
			} else {
				_max_glyph_texture_size = font_glyph_texture_levels_idx[font_size].max_font_size;
			}
		}
	}

	bool FontPool::Inl::has_italic_style(cString& style_name) {
		return style_name.index_of(ITALIC_) != -1 || style_name.index_of(OBLIQUE_) != -1;
	}

	TextStyleValue FontPool::Inl::parse_style_flags(cString& name, cString& style_name) {
		String str = style_name.to_lower_case();

		// TODO ...
		
		// if ( str.index_of(THIN_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::THIN_ITALIC : TextStyleValue::THIN;
		// }
		// if ( str.index_of(ULTRALIGHT_) != -1 || str.index_of(BOOK_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::ULTRALIGHT_ITALIC : TextStyleValue::ULTRALIGHT;
		// }
		// if ( str.index_of(LIGHT_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::LIGHT_ITALIC : TextStyleValue::LIGHT;
		// }
		// if ( str.index_of(REGULAR_) != -1 || str.index_of(NORMAL_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::ITALIC : TextStyleValue::NORMAL;
		// }
		// if ( str.index_of(MEDIUM_) != -1 || str.index_of(DEMI_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::MEDIUM_ITALIC : TextStyleValue::MEDIUM;
		// }
		// if ( str.index_of(SEMIBOLD_) != -1 || str.index_of(ROMAN_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::SEMIBOLD_ITALIC : TextStyleValue::SEMIBOLD;
		// }
		// if ( str.index_of(BOLD_) != -1 || str.index_of(CONDENSED_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::BOLD_ITALIC : TextStyleValue::BOLD;
		// }
		// if ( str.index_of(HEAVY_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::HEAVY_ITALIC : TextStyleValue::HEAVY;
		// }
		// if ( str.index_of(BLACK_) != -1 ) {
		// 	return has_italic_style(str) ? TextStyleValue::BLACK_ITALIC : TextStyleValue::BLACK;
		// }
		// return TextStyleValue::OTHER;

		return TextStyleValue::NORMAL;
	}
	
	Handle<FontPool::SimpleFontFamily> FontPool::Inl::inl_read_font_file(cString& path, FT_Library lib) {
		FT_Face face;
		FT_Error err = FT_New_Face(lib, Path::fallback_c(path), 0, &face);
		
		if (err) {
			FX_WARN("Unable to load font file \"%s\", Freetype2 error code: %d", *path, err);
		} else if (!face->family_name) {
			FX_WARN("Unable to load font file \"%s\", not family name", *path);
		} else {
			
			FT_Long num_faces = face->num_faces;
			
			Handle<FontPool::SimpleFontFamily> sff = new FontPool::SimpleFontFamily();
			
			sff->path = path;
			sff->family = face->family_name;
			
			int face_index = 0;
			
			while (1) {
				
				if (face->charmap &&
						face->charmap->encoding == FT_ENCODING_UNICODE && // 必须要有unicode编码表
						FT_IS_SCALABLE(face)                              // 必须为矢量字体
				) {
					
					FT_Set_Char_Size(face, 0, 64 * 64, 72, 72);
					
					// 以64 pt 为标准
					float ratio = face->units_per_EM / 4096.0 /*( 64 * 64 = 4096 )*/;
					int height = face->height / ratio;
					int max_advance_width = face->max_advance_width / ratio;
					int ascender = face->ascender / ratio;
					int descender = -face->descender / ratio;
					int underline_position	= face->underline_position;
					int underline_thickness = face->underline_thickness;
					String name = FT_Get_Postscript_Name(face);

					DLOG("------------inl_read_font_file, %s, %s", *name, face->style_name);

					sff->fonts.push({
						name,
						parse_style_flags(name, face->style_name),
						(uint32_t)face->num_glyphs,
						height,
						max_advance_width,
						ascender,
						descender,
						underline_position,
						underline_thickness
					});
				}
				
				face_index++;
				
				FT_Done_Face(face);
				
				if (face_index < num_faces) {
					err = FT_New_Face(lib, Path::fallback_c(path), face_index, &face);
					if (err) {
						FX_WARN("Unable to load font file \"%s\", Freetype2 error code: %d", *path, err); break;
					}
				} else {
					if (sff->fonts.length())
						return sff;
					else
						break;
				}
			}
		}
		return Handle<FontPool::SimpleFontFamily>();
	}

	/**
	* @constructor
	*/
	FontPool::FontPool(Application* host)
		: _ft_lib(nullptr)
		, _host(host)
		, _total_data_size(0)
		, _max_glyph_texture_size(0)
		, _display_port_scale(0)
	{
		ASSERT(host);
		ASSERT(_host->display());

		_host->display()->FX_On(change, &Inl::display_port_change_handle, _inl_pool(this));
		
		FT_Init_FreeType((FT_Library*)&_ft_lib);
			
		{ // 载入内置字体
			uint32_t count = sizeof(native_fonts_) / sizeof(Native_font_data_);
			
			for (uint32_t i = 0 ; i < count; i++) {
				WeakBuffer data((cChar*)native_fonts_[i].data, native_fonts_[i].count);
				auto font_data = new FontFromData::Data(data);
				// LOG("register_font,%d", i);
				_inl_pool(this)->register_font(font_data, i == 1 ? "icon" : String());
				// LOG("register_font ok,%d", i);
			}

			if ( _familys.count("langou") ) {
				// LOG("_familys.has langou ok");
				// 这个内置字体必须载入成功,否则退出程序
				// 把载入的一个内置字体做为默认备用字体,当没有任何字体可用时候,使用这个内置字体
				_spare_family = _familys["langou"];
			} else {
				FX_FATAL("Unable to initialize flare font");
			}
		}
		
		{
			// 载入系统字体
			const Array<SimpleFontFamily>& arr = system_font_family();
			
			for (auto sffd : arr) {
				
				for (uint32_t i = 0; i < sffd.fonts.length(); i++) {
					const SimpleFont& sfd = sffd.fonts[i];
					
					_inl_pool(this)->register_font(sffd.family, sfd.name,
																				sfd.style, sfd.num_glyphs,
																				i, sfd.height,
																				sfd.max_advance, sfd.ascender,
																				sfd.descender, sfd.underline_position,
																				sfd.underline_thickness, sffd.path, nullptr);
					// end for
				}
			}
		}
		
		_inl_pool(this)->initialize_default_fonts(); // 设置默认字体列表
	}

	/**
	* @destructor
	*/
	FontPool::~FontPool() {
		
		for ( auto& i : _familys ) {
			Release(i.value); // delete
		}
		for ( auto& i : _fonts ) {
			Release(i.value); // delete
		}
		for ( auto& i : _tables ) {
			Release(i.value); // delete
		}
		
		_familys.clear();
		_fonts.clear();
		_tables.clear();
		_blend_fonts.clear();
		_default_fonts.clear();
		
		FT_Done_FreeType((FT_Library)_ft_lib); _ft_lib = nullptr;
		
		if ( _host->display() ) {
      _host->display()->FX_Off(change, &Inl::display_port_change_handle, _inl_pool(this));
		}
	}

	/**
	* @func set_default_fonts # 尝试设置默认字体
	* @arg first {const Array<String>*}  # 第一字体列表
	* @arg ... {const Array<String>*} # 第2/3/4..字体列表
	*/
	void FontPool::set_default_fonts(const Array<String>* first, ...) {
		
		_default_fonts.clear();
		Dict<String, bool> has;
		
		auto end = _blend_fonts.end();
		
		for (auto i: *first) {
			auto j = _blend_fonts.find(i);
			if (j != end) {
				has[j->value->name()] = true;
				_default_fonts.push(j->value);
				break;
			}
		}
		
		va_list arg;
		va_start(arg, first);
		
		const Array<String>* ls = va_arg(arg, const Array<String>*);
		
		while (ls) {
			for (auto i: *ls) {
				auto j = _blend_fonts.find(i);
				if (j != end) {
					if ( ! has.count(j->value->name()) ) {
						has[j->value->name()] = true;
						_default_fonts.push(j->value);
					}
					break;
				}
			}
			ls = va_arg(arg, const Array<String>*);
		}
		
		va_end(arg);
		
		if ( !has.count(_spare_family->name()) ) {
			_default_fonts.push(_spare_family);
		}
	}

	/**
	* @func set_default_fonts # 在当前字体库找到字体名称,设置才能生效
	* @arg fonts {const Array<String>&} # 要设置的默认字体的名称
	*/
	void FontPool::set_default_fonts(const Array<String>& fonts) {
		
		_default_fonts.clear();
		Dict<String, bool> has;
		
		auto end = _blend_fonts.end();
		
		for (uint32_t i = 0; i < fonts.length(); i++) {
			auto j = _blend_fonts.find(fonts[i].trim());
			if (j != end) {
				if ( ! has.count(j->value->name()) ) {
					has[j->value->name()] = true;
					_default_fonts.push(j->value);
				}
			}
		}
		
		if ( !has.count(_spare_family->name()) ) {
			_default_fonts.push(_spare_family);
		}
	}

	/**
	* @func default_font_names
	*/
	Array<String> FontPool::default_font_names() const {
		Array<String> rev;
		for (uint32_t i = 0; i < _default_fonts.length(); i++)
			rev.push(_default_fonts[i]->name());
		return rev;
	}
	
	
	/**
	* @func font_familys
	*/
	Array<String> FontPool::family_names() const {
		Array<String> names;
		for (auto i: _familys) {
			names.push(i.key);
		}
		return names;
	}
	

	/**
	* @func font_names()
	*/
	Array<String> FontPool::font_names(cString& family_name) const {
		FontFamily* ff = const_cast<FontPool*>(this)->get_font_family(family_name);
		return ff ? ff->font_names() : Array<String>();
	}

	/**
	* @func get_font_family()
	*/
	FontFamily* FontPool::get_font_family(cString& family_name) {
		auto i = _familys.find(family_name);
		return i == _familys.end() ? NULL : i->value;
	}

	/**
	* @func get_font # 通过名称获得一个字体对像
	* @arg name {cString&} # 字体名称或家族名称
	* @arg [style = fs_regular] {Font::TextStyle}
	* @ret {Font*}
	*/
	Font* FontPool::get_font(cString& name, TextStyleValue style) {
		auto i = _blend_fonts.find(name);
		return i == _blend_fonts.end() ? NULL : i->value->font(style);
	}

	/**
	* @func get_group # 通过字体名称列表获取字型集合
	* @arg id {FFID} # FFID
	* @arg [style = fs_regular] {Font::TextStyle} # 使用的字体家族才生效
	*/
	FontGlyphTable* FontPool::get_table(FFID ffid, TextStyleValue style) {
		ASSERT(ffid);
		
		uint32_t code = ffid->code() + (uint32_t)style;
		
		auto i = _tables.find(code);
		if ( i != _tables.end() ) {
			return i->value;
		}
		
		FontGlyphTable* table = new FontGlyphTable();
		_inl_table(table)->initialize(ffid, style, this);
		
		_tables[code] = table;
		
		return table;
	}

	/**
	* @func get_table # 获取默认字型集合
	* @arg [style = fs_regular] {TextStyleValue}
	*/
	FontGlyphTable* FontPool::get_table(TextStyleValue style) {
		return get_table(get_font_familys_id(String()), style);
	}

	/**
	* @func register_font # 通过Buffer数据注册字体
	* @arg buff {cBuffer} # 字体数据
	* @arg [family_alias = String()] {cString&} # 给所属家族添加一个别名
	*/
	bool FontPool::register_font(Buffer buff, cString& family_alias) {
		DLOG("register_font,%d", buff.length());
		return _inl_pool(this)->register_font(new FontFromData::Data(buff), family_alias);
	}

	/**
	* @func register_font_file  # 注册本地字体文件
	* @arg path {cString&} # 字体文件的本地路径
	* @arg [family_alias = String()] {cString&} # 给所属家族添加一个别名
	*/
	bool FontPool::register_font_file(cString& path, cString& family_alias) {
		
		if (!_paths.count(path) ) { //
			
			Handle<SimpleFontFamily> sffd = Inl::inl_read_font_file(path, (FT_Library)_ft_lib);
			
			if ( !sffd.is_null() ) {
				
				for (uint32_t i = 0; i < sffd->fonts.length(); i++) {
					
					SimpleFont& sfd = sffd->fonts[i];
					if (!
					_inl_pool(this)->register_font(sffd->family,
																				sfd.name,
																				sfd.style,
																				sfd.num_glyphs,
																				i,
																				sfd.height,
																				sfd.max_advance,
																				sfd.ascender,
																				sfd.descender,
																				sfd.underline_position,
																				sfd.underline_thickness, sffd->path, nullptr)
							) {
						return false;
					}
				}
				// set family_alias
				set_family_alias(sffd->family, family_alias);
				
				return true;
			}
		}
		
		return false;
	}

	/**
	* @func set_family_alias
	*/
	void FontPool::set_family_alias(cString& family, cString& alias) {
		if ( ! alias.is_empty() ) {
			
			auto i = _blend_fonts.find(family);
			
			if (i != _blend_fonts.end() && !_blend_fonts.count(alias)) {
				_blend_fonts[alias] = i->value; // 设置一个别名
			}
		}
	}

	/**
	* @func clear(full) 释放池中不用的字体数据
	* @arg [full = false] {bool} # 全面清理资源尽可能最大程度清理
	*/
	void FontPool::clear(bool full) {
		for ( auto& i : _tables ) {
			_inl_table(i.value)->clear_table();
		}
		for ( auto& i : _fonts ) {
			_inl_font(i.value)->clear(full);
		}
	}

	/**
	* @func get_glyph_texture_level 通过字体尺寸获取纹理等级,与纹理大小font_size
	*/
	FGTexureLevel FontPool::get_glyph_texture_level(float& font_size_out) {
		if (font_size_out > _max_glyph_texture_size) {
			return FontGlyph::LEVEL_NONE;
		}
		uint32_t index = ceilf(font_size_out);
		
		FontGlyphTextureLevel leval = font_glyph_texture_levels_idx[index];
		
		font_size_out = leval.max_font_size;
		
		return leval.level;
	}

	/**
	* @func get_family_name(path) get current register family name by font file path
	*/
	String FontPool::get_family_name(cString& path) const {
		auto it = _paths.find(path);
		if ( it == _paths.end() ) {
			return String();
		}
		return it->value;
	}

	/**
	* @func get_glyph_texture_level # 根据字体尺寸获取纹理等级
	*/
	float FontPool::get_glyph_texture_size(FGTexureLevel leval) {
		ASSERT( leval < FontGlyph::LEVEL_NONE );
		
		const float glyph_texture_levels_size[13] = {
			10, 12, 14, 16, 18, 20, 25, 32, 64, 128, 256, 512, 0
		};
		return glyph_texture_levels_size[leval];
	}

	/**
	* @func default_font_familys_id
	*/
	static FFID default_font_familys_id() {
		static FontFamilysID* id = nullptr; // default group i
		if ( ! id ) {
			id = new FontFamilysID();
			_inl_ff_id(id)->initialize(Array<String>());
		}
		return id;
	}

	/**
	* @func get_font_familys_id
	*/
	FFID FontPool::get_font_familys_id(const Array<String> fonts) {
		
		static Dict<uint32_t, FontFamilysID*> ffids; // global groups
		
		// TODO: 这里如果是同一个字体的不同别名会导致不相同的`ID`
		
		if ( fonts.length() ) {
			FontFamilysID id;
			_inl_ff_id(&id)->initialize(fonts);
			
			auto it = ffids.find(id.code());
			if (it != ffids.end()) {
				return it->value;
			} else {
				FontFamilysID* id_p = new FontFamilysID(std::move( id ));
				ffids[ id_p->code() ] = id_p;
				return id_p;
			}
		} else {
			return default_font_familys_id();
		}
	}

	/**
	* @func read_font_file
	*/
	Handle<FontPool::SimpleFontFamily> FontPool::read_font_file(cString& path) {
		FT_Library lib;
		FT_Init_FreeType(&lib);
		
		ScopeClear clear([&lib]() {
			FT_Done_FreeType(lib);
		});
		return Inl::inl_read_font_file(path, lib);
	}

	/**
	* @func get_font_familys_id
	*/
	FFID FontPool::get_font_familys_id(cString fonts) {
		if ( fonts.is_empty() ) {
			return default_font_familys_id();
		} else {
			Array<String> ls = fonts.split(',');
			Array<String> ls2;
			
			for (int i = 0, len = ls.length(); i < len; i++) {
				String name = ls[i].trim();
				if ( ! name.is_empty() ) {
					ls2.push(name);
				}
			}
			return get_font_familys_id(ls2);
		}
	}

}
