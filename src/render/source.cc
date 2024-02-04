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

#include "./source.h"
#include "../util/fs.h"
#include "./codec/codec.h"
#include "./render.h"

namespace qk {

	// -------------------- I m a g e . S o u r c e --------------------

	ImageSource::ImageSource(cString& uri, RunLoop *loop): Qk_Init_Event(State)
		, _state(kSTATE_NONE)
		, _loadId(0), _render(nullptr), _loop(loop), _isMipmap(true)
	{
		if (!uri.isEmpty())
			_uri = fs_reader()->format(uri);
	}

	ImageSource::ImageSource(Array<Pixel>&& pixels, RunLoop *loop): Qk_Init_Event(State)
		, _state(kSTATE_NONE)
		, _loadId(0), _render(nullptr), _loop(loop), _isMipmap(true)
	{
		if (pixels.length()) {
			_state = kSTATE_LOAD_COMPLETE;
			_info = pixels[0];
			_pixels = std::move(pixels);
		}
	}

	ImageSource::ImageSource(cPixelInfo &info, RenderBackend *render, RunLoop *loop): Qk_Init_Event(State)
		, _state(kSTATE_NONE)
		, _info(info)
		, _loadId(0), _render(render), _loop(loop), _isMipmap(true) {
	}

	ImageSource::~ImageSource() {
		_Unload(true);
	}

	/**
		* 
		* mark as gpu texture
		*
	 * @method markAsTexture()
	 */
	bool ImageSource::markAsTexture(RenderBackend *render) {
		if (_render)
			return true;
		if (!render)
			return false;
		_render = render;
		reload(std::move(_pixels));
		return true;
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

		_loop->post(Cb([this](auto &e) {
			if (_state & kSTATE_LOADING)
				return;
			_state = State(_state | kSTATE_LOADING);
			Qk_Trigger(State, _state); // trigger

			_loadId = fs_reader()->read_file(_uri, Cb([this](auto& e) { // read data
				if (_state & kSTATE_LOADING) {
					if (e.error) {
						_state = State((_state | kSTATE_LOAD_ERROR) & ~kSTATE_LOADING);
						Qk_DEBUG("#ImageSource::_Load kSTATE_LOAD_ERROR, %s", e.error->message().c_str());
						Qk_Trigger(State, _state);
					} else {
						_Decode(*static_cast<Buffer*>(e.data));
					}
				}
				_loadId = 0;
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

		_loop->work(Cb([this, ctx](auto& e) {
			ctx->isComplete = img_decode(ctx->data, &ctx->pixels);
		}), Cb([this, ctx](auto& e) {
			if (_state & kSTATE_LOADING) {
				if (ctx->isComplete) { // decode image complete
					auto state = State((_state | kSTATE_LOAD_COMPLETE) & ~kSTATE_LOADING);
					if (_render) {
						_state = state;
						reload(std::move(ctx->pixels));
					} else {
						_info = ctx->pixels[0];
						_pixels = std::move(ctx->pixels);
						_state = state;
					}
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
		_loop->post(Cb([this](auto &e) {
			_Unload(false);
			Qk_Trigger(State, _state);
		}, this));
	}

	void ImageSource::_Unload(bool destroy) {
		_state = State( _state & ~(kSTATE_LOADING | kSTATE_LOAD_COMPLETE) );

		if (_loadId) {
			fs_reader()->abort(_loadId); // cancel load and ready
			_loadId = 0;
		}
		if (_render) { // as texture, Must be processed in the rendering thread
			auto render = _render;
			if (destroy) {
				auto pix = new Array<Pixel>(std::move(_pixels));
				render->post_message(Cb([render,pix](auto& data) {
					for (auto &i: *pix)
						if (i._texture) render->deleteTexture(const_cast<TexStat *>(i._texture));
					Release(pix);
				}));
			} else {
				render->post_message(Cb([render,this](auto& data) { 
					for (auto &i: _pixels)
						if (i._texture) render->deleteTexture(const_cast<TexStat *>(i._texture));
					_pixels.clear();
				}, this));
			}
		} else {
			_pixels.clear();
		}
	}

	void ImageSource::reload(Array<Pixel>&& pixels_) {
		if (!pixels_.length() || _state & kSTATE_LOADING)
			return;

		_state = kSTATE_LOAD_COMPLETE;
		_info = pixels_[0]; // TODO Is it thread safe ?

		auto pixels = new Array<Pixel>(std::move(pixels_));

		if (!_render) {
			// Must be changed by the same thread
			_loop->post(Cb([this,pixels](auto& e){
				Sp<Array<Pixel>> hold(pixels);
				_state = kSTATE_LOAD_COMPLETE; // Reset to ensure it is valid
				_info = pixels->at(0);
				_pixels = std::move(*pixels);
			}, this));
			return;
		}

		// set gpu texture, Must be processed in the rendering thread
		_render->post_message(Cb([this,pixels](auto& data) {
			Sp<Array<Pixel>> hold(pixels);
			int i = 0;
			int old_len = _pixels.length();

			for (int len = pixels->length(); i < len; i++) {
				auto pix = pixels->val() + i;
				auto tex = const_cast<TexStat *>(i < old_len ? _pixels[i]._texture: nullptr);
				_render->makeTexture(pix, tex, true);
				pix->_body.clear(); // clear memory pixel data
				pix->_texture = tex; // set tex
			}

			while(i < old_len) {
				if (_pixels[i]._texture)
					_render->deleteTexture(const_cast<TexStat *>(_pixels[i]._texture));
				i++;
			}

			_state = kSTATE_LOAD_COMPLETE; // Reset to ensure it is valid
			_info = pixels->at(0);
			_pixels = std::move(*pixels);
		}, this));
	}

	class ImageSourceInl: public ImageSource {
	public:
		friend void setTex_SourceImage(ImageSource* s, cPixelInfo &i, const TexStat *tex, bool isMipmap);
	};

	void setTex_SourceImage(ImageSource* s, cPixelInfo &i, const TexStat *tex, bool isMipmap) {
		static_cast<ImageSourceInl*>(s)->_SetTex(i, tex, isMipmap);
	}

	void ImageSource::_SetTex(const PixelInfo &info, const TexStat *tex, bool isMipmap) {
		if (_pixels.length()) {
			auto oldTex = _pixels[0]._texture;
			if (oldTex && oldTex != tex)
				_render->deleteTexture(const_cast<TexStat *>(_pixels.val()->_texture));
			_pixels[0] = info;
		} else {
			_pixels.push(info);
		}
		_state = kSTATE_LOAD_COMPLETE;
		_info = info;
		_pixels[0]._texture = tex;
		_isMipmap = isMipmap;
	}

	// -------------------- I m a g e . S o u r c e . P o o l --------------------

	void ImageSourcePool::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		ScopeLock locl(_Mutex);
		auto id = evt.sender()->uri().hashCode();
		auto it = _sources.find(id);
		if (it != _sources.end()) {
			if (*evt.data() == ImageSource::kSTATE_LOAD_COMPLETE) {
				auto info = evt.sender()->info();
				int ch = int(info.bytes()) - int(it->value.bytes);
				if (ch != 0) {
					_capacity += ch; // change
					it->value.bytes = info.bytes();
					it->value.time = time_micro();
				}
			}
		}
	}

	ImageSourcePool::ImageSourcePool(RunLoop *loop): _loop(loop) {
	}

	ImageSourcePool::~ImageSourcePool() {
		_Mutex.lock();
		for (auto& it: _sources) {
			it.value.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
		}
		_Mutex.unlock();
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
		ImageSource* source = new ImageSource(_uri, _loop);
		source->Qk_On(State, &ImageSourcePool::handleSourceState, this);
		auto info = source->info();
		_sources.set(id, { source, info.bytes(), 0 });
		_capacity += info.bytes();

		return source;
	}

	ImageSource* ImageSourcePool::load(cString& uri) {
		auto s = get(uri);
		if (s)
			s->load();
		return s;
	}

	void ImageSourcePool::remove(cString& uri) {
		ScopeLock local(_Mutex);
		String _uri = fs_reader()->format(uri);
		auto it = _sources.find(_uri.hashCode());
		if (it != _sources.end()) {
			it->value.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
			_sources.erase(it);
			_capacity -= it->value.bytes;
		}
	}

	void ImageSourcePool::clear(bool all) {
		ScopeLock local(_Mutex);
		if (all) {
			for (auto &i: _sources) {
				if (i.value.source->state() & (
						ImageSource::kSTATE_LOADING | ImageSource::kSTATE_LOAD_COMPLETE
					)
				) {
					i.value.source->unload();
					_capacity -= i.value.bytes;
					i.value.bytes = 0;
					i.value.time = 0;
				}
			}
		} else {
			// sort by time asc and size desc
		}
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
		auto pool = imgPool();
		set_source(pool ? pool->get(value): new ImageSource(value));
	}

	ImagePool* ImageSourceHolder::imgPool() {
		return nullptr;
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
		if (*evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {}
	}

}
