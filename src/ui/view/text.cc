/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
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

#include "../window.h"
#include "../painter.h"
#include "./text.h"
#include "../text/text_lines.h"
#include "../app.h"

namespace qk {

	void Text::set_value(String val, bool isRt) {
		_value = val;
		mark_layout(kLayout_Typesetting, isRt);
	}

	void Text::text_config(TextOptions* inherit) {
		resolve_text_config(inherit, this);
	}

	void Text::layout_forward(uint32_t mark) {
		if (mark & kText_Options) {
			text_config(getClosestTextOptions()); // config text options first
		}
		Box::layout_forward(mark);
	}

	void Text::layout_reverse(uint32_t mark_) {
		if (mark_ & kLayout_Typesetting) {
			_lines = new TextLines(this, text_align_value(), _container.to_range(), _container.float_x());
			_lines->set_init_line_height(text_size().value, text_line_height().value, false);

			_blob_visible.clear();
			_blob.clear();

			String value(_value); // safe hold
			TextBlobBuilder(*_lines, this, &_blob).make(value);

			auto v = first();
			if (v) {
				do {
					if (v->visible())
						v->layout_text(*_lines, this);
					v = v->next();
				} while(v);
			}
			_lines->finish();

			set_content_size({
				_container.float_x() ? _container.clamp_width(_lines->max_width()): _container.content[0],
				_container.float_y() ? _container.clamp_height(_lines->max_height()): _container.content[1],
			});
			delete_lock_state();
			mark(kVisible_Region, true); // force test region and lines region
			unmark(kLayout_Typesetting);
		}
	}

	View* Text::getViewForTextOptions() {
		return this;
	}

	void Text::solve_visible_area(const Mat &mat) {
		Box::solve_visible_area(mat);
		if (_visible_area && _lines) {
			_lines->solve_visible_area(mat);
			_lines->solve_visible_area_blob(&_blob, &_blob_visible);
		}
	}

	void Text::onActivate() {
		_textFlags = 0xffffffffu; // force all text options flags changed
		mark_layout(kText_Options, true); // force text options resolve
	}

	TextOptions* Text::asTextOptions() {
		return this;
	}

	ViewType Text::viewType() const {
		return kText_ViewType;
	}

}
