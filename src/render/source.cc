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

	Qk_DEFINE_INLINE_MEMBERS(ImageSource, Inl) {
	public:
		void setTex_Rt(cPixelInfo &info, const TexStat *tex, bool isMipmap);
	};

	void setTex_SourceImage_Rt(ImageSource* img, cPixelInfo &i, const TexStat *tex, bool isMipmap) {
		static_cast<ImageSource::Inl*>(img)->setTex_Rt(i, tex, isMipmap);
	}

	ImageSource::ImageSource(RenderBackend *render, RunLoop *loop): Qk_Init_Event(State)
		, _state(kSTATE_NONE)
		, _loadId(0), _render(render), _loop(loop), _isMipmap(true)
	{
	}

	Sp<ImageSource> ImageSource::Make(cString& uri, RunLoop *loop)
	{
		auto img = new ImageSource(nullptr, loop);
		if (!uri.isEmpty())
			img->_uri = fs_reader()->format(uri);
		return img;
	}

	Sp<ImageSource> ImageSource::Make(cPixelInfo &info, RenderBackend *render, RunLoop *loop) {
		auto img = new ImageSource(render, loop);
		img->_info = info;
		return img;
	}

	Sp<ImageSource> ImageSource::Make(Array<Pixel>&& pixels, RenderBackend *render, RunLoop *loop)
	{
		Sp<ImageSource> img = new ImageSource(render, loop);
		if (pixels.length()) {
			img->_state = kSTATE_LOAD_COMPLETE;
			img->_info = pixels[0];
			if (render) {
				img->_pixels = img->_copyInfo(pixels);
				img->_ReloadTexture(pixels);
			} else {
				img->_pixels = std::move(pixels);
			}
		}
		Qk_ReturnLocal(img);
	}

	ImageSource::~ImageSource() {
		_Unload(true);
	}

	bool ImageSource::markAsTexture(RenderBackend *render) {
		if (!render) return false;
		if (_render)
			return _render == render;
		_render = render;
		if (_pixels.length() && _pixels.front().body().length()) {
			Array<Pixel> pixels(std::move(_pixels));
			_pixels = _copyInfo(pixels);
			_ReloadTexture(pixels);
		}
		return true;
	}

	bool ImageSource::load() {
		if (_state & kSTATE_LOAD_COMPLETE)
			return true;
		if (_state & (kSTATE_LOADING | kSTATE_LOAD_ERROR | kSTATE_DECODE_ERROR))
			return false;

		if (!_loop || _uri.isEmpty()) // empty uri or loop null
			return false;

		_loop->post(Cb([this](auto &e) {
			if (_state & kSTATE_LOADING)
				return;
			_state = State(_state | kSTATE_LOADING);
			Qk_Trigger(State, _state); // trigger

			_loadId = fs_reader()->read_file(_uri, Callback<Buffer>([this](auto& e) { // read data
				if (_state & kSTATE_LOADING) {
					if (e.error) {
						_state = State((_state | kSTATE_LOAD_ERROR) & ~kSTATE_LOADING);
						Qk_DEBUG("#ImageSource::_Load kSTATE_LOAD_ERROR, %s", e.error->message().c_str());
						Qk_Trigger(State, _state);
					} else {
						_Decode(*e.data);
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
					_state = State((_state | kSTATE_LOAD_COMPLETE) & ~kSTATE_LOADING);
					_info = ctx->pixels[0];
					if (_render) {
						_pixels = _copyInfo(ctx->pixels);
						_ReloadTexture(ctx->pixels);
					} else {
						_pixels = std::move(ctx->pixels);
					}
				} else { // decode fail
					_state = State((_state | kSTATE_DECODE_ERROR)  & ~kSTATE_LOADING);
				}
				Qk_Trigger(State, _state);
			}
			delete ctx;
		}, this));
	}

	void ImageSource::_ReloadTexture(Array<Pixel>& pixels) {
		// set gpu texture, Must be processed in the rendering thread
		auto ptr = new Array<Pixel>(std::move(pixels));
		_render->post_message(Cb([this,ptr](auto& data) {
			Sp<Array<Pixel>> hold(ptr);
			int i = 0;
			int len = ptr->length(), old_len = _tex_Rt.length();
			Array<const TexStat*> texStat(len);

			while (i < len) {
				auto tex = const_cast<TexStat *>(i < old_len ? _tex_Rt[i]: nullptr);
				_render->makeTexture(ptr->val() + i, tex, true);
				texStat[i++] = tex;
			}

			while(i < old_len) {
				if (_tex_Rt[i])
					_render->deleteTexture(const_cast<TexStat *>(_tex_Rt[i]));
				i++;
			}
			_tex_Rt = std::move(texStat);
		}, this));
	}

	Array<Pixel> ImageSource::_copyInfo(Array<Pixel>& src) {
		Array<Pixel> dest;
		for (auto &pixel: src) {
			dest.push(PixelInfo(pixel));
		}
		Qk_ReturnLocal(dest);
	}

	void ImageSource::unload() {
		if (_loop) {
			_loop->post(Cb([this](auto &e) {
				_Unload(false);
				Qk_Trigger(State, _state);
			}, this));
		}
	}

	void ImageSource::_Unload(bool isDestroy) {
		_state = State( _state & ~(kSTATE_LOADING | kSTATE_LOAD_COMPLETE) );
		_pixels.clear();

		if (_loadId) {
			fs_reader()->abort(_loadId); // cancel load and ready
			_loadId = 0;
		}

		if (_render) { // as texture, Must be processed in the rendering thread
			auto render = _render;
			auto tex = isDestroy ? new Array<const TexStat*>(std::move(_tex_Rt)): &_tex_Rt;
			render->post_message(Cb([isDestroy,render,tex](auto& data) {
				for (auto i: *tex) {
					if (i)
						render->deleteTexture(const_cast<TexStat *>(i));
				}
				if (isDestroy) Release(tex); else tex->clear();
			}, isDestroy ? nullptr: this));
		}
	}

	void ImageSource::Inl::setTex_Rt(cPixelInfo &info, const TexStat *tex, bool isMipmap) {
		if (_tex_Rt.length()) { // replace old texture
			auto oldTex = _tex_Rt[0];
			if (oldTex && oldTex != tex)
				_render->deleteTexture(const_cast<TexStat *>(oldTex));
			_tex_Rt[0] = tex;
		} else {
			_tex_Rt.push(tex);
		}
		_state = kSTATE_LOAD_COMPLETE;
		_info = info;
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
		auto source = ImageSource::Make(_uri, _loop);
		source->Qk_On(State, &ImageSourcePool::handleSourceState, this);
		auto info = source->info();
		_sources.set(id, { source, info.bytes(), 0 });
		_capacity += info.bytes();

		return *source;
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
		if (_imageSource)
			_imageSource->Qk_Off(State, &ImageSourceHolder::handleSourceState, this);
		// Qk_Fatal_Assert(!_imageSource);
	}

	String ImageSourceHolder::src() const {
		return _imageSource ? _imageSource->uri(): String();
	}

	ImageSource* ImageSourceHolder::source() {
		return *_imageSource;
	}

	void ImageSourceHolder::set_src(String value) {
		auto pool = imgPool();
		set_source(pool ? pool->get(value): *ImageSource::Make(value));
	}

	void ImageSourceHolder::set_source(ImageSource* source) {
		if (*_imageSource != source) {
			if (_imageSource) {
				_imageSource->Qk_Off(State, &ImageSourceHolder::handleSourceState, this);
				auto pool = imgPool();
				if (pool)
					// retain source and delay to main loop remease
					pool->loop()->post(Cb([](auto&e){}, *_imageSource));
			}
			if (source) {
				source->Qk_On(State, &ImageSourceHolder::handleSourceState, this);
			}
			_imageSource = source;
		}
	}

	void ImageSourceHolder::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		onSourceState(evt);
	}

	void ImageSourceHolder::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (*evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {}
	}

	ImagePool* ImageSourceHolder::imgPool() {
		return nullptr;
	}

}
