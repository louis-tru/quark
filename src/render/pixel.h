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

#ifndef __quark__render__pixel__
#define __quark__render__pixel__

#include "../util/util.h"
#include "../util/string.h"
#include "../util/array.h"

namespace quark {

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
		kColor_Type_ARGB_4444,
		kColor_Type_RGBA_8888,
		kColor_Type_RGB_888X,
		kColor_Type_BGRA_8888,
		kColor_Type_RGBA_1010102,
		kColor_Type_BGRA_1010102,
		kColor_Type_RGB_101010X,
		kColor_Type_BGR_101010X,
		kColor_Type_Gray_8,
		kColor_Type_RGB_888,
		kColor_Type_RGBA_5551,
		kColor_Type_Luminance_Alpha_88,
	};

	enum AlphaType {
		kAlphaType_Unknown,
		kAlphaType_Opaque,   //!< pixel is opaque
		kAlphaType_Premul,   //!< pixel components are premultiplied by alpha
		kAlphaType_Unpremul, //!< pixel components are independent of alpha
	};

	class Qk_EXPORT PixelInfo: public Object {
	public:
		PixelInfo();
		PixelInfo(int width, int height, ColorType type, AlphaType alphaType = kAlphaType_Unknown);
		Qk_Define_Prop_Get(int, width); //!< width 图像宽度
		Qk_Define_Prop_Get(int, height); //!< height 图像高度
		Qk_Define_Prop_Get(ColorType, type); //!< format 图像像素的排列格式
		Qk_Define_Prop_Get(AlphaType, alphaType); //!< 图像数据是否对通道信息进行了预先处理,存在alpha通道才有效.
	};

	/**
	* @class Pixel
	*/
	class Qk_EXPORT Pixel: public PixelInfo {
	public:

		/**
		* @func pixel_bit_size()
		*/
		static uint32_t bytes_per_pixel(ColorType type);

		/**
		 *
		 * decode jpg/png/gif... image format data
		 *
		 * @func decode()
		 */
		static Pixel decode(cBuffer& raw);

		Pixel();
		Pixel(cPixel& data);
		Pixel(Pixel&& data);
		Pixel(cPixelInfo& info, Buffer body);
		Pixel(cPixelInfo& info, cWeakBuffer& body);

		/**
		* Returns image data body
		*/
		inline  WeakBuffer& body() { return _body; }
		inline cWeakBuffer& body() const { return _body; }

		/**
		* Returns pointer for image data body
		*/
		inline const void* ptr() const { return _body.val(); }
		
	private:
		Buffer     _data; // hold data
		WeakBuffer _body;
	};

}
#endif
