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


namespace quark {

	// -------------------- I m a g e . S o u r c e --------------------

	ImageSource::ImageSource(cString& uri)
		: Qk_Init_Event(State)
		, _uri(fs_reader()->format(uri))
		, _state(STATE_NONE)
		, _load_id(0), _size(0), _used(0)
		, _inl(nullptr)
	{
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
			RunLoop::first()->post(Cb([this](Cb::Data& e){
				Qk_Trigger(State, _state);
				_Decode();
			}, this));
		} else { // load and decode
			load();
		}
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
