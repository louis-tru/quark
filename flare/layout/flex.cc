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

	// flex private members method
	FX_DEFINE_INLINE_MEMBERS(FlexLayout, Inl) {
	 public:
		#define _inl(self) static_cast<FlexLayout::Inl*>(self)

		// content wrap horizontal
		Vec2 layout_typesetting_from_wrap_x(Size cur_size, bool isUpdate) { // wrap
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
			float content_width = 0;
			float content_height = size.wrap_y ? 0 : cur_size.content_size.y();

			if (!isUpdate) {
				auto v = first();
				while (v) {
					auto size = v->layout_raw_size(cur_size);
					if (size.wrap_y)
						content_height = FX_MAX(content_height, size.layout_size.y());
					content_width += size.layout_size.x();
					v = v->next();
				}
				return Vec2(content_width, content_height);
			}

			Array<Size> child_size;

			if (size.wrap_y) { // wrap y
				auto v = first();
				while (v) {
					auto size = v->layout_raw_size(cur_size);
					content_height = FX_MAX(content_height, size.layout_size.y()); // solve content height
					child_size.push(size);
					v = v->next();
				}
			}

			int i = 0;
			auto v = first();

			while (v) {
				auto raw_ch = child_size.length() ? child_size[i++]: v->layout_raw_size(cur_size);
				auto align = v->layout_align();
				float offset_y = 0;

				switch (align == Align::AUTO ? _cross_align: align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_y = (content_height - raw_ch.layout_size.y()) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_y = content_height - raw_ch.layout_size.y(); break;
					case CrossAlign::STRETCH: // 如果项目未设置高度或设为auto,将占满交叉轴内空间
						if (raw_ch.wrap_y) {
							raw_ch.wrap_y = false;
							raw_ch.layout_size.y(content_height);
						}
						break;
				}
				v->layout_lock(raw_ch.layout_size, &raw_ch.wrap_x);
				v->set_layout_offset(Vec2(content_width, offset_y));
				content_width += raw_ch.layout_size.x();
				v = v->next();
			}

			Vec2 new_content_size(content_width, content_height);

			if (cur_size.content_size != new_content_size) {
				set_layout_size(new_content_size);
				parent()->layout_typesetting_change(this);
			}

			return new_content_size;
		}

		// no content wrap horizontal
		Vec2 layout_typesetting_from_x(Size cur_size, bool isUpdate) {
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
			return Vec2();
		}

		// content wrap vertical
		Vec2 layout_typesetting_from_wrap_y(Size cur_size, bool isUpdate) { // wrap
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
			float content_width = cur_size.content_size.x();
			float content_height = 0;

			if (!isUpdate) {
				auto v = first();
				while (v) {
					auto size = v->layout_raw_size(cur_size);
					if (size.wrap_x)
						content_width = FX_MAX(content_width, size.layout_size.w());
					content_height += size.layout_size.y();
					v = v->next();
				}
				return Vec2(content_width, content_height);
			}

			return Vec2();
		}

		// no content wrap vertical
		Vec2 layout_typesetting_from_y(Size cur_size, bool isUpdate) {
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
			return Vec2();
		}

		void layout_typesetting(Size cur_size, bool isUpdate) {
			if (_direction == ROW || _direction == ROW_REVERSE) {
				return cur_size.wrap_x ? 
					layout_typesetting_from_wrap_x(cur_size, isUpdate):
					layout_typesetting_from_x(cur_size, isUpdate);
			} else {
				return cur_size.wrap_y ? 
					layout_typesetting_from_wrap_y(cur_size, isUpdate):
					layout_typesetting_from_y(cur_size, isUpdate);
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
				_inl(this)->layout_typesetting(layout_size(), true);
			}
			unmark(M_MARK);
		}
		return false;
	}

	bool FlexLayout::layout_reverse(uint32_t mark) {
		return mark & M_LAYOUT_TYPESETTING; // 在正向迭代中处理
	}

	Vec2 FlexLayout::layout_lock(Vec2 layout_size, bool is_wrap[2]) {
		return solve_layout_lock(layout_size, is_wrap, false);
	}

	Layout::Size layout_raw_size(Size size) {
		size.content_size.x(solve_layout_content_width(size.content_size.x(), &size.wrap_x));
		size.content_size.x(solve_layout_content_height(size.content_size.y(), &size.wrap_y));

		if (size.wrap_x || size.wrap_y) {
			size.content_size = _inl(this)->layout_typesetting(size, false);
		}
		size.layout_size.x(_margin_left + _margin_right + size.content_size.x() + _padding_left + _padding_right);
		size.layout_size.y(_margin_top + _margin_bottom + size.content_size.y() + _padding_top + _padding_bottom);
		return size;
	}

	void FlexLayout::layout_typesetting_change(Layout* child, TypesettingChangeMark mark) {
		mark(M_LAYOUT_TYPESETTING);
	}

	bool FlexLayout::is_child_layout_locked() {
		return true;
	}

}