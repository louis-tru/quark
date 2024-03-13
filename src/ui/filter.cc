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

#include "./filter.h"
#include "./layout/box.h"
#include "./app.h"
#include "../render/source.h"
#include "../render/render.h"
#include <math.h>

namespace qk {

	template<typename T>
	inline T transition_value(T v1, T v2, float t) {
		auto v = v1 - (v1 - v2) * t;
		return v;
	}

	Repeat transition_value(Repeat v1, Repeat v2, float t) {
		return t < 1.0 ? v1: v2;
	}

	FillPosition transition_value(FillPosition v1, FillPosition v2, float t) {
		if ( v1.kind == v2.kind ) {
			return { transition_value(v1.value, v2.value, t), v2.kind };
		} else {
			return  t < 1.0 ? v1 : v2;
		}
	}

	FillSize transition_value(FillSize v1, FillSize v2, float t) {
		if ( v1.kind == v2.kind ) {
			return { transition_value(v1.value, v2.value, t), v2.kind };
		} else {
			return  t < 1.0 ? v1 : v2;
		}
	}

	Color4f transition_value(Color4f v1, Color4f v2, float t) {
		return Color4f(
			transition_value(v1[0], v2[0], t),
			transition_value(v1[1], v2[1], t),
			transition_value(v1[2], v2[2], t),
			transition_value(v1[3], v2[3], t)
		);
	}

	template<>
	Shadow transition_value(Shadow v1, Shadow v2, float t);

	// ---------------------------------------------------------------------------------------------

	BoxFilter::BoxFilter(): _next(nullptr), _host(nullptr)
	{}

	BoxFilter::~BoxFilter() {
		if (_next) {
			_next->release();
			_next = nullptr;
		}
	}

	bool BoxFilter::check_loop_reference(BoxFilter* value) {
		while (value) {
			if (value == this) {
				Qk_ERR("Box filter loop reference error");
				return true;
			}
			value = value->_next;
		}
		return false;
	}

	static BoxFilter* assign_no_check(BoxFilter* left, BoxFilter* right) {
		if (right) {
			if (left == right) {
				return left;
			} else {
				auto new_left = right->copy(left); // copy
				if (new_left != left) {
					if (left) {
						left->release();
					}
				}
				return new_left;
			}
		} else {
			if (left) {
				left->release();
			}
			return nullptr;
		}
	}

	BoxFilter* BoxFilter::assign(BoxFilter* left, BoxFilter* right, Layout *host) {
		if (left != right) {
			if (left && right && left->check_loop_reference(right->_next)) {
				return left;
			} else {
				left = assign_no_check(left, right);
			}
		}
		left->set_host(host);
		return left;
	}

	void BoxFilter::set_next_no_check(BoxFilter* value) {
		_next = assign_no_check(_next, value);
		onChange();
	}

	void BoxFilter::set_next(BoxFilter* value) {
		if (_next != value) {
			if (!check_loop_reference(value)) {
				set_next_no_check(value);
			}
		}
	}

	void BoxFilter::set_host(Layout* host) {
		if (_host != host) {
			_host = host;
			if (_next) {
				_next->set_host(host);
			}
		}
	}

	void BoxFilter::onChange() {
		if (_host) {
			_host->mark_render();
		}
	}

	BoxFilter::Type FillImage::type() const { return kImage; }
	BoxFilter::Type FillGradientLinear::type() const { return kGradientLinear; }
	BoxFilter::Type FillGradientRadial::type() const { return kGradientRadial; }
	BoxFilter::Type BoxShadow::type() const { return kShadow; }

	// ------------------------------ F i l l . I m a g e ------------------------------

	FillImage::FillImage(): _repeat(Repeat::kRepeat) {}
	FillImage::FillImage(cString& src, Init init)
		: _size_x(init.size_x)
		, _size_y(init.size_y)
		, _position_x(init.position_x)
		, _position_y(init.position_y)
		, _repeat(init.repeat)
	{
		if (!src.isEmpty()) {
			set_src(src);
		}
	}

	BoxFilter* FillImage::copy(BoxFilter* dest) {
		auto target = (dest && dest->type() == kImage) ?
				static_cast<FillImage*>(dest) : new FillImage();
		target->set_next_no_check(next());
		target->_repeat = _repeat;
		target->_position_x = _position_x;
		target->_position_y = _position_y;
		target->_size_x = _size_x;
		target->_size_y = _size_y;
		target->set_source(source());
		return target;
	}

	BoxFilter* FillImage::transition(BoxFilter *to, float t, BoxFilter* dest) {
		if (to && dest && to->type() == kImage && dest->type() == kImage) {
			auto target = static_cast<FillImage*>(dest);
			auto to1 = static_cast<FillImage*>(to);
			if (_next)
				_next->transition(to->next(), t, dest->next());
			target->_repeat = transition_value(_repeat, to1->_repeat, t);
			target->_position_x = transition_value(_position_x, to1->_position_x, t);
			target->_position_y = transition_value(_position_y, to1->_position_y, t);
			target->_size_x = transition_value(_size_x, to1->_size_x, t);
			target->_size_y = transition_value(_size_y, to1->_size_y, t);
			target->set_source(t < 1.0 ? source(): to1->source());
		}
		return dest;
	}

	void FillImage::set_repeat(Repeat value) {
		if (_repeat != value) {
			_repeat = value;
			onChange();
		}
	}

	void FillImage::set_position_x(FillPosition value) {
		if (value != _position_x) {
			_position_x = value;
			onChange();
		}
	}

	void FillImage::set_position_y(FillPosition value) {
		if (value != _position_y) {
			_position_y = value;
			onChange();
		}
	}

	void FillImage::set_size_x(FillSize value) {
		if (value != _size_x) {
			_size_x = value;
			onChange();
		}
	}

	void FillImage::set_size_y(FillSize value) {
		if (value != _size_y) {
			_size_y = value;
			onChange();
		}
	}

	bool FillImage::compute_size(FillSize size, float host, float& out) {
		switch (size.kind) {
			default: return false; // AUTO
			case FillSizeKind::kPixel: out = size.value; break;
			case FillSizeKind::kRatio: out = size.value * host; break;
		}
		return true;
	}

	float FillImage::compute_position(FillPosition pos, float host, float size) {
		float out = 0;
		switch (pos.kind) {
			default: break;
			//case FillPositionType::START: out = 0; break;
			case FillPositionKind::kPixel: out = pos.value; break;
			case FillPositionKind::kRatio: out = pos.value * host; break;
			case FillPositionKind::kEnd: out = host - size; break;
			case FillPositionKind::kCenter: out = (host - size) / 2; break;
		}
		return out;
	}

	// ------------------------------ F i l l . G r a d i e n t ------------------------------

	Array<Color4f> FillGradient::to_color4fs(cArray<Color>& colors) {
		Array<Color4f> _colors(colors.length());
		for (int i = 0, l = colors.length(); i <  l; i++) {
			_colors[i] = colors[i].to_color4f();
		}
		Qk_ReturnLocal(_colors);
	}

	FillGradient::FillGradient(cArray<float>& pos, cArray<Color4f>& colors)
		: _pos(pos)
		, _colors(colors)
	{
	}

	void FillGradient::set_positions(cArray<float>& pos) {
		_pos = pos;
		onChange();
	}

	void FillGradient::set_colors(cArray<Color4f>& colors) {
		_colors = std::move(colors);
		onChange();
	}

	bool FillGradient::transition_gradient(BoxFilter *to, float t, BoxFilter* dest) {
		if (to && dest && to->type() == type() && dest->type() == type()) {
			auto target = static_cast<FillGradient*>(dest);
			auto to1 = static_cast<FillGradient*>(to);
			if (_next)
				_next->transition(to->next(), t, dest->next());

			target->set_positions(positions());
			target->set_colors(colors());

			auto posLen = Uint32::min(
				target->_pos.length(), 
				Uint32::min(_pos.length(), to1->_pos.length())
			);
			auto colorLen = Uint32::min(
				target->_colors.length(), 
				Uint32::min(_colors.length(), to1->_colors.length())
			);
			for (auto i = 0; i < posLen; i++) {
				target->_pos[i] = transition_value(_pos[i], to1->_pos[i], t);
			}
			for (auto i = 0; i < colorLen; i++) {
				target->_colors[i] = transition_value(_colors[i], to1->_colors[i], t);
			}
			return true;
		}
		return false;
	}

	FillGradientLinear::FillGradientLinear(
		float angle, cArray<float>& pos, cArray<Color4f>& colors
	)
		: FillGradient(pos, colors)
		, _angle(angle)
	{
		setRadian();
	}

	FillGradientRadial::FillGradientRadial(cArray<float>& pos, cArray<Color4f>& colors)
		: FillGradient(pos, colors)
	{}

	void FillGradientLinear::setRadian() {
		float angle = _angle + 90;
		_radian = angle * Qk_PI_RATIO_180;
		_quadrant = int(angle * 0.0111111111111111111) % 4;
	}

	void FillGradientLinear::set_angle(float val) {
		if (val != _angle) {
			_angle = val;
			setRadian();
			onChange();
		}
	}

	BoxFilter* FillGradientLinear::copy(BoxFilter* dest) {
		auto target = (dest && dest->type() == kGradientLinear) ?
			static_cast<FillGradientLinear*>(dest): new FillGradientLinear(_angle, positions(), colors());
		target->set_next_no_check(next());
		target->_radian = _radian;
		target->_quadrant = _quadrant;
		return target;
	}

	BoxFilter* FillGradientRadial::copy(BoxFilter* dest) {
		auto target = (dest && dest->type() == kGradientRadial) ?
			static_cast<FillGradientRadial*>(dest): new FillGradientRadial(positions(), colors());
		target->set_next_no_check(next());
		return target;
	}

	BoxFilter* FillGradientLinear::transition(BoxFilter *to, float t, BoxFilter* dest) {
		if (FillGradient::transition_gradient(to, t, dest)) {
			auto to1 = static_cast<FillGradientLinear*>(to);
			auto target = static_cast<FillGradientLinear*>(dest);
			target->_angle = transition_value(_angle, to1->_angle, t);
			target->setRadian();
		}
		return dest;
	}

	BoxFilter* FillGradientRadial::transition(BoxFilter *to, float t, BoxFilter* dest) {
		FillGradient::transition_gradient(to, t, dest);
		return dest;
	}

	// ------------------------------ B o x . S h a d o w ------------------------------

	BoxShadow::BoxShadow() {}
	BoxShadow::BoxShadow(Shadow value): _value(value) {}
	BoxShadow::BoxShadow(float x, float y, float s, Color color): _value{x,y,s,color} {}

	BoxFilter* BoxShadow::copy(BoxFilter* dest) {
		auto target = (dest && dest->type() == kShadow) ?
			static_cast<BoxShadow*>(dest): new BoxShadow();
		target->set_next_no_check(next());
		target->_value = _value;
		return target;
	}

	BoxFilter* BoxShadow::transition(BoxFilter *to, float t, BoxFilter* dest) {
		if (to && dest && to->type() == kShadow && dest->type() == kShadow) {
			auto target = static_cast<BoxShadow*>(dest);
			auto to1 = static_cast<BoxShadow*>(to);
			if (_next)
				_next->transition(to->next(), t, dest->next());
			target->_value = transition_value(_value, to1->_value, t);
		}
		return dest;
	}
}
