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

#include "./float.h"

namespace qk {

	bool Float::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting()) return false; // continue iteration
			layout_typesetting_float();
		}
		return true; // complete iterations
	}

	Vec2 Float::layout_typesetting_float() {
		Vec2 full_size;

		auto v = first_Rt();
		if (v) {
			auto size = content_size();
			auto size_x = size.x();

			if ( _layout_wrap_x_Rt ) { // wrap width
				size_x = 0;
				do {
					if (v->visible()) {
						size_x = Float32::max(size_x, v->layout_size().layout_size.x());
					}
					v = v->next_Rt();
				} while(v);
				v = first_Rt();
			}

			float offset_left = 0, offset_right = 0;
			float offset_y = 0;
			float line_width = 0, max_width = 0;
			float line_height = 0;

			do {
				if (v->visible()) {
					auto size = v->layout_size().layout_size;
					auto align = v->layout_align();

					if (align == Align::Auto) { // new line
						offset_y += line_height;
						v->set_layout_offset(Vec2(0, offset_y));
						offset_left = offset_right = 0;
						line_width = line_height = 0;
						offset_y += size.y();
					} else {
						auto new_width = line_width + size.x();

						if (size.y() > line_height) {
							line_height = size.y(); // select max
						}
						if (new_width > size_x && line_width != 0) { // new line
							if (line_width > max_width) {
								max_width = line_width; // select max
							}
							offset_left = offset_right = 0;
							offset_y += line_height;
							line_width = size.x();
							line_height = size.y();
						} else {
							line_width = new_width;
						}

						switch (align) {
							case Align::LeftTop:
							case Align::LeftCenter:
							case Align::LeftBottom: // left
								v->set_layout_offset(Vec2(offset_left, offset_y));
								offset_left += size.x();
								break;
							default: // right
								v->set_layout_offset(Vec2(size_x - offset_right - size.x(), offset_y));
								offset_right += size.x();
								break;
						}
					}
				}
				v = v->next_Rt();
			} while(v);

			full_size = Vec2(Float32::max(max_width, line_width), offset_y + line_height);

			Vec2 new_size(
				_layout_wrap_x_Rt ? solve_layout_content_wrap_limit_width(full_size.x()): size.x(),
				_layout_wrap_y_Rt ? solve_layout_content_wrap_limit_height(full_size.y()): size.y()
			);

			if (new_size != size) {
				set_content_size(new_size);
				parent_Rt()->onChildLayoutChange(this, kChild_Layout_Size);
			}
		}

		unmark(kLayout_Typesetting);

		// check transform_origin change
		// solve_origin_value();

		return full_size;
	}

	ViewType Float::viewType() const {
		return kFloat_ViewType;
	}

}
