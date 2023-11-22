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

#ifndef __quark_textlines__
#define __quark_textlines__

#include "../layout/layout.h"
#include "../../render/font/font.h"

namespace qk {

	class FontMetrics;
	class TextOptions;
	class TextBlob;
	class View;
	class TextBlobBuilder;

	class Qk_EXPORT TextLines: public Reference {
	public:
		struct Line {
			float start_y, end_y, width;
			float baseline, top, bottom, origin;
			uint32_t line;
			bool visible_region;
		};

		struct PreTextBlob {
			Sp<Typeface>    typeface;
			float           text_size;
			float           line_height;
			uint32_t        index_of_unichar;
			Array<TextBlob> *blob;
			Array<GlyphID>  glyphs;
			Array<Vec2>     offset;
		};

		// defines props
		Qk_DEFINE_PROP(float, pre_width);
		Qk_DEFINE_PROP(bool,  trim_start);
		Qk_DEFINE_PROP_GET(bool, no_wrap);
		Qk_DEFINE_PROP_GET(bool, visible_region);
		Qk_DEFINE_PROP_GET(TextAlign, text_align);
		Qk_DEFINE_PROP_GET(Vec2, host_size);
		Qk_DEFINE_PROP_GET(Line*, last);
		Qk_DEFINE_PROP_GET(View*, host);
		Qk_DEFINE_PROP_GET(float, max_width);
		Qk_DEFINE_PROP_GET(float, min_origin);

		// defines methods
		TextLines(View *host, TextAlign text_align, Vec2 host_size, bool no_wrap);
		void lineFeed(TextBlobBuilder* builder, uint32_t index_of_unichar); // push new row
		void push(TextOptions *opts = nullptr, bool trim_start = false); // first call finish() then add new row
		void finish(); // finish all
		void finish_text_blob_pre();
		void set_metrics(float top, float bottom);
		void set_metrics(FontMetricsBase *metrics, float line_height);
		void set_metrics(TextOptions *opts);
		void add_layout(Layout* layout);
		void add_text_blob(PreTextBlob pre, const Array<GlyphID>& glyphs, const Array<Vec2>& offset, bool is_pre);
		void solve_visible_region();
		void solve_visible_region_blob(Array<TextBlob> *blob, Array<uint32_t> *blob_visible);
		uint32_t length() const { return _lines.length(); }
		float max_height() const { return _last->end_y; }
		Line& operator[](uint32_t idx) { return _lines[idx]; }
		Line& line(uint32_t idx) { return _lines[idx]; }
	private:
		void finish_line(); // finish line
		void clear();
		void add_text_blob(PreTextBlob& pre, const Array<GlyphID>& glyphs, const Array<Vec2>& offset);
		void add_text_blob_empty(TextBlobBuilder* builder, uint32_t index_of_unichar);
		Array<Line> _lines;
		Array<Array<Layout*>> _preLayout;
		Array<PreTextBlob>    _preBlob;
	};
}
#endif
