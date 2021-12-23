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

#include "./image_source.h"
#include "./util/fs.h"
#include "skia/core/SkImage.h"

namespace flare {

	// -------------------- P i x e l D a t a --------------------

	/**
	* @func pixel_bit_size
	*/
	uint32_t PixelData::bytes_per_pixel(ColorType type) {
		return SkColorTypeBytesPerPixel(SkColorType(type));
	}

	PixelData PixelData::decode(cBuffer& buf) {
		auto img = SkImage::MakeFromEncoded(SkData::MakeWithProc(buf.val(), buf.length(), nullptr, nullptr));
		SkImageInfo info = img->imageInfo();
		auto rowBytes = info.minRowBytes();
		auto body = Buffer::alloc((uint32_t)rowBytes * info.height());
		if (img->readPixels(nullptr, info, body.val(), rowBytes, 0, 0)) {
			return PixelData(std::move(body), info.width(), info.height(), ColorType(info.colorType()));
		}
		return PixelData();
	}

	PixelData::PixelData()
		: _data()
		, _width(0)
		, _height(0)
		, _body()
		, _type(COLOR_TYPE_INVALID) {
	}

	PixelData::PixelData(cPixelData& body)
		: _data()
		, _width(body._width)
		, _height(body._height)
		, _body(body._body)
		, _type(body._type) {
	}

	PixelData::PixelData(PixelData&& body)
		: _data(body._data)
		, _width(body._width)
		, _height(body._height)
		, _body(std::move(body._body))
		, _type(body._type) {
	}

	PixelData::PixelData(ColorType type)
		: _data()
		, _width(0)
		, _height(0)
		, _body()
		, _type(type) {
	}

	PixelData::PixelData(Buffer body, int width, int height, ColorType type)
		: _data(body)
		, _width(width)
		, _height(height)
		, _body()
		, _type(type) {
		_body.push(WeakBuffer(*_data, _data.length()));
	}

	PixelData::PixelData(WeakBuffer body, int width, int height, ColorType type)
		: _data()
		, _width(width)
		, _height(height)
		, _body()
		, _type(type) {
		_body.push(body);
	}

	PixelData::PixelData(const Array<WeakBuffer>& body, int width, int height, ColorType type)
		: _data()
		, _width(width)
		, _height(height)
		, _body(body)
		, _type(type) {
	}

	// -------------------- I m a g e S o u r c e --------------------

	#define sk_I(img) static_cast<SkImage*>(img)

	F_DEFINE_INLINE_MEMBERS(ImageSource, Inl) {
	 public:
		static SkImage* CastSk(ImageSource* img) {
			return sk_I(img->_inl);
		}
	};

	SkImage* CastSkImage(ImageSource* img) {
		return ImageSource::Inl::CastSk(img);
	}

	ImageSource::ImageSource(cString& uri)
		: F_Init_Event(State)
		, _id(fs_reader()->format(uri))
		, _state(STATE_NONE)
		, _load_id(0)
		, _inl(nullptr)
	{
	}

	ImageSource::ImageSource(PixelData pixel)
		: F_Init_Event(State)
		, _state(STATE_NONE)
		, _width(pixel.width())
		, _height(pixel.height())
		, _type(pixel.type())
		, _memPixel(std::move(pixel))
		, _load_id(0)
		, _inl(nullptr)
	{
		SkImageInfo info = SkImageInfo::Make(_memPixel.width(),
																				 _memPixel.height(), SkColorType(_memPixel.type()), kOpaque_SkAlphaType);
		SkPixmap skpixel(info, _memPixel.body().val(), _memPixel.width() * PixelData::bytes_per_pixel(_memPixel.type()));
		auto img = SkImage::MakeFromRaster(skpixel, nullptr, nullptr);
		img->ref();
		_inl = img.get();
		_id = String("mem://").append(sk_I(_inl)->uniqueID());
		_state = State(STATE_LOAD_COMPLETE | STATE_DECODEING);
	}

	ImageSource::~ImageSource() {
		if (_inl) {
			sk_I(_inl)->unref();
			_inl = nullptr;
			_state = STATE_NONE;
		}
		unload();
	}

	void ImageSource::_Decode() {
		F_ASSERT(_state & STATE_LOAD_COMPLETE);
		F_ASSERT(!_inl);
		// decode image
		
		struct Ctx {
			Handle<ImageSource> src;
			sk_sp<SkImage> img;
		};
		auto ctx = new Ctx({ this, sk_sp<SkImage>(sk_I(_inl)) });
		
		RunLoop::first()->work(Cb([this,ctx](CbData& e){
			char tmp[16];
			SkImageInfo info = SkImageInfo::Make(1, 1, ctx->img->colorType(), ctx->img->alphaType());
			if (!ctx->img->readPixels(nullptr, info, tmp, SkColorTypeBytesPerPixel(ctx->img->colorType()), 0, 0)) {
				ctx->img.reset(); // fail decode
			}
		}), Cb([this,ctx](CbData& e){
			if (_state & STATE_DECODEING) {
				if (ctx->img) { // decode image complete
					_state = State((_state | STATE_DECODE_COMPLETE) & ~STATE_DECODEING);
				} else { // decode fail
					_state = State((_state | STATE_DECODE_ERROR)    & ~STATE_DECODEING);
				}
				F_Trigger(State, _state);
			}
			delete ctx;
		}));
	}

	/**
		* @func load() async load image source
		*/
	bool ImageSource::load() {
		if (_state & STATE_LOAD_COMPLETE) {
			return true;
		}
		if (_state & STATE_LOADING) {
			return false;
		}
		_state = State(_state | STATE_LOADING);
		
		RunLoop::first()->post(Cb([this](CbData& e){
			F_Trigger(State, _state); // trigger
			_load_id = fs_reader()->read_file(_id, Cb([this](CbData& e){ // read data
				if (!(_state & STATE_LOADING)) {
					_state = State((_state | STATE_LOAD_COMPLETE) & ~STATE_LOADING);
					_loaded = *static_cast<Buffer*>(e.data);

					auto img = SkImage::MakeFromEncoded(SkData::MakeWithProc(_loaded.val(), _loaded.length(), nullptr, nullptr));
					auto info = img->imageInfo();
					img->ref();

					_inl = img.get();
					_width = info.width();
					_height = info.height();
					_type = ColorType(info.colorType());
					
					F_Trigger(State, _state);

					if (_state & STATE_DECODEING) { // decode
						_Decode();
					}
				}
			}, this));
		}, this));
		return false;
	}

	/**
		* @func ready() async ready
		*/
	bool ImageSource::ready() {
		if (is_ready()) {
			return true;
		}
		if (_state & STATE_DECODEING) {
			return false;
		}
		_state = State(_state | STATE_DECODEING);
		
		if (_state & STATE_LOAD_COMPLETE) {
			RunLoop::first()->post(Cb([this](CbData& e){
				F_Trigger(State, _state);
				_Decode();
			}, this));
		} else { // load and decode
			load();
		}
		return false;
	}

	/**
		* @func unload() delete load and ready
		*/
	void ImageSource::unload() {
		if (_inl) {
			if (_memPixel.body().is_null()) { // no mem pixel
				sk_I(_inl)->unref();
				_inl = nullptr;
				_state = State( _state & ~(STATE_LOADING | STATE_LOAD_COMPLETE | STATE_DECODEING | STATE_DECODE_COMPLETE) );
				_loaded.clear(); // clear raw data
				if (_load_id) { // cancel load and ready
					fs_reader()->abort(_load_id);
					_load_id = 0;
				}
				// trigger event
				RunLoop::first()->post(Cb([this](CbData& e){
					F_Trigger(State, _state);
				}, this));
			}
		}
	}

	/**
		* 
		* mark as gpu texture
		*
		* @func mark_as_texture()
		*/
	bool ImageSource::mark_as_texture() {
		// TODO ...
		return false;
	}

	// -------------------- I m a g e P o o l --------------------

	void ImagePool::clear(bool full) {
		// TODO ..
	}

}

