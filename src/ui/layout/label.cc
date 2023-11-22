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

#include "./label.h"
#include "../screen.h"
#include "../app.h"

namespace qk {

	Label::Label(): View() {}

	void Label::set_text_value(String val) {
		if (_text_value != val) {
			_text_value = std::move(val);
			mark(kLayout_Size_Width | kLayout_Size_Height);
		}
	}

	void Label::onTextChange(uint32_t value) {
		value ? mark(value): mark_render();
	}

	bool Label::layout_forward(uint32_t mark) {
		return false; // continue iteration
	}

	bool Label::layout_reverse(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting)) {
			parent()->onChildLayoutChange(this, kChild_Layout_Text);
			unmark(kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting);
		}
		return true; // complete
	}

	void Label::layout_text(TextLines *lines, TextConfig *base) {
		TextConfig cfg(this, base);

		_blob_visible.clear();
		_blob.clear();
		_lines = lines;

		TextBlobBuilder tbb(lines, this, &_blob);

		tbb.make(_text_value);

		auto v = first();
		while(v) {
			if (v->visible()) {
				v->layout_text(lines, &cfg);
			}
			v = v->next();
		}
	}

	// disable layout matrix prop
	Mat Label::layout_matrix() {
		Vec2 translate = parent()->layout_offset_inside();
		return Mat(
			1, 0, translate.x(),
			0, 1, translate.y()
		);
	}

	void Label::set_layout_offset(Vec2 val) {
		auto size = parent()->layout_size().content_size;
		Sp<TextLines> lines = new TextLines(this, TextAlign::kLeft, size, false); // use left align
		layout_text(*lines, shared_app()->defaultTextOptions());
		lines->finish();
		mark_render(kRecursive_Transform);
	}

	void Label::set_layout_offset_lazy(Vec2 size) {
		Sp<TextLines> lines = new TextLines(this, TextAlign::kLeft, size, false); // use left align
		layout_text(*lines, shared_app()->defaultTextOptions());
		lines->finish();
		mark_render(kRecursive_Transform);
	}

	void Label::onParentLayoutContentSizeChange(Layout* parent, uint32_t value) {
		mark(value);
	}

	bool Label::solve_visible_region() {
		if (_lines->host() == this)
			_lines->solve_visible_region();
		_lines->solve_visible_region_blob(&_blob, &_blob_visible);
		return _blob_visible.length();
	}

	void Label::set_visible(bool val) {
		View::set_visible(val);
		_text_flags = 0xffffffff;
	}

	void Label::set_parent(View *val) {
		View::set_parent(val);
		_text_flags = 0xffffffff;
	}

}
