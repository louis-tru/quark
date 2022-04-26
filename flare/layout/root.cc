/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./root.h"
#include "../effect.h"
#include "../app.inl"
#include "../util/handle.h"
#include "../display.h"
#include "../render/render.h"

namespace flare {

	void __View_SetVisible(View* self, bool val, uint32_t layout_depth);

	void Root::mark_layout_change() {
		auto region = app()->display()->display_region();
		float scale = app()->display()->scale();
		mark(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		set_translate(Vec2(region.x / scale, region.y / scale));
	}

	Root* Root::create() {
		auto app = flare::app();
		F_CHECK(app, "Before you create a root, you need to create a Application");
		Handle<Root> r = new Root();
		r->set_layout_depth(1);
		r->set_receive(1);
		r->set_width({0, BoxSizeType::MATCH});
		r->set_height({0, BoxSizeType::MATCH});
		
		auto region = app->display()->display_region();
		float scale = app->display()->scale();

		r->set_translate(Vec2(region.x / scale, region.y / scale));

		r->mark(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		r->set_fill_color(Color(255, 255, 255, 255)); // 默认白色背景
		r->mark_recursive(kRecursive_Transform);
		_inl_app(app)->set_root(*r);
		return r.collapse();
	}

	Root::Root(){}

	/**
	* @destructor
	*/
	Root::~Root() {
		F_DEBUG("destructor root");
	}

	void Root::set_visible(bool val) {
		if (visible() != val) {
			__View_SetVisible(this, val, val ? 1: 0);
		}
	}

	bool Root::layout_forward(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			Size size = { Vec2(), display()->size(), false, false };
			auto x = solve_layout_content_width(size);
			auto y = solve_layout_content_height(size);
			Vec2 mp(
				margin_left() + margin_right() + padding_left() + padding_right(), 
				margin_top() + margin_bottom() + padding_top() + padding_bottom()
			);
			layout_lock(Vec2(x, y) + mp, &size.wrap_x);
			unmark(kLayout_Size_Width | kLayout_Size_Height);
		}
		return (layout_mark() & kLayout_Typesetting);
	}

	bool Root::layout_reverse(uint32_t mark) {
		if (mark & (kLayout_Typesetting)) {
			auto v = first();
			if (v) {
				Vec2 origin(margin_left() + padding_left(), margin_top() + padding_top());
				Vec2 size = layout_size().content_size;
				if (_border) {
					origin.val[0] += _border->width_left;
					origin.val[1] += _border->width_top;
				}
				while (v) {
					v->set_layout_offset_lazy(origin, size); // lazy layout
					v = v->next();
				}
			}
			unmark(kLayout_Typesetting);
		}
		return false; // stop iteration
	}

	void Root::set_parent(View* parent) {
		F_UNREACHABLE();
	}

}
