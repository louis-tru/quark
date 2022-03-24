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

#include "../app.h"
#include "./source.h"
#include "../pre_render.h"
#include "../util/fs.h"
#include "skia/core/SkImage.h"

F_NAMESPACE_START

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
	, _uri(fs_reader()->format(uri))
	, _state(STATE_NONE)
	, _load_id(0), _size(0), _used(0)
	, _inl(nullptr)
{
}

ImageSource::ImageSource(Pixel pixel)
	: F_Init_Event(State)
	, _state(STATE_NONE)
	, _width(pixel.width())
	, _height(pixel.height())
	, _type(pixel.type())
	, _memPixel(std::move(pixel))
	, _load_id(0), _size(0), _used(0)
	, _inl(nullptr)
{
	SkImageInfo info = SkImageInfo::Make(_memPixel.width(),
																				_memPixel.height(), SkColorType(_memPixel.type()), kOpaque_SkAlphaType);
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
	F_ASSERT(_state & STATE_LOAD_COMPLETE);
	F_ASSERT(_inl);
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
	_used++;
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

// -------------------- S o u r c e H o l d --------------------

SourceHold::~SourceHold() {
	if (_source) {
		_source->F_Off(State, &SourceHold::handleSourceState, this);
	}
}

String SourceHold::src() const {
	return _source ? _source->uri(): String();
}

ImageSource* SourceHold::source() {
	return _source.value();
}

void SourceHold::set_src(cString& value) {
	set_source(app() ? app()->img_pool()->get(value): new ImageSource(value));
}

void SourceHold::set_source(ImageSource* source) {
	if (_source.value() != source) {
		if (_source) {
			_source->F_Off(State, &SourceHold::handleSourceState, this);
		}
		if (source) {
			source->F_On(State, &SourceHold::handleSourceState, this);
		}
		_source = Handle<ImageSource>(source);
	}
}

void SourceHold::handleSourceState(Event<ImageSource, ImageSource::State>& evt) { // 收到图像变化通知
	onSourceState(evt);
}

void SourceHold::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
	if (*evt.data() & ImageSource::STATE_DECODE_COMPLETE) {
		auto app_ = app();
		// F_ASSERT(app_, "Application needs to be initialized first");
		if (app_) {
			app_->pre_render()->mark_none();
		}
	}
}

// -------------------- I m a g e P o o l --------------------

F_DEFINE_INLINE_MEMBERS(ImagePool, Inl) {
	public:
	#define _inl_pool(self) static_cast<ImagePool::Inl*>(self)

	void source_state_handle(Event<ImageSource, ImageSource::State>& evt) {
		ScopeLock locl(_Mutex);
		auto id = evt.sender()->uri().hash_code();
		auto it = _sources.find(id);
		if (it != _sources.end()) {
			int ch = int(evt.sender()->size()) - int(it->value.size);
			if (ch != 0) {
				_total_data_size += ch; // change
				it->value.size = evt.sender()->size();
			}
		}
	}
	
};

ImagePool::ImagePool(Application* host): _host(host) {
}

ImagePool::~ImagePool() {
	for (auto& it: _sources) {
		it.value.source->F_Off(State, &Inl::source_state_handle, _inl_pool(this));
	}
}

ImageSource* ImagePool::get(cString& uri) {
	ScopeLock local(_Mutex);
	String _uri = fs_reader()->format(uri);
	uint64_t id = _uri.hash_code();

	// 通过路径查找
	auto it = _sources.find(id);
	if ( it != _sources.end() ) {
		return it->value.source.value();
	}

	ImageSource* source = new ImageSource(_uri);
	source->F_On(State, &Inl::source_state_handle, _inl_pool(this));
	_sources.set(id, { source->size(), source });
	_total_data_size += source->size();

	return source;
}

void ImagePool::remove(cString& uri) {
	ScopeLock local(_Mutex);
	String _uri = fs_reader()->format(uri);
	auto it = _sources.find(_uri.hash_code());
	if (it != _sources.end()) {
		it->value.source->F_Off(State, &Inl::source_state_handle, _inl_pool(this));
		_sources.erase(it);
		_total_data_size -= it->value.size;
	}
}

void ImagePool::clear(bool full) {
	ScopeLock local(_Mutex);
	// TODO ..
}

F_NAMESPACE_END
