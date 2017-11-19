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

typedef StyleSheets::Property Property;
typedef PropertysAccessor::Accessor Accessor;
typedef KeyframeAction::Frame Frame;

/**
 * @class CSSProperty2
 */
template<class T> class CSSProperty0: public Property {
public:
  
  typedef T (View::*Get)();
  typedef void (View::*Set)(T value);
  typedef void (TextFont::*Set2)(T value);
  typedef void (TextLayout::*Set3)(T value);
  
  CSSProperty0(T value): m_value(value) { }
  
  inline T value() const { return m_value; }
  inline void set_value(T value) { m_value = value; }
  
  inline void assignment(View* view, PropertyName name) {
    XX_ASSERT(view);
    PropertysAccessor::Accessor accessor =
    PropertysAccessor::shared()->accessor(view->view_type(), name);
    if ( accessor.set_accessor ) {
      (view->*reinterpret_cast<Set>(accessor.set_accessor))(m_value);
    }
  }
  
protected:
  
  T m_value;
};

template<> inline void CSSProperty0<TextColor>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextSize>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextStyle>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextFamily>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextLineHeight>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextShadow>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextDecoration>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextOverflow>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_layout()->*reinterpret_cast<Set3>(accessor.set_accessor))(m_value);
  }
}
template<> inline void CSSProperty0<TextWhiteSpace>::assignment(View* view, PropertyName Name) {
  XX_ASSERT(view);
  Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
  if ( accessor.set_accessor ) {
    (view->as_text_layout()->*reinterpret_cast<Set3>(accessor.set_accessor))(m_value);
  }
}

/**
 * @class CSSProperty
 */
template<class T, PropertyName Name> class CSSProperty: public CSSProperty0<T> {
public:
  
  CSSProperty(T value): CSSProperty0<T>(value) { }
  
  virtual void assignment(View* view) {
    CSSProperty0<T>::assignment(view, Name);
  }
  
  virtual void assignment(Frame* frame) {
    XX_UNREACHABLE();
  }
  
};

#define xx_def_property(ENUM, TYPE, NAME) \
template<> void CSSProperty<TYPE, ENUM>::assignment(Frame* frame) { \
  frame->set_##NAME(m_value); \
}
xx_each_property_table(xx_def_property)
#undef xx_def_property
