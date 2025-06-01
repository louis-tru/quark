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

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)

namespace qk {

	void Label::set_value(String val, bool isRt) {
		_value = val;
		mark_layout(kLayout_Size_Width | kLayout_Size_Height, isRt);
	}

	View* Label::getViewForTextOptions() {
		return this;
	}

	void Label::layout_reverse(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting)) {
			_IfParent()
				_parent->onChildLayoutChange(this, kChild_Layout_Text);
			unmark(kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting);
		} else if (mark & kText_Options) {
			unmark(kText_Options);
			_IfParent()
				if (_parent->viewType() == kText_ViewType) {
					_parent->mark_layout(kText_Options, true);
					return;
				}
			text_config(shared_app()->defaultTextOptions());
		}
	}

	void Label::layout_text(TextLines *lines, TextConfig *base) {
		TextConfig cfg(this, base);

		_blob_visible.clear();
		_blob.clear();
		_lines = lines;

		String value(_value); // safe hold
		TextBlobBuilder(lines, this, &_blob).make(value);

		auto v = first();
		while(v) {
			if (v->visible()) {
				v->layout_text(lines, &cfg);
			}
			v = v->next();
		}

		// mark(kTransform, true); // mark recursive transform
		mark(kVisible_Region, true);
	}

	void Label::text_config(TextConfig* base) {
		TextConfig cfg(this, base);
		auto v = first();
		while(v) {
			if (v->visible()) {
				v->text_config(&cfg);
			}
			v = v->next();
		}
	}

	void Label::set_layout_offset(Vec2 val) {
		Sp<TextLines> lines = new TextLines(this, text_align_value(), {0,0}); // no limit
		lines->set_ignore_single_white_space(true);
		layout_text(*lines, shared_app()->defaultTextOptions());
		lines->finish();
		mark(kTransform, true);
	}

	void Label::set_layout_offset_free(Vec2 size) {
		Sp<TextLines> lines = new TextLines(this, text_align_value(), {{}, size});
		lines->set_ignore_single_white_space(true);
		layout_text(*lines, shared_app()->defaultTextOptions());
		lines->finish();
		mark(kTransform, true);
	}

	void Label::solve_visible_region(const Mat &mat) {
		if (_lines) {
			if (_lines->host() == this) // At Label::set_layout_offset_free(), new TextLines()
				_lines->solve_visible_region(mat);
			_lines->solve_visible_region_blob(&_blob, &_blob_visible);
			_visible_region = _blob_visible.length();
		}
	}

	void Label::onActivate() {
		_textFlags = 0xffffffff;
	}

	TextOptions* Label::asTextOptions() {
		return this;
	}

	ViewType Label::viewType() const {
		return kLabel_ViewType;
	}
}
