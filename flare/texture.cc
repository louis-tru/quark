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

#include "./app.h"
#include "./texture.h"
#include "./util/fs.h"
#include "./util/array.h"
#include "./display.h"
#include <math.h>
// #include "./render/render.h"

namespace flare {

	typedef PixelData::Format PixelFormat;

	static const Texture::Level levels_table[] {
		Texture::LEVEL_0, // 0
		Texture::LEVEL_0, // 1
		Texture::LEVEL_1, // 2
		Texture::LEVEL_1, // 3
		Texture::LEVEL_2, // 4
		Texture::LEVEL_2, // 5
		Texture::LEVEL_2, // 6
		Texture::LEVEL_2, // 7
		Texture::LEVEL_3, // 8
		Texture::LEVEL_3, // 9
		Texture::LEVEL_3, // 10
		Texture::LEVEL_3, // 11
		Texture::LEVEL_3, // 12
		Texture::LEVEL_3, // 13
		Texture::LEVEL_3, // 14
		Texture::LEVEL_3, // 15
		Texture::LEVEL_4, // 16
		Texture::LEVEL_4, // 17
		Texture::LEVEL_4, // 18
		Texture::LEVEL_4, // 19
		Texture::LEVEL_4, // 20
		Texture::LEVEL_4, // 21
		Texture::LEVEL_4, // 22
		Texture::LEVEL_4, // 23
		Texture::LEVEL_4, // 24
		Texture::LEVEL_4, // 25
		Texture::LEVEL_4, // 26
		Texture::LEVEL_4, // 27
		Texture::LEVEL_4, // 28
		Texture::LEVEL_4, // 29
		Texture::LEVEL_4, // 30
		Texture::LEVEL_4, // 31
		Texture::LEVEL_5, // 32
		Texture::LEVEL_5, // 33
		Texture::LEVEL_5, // 34
		Texture::LEVEL_5, // 35
		Texture::LEVEL_5, // 36
		Texture::LEVEL_5, // 37
		Texture::LEVEL_5, // 38
		Texture::LEVEL_5, // 39
		Texture::LEVEL_5, // 40
		Texture::LEVEL_5, // 41
		Texture::LEVEL_5, // 42
		Texture::LEVEL_5, // 43
		Texture::LEVEL_5, // 44
		Texture::LEVEL_5, // 45
		Texture::LEVEL_5, // 46
		Texture::LEVEL_5, // 47
		Texture::LEVEL_5, // 48
		Texture::LEVEL_5, // 49
		Texture::LEVEL_5, // 50
		Texture::LEVEL_5, // 51
		Texture::LEVEL_5, // 52
		Texture::LEVEL_5, // 53
		Texture::LEVEL_5, // 54
		Texture::LEVEL_5, // 55
		Texture::LEVEL_5, // 56
		Texture::LEVEL_5, // 57
		Texture::LEVEL_5, // 58
		Texture::LEVEL_5, // 59
		Texture::LEVEL_5, // 60
		Texture::LEVEL_5, // 61
		Texture::LEVEL_5, // 62
		Texture::LEVEL_5, // 63
		Texture::LEVEL_6, // 64
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6,
		Texture::LEVEL_6, // 127
		Texture::LEVEL_7, // 128
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7,
		Texture::LEVEL_7, // 255
	};

	inline static void set_texture_total_data_size(TexturePool* pool, int size);

	inline static bool is_valid_texture(uint32_t handle) {
		return handle && handle < Uint32::limit_max;
	}

	F_DEFINE_INLINE_MEMBERS(Texture, Inl) {
	 public:

		/**
		 * @func load_mipmap_data 通过像素数据载入mipmap纹理到GPU,如果成功返回true
		 */
		bool load_mipmap_data(const Array<PixelData>& mipmap_data) {
			// TODO ...
			// Render* ctx = _host->render();
			// if (!ctx) return;
			uint32_t size = 0;
			uint32_t size_pixel = PixelData::get_pixel_data_size(mipmap_data[0].format());

			F_ASSERT_STRICT_RENDER_THREAD();

			for (uint32_t i = 0; i < mipmap_data.length(); i++) {
				auto data = mipmap_data[i];
				size += data.width() * data.height() * size_pixel;
			}
			
			if (_host->adjust_texture_memory(size)) {
				
				// TODO ...
				// uint32_t handle = ctx->set_texture(mipmap_data);
				// if (handle) {
				// 	if (_handle[0]) {
				// 		ctx->del_texture(_handle[0]);
				// 	}
				// 	cPixelData& pixel = mipmap_data[0];
				// 	_width  = pixel.width();
				// 	_height = pixel.height();
				// 	_format = pixel.format();
				// 	_handle[0] = handle;
				// 	_data_size[0] = size;
				// 	_use_count[0] = 1;
				// 	_repeat[0] = Repeat::NONE;
				// 	set_texture_total_data_size(_host->tex_pool(), size);
					
				// 	if (mipmap_data.size() > 1) {
				// 		_status |= TEXTURE_HARDWARE_MIPMAP;
				// 	} else {
				// 		_status &= ~TEXTURE_HARDWARE_MIPMAP;
				// 	}
				// 	return true;
				// }
			}
			return false;
		}
		
		void generate_texture() {
			// TODO ...
			// Render* ctx = _host->render();
			// if (!ctx) return;
			F_ASSERT_STRICT_RENDER_THREAD();
			
			_status |= TEXTURE_LOADING;
			int status = TEXTURE_NO_LOADED;
			
			if (_status & TEXTURE_HARDWARE_MIPMAP) {
				status |= (_status & TEXTURE_CHANGE_LEVEL_MASK);
			} else {
				if (_status & (TEXTURE_CHANGE_LEVEL_MASK & ~TEXTURE_CHANGE_LEVEL_0)) {
					int mark = TEXTURE_CHANGE_LEVEL_1;
					uint32_t prev_level_handle = _handle[0];
					uint32_t width = _width / 2;
					uint32_t height = _height / 2;
					
					for (int i = LEVEL_1; i < LEVEL_NONE; i++) {
						uint32_t handle = _handle[i];
						if (width >= 16 && height >= 16) { // 不生成16像素以下的纹理
							if (is_valid_texture(handle)) {
								prev_level_handle = handle;
							} else {
								uint32_t size = width * height * 4;
								if ((_status & mark) && prev_level_handle &&
										_host->adjust_texture_memory(size))
								{ // gen target level
									// TODO ...
									// handle = ctx->gen_texture(prev_level_handle, width, height);
									// if (handle) {
									// 	_handle[i] = handle;
									// 	_data_size[i] = size;
									// 	_repeat[i] = Repeat::NONE;
									// 	_use_count[i] = 1;
									// 	set_texture_total_data_size(ctx->tex_pool(), size);
									// 	prev_level_handle = handle;
									// 	status |= mark;
									// }
								}
							}
						} else {
							if (!handle)
								_handle[i] = Uint32::limit_max;
						}
						width /= 2; height /= 2;
						mark <<= 1;
					}
					// for end
				}
				status |= (_status & TEXTURE_CHANGE_LEVEL_0);
			}
			if (_status & TEXTURE_COMPLETE) { // reload
				status |= TEXTURE_CHANGE_RELOADED;
			} else { // COMPLETE
				status |= TEXTURE_COMPLETE;
				_status |= TEXTURE_COMPLETE;
			}
			_status &= ~(TEXTURE_CHANGE_LEVEL_MASK | TEXTURE_LOADING); // delete mark
			_host->main_loop()->post(Cb([this, status](CbData& e) {
				F_Trigger(Change, status);
			}, this));
		}

		void clear() {
			// auto ctx = draw_ctx();
			bool post_to_render = !_host->has_current_render_thread();
			Array<uint32_t> post_to_render_handles;

			for (int i = 0; i < 8; i++) {
				if (is_valid_texture(_handle[i])) {
					// TODO ...
					// if (ctx) {
					// if (post_to_render) {
					// 	post_to_render_handles.push(_handle[i]);
					// } else {
					// 	ctx->del_texture(_handle[i]); // 从GPU中删除纹理数据
					// }
					// set_texture_total_data_size(tex_pool(), -_data_size[i]);
					// }
					_handle[i] = 0;
					_data_size[i] = 0;
					_use_count[i] = 0;
					_repeat[i] = Repeat::NONE;
				}
			}

			if (post_to_render_handles.length()) {
				_host->render_loop()->post(Cb([post_to_render_handles](CbData& e) {
					// TODO ...
					// auto ctx = draw_ctx();
					// if (ctx) {
					// for (int i = 0; i < post_to_render_handles.length(); i++) {
						// ctx->del_texture(post_to_render_handles[i]);
					// }
					// }
				}));
			}

			_status &= ~TEXTURE_HARDWARE_MIPMAP;
		}
	};

	Texture* Texture::create(cPixelData& data) {
		Texture* tex = new Texture();
		if ( Inl_Texture(tex)->load_mipmap_data({ data }) ) {
			return tex;
		}
		tex->release();
		return nullptr;
	}

	Texture* Texture::create(const Array<PixelData>& data) {
		Texture* tex = new Texture();
		if ( Inl_Texture(tex)->load_mipmap_data(data) ) {
			return tex;
		}
		tex->release();
		return nullptr;
	}

	/**
	 * @func get_texture_level()
	 */
	Texture::Level Texture::get_texture_level(uint32_t ratio) {
		if (ratio > 255) {
			return LEVEL_7;
		} else {
			return levels_table[ratio];
		}
	}

	/**
	 * @func get_texture_level_from_convex_quadrilateral(vertex)
	 */
	Texture::Level Texture::get_texture_level_from_convex_quadrilateral(Vec2 vertex[4]) {
		if (_width) {
			auto dp = _host->display();
			if (!dp) {
				return Texture::LEVEL_0;
			}
			Vec2 scale = dp->scale();
			float scale_value = (scale.x() + scale.y()) / 2;
			float diagonal = (vertex[0].distance(vertex[2]) + vertex[1].distance(vertex[3])) / 2 * scale_value;
			return get_texture_level(floorf(_diagonal / F_MAX(diagonal, 16)));
		} else {
			return Texture::LEVEL_0;
		}
	}

	Texture::Texture()
		: F_Init_Event(Change)
		, _host(app())
		, _status(TEXTURE_NO_LOADED)
		, _width(0)
		, _height(0)
		, _diagonal(0)
		, _format(PixelData::INVALID)
	{
		memset(_handle, 0, sizeof(uint32_t) * 24 + sizeof(Repeat) * 8);
	}

	Texture::~Texture() {
		Inl_Texture(this)->clear();
	}

	String Texture::id() const {
		return String();
	}

	/**
	 * @func load_data 通过数据载入纹理到GPU,如果成功返回true
	 */
	bool Texture::load_data(cPixelData& data) {
		return Inl_Texture(this)->load_mipmap_data({ data });
	}

	bool Texture::use(uint32_t slot, Level level, Repeat repeat) {
		uint32_t handle = _handle[level];
		if (!is_valid_texture(handle)) {
			load(level);
			if (!is_valid_texture(handle = _handle[level])) {
				for (int i = level - 1; i >= 0; i--) {
					if (is_valid_texture(handle = _handle[i])) {
						level = (Level)i;
						goto use;
					}
				}
				for (int i = level + 1; i < LEVEL_NONE; i++) {
					if (is_valid_texture(handle = _handle[i])) {
						level = (Level)i;
						goto use;
					}
				}
				return false;
			}
		}
	 use:
	 // TODO ..
		if ( repeat == _repeat[level] ) {
			// draw_ctx()->use_texture(handle, slot);
		} else {
			// draw_ctx()->use_texture(handle, repeat, slot);
			_repeat[level] = repeat;
		}
		_use_count[level]++;
		return true;
	}

	bool TextureYUV::load_yuv(cPixelData& data) {
		// TODO ...
		// auto pool = tex_pool();
		// if (!pool) return false;
		auto pool = _host->tex_pool();
		int old_size = _data_size[0] + _data_size[1];
		set_texture_total_data_size(pool, -old_size);
		int size = data.width() * data.height();
		int new_size = size + size / 2;

		F_ASSERT_STRICT_RENDER_THREAD();

		if (_host->adjust_texture_memory(new_size)) {
			// TODO ...
			// if ( draw_ctx()->set_yuv_texture(this, data) ) {
			// 	_data_size[0] = size;
			// 	_data_size[1] = size / 2;
			// 	set_texture_total_data_size(pool, new_size);
				
			// 	if (_width != data.width() ||
			// 			_height != data.height() || _format != data.format()) {
			// 		_width = data.width();
			// 		_height = data.height();
			// 		_diagonal = Vec2(_width, _height).diagonal();
			// 		_format = data.format();
			// 		_status = TEXTURE_COMPLETE;
			// 		main_loop()->post(Cb([this](CbData& e) {
			// 			F_Trigger(Change, TEXTURE_CHANGE_RELOADED | TEXTURE_CHANGE_LEVEL_MASK);
			// 		}, this));
			// 	}
			// 	return true;
			// }
		}
		set_texture_total_data_size(pool, old_size);
		
		return false;
	}

	bool TextureYUV::unload(Level level) {
		Inl_Texture(this)->clear();
		return true;
	}

	FileTexture::FileTexture(cString& path)
		: _path(fs_reader()->format(path))
		, _load_id(0)
		, _image_format(ImageCodec::get_image_format(_path))
		, _pool(nullptr) {
	}

	FileTexture::~FileTexture() {
		F_ASSERT(_pool == nullptr);
		F_ASSERT(_load_id == 0);
	}

	String FileTexture::id() const {
		return _path;
	}

	void FileTexture::load(Level level) {
		
		if (level == LEVEL_NONE) {
			if (_status & (TEXTURE_ERROR | TEXTURE_COMPLETE)) return;
		} else {
			if (_handle[level]) return;
			if (_status & (TEXTURE_ERROR | TEXTURE_HARDWARE_MIPMAP)) return;
			int mark = TEXTURE_CHANGE_LEVEL_0 << level;
			int i = level;
			_status |= mark;
			while (i) {
				i--; mark >>= 1;
				if (is_valid_texture(_handle[i]) || (_status & mark)) break;
			}
			if (mark == TEXTURE_CHANGE_LEVEL_0 && !_handle[0]) {
				_status |= TEXTURE_CHANGE_LEVEL_0;
			}
		}
		
		if (_status & TEXTURE_LOADING) return;
		
		#define LoaderTextureError(err) { \
			_status = TEXTURE_ERROR;  \
			F_ERR(TEXTURE, err, *_path); \
			_host->main_loop()->post(Cb([this](CbData& e) { \
				F_Trigger(Change, TEXTURE_CHANGE_ERROR); \
			}, this)); \
		}
		
		if (!(_status & TEXTURE_CHANGE_LEVEL_0) && (_status & TEXTURE_COMPLETE)) {
			Inl_Texture(this)->generate_texture();
			return;
		}
		
		_status |= TEXTURE_LOADING;
		
		struct DecodeContext {
			typedef NonObjectTraits Traits;
			Buffer input;
			Array<PixelData> output;
		};
		
		// load level 0
		_load_id = fs_reader()->read_file(_path, Cb([this](CbData& d) {
			UILock lock;
			_load_id = 0;
			
			if (d.error) {
				LoaderTextureError("Error: Error reading the image file, %s");
			}
			else if (_status & TEXTURE_LOADING) {
				// 如果不等于`TEXTURE_STATUS_LOADING`表示已经被取消所以无需载入了
				ImageCodec* parser = ImageCodec::shared(_image_format);
				if (!parser) {
					LoaderTextureError("Error: File format is not supported, %s");
					return;
				}
				auto app = Application::shared();
				if (!app) {
					_status &= ~TEXTURE_LOADING;
					F_WARN(TEXTURE, "Unable to load the texture %s, need to initialize first Application", *_path);
					return;
				}
				
				auto ctx = new DecodeContext();
				ctx->input = *static_cast<Buffer*>(d.data);
				
				Cb complete([this, ctx](CbData& e) { // 完成
					Handle<DecodeContext> handle(ctx);
					if (!ctx->output.length()) {
						LoaderTextureError("Error: Loader image file error, %s");
						return;
					}
					if (ctx->output[0].body_count() &&
							ctx->output[0].body().length()) {
						if (!Inl_Texture(this)->load_mipmap_data(ctx->output)) {
							LoaderTextureError("Error: Loader image file error, %s");
							return;
						}
					} else {
						_width = ctx->output[0].width();
						_height = ctx->output[0].height();
						_diagonal = Vec2(_width, _height).diagonal();
						_format = ctx->output[0].format();
					}
					Inl_Texture(this)->generate_texture();
				});
				
				if (_status & TEXTURE_CHANGE_LEVEL_0) {
					// 解码需要时间,发送到工作线程执行解码操作
					app->render_loop()->work(Cb([this, ctx, parser](CbData& e) {
						ctx->output = parser->decode(ctx->input);
					}, this), complete);
				} else {
					PixelData pd = parser->decode_header(ctx->input);
					if (pd.width()) {
						ctx->output = Array<PixelData>({ pd });
					}
					complete->resolve();
				}
			}
		}, this));
		
		#undef LoaderTextureError
	}

	bool FileTexture::unload(Level level) {
		F_ASSERT_STRICT_RENDER_THREAD();

		if (level == LEVEL_NONE) { // unload all
			if (_status & TEXTURE_LOADING) {
				_status &= ~TEXTURE_LOADING;
				if (_load_id) { // 取消正在载入的纹理,比如正在从http服务器下载纹理数据...
					fs_reader()->abort(_load_id);
					_load_id = 0;
				}
			}
			auto ctx = _host->render();// draw_ctx();
			for (int i = 0; i < LEVEL_NONE; i++) {
				if (is_valid_texture(_handle[i])) {
					// TODO ...
					// if (ctx) {
					// 	ctx->del_texture(_handle[i]); // 从GPU中删除纹理数据
					// 	set_texture_total_data_size(_host->tex_pool(), -_data_size[i]);
					// }
				}
				_handle[i] = 0;
				_repeat[i] = Repeat::NONE;
				_data_size[i] = 0;
				_use_count[i] = 0;
			}
			_status &= ~(TEXTURE_CHANGE_LEVEL_MASK | TEXTURE_HARDWARE_MIPMAP); // delete mark
		} else { // unload level
			if (is_valid_texture(_handle[level])) {
				// TODO ...
				// auto ctx = draw_ctx();
				// if (ctx) {
				// 	ctx->del_texture(_handle[level]);
				// 	set_texture_total_data_size(_host->tex_pool(), -_data_size[level]);
				// }
				if (level == LEVEL_0)
					_status &= ~TEXTURE_HARDWARE_MIPMAP;
			}
			_handle[level] = 0;
			_repeat[level] = Repeat::NONE;
			_data_size[level] = 0;
			_use_count[level] = 0;
			_status &= ~(TEXTURE_CHANGE_LEVEL_0 << level); // delete mark
		}
		return true;
	}

	F_DEFINE_INLINE_MEMBERS(TexturePool, Inl) {
	 public:
		#define _inl_pool(self) static_cast<TexturePool::Inl*>(self)
		
		void texture_change_handle(Event<Texture, int>& evt) {
			int status = *evt.data();
			if (status & TEXTURE_CHANGE_COMPLETE) {
				UILock lock;
				_completes.set(evt.sender(), 1); // 完成后加入完成列表
				auto sender = static_cast<FileTexture*>(evt.sender());
				TexturePoolEventData data = { progress(), sender };
				F_Trigger(Change, data);
			}
		}
		
		void add_texture_for_pool(FileTexture* tex, cString& name) {
			F_ASSERT(!tex->_pool);
			F_ASSERT(!_textures[name]);
			_textures[name] = tex;
			tex->retain();
			tex->_pool = this;
		}
		
		void del_texture_for_pool(FileTexture* tex) {
			F_ASSERT(tex->_pool == this);
			F_ASSERT(tex->ref_count() > 0);
			tex->_pool = nullptr;
			_completes.erase(tex);
			tex->release();
		}
		
		void trigger_change() {
			_host->main_loop()->post(Cb([this](CbData& e) {
				TexturePoolEventData data = { progress(), nullptr };
				F_Trigger(Change, data);
			}));
		}
		
		void set_texture_total_data_size(int size) {
			_total_data_size += size;
			// F_DEBUG("texture_total_data_size: %d", _total_data_size);
			F_ASSERT(_total_data_size >= 0);
		}
	};

	static void set_texture_total_data_size(TexturePool* pool, int size) {
		F_ASSERT(pool);
		_inl_pool(pool)->set_texture_total_data_size(size);
	}

	TexturePool::TexturePool(Application* host)
		: F_Init_Event(Change)
		, _host(host)
		, _total_data_size(0)
	{
		F_ASSERT(host); // "Did not find GLDraw"
	}

	TexturePool::~TexturePool() {
		for ( auto& i : _textures ) {
			auto tex = i.value;
			tex->_pool = nullptr;
			F_ASSERT( tex->ref_count() > 0 );
			// if ( tex->ref_count() == 1 ) {
			tex->unload();
			Release(tex);
			// }
		}
	}

	FileTexture* TexturePool::get_texture(cString& path) {
		String pathname = fs_reader()->format(path);
		
		// 通过路径查找纹理对像
		auto it = _textures.find(pathname);
		if ( it != _textures.end() ) {
			return it->value;
		}
		
		// 在当前池中创建一个纹理
		FileTexture* texture = new FileTexture(pathname);
		_inl_pool(this)->add_texture_for_pool(texture, pathname);
		
		texture->F_On(Change, &Inl::texture_change_handle, _inl_pool(this));
		
		return texture;
	}

	void TexturePool::load(const Array<String>& paths) {
		for ( int i = 0; i < paths.length(); i++ ) {
			get_texture(paths[i])->load();
		}
	}

	void TexturePool::load_all() {
		for (auto& i : _textures) {
			i.value->load();
		}
	}

	float TexturePool::progress() const {
		if ( _textures.length() > 0 ) {
			return _completes.length() / float(_textures.length());
		} else {
			return 1.0f;
		}
	}

	void TexturePool::clear(bool full) {
		F_ASSERT_STRICT_RENDER_THREAD();

		bool del_mark = false;
		auto it = _textures.begin(), end = _textures.end();
		
		if (full) {
			while ( it != end ) {
				FileTexture* texture = it->value;
				texture->unload();
				F_ASSERT(texture->ref_count() > 0);
				if ( texture->ref_count() == 1 ) { // 不需要使用的纹理可以删除
					_inl_pool(this)->del_texture_for_pool(texture);
					it->value = nullptr;
					_textures.erase(it++);
					del_mark = true;
				} else {
					it++;
				}
			}
		} else {
			struct TexWrap {
				FileTexture* tex;
				Texture::Level level;
				uint32_t use_count;
			};
			List<TexWrap> texture_sort;
			uint64_t total_data_size = 0;
			
			// 先按使用使用次数排序纹理对像
			while ( it != end ) {
				FileTexture* tex = it->value;
				F_ASSERT( tex->ref_count() > 0 );
				
				if ( tex->ref_count() == 1 ) { // 不需要使用的纹理可以删除
					tex->unload();
					_inl_pool(this)->del_texture_for_pool(tex);
					_textures.erase(it++);
					del_mark = true;
					memset(tex->_use_count, 0, sizeof(uint32_t) * 8); // 置零纹理使用次数
				} else {
					for (int k = 0; k < Texture::LEVEL_NONE; k++) {
						auto level = Texture::Level(k);
						if (is_valid_texture(tex->_handle[k])) {
							auto j = texture_sort.begin(), end = texture_sort.end();
							uint32_t use_count = tex->_use_count[k];
							while ( j != end ) {
								if ( use_count <= j->use_count ) {
									end = j; break;
								}
								j++;
							}
							texture_sort.push_back({ tex, level, use_count });
							total_data_size += tex->_data_size[k];
							tex->_use_count[k] /= 2;
						}
					}
				}
			}
			
			if ( texture_sort.length() ) {
				uint64_t total_data_size_1_3 = total_data_size / 3;
				uint64_t del_data_size = 0;
				// 从排序列表顶部开始卸载总容量的1/3
				for ( auto tex : texture_sort ) {
					if (del_data_size < total_data_size_1_3) {
						del_data_size += tex.tex->_data_size[tex.level];
						tex.tex->unload(tex.level);
					} else {
						break;
					}
				}
				F_DEBUG("TEXTURE", "Texture memory clear, %ld", del_data_size);
			}
		}
		
		if (del_mark) {
			_inl_pool(this)->trigger_change();
		}
	}

	// TODO ...

	static Char empty_[4] = { 0, 0, 0, 0 };
	static cPixelData empty_pixel_data(WeakBuffer(empty_, 4), 1, 1, PixelData::RGBA8888);

	/**
	* @class TextureEmpty
	*/
	class TextureEmpty: public Texture {
	 public:
		virtual void load() {
			if (_status == TEXTURE_NO_LOADED) {
				F_ASSERT(load_data(empty_pixel_data), "Load temp texture error");
			}
		}
	};

}
