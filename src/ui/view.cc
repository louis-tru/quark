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
#include "./window.h"
#include "./layout/layout.h"

namespace qk {

	View::View(Layout *layout)
		: Notification<UIEvent, UIEventName, Reference>()
		, _action(nullptr), _layout(layout)
		, _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _receive(false)
	{
		layout->_view = this;
	}

	View::~View() {
		Qk_ASSERT(_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
		set_action(nullptr); // del action
		remove_all_child(); // 删除子视图
	}

	void View::set_receive(bool val) {
		_receive = val;
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
		_layout->before(view->_layout); // TODO ... layout cmd
	}

	void View::after(View *view) {
		if (view == this) return;
		if (_parent) {
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
		_layout->after(view->_layout); // TODO ... layout cmd
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
		} else { // 当前还没有子视图
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
		_layout->prepend(child->_layout); // TODO ... layout cmd
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
		} else { // 当前还没有子视图
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
		_layout->append(child->_layout); // TODO ... layout cmd
	}

	void View::remove() {
		if (_parent) {
			clear_link();
			_parent = nullptr;
			release(); // Disconnect from parent view strong reference
			_layout->remove(); // TODO ... layout cmd
		}
	}

	void View::remove_all_child() {
		while (_first) {
			_first->remove_all_child();
			_first->remove();
		}
		// _layout->remove_all_child(); // TODO ... layout cmd
	}

	void View::set_visible(bool val) {
		_layout->set_visible(val); // TODO ... layout cmd
	}

	bool View::visible() const {
		return _layout->visible(); // TODO ... layout cmd
	}

	void View::set_is_focus(bool value) {
		if ( value ) {
			focus();
		} else {
			blur();
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

	bool View::is_focus() const {
		return _layout->window() && _layout->window()->dispatch()->focus() == this;
	}

	bool View::blur() {
		if ( is_focus() ) {
			auto root = _layout->window()->root();
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

	bool View::clip() {
		return false;
	}

	Button* View::as_button() {
		return nullptr;
	}

	// @private
	// --------------------------------------------------------------------------------------

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

	void View::set_parent(View *parent) {
		if (parent != _parent) {
			#define is_root() (_layout->_window && _layout->_window->root() == this)
			// Qk_STRICT_ASSERT(!is_root(), "root view not allow set parent"); // check
			clear_link();
			if ( !_parent ) {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
		}
	}

}
