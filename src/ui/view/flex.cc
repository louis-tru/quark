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

namespace qk {

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
					offset_x = space = overflow / (count + 1);
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
		auto	wrap_x = _layout_wrap_x_Rt,
					wrap_y = _layout_wrap_y_Rt;
		Vec2	cur = content_size();
		float offset = 0, max_cross = 0, total_main = 0;
		uint32_t items = 0;

		auto is_wrap_cross = is_horizontal ? wrap_y: wrap_x;
		if (!is_wrap_cross) { // no wrap cross axis
			max_cross = is_horizontal ? cur.y(): cur.x();
		}
		auto v = first_Rt();
		while (v) {
			if (v->visible()) {
				auto size = v->layout_size().layout_size;
				auto cross = is_horizontal ? size.y(): size.x();
				if (is_wrap_cross) // wrap cross axis
					max_cross = Qk_MAX(max_cross, cross);
				total_main += is_horizontal ? size.x(): size.y();
			}
			v = v->next_Rt();
			items++;
		}

		Vec2 new_size = is_horizontal ? Vec2{
			solve_layout_content_wrap_limit_width(total_main),
			wrap_y ? solve_layout_content_wrap_limit_height(max_cross): max_cross,
		}: Vec2{
			wrap_x ? solve_layout_content_wrap_limit_width(max_cross): max_cross,
			solve_layout_content_wrap_limit_height(total_main),
		};

		if (items) {
			float space = 0;
			float overflow = (is_horizontal ? new_size.x(): new_size.y()) - total_main;
			if (overflow != 0) {
				offset = parse_align_space(_items_align, is_reverse, overflow, items, &space);
			}
			v = is_reverse ? last_Rt(): first_Rt();
			do {
				if (v->visible()) {
					auto size = v->layout_size().layout_size;
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
						v->set_layout_offset(Vec2(offset, offset_cross));
						offset += (size.x() + space);
					} else {
						v->set_layout_offset(Vec2(offset_cross, offset));
						offset += (size.y() + space);
					}
				}
				v = is_reverse ? v->prev_Rt() : v->next_Rt();
			} while (v);
		}

		if (new_size != cur) {
			set_content_size(new_size);
			parent_Rt()->onChildLayoutChange(this, kChild_Layout_Size);
		}
	}

	// flex typesetting of horizontal or vertical
	template<bool is_horizontal>
	void Flex::layout_typesetting_flex(bool is_reverse) { // flex
		Size cur_size = layout_size();
		Vec2 cur = cur_size.content_size;
		bool is_wrap_cross = is_horizontal ? cur_size.wrap_y: cur_size.wrap_x;
		float main_size = is_horizontal ? cur.x(): cur.y();
		float cross_size_old = is_horizontal ? cur.y(): cur.x();
		float cross_size = is_wrap_cross ? 0: cross_size_old;

		if (first_Rt()) {
			struct Item { Vec2 size; View* view; };
			Array<Item> items;
			float total_main = 0, max_cross = 0;
			float total_weight = 0;

			auto v = is_reverse ? last_Rt(): first_Rt();
			do {
				if (v->visible()) {
					auto size = v->layout_raw_size(cur_size).layout_size;
					max_cross = Qk_MAX(max_cross, size.y()); // solve content height
					total_main += size.x();
					total_weight += v->layout_weight();
					items.push({size, v});
				}
				v = is_reverse ? v->prev_Rt() : v->next_Rt();
			} while(v);

			if (is_wrap_cross) {
				cross_size = max_cross;
			}
			float overflow = main_size - total_main; // flex size - child total main size

			if (total_weight > 0) {
				total_main = 0;
				float min_total_weight = Qk_MIN(total_weight, 1);
				float C = total_weight / (overflow * min_total_weight);
				// in flex：size = size_raw + overflow * (weight / total_weight) * min(total_weight, 1)
				for (auto i: items) {
					Vec2 size = i.size;
					View *v = i.view;
					if (is_horizontal) {
						size.set_x( size.x() + v->layout_weight() * C);
					} else {
						size.set_y( size.y() + v->layout_weight() * C);
					}
					size = v->layout_lock(size);
					total_main += (is_horizontal ? size.x(): size.y());
				}
				overflow = main_size - total_main;
			}

			float space = 0;
			float offset = parse_align_space(_items_align, is_reverse, overflow, items.length(), &space);

			for (auto i: items) {
				auto size = i.size;
				auto v = i.view;
				auto align = v->layout_align();
				float offset_cross = 0;
				switch (align == Align::Auto ? _cross_align: CrossAlign(int(align) - 1)) {
					default:
					case CrossAlign::Start: break; // 与交叉轴内的起点对齐
					case CrossAlign::Center: // 与交叉轴内的中点对齐
						offset_cross = (cross_size - size.y()) * 0.5; break;
					case CrossAlign::End: // 与交叉轴内的终点对齐
						offset_cross = cross_size - size.y(); break;
				}
				if (total_weight == 0) {
					size = v->layout_lock(size);
				}
				if (is_horizontal) {
					v->set_layout_offset(Vec2(offset, offset_cross));
					offset += (size.x() + space);
				} else {
					v->set_layout_offset(Vec2(offset_cross, offset));
					offset += (size.y() + space);
				}
			}
		} // end  if (first())

		if (is_wrap_cross) {
			cross_size = is_horizontal ?
			solve_layout_content_wrap_limit_height(cross_size): solve_layout_content_wrap_limit_width(cross_size);
		}
		if (cross_size != cross_size_old) {
			set_content_size(is_horizontal ? Vec2{main_size, cross_size}: Vec2{cross_size, main_size});
			parent_Rt()->onChildLayoutChange(this, kChild_Layout_Size);
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
		, _is_lock_child(false)
	{
	}

	void Flex::set_direction(Direction val, bool isRt) {
		if (val != _direction) {
			_direction = val;
			// The layout parameters have been changed, and the sub layout needs to be rearranged in the future
			mark_layout(kLayout_Typesetting, isRt);
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

	bool Flex::update_IsLockChild() {
		bool is_lock_child = false;

		if (parent_Rt()->is_lock_child_layout_size()) { // parent lock
			is_lock_child = true;
		}
		else if (_direction == Direction::Row || _direction == Direction::RowReverse) {
			if (!_layout_wrap_x_Rt) is_lock_child = true; // Explicit size x, no Line feed
		}
		else {
			if (!_layout_wrap_y_Rt) is_lock_child = true; // Explicit size y, no Line feed
		}

		if (_is_lock_child != is_lock_child) {
			_is_lock_child = is_lock_child;
			return true;
		}
		return false;
	}

	bool Flex::layout_forward(uint32_t _mark) {
		if (_mark & (kLayout_Typesetting | kLayout_Size_Width | kLayout_Size_Height)) {
			auto change_mark = solve_layout_size_forward(_mark);

			if (update_IsLockChild()) {
				change_mark = kLayout_Size_Width | kLayout_Size_Height;
			}

			if (change_mark) {
				mark_layout(kLayout_Typesetting, true); // rearrange
				mark(kRecursive_Visible_Region, true);
			}

			// if no lock child layout then must be processed in reverse iteration, layout_reverse()
			if (!_is_lock_child) { // no lock
				if (change_mark) { // if no lock child and mark value
					auto v = first_Rt();
					while (v) {
						v->onParentLayoutContentSizeChange(this, change_mark);
						v = v->next_Rt();
					}
				}
				return false;
			}

			// is ready typesetting
			if (!is_ready_layout_typesetting()) {
				// not ready continue iteration, wait call layout_reverse
				return false;
			}

			// flex lock child
			if (_direction == Direction::Row || _direction == Direction::RowReverse) { // ROW horizontal flex layout
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
			} else { // COLUMN vertical flex layout
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

			unmark(kLayout_Typesetting);

			//solve_origin_value(); // check transform_origin change
		}
		// else if (_mark & kTransform_Origin) {
			//solve_origin_value();
		// }

		return true; // complete
	}

	bool Flex::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting()) return false; // continue iteration

			if (_direction == Direction::Row || _direction == Direction::RowReverse) { // ROW
				if (!_layout_wrap_x_Rt) // no wrap, flex layout must be handled in forward iteration
					return true; // layout_forward()
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
			} else { // COLUMN
				if (_layout_wrap_y_Rt) // no wrap, flex layout must be handled in forward iteration
					return false; // layout_forward()
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
			}

			unmark(kLayout_Typesetting);

			//solve_origin_value(); // check transform_origin change
		}
		return true; // complete
	}

	Vec2 Flex::layout_lock(Vec2 layout_size) {
		bool is_wrap[2] = { false, false};
		set_layout_size(layout_size, is_wrap, true);
		return _layout_size;
	}

	bool Flex::is_lock_child_layout_size() {
		return _is_lock_child;
	}

	void Flex::onChildLayoutChange(View* child, uint32_t value) {
		if (value & (kChild_Layout_Size | kChild_Layout_Align | 
								kChild_Layout_Visible | kChild_Layout_Weight | kChild_Layout_Text)) {
			mark_layout(kLayout_Typesetting, true);
		}
	}

	ViewType Flex::viewType() const {
		return kFlex_ViewType;
	}
}
