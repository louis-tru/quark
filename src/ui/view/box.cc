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
#include "../geometry.h"

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue
#define _Border() auto _border = this->_border.load()
#define _IfBorder() _Border(); if (_border)

namespace qk {

	Box::Box()
		: _clip(false)
		, _free(false)
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
		, _container({{},{},{0,Float32::limit_max},{0,Float32::limit_max},kNone_FloatState,kNone_FloatState,false,false})
		, _background(nullptr)
		, _box_shadow(nullptr)
		, _border(nullptr)
	{
		// Qk_DLog("Box, %d", sizeof(Box));
	}

	Box::~Box() {
		Releasep(_background);
		Releasep(_box_shadow);
		::free(_border.load());
	}

	void Box::set_free(bool val, bool isRt) {
		if (_free != val) {
			_free = val;
			mark_layout(kLayout_Typesetting, isRt);
		}
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
			// may affect the inside content height,
			// so mark kLayout_Inner_Height with kLayout_Outside_Width together
			mark_layout(kLayout_Size_Height/*| kTransform*/, isRt);
		}
	}

	void Box::set_padding_left(float val, bool isRt) {
		if (_padding_left != val) {
			_padding_left = val;
			mark_layout(kLayout_Size_Width/*| kTransform*/, isRt);
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

	ArrayFloat Box::border_radius() const {
		return ArrayFloat{
			_border_radius_left_top,     _border_radius_right_top,
			_border_radius_right_bottom, _border_radius_left_bottom
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

	static Border     default_border{0,Color::from(0)};
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

	Border Box::border_top() const {
		_Border();
		return _border ? Border{_border->width[0],_border->color[0]}: default_border;
	}

	Border Box::border_right() const {
		_Border();
		return _border ? Border{_border->width[1],_border->color[1]}: default_border;
	}

	Border Box::border_bottom() const {
		_Border();
		return _border ? Border{_border->width[2],_border->color[2]}: default_border;
	}

	Border Box::border_left() const {
		_Border();
		return _border ? Border{_border->width[3],_border->color[3]}: default_border;
	}

	void Box::set_border_top(Border border, bool isRt) {
		set_border_width_top(border.width, isRt);
		set_border_color_top(border.color, isRt);
	}

	void Box::set_border_right(Border border, bool isRt) {
		set_border_width_right(border.width, isRt);
		set_border_color_right(border.color, isRt);
	}

	void Box::set_border_bottom(Border border, bool isRt) {
		set_border_width_bottom(border.width, isRt);
		set_border_color_bottom(border.color, isRt);
	}

	void Box::set_border_left(Border border, bool isRt) {
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
		BorderInl* alloc() {
			auto border = _border.load();
			if (!border) {
				border = (BorderInl*)malloc(sizeof(BorderInl));
				memset(border, 0, sizeof(BorderInl));
				if (_border) {
					::free(border);
				} else {
					_border.store(border);
				}
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
			mark_layout(kLayout_Size_Height/*| kTransform*/, isRt);
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
			mark_layout(kLayout_Size_Width/*| kTransform*/, isRt);
		}
	}

	void Box::set_background_color(Color color, bool isRt) {
		if (_background_color != color) {
			_background_color = color;
			mark(kLayout_None,isRt);
		}
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
		auto filter = _box_shadow.load();
		auto newFilter = static_cast<BoxShadow*>(BoxFilter::assign(filter, val, this, isRt));
		if (filter != newFilter) {
			_box_shadow.store(newFilter);
		}
	}

	void Box::set_content_size(Vec2 size) {
		bool change = false;

		if (size[0] != _container.content[0]) {
			change = true;
			_container.content[0] = size[0];
			_container.content_diff_before_locking[0] = 0;
			_client_size[0] = size[0] + _padding_left + _padding_right;
			_IfBorder() {
				_client_size[0] += _border->width[3] + _border->width[1]; // left + right
			}
			_layout_size[0] = _margin_left + _margin_right + _client_size[0];
		}

		if (size[1] != _container.content[1]) {
			change = true;
			_container.content[1] = size[1];
			_container.content_diff_before_locking[1] = 0;
			_client_size[1] = size[1] + _padding_top + _padding_bottom;
			_IfBorder() {
				_client_size[1] += _border->width[0] + _border->width[2]; // top + bottom
			}
			_layout_size[1] = _margin_top + _margin_bottom + _client_size[1];
		}

		if (change) {
			_IfParent() {
				_parent->onChildLayoutChange(this, kChild_Layout_Size);
			}
			mark(kVisible_Region, true);
		}
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

	void Box::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			unmark(kTransform | kVisible_Region); // unmark
			auto v = parent->layout_offset_inside() + layout_offset() + Vec2(_margin_left, _margin_top);
			_position =
				mat.mul_vec2_no_translate(v) + parent->position(); // the left-top world coords
			solve_visible_area(Mat(mat).set_translate(_position));
		} else if (mark & kVisible_Region) {
			unmark(kVisible_Region); // unmark
			solve_visible_area(Mat(mat).set_translate(_position));
		}
	}

	void Box::solve_visible_area(const Mat &mat) {
		Vec2 size = _client_size;
		_boxBounds[0] = Vec2{mat[2], mat[5]}; // left,top
		_boxBounds[1] = mat * Vec2(size.x(), 0); // right,top
		_boxBounds[2] = mat * size; // right,bottom
		_boxBounds[3] = mat * Vec2(0, size.y()); // left,bottom
		_visible_area = compute_visible_area(mat, _boxBounds);
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
			if (isRt) {
				auto _parent = parent();
				if (_parent)
					_parent->onChildLayoutChange(this, kChild_Layout_Align);
			} else {
				preRender().async_call([](auto self, auto arg) {
					auto _parent = self->parent();
					if (_parent)
						_parent->onChildLayoutChange(self, kChild_Layout_Align);
				}, this, 0);
			}
		}
	}

	void Box::set_weight(Vec2 weight, bool isRt) {
		if (_weight != weight) {
			_weight = weight;
			if (isRt) {
				auto _parent = parent();
				if (_parent)
					_parent->onChildLayoutChange(this, kChild_Layout_Weight);
			} else {
				preRender().async_call([](auto self, auto arg) {
					auto _parent = self->parent();
					if (_parent)
						_parent->onChildLayoutChange(self, kChild_Layout_Weight);
				}, this, 0);
			}
		}
	}

	void Box::set_layout_offset(Vec2 val) {
		_layout_offset = val;
		mark(kTransform, true); // mark recursive transform
	}

	Vec2 Box::client_size() {
		return _client_size;
	}

	Region Box::client_region() {
		return Region{0, _client_size, _layout_offset};
	}

	bool Box::overlap_test(Vec2 point) {
		return test_overlap_from_convex_quadrilateral(_boxBounds, point);
	}

	bool Box::is_clip() {
		return _clip;
	}

	ViewType Box::viewType() const {
		return kBox_ViewType;
	}

}
