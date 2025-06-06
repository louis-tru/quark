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
	float set_center_center_space(Array<FlexItem> &items, float overflow, float main_size, float main_total, float *space_out) {
		if (overflow > 0) {
			if (items.length() > 2) {
				float begin = is_horizontal ? items[0].size.x(): items[0].size.y();
				float end = is_horizontal ? items.back().size.x(): items.back().size.y();
				float centerSize = main_total - begin - end;
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
		auto	float_x = _container.float_x,
					float_y = _container.float_y;
		Vec2	cur = _container.content;
		auto is_float_cross = is_horizontal ? float_y: float_x;
		float cross_size_old = is_horizontal ? cur.y(): cur.x();
		float cross_size = is_float_cross ? 0: cross_size_old;

		Array<FlexItem> items;
		float total_main = 0;

		auto v = is_reverse ? last(): first();
		while (v) {
			if (v->visible()) {
				auto size = v->layout_size();
				if (is_float_cross)
					cross_size = Float32::max(cross_size, is_horizontal ? size.y(): size.x());
				total_main += is_horizontal ? size.x(): size.y();
				items.push({size, {}, v});
			}
			v = is_reverse ? v->prev() : v->next();
		}

		Vec2 new_size = is_horizontal ? Vec2{
			_container.clamp_width(total_main),
			float_y ?
			_container.clamp_height(cross_size): cross_size,
		}: Vec2{
			float_x ?
			_container.clamp_width(cross_size): cross_size,
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
						offset_cross = (cross_size - (is_horizontal ? size.y(): size.x())) * 0.5; break;
					case CrossAlign::End: // 与交叉轴内的终点对齐
						offset_cross = cross_size - (is_horizontal ? size.y(): size.x()); break;
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
		Vec2 cur = _container.content;

		auto v = is_reverse ? last(): first();
		if (v) {
			auto clamp_main = [](const Container &cont, float v) {
				return is_horizontal ? cont.clamp_width(v): cont.clamp_height(v);
			};
			auto clamp_cross = [](const Container &cont, float v) {
				return is_horizontal ? cont.clamp_height(v): cont.clamp_width(v);
			};
			auto get_main = [](Vec2 v){ return is_horizontal ? v.x(): v.y(); };
			auto get_cross = [](Vec2 v){ return is_horizontal ? v.y(): v.x(); };

			auto is_float_main = is_horizontal ? _container.float_x: _container.float_y;
			bool is_float_cross = is_horizontal ? _container.float_y: _container.float_x;

			float main_size = get_main(cur);
			float cross_size = 0;

			Array<FlexItem> items;
			float main_total = 0;
			Vec2  weight_total;
			do {
				if (v->visible()) {
					auto weight = v->layout_weight();
					auto size = v->layout_size();
					if (is_float_cross)
						cross_size = Float32::max(cross_size, get_cross(size));
					main_total += get_main(size);
					weight_total += weight;
					items.push({size, weight, v});
				}
				v = is_reverse ? v->prev() : v->next();
			} while(v);

			if (is_float_cross) {
				cross_size = clamp_cross(_container, cross_size);
			} else { // no wrap
				cross_size = get_cross(cur);
			}

			if (is_float_main) {
				main_size = clamp_main(_container, main_total);
			}
			float overflow = main_size - main_total; // flex size - child total main size
			bool grow = overflow > 0; // is grow

			if ((grow && weight_total[0] > 0) || (overflow < 0 && weight_total[1] > 0)) {
				int wIdx = grow ? 0: 1;
				float min_weight_total = Qk_Min(weight_total[wIdx], 1);
				float C = (overflow * min_weight_total) / weight_total[wIdx];
				// in flex：size = size_raw + (weight / weight_total) * overflow * min(weight_total, 1)
				main_total = 0;
				for (auto &i: items) {
					auto weight = i.weight[wIdx];
					if (weight > 0) {
						auto ch = weight * C;
						auto size = is_horizontal ?
							Vec2{i.size.x()+ch, i.size.y()}: Vec2{i.size.x(), i.size.y()+ch};
						i.size = i.view->layout_lock(size); // force lock subview layout size
					}
					main_total += get_main(i.size);
				}
				if (is_float_main) {
					main_size = clamp_main(_container, main_total);
				}
				overflow = main_size - main_total;
			}

			float offset = 0, space = 0;

			if (ItemsAlign::CenterCenter == _items_align) {
				offset = set_center_center_space<is_horizontal>(items, overflow, main_size, main_total, &space);
			} else {
				offset = parse_align_space(_items_align, is_reverse, overflow, items.length(), &space);
			}

			for (auto i: items) {
				auto size = i.size;
				auto v = i.view;
				auto align = v->layout_align();
				float offset_cross = 0;
				float cross = get_cross(size);
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
		} else {
			if ( _container.float_x )
				cur[0] = _container.clamp_width(0);
			if ( _container.float_y )
				cur[1] = _container.clamp_height(0);
		}

		if (cur != _container.content) {
			set_content_size(cur);
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
				/*
					|-------------....------------|
					|          width=FLOAT         |
					|   ___ ___ ___         ___   |
					|  | L | L | L | ----> | L |  |
					|   --- --- ---         ---   |
					|                             |
					|-------------....------------|
				*/

				// or

				/*
					|-------------------------------|
					| width=FIXED                   |
					|   ___ ___ ___          ___    |
					|  | L | L | L | -----> | L |   |
					|   --- --- ---          ---    |
					|                               |
					|-------------------------------|
				*/
				layout_typesetting_flex<true>(_direction == Direction::RowReverse); // flex horizontal
			} else { // COLUMN
				/*
					|------------|
					|height=FLOAT|
					|    ___     |
					|   | L |    |
					|    ---     |
					|   | L |    |
					|    ---     |
					.   | L |    .
					.    ---     .
					.     |      .
					.     |      .
					|     v      |
					|    ___     |
					|   | L |    |
					|    ---     |
					|----------- |
				*/

				// or

				/*
					|--------------------------------|
					| height=FIXED                   |
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
