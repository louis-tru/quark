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

#include "ngui/version.h"
#include "ngui/utils/json.h"
#include "tinyxml2.h"

XX_NS(ngui)

using namespace tinyxml2;

#define xx_font_family_info_cache ".simple_font_family_cache"

#if XX_IOS || XX_OSX
static String system_fonts_dir = "/System/Library/Fonts";
#elif XX_ANDROID
static String system_fonts_dir = "/system/fonts";
#elif XX_LINUX
static String system_fonts_dir = "/usr/share/fonts";
#endif
typedef Array<FontPool::SimpleFontFamily> SimpleFontList;
static SimpleFontList* system_font_family_list = nullptr;
static String system_first_font_family_name;
static String system_second_font_family_name;

/**
 * @func find_font_family_by_path(path)
 */
static String find_font_family_by_path(cString& path) {
	XX_ASSERT(system_font_family_list);
	for ( auto& i : *system_font_family_list ) {
		if (i.value().path == path) {
			return i.value().family;
		}
	}
	return String();
}

/**
 * @func parse_system_font_family_name()
 */
static void parse_system_font_family_name() {
	XX_ASSERT(system_font_family_list);

	XMLDocument* config = nullptr;

 #if XX_ANDROID
	config = new XMLDocument();

	if (config->LoadFile("/system/etc/fonts.xml") == XML_NO_ERROR) {
		auto root = config->RootElement();
		// set first
		const XMLElement* first = root->FirstChildElement("family");
		
		if ( first && (first = first->FirstChildElement("font")) ) {
			cchar* path = first->GetText();
			if ( path ) {
				system_first_font_family_name = 
					find_font_family_by_path(Path::format("%s/%s", *system_fonts_dir, path));
			}
		}
		// set second
		first = root->FirstChildElement();

		while (first) {
			if ( strcmp(first->Name(), "family") == 0 ) {

			 #if defined(DEBUG) && 0
				auto att = first->FirstAttribute();
				LOG("%s, Attributes:", first->Name());
				while ( att ) {
					LOG("     %s:%s", att->Name(), att->Value());
					att = att->Next();
				}
			 #endif
				auto lang = first->FindAttribute("lang");
				if ( lang && strcmp(lang->Value(), "zh-Hans") == 0 ) {

					if ( (first = first->FirstChildElement("font")) ) {
						cchar* path = first->GetText();
						if ( path ) {
							system_second_font_family_name = 
								find_font_family_by_path(Path::format("%s/%s", *system_fonts_dir, path));
						}
					}
					break;
				}
			}
			first = first->NextSiblingElement();
		}
	}
 #elif XX_LINUX 
 #endif // XX_ANDROID End

	delete config;
}

/**
 * @func get_system_font_family_cache()
 */
static bool get_system_font_family_cache() {
	
	String path = Path::temp(xx_font_family_info_cache);
	
	if ( !FileHelper::exists_sync(path) ) { // 这个文件不存在
		return false;
	}

	String data = FileHelper::read_file_sync(path);
	Array<String> ls = data.split('\n');
	
	if ( ls.length() != 2 ) {
		return false;
	}
	
	String check_code = ls[0]; // 第一行代码为文件校验码
	String json_str = ls[1];
	
	if ( check_code != hash(json_str) ) { // 验证文件是否被改动或损坏
		return false;
	}
	JSON json = JSON::parse(json_str);
	String sys_id = hash(sys::info()); // 系统标识
	String lib_version = NGUI_VERSION;

	if (sys_id != json["sys_id"].to_cstring()) { // 操作系统与是否升级
		return false;
	}
	if (lib_version != json["library_version"].to_cstring()) {// lib版本是否变化
		return false;
	}

	JSON familys = json["font_familys"];

	system_first_font_family_name = json["system_first_font_family_name"].to_string();
	system_second_font_family_name = json["system_second_font_family_name"].to_string();
	
	for ( int i = 0, len = familys.length(); i < len; i++ ) {
		JSON& item = familys[i];
		JSON& fonts = item["fonts"]; // fonts
		
		FontPool::SimpleFontFamily sffd = {
			item["path"].to_string(), // path
			item["family"].to_string(), // family
		};
		
		DLOG("family:%s, %s", item["family"].to_cstring(), item["path"].to_cstring());
		
		for ( int j = 0, o = fonts.length(); j < o; j++ ) {
			JSON& font = fonts[j];
			sffd.fonts.push({
				font[0].to_string(),  // name
				TextStyleEnum(font[1].to_uint()), // style
				font[2].to_uint(),    // num_glyphs
				font[3].to_int(),     // height
				font[4].to_int(),     // max_advance
				font[5].to_int(),     // ascender
				font[6].to_int(),     // descender
				font[7].to_int(),     // underline_position
				font[8].to_int(),     // underline_thickness
			});
			
			// LOG("       %s", *JSON::stringify(font));
		}
		system_font_family_list->push( move(sffd) );
	}
	return true; // ok
}

/**
 * @func initialize_default_fonts()
 */
void FontPool::Inl::initialize_default_fonts() {
	
	Array<String> first;    // 第二默认字体(英文字符集)
	Array<String> second;   // 第二默认字体
	Array<String> third;    // 第三默认字体
	
	if ( !system_first_font_family_name.is_empty() ) {
		first.push(system_first_font_family_name);
	}
	if ( !system_second_font_family_name.is_empty() ) {
		second.push(system_second_font_family_name);
	}
	
 #if XX_IOS || XX_OSX
	first.push("Helvetica Neue");
	first.push("Helvetica");
	first.push("Thonburi");
	second.push("PingFang HK");
	second.push("HeitiFallback");
	second.push("Heiti TC");
	second.push(".HeitiFallback");
 #elif XX_ANDROID
	first.push("Roboto");
	first.push("Droid Sans");
	first.push("Droid Sans Mono");
	second.push("Noto Sans CJK");
	second.push("Noto Sans CJK JP");
	second.push("Noto Sans SC");
	third.push("Droid Sans Fallback");
 #elif XX_LINUX
	first.push("Roboto");
	first.push("DejaVu Sans");
	first.push("DejaVu Sans Mono");
	second.push("Noto Sans CJK");
	second.push("Noto Sans CJK JP");
	second.push("Noto Sans SC");
	second.push("AR PL UMing CN");
	third.push("Droid Sans Fallback");
 #endif
	
	set_default_fonts(&first, &second, &third, nullptr);
}

/**
 * @func system_font_family
 */
const SimpleFontList& FontPool::system_font_family() {

	if ( system_font_family_list ) {
		return *system_font_family_list;
	}

	system_font_family_list = new Array<SimpleFontFamily>();

	// 先读取缓存文件,如果找不到缓存文件遍历字体文件夹
	if ( get_system_font_family_cache() ) {
		return *system_font_family_list;
	}

	String sys_id = hash( sys::info() ); // 系统标识
	
	FT_Library ft_lib;
	FT_Init_FreeType( &ft_lib );
	
	ScopeClear clear([&ft_lib]() { FT_Done_FreeType(ft_lib); });
	
	JSON font_familys = JSON::array();
	
	FileHelper::each_sync(system_fonts_dir, Cb([&](Se& d) {
		
		Dirent* ent = static_cast<Dirent*>(d.data);
		
		if ( ent->type == FILE_FILE ) {
			Handle<SimpleFontFamily> sffd = Inl::inl_read_font_file(ent->pathname, ft_lib);
			
			if ( ! sffd.is_null() ) {
				JSON item = JSON::object();
				item["path"] = sffd->path;
				item["family"] = sffd->family;
				JSON fonts = JSON::array();
				
				for ( uint i = 0; i < sffd->fonts.length(); i++ ) {
					SimpleFont& sfd = sffd->fonts[i];
					JSON font = JSON::array();
					font[0] = sfd.name;
					font[1] = int(sfd.style);
					font[2] = sfd.num_glyphs;
					font[3] = sfd.height;
					font[4] = sfd.max_advance;
					font[5] = sfd.ascender;
					font[6] = sfd.descender;
					font[7] = sfd.underline_position;
					font[8] = sfd.underline_thickness;
					fonts[i] = font;
				}
				item[ "fonts" ] = fonts;
				font_familys[ font_familys.length() ] = item;
				system_font_family_list->push( move(**sffd) );
			}
		}
		d.return_value = 1;
	}));
	
	parse_system_font_family_name();
	
	JSON json = JSON::object();
	json["sys_id"] = sys_id;
	json["library_version"] = NGUI_VERSION;
	json["system_first_font_family_name"] = system_first_font_family_name;
	json["system_second_font_family_name"] = system_second_font_family_name;
	json["font_familys"] = font_familys;
	String json_str = JSON::stringify( json );
	String data = String::format( "%s\n%s", *hash(json_str), *json_str );
	FileHelper::write_file_sync(Path::temp(xx_font_family_info_cache), data); // 写入文件
	
	return *system_font_family_list;
}

XX_END
