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


#ifndef __quark_textblob__
#define __quark_textblob__

#include "../render/font/familys.h"
#include "../render/source.h"
#include "./text_lines.h"
#include "./text_opts.h"

namespace quark {

	// @dev text layout 

	Qk_EXPORT Array<Array<Unichar>> string4_to_unichar(const Unichar *src, uint32_t length,
		bool is_merge_space, bool is_merge_line_feed, bool disable_line_feed);
	Qk_EXPORT Array<Array<Unichar>> string4_to_unichar(cString4& str,
		bool is_merge_space, bool is_merge_line_feed, bool disable_line_feed);
	Qk_EXPORT Array<Array<Unichar>> string_to_unichar(cString& str, TextWhiteSpace space);

	struct TextBlob {
		/**
		 * @enum SizeLevel font size level
		 */
		enum SizeLevel {
			LEVEL_0,        //  0 Invalid
			LEVEL_10,       //  0-10
			LEVEL_12,       // 10-12
			LEVEL_14,       // 12-14
			LEVEL_16,       // 14-16
			LEVEL_18,       // 16-18
			LEVEL_20,       // 18-20
			LEVEL_26,       // 20-26
			LEVEL_32,       // 26-32
			LEVEL_64,       // 32-64
			LEVEL_128,      // 64-128
			LEVEL_256,      // 128-256
			LEVEL_PATH,     // >256
		};
		Sp<Typeface>    typeface;
		Array<GlyphID>  glyphs;
		Array<Vec2>     offset;
		float           ascent; // 当前blob基线距离文本顶部
		float           height; // 当前blob高度
		float           origin; // offset origin start
		uint32_t        line;   // line number
		uint32_t        index;  // blob index in unichar glyphs
		Sp<ImageSource> image;  // image cache
		Sp<PathLine>    path;   // path cache
		SizeLevel       level;
	};

	class Qk_EXPORT TextBlobBuilder {
	public:
		TextBlobBuilder(TextLines *lines, TextOptions *opts, Array<TextBlob>* blob);
		Qk_DEFINE_PROP(bool, disable_overflow);
		Qk_DEFINE_PROP(bool, disable_auto_wrap);
		Qk_DEFINE_PROP(TextLines*, lines);
		Qk_DEFINE_PROP(TextOptions*, opts);
		Qk_DEFINE_PROP(Array<TextBlob>*, blob);
		void make(cString& text);
		void make(Array<Array<Unichar>>& lines);
		void make(Array<Array<Unichar>>&& lines);
	private:
		void as_normal(FontGlyphs &fg, Unichar *unichar, uint32_t index, bool is_BREAK_WORD, bool is_KEEP_ALL);
		void as_break_all(FontGlyphs &fg, Unichar *unichar, uint32_t index);
		void as_no_auto_wrap(FontGlyphs &fg, uint32_t index);
	};

}
#endif
