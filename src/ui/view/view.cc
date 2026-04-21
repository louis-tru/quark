/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
#define _async_call(block, param) async_call([](auto self, auto arg) block, this, param)

namespace qk {

	static bool check_loop_ref(View *self, View *parent) {
		while (parent) {
			if (self == parent)
				return true;
			parent = parent->parent();
		}
		return false;
	}

	CssPropAccessor* get_props_accessor(ViewType type, CssProp prop);

	typedef View::Container Container;

	Range Container::to_range() const {
		Vec2 origin(content), end(content);
		if (state_x == kNone_FloatState) {
			origin[0] = pre_width_min;
			end[0] = pre_width_max;
		}
		if (state_y == kNone_FloatState) {
			origin[1] = pre_height_min;
			end[1] = pre_height_max;
		}
		return { origin,end };
	}

	float Container::clamp_width(float value) const {
		return Float32::clamp(value, pre_width_min, pre_width_max);
	}

	float Container::clamp_height(float value) const {
		return Float32::clamp(value, pre_height_min, pre_height_max);
	}

	bool Container::set_pre_width(Container::Pre pre) {
		if (pre_width_min != pre.min || pre_width_max != pre.max || locked_x) {
			pre_width_min = pre.min;
			pre_width_max = pre.max;
			state_x = pre.state;
			locked_x = false; // clear locked
			return true;
		}
		return false;
	}

	bool Container::set_pre_height(Container::Pre pre) {
		if (pre_height_min != pre.min || pre_height_max != pre.max || locked_y) {
			pre_height_min = pre.min;
			pre_height_max = pre.max;
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
		: _parent(nullptr), _prev(nullptr)
		, _next(nullptr), _first(nullptr), _last(nullptr)
		, _parent_rt(nullptr), _prev_rt(nullptr)
		, _next_rt(nullptr), _first_rt(nullptr), _last_rt(nullptr)
		, _cssclass(nullptr)
		, _window(nullptr)
		, _action(nullptr)
		, _accessor(nullptr)
		, _style_flags{0}
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

	View* View::init(Window* win) {
		Qk_ASSERT(win);
		_window = win;
		_accessor = get_props_accessor(view_type(), kCOLOR_CssProp);
		return this;
	}

	void View::destroy() {
		// The object maintained by the parent view should not be deconstructed,
		// where the parent must be empty
		Qk_ASSERT_EQ(_parent, nullptr);
		Releasep(_cssclass);
		set_action(nullptr); // Delete action
		remove_all_child(); // Delete sub views

		async_call([](auto self, auto arg) {
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

	View* View::try_retain_rt() {
		if (_refCount.load(std::memory_order_acquire) > 0) {
			if (_refCount.fetch_add(1, std::memory_order_acq_rel) > 0) { // _refCount++ > 0
				return this;
			} else {
				_refCount.fetch_sub(1, std::memory_order_release); // Revoke self increase, Maybe not safe
			}
		}
		return nullptr;
	}

	float View::opacity() const {
		return _color.to_float_alpha();
	}

	// =========================================================================
	void View::set_receive(bool val) {
		mark_style_flag(kRECEIVE_CssProp);
		_receive = val;
	}

	void View::set_aa(bool val) {
		mark_style_flag(kAA_CssProp);
		_aa = val;
	}

	void View::set_cursor(CursorStyle val) {
		mark_style_flag(kCURSOR_CssProp);
		_cursor = val;
	}

	void View::set_z_index(uint32_t val) {
		mark_style_flag(kZ_INDEX_CssProp);
		_async_call({ self->set_z_index_direct(arg, true); }, val);
	}

	void View::set_visible(bool val) {
		mark_style_flag(kVISIBLE_CssProp);
		_async_call({ self->set_visible_direct(arg, true); }, val);
	}

	void View::set_opacity(float val) {
		mark_style_flag(kOPACITY_CssProp);
		_async_call({ self->set_opacity_direct(arg, true); }, val);
	}

	void View::set_color(Color val) {
		mark_style_flag(kCOLOR_CssProp);
		_async_call({ self->set_color_direct(arg, true); }, val);
	}

	void View::set_cascade_color(CascadeColor val) {
		mark_style_flag(kCASCADE_COLOR_CssProp);
		_async_call({ self->set_cascade_color_direct(arg, true); }, val);
	}

	// =========================================================================
	void View::set_receive_direct(bool val, bool isRT) {
		_receive = val;
	}

	void View::set_aa_direct(bool val, bool isRT) {
		_aa = val;
	}

	void View::set_cursor_direct(CursorStyle val, bool isRT) {
		_cursor = val;
	}

	void View::set_z_index_direct(uint32_t val, bool isRT) {
		if (_z_index != val) {
			_z_index = val;
			mark_rerender();
		}
	}

	void View::set_visible_direct(bool val, bool isRT) {
		if (_visible != val) {
			_visible = val;
			if (isRT) {
				set_visible_rt(val);
			} else {
				_async_call({ self->set_visible_rt(arg); }, val);
			}
		}
	}

	void View::set_opacity_direct(float val, bool isRT) {
		uint8_t alpha8 = val * 255;
		if (_color.a() != alpha8) {
			_color.set_a(alpha8);
			mark_rerender();
		}
	}

	void View::set_color_direct(Color val, bool isRT) {
		if (_color != val) {
			_color = val;
			mark_rerender();
		}
	}

	void View::set_cascade_color_direct(CascadeColor val, bool isRT) {
		if (_cascade_color != val) {
			_cascade_color = val;
			mark_rerender(); // mark reader only
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

	bool View::has_class(cString& name) const {
		_IfCssclass()
			return _cssclass->has(name);
		return false;
	}

	void View::add_class(cString& name) {
		cssclass()->add(name);
	}

	void View::remove_class(cString& name) {
		_IfCssclass()
			_cssclass->remove(name);
	}

	void View::set_style_flag(CssProp prop) {
		if (prop < sizeof(_style_flags) * 8)
			_style_flags[prop / 32] |= (1u << (prop % 32));
	}

	void View::remove_style_flag(CssProp prop) {
		if (prop < sizeof(_style_flags) * 8)
			_style_flags[prop / 32] &= ~(1u << (prop % 32));
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

	TextOptions* View::asTextOptions() {
		return nullptr;
	}

	TextOptions* View::get_closest_text_options() {
		auto _parent = _parent_rt;
		if (!_parent)
			return shared_app()->defaultTextOptions();
		auto opts = _parent->asTextOptions();
		return opts ? opts : _parent->get_closest_text_options();
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
		{}, {}, 0,0, 0,0, View::kFixed_FloatState, View::kFixed_FloatState, false, false
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
			auto v = _first_rt;
			while (v) {
				v->set_layout_offset_free(Vec2()); // lazy view
				v = v->_next_rt;
			}
			unmark(kLayout_Typesetting);
		}
	}

	void View::layout_text(TextLines *lines, TextOptions* opts) {
		// Noop, don't support text layout in text container
	}

	void View::text_config(TextOptions* opts) {
		// Forward to subviews
		auto v = _first_rt;
		while (v) {
			if (v->visible())
				v->text_config(opts); // config subview
			v = v->_next_rt;
		}
	}

	void View::onChildLayoutChange(View *child, uint32_t mark) {
		if (mark & (kChild_Layout_Size | kChild_Layout_Visible | kChild_Layout_Align | kChild_Layout_Text)) {
			// Optimize mark value @ mark_layout<true>(kLayout_Typesetting)
			_mark_value |= kLayout_Typesetting;
			if (_mark_index == 0 && _level) {
				pre_render().mark_layout(this, _level); // push to pre render
			}
		}
	}

	void View::onActivate() {
	}

	void View::mark_(uint32_t mark) {
		if (mark) {
			_async_call({ self->mark_rt_(arg); }, mark);
		} else {
			pre_render()._rerender = true;
		}
	}

	void View::mark_layout_(uint32_t mark) {
		_async_call({ self->mark_layout_rt_(arg); }, mark);
	}

	void View::mark_rt_(uint32_t mark) {
		_mark_value |= mark;
		pre_render()._rerender = true;
	}

	void View::mark_layout_rt_(uint32_t mark) {
		_mark_value |= mark;
		if (_mark_index == 0) {
			if (_level) {
				pre_render().mark_layout(this, _level); // push to pre render
			}
		}
	}

	bool View::is_clip() {
		return false;
	}

	ViewType View::view_type() const {
		return kView_ViewType;
	}

	bool View::is_text_container() const {
		return false;
	}

	void View::set_is_focus(bool value) {
		if ( value )
			focus();
		else
			blur();
	}

	bool View::is_focus() const {
		return _window->activeView() == this;
	}

	Array<String> View::classNames() const {
		if (auto cssclass = _cssclass.load())
			return cssclass->names().keys();
		return {};
	}

	void View::set_action(Action* action) throw(Error) {
		if (action) {
			Qk_IfThrow(action->window() != _window,
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

	// NOTE: _level is only reliable on the render thread.
	// This method must not be called from the work thread.
	bool View::is_child_rt(View *child) {
		auto v = child->_parent_rt;
		while (v && v->_level >= _level) { // only check views in the same or higher level
			if ( v == this )
				return true;
			v = v->_parent_rt;
		}
		return false;
	}

	// NOTE: This method does not rely on _level.
	// It provides a conservative hierarchy check for the work thread,
	// where render-thread structural data may be stale.
	bool View::is_child(View *child) {
		if ( child ) {
			auto v = child->_parent;
			while (v) {
				if ( v == this )
					return true;
				v = v->_parent;
			}
		}
		return false;
	}

	void View::before(View *view) {
		if (view == this) return;
		auto _parent = this->_parent;
		if (_parent) {
			if (view->_parent == _parent) {
				if (view->_next == this)
					return; // already before
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
			this->_prev = view;

			async_call([](auto self, auto view) {
				auto _parent = self->_parent_rt;
				Qk_ASSERT(_parent, "View::before, parent_rt should not be null");
				if (self == view->_parent_rt) {
					view->clear_link_rt();
				} else {
					view->set_parent_rt(_parent);
				}
				auto _prev = self->_prev_rt;
				if (_prev) {
					_prev->_next_rt = view;
				} else { // There are no brothers on top
					_parent->_first_rt = view;
				}
				view->_prev_rt = _prev;
				view->_next_rt = self;
				self->_prev_rt = view;
			}, this, view);
		}
	}

	void View::after(View *view) {
		if (view == this) return;
		auto _parent = this->_parent;
		if (_parent) {
			if (view->_parent == _parent) {
				if (view->_prev == this)
					return; // already after
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
			this->_next = view;

			async_call([](auto self, auto view) {
				auto _parent = self->_parent_rt;
				Qk_ASSERT(_parent, "View::after, parent_rt should not be null");
				if (self == view->_parent_rt) {
					view->clear_link_rt();
				} else {
					view->set_parent_rt(_parent);
				}
				auto _next = self->_next_rt;
				if (_next) {
					_next->_prev_rt = view;
				} else { // There are no brothers below
					_parent->_last_rt = view;
				}
				view->_prev_rt = self;
				view->_next_rt = _next;
				self->_next_rt = view;
			}, this, view);
		}
	}

	void View::prepend(View *child) {
		if (this == child->_parent) {
			if (_first == child)
				return; // already first
			child->clear_link();
		} else {
			child->set_parent(this);
		}
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

		async_call([](auto self, auto child) {
			if (self == child->_parent_rt) {
				child->clear_link_rt();
			} else {
				child->set_parent_rt(self);
			}
			auto _first = self->_first_rt;
			if (_first) {
				child->_prev_rt = nullptr;
				child->_next_rt = _first;
				_first->_prev_rt = child;
				self->_first_rt = child;
			} else { // There are currently no sub views available yet
				child->_prev_rt = nullptr;
				child->_next_rt = nullptr;
				self->_first_rt = child;
				self->_last_rt = child;
			}
		}, this, child);
	}

	void View::append(View *child) {
		if (this == child->_parent) {
			if (_last == child)
				return; // already last
			child->clear_link();
		} else {
			child->set_parent(this);
		}
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

		async_call([](auto self, auto child) {
			if (self == child->_parent_rt) {
				child->clear_link_rt();
			} else {
				child->set_parent_rt(self);
			}
			auto _last = self->_last_rt;
			if (_last) {
				child->_prev_rt = _last;
				child->_next_rt = nullptr;
				_last->_next_rt = child;
				self->_last_rt = child;
			} else { // There are currently no sub views available yet
				child->_prev_rt = nullptr;
				child->_next_rt = nullptr;
				self->_first_rt = child;
				self->_last_rt = child;
			}
		}, this, child);
	}

	void View::remove() {
		if (_parent) {
			remove_mt();
			_async_call({
				self->remove_rt();
			}, 0);
		}
	}

	void View::remove_all_child() {
		if (_first) {
			remove_all_child_mt();
			_async_call({
				self->remove_all_child_rt();
			}, 0);
		}
	}

	void View::remove_mt() {
		blur();
		set_action(nullptr); // Delete action
		clear_link();
		_parent = nullptr;
		release(); // Disconnect from parent view strong reference
	}

	void View::remove_rt() {
		clear_link_rt();
		_parent_rt = nullptr;
		if (_level) {
			clear_level_rt();
		}
	}

	void View::remove_all_child_mt() {
		auto _first = this->_first;
		while (_first) {
			_first->remove_all_child_mt();
			_first->remove_mt();
			_first = this->_first;
		}
	}

	void View::remove_all_child_rt() {
		auto _first = this->_first_rt;
		while (_first) {
			_first->remove_all_child_rt();
			_first->remove_rt();
			_first = this->_first_rt;
		}
	}

	void View::clear_link() { // Cleaning up associated view information
		auto _parent = this->_parent;
		auto _prev = this->_prev;
		auto _next = this->_next;
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

	void View::clear_link_rt() {
		auto _parent = this->_parent_rt;
		auto _prev = this->_prev_rt;
		auto _next = this->_next_rt;
		/* Currently the first sub view */
		if (_parent->_first_rt == this) {
			_parent->_first_rt = _next;
		} else {
			_prev->_next_rt = _next;
		}
		/* Currently the last sub view */
		if (_parent->_last_rt == this) {
			_parent->_last_rt = _prev;
		} else {
			_next->_prev_rt = _prev;
		}
		_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent view
	}

	void View::set_parent(View *parent) {
		Qk_CHECK(_window == parent->_window, "window no match, parent->_window no equal _window");
		Qk_CHECK(!check_loop_ref(this, parent), "View::set_parent(parent), loop ref error");
		auto _parent = this->_parent; // old parent
		if (parent != _parent) {
			Qk_CHECK(_window->root() != this, "root view not allow set parent"); // check
			if (_parent) {
				clear_link(); // clear link from old parent
			} else { // Only when there is no parent
				retain(); // link to parent and retain ref
			}
			this->_parent = parent;
		}
	}

	void View::set_parent_rt(View *parent) {
		auto _parent = this->_parent_rt;
		if (_parent != parent) {
			if (_parent) {
				clear_link_rt();
			}
			this->_parent_rt = parent; // set new parent
			if ( _parent ) { // notice old parent
				_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent view
			}
			auto level = parent->_level;
			if (_visible && level) {
				if (_level != ++level)
					set_level_rt(level);
			} else {
				if (_level)
					clear_level_rt();
			}
			parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice new parent view
			mark_layout<true>(kLayout_Inner_Width | kLayout_Inner_Height); // mark view size, reset view size

			auto cssclass = _cssclass.load();
			if (cssclass) {
				cssclass->updateClass_rt();
			}
			onActivate();
		}
	}

	// --------------------------------------------------------------------------------------

	void View::set_visible_rt(bool visible) {
		auto _parent = this->_parent_rt;
		auto level =
			_parent && _parent->_level         ? _parent->_level + 1:
			visible && _window->root() == this ? 1: 0; // if visible and is root view then level = 1, else level = 0

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
			mark_layout<true>(kLayout_Inner_Width | kLayout_Inner_Height); // reset view size
			_IfCssclass() {
				// _cssclass->updateClass_rt();
				mark_layout<true>(View::kClass_Recursive); // mark class recursive
			}
		}
	}

	void View::set_level_rt(uint32_t level) { // settings level
		if (_visible) {
			// if level > 0 then
			if (_mark_index) {
				pre_render().unmark_layout(this, _level);
			}
			pre_render().mark_layout(this, level);
			_level = level++;
			onActivate();

			auto v = _first_rt;
			while ( v ) {
				v->set_level_rt(level);
				v = v->_next_rt;
			}
		} else {
			if ( _level )
				clear_level_rt();
		}
	}

	void View::clear_level_rt() { //  clear view depth
		auto win = _window;
		if (win->dispatch()->activeView() == this) {
			pre_render().post(Cb([this,win](auto e) {
				if (win->dispatch()->activeView() == this)
					blur();
			}),this);
		}
		if (_mark_index) {
			pre_render().unmark_layout(this, _level);
		}
		_level = 0;
		onActivate();
		auto v = _first_rt;
		while ( v ) {
			v->clear_level_rt();
			v = v->_next_rt;
		}
	}

	void View::apply_class_rt(CStyleSheetsClass *parent) {
		_Cssclass();
		if (kClass_Recursive & _mark_value) {
			apply_class_recursive_rt(parent, false);
		} else {
			_cssclass->apply_rt(parent, false, true);
			unmark(kClass_All);
		}
	}

	void View::apply_class_recursive_rt(CStyleSheetsClass *parent, bool alwaysApply) {
		_Cssclass();
		if (_cssclass) { // Impact sub view
			_cssclass->apply_rt(parent, alwaysApply, false);
			if (_cssclass->hasPropagatingStyles()) {
				parent = _cssclass;
			}
		}
		if (_visible) { // apply all sub views if visible
			auto v = _first_rt;
			while (v) {
				v->apply_class_recursive_rt(parent, alwaysApply);
				v = v->_next_rt;
			}
		}
		unmark(kClass_All);
	}

	CStyleSheetsClass* View::parent_cssclass_rt() {
		auto _parent = _parent_rt;
		while (_parent) {
			auto ss = _parent->_cssclass.load();
			if (ss && ss->hasPropagatingStyles()) {
				return ss;
			}
			_parent = _parent->_parent_rt;
		}
		return nullptr;
	}

	CursorStyle View::cursor_style_exec() {
		auto v = this;
		do {
			if (v->_cursor != CursorStyle::Normal)
				return v->_cursor;
			v = v->_parent;
		} while (v);
		return CursorStyle::Normal;
	}

	void Br::layout_text(TextLines *lines, TextOptions* opts) {
		lines->finish_text_blob_pre(); // finish previous blob
		lines->push(opts); // push new line
	}

}
