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

class Frame::Inl: public Frame {
public:
#define _inl_frame(self) static_cast<KeyframeAction::Frame::Inl*>(self)
  
  /**
   * @func property_value
   */
  template<PropertyName Name, class T> inline T property_value() {
    auto it = m_host->m_property.find(Name);
    if ( it != m_host->m_property.end() ) {
      Property2<T>* prop = static_cast<Property2<T>*>(it.value());
      T v = prop->frame(m_index);
      return v;
    }
    return T();
  }
  
  /**
   * @func set_property_value
   */
  template<PropertyName Name, class T>
  inline void set_property_value(T value) {
    Map<PropertyName, Property*>& property = m_host->m_property;
    auto it = property.find(Name);
    if ( it.is_null() ) {
      //
      Property3<T, Name>* prop = new Property3<T, Name>(m_host->length());
      //
      property.set(Name, prop);
      prop->bind_view(m_host->m_bind_view_type);
      prop->frame(m_index, value);
    } else {
      Property2<T>* prop = static_cast<Property2<T>*>(it.value());
      prop->frame(m_index, value);
    }
  }
  
};

#define xx_def_property(ENUM, TYPE, NAME) \
TYPE Frame::NAME() { \
  return _inl_frame(this)->property_value<ENUM, TYPE>(); \
}\
void Frame::set_##NAME(TYPE value) { \
  _inl_frame(this)->set_property_value<ENUM>(value); \
}
xx_each_property_table(xx_def_property)
#undef xx_def_accessor

void Frame::set_margin(Value value) {
  set_margin_left(value);  set_margin_top(value);
  set_margin_right(value); set_margin_bottom(value);
}
void Frame::set_border(Border value) {
  set_border_left(value);  set_border_top(value);
  set_border_right(value); set_border_bottom(value);
}
void Frame::set_border_width(float value) {
  set_border_left_width(value);   set_border_top_width(value);
  set_border_right_width(value);  set_border_bottom_width(value);
}
void Frame::set_border_color(Color value) {
  set_border_left_color(value);   set_border_top_color(value);
  set_border_right_color(value);  set_border_bottom_color(value);
}
void Frame::set_border_radius(float value) {
  set_border_radius_left_top(value);      set_border_radius_right_top(value);
  set_border_radius_right_bottom(value);  set_border_radius_left_bottom(value);
}
