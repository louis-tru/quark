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

#include "./flow.h"

namespace qk {
	struct FlexItem {
		Vec2 size, weight;
		View* view;
		CrossAlign align;
		bool alignBoth;
	};

	struct Line {
		float main_total, cross_max;
		Vec2  weight_total;
		Array<FlexItem> items;
	};

	float center_center_space(bool is_horizontal, Array<FlexItem> &items, float overflow,
		float main_size, float main_total, float *space_out);

	float parse_align_space(ItemsAlign align,  bool is_reverse, float overflow, int count, float *space_out);

	// content wrap typesetting of horizontal or vertical
	template<bool is_horizontal>
	void Flow::layout_typesetting_wrap(bool is_reverse) { // wrap Line feed
		Vec2 cur = _container.content;

		constexpr int mainIdx = is_horizontal ? 0: 1;
		constexpr int crossIdx = is_horizontal ? 1: 0;

		auto clamp_main = [](const Container &cont, float v) {
			return is_horizontal ? cont.clamp_width(v): cont.clamp_height(v);
		};
		auto clamp_cross = [](const Container &cont, float v) {
			return is_horizontal ? cont.clamp_height(v): cont.clamp_width(v);
		};

		auto v = first();
		if (v) {
			bool is_float_main = is_horizontal ? _container.float_x(): _container.float_y();
			bool is_float_cross = is_horizontal ? _container.float_y(): _container.float_x();
			bool is_wrap_reverse = _wrap == Wrap::WrapReverse;

			float main_max = 0;
			float cross_total = 0;
			float main_limit = is_horizontal ? _container.pre_width[1]: _container.pre_height[1];

			Array<Line> lines;
			Array<FlexItem> _items; // temp items
			float _main_total = 0, _cross_max = 0;
			Vec2  _weight_total;

			auto pushLine = [&]() {
				if (is_reverse)
					_items.reverse();
				lines.push({ _main_total, _cross_max, _weight_total, std::move(_items) });
				main_max = Qk_Max(main_max, _main_total);
				cross_total += _cross_max;
			};

			do {
				if (v->visible()) {
					auto weight = v->layout_weight();
					auto size = v->layout_container().layout_size_before_locking(v->layout_size());
					auto align = v->layout_align();
					auto main = _main_total + size[mainIdx];
					auto crossAlign = align == Align::Normal ? _cross_align: CrossAlign(align);
					if (main > main_limit) { // Line feed
						pushLine();
						_main_total = size[mainIdx];
						_cross_max = size[crossIdx];
						_weight_total = weight;
					} else {
						_main_total = main;
						_cross_max = Qk_Max(_cross_max, size[crossIdx]);
						_weight_total += weight;
					}
					_items.push({ size, weight, v, crossAlign, crossAlign == CrossAlign::Both });
				}
				v = v->next();
			} while(v);

			if (_items.length()) { // last items
				pushLine();
			}

			float main_size = is_float_main ? clamp_main(_container, main_max): cur[mainIdx];
			float cross_size = is_float_cross ? clamp_cross(_container, cross_total): cur[crossIdx];
			float cross_overflow = cross_size - cross_total;
			float cross_overflow_item = 0;
			float cross_space = 0, cross_offset = 0;

			if (_wrap_align == WrapAlign::Stretch) {
				if (cross_overflow > 0) {
					cross_overflow_item = cross_overflow / lines.length();
				} else if (is_wrap_reverse) {
					cross_offset = cross_overflow;
				}
			} else {
				cross_offset = parse_align_space(
					ItemsAlign(_wrap_align), is_wrap_reverse, cross_overflow, lines.length(), &cross_space);
			}

			if (is_wrap_reverse) {
				lines.reverse();
			}

			for (auto &line: lines) {
				float overflow = main_size - line.main_total, main_total; // flex size - child total main size
				int wIdx = overflow > 0 ? 0: 1; // is grow or shrink
				float cur_weight_total = line.weight_total[wIdx];

				do {
					float min_weight_total = Qk_Min(cur_weight_total, 1);
					float C = cur_weight_total > 0 ? (overflow * min_weight_total) / cur_weight_total: 0;
					// Flex：mainSize = size_old + (weight / weight_total) * overflow * min(weight_total, 1)
					main_total = cur_weight_total = 0;

					for (auto &it: line.items) {
						auto adjustMain = it.weight[wIdx] * C;
						if (adjustMain) {
							auto size = it.size[mainIdx] + adjustMain;
							it.size[mainIdx] = is_horizontal ? // force lock subview layout size
								it.view->layout_lock_width(size): it.view->layout_lock_height(size);
							if (it.size[mainIdx] == size) {
								cur_weight_total += it.weight[wIdx];
							} else {
								it.weight[wIdx] = 0;
							}
						}
						if (it.alignBoth) {
							it.size[crossIdx] = is_horizontal ?
								it.view->layout_lock_height(cross_size): it.view->layout_lock_width(cross_size);
							it.alignBoth = false;
						}
						main_total += it.size[mainIdx];
					}
					overflow = main_size - main_total;
				} while (overflow && cur_weight_total); //

				float offset = 0, space = 0;

				if (ItemsAlign::CenterCenter == _items_align) {
					offset = center_center_space(is_horizontal, line.items, overflow, main_size, main_total, &space);
				} else {
					offset = parse_align_space(_items_align, is_reverse, overflow, line.items.length(), &space);
				}

				float cross_item = line.cross_max + cross_overflow_item;
				for (auto it: line.items) {
					float cross_offset_item = cross_offset;
					switch (it.align) {
						default:
						case CrossAlign::Start: break; // 与交叉轴内的起点对齐
						case CrossAlign::Center: // 与交叉轴内的中点对齐
							cross_offset_item += (cross_item - it.size[crossIdx]) * 0.5; break;
						case CrossAlign::End: // 与交叉轴内的终点对齐
							cross_offset_item += cross_item - it.size[crossIdx]; break;
					}
					if (is_horizontal) {
						it.view->set_layout_offset(Vec2(offset, cross_offset_item));
					} else {
						it.view->set_layout_offset(Vec2(cross_offset_item, offset));
					}
					offset += it.size[mainIdx] + space;
				}
				cross_offset += (cross_item + cross_space);
			}

			cur[mainIdx] = main_size;
			cur[crossIdx] = cross_size;
		} else {
			if ( _container.float_x() )
				cur[0] = _container.clamp_width(0);
			if ( _container.float_y() )
				cur[1] = _container.clamp_height(0);
		}

		set_content_size(cur);
	}

	Flow::Flow()
		: _wrap(Wrap::Wrap)
		, _wrap_align(WrapAlign::Start)
	{
	}

	void Flow::set_wrap(Wrap wrap, bool isRt) {
		if (wrap != _wrap) {
			_wrap = wrap;
			mark_layout(kLayout_Typesetting, isRt);
		}
	}

	void Flow::set_wrap_align(WrapAlign align, bool isRt) {
		if (align != _wrap_align) {
			_wrap_align = align;
			mark_layout(kLayout_Typesetting, isRt);
		}
	}

	// --------------- o v e r w r i t e ---------------

	ViewType Flow::viewType() const {
		return kFlow_ViewType;
	}

	void Flow::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {

			if (_direction == Direction::Row || _direction == Direction::RowReverse) { // ROW
				if (_wrap == Wrap::NoWrap) { // no wrap, single-line
					/*
						|-------------....------------|
						|          wrap=NoWrap        |
						|   ___ ___ ___         ___   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|                             |
						|-------------....------------|
					*/
					layout_typesetting_flex<true>(_direction == Direction::RowReverse);
				} else { // wrap, multi-line
					/*
						|-------------....------------|
						|         wrap=WRAP           |
						|   ___ ___ ___         ___   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						.  | L | L | L | ----> | L |  .
						.   --- --- ---         ---   .
						.              |              .
						.              |              .
						|              v              |
						|   ___ ___ ___         ___   |
						|  | L | L | L |  ---> | L |  |
						|   --- --- ---         ---   |
						|-------------....------------|
					*/
					layout_typesetting_wrap<true>(_direction == Direction::RowReverse);
				}
			} else { // COLUMN
				if (_wrap == Wrap::NoWrap) { // no wrap, single-line
					/*
						|-----------|
						|wrap=NoWrap|
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
					layout_typesetting_flex<false>(_direction == Direction::ColumnReverse);
				} else { // wrap, multi-line
					/*
						|-------------....------------|
						|          wrap=WRAP          |
						|   ___ ___ ___         ___   |
						|  | L | L | L |       | L |  |
						|   --- --- ---         ---   |
						|  | L | L | L |       | L |  |
						|   --- --- ---         ---   |
						.  | L | L | L | ----> | L |  .
						.   --- --- ---         ---   .
						.    |   |   |           |    .
						.    |   |   |           |    .
						|    v   v   v           v    |
						|   ___ ___ ___         ___   |
						|  | L | L | L |       | L |  |
						|   --- --- ---         ---   |
						|-------------....------------|
					*/
					layout_typesetting_wrap<false>(_direction == Direction::ColumnReverse);
				}
			}

			delete_lock_state();
			unmark(kLayout_Typesetting);
		}
	}

	void Flow::onChildLayoutChange(View* child, uint32_t value) {
		Box::onChildLayoutChange(child, value);
	}
}
