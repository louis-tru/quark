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

#include "./pixel.h"

namespace qk {

	PixelInfo::PixelInfo(): _width(0), _height(0), _type(kColor_Type_Invalid), _alphaType(kAlphaType_Unknown) {
	}
	PixelInfo::PixelInfo(int width, int height, ColorType type, AlphaType alphaType)
		: _width(width), _height(height), _type(type), _alphaType(alphaType) {
	}

	uint32_t PixelInfo::rowbytes() const {
		return _width * Pixel::bytes_per_pixel(_type);
	}

	uint32_t PixelInfo::bytes() const {
		uint32_t rowbytes = _width * Pixel::bytes_per_pixel(_type);
		return rowbytes * _height;
	}

	// -------------------- P i x e l --------------------

	/**
	* @func pixel_bit_size
	*/
	uint32_t Pixel::bytes_per_pixel(ColorType type) {
		switch (type) {
			case kColor_Type_Invalid: return 0;
			case kColor_Type_Alpha_8: return 1;
			case kColor_Type_RGB_565: return 2;
			case kColor_Type_RGBA_4444: return 2;
			case kColor_Type_RGB_444X:  return 2;
			case kColor_Type_RGBA_8888: return 4;
			case kColor_Type_RGB_888X: return 4;
			case kColor_Type_BGRA_8888: return 4;
			case kColor_Type_RGBA_1010102: return 4;
			case kColor_Type_BGRA_1010102: return 4;
			case kColor_Type_RGB_101010X: return 4;
			case kColor_Type_BGR_101010X: return 4;
			case kColor_Type_RGB_888: return 3;
			case kColor_Type_RGBA_5551: return 2;
			case kColor_Type_Luminance_8: return 1;
			case kColor_Type_Luminance_Alpha_88: return 2;
			case kColor_Type_SDF_Float: return 4;
			case kColor_Type_YUV420P_Y_8: return 1; // kColor_Type_YUV420SP_Y_8
			case kColor_Type_YUV420P_U_8: return 1;
			case kColor_Type_YUV420SP_UV_88: return 2;
			default: return 0; // Invalid is compressed format
		}
	}

	Pixel::Pixel(): _texture(0), _body() {
	}

	Pixel::Pixel(cPixel& pixel): PixelInfo(pixel)
		, _texture(pixel._texture)
		, _body(pixel._body.copy()) {
	}

	Pixel::Pixel(Pixel&& pixel): PixelInfo(pixel)
		, _texture(pixel._texture)
		, _body(pixel._body) {
	}

	Pixel::Pixel(const PixelInfo& info, Buffer body): PixelInfo(info)
		, _texture(0)
		, _body(body) {
	}

	Pixel::Pixel(cPixelInfo& info): PixelInfo(info), _texture(0) {
	}

	Pixel& Pixel::operator=(cPixel& pixel) {
		PixelInfo::operator=(pixel);
		_texture = pixel._texture;
		_body = pixel._body.copy();
		return *this;
	}

	Pixel& Pixel::operator=(Pixel&& pixel) {
		PixelInfo::operator=(pixel);
		_texture = pixel._texture;
		_body = pixel._body;
		return *this;
	}

}
