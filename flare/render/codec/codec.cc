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

#include "ftr/image-codec.h"
#include "ftr/util/string.h"

FX_NS(ftr)

ImageCodec* tga_image_codec = nullptr;
ImageCodec* jpeg_image_codec = nullptr;
ImageCodec* gif_image_codec = nullptr;
ImageCodec* png_image_codec = nullptr;
ImageCodec* webp_image_codec = nullptr;
ImageCodec* pvrtc_image_codec = nullptr;

/**
 * @func get_image_format 通过路径获取图片类型
 */
ImageCodec::ImageFormat ImageCodec::get_image_format(cString& path) {
	
	String str = path.to_lower_case();
	int len = str.length();
	
	if (str.last_index_of(".pvr") != -1) {
		return PVRTC;
	}
	else if (str.last_index_of(".tga") != -1) {
		return TGA;
	}
	// JPF、JPX、J2C、JP2、J2K、JPC、LWF
	else if ( str.last_index_of(".jpg") != -1 ||
						str.last_index_of(".jpf") != -1 ||
						str.last_index_of(".jpeg") != -1
	) {
		return JPEG;
	}
	else if (str.last_index_of(".gif") != -1) {
		return GIF;
	}
	else if (str.last_index_of(".png") != -1) {
		return PNG;
	}
	else if (str.last_index_of(".webp") != -1) {
		return WEBP;
	}
	return Unknown;
}

ImageCodec* ImageCodec::shared(ImageFormat format) {
	switch (format) {
		case TGA:
			if (!tga_image_codec)
				tga_image_codec = new TGAImageCodec();
			return tga_image_codec;
		case JPEG:
			if (!jpeg_image_codec)
				jpeg_image_codec = new JPEGImageCodec();
			return jpeg_image_codec;
		case GIF:
			if (!gif_image_codec)
				gif_image_codec = new GIFImageCodec();
			return gif_image_codec;
		case PNG:
			if (!png_image_codec)
				png_image_codec = new PNGImageCodec();
			return png_image_codec;
		case WEBP:
			if (!webp_image_codec)
				webp_image_codec = new WEBPImageCodec();
			return webp_image_codec;
		case PVRTC:
			if (!pvrtc_image_codec)
				pvrtc_image_codec = new PVRTCImageCodec();
			return pvrtc_image_codec;
		default:
			return nullptr;
	}
}

FX_END
