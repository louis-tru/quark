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
#include "../../util/string.h"

namespace qk {

#if Qk_APPLE
	bool apple_img_test(cBuffer& data, PixelInfo* out);
	bool apple_img_decode(cBuffer& data, Array<Pixel>* out);
#else
	bool img_jpeg_test(cBuffer& data, PixelInfo* out);
	bool img_gif_test(cBuffer& data, PixelInfo* out);
	bool img_png_test(cBuffer& data, PixelInfo* out);
	bool img_webp_test(cBuffer& data, PixelInfo* out);
	bool img_tga_test(cBuffer& data, PixelInfo* out);
	// decode
	bool img_jpeg_decode(cBuffer& data, Array<Pixel> *out);
	bool img_gif_decode(cBuffer& data, Array<Pixel> *out);
	bool img_png_decode(cBuffer& data, Array<Pixel> *out);
	bool img_webp_decode(cBuffer& data, Array<Pixel> *out);
	bool img_tga_decode(cBuffer& data, Array<Pixel> *out);
#endif

	bool img_pvrt_test(cBuffer& data, PixelInfo* out);
	bool img_pvrt_decode(cBuffer& data, Array<Pixel> *out);

	bool img_test(cBuffer& data, PixelInfo* out, ImageFormat fmt) {
#if Qk_APPLE
		return apple_img_test(data, out) || img_pvrt_test(data, out);
#else
		bool ok = false;

		switch (fmt) {
			case kJPEG_ImageFormat: ok = img_jpeg_test(data, out); break;
			case kGIF_ImageFormat: ok = img_gif_test(data, out); break;
			case kPNG_ImageFormat: ok = img_png_test(data, out); break;
			case kWEBP_ImageFormat: ok = img_webp_test(data, out); break;
			case kTGA_ImageFormat: ok = img_tga_test(data, out); break;
			case kPVRTC_ImageFormat: ok = img_pvrt_test(data, out); break;
			default: break;
		}

		return ok
			|| img_jpeg_test(data, out)
			|| img_gif_test(data, out)
			|| img_png_test(data, out)
			|| img_webp_test(data, out)
			|| img_tga_test(data, out)
			|| img_pvrt_test(data, out);
#endif
	}

	bool img_decode(cBuffer& data, Array<Pixel> *out, ImageFormat fmt) {
		// Array<Pixel> out;
#if Qk_APPLE
	return apple_img_decode(data, out) || img_pvrt_decode(data, out);
#else
		bool ok = false;

		switch (fmt) {
			case kJPEG_ImageFormat: ok = img_jpeg_decode(data, out); break;
			case kGIF_ImageFormat: ok = img_gif_decode(data, out); break;
			case kPNG_ImageFormat: ok = img_png_decode(data, out); break;
			case kWEBP_ImageFormat: ok = img_webp_decode(data, out); break;
			case kTGA_ImageFormat: ok = img_tga_decode(data, out); break;
			case kPVRTC_ImageFormat: ok = img_pvrt_decode(data, out); break;
			default: break;
		}

		return ok
			|| img_jpeg_decode(data, out)
			|| img_gif_decode(data, out)
			|| img_png_decode(data, out)
			|| img_webp_decode(data, out)
			|| img_tga_decode(data, out)
			|| img_pvrt_decode(data, out);
#endif
	}

	ImageFormat img_format_from(cString& path) {

		String str = path.to_lower_case();
		int len = str.length();
		
		if (str.last_index_of(".pvr") != -1) {
			return kPVRTC_ImageFormat;
		}
		else if (str.last_index_of(".tga") != -1) {
			return kTGA_ImageFormat;
		}
		// JPF、JPX、J2C、JP2、J2K、JPC、LWF
		else if ( str.last_index_of(".jpg") != -1 ||
							str.last_index_of(".jpf") != -1 ||
							str.last_index_of(".jpeg") != -1
		) {
			return kJPEG_ImageFormat;
		}
		else if (str.last_index_of(".gif") != -1) {
			return kGIF_ImageFormat;
		}
		else if (str.last_index_of(".png") != -1) {
			return kPNG_ImageFormat;
		}
		else if (str.last_index_of(".webp") != -1) {
			return kWEBP_ImageFormat;
		}
		return kUnknown_ImageFormat;
	}

}
