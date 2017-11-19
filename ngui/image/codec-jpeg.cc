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
#include <setjmp.h>

extern "C" {
#include <jpeglib.h>
}

XX_NS(ngui)

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
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = jpeg_error_output;
  
  JPEGClientData client_data;
  cinfo.client_data = &client_data;
  jpeg_create_decompress(&cinfo);
  
  ScopeClear clear([&cinfo]() {
    jpeg_destroy_decompress(&cinfo);
  });
  jpeg_mem_src(&cinfo, (byte*)*data, data.length());
  
  if ( setjmp(client_data.jmpbuf) ) {
    return rv;
  }
  
  jpeg_read_header(&cinfo, TRUE);
  
  uint w = cinfo.image_width;
  uint h = cinfo.image_height;
  uint num = cinfo.num_components;
  
  if (num == 1 || num == 3) {
    uint rowbytes = w * num;
    uint count = w * h * num;
    Buffer buff(count);
    
    jpeg_start_decompress(&cinfo);
    JSAMPROW row;
    while (cinfo.output_scanline < cinfo.output_height) {
      row = (byte*)buff.value() + cinfo.output_scanline * rowbytes;
      jpeg_read_scanlines(&cinfo, &row, 1);
    }
    jpeg_finish_decompress(&cinfo);
    
    rv.push( PixelData(buff, w, h, num == 1 ? PixelData::LUMINANCE8 : PixelData::RGB888, false) );
  }
  return rv;
}

PixelData JPEGImageCodec::decode_header(cBuffer& data) {
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = jpeg_error_output;
  
  JPEGClientData client_data;
  cinfo.client_data = &client_data;
  jpeg_create_decompress(&cinfo);
  
  ScopeClear clear([&cinfo]() {
    jpeg_destroy_decompress(&cinfo);
  });
  jpeg_mem_src(&cinfo, (byte*)*data, data.length());
  
  if ( setjmp(client_data.jmpbuf) ) {
    return PixelData();
  }
  
  jpeg_read_header(&cinfo, TRUE);
  
  uint w = cinfo.image_width;
  uint h = cinfo.image_height;
  uint num = cinfo.num_components;
  
  return PixelData(Buffer(), w, h,
                   num == 1 ? PixelData::LUMINANCE8:
                   num == 3 ? PixelData::RGB888: PixelData::INVALID, false);
}

Buffer JPEGImageCodec::encode(const PixelData& pixel_data) {
  return Buffer();
}

XX_END