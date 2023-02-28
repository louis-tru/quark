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
#include <gif_lib.h>

namespace quark {

	struct GifSource {
		cBuffer* buff;
		uint32_t index;
	};

	int GifInputFunc(GifFileType * gif, GifByteType* data, int size) {
		GifSource* source = (GifSource*)gif->UserData;
		memcpy(data, source->buff->val() + source->index, size);
		source->index += size;
		return size;
	}

	bool img_gif_decode(cBuffer& data, Array<Pixel> *rv) {
		
		GifSource source = { &data, 0 };
		GifFileType* gif = DGifOpen(&source, GifInputFunc, NULL);
		
		if ( ! gif )
			return false;
		
		CPointer<GifFileType> scope(gif, [](GifFileType* gif) {
			DGifCloseFile(gif, NULL);
		});

		if ( DGifSlurp(gif) == GIF_ERROR )
			return false;

		uint32_t width = gif->SWidth;
		uint32_t height = gif->SHeight;
		uint32_t row_size = width * 2;

		for ( int i = 0; i < gif->ImageCount; i++ ) { // 暂时只读取一张图像
			auto buff = Buffer::alloc(row_size * height); // RGBA5551
			memset(*buff, 0, buff.length());

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
				uint16_t* out = (uint16_t*)(buff.val() + ((desc->Top + row) * row_size) + desc->Left * 2);
				
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

			PixelInfo info(width, height, kColor_Type_RGBA_5551, kAlphaType_Unpremul);

			rv->push( Pixel(info, buff) );
		}

		return true;
	}

	bool img_gif_test(cBuffer& data, PixelInfo *out) {
		GifSource source = { &data, 0 };
		GifFileType* gif = DGifOpen(&source, GifInputFunc, NULL);
		
		if ( ! gif )
			return false;
		
		uint32_t w = gif->SWidth;
		uint32_t h = gif->SHeight;
		DGifCloseFile(gif, NULL);

		*out = PixelInfo(w, h, kColor_Type_RGBA_5551, kAlphaType_Unpremul);

		return true;
	}

}