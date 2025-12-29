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

#ifndef __quark_textlines__
#define __quark_textlines__

#include "../types.h"
#include "../../render/font/families.h"

namespace qk {

	class TextOptions;
	class TextBlob;
	class View;
	class TextBlobBuilder;
	struct TextLine {
		float start_y, end_y, width, line_height;
		float baseline, top, bottom, origin;
		bool visible_area;
	};

	/**
	 * To save memory space, we separate the rendering related data
	 * from the text lines data structure.
	*/
	class TextLinesCore: public Array<TextLine, Allocator, Reference> {
		Qk_DISABLE_COPY(TextLinesCore);
	public:
		typedef TextLine Line;
		Qk_DEFINE_PROP_GET(float, max_width, Const);
		Qk_DEFINE_PROP_GET(float, min_origin, Const);
		Qk_DEFINE_PROP_GET(bool, visible_area, Const);
		Line& last() { return back(); }
		Line& line(int idx) { return at(idx); }
		uint32_t length() const { return length(); }
		uint32_t lineNum() const { return length() - 1; }
		float max_height() const { return back().end_y; }
		void solve_visible_area(View* host, const Mat &mat);
		void solve_visible_area_blob(View* host, Array<TextBlob> *blob, Array<uint32_t> *blob_visible);
		friend class TextLines;
	};

	class Qk_EXPORT TextLines {
	public:
		typedef TextLinesCore::Line Line;
		struct PreTextBlob {
			Sp<Typeface>    typeface;
			float           text_size, line_height;
			uint32_t        index_of_unichar;
			Array<TextBlob> *blobOut;
			Array<GlyphID>  glyphs;
			Array<Vec2>     offset;
		};
		// defines props
		Qk_DEFINE_PROPERTY(float, pre_width, Const);
		Qk_DEFINE_PROPERTY(bool, ignore_single_white_space, Const);
		Qk_DEFINE_PROP_GET(bool, have_init_line_height, Const);
		Qk_DEFINE_PROP_GET(bool, visible_area, Const);
		Qk_DEFINE_PROP_GET(bool, host_float_x, Const);
		Qk_DEFINE_PROP_GET(TextAlign, text_align, Const);
		Qk_DEFINE_PROP_GET(Range, limit_range, Const);
		Qk_DEFINE_PROP_GET(Line*, last);
		Qk_DEFINE_PROP_GET(View*, host);
		Qk_DEFINE_PROP_GET(TextLinesCore*, core);

		// defines methods
		TextLines(View *host, TextAlign text_align, Range limit_range, bool host_float_x);
		void lineFeed(TextBlobBuilder* builder, uint32_t index_of_unichar); // push new row
		void push(TextOptions *opts = nullptr); // first call finish() then add new row
		void finish(); // finish all
		void finish_text_blob_pre();
		void add_view(View* view, float lineWidth);
		void add_text_blob(PreTextBlob pre, cArray<GlyphID>& glyphs, cArray<Vec2>& offset, bool isPre);
		float max_height() const { return _last->end_y; }
		void set_init_line_height(float fontSize, float line_height, bool have_init_line_height);
		void add_text_empty_blob(TextBlobBuilder* builder, uint32_t index_of_unichar);

	private:
		void set_line_height(float top, float bottom);
		void set_line_height(FontMetricsBase *metrics, float line_height);
		void finish_line(); // finish line
		void clear();
		void add_text_blob(PreTextBlob& pre, cArray<GlyphID>& glyphs, cArray<Vec2>& offset);
		Array<Array<View*>> _preView;
		Array<PreTextBlob> _preBlob;
		float _line_height;
		FontMetricsBase _UnitMetrics;
	};
}
#endif
