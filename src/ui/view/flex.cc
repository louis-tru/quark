/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./flex.h"

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)

namespace qk {

	struct FlexItem { Vec2 size; Vec2 weight; View* view; };

	template<bool is_horizontal>
	float set_center_center_space(Array<FlexItem> &items, float overflow, float main_size, float total_main, float *space_out) {
		if (overflow > 0) {
			if (items.length() > 2) {
				float begin = is_horizontal ? items[0].size.x(): items[0].size.y();
				float end = is_horizontal ? items.back().size.x(): items.back().size.y();
				float centerSize = total_main - begin - end;
				float startOffset = (main_size - centerSize) * 0.5;

				if (begin > startOffset) { // can't
					startOffset = begin;
				} else {
					float diff = (centerSize + startOffset) - (main_size - end);
					if (diff > 0) { // can't
						startOffset -= diff;
					}
					items[0].size[is_horizontal ? 0: 1] = startOffset;
				}
				items[items.length() - 2].size[is_horizontal ? 0: 1] += main_size - startOffset - centerSize - end;
			} else {
				*space_out = overflow;
			}
		} else {
			return overflow * .5;
		}
		return 0;
	}

	float parse_align_space(ItemsAlign align, bool is_reverse, float overflow, int count, float *space_out) {
		float offset_x = 0, space = 0;

		switch (align) {
			default: break;
			case ItemsAlign::Start: // 左对齐
				if (is_reverse) {
					offset_x = overflow;
				}
				break;
			case ItemsAlign::Center: // 居中
				offset_x =  overflow * .5;
				break;
			case ItemsAlign::End: // 右对齐
				if (!is_reverse) {
					offset_x = overflow;
				}
				break;
			case ItemsAlign::SpaceBetween: // 两端对齐，项目之间的间隔都相等
				if (overflow > 0) {
					space = overflow / (count - 1);
				} else {
					if (is_reverse) {
						offset_x = overflow;
					}
				}
				break;
			case ItemsAlign::SpaceAround: // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
				if (overflow > 0) {
					space = overflow / count;
					offset_x = space * .5;
				} else {
					offset_x = overflow * .5;
				}
				break;
			case ItemsAlign::SpaceEvenly: // 每个项目两侧的间隔相等,这包括边框的间距
				if (overflow > 0) {
					space = overflow / (count + 1);
					offset_x = space;
				} else {
					offset_x = overflow * .5;
				}
				break;
		}

		*space_out = space;

		return offset_x;
	}

	// auto typesetting of horizontal or vertical
	template<bool is_horizontal>
	void Flex::layout_typesetting_auto(bool is_reverse) {
		// get views raw size total
		auto	wrap_x = _container.wrap_x,
					wrap_y = _container.wrap_y;
		Vec2	cur = _container.content;
		Array<FlexItem> items;
		float max_cross = 0, total_main = 0;

		auto is_wrap_cross = is_horizontal ? wrap_y: wrap_x;
		if (!is_wrap_cross) { // no wrap cross axis
			max_cross = is_horizontal ? cur.y(): cur.x();
		}

		auto v = is_reverse ? last(): first();
		while (v) {
			if (v->visible()) {
				auto size = v->layout_size();
				auto cross = is_horizontal ? size.y(): size.x();
				if (is_wrap_cross) // wrap cross axis
					max_cross = Qk_Max(max_cross, cross);
				total_main += is_horizontal ? size.x(): size.y();
				items.push({size, {}, v});
			}
			v = is_reverse ? v->prev() : v->next();
		}

		Vec2 new_size = is_horizontal ? Vec2{
			_container.clamp_width(total_main),
			wrap_y ?
			_container.clamp_height(max_cross): max_cross,
		}: Vec2{
			wrap_x ?
			_container.clamp_width(max_cross): max_cross,
			_container.clamp_height(total_main),
		};

		if (items.length()) {
			float offset = 0, space = 0;
			float main_size = is_horizontal ? new_size.x(): new_size.y();
			float overflow = main_size - total_main;
			if (overflow != 0) {
				if (ItemsAlign::CenterCenter == _items_align) {
					offset = set_center_center_space<is_horizontal>(items, overflow, main_size, total_main, &space);
				} else {
					offset = parse_align_space(_items_align, is_reverse, overflow, items.length(), &space);
				}
			}
			for (auto &i : items) {
				auto size = i.size;
				auto v = i.view;
				auto align = v->layout_align();
				float offset_cross = 0;
				switch (align == Align::Auto ? _cross_align: CrossAlign(int(align) - 1)) {
					default:
					case CrossAlign::Start: break; // 与交叉轴内的起点对齐
					case CrossAlign::Center: // 与交叉轴内的中点对齐
						offset_cross = (max_cross - (is_horizontal ? size.y(): size.x())) * 0.5; break;
					case CrossAlign::End: // 与交叉轴内的终点对齐
						offset_cross = max_cross - (is_horizontal ? size.y(): size.x()); break;
				}
				if (is_horizontal) {
					v->set_layout_offset({offset, offset_cross});
					offset += (size.x() + space);
				} else {
					v->set_layout_offset({offset_cross, offset});
					offset += (size.y() + space);
				}
			}
		}

		if (new_size != cur) {
			set_content_size(new_size);
			_IfParent()
				_parent->onChildLayoutChange(this, kChild_Layout_Size);
		}
	}

	// flex typesetting of horizontal or vertical
	template<bool is_horizontal>
	void Flex::layout_typesetting_flex(bool is_reverse) { // flex
		auto &cur_size = layout_container();
		Vec2 cur = cur_size.content;
		bool is_wrap_cross = is_horizontal ? cur_size.wrap_y: cur_size.wrap_x;
		float main_size = is_horizontal ? cur.x(): cur.y();
		float cross_size_old = is_horizontal ? cur.y(): cur.x();
		float cross_size = is_wrap_cross ? 0: cross_size_old;

		auto v = is_reverse ? last(): first();
		if (v) {
			Array<FlexItem> items;
			float total_main = 0, max_cross = 0;
			Vec2 total_weight;
			do {
				if (v->visible()) {
					auto size = v->layout_size();
					auto weight = v->layout_weight();
					max_cross = Qk_Max(max_cross, size.y()); // solve content height
					total_main += is_horizontal ? size.x(): size.y();
					total_weight += weight;
					items.push({size, weight, v});
				}
				v = is_reverse ? v->prev() : v->next();
			} while(v);

			if (is_wrap_cross) {
				cross_size = max_cross;
			}
			float overflow = main_size - total_main; // flex size - child total main size
			bool grow = overflow > 0; // is grow

			if ((grow && total_weight[0] > 0) || (overflow < 0 && total_weight[1] > 0)) {
				int wIdx = grow ? 0: 1;
				float min_total_weight = Qk_Min(total_weight[wIdx], 1);
				float C = (overflow * min_total_weight) / total_weight[wIdx];
				// in flex：size = size_raw + (weight / total_weight) * overflow * min(total_weight, 1)
				total_main = 0;
				for (auto &i: items) {
					auto weight = i.weight[wIdx];
					if (weight > 0) {
						auto ch = weight * C;
						auto size = is_horizontal ?
							Vec2{i.size.x()+ch, i.size.y()}: Vec2{i.size.x(), i.size.y()+ch};
						i.size = i.view->layout_lock(size); // force lock subview layout size
					}
					total_main += (is_horizontal ? i.size.x(): i.size.y());
				}
				overflow = main_size - total_main;
			}

			float offset = 0, space = 0;

			if (ItemsAlign::CenterCenter == _items_align) {
				offset = set_center_center_space<is_horizontal>(items, overflow, main_size, total_main, &space);
			} else {
				offset = parse_align_space(_items_align, is_reverse, overflow, items.length(), &space);
			}

			for (auto i: items) {
				auto size = i.size;
				auto v = i.view;
				auto align = v->layout_align();
				float offset_cross = 0;
				float cross = is_horizontal ? size.y(): size.x();
				switch (align == Align::Auto ? _cross_align: CrossAlign(int(align) - 1)) {
					default:
					case CrossAlign::Start: break; // 与交叉轴内的起点对齐
					case CrossAlign::Center: // 与交叉轴内的中点对齐
						offset_cross = (cross_size - cross) * 0.5; break;
					case CrossAlign::End: // 与交叉轴内的终点对齐
						offset_cross = cross_size - cross; break;
				}
				if (is_horizontal) {
					v->set_layout_offset({offset, offset_cross});
					offset += (size.x() + space);
				} else {
					v->set_layout_offset({offset_cross, offset});
					offset += (size.y() + space);
				}
			}
		} // end  if (first())

		if (is_wrap_cross) {
			cross_size = is_horizontal ?
				_container.clamp_height(cross_size):
				_container.clamp_width(cross_size);
		}
		if (cross_size != cross_size_old) {
			set_content_size(is_horizontal ? Vec2{main_size, cross_size}: Vec2{cross_size, main_size});
			_IfParent()
				_parent->onChildLayoutChange(this, kChild_Layout_Size);
		}
	}

	void Flex::layout_typesetting_auto_impl(bool is_horizontal, bool is_reverse) {
		if (is_horizontal) {
			layout_typesetting_auto<true>(is_reverse);
		} else {
			layout_typesetting_auto<false>(is_reverse);
		}
	}

	Flex::Flex()
		: _direction(Direction::Row)
		, _items_align(ItemsAlign::Start)
		, _cross_align(CrossAlign::Start)
	{
	}

	void Flex::set_direction(Direction val, bool isRt) {
		if (val != _direction) {
			_direction = val;
			// The layout parameters have been changed, and the sub layout needs to be rearranged in the future
			mark_layout(kLayout_Typesetting | kLayout_Child_Size, isRt);
		}
	}

	void Flex::set_items_align(ItemsAlign align, bool isRt) {
		if (align != _items_align) {
			_items_align = align;
			mark_layout(kLayout_Typesetting, isRt);
		}
	}

	void Flex::set_cross_align(CrossAlign align, bool isRt) {
		if (align != _cross_align) {
			_cross_align = align;
			mark_layout(kLayout_Typesetting, isRt);
		}
	}

	void Flex::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {

			if (_direction == Direction::Row || _direction == Direction::RowReverse) { // ROW
				if (_container.wrap_x) {
					/*
						|-------------....------------|
						|          width=WRAP         |
						|   ___ ___ ___         ___   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|                             |
						|-------------....------------|
					*/
					// auto horizontal layout
					layout_typesetting_auto<true>(_direction == Direction::RowReverse);
				} else {
					/*
						|-------------------------------|
						| width=Explicit                |
						|   ___ ___ ___          ___    |
						|  | L | L | L | -----> | L |   |
						|   --- --- ---          ---    |
						|                               |
						|-------------------------------|
					*/
					layout_typesetting_flex<true>(_direction == Direction::RowReverse); // flex horizontal
				}
			} else { // COLUMN
				if (_container.wrap_y) {
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
					// auto vertical layout
					layout_typesetting_auto<false>(_direction == Direction::ColumnReverse);
				} else {
					/*
						|--------------------------------|
						| height=Explicit                |
						|              ___               |
						|             | L |              |
						|              ---               |
						|             | L |              |
						|              ---               |
						|             | L |              |
						|              ---               |
						|               |                |
						|               |                |
						|               v                |
						|              ___               |
						|             | L |              |
						|              ---               |
						|--------------------------------|
					*/
					layout_typesetting_flex<false>(_direction == Direction::ColumnReverse); // flex vertical
				}
			}

			unmark(kLayout_Typesetting);
		}
	}

	void Flex::onChildLayoutChange(View* child, uint32_t value) {
		if (value & (kChild_Layout_Size | kChild_Layout_Visible |
								kChild_Layout_Align | kChild_Layout_Text | kChild_Layout_Weight)
		) {
			mark_layout(kLayout_Typesetting, true);
		}
	}

	ViewType Flex::viewType() const {
		return kFlex_ViewType;
	}
}
