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

#include "../window.h"
#include "../app.h"
#include "../view/view.h"
#include "../../render/font/pool.h"
#include "./text_lines.h"
#include "./text_opts.h"
#include "./text_blob.h"
#include "../view/box.h"

namespace qk {

	TextLines::TextLines(View *host, TextAlign text_align, Vec2 limit_size, bool host_wrap_x)
		: _pre_width(0), _ignore_single_white_space(false), _have_init_line_height(false)
		, _limit_size(limit_size), _host(host), _host_wrap_x(host_wrap_x)
		, _text_align(text_align), _visible_region(false)
	{
		clear();
	}

	void TextLines::clear() {
		_lines.clear();
		_lines.push({ 0, 0, 0, 0, 0, 0, 0, 0 });
		_last = &_lines[0];
		_preView.clear();
		_preView.push(Array<View*>());
		_max_width = 0;
		_min_origin = Float32::limit_max;
		_visible_region = false;
	}

	void TextLines::set_init_line_height(float fontSize, float line_height) {
		if (fontSize) {
			_have_init_line_height = true;
			_init_line_height = line_height;
			_host->window()->host()->fontPool()->getMaxMetrics(&_init_Metrics, fontSize);
			set_line_height(&_init_Metrics, line_height);
		} else {
			_have_init_line_height = false;
		}
	}

	void TextLines::lineFeed(TextBlobBuilder* builder, uint32_t index_of_unichar) {
		finish_text_blob_pre(); // force line feed
		add_text_empty_blob(builder, index_of_unichar);
		push(builder->opts()); // new row
	}

	void TextLines::push(TextOptions *opts) {
		finish_line();

		_lines.push({ _last->end_y, _last->end_y, 0, 0, 0, 0, 0, _lines.length() });
		_last = &_lines.back();
		_pre_width = 0;
		_preView.push(Array<View*>());

		for (auto &blob: _preBlob) {
			_pre_width += blob.offset.back().x() - blob.offset.front().x();
		}

		if (_have_init_line_height) {
			set_line_height(&_init_Metrics, _init_line_height);
		} else if (opts) {
			auto tf = opts->text_family().value->match(opts->font_style());
			FontMetricsBase metrics;
			tf->getMetrics(&metrics, opts->text_size().value);
			set_line_height(&metrics, opts->text_line_height().value);
		}
	}

	void TextLines::set_line_height(float top, float bottom) {
		if (top > _last->top) {
			_last->top = top;
			_last->baseline = top + _last->start_y;
			_last->end_y = _last->baseline;
		}
		if (bottom > _last->bottom) {
			_last->bottom = bottom;
			_last->end_y = bottom + _last->baseline;
		}
	}

	void TextLines::set_line_height(FontMetricsBase *metrics, float line_height) {
		auto top = -metrics->fAscent;
		auto bottom = metrics->fDescent + metrics->fLeading;
		auto height = top + bottom;
		if (line_height != 0) { // value, not auto
			if (line_height <= 2) { // use percentage
				if (_limit_size.y() == 0) { // height == wrap y, no limit
					return set_line_height(top, bottom); // use auto
				}
				line_height *= _limit_size.y(); // use percentage
			}
			auto diff = (line_height - height) * 0.5f;
			top += diff;
			bottom += diff;
			if (bottom < 0) {
				top += bottom;
			}
		} // else use default metrics value
		set_line_height(top, bottom);
	}

	void TextLines::finish_line() {
		if ( _last->width > _max_width ) {
			_max_width = _last->width;
		}
		auto top = _last->top;
		auto bottom = _last->bottom;

		for (auto view: _preView.back()) {
			auto height = view->layout_size().layout.height();
			switch (view->layout_align()) {
				case Align::Top:
					set_line_height(top, height - bottom); break;
				case Align::Middle:
					height = (height - top - bottom) / 2;
					set_line_height(height + top, height + bottom); break;
				case Align::Bottom:
					set_line_height(height - bottom, bottom); break;
				default: // bottom and baseline align
					set_line_height(height, 0); break;
			}
		}
	}

	void TextLines::finish() {
		finish_text_blob_pre();
		finish_line();

		auto width = _host_wrap_x ? _max_width: _limit_size.x();

		for (auto &line: _lines) {
			switch(_text_align) {
				default:
				case TextAlign::Left: break;
				case TextAlign::Center: line.origin = (width - line.width) / 2; break;
				case TextAlign::Right:  line.origin = width - line.width; break;
			}
			if ( line.origin < _min_origin) {
				_min_origin = line.origin;
			}

			auto top = line.top;
			auto bottom = line.bottom;

			for (auto view: _preView[line.line]) {
				auto size_y = view->layout_size().layout.y();
				auto x = _last->origin + view->layout_offset().x();
				float y;

				switch (view->layout_align()) {
					case Align::Top:
						y = _last->baseline - top; break;
					case Align::Middle:
						y = _last->baseline - (size_y + top - bottom) / 2; break;
					case Align::Bottom:
						y = _last->baseline - size_y + bottom; break;
					default: // bottom and baseline align
						y = _last->baseline - size_y; break;
				}
				view->set_layout_offset(Vec2(x, y));
			}
		}

		_preView.clear();
	}

	void TextLines::finish_text_blob_pre() {
		if (_preBlob.length()) {
			for (auto& i: _preBlob)
				add_text_blob(i, i.glyphs, i.offset);
			_preBlob.clear();
		}
	}

	void TextLines::add_view(View* view) {
		_preView.back().push(view);
	}

	void TextLines::add_text_blob(PreTextBlob pre, cArray<GlyphID>& glyphs, cArray<Vec2>& offset, bool isPre) {
		if (isPre) {
			if (glyphs.length()) {
				pre.glyphs = glyphs.copy();
				pre.offset = offset.copy();
				_preBlob.push(std::move(pre));
			}
		} else {
			finish_text_blob_pre(); // finish pre
			add_text_blob(pre, glyphs, offset);
		}
	}

	void TextLines::add_text_blob(PreTextBlob& pre, cArray<GlyphID>& glyphs, cArray<Vec2>& offset) {
		if (glyphs.length() == 0)
			return;

		auto frontOffset = -offset.front().x();
		auto line = _last->line;

		if (pre.blobOut->length()) {
			auto& blob = pre.blobOut->back();
			// merge glyphs blob
			if (blob.line == line && pre.typeface == blob.blob.typeface) {
				blob.blob.glyphs.write(glyphs.val(), glyphs.length());
				frontOffset += blob.blob.offset.back().x();
				for (int i = 1; i < offset.length(); i++) {
					blob.blob.offset.push({offset[i].x() + frontOffset, offset[i].y()});
				}
				_last->width = blob.origin + blob.blob.offset.back().x();
				return;
			}
		}

		FontMetricsBase metrics;
		auto height = pre.typeface->getMetrics(&metrics, pre.text_size);
		auto ascent = -metrics.fAscent;
		auto origin = _last->width;

		auto &blob = pre.blobOut->push({
			ascent, height, origin, line, pre.index_of_unichar,
			{pre.typeface, glyphs.copy(), offset},
		}).back();

		if (frontOffset < 0) {
			for (auto &i: blob.blob.offset) {
				i[0] += frontOffset;
			}
		}
		_last->width = origin + blob.blob.offset.back().x();

		// if (!_have_init_line_height)
		set_line_height(&metrics, pre.line_height);
	}

	void TextLines::add_text_empty_blob(TextBlobBuilder* builder, uint32_t index_of_unichar) {
		auto _opts = builder->opts();
		auto _blob = builder->blobOut();
		if (!_blob->length() || _blob->back().line != last()->line) { // empty line
			auto tf = _opts->text_family().value->match(_opts->font_style(), 0);
			FontMetricsBase metrics;
			auto height = tf->getMetrics(&metrics, _opts->text_size().value);
			auto ascent = -metrics.fAscent;
			auto origin = _last->width;

			_blob->push({
				ascent, height, _last->width, _last->line, index_of_unichar, {tf, .offset={Vec2()}},
			});
		}
	}

	void TextLines::solve_visible_region(const Mat &mat) {
		// solve lines visible region
		auto& clip = _host->window()->getClipRegion();
		auto  offset_in = _host->layout_offset_inside();
		auto  x1 = _min_origin + offset_in.x();
		auto  x2 = x1 + _max_width;
		auto  y  = offset_in.y();

		Vec2 vertex[4];

		vertex[0] = mat * Vec2(x1, _lines.front().start_y + y);
		vertex[1] = mat * Vec2(x2, _lines.front().start_y + y);
		
		bool is_all_false = false;

		_visible_region = false;

		// TODO
		// Use optimization algorithm using dichotomy

		for (auto &line: _lines) {
			if (is_all_false) {
				line.visible_region = false;
				continue;
			}
			auto y2 = line.end_y + y;
			vertex[3] = mat * Vec2(x1, y2);
			vertex[2] = mat * Vec2(x2, y2);

			auto re = screen_region_from_convex_quadrilateral(vertex);

			if (Qk_Max( clip.end.y(), re.end.y() ) - Qk_Min( clip.origin.y(), re.origin.y() )
						<= re.end.y() - re.origin.y() + clip.size.y() &&
					Qk_Max( clip.end.x(), re.end.x() ) - Qk_Min( clip.origin.x(), re.origin.x() )
						<= re.end.x() - re.origin.x() + clip.size.x()
			) {
				_visible_region = true;
				line.visible_region = true;
			} else {
				if (_visible_region) is_all_false = true;
				line.visible_region = false;
			}
			vertex[0] = vertex[3];
			vertex[1] = vertex[2];
		}

	}

	void TextLines::solve_visible_region_blob(Array<TextBlob> *blob, Array<uint32_t> *blob_visible) {
		//Qk_DLog("TextLines::solve_visible_region_blob");

		blob_visible->clear();

		if (!visible_region()) {
			return;
		}

		auto& clip = _host->window()->getClipRegion();
		bool is_break = false;

		for (int i = 0, len = blob->length(); i < len; i++) {
			auto &item = (*blob)[i];
			if (item.blob.glyphs.length() == 0)
				continue;
			auto &line = this->line(item.line);
			if (line.visible_region) {
				is_break = true;
				blob_visible->push(i);
			} else {
				if (is_break)
					break;
			}
			//Qk_DLog("blob, origin: %f, line: %d, glyphs: %d, visible: %i",
			//	item.origin, item.line, item.blob.glyphs.length(), line.visible_region);
		}
	}

	void TextLines::set_pre_width(float value) {
		_pre_width = value;
	}

	void TextLines::set_ignore_single_white_space(bool val) {
		_ignore_single_white_space = val;
	}

}
