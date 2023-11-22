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

	TextLayout::TextLayout(): Box(), _text_align(TextAlign::kLeft) {}

	void TextLayout::set_text_align(TextAlign value) {
		if(_text_align != value) {
			_text_align = value;
			mark(kLayout_Typesetting);
		}
	}

	bool TextLayout::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting()) return false; // continue iteration

			auto size = content_size();
			auto v = first();
			_lines = new TextLines(this, _text_align, size, layout_wrap_x());

			if (v) {
				TextConfig cfg(this, shared_app()->defaultTextOptions());
				do {
					v->layout_text(*_lines, &cfg);
					v = v->next();
				} while(v);
			}
			_lines->finish();

			Vec2 new_size(
				layout_wrap_x() ? _lines->max_width(): size.x(),
				layout_wrap_y() ? _lines->max_height(): size.y()
			);

			if (new_size != size) {
				set_content_size(new_size);
				parent()->onChildLayoutChange(this, kChild_Layout_Size);
			}

			unmark(kLayout_Typesetting);
			mark_render(kRecursive_Visible_Region); // force test region and lines region

			// check transform_origin change
			// solve_origin_value();
		}

		return true; // complete
	}

	void TextLayout::onTextChange(uint32_t value) {
		value ? mark(value): mark_render();
	}

	bool TextLayout::solve_visible_region() {
		bool ok = Box::solve_visible_region();
		if (ok && _lines) {
			_lines->solve_visible_region();
		}
		return ok;
	}

	void TextLayout::set_visible(bool val) {
		Box::set_visible(val);
		_text_flags = 0xffffffffu;
	}

	void TextLayout::set_parent(View *val) {
		Box::set_parent(val);
		_text_flags = 0xffffffffu;
	}

}
