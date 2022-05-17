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

#include "./layout/layout.h"
#include "./text_settings.h"
#include "./render/font/pool.h"

namespace noug {

	void TextSettings::onTextChange(uint32_t mark, uint32_t flags) {
		// noop
	}

	void TextSettings::set_text_weight(TextWeight value) {
		if (value != _text_weight) {
			_text_weight = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height, (1 << 0));
		}
	}

	void TextSettings::set_text_style(TextStyle value) {
		if (value != _text_style) {
			_text_style = value;
			onTextChange(Layout::kLayout_None, (1 << 1));
		}
	}

	void TextSettings::set_text_decoration(TextDecoration value) {
		if (value != _text_decoration) {
			_text_decoration = value;
			onTextChange(Layout::kLayout_None, (1 << 2));
		}
	}

	void TextSettings::set_text_overflow(TextOverflow value) {
		if (value != _text_overflow) {
			_text_overflow = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height, (1 << 3));
		}
	}

	void TextSettings::set_text_white_space(TextWhiteSpace value) {
		if (value != _text_white_space) {
			_text_white_space = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height, (1 << 4));
		}
	}

	void TextSettings::set_text_size(TextSize value) {
		if (value != _text_size) {
			_text_size = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height, (1 << 5));
		}
	}

	void TextSettings::set_text_background_color(TextColor value) {
		if (value != _text_background_color) {
			_text_background_color = value;
			onTextChange(Layout::kLayout_None, (1 << 6));
		}
	}

	void TextSettings::set_text_color(TextColor value) {
		if (value != _text_color) {
			_text_color = value;
			onTextChange(Layout::kLayout_None, (1 << 7));
		}
	}

	void TextSettings::set_text_shadow(TextShadow value) {
		if (value != _text_shadow) {
			_text_shadow = value;
			onTextChange(Layout::kLayout_None, (1 << 8));
		}
	}

	void TextSettings::set_text_line_height(TextLineHeight value) {
		if (value != _text_line_height) {
			_text_line_height = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height, (1 << 9));
		}
	}

	void TextSettings::set_text_family(TextFamily value) {
		if (value != _text_family) {
			_text_family = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height, (1 << 10));
		}
	}

	// ---------------- T e x t . S e t t i n g s . A c c e s s o r ----------------

	TextSettingsAccessor::TextSettingsAccessor(TextSettings* textSet, TextSettingsAccessor* prototype)
		: _textSettings(textSet), _prototype(prototype), _flags(0xffffffffu)
	{}

#define N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL(Type, name, flag) \
	Type TextSettingsAccessor::name() { \
		if (_flags & (1 << flag)) { \
			if (_textSettings->_##name == Type::INHERIT) \
				_##name = _prototype->name(); \
			_flags &= ~(1 << flag); \
		} \
		return _##name; \
	} \

#define N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL_2(Type, name, flag, Default) \
	Type TextSettingsAccessor::name() { \
		if (_flags & (1 << flag)) { \
			if (_textSettings->_##name.kind == TextValueKind::INHERIT) {  \
				_textSettings->_##name.value = _prototype->name(); \
			} else if (_textSettings->_##name.kind == TextValueKind::DEFAULT) { \
				_textSettings->_##name.value = Default; \
			} \
			_flags &= ~(1 << flag); \
		} \
		return _textSettings->_##name.value; \
	} \

	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL(TextWeight, text_weight, 0);
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL(TextStyle, text_style, 1);
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL(TextDecoration, text_decoration, 2);
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL(TextOverflow, text_overflow, 3);
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL(TextWhiteSpace, text_white_space, 4);
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL_2(float, text_size, 5, 16);
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL_2(Color, text_background_color, 6, Color(0, 0, 0, 0));
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL_2(Color, text_color, 7, Color(0, 0, 0));
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL_2(Shadow, text_shadow, 8, (Shadow{ 0, 0, 0, Color(0, 0, 0, 0) }));
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL_2(float, text_line_height, 9, 0);
	N_DEFINE_TEXT_SETTINGS_ACCESSOR_IMPL_2(FFID, text_family, 10, (_prototype->text_family()->pool()->getFFID()));


	// ---------------- D e f a u l t . T e x t . S e t t i n g s ----------------

	DefaultTextSettings::DefaultTextSettings(FontPool *pool)
		: TextSettings()
		, TextSettingsAccessor(this, new TextSettingsAccessor(new TextSettings(), nullptr))
	{
		auto text = _prototype->textSettings();
		text->set_text_weight(TextWeight::DEFAULT);
		text->set_text_style(TextStyle::DEFAULT);
		text->set_text_decoration(TextDecoration::DEFAULT);
		text->set_text_overflow(TextOverflow::DEFAULT);
		text->set_text_white_space(TextWhiteSpace::DEFAULT);
		text->set_text_background_color({Color(0, 0, 0, 0), TextValueKind::VALUE});
		text->set_text_color({Color(0, 0, 0), TextValueKind::VALUE});
		text->set_text_size({16, TextValueKind::VALUE});
		text->set_text_line_height({0, TextValueKind::DEFAULT});
		text->set_text_family({pool->getFFID(), TextValueKind::VALUE});
		text->set_text_shadow({{ 0, 0, 0, Color(0, 0, 0, 0) }, TextValueKind::VALUE});
	}

	DefaultTextSettings::~DefaultTextSettings() {
		delete _prototype->textSettings();
		delete _prototype;
	}

	void DefaultTextSettings::onTextChange(uint32_t mark, uint32_t flags) {
		_flags |= flags; // mark
	}
}
