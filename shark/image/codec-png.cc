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

#include "../image-codec.h"
#include <png.h>

XX_NS(shark)

struct PngDataSource {
	cBuffer* buff;
	uint index;
};

static void png_rw_fn(png_structp png, png_bytep bytep, png_size_t size) {
	PngDataSource* s = (PngDataSource*)png->io_ptr;
	memcpy(bytep, s->buff->value() + s->index, size);
	s->index += size;
}

Array<PixelData> PNGImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv;
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	ScopeClear scope([&png]() {
		png_destroy_read_struct(&png, NULL, NULL);
	});
	
	png_infop info = png_create_info_struct(png);
	
	if ( ! info ) {
		return rv;
	}
	if ( setjmp(png_jmpbuf(png)) ) {
		return rv;
	}
	
	PngDataSource source = { &data, 0 };
	png_set_read_fn(png, &source, png_rw_fn);
	png_read_info(png, info);
	
	png_uint_32 w, h;
	int bit_depth;
	int color_type;
	png_uint_32 r = png_get_IHDR(png, info, &w, &h, &bit_depth, &color_type, NULL, NULL, NULL);
	
	// PNG_COLOR_TYPE_GRAY          // 灰度图像,1,2,4,8或16 (1/2/4/8/16)
	// PNG_COLOR_TYPE_PALETTE       // 索引彩色图像,1,2,4或8 (4/8/16/32)
	// PNG_COLOR_TYPE_RGB           // 真彩色图像,8或16 (24/48)
	// PNG_COLOR_TYPE_RGB_ALPHA     // 带α通道数据的真彩色图像,8或16 (32/64)
	// PNG_COLOR_TYPE_GRAY_ALPHA    // 带α通道数据的灰度图像,8或16 (16/32)
	
	if ( bit_depth == 16 ) {
		png_set_strip_16(png);
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(png);
	}
	if ( bit_depth < 8 ) {
		png_set_expand_gray_1_2_4_to_8(png);
	}
	if ( png_get_valid(png, info, PNG_INFO_tRNS) ) {
		png_set_expand(png);
	}
	
	png_read_update_info(png, info);
	
	png_uint_32 rowbytes = png_get_rowbytes(png, info);
	png_uint_32 channel = rowbytes / w;
	PixelData::Format format;
	
	switch (channel) {
		case 1: format = PixelData::LUMINANCE8; break;
		case 2: format = PixelData::LUMINANCE_ALPHA88; break;
		case 3: format = PixelData::RGB888; break;
		case 4: format = PixelData::RGBA8888; break;
		default: // unknown error
			return rv;
	}
	
	Buffer buff((uint)(h * rowbytes));
	Array<png_bytep> row_pointers((uint)h);
	
	for (uint i = 0; i < h; i++) {
		row_pointers[i] = (byte*)buff.value() + rowbytes * i;
	}
	png_read_image(png, &row_pointers[0]);
	png_read_end(png, info);
	
	rv.push( PixelData(buff, (uint)w, (uint)h, format, false) );
	return rv;
}

PixelData PNGImageCodec::decode_header(cBuffer& data) {
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	ScopeClear scope([&png]() {
		png_destroy_read_struct(&png, NULL, NULL);
	});
	
	png_infop info = png_create_info_struct(png);
	
	if ( ! info ) {
		return PixelData();
	}
	if ( setjmp(png_jmpbuf(png)) ) {
		return PixelData();
	}
	
	PngDataSource source = { &data, 0 };
	png_set_read_fn(png, &source, png_rw_fn);
	png_read_info(png, info);
	
	png_uint_32 w, h;
	int bit_depth;
	int color_type;
	png_uint_32 r = png_get_IHDR(png, info, &w, &h, &bit_depth, &color_type, NULL, NULL, NULL);
	
	if ( bit_depth == 16 ) {
		png_set_strip_16(png);
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(png);
	}
	if ( bit_depth < 8 ) {
		png_set_expand_gray_1_2_4_to_8(png);
	}
	if ( png_get_valid(png, info, PNG_INFO_tRNS) ) {
		png_set_expand(png);
	}
	
	png_read_update_info(png, info);
	
	png_uint_32 rowbytes = png_get_rowbytes(png, info);
	png_uint_32 channel = rowbytes / w;
	PixelData::Format format;
	
	switch (channel) {
		case 1: format = PixelData::LUMINANCE8; break;
		case 2: format = PixelData::LUMINANCE_ALPHA88; break;
		case 3: format = PixelData::RGB888; break;
		case 4: format = PixelData::RGBA8888; break;
		default: // unknown error
			return PixelData();
	}
	return PixelData(Buffer(), (uint)w, (uint)h, format, false);
}

Buffer PNGImageCodec::encode(const PixelData& pixel_data) {
	return Buffer();
}

XX_END