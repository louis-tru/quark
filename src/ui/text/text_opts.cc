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

#include "../app.h"
#include "./text_opts.h"
#include "./text_lines.h"
#include "./text_blob.h"
#include "../window.h"
#include "../view/view.h"
#include "../../render/font/pool.h"

namespace qk {

	static TextOptions::SecondaryProps defaultSecondaryProps{
		.text_family={ .value=nullptr, .kind=TextValueKind::Inherit },
		.text_shadow={ .kind=TextValueKind::Inherit },
		.text_background_color={ .kind=TextValueKind::Inherit },
		.text_weight=TextWeight::Inherit, .text_weight_value=TextWeight::Default,
		.text_slant=TextSlant::Inherit, .text_slant_value=TextSlant::Default,
		.text_decoration=TextDecoration::Inherit, .text_decoration_value=TextDecoration::Default,
		.text_overflow=TextOverflow::Inherit, .text_overflow_value=TextOverflow::Default,
		.text_white_space=TextWhiteSpace::Inherit, .text_white_space_value=TextWhiteSpace::Default,
		.text_word_break=TextWordBreak::Inherit, .text_word_break_value=TextWordBreak::Default,
	};

	TextOptions::TextOptions()
		: _text_align(TextAlign::Inherit)
		, _text_align_value(TextAlign::Default)
		, _text_size{ .kind=TextValueKind::Inherit }
		, _text_color{ .kind=TextValueKind::Inherit }
		, _text_line_height{ .kind=TextValueKind::Inherit }
		, _isInheritSecondaryProps(true)
		, _textFlags(0xffffffffu)
		, _secondaryProps(&defaultSecondaryProps)
	{
	}

	TextOptions::~TextOptions() {
		if (!_isInheritSecondaryProps) {
			free(_secondaryProps); _secondaryProps = nullptr;
		}
	}

	void TextOptions::onTextChange(uint32_t mark, uint32_t type, bool isRt) {
		auto view = getViewForTextOptions();
		if (view) {
			if (isRt) {
				_textFlags |= (1 << type);
				mark ? view->mark_layout(mark, true): view->mark(0, true);
			} else {
				struct Data { uint32_t mark, type; };
				view->preRender().async_call([](auto self, auto arg) {
					auto view = self->getViewForTextOptions();
					self->_textFlags |= (1 << arg.arg.type);
					arg.arg.mark ? view->mark_layout(arg.arg.mark, true): view->mark(0, true);
				}, this, Data{mark,type});
			}
		} else {
			_textFlags |= (1 << type);
		}
	}

	View* TextOptions::getViewForTextOptions() {
		return nullptr;
	}

	void TextOptions::initSecondaryProps() {
		if (_isInheritSecondaryProps) {
			_isInheritSecondaryProps = false;
			_secondaryProps = (SecondaryProps*)malloc(sizeof(SecondaryProps));
			*_secondaryProps = defaultSecondaryProps;
			_textFlags |= 0b111111110000;
		}
	}

	// main props
	// -------------------------------------------------------------------------------

	void TextOptions::set_text_align(TextAlign value, bool isRt) {
		if(_text_align != value) {
			_text_align = _text_align_value = value;
			onTextChange(View::kLayout_Typesetting, 0, isRt);
		}
	}

	void TextOptions::set_text_size(TextSize value, bool isRt) {
		if (value != _text_size) {
			value.value = Qk_Max(1, value.value);
			_text_size = value;
			onTextChange(View::kLayout_Typesetting, 1, isRt);
		}
	}

	void TextOptions::set_text_color(TextColor value, bool isRt) {
		if (value != _text_color) {
			_text_color = value;
			onTextChange(View::kText_Options, 2, isRt);
		}
	}

	void TextOptions::set_text_line_height(TextLineHeight value, bool isRt) {
		if (value != _text_line_height) {
			value.value = Qk_Max(0, value.value);
			_text_line_height = value;
			onTextChange(View::kLayout_Typesetting, 3, isRt);
		}
	}

	// secondary props
	// -------------------------------------------------------------------------------

	TextFamily TextOptions::text_family() const {
		Qk_ASSERT(_secondaryProps->text_family.value);
		return _secondaryProps->text_family;
	}

	TextShadow TextOptions::text_shadow() const {
		return _secondaryProps->text_shadow;
	}

	TextColor TextOptions::text_background_color() const {
		return _secondaryProps->text_background_color;
	}

	TextWeight TextOptions::text_weight() const {
		return _secondaryProps->text_weight;
	}

	TextWeight TextOptions::text_weight_value() const {
		return _secondaryProps->text_weight_value;
	}

	TextSlant TextOptions::text_slant() const {
		return _secondaryProps->text_slant;
	}

	TextSlant TextOptions::text_slant_value() const {
		return _secondaryProps->text_slant_value;
	}

	TextDecoration TextOptions::text_decoration() const {
		return _secondaryProps->text_decoration;
	}

	TextDecoration TextOptions::text_decoration_value() const {
		return _secondaryProps->text_decoration_value;
	}

	TextOverflow TextOptions::text_overflow() const {
		return _secondaryProps->text_overflow;
	}

	TextOverflow TextOptions::text_overflow_value() const {
		return _secondaryProps->text_overflow_value;
	}

	TextWhiteSpace TextOptions::text_white_space() const {
		return _secondaryProps->text_white_space;
	}

	TextWhiteSpace TextOptions::text_white_space_value() const {
		return _secondaryProps->text_white_space_value;
	}

	TextWordBreak TextOptions::text_word_break() const {
		return _secondaryProps->text_word_break;
	}

	TextWordBreak TextOptions::text_word_break_value() const {
		return _secondaryProps->text_word_break_value;
	}

	FontStyle TextOptions::font_style() const {
		return {
			_secondaryProps->text_weight_value, TextWidth::Default, _secondaryProps->text_slant_value,
		};
	}

	// set secondary props
	// -------------------------------------------------------------------------------

	void TextOptions::set_text_family(TextFamily value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_family) {
			if (!value.value) {
				auto v = getViewForTextOptions();
				if (v) {
					value.value = v->window()->fontPool()->defaultFontFamilies();
				}
			}
			// After alignment, `_text_family.value` pointers can be read and written atomically
			_secondaryProps->text_family = value;
			onTextChange(View::kLayout_Typesetting, 4, isRt);
		}
	}

	void TextOptions::set_text_shadow(TextShadow value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_shadow) {
			_secondaryProps->text_shadow = value;
			onTextChange(View::kText_Options, 5, isRt);
		}
	}

	void TextOptions::set_text_background_color(TextColor value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_background_color) {
			_secondaryProps->text_background_color = value;
			onTextChange(View::kText_Options, 6, isRt);
		}
	}

	void TextOptions::set_text_weight(TextWeight value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_weight) {
			_secondaryProps->text_weight = _secondaryProps->text_weight_value = value;
			onTextChange(View::kLayout_Typesetting, 7, isRt);
		}
	}

	void TextOptions::set_text_slant(TextSlant value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_slant) {
			_secondaryProps->text_slant = _secondaryProps->text_slant_value = value;
			onTextChange(View::kText_Options, 8, isRt);
		}
	}

	void TextOptions::set_text_decoration(TextDecoration value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_decoration) {
			_secondaryProps->text_decoration = _secondaryProps->text_decoration_value = value;
			onTextChange(View::kText_Options, 9, isRt);
		}
	}

	void TextOptions::set_text_overflow(TextOverflow value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_overflow) {
			_secondaryProps->text_overflow = _secondaryProps->text_overflow_value = value;
			onTextChange(View::kLayout_Typesetting, 10, isRt);
		}
	}

	void TextOptions::set_text_white_space(TextWhiteSpace value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_white_space) {
			_secondaryProps->text_white_space = _secondaryProps->text_white_space_value = value;
			onTextChange(View::kLayout_Typesetting, 11, isRt);
		}
	}

	void TextOptions::set_text_word_break(TextWordBreak value, bool isRt) {
		initSecondaryProps();
		if (value != _secondaryProps->text_word_break) {
			_secondaryProps->text_word_break = _secondaryProps->text_word_break_value = value;
			onTextChange(View::kLayout_Typesetting, 12, isRt);
		}
	}

	Vec2 TextOptions::compute_layout_size(cString& value) {
		TextLines lines(getViewForTextOptions(), text_align_value(), {/*no limit*/}, false);
		TextConfig cfg(this, shared_app()->defaultTextOptions());
		Array<TextBlob> blob;
		TextBlobBuilder(&lines, this, &blob).make(value);
		lines.finish();
		return Vec2(lines.max_width(), lines.max_height());
	}

	// ---------------- T e x t . C o n f i g ----------------

	#define Qk_COMPUTE_TEXT_OPTIONS(Type, name, flag) \
		if (_opts->_##name == Type::Inherit) { \
			_opts->_##name##_value = _inherit_opts->_##name##_value; \
		}

	#define Qk_COMPUTE_TEXT_OPTIONS_2(Type, name, flag, _Default) \
		if (_opts->_##name.kind == TextValueKind::Inherit) {  \
			_opts->_##name.value = _inherit_opts->_##name.value; \
		} else if (_opts->_##name.kind == TextValueKind::Default) { \
			_opts->_##name.value = _Default; \
		}

	#define Qk_COMPUTE_TEXT_OPTIONS_Secondary(Type, name, flag) \
		if (_opts->_secondaryProps->name == Type::Inherit) { \
			_opts->_secondaryProps->name##_value = _inherit_opts->_secondaryProps->name##_value; \
		}

	#define Qk_COMPUTE_TEXT_OPTIONS_2_Secondary(Type, name, flag, _Default) \
		if (_opts->_secondaryProps->name.kind == TextValueKind::Inherit) {  \
			_opts->_secondaryProps->name.value = _inherit_opts->_secondaryProps->name.value; \
		} else if (_opts->_secondaryProps->name.kind == TextValueKind::Default) { \
			_opts->_secondaryProps->name.value = _Default; \
		}

	TextConfig::TextConfig(TextOptions* opts, TextConfig* inherit)
		: _opts(opts), _inherit(inherit)
	{
		if (_opts->_textFlags || _inherit->_opts->_textFlags) {
			_opts->_textFlags |= _inherit->_opts->_textFlags;
			auto _inherit_opts = _inherit->_opts;
			Qk_COMPUTE_TEXT_OPTIONS(TextAlign, text_align, 0);
			Qk_COMPUTE_TEXT_OPTIONS_2(float, text_size, 1, 16);
			Qk_COMPUTE_TEXT_OPTIONS_2(Color, text_color, 2, Color(0, 0, 0));
			Qk_COMPUTE_TEXT_OPTIONS_2(float, text_line_height, 3, 0);
			if (_opts->_isInheritSecondaryProps) {
				_opts->_secondaryProps = _inherit_opts->_secondaryProps;
			} else {
				Qk_COMPUTE_TEXT_OPTIONS_2_Secondary(FFID,
					text_family, 4, (_inherit_opts->text_family().value->pool()->defaultFontFamilies()));
				Qk_COMPUTE_TEXT_OPTIONS_2_Secondary(Shadow,
					text_shadow, 5, (Shadow{ 0, 0, 0, Color(0, 0, 0, 0) }));
				Qk_COMPUTE_TEXT_OPTIONS_2_Secondary(Color, text_background_color, 6, Color(0, 0, 0, 0));
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextWeight, text_weight, 7);
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextSlant, text_slant, 8);
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextDecoration, text_decoration, 9);
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextOverflow, text_overflow, 10);
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextWhiteSpace, text_white_space, 11);
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextWordBreak, text_word_break, 12);
			}
		}
	}

	TextConfig::TextConfig() {}

	TextConfig::~TextConfig() {
		_opts->_textFlags = 0; // clear flags
	}

	DefaultTextOptions::DefaultTextOptions(FontPool *pool)
	{
		auto setDefault = [](TextOptions *opts, FontPool *pool) {
			opts->set_text_align(TextAlign::Default);
			opts->set_text_size({16, TextValueKind::Value});
			opts->set_text_color({Color(0, 0, 0), TextValueKind::Value});
			opts->set_text_line_height({0, TextValueKind::Value});
			opts->set_text_family({pool->defaultFontFamilies(), TextValueKind::Value});
			opts->set_text_shadow({{ 0, 0, 0, Color(0, 0, 0, 0) }, TextValueKind::Value});
			opts->set_text_background_color({Color(0, 0, 0, 0), TextValueKind::Value});
			opts->set_text_weight(TextWeight::Default);
			opts->set_text_slant(TextSlant::Default);
			opts->set_text_decoration(TextDecoration::Default);
			opts->set_text_overflow(TextOverflow::Default);
			opts->set_text_white_space(TextWhiteSpace::Default);
			opts->set_text_word_break(TextWordBreak::Default);
		};
		setDefault(&_default, pool); // set base
		setDefault(this, pool); // set self
		_opts = this;
		_inherit = this;
	}

	void DefaultTextOptions::onTextChange(uint32_t mark, uint32_t type, bool isRt) {
		auto _opts = this;
		auto _inherit_opts = &_default;
		switch(type) {
			case 0:
				Qk_COMPUTE_TEXT_OPTIONS(TextAlign, text_align, 0);
				break;
			case 1:
				Qk_COMPUTE_TEXT_OPTIONS_2(float, text_size, 1, 16);
				break;
			case 2:
				Qk_COMPUTE_TEXT_OPTIONS_2(Color, text_color, 2, Color(0, 0, 0));
				break;
			case 3:
				Qk_COMPUTE_TEXT_OPTIONS_2(float, text_line_height, 3, 0);
				break;
			case 4:
				Qk_COMPUTE_TEXT_OPTIONS_2_Secondary(FFID,
					text_family, 4, (
						_inherit_opts->text_family().value->pool()->defaultFontFamilies()
					)
				);
				break;
			case 5:
				Qk_COMPUTE_TEXT_OPTIONS_2_Secondary(Shadow,
					text_shadow, 5, (Shadow{ 0, 0, 0, Color(0, 0, 0, 0) })
				);
				break;
			case 6:
				Qk_COMPUTE_TEXT_OPTIONS_2_Secondary(Color, text_background_color, 6, Color(0, 0, 0, 0));
				break;
			case 7:
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextWeight, text_weight, 7);
				break;
			case 8:
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextSlant, text_slant, 8);
				break;
			case 9:
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextDecoration, text_decoration, 9);
				break;
			case 10:
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextOverflow, text_overflow, 10);
				break;
			case 11:
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextWhiteSpace, text_white_space, 11);
				break;
			case 12:
				Qk_COMPUTE_TEXT_OPTIONS_Secondary(TextWordBreak, text_word_break, 12);
				break;
		}
		_textFlags = 0; // clear flags
	}
}
