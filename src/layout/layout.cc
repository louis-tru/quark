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
#include "../pre_render.h"
#include "../app.h"

namespace noug {

	Layout::Layout(): Layout(noug::pre_render()) {
	}

	/**
		* @constructors
		*/
	Layout::Layout(PreRender* pre)
		: _mark_index(-1)
		, _layout_mark(kLayout_None)
		, _layout_depth(0)
		, _pre_render(pre)
	{
		N_Assert(pre, "#Layout#Layout PreRender host cannot be null");
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
	Align Layout::layout_align() {
		return Align::AUTO;
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
				_pre_render->delete_mark(this, oldDepth);
				if (newDepth) {
					_pre_render->mark(this, newDepth);
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
			if (_layout_depth) {
				_pre_render->mark(this, _layout_depth); // push to pre render
			}
		}
	}

	void Layout::mark_none(uint32_t mark) {
		_layout_mark |= mark;
		if (_layout_depth) {
			_pre_render->mark_none(); // push to pre render
		}
	}


// *******************************************************************
}
