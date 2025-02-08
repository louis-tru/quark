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
#include <webp/decode.h>

namespace qk {

	bool img_webp_test(cBuffer& data, PixelInfo *out) {
		int width = 0, height = 0;
		int ok = WebPGetInfo((uint8_t*)data.val(), data.length(), &width, &height);
		if ( ok == VP8_STATUS_OK ) {
			*out = PixelInfo( width, height, kRGBA_8888_ColorType, kUnpremul_AlphaType);
			return true;
		}
		return false;
	}

	bool img_webp_decode(cBuffer& data, Array<Pixel> *rv) {
		int width, height;
		uint8_t* buff = WebPDecodeRGBA((uint8_t*)data.val(), data.length(), &width, &height);
		if (buff) {
			auto bf = Buffer::from((char*)buff, width * height * 4);
			rv->push( Pixel( PixelInfo(width, height, kRGBA_8888_ColorType, kUnpremul_AlphaType), bf) );
		}
		return true;
	}

}
