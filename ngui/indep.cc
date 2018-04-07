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

#include "indep.h"
#include "hybrid.h"
#include "limit-indep.h"

XX_NS(ngui)

class Indep::_Inl : public Indep {
public:
#define _inl(self) static_cast<Indep::_Inl*>(static_cast<Indep*>(self))
  
  /**
   * @func set_indep_offset
   */
  void set_indep_offset(Box* parent) { XX_ASSERT(parent);
    
    if (m_align_x == Align::RIGHT) { // 水平右对齐
      m_offset_start.x(parent->m_final_width - m_raw_client_width);
      m_offset_end.x(parent->m_final_width);
    } else if (m_align_x == Align::CENTER) { // 水平居中
      m_offset_start.x(-m_raw_client_width / 2 + parent->m_final_width / 2);
      m_offset_end.x(m_raw_client_width / 2 + parent->m_final_width / 2);
    } else { // 水平左对齐
      m_offset_start.x(0);
      m_offset_end.x(m_raw_client_width);
    }
    
    if (m_align_y == Align::BOTTOM) { // 垂直底部对齐
      m_offset_start.y(parent->m_final_height - m_raw_client_height);
      m_offset_end.y(parent->m_final_height);
    } else if (m_align_y == Align::CENTER) { // 垂直居中
      m_offset_start.y(-m_raw_client_height / 2 + parent->m_final_height / 2);
      m_offset_end.y(m_raw_client_height / 2 + parent->m_final_height / 2);
    } else { // 垂直顶部对齐
      m_offset_start.y(0);
      m_offset_end.y(m_raw_client_height);
    }
    
    mark(M_MATRIX); // mark transform
  }
};

Indep::Indep(): m_align_x(Align::LEFT), m_align_y(Align::TOP) { }

void Indep::set_align(Align x, Align y) {
  set_align_x(x);
  set_align_y(y);
}

void Indep::set_align_x(Align value) {
  if (value == Align::LEFT || value == Align::RIGHT || value == Align::CENTER) {
    m_align_x = value;
    mark_pre(M_MATRIX | M_LAYOUT | M_SIZE_HORIZONTAL);
  }
}

void Indep::set_align_y(Align value) {
  if (value == Align::TOP || value == Align::BOTTOM || value == Align::CENTER) {
    m_align_y = value;
    mark_pre(M_MATRIX | M_LAYOUT | M_SIZE_VERTICAL);
  }
}

void Indep::set_parent(View* parent) throw(Error) {
  if (parent != View::parent()) {
    View::set_parent(parent);
    mark_pre(M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL);
  }
}

void Indep::set_layout_content_offset() {
  
  if (m_final_visible) {
    Vec2 squeeze;
    if (  set_div_content_offset(squeeze, Vec2()) ) { //
      // 高度或宽度被挤压即形状发生变化
      mark(M_SHAPE);
      
      Box* box = parent()->as_box();
      if ( box ) {
        m_parent_layout = box;
        mark_pre(M_LAYOUT_THREE_TIMES);
      } else { // 父视图只是个普通视图,默认将偏移设置为0
        set_default_offset_value();
      }
    }
  }
}

Box* Indep::set_offset_horizontal(Box* prev, Vec2& squeeze, float limit, Div* div) {
  if ( m_visible ) {
    m_parent_layout = div; XX_ASSERT(div);
    mark_pre(M_LAYOUT_THREE_TIMES);
  } else {
    set_default_offset_value();
  }
  return prev;
}

Box* Indep::set_offset_vertical(Box* prev, Vec2& squeeze, float limit, Div* div) {
  if ( m_visible ) {
    m_parent_layout = div; XX_ASSERT(div);
    mark_pre(M_LAYOUT_THREE_TIMES);
  } else {
    set_default_offset_value();
  }
  return prev;
}

void Indep::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
  if ( m_visible ) {
    if ( hybrid ) {
      m_parent_layout = hybrid;
      mark_pre(M_LAYOUT_THREE_TIMES);
    }
  } else {
    set_default_offset_value();
  }
}


void Indep::set_layout_three_times(bool horizontal, bool hybrid) {
  if ( m_visible ) {
    // Div::set_layout_three_times(horizontal, false);
    _inl(this)->set_indep_offset(static_cast<Box*>(m_parent_layout));
  }
}

void LimitIndep::set_layout_three_times(bool horizontal, bool hybrid) {
  if ( m_visible ) {
    _inl(this)->set_indep_offset(static_cast<Box*>(m_parent_layout));
  }
}

Vec2 Indep::layout_offset() {
  return Vec2( m_offset_start.x() + m_final_margin_left + m_border_left_width,
               m_offset_start.y() + m_final_margin_top + m_border_top_width);
}

XX_END
