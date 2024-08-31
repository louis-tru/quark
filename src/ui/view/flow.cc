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

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue

namespace qk {

	float parse_align_space(ItemsAlign align,  bool is_reverse, float overflow, int count, float *space_out);

	// content wrap typesetting of horizontal or vertical
	template<bool is_horizontal>
	void Flow::layout_typesetting_wrap(bool is_reverse) { // wrap Line feed
		auto wrap_x = _wrap_x,
				 wrap_y = _wrap_y;
		Vec2 cur = content_size(), new_size;
		
		auto v = first();
		if (v) {
			bool is_wrap_main = is_horizontal ? wrap_x: wrap_y;
			bool is_wrap_cross = is_horizontal ? wrap_y: wrap_x;
			float max_main = 0;
			float total_cross = 0;
			bool wrap_reverse = _wrap == Wrap::WrapReverse;

			struct Line {
				struct Item {
					Vec2 size; View* view;
				};
				float total_main, max_cross;
				Array<Item> items;
			};
			Array<Line> lines;
			Array<typename Line::Item> _items; // temp items
			float _total_main = 0, _max_cross = 0;

			_CheckParent();

			float main_limit = is_horizontal ? (
				is_wrap_main ? get_max_width_limit_value(_parent->layout_size()): cur.x()
			): (
				is_wrap_main ? get_max_height_limit_value(_parent->layout_size()): cur.y()
			);
			do {
				if (v->visible()) {
					auto size = v->layout_size().layout;
					auto main = _total_main + (is_horizontal ? size.x(): size.y());
					if (main > main_limit) { // Line feed
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

			if (_items.length()) { // last items
				if (is_reverse)
					_items.reverse();
				lines.push({ _total_main, _max_cross, std::move(_items) });
				max_main = Qk_MAX(max_main, _total_main);
				total_cross += _max_cross;
			}
			if (wrap_reverse) {
				lines.reverse();
			}

			float main_size = is_wrap_main ? (
				is_horizontal ?
				solve_layout_content_wrap_limit_width(max_main): solve_layout_content_wrap_limit_height(max_main)
			) : (
				is_horizontal ? cur.x(): cur.y()
			);
			float cross_size = is_wrap_main ? (
				is_horizontal ?
				solve_layout_content_wrap_limit_height(total_cross): solve_layout_content_wrap_limit_width(total_cross)
			) : (
				is_horizontal ? cur.y(): cur.x()
			);
			float cross_overflow = cross_size - total_cross;
			float cross_overflow_item = 0;
			float cross_space = 0, cross_offset = 0;

			if (!is_wrap_cross) {
				if (WrapAlign::Stretch == _wrap_align) {
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
					auto s = j.size;
					auto v = j.view;
					auto align = v->layout_align();
					float cross_offset_item = cross_offset;
					switch (align == Align::Auto ? _cross_align: CrossAlign(int(align) - 1)) {
						default:
						case CrossAlign::Start: break; // 与交叉轴内的起点对齐
						case CrossAlign::Center: // 与交叉轴内的中点对齐
							cross_offset_item += ((cross - (is_horizontal ? s.y(): s.x())) * .5); break;
						case CrossAlign::End: // 与交叉轴内的终点对齐
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

			new_size = is_horizontal ? Vec2(main_size, cross_size): Vec2(cross_size, main_size);
		} else {
			new_size = Vec2{
				wrap_x ? solve_layout_content_wrap_limit_width(0): cur.x(),
				wrap_y ? solve_layout_content_wrap_limit_height(0): cur.y(),
			};
		} // end if (first())

		if (new_size != cur) {
			set_content_size(new_size);
			_IfParent()
				_parent->onChildLayoutChange(this, kChild_Layout_Size);
		}
	}

	Flow::Flow()
		: _wrap(Wrap::NoWrap)
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
				if (_wrap_x && _wrap == Wrap::NoWrap) { // no wrap, single-line
					/*
						|-------------....------------|
						|          width=WRAP         |
						|   ___ ___ ___         ___   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|                             |
						|-------------....------------|
					*/
					layout_typesetting_auto_impl(true, _direction == Direction::RowReverse);
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
					layout_typesetting_wrap<true>(_direction == Direction::RowReverse);
				}
			} else { // COLUMN
				if (_wrap_y && _wrap == Wrap::NoWrap) { // no wrap, single-line
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
					layout_typesetting_auto_impl(false, _direction == Direction::ColumnReverse);
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
					layout_typesetting_wrap<false>(_direction == Direction::ColumnReverse);
				}
			}

			unmark(kLayout_Typesetting);
		}
	}

	void Flow::onChildLayoutChange(View* child, uint32_t value) {
		Box::onChildLayoutChange(child, value);
	}
}
