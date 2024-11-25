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

#ifndef __quark__render__pixel__
#define __quark__render__pixel__

#include "../util/util.h"
#include "../util/string.h"
#include "../util/array.h"

namespace qk {
	class         PixelInfo;
	class         Pixel;
	typedef const Pixel cPixel;
	typedef const PixelInfo cPixelInfo;

	/**
	 * @enum ColorType color tye enum
	 */
	enum ColorType {
		kInvalid_ColorType, //!< Invalid 
		kAlpha_8_ColorType, //!< Alpha 8 bit
		kRGB_565_ColorType,
		kRGBA_4444_ColorType,
		kRGB_444X_ColorType,
		kRGBA_8888_ColorType,
		kRGB_888X_ColorType,
		kBGRA_8888_ColorType,
		kRGBA_1010102_ColorType,
		kBGRA_1010102_ColorType,
		kRGB_101010X_ColorType,
		kBGR_101010X_ColorType,
		kRGB_888_ColorType,
		kRGBA_5551_ColorType,
		kGray_8_ColorType,
		kLuminance_8_ColorType = kGray_8_ColorType,
		kLuminance_Alpha_88_ColorType,
		kSDF_Float_ColorType, //! signed distance function
		kYUV420P_Y_8_ColorType,  // YUV420P
		kYUV420SP_Y_8_ColorType = kYUV420P_Y_8_ColorType, // YUV420SP
		kYUV420P_U_8_ColorType,
		kYUV420P_V_8_ColorType = kYUV420P_U_8_ColorType,
		kYUV420SP_UV_88_ColorType,
		// Compressed package for pvrtc
		kPVRTCI_2BPP_RGB_ColorType,
		kPVRTCI_2BPP_RGBA_ColorType,
		kPVRTCI_4BPP_RGB_ColorType,
		kPVRTCI_4BPP_RGBA_ColorType,
		kPVRTCII_2BPP_ColorType,
		kPVRTCII_4BPP_ColorType,
		kETC1_ColorType,
		kDXT1_ColorType,
		kDXT2_ColorType,
		kDXT3_ColorType,
		kDXT4_ColorType,
		kDXT5_ColorType,
		kBC1_ColorType = kDXT1_ColorType,
		kBC2_ColorType = kDXT3_ColorType,
		kBC3_ColorType = kDXT5_ColorType,
		kBC4_ColorType,
		kBC5_ColorType,
		kBC6_ColorType,
		kBC7_ColorType,
		kUYVY_ColorType,
		kYUY2_ColorType,
		kYUV420P_ColorType, // yuv420p
		kYUV420SP_ColorType,
		kYUV411P_ColorType,
		kYUV411SP_ColorType,
		kBW1BPP_ColorType,
		kSharedExponentR9G9B9E5_ColorType,
		kRGBG8888_ColorType,
		kGRGB8888_ColorType,
		kETC2_RGB_ColorType,
		kETC2_RGBA_ColorType,
		kETC2_RGB_A1_ColorType,
		kEAC_R11_ColorType,
		kEAC_RG11_ColorType,
	};

	enum AlphaType {
		kUnknown_AlphaType,
		kOpaque_AlphaType,   //!< pixel is opaque
		kPremul_AlphaType,   //!< pixel components are premultiplied by alpha
		kUnpremul_AlphaType, //!< pixel components are independent of alpha
	};

	class Qk_Export PixelInfo {
	public:
		Qk_DEFINE_P_GET(int, width, Const); //!< bitmap width
		Qk_DEFINE_P_GET(int, height, Const); //!< bitmap height
		Qk_DEFINE_P_GET(ColorType, type, Const); //!< bitmap pixel color type
		Qk_DEFINE_P_GET(AlphaType, alphaType, Const); //!< is premultiplied by alpha
		PixelInfo();
		PixelInfo(int width, int height, ColorType type, AlphaType alphaType = kUnknown_AlphaType);
		uint32_t rowbytes() const;
		uint32_t bytes() const;
	};

	/**
	 * @class Pixel
	 */
	class Qk_Export Pixel: public PixelInfo {
	public:
		/**
		 * @method pixel_bit_size()
		*/
		static uint32_t bytes_per_pixel(ColorType type);

		class Body {
		public:
			virtual void release() = 0;
			virtual uint8_t* val() = 0;
			virtual uint32_t len() = 0;
		};

		Qk_DEFINE_P_GET(uint8_t*, val);
		Qk_DEFINE_P_GET(uint32_t, length, Const);
		Qk_DEFINE_A_GET(WeakBuffer, body, Const);

		Pixel();
		Pixel(cPixel& data); // copy
		Pixel(Pixel&& data); // move
		Pixel(cPixelInfo& info, Buffer body); // move body
		Pixel(cPixelInfo& info, Body *body); // move body
		Pixel(cPixelInfo& info);

		~Pixel();

		// operator=
		Pixel& operator=(cPixel& pixel); // copy
		Pixel& operator=(Pixel&& pixel); // move

	private:
		Body *_body;
	};

}
#endif
