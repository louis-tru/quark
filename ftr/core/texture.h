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

#ifndef __ftr__texture__
#define __ftr__texture__

#include "./util/event.h"
#include "./util/string.h"
#include "./util/handle.h"
#include "./util/array.h"
#include "./codec/codec.h"
#include "./value.h"

namespace ftr {

	class Draw;
	class FileTexture;
	class TexturePool;

	/**
	* @enum TextureStatus 纹理的状态标识
	*/
	enum TextureStatus {
		TEXTURE_NO_LOADED = 0,
		TEXTURE_HARDWARE_MIPMAP = (1 << 0),
		TEXTURE_LOADING = (1 << 1),
		TEXTURE_ERROR = (1 << 2),
		TEXTURE_COMPLETE = (1 << 3),
		TEXTURE_CHANGE_LOADING = TEXTURE_LOADING,
		TEXTURE_CHANGE_ERROR = TEXTURE_ERROR,
		TEXTURE_CHANGE_COMPLETE = TEXTURE_COMPLETE,
		TEXTURE_CHANGE_RELOADED = (1 << 4),
		TEXTURE_CHANGE_OK = (TEXTURE_CHANGE_COMPLETE | TEXTURE_CHANGE_RELOADED),
		TEXTURE_CHANGE_LEVEL_0 = (1 << 5),
		TEXTURE_CHANGE_LEVEL_1 = (1 << 6),
		TEXTURE_CHANGE_LEVEL_2 = (1 << 7),
		TEXTURE_CHANGE_LEVEL_3 = (1 << 8),
		TEXTURE_CHANGE_LEVEL_4 = (1 << 9),
		TEXTURE_CHANGE_LEVEL_5 = (1 << 10),
		TEXTURE_CHANGE_LEVEL_6 = (1 << 11),
		TEXTURE_CHANGE_LEVEL_7 = (1 << 12),
		TEXTURE_CHANGE_LEVEL_MASK = (0xFF << 5),
	};

	/**
	* @class Texture
	*/
	class FX_EXPORT Texture: public Reference {
		FX_HIDDEN_ALL_COPY(Texture);
		public:

		typedef PixelData::Format PixelFormat;

		enum Level {
			LEVEL_0 = 0, // raw image
			LEVEL_1,
			LEVEL_2,
			LEVEL_3,
			LEVEL_4,
			LEVEL_5,
			LEVEL_6,
			LEVEL_7,
			LEVEL_NONE,
		};

		/**
		* @event onchange 纹理变化事件,比如尺寸了生了变化
		*/
		FX_Event(change, Event<int, Texture>);

		/**
		* @func create() 通过图像数据创建一个新的纹理对像,如果成功返回纹理对像
		*/
		static Texture* create(cPixelData& data);

		/**
		* @func create() 通过mipmap图像数据创建一个新的纹理对像,如果成功返回纹理对像
		*/
		static Texture* create(const Array<PixelData>& data);

		/**
		* @func get_texture_level()
		*/
		static Level get_texture_level(uint32_t ratio);

		/**
		* @func get_texture_level_from_convex_quadrilateral(vertex)
		*/
		Level get_texture_level_from_convex_quadrilateral(Vec2 vertex[4]);

		/**
		* @destructor
		*/
		virtual ~Texture();

		/**
		* @func id() 纹理id
		*/
		virtual String id() const;

		/**
		* @func load() 载入纹理数据到GPU,载入成功后触发change事件.
		*/
		virtual void load(Level level = LEVEL_NONE) {}
		virtual bool unload(Level level = LEVEL_NONE) { return false; }

		/**
		* @func use()  绑定纹理到指定槽,成功返回true,否则返回false并调用load尝试加载纹理到GPU
		*/
		bool use(uint32_t slot = 0,
						Level level = LEVEL_0,
						Repeat repeat = Repeat::NONE);
		inline int status() const { return _status; }
		inline bool is_available() const { return _width != 0; }
		inline const uint32_t* handle() const { return _handle; }
		inline const uint32_t* data_size() const { return _data_size; }
		inline const uint32_t* use_count() const { return _use_count; }
		inline const Repeat* repeat() const { return _repeat; }
		inline int width() const { return _width; }
		inline int height() const { return _height; }
		inline int diagonal() const { return _diagonal; }
		inline PixelFormat format() const { return _format; }

		protected:
		/**
		* @constructor
		*/
		Texture();

		/**
		* @func load_data() 通过像素数据载入纹理到GPU,如果成功返回true
		*/
		bool load_data(cPixelData& data);

		int   _status;
		uint32_t  _handle[8];
		uint32_t  _data_size[8];
		uint32_t  _use_count[8];
		Repeat _repeat[8];
		int   _width;
		int   _height;
		uint32_t  _diagonal;
		PixelFormat _format;

		friend class GLDraw;
		FX_DEFINE_INLINE_CLASS(Inl);
	};

	class FX_EXPORT TextureYUV: public Texture {
		public:
		bool load_yuv(cPixelData& data);
		virtual bool unload(Level level = LEVEL_NONE);
	};

	/**
	* @class FileTexture
	*/
	class FX_EXPORT FileTexture: public Texture {
		public:
		typedef ImageCodec::ImageFormat ImageFormat;
		
		/**
		* @destructor
		*/
		virtual ~FileTexture();
		
		/**
		* @overwrite
		*/
		virtual String id() const;
		virtual void load(Level level = LEVEL_NONE);
		virtual bool unload(Level level = LEVEL_NONE);
		
		/**
		* @func image_format 返回纹理的原始路径中的格式
		*/
		inline ImageFormat image_format() const {
			return _image_format;
		}
		
		private:
		
		FileTexture(cString& path);
		
		String        _path;
		uint32_t          _load_id;
		ImageFormat   _image_format;
		TexturePool*  _pool;
		
		friend class TexturePool;
	};

	/**
	* @struct TexturePoolEventData 纹理池事件数据
	*/
	struct TexturePoolEventData {
		float progress;
		FileTexture*  texture;
	};

	typedef Event<TexturePoolEventData, TexturePool> TexturePoolEvent;

	/**
	* @class TexturePool 统一管理纹理数据的池
	*/
	class FX_EXPORT TexturePool: public Object {
		FX_HIDDEN_ALL_COPY(TexturePool);
		public:
		
		/**
		* @event onchange 纹理载入变化事件
		*/
		FX_Event(change, TexturePoolEvent);
		
		/**
		* @constructor
		*/
		TexturePool(Draw* ctx);
		
		/**
		* @destructor
		*/
		virtual ~TexturePool();
		
		/**
		* @func get_texture 通过本地或网络路径获取纹理对像
		*/
		FileTexture* get_texture(cString& path);
		
		/**
		* @func load 通过路径列表批量加载纹理
		*/
		void load(const Array<String>& paths);
		
		/**
		* @func load 载入当前池中所有未载入的纹理对像
		*/
		void load_all();
		
		/**
		* @func progress 当前载入完成的百分比
		*/
		float progress() const;
		
		/**
		* @func clear
		*/
		void clear(bool full = false);
		
		private:
		
		Draw* _draw_ctx;
		Dict<String, FileTexture*> _textures;
		Dict<Texture*, bool> _completes;
		uint64_t _total_data_size; /* 纹池当前数据总量 */
		
		FX_DEFINE_INLINE_CLASS(Inl)
		
		friend class Draw;
	};

}
#endif
