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

#include "./_css.h"

N_NAMESPACE_START

template<> inline void CSSProperty<TextColor>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextSize>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextSlant>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextFamily>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextLineHeight>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextShadow>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextDecoration>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_font()->*reinterpret_cast<Set2>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextOverflow>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_layout()->*reinterpret_cast<Set3>(accessor.set_accessor))(_value);
	}
}
template<> inline void CSSProperty<TextWhiteSpace>::assignment(View* view, PropertyName Name) {
	N_Assert(view);
	Accessor accessor = PropertysAccessor::shared()->accessor(view->view_type(), Name);
	if ( accessor.set_accessor ) {
		(view->as_text_layout()->*reinterpret_cast<Set3>(accessor.set_accessor))(_value);
	}
}
template <> CSSProperty<BackgroundPtr>::~CSSProperty() {
	Release(_value);
}
template <> BackgroundPtr CSSProperty<BackgroundPtr>::value() const {
	return _value;
}
template <> void CSSProperty<BackgroundPtr>::set_value(BackgroundPtr value) {
	_value = Background::assign(_value, value ? value : new BackgroundImage());
}
template <> CSSProperty<BackgroundPtr>::CSSProperty(BackgroundPtr val): _value(nullptr) {
	CSSProperty<BackgroundPtr>::set_value(val);
}

#define fx_def_property(ENUM, TYPE, NAME) \
	template<> void CSSProperty1<TYPE, ENUM>::assignment(Frame* frame) { \
		frame->set_##NAME(_value); \
	}
N_EACH_PROPERTY_TABLE(fx_def_property)
#undef fx_def_property

N_NAMESPACE_END