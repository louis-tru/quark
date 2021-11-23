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

	void View::Visitor::visitFlexLayout(FlexLayout *v) {
		visitFlowLayout(v);
	}

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void FlexLayout::accept(Visitor *visitor) {
		visitor->visitFlexLayout(this);
	}

	float __Flow_ParseAlignSpace(WrapAlign align,  bool is_reverse, float overflow, int count, float *space_out);

	F_DEFINE_INLINE_MEMBERS(FlowLayout, Inl) {
	 public:
		#define _inl_flow(self) static_cast<FlowLayout::Inl*>(static_cast<FlowLayout*>(self))
		void set_wrap(Wrap wrap);
	};

	// flex private members method
	F_DEFINE_INLINE_MEMBERS(FlexLayout, Inl) {
	 public:
		#define _inl(self) static_cast<FlexLayout::Inl*>(self)

		// auto layout horizontal or vertical
		template<bool is_horizontal>
		void layout_typesetting_from_auto(Size cur_size, bool is_reverse) {
			// get layouts raw size total
			float offset = 0, max_cross = 0;
			Vec2 cur = cur_size.content_size;

			Vec2 origin(margin_left() + padding_left(), margin_top() + padding_top());

			if (cur_size.wrap_y) { // wrap y
				auto v = first();
				while (v) {
					auto size = v->layout_size().layout_size;
					auto cross = is_horizontal ? size.y(): size.x();
					max_cross = F_MAX(max_cross, cross);
					v = v->next();
				}
			} else {
				max_cross = is_horizontal ? cur.y(): cur.x();
			}

			auto v = is_reverse ? last(): first();
			while (v) {
				auto size = v->layout_size().layout_size;
				auto align = v->layout_align();
				float offset_cross = 0;
				switch (align == Align::AUTO ? cross_align(): (CrossAlign)align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_cross = (max_cross - (is_horizontal ? size.y(): size.x())) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_cross = max_cross - (is_horizontal ? size.y(): size.x()); break;
				}
				if (is_horizontal) {
					v->set_layout_offset(Vec2(offset, offset_cross) + origin);
					offset += size.x();
				} else {
					v->set_layout_offset(Vec2(offset_cross, offset) + origin);
					offset += size.y();
				}
				v = is_reverse ? v->prev() : v->next();
			}

			Vec2 new_size = is_horizontal ? Vec2(offset, max_cross): Vec2(max_cross, offset);

			if (cur != new_size) {
				set_layout_size(new_size);
				parent()->layout_typesetting_change(this);
			}
		}

		// flex horizontal or vertical
		template<bool is_horizontal>
		void layout_typesetting_from_flex(Size cur_size, bool is_reverse) { // flex
			struct Item { Vec2 s; View* v; };
			float total_main = 0, max_cross = 0;
			Array<Item> items;
			float weight_total = 0;
			Vec2 cur = cur_size.content_size;
			float main_size = is_horizontal ? cur.x(): cur.y();
			float cross_size_old = is_horizontal ? cur.y(): cur.x();

			Vec2 origin(margin_left() + padding_left(), margin_top() + padding_top());

			auto v = is_reverse ? last(): first();
			while (v) {
				auto size = v->layout_raw_size(cur_size).layout_size;
				max_cross = F_MAX(max_cross, size.y()); // solve content height
				total_main += size.x();
				weight_total += v->layout_weight();
				items.push({size, v});
				v = is_reverse ? v->prev() : v->next();
			}

			bool is_wrap_cross = is_horizontal ? cur_size.wrap_y: cur_size.wrap_x;
			float cross_size = is_wrap_cross ? max_cross: cross_size_old;
			float overflow = main_size - total_main;
			bool wrap_const[2] = { false, false };

			if (weight_total) {
				total_main = 0;
				float min_weight_total = F_MIN(weight_total, 1);
				for (auto i: items) {
					auto size = i.s;
					auto v = i.v;
					// 在flex中：size = size_raw + overflow * weight / weight_total * min(weight_total, 1)
					if (is_horizontal) {
						size.x( size.x() + overflow * (v->layout_weight() / weight_total) * min_weight_total );
					} else {
						size.y( size.y() + overflow * (v->layout_weight() / weight_total) * min_weight_total );
					}
					size = v->layout_lock(size, wrap_const);
					total_main += (is_horizontal ? size.x(): size.y());
				}
				overflow = main_size - total_main;
			}

			float space = 0;
			float offset = __Flow_ParseAlignSpace((WrapAlign)_items_align, is_reverse, overflow, items.length(), &space);

			for (auto i: items) {
				auto size = i.s;
				auto v = i.v;
				auto align = v->layout_align();
				float offset_cross = 0;
				switch (align == Align::AUTO ? cross_align(): (CrossAlign)align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_cross = (cross_size - size.y()) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_cross = cross_size - size.y(); break;
				}
				if (!weight_total) {
					size = v->layout_lock(size, wrap_const);
				}
				if (is_horizontal) {
					v->set_layout_offset(Vec2(offset, offset_cross) + origin);
					offset += (size.x() + space);
				} else {
					v->set_layout_offset(Vec2(offset_cross, offset) + origin);
					offset += (size.y() + space);
				}
			}

			if (cross_size != cross_size_old) {
				if (is_horizontal) {
					set_layout_size(Vec2(main_size, cross_size));
				} else {
					set_layout_size(Vec2(cross_size, main_size));
				}
				parent()->layout_typesetting_change(this);
			}
		}

		// wrap content horizontal or vertical
		template<bool is_horizontal>
		void layout_typesetting_from_wrap(Size cur_size, bool is_reverse) { // wrap Line feed
			struct Line {
				struct Item {
					Vec2 s; View* v;
				};
				float total_main;
				float max_cross;
				Array<Item> items;
			};
			Vec2 cur = cur_size.content_size;
			Array<Line> lines;
			bool is_wrap_main = is_horizontal ? cur_size.wrap_x: cur_size.wrap_y;
			bool is_wrap_cross = is_horizontal ? cur_size.wrap_y: cur_size.wrap_x;
			float main_size = is_wrap_main ? 0 : (is_horizontal ? cur.x(): cur.y());
			float max_main = 0;
			float total_cross = 0;
			bool wrap_reverse = wrap() == Wrap::WRAP_REVERSE;

			Vec2 origin(margin_left() + padding_left(), margin_top() + padding_top());

			Array<typename Line::Item> _items;
			float _total_main = 0, _max_cross = 0;

			auto v = first();
			while (v) {
				auto size = v->layout_size().layout_size;
				auto main = _total_main + (is_horizontal ? size.x(): size.y());
				if (main > main_size) { // Line feed
					if (is_reverse)
						_items.reverse();
					lines.push({ _total_main, _max_cross, std::move(_items) });
					max_main = F_MAX(max_main, _total_main);
					total_cross += _max_cross;
					_total_main = is_horizontal ? size.x(): size.y();
					_max_cross = is_horizontal ? size.y(): size.x();
				} else {
					_total_main = main;
					_max_cross = F_MAX(_max_cross, size.y());
				}
				_items.push({ size, v });
				v = v->next();
			}

			if (_items.length()) {
				if (is_reverse)
					_items.reverse();
				lines.push({ _total_main, _max_cross, std::move(_items) });
				max_main = F_MAX(max_main, _total_main);
				total_cross += _max_cross;
			}

			if (wrap_reverse) {
				lines.reverse();
			}

			if (is_wrap_main) main_size = max_main;
			float cross_size = is_wrap_cross ? total_cross: (is_horizontal ? cur.y(): cur.y());
			float cross_overflow = cross_size - total_cross;
			float cross_overflow_item = 0;
			float cross_space = 0, cross_offset = 0;

			if (!is_wrap_cross) {
				if (WrapAlign::STRETCH == wrap_align()) {
					cross_overflow_item = lines.length() ? cross_overflow / lines.length() : 0;
				} else {
					cross_offset = __Flow_ParseAlignSpace(
						wrap_align(), wrap_reverse, cross_overflow, lines.length(), &cross_space);
				}
			}

			for (auto i: lines) {
				float cross = i.max_cross + cross_overflow_item;
				float overflow = main_size - i.total_main;
				float space = 0;
				float offset = __Flow_ParseAlignSpace((WrapAlign)_items_align, is_reverse, overflow, i.items.length(), &space);

				for (auto j: i.items) {
					auto s = j.s;
					auto v = j.v;
					auto align = v->layout_align();
					float cross_offset_item = cross_offset;
					switch (align == Align::AUTO ? cross_align(): (CrossAlign)align) {
						default:
						case CrossAlign::START: break; // 与交叉轴内的起点对齐
						case CrossAlign::CENTER: // 与交叉轴内的中点对齐
							cross_offset_item += ((cross - (is_horizontal ? s.y(): s.x())) / 2.0); break;
						case CrossAlign::END: // 与交叉轴内的终点对齐
							cross_offset_item += (cross - (is_horizontal ? s.y(): s.x())); break;
					}
					if (is_horizontal) {
						v->set_layout_offset(Vec2(offset, cross_offset_item) + origin);
						offset += (s.x() + space);
					} else {
						v->set_layout_offset(Vec2(cross_offset_item, offset) + origin);
						offset += (s.y() + space);
					}
				}
				cross_offset += (cross + cross_space);
			}

			Vec2 new_size = is_horizontal ? Vec2(main_size, cross_size): Vec2(cross_size, main_size);

			if (new_size != cur) {
				set_layout_size(new_size);
				parent()->layout_typesetting_change(this);
			}
		}

		// ------------------------------------------------

		void update_IsLockChild(uint32_t layout_content_size_change_mark) {
			bool is_lock_child = false;

			if (parent()->is_layout_lock_child()) {
				if (wrap() == Wrap::NO_WRAP) {
					is_lock_child = true;
				}
			} else if (direction() == Direction::ROW || direction() == Direction::ROW_REVERSE) {
				if (!layout_wrap_x() && wrap() == Wrap::NO_WRAP) {
					is_lock_child = true;
				}
			} else {
				if (!layout_wrap_y() && wrap() == Wrap::NO_WRAP) {
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
					v->layout_content_size_change(this, layout_content_size_change_mark);
					v = v->next();
				}
			}
		}

	};

	/**
		* @constructors
		*/
	FlexLayout::FlexLayout()
		: _items_align(ItemsAlign::START)
		, _is_lock_child(false)
	{
		_inl_flow(this)->set_wrap(Wrap::NO_WRAP);
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


	bool FlexLayout::layout_forward(uint32_t mark) {
		auto layout_content_size_change_mark = solve_layout_size(mark);

		if (layout_content_size_change_mark) {
			mark_recursive(M_LAYOUT_SHAPE);
		}

		if (mark & (M_LAYOUT_TYPESETTING | M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT)) {
			_inl(this)->update_IsLockChild(layout_content_size_change_mark);

			if (!is_ready_layout_typesetting()) {
				return true;
			}
			if (_is_lock_child) { // flex
				if (direction() == Direction::ROW || direction() == Direction::ROW_REVERSE) {
					/*
						|-----------------------------|
						|  width=PIXEL,wrap=NO_WRAP   |
						|   ___ ___ ___         ___   |
						|  | L | L | L | ----> | L |  |
						|   --- --- ---         ---   |
						|                             |
						|-----------------------------|
					*/
					_inl(this)->layout_typesetting_from_flex<true>(layout_size(), direction() == Direction::ROW_REVERSE); // flex horizontal
				} else {
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
					_inl(this)->layout_typesetting_from_flex<false>(layout_size(), direction() == Direction::COLUMN_REVERSE); // flex vertical
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
			if (direction() == Direction::ROW || direction() == Direction::ROW_REVERSE) {
				if (wrap() == Wrap::NO_WRAP) {
					if (layout_wrap_x()) { // auto horizontal layout
						/*
							|-------------....------------|
							|          width=WRAP         |
							|   ___ ___ ___         ___   |
							|  | L | L | L | ----> | L |  |
							|   --- --- ---         ---   |
							|                             |
							|-------------....------------|
						*/
						_inl(this)->layout_typesetting_from_auto<true>(layout_size(), direction() == Direction::ROW_REVERSE);
					} else { // flex
						return true; // layout_forward() 必需在正向迭代中处理
					}
				} else { // wrap Line feed
						/*
							|-----------------------------|
							|  width=PIXEL|WRAP,wrap=WRAP |
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
							|  | L | L | L | <---> | L |  |
							|   --- --- ---         ---   |
							|-----------------------------|
						*/
					_inl(this)->layout_typesetting_from_wrap<true>(layout_size(), direction() == Direction::ROW_REVERSE);
				}
			} else {
				if (wrap() == Wrap::NO_WRAP) {
					if (layout_wrap_y()) { // auto vertical layout
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
						_inl(this)->layout_typesetting_from_auto<false>(layout_size(), direction() == Direction::COLUMN_REVERSE);
					} else {
						return true; // layout_forward()
					}
				} else { // wrap Line feed
					/*
						|-----------------------------|
						| height=PIXEL|WRAP,wrap=WRAP |
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
					_inl(this)->layout_typesetting_from_wrap<false>(layout_size(), direction() == Direction::COLUMN_REVERSE);
				}
			}
			unmark(M_LAYOUT_TYPESETTING);
		}
		return false;
	}

	bool FlexLayout::is_layout_lock_child() {
		return _is_lock_child;
	}

	void FlexLayout::layout_typesetting_change(Layout* child, TypesettingChangeMark _mark) {
		mark(M_LAYOUT_TYPESETTING);
	}

}