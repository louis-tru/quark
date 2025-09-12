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

	static Array<Pixel> copyInfo(cArray<Pixel>& src) {
		Array<Pixel> dest;
		for (auto &pixel: src) {
			dest.push(PixelInfo(pixel));
		}
		Qk_ReturnLocal(dest);
	}

	static void deleteTextures(RenderResource *res, cArray<const TexStat*> &tex) {
		for (auto i: tex)
			if (i)
				res->deleteTexture(const_cast<TexStat *>(i));
	}

	RenderResource* getSharedRenderResource();

	Qk_DEFINE_INLINE_MEMBERS(ImageSource, Inl) {
	public:
		void setMipmap(bool val) {
			_isMipmap = val;
		}
		void setTex(RenderResource *res, cPixelInfo &info, const TexStat *tex);
	};

	void setMipmap_SourceImage(ImageSource* img, bool val) {
		static_cast<ImageSource::Inl*>(img)->setMipmap(val);
	}
	void setTex_SourceImage(RenderResource *res,
			ImageSource* img, cPixelInfo &i, const TexStat *tex)
	{
		static_cast<ImageSource::Inl*>(img)->setTex(res, i, tex);
	}

	ImageSource::ImageSource(RenderResource *res, RunLoop *loop): Qk_Init_Event(State)
		, _state(kSTATE_NONE)
		, _loadId(0), _res(res), _loop(loop), _isMipmap(true)
	{
	}

	Sp<ImageSource> ImageSource::Make(cString& uri, RunLoop *loop)
	{
		auto img = new ImageSource(nullptr, loop);
		if (!uri.isEmpty())
			img->_uri = fs_reader()->format(uri);
		return img;
	}

	Sp<ImageSource> ImageSource::Make(Array<Pixel>&& pixels, RenderResource *res)
	{
		Sp<ImageSource> img = new ImageSource(res, nullptr);
		if (pixels.length()) {
			img->_info = pixels[0];
			if (pixels[0].length()) {
				img->_state = kSTATE_LOAD_COMPLETE;
				img->_pixels = std::move(pixels);
				if (res) {
					img->_ReloadTexture();
				}
			}
		}
		Qk_ReturnLocal(img);
	}

	Sp<ImageSource> ImageSource::Make(Pixel&& pixel, RenderResource *res) {
		if (pixel.val()) {
			Array<Pixel> pixels;
			pixels.push(std::move(pixel));
			return Make(std::move(pixels), res);
		} else {
			auto img = new ImageSource(res, nullptr);
			img->_info = pixel;
			return img;
		}
	}

	ImageSource::~ImageSource() {
		_Unload(true);
	}

	bool ImageSource::markAsTexture() {
		if (_res)
			return true;
		_res = getSharedRenderResource();

		if (!_res) {
			return false;
		}
		if (_pixels.length()) {
			_ReloadTexture();
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

		_loop->post(Cb([this](auto e) { // to call from mt
			if (_state & kSTATE_LOADING)
				return;
			_state = State(_state | kSTATE_LOADING);
			Qk_Trigger(State, _state); // trigger

			_loadId = fs_reader()->read_file(_uri, Callback<Buffer>([this](auto e) { // read data
				if (_state & kSTATE_LOADING) {
					if (e.error) {
						_state = State((_state | kSTATE_LOAD_ERROR) & ~kSTATE_LOADING);
						Qk_DLog("ImageSource::load() kSTATE_LOAD_ERROR, %s", e.error->message().c_str());
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
		struct Running: Cb::Core {
			void call(Data& evt) override { // to call from mt
				auto self = source.get();
				if (self->_state & kSTATE_LOADING) {
					if (completed) { // decode image complete
						self->_onState.lock(); // lock, safe assign `_pixels`
						self->_state = State((self->_state | kSTATE_LOAD_COMPLETE) & ~kSTATE_LOADING);
						self->_info = pixels[0];
						self->_pixels = std::move(pixels);
						self->_onState.unlock();
						if (self->_res) {
							self->_ReloadTexture();
						}
					} else { // decode fail
						self->_state = State((self->_state | kSTATE_DECODE_ERROR)  & ~kSTATE_LOADING);
					}
					self->Qk_Trigger(State, self->_state);
				}
			}
			void decode() {
				completed = img_decode(data, &pixels);
			}
			void run(ImageSource *source_, Buffer &data_) {
				source = source_;
				data = data_;
				source->_loop->work(Cb([this](auto e) { decode(); }), Cb(this));
			}
			Buffer data;
			Array<Pixel> pixels;
			Sp<ImageSource> source; // hold source
			bool completed = false;
		};
		New<Running>()->run(this, data);
	}

	void ImageSource::_ReloadTexture() {
		// set gpu texture, Must be processed in the rendering thread
		struct Running: Cb::Core {
			Running(ImageSource* s): source(s) {
			}
			void call(Data& evt) override {
				AutoSharedMutexExclusive ame(source.get()->_onState);
				auto self = source.get();
				auto &pixels = self->_pixels;
				if (!pixels.length() || !pixels[0].length())
					return;
				int i = 0;
				int levels = 1;
				int len = pixels.length(), old_len = self->_tex.length();
				if (len > 1 && self->_info.type() >= kPVRTCI_2BPP_RGB_ColorType) {
					if (pixels[0].width() >> 1 == pixels[1].width()) {
						levels = len;
						len = 1;
					}
				}
				Array<const TexStat*> texStat(len);

				// Qk_DLog("_ReloadTexture(), len: %d, uri: %s", pixels[0].length(), self->_uri.c_str());

				while (i < len) {
					auto tex = const_cast<TexStat *>(i < old_len ? self->_tex[i]: nullptr);
					self->_res->newTexture(pixels.val() + i, levels, tex, true);
					texStat[i++] = tex;
				}

				while(i < old_len) {
					if (self->_tex[i])
						self->_res->deleteTexture(const_cast<TexStat *>(self->_tex[i]));
					i++;
				}
				self->_tex = std::move(texStat);
				self->_pixels = copyInfo(self->_pixels); // delete pixels data as save memory space
			}
			Sp<ImageSource> source;
		};
		_res->post_message(Cb(new Running(this)));
	}

	void ImageSource::unload() {
		if (_loop) {
			_loop->post(Cb([this](auto e) {
				_Unload(false);
				Qk_Trigger(State, _state);
			}, this));
		}
	}

	void ImageSource::_Unload(bool destroy) {
		{
			AutoSharedMutexExclusive ame(_onState); // lock, safe assign `_pixels`

			_state = State( _state & ~(kSTATE_LOADING | kSTATE_LOAD_COMPLETE) );
			_pixels.clear();
			if (_loadId) {
				fs_reader()->abort(_loadId); // to cancel load and ready status
				_loadId = 0;
			}

			if (!_res) return;

			// as texture, Must be processed in the rendering thread

			if (destroy) {
				auto res = _res;
				auto tex = new Array<const TexStat*>(std::move(_tex));
				_res->post_message(Cb([res,tex](auto e) { // to call from Rt
					deleteTextures(res, *tex);
					Release(tex);
				}));
			}
		}

		// avoiding deadlock
		if (!destroy) {
			_res->post_message(Cb([this](auto e) { // to call from Rt
				AutoSharedMutexExclusive ame(_onState);
				deleteTextures(_res, _tex);
				_tex.clear();
			}, this));
		}
	}

	void ImageSource::Inl::setTex(RenderResource *res, cPixelInfo &info, const TexStat *tex) {
		AutoSharedMutexExclusive ame(_onState);
		if (!_res)
			_res = res;
		if (_tex.length()) { // replace old texture
			auto oldTex = _tex[0];
			if (oldTex && oldTex != tex)
				res->deleteTexture(const_cast<TexStat *>(oldTex));
			_tex[0] = tex;
		} else {
			_tex.push(tex);
		}
		_state = kSTATE_LOAD_COMPLETE; //
		_info = info; //
	}

	// -------------------- I m a g e . S o u r c e . P o o l --------------------

	void ImageSourcePool::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		AutoMutexExclusive locl(_Mutex);
		auto id = evt.sender()->uri().hashCode();
		auto it = _sources.find(id);
		if (it != _sources.end()) {
			if (evt.data() == ImageSource::kSTATE_LOAD_COMPLETE) {
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

	ImageSourcePool::ImageSourcePool(RunLoop *loop): _loop(loop), _isMarkAsTexture(true) {
		Qk_CHECK(loop, "Create the ImageSourcePool fail, Haven't param RunLoop");
	}

	ImageSourcePool::~ImageSourcePool() {
		_Mutex.lock();
		for (auto& it: _sources) {
			it.value.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
		}
		_Mutex.unlock();
	}

	ImageSource* ImageSourcePool::get(cString& uri) {
		AutoMutexExclusive local(_Mutex);
		String _uri = fs_reader()->format(uri);
		uint64_t id = _uri.hashCode();

		// find image source by path
		auto it = _sources.find(id);
		if ( it != _sources.end() ) {
			return it->value.source.get();
		}
		auto source = ImageSource::Make(_uri, _loop);
		if (_isMarkAsTexture)
			source->markAsTexture();
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
		AutoMutexExclusive local(_Mutex);
		String _uri = fs_reader()->format(uri);
		auto it = _sources.find(_uri.hashCode());
		if (it != _sources.end()) {
			it->value.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
			_sources.erase(it);
			_capacity -= it->value.bytes;
		}
	}

	void ImageSourcePool::clear(bool all) {
		AutoMutexExclusive local(_Mutex);
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

	ImageSourceHold::ImageSourceHold(): _imageSource(nullptr) {
	}

	ImageSourceHold::~ImageSourceHold() {
		auto source = _imageSource.load();
		if (source) {
			source->Qk_Off(State, &ImageSourceHold::handleSourceState, this);
			source->release();
		}
	}

	String ImageSourceHold::src() const {
		auto source = _imageSource.load();
		return source ? source->uri(): String();
	}

	Sp<ImageSource> ImageSourceHold::source() {
		return _imageSource.load();
	}

	void ImageSourceHold::set_src(String value) {
		auto pool = imgPool();
		if (pool) {
			set_source(pool->get(value));
		} else {
			set_source(*ImageSource::Make(value));
		}
	}

	void ImageSourceHold::set_source(Sp<ImageSource> source) {
		auto oldSrc = _imageSource.load();
		auto newSrc = source.get();
		if (oldSrc != newSrc) {
			_imageSource = newSrc;
			if (newSrc) {
				newSrc->Qk_On(State, &ImageSourceHold::handleSourceState, this);
				newSrc->retain();
			}
			if (oldSrc) {
				oldSrc->Qk_Off(State, &ImageSourceHold::handleSourceState, this);
				oldSrc->release();
			}
			onSourceState(ImageSource::kSTATE_NONE);
		}
	}

	void ImageSourceHold::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		onSourceState(evt.data());
	}

	void ImageSourceHold::onSourceState(ImageSource::State state) {
		if (state & ImageSource::kSTATE_LOAD_COMPLETE) {}
	}

	ImagePool* ImageSourceHold::imgPool() {
		return nullptr;
	}

}
