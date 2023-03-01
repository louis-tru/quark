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

	float parse_align_space(ItemsAlign align,  bool is_reverse, float overflow, int count, float *space_out);

	// content wrap typesetting of horizontal or vertical
	template<bool is_horizontal>
	void FlowLayout::layout_typesetting_wrap(bool is_reverse) { // wrap Line feed

		Size cur_size = layout_size();
		Vec2 cur = cur_size.content_size;
		bool is_wrap_main = is_horizontal ? cur_size.wrap_x: cur_size.wrap_y;
		bool is_wrap_cross = is_horizontal ? cur_size.wrap_y: cur_size.wrap_x;
		float main_size = is_wrap_main ? 0 : (is_horizontal ? cur.x(): cur.y());
		float cross_size = is_wrap_cross ? 0: (is_horizontal ? cur.y(): cur.y());

		if (first()) {
			float max_main = 0;
			float total_cross = 0;
			bool wrap_reverse = _wrap == Wrap::WRAP_REVERSE;

			struct Line {
				struct Item {
					Vec2 s; View* v;
				};
				float total_main;
				float max_cross;
				Array<Item> items;
			};
			Array<Line> lines;
			Array<typename Line::Item> _items;
			float _total_main = 0, _max_cross = 0;

			auto v = first();
			do {
				if (v->visible()) {
					auto size = v->layout_size().layout_size;
					auto main = _total_main + (is_horizontal ? size.x(): size.y());
					if (main > main_size) { // Line feed
						if (is_reverse)
							_items.reverse();
						lines.push({ _total_main, _max_cross, std::move(_items) });
						max_main = Qk_MAX(max_main, _total_main);
						total_cross += _max_cross;
						_total_main = is_horizontal ? size.x(): size.y();
						_max_cross = is_horizontal ? size.y(): size.x();
					} else {
						_total_main = main;
						_max_cross = Qk_MAX(_max_cross, size.y());
					}
					_items.push({ size, v });
				}
				v = v->next();
			} while(v);

			if (_items.length()) {
				if (is_reverse)
					_items.reverse();
				lines.push({ _total_main, _max_cross, std::move(_items) });
				max_main = Qk_MAX(max_main, _total_main);
				total_cross += _max_cross;
			}

			if (wrap_reverse) {
				lines.reverse();
			}

			if (is_wrap_main)
				main_size = max_main;
			if (is_wrap_cross)
				cross_size = total_cross;
			float cross_overflow = cross_size - total_cross;
			float cross_overflow_item = 0;
			float cross_space = 0, cross_offset = 0;

			if (!is_wrap_cross) {
				if (WrapAlign::STRETCH == _wrap_align) {
					cross_overflow_item = lines.length() ? cross_overflow / lines.length() : 0;
				} else {
					cross_offset = parse_align_space(
						ItemsAlign(_wrap_align), wrap_reverse, cross_overflow, lines.length(), &cross_space);
				}
			}

			for (auto& i: lines) {
				float cross = i.max_cross + cross_overflow_item;
				float overflow = main_size - i.total_main;
				float space = 0;
				float offset = parse_align_space(_items_align, is_reverse, overflow, i.items.length(), &space);

				for (auto j: i.items) {
					auto s = j.s;
					auto v = j.v;
					auto align = v->layout_align();
					float cross_offset_item = cross_offset;
					switch (align == Align::AUTO ? _cross_align: CrossAlign(int(align) - 1)) {
						default:
						case CrossAlign::START: break; // 与交叉轴内的起点对齐
						case CrossAlign::CENTER: // 与交叉轴内的中点对齐
							cross_offset_item += ((cross - (is_horizontal ? s.y(): s.x())) * .5); break;
						case CrossAlign::END: // 与交叉轴内的终点对齐
							cross_offset_item += (cross - (is_horizontal ? s.y(): s.x())); break;
					}
					if (is_horizontal) {
						v->set_layout_offset(Vec2(offset, cross_offset_item));
						offset += (s.x() + space);
					} else {
						v->set_layout_offset(Vec2(cross_offset_item, offset));
						offset += (s.y() + space);
					}
				}
				cross_offset += (cross + cross_space);
			}
		} // end if (first())

		Vec2 new_size = is_horizontal ? Vec2(main_size, cross_size): Vec2(cross_size, main_size);

		if (new_size != cur) {
			set_content_size(new_size);
			parent()->onChildLayoutChange(this, kChild_Layout_Size);
		}
	}

	FlowLayout::FlowLayout()
		: _wrap(Wrap::NO_WRAP)
		, _wrap_align(WrapAlign::START)
	{
	}

	void FlowLayout::set_wrap(Wrap wrap) {
		if (wrap != _wrap) {
			_wrap = wrap;
			mark(kLayout_Typesetting);
		}
	}

	void FlowLayout::set_wrap_align(WrapAlign align) {
		if (align != _wrap_align) {
			_wrap_align = align;
			mark(kLayout_Typesetting);
		}
	}

	// --------------- o v e r w r i t e ---------------

	bool FlowLayout::layout_forward(uint32_t mark) {
		return Box::layout_forward(mark);
	}

	bool FlowLayout::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting()) return true; // continue iteration

			if (_direction == Direction::ROW || _direction == Direction::ROW_REVERSE) { // ROW
				if (layout_wrap_x() && _wrap == Wrap::NO_WRAP) { // no wrap, single-line
					/*
						|-------------....------------|
						|          width=WRAP         |
						|   ___ ___ ___         ___   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|                             |
						|-------------....------------|
					*/
					layout_typesetting_auto_impl(true, _direction == Direction::ROW_REVERSE);
				} else { // wrap, multi-line
					/*
						|-----------------------------|
						|  width=Explicit,wrap=WRAP   |
						|   ___ ___ ___         ___   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|              |              |
						|              |              |
						|              v              |
						|   ___ ___ ___         ___   |
						|  | L | L | L |  ---> | L |  |
						|   --- --- ---         ---   |
						|-----------------------------|
					*/
					layout_typesetting_wrap<true>(_direction == Direction::ROW_REVERSE);
				}
			} else { // COLUMN
				if (layout_wrap_y() && _wrap == Wrap::NO_WRAP) { // no wrap, single-line
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
					layout_typesetting_auto_impl(false, _direction == Direction::COLUMN_REVERSE);
				} else { // wrap, multi-line
					/*
						|-----------------------------|
						| height=Explicit,wrap=WRAP   |
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
					layout_typesetting_wrap<false>(_direction == Direction::COLUMN_REVERSE);
				}
			}

			unmark(kLayout_Typesetting);

			// check transform_origin change
			solve_origin_value();
		}
		return false;
	}

	Vec2 FlowLayout::layout_lock(Vec2 layout_size) {
		return Box::layout_lock(layout_size);
	}

	bool FlowLayout::is_lock_child_layout_size() {
		return false;
	}

	void FlowLayout::onChildLayoutChange(Layout* child, uint32_t value) {
		Box::onChildLayoutChange(child, value);
	}

// *******************************************************************
}
