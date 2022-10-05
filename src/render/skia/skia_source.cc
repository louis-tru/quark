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


namespace quark {

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
		, _load_id(0), _size(0), _used(0)
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

	ImageSource::~ImageSource() {
		if (_inl) {
			sk_I(_inl)->unref();
			_inl = nullptr;
			_state = STATE_NONE;
			_size = 0;
		}
		unload();
	}

	void ImageSource::_Decode() {
		Qk_Assert(_state & STATE_LOAD_COMPLETE);
		Qk_Assert(_inl);
		// decode image
		
		struct Ctx {
			Handle<ImageSource> src;
			sk_sp<SkImage> img;
		};
		auto ctx = new Ctx({ this, sk_sp<SkImage>(sk_I(_inl)) });
		ctx->img->ref();
		
		RunLoop::first()->work(Cb([this,ctx](CbData& e){
			char tmp[16];
			SkImageInfo info = SkImageInfo::Make(1, 1, ctx->img->colorType(), ctx->img->alphaType());
			if (!ctx->img->readPixels(nullptr, info, tmp, SkColorTypeBytesPerPixel(ctx->img->colorType()), 0, 0)) {
				ctx->img = nullptr; // fail decode
			}
		}), Cb([this,ctx](CbData& e){
			if (_state & STATE_DECODEING) {
				if (ctx->img) { // decode image complete
					_size += Pixel::bytes_per_pixel(_type) * _width * _height;
					_state = State((_state | STATE_DECODE_COMPLETE) & ~STATE_DECODEING);
				} else { // decode fail
					_state = State((_state | STATE_DECODE_ERROR)    & ~STATE_DECODEING);
				}
				Qk_Trigger(State, _state);
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
			Qk_Trigger(State, _state); // trigger
			_load_id = fs_reader()->read_file(_uri, Cb([this](CbData& e){ // read data
				if (_state & STATE_LOADING) {
					_state = State((_state | STATE_LOAD_COMPLETE) & ~STATE_LOADING);
					_loaded = *static_cast<Buffer*>(e.data);
					_size = _loaded.length();

					auto img = SkImage::MakeFromEncoded(SkData::MakeWithProc(_loaded.val(), _loaded.length(), nullptr, nullptr));
					auto info = img->imageInfo();
					img->ref();

					_inl = img.get();
					_width = info.width();
					_height = info.height();
					_type = ColorType(info.colorType());
					
					Qk_Trigger(State, _state);

					if (_state & STATE_DECODEING) { // decode
						_Decode();
					}
				}
			}, this));
		}, this));
		return false;
	}

	/**
		* @func unload() delete load and ready
		*/
	void ImageSource::unload() {
		_used = 0;
		if (_inl) {
			if (_memPixel.body().is_null()) { // no mem pixel
				sk_I(_inl)->unref();
				_inl = nullptr;
				_state = State( _state & ~(STATE_LOADING | STATE_LOAD_COMPLETE | STATE_DECODEING | STATE_DECODE_COMPLETE) );
				_loaded.clear(); // clear raw data
				_size = 0;
				if (_load_id) { // cancel load and ready
					fs_reader()->abort(_load_id);
					_load_id = 0;
				}
				// trigger event
				RunLoop::first()->post(Cb([this](CbData& e){
					Qk_Trigger(State, _state);
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

}