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

	// view private members method
	FX_DEFINE_INLINE_MEMBERS(View, Inl) {
		public:
		#define _inl(self) static_cast<View::Inl*>(self)
		// TODO ...
	};

	View::View()
		: _action(nullptr), _parent(nullptr)
		, _first(nullptr), _last(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _next_pre_mark(nullptr)
		, _rotate(0), _opacity(1)
		, _layout_weight(0)
		, _level(0)
	{
	}

	View::~View() {
		// TODO ...
		// unload action
	}

	/**
		*
		* Add a sibling view to the front
		*
		* @func before(view)
		*/
	void View::before(View* view) throw(Error) {
		// TODO ...
	}

	/**
		*
		* Add a sibling view to the back
		*
		* @func after(view)
		*/
	void View::after(View* view) throw(Error) {
		// TODO ...
	}

	/**
		* 
		* Append subview to front
		* 
		* @func prepend(child)
		*/
	virtual void View::prepend(View* child) throw(Error) {
		// TODO ...
	}

	/**
		*
		* Append subview to end
		*
		* @func append(child)
		*/
	virtual void View::append(View* child) throw(Error) {
		// TODO ...
	}

	/**
		*
		* Remove self from parent view
		* 
		* @func remove_from_parent()
		*/
	virtual void View::remove_from_parent() {
		// TODO ...
	}

	/**
		*
		* remove all subview
		*
		* @func remove_all_child()
		*/
	virtual void View::remove_all_child() {
		// TODO ...
	}

	/**
		*
		* Setting parent parent view
		*
		* @func set_parent(parent)
		*/
	virtual void View::set_parent(View* parent) throw(Error) {
		// TODO ...
	}

	/**
		*
		* redraw view and subview
		* 
		* @func draw()
		*/
	void View::draw() {
		// TODO ...
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
				_parent->layout_weight_change_notice(this);
			}
		}
	}

	// *******************************************************************

	/**
		*
		* 从外向内正向迭代布局，比如一些布局方法是先从外部到内部先确定盒子的明确尺寸
		* 
		* @func layout_forward()
		*/
	void View::layout_forward() {
		// TODO ...
	}

	/**
		* 
		* 从内向外反向迭代布局，比如有些视图外部并没有明确的尺寸，
		* 尺寸是由内部视图挤压外部视图造成的，所以只能先明确内部视图的尺寸
		* 
		* @func layout_reverse()
		*/
	void View::layout_reverse() {
		// TODO ...
	}

	/**
		* 
		* Setting the layout offset of the view object in the parent view
		*
		* @func set_layout_offset(val)
		*/
	void View::set_layout_offset(Vec2 val) {
		if (_layout_offset != val) {
			_layout_offset = val;
			// TODO 布局偏移改变时视图以及子视图变换矩阵也会改变，MARK: MATRIX、CHILD MATRIX
		}
	}

	/**
		* 当一个父布局视图对其中所有的子视图进行布局时，为了调整各个子视图合适位置与尺寸，会调用这个函数对子视图做尺寸上的限制
		* 这个函数被调用后，其它调用尺寸更改的方法都应该失效，但应该记录被设置的数值一旦解除锁定后设置属性的才能生效
		* 
		* 调用`layout_size_lock(false)`解除锁定
		* 
		* 子类实现这个方法
		* 
		* @func layout_size_lock()
		*/
	void View::layout_size_lock(bool lock, Vec2 layout_size) {
		if (!lock) { // No locak default Vec2(0, 0)
			layout_size = Vec2();
		}
		if (layout_size != _layout_size) {
			_layout_size = layout_size;
			// TODO 布局尺寸改变时视图形状、子视图布局、兄弟视图布局都会改变，MARK: SHAPE、CHILD LAYOUT
		}
	}

	/**
		* 
		* This method of the parent view is called when the layout weight of the child view changes
		*
		* @func layout_weight_change_notice(child)
		*/
	void View::layout_weight_change_notice(View* from_child) {
		// noop
	}

	// *******************************************************************

	/**
		*
		* Returns the layout content size of object view, 
		* Returns false to indicate that the size is unknown
		*
		* @func layout_content_size(size)
		*/
	bool View::layout_content_size(Vec2& size) {
		size = _layout_size; // Explicit layout size
		return true;
	}

	/**
		* Returns internal layout offset compensation of the view, which affects the sub view offset position
		* 
		* For example: when a view needs to set the scrolling property scroll of a subview, you can set this property
		*
		* @func layout_offset_inside()
	*/
	Vec2 View::layout_offset_inside() {
		return _layout_origin;
	}

	/**
		* 
		* Returns layout transformation matrix of the object view
		* 
		* Mat(layout_offset + layout_origin + translate - parent->layout_inside_offset, scale, rotate, skew)
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

	/**
		* 
		* Returns final transformation matrix of the view layout
		*
		* parent.transform_matrix * layout_matrix
		* 
		* @func transform_matrix()
		*/
	const Mat& View::transform_matrix() {
		if (1/*MATRIX*/) { // update transform matrix
			if (_parent) {
				_parent->transform_matrix().multiplication(layout_matrix(), _transform_matrix);
			} else {
				_transform_matrix = layout_matrix();
			}
		}
		return _transform_matrix;
	}

}

// *******************************************************************