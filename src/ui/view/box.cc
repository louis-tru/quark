/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
#define _async_call(block, param) async_call([](auto self, auto arg) block, this, param)

namespace qk {

	static Border     default_border{0,Color::from(0)};
	static ArrayColor default_border_color{Color::from(0),Color::from(0),Color::from(0),Color::from(0)};
	static ArrayFloat default_border_width(4);

	Box::Box()
		: _clip(false)
		, _layout(LayoutType::Normal) // default normal layout
		, _align(Align::Normal)
		, _box_sizing(BoxSizing::ContentBox)
		, _min_width{0, BoxSizeKind::Auto}, _min_height{0, BoxSizeKind::Auto}
		, _max_width{0, BoxSizeKind::None}, _max_height{0, BoxSizeKind::None}
		, _margin_top(0), _margin_right(0)
		, _margin_bottom(0), _margin_left(0)
		, _padding_top(0), _padding_right(0)
		, _padding_bottom(0), _padding_left(0)
		, _border_top_left_radius(0), _border_top_right_radius(0)
		, _border_bottom_right_radius(0), _border_bottom_left_radius(0)
		, _background_color(Color::from(0))
		, _weight(0,0)
		, _container({{},{},0,Float32::limit_max,0,Float32::limit_max,kNone_FloatState,kNone_FloatState,false,false})
		, _background(nullptr)
		, _box_shadow(nullptr)
		, _border(nullptr)
	{
		// Qk_DLog("Box, %d", sizeof(Box));
		// sizeof(BoxSize[4]);
	}

	Box::~Box() {
		Qk_ASSERT_EQ(_background, nullptr);
		Qk_ASSERT_EQ(_box_shadow, nullptr);
		// cannot release in destroy() because may use in layout.
		// but _background and _box_shadow is can be released immediately in destroy()
		::free(_border.load());
	}

	void Box::destroy() {
		Releasep(_background); // Can be released immediately
		Releasep(_box_shadow);
		View::destroy();
	}

	BoxSize Box::width() const {
		return _min_width;
	}

	BoxSize Box::height() const {
		return _min_height;
	}

	ArrayFloat Box::margin() const {
		return ArrayFloat{_margin_top,_margin_right,_margin_bottom,_margin_left};
	}

	ArrayFloat Box::padding() const {
		return ArrayFloat{_padding_top,_padding_right,_padding_bottom,_padding_left};
	}

	ArrayFloat Box::border_radius() const {
		return ArrayFloat{
			_border_top_left_radius,     _border_top_right_radius,
			_border_bottom_right_radius, _border_bottom_left_radius
		};
	}

	ArrayBorder Box::border() const {
		_Border();
		return _border ? ArrayBorder{
			{_border->width[0],_border->color[0]},{_border->width[1],_border->color[1]},
			{_border->width[2],_border->color[2]},{_border->width[3],_border->color[3]},
		}: ArrayBorder{default_border,default_border,default_border,default_border};
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

	ArrayFloat Box::border_width() const {
		_Border();
		return _border ?
			ArrayFloat({_border->width[0],_border->width[1],_border->width[2],_border->width[3]}):
			default_border_width;
	}

	ArrayColor Box::border_color() const {
		_Border();
		return _border ?
			ArrayColor({_border->color[0],_border->color[1],_border->color[2],_border->color[3]}):
			default_border_color;
	}

	Color Box::border_top_color() const {
		_Border();
		return _border ? _border->color[0]: Color::from(0);
	}

	Color Box::border_right_color() const {
		_Border();
		return _border ? _border->color[1]: Color::from(0);
	}

	Color Box::border_bottom_color() const {
		_Border();
		return _border ? _border->color[2]: Color::from(0);
	}

	Color Box::border_left_color() const {
		_Border();
		return _border ? _border->color[3]: Color::from(0);
	}

	float Box::border_top_width() const {
		_Border();
		return _border ? _border->width[0]: 0;
	}

	float Box::border_right_width() const {
		_Border();
		return _border ? _border->width[1]: 0;
	}

	float Box::border_bottom_width() const {
		_Border();
		return _border ? _border->width[2]: 0;
	}

	float Box::border_left_width() const {
		_Border();
		return _border ? _border->width[3]: 0;
	}

	// =========================================================================
	void Box::set_layout(LayoutType val) {
		mark_style_flag(kLAYOUT_CssProp); // mark layout style flag
		// To avoid in-frame jitter,
		// all layout and rendering-related data attributes must be modified on the rendering thread,
		// and after calling the `async_call` method,
		// all modifications will be submitted to the rendering thread at the end of the main thread's tick.
		_async_call({ self->set_layout_direct(arg, true); }, val);
	}

	// is clip box display range
	void Box::set_clip(bool val) {
		mark_style_flag(kCLIP_CssProp);
		_async_call({ self->set_clip_direct(arg, true); }, val);
	}

	void Box::set_box_sizing(BoxSizing box_sizing) {
		mark_style_flag(kBOX_SIZING_CssProp);
		_async_call({ self->set_box_sizing_direct(arg, true); }, box_sizing);
	}

	void Box::set_width(BoxSize val) {
		mark_style_flag(kWIDTH_CssProp);
		_async_call({ self->set_width_direct(arg, true); }, val);
	}

	void Box::set_height(BoxSize val) {
		mark_style_flag(kHEIGHT_CssProp);
		_async_call({ self->set_height_direct(arg, true); }, val);
	}

	void Box::set_min_width(BoxSize val) {
		mark_style_flag(kMIN_WIDTH_CssProp); // mark min-width style flag together
		set_width(val); // min-width is alias of width
	}

	void Box::set_min_height(BoxSize val) {
		mark_style_flag(kMIN_HEIGHT_CssProp); // mark min-height style flag together
		set_height(val); // min-height is alias of height
	}

	void Box::set_max_width(BoxSize val) {
		mark_style_flag(kMAX_WIDTH_CssProp);
		_async_call({ self->set_max_width_direct(arg, true); }, val);
	}

	void Box::set_max_height(BoxSize val) {
		mark_style_flag(kMAX_HEIGHT_CssProp);
		_async_call({ self->set_max_height_direct(arg, true); }, val);
	}

	void Box::set_margin(ArrayFloat val) {
		// mark margin top/right/bottom/left style flags
		_style_flags[kMARGIN_TOP_CssProp / 32] |= (0b1111 << kMARGIN_TOP_CssProp);
		_async_call({
			Sp<ArrayFloat> handle(arg); // auto release
			self->set_margin_direct(std::move(*arg), true);
		}, new ArrayFloat(std::move(val)));
	}

	void Box::set_margin_left(float val) {
		mark_style_flag(kMARGIN_LEFT_CssProp);
		_async_call({ self->set_margin_left_direct(arg, true); }, val);
	}

	void Box::set_margin_top(float val) { // margin
		mark_style_flag(kMARGIN_TOP_CssProp);
		_async_call({ self->set_margin_top_direct(arg, true); }, val);
	}

	void Box::set_margin_right(float val) {
		mark_style_flag(kMARGIN_RIGHT_CssProp);
		_async_call({ self->set_margin_right_direct(arg, true); }, val);
	}

	void Box::set_margin_bottom(float val) {
		mark_style_flag(kMARGIN_BOTTOM_CssProp);
		_async_call({ self->set_margin_bottom_direct(arg, true); }, val);
	}

	void Box::set_padding(ArrayFloat val) {
		// mark padding top/right/bottom/left style flags
		_style_flags[kPADDING_TOP_CssProp / 32] |= (0b1111 << kPADDING_TOP_CssProp);
		_async_call({
			Sp<ArrayFloat> handle(arg); // auto release
			self->set_padding_direct(std::move(*arg), true);
		}, new ArrayFloat(std::move(val)));
	}

	void Box::set_padding_top(float val) { // padding
		mark_style_flag(kPADDING_TOP_CssProp);
		_async_call({ self->set_padding_top_direct(arg, true); }, val);
	}

	void Box::set_padding_left(float val) {
		mark_style_flag(kPADDING_LEFT_CssProp);
		_async_call({ self->set_padding_left_direct(arg, true); }, val);
	}

	void Box::set_padding_right(float val) {
		mark_style_flag(kPADDING_RIGHT_CssProp);
		_async_call({ self->set_padding_right_direct(arg, true); }, val);
	}

	void Box::set_padding_bottom(float val) {
		mark_style_flag(kPADDING_BOTTOM_CssProp);
		_async_call({ self->set_padding_bottom_direct(arg, true); }, val);
	}

	void Box::set_border_radius(ArrayFloat val) {
		// mark border radius top-left/top-right/bottom-right style flags
		_style_flags[kBORDER_TOP_LEFT_RADIUS_CssProp / 32] |= (0b111 << kBORDER_TOP_LEFT_RADIUS_CssProp);
		// mark border radius bottom-left style flag
		_style_flags[kBORDER_BOTTOM_LEFT_RADIUS_CssProp / 32] |= (0b1 << (kBORDER_BOTTOM_LEFT_RADIUS_CssProp % 32));
		_async_call({
			Sp<ArrayFloat> handle(arg); // auto release
			self->set_border_radius_direct(std::move(*arg), true);
		}, new ArrayFloat(std::move(val)));
	}

	void Box::set_border_top_left_radius(float val) {
		mark_style_flag(kBORDER_TOP_LEFT_RADIUS_CssProp);
		_async_call({ self->set_border_top_left_radius_direct(arg, true); }, val);
	}

	void Box::set_border_top_right_radius(float val) {
		mark_style_flag(kBORDER_TOP_RIGHT_RADIUS_CssProp);
		_async_call({ self->set_border_top_right_radius_direct(arg, true); }, val);
	}

	void Box::set_border_bottom_right_radius(float val) {
		mark_style_flag(kBORDER_BOTTOM_RIGHT_RADIUS_CssProp);
		_async_call({ self->set_border_bottom_right_radius_direct(arg, true); }, val);
	}

	void Box::set_border_bottom_left_radius(float val) {
		mark_style_flag(kBORDER_BOTTOM_LEFT_RADIUS_CssProp);
		_async_call({ self->set_border_bottom_left_radius_direct(arg, true); }, val);
	}

	void Box::set_border(ArrayBorder val) {
		// mark border top/right/bottom/left style flags
		// include border width and color
		_style_flags[kBORDER_TOP_COLOR_CssProp / 32] |= (0b11111111 << (kBORDER_TOP_COLOR_CssProp % 32));
		_async_call({
			Sp<ArrayBorder> handle(arg); // auto release
			self->set_border_direct(std::move(*arg), true);
		}, new ArrayBorder(std::move(val)));
	}

	void Box::set_border_top(Border border) {
		mark_style_flag(kBORDER_TOP_COLOR_CssProp);
		mark_style_flag(kBORDER_TOP_WIDTH_CssProp);
		_async_call({ self->set_border_top_direct(arg, true); }, border);
	}

	void Box::set_border_right(Border border) {
		mark_style_flag(kBORDER_RIGHT_COLOR_CssProp);
		mark_style_flag(kBORDER_RIGHT_WIDTH_CssProp);
		_async_call({ self->set_border_right_direct(arg, true); }, border);
	}

	void Box::set_border_bottom(Border border) {
		mark_style_flag(kBORDER_BOTTOM_COLOR_CssProp);
		mark_style_flag(kBORDER_BOTTOM_WIDTH_CssProp);
		_async_call({ self->set_border_bottom_direct(arg, true); }, border);
	}

	void Box::set_border_left(Border border) {
		mark_style_flag(kBORDER_LEFT_COLOR_CssProp);
		mark_style_flag(kBORDER_LEFT_WIDTH_CssProp);
		_async_call({ self->set_border_left_direct(arg, true); }, border);
	}

	void Box::set_border_width(ArrayFloat val) {
		// mark border width top/right/bottom/left style flags
		_style_flags[kBORDER_TOP_WIDTH_CssProp / 32] |= (0b1111 << (kBORDER_TOP_WIDTH_CssProp % 32));
		_async_call({
			Sp<ArrayFloat> handle(arg); // auto release
			self->set_border_width_direct(std::move(*arg), true);
		}, new ArrayFloat(std::move(val)));
	}

	void Box::set_border_color(ArrayColor val) {
		// mark border color top/right/bottom/left style flags
		_style_flags[kBORDER_TOP_COLOR_CssProp / 32] |= (0b1111 << (kBORDER_TOP_COLOR_CssProp % 32));
		_async_call({
			Sp<ArrayColor> handle(arg); // auto release
			self->set_border_color_direct(std::move(*arg), true);
		}, new ArrayColor(std::move(val)));
	}

	void Box::set_border_top_color(Color val) {
		mark_style_flag(kBORDER_TOP_COLOR_CssProp);
		_async_call({ self->set_border_top_color_direct(arg, true); }, val);
	}

	void Box::set_border_right_color(Color val) {
		mark_style_flag(kBORDER_RIGHT_COLOR_CssProp);
		_async_call({ self->set_border_right_color_direct(arg, true); }, val);
	}

	void Box::set_border_bottom_color(Color val) {
		mark_style_flag(kBORDER_BOTTOM_COLOR_CssProp);
		_async_call({ self->set_border_bottom_color_direct(arg, true); }, val);
	}

	void Box::set_border_left_color(Color val) {
		mark_style_flag(kBORDER_LEFT_COLOR_CssProp);
		_async_call({ self->set_border_left_color_direct(arg, true); }, val);
	}

	void Box::set_border_top_width(float val) {
		mark_style_flag(kBORDER_TOP_WIDTH_CssProp);
		_async_call({ self->set_border_top_width_direct(arg, true); }, val);
	}

	void Box::set_border_right_width(float val) {
		mark_style_flag(kBORDER_RIGHT_WIDTH_CssProp);
		_async_call({ self->set_border_right_width_direct(arg, true); }, val);
	}

	void Box::set_border_bottom_width(float val) {
		mark_style_flag(kBORDER_BOTTOM_WIDTH_CssProp);
		_async_call({ self->set_border_bottom_width_direct(arg, true); }, val);
	}

	void Box::set_border_left_width(float val) {
		mark_style_flag(kBORDER_LEFT_WIDTH_CssProp);
		_async_call({ self->set_border_left_width_direct(arg, true); }, val);
	}

	void Box::set_background_color(Color color) {
		mark_style_flag(kBACKGROUND_COLOR_CssProp);
		_async_call({ self->set_background_color_direct(arg, true); }, color);
	}

	void Box::set_background(BoxFilter* val) {
		mark_style_flag(kBACKGROUND_CssProp);
		Retain(val); // Retain before async call to ensure the object is not released before the async call is executed
		_async_call({
			self->set_background_direct(arg, true);
			Release(arg); // Release after async call to ensure the object is released after the async call is executed
		}, val);
	}

	void Box::set_box_shadow(BoxShadow* val) {
		mark_style_flag(kBOX_SHADOW_CssProp);
		Retain(val);
		_async_call({
			self->set_box_shadow_direct(arg, true);
			Release(arg);
		}, val);
	}

	void Box::set_align(Align align) {
		mark_style_flag(kALIGN_CssProp);
		_async_call({ self->set_align_direct(arg, true); }, align);
	}

	void Box::set_weight(Vec2 weight) {
		mark_style_flag(kWEIGHT_CssProp);
		_async_call({ self->set_weight_direct(arg, true); }, weight);
	}

	// =========================================================================

	void Box::set_clip_direct(bool val, bool isRT) {
		if (_clip != val) {
			_clip = val;
			mark_render();
		}
	}

	void Box::set_layout_direct(LayoutType val, bool isRT) {
		if (_layout != val) {
			_layout = val;
			mark_layout(kLayout_Typesetting, isRT);
		}
	}

	void Box::set_box_sizing_direct(BoxSizing val, bool isRT) {
		if (_box_sizing != val) {
			_box_sizing = val;
			mark_layout(kLayout_Size_ALL, isRT);
		}
	}

	void Box::set_width_direct(BoxSize val, bool isRT) {
		if (_min_width != val) {
			_min_width = val;
			mark_layout(kLayout_Inner_Width, isRT);
		}
	}

	void Box::set_height_direct(BoxSize val, bool isRT) {
		if (_min_height != val) {
			_min_height = val;
			mark_layout(kLayout_Inner_Height, isRT);
		}
	}

	void Box::set_min_width_direct(BoxSize val, bool isRT) {
		set_width_direct(val, isRT); // min-width is alias of width
	}

	void Box::set_min_height_direct(BoxSize val, bool isRT) {
		set_height_direct(val, isRT); // min-height is alias of height
	}

	void Box::set_max_width_direct(BoxSize val, bool isRT) {
		if (_max_width != val) {
			_max_width = val;
			mark_layout(kLayout_Inner_Width, isRT);
		}
	}

	void Box::set_max_height_direct(BoxSize val, bool isRT) {
		if (_max_height != val) {
			_max_height = val;
			mark_layout(kLayout_Inner_Height, isRT);
		}
	}

	void Box::set_margin_direct(ArrayFloat val, bool isRT) {
		switch (val.length()) {
			case 1:
				set_margin_left_direct(val[0], isRT);
				set_margin_top_direct(val[0], isRT);
				set_margin_right_direct(val[0], isRT);
				set_margin_bottom_direct(val[0], isRT);
				break;
			case 2:
				set_margin_top_direct(val[0], isRT);
				set_margin_bottom_direct(val[0], isRT);
				set_margin_left_direct(val[1], isRT);
				set_margin_right_direct(val[1], isRT);
				break;
			case 3:
				set_margin_top_direct(val[0], isRT);
				set_margin_left_direct(val[1], isRT);
				set_margin_right_direct(val[1], isRT);
				set_margin_bottom_direct(val[2], isRT);
				break;
			case 4: // 4
				set_margin_top_direct(val[0], isRT);
				set_margin_right_direct(val[1], isRT);
				set_margin_bottom_direct(val[2], isRT);
				set_margin_left_direct(val[3], isRT);
				break;
			default: break;
		}
	}

	void Box::set_margin_top_direct(float val, bool isRT) { // margin
		if (_margin_top != val) {
			_margin_top = val;
			mark_layout(kLayout_Size_Height | kTransform, isRT);
		}
	}

	void Box::set_margin_left_direct(float val, bool isRT) {
		if (_margin_left != val) {
			_margin_left = val;
			mark_layout(kLayout_Size_Width | kTransform, isRT);
		}
	}

	void Box::set_margin_right_direct(float val, bool isRT) {
		if (_margin_right != val) {
			_margin_right = val;
			mark_layout(kLayout_Size_Width, isRT);
		}
	}

	void Box::set_margin_bottom_direct(float val, bool isRT) {
		if (_margin_bottom != val) {
			_margin_bottom = val;
			mark_layout(kLayout_Size_Height, isRT);
		}
	}

	void Box::set_padding_direct(ArrayFloat val, bool isRT) {
		switch (val.length()) {
			case 1:
				set_padding_left_direct(val[0], isRT);
				set_padding_top_direct(val[0], isRT);
				set_padding_right_direct(val[0], isRT);
				set_padding_bottom_direct(val[0], isRT);
				break;
			case 2:
				set_padding_top_direct(val[0], isRT);
				set_padding_bottom_direct(val[0], isRT);
				set_padding_left_direct(val[1], isRT);
				set_padding_right_direct(val[1], isRT);
				break;
			case 3:
				set_padding_top_direct(val[0], isRT);
				set_padding_left_direct(val[1], isRT);
				set_padding_right_direct(val[1], isRT);
				set_padding_bottom_direct(val[2], isRT);
				break;
			case 4: // 4
				set_padding_top_direct(val[0], isRT);
				set_padding_right_direct(val[1], isRT);
				set_padding_bottom_direct(val[2], isRT);
				set_padding_left_direct(val[3], isRT);
				break;
			default: break;
		}
	}

	void Box::set_padding_top_direct(float val, bool isRT) {
		if (_padding_top != val) {
			_padding_top = val;
			// may affect the inside content height,
			// so mark kLayout_Inner_Height with kLayout_Outside_Width together
			mark_layout(kLayout_Size_Height, isRT);
		}
	}

	void Box::set_padding_left_direct(float val, bool isRT) {
		if (_padding_left != val) {
			_padding_left = val;
			mark_layout(kLayout_Size_Width, isRT);
		}
	}

	void Box::set_padding_right_direct(float val, bool isRT) {
		if (_padding_right != val) {
			_padding_right = val;
			mark_layout(kLayout_Size_Width, isRT);
		}
	}

	void Box::set_padding_bottom_direct(float val, bool isRT) {
		if (_padding_bottom != val) {
			_padding_bottom = val;
			mark_layout(kLayout_Size_Height, isRT);
		}
	}

	void Box::set_border_radius_direct(ArrayFloat val, bool isRT) {
		switch (val.length()) {
			case 1:
				set_border_top_left_radius_direct(val[0], isRT);
				set_border_top_right_radius_direct(val[0], isRT);
				set_border_bottom_right_radius_direct(val[0], isRT);
				set_border_bottom_left_radius_direct(val[0], isRT);
				break;
			case 2:
				set_border_top_left_radius_direct(val[0], isRT);
				set_border_top_right_radius_direct(val[0], isRT);
				set_border_bottom_right_radius_direct(val[1], isRT);
				set_border_bottom_left_radius_direct(val[1], isRT);
				break;
			case 3:
				set_border_top_left_radius_direct(val[0], isRT);
				set_border_top_right_radius_direct(val[1], isRT);
				set_border_bottom_right_radius_direct(val[2], isRT);
				set_border_bottom_left_radius_direct(val[2], isRT);
				break;
			case 4: // 4
				set_border_top_left_radius_direct(val[0], isRT);
				set_border_top_right_radius_direct(val[1], isRT);
				set_border_bottom_right_radius_direct(val[2], isRT);
				set_border_bottom_left_radius_direct(val[3], isRT);
				break;
			default: break;
		}
	}

	void Box::set_border_top_left_radius_direct(float val, bool isRT) {
		if (val >= 0.0 && _border_top_left_radius != val) {
			_border_top_left_radius = val;
			mark_render();
		}
	}

	void Box::set_border_top_right_radius_direct(float val, bool isRT) {
		if (val >= 0.0 && _border_top_right_radius != val) {
			_border_top_right_radius = val;
			mark_render();
		}
	}

	void Box::set_border_bottom_right_radius_direct(float val, bool isRT) {
		if (val >= 0.0 && _border_bottom_right_radius != val) {
			_border_bottom_right_radius = val;
			mark_render();
		}
	}

	void Box::set_border_bottom_left_radius_direct(float val, bool isRT) {
		if (val >= 0.0 && _border_bottom_left_radius != val) {
			_border_bottom_left_radius = val;
			mark_render();
		}
	}

	void Box::set_border_direct(ArrayBorder val, bool isRT) {
		switch (val.length()) {
			case 1:
				set_border_top_direct(val[0], isRT);
				set_border_right_direct(val[0], isRT);
				set_border_bottom_direct(val[0], isRT);
				set_border_left_direct(val[0], isRT);
				break;
			case 2:
				set_border_top_direct(val[0], isRT);
				set_border_bottom_direct(val[0], isRT);
				set_border_left_direct(val[1], isRT);
				set_border_right_direct(val[1], isRT);
				break;
			case 3:
				set_border_top_direct(val[0], isRT);
				set_border_left_direct(val[1], isRT);
				set_border_right_direct(val[1], isRT);
				set_border_bottom_direct(val[2], isRT);
				break;
			case 4: // 4
				set_border_top_direct(val[0], isRT);
				set_border_right_direct(val[1], isRT);
				set_border_bottom_direct(val[2], isRT);
				set_border_left_direct(val[3], isRT);
				break;
			default: break;
		}
	}

	void Box::set_border_top_direct(Border border, bool isRT) {
		set_border_top_width_direct(border.width, isRT);
		set_border_top_color_direct(border.color, isRT);
	}

	void Box::set_border_right_direct(Border border, bool isRT) {
		set_border_right_width_direct(border.width, isRT);
		set_border_right_color_direct(border.color, isRT);
	}

	void Box::set_border_bottom_direct(Border border, bool isRT) {
		set_border_bottom_width_direct(border.width, isRT);
		set_border_bottom_color_direct(border.color, isRT);
	}

	void Box::set_border_left_direct(Border border, bool isRT) {
		set_border_left_width_direct(border.width, isRT);
		set_border_left_color_direct(border.color, isRT);
	}

	void Box::set_border_width_direct(ArrayFloat val, bool isRT) {
		switch (val.length()) {
			case 1:
				set_border_top_width_direct(val[0], isRT);
				set_border_right_width_direct(val[0], isRT);
				set_border_bottom_width_direct(val[0], isRT);
				set_border_left_width_direct(val[0], isRT);
				break;
			case 2:
				set_border_top_width_direct(val[0], isRT);
				set_border_bottom_width_direct(val[0], isRT);
				set_border_left_width_direct(val[1], isRT);
				set_border_right_width_direct(val[1], isRT);
				break;
			case 3:
				set_border_top_width_direct(val[0], isRT);
				set_border_left_width_direct(val[1], isRT);
				set_border_right_width_direct(val[1], isRT);
				set_border_bottom_width_direct(val[2], isRT);
				break;
			case 4: // 4
				set_border_top_width_direct(val[0], isRT);
				set_border_right_width_direct(val[1], isRT);
				set_border_bottom_width_direct(val[2], isRT);
				set_border_left_width_direct(val[3], isRT);
				break;
			default: break;
		}
	}

	void Box::set_border_color_direct(ArrayColor val, bool isRT) {
		switch (val.length()) {
			case 1:
				set_border_top_color_direct(val[0], isRT);
				set_border_right_color_direct(val[0], isRT);
				set_border_bottom_color_direct(val[0], isRT);
				set_border_left_color_direct(val[0], isRT);
				break;
			case 2:
				set_border_top_color_direct(val[0], isRT);
				set_border_bottom_color_direct(val[0], isRT);
				set_border_left_color_direct(val[1], isRT);
				set_border_right_color_direct(val[1], isRT);
				break;
			case 3:
				set_border_top_color_direct(val[0], isRT);
				set_border_left_color_direct(val[1], isRT);
				set_border_right_color_direct(val[1], isRT);
				set_border_bottom_color_direct(val[2], isRT);
				break;
			case 4: // 4
				set_border_top_color_direct(val[0], isRT);
				set_border_right_color_direct(val[1], isRT);
				set_border_bottom_color_direct(val[2], isRT);
				set_border_left_color_direct(val[3], isRT);
				break;
			default: break;
		}
	}

	struct BorderAlloc: Box {
		#define _BorderAlloc() auto _border = static_cast<BorderAlloc*>(this)->alloc()
		BorderInl* alloc() {
			BorderInl* border = _border.load(std::memory_order_acquire);
			if (Qk_UNLIKELY(!border)) {
				BorderInl* newBorder = (BorderInl*)malloc(sizeof(BorderInl));
				memset(newBorder, 0, sizeof(BorderInl));
				// Safe exchange _border pointer
				BorderInl* expected = nullptr;
				if (_border.compare_exchange_strong(
						expected,
						newBorder,
						std::memory_order_release,
						std::memory_order_acquire)) {
					// we won, newBorder is now global
					border = newBorder;
				} else {
					// lost race, someone else installed it
					::free(newBorder); // free ours
					border = expected; // expected now holds the installed value
				}
			}
			return border;
		}
	};

	void Box::set_border_top_color_direct(Color val, bool isRT) {
		_BorderAlloc();
		if (_border->color[0] != val) {
			_border->color[0] = val;
			mark_render();
		}
	}

	void Box::set_border_right_color_direct(Color val, bool isRT) {
		_BorderAlloc();
		if (_border->color[1] != val) {
			_border->color[1] = val;
			mark_render();
		}
	}

	void Box::set_border_bottom_color_direct(Color val, bool isRT) {
		_BorderAlloc();
		if (_border->color[2] != val) {
			_border->color[2] = val;
			mark_render();
		}
	}

	void Box::set_border_left_color_direct(Color val, bool isRT) {
		_BorderAlloc();
		if (_border->color[3] != val) {
			_border->color[3] = val;
			mark_render();
		}
	}

	void Box::set_border_top_width_direct(float val, bool isRT) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[0] != val) {
			_border->width[0] = val;
			mark_layout(kLayout_Size_Height, isRT);
		}
	}

	void Box::set_border_right_width_direct(float val, bool isRT) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[1] != val) {
			_border->width[1] = val;
			mark_layout(kLayout_Size_Width, isRT);
		}
	}

	void Box::set_border_bottom_width_direct(float val, bool isRT) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[2] != val) {
			_border->width[2] = val;
			mark_layout(kLayout_Size_Height, isRT);
		}
	}

	void Box::set_border_left_width_direct(float val, bool isRT) {
		_BorderAlloc();
		val = Qk_Max(0, val);
		if (_border->width[3] != val) {
			_border->width[3] = val;
			mark_layout(kLayout_Size_Width, isRT);
		}
	}

	void Box::set_background_color_direct(Color color, bool isRT) {
		if (_background_color != color) {
			_background_color = color;
			mark_render();
		}
	}

	Vec2 Box::content_size() const {
		return _container.content;
	}

	void Box::set_background_direct(BoxFilter* val, bool isRT) {
		BoxFilter::assign_atomic(_background, val, this);
	}

	void Box::set_box_shadow_direct(BoxShadow* val, bool isRT) {
		BoxFilter::assign_atomic(reinterpret_cast<std::atomic<BoxFilter*>&>(_box_shadow), val, this);
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
			mark<true>(kVisible_Region);
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
			// Actual world matrix
			Mat matrix = Mat(mat).set_translate(_position);
			solve_visible_area(matrix);
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

	static void call_onChildLayoutChange(Box* self, Box::ChildLayoutChangeMark arg) {
		auto _parent = self->parent();
		if (_parent)
			_parent->onChildLayoutChange(self, arg);
	}

	void Box::set_align_direct(Align align, bool isRT) {
		if (_align != align) {
			_align = align;
			if (isRT) {
				call_onChildLayoutChange(this, kChild_Layout_Align);
			} else {
				_async_call({ call_onChildLayoutChange(self, kChild_Layout_Align); }, 0);
			}
		}
	}

	void Box::set_weight_direct(Vec2 weight, bool isRT) {
		if (_weight != weight) {
			_weight = weight;
			if (isRT) {
				call_onChildLayoutChange(this, kChild_Layout_Weight);
			} else {
				_async_call({ call_onChildLayoutChange(self, kChild_Layout_Weight); }, 0);
			}
		}
	}

	void Box::set_layout_offset(Vec2 val) {
		_layout_offset = val;
		mark<true>(kTransform); // mark recursive transform
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

	ViewType Box::view_type() const {
		return kBox_ViewType;
	}

	Free::Free() {
		_layout = LayoutType::Free; // default free layout
	}

}
