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

#ifndef __quark__text_opts__
#define __quark__text_opts__

#include "./value.h"

namespace quark {

	class FontStyle;
	class FontPool;
	class TextConfig;

	class Qk_EXPORT TextOptions {
	public:
		TextOptions();
		Qk_Define_Prop(TextWeight, text_weight);
		Qk_Define_Prop(TextSlant,  text_slant);
		Qk_Define_Prop(TextDecoration, text_decoration);
		Qk_Define_Prop(TextOverflow,   text_overflow);
		Qk_Define_Prop(TextWhiteSpace, text_white_space);
		Qk_Define_Prop(TextWordBreak,  text_word_break);
		Qk_Define_Prop(TextSize,  text_size); // TextValueWrap
		Qk_Define_Prop(TextColor, text_background_color);
		Qk_Define_Prop(TextColor, text_color);
		Qk_Define_Prop(TextShadow, text_shadow);
		Qk_Define_Prop(TextLineHeight, text_line_height);
		Qk_Define_Prop(TextFamily, text_family);
		// compute text final props
		Qk_Define_Prop_Get(TextWeight, text_weight_value);
		Qk_Define_Prop_Get(TextSlant, text_slant_value);
		Qk_Define_Prop_Get(TextDecoration, text_decoration_value);
		Qk_Define_Prop_Get(TextOverflow, text_overflow_value);
		Qk_Define_Prop_Get(TextWhiteSpace, text_white_space_value);
		Qk_Define_Prop_Get(TextWordBreak, text_word_break_value);
		FontStyle font_style() const;
	protected:
		virtual void onTextChange(uint32_t mark);
		uint32_t     _text_flags;
		friend class TextConfig;
	};

	class Qk_EXPORT TextConfig {
	public:
		TextConfig(TextOptions* opts, TextConfig* base);
		~TextConfig();
		Qk_Define_Prop_Get(TextOptions*, opts);
		Qk_Define_Prop_Get(TextConfig*,  base);
	};

	class Qk_EXPORT DefaultTextOptions: public TextOptions, public TextConfig {
	public:
		DefaultTextOptions(FontPool *pool);
		~DefaultTextOptions();
	};

}
#endif
