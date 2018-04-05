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

#include "panel.h"
#include "button.h"
#include "app-1.h"

XX_NS(ngui)

XX_DEFINE_INLINE_MEMBERS(Panel, Inl) {
public:
  
  static Button* first_button(View* v) {
    
    if ( v->as_panel() ) {
      return nullptr;
    }
    else
    if ( v->as_button() ) {
      if ( v->final_visible() ) {
        return v->as_button();
      }
    }
    else {
      v = v->first();
      
      while (v) {
        Button* button = first_button(v);
        if ( button ) {
          return button;
        }
        v = v->next();
      }
    }
    
    return nullptr;
  }
};

/**
 * @constructor
 */
Panel::Panel()
: m_allow_leave(false)
, m_allow_entry(false)
, m_interval_time(0), m_enable_select(true) {
  
}

/**
 * @func is_activity
 */
bool Panel::is_activity() const {
  View* view = app()->focus_view();
  if (view) {
    if ( view->as_button() ) {
      return view->as_button()->panel() == this;
    }
  }
  return false;
}

/**
 * @func parent_panel
 */
Panel* Panel::parent_panel() {
  View* v = parent();
  while( v ) {
    auto p = v->as_panel();
    if ( p ) {
      return p;
    }
    v = v->parent();
  }
  return nullptr;
}

XX_END
