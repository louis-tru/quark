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



namespace quark {

	// -------------------- I m a g e . S o u r c e --------------------

	ImageSource::ImageSource(cString& uri): Qk_Init_Event(State)
		, _uri(fs_reader()->format(uri))
		, _state(STATE_NONE)
		, _load_id(0), _size(0)
		, _inl(nullptr)
	{}

	ImageSource::~ImageSource() {
		if (_inl) {
			// TODO ..
			// sk_I(_inl)->unref();
			_inl = nullptr;
			_state = STATE_NONE;
			_size = 0;
		}
		unload();
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

		RunLoop::first()->post(Cb([this](auto e) {
			_Load();
		}, this));

		return false;
	}

	/**
		* @func ready() async ready decode
		*/
	bool ImageSource::ready() {
		if (is_ready()) {
			return true;
		}
		if (_state & STATE_DECODEING) {
			return false;
		}

		RunLoop::first()->post(Cb([this](Cb::Data& e) {
			if (_state & STATE_DECODEING)
				return;
			_state = State(_state | STATE_DECODEING);
			Qk_Trigger(State, _state);

			if (_state & STATE_LOAD_COMPLETE) {
				_Decode();
			} else { // load and decode
				_Load();
			}
		}, this));

		return false;
	}

	void ImageSource::_Load() {
		if (_state & STATE_LOADING)
			return;

		_state = State(_state | STATE_LOADING);
		Qk_Trigger(State, _state); // trigger

		_load_id = fs_reader()->read_file(_uri, Cb([this](Cb::Data& e){ // read data
			if (_state & STATE_LOADING) {
				_state = State((_state | STATE_LOAD_COMPLETE) & ~STATE_LOADING);
				_loaded = *static_cast<Buffer*>(e.data);
				_size = _loaded.length();

				PixelInfo info;
				if (img_test(_loaded, &info)) {
					_width = info.width();
					_height = info.height();
					_type = ColorType(info.type());

					Qk_Trigger(State, _state);

					if (_state & STATE_DECODEING) { // decode
						_Decode();
					}
				} else {
					_state = State(_state | STATE_DECODE_ERROR);

					Qk_Trigger(State, _state);
				}
			}
		}, this));
	}

	void ImageSource::_Decode() {
		Qk_ASSERT(_state & STATE_LOAD_COMPLETE);
		// Qk_ASSERT(_inl);
		// decode image

		RunLoop::first()->work(Cb([this](Cb::Data& e) {
				Array<Pixel> pixel;
				if (img_decode(_loaded, &pixel)) {
				}
		}), Cb([this](Cb::Data& e) {
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
		}, this));
	}

	/**
		* @func unload() delete load and ready
		*/
	void ImageSource::unload() {
		if (_inl) {
			if (_memPixel.body().is_null()) { // no mem pixel
				// TODO ..
				// sk_I(_inl)->unref();
				_inl = nullptr;
				_state = State( _state & ~(STATE_LOADING | STATE_LOAD_COMPLETE | STATE_DECODEING | STATE_DECODE_COMPLETE) );
				_loaded.clear(); // clear raw data
				_size = 0;
				if (_load_id) { // cancel load and ready
					fs_reader()->abort(_load_id);
					_load_id = 0;
				}
				// trigger event
				RunLoop::first()->post(Cb([this](Cb::Data& e){
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

	// -------------------- I m a g e . S o u r c e . P o o l --------------------

	Qk_DEFINE_INLINE_MEMBERS(ImageSourcePool, Inl) {
	public:
		#define _inl_pool(self) static_cast<ImageSourcePool::Inl*>(self)

		void onSourceStateHandle(Event<ImageSource, ImageSource::State>& evt) {
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

	ImageSourcePool::ImageSourcePool(Application* host): _host(host) {
	}

	ImageSourcePool::~ImageSourcePool() {
		for (auto& it: _sources) {
			it.value.source->Qk_Off(State, &Inl::onSourceStateHandle, _inl_pool(this));
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
		source->Qk_On(State, &Inl::onSourceStateHandle, _inl_pool(this));
		_sources.set(id, { source->size(), source });
		_total_data_size += source->size();

		return source;
	}

	void ImageSourcePool::remove(cString& uri) {
		ScopeLock local(_Mutex);
		String _uri = fs_reader()->format(uri);
		auto it = _sources.find(_uri.hash_code());
		if (it != _sources.end()) {
			it->value.source->Qk_Off(State, &Inl::onSourceStateHandle, _inl_pool(this));
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
		if (*evt.data() & ImageSource::STATE_DECODE_COMPLETE) {
			auto _ = app();
			// Qk_ASSERT(_, "Application needs to be initialized first");
			if (_) {
				_->pre_render()->mark_none();
			}
		}
	}

}
