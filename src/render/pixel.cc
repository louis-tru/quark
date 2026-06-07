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

#include "./pixel.h"

namespace qk {

	PixelInfo::PixelInfo(): _width(0), _height(0), _type(kInvalid_ColorType), _alphaType(kUnknown_AlphaType) {
	}
	PixelInfo::PixelInfo(int width, int height, ColorType type, AlphaType alphaType)
		: _width(width), _height(height), _type(type), _alphaType(alphaType) {
	}

	uint32_t PixelInfo::rowbytes() const {
		auto block = Pixel::block_info(_type);
		return block.width ? ((_width + block.width - 1) / block.width) * block.bytes: 0;
	}

	uint32_t PixelInfo::bytes() const {
		auto block = Pixel::block_info(_type);
		return block.height ? rowbytes() * ((_height + block.height - 1) / block.height): 0;
	}

	// -------------------- P i x e l --------------------

	/**
	* @func pixel_bit_size
	*/
	uint32_t PixelInfo::bytes_per_pixel(ColorType type) {
		switch (type) {
			case kInvalid_ColorType: return 0;
			case kAlpha_8_ColorType: return 1;
			case kRGB_565_ColorType: return 2;
			case kRGBA_4444_ColorType: return 2;
			case kRGB_444X_ColorType:  return 2;
			case kRGBA_8888_ColorType: return 4;
			case kRGB_888X_ColorType: return 4;
			case kBGRA_8888_ColorType: return 4;
			case kBGR_888X_ColorType: return 4;
			case kRGBA_1010102_ColorType: return 4;
			case kRGB_101010X_ColorType: return 4;
			case kRGB_888_ColorType: return 3;
			case kRGBA_5551_ColorType: return 2;
			case kLuminance_8_ColorType: return 1;
			case kLuminance_Alpha_88_ColorType: return 2;
			case kSDF_F32_ColorType: return 4;
			case kSDF_Unsigned_F32_ColorType: return 4;
			case kYUV420P_Y_8_ColorType: return 1; // kColor_Type_YUV420SP_Y_8
			case kYUV420P_U_8_ColorType: return 1;
			case kYUV420SP_UV_88_ColorType: return 2;
			case kUYVY_ColorType:
			case kYUY2_ColorType:
			case kRGBG_8888_ColorType:
			case kGRGB_8888_ColorType:
				return 2;
			case kSharedExponentR9G9B9E5_ColorType:
				return 4;
			default: return 0; // Block-compressed or planar format.
		}
	}

	PixelBlockInfo PixelInfo::block_info(ColorType type) {
		switch (type) {
			case kUYVY_ColorType:
			case kYUY2_ColorType:
			case kRGBG_8888_ColorType:
			case kGRGB_8888_ColorType:
				return {2, 1, 4};
			default:
				break;
		}

		auto bytes = bytes_per_pixel(type);
		if (bytes)
			return {1, 1, uint8_t(bytes)};

		switch (type) {
			case kPVRTCI_2BPP_RGB_ColorType:
			case kPVRTCI_2BPP_RGBA_ColorType:
				return {16, 8, 32};
			case kPVRTCI_4BPP_RGB_ColorType:
			case kPVRTCI_4BPP_RGBA_ColorType:
				return {8, 8, 32};
			case kPVRTCII_2BPP_ColorType:
				return {8, 4, 8};
			case kPVRTCII_4BPP_ColorType:
				return {4, 4, 8};
			case kETC1_ColorType:
			case kETC2_RGB_ColorType:
			case kETC2_RGB_A1_ColorType:
			case kDXT1_ColorType:
			case kBC4_ColorType:
			case kEAC_R11_ColorType:
				return {4, 4, 8};
			case kETC2_RGBA_ColorType:
			case kDXT2_ColorType:
			case kDXT3_ColorType:
			case kDXT4_ColorType:
			case kDXT5_ColorType:
			case kBC5_ColorType:
			case kBC6_ColorType:
			case kBC7_ColorType:
			case kEAC_RG11_ColorType:
				return {4, 4, 16};
			case kBW1BPP_ColorType:
				return {8, 1, 1};
			default:
				return {0, 0, 0};
		}
	}

	Pixel::Pixel(): _val(nullptr), _length(0), _body(nullptr)
	{}

	Pixel::Pixel(cPixel& pixel): PixelInfo(pixel)
		, _val(reinterpret_cast<uint8_t*>(pixel.buffer().copy().collapse()))
		, _length(pixel._length)
		, _body(nullptr) {
	}

	Pixel::Pixel(Pixel&& pixel): PixelInfo(pixel)
		, _val(pixel._val), _length(pixel._length), _body(pixel._body) {
		pixel._val = nullptr;
		pixel._length = 0;
		pixel._body = nullptr;
	}

	Pixel::Pixel(cPixelInfo& info, Buffer body): PixelInfo(info)
		, _length(body.length())
		, _val(nullptr)
		, _body(nullptr) {
		_val = reinterpret_cast<uint8_t*>(body.collapse());
	}

	Pixel::Pixel(cPixelInfo& info, Body *body): PixelInfo(info)
		, _length(body->len())
		, _val(body->val())
		, _body(body) {
	}

	Pixel::Pixel(cPixelInfo& info): PixelInfo(info)
		, _val(nullptr), _length(0), _body(nullptr)
	{}

	Pixel::~Pixel() {
		if (_body) {
			_body->release(); _body = nullptr;
		} else {
			::free(_val);
		}
		_val = nullptr;
		_length = 0;
	}

	Pixel& Pixel::operator=(cPixel& pixel) {
		PixelInfo::operator=(pixel);
		this->~Pixel();
		_val    = reinterpret_cast<uint8_t*>(pixel.buffer().copy().collapse());
		_length = pixel.length();
		return *this;
	}

	Pixel& Pixel::operator=(Pixel&& pixel) {
		PixelInfo::operator=(pixel);
		this->~Pixel();
		_val    = pixel._val;    pixel._val = nullptr;
		_length = pixel._length; pixel._length = 0;
		_body   = pixel._body;   pixel._body = nullptr;
		return *this;
	}

	WeakBuffer Pixel::buffer() const {
		return WeakBuffer(reinterpret_cast<cChar*>(_val), _length);
	}

}
