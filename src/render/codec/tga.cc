/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./codec.h"

namespace qk {

	struct TGAHeader {
		#pragma pack(push,1)
		uint8_t idlength;              /* 00h Size of Image ID field */
		uint8_t color_map_type;        /* 01h Color map type */
		// 2-rgb
		// 3-grayscale
		// 10-rle rgb
		// 11-rle grayscale
		uint8_t data_type_code;        /* 02h Image type code */
		uint16_t color_map_origin;    /* 03h Color map origin */
		uint16_t color_map_length;    /* 05h Color map length */
		uint8_t color_map_depth;       /* 07h Depth of color map entries */
		uint16_t x_origin;            /* 08h X origin of image */
		uint16_t y_origin;            /* 0Ah Y origin of image */
		uint16_t width;               /* 0Ch Width of image */
		uint16_t height;              /* 0Eh Height of image */
		// 16、24、32
		uint8_t bits_per_pixel;        /* 10h Image pixel size */
		uint8_t image_descriptor;      /* 11h Image descriptor byte */
		#pragma pack(pop)
	};

	typedef void (*TGAReadDataBlackFunc)(uint8_t** in, uint8_t** out, int alpha);
	
	/**
	 * @func tga_parse_rgb_rle # 解析RLE RGB图
	 * @arg data {const byte*} # 图像数据指针
	 * @arg new_data {byte*} # 新的通胀图像数据指针
	 * @arg pixex_size {int} # 图像像素数量
	 * @arg func {ReadDataBlackFunc} # 处理函数
	 * @private
	 */
	void tga_parse_rgb_rle(uint8_t* in, uint8_t* out,
											int bytes,
											int pixex_size, TGAReadDataBlackFunc func, int alpha) {
		for (int i = 0; i < pixex_size; i++) {
			uint8_t mask = in[0];
			in++;
			(func)(&in, &out, alpha);
			
			int j = mask & 0x7f;    // data[0] & 01111111
			if (mask & 0x80) {       // data[0] & 10000000
				// 相同的像素
				uint8_t* cp = out - bytes;
				for (int k = 0; k < j; k++, i++) {
					memcpy(out, cp, bytes);
					out += bytes;
				}
			} else {
				for (int k = 0; k < j; k++, i++) {
					(func)(&in, &out, alpha);
				}
			}
		}
	}

	void tga_read_gray_data_black(uint8_t** in, uint8_t** out, int bytes, int alpha);
	
	// RLE 灰度图
	void tga_parse_gray_rle(uint8_t* in, uint8_t* out, int bytes, int pixex_size, int alpha) {
		for (int i = 0; i < pixex_size; i++) {
			uint8_t mask = in[0];
			in += 1;
			tga_read_gray_data_black(&in, &out, bytes, alpha);
			
			int j = mask & 0x7f;    // data[0] & 01111111
			if (mask & 0x80) {       // data[0] & 10000000
				// 相同的像素
				uint8_t* tmp = out - 2;
				for (int k = 0; k < j; k++, i++) {
					memcpy(out, tmp, 2);
					out += 4;
				}
			}
			else {
				for (int k = 0; k < j; k++, i++) {
					tga_read_gray_data_black(&in, &out, bytes, alpha);
				}
			}
		}
	}
	
	void tga_read_16_data_black(uint8_t** in, uint8_t** out, int alpha) {
		uint16_t* in_ = (uint16_t*)*in;
		uint16_t* out_ = (uint16_t*)*out;
		*out_ = (in_[0] << 1) | (alpha ? (in_[0] & 0x8000) : 1);
		*in = (uint8_t*)(in_ + 1);
		*out = (uint8_t*)(out_ + 1);
	}
	
	void tga_read_24_data_black(uint8_t** in, uint8_t** out, int alpha) {
		uint8_t* in_ = *in;
		uint8_t* out_ = *out;
		out_[2] = in_[0];
		out_[1] = in_[1];
		out_[0] = in_[2];
		*in = in_ + 3;
		*out = out_ + 3;
	}
	
	void tga_read_32_data_black(uint8_t** in, uint8_t** out, int alpha) {
		uint8_t* in_ = *in;
		uint8_t* out_ = *out;
		out_[2] = in_[0];
		out_[1] = in_[1];
		out_[0] = in_[2];
		out_[3] = alpha ? in_[3] : 255;
		*in = in_ + 4;
		*out = out_ + 4;
	}
	
	void tga_read_gray_data_black(uint8_t** in, uint8_t** out, int bytes, int alpha) {
		uint8_t* in_ = *in;
		uint8_t* out_ = *out;
		out_[0] = in_[0];
		out_[1] = alpha ? in_[1] : 255;
		*in = in_ + bytes;
		*out = out_ + bytes;
	}

	void tga_premultiplied_alpha(uint8_t* data, int pixex_size) {
		for(int i = 0; i < pixex_size; i++){
			float alpha = data[3] / 255;
			data[0] *= alpha;
			data[1] *= alpha;
			data[2] *= alpha;
			data += 4;
		}
	}

	Buffer tga_flip_vertical(cChar* data, int width, int height, int bytes) {
		
		Buffer rev = Buffer::alloc(width * height * bytes);
		char* p = *rev;
		int row_size = width * bytes;
		char* tmp = p + (height - 1) * row_size;
		
		for ( int i = 0; i < height; i++ ) {
			memcpy(tmp, data, row_size);
			tmp -= row_size;
			data += row_size;
		}
		return rev;
	}

	bool img_tga_test(cBuffer& data, PixelInfo* out) {
		TGAHeader* header = (TGAHeader*)*data;
		bool alpha = header->image_descriptor & 0x08;
		int bytes = header->bits_per_pixel / 8; // 2、3、4
		uint8_t code = header->data_type_code;
		ColorType format;
		
		if( bytes == 2) {
			if (code == 2 || code == 10) { // RGB | RLE RGB
				format = kRGBA_5551_ColorType;
			} else { // GRAY | RLE GRAY
				format = kLuminance_Alpha_88_ColorType;
			}
		} else if (bytes == 3) {
			format = kRGB_888_ColorType;
		} else {
			format = alpha ? kRGBA_8888_ColorType: kRGB_888X_ColorType;
		}

		*out = PixelInfo(header->width, header->height, format, kUnpremul_AlphaType);

		return true;
	}

	bool img_tga_decode(cBuffer& data, Array<Pixel> *pixel) {
		
		TGAHeader* header = (TGAHeader*)*data; // 适用小端格式CPU
		// parse image
		int alpha = header->image_descriptor & 0x08;
		int bytes = header->bits_per_pixel / 8; // 2、3、4
		uint8_t code = header->data_type_code;
		
		ColorType format;
		TGAReadDataBlackFunc func;
		
		if (bytes == 2) {
			if (code == 2 || code == 10) { // RGB | RLE RGB
				format = kRGBA_5551_ColorType; // RGBA5551
			} else { // GRAY | RLE GRAY
				format = kLuminance_Alpha_88_ColorType;
			}
			func = &tga_read_16_data_black;
		} else if (bytes == 3) {
			format = kRGB_888_ColorType;
			func = &tga_read_24_data_black;
		} else {
			format = alpha ? kRGBA_8888_ColorType: kRGB_888X_ColorType;
			func = &tga_read_32_data_black;
		}
		
		int width = header->width;
		int height = header->height;
		int pixex_size = width * height;
		int out_size = pixex_size * bytes;
		Buffer out = Buffer::alloc(out_size);
		uint8_t* out_p = (uint8_t*)out.val();
		uint8_t* in_p = ((uint8_t*)data.val()) + sizeof(TGAHeader) + header->idlength;
		
		switch ( code ) {
			case 2:  // RGB
			case 10: // RLE RGB
				if ( code == 2 ) { // RGB
					for ( int i = 0; i < pixex_size; i++ ) {
						func(&in_p, &out_p, alpha);
					}
				} else {  // RLE RGB
					tga_parse_rgb_rle(in_p, out_p, bytes, pixex_size, func, alpha);
				}
				break;
			case 3:  // GRAY
				for (int i = 0; i < pixex_size; i++) {
					out_p[0] = in_p[0];
					out_p[1] = alpha ? in_p[1] : 255;
					in_p  += bytes;
					out_p += bytes;
				}
				break;
			case 11: // RLE GRAY
				tga_parse_gray_rle(in_p, out_p, bytes, pixex_size, alpha);
				break;
			default:
				Qk_DLog("Parse tga image error, data type code undefined");
				return false;
		}
		
		// Indicates that the pixel starts from the bottom and needs to be adjusted
		if ( ! (header->image_descriptor & 0x20) ) {
		// BOTTOM_LEFT
			out = tga_flip_vertical(*out, width, height, bytes);
		}
		pixel->push( Pixel(PixelInfo(width, height, format, kUnpremul_AlphaType), out) );

		return true;
	}

	Buffer img_tga_encode(cPixel& pixel) {
		auto type = pixel.type();
		auto pix = &pixel;

		if (pix->type() == kRGBA_8888_ColorType ||
			type == kAlpha_8_ColorType || type == kLuminance_8_ColorType
		) {
			auto ret_data = Buffer::alloc(sizeof(TGAHeader) + pix->width() * pix->height() * 4);

			TGAHeader header;
			header.idlength = 0;
			header.color_map_type = 0;
			header.data_type_code = 2;
			header.color_map_origin = 0;
			header.color_map_length = 0;
			header.color_map_depth = 0;
			header.x_origin = 0;
			header.y_origin = 0;
			header.width = pix->width();
			header.height = pix->height();
			header.bits_per_pixel = 32;
			header.image_descriptor =  0x08 | 0x20; //alpha flag | top-left flag

			memcpy(*ret_data, &header, sizeof(TGAHeader));

			auto pixels = pix->width() * pix->height();
			auto src  = (const uint8_t*)pix->body().val();
			auto dest = (uint8_t*)*ret_data + sizeof(TGAHeader);

			if (pix->type() != kRGBA_8888_ColorType) {
				for ( int i = 0; i < pixels; i++ ) {
					dest[2] = 0;
					dest[1] = 0;
					dest[0] = 0;
					dest[3] = *src;
					dest += 4;
					src ++;
				}
			} else {
				// 写入BGRA数据
				if ( pix->alphaType() == kPremul_AlphaType ) {
					for (int i = 0; i < pixels; i++) {
						float alpha = src[3] / 255;
						dest[2] = src[0] / alpha;
						dest[1] = src[1] / alpha;
						dest[0] = src[2] / alpha;
						dest[3] = src[3];
						dest += 4;
						src += 4;
					}
				} else {
					for ( int i = 0; i < pixels; i++ ) {
						dest[2] = src[0];
						dest[1] = src[1];
						dest[0] = src[2];
						dest[3] = src[3];
						dest += 4;
						src += 4;
					}
				}
			}
			Qk_ReturnLocal(ret_data);
		}

		Qk_DLog("Pixel data: Invalid data, required for RGBA_8888 and Alpha_8 and Luminance_8 format");
		return Buffer();
	}

}
