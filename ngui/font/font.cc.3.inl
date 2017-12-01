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
#include "ngui/base/json.h"
#include "tinyxml2.h"

XX_NS(ngui)

using namespace tinyxml2;

#define xx_font_family_info_save ".simple_font_family_info"

#if XX_IOS
static cString system_fonts_dir = "/System/Library/Fonts";
#elif XX_ANDROID
static cString system_fonts_dir = "/system/fonts";
static cString system_fonts_config = "/system/etc/fonts.xml";
#elif XX_OSX
static cString system_fonts_dir = "/System/Library/Fonts";
#endif

static Array<FontPool::SimpleFontFamily>* sys_font_family_info = nullptr;
static String first_sys_font_family;
static String second_sys_font_family;

/**
 * @func find_font_family_by_path(path)
 */
static String find_font_family_by_path(cString& path) {
  XX_ASSERT(sys_font_family_info);
  for ( auto& i : *sys_font_family_info ) {
    if (i.value().path == path) {
      return i.value().family;
    }
  }
  return String();
}

/**
 * @func read_system_default_font_family()
 */
static void read_system_default_font_family() {
  XX_ASSERT(sys_font_family_info);
#if XX_ANDROID
  XMLDocument doc;
  XMLError r = doc.LoadFile(*system_fonts_config);
  
  if ( r == XML_NO_ERROR ) {
    auto root = doc.RootElement();

    // set first
    const XMLElement* first = root->FirstChildElement("family");
    
    if ( first && (first = first->FirstChildElement("font")) ) {
      cchar* path = first->GetText();
      if ( path ) {
        first_sys_font_family = find_font_family_by_path(Path::format("%s/%s", *system_fonts_dir, path));
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
              second_sys_font_family = find_font_family_by_path(Path::format("%s/%s", *system_fonts_dir, path));
            }
          }
          break;
        }
      }
      first = first->NextSiblingElement();
    }
  }
#endif
}

/**
 * @func read_system_font_family
 */
static bool read_system_font_family() {
  
  String path = Path::temp(xx_font_family_info_save);
  
  if ( FileHelper::exists_sync(path) ) { // 这个文件存在
    String data = FileHelper::read_file_sync(path);
    Array<String> ls = data.split('\n');
    
    if ( ls.length() != 2 )
      return false;
    
    String check_code = ls[0]; // 第一行代码为文件校验码
    String json_str = ls[1];
    
    if ( check_code == hash(json_str) ) { // 验证文件是否被改动或损坏
      JSON json = JSON::parse(json_str);
      String sys_id = hash(sys::info()); // 系统标识
      String lib_version = NGUI_VERSION;
      
      if (sys_id == json["sys_id"].to_cstring() && // 操作系统与是否升级
          lib_version == json["library_version"].to_cstring() // lib版本是否变化
      ) {
        JSON familys = json["font_familys"];
        
        first_sys_font_family = json["first_sys_font_family"].to_string();
        second_sys_font_family = json["second_sys_font_family"].to_string();
        
        for ( int i = 0, len = familys.length(); i < len; i++ ) {
          JSON& item = familys[i];
          JSON& fonts = item["fonts"]; // fonts
          
          FontPool::SimpleFontFamily sffd = {
            item["path"].to_string(), // path
            item["family"].to_string(), // family
          };
          
          //LOG("family:%s, %s", item["family"].to_cstring(), item["path"].to_cstring());
          
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
            
            //LOG("       %s", *JSON::stringify(font));
          }
          sys_font_family_info->push( move(sffd) );
        }
        return true; // ok
      }
    }
  }
  return false;
}

/**
 * @func initialize_default_fonts
 */
void FontPool::Inl::initialize_default_fonts() {
  
  Array<String> first;    // 第二默认字体(英文字符集)
  Array<String> second;   // 第二默认字体
  Array<String> third;    // 第三默认字体
  Array<String> four;
  
  if ( !first_sys_font_family.is_empty() ) {
    first.push(first_sys_font_family);
  }
  if ( !second_sys_font_family.is_empty() ) {
    second.push(second_sys_font_family);
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
  second.push("Noto Sans SC");
  second.push("Droid Sans Fallback");
#endif
  
  set_default_fonts(&first, &second, &third, &four, nullptr);
}

/**
 * @func system_font_family
 */
const Array<FontPool::SimpleFontFamily>& FontPool::system_font_family() {
  
  if ( sys_font_family_info ) {
    return *sys_font_family_info;
  }

  sys_font_family_info = new Array<SimpleFontFamily>();
  
  // 先读取缓存文件,如果找不到缓存文件遍历字体文件夹
  if ( ! read_system_font_family() ) {
    
    String sys_id = hash( sys::info() ); // 系统标识
    
    FT_Library ft_lib;
    FT_Init_FreeType( &ft_lib );
    
    ScopeClear clear([&ft_lib]() { FT_Done_FreeType( ft_lib ); });
    
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
          
          for ( int i = 0; i < sffd->fonts.length(); i++ ) {
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
          sys_font_family_info->push( move(**sffd) );
        }
      }
      d.return_value = 1;
    }));
    
    read_system_default_font_family();
    
    JSON json = JSON::object();
    json["sys_id"] = sys_id;
    json["library_version"] = NGUI_VERSION;
    json["first_sys_font_family"] = first_sys_font_family;
    json["second_sys_font_family"] = second_sys_font_family;
    json["font_familys"] = font_familys;
    String json_str = JSON::stringify( json );
    // LOG(json_str);
    
    String data = String::format( "%s\n%s", *hash(json_str), *json_str );
    FileHelper::write_file_sync(Path::temp(xx_font_family_info_save), data); // 写入文件
  }
  
  return *sys_font_family_info;
}

XX_END
