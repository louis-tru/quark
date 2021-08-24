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
		inline bool wrap_x() const { return _wrap_x; }
		inline bool wrap_y() const { return _wrap_y; }
	};

	// flex private members method
	FX_DEFINE_INLINE_MEMBERS(FlexLayout, Inl) {
	 public:
		#define _inl(self) static_cast<FlexLayout::Inl*>(self)

		// content wrap horizontal
		void layout_typesetting_from_wrap_x() { // wrap
			/*
				|-------------....------------|
				|          width=WRAP         |
				|   ___ ___ ___         ___   |
				|  | L | L | L | ----> | L |  |
				|   --- --- ---         ---   |
				|                             |
				|-------------....------------|
			*/
			// get layouts raw size total
			float total_width = 0;
			float max_height = 0;

			if (_box(this)->wrap_y()) { // wrap y
				// Array<Vec2> size_arr;
				// while (v) {
				// 	auto size = v->layout_raw_size();
				// 	total_width += size.layout_size.x();
				// 	max_height = FX_MAX(max_height, size.layout_size.y());
				// 	size_arr.push(size.layout_size);
				// 	v = v->next();
				// }
			} else { // no wrap y
				Vec2 c_size = layout_size().content_size;
				auto v = first();
				while (v) {
					auto raw = v->layout_raw_size();
					auto align = v->layout_align();
					auto cross_align = align == Align::AUTO ? _cross_align: align;
					float offset_y = 0;
					// enum CrossAlign: uint8_t {
					// 	START = value::START, // 与交叉轴内的起点对齐
					// 	CENTER = value::CENTER, // 与交叉轴内的中点对齐
					// 	END = value::END, // 与交叉轴内的终点对齐
					// 	STRETCH = value::STRETCH, // 如果项目未设置高度或设为auto,将占满交叉轴内空间
					// };
					switch(cross_align) {
						default:
						case CrossAlign::START: break;
						case CrossAlign::CENTER: offset_y = (c_size.y() - raw.layout_size.y()) / 2.0; break;
						case CrossAlign::END: offset_y = c_size.y() - raw.layout_size.y(); break;
						case CrossAlign::STRETCH:
							break;
					}
					v->set_layout_offset(Vec2(total_width, offset_y));
					// v->layout_lock(raw.layout_size);
					total_width += raw.layout_size.x();
					v = v->next();
				}
				if (c_size.x() != total_width) {
					set_layout_size(Vec2(total_width, c_size.y()));
					parent()->layout_typesetting_change(this);
				}
			}
		}

		// no content wrap horizontal
		void layout_typesetting_from_x() {
			if (_wrap == Wrap::NO_WRAP) { // no wrap
				/*
					|-----------------------------|
					|  width=PIXEL,wrap=NO_WRAP   |
					|   ___ ___ ___         ___   |
					|  | L | L | L | ----> | L |  |
					|   --- --- ---         ---   |
					|                             |
					|-----------------------------|
				*/
			} else { // wrap Line feed
				/*
					|-----------------------------|
					|   width=PIXEL,wrap=WRAP     |
					|   ___ ___ ___         ___   |
					|  | L | L | L | ----> | L |  |
					|   --- --- ---         ---   |
					|  | L | L | L | ----> | L |  |
					|   --- --- ---         ---   |
					|  | L | L | L | ----> | L |  |
					|   --- --- ---         ---   |
					|            |                |
					|            |                |
					|            v                |
					|   ___ ___ ___         ___   |
					|  | L | L | L | <---> | L |  |
					|   --- --- ---         ---   |
					|-----------------------------|
				*/
			}
		}

		// content wrap vertical
		void layout_typesetting_from_wrap_y() { // wrap
			/*
			  |-----------|
			  |height=WRAP|
			  |    ___    |
			  |   | L |   |
			  |    ---    |
			  |   | L |   |
			  |    ---    |
			  .   | L |   .
			  .    ---    .
			  .     |     .
			  .     |     .
			  |     v     |
			  |    ___    |
			  |   | L |   |
			  |    ---    |
			  |-----------|
			*/
		}

		// no content wrap vertical
		void layout_typesetting_from_y() {
			if (_wrap == Wrap::NO_WRAP) { // no wrap
				/*
					|---------------------------|
					| height=PIXEL,wrap=NO_WRAP |
					|            ___            |
					|           | L |           |
					|            ---            |
					|           | L |           |
					|            ---            |
					|           | L |           |
					|            ---            |
					|             |             |
					|             |             |
					|             v             |
					|            ___            |
					|           | L |           |
					|            ---            |
					|---------------------------|
				*/
			} else { // wrap Line feed
				/*
					|-----------------------------|
					|  height=PIXEL,wrap=WRAP     |
					|   ___ ___ ___         ___   |
					|  | L | L | L |       | L |  |
					|   --- --- ---         ---   |
					|  | L | L | L |       | L |  |
					|   --- --- ---         ---   |
					|  | L | L | L | ----> | L |  |
					|   --- --- ---         ---   |
					|    |   |   |           |    |
					|    |   |   |           |    |
					|    v   v   v           v    |
					|   ___ ___ ___         ___   |
					|  | L | L | L |       | L |  |
					|   --- --- ---         ---   |
					|-----------------------------|
				*/
			}
		}

		void layout_typesetting() {
			switch (_direction) {
				case Direction::ROW:
				case Direction::ROW_REVERSE:
					if (_box(this)->wrap_x()) { // wrap
						layout_typesetting_from_wrap_x();
					} else {
						layout_typesetting_from_x();
					}
					break;
				default:
					if (_box(this)->wrap_y()) {
						layout_typesetting_from_wrap_y();
					} else {
						layout_typesetting_from_y();
					}
			}
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
			mark(M_LAYOUT_TYPESETTING/* | M_LAYOUT_TYPESETTING_FLEX_DIRECTION*/); // 排版参数改变,后续需对子布局重新排版
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
			mark(M_LAYOUT_TYPESETTING);
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
			mark(M_LAYOUT_TYPESETTING);
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
		solve_layout_size(mark);

		auto M_MARK =
			M_LAYOUT_TYPESETTING | 
			M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT;

		if (mark & M_MARK) {
			if (is_ready_layout_typesetting()) {
				_inl(this)->layout_typesetting();
			}
			unmark(M_MARK);
		}
		return false;
	}

	bool FlexLayout::layout_reverse(uint32_t mark) {
		return mark & M_LAYOUT_TYPESETTING; // 必须在正向迭代中处理
	}

	Vec2 FlexLayout::layout_lock(Vec2 layout_size) {
		return solve_layout_lock(layout_size, false);
	}

	void FlexLayout::layout_typesetting_change(Layout* child, TypesettingChangeMark mark) {
		mark(M_LAYOUT_TYPESETTING);
	}

	bool FlexLayout::is_child_layout_locked() {
		return true;
	}

}