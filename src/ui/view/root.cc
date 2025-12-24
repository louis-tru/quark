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

#include "./root.h"
#include "./free.h"
#include "../filter.h"
#include "../app.h"
#include "../window.h"
#include "../../util/handle.h"
#include "../../render/render.h"

namespace qk {

	Root::Root(Window *win) {
		init(win);
		_level = 1;
		set_receive(true);
		set_free(true); // free layout
		set_width({0, BoxSizeKind::Match});
		set_height({0, BoxSizeKind::Match});
		mark_layout(kLayout_Inner_Width | kLayout_Inner_Height, false);
		set_background_color(Color(255, 255, 255, 255)); // default white background
		mark(kTransform, false);
	}

	void Root::reload_rt() {
		mark_layout(kLayout_Inner_Width | kLayout_Inner_Height, true);
	}

	void Root::layout_forward(uint32_t mark) {
		if (mark & kLayout_Size_ALL) {
			layout_lock_width(window()->size()[0]);
			layout_lock_height(window()->size()[1]);
			_container.state_x = kFixed_FloatState;
			_container.state_y = kFixed_FloatState;
			_container.pre_width = _container.content[0];
			_container.pre_height = _container.content[1];
			unmark(kLayout_Inner_Width | kLayout_Inner_Height);
		}
		Box::layout_forward(mark_value());
	}

	void Root::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			unmark(kTransform | kVisible_Region); // unmark
			solve_origin_value();
			_position = Vec2(margin_left(), margin_top()) + origin_value() + translate();
			_matrix = Mat(_position, scale(), -rotate_z(), skew());
			solve_visible_area(_matrix);
		}
		else if (mark & kVisible_Region) {
			unmark(kVisible_Region); // unmark
			solve_visible_area(_matrix.set_translate(_position));
		}
	}

	ViewType Root::viewType() const {
		return kRoot_ViewType;
	}

	bool Root::can_become_focus() {
		return true;
	}

	void Root::apply_class_all() {
		preRender().async_call([](auto self, auto arg) {
			self->apply_class_all_rt(nullptr, true);
		}, this, 0);
	}

}
