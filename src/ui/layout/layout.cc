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

#include "./layout.h"
#include "../window.h"

namespace qk {

	// view private members method
	Qk_DEFINE_INLINE_MEMBERS(Layout, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<Layout::Inl*>(self)

		static void set_visible_static(Layout::Inl* self, bool val, uint32_t layout_depth) {
			self->_visible = val;
			if (self->_parent) {
				self->_parent->onChildLayoutChange(self, kChild_Layout_Visible); // mark parent layout 
			}
			if (val) {
				self->mark_layout(kLayout_Size_Width | kLayout_Size_Height); // reset layout size
			}
			if (layout_depth) {
				self->set_layout_depth_(layout_depth);
			} else {
				self->clear_layout_depth();
			}
		}

		void clear_link() { // Cleaning up associated view information
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

		void clear_layout_depth() { //  clear layout depth
			if ( layout_depth() ) {
				// blur();
				set_layout_depth(0);
				auto v = _first;
				while ( v ) {
					_inl(v)->clear_layout_depth();
					v = v->_next;
				}
			}
		}

		void set_layout_depth_(uint32_t depth) { // settings depth
			if (_visible) {
				if ( layout_depth() != depth ) {
					set_layout_depth(depth++);
					set_window(_parent->window()); // set pre render

					if ( layout_mark() ) { // remark
						mark_layout(kLayout_None);
					}
					mark_render(kRecursive_Transform);

					auto v = _first;
					while ( v ) {
						_inl(v)->set_layout_depth_(depth);
						v = v->_next;
					}
				}
			} else {
				clear_layout_depth();
			}
		}

	};

	/**
		* @constructors
		*/
	Layout::Layout()
		: _mark_index(-1)
		, _layout_mark(kLayout_None)
		, _layout_depth(0)
		, _window(nullptr)
		, _parent(nullptr), _first(nullptr)
		, _last(nullptr), _prev(nullptr), _next(nullptr)
		, _visible(true)
		, _visible_region(false)
	{}

	/**
		* @destructor
		*/
	Layout::~Layout() {
		if (_mark_index >= 0) {
			_window->unmark_layout(this, _layout_depth); // clear mark
		}
	}

	/**
		*
		* 布局权重（比如在flex布局中代表布局的尺寸）
		*
		* @func layout_weight()
		*/
	float Layout::layout_weight() {
		return 0;
	}

	/**
		*
		* 布局的对齐方式（九宫格）
		*
		* @func layout_align()
		*/
	Align Layout::layout_align() {
		return Align::kAuto;
	}

	/**
		* 
		* Relative to the parent view (layout_offset) to start offset
		* 
		* @func layout_offset()
		*/
	Vec2 Layout::layout_offset() {
		return Vec2();
	}

	/**
		*
		* Returns the layout size of view object (if is box view the: size=margin+border+padding+content)
		*
		* Returns the layout content size of object view, 
		* Returns false to indicate that the size is unknown,
		* indicates that the size changes with the size of the subview, and the content is wrapped
		*
		* @func layout_size()
		*/
	Layout::Size Layout::layout_size() {
		return {
			Vec2(), Vec2(), true, true,
		};
	}

	/**
		*
		* Returns the and compute layout size of object view
		*
		* @func layout_raw_size()
		*/
	Layout::Size Layout::layout_raw_size(Size parent_content_size) {
		return {
			Vec2(), Vec2(), true, true,
		};
	}

	/**
		* Returns internal layout offset compensation of the view, which affects the sub view offset position
		* 
		* For example: when a view needs to set the scrolling property scroll of a subview, you can set this property
		*
		* @func layout_offset_inside()
		*/
	Vec2 Layout::layout_offset_inside() {
		return Vec2();
	}

	/**
		* 
		* Setting the layout offset of the view object in the parent view
		*
		* @func set_layout_offset(val)
		*/
	void Layout::set_layout_offset(Vec2 val) {
		// noop
	}

	/**
		* 
		* Setting layout offset values lazily mode for the view object
		*
		* @func set_layout_offset_lazy(size)
		*/
	void Layout::set_layout_offset_lazy(Vec2 size) {
		// noop
	}

	/**
		* 锁定布局的尺寸
		* 调用后自身的尺寸属性应该失效直到被解除
		* 这个方法应该在`layout_forward()`正向迭代中由父布局调用,因为尺寸的调整一般在正向迭代中
		* 
		* 返回锁定后的最终尺寸，调用后视返回后的尺寸为最终尺寸
		* 
		* @func layout_lock(layout_size)
		*/
	Vec2 Layout::layout_lock(Vec2 layout_size) {
		// noop
		return Vec2();
	}

	/**
		* @func is_lock_child_layout_size()
		*/
	bool Layout::is_lock_child_layout_size() {
		return false;
	}

	/**
		* @func set_layout_depth(newDepth)
		*/
	void Layout::set_layout_depth(uint32_t newDepth) {
		if (_layout_depth != newDepth) {
			auto oldDepth = _layout_depth;
			_layout_depth = newDepth;
			if (_mark_index >= 0) {
				_window->unmark_layout(this, oldDepth);
				if (newDepth) {
					_window->mark_layout(this, newDepth);
				}
			}
		}
	}

	/**
		* @func mark_layout(mark)
		*/
	void Layout::mark_layout(uint32_t mark) {
		_layout_mark |= mark;
		if (_mark_index < 0) {
			if (_layout_depth) {
				_window->mark_layout(this, _layout_depth); // push to pre render
			}
		}
	}

	void Layout::mark_render(uint32_t mark) {
		_layout_mark |= mark;
		if (_layout_depth) {
			_window->mark_render(); // push to pre render
		}
	}

// *******************************************************************

	/**
		* 
		* Returns layout transformation matrix of the object view
		* 
		* Mat(layout_offset + transform_origin + translate + parent->layout_offset_inside, scale, rotate, skew)
		* 
		* @method layout_matrix()
		*/
	Mat Layout::layout_matrix() {
		Vec2 translate = layout_offset() + _parent->layout_offset_inside();
		return Mat(
			1, 0, translate.x(),
			0, 1, translate.y()
		);
	}

	bool Layout::layout_forward(uint32_t mark) {
		return !(mark & kLayout_Typesetting);
	}

	bool Layout::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			auto v = _first;
			while (v) {
				v->set_layout_offset_lazy(Vec2()); // lazy layout
				v = v->next();
			}
			unmark(kLayout_Typesetting | kLayout_Size_Width | kLayout_Size_Height);
		}
		return true; // complete
	}

	void Layout::layout_text(TextLines *lines, TextConfig* cfg) {
		// NOOP
	}

	void Layout::onChildLayoutChange(Layout* child, uint32_t value) {
		if (value & (kChild_Layout_Size | kChild_Layout_Visible | kChild_Layout_Align | kChild_Layout_Text)) {
			mark_layout(kLayout_Typesetting);
		}
	}

	void Layout::onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) {
		// NOOP
	}

	void Layout::solve_marks(uint32_t mark) {

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

	Vec2 Layout::position() {
		return Vec2(_matrix[2], _matrix[5]);
	}

	bool Layout::solve_visible_region() {
		return true;
	}

	/**
	* @method overlap_test Overlap test, test whether the point on the screen overlaps with the view
	*/
	bool Layout::overlap_test(Vec2 point) {
		return false;
	}

	/**
		*
		* Setting parent parent view
		*
		* @method set_parent(parent)
		*/
	void Layout::set_parent(Layout* parent) {
		// clear parent
		if (parent != _parent) {
			_this->clear_link();

			if ( _parent ) {
				_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			} else {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
			_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			mark_layout(kLayout_Size_Width | kLayout_Size_Height); // mark layout size, reset layout size

			uint32_t depth = parent->layout_depth();
			if (depth) {
				_this->set_layout_depth_(depth + 1);
			} else {
				_this->clear_layout_depth();
			}
		}
	}

	/**
		*
		* Append subview to end
		*
		* @method append(child)
		*/
	void Layout::append(Layout* child) {
		if (this == child->_parent) {
			_inl(child)->clear_link();
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
		* Add a sibling view to the back
		*
		* @method after(view)
		*/
	void Layout::after(Layout* view) {
		if (_parent) {
			if (view == this) return;
			if (view->_parent == _parent) {
				_inl(view)->clear_link();
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

}
