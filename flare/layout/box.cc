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

#include "./box.h"
#include "../app.h"
#include "../display.h"
#include "../render/render.h"

F_NAMESPACE_START


float Box::solve_layout_content_width(Size &parent_layout_size) {
	float ps = parent_layout_size.content_size.x();
	bool* is_wrap_in_out = &parent_layout_size.wrap_x;
	float result;

	switch (_width.type) {
		default: // NONE /* none default wrap content */
		case BoxSizeType::WRAP: /* 包裹内容 wrap content */
			*is_wrap_in_out = true;
			result = 0; // invalid wrap width
			break;
		case BoxSizeType::PIXEL: /* 明确值 value px */
			*is_wrap_in_out = false;
			result = _width.value;
			break;
		case BoxSizeType::MATCH: /* 匹配父视图 match parent */
			if (*is_wrap_in_out) {
				result = 0; // invalid wrap width
			} else { // use wrap
				result = Number<float>::max(
					ps - _margin_left - _margin_right - _padding_left - _padding_right, 0
				);
			}
			// *is_wrap_in_out = *is_wrap_in_out;
			break;
		case BoxSizeType::RATIO: /* 百分比 value % */
			if (*is_wrap_in_out) {
				result = 0; // invalid wrap width
			} else { // use wrap
				result = Number<float>::max(ps * _width.value, 0);
			}
			// *is_wrap_in_out = *is_wrap_in_out;
			break;
		case BoxSizeType::MINUS: /* 减法(parent-value) value ! */
			if (*is_wrap_in_out) {
				result = 0; // invalid wrap width
			} else { // use wrap
				result = Number<float>::max(ps - _width.value, 0);
			}
			// *is_wrap_in_out = *is_wrap_in_out;
			break;
	}
	return result;
}

float Box::solve_layout_content_height(Size &parent_layout_size) {
	float ps = parent_layout_size.content_size.y();
	bool* is_wrap_in_out = &parent_layout_size.wrap_y;
	float result;

	switch (_height.type) {
		default: // NONE /* none default wrap content */
		case BoxSizeType::WRAP: /* 包裹内容 wrap content */
			*is_wrap_in_out = true;
			result = 0; // invalid wrap height
			break;
		case BoxSizeType::PIXEL: /* 明确值 value px */
			*is_wrap_in_out = false;
			result = _height.value;
			break;
		case BoxSizeType::MATCH: /* 匹配父视图 match parent */
			if (*is_wrap_in_out) {
				result = 0; // invalid wrap height
			} else { // use wrap
				result = Number<float>::max(
					ps - _margin_top - _margin_bottom - _padding_top - _padding_bottom, 0
				);
			}
			// *is_wrap_in_out = *is_wrap_in_out;
			break;
		case BoxSizeType::RATIO: /* 百分比 value % */
			if (*is_wrap_in_out) {
				result = 0; // invalid wrap height
			} else { // use wrap
				result = Number<float>::max(ps * _height.value, 0);
			}
			// *is_wrap_in_out = *is_wrap_in_out;
			break;
		case BoxSizeType::MINUS: /* 减法(parent-value) value ! */
			if (*is_wrap_in_out) {
				result = 0; // invalid wrap height
			} else { // use wrap
				result = Number<float>::max(ps - _height.value, 0);
			}
			// *is_wrap_in_out = *is_wrap_in_out;
			break;
	}
	return result;
}

void Box::mark_layout_size(uint32_t mark_) {
	auto paren = parent();
	if (paren) {
		if (paren->is_layout_lock_child()) {
			paren->layout_typesetting_change(this);
		} else {
			mark(mark_);
		}
	}
}

/**
	* @constructors
	*/
Box::Box()
	: _layout_wrap_x(true), _layout_wrap_y(true), _is_radius(false), _is_clip(false)
	, _width_limit{0, BoxSizeType::NONE}, _height_limit{0, BoxSizeType::NONE}
	, _margin_top(0), _margin_right(0)
	, _margin_bottom(0), _margin_left(0)
	, _padding_top(0), _padding_right(0)
	, _padding_bottom(0), _padding_left(0)
	, _radius_left_top(0), _radius_right_top(0)
	, _radius_right_bottom(0), _radius_left_bottom(0)
	, _fill_color(Color::from(0))
	, _fill(nullptr)
	, _effect(nullptr)
	, _layout_weight(0), _layout_align(Align::AUTO)
	, _border(nullptr)
{
}

Box::~Box() {
	Release(_fill); _fill = nullptr;
	::free(_border); _border = nullptr;
}

// is clip box display range
void Box::set_is_clip(bool val) {
	if (_is_clip != val) {
		_is_clip = val;
		mark_none();
	}
}

/**
	*
	* 设置宽度
	*
	* @func set_width(width)
	*/
void Box::set_width(BoxSize val) {
	if (_width != val) {
		_width = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
	}
}

/**
	*
	* 设置高度
	*
	* @func set_height(height)
	*/
void Box::set_height(BoxSize val) {
	if (_height != val) {
		_height = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
	}
}

/**
	*
	* 设置最大宽度限制
	*
	* @func set_width_limit(width_limit)
	*/
void Box::set_width_limit(BoxSize val) {
	if (_width_limit != val) {
		_width_limit = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
	}
}

/**
	*
	* 设置最大高度限制
	*
	* @func set_height_limit(height_limit)
	*/
void Box::set_height_limit(BoxSize val) {
	if (_height_limit != val) {
		_height_limit = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
	}
}

void Box::set_margin_top(float val) { // margin
	if (_margin_top != val) {
		_margin_top = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
		mark_recursive(M_TRANSFORM);
	}
}

void Box::set_margin_left(float val) {
	if (_margin_left != val) {
		_margin_left = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
		mark_recursive(M_TRANSFORM);
	}
}

void Box::set_padding_top(float val) { // padding
	if (_padding_top != val) {
		_padding_top = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
		// 没有直接的影响到`transform`但可能导致`layout_size`变化导致
		// `transform_origin`百分比属性变化,间接影响`transform`变化, 但可以肯定这个会影响子布局偏移
		// mark_recursive(M_TRANSFORM); 
	}
}

void Box::set_padding_left(float val) {
	if (_padding_left != val) {
		_padding_left = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
		//mark_recursive(M_TRANSFORM); // @`set_padding_top(val)`
	}
}

// --
void Box::set_margin_right(float val) {
	if (_margin_right != val) {
		_margin_right = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
		//mark_recursive(M_TRANSFORM); // @`set_padding_top(val)`
	}
}

void Box::set_margin_bottom(float val) {
	if (_margin_bottom != val) {
		_margin_bottom = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
		//mark_recursive(M_TRANSFORM); // @`set_padding_top(val)`
	}
}

void Box::set_padding_right(float val) {
	if (_padding_right != val) {
		_padding_right = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
		//mark_recursive(M_TRANSFORM); // @`set_padding_top(val)`
	}
}

void Box::set_padding_bottom(float val) {
	if (_padding_bottom != val) {
		_padding_bottom = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
		//mark_recursive(M_TRANSFORM); // @`set_padding_top(val)`
	}
}

// -- border radius

void Box::set_radius_left_top(float val) {
	if (val >= 0.0 && _radius_left_top != val) {
		_radius_left_top = val;
		_is_radius = 
			*reinterpret_cast<uint64_t*>(&_radius_left_top) | 
			*reinterpret_cast<uint64_t*>(&_radius_right_bottom);
		mark_none();
	}
}

void Box::set_radius_right_top(float val) {
	if (val >= 0.0 && _radius_right_top != val) {
		_radius_right_top = val;
		_is_radius = 
			*reinterpret_cast<uint64_t*>(&_radius_left_top) | 
			*reinterpret_cast<uint64_t*>(&_radius_right_bottom);
		mark_none();
	}
}

void Box::set_radius_right_bottom(float val) {
	if (val >= 0.0 && _radius_right_bottom != val) {
		_radius_right_bottom = val;
		_is_radius = 
			*reinterpret_cast<uint64_t*>(&_radius_left_top) | 
			*reinterpret_cast<uint64_t*>(&_radius_right_bottom);
		mark_none();
	}
}

void Box::set_radius_left_bottom(float val) {
	if (val >= 0.0 && _radius_left_bottom != val) {
		_radius_left_bottom = val;
		_is_radius = 
			*reinterpret_cast<uint64_t*>(&_radius_left_top) | 
			*reinterpret_cast<uint64_t*>(&_radius_right_bottom);
		mark_none();
	}
}

Color Box::border_color_top() const {
	return _border ? _border->color_top: Color::from(0);
}

Color Box::border_color_right() const {
	return _border ? _border->color_right: Color::from(0);
}

Color Box::border_color_bottom() const {
	return _border ? _border->color_bottom: Color::from(0);
}

Color Box::border_color_left() const {
	return _border ? _border->color_left: Color::from(0);
}

float Box::border_width_top() const {
	return _border ? _border->width_top: 0;
} // border_widrh

float Box::border_width_right() const {
	return _border ? _border->width_right: 0;
}

float Box::border_width_bottom() const {
	return _border ? _border->width_bottom: 0;
}

float Box::border_width_left() const {
	return _border ? _border->width_left: 0;
}

BorderStyle Box::border_style_top() const {
	return _border ? _border->style_top: BorderStyle::SOLID;
} // border_style

BorderStyle Box::border_style_right() const {
	return _border ? _border->style_right: BorderStyle::SOLID;
}

BorderStyle Box::border_style_bottom() const {
	return _border ? _border->style_bottom: BorderStyle::SOLID;
}

BorderStyle Box::border_style_left() const {
	return _border ? _border->style_left: BorderStyle::SOLID;
}

// set border

void Box::alloc_border() {
	if (!_border) {
		_border = (Border*)::malloc(sizeof(Border));
		::memset(_border, 0, sizeof(Border));
	}
}

void Box::set_border_color_top(Color val) {
	alloc_border();
	if (_border->color_top != val) {
		_border->color_top = val;
		mark_none();
	}
}

void Box::set_border_color_right(Color val) {
	alloc_border();
	if (_border->color_right != val) {
		_border->color_right = val;
		mark_none();
	}
}

void Box::set_border_color_bottom(Color val) {
	alloc_border();
	if (_border->color_bottom != val) {
		_border->color_bottom = val;
		mark_none();
	}
}

void Box::set_border_color_left(Color val) {
	alloc_border();
	if (_border->color_top != val) {
		_border->color_top = val;
		mark_none();
	}
}

void Box::set_border_width_top(float val) {
	alloc_border();
	if (_border->width_top != val) {
		_border->width_top = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
	}
} // border_widrh

void Box::set_border_width_right(float val) {
	alloc_border();
	if (_border->width_right != val) {
		_border->width_right = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
	}
}

void Box::set_border_width_bottom(float val) {
	alloc_border();
	if (_border->width_bottom != val) {
		_border->width_bottom = val;
		mark_layout_size(M_LAYOUT_SIZE_HEIGHT);
	}
}

void Box::set_border_width_left(float val) {
	alloc_border();
	if (_border->width_left != val) {
		_border->width_left = val;
		mark_layout_size(M_LAYOUT_SIZE_WIDTH);
	}
}

void Box::set_border_style_top(BorderStyle val) {
	alloc_border();
	if (_border->style_top != val) {
		_border->style_top = val;
		mark_none();
	}
} // border_style

void Box::set_border_style_right(BorderStyle val) {
	alloc_border();
	if (_border->style_right != val) {
		_border->style_right = val;
		mark_none();
	}
}

void Box::set_border_style_bottom(BorderStyle val) {
	alloc_border();
	if (_border->style_bottom != val) {
		_border->style_bottom = val;
		mark_none();
	}
}

void Box::set_border_style_left(BorderStyle val) {
	alloc_border();
	if (_border->style_left != val) {
		_border->style_left = val;
		mark_none();
	}
}


// -- paint

void Box::set_fill_color(Color color) {
	if (_fill_color != color) {
		_fill_color = color;
		mark_none();
	}
}

void Box::set_fill(Fill* val) {
	if (_fill != val) {
		_fill = static_cast<Fill*>(Effect::assign(_fill, val));
		mark_none();
	}
}

void Box::set_effect(Effect* val) {
	if (_effect != val) {
		_effect = Effect::assign(_effect, val);
		mark_none();
	}
}

/**
	* @func solve_layout_size()
	*/
uint32_t Box::solve_layout_size(uint32_t mark) {
	uint32_t layout_content_size_change_mark = M_NONE;

	if (mark & M_LAYOUT_SIZE_WIDTH) {
		if (!parent()->is_layout_lock_child()) {
			auto size = parent()->layout_size();
			auto val = solve_layout_content_width(size);

			if (val != _layout_content_size.x() || _layout_wrap_x != size.wrap_x) {
				_layout_content_size.set_x(val);
				_layout_wrap_x = size.wrap_x;
				// mark(M_LAYOUT_TYPESETTING);
				layout_content_size_change_mark = M_LAYOUT_SIZE_WIDTH;
			}
			_client_size.set_x(_padding_left + _padding_right + val);
			_layout_size.set_x(_margin_left + _margin_right + _client_size.x());
		} // else The layout is locked and does not need to be updated
		parent()->layout_typesetting_change(this);
		unmark(M_LAYOUT_SIZE_WIDTH);
	}

	if (mark & M_LAYOUT_SIZE_HEIGHT) {
		if (!parent()->is_layout_lock_child()) {
			auto size = parent()->layout_size();
			auto val = solve_layout_content_height(size);

			if (val != _layout_content_size.y() || _layout_wrap_y != size.wrap_y) {
				_layout_content_size.set_y(val);
				_layout_wrap_y = size.wrap_y;
				layout_content_size_change_mark |= M_LAYOUT_SIZE_HEIGHT;
			}
			_client_size.set_y(_padding_top + _padding_bottom + val);
			_layout_size.set_y(_margin_top + _margin_bottom + _client_size.y());
		} // else The layout is locked and does not need to be updated
		parent()->layout_typesetting_change(this);
		unmark(M_LAYOUT_SIZE_HEIGHT);
	}

	return layout_content_size_change_mark;
}

bool Box::layout_forward(uint32_t _mark) {
	uint32_t layout_content_size_change_mark = solve_layout_size(_mark);

	if (layout_content_size_change_mark) {
		auto v = first();
		while (v) {
			v->layout_content_size_change(this, layout_content_size_change_mark);
			v = v->next();
		}
		mark(M_LAYOUT_TYPESETTING); // rearrange
		mark_recursive(M_LAYOUT_SHAPE);
		// TODO check transform_origin change ...
	}

	return (layout_mark() & M_LAYOUT_TYPESETTING);
}

bool Box::layout_reverse(uint32_t mark) {
	if (mark & (M_LAYOUT_TYPESETTING)) {
		if (!is_ready_layout_typesetting()) {
			return true; // continue iteration
		}
		auto v = first();
		Vec2 origin(_margin_left + _padding_left, _margin_top + _padding_top);
		Vec2 size = _layout_content_size;
		while (v) {
			v->set_layout_offset_lazy(origin, size); // lazy layout
			v = v->next();
		}
		unmark(M_LAYOUT_TYPESETTING);
	}
	return false; // stop iteration
}

Vec2 Box::layout_offset() {
	return _layout_offset;
}

Layout::Size Box::layout_size() {
	return {
		_layout_size, _layout_content_size, _layout_wrap_x, _layout_wrap_y
	};
}

Layout::Size Box::layout_raw_size(Size size) {
	size.content_size.set_x(solve_layout_content_width(size));
	size.content_size.set_x(solve_layout_content_height(size));
	size.layout_size.set_x(_margin_left + _margin_right + size.content_size.x() + _padding_left + _padding_right);
	size.layout_size.set_y(_margin_top + _margin_bottom + size.content_size.y() + _padding_top + _padding_bottom);
	return size;
}

float Box::layout_weight() {
	return _layout_weight;
}

Align Box::layout_align() {
	return _layout_align;
}

Mat Box::layout_matrix() {
	Vec2 in = _parent ? _parent->layout_offset_inside(): Vec2();
	if (_transform) {
		return Mat(
			layout_offset() + Vec2(_margin_left, _margin_top) +
								_transform_origin + _transform->translate - in, // translate
			_transform->scale,
			-_transform->rotate,
			_transform->skew
		);
	} else {
		Vec2 translate = layout_offset() +
		Vec2(_margin_left, _margin_top) + _transform_origin - in;
		return Mat(
			1, 0, translate.x(),
			0, 1, translate.y()
		);
	}
}

Vec2 Box::layout_offset_inside() {
	return Vec2(_margin_left, _margin_top) + _transform_origin;
}

/**
	*
	* 设置布局对齐方式
	*
	* @func set_layout_align(align)
	*/
void Box::set_layout_align(Align align) {
	if (_layout_align != align) {
		_layout_align = align;
		if (parent()) {
			parent()->layout_typesetting_change(this);
		}
	}
}

/**
	*
	* 设置布局权重
	*
	* @func set_layout_weight(weight)
	*/
void Box::set_layout_weight(float weight) {
	if (_layout_weight != weight) {
		_layout_weight = weight;
		if (parent()) {
			parent()->layout_typesetting_change(this, T_CHILD_WEIGHT);
		}
	}
}

/**
	* 锁定布局的尺寸。在特定的布局类型中自身受父布局约束无法直接简单确定其自身尺寸，一般由父布局调用如：flex布局类型
	*
	* 这个方法应该在`layout_forward()`正向迭代中由父布局调用,因为尺寸的调整一般在正向迭代中
	* 
	* 返回锁定后的最终尺寸，调用后视返回后的尺寸为最终尺寸
	* 
	* @func layout_lock(layout_size, is_wrap)
	*/
Vec2 Box::layout_lock(Vec2 layout_size, bool is_wrap[2]) {
	uint32_t layout_content_size_change_mark = M_NONE;
	auto layout_content_size = _layout_content_size;

	auto m_x = _margin_left + _margin_right;
	auto m_y = _margin_top + _margin_bottom;
	auto p_x = _padding_left + _padding_right;
	auto p_y = _padding_left + _padding_right;
	auto mp_x = m_x + p_x;
	auto mp_y = m_y + p_y;

	_layout_content_size = Vec2(
		layout_size.x() > mp_x ? layout_size.x() - mp_x: 0,
		layout_size.y() > mp_y ? layout_size.y() - mp_y: 0
	);
	_client_size = Vec2(p_x + _layout_content_size.x(), p_y + _layout_content_size.y());
	_layout_size = Vec2(mp_x + _layout_content_size.x(), mp_y + _layout_content_size.y());

	if (layout_content_size.x() != _layout_content_size.x() || _layout_wrap_x != is_wrap[0]) {
		layout_content_size_change_mark = M_LAYOUT_SIZE_WIDTH;
	}
	if (layout_content_size.y() != _layout_content_size.y() || _layout_wrap_y != is_wrap[1]) {
		layout_content_size_change_mark |= M_LAYOUT_SIZE_HEIGHT;
	}

	_layout_wrap_x = is_wrap[0];
	_layout_wrap_y = is_wrap[1];

	if (layout_content_size_change_mark)  {
		if (!is_layout_lock_child()) {
			auto v = first();
			while (v) {
				v->layout_content_size_change(this, layout_content_size_change_mark);
				v = v->next();
			}
		}
		mark(M_LAYOUT_TYPESETTING); // rearrange
		mark_recursive(M_LAYOUT_SHAPE);
	}

	unmark(M_LAYOUT_SIZE_WIDTH | M_LAYOUT_SIZE_HEIGHT);

	return _layout_size;
}

/**
	* @func set_layout_size(layout_content_size)
	*/
void Box::set_layout_size(Vec2 layout_content_size) {
	_layout_content_size = layout_content_size;
	_client_size = Vec2(layout_content_size.x() + _padding_left + _padding_right, layout_content_size.y() + _padding_top + _padding_bottom);
	_layout_size = Vec2(_margin_left + _margin_right + _client_size.x(), _margin_top + _margin_bottom + _client_size.y());
}

void Box::set_layout_offset(Vec2 val) {
	if (val != _layout_offset) {
		_layout_offset = val;
		mark_recursive(M_TRANSFORM); // mark recursive transform
	}
}

void Box::set_layout_offset_lazy(Vec2 origin, Vec2 size) {
	Vec2 offset;

	switch(_layout_align) {
		default:
		case Align::LEFT_TOP: // left top
			offset = origin;
			break;
		case Align::CENTER_TOP: // center top
			offset = Vec2(
				origin.x() + (size.x() - _layout_size.x()) / 2.0,
				origin.y());
			break;
		case Align::RIGHT_TOP: // right top
			offset = Vec2(
				origin.x() + size.x() - _layout_size.x(),
				origin.y());
			break;
		case Align::START:
		case Align::LEFT_CENTER: // left center
			offset = Vec2(
				origin.x(),
				origin.y() + (size.y() - _layout_size.y()) / 2.0);
			break;
		case Align::CENTER:
		case Align::CENTER_CENTER: // center center
			offset = Vec2(
				origin.x() + (size.x() - _layout_size.x()) / 2.0,
				origin.y() + (size.y() - _layout_size.y()) / 2.0);
			break;
		case Align::END:
		case Align::RIGHT_CENTER: // right center
			offset = Vec2(
				origin.x() + (size.x() - _layout_size.x()),
				origin.y() + (size.y() - _layout_size.y()) / 2.0);
			break;
		case Align::LEFT_BOTTOM: // left bottom
			offset = Vec2(
				origin.x(),
				origin.y() + (size.y() - _layout_size.y()));
			break;
		case Align::CENTER_BOTTOM: // center bottom
			offset = Vec2(
				origin.x() + (size.x() - _layout_size.x()) / 2.0,
				origin.y() + (size.y() - _layout_size.y()));
			break;
		case Align::RIGHT_BOTTOM: // right bottom
			offset = Vec2(
				origin.x() + (size.x() - _layout_size.x()),
				origin.y() + (size.y() - _layout_size.y()));
			break;
	}
	set_layout_offset(offset);
}

void Box::layout_content_size_change(Layout* parent, uint32_t mark_) {
	mark(mark_);
}

/**
	* 
	* is ready layout layout typesetting in the `layout_reverse() or layout_forward()` func
	*
	* @func is_ready_layout_typesetting()
	*/
bool Box::is_ready_layout_typesetting() {
	if (parent()->is_layout_lock_child()) { // layout size locked by parent layout
		if (parent()->layout_mark() & M_LAYOUT_TYPESETTING) {
			// The parent layout needs to be readjusted
			return false;
		}
	}
	return true;
}

/**
	* @func solve_rect_vertex(vertex)
	*/
void Box::solve_rect_vertex(Vec2 vertex[4]) {
	auto& mat = matrix();
	Vec2 start(-_transform_origin.x(), -_transform_origin.y());
	Vec2 end(_client_size.x() + start.x(), _client_size.y() + start.y());
	vertex[0] = mat * start;
	vertex[1] = mat * Vec2(end.x(), start.y());
	vertex[2] = mat * end;
	vertex[3] = mat * Vec2(start.x(), end.y());
}

bool Box::solve_visible_region() {
	Vec2 vertex[4];
	solve_rect_vertex(vertex);

	/*
	* 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
	* 这种模糊测试在大多数时候都是正确有效的。
	*/
	DisplayRegion clip = display()->clip_region();
	Region re = screen_region_from_convex_quadrilateral(vertex);
	
	if (F_MAX( clip.y2, re.end.y() ) - F_MIN( clip.y, re.origin.y() ) <= re.end.y() - re.origin.y() + clip.height &&
			F_MAX( clip.x2, re.end.x() ) - F_MIN( clip.x, re.origin.x() ) <= re.end.x() - re.origin.x() + clip.width
	) {
		return true;
	}

	return false;
}

F_NAMESPACE_END
