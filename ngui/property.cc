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

#include "property.h"
#include "view.h"
#include "box.h"
#include "div.h"
#include "hybrid.h"
#include "text.h"
#include "limit.h"
#include "limit-indep.h"
#include "indep.h"
#include "text-node.h"
#include "label.h"
#include "image.h"
#include "sprite.h"
#include "box-shadow-1.h"

XX_NS(ngui)

#define set_func(view, cls, Name, func) view.set(Name, Accessor(&cls::func, &cls::set_##func))

static PropertysAccessor* xx_accessor_shared = nullptr;

/**
 * @func shared
 */
PropertysAccessor* PropertysAccessor::shared() {
  if ( ! xx_accessor_shared ) {
    xx_accessor_shared = new PropertysAccessor();
  }
  return xx_accessor_shared;
}

PropertysAccessor::PropertysAccessor() {
  
  Map<PropertyName, Accessor> view;
  
  set_func(view, View, PROPERTY_X, x);
  set_func(view, View, PROPERTY_Y, y);
  set_func(view, View, PROPERTY_SCALE_X, scale_x);
  set_func(view, View, PROPERTY_SCALE_Y, scale_y);
  set_func(view, View, PROPERTY_SKEW_X, skew_x);
  set_func(view, View, PROPERTY_SKEW_Y, skew_y);
  set_func(view, View, PROPERTY_ROTATE_Z, rotate_z);
  set_func(view, View, PROPERTY_ORIGIN_X, origin_x);
  set_func(view, View, PROPERTY_ORIGIN_Y, origin_y);
  set_func(view, View, PROPERTY_OPACITY, opacity);
  view.set(PROPERTY_VISIBLE, Accessor(&View::visible, &View::set_visible_0));
  
  Map<PropertyName, Accessor> box = view;
  
  set_func(box, Box, PROPERTY_WIDTH, width);  // Value
  set_func(box, Box, PROPERTY_HEIGHT, height); // Value
  set_func(box, Box, PROPERTY_MARGIN_LEFT, margin_left);
  set_func(box, Box, PROPERTY_MARGIN_TOP, margin_top);
  set_func(box, Box, PROPERTY_MARGIN_RIGHT, margin_right);
  set_func(box, Box, PROPERTY_MARGIN_BOTTOM, margin_bottom);
  set_func(box, Box, PROPERTY_BORDER_LEFT, border_left);
  set_func(box, Box, PROPERTY_BORDER_TOP, border_top);
  set_func(box, Box, PROPERTY_BORDER_RIGHT, border_right);
  set_func(box, Box, PROPERTY_BORDER_BOTTOM, border_bottom);
  set_func(box, Box, PROPERTY_BORDER_LEFT_WIDTH, border_left_width);
  set_func(box, Box, PROPERTY_BORDER_TOP_WIDTH, border_top_width);
  set_func(box, Box, PROPERTY_BORDER_RIGHT_WIDTH, border_right_width);
  set_func(box, Box, PROPERTY_BORDER_BOTTOM_WIDTH, border_bottom_width);
  set_func(box, Box, PROPERTY_BORDER_LEFT_COLOR, border_left_color);
  set_func(box, Box, PROPERTY_BORDER_TOP_COLOR, border_top_color);
  set_func(box, Box, PROPERTY_BORDER_RIGHT_COLOR, border_right_color);
  set_func(box, Box, PROPERTY_BORDER_BOTTOM_COLOR, border_bottom_color);
  set_func(box, Box, PROPERTY_BORDER_RADIUS_LEFT_TOP, border_radius_left_top);
  set_func(box, Box, PROPERTY_BORDER_RADIUS_RIGHT_TOP, border_radius_right_top);
  set_func(box, Box, PROPERTY_BORDER_RADIUS_RIGHT_BOTTOM, border_radius_right_bottom);
  set_func(box, Box, PROPERTY_BORDER_RADIUS_LEFT_BOTTOM, border_radius_left_bottom);
  set_func(box, Box, PROPERTY_BACKGROUND_COLOR, background_color);
  set_func(box, Box, PROPERTY_NEWLINE, newline);

  Map<PropertyName, Accessor> div = box;
  Map<PropertyName, Accessor> hybrid = box;
  set_func(div, Div, PROPERTY_CONTENT_ALIGN, content_align);
  set_func(hybrid, Hybrid, PROPERTY_TEXT_ALIGN, text_align);

  m_property_func_table.set(View::VIEW, view);
  m_property_func_table.set(View::BOX, box);
  m_property_func_table.set(View::DIV, div);
  m_property_func_table.set(View::INDEP, div);
  m_property_func_table.set(View::SCROLL, div);
  m_property_func_table.set(View::ROOT, div);
  m_property_func_table.set(View::LIMIT, div);
  m_property_func_table.set(View::IMAGE, div);
  m_property_func_table.set(View::BOX_SHADOW, div);
  m_property_func_table.set(View::SELECT_PANEL, div);
  m_property_func_table.set(View::CLIP, div);
  m_property_func_table.set(View::SPAN, view);
  m_property_func_table.set(View::LABEL, view);
  m_property_func_table.set(View::SPRITE, view);
  m_property_func_table.set(View::HYBRID, hybrid);
  // indep/limit_indep
  set_func(m_property_func_table[View::INDEP], Indep, PROPERTY_ALIGN_X, align_x);
  set_func(m_property_func_table[View::INDEP], Indep, PROPERTY_ALIGN_Y, align_y);
  m_property_func_table.set(View::LIMIT_INDEP, m_property_func_table[View::INDEP]);
  // shadow
  set_func(m_property_func_table[View::BOX_SHADOW], BoxShadow, PROPERTY_REPEAT, shadow);
  // limit/limit_indep
  set_func(m_property_func_table[View::LIMIT], Limit, PROPERTY_MAX_WIDTH, max_width);
  set_func(m_property_func_table[View::LIMIT], Limit, PROPERTY_MAX_HEIGHT, max_height);
  set_func(m_property_func_table[View::LIMIT_INDEP], LimitIndep, PROPERTY_MAX_WIDTH, max_width);
  set_func(m_property_func_table[View::LIMIT_INDEP], LimitIndep, PROPERTY_MAX_HEIGHT, max_height);
  // video/image
  set_func(m_property_func_table[View::IMAGE], Image, PROPERTY_SRC, src);
  set_func(m_property_func_table[View::IMAGE], Image, PROPERTY_BACKGROUND_IMAGE, background_image);
  m_property_func_table[View::VIDEO] = m_property_func_table[View::IMAGE];
  // sprite
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_WIDTH2, width);
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_HEIGHT2, height);
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_START_X, start_x);
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_START_Y, start_y);
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_RATIO_X, ratio_x);
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_RATIO_Y, ratio_y);
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_REPEAT, repeat);
  set_func(m_property_func_table[View::SPRITE], Sprite, PROPERTY_SRC, src);
  // label
  set_func(m_property_func_table[View::LABEL], Label, PROPERTY_TEXT_ALIGN, text_align);
  
  // text-font
  Map<PropertyName, Accessor> font;
  set_func(font, TextFont, PROPERTY_TEXT_BACKGROUND_COLOR, text_background_color);
  set_func(font, TextFont, PROPERTY_TEXT_COLOR, text_color);
  set_func(font, TextFont, PROPERTY_TEXT_SIZE, text_size);
  set_func(font, TextFont, PROPERTY_TEXT_STYLE, text_style);
  set_func(font, TextFont, PROPERTY_TEXT_FAMILY, text_family);
  set_func(font, TextFont, PROPERTY_TEXT_SHADOW, text_shadow);
  set_func(font, TextFont, PROPERTY_TEXT_LINE_HEIGHT, text_line_height);
  set_func(font, TextFont, PROPERTY_TEXT_DECORATION, text_decoration);
  
  for (auto& i : font) { // extend
    m_property_func_table[View::LABEL].set(i.key(), i.value()); // label
  }
  
  // text-layout
  set_func(font, TextLayout, PROPERTY_TEXT_OVERFLOW, text_overflow);
  set_func(font, TextLayout, PROPERTY_TEXT_WHITE_SPACE, text_white_space);
  
  for (auto& i : font) { // extend
    m_property_func_table[View::HYBRID].set(i.key(), i.value());  // hybrid
    m_property_func_table[View::SPAN].set(i.key(), i.value());  // span
  }
  
  m_property_func_table.set(View::BUTTON, m_property_func_table[View::HYBRID]);
  m_property_func_table.set(View::TEXT, m_property_func_table[View::HYBRID]);
  m_property_func_table.set(View::INPUT, m_property_func_table[View::HYBRID]);
  m_property_func_table.set(View::TEXTAREA, m_property_func_table[View::HYBRID]);
  m_property_func_table.set(View::TEXT_NODE, m_property_func_table[View::SPAN]);
}

/**
 * @func accessor
 */
PropertysAccessor::Accessor
PropertysAccessor::accessor(ViewType type, PropertyName name) {
  return m_property_func_table[type][name];
}

/**
 * @func has_accessor
 */
bool PropertysAccessor::has_accessor(int view_type, PropertyName name) {
  return m_property_func_table[view_type].has(name);
}

XX_END
