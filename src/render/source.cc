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

#include "../app.h"
#include "./source.h"
#include "../pre_render.h"
#include "../util/fs.h"
#include "./codec/codec.h"



namespace qk {

	// -------------------- I m a g e . S o u r c e --------------------

	ImageSource::ImageSource(cString& uri): Qk_Init_Event(State)
		, _uri(fs_reader()->format(uri))
		, _state(kSTATE_NONE)
		, _load_id(0)
	{}

	ImageSource::ImageSource(Array<Pixel>&& pixels)
		: Qk_Init_Event(State)
		, _state(kSTATE_NONE)
		, _pixels(std::move(pixels))
		, _load_id(0)
	{
		if (_pixels.length()) {
			_info = _pixels[0];
			_uri = String("mem://").append(random());
			_state = State(kSTATE_LOAD_COMPLETE | kSTATE_DECODE_COMPLETE);

			uint32_t rowbytes = _info.width() * Pixel::bytes_per_pixel(_info.type());
			uint32_t size = rowbytes * _info.height();
			Qk_ASSERT(size == _pixels[0].body().length(), "#ImageSource::ImageSource pixel size no match");
		}
	}

	ImageSource::~ImageSource() {
		_state = kSTATE_NONE;
		unload();
	}

	/**
		* @func load() async load image source
		*/
	bool ImageSource::load() {
		if (_state & (kSTATE_LOAD_COMPLETE | kSTATE_DECODE_COMPLETE))
			return true;
		if (_state & kSTATE_LOADING)
			return false;

		RunLoop::first()->post(Cb([this](auto e) {
			_Load();
		}, this));

		return false;
	}

	/**
		* @func ready() async ready decode
		*/
	bool ImageSource::ready() {
		if (is_ready())
			return true;
		if (_state & kSTATE_DECODEING)
			return false;

		RunLoop::first()->post(Cb([this](Cb::Data& e) {
			if (_state & kSTATE_DECODEING)
				return;
			_state = State(_state | kSTATE_DECODEING);
			Qk_Trigger(State, _state);

			if (_state & kSTATE_LOAD_COMPLETE) {
				_Decode();
			} else { // load and decode
				_Load();
			}
		}, this));

		return false;
	}

	void ImageSource::_Load() {
		if (_state & kSTATE_LOADING)
			return;

		_state = State(_state | kSTATE_LOADING);
		Qk_Trigger(State, _state); // trigger

		_load_id = fs_reader()->read_file(_uri, Cb([this](Cb::Data& e) { // read data
			if (_state & kSTATE_LOADING) {
				if (e.error) {
					_state = State((_state | kSTATE_LOAD_ERROR) & ~kSTATE_LOADING);
					Qk_DEBUG("#ImageSource::_Load kSTATE_LOAD_ERROR, %s", e.error->message().c_str());
					Qk_Trigger(State, _state);
					return;
				}
				_state = State((_state | kSTATE_LOAD_COMPLETE) & ~kSTATE_LOADING);
				_loaded = *static_cast<Buffer*>(e.data);

				if (_state & kSTATE_DECODEING) {
					_Decode();
				} else {
					if (!img_test(_loaded, &_info)) // test fail
						_state = State(_state | kSTATE_DECODE_ERROR);
					Qk_Trigger(State, _state);
				}
			}
		}, this));
	}

	void ImageSource::_Decode() {
		Qk_ASSERT(_state & kSTATE_LOAD_COMPLETE, "#ImageSource::_Decode");

		struct Ctx {
			bool isComplete;
			Array<Pixel> pixels;
		};
		auto ctx = new Ctx{false};

		RunLoop::first()->work(Cb([this, ctx](Cb::Data& e) {
			ctx->isComplete = img_decode(_loaded, &ctx->pixels);
		}), Cb([this, ctx](Cb::Data& e) {
			if (_state & kSTATE_DECODEING) {
				if (ctx->isComplete) { // decode image complete
					_loaded.clear(); // clear load data
					_info = ctx->pixels[0];
					_pixels = std::move(ctx->pixels);
					_state = State((_state | kSTATE_DECODE_COMPLETE) & ~kSTATE_DECODEING);
				} else { // decode fail
					_state = State((_state | kSTATE_DECODE_ERROR)    & ~kSTATE_DECODEING);
				}
				Qk_Trigger(State, _state);
			}
		}, this));
	}

	/**
		* @func unload() delete load and ready
		*/
	void ImageSource::unload() {
		_state = State( _state & ~(kSTATE_LOADING | kSTATE_LOAD_COMPLETE | kSTATE_DECODEING | kSTATE_DECODE_COMPLETE) );
		_loaded.clear(); // clear raw data
		if (_load_id) { // cancel load and ready
			fs_reader()->abort(_load_id);
			_load_id = 0;
		}
		// trigger event
		RunLoop::first()->post(Cb([this](Cb::Data& e){
			Qk_Trigger(State, _state);
		}, this));
	}

	/**
		* 
		* mark as gpu texture
		*
		* @func mark_as_texture()
		*/
	Sp<ImageSource> ImageSource::mark_as_texture() {
		// TODO ...
	}

	// -------------------- I m a g e . S o u r c e . P o o l --------------------

	void ImageSourcePool::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		ScopeLock locl(_Mutex);
		auto id = evt.sender()->uri().hash_code();
		auto it = _sources.find(id);
		if (it != _sources.end()) {
			auto info = evt.sender()->info();
			int ch = int(info.size()) - int(it->value.size);
			if (ch != 0) {
				_total_data_size += ch; // change
				it->value.size = info.size();
			}
		}
	}

	ImageSourcePool::ImageSourcePool(Application* host): _host(host) {
	}

	ImageSourcePool::~ImageSourcePool() {
		for (auto& it: _sources) {
			it.value.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
		}
	}

	ImageSource* ImageSourcePool::get(cString& uri) {
		ScopeLock local(_Mutex);
		String _uri = fs_reader()->format(uri);
		uint64_t id = _uri.hash_code();

		// 通过路径查找
		auto it = _sources.find(id);
		if ( it != _sources.end() ) {
			return it->value.source.value();
		}

		ImageSource* source = new ImageSource(_uri);
		source->Qk_On(State, &ImageSourcePool::handleSourceState, this);
		auto info = source->info();
		_sources.set(id, { info.size(), source });
		_total_data_size += info.size();

		return source;
	}

	void ImageSourcePool::remove(cString& uri) {
		ScopeLock local(_Mutex);
		String _uri = fs_reader()->format(uri);
		auto it = _sources.find(_uri.hash_code());
		if (it != _sources.end()) {
			it->value.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
			_sources.erase(it);
			_total_data_size -= it->value.size;
		}
	}

	void ImageSourcePool::clear(bool full) {
		ScopeLock local(_Mutex);
		// TODO ..
	}

	// -------------------- I m a g e . S o u r c e . H o l d --------------------

	ImageSourceHold::~ImageSourceHold() {
		if (_imageSource) {
			_imageSource->Qk_Off(State, &ImageSourceHold::handleSourceState, this);
		}
	}

	String ImageSourceHold::src() const {
		return _imageSource ? _imageSource->uri(): String();
	}

	ImageSource* ImageSourceHold::source() {
		return _imageSource.value();
	}

	void ImageSourceHold::set_src(String value) {
		set_source(app() ? app()->img_pool()->get(value): new ImageSource(value));
	}

	void ImageSourceHold::set_source(ImageSource* source) {
		if (_imageSource.value() != source) {
			if (_imageSource) {
				_imageSource->Qk_Off(State, &ImageSourceHold::handleSourceState, this);
			}
			if (source) {
				source->Qk_On(State, &ImageSourceHold::handleSourceState, this);
			}
			_imageSource = Handle<ImageSource>(source);
		}
	}

	void ImageSourceHold::handleSourceState(Event<ImageSource, ImageSource::State>& evt) { // 收到图像变化通知
		onSourceState(evt);
	}

	void ImageSourceHold::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (*evt.data() & ImageSource::kSTATE_DECODE_COMPLETE) {
			auto _ = app();
			// Qk_ASSERT(_, "Application needs to be initialized first");
			if (_) {
				_->pre_render()->mark_none();
			}
		}
	}

}
