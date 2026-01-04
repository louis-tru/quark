/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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

	#define defaultOpts shared_app()->defaultTextOptions()

	static TextOptions::SecondOpts defaultSecondOpts {
		.text_family={ .value=nullptr, .kind=TextValueKind::Inherit },
		.text_shadow={ .kind=TextValueKind::Inherit },
		.text_background_color={ .kind=TextValueKind::Inherit },
		.text_slant=TextSlant::Inherit, .text_slant_value=TextSlant::Normal,
		.text_decoration=TextDecoration::Inherit, .text_decoration_value=TextDecoration::None,
		.text_overflow=TextOverflow::Inherit, .text_overflow_value=TextOverflow::Normal,
		.text_word_break=TextWordBreak::Inherit, .text_word_break_value=TextWordBreak::Normal,
	};

	TextOptions::TextOptions()
		: _text_align(TextAlign::Inherit), _text_align_value(TextAlign::Left)
		, _text_weight(TextWeight::Inherit), _text_weight_value(TextWeight::Regular)
		, _text_white_space(TextWhiteSpace::Inherit), _text_white_space_value(TextWhiteSpace::Normal)
		, _text_size{ .kind=TextValueKind::Inherit, .value=16.0f }
		, _text_color{ .kind=TextValueKind::Inherit, .value=Color(0,0,0) }
		, _text_line_height{ .kind=TextValueKind::Inherit, .value=0.0f }
		, _isHoldSecondOpts(false)
		, _textFlags(0xffffffffu)
		, _second(&defaultSecondOpts)
	{
	}

	TextOptions::~TextOptions() {
		if (_isHoldSecondOpts) {
			Releasep(_second);
		}
	}

	View* TextOptions::getViewForTextOptions() {
		return nullptr;
	}

	void TextOptions::initSecondOpts() {
		if (!_isHoldSecondOpts) {
			_isHoldSecondOpts = true;
			_second = (SecondOpts*)malloc(sizeof(SecondOpts));
			*_second = defaultSecondOpts; // copy default
		}
	}

	// secondary props

	TextFamily TextOptions::text_family() const {
		Qk_ASSERT(_second->text_family.value);
		return _second->text_family;
	}

	TextShadow TextOptions::text_shadow() const {
		return _second->text_shadow;
	}

	TextColor TextOptions::text_background_color() const {
		return _second->text_background_color;
	}

	TextStroke TextOptions::text_stroke() const {
		return _second->text_stroke;
	}

	TextSlant TextOptions::text_slant() const {
		return _second->text_slant;
	}

	TextSlant TextOptions::text_slant_value() const {
		return _second->text_slant_value;
	}

	TextDecoration TextOptions::text_decoration() const {
		return _second->text_decoration;
	}

	TextDecoration TextOptions::text_decoration_value() const {
		return _second->text_decoration_value;
	}

	TextOverflow TextOptions::text_overflow() const {
		return _second->text_overflow;
	}

	TextOverflow TextOptions::text_overflow_value() const {
		return _second->text_overflow_value;
	}

	TextWordBreak TextOptions::text_word_break() const {
		return _second->text_word_break;
	}

	TextWordBreak TextOptions::text_word_break_value() const {
		return _second->text_word_break_value;
	}

	FontStyle TextOptions::font_style() const {
		return {
			_text_weight_value, TextWidth::Normal, _second->text_slant_value,
		};
	}

	// main props
	// -------------------------------------------------------------------------------

	void TextOptions::set_text_align(TextAlign value, bool isRt) {
		if(_text_align != value) {
			_text_align = _text_align_value = value;
			if (value == TextAlign::Default) {
				_text_align_value = shared_app()->defaultTextOptions()->text_align_value();
			}
			onTextChange(View::kLayout_Typesetting, kTextAlign_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_weight(TextWeight value, bool isRt) {
		if (_text_weight != value) {
			_text_weight = _text_weight_value = value;
			if (value == TextWeight::Default) {
				_text_weight_value = defaultOpts->text_weight_value();
			}
			onTextChange(View::kLayout_Typesetting, kTextWeight_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_white_space(TextWhiteSpace value, bool isRt) {
		if (_text_white_space != value) {
			_text_white_space = _text_white_space_value = value;
			if (value == TextWhiteSpace::Default) {
				_text_white_space_value = defaultOpts->text_white_space_value();
			}
			onTextChange(View::kLayout_Typesetting, kTextWhiteSpace_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_size(TextSize value, bool isRt) {
		if (value != _text_size) {
			value.value = Qk_Max(1, value.value);
			_text_size = value;
			if (_text_size.kind == TextValueKind::Default) {
				_text_size.value = defaultOpts->text_size().value;
			}
			onTextChange(View::kLayout_Typesetting, kTextSize_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_color(TextColor value, bool isRt) {
		if (value != _text_color) {
			_text_color = value;
			if (_text_color.kind == TextValueKind::Default) {
				_text_color.value = defaultOpts->text_color().value;
			}
			onTextChange(View::kText_Options, kTextColor_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_line_height(TextLineHeight value, bool isRt) {
		if (value != _text_line_height) {
			value.value = Qk_Max(0, value.value);
			_text_line_height = value;
			if (_text_line_height.kind == TextValueKind::Default) {
				_text_line_height.value = defaultOpts->text_line_height().value;
			}
			onTextChange(View::kLayout_Typesetting, kTextLineHeight_TextOpt, isRt);
		}
	}

	// set secondary props
	// -------------------------------------------------------------------------------

	void TextOptions::set_text_family(TextFamily value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_family) {
			if (!value.value) {
				value.value = shared_fontPool()->defaultFontFamilies();
			}
			// After alignment, `_text_family.value` pointers can be read and written atomically
			_second->text_family = value;
			if (_second->text_family.kind == TextValueKind::Default) {
				_second->text_family.value = defaultOpts->text_family().value;
			}
			onTextChange(View::kLayout_Typesetting, kTextFamily_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_shadow(TextShadow value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_shadow) {
			_second->text_shadow = value;
			if (_second->text_shadow.kind == TextValueKind::Default) {
				_second->text_shadow.value = defaultOpts->text_shadow().value;
			}
			onTextChange(View::kText_Options, kTextShadow_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_background_color(TextColor value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_background_color) {
			_second->text_background_color = value;
			if (_second->text_background_color.kind == TextValueKind::Default) {
				_second->text_background_color.value = defaultOpts->text_background_color().value;
			}
			onTextChange(View::kText_Options, kTextBackgroundColor_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_stroke(TextStroke value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_stroke) {
			_second->text_stroke = value;
			_second->text_stroke.value.width = Qk_Max(0, value.value.width);
			if (_second->text_stroke.kind == TextValueKind::Default) {
				_second->text_stroke.value = defaultOpts->text_stroke().value;
			}
			onTextChange(View::kText_Options, kTextStroke_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_slant(TextSlant value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_slant) {
			_second->text_slant = _second->text_slant_value = value;
			if (value == TextSlant::Default) {
				_second->text_slant_value = defaultOpts->text_slant_value();
			}
			onTextChange(View::kText_Options, kTextSlant_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_decoration(TextDecoration value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_decoration) {
			_second->text_decoration = _second->text_decoration_value = value;
			if (value == TextDecoration::Default) {
				_second->text_decoration_value = defaultOpts->text_decoration_value();
			}
			onTextChange(View::kText_Options, kTextDecoration_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_overflow(TextOverflow value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_overflow) {
			_second->text_overflow = _second->text_overflow_value = value;
			if (value == TextOverflow::Default) {
				_second->text_overflow_value = defaultOpts->text_overflow_value();
			}
			onTextChange(View::kLayout_Typesetting, kTextOverflow_TextOpt, isRt);
		}
	}

	void TextOptions::set_text_word_break(TextWordBreak value, bool isRt) {
		initSecondOpts();
		if (value != _second->text_word_break) {
			_second->text_word_break = _second->text_word_break_value = value;
			if (value == TextWordBreak::Default) {
				_second->text_word_break_value = defaultOpts->text_word_break_value();
			}
			onTextChange(View::kLayout_Typesetting, kTextWordBreak_TextOpt, isRt);
		}
	}

	Vec2 TextOptions::compute_layout_size(cString& value) {
		TextOptions opts(*this); // copy current avoid modify self
		opts.inherit_text_config(shared_app()->defaultTextOptions()); // inherit default
		TextLines lines(opts.text_align_value(), {/*no limit*/}, false);
		Array<TextBlob> blob;
		TextBlobBuilder(&lines, &opts, &blob).make(value);
		lines.finish();
		opts._isHoldSecondOpts = false; // prevent double free
		return Vec2(lines.core()->max_width(), lines.core()->max_height());
	}

	#define Qk_COMPUTE_TEXT_OPTIONS(Type, name, flag, val) \
		if (_opts->_##name == Type::Inherit && \
				_opts->_##val != _inherit->_##val) { \
			_opts->_##val = _inherit->_##val; \
			_opts->_textFlags |= flag##_TextOpt; \
		}

	#define Qk_COMPUTE_TEXT_OPTIONS_Second(Type, name, flag, val) \
		if (_opts->_second->name == Type::Inherit && \
				_opts->_second->val != _inherit->_second->val) { \
			_opts->_second->val = _inherit->_second->val; \
		}

	void TextOptions::inherit_text_config(TextOptions *_inherit) {
		auto _opts = this;
		Qk_COMPUTE_TEXT_OPTIONS(TextAlign, text_align, kTextAlign,text_align_value);
		Qk_COMPUTE_TEXT_OPTIONS(TextWeight, text_weight, kTextWeight,text_weight_value);
		Qk_COMPUTE_TEXT_OPTIONS(TextWhiteSpace, text_white_space, kTextWhiteSpace,text_white_space_value);
		Qk_COMPUTE_TEXT_OPTIONS(TextValueKind, text_size, kTextSize,text_size.value);
		Qk_COMPUTE_TEXT_OPTIONS(TextValueKind, text_color, kTextColor,text_color.value);
		Qk_COMPUTE_TEXT_OPTIONS(TextValueKind, text_line_height, kTextLineHeight,text_line_height.value);
		if (_opts->_isHoldSecondOpts) {
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextSlant, text_slant, kTextSlant,text_slant_value);
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextDecoration, text_decoration, kTextDecoration,text_decoration_value);
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextOverflow, text_overflow, kTextOverflow,text_overflow_value);
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextWordBreak, text_word_break, kTextWordBreak,text_word_break_value);
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_family, kTextFamily,text_family.value);
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_shadow, kTextShadow,text_shadow.value);
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_background_color, kTextBackgroundColor,text_background_color.value);
		Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_stroke, kTextStroke,text_stroke.value);
		} else {
			// inherit all second options directly
			_opts->_second = _inherit->_second;
			// mark second opts changed only
			_opts->_textFlags |= kSecondOpts_Mask_TextOpt & _inherit->_textFlags;
		}
	}

	void TextOptions::resolve_text_config(TextOptions *_inherit, View *host) {
		Qk_ASSERT_EQ(getViewForTextOptions(), host);
		inherit_text_config(_inherit); // settings and inherit options
		// pass text options to subview
		if (_textFlags) {
			auto v = host->first();
			while (v) {
				if (v->visible())
					v->text_config(this); // config subview
				v = v->next();
			}
			if (_textFlags & kLayout_Typesetting_Mask_TextOpt) {
				host->_mark_value |= View::kLayout_Typesetting; // mark typesetting
			}
			_textFlags = 0; // clear flags
		}
		host->unmark(View::kText_Options);
	}

	void TextOptions::onTextChange(uint32_t mark, uint32_t flag, bool isRt) {
		auto view = getViewForTextOptions();
		if (view) {
			if (isRt) {
				_textFlags |= flag;
				view->mark_layout(View::kText_Options, true);
			} else {
				struct Data { uint32_t mark, flag; };
				view->preRender().async_call([](auto self, auto arg) {
					auto view = self->getViewForTextOptions();
					self->_textFlags |= arg.arg.flag;
					view->mark_layout(View::kText_Options, true);
				}, this, Data{mark,flag});
			}
		} else {
			_textFlags |= flag;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////

	DefaultTextOptions::DefaultTextOptions(FontPool *pool)
	{
		auto setDefault = [](TextOptions *opts, FontPool *pool) {
			opts->set_text_align(TextAlign::Left);
			opts->set_text_size({16, TextValueKind::Value});
			opts->set_text_color({Color(0, 0, 0), TextValueKind::Value});
			opts->set_text_line_height({0, TextValueKind::Value});
			opts->set_text_family({pool->defaultFontFamilies(), TextValueKind::Value});
			opts->set_text_shadow({{ 0, 0, 0, Color(0, 0, 0, 0) }, TextValueKind::Value});
			opts->set_text_background_color({Color(0, 0, 0, 0), TextValueKind::Value});
			opts->set_text_stroke({{0,Color(0, 0, 0, 0)}, TextValueKind::Value});
			opts->set_text_weight(TextWeight::Regular);
			opts->set_text_slant(TextSlant::Normal);
			opts->set_text_decoration(TextDecoration::None);
			opts->set_text_overflow(TextOverflow::Normal);
			opts->set_text_white_space(TextWhiteSpace::Normal);
			opts->set_text_word_break(TextWordBreak::Normal);
		};
		setDefault(&_default, pool); // set base
		setDefault(this, pool); // set self
	}

	void DefaultTextOptions::onTextChange(uint32_t mark, uint32_t flag, bool isRt) {
		auto _opts = this;
		auto _inherit = &_default;
		switch(flag) {
			case kTextAlign_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS(TextAlign, text_align, kTextAlign,text_align_value);
				break;
			case kTextSize_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS(TextValueKind, text_size, kTextSize,text_size.value);
				break;
			case kTextColor_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS(TextValueKind, text_color, kTextColor, text_color.value);
				break;
			case kTextLineHeight_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS(TextValueKind, text_line_height, kTextLineHeight, text_line_height.value);
				break;
			case kTextFamily_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_family, kTextFamily, text_family.value);
				break;
			case kTextShadow_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_shadow, kTextShadow, text_shadow.value);
				break;
			case kTextBackgroundColor_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_background_color, kTextBackgroundColor, text_background_color.value);
				break;
			case kTextStroke_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextValueKind, text_stroke, kTextStroke, text_stroke.value);
				break;
			case kTextWeight_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS(TextWeight, text_weight, kTextWeight, text_weight_value);
				break;
			case kTextSlant_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextSlant, text_slant, kTextSlant, text_slant_value);
				break;
			case kTextDecoration_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextDecoration, text_decoration, kTextDecoration, text_decoration_value);
				break;
			case kTextOverflow_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextOverflow, text_overflow, kTextOverflow, text_overflow_value);
				break;
			case kTextWhiteSpace_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS(TextWhiteSpace, text_white_space, kTextWhiteSpace, text_white_space_value);
				break;
			case kTextWordBreak_TextOpt:
				Qk_COMPUTE_TEXT_OPTIONS_Second(TextWordBreak, text_word_break, kTextWordBreak, text_word_break_value);
				break;
		}
		// TODO: update TextOptions views of all windows using default text options ??
		_textFlags = 0; // clear flags
	}
}
