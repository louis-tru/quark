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

XX_NS(ngui)

static PixelData xx_decode(cBuffer& data) {
	return PixelData();
}

static PixelData xx_decode_header(cBuffer& data) {
	return PixelData();
}

Array<PixelData> JPEGImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(xx_decode(data));
	return rv;
}

PixelData JPEGImageCodec::decode_header(cBuffer& data) {
	return xx_decode_header(data);
}

Buffer JPEGImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

Array<PixelData> GIFImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(xx_decode(data));
	return rv;
}

PixelData GIFImageCodec::decode_header(cBuffer& data) {
	return xx_decode_header(data);
}

Buffer GIFImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

Array<PixelData> PNGImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(xx_decode(data));
	return rv;
}

PixelData PNGImageCodec::decode_header(cBuffer& data) {
	return xx_decode_header(data);
}

Buffer PNGImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

Array<PixelData> WEBPImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(xx_decode(data));
	return rv;
}

PixelData WEBPImageCodec::decode_header(cBuffer& data) {
	return xx_decode_header(data);
}

Buffer WEBPImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

XX_END
