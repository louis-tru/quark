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
#include "./root.h"
#include "../window.h"
#include "../app.h"
#include "../css/css.h"
#include "../action/action.h"
#include "../../errno.h"

#define _isRt (RunLoop::first()->thread_id() != thread_self_id())

namespace qk {

	View::View()
		: _cssclass(nullptr)
		, _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _parent_Rt(nullptr)
		, _prev_Rt(nullptr), _next_Rt(nullptr)
		, _first_Rt(nullptr), _last_Rt(nullptr)
		, _window(nullptr)
		, _action(nullptr)
		, _accessor(nullptr)
		, _mark_value(kLayout_None)
		, _mark_index(-1)
		, _level(0)
		, _opacity(1.0)
		, _cursor(CursorStyle::Arrow)
		, _visible(true)
		, _visible_region(false)
		, _receive(true)
	{
	}

	void View::destroy() {
		// The object maintained by the parent view should not be deconstructed,
		// where the parent must be empty
		Qk_ASSERT(_parent == nullptr);
		auto cssclass = _cssclass;
		_cssclass = nullptr;
		Release(cssclass);
		set_action(nullptr); // Delete action
		remove_all_child(); // Delete sub views

		preRender().async_call([](auto self, auto arg) {
			// To ensure safety and efficiency,
			// it should be Completely destroyed in RT (render thread)
			self->Object::destroy();
		}, this, 0);
		_window = nullptr;
	}

	View* View::tryRetain() {
		if (_refCount++ > 0) {
			return this;
		} else {
			_refCount--; // Revoke self increase
			return nullptr;
		}
	}

	void View::set_receive(bool val, bool isRt) {
		_receive = val;
	}

	void View::set_cursor(CursorStyle val, bool isRt) {
		_cursor = val;
	}

	void View::set_visible(bool val, bool isRt) {
		#define _Is_root(self) (self->_window->root() == self)
		#define _Level(self, val) auto level = self->_parent_Rt && self->_parent_Rt->_level ? \
			self->_parent_Rt->_level + 1: val && _Is_root(self) ? 1: 0
		if (_visible != val) {
			_visible = val;
			if (isRt) {
				_Level(this, val);
				set_visible_Rt(val, level);
			} else {
				preRender().async_call([](auto self, auto arg) {
					_Level(self, arg.arg);
					self->set_visible_Rt(arg.arg, level);
				}, this, val);
			}
		}
	}

	void View::set_opacity(float val, bool isRt) {
		if (_opacity != val) {
			val = Qk_MAX(0, Qk_MIN(val, 1));
			if (_opacity != val)
				mark(0, isRt); // mark render
		}
	}

	CStyleSheetsClass* View::cssclass() {
		if (!_cssclass) {
			// After alignment, pointers can be read and written atomically
			_cssclass = new CStyleSheetsClass(this);
		}
		return _cssclass;
	}

	Matrix* View::matrix() {
		auto *v = this;
		do {
			auto t = v->asMatrix();
			if (t)
				return t;
			v = v->_parent;
		} while(v);
		return nullptr;
	}

	void View::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kRecursive_Transform) { // update transform matrix
			unmark(kRecursive_Transform | kRecursive_Visible_Region); // unmark
			_position =
				mat.mul_vec2_no_translate(layout_offset() + _parent_Rt->layout_offset_inside()) + 
				_parent_Rt->_position;
			_visible_region = solve_visible_region(Mat(mat).set_translate(_position));
		} else if (mark & kRecursive_Visible_Region) {
			unmark(kRecursive_Visible_Region); // unmark
			_visible_region = solve_visible_region(Mat(mat).set_translate(_position));
		}
	}

	Vec2 View::center() {
		return Vec2();
	}

	bool View::solve_visible_region(const Mat &mat) {
		return true;
	}

	bool View::overlap_test(Vec2 point) {
		return false;
	}

	TextInput* View::asTextInput() {
		return nullptr;
	}

	Matrix* View::asMatrix() {
		return nullptr;
	}

	TextOptions* View::asTextOptions() {
		return nullptr;
	}

	ScrollBase* View::asScrollBase() {
		return nullptr;
	}

	float View::layout_weight() {
		return 0;
	}

	Align View::layout_align() {
		return Align::Auto;
	}

	Vec2 View::layout_offset() {
		return Vec2();
	}

	View::Size View::layout_size() {
		return { Vec2(), Vec2(), true, true };
	}

	Vec2 View::layout_offset_inside() {
		return Vec2();
	}

	void View::set_layout_offset(Vec2 val) {
	}

	void View::set_layout_offset_free(Vec2 size) {
	}

	Vec2 View::layout_lock(Vec2 layout_size) {
		return Vec2();
	}

	void View::layout_forward(uint32_t mark) {
	}

	void View::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			auto v = _first_Rt;
			while (v) {
				v->set_layout_offset_free(Vec2()); // lazy view
				v = v->_next_Rt;
			}
			unmark(kLayout_Typesetting | kLayout_Size_Width | kLayout_Size_Height);
		}
	}

	void View::layout_text(TextLines *lines, TextConfig *cfg) {
	}

	void View::onChildLayoutChange(View *child, uint32_t value) {
		if (value & (kChild_Layout_Size | kChild_Layout_Visible | kChild_Layout_Align | kChild_Layout_Text)) {
			mark_layout(kLayout_Typesetting, true);
		}
	}

	void View::onActivate() {
	}

	PreRender& View::preRender() {
		return _window->preRender();
	}

	void View::mark_layout(uint32_t mark, bool isRt) {
		Qk_Assert_Eq(_isRt, isRt, "View::mark_layout(), isRt param no match");
		if (isRt) {
			_mark_value |= mark;
			if (_mark_index < 0) {
				if (_level) {
					preRender().mark_layout(this, _level); // push to pre render
				}
			}
		} else {
			preRender().async_call([](auto self, auto arg) {
				self->_mark_value |= arg.arg;
				if (self->_mark_index < 0) {
					if (self->_level) {
						self->preRender().mark_layout(self, self->_level); // push to pre render
					}
				}
			}, this, mark);
		}
	}

	void View::mark(uint32_t mark, bool isRt) {
		if (mark) {
			Qk_Assert_Eq(_isRt, isRt, "View::mark(), isRt param no match");
			if (isRt) {
				_mark_value |= mark;
				if (_level) {
					preRender().mark_render(); // push to pre render
				}
			} else {
				preRender().async_call([](auto self, auto arg) {
					self->_mark_value |= arg.arg;
					if (self->_level) {
						self->preRender().mark_render(); // push to pre render
					}
				}, this, mark);
			}
		} else {
			preRender().mark_render();
		}
	}

	bool View::is_clip() {
		return false;
	}

	ViewType View::viewType() const {
		return kView_ViewType;
	}

	void View::set_is_focus(bool value) {
		if ( value ) {
			focus();
		} else {
			blur();
		}
	}

	bool View::is_focus() const {
		return _window->dispatch()->focus_view() == this;
	}

	void View::set_action(Action* action) throw(Error) {
		if (action) {
			Qk_Check(action->window() == _window,
				ERR_ACTION_SET_WINDOW_NO_MATCH, "View::set_action, set action window not match"
			);
		}
		if (action != _action) {
			if ( _action ) {
				auto action = _action;
				_action = nullptr;
				action->del_target(this);
				action->release();
			}
			if ( action ) {
				_action = action;
				action->set_target(this);
				action->retain(); // retain from view view
			}
		}
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

	bool View::can_become_focus() {
		return false;
	}

	Button* View::asButton() {
		return nullptr;
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
		preRender().async_call([](auto self, auto arg) { self->before_Rt(arg.arg); }, this, view);
	}

	void View::after(View *view) {
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
		preRender().async_call([](auto self, auto arg) { self->after_Rt(arg.arg); }, this, view);
	}

	void View::prepend(View *child) {
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
		preRender().async_call([](auto self, auto arg) { self->prepend_Rt(arg.arg); }, this, child);
	}

	void View::append(View *child) {
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
		preRender().async_call([](auto self, auto arg) { self->append_Rt(arg.arg); }, this, child);
	}

	void View::remove() {
		if (_parent) {
			blur();
			clear_link();
			_parent = nullptr;
			preRender().async_call([](auto self, auto arg) { self->remove_Rt(); }, this, 0);
			release(); // Disconnect from parent view strong reference
		}
	}

	void View::remove_all_child() {
		while (_first) {
			_first->remove_all_child();
			_first->remove();
		}
	}

	// @private
	// --------------------------------------------------------------------------------------

	View* View::init(Window* win) {
		Qk_ASSERT(win);
		_window = win;
		_accessor = prop_accessor_at_view(viewType(), kOPACITY_ViewProp);
		return this;
	}

	void View::clear_link() { // Cleaning up associated view information
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

	void View::set_parent(View *parent) {
		Qk_Fatal_Assert(_window == parent->_window, "window no match, parent->_window no equal _window");
		if (parent != _parent) {
			#define is_root (_window->root() == this)
			Qk_Fatal_Assert(!is_root, "root view not allow set parent"); // check
			clear_link();
			if ( !_parent ) {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
		}
	}

	void View::before_Rt(View *_view) {
		if (_view == this) return;
		if (_parent_Rt) {
			if (_view->_parent_Rt == _parent_Rt) {
				_view->clear_link_Rt();  // clear link
			} else {
				_view->set_parent_Rt(_parent_Rt);
			}
			if (_prev_Rt) {
				_prev_Rt->_next_Rt = _view;
			} else { // There are no brothers on top
				_parent_Rt->_first_Rt = _view;
			}
			_view->_prev_Rt = _prev_Rt;
			_view->_next_Rt = this;
			_prev_Rt = _view;
		}
	}

	void View::after_Rt(View *_view) {
		if (_view == this) return;
		if (_parent_Rt) {
			if (_view->_parent_Rt == _parent_Rt) {
				_view->clear_link_Rt(); // clear link
			} else {
				_view->set_parent_Rt(_parent_Rt);
			}
			if (_next_Rt) {
				_next_Rt->_prev_Rt = _view;
			} else { // There are no brothers below
				_parent_Rt->_last_Rt = _view;
			}
			_view->_prev_Rt = this;
			_view->_next_Rt = _next_Rt;
			_next_Rt = _view;
		}
	}

	void View::prepend_Rt(View *_child) {
		if (this == _child->_parent_Rt) {
			_child->clear_link_Rt();
		} else {
			_child->set_parent_Rt(this);
		}
		if (_first_Rt) {
			_child->_prev_Rt = nullptr;
			_child->_next_Rt = _first_Rt;
			_first_Rt->_prev_Rt = _child;
			_first_Rt = _child;
		} else { // There are currently no sub views available yet
			_child->_prev_Rt = nullptr;
			_child->_next_Rt = nullptr;
			_first_Rt = _child;
			_last_Rt = _child;
		}
	}

	void View::append_Rt(View *_child) {
		if (this == _child->_parent_Rt) {
			_child->clear_link_Rt();
		} else {
			_child->set_parent_Rt(this);
		}
		if (_last_Rt) {
			_child->_prev_Rt = _last_Rt;
			_child->_next_Rt = nullptr;
			_last_Rt->_next_Rt = _child;
			_last_Rt = _child;
		} else { // There are currently no sub views available yet
			_child->_prev_Rt = nullptr;
			_child->_next_Rt = nullptr;
			_first_Rt = _child;
			_last_Rt = _child;
		}
	}

	void View::remove_Rt() {
		if (_parent_Rt) {
			clear_link_Rt();
			_parent_Rt = nullptr;
			if (_level) {
				clear_level_Rt();
			}
		}
	}

	void View::clear_link_Rt() { // Cleaning up associated view information
		if (_parent_Rt) {
			/* Currently the first sub view */
			if (_parent_Rt->_first_Rt == this) {
				_parent_Rt->_first_Rt = _next_Rt;
			} else {
				_prev_Rt->_next_Rt = _next_Rt;
			}
			/* Currently the last sub view */
			if (_parent_Rt->_last_Rt == this) {
				_parent_Rt->_last_Rt = _prev_Rt;
			} else {
				_next_Rt->_prev_Rt = _prev_Rt;
			}
		}
	}

	void View::set_visible_Rt(bool visible, uint32_t level) {
		// _visible = visible;
		if (visible && level) {
			if (_level != level)
				set_level_Rt(level);
		} else { // set level = 0
			if (_level)
				clear_level_Rt();
		}
		if (_parent_Rt) {
			_parent_Rt->onChildLayoutChange(this, kChild_Layout_Visible); // mark parent view 
		}
		if (visible) {
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, true); // reset view size
			if (_cssclass) {
				_cssclass->updateClass_Rt();
			}
		}
	}

	void View::clear_level_Rt() { //  clear view depth
		auto win = _window;
		if (win->dispatch()->focus_view() == this) {
		
			preRender().post(Cb([this,win](auto &e) {
				if (win->dispatch()->focus_view() == this) {
					blur();
				}
			}, win));
		}
		if (_mark_index >= 0) {
			preRender().unmark_layout(this, _level);
		}
		_level = 0;
		onActivate();
		auto v = _first_Rt;
		while ( v ) {
			v->clear_level_Rt();
			v = v->_next_Rt;
		}
	}

	void View::set_level_Rt(uint32_t level) { // settings level
		if (_visible) {
			// if level > 0 then
			if (_mark_index >= 0) {
				preRender().unmark_layout(this, _level);
			}
			preRender().mark_layout(this, level);
			_level = level++;
			onActivate();

			auto v = _first_Rt;
			while ( v ) {
				v->set_level_Rt(level);
				v = v->_next_Rt;
			}
		} else {
			if ( _level )
				clear_level_Rt();
		}
	}

	void View::set_parent_Rt(View *parent) {
		if (parent != _parent_Rt) {
			clear_link_Rt();
			if ( _parent_Rt ) {
				_parent_Rt->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent view
			}
			_parent_Rt = parent;

			auto level = parent->_level;
			if (_visible && level) {
				if (_level != ++level)
					set_level_Rt(level);
			} else {
				if (_level)
					clear_level_Rt();
			}
			_parent_Rt->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent view
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, true); // mark view size, reset view size

			if (_cssclass) {
				_cssclass->updateClass_Rt();
			}
			onActivate();
		}
	}

	void View::applyClass_Rt(CStyleSheetsClass *ssc) {
		if (_cssclass->apply_Rt(ssc)) { // Impact sub view
			if (_cssclass->haveSubstyles())
				ssc = _cssclass;
			auto l = _first_Rt;
			while (l) {
				if (l->_visible && l->_cssclass) {
					l->applyClass_Rt(ssc);
				}
				l = l->_next_Rt;
			}
		}
		unmark(kStyle_Class);
	}

	CStyleSheetsClass* View::parentSsclass_Rt() {
		auto view = _parent_Rt;
		while (view) {
			auto ss = view->_cssclass;
			if (ss && ss->haveSubstyles()) {
				return ss;
			}
			view = view->_parent_Rt;
		}
		return nullptr;
	}

}
