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
#include "../app.h"
#include "../window.h"
#include "../screen.h"
#include "../../render/render.h"
#include "../text/text_lines.h"
#include "../text/text_opts.h"

namespace qk {

	float Box::solve_layout_content_width(Size &pSize) {
		float result = _content_size.x();
		if (_max_width.kind != BoxSizeKind::None) { // use range size wrap
			pSize.wrap_x = true;
			return result; // return old value
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
				if (!pSize.wrap_x) {// explicit value
					result = size - _margin_left - _margin_right - _padding_left - _padding_right;
					if (_border)
						result -= (_border->width[3] + _border->width[1]); // left + right
					result = Float32::max(result, 0);
				}
				break;
			case BoxSizeKind::Ratio: /* 百分比 value % */
				if (!pSize.wrap_x)
					result = Float32::max(size * val.value, 0);
				break;
			case BoxSizeKind::Minus: /* 减法(parent-value) value ! */
				if (!pSize.wrap_x)
					result = Float32::max(size - val.value, 0);
				break;
		}
		return result;
	}

	float Box::solve_layout_content_height(Size &pSize) {
		float result = _content_size.y();
		if (_max_height.kind != BoxSizeKind::None) { // use range size wrap
			pSize.wrap_y = true;
			return result; // return old value
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
				if (!pSize.wrap_y) {
					result = size - _margin_top - _margin_bottom - _padding_top - _padding_bottom;
					if (_border)
						result -= (_border->width[0] + _border->width[2]); // top + bottom
					result = Float32::max(result, 0);
				}
				break;
			case BoxSizeKind::Ratio: /* 百分比 value % */
				if (!pSize.wrap_y)
					result = Float32::max(size * val.value, 0);
				break;
			case BoxSizeKind::Minus: /* 减法(parent-value) value ! */
				if (!pSize.wrap_y)
					result = Float32::max(size - val.value, 0);
				break;
		}
		return result;
	}

	float Box::get_max_width_limit_value(const Size &pSize) {
		auto size = pSize.content.x();
		float limit_max = Float32::limit_max;

		switch(_max_width.kind) {
			default: break;
			case BoxSizeKind::Value:
				limit_max = _max_width.value;
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_x) {
					limit_max = size - _margin_left - _margin_right - _padding_left - _padding_right;
					if (_border)
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

	float Box::get_max_height_limit_value(const Size &pSize) {
		auto size = pSize.content.y();
		float limit_max = Float32::limit_max;

		switch(_max_height.kind) {
			default: break;
			case BoxSizeKind::Value:
				limit_max = _max_height.value;
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_y) {
					limit_max = size - _margin_top - _margin_bottom - _padding_top - _padding_bottom;
					if (_border)
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

	float Box::solve_layout_content_wrap_limit_width(float inside) {
		if (_max_width.kind == BoxSizeKind::None) { // no limit
			return inside;
		}
		auto pSize = parent_Rt()->layout_size();
		auto size = pSize.content.x();
		float limit_min = 0, limit_max = get_max_width_limit_value(pSize);

		switch(_min_width.kind) {
			default: break;
			case BoxSizeKind::Value:
				limit_min = Float32::max(_min_width.value, 0);
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_x) {
					limit_min = size - _margin_left - _margin_right - _padding_left - _padding_right;
					if (_border)
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

		if (limit_max <= limit_min) {
			return limit_min;
		}
		return Float32::clamp(inside, limit_min, limit_max);
	}

	float Box::solve_layout_content_wrap_limit_height(float inside) {
		if (_max_height.kind == BoxSizeKind::None) { // no limit
			return inside;
		}
		auto pSize = parent_Rt()->layout_size();
		auto size = pSize.content.y();
		float limit_min = 0, limit_max = get_max_height_limit_value(pSize);

		switch(_min_height.kind) {
			default: break;
			case BoxSizeKind::Value:
				limit_min = Float32::max(_min_height.value, 0);
				break;
			case BoxSizeKind::Match:
				if (!pSize.wrap_y) {
					limit_min = size - _margin_top - _margin_bottom - _padding_top - _padding_bottom;
					if (_border)
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

		if (limit_max <= limit_min) {
			return limit_min;
		}
		return Float32::clamp(inside, limit_min, limit_max);
	}

	Box::Box()
		: _wrap_x(true), _wrap_y(true), _clip(false)
		, _align(Align::Auto)
		, _min_width{0, BoxSizeKind::Auto}, _min_height{0, BoxSizeKind::Auto}
		, _max_width{0, BoxSizeKind::None}, _max_height{0, BoxSizeKind::None}
		, _margin_top(0), _margin_right(0)
		, _margin_bottom(0), _margin_left(0)
		, _padding_top(0), _padding_right(0)
		, _padding_bottom(0), _padding_left(0)
		, _border_radius_left_top(0), _border_radius_right_top(0)
		, _border_radius_right_bottom(0), _border_radius_left_bottom(0)
		, _background_color(Color::from(0))
		, _weight(0)
		, _background(nullptr)
		, _boxShadow(nullptr)
		, _border(nullptr)
	{
	}

	Box::~Box() {
		Release(_background); _background = nullptr;
		Release(_boxShadow); _boxShadow = nullptr;
		::free(_border); _border = nullptr;
	}

	// is clip box display range
	void Box::set_clip(bool val, bool isRt) {
		if (_clip != val) {
			_clip = val;
			mark(kLayout_None, isRt);
		}
	}

	BoxSize Box::width() const {
		return _min_width;
	}

	BoxSize Box::height() const {
		return _min_height;
	}

	void Box::set_width(BoxSize val, bool isRt) {
		if (_min_width != val) {
			_min_width = val;
			mark_layout(kLayout_Size_Width, isRt);
		}
	}

	void Box::set_height(BoxSize val, bool isRt) {
		if (_min_height != val) {
			_min_height = val;
			mark_layout(kLayout_Size_Height, isRt);
		}
	}

	void Box::set_min_width(BoxSize val, bool isRt) {
		set_width(val, isRt);
	}

	void Box::set_min_height(BoxSize val, bool isRt) {
		set_height(val, isRt);
	}

	void Box::set_max_width(BoxSize val, bool isRt) {
		if (_max_width != val) {
			_max_width = val;
			mark_layout(kLayout_Size_Width, isRt);
		}
	}

	void Box::set_max_height(BoxSize val, bool isRt) {
		if (_max_height != val) {
			_max_height = val;
			mark_layout(kLayout_Size_Height, isRt);
		}
	}

	ArrayFloat Box::margin() const {
		return ArrayFloat{_margin_top,_margin_right,_margin_bottom,_margin_left};
	}

	void Box::set_margin(ArrayFloat val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_margin_left(val[0], isRt);
				set_margin_top(val[0], isRt);
				set_margin_right(val[0], isRt);
				set_margin_bottom(val[0], isRt);
				break;
			case 2:
				set_margin_top(val[0], isRt);
				set_margin_bottom(val[0], isRt);
				set_margin_left(val[1], isRt);
				set_margin_right(val[1], isRt);
				break;
			case 3:
				set_margin_top(val[0], isRt);
				set_margin_left(val[1], isRt);
				set_margin_right(val[1], isRt);
				set_margin_bottom(val[2], isRt);
				break;
			case 4: // 4
				set_margin_top(val[0], isRt);
				set_margin_right(val[1], isRt);
				set_margin_bottom(val[2], isRt);
				set_margin_left(val[3], isRt);
				break;
			default: break;
		}
	}

	void Box::set_margin_top(float val, bool isRt) { // margin
		if (_margin_top != val) {
			_margin_top = val;
			mark_layout(kLayout_Size_Height | kRecursive_Transform, isRt);
		}
	}

	void Box::set_margin_left(float val, bool isRt) {
		if (_margin_left != val) {
			_margin_left = val;
			mark_layout(kLayout_Size_Width | kRecursive_Transform, isRt);
		}
	}

	void Box::set_margin_right(float val, bool isRt) {
		if (_margin_right != val) {
			_margin_right = val;
			mark_layout(kLayout_Size_Width, isRt);
		}
	}

	void Box::set_margin_bottom(float val, bool isRt) {
		if (_margin_bottom != val) {
			_margin_bottom = val;
			mark_layout(kLayout_Size_Height, isRt);
		}
	}

	ArrayFloat Box::padding() const {
		return ArrayFloat{_padding_top,_padding_right,_padding_bottom,_padding_left};
	}

	void Box::set_padding(ArrayFloat val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_padding_left(val[0], isRt);
				set_padding_top(val[0], isRt);
				set_padding_right(val[0], isRt);
				set_padding_bottom(val[0], isRt);
				break;
			case 2:
				set_padding_top(val[0], isRt);
				set_padding_bottom(val[0], isRt);
				set_padding_left(val[1], isRt);
				set_padding_right(val[1], isRt);
				break;
			case 3:
				set_padding_top(val[0], isRt);
				set_padding_left(val[1], isRt);
				set_padding_right(val[1], isRt);
				set_padding_bottom(val[2], isRt);
				break;
			case 4: // 4
				set_padding_top(val[0], isRt);
				set_padding_right(val[1], isRt);
				set_padding_bottom(val[2], isRt);
				set_padding_left(val[3], isRt);
				break;
			default: break;
		}
	}

	void Box::set_padding_top(float val, bool isRt) { // padding
		if (_padding_top != val) {
			_padding_top = val;
			mark_layout(kLayout_Size_Height, isRt);
		}
	}

	void Box::set_padding_left(float val, bool isRt) {
		if (_padding_left != val) {
			_padding_left = val;
			mark_layout(kLayout_Size_Width, isRt);
		}
	}

	void Box::set_padding_right(float val, bool isRt) {
		if (_padding_right != val) {
			_padding_right = val;
			mark_layout(kLayout_Size_Width, isRt);
		}
	}

	void Box::set_padding_bottom(float val, bool isRt) {
		if (_padding_bottom != val) {
			_padding_bottom = val;
			mark_layout(kLayout_Size_Height, isRt);
		}
	}

	// -- border radius

	ArrayFloat Box::border_radius() const {
		return ArrayFloat{
			_border_radius_left_top,_border_radius_right_top,
			_border_radius_right_bottom,_border_radius_left_bottom
		};
	}

	void Box::set_border_radius(ArrayFloat val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_border_radius_left_top(val[0], isRt);
				set_border_radius_right_top(val[0], isRt);
				set_border_radius_right_bottom(val[0], isRt);
				set_border_radius_left_bottom(val[0], isRt);
				break;
			case 2:
				set_border_radius_left_top(val[0], isRt);
				set_border_radius_right_top(val[0], isRt);
				set_border_radius_right_bottom(val[1], isRt);
				set_border_radius_left_bottom(val[1], isRt);
				break;
			case 3:
				set_border_radius_left_top(val[0], isRt);
				set_border_radius_right_top(val[1], isRt);
				set_border_radius_right_bottom(val[2], isRt);
				set_border_radius_left_bottom(val[2], isRt);
				break;
			case 4: // 4
				set_border_radius_left_top(val[0], isRt);
				set_border_radius_right_top(val[1], isRt);
				set_border_radius_right_bottom(val[2], isRt);
				set_border_radius_left_bottom(val[3], isRt);
				break;
			default: break;
		}
	}

	void Box::set_border_radius_left_top(float val, bool isRt) {
		if (val >= 0.0 && _border_radius_left_top != val) {
			_border_radius_left_top = val;
			mark(kLayout_None, isRt);
		}
	}

	void Box::set_border_radius_right_top(float val, bool isRt) {
		if (val >= 0.0 && _border_radius_right_top != val) {
			_border_radius_right_top = val;
			mark(kLayout_None, isRt);
		}
	}

	void Box::set_border_radius_right_bottom(float val, bool isRt) {
		if (val >= 0.0 && _border_radius_right_bottom != val) {
			_border_radius_right_bottom = val;
			mark(kLayout_None, isRt);
		}
	}

	void Box::set_border_radius_left_bottom(float val, bool isRt) {
		if (val >= 0.0 && _border_radius_left_bottom != val) {
			_border_radius_left_bottom = val;
			mark(kLayout_None, isRt);
		}
	}

	static BoxBorder  default_border{0,Color::from(0)};
	static ArrayColor default_border_color{Color::from(0),Color::from(0),Color::from(0),Color::from(0)};
	static ArrayFloat default_border_width(4);

	ArrayBorder Box::border() const {
		return _border ? ArrayBorder{
			{_border->width[0],_border->color[0]},{_border->width[1],_border->color[1]},
			{_border->width[2],_border->color[2]},{_border->width[3],_border->color[3]},
		}: ArrayBorder{default_border,default_border,default_border,default_border};
	}

	void Box::set_border(ArrayBorder val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_border_top(val[0], isRt);
				set_border_right(val[0], isRt);
				set_border_bottom(val[0], isRt);
				set_border_left(val[0], isRt);
				break;
			case 2:
				set_border_top(val[0], isRt);
				set_border_bottom(val[0], isRt);
				set_border_left(val[1], isRt);
				set_border_right(val[1], isRt);
				break;
			case 3:
				set_border_top(val[0], isRt);
				set_border_left(val[1], isRt);
				set_border_right(val[1], isRt);
				set_border_bottom(val[2], isRt);
				break;
			case 4: // 4
				set_border_top(val[0], isRt);
				set_border_right(val[1], isRt);
				set_border_bottom(val[2], isRt);
				set_border_left(val[3], isRt);
				break;
			default: break;
		}
	}

	BoxBorder Box::border_top() const {
		return _border ? BoxBorder{_border->width[0],_border->color[0]}: default_border;
	}

	BoxBorder Box::border_right() const {
		return _border ? BoxBorder{_border->width[1],_border->color[1]}: default_border;
	}

	BoxBorder Box::border_bottom() const {
		return _border ? BoxBorder{_border->width[2],_border->color[2]}: default_border;
	}

	BoxBorder Box::border_left() const {
		return _border ? BoxBorder{_border->width[3],_border->color[3]}: default_border;
	}

	void Box::set_border_top(BoxBorder border, bool isRt) {
		set_border_width_top(border.width, isRt);
		set_border_color_top(border.color, isRt);
	}

	void Box::set_border_right(BoxBorder border, bool isRt) {
		set_border_width_right(border.width, isRt);
		set_border_color_right(border.color, isRt);
	}

	void Box::set_border_bottom(BoxBorder border, bool isRt) {
		set_border_width_bottom(border.width, isRt);
		set_border_color_bottom(border.color, isRt);
	}

	void Box::set_border_left(BoxBorder border, bool isRt) {
		set_border_width_left(border.width, isRt);
		set_border_color_left(border.color, isRt);
	}

	ArrayFloat Box::border_width() const {
		return _border ? 
			ArrayFloat({_border->width[0],_border->width[1],_border->width[2],_border->width[3]}):
			default_border_width;
	}

	void Box::set_border_width(ArrayFloat val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_border_width_top(val[0], isRt);
				set_border_width_right(val[0], isRt);
				set_border_width_bottom(val[0], isRt);
				set_border_width_left(val[0], isRt);
				break;
			case 2:
				set_border_width_top(val[0], isRt);
				set_border_width_bottom(val[0], isRt);
				set_border_width_left(val[1], isRt);
				set_border_width_right(val[1], isRt);
				break;
			case 3:
				set_border_width_top(val[0], isRt);
				set_border_width_left(val[1], isRt);
				set_border_width_right(val[1], isRt);
				set_border_width_bottom(val[2], isRt);
				break;
			case 4: // 4
				set_border_width_top(val[0], isRt);
				set_border_width_right(val[1], isRt);
				set_border_width_bottom(val[2], isRt);
				set_border_width_left(val[3], isRt);
				break;
			default: break;
		}
	}

	ArrayColor Box::border_color() const {
		return _border ? 
			ArrayColor({_border->color[0],_border->color[1],_border->color[2],_border->color[3]}):
			default_border_color;
	}

	void Box::set_border_color(ArrayColor val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_border_color_top(val[0], isRt);
				set_border_color_right(val[0], isRt);
				set_border_color_bottom(val[0], isRt);
				set_border_color_left(val[0], isRt);
				break;
			case 2:
				set_border_color_top(val[0], isRt);
				set_border_color_bottom(val[0], isRt);
				set_border_color_left(val[1], isRt);
				set_border_color_right(val[1], isRt);
				break;
			case 3:
				set_border_color_top(val[0], isRt);
				set_border_color_left(val[1], isRt);
				set_border_color_right(val[1], isRt);
				set_border_color_bottom(val[2], isRt);
				break;
			case 4: // 4
				set_border_color_top(val[0], isRt);
				set_border_color_right(val[1], isRt);
				set_border_color_bottom(val[2], isRt);
				set_border_color_left(val[3], isRt);
				break;
			default: break;
		}
	}

	Color Box::border_color_top() const {
		return _border ? _border->color[0]: Color::from(0);
	}

	Color Box::border_color_right() const {
		return _border ? _border->color[1]: Color::from(0);
	}

	Color Box::border_color_bottom() const {
		return _border ? _border->color[2]: Color::from(0);
	}

	Color Box::border_color_left() const {
		return _border ? _border->color[3]: Color::from(0);
	}

	float Box::border_width_top() const {
		return _border ? _border->width[0]: 0;
	}

	float Box::border_width_right() const {
		return _border ? _border->width[1]: 0;
	}

	float Box::border_width_bottom() const {
		return _border ? _border->width[2]: 0;
	}

	float Box::border_width_left() const {
		return _border ? _border->width[3]: 0;
	}

	struct SetBorder: public Box {
		#define _SetBorder static_cast<SetBorder*>(this)->set
		void alloc() {
			if (!_border) {// alloc border memory
				_border = (BoxBorderInl*)::malloc(sizeof(BoxBorderInl));
				::memset(_border, 0, sizeof(BoxBorderInl));
			}
		}
		template<typename E, typename Arg>
		void set(E exec, Arg arg, bool isRt) {
			typedef void (*Exec)(Box*, Arg, bool isRt);
			if (_border) {
				((Exec)exec)(this, arg, isRt);
			} else if (isRt) {
				alloc();
				((Exec)exec)(this, arg, true);
			} else {
				struct Arg_ { Arg val; Exec exec; };
				preRender().async_call([](auto self, auto arg) {
					self->alloc();
					arg.arg->exec(self, arg.arg->val, true);
					delete arg.arg;
				}, this, new Arg_{arg,(Exec)exec});
			}
		}
	};

	void Box::set_border_color_top(Color val, bool isRt) {
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->color[0] != val) {
				self->_border->color[0] = val;
				self->mark(kLayout_None, isRt);
			}
		}, val, isRt);
	}

	void Box::set_border_color_right(Color val, bool isRt) {
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->color[1] != val) {
				self->_border->color[1] = val;
				self->mark(kLayout_None, isRt);
			}
		}, val, isRt);
	}

	void Box::set_border_color_bottom(Color val, bool isRt) {
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->color[2] != val) {
				self->_border->color[2] = val;
				self->mark(kLayout_None, isRt);
			}
		}, val, isRt);
	}

	void Box::set_border_color_left(Color val, bool isRt) {
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->color[3] != val) {
				self->_border->color[3] = val;
				self->mark(kLayout_None, isRt);
			}
		}, val, isRt);
	}

	void Box::set_border_width_top(float val, bool isRt) {
		val = Qk_MAX(0, val);
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->width[0] != val) {
				self->_border->width[0] = val;
				self->mark_layout(kLayout_Size_Height, isRt);
			}
		}, val, isRt);
	}

	void Box::set_border_width_right(float val, bool isRt) {
		val = Qk_MAX(0, val);
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->width[1] != val) {
				self->_border->width[1] = val;
				self->mark_layout(kLayout_Size_Width, isRt);
			}
		}, val, isRt);
	}

	void Box::set_border_width_bottom(float val, bool isRt) {
		val = Qk_MAX(0, val);
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->width[2] != val) {
				self->_border->width[2] = val;
				self->mark_layout(kLayout_Size_Height, isRt);
			}
		}, val, isRt);
	}

	void Box::set_border_width_left(float val, bool isRt) {
		val = Qk_MAX(0, val);
		_SetBorder([](auto self, auto val, auto isRt) {
			if (self->_border->width[3] != val) {
				self->_border->width[3] = val;
				self->mark_layout(kLayout_Size_Width, isRt);
			}
		}, val, isRt);
	}

	void Box::set_background_color(Color color, bool isRt) {
		if (_background_color != color) {
			_background_color = color;
			mark(kLayout_None,isRt);
		}
	}

	BoxFilter* Box::background() {
		return BoxFilter::safe_filter(_background);
	}

	BoxShadow* Box::box_shadow() {
		return static_cast<BoxShadow*>(BoxFilter::safe_filter(_boxShadow));
	}

	void Box::set_background(BoxFilter* val, bool isRt) {
		if (isRt) {
			if (_background != val) {
				_background = BoxFilter::assign_Rt(_background, val, this);
			}
		} else {
			preRender().async_call([](auto self, auto arg) {
				if (self->_background != arg.arg) {
					self->_background = BoxFilter::assign_Rt(self->_background, arg.arg, self);
				}
			}, this, val);
		}
	}

	void Box::set_box_shadow(BoxShadow* val, bool isRt) {
		if (isRt) {
			if (_boxShadow != val) {
				_boxShadow = static_cast<BoxShadow*>(BoxFilter::assign_Rt(_boxShadow, val, this));
			}
		} else {
			preRender().async_call([](auto self, auto arg) {
				if (self->_boxShadow != arg.arg) {
					self->_boxShadow = static_cast<BoxShadow*>(BoxFilter::assign_Rt(self->_boxShadow, arg.arg, self));
				}
			}, this, val);
		}
	}

	uint32_t Box::solve_layout_forward(uint32_t mark) {
		uint32_t change_mark = kLayout_None;

		if (mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			uint32_t child_layout_change_mark = kLayout_None;
			auto parent = parent_Rt();
			auto size = parent->layout_size();

			if (mark & kLayout_Size_Width)
			{
				auto val = solve_layout_content_width(size);
				if (val != _content_size.x() || _wrap_x != size.wrap_x) {
					_content_size[0] = val;
					_wrap_x = size.wrap_x;
					change_mark = kLayout_Size_Width;
				}
				_client_size[0] = _padding_left + _padding_right + val;
				if (_border)
					_client_size[0] += _border->width[3] + _border->width[1]; // left + right

				float x = _margin_left + _margin_right + _client_size[0];
				if (_layout_size[0] != x) {
					_layout_size[0] = x;
					child_layout_change_mark = kChild_Layout_Size;
				}
			}

			if (mark & kLayout_Size_Height)
			{
				auto val = solve_layout_content_height(size);
				if (val != _content_size.y() || _wrap_y != size.wrap_y) {
					_content_size.set_y(val);
					_wrap_y = size.wrap_y;
					change_mark |= kLayout_Size_Height;
				}
				_client_size[1] = _padding_top + _padding_bottom + val;
				if (_border)
					_client_size[1] += _border->width[0] + _border->width[2]; // top + bottom

				float y = _margin_top + _margin_bottom + _client_size[1];
				if (_layout_size[1] != y) {
					_layout_size[1] = y;
					child_layout_change_mark = kChild_Layout_Size;
				}
			}

			parent->onChildLayoutChange(this, child_layout_change_mark); // notice parent
			unmark(kLayout_Size_Width | kLayout_Size_Height);
		}

		if (mark & kLayout_Child_Size) {
			change_mark = kLayout_Size_Width | kLayout_Size_Height;
			unmark(kLayout_Child_Size);
		}

		return change_mark;
	}

	void Box::layout_forward(uint32_t _mark) {
		uint32_t change_mark = solve_layout_forward(_mark);

		if (change_mark) {
			auto v = first_Rt();
			while (v) {
				if (v->visible())
					v->layout_forward(change_mark | v->mark_value());
				v = v->next_Rt();
			}
			mark_layout(kLayout_Typesetting | kRecursive_Visible_Region, true); // layout reverse
		}
	}

	void Box::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			layout_typesetting_box();
		}
	}

	Vec2 Box::layout_lock(Vec2 layout_size) {
		uint32_t change_mark = kLayout_None;

		auto bp_x = _padding_left + _padding_right;
		auto bp_y = _padding_left + _padding_right;
		if (_border) {
			bp_x += _border->width[3] + _border->width[1]; // left + right
			bp_y += _border->width[0] + _border->width[2]; // top + bottom
		}
		auto mbp_x = _margin_left + _margin_right + bp_x;
		auto mbp_y = _margin_top + _margin_bottom + bp_y;

		Vec2 content(
			solve_layout_content_wrap_limit_width(layout_size.x() > mbp_x ? layout_size.x() - mbp_x: 0),
			solve_layout_content_wrap_limit_height(layout_size.y() > mbp_y ? layout_size.y() - mbp_y: 0)
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
			auto v = first_Rt();
			while (v) {
				if (v->visible())
					v->layout_forward(change_mark | v->mark_value());
				v = v->next_Rt();
			}
			mark_layout(kLayout_Typesetting | kRecursive_Visible_Region, true); // rearrange
		}

		unmark(kLayout_Size_Width | kLayout_Size_Height);

		return _layout_size;
	}

	Vec2 Box::layout_typesetting_box() {
		Vec2 inner_size;
		auto cur = content_size();
		auto cur_x = cur.x();

		auto v = first_Rt();
		if (v) {
			if ( _wrap_x ) { // wrap width
				cur_x = 0;
				do {
					if (v->visible()) {
						cur_x = Float32::max(cur_x, v->layout_size().layout.x());
					}
					v = v->next_Rt();
				} while(v);
				v = first_Rt();
				cur_x = solve_layout_content_wrap_limit_width(cur_x);
			}

			float offset_left = 0, offset_right = 0;
			float offset_y = 0;
			float line_width = 0, max_width = 0;
			float line_height = 0;

			do {
				if (v->visible()) {
					auto size = v->layout_size().layout;
					auto align = v->layout_align();

					if (align == Align::Auto) { // new line
						offset_y += line_height;
						v->set_layout_offset({0, offset_y});
						offset_left = offset_right = 0;
						line_width = line_height = 0;
						offset_y += size.y();
					} else {
						auto new_line_width = line_width + size.x();

						if (new_line_width > cur_x && line_width != 0) { // new line
							max_width = Float32::max(max_width, line_width); // select max
							offset_left = offset_right = 0;
							offset_y += line_height;
							line_width = size.x();
							line_height = size.y();
						} else {
							line_width = new_line_width;
							line_height = Float32::max(line_height, size.y()); // select max
						}

						switch (align) {
							case Align::LeftTop:
							case Align::LeftCenter:
							case Align::LeftBottom: // left
								v->set_layout_offset(Vec2(offset_left, offset_y));
								offset_left += size.x();
								break;
							default: // right
								v->set_layout_offset(Vec2(cur_x - offset_right - size.x(), offset_y));
								offset_right += size.x();
								break;
						}
					}
				}
				v = v->next_Rt();
			} while(v);

			inner_size = Vec2(Float32::max(max_width, line_width), offset_y + line_height);
		} else {
			if ( _wrap_x ) { // wrap width
				cur_x = solve_layout_content_wrap_limit_width(cur_x);
			}
		}
		Vec2 new_size(
			cur_x,
			_wrap_y ? solve_layout_content_wrap_limit_height(inner_size.y()): cur.y()
		);

		if (new_size != cur) {
			set_content_size(new_size);
			parent_Rt()->onChildLayoutChange(this, kChild_Layout_Size);
		}

		unmark(kLayout_Typesetting);

		return inner_size;
	}

	void Box::layout_text(TextLines *lines, TextConfig *cfg) {
		auto opts = cfg->opts();
		auto text_white_space = opts->text_white_space_value();
		//auto text_word_break = opts->text_word_break_value();
		bool is_auto_wrap = true;
		auto limitX = lines->host_size().x();
		auto origin = lines->pre_width();

		if (lines->no_wrap() || // 容器没有固定宽度
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
			set_layout_offset(Vec2());
			lines->set_pre_width(_layout_size.x());
		} else {
			set_layout_offset(Vec2(origin, 0));
			lines->finish_text_blob_pre();
			lines->set_pre_width(origin + _layout_size.x());
		}

		lines->add_view(this);
	}

	void Box::set_content_size(Vec2 content_size) {
		_content_size = content_size;
		_client_size = Vec2(content_size.x() + _padding_left + _padding_right,
												content_size.y() + _padding_top + _padding_bottom);
		if (_border) {
			_client_size.val[0] += _border->width[3] + _border->width[1]; // left + right
			_client_size.val[1] += _border->width[0] + _border->width[2]; // top + bottom
		}
		_layout_size = Vec2(_margin_left + _margin_right + _client_size.x(),
												_margin_top + _margin_bottom + _client_size.y());

		mark(kRecursive_Visible_Region, true);
	}

	Vec2 Box::layout_offset() {
		return _layout_offset;
	}

	View::Size Box::layout_size() {
		return {
			_layout_size, _content_size, _wrap_x, _wrap_y
		};
	}

	float Box::layout_weight() {
		return _weight;
	}

	Align Box::layout_align() {
		return _align;
	}

	void Box::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kRecursive_Transform) { // update transform matrix
			unmark(kRecursive_Transform | kRecursive_Visible_Region); // unmark
			Vec2 offset = layout_offset() + parent_Rt()->layout_offset_inside()
				+ Vec2(_margin_left, _margin_top);
			_position =
				mat.mul_vec2_no_translate(offset) + parent_Rt()->position();
			_visible_region = solve_visible_region(Mat(mat).set_translate(_position));
		} else if (mark & kRecursive_Visible_Region) {
			unmark(kRecursive_Visible_Region); // unmark
			_visible_region = solve_visible_region(Mat(mat).set_translate(_position));
		}
	}

	Vec2 Box::layout_offset_inside() {
		Vec2 offset(
			_padding_left, _padding_top
		);
		if (_border) {
			offset.val[0] += _border->width[3]; // left
			offset.val[1] += _border->width[0]; // top
		}
		return offset;
	}

	void Box::set_align(Align align, bool isRt) {
		if (_align != align) {
			_align = align;
			preRender().async_call([](auto self, auto arg) {
				if (self->parent_Rt()) {
					self->parent_Rt()->onChildLayoutChange(self, kChild_Layout_Align);
				}
			}, this, 0);
		}
	}

	void Box::set_weight(float weight, bool isRt) {
		if (_weight != weight) {
			_weight = weight;
			preRender().async_call([](auto self, auto arg) {
				if (self->parent_Rt()) {
					self->parent_Rt()->onChildLayoutChange(self, kChild_Layout_Weight);
				}
			}, this, 0);
		}
	}

	void Box::set_layout_offset(Vec2 val) {
		_layout_offset = val;
		mark(kRecursive_Transform, true); // mark recursive transform
	}

	void Box::set_layout_offset_free(Vec2 size) {
		Vec2 offset;

		switch(_align) {
			default:
			case Align::LeftTop: // left top
				break;
			case Align::CenterTop: // center top
				offset = Vec2((size.x() - _layout_size.x()) * .5, 0);
				break;
			case Align::RightTop: // right top
				offset = Vec2(size.x() - _layout_size.x(), 0);
				break;
			case Align::LeftCenter: // left center
				offset = Vec2(0, (size.y() - _layout_size.y()) * .5);
				break;
			case Align::CenterCenter: // center center
				offset = Vec2(
					(size.x() - _layout_size.x()) * .5,
					(size.y() - _layout_size.y()) * .5);
				break;
			case Align::RightCenter: // right center
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

	Vec2 Box::center() {
		Vec2 point(
			_client_size.x() * 0.5,
			_client_size.y() * 0.5
		);
		return point;
	}

	void Box::solve_rect_vertex(const Mat &mat, Vec2 vertex[4]) {
		Vec2 origin;
		Vec2 end = _client_size;
		vertex[0] = mat * origin;
		vertex[1] = mat * Vec2(end.x(), origin.y());
		vertex[2] = mat * end;
		vertex[3] = mat * Vec2(origin.x(), end.y());
	}

	bool Box::solve_visible_region(const Mat &mat) {
		solve_rect_vertex(mat, _vertex);
		/*
		* 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
		* 这种模糊测试在大多数时候都是正确有效的。
		*/
		auto& clip = window()->getClipRegion();
		auto  re   = screen_region_from_convex_quadrilateral(_vertex);

		if (Qk_MAX( clip.end.y(), re.end.y() ) - Qk_MIN( clip.origin.y(), re.origin.y() )
					<= re.end.y() - re.origin.y() + clip.size.y() &&
				Qk_MAX( clip.end.x(), re.end.x() ) - Qk_MIN( clip.origin.x(), re.origin.x() )
					<= re.end.x() - re.origin.x() + clip.size.x()
				) {
			return true;
		}

#if 0
		Qk_DEBUG("visible_region-x: %f<=%f", Qk_MAX( clip.y2, re.end.y() ) - Qk_MIN( clip.y, re.origin.y() ),
																				re.end.y() - re.origin.y() + clip.height);
		Qk_DEBUG("visible_region-y: %f<=%f", Qk_MAX( clip.x2, re.end.x() ) - Qk_MIN( clip.x, re.origin.x() ),
																				re.end.x() - re.origin.x() + clip.width);
#endif

		return false;
	}

	bool Box::overlap_test(Vec2 point) {
		return overlap_test_from_convex_quadrilateral(_vertex, point);
	}

	bool overlap_test_from_convex_quadrilateral(Vec2* quadrilateral_vertex, Vec2 point) {
		/*
		* 直线方程：(x-x1)(y2-y1)-(y-y1)(x2-x1)=0
		* 平面座标系中凸四边形内任一点是否存在：
		* [(x-x1)(y2-y1)-(y-y1)(x2-x1)][(x-x4)(y3-y4)-(y-y4)(x3-x4)] < 0  and
		* [(x-x2)(y3-y2)-(y-y2)(x3-x2)][(x-x1)(y4-y1)-(y-y1)(x4-x1)] < 0
		*/
		
		float x = point.x();
		float y = point.y();
		
		#define x1 quadrilateral_vertex[0].x()
		#define y1 quadrilateral_vertex[0].y()
		#define x2 quadrilateral_vertex[1].x()
		#define y2 quadrilateral_vertex[1].y()
		#define x3 quadrilateral_vertex[2].x()
		#define y3 quadrilateral_vertex[2].y()
		#define x4 quadrilateral_vertex[3].x()
		#define y4 quadrilateral_vertex[3].y()
		
		if (((x-x1)*(y2-y1)-(y-y1)*(x2-x1))*((x-x4)*(y3-y4)-(y-y4)*(x3-x4)) < 0 &&
				((x-x2)*(y3-y2)-(y-y2)*(x3-x2))*((x-x1)*(y4-y1)-(y-y1)*(x4-x1)) < 0
		) {
			return true;
		}
		
		#undef x1
		#undef y1
		#undef x2
		#undef y2
		#undef x3
		#undef y3
		#undef x4
		#undef y4
		
		return false;
	}

	Region screen_region_from_convex_quadrilateral(Vec2* quadrilateral_vertex) {
		#define A quadrilateral_vertex[0]
		#define B quadrilateral_vertex[1]
		#define C quadrilateral_vertex[2]
		#define D quadrilateral_vertex[3]
		
		Vec2 min, max;//, size;
		
		float w1 = fabs(A.x() - C.x());
		float w2 = fabs(B.x() - D.x());
		
		if (w1 > w2) {
			if ( A.x() > C.x() ) {
				max.set_x( A.x() ); min.set_x( C.x() );
			} else {
				max.set_x( C.x() ); min.set_x( A.x() );
			}
			if ( B.y() > D.y() ) {
				max.set_y( B.y() ); min.set_y( D.y() );
			} else {
				max.set_y( D.y() ); min.set_y( B.y() );
			}
			//size = Vec2(w1, max.y() - min.y());
		} else {
			if ( B.x() > D.x() ) {
				max.set_x( B.x() ); min.set_x( D.x() );
			} else {
				max.set_x( D.x() ); min.set_x( B.x() );
			}
			if ( A.y() > C.y() ) {
				max.set_y( A.y() ); min.set_y( C.y() );
			} else {
				max.set_y( C.y() ); min.set_y( A.y() );
			}
			//size = Vec2(w2, max.y() - min.y());
		}
		
		#undef A
		#undef B
		#undef C
		#undef D
			
		return {
			min, max
		};
	}

	bool Box::is_clip() {
		return _clip;
	}

	ViewType Box::viewType() const {
		return kBox_ViewType;
	}

}
