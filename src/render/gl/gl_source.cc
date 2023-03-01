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

#include "../source.h"
#include "../../util/fs.h"
#include "skia/core/SkImage.h"


namespace qk {

	#define sk_I(img) static_cast<SkImage*>(img)

	Qk_DEFINE_INLINE_MEMBERS(ImageSource, Inl) {
	public:
		static SkImage* CastSk(ImageSource* img) {
			return sk_I(img->_inl);
		}
	};

	SkImage* CastSkImage(ImageSource* img) {
		return ImageSource::Inl::CastSk(img);
	}

	ImageSource::ImageSource(Pixel pixel)
		: Qk_Init_Event(State)
		, _state(STATE_NONE)
		, _width(pixel.width())
		, _height(pixel.height())
		, _type(pixel.type())
		, _memPixel(std::move(pixel))
		, _load_id(0), _size(0)
		, _inl(nullptr)
	{
		SkImageInfo info = SkImageInfo::Make(_memPixel.width(), _memPixel.height(), 
			SkColorType(_memPixel.type()), SkAlphaType(_memPixel.alphaType()));
		SkPixmap skpixel(info, _memPixel.body().val(), _memPixel.width() * Pixel::bytes_per_pixel(_memPixel.type()));
		auto img = SkImage::MakeFromRaster(skpixel, nullptr, nullptr);
		img->ref();
		_inl = img.get();
		_uri = String("mem://").append(sk_I(_inl)->uniqueID());
		_state = State(STATE_LOAD_COMPLETE | STATE_DECODEING);
		_size = _memPixel.body().length() + Pixel::bytes_per_pixel(_type) * _width * _height;
	}

}