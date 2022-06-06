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

#include "./text_lines.h"
#include "./text_opts.h"
#include "./text_blob.h"
#include "./render/font/font.h"

namespace noug {

	TextLines::TextLines(Vec2 size, bool wrap_x, bool wrap_y, TextAlign text_align)
		: _pre_width(0)
		, _wrap_x(wrap_x), _wrap_y(wrap_y), _size(size), _text_align(text_align)
	{
		clear();
	}

	void TextLines::clear() {
		_lines.clear();
		_lines.push({ 0, 0, 0, 0, 0, 0, 0, 0 });
		_last = &_lines[0];
		_max_width = 0;
	}

	void TextLines::push() {
		finish_line();
		_lines.push({ _last->end_y, 0, 0, 0, 0, 0, 0, _lines.length() });
		_last = &_lines.back();
		_pre_width = 0;
	}

	void TextLines::push(TextOptions *opts) {
		FontMetrics metrics;
		FontGlyphs::get_metrics(&metrics, opts->text_family().value, opts->font_style(), opts->text_size().value);
		push(); // new row
		set_metrics(&metrics);
	}

	void TextLines::finish_line() {
		if ( _last->width > _max_width ) {
			_max_width = _last->width;
		}

		switch(_text_align) {
			case TextAlign::LEFT: break;
			case TextAlign::CENTER: _last->origin = (_size.x() - _last->width) / 2; break;
			case TextAlign::RIGHT:  _last->origin = _size.x() - _last->width; break;
		}

		for (auto layout: _preLayout) {
			auto size = layout->layout_size().layout_size;
			auto offset = layout->layout_offset();
			layout->set_layout_offset(Vec2(_last->origin + offset.x(), _last->baseline - size.y()));
		}
		_preLayout.clear();
	}

	void TextLines::finish() {
		add_text_blob({}, Array<GlyphID>(), Array<float>(), false); // solve text blob
		finish_line();
	}

	void TextLines::set_metrics(float ascent, float descent) {
		if (ascent != _last->ascent || descent != _last->descent) {
			_last->ascent = ascent;
			_last->descent = descent;
			_last->baseline = _last->start_y + _last->ascent;
			_last->end_y = _last->baseline + _last->descent;
		}
	}

	void TextLines::set_metrics(FontMetrics *metrics) {
		set_metrics(-metrics->fAscent, metrics->fDescent + metrics->fLeading);
	}

	void TextLines::add_layout(Layout* layout) {
		// auto size = layout->layout_size().layout_size;
		// auto align = layout->layout_align();
		// set_metrics(size.y(), 0);
		_preLayout.push(layout);
	}

	void TextLines::add_text_blob(PreTextBlob blob, const Array<GlyphID>& glyphs, const Array<float>& offset, bool is_pre) {

		if (is_pre) {
			blob.glyphs = glyphs.copy();
			blob.offset = offset.copy();
			_preBlob.push(std::move(blob));
			return;
		}

		auto add = [&](PreTextBlob& blob, const Array<GlyphID>& glyphs, const Array<float>& offset) {
			auto line = _last->line;

			if (blob.blob->length()) {
				auto& last = blob.blob->back();
				// merge glyphs
				if (last.line == line && last.offset.back() == offset.front()) {
					last.glyphs.write(glyphs);
					last.offset.write(offset, -1, -1, 1);
					_last->width = last.origin + last.offset.back();
					return;
				}
			}

			auto origin  = _last->width - blob.offset[0];
			blob.blob->push({
				blob.typeface, glyphs.copy(), offset.copy(), origin, line });
			_last->width = origin + blob.offset.back();

			FontMetrics metrics;
			FontGlyphs::get_metrics(&metrics, blob.typeface, blob.text_size);
			set_metrics(&metrics);
		};

		if (_preBlob.length()) {
			for (auto& i: _preBlob)
				add(i, i.glyphs, i.offset);
			_preBlob.clear();
		}
		if (glyphs.length()) {
			add(blob, glyphs, offset);
		}
	}

	void TextLines::set_pre_width(float value) {
		_pre_width = value;
	}

}
