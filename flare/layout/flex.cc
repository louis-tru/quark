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

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void Flex::accept(Visitor *visitor) {
		visitor->visitFlex(this);
	}

	// flex private members method
	FX_DEFINE_INLINE_MEMBERS(Flex, Inl) {
	 public:
		#define _inl(self) static_cast<Flex::Inl*>(self)

		float parseAlign(ItemsAlign align,  bool is_reverse, float overflow, int count, float *space_out) {
			float offset_x = 0, space = 0;

			switch (align) {
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
					if (overflow > 0) {
						space = overflow / (count - 1);
					} else {
						if (is_reverse) {
							offset_x = overflow;
						}
					}
					break;
				case ItemsAlign::SPACE_AROUND: // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
					if (overflow > 0) {
						space = overflow / count;
						offset_x = space / 2;
					} else {
						offset_x = overflow / 2;
					}
					break;
				case ItemsAlign::SPACE_EVENLY: // 每个项目两侧的间隔相等,这包括边框的间距
					if (overflow > 0) {
						offset_x = space = overflow / (count + 1);
					} else {
						offset_x = overflow / 2;
					}
					break;
			}

			*space_out = space;

			return offset_x;
		}

		// auto layout horizontal or vertical
		template<bool is_horizontal>
		void layout_typesetting_from_auto(Size cur_size) {
			// get layouts raw size total
			float offset = 0, max_cross = 0;
			bool is_reverse = _direction == ROW_REVERSE;
			Vec2 cur = cur_size.content_size;

			if (cur_size.wrap_y) { // wrap y
				auto v = first();
				while (v) {
					auto size = v->layout_size().layout_size;
					auto cross = is_horizontal ? size.y(): size.x();
					max_cross = FX_MAX(max_cross, cross);
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
				switch (align == Align::AUTO ? _cross_align: align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_cross = (max_cross - (is_horizontal ? size.y(): size.x())) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_cross = max_cross - (is_horizontal ? size.y(): size.x()); break;
				}
				if (is_horizontal) {
					v->set_layout_offset(Vec2(offset, offset_cross));
					offset += size.x();
				} else {
					v->set_layout_offset(Vec2(offset_cross, offset));
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

		// wrap content horizontal or vertical
		template<bool is_horizontal>
		void layout_typesetting_from_wrap(Size cur_size) { // wrap Line feed
			struct Line {
				struct Item {
					Vec2: s; View* v;
				};
				float total_main;
				float max_cross;
				Array<Item> items;
			};
			Vec2 cur = cur_size.content_size;
			Array<Line> line;
			float main_size = is_horizontal ? cur.x(): cur.y();
			float total_cross = 0;
			bool is_reverse = _direction == ROW_REVERSE;
			bool wrap_reverse = _direction == Wrap::WRAP_REVERSE;

			Array<Item> tmp_items
			float tmp_total_main = 0, tmp_max_cross = 0;

			auto v = first();
			while (v) {
				auto size = v->layout_size().layout_size;
				auto main = tmp_total_main + (is_horizontal ? size.x(): size.y());
				if (main > main_size) { // Line feed
					if (is_reverse)
						tmp_items.reverse();
					line.push({ tmp_total_main, tmp_max_cross, std::move(tmp_items) });
					total_cross += tmp_max_cross;
					tmp_total_main = is_horizontal ? size.x(): size.y();
					tmp_max_cross = is_horizontal ? size.y(): size.x();
				} else {
					tmp_total_main = main;
					tmp_max_cross = FX_MAX(tmp_max_cross, size.y());
				}
				tmp_items.push({ size, v });
				v = v->next();
			}

			if (tmp_items.length()) {
				if (is_reverse)
					tmp_items.reverse();
				line.push({ tmp_total_main, tmp_max_cross, std::move(tmp_items) });
				total_cross += tmp_max_cross;
			}

			if (wrap_reverse) {
				line.reverse();
			}

			bool is_wrap_cross = is_horizontal ? cur_size.wrap_y: cur_size.wrap_x;
			float cross_size_old = is_horizontal ? cur.y(): cur.y();
			float cross_size = is_wrap_cross ? total_cross: cross_size_old;
			float cross_overflow = cross_size - total_cross;
			float cross_overflow_item = 0;
			float cross_space = 0, cross_offset = 0;

			if (!is_wrap_cross) {
				if (STRETCH == _wrap_align) {
					cross_overflow_item = line.length() ? cross_overflow / line.length() : 0;
				} else {
					cross_offset = parseAlign((ItemsAlign)
						_wrap_align, wrap_reverse, cross_overflow, line.length(), &cross_space);
				}
			}

			for (auto i: line) {
				float cross = i.max_cross + cross_overflow_item;
				float overflow = main_size - i.total_main;
				float space = 0;
				float offset = parseAlign(_items_align, is_reverse, overflow, i.items.length(), &space);

				for (auto j: i.items) {
					auto s = i.s;
					auto v = i.v;
					auto align = v->layout_align();
					float cross_offset_item = cross_offset;
					switch (align == Align::AUTO ? _cross_align: align) {
						default:
						case CrossAlign::START: break; // 与交叉轴内的起点对齐
						case CrossAlign::CENTER: // 与交叉轴内的中点对齐
							cross_offset_item += ((cross - (is_horizontal ? s.y(): s.x())) / 2.0); break;
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

			if (cross_size != cross_size_old) {
				if (is_horizontal) {
					set_layout_size(Vec(main_size, cross_size));
				} else {
					set_layout_size(Vec(cross_size, main_size));
				}
				parent()->layout_typesetting_change(this);
			}
		}

		// flex horizontal or vertical
		template<bool is_horizontal>
		void layout_typesetting_from_flex(Size cur_size) { // flex
			struct Item { Vec2: s; View* v; };
			float total_main = 0, max_cross = 0;
			bool is_reverse = _direction == ROW_REVERSE;
			Array<Item> items;
			float weight_total = 0;
			Vec2 cur = cur_size.content_size;
			float main_size = is_horizontal ? cur.x(): cur.y();
			float cross_size_old = is_horizontal ? cur.y(): cur.x();

			auto v = is_reverse ? last(): first();
			while (v) {
				auto size = v->layout_raw_size(cur_size).layout_size;
				max_cross = FX_MAX(max_cross, size.y()); // solve content height
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
				float min_weight_total = Math.min(weight_total, 1);
				for (auto i: arr) {
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
			float offset = parseAlign(_items_align, is_reverse, overflow, items.length(), &space);

			for (auto i: arr) {
				auto size = i.s;
				auto v = i.v;
				auto align = v->layout_align();
				float offset_cross = 0;
				switch (align == Align::AUTO ? _cross_align: align) {
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
					v->set_layout_offset(Vec2(offset, offset_cross));
					offset += (size.x(): size.y() + space);
				} else {
					v->set_layout_offset(Vec2(offset_cross, offset));
					offset += (size.y() + space);
				}
			}

			if (cross_size != cross_size_old) {
				if (is_horizontal) {
					set_layout_size(Vec(main_size, cross_size));
				} else {
					set_layout_size(Vec(cross_size, main_size));
				}
				parent()->layout_typesetting_change(this);
			}
		}

		// ------------------------------------------------

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
			layout_typesetting_from_auto<true>(cur_size);
		}

		// wrap content horizontal
		void layout_typesetting_from_wrap(Size cur_size) {
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
				|              |              |
				|              |              |
				|              v              |
				|   ___ ___ ___         ___   |
				|  | L | L | L | <---> | L |  |
				|   --- --- ---         ---   |
				|-----------------------------|
			*/
			layout_typesetting_from_wrap<true>(cur_size);
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
			layout_typesetting_from_flex<true>(cur_size);
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
			layout_typesetting_from_auto<false>(cur_size);
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
			layout_typesetting_from_wrap<false>(cur_size);
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
			layout_typesetting_from_flex<false>(cur_size);
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
		* @constructors
		*/
	Flex::Flex()
		: _items_align(ItemsAlign::START)
		, _is_lock_child(false)
	{
		_wrap = NO_WRAP;
	}

	/**
		* 
		* 设置主轴的对齐方式
		*
		* @func stt_items_align(align)
		*/
	void Flex::set_items_align(ItemsAlign align) {
		if (align != _items_align) {
			_items_align = align;
			mark(M_LAYOUT_TYPESETTING);
		}
	}


	bool Flex::layout_forward(uint32_t mark) {
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

	bool Flex::layout_reverse(uint32_t mark) {
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

	bool Flex::is_layout_lock_child() {
		return _is_lock_child;
	}

	void Flex::layout_typesetting_change(Layout* child, TypesettingChangeMark mark) {
		mark(M_LAYOUT_TYPESETTING);
	}

}