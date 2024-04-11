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

#include "./root.h"
#include "../filter.h"
#include "../app.h"
#include "../window.h"
#include "../../util/handle.h"
#include "../../render/render.h"

namespace qk {

	Root::Root(Window *win) {
		set_window(win);
		_level = 1;
		set_receive(true);
		set_width({0, BoxSizeKind::kMatch});
		set_height({0, BoxSizeKind::kMatch});
		mark_layout(View::kLayout_Size_Width | View::kLayout_Size_Height);
		set_background_color(Color(255, 255, 255, 255)); // 默认白色背景
		mark(kRecursive_Transform);
	}

	void Root::reload_Rt() {
		mark_layout(View::kLayout_Size_Width | View::kLayout_Size_Height);
	}

	bool Root::layout_forward(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			auto win = window();
			Size size{ Vec2(), win->size(), false, false };
			Vec2 xy(solve_layout_content_width(size), solve_layout_content_height(size));

			xy += Vec2(margin_left() + margin_right(), margin_top() + margin_bottom());
			xy += Vec2(padding_left() + padding_right(), padding_top() + padding_bottom());
			if (_border) {
				xy += Vec2(
					_border->width[3] + _border->width[1], // left + right
					_border->width[0] + _border->width[2] // top + bottom
				);
			}
			set_layout_size(xy, &size.wrap_x, false);
		}

		if (mark & kLayout_Typesetting) {
			return false;
		} else if (mark & kTransform_Origin) {
			solve_origin_value(); // check transform_origin change
		}

		return true; // complete
	}

	bool Root::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			Vec2 size = content_size();
			auto v = first_Rt();
			while (v) {
				v->set_layout_offset_lazy(size); // lazy view
				v = v->next_Rt();
			}
			unmark(kLayout_Typesetting);

			solve_origin_value(); // check transform_origin change
		}
		return true; // complete iteration
	}

	void Root::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kRecursive_Transform) { // update transform matrix
			unmark(kRecursive_Transform | kRecursive_Visible_Region); // unmark
			_position = layout_offset() + Vec2(margin_left(), margin_top()) + origin_value();
			_matrix = Mat(_position, scale(), -rotate_z(), skew());
			_visible_region = solve_visible_region(_matrix);
			_matrix.set_translate(Vec2(0)); // clear translate, use position value
		}
		else if (mark & kRecursive_Visible_Region) {
			unmark(kRecursive_Visible_Region); // unmark
			_visible_region = solve_visible_region(_matrix.set_translate(_position));
		}
	}

	ViewType Root::viewType() const {
		return kRoot_ViewType;
	}

	bool Root::can_become_focus() {
		return true;
	}

}
