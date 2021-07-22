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
		#define _inl(self) static_cast<Box::Inl*>(self)

		float layout_content_width(float parent_content_size, bool *is_wrap_in_out) {
			float result;

			switch (_width.type) {
				default: // NONE /* none default wrap content */
				case WRAP: /* 包裹内容 wrap content */
					*is_wrap_in_out = true;
					result = 0; // invalid wrap width
					break;
				case PIXEL: /* 明确值 value px */
					*is_wrap_in_out = false;
					result = _width.value;
					break;
				case MATCH: /* 匹配父视图 match parent */
					if (*is_wrap_in_out) {
						result = 0; // invalid wrap width
					} else { // use wrap
						result = Number<float>::max(
							parent_content_size - _margin_left - _margin_right - _padding_left - _padding_right, 0
						);
					}
					// *is_wrap_in_out = *is_wrap_in_out;
					break;
				case RATIO: /* 百分比 value % */
					if (*is_wrap_in_out) {
						result = 0; // invalid wrap width
					} else { // use wrap
						result = Number<float>::max(parent_content_size * _width.value, 0);
					}
					// *is_wrap_in_out = *is_wrap_in_out;
					break;
				case MINUS: /* 减法(parent-value) value ! */
					if (*is_wrap_in_out) {
						result = 0; // invalid wrap width
					} else { // use wrap
						result = Number<float>::max(parent_content_size - _width.value, 0);
					}
					// *is_wrap_in_out = *is_wrap_in_out;
					break;
			}
			return result;
		}

		float layout_content_height(float parent_content_size, bool *is_wrap_in_out) {
			float result;

			switch (_height.type) {
				default: // NONE /* none default wrap content */
				case WRAP: /* 包裹内容 wrap content */
					*is_wrap_in_out = true;
					result = 0; // invalid wrap height
					break;
				case PIXEL: /* 明确值 value px */
					*is_wrap_in_out = false;
					result.height(_height.value);
					break;
				case MATCH: /* 匹配父视图 match parent */
					if (*is_wrap_in_out) {
						result = 0; // invalid wrap height
					} else { // use wrap
						result = Number<float>::max(
							parent_content_size - _margin_top - _margin_bottom - _padding_top - _padding_bottom, 0
						);
					}
					// *is_wrap_in_out = *is_wrap_in_out;
					break;
				case RATIO: /* 百分比 value % */
					if (*is_wrap_in_out) {
						result = 0; // invalid wrap height
					} else { // use wrap
						result = Number<float>::max(parent_content_size * _height.value, 0);
					}
					// *is_wrap_in_out = *is_wrap_in_out;
					break;
				case MINUS: /* 减法(parent-value) value ! */
					if (*is_wrap_in_out) {
						result = 0; // invalid wrap height
					} else { // use wrap
						result = Number<float>::max(parent_content_size - _height.value, 0);
					}
					// *is_wrap_in_out = *is_wrap_in_out;
					break;
			}
			return result;
		}
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
		, _wrap_width(true), _wrap_height(true)
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
			mark(M_LAYOUT_SIZE_WIDTH);
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

			bool is_wrap, _;
			float val = _inl(this)->layout_content_width(
				parent()->layout_content_size(&is_wrap).width(), &is_wrap
			);
			_wrap_width = is_wrap;

			if (val != _layout_content_size.width()) {
				_layout_content_size.width(val);
				// mark(M_LAYOUT_TYPESETTING);
				layout_content_size_change_mark |= M_LAYOUT_SIZE_WIDTH;
			}

			_layout_size.width(_margin_left + _margin_right + val + _padding_left + _padding_right);

			unmark(M_LAYOUT_SIZE_WIDTH);

			parent()->layout_typesetting_change(this);
		}

		if (mark & M_LAYOUT_SIZE_HEIGHT) {

			bool _, is_wrap;
			float val = _inl(this)->layout_content_height(
				parent()->layout_content_size(&_).height(), &is_wrap
			);
			_wrap_height = is_wrap;

			if (val != _layout_content_size.height()) {
				_layout_content_size.height(val);
				// mark(M_LAYOUT_TYPESETTING);
				layout_content_size_change_mark |= M_LAYOUT_SIZE_HEIGHT;
			}

			_layout_size.height(_margin_top + _margin_bottom + val + _padding_top + _padding_bottom);

			unmark(M_LAYOUT_SIZE_HEIGHT);

			parent()->layout_typesetting_change(this);
		}

		if (layout_content_size_change_mark) {
			auto v = first();
			while (v) {
				v->layout_content_size_change(this, layout_content_size_change_mark);
				v = v->next();
			}
			mark(M_LAYOUT_TYPESETTING); // rearrange
		}

		return !(layout_mark() & M_LAYOUT_TYPESETTING);
	}

	bool Box::layout_reverse(uint32_t mark) {

		if (mark & (M_LAYOUT_TYPESETTING)) {
			auto v = first();
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

	Vec2 Box::layout_content_size(bool is_wrap_out[2]) {
		is_wrap_out[0] = _wrap_width;
		is_wrap_out[1] = _wrap_height;
		return _layout_content_size;
	}

	float Box::layout_raw_size(float parent_content_size, bool *is_wrap_in_out, bool is_horizontal) {
		if (is_horizontal) {
			return _margin_left + _margin_right + _padding_left + _padding_right +
				_inl(this)->layout_content_width(parent_content_size, is_wrap_in_out);
		} else {
			return _margin_top + _margin_bottom + _padding_top + _padding_bottom +
				_inl(this)->layout_content_height(parent_content_size, is_wrap_in_out);
		}
	}

	float Box::layout_wrap_size(bool is_horizontal) {
		if (is_horizontal) {
			// TODO ...
		}
		return 0;
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
				parent()->layout_typesetting_change_from_child_weight(this);
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
		Vec2 offset;

		switch(_layout_align) {
			default:
			case LEFT_TOP:
				offset = rect.origin;
				break;
			case CENTER_TOP:
				offset = Vec2(
					rect.origin.x() + (rect.size.x() - _layout_size.x()) / 2.0,
					rect.origin.y());
				break;
			case RIGHT_TOP:
				offset = Vec2(
					rect.origin.x() + rect.size.x() - _layout_size.x(),
					rect.origin.y());
				break;
			case LEFT_CENTER:
				offset = Vec2(
					rect.origin.x(),
					rect.origin.y() + (rect.size.y() - _layout_size.y()) / 2.0);
				break;
			case CENTER_CENTER:
				offset = Vec2(
					rect.origin.x() + (rect.size.x() - _layout_size.x()) / 2.0,
					rect.origin.y() + (rect.size.y() - _layout_size.y()) / 2.0);
				break;
			case RIGHT_CENTER:
				offset = Vec2(
					rect.origin.x() + (rect.size.x() - _layout_size.x()),
					rect.origin.y() + (rect.size.y() - _layout_size.y()) / 2.0);
				break;
			case LEFT_BOTTOM:
				offset = Vec2(
					rect.origin.x(),
					rect.origin.y() + (rect.size.y() - _layout_size.y()));
				break;
			case CENTER_BOTTOM:
				offset = Vec2(
					rect.origin.x() + (rect.size.x() - _layout_size.x()) / 2.0,
					rect.origin.y() + (rect.size.y() - _layout_size.y()));
				break;
			case RIGHT_BOTTOM:
				offset = Vec2(
					rect.origin.x() + (rect.size.x() - _layout_size.x()),
					rect.origin.y() + (rect.size.y() - _layout_size.y()));
				break;
		}
		set_layout_offset(offset);
	}

	void Box::layout_content_size_change(Layout* parent, uint32_t mark_) {
		mark(mark_);
	}

}

// *******************************************************************