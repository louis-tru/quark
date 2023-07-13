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
#include "./render.h"


namespace qk {

	// -------------------- I m a g e . S o u r c e --------------------

	ImageSource::ImageSource(cString& uri): Qk_Init_Event(State)
		, _state(kSTATE_NONE)
		, _load_id(0), _device(nullptr)
	{
		if (!uri.isEmpty())
			_uri = fs_reader()->format(uri);
	}

	ImageSource::ImageSource(Array<Pixel>&& pixels): Qk_Init_Event(State)
		, _uri(String::format("mem://%d", random()))
		, _state(kSTATE_NONE)
		, _load_id(0), _device(nullptr)
	{
		reload_unsafe(std::move(pixels));
	}

	ImageSource::~ImageSource() {
		_Unload();
	}

	bool ImageSource::reload_unsafe(Array<Pixel>&& pixels, BackendDevice *device) {
		if (!pixels.length())
			return false;
		if (_state & kSTATE_LOADING)
			return false;

		Qk_ASSERT(!_pixels.length() || _info.bytes() == _pixels[0].body().length(), "old pixel bytes size no match");

		if (_device) {
			Qk_ASSERT(_device == device, "device no match");
		} else {
			device = _device;
		}

		if (device) { // mark as texture
			uint32_t i = 0;
			uint32_t old_len = _pixels.length();

			static auto bollback = [](
				BackendDevice *device,
				int idx, Array<Pixel> &old, Array<Pixel> &pixels
			) {
				for (int j = 0; j < idx; j++) {
					if (old[j]._texture == 0 && pixels[j]._texture != 0) {
						device->deleteTextures(&pixels[j]._texture, 1); // RollBACK
						pixels[j]._texture = 0;
					}
				}
			};

			while (i < pixels.length()) {
				auto &pix = pixels[i];
				auto id = device->makeTexture(&pix, i < old_len ? _pixels[i]._texture: 0);
				if (id == 0) {
					bollback(device, i, _pixels, pixels);
					return false;
				}
				pix = PixelInfo(pix); // clear memory pixel data
				pix._texture = id;
				i++;
			}

			if (i < old_len) {
				do
					device->deleteTextures(&_pixels[i]._texture, 1);
				while(++i < old_len);
			}
		}

		_info = pixels[0];
		_state = kSTATE_LOAD_COMPLETE;
		_pixels = std::move(pixels);
		_device = device;

		return true;
	}

	/**
		* 
		* mark as gpu texture
		*
	 * @method mark_as_texture_unsafe()
	 */
	Sp<ImageSource> ImageSource::mark_as_texture_unsafe(BackendDevice *device) const {
		if (!device && _device)
			return nullptr;
		auto src = new ImageSource();
		src->_uri = _uri;
		src->reload_unsafe(Array<Pixel>(_pixels), device);
		return src;
	}

	/**
	 * @method load() async load source and decode
	 */
	bool ImageSource::load() {
		if (_state & kSTATE_LOAD_COMPLETE)
			return true;
		if (_state & (kSTATE_LOADING | kSTATE_LOAD_ERROR | kSTATE_DECODE_ERROR))
			return false;

		if (_uri.isEmpty()) // empty uri
			return false;

		RunLoop::first()->post(Cb([this](auto e) {
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
					} else {
						_Decode(*static_cast<Buffer*>(e.data));
					}
				}
			}, this));
		}, this));

		return false;
	}

	void ImageSource::_Decode(Buffer& data) {
		struct Ctx {
			bool isComplete;
			Buffer data;
			Array<Pixel> pixels;
		};
		auto ctx = new Ctx{false, data};

		RunLoop::first()->work(Cb([this, ctx](Cb::Data& e) {
			ctx->isComplete = img_decode(ctx->data, &ctx->pixels);
		}), Cb([this, ctx](Cb::Data& e) {
			if (_state & kSTATE_LOADING) {
				if (ctx->isComplete) { // decode image complete
					_state = State((_state | kSTATE_LOAD_COMPLETE) & ~kSTATE_LOADING);
					_info = ctx->pixels[0];
					_pixels = std::move(ctx->pixels);
				} else { // decode fail
					_state = State((_state | kSTATE_DECODE_ERROR)  & ~kSTATE_LOADING);
				}
				Qk_Trigger(State, _state);
			}
			delete ctx;
		}, this));
	}

	/**
	 * @method unload() delete load and ready
	 */
	void ImageSource::unload() {
		RunLoop::first()->post(Cb([this](auto e) {
			_Unload();
			Qk_Trigger(State, _state);
		}, this));
	}

	void ImageSource::_Unload() {
		_state = State( _state & ~(kSTATE_LOADING | kSTATE_LOAD_COMPLETE) );
		fs_reader()->abort(_load_id); // cancel load and ready
		_load_id = 0;

		if (_device) {
			Array<uint32_t> IDs;
			for (auto &pix: _pixels) {
				IDs.push(pix.texture());
			}
			auto device = _device;
			_device = nullptr;

			device->post_message(Cb([device,IDs](Cb::Data& data) {
				device->deleteTextures(IDs.val(), IDs.length());
			}));
		}
		_pixels.clear();
	}

	// -------------------- I m a g e . S o u r c e . P o o l --------------------

	void ImageSourcePool::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		ScopeLock locl(_Mutex);
		auto id = evt.sender()->uri().hashCode();
		auto it = _sources.find(id);
		if (it != _sources.end()) {
			auto info = evt.sender()->info();
			int ch = int(info.bytes()) - int(it->value.size);
			if (ch != 0) {
				_total_data_size += ch; // change
				it->value.size = info.bytes();
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
		uint64_t id = _uri.hashCode();

		// find image source by path
		auto it = _sources.find(id);
		if ( it != _sources.end() ) {
			return it->value.source.value();
		}

		ImageSource* source = new ImageSource(_uri);
		source->Qk_On(State, &ImageSourcePool::handleSourceState, this);
		auto info = source->info();
		_sources.set(id, { info.bytes(), source });
		_total_data_size += info.bytes();

		return source;
	}

	void ImageSourcePool::remove(cString& uri) {
		ScopeLock local(_Mutex);
		String _uri = fs_reader()->format(uri);
		auto it = _sources.find(_uri.hashCode());
		if (it != _sources.end()) {
			it->value.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
			_sources.erase(it);
			_total_data_size -= it->value.size;
		}
	}

	void ImageSourcePool::clean(bool full) {
		ScopeLock local(_Mutex);
		// TODO ..
	}

	// -------------------- I m a g e . S o u r c e . H o l d e r --------------------

	ImageSourceHolder::~ImageSourceHolder() {
		if (_imageSource) {
			_imageSource->Qk_Off(State, &ImageSourceHolder::handleSourceState, this);
		}
	}

	String ImageSourceHolder::src() const {
		return _imageSource ? _imageSource->uri(): String();
	}

	ImageSource* ImageSourceHolder::source() {
		return _imageSource.value();
	}

	void ImageSourceHolder::set_src(String value) {
		set_source(shared_app() ? shared_app()->img_pool()->get(value): new ImageSource(value));
	}

	void ImageSourceHolder::set_source(ImageSource* source) {
		if (_imageSource.value() != source) {
			if (_imageSource) {
				_imageSource->Qk_Off(State, &ImageSourceHolder::handleSourceState, this);
			}
			if (source) {
				source->Qk_On(State, &ImageSourceHolder::handleSourceState, this);
			}
			_imageSource = Handle<ImageSource>(source);
		}
	}

	void ImageSourceHolder::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		onSourceState(evt);
	}

	void ImageSourceHolder::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (*evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
			auto _ = shared_app();
			// Qk_ASSERT(_, "Application needs to be initialized first");
			if (_) {
				_->pre_render()->mark_none();
			}
		}
	}

}
