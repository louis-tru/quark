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

#include "./box.h"
#include "../text/text_lines.h"
#include "../text/text_opts.h"

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue
#define _Border() auto _border = this->_border.load()
#define _IfBorder() _Border(); if (_border)

namespace qk {

	float Box::solve_layout_content_width_pre(Size &pSize) {
		float result = _content_size.x();
		bool pWrap_x = pSize.wrap_x;

		switch (_max_width.kind) {
			case BoxSizeKind::None:
				break;
			case BoxSizeKind::Auto:
			case BoxSizeKind::Value:
				pSize.wrap_x = true;
				return result; // return old value
			case BoxSizeKind::Match:
			case BoxSizeKind::Ratio:
			case BoxSizeKind::Minus:
				pSize.wrap_x = true;
				return pWrap_x ?
					result: // return old value
					0; // return unknown value
		}

		auto val = _min_width;
		auto size = pSize.content.x();

		switch (val.kind) {
			default: /* None default wrap content */
			case BoxSizeKind::Auto: /* 包裹内容 wrap content */
				pSize.wrap_x = true;
				break;
			case BoxSizeKind::Value: /* 明确值 value rem */
				pSize.wrap_x = false;
				result = val.value; // explicit value
				break;
			case BoxSizeKind::Match: /* 匹配父视图 match parent */
				if (!pWrap_x) {// explicit value
					result = size - _margin_left - _margin_right - _padding_left - _padding_right;
					_IfBorder()
						result -= (_border->width[3] + _border->width[1]); // left + right
					result = Float32::max(result, 0);
				}
				break;
			case BoxSizeKind::Ratio: /* 百分比 value % */
				if (!pWrap_x)
					result = Float32::max(size * val.value, 0);
				break;
			case BoxSizeKind::Minus: /* 减法(parent-value) value ! */
				if (!pWrap_x)
					result = Float32::max(size - val.value, 0);
				break;
		}

		return result;
	}

	float Box::solve_layout_content_height_pre(Size &pSize) {
		float result = _content_size.y();
		bool pWrap_y = pSize.wrap_y;

		switch (_max_height.kind) {
			case BoxSizeKind::None:
				break;
			case BoxSizeKind::Auto:
			case BoxSizeKind::Value:
				pSize.wrap_y = true;
				return result; // return old value
			case BoxSizeKind::Match:
			case BoxSizeKind::Ratio:
			case BoxSizeKind::Minus:
				pSize.wrap_y = true;
				return pWrap_y ?
					result: // return old value
					0; // return unknown value
		}

		auto val = _min_height;
		auto size = pSize.content.y();

		switch (val.kind) {
			default: // NONE /* none default wrap content */
			case BoxSizeKind::Auto: /* 包裹内容 wrap content */
				pSize.wrap_y = true;
				break;
			case BoxSizeKind::Value: /* 明确值 value rem */
				pSize.wrap_y = false;
				result = val.value; // explicit value
				break;
			case BoxSizeKind::Match: /* 匹配父视图 match parent */
				if (!pWrap_y) {
					result = size - _margin_top - _margin_bottom - _padding_top - _padding_bottom;
					_IfBorder()
						result -= (_border->width[0] + _border->width[2]); // top + bottom
					result = Float32::max(result, 0);
				}
				break;
			case BoxSizeKind::Ratio: /* 百分比 value % */
				if (!pWrap_y)
					result = Float32::max(size * val.value, 0);
				break;
			case BoxSizeKind::Minus: /* 减法(parent-value) value ! */
				if (!pWrap_y)
					result = Float32::max(size - val.value, 0);
				break;
		}
		return result;
	}

	float Box::solve_layout_content_width_limit(float checkValue) {
		if (_max_width.kind == BoxSizeKind::None) { // no limit
			return checkValue;
		}
		_CheckParent(checkValue);
		auto pSize = _parent->layout_size();
		float limit_min = get_layout_content_min_width_limit(pSize);
		float limit_max = get_layout_content_max_width_limit(pSize);

		if (limit_max <= limit_min) {
			return limit_min; // use min
		}
		return Float32::clamp(checkValue, limit_min, limit_max);
	}

	float Box::solve_layout_content_height_limit(float checkValue) {
		if (_max_height.kind == BoxSizeKind::None) { // no limit
			return checkValue;
		}
		_CheckParent(checkValue);
		auto pSize = _parent->layout_size();
		float limit_min = get_layout_content_min_height_limit(pSize);
		float limit_max = get_layout_content_max_height_limit(pSize);

		if (limit_max <= limit_min) {
			return limit_min; // use min
		}
		return Float32::clamp(checkValue, limit_min, limit_max);
	}

	float Box::get_layout_content_min_width_limit(const Size &pSize) {
		Qk_ASSERT_NE(_max_width.kind, BoxSizeKind::None);
		float size = pSize.content.x();
		float limit_min = 0;
		switch(_min_width.kind) {
			default: break; // Auto, use 0
			case BoxSizeKind::Value:
				limit_min = Float32::max(_min_width.value, 0);
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_x) {
					limit_min = size - _margin_left - _margin_right - _padding_left - _padding_right;
					_IfBorder()
						limit_min -= (_border->width[3] + _border->width[1]); // left + right
					limit_min = Float32::max(limit_min, 0);
				}
				break;
			case BoxSizeKind::Ratio:
				if (!pSize.wrap_x)
					limit_min = Float32::max(size * _min_width.value, 0);
				break;
			case BoxSizeKind::Minus:
				if (!pSize.wrap_x)
					limit_min = Float32::max(size - _min_width.value, 0);
				break;
		}
		return limit_min;
	}

	float Box::get_layout_content_min_height_limit(const Size &pSize) {
		Qk_ASSERT_NE(_max_height.kind, BoxSizeKind::None);
		float size = pSize.content.y();
		float limit_min = 0;
		switch(_min_height.kind) {
			default: break; // AUTO, use 0
			case BoxSizeKind::Value:
				limit_min = Float32::max(_min_height.value, 0);
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_y) {
					limit_min = size - _margin_top - _margin_bottom - _padding_top - _padding_bottom;
					_IfBorder()
						limit_min -= (_border->width[0] + _border->width[2]); // top + bottom
					limit_min = Float32::max(limit_min, 0);
				}
				break;
			case BoxSizeKind::Ratio:
				if (!pSize.wrap_x)
					limit_min = Float32::max(size * _min_height.value, 0);
				break;
			case BoxSizeKind::Minus:
				if (!pSize.wrap_y)
					limit_min = Float32::max(size - _min_height.value, 0);
				break;
		}
		return limit_min;
	}

	float Box::get_layout_content_max_width_limit(const Size &pSize) {
		Qk_ASSERT_NE(_max_width.kind, BoxSizeKind::None);
		float size = pSize.content.x();
		float limit_max = Float32::limit_max; // no limit

		switch(_max_width.kind) {
			default: break; // Auto, use most biggest
			case BoxSizeKind::Value:
				limit_max = _max_width.value;
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_x) {
					limit_max = size - _margin_left - _margin_right - _padding_left - _padding_right;
					_IfBorder()
						limit_max -= (_border->width[3] + _border->width[1]); // left + right
				}
				break;
			case BoxSizeKind::Ratio:
				if (!pSize.wrap_x)
					limit_max = size * _max_width.value;
				break;
			case BoxSizeKind::Minus:
				if (!pSize.wrap_x)
					limit_max = size - _max_width.value;
				break;
		}
		return limit_max;
	}

	float Box::get_layout_content_max_height_limit(const Size &pSize) {
		Qk_ASSERT_NE(_max_height.kind, BoxSizeKind::None);
		float size = pSize.content.y();
		float limit_max = Float32::limit_max;

		switch(_max_height.kind) {
			default: break; // Auto, use most biggest
			case BoxSizeKind::Value:
				limit_max = _max_height.value;
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_y) {
					limit_max = size - _margin_top - _margin_bottom - _padding_top - _padding_bottom;
					_IfBorder()
						limit_max -= (_border->width[0] + _border->width[2]); // top + bottom
				}
				break;
			case BoxSizeKind::Ratio:
				if (!pSize.wrap_y)
					limit_max = size * _max_height.value;
				break;
			case BoxSizeKind::Minus:
				if (!pSize.wrap_y)
					limit_max = size - _max_height.value;
				break;
		}
		return limit_max;
	}

	Region Box::get_layout_content_limit_range(bool onlyX) {
		float min_x = 0, max_x = 0, // Zero means no limit max
					min_y = 0, max_y = 0;

		if (_wrap_x) {
			if (_max_width.kind != BoxSizeKind::None) { // use range limit
				_IfParent() {
					auto pSize = _parent->layout_size();
					min_x = get_layout_content_min_width_limit(pSize);
					max_x = get_layout_content_max_width_limit(pSize);
					max_x = Qk_Max(max_x, 0.1f);
				}
			}
		} else { // no wrap
			min_x = max_x = _content_size.x();
			max_x = Qk_Max(max_x, 0.1f);
		}

		if (_wrap_y) {
			if (!onlyX && _max_height.kind != BoxSizeKind::None) { // use range limit
				_IfParent() {
					auto pSize = _parent->layout_size();
					min_y = get_layout_content_min_height_limit(pSize);
					max_y = get_layout_content_max_height_limit(pSize);
					max_y = Qk_Max(max_y, 0.1f);
				}
			}
		} else {  // no wrap
			min_y = max_y = _content_size.y();
			max_y = Qk_Max(max_y, 0.1f);
		}

		return { { min_x, min_y }, { max_x, max_y } };
	}

	void Box::layout_forward(uint32_t mark) {
		uint32_t change_mark = kLayout_None;

		if (mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			_IfParent() {
				uint32_t child_layout_change_mark = kLayout_None;
				auto size = _parent->layout_size();

				if (mark & kLayout_Size_Width)
				{
					auto val = solve_layout_content_width_pre(size);
					if (val != _content_size.x() || _wrap_x != size.wrap_x) {
						_content_size[0] = val;
						_wrap_x = size.wrap_x;
						change_mark = kLayout_Size_Width;
					}
					_client_size[0] = _padding_left + _padding_right + val;
					_IfBorder() {
						_client_size[0] += _border->width[3] + _border->width[1]; // left + right
					}

					float x = _margin_left + _margin_right + _client_size[0];
					if (_layout_size[0] != x) {
						_layout_size[0] = x;
						child_layout_change_mark = kChild_Layout_Size;
					}
				}

				if (mark & kLayout_Size_Height)
				{
					auto val = solve_layout_content_height_pre(size);
					if (val != _content_size.y() || _wrap_y != size.wrap_y) {
						_content_size[1] = val;
						_wrap_y = size.wrap_y;
						change_mark |= kLayout_Size_Height;
					}
					_client_size[1] = _padding_top + _padding_bottom + val;
					_IfBorder() {
						_client_size[1] += _border->width[0] + _border->width[2]; // top + bottom
					}

					float y = _margin_top + _margin_bottom + _client_size[1];
					if (_layout_size[1] != y) {
						_layout_size[1] = y;
						child_layout_change_mark = kChild_Layout_Size;
					}
				}

				_parent->onChildLayoutChange(this, child_layout_change_mark); // notice parent
				unmark(kLayout_Size_Width | kLayout_Size_Height);
			}
		}

		if (mark & kLayout_Child_Size) {
			change_mark = kLayout_Size_Width | kLayout_Size_Height;
			unmark(kLayout_Child_Size);
		}

		if (change_mark) {
			auto v = first();
			while (v) {
				if (v->visible())
					v->layout_forward(change_mark | v->mark_value());
				v = v->next();
			}
			mark_layout(kLayout_Typesetting | kRecursive_Visible_Region, true); // layout reverse
		}
	}

	void Box::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			layout_typesetting_float();
		}
	}

	Vec2 Box::layout_lock(Vec2 layout_size) {
		uint32_t change_mark = kLayout_None;

		auto bp_x = _padding_left + _padding_right;
		auto bp_y = _padding_top + _padding_bottom;
		_IfBorder() {
			bp_x += _border->width[3] + _border->width[1]; // left + right
			bp_y += _border->width[0] + _border->width[2]; // top + bottom
		}
		auto mbp_x = _margin_left + _margin_right + bp_x;
		auto mbp_y = _margin_top + _margin_bottom + bp_y;

		Vec2 content(
			solve_layout_content_width_limit(Float32::max(layout_size.x() - mbp_x, 0)),
			solve_layout_content_height_limit(Float32::max(layout_size.y() - mbp_y, 0))
		);

		_client_size = Vec2(bp_x + content.x(), bp_y + content.y());
		_layout_size = Vec2(mbp_x + content.x(), mbp_y + content.y());

		if (_content_size.x() != content.x() || _wrap_x) {
			change_mark = kLayout_Size_Width;
		}
		if (_content_size.y() != content.y() || _wrap_y) {
			change_mark |= kLayout_Size_Height;
		}

		// TODO: In a flex layout, only one axis needs to be forcibly locked,
		// Locking all directions may not be the most correct way,
		// but locking all directions here can avoid cyclic iteration and jitter of size.
		_wrap_x = _wrap_y = false;
		_content_size = content;

		if (change_mark) {
			auto v = first();
			while (v) {
				if (v->visible())
					v->layout_forward(change_mark | v->mark_value());
				v = v->next();
			}
			mark_layout(kLayout_Typesetting | kRecursive_Visible_Region, true); // rearrange
		}

		unmark(kLayout_Size_Width | kLayout_Size_Height);

		return _layout_size;
	}

	Vec2 Box::layout_typesetting_float() {
		Vec2 inner_size;
		auto cur = content_size();
		auto cur_x = cur.x();

		auto v = first();
		if (v) {
			if ( _wrap_x ) { // wrap width
				cur_x = 0;
				do {
					if (v->visible()) {
						cur_x = Float32::max(cur_x, v->layout_size().layout.x());
					}
					v = v->next();
				} while(v);
				v = first();
				cur_x = solve_layout_content_width_limit(cur_x);
			}

			float left = 0, right = 0;
			float offset_y = 0;
			float line_width = 0, max_width = 0;
			float line_height = 0;
			Array<View*> centerV;

			auto solveCenter = [&]() {
				if (centerV.length()) {
					float centerSize = (line_width - left - right);
					float startOffset = (cur_x - centerSize) * 0.5;
					if (left > startOffset) { // can't
						startOffset = left;
					} else {
						float diff = (centerSize + startOffset) - (cur_x - right);
						if (diff > 0) { // can't
							startOffset -= diff;
						}
					}
					for (auto v: centerV) {
						v->set_layout_offset({startOffset, offset_y});
						startOffset += v->layout_size().layout.x();
					}
					centerV.clear();
				}
			};

			auto nextStep = [&](Vec2 size) {
				auto new_line_width = line_width + size.x();
				if (new_line_width > cur_x && line_width != 0) { // new line
					solveCenter();
					max_width = Float32::max(max_width, line_width); // select max
					left = right = 0;
					offset_y += line_height;
					line_width = size.x();
					line_height = size.y();
				} else {
					line_width = new_line_width;
					line_height = Float32::max(line_height, size.y()); // select max
				}
			};

			do {
				if (v->visible()) {
					auto size = v->layout_size().layout;
					auto align = v->layout_align();
					switch (align) {
						default:
						case Align::Start: // float start
							nextStep(size);
							v->set_layout_offset(Vec2(left, offset_y));
							left += size.x();
							break;
						case Align::Center: // float center
						case Align::CenterMiddle:
						case Align::CenterBottom:
							nextStep(size);
							centerV.push(v);
							break;
						case Align::End: // float end
						case Align::RightMiddle:
						case Align::RightBottom:
							nextStep(size);
							v->set_layout_offset(Vec2(cur_x - right - size.x(), offset_y));
							right += size.x();
							break;
					}
				}
				v = v->next();
			} while(v);
			solveCenter();
			inner_size = Vec2(Float32::max(max_width, line_width), offset_y + line_height);
		} else {
			if ( _wrap_x ) { // wrap width
				cur_x = solve_layout_content_width_limit(cur_x);
			}
		}
		Vec2 new_size(
			cur_x,
			_wrap_y ? solve_layout_content_height_limit(inner_size.y()): cur.y()
		);

		if (new_size != cur) {
			set_content_size(new_size);
			_IfParent()
				_parent->onChildLayoutChange(this, kChild_Layout_Size);
		}

		unmark(kLayout_Typesetting);

		return inner_size;
	}

	void Box::layout_text(TextLines *lines, TextConfig *cfg) {
		auto opts = cfg->opts();
		auto text_white_space = opts->text_white_space_value();
		//auto text_word_break = opts->text_word_break_value();
		bool is_auto_wrap = true;
		auto limitX = lines->limit_range().end.x();
		auto origin = lines->pre_width();

		if (limitX == 0 || // no limit width
				text_white_space == TextWhiteSpace::NoWrap ||
				text_white_space == TextWhiteSpace::Pre
		) { // 不使用自动wrap
			is_auto_wrap = false;
		}

		if (is_auto_wrap) {
			if (origin + _layout_size.x() > limitX) {
				lines->finish_text_blob_pre();
				lines->push();
				origin = 0;
			}
		} else {
			lines->finish_text_blob_pre();
		}
		set_layout_offset({origin, 0});
		lines->set_pre_width(origin + _layout_size.x());
		lines->add_view(this);
	}

	void Box::set_layout_offset_free(Vec2 size) {
		Vec2 offset;
		switch(_align) {
			case Align::Auto:
			case Align::LeftTop: // left top
				break;
			case Align::CenterTop: // center top
				offset = Vec2((size.x() - _layout_size.x()) * .5, 0);
				break;
			case Align::RightTop: // right top
				offset = Vec2(size.x() - _layout_size.x(), 0);
				break;
			case Align::LeftMiddle: // left Middle
				offset = Vec2(0, (size.y() - _layout_size.y()) * .5);
				break;
			case Align::CenterMiddle: // center Middle
				offset = Vec2(
					(size.x() - _layout_size.x()) * .5,
					(size.y() - _layout_size.y()) * .5);
				break;
			case Align::RightMiddle: // right Middle
				offset = Vec2(
					(size.x() - _layout_size.x()),
					(size.y() - _layout_size.y()) * .5);
				break;
			case Align::LeftBottom: // left bottom
				offset = Vec2(0, (size.y() - _layout_size.y()));
				break;
			case Align::CenterBottom: // center bottom
				offset = Vec2(
					(size.x() - _layout_size.x()) * .5,
					(size.y() - _layout_size.y()));
				break;
			case Align::RightBottom: // right bottom
				offset = Vec2(
					(size.x() - _layout_size.x()),
					(size.y() - _layout_size.y()));
				break;
		}
		set_layout_offset(offset);
	}

}
