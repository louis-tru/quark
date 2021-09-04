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

#include "./text.h"
#include "../font/pool.h"

namespace flare {

	void View::Visitor::visitText(Text *v) {
		visitBox(v);
	}

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void Text::accept(Visitor *visitor) {
		visitor->visitText(this);
	}

	DefaultTextSettings::DefaultTextSettings()
		: _text_background_color(Color(0, 0, 0, 0), TextValueType::VALUE)
		, _text_color(Color(0, 0, 0), TextValueType::VALUE)
		, _text_size(16, TextValueType::VALUE)
		, _text_weight(TextWeightValue::REGULAR, TextValueType::VALUE)
		, _text_style(TextStyleValue::NORMAL, TextValueType::VALUE)
		, _text_family(FontPool::get_font_familys_id(String()), TextValueType::VALUE)
		, _text_shadow({ 0, 0, 0, Color(0, 0, 0) }, TextValueType::VALUE)
		, _text_line_height(0, TextValueType::VALUE)
		, _text_decoration(TextDecorationValue::NONE, TextValueType::VALUE)
		, _text_overflow(TextOverflowValue::NORMAL, TextValueType::VALUE)
		, _text_white_space(TextWhiteSpaceValue::NORMAL, TextValueType::VALUE)
	{
	}

	void DefaultTextSettings::set_text_background_color(TextColor value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_background_color = value;
		}
	}
	void DefaultTextSettings::set_text_color(TextColor value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_color = value;
		}
	}
	void DefaultTextSettings::set_text_size(TextSize value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_size = value;
		}
	}
	void DefaultTextSettings::set_text_style(TextStyle value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_style = value;
		}
	}
	void DefaultTextSettings::set_text_family(TextFamily value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_family = value;
		}
	}
	void DefaultTextSettings::set_text_shadow(TextShadow value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_shadow = value;
		}
	}
	void DefaultTextSettings::set_text_line_height(TextLineHeight value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_line_height = value;
		}
	}
	void DefaultTextSettings::set_text_decoration(TextDecoration value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_decoration = value;
		}
	}
	void DefaultTextSettings::set_text_overflow(TextOverflow value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_overflow = value;
		}
	}
	void DefaultTextSettings::set_text_white_space(TextWhiteSpace value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_white_space = value;
		}
	}

}