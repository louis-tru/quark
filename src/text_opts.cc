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

#include "./text_opts.h"
#include "./layout/layout.h"
#include "./render/font/pool.h"

namespace noug {

	TextOptions::TextOptions()
		: _text_size{ .kind=TextValueKind::INHERIT }
		, _text_background_color{ .kind=TextValueKind::INHERIT }
		, _text_color{ .kind=TextValueKind::INHERIT }
		, _text_shadow{ .kind=TextValueKind::INHERIT }
		, _text_line_height{ .kind=TextValueKind::INHERIT }
		, _text_family{ .kind=TextValueKind::INHERIT }
		, _text_flags(0xffffffff)
	{
	}

	void TextOptions::onTextChange(uint32_t mark) {
		// noop
	}

	void TextOptions::set_text_weight(TextWeight value) {
		if (value != _text_weight) {
			_text_weight = _text_weight_value = value;
			_text_flags |= (1 << 0);
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextOptions::set_text_slant(TextSlant value) {
		if (value != _text_slant) {
			_text_slant = _text_slant_value = value;
			_text_flags |= (1 << 1);
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextOptions::set_text_decoration(TextDecoration value) {
		if (value != _text_decoration) {
			_text_decoration = _text_decoration_value = value;
			_text_flags |= (1 << 2);
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextOptions::set_text_overflow(TextOverflow value) {
		if (value != _text_overflow) {
			_text_overflow = _text_overflow_value = value;
			_text_flags |= (1 << 3);
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextOptions::set_text_white_space(TextWhiteSpace value) {
		if (value != _text_white_space) {
			_text_white_space = _text_white_space_value = value;
			_text_flags |= (1 << 4);
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextOptions::set_text_word_break(TextWordBreak value) {
		if (value != _text_word_break) {
			_text_word_break = _text_word_break_value = value;
			_text_flags |= (1 << 5);
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextOptions::set_text_size(TextSize value) {
		if (value != _text_size) {
			value.value = N_MAX(1, value.value);
			_text_size = value;
			_text_flags |= (1 << 6);
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextOptions::set_text_background_color(TextColor value) {
		if (value != _text_background_color) {
			_text_background_color = value;
			_text_flags |= (1 << 7);
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextOptions::set_text_color(TextColor value) {
		if (value != _text_color) {
			_text_color = value;
			_text_flags |= (1 << 8);
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextOptions::set_text_shadow(TextShadow value) {
		if (value != _text_shadow) {
			_text_shadow = value;
			_text_flags |= (1 << 9);
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextOptions::set_text_line_height(TextLineHeight value) {
		if (value != _text_line_height) {
			value.value = N_MAX(0, value.value);
			_text_line_height = value;
			_text_flags |= (1 << 10);
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextOptions::set_text_family(TextFamily value) {
		if (value != _text_family) {
			_text_family = value;
			_text_flags |= (1 << 11);
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	FontStyle TextOptions::font_style() const {
		return {_text_weight_value, TextWidth::DEFAULT, _text_slant_value};
	}

	// ---------------- T e x t . C o n f i g ----------------

#define N_DEFINE_COMPUTE_TEXT_OPTIONS(Type, name, flag) \
	if (_opts->_##name == Type::INHERIT) { \
		_opts->_##name##_value = _base->_opts->_##name##_value; \
	}

#define N_DEFINE_COMPUTE_TEXT_OPTIONS_2(Type, name, flag, Default) \
	if (_opts->_##name.kind == TextValueKind::INHERIT) {  \
		_opts->_##name.value = _base->_opts->_##name.value; \
	} else if (_opts->_##name.kind == TextValueKind::DEFAULT) { \
		_opts->_##name.value = Default; \
	}

	TextConfig::TextConfig(TextOptions* opts, TextConfig* base)
		: _opts(opts), _base(base)
	{
		if (_base && (_opts->_text_flags || _base->_opts->_text_flags)) {
			_opts->_text_flags |= _base->_opts->_text_flags;
			N_DEFINE_COMPUTE_TEXT_OPTIONS(TextWeight, text_weight, 0);
			N_DEFINE_COMPUTE_TEXT_OPTIONS(TextSlant, text_slant, 1);
			N_DEFINE_COMPUTE_TEXT_OPTIONS(TextDecoration, text_decoration, 2);
			N_DEFINE_COMPUTE_TEXT_OPTIONS(TextOverflow, text_overflow, 3);
			N_DEFINE_COMPUTE_TEXT_OPTIONS(TextWhiteSpace, text_white_space, 4);
			N_DEFINE_COMPUTE_TEXT_OPTIONS(TextWordBreak, text_word_break, 5);
			N_DEFINE_COMPUTE_TEXT_OPTIONS_2(float, text_size, 6, 16);
			N_DEFINE_COMPUTE_TEXT_OPTIONS_2(Color, text_background_color, 7, Color(0, 0, 0, 0));
			N_DEFINE_COMPUTE_TEXT_OPTIONS_2(Color, text_color, 8, Color(0, 0, 0));
			N_DEFINE_COMPUTE_TEXT_OPTIONS_2(Shadow, text_shadow, 9, (Shadow{ 0, 0, 0, Color(0, 0, 0, 0) }));
			N_DEFINE_COMPUTE_TEXT_OPTIONS_2(float, text_line_height, 10, 0);
			N_DEFINE_COMPUTE_TEXT_OPTIONS_2(FFID, text_family, 11, (_base->_opts->_text_family.value->pool()->getFFID()));
		}
	}

	TextConfig::~TextConfig() {
		_opts->_text_flags = 0; // clear flags
	}

	// ---------------- D e f a u l t . T e x t . S e t t i n g s ----------------

	class DefaultTextConfig: public TextConfig {
	public:
		DefaultTextConfig(FontPool *pool): TextConfig(new TextOptions(), nullptr) {
			auto opts = this->opts();
			opts->set_text_weight(TextWeight::DEFAULT);
			opts->set_text_slant(TextSlant::DEFAULT);
			opts->set_text_decoration(TextDecoration::DEFAULT);
			opts->set_text_overflow(TextOverflow::DEFAULT);
			opts->set_text_white_space(TextWhiteSpace::DEFAULT);
			opts->set_text_word_break(TextWordBreak::DEFAULT);
			opts->set_text_background_color({Color(0, 0, 0, 0), TextValueKind::VALUE});
			opts->set_text_color({Color(0, 0, 0), TextValueKind::VALUE});
			opts->set_text_size({16, TextValueKind::VALUE});
			opts->set_text_line_height({0, TextValueKind::DEFAULT});
			opts->set_text_family({pool->getFFID(), TextValueKind::VALUE});
			opts->set_text_shadow({{ 0, 0, 0, Color(0, 0, 0, 0) }, TextValueKind::VALUE});
		}
	};

	DefaultTextOptions::DefaultTextOptions(FontPool *pool)
		: TextOptions()
		, TextConfig(this, new DefaultTextConfig(pool))
	{}

	DefaultTextOptions::~DefaultTextOptions() {
		delete base()->opts();
		delete base();
	}

}
