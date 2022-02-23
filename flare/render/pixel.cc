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

#include "./pixel.h"

namespace flare {

	// -------------------- P i x e l --------------------

	/**
	* @func pixel_bit_size
	*/
	uint32_t Pixel::bytes_per_pixel(ColorType type) {
		return SkColorTypeBytesPerPixel(SkColorType(type));
	}

	Pixel Pixel::decode(cBuffer& buf) {
		auto img = SkImage::MakeFromEncoded(SkData::MakeWithProc(buf.val(), buf.length(), nullptr, nullptr));
		SkImageInfo info = img->imageInfo();
		auto rowBytes = info.minRowBytes();
		auto body = Buffer::alloc((uint32_t)rowBytes * info.height());
		if (img->readPixels(nullptr, info, body.val(), rowBytes, 0, 0)) {
			return Pixel(std::move(body), info.width(), info.height(), ColorType(info.colorType()));
		}
		return Pixel();
	}

	Pixel::Pixel()
		: _data()
		, _width(0)
		, _height(0)
		, _body()
		, _type(COLOR_TYPE_INVALID) {
	}

	Pixel::Pixel(cPixel& body)
		: _data()
		, _width(body._width)
		, _height(body._height)
		, _body(body._body)
		, _type(body._type) {
	}

	Pixel::Pixel(Pixel&& body)
		: _data(body._data)
		, _width(body._width)
		, _height(body._height)
		, _body(std::move(body._body))
		, _type(body._type) {
	}

	Pixel::Pixel(ColorType type)
		: _data()
		, _width(0)
		, _height(0)
		, _body()
		, _type(type) {
	}

	Pixel::Pixel(Buffer body, int width, int height, ColorType type)
		: _data(body)
		, _width(width)
		, _height(height)
		, _body()
		, _type(type) {
		_body.push(WeakBuffer(*_data, _data.length()));
	}

	Pixel::Pixel(WeakBuffer body, int width, int height, ColorType type)
		: _data()
		, _width(width)
		, _height(height)
		, _body()
		, _type(type) {
		_body.push(body);
	}

	Pixel::Pixel(const Array<WeakBuffer>& body, int width, int height, ColorType type)
		: _data()
		, _width(width)
		, _height(height)
		, _body(body)
		, _type(type) {
	}

}

