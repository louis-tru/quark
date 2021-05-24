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

#ifndef __flare__render__
#define __flare__render__

#include "../util/util.h"
#include "../util/string.h"
#include "../util/event.h"
#include "../codec/codec.h"
#include "../math/math.h"
#include "../value.h"
#include "../app.h"
#include "../view/view.h"

namespace flare {

	class Texture;
	class TextureYUV;
	class Font;
	class FontGlyph;
	class FontPool;
	class TexturePool;

	/**
	* @enum RenderLibrary
	*/
	enum RenderLibrary {
		RENDER_LIBRARY_INVALID,
		RENDER_LIBRARY_GLES2,
		RENDER_LIBRARY_GLES3,
		RENDER_LIBRARY_GL3,
		RENDER_LIBRARY_GL4,
	};

	/**
	* @class RenderData
	*/
	class FX_EXPORT RenderData {
		public:
		typedef NonObjectTraits Traits;
		virtual ~RenderData() = default;
	};

	/**
	* @class Draw
	*/
	class FX_EXPORT ViewRender: public View::Visitor {
		FX_HIDDEN_ALL_COPY(ViewRender);
		public:
		
		/**
		* @constructor
		* @arg options {Map} { multisample: 0-4 }
		*/
		Draw(GUIApplication* host, cJSON& options);
		
		/**
		* @destructor
		*/
		virtual ~Draw();
		
		/**
		* @event surface_size_change_r 绘图表面尺寸发生变化时从渲染线程触发
		*/
		FX_Event(surface_size_change_r);
		
		inline GUIApplication* host() const { return _host; }
		inline DrawLibrary library() { return _library; }
		inline Vec2 surface_size() const { return _surface_size; }
		inline CGRect selected_region() const { return _selected_region; }
		bool set_surface_size(Vec2 surface_size, CGRect* select_region = nullptr);
		inline float best_display_scale() const { return _best_display_scale; }
		inline uint32_t multisample() const { return _multisample; }
		inline void set_best_display_scale(float value) { _best_display_scale = value; }
		inline Texture* empty_texture() { return _empty_texture; }
		inline FontPool* font_pool() const { return _font_pool; }
		inline TexturePool* tex_pool() const { return _tex_pool; }
		inline static Draw* current() { return _draw_ctx; }
		virtual void clear(bool full = false);
		
		/**
		* @func max_texture_memory_limit()
		*/
		inline uint64_t max_texture_memory_limit() const {
			return _max_texture_memory_limit;
		}
		
		/**
		* @func set_texture_memory_limit(limit) 设置纹理内存限制，不能小于64MB，默认为512MB.
		*/
		void set_max_texture_memory_limit(uint64_t limit);
		
		/**
		* @func used_memory() 当前纹理数据使用的内存数量
		*/
		uint64_t used_texture_memory() const;
		
		/**
		* @func adjust_texture_memory()
		*/
		bool adjust_texture_memory(uint64_t will_alloc_size);
		
		// ----------------------------------------
		
		virtual void refresh_buffer() = 0;
		virtual void refresh_root_matrix(const Mat4& root, const Mat4& query) = 0;
		virtual void refresh_font_pool(FontPool* pool) = 0;
		virtual void begin_render() = 0;
		virtual void commit_render() = 0;
		virtual void begin_screen_occlusion_query() = 0;
		virtual void end_screen_occlusion_query() = 0;
		virtual uint32_t set_texture(const Array<PixelData>& data) = 0;
		virtual void del_texture(uint32_t id) = 0;
		virtual void del_buffer(uint32_t id) = 0;
		virtual uint32_t gen_texture(uint32_t origin_texture, uint32_t width, uint32_t height) = 0;
		virtual void use_texture(uint32_t id, Repeat repeat, uint32_t slot = 0) = 0;
		virtual void use_texture(uint32_t id, uint32_t slot = 0) = 0;
		virtual bool set_yuv_texture(TextureYUV* yuv_tex, cPixelData& data) = 0;
		virtual bool set_font_glyph_vertex_data(Font* font, FontGlyph* glyph) = 0;
		virtual bool set_font_glyph_texture_data(Font* font, FontGlyph* glyph, int level) = 0;
		virtual void clear_color(Color color) = 0;
		
		// draw()
		virtual void draw(Root* v) = 0;
		virtual void draw(Video* v) = 0;
		virtual void draw(Image* v) = 0;
		virtual void draw(BoxShadow* v) = 0;
		virtual void draw(Box* v) = 0;
		virtual void draw(TextNode* v) = 0;
		virtual void draw(Label* v) = 0;
		virtual void draw(Text* v) = 0;
		virtual void draw(Sprite* v) = 0;
		virtual void draw(Scroll* v) = 0;
		virtual void draw(Input* v) = 0;
		virtual void draw(Textarea* v) = 0;
	
		protected:
		GUIApplication*     _host;
		uint32_t            _multisample;      /* 是否启用多重采样 default false */
		Vec2                _surface_size;     /* 当前绘图表面支持的大小 */
		CGRect              _selected_region;  /* 选择绘图表面有区域 */
		Texture*            _empty_texture;
		FontPool*           _font_pool;        /* 字体纹理池 */
		TexturePool*        _tex_pool;         /* 文件纹理池 */
		uint64_t            _max_texture_memory_limit;
		float               _best_display_scale;
		DrawLibrary         _library;
		static Draw*        _draw_ctx;
		
		friend Draw*        draw_ctx();
		friend FontPool*    font_pool();
		friend TexturePool* tex_pool();
		friend class Texture;
		friend class TextureYUV;
	};

	inline Draw* draw_ctx() {
		return Draw::_draw_ctx;
	}
	inline FontPool* font_pool() {
		return Draw::_draw_ctx->_font_pool;
	}
	inline TexturePool* tex_pool() {
		return Draw::_draw_ctx->_tex_pool;
	}

}
#endif
