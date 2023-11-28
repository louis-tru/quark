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

#include "./view.h"
#include "../window.h"
#include "./root.h"
#include <math.h>

namespace qk {

	class UILocks {
		struct UILock_Inl {
			Window *win;
			bool lock;
		};
		UILock_Inl _l0,_l1;
	public:
		UILocks(View *v0, View *v1): _l0{nullptr}, _l1{nullptr} {
			if (v0 && v0->window()) {
				new(&_l0)UILock(v0->window());
			} 
			if (v1 && v1->window() && v1->window() != _l0.win) {
				new(&_l1)UILock(v1->window());
			}
		}
		~UILocks() {
			if (_l0.win) {
				((UILock*)&_l0)->unlock();
			}
			if (_l1.win) {
				((UILock*)&_l1)->unlock();
			}
		}
	};

	View::View()
		: Notification<UIEvent, UIEventName, Layout>()
		, _action(nullptr), _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _opacity(1.0)
		, _visible(true)
		, _visible_region(false)
	{}

	View::~View() {
		UILocks lock(this, nullptr);
		Qk_ASSERT(_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
		set_action(nullptr); // del action
		remove_all_child_(); // 删除子视图
	}

	bool View::is_self_child(View *child) {
		if ( child ) {
			auto parent = child->_parent;
			while (parent) {
				if ( parent == this ) {
					return true;
				}
				parent = parent->_parent;
			}
		}
		return false;
	}

	void View::before(View *view) {
		if (view == this) return;
		if (_parent) {
			UILocks lock(this, view);
			if (view->_parent == _parent) {
				view->clear_link();  // 清除关联
			} else {
				view->set_parent(_parent);
			}
			if (_prev) {
				_prev->_next = view;
			} else { // 上面没有兄弟
				_parent->_first = view;
			}
			view->_prev = _prev;
			view->_next = this;
			_prev = view;
		}
	}

	void View::after(View *view) {
		if (view == this) return;
		if (_parent) {
			UILocks lock(this, view);
			if (view->_parent == _parent) {
				view->clear_link(); // 清除关联
			} else {
				view->set_parent(_parent);
			}
			if (_next) {
				_next->_prev = view;
			} else { // 下面没有兄弟
				_parent->_last = view;
			}
			view->_prev = this;
			view->_next = _next;
			_next = view;
		}
	}

	void View::prepend(View *child) {
		UILocks lock(this, child);
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
		} else { // 当前还没有子视图
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
	}

	void View::append(View *child) {
		UILocks lock(this, child);
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
		} else { // 当前还没有子视图
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
	}

	void View::remove() {
		if (_parent) {
			UILocks lock(this, nullptr);
			clear_link();
			_parent = nullptr;
			clear_level(nullptr);
			release(); // Disconnect from parent view strong reference
		}
	}

	void View::remove_all_child() {
		UILocks lock(this, nullptr);
		remove_all_child_();
	}

	void View::set_visible(bool val) {
		if (_visible != val) {
			set_visible_(val, _parent && _parent->_level ?
				_parent->_level + 1: is_root() && val ? 1: 0);
		}
	}

	void View::set_action(Action* val) {
		if (_action != val) {
			// TODO ...
			if (_action) {
				// unload _action
				_action = nullptr;
			}
			if (val) {
				_action = val;
				// load new action
			}
		}
	}

	void View::set_opacity(float val) {
		if (_opacity != val) {
			_opacity = Qk_MAX(0, Qk_MIN(val, 1));
			mark_layout(kLayout_None); // mark none
		}
	}

	void View::set_is_focus(bool value) {
		if ( value ) {
			focus();
		} else {
			blur();
		}
	}

	bool View::is_focus() const {
		return _window && _window->dispatch()->focus_view() == this;
	}

	bool View::can_become_focus() {
		return false;
	}

	bool View::blur() {
		if ( is_focus() ) {
			auto root = _window->root();
			if ( root && root != this ) {
				return root->focus();
			}
			return false;
		}
		return true;
	}

	Mat View::layout_matrix() {
		Vec2 translate = layout_offset() + _parent->layout_offset_inside();
		return Mat(
			1, 0, translate.x(),
			0, 1, translate.y()
		);
	}

	void View::solve_marks(uint32_t mark) {
		if (mark & kRecursive_Transform) { // update transform matrix
			unmark(kRecursive_Transform | kRecursive_Visible_Region); // unmark
			if (_parent) {
				_parent->matrix().mul(layout_matrix(), _matrix);
			} else {
				_matrix = layout_matrix();
			}
			goto visible_region;
		}
		if (mark & kRecursive_Visible_Region) {
			unmark(kRecursive_Visible_Region); // unmark
		visible_region:
			_visible_region = solve_visible_region();
		}
	}

	Vec2 View::position() {
		return Vec2(_matrix[2], _matrix[5]);
	}

	bool View::solve_visible_region() {
		return true;
	}

	bool View::overlap_test(Vec2 point) {
		return false;
	}
	
	void View::accept(ViewVisitor *visitor) {
		visitor->visitView(this);
	}

	TextInput* View::as_text_input() {
		return nullptr;
	}

	Button* View::as_button() {
		return nullptr;
	}

	bool View::layout_forward(uint32_t mark) {
		return !(mark & kLayout_Typesetting);
	}

	bool View::layout_reverse(uint32_t mark) {
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

	void View::layout_text(TextLines *lines, TextConfig* cfg) {
		// NOOP
	}

	void View::onChildLayoutChange(Layout* child, uint32_t value) {
		if (value & (kChild_Layout_Size | kChild_Layout_Visible | kChild_Layout_Align | kChild_Layout_Text)) {
			mark_layout(kLayout_Typesetting);
		}
	}

	void View::onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) {
		// NOOP
	}

	// --------------------------------------------------------------------------------------
	// @private

	void View::clear_link() { // Cleaning up associated view information
		if (_parent) {
			/* 当前为第一个子视图 */
			if (_parent->_first == this) {
				_parent->_first = _next;
			} else {
				_prev->_next = _next;
			}
			/* 当前为最后一个子视图 */
			if (_parent->_last == this) {
				_parent->_last = _prev;
			} else {
				_next->_prev = _prev;
			}
		}
	}

	void View::set_visible_(bool visible, uint32_t level) {
		_visible = visible;

		if (visible && level) {
			if (_level != level)
				set_level_(level, nullptr);
		} else {  // set level = 0
			if (_level)
				clear_level(_window/*no set window*/);
		}
	}

	void View::clear_level(Window *win) { //  clear layout depth
		blur();
		onSetParentOrLevel(0);
		set_level(0);
		_window = win;
		auto v = _first;
		while ( v ) {
			v->clear_level(win);
			v = v->_next;
		}
	}

	void View::set_level_(uint32_t level, Window *win) { // settings level
		if (_visible) {
			// if level > 0 then
			if (win) { // change new window
				blur();
				_window = win;
			}
			onSetParentOrLevel(level);
			set_level(level++);
			auto v = _first;
			while ( v ) {
				v->set_level_(level, win);
				v = v->_next;
			}
		} else {
			if ( _level )
				clear_level(_window/*no set window*/);
		}
	}

	void View::onSetParentOrLevel(uint32_t level) {
		if (level == 0) {
			// set_action(nullptr);
		}
	}

	/**
		*
		* Setting parent parent view
		*
		* @method set_parent(parent)
		*/
	void View::set_parent(View *parent) {
		if (parent != _parent) {
			Qk_STRICT_ASSERT(!is_root(), "root view not allow set parent");
			clear_link();

			if ( _parent ) {
				_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			} else {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
			_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			mark_layout(kLayout_Size_Width | kLayout_Size_Height); // mark layout size, reset layout size

			auto level = parent->_level;
			auto win = parent->_window;

			if (_visible && level) {
				if (_level != ++level || _window != win)
					set_level_(level, _window != win ? win: nullptr);
			} else {
				if (_level || _window != win)
					clear_level(win);
			}

			onSetParentOrLevel(_level);
		}
	}

	void View::remove_all_child_() {
		while (_first) {
			_first->remove_all_child();
			_first->remove();
		}
	}

	bool View::is_root() {
		return _window && _window->root() == this;
	}

}
