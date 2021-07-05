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

#include "./layout.h"
#include "../_pre-render.h"

namespace flare {

	/**
		* @constructors
		*/
	Layout::Layout()
		: _mark_index(-1)
		, _recursive_mark_index(-1)
		, _layout_mark(M_NONE)
		, _depth(0)
	{
		ASSERT(app());
	}

	/**
		* @destructor
		*/
	Layout::~Layout() {
		set_layout_depth(0);
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
	Layout::LayoutAlign Layout::layout_align() {
		return AUTO;
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
		* @func layout_size()
		*/
	Vec2 Layout::layout_size() {
		return Vec2();
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
		* Returns the layout content size of object view, 
		* Returns false to indicate that the size is unknown,
		* indicates that the size changes with the size of the subview, and the content is wrapped
		*
		* @func layout_content_size(is_explicit_width, is_explicit_height)
		*/
	Vec2 Layout::layout_content_size(bool& is_explicit_width, bool& is_explicit_height) {
		is_explicit_width = false; // No definite size
		is_explicit_height = false;
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
		* @func set_layout_offset_lazy(rect)
		*/
	void Layout::set_layout_offset_lazy(Rect rect) {
		// noop
	}

	/**
		* 当一个父布局视图对其中所拥有的子视图进行布局时，为了调整各个子视图合适位置与尺寸，如有必要可以调用这个函数对子视图做尺寸限制
		* 这个函数被调用后，子视图上任何调用尺寸更改的方法都应该失效，但应该记录更改的数值一旦解除锁定后之前更改尺寸属性才可生效
		* 
		* 调用`lock_layout_size(Vec2(-1,-1))`解除锁定
		* 
		* 子类实现这个方法
		* 
		* 返回锁定后的最终尺寸
		* 
		* @func lock_layout_size(layout_size)
		*/
	Vec2 Layout::lock_layout_size(Vec2 layout_size) {
		// noop
		return layout_size();
	}

	/**
		*
		* This method of the parent view is called when the layout content of the child view changes
		* 
		* @func layout_typesetting_change(child)
		*/
	void Layout::layout_typesetting_change(Layout* child) {
		// noop
	}

	/**
		* 
		* This method of the parent view is called when the layout weight of the child view changes
		* 
		* @func layout_typesetting_change_from_child_weight(child)
		*/
	void Layout::layout_typesetting_change_from_child_weight(Layout* child) {
		// noop
	}

	/**
		* 
		* This method of the child view is called when the layout size of the parent view changes
		* 
		* @func layout_content_size_change_from_parent(parent, mark)
		*/
	void Layout::layout_content_size_change_from_parent(Layout* parent, uint32_t mark) {
		// noop
	}

	/**
		* @func set_layout_depth(newDepth)
		*/
	void Layout::set_layout_depth(uint32_t newDepth) {
		if (_depth != newDepth) {
			auto oldDepth = _depth;
			_depth = newDepth;
			// ASSERT(app());
			auto pre = app()->pre_render();
			if (_mark_index >= 0) {
				pre->delete_mark(this, oldDepth);
				if (newDepth) {
					pre->mark(this, newDepth);
				}
			}
			if (_recursive_mark_index >= 0) {
				pre->delete_mark_recursive(this, oldDepth);
				if (newDepth) {
					pre->mark_recursive(this, newDepth);
				}
			}
		}
	}

	/**
		* @func mark(mark)
		*/
	void Layout::mark(uint32_t mark) {
		_layout_mark |= mark;
		if (_mark_index < 0) {
			auto depth = layout_depth();
			if (depth) {
				app()->pre_render()->mark(this, depth); // push to pre render
			}
		}
	}

	void Layout::mark_recursive(uint32_t mark) {
		_layout_mark |= mark;
		if (_recursive_mark_index < 0) {
			auto depth = layout_depth();
			if (depth) {
				app()->pre_render()->mark_recursive(this, depth); // push to pre render
			}
		}
	}

	/**
		* @func mark_none()
		*/
	void Layout::mark_none() {
		app()->pre_render()->mark_none(this);
	}

}

// *******************************************************************