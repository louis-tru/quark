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

#include "./flow.h"

namespace flare {

	void View::Visitor::visitFlowLayout(FlowLayout *v) {
		visitBox(v);
	}

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void FlowLayout::accept(Visitor *visitor) {
		visitor->visitFlowLayout(this);
	}

	F_DEFINE_INLINE_MEMBERS(FlowLayout, Inl) {
	 public:
		#define _inl(self) static_cast<FlowLayout::Inl*>(self)

		void set_wrap(Wrap wrap) {
			_wrap = wrap;
		}

		static float parseAlignSpace(WrapAlign align,  bool is_reverse, float overflow, int count, float *space_out) {
			float offset_x = 0, space = 0;

			switch (align) {
				default: break;
				case WrapAlign::START: // 左对齐
					if (is_reverse) {
						offset_x = overflow;
					}
					break;
				case WrapAlign::CENTER: // 居中
					offset_x =  overflow / 2;
					break;
				case WrapAlign::END: // 右对齐
					if (!is_reverse) {
						offset_x = overflow;
					}
					break;
				case WrapAlign::SPACE_BETWEEN: // 两端对齐，项目之间的间隔都相等
					if (overflow > 0) {
						space = overflow / (count - 1);
					} else {
						if (is_reverse) {
							offset_x = overflow;
						}
					}
					break;
				case WrapAlign::SPACE_AROUND: // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
					if (overflow > 0) {
						space = overflow / count;
						offset_x = space / 2;
					} else {
						offset_x = overflow / 2;
					}
					break;
				case WrapAlign::SPACE_EVENLY: // 每个项目两侧的间隔相等,这包括边框的间距
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
		void layout_typesetting_from_auto(Size cur_size, bool is_reverse) {
			// get layouts raw size total
			float offset = 0, max_cross = 0;
			Vec2 cur = cur_size.content_size;
			struct Item { Vec2 s; View* v; };
			Array<Item> start, center, end;

			Vec2 origin(margin_left() + padding_left(), margin_top() + padding_top());

			auto v = first();
			while (v) {
				auto size = v->layout_size().layout_size;
				if (cur_size.wrap_y) {
					auto cross = is_horizontal ? size.y(): size.x();
					max_cross = F_MAX(max_cross, cross);
				}
				switch (v->layout_align()) {
					default:
					case Align::START: start.push({ size, v }); break;
					case Align::CENTER: center.push({ size, v }); break;
					case Align::END: end.push({ size, v }); break;
				}
				v = v->next();
			}

			if (!cur_size.wrap_y) { // no wrap y
				max_cross = is_horizontal ? cur.y(): cur.x();
			}

			start.concat(std::move(center)).concat(std::move(end));
			if (is_reverse) {
				start.reverse();
			}

			for (auto i: start) {
				auto v = i.v;
				auto s = i.s;
				float offset_cross = 0;
				switch (_cross_align) {
					default:
					case CrossAlign::START: break; // 与交叉轴内的起点对齐
					case CrossAlign::CENTER: // 与交叉轴内的中点对齐
						offset_cross = (max_cross - (is_horizontal ? s.y(): s.x())) / 2.0; break;
					case CrossAlign::END: // 与交叉轴内的终点对齐
						offset_cross = max_cross - (is_horizontal ? s.y(): s.x()); break;
				}
				if (is_horizontal) {
					v->set_layout_offset(Vec2(offset, offset_cross) + origin);
					offset += s.x();
				} else {
					v->set_layout_offset(Vec2(offset_cross, offset) + origin);
					offset += s.y();
				}
			}

			Vec2 new_size = is_horizontal ? Vec2(offset, max_cross): Vec2(max_cross, offset);

			if (cur != new_size) {
				set_layout_size(new_size);
				parent()->layout_typesetting_change(this);
			}
		}

		template<bool is_horizontal>
		void layout_typesetting_from_wrap(Size cur_size, bool is_reverse) {
			struct Line {
				struct Item {
					Vec2 s; View* v; bool space;
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
			bool wrap_reverse = _wrap == Wrap::WRAP_REVERSE;

			Vec2 origin(margin_left() + padding_left(), margin_top() + padding_top());

			Array<typename Line::Item> _start, _center, _end;
			float _total_main = 0, _max_cross = 0;

			auto v = first();
			while (v) {
				auto size = v->layout_size().layout_size;
				auto main = _total_main + (is_horizontal ? size.x(): size.y());
				if (main > main_size) { // Line feed
					_start.push({0,0}).concat(std::move(_center))
									 .push({0,0}).concat(std::move(_end));
					if (is_reverse)
						_start.reverse();
					lines.push({ _total_main, _max_cross, std::move(_start) });
					max_main = F_MAX(max_main, _total_main);
					total_cross += _max_cross;
					_total_main = is_horizontal ? size.x(): size.y();
					_max_cross = is_horizontal ? size.y(): size.x();
				} else {
					_total_main = main;
					_max_cross = F_MAX(_max_cross, size.y());
				}
				switch (v->layout_align()) {
					default:
					case Align::START: _start.push({ size,v }); break;
					case Align::CENTER: _center.push({ size,v }); break;
					case Align::END: _end.push({ size,v }); break;
				}
				v = v->next();
			}

			if (_start.length()+_center.length()+_end.length()) {
				_start.push({0,0}).concat(std::move(_center))
								 .push({0,0}).concat(std::move(_end));
				if (is_reverse)
					_start.reverse();
				lines.push({ _total_main, _max_cross, std::move(_start) });
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
				if (WrapAlign::STRETCH == _wrap_align) {
					cross_overflow_item = lines.length() ? cross_overflow / lines.length() : 0;
				} else {
					cross_offset = parseAlignSpace(
						_wrap_align, wrap_reverse, cross_overflow, lines.length(), &cross_space);
				}
			}

			for (auto& i: lines) {
				float cross = i.max_cross + cross_overflow_item;
				float overflow = main_size - i.total_main;
				float offset = 0;
				float space = overflow / 2;

				for (auto j: i.items) {
					auto s = j.s;
					auto v = j.v;
					if (v) {
						float cross_offset_item = cross_offset;
						switch (_cross_align) {
							default:
							case CrossAlign::START: break; // 与交叉轴内的起点对齐
							case CrossAlign::CENTER: // 与交叉轴内的中点对齐
								cross_offset_item += ((cross - (is_horizontal ? s.y(): s.x())) / 2.0); break;
							case CrossAlign::END: // 与交叉轴内的终点对齐
								cross_offset_item += (cross - (is_horizontal ? s.y(): s.x())); break;
						}
						if (is_horizontal) {
							v->set_layout_offset(Vec2(offset, cross_offset_item) + origin);
							offset += s.x();
						} else {
							v->set_layout_offset(Vec2(cross_offset_item, offset) + origin);
							offset += s.y();
						}
					} else { // space
						offset += space;
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

	};

	float __Flow_ParseAlignSpace(WrapAlign align,  bool is_reverse, float overflow, int count, float *space_out) {
		return FlowLayout::Inl::parseAlignSpace(align, is_reverse, overflow, count, space_out);
	}

	void __Flow_set_wrap(FlowLayout* self, Wrap wrap) {
		_inl(self)->set_wrap(wrap);
	}

	/**
		* @constructors
		*/
	FlowLayout::FlowLayout()
		: _direction(Direction::ROW)
		, _cross_align(CrossAlign::START)
		, _wrap(Wrap::WRAP)
		, _wrap_align(WrapAlign::START)
	{
	}

	/**
		*
		* 设置主轴的方向
		*
		* @func set_direction(val)
		*/
	void FlowLayout::set_direction(Direction val) {
		if (val != _direction) {
			_direction = val;
			mark(M_LAYOUT_TYPESETTING); // 排版参数改变,后续需对子布局重新排版
		}
	}

	/**
		* 
		* 设置交叉轴的对齐方式
		*
		* @func set_cross_align(align)
		*/
	void FlowLayout::set_cross_align(CrossAlign align) {
		if (align != _cross_align) {
			_cross_align = align;
			mark(M_LAYOUT_TYPESETTING);
		}
	}

	/**
		* 
		* 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
		*
		* @func set_wrap_reverse(wrap)
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

	// --------------- o v e r w r i t e ---------------

	bool FlowLayout::layout_reverse(uint32_t mark) {
		if (mark & (M_LAYOUT_TYPESETTING)) {
			if (!is_ready_layout_typesetting()) {
				return true; // continue iteration
			}
			if (_direction == Direction::ROW || _direction == Direction::ROW_REVERSE) {
				if (_wrap == Wrap::NO_WRAP) { // no wrap
					_inl(this)->layout_typesetting_from_auto<true>(layout_size(), _direction == Direction::ROW_REVERSE);
				} else {
					_inl(this)->layout_typesetting_from_wrap<true>(layout_size(), _direction == Direction::ROW_REVERSE);
				}
			} else {
				if (_wrap == Wrap::NO_WRAP) { // no wrap
					_inl(this)->layout_typesetting_from_auto<false>(layout_size(), _direction == Direction::COLUMN_REVERSE);
				} else {
					_inl(this)->layout_typesetting_from_wrap<false>(layout_size(), _direction == Direction::COLUMN_REVERSE);
				}
			}

			unmark(M_LAYOUT_TYPESETTING);
		}
		return false;
	}

}

// *******************************************************************
