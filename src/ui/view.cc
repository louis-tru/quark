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

#include "./layout/layout.h"
#include "./layout/root.h"
#include "./window.h"
#include "./css/css.h"

namespace qk {

	View::View(Layout *layout)
		: Notification<UIEvent, UIEventName, Reference>()
		, _action(nullptr), _layout(layout)
		, _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr), _accessor(nullptr), _ssclass(nullptr)
	{
		layout->_view = this;
		layout->_accessor = prop_accessor_at_layout(layout->viewType(), kOPACITY_ViewProp);
		_accessor = prop_accessor_at_view(layout->viewType(), kOPACITY_ViewProp);
	}

	View::~View() {
		// The object maintained by the parent view should not be deconstructed,
		// where the parent must be empty
		Qk_ASSERT(_parent == nullptr);
		set_action(nullptr); // Delete action
		remove_all_child(); // Delete sub views
		_layout->_view = nullptr;

		preRender().async_call([](auto ctx, auto val) {
			delete ctx;
		}, _layout, 0);
	}

	Window* View::window() const {
		return _layout->_window;
	}

	StyleSheetsClass* View::ssclass() {
		if (!_ssclass) {
			_ssclass = new StyleSheetsClass(_layout);
			preRender().async_call([](auto ctx, auto val) { ctx->_ssclass = val; }, _layout, _ssclass);
		}
		return _ssclass;
	}

	float View::opacity() const {
		return _layout->_opacity;
	}

	void View::set_opacity(float val) { // async call set_opacity()
		preRender().async_call([](auto ctx, auto val) { ctx->set_opacity(val); }, _layout, val);
	}

	bool View::visible() const {
		return _layout->_visible;
	}

	void View::set_visible(bool val) { // async call set_visible()
		preRender().async_call([](auto ctx, auto val) { ctx->set_visible(val); }, _layout, val);
	}

	bool View::receive() const {
		return _layout->_receive;
	}

	void View::set_receive(bool val) { // async call set_receive()
		preRender().async_call([](auto ctx, auto val) { ctx->set_receive(val); }, _layout, val);
	}

	uint32_t View::level() const {
		return _layout->_level;
	}

	ViewType View::viewType() const {
		return _layout->viewType();
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
		return _layout->_window->dispatch()->focus_view() == this;
	}

	bool View::blur() {
		if ( is_focus() ) {
			auto root = _layout->_window->root();
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

	Button* View::as_button() {
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
		preRender().async_call([](auto ctx, auto val) { ctx->before(val); }, _layout, view->_layout);
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
		preRender().async_call([](auto ctx, auto val) { ctx->after(val); }, _layout, view->_layout);
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
		preRender().async_call([](auto ctx, auto val) { ctx->prepend(val); }, _layout, child->_layout);
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
		preRender().async_call([](auto ctx, auto val) {
			ctx->append(val);
		}, _layout, child->_layout);
	}

	void View::remove() {
		if (_parent) {
			blur();
			clear_link();
			_parent = nullptr;
			preRender().async_call([](auto ctx, auto val) { ctx->remove(); }, _layout, 0);
			release(); // Disconnect from parent view strong reference
		}
	}

	void View::remove_all_child() {
		while (_first) {
			_first->remove_all_child();
			_first->remove();
		}
	}

	PreRender& View::preRender() {
		return _layout->_window->preRender();
	}

	// @private
	// --------------------------------------------------------------------------------------

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
		if (parent != _parent) {
			#define is_root (_layout->_window->root() == this)
			Qk_STRICT_ASSERT(!is_root, "root view not allow set parent"); // check
			clear_link();
			if ( !_parent ) {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
		}
	}

}
