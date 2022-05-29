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

#include "./text_rows.h"
#include "./text_opts.h"
#include "./render/font/font.h"

namespace noug {

	TextRows::TextRows(Vec2 size, bool wrap_x, bool wrap_y, TextAlign text_align)
		: _is_clip(false)
		, _wrap_x(wrap_x), _wrap_y(wrap_y), _size(size), _text_align(text_align)
	{
		clear();
	}

	void TextRows::clear() {
		_rows.clear();
		_rows.push({ 0, 0, 0, 0, 0, 0, 0, 0 });
		_last = &_rows[0];
		_max_width = 0;
		_is_clip = false;
	}

	void TextRows::push() {
		finish();
		_rows.push({ _last->end_y, 0, 0, 0, 0, 0, 0, _rows.length() });
		_last = &_rows.back();
	}

	void TextRows::push(TextConfig *cfg) {
		FontMetrics metrics;
		FontGlyphs::get_metrics(&metrics, cfg->text_family(), cfg->font_style(), cfg->text_size());
		push(); // new row
		set_metrics(&metrics);
	}

	void TextRows::finish() {
		if ( _last->width > _max_width ) {
			_max_width = _last->width;
		}

		switch(_text_align) {
			case TextAlign::LEFT: break;
			case TextAlign::CENTER: _last->origin = (_size.x() - _last->width) / 2; break;
			case TextAlign::RIGHT:  _last->origin = _size.x() - _last->width; break;
		}

		for (auto layout: _rowLayout) {
			auto size = layout->layout_size().layout_size;
			auto offset = layout->layout_offset();
			layout->set_layout_offset(Vec2(_last->origin + offset.x(), _last->baseline - size.y()));
		}
		_rowLayout.clear();
	}

	void TextRows::set_metrics(float ascent, float descent) {
		if (ascent != _last->ascent || descent != _last->descent) {
			_last->ascent = ascent;
			_last->descent = descent;
			_last->baseline = _last->start_y + _last->ascent;
			_last->end_y = _last->baseline + _last->descent;
		}
	}

	void TextRows::set_metrics(FontMetrics *metrics) {
		set_metrics(-metrics->fAscent, metrics->fDescent + metrics->fLeading);
	}

	void TextRows::add_row_layout(Layout* layout) {
		// auto size = layout->layout_size().layout_size;
		// auto align = layout->layout_align();
		// set_metrics(size.y(), 0);
		_rowLayout.push(layout);
	}

	void TextRows::set_is_clip(bool value) {
		_is_clip = value;
	}

}
