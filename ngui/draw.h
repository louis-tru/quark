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

#ifndef __ngui__draw__
#define __ngui__draw__

#include "base/array.h"
#include "base/util.h"
#include "base/string.h"
#include "base/event.h"
#include "image-codec.h"
#include "mathe.h"
#include "value.h"
#include "app.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

class Texture;
class TextureYUV;
class Font;
class FontGlyph;
class FontPool;
class TexturePool;
// view
class Box;
class BoxShadow;
class Image;
class Scroll;
class Video;
class Root;
class Sprite;
class Text;
class TextNode;
class Label;
class Input;
class Textarea;

/**
 * @enum DrawLibrary
 */
enum DrawLibrary {
  DRAW_LIBRARY_INVALID,
  DRAW_LIBRARY_GLES2,
  DRAW_LIBRARY_GLES3,
  DRAW_LIBRARY_GL3,
  DRAW_LIBRARY_GL4,
};

/**
 * @class DrawData
 */
class XX_EXPORT DrawData {
 public:
  typedef NonObjectTraits Traits;
  virtual ~DrawData() {}
};

/**
 * @class Draw
 */
class XX_EXPORT Draw: public Object {
  XX_HIDDEN_ALL_COPY(Draw);
 public:
  
  /**
   * @constructor
   * @arg options {Map} { multisample: 0-4 }
   */
  Draw(GUIApplication* host, const Map<String, int>& options);
  
  /**
   * @destructor
   */
  virtual ~Draw();
  
  /**
   * @event onsurface_size_change 绘图表面尺寸发生变化时触发
   */
  XX_EVENT(onsurface_size_change);
  
  inline GUIApplication* host() const { return m_host; }
  inline DrawLibrary library() { return m_library; }
  inline Vec2 surface_size() const { return m_surface_size; }
  inline CGRect selected_region() const { return m_selected_region; }
  bool set_surface_size(Vec2 surface_size, CGRect* select_region = nullptr);
  inline float best_display_scale() const { return m_best_display_scale; }
  inline uint multisample() const { return m_multisample; }
  inline void set_best_display_scale(float value) { m_best_display_scale = value; }
  inline Texture* empty_texture() { return m_empty_texture; }
  inline FontPool* font_pool() const { return m_font_pool; }
  inline TexturePool* tex_pool() const { return m_tex_pool; }
  inline static Draw* current() { return m_draw_ctx; }
  virtual void clear(bool full = false);
  
  /**
   * @func max_texture_memory_limit()
   */
  inline uint64 max_texture_memory_limit() const {
    return m_max_texture_memory_limit;
  }
  
  /**
   * @func set_texture_memory_limit(limit) 设置纹理内存限制，不能小于64MB，默认为512MB.
   */
  void set_max_texture_memory_limit(uint64 limit);
  
  /**
   * @func used_memory() 当前纹理数据使用的内存数量
   */
  uint64 used_texture_memory() const;
  
  /**
   * @func adjust_texture_memory()
   */
  bool adjust_texture_memory(uint64 will_alloc_size);
  
  // ----------------------------------------
  
  virtual void refresh_buffer() = 0;
  virtual void refresh_root_matrix(const Mat4& root, const Mat4& query) = 0;
  virtual void refresh_font_pool(FontPool* pool) = 0;
  virtual void begin_render() = 0;
  virtual void commit_render() = 0;
  virtual void begin_screen_occlusion_query() = 0;
  virtual void end_screen_occlusion_query() = 0;
  virtual uint set_texture(const Array<PixelData>& data) = 0;
  virtual void del_texture(uint id) = 0;
  virtual void del_buffer(uint id) = 0;
  virtual uint gen_texture(uint origin_texture, uint width, uint height) = 0;
  virtual void use_texture(uint id, Repeat repeat, uint slot = 0) = 0;
  virtual void use_texture(uint id, uint slot = 0) = 0;
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

  GUIApplication*     m_host;
  uint                m_multisample;      /* 是否启用多重采样 default false */
  Vec2                m_surface_size;     /* 当前绘图表面支持的大小 */
  CGRect              m_selected_region;  /* 选择绘图表面有区域 */
  Texture*            m_empty_texture;
  FontPool*           m_font_pool;        /* 字体纹理池 */
  TexturePool*        m_tex_pool;         /* 文件纹理池 */
  uint64              m_max_texture_memory_limit;
  float               m_best_display_scale;
  DrawLibrary         m_library;
  static Draw*        m_draw_ctx;
  
  friend Draw*        draw_ctx();
  friend FontPool*    font_pool();
  friend TexturePool* tex_pool();
  friend class Texture;
  friend class TextureYUV;
};

XX_INLINE Draw* draw_ctx() {
  return Draw::m_draw_ctx;
}
XX_INLINE FontPool* font_pool() {
  return Draw::m_draw_ctx->m_font_pool;
}
XX_INLINE TexturePool* tex_pool() {
  return Draw::m_draw_ctx->m_tex_pool;
}

XX_END
#endif
