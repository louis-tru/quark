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
#include "../css/css_props.h"
#include "../geometry.h"
#include "../painter.h"

#if DEBUG
# define _Assert_IsRt(isRt, ...) \
	Qk_ASSERT(!_window->root() || (_isRt==isRt), ##__VA_ARGS__)
#else
# define _Assert_IsRt(isRt, ...)
#endif
#define _isRt (!RunLoop::is_first())
#define _Cssclass() auto _cssclass = this->_cssclass.load()
#define _IfCssclass() _Cssclass(); if (_cssclass)
#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue

namespace qk {

	CssPropAccessor* get_props_accessor(ViewType type, CssProp prop);

	typedef View::Container Container;

Range Container::to_range() const {
		Vec2 origin(content), end(content);
		if (state_x == kNone_FloatState) {
			origin[0] = pre_width[0];
			end[0] = pre_width[1];
		}
		if (state_y == kNone_FloatState) {
			origin[1] = pre_height[0];
			end[1] = pre_height[1];
		}
		return { origin,end };
	}

	float Container::clamp_width(float value) const {
		return Float32::clamp(value, pre_width[0], pre_width[1]);
	}

	float Container::clamp_height(float value) const {
		return Float32::clamp(value, pre_height[0], pre_height[1]);
	}

	bool Container::set_pre_width(Container::Pre pre) {
		if (pre_width != pre.value || locked_x) {
			pre_width = pre.value;
			state_x = pre.state;
			locked_x = false; // clear locked
			return true;
		}
		return false;
	}

	bool Container::set_pre_height(Container::Pre pre) {
		if (pre_height != pre.value || locked_y) {
			pre_height = pre.value;
			state_y = pre.state;
			locked_y = false; // clear locked
			return true;
		}
		return false;
	}

	Vec2 Container::layout_size_before_locking(Vec2 layout_size) const {
		return layout_size - content_diff_before_locking;
	}

	View::View()
		: _cssclass(nullptr)
		, _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _window(nullptr)
		, _action(nullptr)
		, _accessor(nullptr)
		, _mark_value(kLayout_None)
		, _mark_index(0)
		, _level(0)
		, _color(255,255,255,255) // white
		, _cursor(CursorStyle::Normal)
		, _z_index(0)
		, _cascade_color(CascadeColor::Both)
		, _visible(true)
		, _visible_area(false)
		, _receive(true)
		, _aa(true)
	{
		// Qk_DLog("View sizeof, %d", sizeof(View));
	}

	void View::destroy() {
		// The object maintained by the parent view should not be deconstructed,
		// where the parent must be empty
		Qk_ASSERT_EQ(_parent, nullptr);
		Releasep(_cssclass);
		set_action(nullptr); // Delete action
		remove_all_child(); // Delete sub views
		_mark_value = kDestroy; // force mark destroy

		preRender().async_call([](auto self, auto arg) {
			// To ensure safety and efficiency,
			// it should be Completely destroyed in RT (render thread)
			auto center = self->_window->actionCenter();
			if (center) {
				center->removeCSSTransition_rt(self);
			}
			self->_window = nullptr;
			self->Object::destroy();
		}, this, 0);
	}

	View* View::tryRetain_rt() {
		if (_refCount.load(std::memory_order_acquire) > 0) {
			if (_refCount.fetch_add(1, std::memory_order_acq_rel) > 0) { // _refCount++ > 0
				return this;
			} else {
				_refCount.fetch_sub(1, std::memory_order_release); // Revoke self increase, Maybe not safe
			}
		}
		if (!(_mark_value & kDestroy)) { // If not destroying, try one more time
			_refCount++; // self increase
			if (!(_mark_value & kDestroy)) { // check again
				return this;
			} else {
				_refCount--; // Revoke self increase
			}
		}
		return nullptr;
	}

	void View::set_receive(bool val) {
		_receive = val;
	}

	void View::set_aa(bool val) {
		_aa = val;
	}

	void View::set_cursor(CursorStyle val, bool isRt) {
		_cursor = val;
	}

	void View::set_z_index(uint32_t val, bool isRt) {
		if (_z_index != val) {
			_z_index = val;
			mark(kLayout_None, isRt); // mark render
		}
	}

	void View::set_visible(bool val, bool isRt) {
		if (_visible != val) {
			_visible = val;
			if (isRt) {
				set_visible_rt(val);
			} else {
				preRender().async_call([](auto self, auto arg) {
					self->set_visible_rt(arg.arg);
				}, this, val);
			}
		}
	}

	float View::opacity() const {
		return _color.to_float_alpha();
	}

	void View::set_opacity(float val, bool isRt) {
		uint8_t alpha8 = val * 255;
		if (_color.a() != alpha8) {
			_color.set_a(alpha8);
			mark(0, isRt); // mark render
		}
	}

	void View::set_color(Color val, bool isRt) {
		if (_color != val) {
			_color = val;
			mark(0, isRt); // mark render
		}
	}

	void View::set_cascade_color(CascadeColor val, bool isRt) {
		if (_cascade_color != val) {
			_cascade_color = val;
			mark(0, isRt); // mark render
		}
	}

	CStyleSheetsClass* View::cssclass() {
		auto cssclass = _cssclass.load();
		if (!cssclass) {
			// After alignment, pointers can be read and written atomically
			cssclass = new CStyleSheetsClass(this);
			_cssclass.store(cssclass);
		}
		return cssclass;
	}

	MorphView* View::morph_view() {
		auto v = this;
		do {
			auto t = v->asMorphView();
			if (t)
				return t;
			v = v->_parent;
		} while(v);
		return nullptr;
	}

	void View::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			unmark(kTransform | kVisible_Region); // unmark visible region too
			_position =
				mat.mul_vec2_no_translate(layout_offset() + parent->layout_offset_inside()) +
				parent->_position;
			solve_visible_area(Mat(mat).set_translate(_position));
		} else if (mark & kVisible_Region) {
			unmark(kVisible_Region); // unmark
			solve_visible_area(Mat(mat).set_translate(_position));
		}
	}

	Vec2 View::client_size() {
		return {};
	}

	Region View::client_region() {
		return {};
	}

	void View::solve_visible_area(const Mat &mat) {
		_visible_area = true; // Always visible
	}

	bool View::compute_visible_area(const Mat &mat, Vec2 bounds[4]) {
		/*
		* 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
		* 这种模糊测试在大多数时候都是正确有效的。
		*/
		auto& clip = _window->getClipRange();
		auto  re   = region_aabb_from_convex_quadrilateral(bounds);

		if (Qk_Max( clip.end.y(), re.end.y() ) - Qk_Min( clip.begin.y(), re.begin.y() )
					<= re.end.y() - re.begin.y() + clip.size.y() &&
				Qk_Max( clip.end.x(), re.end.x() ) - Qk_Min( clip.begin.x(), re.begin.x() )
					<= re.end.x() - re.begin.x() + clip.size.x()
				) {
			//_visible_area = !_client_size.is_zero_axis();
			return true;
		} else {

#if 0
			Qk_DLog("visible_area-x: %f<=%f", Qk_Max( clip.y2, re.end.y() ) - Qk_Min( clip.y, re.origin.y() ),
																				re.end.y() - re.origin.y() + clip.height);
			Qk_DLog("visible_area-y: %f<=%f", Qk_Max( clip.x2, re.end.x() ) - Qk_Min( clip.x, re.origin.x() ),
																				re.end.x() - re.origin.x() + clip.width);
#endif
			return false;
		}
	}

	bool View::overlap_test(Vec2 point) {
		return false;
	}

	TextInput* View::asTextInput() {
		return nullptr;
	}

	MorphView* View::asMorphView() {
		return nullptr;
	}

	Entity* View::asEntity() {
		return nullptr;
	}

	Agent* View::asAgent() {
		return nullptr;
	}

	TextOptions* View::asTextOptions() {
		return nullptr;
	}

	ScrollView* View::asScrollView() {
		return nullptr;
	}

	Vec2 View::layout_weight() {
		return Vec2();
	}

	Align View::layout_align() {
		return Align::Normal;
	}

	Vec2 View::layout_offset() {
		return Vec2();
	}

	Vec2 View::layout_size() {
		return Vec2();
	}

	static View::Container zeroContainer{
		{}, {}, {}, {}, View::kFixed_FloatState, View::kFixed_FloatState, false, false
	};

	const View::Container& View::layout_container() {
		return zeroContainer;
	}

	Vec2 View::layout_offset_inside() {
		return Vec2();
	}

	void View::set_layout_offset(Vec2 val) {
	}

	void View::set_layout_offset_free(Vec2 size) {
	}

	float View::layout_lock_width(float size) {
		return 0;
	}

	float View::layout_lock_height(float size) {
		return 0;
	}

	void View::layout_forward(uint32_t mark) {
		if (mark & (kLayout_Inner_Width | kLayout_Inner_Height)) {
			// Noop
			unmark(kLayout_Inner_Width | kLayout_Inner_Height);
		}
	}

	void View::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			auto v = first();
			while (v) {
				v->set_layout_offset_free(Vec2()); // lazy view
				v = v->next();
			}
			unmark(kLayout_Typesetting);
		}
	}

	void View::layout_text(TextLines *lines, TextConfig *cfg) {
	}

	void View::text_config(TextConfig* cfg) {
	}

	void View::onChildLayoutChange(View *child, uint32_t mark) {
		if (mark & (kChild_Layout_Size | kChild_Layout_Visible | kChild_Layout_Align | kChild_Layout_Text)) {
			mark_layout(kLayout_Typesetting, true);
		}
	}

	void View::onActivate() {
	}

	PreRender& View::preRender() {
		return _window->preRender();
	}

	void View::mark_layout(uint32_t mark, bool isRt) {
		_Assert_IsRt(isRt, "View::mark_layout(), isRt param no match");
		if (isRt) {
			_mark_value |= mark;
			if (_mark_index == 0) {
				if (_level) {
					preRender().mark_layout(this, _level); // push to pre render
				}
			}
		} else {
			preRender().async_call([](auto self, auto arg) {
				self->_mark_value |= arg.arg;
				if (self->_mark_index == 0) {
					if (self->_level) {
						self->preRender().mark_layout(self, self->_level); // push to pre render
					}
				}
			}, this, mark);
		}
	}

	void View::mark(uint32_t mark, bool isRt) {
		if (mark) {
			_Assert_IsRt(isRt, "View::mark(), isRt param no match");
			if (isRt) {
				_mark_value |= mark;
				if (_level) {
					preRender()._is_render = true;
				}
			} else {
				preRender().async_call([](auto self, auto arg) {
					self->_mark_value |= arg.arg;
					if (self->_level) {
						self->preRender()._is_render = true;
					}
				}, this, mark);
			}
		} else {
			preRender()._is_render = true;
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
		return _window->dispatch()->focusView() == this;
	}

	void View::set_action(Action* action) throw(Error) {
		if (action) {
			Qk_IfThrow(action->window() == _window,
				ERR_ACTION_SET_WINDOW_NO_MATCH, "View::set_action, Not match the window"
			);
		}
		if (action != _action) {
			if ( _action ) {
				_action->del_target(this);
				_action->release();
				_action = nullptr;
			}
			if ( action ) {
				action->set_target(this);
				action->retain(); // retain from view view
				_action = action;
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

	bool View::is_child(View *child) {
		if ( child ) {
			auto parent = child->parent();
			while (parent) {
				if ( parent == this ) {
					return true;
				}
				parent = parent->parent();
			}
		}
		return false;
	}

	void View::before(View *view) {
		if (view == this) return;
		_IfParent() {
			if (view->_parent == _parent) {
				if (view->_next == this)
					return; // already before
				view->clear_link(true);  // clear link
			} else {
				view->set_parent(_parent);
			}
			auto _prev = this->prev();
			if (_prev) {
				_prev->_next = view;
			} else { // There are no brothers on top
				_parent->_first = view;
			}
			view->_prev = _prev;
			view->_next = this;
			this->_prev = view;
		}
	}

	void View::after(View *view) {
		if (view == this) return;
		_IfParent() {
			if (view->_parent == _parent) {
				if (view->_prev == this)
					return; // already after
				view->clear_link(true); // clear link
			} else {
				view->set_parent(_parent);
			}
			auto _next = this->next();
			if (_next) {
				_next->_prev = view;
			} else { // There are no brothers below
				_parent->_last = view;
			}
			view->_prev = this;
			view->_next = _next;
			this->_next = view;
		}
	}

	void View::prepend(View *child) {
		if (this == child->_parent) {
			if (_first == child)
				return; // already first
			child->clear_link(true);
		} else {
			child->set_parent(this);
		}
		auto _first = this->first();
		if (_first) {
			child->_prev = nullptr;
			child->_next = _first;
			_first->_prev = child;
			this->_first = child;
		} else { // There are currently no sub views available yet
			child->_prev = nullptr;
			child->_next = nullptr;
			this->_first = child;
			this->_last = child;
		}
	}

	void View::append(View *child) {
		if (this == child->_parent) {
			if (_last == child)
				return; // already last
			child->clear_link(true);
		} else {
			child->set_parent(this);
		}
		auto _last = this->last();
		if (_last) {
			child->_prev = _last;
			child->_next = nullptr;
			_last->_next = child;
			this->_last = child;
		} else { // There are currently no sub views available yet
			child->_prev = nullptr;
			child->_next = nullptr;
			this->_first = child;
			this->_last = child;
		}
	}

	void View::remove() {
		if (_parent) {
			blur();
			clear_link(false);
			set_action(nullptr); // Delete action
			preRender().async_call([](auto self, auto arg) {
				if (self->_level) {
					if (arg.arg) { // notice parent view
						arg.arg->onChildLayoutChange(self, kChild_Layout_Visible);
					}
					self->clear_level_rt();
				}
			}, this, _parent.load());
			_parent = nullptr;
			release(); // Disconnect from parent view strong reference
		}
	}

	void View::remove_all_child() {
		auto _first = this->first();
		while (_first) {
			_first->remove_all_child();
			_first->remove();
			_first = this->first();
		}
	}

	void View::clear_link(bool notice) { // Cleaning up associated view information
		_IfParent() {
			auto _prev = this->prev();
			auto _next = this->next();
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
			if (notice) {
				preRender().async_call([](auto self, auto arg) {
					if (self->_level)
						arg.arg->onChildLayoutChange(self, kChild_Layout_Visible);
				}, this, _parent);
			}
		}
	}

	static bool check_loop_ref(View *self, View *parent) {
		while (parent) {
			if (self == parent) {
				return true;
			}
			parent = parent->parent();
		}
		return false;
	}

	void View::set_parent(View *parent) {
		Qk_CHECK(_window == parent->_window, "window no match, parent->_window no equal _window");
		Qk_CHECK(!check_loop_ref(this, parent), "View::set_parent(parent), loop ref error");
		_Parent();
		if (parent != _parent) {
			Qk_CHECK(_window->root() != this, "root view not allow set parent"); // check
			clear_link(false);
			if ( !_parent ) {
				retain(); // link to parent and retain ref
			}
			// to Rt, set level
			preRender().async_call([](auto self, auto arg) {
				if ( arg.arg ) { // notice old parent
					arg.arg->onChildLayoutChange(self, kChild_Layout_Visible); // notice parent view
				}
				auto _parent = self->parent();
				if (_parent) {
					auto level = _parent->_level;
					if (self->_visible && level) {
						if (self->_level != ++level)
							self->set_level_rt(level);
					} else {
						if (self->_level)
							self->clear_level_rt();
					}
					_parent->onChildLayoutChange(self, kChild_Layout_Visible); // notice new parent view
				}
				self->mark_layout(kLayout_Inner_Width | kLayout_Inner_Height, true); // mark view size, reset view size

				auto _cssclass = self->_cssclass.load();
				if (_cssclass) {
					_cssclass->updateClass_rt();
				}
				self->onActivate();
			}, this, _parent);
			this->_parent = parent;
		}
	}

	// --------------------------------------------------------------------------------------

	void View::set_visible_rt(bool visible) {
		_Parent();
		auto level =
			_parent && _parent->_level         ? _parent->_level + 1:
			visible && _window->root() == this ? 1: 0;

		if (visible && level) {
			if (_level != level)
				set_level_rt(level);
		} else { // set level = 0
			if (_level)
				clear_level_rt();
		}
		if (_parent) {
			_parent->onChildLayoutChange(this, kChild_Layout_Visible); // mark parent view
		}
		if (visible) {
			mark_layout(kLayout_Inner_Width | kLayout_Inner_Height, true); // reset view size
			_IfCssclass() {
				_cssclass->updateClass_rt();
			}
		}
	}

	void View::set_level_rt(uint32_t level) { // settings level
		if (_visible) {
			// if level > 0 then
			if (_mark_index) {
				preRender().unmark_layout(this, _level);
			}
			preRender().mark_layout(this, level);
			_level = level++;
			onActivate();

			auto v = first();
			while ( v ) {
				v->set_level_rt(level);
				v = v->next();
			}
		} else {
			if ( _level )
				clear_level_rt();
		}
	}

	void View::clear_level_rt() { //  clear view depth
		auto win = _window;
		if (win->dispatch()->focusView() == this) {
			preRender().post(Cb([this,win](auto e) {
				if (win->dispatch()->focusView() == this)
					blur();
			}),this);
		}
		if (_mark_index) {
			preRender().unmark_layout(this, _level);
		}
		_level = 0;
		onActivate();
		auto v = first();
		while ( v ) {
			v->clear_level_rt();
			v = v->next();
		}
	}

	void View::apply_class_rt(CStyleSheetsClass *parentSsclass) {
		_Cssclass();
		if (_cssclass->apply_rt(parentSsclass, false)) { // change affect sub styles
			if (_cssclass->haveSubstyles()) {
				parentSsclass = _cssclass;
			}
			if (_visible) {
				auto v = first();
				while (v) {
					// if (v->_cssclass)
					// 	v->apply_class_rt(parentSsclass);
					v->apply_class_all_rt(parentSsclass, false);
					v = v->next();
				}
			}
		}
		unmark(kClass_All);
	}

	void View::apply_class_all_rt(CStyleSheetsClass *parentSsclass, bool force) {
		_Cssclass();
		if (_cssclass) { // Impact sub view
			_cssclass->apply_rt(parentSsclass, force);
			if (_cssclass->haveSubstyles()) {
				parentSsclass = _cssclass;
			}
		}
		if (_visible) { // apply all sub views if visible
			auto v = first();
			while (v) {
				v->apply_class_all_rt(parentSsclass, force);
				v = v->next();
			}
		}
		unmark(kClass_All);
	}

	CStyleSheetsClass* View::parent_ssclass_rt() {
		_Parent();
		while (_parent) {
			auto ss = _parent->_cssclass.load();
			if (ss && ss->haveSubstyles()) {
				return ss;
			}
			_parent = _parent->parent();
		}
		return nullptr;
	}

	CursorStyle View::cursor_style_exec() {
		auto v = this;
		do {
			if (v->_cursor != CursorStyle::Normal)
				return v->_cursor;
			v = v->parent();
		} while (v);
		return CursorStyle::Normal;
	}

	View* View::init(Window* win) {
		Qk_ASSERT(win);
		_window = win;
		_accessor = get_props_accessor(viewType(), kCOLOR_CssProp);
		return this;
	}

}
