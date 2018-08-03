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

#include <ngui/base/util.h>
#include <ngui/base/string.h>
#include <ngui/base/map.h>
#include <ngui/base/fs.h>
#include <ngui/base/sys.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftimage.h>
#include <freetype/ftoutln.h>

#include <time.h>
#include <chrono>

//
#include "ngui/image-codec.h"
#include "ngui/texture.h"
#include "ngui/image.h"
#include "ngui/sprite.h"
#include "ngui/box.h"
#include "ngui/app.h"
#include "ngui/display-port.h"
#include "ngui/root.h"
#include "ngui/gl/gl.h"

using namespace ngui;

void each_fonts () {
	
	FT_Library library;
	FT_Init_FreeType(&library);
	FT_Face face = NULL;
	FT_Error error;
	
	auto ns = std::chrono::system_clock::now().time_since_epoch();
	int64 st = std::chrono::duration_cast<std::chrono::milliseconds>(ns).count(), st2;
	
	LOG("start st:%d", st);
	
	int count = 0;
	
	//  String path0 = Path::format("%s/res/SF-UI", *Path::resources_dir());
	String path0 = "/System/Library/Fonts";
	
	FileHelper::each_sync(path0, Cb([&](Se& d) {
		
		Dirent* ent = static_cast<Dirent*>(d.data);
		
		if ( ent->type == FILE_FILE ) {
			
			error = FT_New_Face(library, *ent->pathname, 0, &face);
			
			if (error) {
				LOG("error,------------------------path:%s", *ent->pathname);
			}
			else {
				
				LOG("family:------------------------%s | num_faces:%d | path:%s",
							face->family_name,
							face->num_faces,
							*ent->pathname);
				
				int i = 0;
				
				while(1) {
					
					cchar* name = FT_Get_Postscript_Name(face);
					
					LOG("font:%s | style:%s | style_flags:%d | glyphs:%d",
								name,
								face->style_name,
								face->style_flags,
								face->num_glyphs);
					
					//          FT_STYLE_FLAG_BOLD
					
					count++;
					
					FT_Done_Face(face);
					
					i++;
					
					if (i < face->num_faces) {
						error = FT_New_Face(library, *ent->pathname, i, &face);
						if (error) {
							LOG("error"); break;
						}
					} else {
						break;
					}
				}
			}
		}
		
	 d.return_value = 1;
	}));
	
	ns = std::chrono::system_clock::now().time_since_epoch();
	st2 = std::chrono::duration_cast<std::chrono::milliseconds>(ns).count();
	
	LOG("end st:%d,%d", st2, st2 - st);
	
	LOG("font count:%d", count);
	
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void each_glyph() {
	
	FT_Library library;
	FT_Init_FreeType(&library);
	FT_Face face = NULL;
	FT_Error error;
	
	//
	//  String font_path = FileSearch::share()->get_absolute_path("res/font/SourceHanSansCN-Regular.otf");
	//  String font_path = FileSearch::share()->get_absolute_path("res/font/SF-UI-Display-Regular.otf");
	String font_path = FileSearch::shared()->get_absolute_path("res/font/DejaVuSerif.ttf");
	//  String font_path = FileSearch::share()->get_absolute_path("res/font/lateef.ttf");
	//  String font_path = "/System/Library/Fonts/LanguageSupport/PingFang.ttc";
	
	cchar* text = "所有OpenType字体";
	
	error = FT_New_Face(library, *font_path, 0, &face);
	
	XX_ASSERT(!error);
	
	error = FT_Set_Char_Size(face, 0, 12 * 64, 300, 300);
	
	XX_ASSERT(!error);
	
	error = FT_Set_Pixel_Sizes(face, 0, 12);
	
	XX_ASSERT(!error);
	
	FT_GlyphSlot gl = face->glyph;
	
	//  FT_Select_Charmap(face, FT_Encoding::FT_ENCODING_UNICODE);
	//  error = FT_Set_Charmap(face, face->charmaps[0]);
	//  error = FT_Set_Charmap(face, face->charmaps[1]);
	
	XX_ASSERT(!error);
	
	uint ch[6] = { 0, 26970, 23398, 25991, 65533, 65 }; // 楚学文�A
	
	FT_UInt glyph_index = FT_Get_Char_Index( face, 65533 );
	
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	
	uint unicode = 0;
	uint count = 0;
	
	LOG("%s", *Coder::encoding(Encoding::utf8, Ucs2String((uint16*)&unicode, 1)) );
	
	do {
		unicode = FT_Get_Next_Char(face, unicode, &glyph_index);
		
		Buffer data = Coder::encoding(Encoding::utf8, Ucs2String((uint16*)&unicode, 1));
		
		LOG("unicode:%d, glyph_index:%d, char:%s", unicode, glyph_index, *data);
		count++;
		
	} while(glyph_index);
	
	LOG("count:%d", count);
	
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	
	XX_ASSERT(!error);
	
	FT_Pos x, y;
	
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void onload_f(Event<>& evt, void* user) {
	FT_Library library;
	FT_Init_FreeType(&library);
	FT_Face face = NULL;
	FT_Error error;
	
	String font_path = FileSearch::shared()->get_absolute_path("res/font/SourceHanSansCN-Regular.otf");
	
	error = FT_New_Face(library, *font_path, 0, &face);
	
	XX_ASSERT(!error);
	
	float font_size = 16;
	
	error = FT_Set_Char_Size(face, 0, font_size * 64, 72, 72);
	
	XX_ASSERT(!error);
	
	//  error = FT_Set_Pixel_Sizes(face, 0, 64);
	
	XX_ASSERT(!error);
	
	LOG("VERTICAL:%i", FT_HAS_VERTICAL(face));
	
	FT_GlyphSlot gl = face->glyph;
	
	FT_Glyph_Metrics& metrics = gl->metrics;
	
	uint ch[6] = { 0, 26970, 23398, 25991, 65533, 'A' }; // 楚学文�A
	
	FT_UInt glyph_index = FT_Get_Char_Index( face, '\t' );
	
	LOG("glyph_index:%d", glyph_index);
	
	error = FT_Load_Glyph(face, glyph_index, /*FT_LOAD_NO_HINTING*/FT_LOAD_DEFAULT);
	
	/*    If the glyph has been loaded with @FT_LOAD_NO_SCALE, `bbox_mode'   */
	/*    must be set to @FT_GLYPH_BBOX_UNSCALED to get unscaled font        */
	/*    units in 26.6 pixel format.  The value @FT_GLYPH_BBOX_SUBPIXELS    */
	/*    is another name for this constant.                                 */
	
	FT_Glyph glyph;
	
	error = FT_Get_Glyph( gl, &glyph );
	
	XX_ASSERT(!error);
	
	FT_BBox bbox;
	
	FT_Glyph_Get_CBox( glyph, FT_LOAD_NO_SCALE, &bbox );
	
	XX_ASSERT(!error);
	
	if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
		FT_Outline_Embolden(&(gl->outline), 16); // 加粗轮廓线
	}
	
	if (gl->format != FT_GLYPH_FORMAT_BITMAP) {
		error = FT_Render_Glyph(gl, FT_RENDER_MODE_NORMAL);
		XX_ASSERT(!error);
	}
	
	FT_Bitmap bit = gl->bitmap;
	
	LOG("width:%d, height:%d", bit.width, bit.rows);
	
	PixelData data(WeakBuffer((char*)bit.buffer, bit.width * bit.rows),
								 bit.width, bit.rows, PixelData::ALPHA8);
	
	Root* r = New<Root>();
	
	r->set_content_align(ContentAlign::LEFT);
		
	Image* img = New<Image>();
	
	Texture* tex = Texture::create(data);
	
	//  img->margin_top(50);
	img->set_margin(50);
	
	img->set_width(font_size);
	
	//  img->width({ Box::vPERCENT, .1 });
	
	img->set_texture(tex);
	
	img->append_to(r);
	
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void draw_char() {
	GUIApplication app;
	app.XX_ON(load, onload_f);
	app.run();
}

void test_freetype() {
	
	LOG(sys::info());
	
	//  each_fonts();
	
	//  each_glyph();
	
	draw_char();
}

