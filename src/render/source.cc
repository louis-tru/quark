/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
#define Qk_ARM_NEON Qk_ARCH_ARM64
#if Qk_ARM_NEON
#include <arm_neon.h>
#endif
#include "../util/thread/inl.h"

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
		, _premultipliedAlpha(false)
		, _premulFlags(kConvert_PremulFlags)
	{
		// sizeof(ImageSource);
	}

	Sp<ImageSource> ImageSource::Make(cString& uri, RunLoop *loop)
	{
		auto img = new ImageSource(nullptr, loop);
		if (!uri.is_empty())
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
					img->reloadTexture();
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
		unloadInl(true);
	}

	bool ImageSource::markAsTexture() {
		if (_res)
			return true;
		_res = getSharedRenderResource();

		if (!_res) {
			return false;
		}
		if (_pixels.length()) {
			reloadTexture();
		}
		return true;
	}

	void ImageSource::set_premulFlags(PremulFlags val) {
		_premulFlags = val;
	}

	bool ImageSource::load() {
		if (_state & kSTATE_LOAD_COMPLETE)
			return true;
		if (_state & (kSTATE_LOADING | kSTATE_LOAD_ERROR | kSTATE_DECODE_ERROR))
			return false;

		if (!_loop || _uri.is_empty()) // empty uri or loop null
			return false;

		_loop->post(Cb([this](auto e) { // to call from mt
			if (_state & kSTATE_LOADING)
				return;
			_state = State(_state | kSTATE_LOADING);
			Qk_Trigger(State, _state); // trigger

			_loadId = fs_reader()->read_file(_uri, Callback<Buffer>([this](auto e) { // read data
				if (_state & kSTATE_LOADING) {
					if (e.error || e.data->length() == 0) {
						_state = State((_state | kSTATE_LOAD_ERROR) & ~kSTATE_LOADING);
						Qk_DLog("ImageSource::load() kSTATE_LOAD_ERROR, %s", e.error->message().c_str());
						Qk_Trigger(State, _state);
					} else {
						decode(*e.data);
					}
				}
				_loadId = 0;
			}, this));
		}, this));

		return false;
	}

	void ImageSource::decode(Buffer& data) {
		struct Running: Cb::Core {
			void call(Data& evt) override { // to call from mt
				auto self = source.get();
				if (self->_state & kSTATE_LOADING) {
					if (completed) { // decode image complete
						self->_onState.lock(); // lock, safe assign `_pixels`
						self->_state = State((self->_state | kSTATE_LOAD_COMPLETE) & ~kSTATE_LOADING);
						for (auto &pix: pixels) {
							if (pix.alphaType() == kUnpremul_AlphaType) {
								if (self->_premulFlags == kConvert_PremulFlags)
									ImageSource::toPremultipliedAlpha(pix);
								else if (self->_premulFlags == kOnlyMark_PremulFlags)
									pix._alphaType = kPremul_AlphaType; // mark as premultiplied
							}
						}
						self->_info = pixels[0];
						self->_premultipliedAlpha =
							self->_info.alphaType() == kPremul_AlphaType ||
							self->_info.alphaType() == kOpaque_AlphaType; // kOpaque_AlphaType also as premultiplied
						self->_pixels = std::move(pixels);
						self->_onState.unlock();

						if (self->_res) {
							self->reloadTexture();
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

	void ImageSource::reloadTexture() {
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
				unloadInl(false);
				Qk_Trigger(State, _state);
			}, this));
		}
	}

	void ImageSource::unloadInl(bool destroy) {
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

	// --------------------------------------------------
	//  预乘表： premulTable[a][c] = (c * a + 127) / 255
	// --------------------------------------------------
	static uint8_t premulTable[256][256] = {0};
	static bool premulTableInitialized = false;

	// 初始化查表（一次即可）
	static void initPremulTable() {
		if (premulTableInitialized)
			return; // 已初始化，无需重复
		for (int a = 0; a < 256; a++) {
			for (int c = 0; c < 256; c++)
				premulTable[a][c] = (uint8_t)((c * a + 127) / 255);
		}
		premulTableInitialized = true;
	}

	// data: RGBA8888 像素数据
	// count: 像素数量
	static void premultiplyRGBA8888(uint8_t* data, uint32_t count) {
		initPremulTable();
		uint32_t i = 0;
#if Qk_ARM_NEON
		const uint16x8_t add127 = vdupq_n_u16(127);

		// 每次处理 16 个像素（64 字节）
		for (; i + 16 <= count; i += 16) {
			uint8_t* p = data + i * 4;

			// 交织加载: [RGBA RGBA ...] -> rgba.val[0]=R..., val[1]=G..., val[2]=B..., val[3]=A...
			uint8x16x4_t rgba = vld4q_u8(p);

			uint8x16_t r = rgba.val[0];
			uint8x16_t g = rgba.val[1];
			uint8x16_t b = rgba.val[2];
			uint8x16_t a = rgba.val[3];

			// 拆低/高 8 像素
			uint8x8_t r_lo = vget_low_u8(r);
			uint8x8_t r_hi = vget_high_u8(r);
			uint8x8_t g_lo = vget_low_u8(g);
			uint8x8_t g_hi = vget_high_u8(g);
			uint8x8_t b_lo = vget_low_u8(b);
			uint8x8_t b_hi = vget_high_u8(b);
			uint8x8_t a_lo = vget_low_u8(a);
			uint8x8_t a_hi = vget_high_u8(a);

			// low 8 像素: (c * a + 127) >> 8
			{
				uint16x8_t a16 = vmovl_u8(a_lo);
				uint16x8_t r16 = vmlaq_u16(add127, vmovl_u8(r_lo), a16);
				uint16x8_t g16 = vmlaq_u16(add127, vmovl_u8(g_lo), a16);
				uint16x8_t b16 = vmlaq_u16(add127, vmovl_u8(b_lo), a16);
				r_lo = vqmovn_u16(vshrq_n_u16(r16, 8));
				g_lo = vqmovn_u16(vshrq_n_u16(g16, 8));
				b_lo = vqmovn_u16(vshrq_n_u16(b16, 8));
			}
			// high 8 像素
			{
				uint16x8_t a16 = vmovl_u8(a_hi);
				uint16x8_t r16 = vmlaq_u16(add127, vmovl_u8(r_hi), a16);
				uint16x8_t g16 = vmlaq_u16(add127, vmovl_u8(g_hi), a16);
				uint16x8_t b16 = vmlaq_u16(add127, vmovl_u8(b_hi), a16);
				r_hi = vqmovn_u16(vshrq_n_u16(r16, 8));
				g_hi = vqmovn_u16(vshrq_n_u16(g16, 8));
				b_hi = vqmovn_u16(vshrq_n_u16(b16, 8));
			}

			// 合并回 16 个 u8
			rgba.val[0] = vcombine_u8(r_lo, r_hi);
			rgba.val[1] = vcombine_u8(g_lo, g_hi);
			rgba.val[2] = vcombine_u8(b_lo, b_hi);
			// rgba.val[3] = 原始 alpha，不变

			// 交织存回: [R G B A]*16 -> 连续 RGBA 像素
			vst4q_u8(p, rgba);
		}
#endif
		// 处理剩余不足 16 个的尾巴
		for (; i < count; i++) {
			uint8_t* p = data + i * 4;
			uint8_t a = p[3];
			if (a == 0) {
				// 完全透明，RGB 直接清零
				p[0] = p[1] = p[2] = 0;
			} else if (a != 255) {
				p[0] = premulTable[a][p[0]];
				p[1] = premulTable[a][p[1]];
				p[2] = premulTable[a][p[2]];
			}
			// a == 255 不变
		}
	}

	// Luminance_Alpha_88 (2 字节每像素): [L, A]
	static void premultiplyLA88(uint8_t* data, uint32_t count) {
		initPremulTable();
		uint32_t i = 0;
#if Qk_ARM_NEON
		const uint16x8_t add127 = vdupq_n_u16(127);

		// 每次处理 16 个像素（32 字节）
		for (; i + 16 <= count; i += 16) {
			uint8_t* p = data + i * 2;
			uint8x16x2_t la = vld2q_u8(p);
			// la.val[0] = L
			// la.val[1] = A

			uint8x16_t l = la.val[0];
			uint8x16_t a = la.val[1];

			// 拆低/高 8 像素
			uint8x8_t l_lo = vget_low_u8(l);
			uint8x8_t l_hi = vget_high_u8(l);
			uint8x8_t a_lo = vget_low_u8(a);
			uint8x8_t a_hi = vget_high_u8(a);

			// ---- 低 8 像素 ----
			{
				uint16x8_t a16 = vmovl_u8(a_lo);
				uint16x8_t l16 = vmlaq_u16(add127, vmovl_u8(l_lo), a16);
				l_lo = vqmovn_u16(vshrq_n_u16(l16, 8));
			}
			// ---- 高 8 像素 ----
			{
				uint16x8_t a16 = vmovl_u8(a_hi);
				uint16x8_t l16 = vmlaq_u16(add127, vmovl_u8(l_hi), a16);
				l_hi = vqmovn_u16(vshrq_n_u16(l16, 8));
			}

			la.val[0] = vcombine_u8(l_lo, l_hi); // 替换 L 通道
			// alpha 不变
			vst2q_u8(p, la);
		}
#endif
		// 标量处理尾部
		for (; i < count; i++) {
			uint8_t* p = data + i * 2;
			uint8_t a = p[1];
			if (a == 0) {
				p[0] = 0;
			} else if (a != 255) {
				p[0] = premulTable[a][p[0]];
			}
			// a == 255 不变
		}
	}

	bool ImageSource::toPremultipliedAlpha(Pixel &pixel) {
		if (pixel.alphaType() != kUnpremul_AlphaType) {
			return false;
		}
		if (pixel.type() == kRGBA_8888_ColorType) {
			premultiplyRGBA8888(pixel.val(), pixel.width() * pixel.height());
		} else if (pixel.type() == kLuminance_Alpha_88_ColorType) {
			premultiplyLA88(pixel.val(), pixel.width() * pixel.height());
		} else {
			return false;
		}
		pixel._alphaType = kPremul_AlphaType; // mark as premultiplied
		return true;
	}

	void ImageSource::convertToPremultipliedAlpha(Array<Pixel> &pixels) {
		for (auto &pix: pixels) {
			toPremultipliedAlpha(pix);
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
				int ch = int(info.bytes()) - int(it->second.bytes);
				if (ch != 0) {
					_capacity += ch; // change
					it->second.bytes = info.bytes();
					it->second.time = time_millisecond();
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
			it.second.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
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
			return it->second.source.get();
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
			it->second.source->Qk_Off(State, &ImageSourcePool::handleSourceState, this);
			_sources.erase(it);
			_capacity -= it->second.bytes;
		}
	}

	void ImageSourcePool::clear(bool all) {
		if (is_process_exit()) {
			// avoid call when process exit
			// because render resource maybe already destroyed
			return;
		}
		AutoMutexExclusive local(_Mutex);
		if (all) {
			for (auto &i: _sources) {
				if (i.second.source->state() & (
						ImageSource::kSTATE_LOADING | ImageSource::kSTATE_LOAD_COMPLETE
					)
				) {
					i.second.source->unload();
					_capacity -= i.second.bytes;
					i.second.bytes = 0;
					i.second.time = 0;
				}
			}
		} else {
			// sort by time asc and size desc
		}
	}

	static std::mutex _si_mutex;
	static ImagePool* _shared_imagePool = nullptr;

	ImagePool* ImageSourcePool::shared() {
		if (!_shared_imagePool) {
			ScopeLock scope(_si_mutex);
			if (!_shared_imagePool) {
				_shared_imagePool = new ImagePool(first_loop());
			}
		}
		return _shared_imagePool;
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

	bool ImageSourceHold::set_src(String value) {
		auto pool = imgPool();
		if (pool) {
			return set_source(pool->get(value));
		} else {
			return set_source(*ImageSource::Make(value));
		}
	}

	bool ImageSourceHold::set_source(Sp<ImageSource> source) {
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
			return true;
		}
		return false;
	}

	void ImageSourceHold::handleSourceState(Event<ImageSource, ImageSource::State>& evt) {
		onSourceState(evt.data());
	}

	void ImageSourceHold::onSourceState(ImageSource::State state) {
		if (state & ImageSource::kSTATE_LOAD_COMPLETE) {}
	}

	ImagePool* ImageSourceHold::imgPool() {
		return shared_imgPool();
	}

}
