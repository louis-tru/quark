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

namespace flare {

	// --------------- V i e w :: V i s i t o r ---------------

	void View::Visitor::visitView(View *v) {
		v->visit(this);
	}

	void View::Visitor::visitBox(Box *v) {
		visitView(v);
	}

	void View::Visitor::visitGridLayout(GridLayout *v) {
		visitBox(v);
	}

	void View::Visitor::visitFlexLayout(FlexLayout *v) {
		visitBox(v);
	}

	void View::Visitor::visitFlowLayout(FlowLayout *v) {
		visitFlexLayout(v);
	}

	void View::Visitor::visitImage(Image *v) {
		visitBox(v);
	}

	void View::Visitor::visitVideo(Video *v) {
		visitImage(v);
	}

	void View::Visitor::visitText(Text *v) {
		visitBox(v);
	}

	void View::Visitor::visitScroll(Scroll *v) {
		visitFlowLayout(v);
	}

	void View::Visitor::visitRoot(Root *v) {
		visitBox(v);
	}

	void View::Visitor::visitLabel(Label *v) {
		visitView(v);
	}

	void View::Visitor::visitInput(Input *v) {
		visitBox(v);
	}

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

		/**
		* @func set_depth(depth) clear depth
		*/
		void clear_depth() {
			if ( _depth ) {
				auto old_depth = _depth;
				_depth = 0;
				layout_depth_change_notice(old_depth, 0);
				blur();
				
				View *v = _first;
				while ( v ) {
					_inl(v)->clear_depth();
					v = v->_next;
				}
			}
		}

		/**
		* @func set_depth(depth) settings depth
		*/
		void set_depth(uint32_t depth) {
			if (_visible) {
				if ( _depth != depth ) {
					auto old_depth = _depth;
					_depth = depth++;
					layout_depth_change_notice(old_depth, _depth);

					if ( layout_mark() ) { // remark
						mark(M_NONE);
						auto r = layout_mark() & M_RECURSIVE;
						if (r) {
							mark_recursive(r);
						}
					}

					View *v = _first;
					while ( v ) {
						_inl(v)->set_depth(depth);
						v = v->_next;
					}
				}
			} else {
				clear_depth();
			}
		}

		/**
			* 
			* get transform pointer
			* 
			* @func transform()
			*/
		Transform* transform() {
			if (!_transform) {
				_transform = new Transform();
				_transform->scale = Vec2(1);
				_transform->rotate = 0;
			}
			return _transform;
		}

	};

	View::View()
		: _action(nullptr), _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		//, _depth(0)
		, _transform(nullptr), _opacity(1.0)
		, _visible(true)
		, _region_visible(false)
		, _receive(false)
	{
	}

	View::~View() {
		ASSERT(_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
		blur();
		set_action(nullptr); // del action
		_inl(this)->remove_all_child_(); // 删除子视图
		delete _transform; _transform = nullptr;
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
			auto old_depth = _depth;
			_depth = 0;
			layout_depth_change_notice(old_depth, _depth);
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
			
			if ( _parent ) {
				_parent->layout_content_change_notice(this); // notice parent layout
			} else {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
			_parent->layout_content_change_notice(this); // notice parent layout
			mark(M_LAYOUT_SIZE); // mark layout size, reset layout size

			auto depth = parent->_depth;
			if (depth) {
				_inl(this)->set_depth(depth + 1);
			} else {
				_inl(this)->clear_depth();
			}
		}
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
			if (_parent) {
				_parent->layout_content_change_notice(this); // mark parent layout 
			}
			if (_visible) {
				mark(M_LAYOUT_SIZE); // reset layout size
			}
			if (_parent && _parent->_depth) {
				_inl(this)->set_depth(_parent->_depth + 1);
			} else {
				_inl(this)->clear_depth();
			}
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
		* Returns matrix displacement for the view
		*
		* @func translate
		*/
	Vec2 View::translate() const {
		return _transform ? _transform->translate: Vec2();
	}

	/**
		* Returns the Matrix scaling
		*
		* @func scale()
		*/
	Vec2 View::scale() const {
		return _transform ? _transform->scale: Vec2(1);
	}

	/**
		* Returns the Matrix skew
		*
		* @func skew()
		*/
	Vec2 View::skew() const {
		return _transform ? _transform->skew: Vec2();
	}

	/**
		* Returns the z-axis rotation of the matrix
		*
		* @func rotate()
		*/
	float View::rotate() const {
		return _transform ? _transform->rotate: 0;
	}

	/**
		* Set the matrix `translate` properties of the view object
		*
		* @func set_translate(val)
		*/
	void View::set_translate(Vec2 val) {
		if (translate() != val) {
			_inl(this)->transform()->translate = val;
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* Set the matrix `scale` properties of the view object
		*
		* @func set_scale(val)
		*/
	void View::set_scale(Vec2 val) {
		if (scale() != val) {
			_inl(this)->transform()->scale = val;
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* Set the matrix `skew` properties of the view object
		*
		* @func set_skew(val)
		*/
	void View::set_skew(Vec2 val) {
		if (skew() != val) {
			_inl(this)->transform()->skew = val;
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* Set the z-axis  matrix `rotate` properties of the view object
		*
		* @func set_rotate(val)
		*/
	void View::set_rotate(float val) {
		if (rotate() != val) {
			_inl(this)->transform()->rotate = val;
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* 
		* Setting x-axis matrix displacement for the view
		*
		* @func set_x(val)
		*/
	void View::set_x(float val) {
		if (translate().x() != val) {
			_inl(this)->transform()->translate.x(val);
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* 
		* Setting y-axis matrix displacement for the view
		*
		* @func set_y(val)
		*/
	void View::set_y(float val) {
		if (translate().y() != val) {
			_inl(this)->transform()->translate.y(val);
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @func set_scale_x(val)
		*/
	void View::set_scale_x(float val) {
		if (scale().x() != val) {
			_inl(this)->transform()->scale.x(val);
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @func set_scale_y(val)
		*/
	void View::set_scale_y(float val) {
		if (scale().y() != val) {
			_inl(this)->transform()->scale.y(val);
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @func set_skew_x(val)
		*/
	void View::set_skew_x(float val) {
		if (skew().x() != val) {
			_inl(this)->transform()->skew.x(val);
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @func set_skew_y(val)
		*/
	void View::set_skew_y(float val) {
		if (skew().y() != val) {
			_inl(this)->transform()->skew.y(val);
			mark_recursive(M_TRANSFORM); // mark transform
		}
	}
	
	/**
		* Set the `opacity` properties of the view object
		*
		* @func set_opacity(val)
		*/
	void View::set_opacity(float val) {
		if (_opacity != val) {
			_opacity = val;
			mark_none(); // mark none
		}
	}

	// *******************************************************************

	/**
		* 
		* compute the transform origin value
		* 
		* @func layout_transform_origin(t)
		*/
	Vec2 View::layout_transform_origin(Transform& t) {
		// TODO compute transform origin ...
		return Vec2();
	}

	/**
		* 
		* Returns layout transformation matrix of the object view
		* 
		* Mat(layout_offset + transform_origin + translate - parent->layout_offset_inside, scale, rotate, skew)
		* 
		* @func layout_matrix()
		*/
	Mat View::layout_matrix() {
		Vec2 in = _parent ? _parent->layout_offset_inside(): Vec2();
		if (_transform) {
			return Mat(
				layout_offset() + _transform->origin +
				_transform->translate - in,
				_transform->scale,
				-_transform->rotate, _transform->skew
			);
		} else {
			Vec2 translate = layout_offset() - in;
			return Mat(
				1, 0, translate.x(),
				0, 1, translate.y()
			);
		}
	}

	/**
		* Start the matrix transformation from this origin point
		*
		* @func transform_origin()
		*/
	Vec2 View::transform_origin() const {
		return _transform ? _transform->origin: Vec2();
	}

	// --------------- o v e r w r i t e ---------------

	bool View::layout_forward(uint32_t mark) {
		// noop
		return true;
	}

	bool View::layout_reverse(uint32_t mark) {
		// noop
		// call child->set_layout_offset_lazy()
		return true;
	}

	void View::layout_recursive(uint32_t mark) {
		if (!_depth) return;

		if (mark & M_TRANSFORM_ORIGIN) {
			// mark |= M_TRANSFORM;
			// mark &= ~M_TRANSFORM_ORIGIN;
			if (_transform) {
				_transform->origin = layout_transform_origin(*_transform);
				unmark(M_TRANSFORM_ORIGIN); // unmark
				goto tran;
			}
		}

		if (mark & M_TRANSFORM) { // update transform matrix
			tran:
			if (_parent) {
				_parent->matrix().multiplication(layout_matrix(), _matrix);
			} else {
				_matrix = layout_matrix();
			}
			unmark(M_TRANSFORM); // unmark
			
			View *v = _first;
			while (v) {
				layout_recursive(M_TRANSFORM | v->layout_mark());
				v = v->_next;
			}
		}
	}

	Vec2 View::layout_offset_inside() {
		return transform_origin();
	}

}

// *******************************************************************