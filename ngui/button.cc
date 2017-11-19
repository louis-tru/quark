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

#include "button.h"
#include "select-panel.h"

XX_NS(ngui)

/**
 * @class Button::Inl
 */
class Button::Inl: public Button {
public:
#define _inl(self) static_cast<Button::Inl*>(self)
  
  /**
   * @func compute_final_matrix
   */
  void compute_final_matrix() {
    if ( mark_value & M_TRANSFORM ) {
      parent()->final_matrix().multiplication(matrix(), m_final_matrix);
      mark_value &= (~M_TRANSFORM); //  Delete M_TRANSFORM
      mark_value |= M_TRANSFORM_AND_OPACITY_CTX_DATA; // add
    }
  }
  
  /**
   * @func screen_rect_
   */
  CGRect screen_rect_() {
    Vec2 vertex[4];
    compute_final_matrix();
    compute_box_vertex(vertex);
    return View::screen_rect_from_convex_quadrilateral(vertex);
  }
  
  /**
   * @func Find
   */
  class Find: public Object {
  protected:
    SelectPanel* panel;
    Button* self;
    Button* revt;
    Vec2 origin;
    float min;
    
    Find(SelectPanel* p, Button* btn)
    : panel(p), self(btn), min(Float::max), revt(nullptr) {
      CGRect rect = self->screen_rect();
      origin = Vec2(rect.origin.x() + rect.size.width() / 2, 
                    rect.origin.y() + rect.size.height() / 2);
    }
    
  public:
    
    inline Button* result() { return revt; }
    
    virtual ~Find() {  }
    
    /**
     * @func find
     */
    virtual void find(Button* btn) = 0;
    
  };
  
  /**
   * @class FindHorizontal
   */
  template<Direction direction> class FindHorizontal: public Find {

  public:
    
    FindHorizontal(SelectPanel* panel, Button* btn): Find(panel, btn) { }
    
    /**
     * @overwrite
     */
    virtual void find(Button* btn) {
      
      CGRect rect = _inl(btn)->screen_rect_();
      Vec2 origin2 = Vec2(rect.origin.x() + rect.size.width() / 2, 
                          rect.origin.y() + rect.size.height() / 2);

      float x = origin2.x() - origin.x();
      if ( direction == Direction::LEFT ? x < 0 : x > 0 ) {
        float distance = origin.distance( origin2 );
        if ( distance < min ) {
          revt = btn;
          min = distance;
        }
      }
    }
  };
  
  /**
   * @class FindVertical
   */
  template<Direction direction> class FindVertical: public Find {

  public:
    
    FindVertical(SelectPanel* panel, Button* btn): Find(panel, btn) { }
    
    /**
     * @overwrite
     */
    virtual void find(Button* btn) {
            
      CGRect rect = _inl(btn)->screen_rect_();
      Vec2 origin2 = Vec2(rect.origin.x() + rect.size.width() / 2, 
                          rect.origin.y() + rect.size.height() / 2);

      float y = origin2.y() - origin.y();
      if ( direction == Direction::TOP ? y < 0 : y > 0 ) {
        float distance = origin.distance( origin2 );
        if ( distance < min ) {
          revt = btn;
          min = distance;
        }
      }
    }
  };

  /**
   * @func find_next_button_3
   */
  void find_next_button_3(Find* find, View* v) {
    if ( v != this && v->final_visible() ) {
      Button* btn = v->as_button();
      if ( btn ) {
        if ( btn->receive() ) {
          find->find(btn);
        }
      } else {
        if ( ! v->is_select_panel() ) {
          _inl(v)->compute_final_matrix();
          v = v->first();
          while ( v ) {
            find_next_button_3(find, v);
            v = v->next();
          }
        }
      }
    }
  }
  
  /**
   * @func find_next_button_2
   */
  void find_next_button_2(SelectPanel* panel, Find* find) {
    View* v = panel->first();
    while (v) {
      find_next_button_3(find, v);
      v = v->next();
    }
  }

  /**
   * @func NewFind
   */
  Find* NewFind(Direction direction, SelectPanel* panel) {
    switch (direction) {
      default:
      case Direction::LEFT:
        return new FindHorizontal<Direction::LEFT>(panel, this);
      case Direction::RIGHT:
        return new FindHorizontal<Direction::RIGHT>(panel, this);
      case Direction::TOP:
        return new FindVertical<Direction::TOP>(panel, this);
      case Direction::BOTTOM:
        return new FindVertical<Direction::BOTTOM>(panel, this);
    }
  }
  
  /**
   * @func find_next_button
   */
  Button* find_next_button(Direction direction) {
    SelectPanel* panel = this->panel();
    
    if ( panel && panel->enable_select() && final_visible() ) {
      panel->final_matrix(); // update final matrix
      
      Handle<Find> find = NewFind(direction, panel);
      find_next_button_2(panel, *find);
      
      if ( ! find->result() && panel->allow_leave() ) {
        panel = panel->parent_panel();
        if ( panel && panel->enable_select() ) {
          Handle<Find> find = NewFind(direction, panel);
          find_next_button_2(panel, *find);
          return find->result();
        }
      }
      return find->result();
    }
    return nullptr;
  }
  
};

Button::Button() {
  m_text_align = TextAlign::CENTER;
  m_text_size = { TextArrtsType::VALUE, 16 };
}

/**
 * @func find_next_button
 */
Button* Button::find_next_button(Direction direction) {
  return _inl(this)->find_next_button(direction);
}

/**
 * @func panel
 */
SelectPanel* Button::panel() {
  View* view = parent();
  while (view) {
    if ( view->is_select_panel() ) {
      return view->as_select_panel();
    }
    view = view->parent();
  }
  return nullptr;
}

/**
 * @overwrite
 */
bool Button::can_become_focus() {
  return true;
}

XX_END
