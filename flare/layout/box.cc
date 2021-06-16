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

#include "./box.h"

namespace flare {

	/**
		* @constructors
		*/
	Box::Box()
		: _limit_width(0, NONE)
		, _limit_height(0, NONE)
		, _margin_top(0)
		, _margin_right(0)
		, _margin_bottom(0)
		, _margin_left(0)
		, _padding_top(0)
		, _padding_right(0)
		, _padding_bottom(0)
		, _padding_left(0)
		, _fill(nullptr)
		, _layout_weight(0)
		, _layout_align(AUTO)
	{
	}

	/**
		* @destructor
		*/
	Box::~Box() {
	}
	
	/**
		*
		* 设置宽度
		*
		* @func set_width(width)
		*/
	void Box::set_width(SizeValue width) {
		if (_width != width) {
			// TODO ...
		}
	}

	/**
		*
		* 设置高度
		*
		* @func set_height(height)
		*/
	void Box::set_height(SizeValue height) {
		if (_height != height) {
			// TODO ...
		}
	}

	/**
		*
		* 设置最大宽度限制
		*
		* @func set_limit_width(limit_width)
		*/
	void Box::set_limit_width(SizeValue limit_width) {
		if (_limit_width != limit_width) {
			// TODO ...
		}
	}

	/**
		*
		* 设置最大高度限制
		*
		* @func set_limit_height(limit_height)
		*/
	void Box::set_limit_height(SizeValue limit_height) {
		if (_limit_height != limit_height) {
			// TODO ...
		}
	}

	void Box::margin_top(float val) { // margin
		if (_padding_top != val) {
			// TODO ...
		}
	}

	void Box::margin_right(float val) {
		if (_padding_right != val) {
			// TODO ...
		}
	}

	void Box::margin_bottom(float val) {
		if (_padding_bottom != val) {
			// TODO ...
		}
	}

	void Box::margin_left(float val) {
		if (_padding_left != val) {
			// TODO ...
		}
	}

	void Box::padding_top(float val) { // padding
		if (_padding_top != val) {
			// TODO ...
		}
	}

	void Box::padding_right(float val) {
		if (_padding_right != val) {
			// TODO ...
		}
	}

	void Box::padding_bottom(float val) {
		if (_padding_bottom != val) {
			// TODO ...
		}
	}

	void Box::padding_left(float val) {
		if (_padding_left != val) {
			// TODO ...
		}
	}
	
	void Box::fill(FillPtr val) {
		if (_fill != val) {
			// TODO ...
		}
	}

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void Box::accept(Visitor *visitor) {
		visitor->visitBox(this);
	}

	bool Box::layout_forward(uint32_t mark) {
		// ...
		return true;
	}

	bool Box::layout_reverse(uint32_t mark) {
		// ...
		return true;
	}

	void Box::layout_recursive(uint32_t mark) {
		View::layout_recursive(mark);
	}

	Vec2 Box::layout_offset() {
		return _layout_offset;
	}

	Vec2 Box::layout_size() {
		return _layout_size;
	}

	Vec2 Box::layout_content_size(bool& is_explicit_out) {
		// is_explicit_out = false;
		return Vec2();
	}

	float Box::layout_weight() {
		return _layout_weight;
	}

	Layout::LayoutAlign layout_align() {
		return _layout_align;
	}

	/**
		*
		* 设置布局对齐方式
		*
		* @func set_layout_align(align)
		*/
	void Box::set_layout_align(LayoutAlign align) {
		if (_layout_align != align) {
			_layout_align = align;
			if (parent()) {
				parent()->layout_content_change_notice(this);
			}
		}
	}

	/**
		*
		* 设置布局权重
		*
		* @func set_layout_weight(weight)
		*/
	void Box::set_layout_weight(float weight) {
		if (_layout_weight != weight) {
			_layout_weight = weight;
			if (parent()) {
				parent()->layout_weight_change_notice_from_child(this);
			}
		}
	}

	Vec2 Box::layout_transform_origin(Transform& t) {
		// TODO compute transform origin ...
		return Vec2();
	}

	Vec2 Box::lock_layout_size(Vec2 layout_size) {
		// ...
		return Vec2();
	}

	void Box::set_layout_offset(Vec2 val) {
		if (val != _layout_offset) {
			_layout_offset = val;
			mark_recursive(M_TRANSFORM); // mark recursive transform
		}
	}

	void Box::set_layout_offset_lazy() {
		// ...
	}

	void Box::layout_content_change_notice(Layout* child) {
		// ... 
	}

	void View::layout_size_change_notice_from_parent(Layout* parent) {
		// ...
		// mark(M_LAYOUT_SIZE);
	}

}

// *******************************************************************