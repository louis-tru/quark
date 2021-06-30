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

	// box private members method
	FX_DEFINE_INLINE_MEMBERS(View, Inl) {
		public:
	};

	/**
		* @constructors
		*/
	Box::Box()
		: _limit_width(0, NONE), _limit_height(0, NONE)
		, _margin_top(0), _margin_right(0)
		, _margin_bottom(0), _margin_left(0)
		, _padding_top(0), _padding_right(0)
		, _padding_bottom(0), _padding_left(0)
		, _fill(nullptr)
		, _layout_weight(0), _layout_align(AUTO)
		, _explicit_width(false), _explicit_height(false)
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
	void Box::set_width(SizeValue val) {
		if (_width != val) {
			_width = val;
			mark(M_LAYOUT_WIDTH);
		}
	}

	/**
		*
		* 设置高度
		*
		* @func set_height(height)
		*/
	void Box::set_height(SizeValue val) {
		if (_height != val) {
			_height = val;
			mark(M_LAYOUT_SIZE_HEIGHT);
		}
	}

	/**
		*
		* 设置最大宽度限制
		*
		* @func set_limit_width(limit_width)
		*/
	void Box::set_limit_width(SizeValue val) {
		if (_limit_width != val) {
			_limit_width = val;
			mark(M_LAYOUT_SIZE_WIDTH);
		}
	}

	/**
		*
		* 设置最大高度限制
		*
		* @func set_limit_height(limit_height)
		*/
	void Box::set_limit_height(SizeValue val) {
		if (_limit_height != val) {
			_limit_height = val;
			mark(M_LAYOUT_SIZE_HEIGHT);
		}
	}

	void Box::margin_top(float val) { // margin
		if (_margin_top != val) {
			_margin_top = val;
			mark(M_LAYOUT_SIZE_HEIGHT);
			mark_recursive(M_TRANSFORM_ORIGIN);
		}
	}

	void Box::margin_right(float val) {
		if (_margin_right != val) {
			_margin_right = val;
			mark(M_LAYOUT_SIZE_WIDTH);
		}
	}

	void Box::margin_bottom(float val) {
		if (_margin_bottom != val) {
			_margin_bottom = val;
			mark(M_LAYOUT_SIZE_HEIGHT);
		}
	}

	void Box::margin_left(float val) {
		if (_margin_left != val) {
			_margin_left = val;
			mark(M_LAYOUT_SIZE_WIDTH);
			mark_recursive(M_TRANSFORM_ORIGIN); // 必然影响origin变化，因为margin是transform origin的一部分
		}
	}

	void Box::padding_top(float val) { // padding
		if (_padding_top != val) {
			_padding_top = val;
			mark(M_LAYOUT_SIZE_HEIGHT);
			mark_recursive(M_TRANSFORM/* | M_TRANSFORM_ORIGIN*/); // 影响子布局偏移、影响尺寸变化进而影响origin
		}
	}

	void Box::padding_right(float val) {
		if (_padding_right != val) {
			_padding_right = val;
			mark(M_LAYOUT_SIZE_WIDTH);
			// mark_recursive(M_TRANSFORM_ORIGIN);
		}
	}

	void Box::padding_bottom(float val) {
		if (_padding_bottom != val) {
			_padding_bottom = val;
			mark(M_LAYOUT_SIZE_HEIGHT);
			// mark_recursive(M_TRANSFORM_ORIGIN);
		}
	}

	void Box::padding_left(float val) {
		if (_padding_left != val) {
			_padding_left = val;
			mark(M_LAYOUT_SIZE_WIDTH);
			mark_recursive(M_TRANSFORM); // 几乎可以肯定会影响子布局偏移
		}
	}
	
	void Box::fill(FillPtr val) {
		if (_fill != val) {
			// TODO ...
			_fill = val; // ?
			mark_none();
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
		uint32_t layout_content_size_change_mark = M_NONE;

		if (mark & M_LAYOUT_SIZE_WIDTH) {

			bool is_explicit, _;
			float p_value = parent()->layout_content_size(is_explicit, _).width(), value;

			switch (_width.type) {
				default: // NONE /* none default wrap content */
				case WRAP: /* 包裹内容 wrap content */
					_explicit_width = false;
					value = 0; // invalid wrap width
					break;
				case PIXEL: /* 明确值 value px */
					_explicit_width = true;
					value = _width.value;
					break;
				case MATCH: /* 匹配父视图 match parent */
					if (is_explicit) {
						value = Number<float>::max(
							p_value - _margin_left - _margin_right - _padding_left - _padding_right, 0
						);
					} else { // use wrap
						value = 0; // invalid wrap width
					}
					_explicit_width = is_explicit;
					break;
				case RATIO: /* 百分比 value % */
					if (is_explicit) {
						value = Number<float>::max(p_value * _width.value, 0);
					} else { // use wrap
						value = 0; // invalid wrap width
					}
					_explicit_width = is_explicit;
					break;
				case MINUS: /* 减法(parent-value) value ! */
					if (is_explicit) {
						value = Number<float>::max(p_value - _width.value, 0);
					} else { // use wrap
						value = 0; // invalid wrap width
					}
					_explicit_width = is_explicit;
					break;
			}

			if (value != _layout_content_size.width()) {
				_layout_content_size.width(value);
				// mark(M_LAYOUT_TYPESETTING);
				layout_content_size_change_mark |= M_LAYOUT_SIZE_WIDTH;
			}
			unmark(M_LAYOUT_SIZE_WIDTH);

			parent()->layout_typesetting_change_notice_from_child(this);
		}

		if (mark & M_LAYOUT_SIZE_HEIGHT) {

			bool is_explicit, _;
			auto p_value = parent()->layout_content_height(is_explicit, _).height(), value;

			switch (_height.type) {
				default: // NONE /* none default wrap content */
				case WRAP: /* 包裹内容 wrap content */
					_explicit_height = false;
					value = 0; // invalid wrap height
					break;
				case PIXEL: /* 明确值 value px */
					_explicit_height = true;
					_layout_content_size.height(_height.value);
					break;
				case MATCH: /* 匹配父视图 match parent */
					if (is_explicit) {
						value = Number<float>::max(
							p_value - _margin_top - _margin_bottom - _padding_top - _padding_bottom, 0
						);
					} else { // use wrap
						value = 0; // invalid wrap height
					}
					_explicit_height = is_explicit;
					break;
				case RATIO: /* 百分比 value % */
					if (is_explicit) {
						value = Number<float>::max(p_value * _height.value, 0);
					} else { // use wrap
						value = 0; // invalid wrap height
					}
					_explicit_height = is_explicit;
					break;
				case MINUS: /* 减法(parent-value) value ! */
					if (is_explicit) {
						value = Number<float>::max(p_value - _height.value, 0);
					} else { // use wrap
						value = 0; // invalid wrap height
					}
					_explicit_height = is_explicit;
					break;
			}

			if (value != _layout_content_size.height()) {
				_layout_content_size.height(value);
				// mark(M_LAYOUT_TYPESETTING);
				layout_content_size_change_mark |= M_LAYOUT_SIZE_HEIGHT;
			}
			unmark(M_LAYOUT_SIZE_HEIGHT);

			parent()->layout_typesetting_change_notice_from_child(this);
		}

		if (layout_content_size_change_mark) {
			auto v = _first;
			while (v) {
				v->layout_content_size_change_notice_from_parent(this, layout_content_size_change_mark);
				v = v->next();
			}
			mark(M_LAYOUT_TYPESETTING); // rearrange
		}

		return !(layout_mark() & M_LAYOUT_TYPESETTING);
	}

	bool Box::layout_reverse(uint32_t mark) {

		if (mark & (M_LAYOUT_TYPESETTING)) {
			auto v = _first;
			Rect rect = {
				Vec2(_margin_left + _padding_left, _margin_top + _padding_top),
				_layout_content_size,
			};
			while (v) {
				v->set_layout_offset_lazy(rect); // lazy layout
				v = v->next();
			}
			unmark(M_LAYOUT_TYPESETTING);
		}

		return true;
	}

	Vec2 Box::layout_offset() {
		return _layout_offset;
	}

	Vec2 Box::layout_size() {
		return _layout_size;
	}

	Vec2 Box::layout_content_size(bool& is_explicit_width, bool& is_explicit_height) {
		is_explicit_width = _explicit_width;
		is_explicit_height = _explicit_height;
		return _layout_content_size;
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
				parent()->layout_typesetting_change_notice_from_child(this);
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

	Vec2 Box::solve_transform_origin() {
		// TODO compute transform origin ...
		return Vec2(_margin_left, _margin_top);
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

	void Box::set_layout_offset_lazy(Rect rect) {
		// TODO ...
		switch(_layout_align) {
			default:
			case TOP_LEFT:
				set_layout_offset(rect.origin);
				break;
			case TOP_CENTER:
				break;
			case TOP_RIGHT:
				break;
			case CENTER_LEFT:
				break;
			case CENTER_CENTER:
				break;
			case CENTER_RIGHT:
				break;
			case BOTTOM_LEFT:
				break;
			case BOTTOM_CENTER:
				break;
			case BOTTOM_RIGHT:
		}
	}

	void Box::layout_content_size_change_notice_from_parent(Layout* parent, uint32_t mark_value) {
		mark(mark_value);
	}

}

// *******************************************************************