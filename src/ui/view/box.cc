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

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue
#define _Border() auto _border = this->_border.load()
#define _IfBorder() _Border(); if (_border)

namespace qk {

	Box::Box()
		: _clip(false)
		, _align(Align::Normal)
		, _min_width{0, BoxSizeKind::Auto}, _min_height{0, BoxSizeKind::Auto}
		, _max_width{0, BoxSizeKind::None}, _max_height{0, BoxSizeKind::None}
		, _margin_top(0), _margin_right(0)
		, _margin_bottom(0), _margin_left(0)
		, _padding_top(0), _padding_right(0)
		, _padding_bottom(0), _padding_left(0)
		, _border_radius_left_top(0), _border_radius_right_top(0)
		, _border_radius_right_bottom(0), _border_radius_left_bottom(0)
		, _background_color(Color::from(0))
		, _weight(0,0)
		, _container({{},{0,Float32::limit_max},{0,Float32::limit_max},kNone_FloatState,kNone_FloatState,false,false})
		, _background(nullptr)
		, _boxShadow(nullptr)
		, _border(nullptr)
	{
		//Qk_DLog("Box, %d", sizeof(Container));
	}

	Box::~Box() {
		Release(_background.load());
		Release(_boxShadow.load());
		free(_border.load());
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
			mark_layout(kLayout_Inner_Width, isRt);
		}
	}

	void Box::set_height(BoxSize val, bool isRt) {
		if (_min_height != val) {
			_min_height = val;
			mark_layout(kLayout_Inner_Height, isRt);
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
			mark_layout(kLayout_Inner_Width, isRt);
		}
	}

	void Box::set_max_height(BoxSize val, bool isRt) {
		if (_max_height != val) {
			_max_height = val;
			mark_layout(kLayout_Inner_Height, isRt);
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
			mark_layout(kLayout_Size_Height | kTransform, isRt);
		}
	}

	void Box::set_margin_left(float val, bool isRt) {
		if (_margin_left != val) {
			_margin_left = val;
			mark_layout(kLayout_Size_Width | kTransform, isRt);
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
		_Border();
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
		_Border();
		return _border ? BoxBorder{_border->width[0],_border->color[0]}: default_border;
	}

	BoxBorder Box::border_right() const {
		_Border();
		return _border ? BoxBorder{_border->width[1],_border->color[1]}: default_border;
	}

	BoxBorder Box::border_bottom() const {
		_Border();
		return _border ? BoxBorder{_border->width[2],_border->color[2]}: default_border;
	}

	BoxBorder Box::border_left() const {
		_Border();
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
		_Border();
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
		_Border();
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
		_Border();
		return _border ? _border->color[0]: Color::from(0);
	}

	Color Box::border_color_right() const {
		_Border();
		return _border ? _border->color[1]: Color::from(0);
	}

	Color Box::border_color_bottom() const {
		_Border();
		return _border ? _border->color[2]: Color::from(0);
	}

	Color Box::border_color_left() const {
		_Border();
		return _border ? _border->color[3]: Color::from(0);
	}

	float Box::border_width_top() const {
		_Border();
		return _border ? _border->width[0]: 0;
	}

	float Box::border_width_right() const {
		_Border();
		return _border ? _border->width[1]: 0;
	}

	float Box::border_width_bottom() const {
		_Border();
		return _border ? _border->width[2]: 0;
	}

	float Box::border_width_left() const {
		_Border();
		return _border ? _border->width[3]: 0;
	}

	struct SetBorder: public Box {
		#define _BorderAlloc() auto _border = static_cast<SetBorder*>(this)->alloc()
		BoxBorderInl* alloc() {
			auto border = _border.load();
			if (!border) {
				border = (BoxBorderInl*)malloc(sizeof(BoxBorderInl));
				memset(border, 0, sizeof(BoxBorderInl));
				_border.store(border);
			}
			return border;
		}
	};

	void Box::set_border_color_top(Color val, bool isRt) {
		_BorderAlloc();
		if (_border->color[0] != val) {
			_border->color[0] = val;
			mark(kLayout_None, isRt);
		}
	}

	void Box::set_border_color_right(Color val, bool isRt) {
		_BorderAlloc();
		if (_border->color[1] != val) {
			_border->color[1] = val;
			mark(kLayout_None, isRt);
		}
	}

	void Box::set_border_color_bottom(Color val, bool isRt) {
		_BorderAlloc();
		if (_border->color[2] != val) {
			_border->color[2] = val;
			mark(kLayout_None, isRt);
		}
	}

	void Box::set_border_color_left(Color val, bool isRt) {
		_BorderAlloc();
		if (_border->color[3] != val) {
			_border->color[3] = val;
			mark(kLayout_None, isRt);
		}
	}

	void Box::set_border_width_top(float val, bool isRt) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[0] != val) {
			_border->width[0] = val;
			mark_layout(kLayout_Size_Height, isRt);
		}
	}

	void Box::set_border_width_right(float val, bool isRt) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[1] != val) {
			_border->width[1] = val;
			mark_layout(kLayout_Size_Width, isRt);
		}
	}

	void Box::set_border_width_bottom(float val, bool isRt) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[2] != val) {
			_border->width[2] = val;
			mark_layout(kLayout_Size_Height, isRt);
		}
	}

	void Box::set_border_width_left(float val, bool isRt) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[3] != val) {
			_border->width[3] = val;
			mark_layout(kLayout_Size_Width, isRt);
		}
	}

	void Box::set_background_color(Color color, bool isRt) {
		if (_background_color != color) {
			_background_color = color;
			mark(kLayout_None,isRt);
		}
	}

	BoxFilter* Box::background() {
		return _background.load();
	}

	BoxShadow* Box::box_shadow() {
		return _boxShadow.load();
	}

	Vec2 Box::content_size() const {
		return _container.content;
	}

	void Box::set_background(BoxFilter* val, bool isRt) {
		auto filter = _background.load();
		auto newFilter = BoxFilter::assign(filter, val, this, isRt);
		if (filter != newFilter) {
			_background.store(newFilter);
		}
	}

	void Box::set_box_shadow(BoxShadow* val, bool isRt) {
		auto filter = _boxShadow.load();
		auto newFilter = static_cast<BoxShadow*>(BoxFilter::assign(filter, val, this, isRt));
		if (filter != newFilter) {
			_boxShadow.store(newFilter);
		}
	}

	void Box::set_content_size(Vec2 size) {
		if (size == _container.content)
			return;
		_container.content = size;
		_client_size = Vec2(size.x() + _padding_left + _padding_right,
												size.y() + _padding_top + _padding_bottom);
		_IfBorder() {
			_client_size.val[0] += _border->width[3] + _border->width[1]; // left + right
			_client_size.val[1] += _border->width[0] + _border->width[2]; // top + bottom
		}
		_layout_size = Vec2(_margin_left + _margin_right + _client_size.x(),
												_margin_top + _margin_bottom + _client_size.y());

		_IfParent() {
			_parent->onChildLayoutChange(this, kChild_Layout_Size);
		}
		mark(kVisible_Region, true);
	}

	Vec2 Box::layout_offset() {
		return _layout_offset;
	}

	Vec2 Box::layout_size() {
		return _layout_size;
	}

	const View::Container& Box::layout_container() {
		return _container;
	}

	Vec2 Box::layout_weight() {
		return _weight;
	}

	Align Box::layout_align() {
		return _align;
	}

	void Box::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			_CheckParent();
			unmark(kTransform | kVisible_Region); // unmark
			Vec2 point = _parent->layout_offset_inside() + layout_offset()
				+ Vec2(_margin_left, _margin_top);
			_position =
				mat.mul_vec2_no_translate(point) + _parent->position();
			solve_visible_region(Mat(mat).set_translate(_position));
		} else if (mark & kVisible_Region) {
			unmark(kVisible_Region); // unmark
			solve_visible_region(Mat(mat).set_translate(_position));
		}
	}

	Vec2 Box::layout_offset_inside() {
		Vec2 offset(
			_padding_left, _padding_top
		);
		_IfBorder() {
			offset.val[0] += _border->width[3]; // left
			offset.val[1] += _border->width[0]; // top
		}
		return offset;
	}

	void Box::set_align(Align align, bool isRt) {
		if (_align != align) {
			_align = align;
			preRender().async_call([](auto self, auto arg) {
				auto _parent = self->parent();
				if (_parent)
					_parent->onChildLayoutChange(self, kChild_Layout_Align);
			}, this, 0);
		}
	}

	void Box::set_weight(Vec2 weight, bool isRt) {
		if (_weight != weight) {
			_weight = weight;
			preRender().async_call([](auto self, auto arg) {
				auto _parent = self->parent();
				if (_parent)
					_parent->onChildLayoutChange(self, kChild_Layout_Weight);
			}, this, 0);
		}
	}

	void Box::set_layout_offset(Vec2 val) {
		_layout_offset = val;
		mark(kTransform, true); // mark recursive transform
	}

	Vec2 Box::center() {
		Vec2 point(
			_client_size.x() * 0.5,
			_client_size.y() * 0.5
		);
		return point;
	}

	void Box::solve_rect_vertex(const Mat &mat, Vec2 vertex[4]) {
		Vec2 end = _client_size;
		vertex[0] = Vec2{mat[2], mat[5]}; // left,top
		vertex[1] = mat * Vec2(end.x(), 0); // right,top
		vertex[2] = mat * end; // right,bottom
		vertex[3] = mat * Vec2(0, end.y()); // left,bottom
	}

	void Box::solve_visible_region(const Mat &mat) {
		solve_rect_vertex(mat, _vertex);
		/*
		* 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
		* 这种模糊测试在大多数时候都是正确有效的。
		*/
		auto& clip = window()->getClipRegion();
		auto  re   = screen_region_from_convex_quadrilateral(_vertex);

		if (Qk_Max( clip.end.y(), re.end.y() ) - Qk_Min( clip.origin.y(), re.origin.y() )
					<= re.end.y() - re.origin.y() + clip.size.y() &&
				Qk_Max( clip.end.x(), re.end.x() ) - Qk_Min( clip.origin.x(), re.origin.x() )
					<= re.end.x() - re.origin.x() + clip.size.x()
				) {
			//_visible_region = !_client_size.is_zero_axis();
			_visible_region = true;
		} else {

#if 0
			Qk_DLog("visible_region-x: %f<=%f", Qk_Max( clip.y2, re.end.y() ) - Qk_Min( clip.y, re.origin.y() ),
																				re.end.y() - re.origin.y() + clip.height);
			Qk_DLog("visible_region-y: %f<=%f", Qk_Max( clip.x2, re.end.x() ) - Qk_Min( clip.x, re.origin.x() ),
																				re.end.x() - re.origin.x() + clip.width);
#endif
			_visible_region = false;
		}
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
