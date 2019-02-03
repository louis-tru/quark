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

#include "qgr/image-codec.h"
#include <setjmp.h>

extern "C" {
#include <jpeglib.h>
}

XX_NS(qgr)

struct JPEGClientData {
	jmp_buf jmpbuf;
};

static void jpeg_error_output(j_common_ptr cinfo) {
	XX_ERR("%s", "Invalid JPEG file structure: missing SOS marker");
	JPEGClientData* data = (JPEGClientData*)cinfo->client_data;
	longjmp(data->jmpbuf, 0);
}

Array<PixelData> JPEGImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv;
	struct jpeg_decompress_struct jpeg;
	struct jpeg_error_mgr jerr;
	jpeg.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_error_output;
	
	JPEGClientData client_data;
	jpeg.client_data = &client_data;
	jpeg_create_decompress(&jpeg);

	jpeg_mem_src(&jpeg, (byte*)*data, data.length());
	
	if ( setjmp(client_data.jmpbuf) ) {
		jpeg_destroy_decompress(&jpeg);
		return rv;
	}
	
	jpeg_read_header(&jpeg, TRUE);

	uint w = jpeg.image_width;
	uint h = jpeg.image_height;
	int num = jpeg.num_components;
	JSAMPROW row;

	jpeg_start_decompress(&jpeg);

	if (num == 1) {
		uint rowbytes = w * num;
		uint count = h * rowbytes;
		Buffer buff(count);

		while(jpeg.output_scanline < jpeg.output_height) {
			row = (JSAMPROW)*buff + jpeg.output_scanline * rowbytes;
			jpeg_read_scanlines(&jpeg, &row, 1);
		}

		rv.push( PixelData(buff, w, h, PixelData::LUMINANCE8, false) );

	} else if (num == 3) {
		uint rowbytes = w * 4;
		uint count = h * rowbytes;
		Buffer buff(count);

		JSAMPARRAY rows = (*jpeg.mem->alloc_sarray)((j_common_ptr) &jpeg, JPOOL_IMAGE, w * num, 1);

		while(jpeg.output_scanline < jpeg.output_height) {
			row = (JSAMPROW)*buff + jpeg.output_scanline * rowbytes;
			jpeg_read_scanlines(&jpeg, rows, 1);

			JSAMPROW row2 = rows[0];

			for (uint column = 0; column < w; column++) {
				*((int*)row) = *((int*)row2);
				row[3] = 255;
				row += 4; row2 += 3;
			}
		}

		rv.push( PixelData(buff, w, h, PixelData::RGBX8888, false) );
	}

	jpeg_start_decompress(&jpeg);
	jpeg_destroy_decompress(&jpeg);

	return rv;
}

PixelData JPEGImageCodec::decode_header(cBuffer& data) {
	struct jpeg_decompress_struct jpeg;
	struct jpeg_error_mgr jerr;
	jpeg.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_error_output;
	
	JPEGClientData client_data;
	jpeg.client_data = &client_data;
	jpeg_create_decompress(&jpeg);
	
	ScopeClear clear([&jpeg]() {
		jpeg_destroy_decompress(&jpeg);
	});
	jpeg_mem_src(&jpeg, (byte*)*data, data.length());
	
	if ( setjmp(client_data.jmpbuf) ) {
		return PixelData();
	}
	
	jpeg_read_header(&jpeg, TRUE);
	
	uint w = jpeg.image_width;
	uint h = jpeg.image_height;
	int num = jpeg.num_components;

	return PixelData(Buffer(), w, h,
									 num == 1 ? PixelData::LUMINANCE8:
									 num == 3 ? PixelData::RGBX8888: PixelData::INVALID, false);
}

Buffer JPEGImageCodec::encode(const PixelData& pixel_data) {
	return Buffer();
}

XX_END