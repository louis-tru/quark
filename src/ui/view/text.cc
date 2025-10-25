/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include "./text.h"
#include "../text/text_lines.h"
#include "../app.h"

namespace qk {

	void Text::set_value(String val, bool isRt) {
		_value = val;
		mark_layout(kLayout_Typesetting, isRt);
	}

	void Text::layout_reverse(uint32_t mark_) {
		if (mark_ & kLayout_Typesetting) {

			TextConfig cfg(this, shared_app()->defaultTextOptions());
			_lines = new TextLines(this, text_align_value(), _container.to_range(), _container.float_x());
			_lines->set_init_line_height(text_size().value, text_line_height().value, false);

			_blob_visible.clear();
			_blob.clear();

			TextBlobBuilder tbb(*_lines, this, &_blob);

			String value(_value); // safe hold

			tbb.make(value);

			auto v = first();
			if (v) {
				do {
					if (v->visible())
						v->layout_text(*_lines, &cfg);
					v = v->next();
				} while(v);
			}
			_lines->finish();

			set_content_size({
				_container.float_x() ? _container.clamp_width(_lines->max_width()): _container.content[0],
				_container.float_y() ? _container.clamp_height(_lines->max_height()): _container.content[1],
			});
			delete_lock_state();
			unmark(kLayout_Typesetting | kText_Options);
			mark(kVisible_Region, true); // force test region and lines region
		}
		else if (mark_ & kText_Options) {
			TextConfig cfg(this, shared_app()->defaultTextOptions());
			auto v = first();
			while(v) {
				v->text_config(&cfg);
				v = v->next();
			}
			unmark(kText_Options);
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
		_textFlags = 0xffffffffu;
	}

	TextOptions* Text::asTextOptions() {
		return this;
	}

	ViewType Text::viewType() const {
		return kText_ViewType;
	}

}
