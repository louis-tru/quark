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

#include "./free.h"

namespace qk {

	Vec2 free_typesetting(View* view, View::Container &container) {
		auto cur = container.content;
		auto v = view->first();
		if (v) {
			if (container.float_x() || container.float_y()) { // float width
				Vec2 maxSize;
				do {
					if (v->visible()) {
						auto size = v->layout_size();
						if (size[0] > maxSize[0])
							maxSize[0] = size[0];
						if (size[1] > maxSize[1])
							maxSize[1] = size[1];
					}
					v = v->next();
				} while(v);
				if (container.float_x())
					cur[0] = container.clamp_width(maxSize[0]);
				if (container.float_y())
					cur[1] = container.clamp_height(maxSize[1]);
			}

			auto v = view->first();
			do { // lazy free layout
				if (v->visible())
					v->set_layout_offset_free(cur); // free layout
				v = v->next();
			} while(v);
		} else {
			if (container.float_x())
				cur[0] = container.clamp_width(0);
			if (container.float_y())
				cur[1] = container.clamp_height(0);
		}
		view->unmark(View::kLayout_Typesetting);
		return cur;
	}

	void Free::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			auto cur = free_typesetting(this, _container);
			set_content_size(cur);
			delete_lock_state();
		}
	}

	ViewType Free::viewType() const {
		return kFree_ViewType;
	}
}
