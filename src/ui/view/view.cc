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

	View::View()
		: Notification<UIEvent, UIEventName>()
		, _window(nullptr)
		, _action(nullptr), _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _level(0)
		, _visible(true)
	{}

	View::~View() {
		Qk_ASSERT(_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
		set_action(nullptr); // del action
		remove_all_child(); // 删除子视图
	}

	/**
		*
		* Add a sibling view to the front
		*
		* @method before(view)
		*/
	void View::before(View* view) {
		if (_parent) {
			if (view == this) return;
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

	/**
		*
		* Add a sibling view to the back
		*
		* @method after(view)
		*/
	void View::after(View* view) {
		if (_parent) {
			if (view == this) return;
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

	/**
		* 
		* Append subview to front
		* 
		* @method prepend(child)
		*/
	void View::prepend(View* child) {
		if (!is_allow_append_child()) {
			Qk_WARN("Not can allow prepend child view");
			return;
		}
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

	/**
		*
		* Append subview to end
		*
		* @method append(child)
		*/
	void View::append(View* child) {
		if (!is_allow_append_child()) {
			Qk_WARN("Not can allow append child view");
			return;
		}
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

	bool View::is_allow_append_child() {
		return true;
	}

	/**
		*
		* Remove destroy self and child views from parent view
		* 
		* @method remove()
		*/
	void View::remove() {
		if (_parent) {
			clear_link();
			_parent = nullptr;
			clear_level(nullptr);
			release(); // Disconnect from parent view strong reference
		}
	}

	/**
		*
		* remove all subview
		*
		* @method remove_all_child()
		*/
	void View::remove_all_child() {
		while (_first) {
			_first->remove_all_child();
			_first->remove();
		}
	}

	/**
		* 
		* Setting the visibility properties the view object
		*
		* @method set_visible(val)
		*/
	void View::set_visible(bool val) {
		if (_visible != val) {
			set_visible_(val, _parent && _parent->_level ? _parent->_level + 1: 0);
		}
	}

	/**
	 *
	 * Setting the level properties the view object
	 *
	 * @method set_level(val)
	 */
	void View::set_level(uint32_t level) {
		if (level == 0) {
			// set_action(nullptr); // del action
		}
		_level = level;
	}

	/**
		* Set the `action` properties of the view object
		*
		* @method set_action()
		*/
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

	View* View::as_text_input() {
		return nullptr;
	}

	View* View::as_button() {
		return nullptr;
	}

	/**
	 * @method set_is_focus(value)
	 */
	void View::set_is_focus(bool value) {
		if ( value ) {
			focus();
		} else {
			blur();
		}
	}

	/**
	 * @method is_focus()
	 */
	bool View::is_focus() const {
		return _window && _window->dispatch()->focus_view() == this;
	}

	/**
	 *
	 * Can it be the focus
	 * 
	 * @method can_become_focus()
	 */
	bool View::can_become_focus() {
		return false;
	}

	/**
	 * @method blur()
	 */
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

	/**
		* @overwrite
		*/
	void View::trigger_listener_change(uint32_t name, int count, int change) {
		if ( change > 0 ) {
			// _receive = true; // bind event auto open option
		}
	}

	/**
		* @method has_selfChild(child)
		*/
	bool View::is_self_child(View *child) {
		if ( child ) {
			View *parent = child->_parent;
			while (parent) {
				if ( parent == this ) {
					return true;
				}
				parent = parent->_parent;
			}
		}
		return false;
	}

	// @private --------------------------------------------------------

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
		_window = win;
		set_level(0);
		View *v = _first;
		while ( v ) {
			v->clear_level(win);
			v = v->_next;
		}
	}

	void View::set_level_(uint32_t level, Window *win) { // settings level
		if (_visible) {
			if (win) { // set new window
				blur();
				_window = win;
			}
			set_level(level++);
			View *v = _first;
			while ( v ) {
				v->set_level_(level, win);
				v = v->_next;
			}
		} else {
			if ( _level )
				clear_level(_window/*no set window*/);
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
			clear_link();
			_parent = parent;
			auto level = parent->_level;
			auto win = parent->_window;

			if (_visible && level) {
				if (_level != ++level || _window != win)
					set_level_(level, _window != win ? win: nullptr);
			} else {
				if (_level || _window != win)
					clear_level(win);
			}
		}
	}

}
