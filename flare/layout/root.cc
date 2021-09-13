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
#include "../fill.h"
#include "../_app.h"
#include "../util/handle.h"
#include "../display.h"

namespace flare {

	void __View_SetVisible(View* self, bool val, uint32_t layout_depth);

	void View::Visitor::visitRoot(Root *v) {
		visitBox(v);
	}

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void Root::accept(Visitor *visitor) {
		visitor->visitRoot(this);
	}

	Root* Root::create() {
		auto app = flare::app();
		FX_CHECK(app, "Before you create a root, you need to create a Application");
		Handle<Root> r = new Root();
		r->set_layout_depth(1);
		r->set_receive(1);
		r->set_width(SizeValue(0, SizeType::MATCH));
		r->set_height(SizeValue(0, SizeType::MATCH));
		// set_fill(new FillColor(255, 255, 255)); // // 默认白色背景
		r->mark_recursive(M_TRANSFORM);
		_inl_app(app)->set_root(*r);
		return r.collapse();
	}

	/**
	* @destructor
	*/
	Root::~Root() {
		FX_DEBUG("destructor root");
	}

	void Root::set_visible(bool val) {
		if (visible() != val) {
			__View_SetVisible(this, val, val ? 1: 0);
		}
	}

	bool Root::layout_forward(uint32_t mark) {
		if (mark & (M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT)) {
			Size size = { Vec2(), app()->display()->size(), false, false };
			auto x = solve_layout_content_width(size.content_size.x(), &size.wrap_x);
			auto y = solve_layout_content_height(size.content_size.y(), &size.wrap_y);
			Vec2 mp(
				margin_left() + margin_right() + padding_left() + padding_right(), 
				margin_top() + margin_bottom() + padding_top() + padding_bottom()
			);
			layout_lock(Vec2(x, y) + mp, &size.wrap_x);
			unmark(M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT);
		}
		return (layout_mark() & M_LAYOUT_TYPESETTING);
	}

	bool Root::layout_reverse(uint32_t mark) {
		if (mark & (M_LAYOUT_TYPESETTING)) {
			auto v = first();
			Vec2 origin(margin_left() + padding_left(), margin_top() + padding_top());
			Vec2 size = layout_size().content_size;
			while (v) {
				v->set_layout_offset_lazy(origin, size); // lazy layout
				v = v->next();
			}
			unmark(M_LAYOUT_TYPESETTING);
		}
		return false; // stop iteration
	}

	void Root::set_parent(View* parent) {
		FX_UNREACHABLE();
	}

	void Root::draw(SkCanvas* canvas) {
		if (visible() && region_visible()) {
			Box::draw(canvas);
		}
	}

}
