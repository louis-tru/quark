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

#include "app-1.h"
#include "display-port.h"
#include "root.h"
#include "draw.h"

XX_NS(ngui)

/**
 * @constructor
 */
Root::Root() throw(Error) {
  auto app = ngui::app();
  XX_ASSERT_ERR(app, "Before you create a root, you need to create a GUIApplication");
  m_background_color = Color(255, 255, 255); // 默认白色背景
  m_level = 1; // 根视图为1
  m_final_visible = true;
  m_explicit_width = true;
  m_explicit_height = true;
  Vec2 size = display_port()->size();
  set_width(size.width());
  set_height(size.height());
  mark(M_MATRIX);
  Inl_GUIApplication(app)->set_root(this);
}


/**
 * @destructor
 */
Root::~Root() {
  XX_DEBUG("destructor root");
}

/**
 * @overwrite
 */
void Root::draw(Draw* draw) {
  
  if ( m_visible ) {
    
    if ( mark_value ) {
      if ( mark_value & M_BASIC_MATRIX ) { // 变换
        Vec2 offset = layout_offset(); // xy 位移
        
        offset.x( offset.x() + origin().x() + translate().x() );
        offset.y( offset.y() + origin().y() + translate().y() );
        // 更新基础矩阵
        m_matrix = Mat(offset, scale(), -rotate_z(), skew());
      }
      
      if ( mark_value & M_OPACITY ) {
        m_final_opacity = opacity();
      }
      if ( mark_value & M_TRANSFORM ) {
        m_final_matrix = m_matrix;
      }
      
      if ( mark_value & (M_TRANSFORM | M_SHAPE) ) {
        set_visible_draw();
      }
    }
    
    draw->draw(this);
    
    mark_value = M_NONE;
    
  } else {
    draw->clear_screen(Color(0, 0, 0));
  }
}

/**
 * @overwrite
 */
void Root::prepend(View* child) throw(Error) {
  if ( ! m_ctr ) { // set default controller
    (new ViewController())->view(this);
  }
  Box::prepend(child);
}

/**
 * @overwrite
 */
void Root::append(View* child) throw(Error) {
  if ( ! m_ctr) { // set default controller
    (new ViewController())->view(this);
  }
  Box::append(child);
}

/**
 * @overwrite
 */
View* Root::append_text(cUcs2String& str) throw(Error) {
  if ( ! m_ctr) { // set default controller
    (new ViewController())->view(this);
  }
  return Box::append_text(str);
}

/**
 * @overwrite
 */
void Root::set_parent(View* parent) throw(Error) {
  XX_UNREACHABLE();
}

/**
 * @overwrite
 */
bool Root::can_become_focus() {
  return true;
}

/**
 * @overwrite
 */
void Root::set_layout_explicit_size() {
  
  float final_width = m_final_width;
  float final_height = m_final_height;
  
  Vec2 port_size = display_port()->size();

  if ( m_width.type == ValueType::PIXEL ) {
    m_final_width = m_width.value;
  } else {
    m_final_width = port_size.width();
  }

  if ( m_height.type == ValueType::PIXEL ) {
    m_final_height = m_height.value;
  } else {
    m_final_height = port_size.height();
  }
  
  m_raw_client_width = m_final_width;
  m_raw_client_height = m_final_height;
  m_limit.width(m_final_width);
  m_limit.height(m_final_height);
  
  Box::set_default_offset_value();
  
  bool h = m_final_width != final_width;
  bool v = m_final_height != final_height;
  uint child_mark = M_NONE;
  
  if ( h ) {
    if ( m_content_align == ContentAlign::RIGHT ) {
      child_mark = M_MATRIX;
    }
  }
  if ( v ) {
    if ( m_content_align == ContentAlign::BOTTOM ) {
      child_mark = M_MATRIX;
    }
  }
  
  solve_explicit_size_after(h, v, child_mark);
}

/**
 * @overwrite
 */
void Root::set_layout_content_offset() {
  Vec2 squeeze;
  Div::set_div_content_offset(squeeze, Vec2());
}

/**
 * @overwrite
 */
Vec2 Root::layout_offset() {
  return Vec2(m_offset_start.x() + m_final_margin_left + m_border_left.width,
              m_offset_start.y() + m_final_margin_top + m_border_top.width);
}

XX_END
