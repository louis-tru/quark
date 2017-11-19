/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "shadow.h"
#include "draw.h"

XX_NS(ngui)

Shadow::Shadow(): m_is_draw_shadow_box(false), m_shadow() {
  
}

void Shadow::draw(Draw* draw) {
  if ( m_visible ) {
    
    if ( mark_value ) {
      solve();
    }
    
    draw->draw(this);
    
    mark_value = M_NONE;
  }
}

/**
 * @func set_shadow_offset_x
 */
void Shadow::set_shadow_offset_x(float value) {
  m_shadow.offset_x = value;
  mark(M_BOX_SHADOW);
}

/**
 * @func set_shadow_offset_y
 */
void Shadow::set_shadow_offset_y(float value) {
  m_shadow.offset_y = value;
  mark(M_BOX_SHADOW);
}

/**
 * @func set_shadow_size
 */
void Shadow::set_shadow_size(float value) {
  m_shadow.size = value;
  mark(M_BOX_SHADOW);
}

/**
 * @func set_shadow_color
 */
void Shadow::set_shadow_color(Color value) {
  m_shadow.color = value;
  mark(M_BOX_SHADOW);
}

/**
 * @func shadow {Shadow}
 */
void Shadow::set_shadow(ShadowValue value) {
  m_shadow = value;
  mark(M_BOX_SHADOW);
}

XX_END
