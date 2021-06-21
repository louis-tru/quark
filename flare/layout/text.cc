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

namespace flare {

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
		: _text_background_color({ TextValueType::VALUE, Color(0, 0, 0, 0) })
		, _text_color({ TextValueType::VALUE, Color(0, 0, 0) })
		, _text_size({ TextValueType::VALUE, 16 })
		, _text_style({ TextValueType::VALUE, TextStyleEnum::REGULAR })
		, _text_family(TextValueType::VALUE, FontPool::get_font_familys_id(String()))
		, _text_shadow({ TextValueType::VALUE, { 0, 0, 0, Color(0, 0, 0) } })
		, _text_line_height({ TextValueType::VALUE, { 0 } })
		, _text_decoration({ TextValueType::VALUE, TextDecorationEnum::NONE })
		, _text_overflow({ TextValueType::VALUE, TextOverflowEnum::NORMAL })
		, _text_white_space({ TextValueType::VALUE, TextWhiteSpaceEnum::NORMAL })
	{
	}

	void DefaultTextSettings::set_text_background_color(TextColor value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_background_color = value;
		}
	}
	void DefaultTextSettings::set_text_color(TextColor value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_color = value;
		}
	}
	void DefaultTextSettings::set_text_size(TextSize value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_size = value;
		}
	}
	void DefaultTextSettings::set_text_style(TextStyle value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_style = value;
		}
	}
	void DefaultTextSettings::set_text_family(TextFamily value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_family = value;
		}
	}
	void DefaultTextSettings::set_text_shadow(TextShadow value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_shadow = value;
		}
	}
	void DefaultTextSettings::set_text_line_height(TextLineHeight value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_line_height = value;
		}
	}
	void DefaultTextSettings::set_text_decoration(TextDecoration value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_decoration = value;
		}
	}
	void DefaultTextSettings::set_text_overflow(TextOverflow value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_overflow = value;
		}
	}
	void DefaultTextSettings::set_text_white_space(TextWhiteSpace value) {
		if ( value.type == TextValueType::VALUE ) {
			_default_text_white_space = value;
		}
	}

}