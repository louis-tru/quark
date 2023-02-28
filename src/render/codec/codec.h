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

#ifndef __quark__render__codec__codec__
#define __quark__render__codec__codec__

#include "quark/util/handle.h"
#include "quark/util/array.h"
#include "../pixel.h"

namespace quark {

	enum ImageFormat {
		kUnknown_ImageFormat,
		kJPEG_ImageFormat,
		kGIF_ImageFormat,
		kPNG_ImageFormat,
		kWEBP_ImageFormat,
		kTGA_ImageFormat,
		kPVRTC_ImageFormat,
	};

	/**
	 * @func test
	 * 只解码头信息,返回除主体数据以外的描述数据 width、height、format、
	 * 如果当前只需要知道图像的附加信息可调用该函数,
	 * 因为解码像 jpg、png 这种复杂压缩图像格式是很耗时间的.
	 */
	Qk_EXPORT bool img_test(cBuffer& data, PixelInfo* out, ImageFormat fmt = kUnknown_ImageFormat);

	/**
	 * 解码图像为GPU可读取的格式如:RGBA8888/RGBA4444/ETC1/ETC2_RGB/ETC2_RGBA...,并返回mipmap列表
	 * @func decode
	 * @arg data {cBuffer&}
	 * @ret {Array<Pixel>}
	 */
	Qk_EXPORT bool img_decode(cBuffer& data, Array<Pixel> *out, ImageFormat fmt = kUnknown_ImageFormat);

	/**
	 * @func image_format 通过路径获取图片类型
	 */
	Qk_EXPORT ImageFormat img_format_from(cString& path);

	// encode to tga data
	Qk_EXPORT Buffer img_tga_encode(cPixel& pixel);

}
#endif
