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
#include "../app.h"

namespace flare {

	float Box::solve_layout_content_width(float parent_c_szie, bool *is_wrap_in_out) {
		float result;

		switch (_width.type) {
			default: // NONE /* none default wrap content */
			case SizeType::WRAP: /* 包裹内容 wrap content */
				*is_wrap_in_out = true;
				result = 0; // invalid wrap width
				break;
			case SizeType::PIXEL: /* 明确值 value px */
				*is_wrap_in_out = false;
				result = _width.value;
				break;
			case SizeType::MATCH: /* 匹配父视图 match parent */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap width
				} else { // use wrap
					result = Number<float>::max(
						parent_c_szie - _margin_left - _margin_right - _padding_left - _padding_right, 0
					);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case SizeType::RATIO: /* 百分比 value % */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap width
				} else { // use wrap
					result = Number<float>::max(parent_c_szie * _width.value, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case SizeType::MINUS: /* 减法(parent-value) value ! */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap width
				} else { // use wrap
					result = Number<float>::max(parent_c_szie - _width.value, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
		}
		return result;
	}

	float Box::solve_layout_content_height(float parent_c_szie, bool *is_wrap_in_out) {
		float result;

		switch (_height.type) {
			default: // NONE /* none default wrap content */
			case SizeType::WRAP: /* 包裹内容 wrap content */
				*is_wrap_in_out = true;
				result = 0; // invalid wrap height
				break;
			case SizeType::PIXEL: /* 明确值 value px */
				*is_wrap_in_out = false;
				result.height(_height.value);
				break;
			case SizeType::MATCH: /* 匹配父视图 match parent */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap height
				} else { // use wrap
					result = Number<float>::max(
						parent_c_szie - _margin_top - _margin_bottom - _padding_top - _padding_bottom, 0
					);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case SizeType::RATIO: /* 百分比 value % */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap height
				} else { // use wrap
					result = Number<float>::max(parent_c_szie * _height.value, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case SizeType::MINUS: /* 减法(parent-value) value ! */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap height
				} else { // use wrap
					result = Number<float>::max(parent_c_szie - _height.value, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
		}
		return result;
	}

	void Box::mark_layout_size(uint32_t mark_) {
		auto _parent = parent();
		if (_parent) {
			if (_parent->is_layout_lock_child()) {
				_parent->layout_typesetting_change(this);
			} else {
				mark(mark_);
			}
		}
	}

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
		, _layout_weight(0), _layout_align(Align::AUTO)
		, _wrap_x(true), _wrap_y(true)
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
			mark_layout_size(M_LAYOUT_SIZE_WIDTH);
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
			mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
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
			mark_layout_size(M_LAYOUT_SIZE_WIDTH);
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
			mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
		}
	}

	void Box::margin_top(float val) { // margin
		if (_margin_top != val) {
			_margin_top = val;
			mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
			mark_recursive(M_TRANSFORM_ORIGIN);
		}
	}

	void Box::margin_right(float val) {
		if (_margin_right != val) {
			_margin_right = val;
			mark_layout_size(M_LAYOUT_SIZE_WIDTH);
		}
	}

	void Box::margin_bottom(float val) {
		if (_margin_bottom != val) {
			_margin_bottom = val;
			mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
		}
	}

	void Box::margin_left(float val) {
		if (_margin_left != val) {
			_margin_left = val;
			mark_layout_size(M_LAYOUT_SIZE_WIDTH);
			mark_recursive(M_TRANSFORM_ORIGIN); // 必然影响origin变化，因为margin是transform origin的一部分
		}
	}

	void Box::padding_top(float val) { // padding
		if (_padding_top != val) {
			_padding_top = val;
			mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
			mark_recursive(M_TRANSFORM/* | M_TRANSFORM_ORIGIN*/); // 影响子布局偏移、影响尺寸变化进而影响origin
		}
	}

	void Box::padding_right(float val) {
		if (_padding_right != val) {
			_padding_right = val;
			mark_layout_size(M_LAYOUT_SIZE_WIDTH);
			// mark_recursive(M_TRANSFORM_ORIGIN);
		}
	}

	void Box::padding_bottom(float val) {
		if (_padding_bottom != val) {
			_padding_bottom = val;
			mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
			// mark_recursive(M_TRANSFORM_ORIGIN);
		}
	}

	void Box::padding_left(float val) {
		if (_padding_left != val) {
			_padding_left = val;
			mark_layout_size(M_LAYOUT_SIZE_WIDTH);
			mark_recursive(M_TRANSFORM); // 几乎可以肯定会影响子布局偏移
		}
	}
	
	void Box::fill(Fill val) {
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

	/**
		* @func solve_layout_size()
		*/
	uint32_t Box::solve_layout_size(uint32_t mark) {
		uint32_t layout_content_size_change_mark = M_NONE;

		if (mark & M_LAYOUT_SIZE_WIDTH) {
			if (!parent()->is_layout_lock_child()) {
				auto size = parent()->layout_size();
				auto val = solve_layout_content_width(size.content_size.x(), &size.wrap_x);

				if (val != _layout_content_size.width() || _wrap_x != size.wrap_x) {
					_layout_content_size.width(val);
					_wrap_x = size.wrap_x;
					// mark(M_LAYOUT_TYPESETTING);
					layout_content_size_change_mark = M_LAYOUT_SIZE_WIDTH;
				}
				_layout_size.x(_margin_left + _margin_right + val + _padding_left + _padding_right);
			} // else The layout is locked and does not need to be updated
			parent()->layout_typesetting_change(this);
			unmark(M_LAYOUT_SIZE_WIDTH);
		}

		if (mark & M_LAYOUT_SIZE_HEIGHT) {
			if (!parent()->is_layout_lock_child()) {
				auto size = parent()->layout_size();
				auto val = solve_layout_content_height(size.content_size.y(), &size.wrap_y);

				if (val != _layout_content_size.height() || _wrap_y != size.wrap_y) {
					_layout_content_size.height(val);
					_wrap_y = size.wrap_y;
					layout_content_size_change_mark |= M_LAYOUT_SIZE_HEIGHT;
				}
				_layout_size.y(_margin_top + _margin_bottom + val + _padding_top + _padding_bottom);
			} // else The layout is locked and does not need to be updated
			parent()->layout_typesetting_change(this);
			unmark(M_LAYOUT_SIZE_HEIGHT);
		}

		return layout_content_size_change_mark;
	}

	bool Box::layout_forward(uint32_t mark) {
		uint32_t layout_content_size_change_mark = solve_layout_size(mark);

		if (layout_content_size_change_mark) {
			auto v = first();
			while (v) {
				v->layout_content_size_change(this, layout_content_size_change_mark);
				v = v->next();
			}
			mark(M_LAYOUT_TYPESETTING); // rearrange
			mark_recursive(M_LAYOUT_SHAPE);
		}

		return (layout_mark() & M_LAYOUT_TYPESETTING);
	}

	bool Box::layout_reverse(uint32_t mark) {
		if (mark & (M_LAYOUT_TYPESETTING)) {
			if (!is_ready_layout_typesetting()) {
				return true; // continue iteration
			}
			auto v = first();
			Vec2 origin(_margin_left + _padding_left, _margin_top + _padding_top);
			Vec2 size = _layout_content_size;
			while (v) {
				v->set_layout_offset_lazy(origin, size); // lazy layout
				v = v->next();
			}
			unmark(M_LAYOUT_TYPESETTING);
		}
		return false; // stop iteration
	}

	Vec2 Box::layout_offset() {
		return _layout_offset;
	}

	Layout::Size Box::layout_size() {
		return {
			_layout_size, _layout_content_size, _wrap_x, _wrap_y
		};
	}

	Layout::Size Box::layout_raw_size(Size size) {
		size.content_size.x(solve_layout_content_width(size.content_size.x(), &size.wrap_x));
		size.content_size.x(solve_layout_content_height(size.content_size.y(), &size.wrap_y));
		size.layout_size.x(_margin_left + _margin_right + size.content_size.x() + _padding_left + _padding_right);
		size.layout_size.y(_margin_top + _margin_bottom + size.content_size.y() + _padding_top + _padding_bottom);
		return size;
	}

	float Box::layout_weight() {
		return _layout_weight;
	}

	Align Box::layout_align() {
		return _layout_align;
	}

	/**
		*
		* 设置布局对齐方式
		*
		* @func set_layout_align(align)
		*/
	void Box::set_layout_align(Align align) {
		if (_layout_align != align) {
			_layout_align = align;
			if (parent()) {
				parent()->layout_typesetting_change(this);
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
				parent()->layout_typesetting_change(this, T_CHILD_WEIGHT);
			}
		}
	}

	Vec2 Box::solve_transform_origin() {
		// TODO compute transform origin ...
		return Vec2(_margin_left, _margin_top);
	}

	Vec2 Box::layout_lock(Vec2 layout_size, bool is_wrap[2]) {
		auto layout_content_size_change_mark = M_NONE;
		auto layout_content_size = _layout_content_size;

		auto mp_x = _margin_left + _margin_right + _padding_left + _padding_right;
		auto mp_y = _margin_top + _margin_bottom + _padding_top + _padding_bottom;

		_layout_content_size = Vec2(
			layout_size.x() > mp_x ? layout_size.x() - mp_x: 0,
			layout_size.y() > mp_y ? layout_size.y() - mp_y: 0
		);
		_layout_size = Vec2(mp_x + _layout_content_size.x(), mp_y + _layout_content_size.y());

		if (layout_content_size.x() != _layout_content_size.x() || _wrap_x != is_wrap[0]) {
			layout_content_size_change_mark = M_LAYOUT_SIZE_WIDTH;
		}
		if (layout_content_size.y() != _layout_content_size.y() || _wrap_y != is_wrap[1]) {
			layout_content_size_change_mark |= M_LAYOUT_SIZE_HEIGHT;
		}

		_wrap_x = is_wrap[0];
		_wrap_y = is_wrap[1];

		if (layout_content_size_change_mark)  {
			if (!is_layout_lock_child()) {
				auto v = first();
				while (v) {
					v->layout_content_size_change(this, layout_content_size_change_mark);
					v = v->next();
				}
			}
			mark(M_LAYOUT_TYPESETTING); // rearrange
			mark_recursive(M_LAYOUT_SHAPE);
		}

		unmark(M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT);

		return _layout_size;
	}

	/**
		* @func set_layout_size(layout_content_size)
		*/
	void Box::set_layout_size(Vec2 layout_content_size) {
		_layout_content_size = layout_content_size;
		_layout_size = Vec2(
			_margin_left + _margin_right + layout_content_size.x() + _padding_left + _padding_right,
			_margin_top + _margin_bottom + layout_content_size.y() + _padding_top + _padding_bottom,
		);
	}

	void Box::set_layout_offset(Vec2 val) {
		if (val != _layout_offset) {
			_layout_offset = val;
			mark_recursive(M_TRANSFORM); // mark recursive transform
		}
	}

	void Box::set_layout_offset_lazy(Vec2 origin, Vec2 size) {
		Vec2 offset;
		auto _layout_size = layout_size();

		switch(_layout_align) {
			default:
			case LEFT_TOP:
				offset = origin;
				break;
			case CENTER_TOP:
				offset = Vec2(
					origin.x() + (size.x() - _layout_size.x()) / 2.0,
					origin.y());
				break;
			case RIGHT_TOP:
				offset = Vec2(
					origin.x() + size.x() - _layout_size.x(),
					origin.y());
				break;
			case LEFT_CENTER:
				offset = Vec2(
					origin.x(),
					origin.y() + (size.y() - _layout_size.y()) / 2.0);
				break;
			case CENTER_CENTER:
				offset = Vec2(
					origin.x() + (size.x() - _layout_size.x()) / 2.0,
					origin.y() + (size.y() - _layout_size.y()) / 2.0);
				break;
			case RIGHT_CENTER:
				offset = Vec2(
					origin.x() + (size.x() - _layout_size.x()),
					origin.y() + (size.y() - _layout_size.y()) / 2.0);
				break;
			case LEFT_BOTTOM:
				offset = Vec2(
					origin.x(),
					origin.y() + (size.y() - _layout_size.y()));
				break;
			case CENTER_BOTTOM:
				offset = Vec2(
					origin.x() + (size.x() - _layout_size.x()) / 2.0,
					origin.y() + (size.y() - _layout_size.y()));
				break;
			case RIGHT_BOTTOM:
				offset = Vec2(
					origin.x() + (size.x() - _layout_size.x()),
					origin.y() + (size.y() - _layout_size.y()));
				break;
		}
		set_layout_offset(offset);
	}

	void Box::layout_content_size_change(Layout* parent, uint32_t mark_) {
		mark(mark_);
	}

	/**
		* 
		* is ready layout layout typesetting in the `layout_reverse() or layout_forward()` func
		*
		* @func is_ready_layout_typesetting()
		*/
	bool Box::is_ready_layout_typesetting() {
		if (parent()->is_layout_lock_child()) { // layout size locked by parent layout
			if (parent()->layout_mark() & M_LAYOUT_TYPESETTING) {
				// The parent layout needs to be readjusted
				return false;
			}
		}
		return true;
	}

	/**
		* @func solve_rect_vertex(vertex)
		*/
	void Box::solve_rect_vertex(Vec2 vertex[4]) {
		auto final_matrix = matrix();
		auto origin = transform_origin();
		Vec2 start(_margin_left - origin.x(), _margin_top - origin.y());
		Vec2 end(
			_layout_content_size.x() + _padding_left + _padding_right + start.x(),
			_layout_content_size.y() + _padding_top + _padding_bottom + start.y()
		);
		vertex[0] = final_matrix * start;
		vertex[1] = final_matrix * Vec2(end.x(), start.y());
		vertex[2] = final_matrix * end;
		vertex[3] = final_matrix * Vec2(start.x(), end.y());
	}

	bool Box::solve_region_visible() {
		bool visible = false;

		Vec2 vertex[4];

		solve_rect_vertex(vertex);

		/*
		* 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
		* 这种模糊测试在大多数时候都是正确有效的。
		*/
		Region dre = app()->display_port()->draw_region();
		Region re = screen_region_from_convex_quadrilateral(vertex);
		
		if (FX_MAX( dre.y2, re.y2 ) - FX_MIN( dre.y, re.y ) <= re.h + dre.h &&
				FX_MAX( dre.x2, re.x2 ) - FX_MIN( dre.x, re.x ) <= re.w + dre.w
		) {
			visible = true;
		}

		return visible;
	}

}

// *******************************************************************