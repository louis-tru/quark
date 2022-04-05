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

#include "./app.h"
#include "./effect.h"
#include "./pre_render.h"
#include "./layout/box.h"
#include "./render/source.h"
#include "./render/render.h"

F_NAMESPACE_START

bool Effect::check_loop_reference(Effect* value) {
	if (value) {
		auto v = value;
		do {
			if (v == this) {
				return true;
			}
			v = v->_next;
		} while (v);
	}
	return false;
}

Effect* Effect::_Assign(Effect* left, Effect* right) {
	if (right) {
		if (left == right) {
			return left;
		} else {
			if (right->retain()) {
				if (left) {
					left->release();
				}
				return right;
			} else { // copy
				auto new_left = right->copy(left);
				if (new_left != left) {
					if (left) {
						left->release();
					}
					bool ok = new_left->retain();
					F_ASSERT(ok);
				}
				return new_left;
			}
		}
	} else {
		if (left) {
			left->release();
		}
		return nullptr;
	}
}

void Effect::_set_next(Effect* value) {
	_next = assign(_next, value);
	if (_next) {
		_next->set_holder_mode(_holder_mode);
	}
	mark();
}

Effect::Effect()
	: _next(nullptr)
	, _holder_mode(M_INDEPENDENT)
{
}

Effect::~Effect() {
	if (_next) {
		_next->release();
		_next = nullptr;
	}
}

Effect* Effect::set_next(Effect* value) {
	if (value != _next) {
		if (check_loop_reference(value)) {
			F_ERR("Box background loop reference error");
		} else {
			_set_next(value);
		}
	} else {
		mark();
	}
	return this;
}

Effect* Effect::assign(Effect* left, Effect* right) {
	if (left == right) {
		return left;
	} else {
		if (left && right && left->check_loop_reference(right->_next)) {
			F_ERR("Box background loop reference error");
			return left;
		} else {
			return _Assign(left, right);
		}
	}
}

bool Effect::retain() {
	if (_holder_mode == M_DISABLE) {
		return false;
	} else if (_holder_mode == M_INDEPENDENT) {
		if (ref_count() > 0) {
			return false;
		}
	}
	return Reference::retain();
}

/**
* @func set_holder_mode(mode)
*/
Effect* Effect::set_holder_mode(HolderMode mode) {
	if (_holder_mode != mode) {
		_holder_mode = mode;
		if (_next) {
			_next->set_holder_mode(mode);
		}
	}
	return this;
}

void Effect::mark() {
	auto app_ = app();
	// F_ASSERT(app_, "Application needs to be initialized first");
	if (app_) {
		app_->pre_render()->mark_none();
	}
}

Effect::Type BoxShadow::type() const { return M_SHADOW; }
Effect::Type FillImage::type() const { return M_IMAGE; }
Effect::Type FillGradientLinear::type() const { return M_GRADIENT_Linear; }
Effect::Type FillGradientRadial::type() const { return M_GRADIENT_Radial; }

// ------------------------------ B o x . S h a d o w ------------------------------

BoxShadow::BoxShadow() {}
BoxShadow::BoxShadow(Shadow value): _value(value) {}
BoxShadow::BoxShadow(float x, float y, float s, Color color): _value{x,y,s,color} {}

Effect* BoxShadow::copy(Effect* to) {
	auto target = (to && to->type() == M_SHADOW) ?
		static_cast<BoxShadow*>(to): new BoxShadow();
	target->_value = _value;
	target->_set_next(_next);
	return target;
}

// ------------------------------ F i l l . I m a g e ------------------------------

FillImage::FillImage(): _repeat(Repeat::NONE) {}
FillImage::FillImage(cString& src, Init init)
	: _size_x(init.size_x)
	, _size_y(init.size_y)
	, _position_x(init.position_x)
	, _position_y(init.position_y)
	, _repeat(init.repeat)
{
	if (!src.is_empty()) {
		set_src(src);
	}
}

Effect* FillImage::copy(Effect* to) {
	auto target = (to && to->type() == M_IMAGE) ?
			static_cast<FillImage*>(to) : new FillImage();
	target->_repeat = _repeat;
	target->_position_x = _position_x;
	target->_position_y = _position_y;
	target->_size_x = _size_x;
	target->_size_y = _size_y;
	target->set_source(source());
	target->_set_next(_next);
	return target;
}

void FillImage::set_repeat(Repeat value) {
	if (_repeat != value) {
		_repeat = value;
		mark();
	}
}

void FillImage::set_position_x(FillPosition value) {
	if (value != _position_x) {
		_position_x = value;
		mark();
	}
}

void FillImage::set_position_y(FillPosition value) {
	if (value != _position_y) {
		_position_y = value;
		mark();
	}
}

void FillImage::set_size_x(FillSize value) {
	if (value != _size_x) {
		_size_x = value;
		mark();
	}
}

void FillImage::set_size_y(FillSize value) {
	if (value != _size_y) {
		_size_y = value;
		mark();
	}
}

bool FillImage::compute_size(FillSize size, float host, float& out) {
	switch (size.type) {
		default: return false; // AUTO
		case FillSizeType::PIXEL: out = size.value; break;
		case FillSizeType::RATIO: out = size.value * host; break;
	}
	return true;
}

float FillImage::compute_position(FillPosition pos, float host, float size) {
	float out = 0;
	switch (pos.type) {
		default: break;
		//case FillPositionType::START: out = 0; break;
		case FillPositionType::PIXEL: out = pos.value; break;
		case FillPositionType::RATIO: out = pos.value * host; break;
		case FillPositionType::END: out = host - size; break;
		case FillPositionType::CENTER: out = (host - size) / 2; break;
	}
	return out;
}

// ------------------------------ F i l l . G r a d i e n t ------------------------------

FillGradient::FillGradient(const Array<float>& pos, const Array<Color>& colors)
	: _pos(pos)
	, _colors(*reinterpret_cast<const Array<uint32_t>*>(&colors))
{
}

void FillGradient::set_positions(const Array<float>& pos) {
	_pos = pos;
	mark();
}

void FillGradient::set_colors(const Array<Color>& colors) {
	_colors = *reinterpret_cast<const Array<uint32_t>*>(&colors);
	mark();
}

FillGradientLinear::FillGradientLinear(Vec2 start, Vec2 end, const Array<float>& pos, const Array<Color>& colors)
	: FillGradient(pos, colors)
	, _start(start), _end(end)
{}

FillGradientRadial::FillGradientRadial(Vec2 center, float radius, const Array<float>& pos, const Array<Color>& colors)
	: FillGradient(pos, colors)
	, _center(center), _radius(radius)
{}

void FillGradientLinear::set_start(Vec2 val) {
	if (val != _start) {
		_start = val;
		mark();
	}
}

void FillGradientLinear::set_end(Vec2 val) {
	if (val != _end) {
		_end = val;
		mark();
	}
}

void FillGradientRadial::set_center(Vec2 val) {
	if (val != _center) {
		_center = val;
		mark();
	}
}

void FillGradientRadial::set_radius(float val) {
	if (val != _radius) {
		_radius = val;
		mark();
	}
}

Effect* FillGradientLinear::copy(Effect* to) {
	auto target = (to && to->type() == M_GRADIENT_Linear) ?
		static_cast<FillGradientLinear*>(to) : new FillGradientLinear(
			_start, _end, positions(), colors()
		);
	target->_set_next(_next);
	return target;
}

Effect* FillGradientRadial::copy(Effect* to) {
	auto target = (to && to->type() == M_GRADIENT_Radial) ?
		static_cast<FillGradientRadial*>(to) : new FillGradientRadial(
			_center, _radius, positions(), colors()
		);
	target->_set_next(_next);
	return target;
}

F_NAMESPACE_END
