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

#include "./layout.h"
#include "./root.h"
#include "../window.h"
#include "../app.h"

namespace qk {

	Layout::Layout(Window *win)
		: _mark_value(kLayout_None)
		, _mark_index(-1)
		, _level(0)
		, _window(win)
		, _view(nullptr)
		, _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _accessor(nullptr)
		, _opacity(1.0)
		, _visible(true)
		, _visible_region(false)
		, _receive(true)
	{
		Qk_ASSERT(win);
	}

	Layout::~Layout() {
	}

	void Layout::set_receive(bool val) {
		_receive = val;
	}

	void Layout::set_visible(bool val) {
		if (_visible != val) {
			#define is_root (_window->root()->layout() == this)
			set_visible_(val, _parent && _parent->_level ? _parent->_level + 1: val && is_root ? 1: 0);
		}
	}

	void Layout::set_opacity(float val) {
		if (_opacity != val) {
			val = Qk_MAX(0, Qk_MIN(val, 1));
			if (_opacity != val)
				mark_render(); // mark render
		}
	}

	// @layout
	// ------------------------------------------------------------------------------------------

	void Layout::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kRecursive_Transform) { // update transform matrix
			unmark(kRecursive_Transform | kRecursive_Visible_Region); // unmark
			_position =
				mat.mul_vec2_no_translate(layout_offset() + _parent->layout_offset_inside()) + 
				_parent->_position;
			_visible_region = solve_visible_region(Mat(mat).set_translate(_position));
		} else if (mark & kRecursive_Visible_Region) {
			unmark(kRecursive_Visible_Region); // unmark
			_visible_region = solve_visible_region(Mat(mat).set_translate(_position));
		}
	}

	Vec2 Layout::center() {
		return Vec2();
	}

	bool Layout::solve_visible_region(const Mat &mat) {
		return true;
	}

	bool Layout::overlap_test(Vec2 point) {
		return false;
	}

	TextInput* Layout::asTextInput() {
		return nullptr;
	}

	TransformLayout* Layout::asTransform() {
		return nullptr;
	}

	TextOptions* Layout::asTextOptions() {
		return nullptr;
	}

	ScrollLayoutBase* Layout::asScrollLayoutBase() {
		return nullptr;
	}

	float Layout::layout_weight() {
		return 0;
	}

	Align Layout::layout_align() {
		return Align::kAuto;
	}

	Vec2 Layout::layout_offset() {
		return Vec2();
	}

	Layout::Size Layout::layout_size() {
		return { Vec2(), Vec2(), true, true };
	}

	Layout::Size Layout::layout_raw_size(Size parent_content_size) {
		return { Vec2(), Vec2(), true, true };
	}

	Vec2 Layout::layout_offset_inside() {
		return Vec2();
	}

	void Layout::set_layout_offset(Vec2 val) {
	}

	void Layout::set_layout_offset_lazy(Vec2 size) {
	}

	Vec2 Layout::layout_lock(Vec2 layout_size) {
		return Vec2();
	}

	bool Layout::is_lock_child_layout_size() {
		return false;
	}

	bool Layout::layout_forward(uint32_t mark) {
		return !(mark & kLayout_Typesetting);
	}

	bool Layout::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			auto v = _first;
			while (v) {
				v->set_layout_offset_lazy(Vec2()); // lazy layout
				v = v->_next;
			}
			unmark(kLayout_Typesetting | kLayout_Size_Width | kLayout_Size_Height);
		}
		return true; // complete
	}

	void Layout::layout_text(TextLines *lines, TextConfig *cfg) {
	}

	void Layout::onChildLayoutChange(Layout *child, uint32_t value) {
		if (value & (kChild_Layout_Size | kChild_Layout_Visible | kChild_Layout_Align | kChild_Layout_Text)) {
			mark_layout(kLayout_Typesetting);
		}
	}

	void Layout::onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) {
	}

	void Layout::onActivate() {
	}

	void Layout::mark_layout(uint32_t mark) {
		_mark_value |= mark;
		if (_mark_index < 0) {
			if (_level) {
				_window->preRender().mark_layout(this, _level); // push to pre render
			}
		}
	}

	void Layout::mark_render(uint32_t mark) {
		_mark_value |= mark;
		if (_level) {
			_window->preRender().mark_render(); // push to pre render
		}
	}

	bool Layout::is_clip() {
		return false;
	}

	Sp<View> Layout::safe_view() {
		if (_view) {
			std::lock_guard<RecursiveMutex> lock(_window->dispatch()->_view_mutex);
			return _view;
		} else {
			return nullptr;
		}
	}

	TransformLayout* Layout::transform() {
		auto *v = this;
		do {
			auto t = v->asTransform();
			if (t)
				return t;
			v = v->_parent;
		} while(v);
		return nullptr;
	}

	ViewType Layout::viewType() const {
		return kView_ViewType;
	}

	// @private
	// --------------------------------------------------------------------------------------

	void Layout::before(Layout *view) {
		if (view == this) return;
		if (_parent) {
			if (view->_parent == _parent) {
				view->clear_link();  // clear link
			} else {
				view->set_parent(_parent);
			}
			if (_prev) {
				_prev->_next = view;
			} else { // There are no brothers on top
				_parent->_first = view;
			}
			view->_prev = _prev;
			view->_next = this;
			_prev = view;
		}
	}

	void Layout::after(Layout *view) {
		if (view == this) return;
		if (_parent) {
			if (view->_parent == _parent) {
				view->clear_link(); // clear link
			} else {
				view->set_parent(_parent);
			}
			if (_next) {
				_next->_prev = view;
			} else { // There are no brothers below
				_parent->_last = view;
			}
			view->_prev = this;
			view->_next = _next;
			_next = view;
		}
	}

	void Layout::prepend(Layout *child) {
		if (this == child->_parent) {
			child->clear_link();
		} else {
			child->set_parent(this);
		}
		if (_first) {
			child->_prev = nullptr;
			child->_next = _first;
			_first->_prev = child;
			_first = child;
		} else { // There are currently no sub views available yet
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
	}

	void Layout::append(Layout *child) {
		if (this == child->_parent) {
			child->clear_link();
		} else {
			child->set_parent(this);
		}
		if (_last) {
			child->_prev = _last;
			child->_next = nullptr;
			_last->_next = child;
			_last = child;
		} else { // There are currently no sub views available yet
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
	}

	void Layout::remove() {
		if (_parent) {
			clear_link();
			_parent = nullptr;
			if (_level) {
				clear_level();
			}
		}
	}

	void Layout::clear_link() { // Cleaning up associated view information
		if (_parent) {
			/* Currently the first sub view */
			if (_parent->_first == this) {
				_parent->_first = _next;
			} else {
				_prev->_next = _next;
			}
			/* Currently the last sub view */
			if (_parent->_last == this) {
				_parent->_last = _prev;
			} else {
				_next->_prev = _prev;
			}
		}
	}

	void Layout::set_visible_(bool visible, uint32_t level) {
		_visible = visible;
		if (visible && level) {
			if (_level != level)
				set_level_(level);
		} else { // set level = 0
			if (_level)
				clear_level();
		}
		if (_parent) {
			_parent->onChildLayoutChange(this, kChild_Layout_Visible); // mark parent layout 
		}
		if (visible) {
			mark_layout(kLayout_Size_Width | kLayout_Size_Height); // reset layout size
		}
	}

	void Layout::clear_level() { //  clear layout depth
		if (_view && _view == _window->dispatch()->focus_view()) {
			auto view = safe_view();
			auto v = *view;
			if (v && v->is_focus()) {
				_window->host()->loop()->post(Cb([v](auto &e){
					v->blur();
				}, v));
			}
		}
		if (_mark_index >= 0) {
			_window->preRender().unmark_layout(this, _level);
		}
		_level = 0;
		onActivate();
		auto v = _first;
		while ( v ) {
			v->clear_level();
			v = v->_next;
		}
	}

	void Layout::set_level_(uint32_t level) { // settings level
		if (_visible) {
			// if level > 0 then
			if (_mark_index >= 0) {
				_window->preRender().unmark_layout(this, _level);
				_window->preRender().mark_layout(this, level);
			}
			_level = level++;
			onActivate();

			auto v = _first;
			while ( v ) {
				v->set_level_(level);
				v = v->_next;
			}
		} else {
			if ( _level )
				clear_level();
		}
	}

	void Layout::set_parent(Layout *parent) {
		if (parent != _parent) {
			clear_link();

			if ( _parent ) {
				_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			}
			_parent = parent;

			auto level = parent->_level;

			if (_visible && level) {
				if (_level != ++level)
					set_level_(level);
			} else {
				if (_level)
					clear_level();
			}
			_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			mark_layout(kLayout_Size_Width | kLayout_Size_Height); // mark layout size, reset layout size

			onActivate();
		}
	}

}
