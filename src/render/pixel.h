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
	struct        TexStat;
	class         ImageSource;
	class         PixelInfo;
	class         Pixel;
	typedef const Pixel cPixel;
	typedef const PixelInfo cPixelInfo;

	/**
	 * @enum ColorType color tye enum
	 */
	enum ColorType {
		kColor_Type_Invalid, //!< Invalid 
		kColor_Type_Alpha_8, //!< Alpha 8 bit
		kColor_Type_RGB_565,
		kColor_Type_RGBA_4444,
		kColor_Type_RGB_444X,
		kColor_Type_RGBA_8888,
		kColor_Type_RGB_888X,
		kColor_Type_BGRA_8888,
		kColor_Type_RGBA_1010102,
		kColor_Type_BGRA_1010102,
		kColor_Type_RGB_101010X,
		kColor_Type_BGR_101010X,
		kColor_Type_RGB_888,
		kColor_Type_RGBA_5551,
		kColor_Type_Gray_8,
		kColor_Type_Luminance_8 = kColor_Type_Gray_8,
		kColor_Type_Luminance_Alpha_88,
		kColor_Type_SDF_Float, //! signed distance function
		kColor_Type_YUV420P_Y_8,  // YUV420P
		kColor_Type_YUV420SP_Y_8 = kColor_Type_YUV420P_Y_8, // YUV420SP
		kColor_Type_YUV420P_U_8,
		kColor_Type_YUV420P_V_8 = kColor_Type_YUV420P_U_8,
		kColor_Type_YUV420SP_UV_88,
		// Compressed package for pvrtc
		kColor_Type_PVRTCI_2BPP_RGB,
		kColor_Type_PVRTCI_2BPP_RGBA,
		kColor_Type_PVRTCI_4BPP_RGB,
		kColor_Type_PVRTCI_4BPP_RGBA,
		kColor_Type_PVRTCII_2BPP,
		kColor_Type_PVRTCII_4BPP,
		kColor_Type_ETC1,
		kColor_Type_DXT1,
		kColor_Type_DXT2,
		kColor_Type_DXT3,
		kColor_Type_DXT4,
		kColor_Type_DXT5,
		kColor_Type_BC1 = kColor_Type_DXT1,
		kColor_Type_BC2 = kColor_Type_DXT3,
		kColor_Type_BC3 = kColor_Type_DXT5,
		kColor_Type_BC4,
		kColor_Type_BC5,
		kColor_Type_BC6,
		kColor_Type_BC7,
		kColor_Type_UYVY,
		kColor_Type_YUY2,
		kColor_Type_YUV420P,
		kColor_Type_YUV420SP,
		kColor_Type_YUV411P,
		kColor_Type_YUV411SP,
		kColor_Type_BW1BPP,
		kColor_Type_SharedExponentR9G9B9E5,
		kColor_Type_RGBG8888,
		kColor_Type_GRGB8888,
		kColor_Type_ETC2_RGB,
		kColor_Type_ETC2_RGBA,
		kColor_Type_ETC2_RGB_A1,
		kColor_Type_EAC_R11,
		kColor_Type_EAC_RG11,
	};

	enum AlphaType {
		kAlphaType_Unknown,
		kAlphaType_Opaque,   //!< pixel is opaque
		kAlphaType_Premul,   //!< pixel components are premultiplied by alpha
		kAlphaType_Unpremul, //!< pixel components are independent of alpha
	};

	class Qk_EXPORT PixelInfo {
	public:
		Qk_DEFINE_PROP_GET(int, width); //!< bitmap width
		Qk_DEFINE_PROP_GET(int, height); //!< bitmap height
		Qk_DEFINE_PROP_GET(ColorType, type); //!< bitmap pixel color type
		Qk_DEFINE_PROP_GET(AlphaType, alphaType); //!< is premultiplied by alpha
		PixelInfo();
		PixelInfo(int width, int height, ColorType type, AlphaType alphaType = kAlphaType_Unknown);
		uint32_t rowbytes() const;
		uint32_t bytes() const;
	};

	/**
	 * @class Pixel
	 */
	class Qk_EXPORT Pixel: public PixelInfo {
	public:
		Qk_DEFINE_PROP_GET(const TexStat*, texture); // gpu texture id

		/**
		 * @method pixel_bit_size()
		*/
		static uint32_t bytes_per_pixel(ColorType type);

		Pixel();
		Pixel(cPixel& data); // copy
		Pixel(Pixel&& data); // move
		Pixel(cPixelInfo& info, Buffer body); // move body
		Pixel(cPixelInfo& info);

		// operator=
		Pixel& operator=(cPixel& pixel); // copy
		Pixel& operator=(Pixel&& pixel); // move

		/**
		 * Returns image data body
		*/
		inline cBuffer& body() const { return _body; }
		inline uint8_t* val() { return reinterpret_cast<uint8_t*>(_body.val()); }

	private:
		Buffer _body; // pixel data
		friend class ImageSource;
	};

}
#endif
