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

#ifndef __noug__text_lines__
#define __noug__text_lines__

#include "./layout/layout.h"

namespace noug {

	class FontMetrics;
	class TextOptions;
	class TextBlob;

	class N_EXPORT TextLines: public Reference {
	public:
		struct Line {
			float start_y, end_y, width;
			float baseline, ascent, descent, origin;
			uint32_t line;
		};
		struct PreTextBlob {
			Typeface        typeface;
			float           text_size;
			Array<TextBlob> *blob;
			Array<GlyphID>  glyphs;
			Array<float>    offset;
		};
		TextLines(Vec2 size, bool wrap_x, bool wrap_y, TextAlign text_align);
		void push(); // first call finish() then add new row
		void push(TextOptions *opts); // push new row
		void finish(); // finish all
		void set_metrics(float ascent, float descent);
		void set_metrics(FontMetrics *metrics);
		void add_layout(Layout* layout);
		void add_text_blob(PreTextBlob blob,
				const Array<GlyphID>& glyphs, const Array<float>& offset, bool is_pre);
		inline uint32_t length() const { return _lines.length(); }
		inline float max_height() const { return _last->end_y; }
		inline Line& operator[](uint32_t idx) { return _lines[idx]; }
		// defines props
		N_DEFINE_PROP(float, pre_width);
		N_DEFINE_PROP_READ(bool, wrap_x);
		N_DEFINE_PROP_READ(bool, wrap_y);
		N_DEFINE_PROP_READ(Vec2, size);
		N_DEFINE_PROP_READ(TextAlign, text_align);
		N_DEFINE_PROP_READ(Line*,  last);
		N_DEFINE_PROP_READ(float, max_width);
	private:
		void finish_line(); // finish line
		void clear();
		Array<Line> _lines;
		Array<Layout*> _preLayout;
		Array<PreTextBlob> _preBlob;
	};
}
#endif
