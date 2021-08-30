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

		// auto horizontal layout
		void layout_typesetting_from_x(Size cur_size) {
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
			float offset_x = 0, height = 0;
			bool is_reverse = _direction == ROW_REVERSE;

			if (cur_size.wrap_y) { // wrap y
				auto v = first();
				while (v) {
					auto size = v->layout_size().layout_size;
					height = FX_MAX(height, size.y()); // solve content height
					v = v->next();
				}
			} else {
				height = cur_size.content_size.y();
			}

			auto v = is_reverse ? last(): first();
			while (v) {
				auto size = v->layout_size().layout_size;
				auto align = v->layout_align();
				float offset_y = 0;
				switch (align == Align::AUTO ? _cross_align: align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_y = (height - size.y()) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_y = height - size.y(); break;
				}
				v->set_layout_offset(Vec2(offset_x, offset_y));
				offset_x += size.x();
				v = is_reverse ? v->prev() : v->next();
			}

			Vec2 new_content_size(offset_x, height);
			if (cur_size.content_size != new_content_size) {
				set_layout_size(new_content_size);
				parent()->layout_typesetting_change(this);
			}
		}

		// wrap content horizontal
		void layout_typesetting_from_wrap_x(Size cur_size) { // wrap Line feed
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

		// flex horizontal
		void layout_typesetting_from_flex_x(Size cur_size) { // flex
			/*
				|-----------------------------|
				|  width=PIXEL,wrap=NO_WRAP   |
				|   ___ ___ ___         ___   |
				|  | L | L | L | ----> | L |  |
				|   --- --- ---         ---   |
				|                             |
				|-----------------------------|
			*/
			float total_width = 0, max_height = 0;
			bool is_reverse = _direction == ROW_REVERSE;
			Array<Vec2> size_arr;
			float weight_total = 0;

			auto v = is_reverse ? last(): first();
			while (v) {
				auto size = v->layout_raw_size(cur_size).layout_size;
				max_height = FX_MAX(max_height, size.y()); // solve content height
				total_width += size.x();
				weight_total += v->layout_weight();
				size_arr.push(size);
				v = is_reverse ? v->prev() : v->next();
			}

			float offset_x = 0;
			float space = 0;
			float content_height = cur_size.wrap_y ? max_height: cur_size.content_size.y();
			float overflow = cur_size.content_size.x() - total_width;
			bool wrap_const[2] = { false, false };

			if (weight_total) {
				total_width = 0;
				float min_weight_total = Math.min(weight_total, 1);
				auto v = is_reverse ? last(): first();
				while (v) {
					auto size = size_arr[i++];
					// 在flex中：size = size_raw + overflow * weight / weight_total * min(weight_total, 1)
					size.x( size.x() + overflow * (v->layout_weight() / weight_total) * min_weight_total );
					size = v->layout_lock(size, wrap_const);
					total_width += size.x();
					v = is_reverse ? v->prev() : v->next();
				}
				overflow = cur_size.content_size.x() - total_width;
			}

			switch (_items_align) {
				default:
				case ItemsAlign::START: // 左对齐
					if (is_reverse) {
						offset_x = overflow;
					}
					break;
				case ItemsAlign::CENTER: // 居中
					offset_x =  overflow / 2;
					break;
				case ItemsAlign::END: // 右对齐
					if (!is_reverse) {
						offset_x = overflow;
					}
					break;
				case ItemsAlign::SPACE_BETWEEN: // 两端对齐，项目之间的间隔都相等
					if (is_reverse) {
						offset_x = overflow;
					}
					if (overflow > 0) {
						space = overflow / (size_arr.length() - 1);
					}
					break;
				case ItemsAlign::SPACE_AROUND: // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
					offset_x = overflow / 2;
					if (overflow > 0) {
						// set space
					}
					break;
				case ItemsAlign::SPACE_EVENLY: // 每个项目两侧的间隔相等,这包括边框的间距
					offset_x = overflow / 2;
					if (overflow > 0) {
						// set space
					}
					break;
			}

			int i = 0;
			auto v = is_reverse ? last(): first();
			while (v) {
				auto size = size_arr[i++];
				auto align = v->layout_align();
				float offset_y = 0;
				switch (align == Align::AUTO ? _cross_align: align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_y = (content_height - size.y()) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_y = content_height - size.y(); break;
				}
				if (!weight_total)
					size = v->layout_lock(size, wrap_const);
				v->set_layout_offset(Vec2(offset_x, offset_y));
				offset_x += (size.x() + space);
				v = is_reverse ? v->prev() : v->next();
			}
		}

		// auto vertical layout
		void layout_typesetting_from_y(Size cur_size) {
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
			float width = 0, offset_y = 0;
			bool is_reverse = _direction == COLUMN_REVERSE;

			if (cur_size.wrap_y) { // wrap y
				auto v = first();
				while (v) {
					auto size = v->layout_size();
					width = FX_MAX(width, size.layout_size.x()); // solve content width
					v = v->next();
				}
			} else {
				width = cur_size.content_size.x();
			}

			auto v = is_reverse ? last(): first();
			while (v) {
				auto size = v->layout_size();
				auto align = v->layout_align();
				float offset_x = 0;
				switch (align == Align::AUTO ? _cross_align: align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_x = (width - size.layout_size.x()) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_x = width - size.layout_size.x(); break;
				}
				v->set_layout_offset(Vec2(offset_x, offset_y));
				offset_y += size.layout_size.y();
				v = is_reverse ? v->prev() : v->next();
			}

			Vec2 new_content_size(width, offset_y);
			if (cur_size.content_size != new_content_size) {
				set_layout_size(new_content_size);
				parent()->layout_typesetting_change(this);
			}
		}

		// no content wrap vertical
		void layout_typesetting_from_wrap_y(Size cur_size) { // wrap Line feed
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

		void layout_typesetting_from_flex_y(Size cur_size) {
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
			float content_width = 0, content_height = 0;
			bool is_reverse = _direction == COLUMN_REVERSE;

		}

		void update_IsLockChild(uint32_t layout_content_size_change_mark) {
			bool is_lock_child = false;

			if (parent()->is_layout_lock_child()) {
				if (_wrap == Wrap::NO_WRAP) {
					is_lock_child = true;
				}
			} else if (_direction == ROW || _direction == ROW_REVERSE) {
				if (!layout_wrap_x() && _wrap == Wrap::NO_WRAP) {
					is_lock_child = true;
				}
			} else {
				if (!layout_wrap_y() && _wrap == Wrap::NO_WRAP) {
					is_lock_child = true;
				}
			}

			if (is_lock_child != _is_lock_child) {
				_is_lock_child = is_lock_child;
				layout_content_size_change_mark = M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT;
			}

			if (!is_lock_child && layout_content_size_change_mark) {
				auto v = first();
				while (v) {
					v->layout_content_size_change(layout_content_size_change_mark);
					v = v->next();
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
		, _is_lock_child(false)
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
		auto layout_content_size_change_mark = solve_layout_size(mark);

		if (mark & (M_LAYOUT_TYPESETTING | M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT)) {
			_inl(this)->update_IsLockChild(layout_content_size_change_mark);

			if (!is_ready_layout_typesetting()) {
				return true;
			}
			if (_is_lock_child) { // flex
				if (_direction == ROW || _direction == ROW_REVERSE) {
					_inl(this)->layout_typesetting_from_flex_x(layout_size());
				} else {
					_inl(this)->layout_typesetting_from_flex_y(layout_size());
				}
			} else {
				return true; // layout_reverse() 必需在反向迭代中处理
			}

			unmark(M_LAYOUT_TYPESETTING);
		}
		return false;
	}

	bool FlexLayout::layout_reverse(uint32_t mark) {
		if(mark & M_LAYOUT_TYPESETTING) {
			if (!is_ready_layout_typesetting()) {
				return true; // continue iteration
			}
			if (_direction == ROW || _direction == ROW_REVERSE) {
				if (layout_wrap_x()) {
					_inl(this)->layout_typesetting_from_x(layout_size());
				} else if (_wrap == Wrap::NO_WRAP) { // flex
					return true; // layout_forward() 必需在正向迭代中处理
				} else { // wrap Line feed
					_inl(this)->layout_typesetting_from_wrap_x(layout_size());
				}
			} else {
				if (layout_wrap_y()) {
					_inl(this)->layout_typesetting_from_y(layout_size());
				} else if (_wrap == Wrap::NO_WRAP) { // flex
					return true; // layout_forward()
				} else { // wrap Line feed
					_inl(this)->layout_typesetting_from_wrap_y(layout_size());
				}
			}
			unmark(M_LAYOUT_TYPESETTING);
		}
		return false;
	}

	bool FlexLayout::is_layout_lock_child() {
		return _is_lock_child;
	}

	void FlexLayout::layout_typesetting_change(Layout* child, TypesettingChangeMark mark) {
		mark(M_LAYOUT_TYPESETTING);
	}

}