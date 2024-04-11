/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./text_opts.h"
#include "../view/view.h"
#include "../../render/font/pool.h"

namespace qk {

	TextOptions::TextOptions()
		: _text_align(TextAlign::kDefault)
		, _text_weight(TextWeight::kInherit)
		, _text_slant(TextSlant::kInherit)
		, _text_decoration(TextDecoration::kInherit)
		, _text_overflow(TextOverflow::kInherit)
		, _text_white_space(TextWhiteSpace::kInherit)
		, _text_word_break(TextWordBreak::kInherit)
		, _text_size{ .kind=TextValueKind::kInherit }
		, _text_background_color{ .kind=TextValueKind::kInherit }
		, _text_color{ .kind=TextValueKind::kInherit }
		, _text_shadow{ .kind=TextValueKind::kInherit }
		, _text_line_height{ .kind=TextValueKind::kInherit }
		, _text_family{ .value=nullptr, .kind=TextValueKind::kInherit }
		, _text_family_Rt{ .kind=TextValueKind::kInherit }
		, _text_flags(0xffffffffu)
	{
	}

	void TextOptions::onTextChange(uint32_t mark, uint32_t type) {
		_text_flags |= (1 << type);
	}

	View* TextOptions::getViewForTextOptions() {
		return nullptr;
	}

	void TextOptions::onTextChange_async(uint32_t mark, uint32_t type) {
		if (type == 12) {
			struct Data { uint32_t mark; TextFamily textFamily; };
			getViewForTextOptions()->preRender().async_call([](auto self, auto arg) {
				self->_text_flags |= (1 << 12);
				self->_text_family_Rt = arg.arg->textFamily;
				auto view = self->getViewForTextOptions();
				arg.arg->mark ? view->mark_layout(arg.arg->mark): view->mark();
				delete arg.arg;
			}, this, new Data{mark,_text_family});
		} else {
			struct Data { uint32_t mark, type; };
			getViewForTextOptions()->preRender().async_call([](auto self, auto arg) {
				self->_text_flags |= (1 << arg.arg.type);
				auto view = self->getViewForTextOptions();
				arg.arg.mark ? view->mark_layout(arg.arg.mark): view->mark();
			}, this, Data{mark,type});
		}
	}

	void TextOptions::set_text_align(TextAlign value) {
		if(_text_align != value) {
			_text_align = value;
			onTextChange(View::kLayout_Typesetting, 0);
		}
	}

	void TextOptions::set_text_weight(TextWeight value) {
		if (value != _text_weight) {
			_text_weight = _text_weight_value = value;
			onTextChange(View::kLayout_Typesetting, 1);
		}
	}

	void TextOptions::set_text_slant(TextSlant value) {
		if (value != _text_slant) {
			_text_slant = _text_slant_value = value;
			onTextChange(View::kLayout_None, 2);
		}
	}

	void TextOptions::set_text_decoration(TextDecoration value) {
		if (value != _text_decoration) {
			_text_decoration = _text_decoration_value = value;
			onTextChange(View::kLayout_None, 3);
		}
	}

	void TextOptions::set_text_overflow(TextOverflow value) {
		if (value != _text_overflow) {
			_text_overflow = _text_overflow_value = value;
			onTextChange(View::kLayout_Typesetting, 4);
		}
	}

	void TextOptions::set_text_white_space(TextWhiteSpace value) {
		if (value != _text_white_space) {
			_text_white_space = _text_white_space_value = value;
			onTextChange(View::kLayout_Typesetting, 5);
		}
	}

	void TextOptions::set_text_word_break(TextWordBreak value) {
		if (value != _text_word_break) {
			_text_word_break = _text_word_break_value = value;
			onTextChange(View::kLayout_Typesetting, 6);
		}
	}

	void TextOptions::set_text_size(TextSize value) {
		if (value != _text_size) {
			value.value = Qk_MAX(1, value.value);
			_text_size = value;
			onTextChange(View::kLayout_Typesetting, 7);
		}
	}

	void TextOptions::set_text_background_color(TextColor value) {
		if (value != _text_background_color) {
			_text_background_color = value;
			onTextChange(View::kLayout_None, 8);
		}
	}

	void TextOptions::set_text_color(TextColor value) {
		if (value != _text_color) {
			_text_color = value;
			onTextChange(View::kLayout_None, 9);
		}
	}

	void TextOptions::set_text_shadow(TextShadow value) {
		if (value != _text_shadow) {
			_text_shadow = value;
			onTextChange(View::kLayout_None, 10);
		}
	}

	void TextOptions::set_text_line_height(TextLineHeight value) {
		if (value != _text_line_height) {
			value.value = Qk_MAX(0, value.value);
			_text_line_height = value;
			onTextChange(View::kLayout_Typesetting, 11);
		}
	}

	void TextOptions::set_text_family(TextFamily value) {
		if (value != _text_family) {
			_text_family = value;
			onTextChange(View::kLayout_Typesetting, 12);
		}
	}

	FontStyle TextOptions::font_style() const {
		return {_text_weight_value, TextWidth::kDefault, _text_slant_value};
	}

	// ---------------- T e x t . C o n f i g ----------------

	#define Qk_DEFINE_COMPUTE_TEXT_OPTIONS(Type, name, flag) \
		if (_opts->_##name == Type::kInherit) { \
			_opts->_##name##_value = _base_opts->_##name##_value; \
		}

	#define Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(Type, name, flag, Default) \
		if (_opts->_##name.kind == TextValueKind::kInherit) {  \
			_opts->_##name.value = _base_opts->_##name.value; \
		} else if (_opts->_##name.kind == TextValueKind::kDefault) { \
			_opts->_##name.value = Default; \
		}

	TextConfig::TextConfig(TextOptions* opts, TextConfig* base)
		: _opts(opts), _base(base)
	{
		if (_opts->_text_flags || _base->_opts->_text_flags) {
			_opts->_text_flags |= _base->_opts->_text_flags;
			auto _base_opts = _base->_opts;
			auto fontPool = _base_opts->_text_family_Rt.value->pool();
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextWeight, text_weight, 1);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextSlant, text_slant, 2);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextDecoration, text_decoration, 3);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextOverflow, text_overflow, 4);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextWhiteSpace, text_white_space, 5);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextWordBreak, text_word_break, 6);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(float, text_size, 7, 16);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(Color, text_background_color, 8, Color(0, 0, 0, 0));
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(Color, text_color, 9, Color(0, 0, 0));
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(Shadow, text_shadow, 10, (Shadow{ 0, 0, 0, Color(0, 0, 0, 0) }));
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(float, text_line_height, 11, 0);
			Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(FFID, text_family_Rt, 12, (fontPool->defaultFFID()));
		}
	}

	TextConfig::~TextConfig() {
		_opts->_text_flags = 0; // clear flags
	}

	DefaultTextOptions::DefaultTextOptions(FontPool *pool)
		: TextOptions()
		, TextConfig(this, this)
	{
		auto setDefault = [](TextOptions *opts, FontPool *pool) {
			opts->set_text_align(TextAlign::kDefault);
			opts->set_text_weight(TextWeight::kDefault);
			opts->set_text_slant(TextSlant::kDefault);
			opts->set_text_decoration(TextDecoration::kDefault);
			opts->set_text_overflow(TextOverflow::kDefault);
			opts->set_text_white_space(TextWhiteSpace::kDefault);
			opts->set_text_word_break(TextWordBreak::kDefault);
			opts->set_text_background_color({Color(0, 0, 0, 0), TextValueKind::kValue});
			opts->set_text_color({Color(0, 0, 0), TextValueKind::kValue});
			opts->set_text_size({16, TextValueKind::kValue});
			opts->set_text_line_height({0, TextValueKind::kDefault});
			opts->set_text_shadow({{ 0, 0, 0, Color(0, 0, 0, 0) }, TextValueKind::kValue});
			opts->set_text_family({pool->defaultFFID(), TextValueKind::kValue});
		};
		setDefault(&_default, pool); // set base
		setDefault(this, pool); // set self
	}

	void DefaultTextOptions::onTextChange(uint32_t mark, uint32_t type) {
		auto _opts = this;
		auto _base_opts = &_default;
		switch(type) {
			case 0:
				break;
			case 1:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextWeight, text_weight, 1);
				break;
			case 2:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextSlant, text_slant, 2);
				break;
			case 3:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextDecoration, text_decoration, 3);
				break;
			case 4:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextOverflow, text_overflow, 4);
				break;
			case 5:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextWhiteSpace, text_white_space, 5);
				break;
			case 6:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS(TextWordBreak, text_word_break, 6);
				break;
			case 7:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(float, text_size, 7, 16);
				break;
			case 8:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(Color, text_background_color, 8, Color(0, 0, 0, 0));
				break;
			case 9:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(Color, text_color, 9, Color(0, 0, 0));
				break;
			case 10:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(Shadow, text_shadow, 10, (Shadow{ 0, 0, 0, Color(0, 0, 0, 0) }));
				break;
			case 11:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(float, text_line_height, 11, 0);
				break;
			case 12:
				Qk_DEFINE_COMPUTE_TEXT_OPTIONS_2(FFID, text_family, 12, (_base_opts->_text_family.value->pool()->defaultFFID()));
				_text_family_Rt = _text_family;
				break;
		}
		_text_flags = 0; // clear flags
	}

}
