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

#include "sprite.h"
#include "texture.h"
#include "draw.h"
#include "app.h"
#include "display-port.h"

XX_NS(ngui)

XX_DEFINE_INLINE_MEMBERS(Sprite, Inl) {
 public:
  
  /**
   * @func texture_change_handle
   */
  void texture_change_handle(Event<int, Texture>& evt) { // 收到图像变化通知
    GUILock lock;
    int status = *evt.data();
    if (status & TEXTURE_CHANGE_OK) {
      mark(M_TEXTURE); // 标记
    }
  }
  
};

Sprite::Sprite(Vec2 size)
: m_start()
, m_size(size)
, m_ratio(1,1)
, m_texture(draw_ctx()->empty_texture())
, m_tex_level(Texture::LEVEL_0)
, m_repeat(Repeat::NONE)
{
  m_need_draw = false;
  m_texture->retain();
}

/**
 * @destructor
 */
Sprite::~Sprite() {
  m_texture->XX_OFF(change, &Inl::texture_change_handle, Inl_Sprite(this));
  m_texture->release(); // 释放对像
}

/**
 * @overwrite
 */
void Sprite::draw(Draw* draw) {
  if ( m_visible ) {
    
    if ( mark_value ) {
      solve();
    }
    
    draw->draw(this);
    
    mark_value = M_NONE;
  }
}

String Sprite::src() const {
  return m_texture->id();
}

Sprite* Sprite::create(cString& path, Vec2 size) {
  Sprite* sp = New<Sprite>(size);
  sp->set_src(path);
  return sp;
}

Sprite* Sprite::create(Texture* texture, Vec2 size) {
  Sprite* sp = New<Sprite>(size);
  sp->set_texture(texture);
  return sp;
}

void Sprite::set_src(cString& value) {
  if (value.is_empty()) {
    set_texture( draw_ctx()->empty_texture() );
  } else {
    set_texture( tex_pool()->get_texture(value) );
  }
}

void Sprite::set_texture(Texture* value) {
  
  XX_ASSERT(value);
  
  // 如果值相同,不做处理
  if (value == m_texture) {
    return;
  }
  
  m_texture->release(); // 释放对像
  m_texture->XX_OFF(change, &Inl::texture_change_handle, Inl_Sprite(this));
  m_texture = value;
  m_texture->retain(); // 保持对像
  m_texture->XX_ON(change, &Inl::texture_change_handle, Inl_Sprite(this));
  
  // 顶点座标数据受 origin、width、height 的影响
  // 纹理座标数据受 startX、startY、width、height 的影响
  mark(M_TEXTURE);
}

void Sprite::set_start_x(float value) {
  if (m_start.x() != value) {
    m_start.x(value);
    mark(M_TEXTURE);
  }
}

void Sprite::set_start_y(float value) {
  if (m_start.y() != value) {
    m_start.y(value);
    mark(M_TEXTURE);
  }
}

void Sprite::set_width(float value) {
  // 值相同,不做处理
  if (m_size.width() != value) {
    m_size.width(value);
    mark(M_SHAPE | M_TEXTURE); // 标记这两个更新
  }
}

void Sprite::set_height(float value) {
  // 值相同,不做处理
  if (m_size.height() != value) {
    m_size.height(value);
    mark(M_SHAPE | M_TEXTURE); // 标记更新
  }
}

/**
 * @func ratio_x set
 */
void Sprite::set_ratio_x(float value) {
  if (m_ratio.x() != value) {
    m_ratio.x(value);
    mark( M_TEXTURE );
  }
}

/**
 * @func ratio_y set
 */
void Sprite::set_ratio_y(float value) {
  if (m_ratio.y() != value) {
    m_ratio.y(value);
    mark( M_TEXTURE );
  }
}

/**
 * @func repeat set
 */
void Sprite::set_repeat(Repeat value) {
  if (m_repeat != value) {
    m_repeat = value;
    mark( M_TEXTURE );
  }
}

/**
 * @overwrite
 */
void Sprite::set_parent(View* parent) throw(Error) {
  View::set_parent(parent);
  mark(M_TEXTURE);
}

bool Sprite::overlap_test(Vec2 point) {
  return View::overlap_test_from_convex_quadrilateral( m_final_vertex, point );
}

/**
 * @overwrite
 */
CGRect Sprite::screen_rect() {
  final_matrix();
  compute_box_vertex(m_final_vertex);
  return View::screen_rect_from_convex_quadrilateral(m_final_vertex);
}

/**
 * @func compute_final_vertex
 */
void Sprite::compute_box_vertex(Vec2 vertex[4]) {
  
  Vec2 start( -m_origin.x(), -m_origin.y() );
  Vec2 end  ( m_size.width() - m_origin.x(),
              m_size.height() - m_origin.y() );
  
  vertex[0] = m_final_matrix * start;
  vertex[1] = m_final_matrix * Vec2(end.x(), start.y());
  vertex[2] = m_final_matrix * end;
  vertex[3] = m_final_matrix * Vec2(start.x(), end.y());
}

/**
 * @func set_visible_draw
 */
void Sprite::set_visible_draw() {
  
  compute_box_vertex(m_final_vertex);
  
  /*
   * 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
   * 这种模糊测试在大多数时候都是正确有效的。
   */
  Region dre = display_port()->draw_region();
  Region re = screen_region_from_convex_quadrilateral(m_final_vertex);
  
  m_visible_draw = false;
  
  if (XX_MAX( dre.y2, re.y2 ) - XX_MIN( dre.y, re.y ) <= re.h + dre.h &&
      XX_MAX( dre.x2, re.x2 ) - XX_MIN( dre.x, re.x ) <= re.w + dre.w
  ) {
    m_tex_level = m_texture->get_texture_level_from_convex_quadrilateral(m_final_vertex);
    m_visible_draw = true;
  }
}

XX_END
