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

#ifndef __quark_textopts__
#define __quark_textopts__

#include "../types.h"

namespace qk {
	class FontStyle;
	class FontPool;
	class TextConfig;
	class PreRender;
	class View;

	class Qk_EXPORT TextOptions {
	public:
		TextOptions();
		Qk_DEFINE_PROP(TextAlign, text_align, Const);
		Qk_DEFINE_PROP(TextWeight, text_weight, Const);
		Qk_DEFINE_PROP(TextSlant,  text_slant, Const);
		Qk_DEFINE_PROP(TextDecoration, text_decoration, Const);
		Qk_DEFINE_PROP(TextOverflow,   text_overflow, Const);
		Qk_DEFINE_PROP(TextWhiteSpace, text_white_space, Const);
		Qk_DEFINE_PROP(TextWordBreak,  text_word_break, Const);
		Qk_DEFINE_PROP(TextSize,  text_size, Const); // TextValueWrap
		Qk_DEFINE_PROP(TextColor, text_background_color, Const);
		Qk_DEFINE_PROP(TextColor, text_color, Const);
		Qk_DEFINE_PROP(TextLineHeight, text_line_height, Const);
		Qk_DEFINE_PROP(TextShadow, text_shadow, Const);
		Qk_DEFINE_PROP(TextFamily, text_family, Const);
		// compute text final props
		Qk_DEFINE_PROP_GET(TextWeight, text_weight_value, Const); // @safe Rt
		Qk_DEFINE_PROP_GET(TextSlant, text_slant_value, Const); // @safe Rt
		Qk_DEFINE_PROP_GET(TextDecoration, text_decoration_value, Const); // @safe Rt
		Qk_DEFINE_PROP_GET(TextOverflow, text_overflow_value, Const); // @safe Rt
		Qk_DEFINE_PROP_GET(TextWhiteSpace, text_white_space_value, Const); // @safe Rt
		Qk_DEFINE_PROP_GET(TextWordBreak, text_word_break_value, Const); // @safe Rt
		Qk_DEFINE_PROP_ACC_GET(FontStyle, font_style, Const); // @safe Rt

	protected:
		/**
		 * @method onTextChange()
		 * @safe Mt
		 * @note Can only be used in main threads
		*/
		virtual void onTextChange(uint32_t mark, uint32_t type);
		
		/**
		* @method getViewForTextOptions
		*/
		virtual View* getViewForTextOptions();

		/**
		 * @method onTextChange_async
		*/
		void onTextChange_async(uint32_t mark, uint32_t type);

		uint32_t     _text_flags;
		friend class TextConfig;
		friend class DefaultTextOptions;
	};

	class Qk_EXPORT TextConfig {
	public:
		TextConfig(TextOptions* opts, TextConfig* base);
		~TextConfig();
		Qk_DEFINE_PROP_GET(TextOptions*, opts);
		Qk_DEFINE_PROP_GET(TextConfig*,  base);
	};

	class Qk_EXPORT DefaultTextOptions: public TextOptions, public TextConfig {
	public:
		DefaultTextOptions(FontPool *pool);
	protected:
		virtual void onTextChange(uint32_t mark, uint32_t type) override;
		TextOptions _default;
	};

}
#endif
