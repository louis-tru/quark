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

#include "./flex.h"

namespace flare {

	// box private members method
	FX_DEFINE_INLINE_MEMBERS(Box, Inl_FlexLayout) {
		public:
		#define _box(self) static_cast<Box::Inl_FlexLayout*>(self)

		inline bool content_wrap_horizontal() const { return _content_wrap_horizontal; }
		inline bool content_wrap_vertical() const { return _content_wrap_vertical; }
	};

	// flex private members method
	FX_DEFINE_INLINE_MEMBERS(FlexLayout, Inl) {
		public:
		#define _inl(self) static_cast<FlexLayout::Inl*>(self)

		void layout_typesetting_horizontal_wrap() {
			// wrap
		}

		void layout_typesetting_horizontal() {
			// no wrap
		}

		void layout_typesetting_vertical_wrap() {

		}

		void layout_typesetting_vertical() {

		}
		
	};

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void FlexLayout::accept(Visitor *visitor) {
		visitor->visitFlexLayout(this);
	}

	/**
		* @constructors
		*/
	FlexLayout::FlexLayout()
		: _direction(Direction::ROW)
		, _items_align(ItemsAlign::START)
		, _cross_align(CrossAlign::START)
		, _wrap(Wrap::NO_WRAP)
		, _wrap_align(WrapAlign::START)
	{
	}

	/**
		*
		* 设置主轴的方向
		*
		* @func set_direction(val)
		*/
	void FlexLayout::set_direction(Direction val) {
		if (val != _direction) {
			_direction = val;
			mark(M_LAYOUT_TYPESETTING); // 排版参数改变,后续需对子布局重新排版
		}
	}

	/**
		* 
		* 设置主轴的对齐方式
		*
		* @func stt_items_align(align)
		*/
	void FlexLayout::set_items_align(ItemsAlign align) {
		if (align != _items_align) {
			_items_align = align;
			mark(M_LAYOUT_TYPESETTING);
		}
	}

	/**
		* 
		* 设置交叉轴的对齐方式
		*
		* @func set_cross_align(align)
		*/
	void FlexLayout::set_cross_align(CrossAlign align) {
		if (align != _cross_align) {
			_cross_align = align;
			mark(M_LAYOUT_TYPESETTING | M_LAYOUT_TYPESETTING_FLEX_DIRECTION);
		}
	}

	/**
		* 
		* 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
		*
		* @func set_wrap_reverse(reverse)
		*/
	void FlowLayout::set_wrap(Wrap wrap) {
		if (wrap != _wrap) {
			_wrap = wrap;
			mark(M_LAYOUT_TYPESETTING | M_LAYOUT_TYPESETTING_FLEX_WRAP);
		}
	}

	/**
		* 
		* 设置多根交叉轴的对齐方式
		*
		* @func set_wrap_align(align)
		*/
	void FlowLayout::set_wrap_align(WrapAlign align) {
		if (align != _wrap_align) {
			_wrap_align = align;
			mark(M_LAYOUT_TYPESETTING);
		}
	}

	bool FlexLayout::layout_forward(uint32_t mark) {
		auto r = Box::layout_forward(mark);

		if (mark & (M_LAYOUT_TYPESETTING_FLEX_WRAP /*| M_LAYOUT_TYPESETTING_FLEX_DIRECTION*/)) {
			if (_wrap != NO_WRAP) { // WRAP | WRAP_REVERSE
				auto v = first();
				while (v) {
					v->lock_layout_size(); // unlock
					v = v->next();
				}
			}
		}

		if (!r) {
			if (_wrap == NO_WRAP) { // no wrap
				// lock
				// return true;
			}
		}

		return r;
	}

	bool FlexLayout::layout_reverse(uint32_t mark) {
		if (mark & M_LAYOUT_TYPESETTING) {
			switch (_direction) {
				case Direction::ROW:
				case Direction::ROW_REVERSE:
					if (_box(this)->content_wrap_horizontal()) {
						_inl(this)->layout_typesetting_horizontal_wrap();
					} else {
						_inl(this)->layout_typesetting_horizontal();
					}
					break;
				default:
					if (_box(this)->content_wrap_vertical()) {
						_inl(this)->layout_typesetting_vertical_wrap();
					} else {
						_inl(this)->layout_typesetting_vertical();
					}
			}
			unmark(M_LAYOUT_TYPESETTING);
		}
		return true;
	}

	void FlexLayout::layout_typesetting_change(Layout* child, TypesettingChangeMark mark) {
		mark(M_LAYOUT_TYPESETTING);
	}

}