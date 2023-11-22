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

	float Box::solve_layout_content_width(Size &parent_layout_size) {
		float ps = parent_layout_size.content_size.x();
		bool* is_wrap_in_out = &parent_layout_size.wrap_x;
		float result;

		switch (_width.kind) {
			default: // NONE /* none default wrap content */
			case BoxSizeKind::kWrap: /* 包裹内容 wrap content */
				*is_wrap_in_out = true;
				result = 0; // invalid wrap width
				break;
			case BoxSizeKind::kPixel: /* 明确值 value px */
				*is_wrap_in_out = false;
				result = _width.value;
				break;
			case BoxSizeKind::kMatch: /* 匹配父视图 match parent */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap width
				} else { // use wrap
					result = ps - _margin_left - _margin_right - _padding_left - _padding_right;
					if (_border)
						result -= (_border->width[3] + _border->width[1]); // left + right
					result = Number<float>::max(result, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case BoxSizeKind::kRatio: /* 百分比 value % */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap width
				} else { // use wrap
					result = Number<float>::max(ps * _width.value, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case BoxSizeKind::kMinus: /* 减法(parent-value) value ! */
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

		switch (_height.kind) {
			default: // NONE /* none default wrap content */
			case BoxSizeKind::kWrap: /* 包裹内容 wrap content */
				*is_wrap_in_out = true;
				result = 0; // invalid wrap height
				break;
			case BoxSizeKind::kPixel: /* 明确值 value px */
				*is_wrap_in_out = false;
				result = _height.value;
				break;
			case BoxSizeKind::kMatch: /* 匹配父视图 match parent */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap height
				} else { // use wrap
					result = ps - _margin_top - _margin_bottom - _padding_top - _padding_bottom;
					if (_border)
						result -= (_border->width[3] + _border->width[1]); // left + right
					result = Number<float>::max(result, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case BoxSizeKind::kRatio: /* 百分比 value % */
				if (*is_wrap_in_out) {
					result = 0; // invalid wrap height
				} else { // use wrap
					result = Number<float>::max(ps * _height.value, 0);
				}
				// *is_wrap_in_out = *is_wrap_in_out;
				break;
			case BoxSizeKind::kMinus: /* 减法(parent-value) value ! */
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

	void Box::mark_size(uint32_t _mark) {
		auto _parent = parent();
		if (_parent) {
			if (_parent->is_lock_child_layout_size()) {
				_parent->onChildLayoutChange(this, kChild_Layout_Size);
			} else {
				mark(_mark);
			}
		}
	}

	/**
		* @constructors
		*/
	Box::Box()
		: View()
		, _layout_wrap_x(true), _layout_wrap_y(true), _is_clip(false)
		, _width{0, BoxSizeKind::kWrap}, _height{0, BoxSizeKind::kWrap}
		, _width_limit{0, BoxSizeKind::kNone}, _height_limit{0, BoxSizeKind::kNone}
		, _margin_top(0), _margin_right(0)
		, _margin_bottom(0), _margin_left(0)
		, _padding_top(0), _padding_right(0)
		, _padding_bottom(0), _padding_left(0)
		, _radius_left_top(0), _radius_right_top(0)
		, _radius_right_bottom(0), _radius_left_bottom(0)
		, _background_color(Color::from(0))
		, _background(nullptr)
		, _box_shadow(nullptr)
		, _layout_weight(0), _layout_align(Align::kAuto)
		, _border(nullptr)
	{
	}

	Box::~Box() {
		Release(_background); _background = nullptr;
		Release(_box_shadow); _box_shadow = nullptr;
		::free(_border); _border = nullptr;
	}

	// is clip box display range
	void Box::set_is_clip(bool val) {
		if (_is_clip != val) {
			_is_clip = val;
			mark_render();
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
			mark_size(kLayout_Size_Width);
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
			mark_size(kLayout_Size_Height);
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
			mark_size(kLayout_Size_Width);
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
			mark_size(kLayout_Size_Height);
		}
	}

	void Box::set_margin_top(float val) { // margin
		if (_margin_top != val) {
			_margin_top = val;
			mark_size(kLayout_Size_Height);
			mark_render(kRecursive_Transform);
		}
	}

	void Box::set_margin_left(float val) {
		if (_margin_left != val) {
			_margin_left = val;
			mark_size(kLayout_Size_Width);
			mark_render(kRecursive_Transform);
		}
	}

	void Box::set_padding_top(float val) { // padding
		if (_padding_top != val) {
			_padding_top = val;
			mark_size(kLayout_Size_Height);
			// 没有直接的影响到`transform`但可能导致`layout_size`变化导致
			// `transform_origin`百分比属性变化,间接影响`transform`变化, 但可以肯定这个会影响子布局偏移
			// mark_render(kRecursive_Transform); 
		}
	}

	void Box::set_padding_left(float val) {
		if (_padding_left != val) {
			_padding_left = val;
			mark_size(kLayout_Size_Width);
			//mark_render(kRecursive_Transform); // @`set_padding_top(val)`
		}
	}

	// --
	void Box::set_margin_right(float val) {
		if (_margin_right != val) {
			_margin_right = val;
			mark_size(kLayout_Size_Width);
			//mark_render(kRecursive_Transform); // @`set_padding_top(val)`
		}
	}

	void Box::set_margin_bottom(float val) {
		if (_margin_bottom != val) {
			_margin_bottom = val;
			mark_size(kLayout_Size_Height);
			//mark_render(kRecursive_Transform); // @`set_padding_top(val)`
		}
	}

	void Box::set_padding_right(float val) {
		if (_padding_right != val) {
			_padding_right = val;
			mark_size(kLayout_Size_Width);
			//mark_render(kRecursive_Transform); // @`set_padding_top(val)`
		}
	}

	void Box::set_padding_bottom(float val) {
		if (_padding_bottom != val) {
			_padding_bottom = val;
			mark_size(kLayout_Size_Height);
			//mark_render(kRecursive_Transform); // @`set_padding_top(val)`
		}
	}

	// -- border radius

	void Box::set_radius_left_top(float val) {
		if (val >= 0.0 && _radius_left_top != val) {
			_radius_left_top = val;
			mark_render();
		}
	}

	void Box::set_radius_right_top(float val) {
		if (val >= 0.0 && _radius_right_top != val) {
			_radius_right_top = val;
			mark_render();
		}
	}

	void Box::set_radius_right_bottom(float val) {
		if (val >= 0.0 && _radius_right_bottom != val) {
			_radius_right_bottom = val;
			mark_render();
		}
	}

	void Box::set_radius_left_bottom(float val) {
		if (val >= 0.0 && _radius_left_bottom != val) {
			_radius_left_bottom = val;
			mark_render();
		}
	}

	// alloc border memory
	static void alloc_border(BoxBorder*& _border) {
		if (!_border) {
			_border = (BoxBorder*)::malloc(sizeof(BoxBorder));
			::memset(_border, 0, sizeof(BoxBorder));
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
	} // border_widrh

	float Box::border_width_right() const {
		return _border ? _border->width[1]: 0;
	}

	float Box::border_width_bottom() const {
		return _border ? _border->width[2]: 0;
	}

	float Box::border_width_left() const {
		return _border ? _border->width[3]: 0;
	}

	void Box::set_border_color_top(Color val) {
		alloc_border(_border);
		if (_border->color[0] != val) {
			_border->color[0] = val;
			mark_render();
		}
	}

	void Box::set_border_color_right(Color val) {
		alloc_border(_border);
		if (_border->color[1] != val) {
			_border->color[1] = val;
			mark_render();
		}
	}

	void Box::set_border_color_bottom(Color val) {
		alloc_border(_border);
		if (_border->color[2] != val) {
			_border->color[2] = val;
			mark_render();
		}
	}

	void Box::set_border_color_left(Color val) {
		alloc_border(_border);
		if (_border->color[3] != val) {
			_border->color[3] = val;
			mark_render();
		}
	}

	void Box::set_border_width_top(float val) {
		alloc_border(_border);
		val = Qk_MAX(0, val);
		if (_border->width[0] != val) {
			_border->width[0] = val;
			// fix aa sdf stroke for view render
			//val -= (2.0f * 0.45f / pre_render()->window()->scale());
			_border->_fix_width[0] = Qk_MAX(0, val);
			mark_size(kLayout_Size_Height);
		}
	} // border_widrh

	void Box::set_border_width_right(float val) {
		alloc_border(_border);
		val = Qk_MAX(0, val);
		if (_border->width[1] != val) {
			_border->width[1] = val;
			// fix aa sdf stroke for view render
			//val -= (2.0f * 0.45f / pre_render()->window()->scale());
			_border->_fix_width[1] = Qk_MAX(0, val);
			mark_size(kLayout_Size_Width);
		}
	}

	void Box::set_border_width_bottom(float val) {
		alloc_border(_border);
		val = Qk_MAX(0, val);
		if (_border->width[2] != val) {
			_border->width[2] = val;
			// fix aa sdf stroke for view render
			//val -= (2.0f * 0.45f / pre_render()->window()->scale());
			_border->_fix_width[2] = Qk_MAX(0, val);
			mark_size(kLayout_Size_Height);
		}
	}

	void Box::set_border_width_left(float val) {
		alloc_border(_border);
		val = Qk_MAX(0, val);
		if (_border->width[3] != val) {
			_border->width[3] = val;
			// fix aa sdf stroke for view render
			//val -= (2.0f * 0.45f / pre_render()->window()->scale());
			_border->_fix_width[3] = Qk_MAX(0, val);
			mark_size(kLayout_Size_Width);
		}
	}

	void Box::set_background_color(Color color) {
		if (_background_color != color) {
			_background_color = color;
			mark_render();
		}
	}

	void Box::set_background(BoxFill* val) {
		if (_background != val) {
			_background = static_cast<BoxFill*>(BoxFilter::assign(_background, val));
			mark_render();
		}
	}

	void Box::set_box_shadow(BoxShadow* val) {
		if (_box_shadow != val) {
			_box_shadow = static_cast<BoxShadow*>(BoxFilter::assign(_box_shadow, val));
			mark_render();
		}
	}

	/**
		* @func solve_layout_size_forward()
		*/
	uint32_t Box::solve_layout_size_forward(uint32_t mark) {
		uint32_t layout_content_size_change_mark = kLayout_None;

		if (mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			auto Parent = parent();
			uint32_t child_layout_change_mark = 0;

			if (!Parent->is_lock_child_layout_size()) {
				auto size = Parent->layout_size();

				if (mark & kLayout_Size_Width)
				{
					auto val = solve_layout_content_width(size);
					if (val != _content_size.x() || _layout_wrap_x != size.wrap_x) {
						_content_size.set_x(val);
						_layout_wrap_x = size.wrap_x;
						// mark(kLayout_Typesetting);
						layout_content_size_change_mark = kLayout_Size_Width;
					}
					_client_size.set_x(_padding_left + _padding_right + val);
					if (_border)
						_client_size.val[0] += _border->width[3] + _border->width[1]; // left + right

					float x = _margin_left + _margin_right + _client_size.x();
					if (_layout_size.x() != x) {
						_layout_size.set_x(x);
						child_layout_change_mark = kChild_Layout_Size;
					}
				}

				if (mark & kLayout_Size_Height)
				{
					auto val = solve_layout_content_height(size);
					if (val != _content_size.y() || _layout_wrap_y != size.wrap_y) {
						_content_size.set_y(val);
						_layout_wrap_y = size.wrap_y;
						// mark(kLayout_Typesetting);
						layout_content_size_change_mark |= kLayout_Size_Height;
					}
					_client_size.set_y(_padding_top + _padding_bottom + val);
					if (_border)
						_client_size.val[1] += _border->width[0] + _border->width[2]; // top + bottom

					float y = _margin_top + _margin_bottom + _client_size.y();
					if (_layout_size.y() != y) {
						_layout_size.set_y(y);
						child_layout_change_mark = kChild_Layout_Size;
					}
				}

			} // else The layout is locked and does not need to be updated

			Parent->onChildLayoutChange(this, child_layout_change_mark); // notice parent
			unmark(kLayout_Size_Width | kLayout_Size_Height);
		}

		return layout_content_size_change_mark;
	}

	bool Box::layout_forward(uint32_t _mark) {
		uint32_t layout_content_size_change_mark = solve_layout_size_forward(_mark);

		if (layout_content_size_change_mark) {
			auto v = first();
			while (v) {
				v->onParentLayoutContentSizeChange(this, layout_content_size_change_mark);
				v = v->next();
			}
			mark(kLayout_Typesetting); // rearrange
			mark_render(kRecursive_Visible_Region);
			return true;
		}

		if (layout_mark() & kLayout_Typesetting) {
			return true;
		}

		return false;
	}

	bool Box::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting()) return true; // continue iteration

			auto v = first();
			if (v) {
				do { // lazy layout
					if (v->visible())
						v->set_layout_offset_lazy(_content_size); // lazy layout
					v = v->next();
				} while(v);
			}
			unmark(kLayout_Typesetting);
		}

		return false; // stop iteration
	}

	void Box::layout_text(TextLines *lines, TextConfig *cfg) {
		auto opts = cfg->opts();
		auto text_white_space = opts->text_white_space_value();
		//auto text_word_break = opts->text_word_break_value();
		bool is_auto_wrap = true;
		auto limitX = lines->host_size().x();
		auto origin = lines->pre_width();

		if (lines->no_wrap() || // 容器没有固定宽度
				text_white_space == TextWhiteSpace::kNoWrap ||
				text_white_space == TextWhiteSpace::kPre
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

		lines->add_layout(this);
	}

	Vec2 Box::layout_lock(Vec2 layout_size) {
		bool wrap[2] = { false, false };
		set_layout_size(layout_size, wrap, false);
		return _layout_size;
	}

	/**
		* @func set_layout_size(layout_size, is_wrap)
		*/
	void Box::set_layout_size(Vec2 layout_size, bool is_wrap[2], bool is_lock_child) {
		uint32_t layout_content_size_change_mark = kLayout_None;
		auto content_size_old = _content_size;

		auto bp_x = _padding_left + _padding_right;
		auto bp_y = _padding_left + _padding_right;
		if (_border) {
			bp_x += _border->width[3] + _border->width[1]; // left + right
			bp_y += _border->width[0] + _border->width[2]; // top + bottom
		}
		auto mbp_x = _margin_left + _margin_right + bp_x;
		auto mbp_y = _margin_top + _margin_bottom + bp_y;

		_content_size = Vec2(
			layout_size.x() > mbp_x ? layout_size.x() - mbp_x: 0,
			layout_size.y() > mbp_y ? layout_size.y() - mbp_y: 0
		);
		_client_size = Vec2(bp_x + _content_size.x(), bp_y + _content_size.y());
		_layout_size = Vec2(mbp_x + _content_size.x(), mbp_y + _content_size.y());

		if (content_size_old.x() != _content_size.x() || _layout_wrap_x != is_wrap[0]) {
			layout_content_size_change_mark = kLayout_Size_Width;
		}
		if (content_size_old.y() != _content_size.y() || _layout_wrap_y != is_wrap[1]) {
			layout_content_size_change_mark |= kLayout_Size_Height;
		}

		_layout_wrap_x = is_wrap[0];
		_layout_wrap_y = is_wrap[1];

		if (layout_content_size_change_mark) {
			if (!is_lock_child) { // if no lock child then
				auto v = first();
				while (v) {
					v->onParentLayoutContentSizeChange(this, layout_content_size_change_mark);
					v = v->next();
				}
			}
			mark(kLayout_Typesetting); // rearrange
			mark_render(kRecursive_Visible_Region);
		}

		unmark(kLayout_Size_Width | kLayout_Size_Height);
	}

	/**
		* @func set_content_size(content_size)
		*/
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

		mark_render(kRecursive_Visible_Region);
	}

	Vec2 Box::layout_offset() {
		return _layout_offset;
	}

	Layout::Size Box::layout_size() {
		return {
			_layout_size, _content_size, _layout_wrap_x, _layout_wrap_y
		};
	}

	Layout::Size Box::layout_raw_size(Size size) {
		size.content_size.set_x(solve_layout_content_width(size));
		size.content_size.set_x(solve_layout_content_height(size));
		size.layout_size.set_x(_margin_left + _padding_left + size.content_size.x() + _padding_right + _margin_right);
		size.layout_size.set_y(_margin_top + _padding_top + size.content_size.y() + _padding_bottom + _margin_bottom);
		if (_border) {
			size.layout_size.val[0] += _border->width[3] + _border->width[1]; // left + right
			size.layout_size.val[1] += _border->width[0] + _border->width[2]; // top + bottom
		}
		return size;
	}

	float Box::layout_weight() {
		return _layout_weight;
	}

	Align Box::layout_align() {
		return _layout_align;
	}

	Mat Box::layout_matrix() {
		Vec2 translate = layout_offset() + parent()->layout_offset_inside()
			+ Vec2(_margin_left, _margin_top);
		return Mat(
			1, 0, translate.x(),
			0, 1, translate.y()
		);
	}

	Vec2 Box::layout_offset_inside() {
		Vec2 offset(
			_padding_left,
			_padding_top
		);
		if (_border) {
			offset.val[0] += _border->width[3]; // left
			offset.val[1] += _border->width[0]; // top
		}
		return offset;
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
				parent()->onChildLayoutChange(this, kChild_Layout_Align);
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
				parent()->onChildLayoutChange(this, kChild_Layout_Weight);
			}
		}
	}

	void Box::set_layout_offset(Vec2 val) {
		_layout_offset = val;
		mark_render(kRecursive_Transform); // mark recursive transform
	}

	void Box::set_layout_offset_lazy(Vec2 size) {
		Vec2 offset;

		switch(_layout_align) {
			default:
			case Align::kLeftTop: // left top
				break;
			case Align::kCenterTop: // center top
				offset = Vec2((size.x() - _layout_size.x()) * .5, 0);
				break;
			case Align::kRightTop: // right top
				offset = Vec2(size.x() - _layout_size.x(), 0);
				break;
			case Align::kLeftCenter: // left center
				offset = Vec2(0, (size.y() - _layout_size.y()) * .5);
				break;
			case Align::kCenterCenter: // center center
				offset = Vec2(
					(size.x() - _layout_size.x()) * .5,
					(size.y() - _layout_size.y()) * .5);
				break;
			case Align::kRightCenter: // right center
				offset = Vec2(
					(size.x() - _layout_size.x()),
					(size.y() - _layout_size.y()) * .5);
				break;
			case Align::kLeftBottom: // left bottom
				offset = Vec2(0, (size.y() - _layout_size.y()));
				break;
			case Align::kCenterBottom: // center bottom
				offset = Vec2(
					(size.x() - _layout_size.x()) * .5,
					(size.y() - _layout_size.y()));
				break;
			case Align::kRightBottom: // right bottom
				offset = Vec2(
					(size.x() - _layout_size.x()),
					(size.y() - _layout_size.y()));
				break;
		}

		set_layout_offset(offset);
	}

	void Box::onParentLayoutContentSizeChange(Layout* parent, uint32_t value) {
		mark(value);
	}

	/**
		* 
		* is ready layout layout typesetting in the `layout_reverse() or layout_forward()` func
		*
		* @func is_ready_layout_typesetting()
		*/
	bool Box::is_ready_layout_typesetting() {
		if (parent()->is_lock_child_layout_size()) { // layout size locked by parent layout
			if (parent()->layout_mark() & kLayout_Typesetting) {
				// The parent layout needs to be readjusted
				return false;
			}
		}
		return true;
	}

	Vec2 Box::position() {
		Vec2 point(
			_client_size.x() * 0.5,
			_client_size.y() * 0.5
		);
		return matrix() * point;
	}

	/**
		* @func solve_rect_vertex(vertex)
		*/
	void Box::solve_rect_vertex(Vec2 vertex[4]) {
		auto& mat = matrix();
		Vec2 origin;
		Vec2 end = _client_size;
		vertex[0] = mat * origin;
		vertex[1] = mat * Vec2(end.x(), origin.y());
		vertex[2] = mat * end;
		vertex[3] = mat * Vec2(origin.x(), end.y());
	}

	bool Box::solve_visible_region() {
		// Vec2 vertex[4];
		solve_rect_vertex(_vertex);

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

	bool Box::clip() {
		return _is_clip;
	}

	bool Box::overlap_test(Vec2 point) {
		return overlap_test_from_convex_quadrilateral(_vertex, point);
	}

}
