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

#include "../views.h"

namespace qk {
	class FontStyle;
	class FontPool;
	class TextConfig;
	class PreRender;
	class View;

	class Qk_EXPORT TextOptions {
	public:
		struct SecondaryProps {
			TextFamily text_family;
			TextShadow text_shadow;
			TextColor  text_background_color;
			TextStroke text_stroke;
			TextWeight text_weight, text_weight_value;
			TextSlant  text_slant, text_slant_value;
			TextDecoration text_decoration, text_decoration_value;
			TextOverflow   text_overflow, text_overflow_value;
			TextWhiteSpace text_white_space, text_white_space_value;
			TextWordBreak  text_word_break, text_word_break_value;
		};
		// main props
		Qk_DEFINE_VIEW_PROPERTY(TextAlign,      text_align, Const);
		Qk_DEFINE_VIEW_PROP_GET(TextAlign,      text_align_value, Const); // @thread Rt
		Qk_DEFINE_VIEW_PROPERTY(TextSize,       text_size, Const);
		Qk_DEFINE_VIEW_PROPERTY(TextColor,      text_color, Const);
		Qk_DEFINE_VIEW_PROPERTY(TextLineHeight, text_line_height, Const);
		// secondary props
		Qk_DEFINE_VIEW_ACCESSOR(TextFamily,     text_family, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextShadow,     text_shadow, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextColor,      text_background_color, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextStroke,     text_stroke, Const); // border
		Qk_DEFINE_VIEW_ACCESSOR(TextWeight,     text_weight, Const);
		Qk_DEFINE_VIEW_ACCE_GET(TextWeight,     text_weight_value, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextSlant,      text_slant, Const);
		Qk_DEFINE_VIEW_ACCE_GET(TextSlant,      text_slant_value, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextDecoration, text_decoration, Const);
		Qk_DEFINE_VIEW_ACCE_GET(TextDecoration, text_decoration_value, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextOverflow,   text_overflow, Const);
		Qk_DEFINE_VIEW_ACCE_GET(TextOverflow,   text_overflow_value, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextWhiteSpace, text_white_space, Const);
		Qk_DEFINE_VIEW_ACCE_GET(TextWhiteSpace, text_white_space_value, Const);
		Qk_DEFINE_VIEW_ACCESSOR(TextWordBreak,  text_word_break, Const);
		Qk_DEFINE_VIEW_ACCE_GET(TextWordBreak,  text_word_break_value, Const);
		Qk_DEFINE_VIEW_ACCE_GET(FontStyle,      font_style, Const);

		Vec2 compute_layout_size(cString& value);

		TextOptions();
		~TextOptions();
	protected:
		/**
		 * @method onTextChange()
		 * @thread Wt
		 * @note Can only be used in main threads
		*/
		virtual void onTextChange(uint32_t mark, uint32_t prop, bool isRt);

		/**
		* @method getViewForTextOptions
		*/
		virtual View* getViewForTextOptions();

		/**
		 * @method getSecondaryProps
		*/
		void initSecondaryProps();

		// fields
		bool             _isHoldSecondaryProps;
		uint32_t         _textFlags; // text props change flags
		SecondaryProps  *_secondaryProps;

		friend class TextConfig;
		friend class DefaultTextOptions;
	};

	class Qk_EXPORT TextConfig {
	public:
		TextConfig(TextOptions *opts, TextConfig *inherit);
		~TextConfig();
		Qk_DEFINE_PROP_GET(TextOptions*, opts);
		Qk_DEFINE_PROP_GET(TextConfig*,  inherit);
	private:
		TextConfig();
		friend class DefaultTextOptions;
	};

	class Qk_EXPORT DefaultTextOptions: public Object, public TextOptions, public TextConfig {
	public:
		DefaultTextOptions(FontPool *pool);
	private:
		void onTextChange(uint32_t mark, uint32_t type, bool isRt) override;
		TextOptions _default;
	};

}
#endif
