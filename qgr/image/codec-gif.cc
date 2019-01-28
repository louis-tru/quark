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
#include <gif_lib.h>

XX_NS(qgr)

struct GifSource {
	cBuffer* buff;
	uint index;
};

int GifInputFunc(GifFileType * gif, GifByteType* data, int size) {
	GifSource* source = (GifSource*)gif->UserData;
	memcpy(data, source->buff->value() + source->index, size);
	source->index += size;
	return size;
}

Array<PixelData> GIFImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv;
	
	GifSource source = { &data, 0 };
	GifFileType* gif = DGifOpen(&source, GifInputFunc, NULL);
	
	if ( ! gif ) { return rv; }
	
	ScopeClear scope([gif]() {
		DGifCloseFile(gif, NULL);
	});
	
	if ( DGifSlurp(gif) == GIF_ERROR ) {
		return rv;
	}
	
	uint width = gif->SWidth;
	uint height = gif->SHeight;
	uint row_size = width * 2;
	Buffer buff(row_size * height); // RGBA5551
	memset(*buff, 0, buff.length());
	
	for ( int i = 0; i < 1/*gif->ImageCount*/; i++ ) { // 暂时只读取一张图像
		
		SavedImage* image = gif->SavedImages + i;
		GifImageDesc* desc = &image->ImageDesc;
		ColorMapObject* ColorMap =  desc->ColorMap ? desc->ColorMap : gif->SColorMap;
		
		int trans_color = -1;
		for ( int k = 0; k < image->ExtensionBlockCount; k++ ) {
			ExtensionBlock* block = image->ExtensionBlocks + k;
			if ( block->Function == GRAPHICS_EXT_FUNC_CODE && block->ByteCount == 4) {
				// int delay = (block->Bytes[2] << 8 | block->Bytes[1]) * 10;
				/* Can sleep here */
				if( (block->Bytes[0] & 1) == 1 ) {
					trans_color = block->Bytes[3];
				}
				break;
			}
		}
		
		for ( int row = 0; row < desc->Height; row++ ) {
			GifByteType* in = image->RasterBits + row * desc->Width;
			uint16* out = (uint16*)(buff.value() + ((desc->Top + row) * row_size) + desc->Left * 2);
			
			for ( int col = 0; col < desc->Width; col++ ) {
				if ( trans_color == -1 || trans_color != in[col] ) { //
					GifColorType* color = ColorMap->Colors + in[col];
					*out =  ((color->Red >> 3) << 11) |
									((color->Green >> 3) << 6) |
									((color->Blue >> 3) << 1) | 1;
				} // else  transparent
				in++; out++;
			}
		}
	}
	
	rv.push( PixelData(buff, width, height, PixelData::RGBA5551, false) );
	return rv;
}

PixelData GIFImageCodec::decode_header(cBuffer& data) {
	GifSource source = { &data, 0 };
	GifFileType* gif = DGifOpen(&source, GifInputFunc, NULL);
	
	if ( ! gif ) { return PixelData(); }
	
	uint w = gif->SWidth;
	uint h = gif->SHeight;
	DGifCloseFile(gif, NULL);
	
	return PixelData( Buffer(), w, h, PixelData::RGBA5551, false );
}

Buffer GIFImageCodec::encode(const PixelData& pixel_data) {
	return Buffer();
}

XX_END