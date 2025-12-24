/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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

namespace qk {

	struct GifSource { cBuffer* buff; uint32_t index; };

	int GifInputFunc(GifFileType * gif, GifByteType* data, int size) {
		GifSource* source = (GifSource*)gif->UserData;
		memcpy(data, source->buff->val() + source->index, size);
		source->index += size;
		return size;
	}

	bool img_gif_decode(cBuffer& data, Array<Pixel> *rv) {
		GifSource source = { &data, 0 };
		GifFileType* gif = DGifOpen(&source, GifInputFunc, nullptr);

		if (!gif) return false;

		CPointerHold<GifFileType> scope(gif, [](GifFileType* gif) {
			DGifCloseFile(gif, nullptr);
		});

		if (DGifSlurp(gif) == GIF_ERROR) return false;

		typedef uint16_t type_t;
		uint32_t width = gif->SWidth;
		uint32_t height = gif->SHeight;
		uint32_t rowbytes = width * sizeof(type_t);

		for ( int i = 0; i < gif->ImageCount; i++ ) {
			Buffer buff(rowbytes * height); // RGBA5551
			SavedImage* frame = gif->SavedImages + i;
			GifImageDesc* desc = &frame->ImageDesc;
			ColorMapObject* colorMap =  desc->ColorMap ? desc->ColorMap : gif->SColorMap;

			memset(*buff, 0, buff.length());

			int transColor = -1;
			for ( int k = 0; k < frame->ExtensionBlockCount; k++ ) {
				ExtensionBlock* block = frame->ExtensionBlocks + k;
				if ( block->Function == GRAPHICS_EXT_FUNC_CODE && block->ByteCount == 4) {
					// int delay = (block->Bytes[2] << 8 | block->Bytes[1]) * 10;
					/* Can sleep here */
					if( (block->Bytes[0] & 1) == 1 ) {
						transColor = block->Bytes[3];
					}
					break;
				}
			}

			for (int row = 0; row < desc->Height; row++) {
				auto pix = frame->RasterBits + row * desc->Width;
				auto end = pix + desc->Width;
				auto out = ((type_t*)buff.val()) + (desc->Top + row) * width + desc->Left;
				while (pix != end) {
					if (transColor != *pix) {
						auto color = colorMap->Colors[*pix];
						// kRGBA_5551_ColorType
						*out =  ((color.Red >> 3) << 11) |
										((color.Green >> 3) << 6) |
										((color.Blue >> 3) << 1) | 1;
						// kRGBA_565_ColorType
						// *out =  ((color.Red >> 3) << 11) |
						// 				((color.Green >> 2) << 5) |
						// 				((color.Blue >> 3) << 0);
						// kRGBA_4444_ColorType
						// *out =  ((color.Red >> 4) << 12) |
						// 				((color.Green >> 4) << 8) |
						// 				((color.Blue >> 4) << 4) | 0b1111;
						// kRGBA_1010102_ColorType
						// *out =  ((color.Red << 2) << 22) |
						// 				((color.Green << 2) << 12) |
						// 				((color.Blue << 2) << 2) | 0b11;
					} // else transparent
					pix++; out++;
				}
			}
			PixelInfo info(width, height, kRGBA_5551_ColorType, kUnpremul_AlphaType);
			rv->push(Pixel(info, buff));
		}

		return true;
	}

	bool img_gif_test(cBuffer& data, PixelInfo *out) {
		GifSource source = { &data, 0 };
		GifFileType* gif = DGifOpen(&source, GifInputFunc, nullptr);
		
		if ( ! gif )
			return false;

		uint32_t w = gif->SWidth;
		uint32_t h = gif->SHeight;
		DGifCloseFile(gif, nullptr);

		*out = PixelInfo(w, h, kRGBA_5551_ColorType, kUnpremul_AlphaType);

		return true;
	}

}
