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

#include "./label.h"
#include "../screen.h"
#include "../app.h"

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)

namespace qk {

	static TextLinesCore* newEmptyTextLinesCore() {
		auto rv = NewRetain<TextLinesCore>();
		rv->push({0,0,0,0,0,0,0,false}); // avoid null pointer
		return rv;
	}

	static TextLinesCore* default_textLinesCore() {
		static auto rv = newEmptyTextLinesCore(); // singleton
		return rv;
	};

	Vec2 set_layout_offset_free(Align align, Vec2 hostSize, Vec2 layout_size);

	Label::Label(): _align(Align::Normal)
		, _lines(default_textLinesCore()) // avoid null pointer
	{
	}

	void Label::set_value(String val, bool isRt) {
		_value = val;
		mark_layout(kLayout_Typesetting, isRt);
		// constexpr size_t size = sizeof(Label) + sizeof(TextLinesCore);
	}

	View* Label::getViewForTextOptions() {
		return this;
	}

	void Label::text_config(TextOptions* inherit) {
		resolve_text_config(inherit, this);
	}

	void Label::layout_forward(uint32_t mark) {
		if (mark & kText_Options) {
			Label::text_config(get_closest_text_options());
		}
	}

	void Label::layout_reverse(uint32_t _mark) {
		if (_mark & (kLayout_Inner_Width | kLayout_Inner_Height | kLayout_Typesetting)) {
			_Parent();
			if (_parent->is_text_container()) { // layout text in text container
				_parent->onChildLayoutChange(this, kChild_Layout_Text);
			} else {
				// layout text in non text container
				auto &container = _parent->layout_container(); // use parent container
				TextLines lines(text_align_value(), container.to_range(), container.float_x());
				lines.set_ignore_single_space_line(true);
				layout_text(&lines, nullptr); // no text container, use nullptr
				lines.finish();
			}
			unmark(kLayout_Inner_Width | kLayout_Inner_Height | kLayout_Typesetting);
		}
	}

	void Label::layout_text(TextLines *lines, TextOptions* _) {
		_blob_visible.clear();
		_blob.clear();
		_lines = lines->core();

		String value(_value); // safe hold
		TextBlobBuilder(lines, this, &_blob).make(value);

		auto v = first();
		while(v) {
			if (v->visible()) {
				v->layout_text(lines, this);
			}
			v = v->next();
		}
		mark(kVisible_Region, true);
	}

	Vec2 Label::layout_size() {
		return {_lines->max_width(), _lines->max_height() - _lines->front().start_y};
	}

	float Label::layout_lock_width(float size) {
		return _lines->max_width(); // Temporarily refuse to resize
	}

	float Label::layout_lock_height(float size) {
		return _lines->max_height(); // Temporarily refuse to resize
	}

	void Label::set_align(Align val, bool isRt) {
		if (_align != val) {
			_align = val;
			if (isRt) {
				auto _parent = parent();
				if (_parent)
					_parent->onChildLayoutChange(this, kChild_Layout_Align);
			} else {
				pre_render().async_call([](auto self, auto arg) {
					auto _parent = self->parent();
					if (_parent)
						_parent->onChildLayoutChange(self, kChild_Layout_Align);
				}, this, 0);
			}
		}
	}

	Align Label::layout_align() {
		return _align;
	}

	void Label::set_layout_offset(Vec2 val) {
		_lines->set_layout_offset(val);
		mark(/*kTransform*/kVisible_Region, true);
	}

	void Label::set_layout_offset_free(Vec2 size) {
		auto off = qk::set_layout_offset_free(_align, size, Label::layout_size());
		set_layout_offset(off);
	}

	void Label::solve_visible_area(const Mat &mat) {
		if (_lines) {
			if (!parent()->is_text_container()) // At Label::set_layout_offset_free(), new TextLines()
				_lines->solve_visible_area(this, mat);
			_lines->solve_visible_area_blob(this, &_blob, &_blob_visible);
			_visible_area = _blob_visible.length();
		}
	}

	void Label::onActivate() {
		if (level())
			mark(kTransform, true); // mark recursive transform
		_textFlags = 0xffffffff;
		mark_layout(kText_Options, true);
	}

	TextOptions* Label::asTextOptions() {
		return this;
	}

	bool Label::is_text_container() const {
		return true;
	}

	ViewType Label::view_type() const {
		return kLabel_ViewType;
	}
}
