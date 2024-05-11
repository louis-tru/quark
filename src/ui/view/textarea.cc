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

#include "./textarea.h"
#include "../screen.h"
#include "../app.h"
#include "../window.h"

namespace qk {

	Textarea::Textarea(): Input(), ScrollBase(this) {
	}

	bool Textarea::is_multiline() {
		return true;
	}

	bool Textarea::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting())
				return false; // continue iteration
			auto full_size = layout_typesetting_input_text(); // return full content size
			set_scroll_size_Rt(full_size);
		}
		return true; // complete
	}

	void Textarea::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kInput_Status) {
			auto final_width = content_size().x();
			auto max_width = _lines->max_width();

			// setting default test offset
			if (max_width <= final_width) {
				_input_text_offset_x = 0;
			} else { // max_width > final_width
				switch ( text_align_value() ) {
					default: // left align
						_input_text_offset_x = 0; break;
					case TextAlign::Center:
						_input_text_offset_x = (max_width - final_width) / 2.0; break;
					case TextAlign::Right:
						_input_text_offset_x = max_width - final_width; break;
				}
			}

			unmark(kInput_Status);
			solve_cursor_offset(); // text cursor status

			ScrollBase::solve(mark_value());
			Box::solve_marks(mat, mark_value());

			if (_editing) {
				// update system ime input position
				window()->dispatch()->set_ime_keyboard_spot_rect(input_spot_rect());
			}
		} else {
			ScrollBase::solve(mark);
			Box::solve_marks(mat, mark_value());
		}
	}

	Vec2 Textarea::input_text_offset() {
		return Vec2( -scroll_x() + _input_text_offset_x, -scroll_y() );
	}

	void Textarea::set_input_text_offset(Vec2 value) {
		set_scroll( Vec2(-value.x() + _input_text_offset_x, -value.y()) );
	}

	ScrollBase* Textarea::asScrollBase() {
		return this;
	}

	ViewType Textarea::viewType() const {
		return kTextarea_ViewType;
	}
}
