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

#include "./view.h"

namespace ftr {

	// --------------- L a y o u t  V i e w ---------------

	// view private members method
	FX_DEFINE_INLINE_MEMBERS(View, Inl) {
		public:
		#define _inl(self) static_cast<View::Inl*>(self)

		/**
		* @func remove_all_child_()
		*/
		void remove_all_child_() {
			while (_first) {
				_first->remove();
			}
		}
		
		/**
		* @func clear() Cleaning up associated view information
		*/
		void clear() {
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

		void clear_depth_and_visibility() {
			if ( _depth ) {
				_depth = 0;
				_visibility = false;
				layout_depth_change_notice(_depth);
				mark(M_LAYOUT); // TODO ...
				blur();

				View *v = _first;
				while ( v ) {
					_inl(v)->clear_depth_and_visibility();
					v = v->_next;
				}
			}
		}
		
		/**
		* @func set_depth_and_visibility(depth, visibility) settings depth and final visibility
		*/
		void set_depth_and_visibility(uint32_t depth, bool visibility) {
			_depth = depth++;
			_visibility = visibility = visibility && _visible;
			layout_depth_change_notice(_depth);
			mark(M_LAYOUT); // TODO ...

			if ( !visibility ) {
				blur();
			}
			View *v = _first;
			while ( v ) {
				_inl(v)->set_depth_and_visibility(depth, visibility);
				v = v->_next;
			}
		}
		
		/**
		* @func set_visibility_true
		*/
		void set_visibility_true() {
			if ( _visible && ! _visibility ) {
				_visibility = true;
				mark(M_LAYOUT); // TODO ...

				View *v = _first;
				while (v) {
					_inl(v)->set_visibility_true();
					v = v->_next;
				}
			}
		}

		/**
		* @func set_visibility_false
		*/
		void set_visibility_false() {
			if ( _visibility ) {
				_visibility = false;
				mark(M_LAYOUT); // TODO ...
				blur();

				View *v = _first;
				while (v) {
					_inl(v)->set_visibility_false();
					v = v->_next;
				}
			}
		}
		
	};

	View::View()
		: _action(nullptr), _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _depth(0), _layout_weight(0.0)
		, _rotate(0.0), _opacity(1.0)
		, _visible(true)
		, _visibility(false)
		, _region_visible(false)
		, _receive(false)
	{
	}

	View::~View() {
		ASSERT(_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
		blur();
		set_action(nullptr); // del action
		_inl(this)->remove_all_child_(); // 删除子视图
	}

	/**
		*
		* Add a sibling view to the front
		*
		* @func before(view)
		*/
	void View::before(View* view) {
		if (_parent) {
			if (view == this) return;
			if (view->_parent == _parent) {
				_inl(view)->clear();  // 清除关联
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
		* @func after(view)
		*/
	void View::after(View* view) {
		if (_parent) {
			if (view == this) return;
			if (view->_parent == _parent) {
				_inl(view)->clear(); // 清除关联
			} else {
				view->set_parent(_parent);
			}
			if (m_next) {
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
		* @func prepend(child)
		*/
	void View::prepend(View* child) {
		if (this == child->_parent) {
			_inl(child)->clear();
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
		* @func append(child)
		*/
	void View::append(View* child) {
		if (this == child->_parent) {
			_inl(child)->clear();
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

	/**
		*
		* Remove and destroy self
		* 
		* @func remove()
		*/
	void View::remove() {
		if (_parent) {
			blur(); // 辞去焦点
			set_action(nullptr); // del action
			_inl(this)->remove_all_child_(); // 删除子视图
			_inl(this)->clear();
			// remove_event_listener(); // TODO
			_depth = 0;
			layout_depth_change_notice(_depth);
			_parent = _prev = _next = nullptr;
			release(); // Disconnect from parent view strong reference
		}
		else {
			// remove_event_listener();
			set_action(nullptr); // del action
			_inl(this)->remove_all_child_(); // 删除子视图
		}
	}

	/**
		*
		* remove all subview
		*
		* @func remove_all_child()
		*/
	void View::remove_all_child() {
		_inl(this)->remove_all_child_();
	}

	/**
		*
		* Setting parent parent view
		*
		* @func set_parent(parent)
		*/
	void View::set_parent(View* parent) {
		// clear parent
		if (parent != _parent) {
			_inl(this)->clear();
			
			if ( !_parent ) {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
			
			uint32_t depth = parent->_depth;
			if (depth) { // 设置depth
				if ( depth + 1 != _depth ) {
					_inl(this)->set_depth_and_visibility(depth + 1, parent->_visibility);
				} else {
					if ( _visibility != parent->_visibility ) {
						if ( _visibility ) {
							_inl(this)->set_visibility_false();
						} else {
							_inl(this)->set_visibility_true();
						}
					}
				}
			} else {
				_inl(this)->clear_depth_and_visibility();
			}
			// TODO ...
			// 这些标记是必需的
			// mark_pre( M_MATRIX | M_SHAPE | M_OPACITY | M_STYLE_FULL );
		}
	}

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void View::accept(Visitor *visitor) {
		visitor->visitView(this);
	}

	/**
		* @func visit(visitor)
		*/
	void View::visit(Visitor *visitor) {
		auto v = _first;
		while(v) {
			v->accept(visitor);
			v = v->_next;
		}
	}

	/**
		* 
		* Sets whether the view needs to receive or handle event throws from the system
		*
		* @func set_receive()
		*/
	void View::set_receive(bool val) {
		_receive = val;
	}

	/**
		* 
		* Setting the visibility properties the view object
		*
		* @func set_visible(val)
		*/
	void View::set_visible(bool val) {
		if (_visible != val) {
			_visible = val;
			if (_visible) {
				if ( _parent && _parent->_visibility ) { // 父视图的显示状态必须要为true才能生效
					_inl(this)->set_visibility_true();
				}
			} else {
				_inl(this)->set_visibility_false();
			}
			// TODO MARK: M_VISIBLE
		}
	}

	/**
	 * @func focus()
	 */
	bool View::focus() {
		return true; // TODO ...
	}
	
	/**
	 * @func blur()
	 */
	bool View::blur() {
		return true; // TODO ...
	}
	
	/**
	 * @func is_focus()
	 */
	bool View::is_focus() const {
		return true; // TODO ...
	}
	
	/**
	 *
	 * Can it be the focus
	 * 
	 * @func can_become_focus()
	 */
	bool View::can_become_focus() {
		return true; // TODO ...
	}

	// *******************************************************************

	/**
		* Set the `action` properties of the view object
		*
		* @func set_action()
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

	/**
		* Set the matrix `translate` properties of the view object
		*
		* @func set_translate()
		*/
	void View::set_translate(Vec2 val) {
		if (_translate != val) {
			_translate = val;
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* Set the matrix `scale` properties of the view object
		*
		* @func set_scale()
		*/
	void View::set_scale(Vec2 val) {
		if (_scale != val) {
			_scale = val;
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* Set the matrix `skew` properties of the view object
		*
		* @func set_skew()
		*/
	void View::set_skew(Vec2 val) {
		if (_skew != val) {
			_skew = val;
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* Set the z-axis  matrix `rotate` properties of the view object
		*
		* @func set_rotate()
		*/
	void View::set_rotate(float val) {
		if (_rotate != val) {
			_rotate = val;
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* 
		* Setting x-axis matrix displacement for the view
		*
		* @func x()
		*/
	void View::set_x(float val) {
		if (_translate.x() != val) {
			_translate.x(val);
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* 
		* Setting y-axis matrix displacement for the view
		*
		* @func y()
		*/
	void View::set_y(float val) {
		if (_translate.y() != val) {
			_translate.y(val);
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @func scale_x()
		*/
	void View::scale_x(float val) {
		if (_scale.x() != val) {
			_scale.x(val);
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @func scale_y()
		*/
	void View::scale_y(float val) {
		if (_scale.y() != val) {
			_scale.y(val);
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @func skew_x()
		*/
	void View::skew_x(float val) {
		if (_skew.x() != val) {
			_skew.x(val);
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @func skew_y()
		*/
	void View::skew_y(float val) {
		if (_skew.y() != val) {
			_skew.y(val);
			// TODO MARK: MATRIX、CHILD MATRIX
		}
	}
	
	/**
		* Set the `opacity` properties of the view object
		*
		* @func set_opacity()
		*/
	void View::set_opacity(float val) {
		if (_opacity != val) {
			_opacity = val;
			// TODO Mark redraw view if visible，MARK: REDRAW
		}
	}

	// *******************************************************************

	/**
		* 
		* setting the layout weight of the view object
		* 
		* @func set_layout_weight(val)
		*/
	void View::set_layout_weight(float val) {
		if (_layout_weight != val) {
			_layout_weight = val;
			// TODO 重新标记父视图需要重新对子视图进行偏移布局，MAKE: PARENT WEIGHT LAYOUT
			if (_parent) {
				_parent->layout_weight_change_notice_from_child(this);
			}
		}
	}

	/**
		* 
		* Returns layout transformation matrix of the object view
		* 
		* Mat(layout_offset + layout_origin + translate - parent->layout_offset_inside, scale, rotate, skew)
		* 
		* @func layout_matrix()
		*/
	Mat View::layout_matrix() const {
		Vec2 in = _parent ? _parent->layout_offset_inside(): Vec2();
		Vec2 translate(
			_layout_offset.x() + _layout_origin.x() + _translate.x() - in.x(),
			_layout_offset.y() + _layout_origin.y() + _translate.y() - in.y()
		); // xy offset
		return Mat(translate, _scale, -_rotate, _skew);
	}

	// --------------- o v e r w r i t e ---------------

	uint32_t View::layout_depth() {
		return _depth;
	}

	bool View::layout_forward(uint32_t mark) {
		// TODO ...
		return true;
	}

	bool View::layout_reverse(uint32_t mark) {
		// TODO ...
		return true;
	}

	void View::layout_recursive(uint32_t mark) {
		if (!_visibility) return;

		if (mark & M_TRANSFORM) { // update transform matrix
			if (_parent) {
				_parent->matrix().multiplication(layout_matrix(), _matrix);
			} else {
				_matrix = layout_matrix();
			}
			unmark(M_TRANSFORM); // unmark
			
			View *v = _first;
			while (v) {
				layout_recursive(mark & v->layout_mark());
				v = v->_next;
			}
		}
	}

	void View::layout_size_lock(bool lock, Vec2 layout_size) {
		if (!lock) { // No locak default Vec2(0, 0)
			layout_size = Vec2();
		}
		if (layout_size != _layout_size) {
			_layout_size = layout_size;
			// TODO 布局尺寸改变时视图形状、子视图布局、兄弟视图布局都会改变，MARK: SHAPE、CHILD LAYOUT
		}
	}

	Vec2 View::layout_size() {
		return _layout_size;
	}

	bool View::layout_content_size(Vec2& size) {
		size = _layout_size; // Explicit layout size
		return true;
	}

	Vec2 View::layout_offset_inside() {
		return _layout_origin;
	}

	float View::layout_weight() {
		return _layout_weight;
	}

}

// *******************************************************************