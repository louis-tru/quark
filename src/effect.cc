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

#include "./app.h"
#include "./effect.h"
#include "./pre_render.h"
#include "./layout/box.h"
#include "./render/source.h"
#include "./render/render.h"
#include <math.h>

namespace quark {

	bool Copying::check_loop_reference(Copying* value) {
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

	Copying* Copying::assign2(Copying* left, Copying* right) {
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
						Qk_ASSERT(ok);
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

	Copying::Copying()
		: _next(nullptr)
		, _holder_mode(M_INDEPENDENT)
	{
	}

	Copying::~Copying() {
		if (_next) {
			_next->release();
			_next = nullptr;
		}
	}

	void Copying::set_next2(Copying* value) {
		_next = assign(_next, value);
		if (_next) {
			_next->set_holder_mode(_holder_mode);
		}
		onChange();
	}

	void Copying::set_next(Copying* value) {
		if (value != _next) {
			if (check_loop_reference(value)) {
				Qk_ERR("Box background loop reference error");
			} else {
				set_next2(value);
			}
		} else {
			onChange();
		}
	}

	Copying* Copying::assign(Copying* left, Copying* right) {
		if (left == right) {
			return left;
		} else {
			if (left && right && left->check_loop_reference(right->_next)) {
				Qk_ERR("Box background loop reference error");
				return left;
			} else {
				return assign2(left, right);
			}
		}
	}

	bool Copying::retain() {
		if (_holder_mode == M_DISABLE) {
			return false;
		} else if (_holder_mode == M_INDEPENDENT) {
			if (ref_count() > 0) {
				return false;
			}
		}
		return Reference::retain();
	}

	void Copying::set_holder_mode(HolderMode mode) {
		if (_holder_mode != mode) {
			_holder_mode = mode;
			if (_next) {
				_next->set_holder_mode(mode);
			}
		}
	}

	void Copying::onChange() {
		auto app_ = app();
		// Qk_ASSERT(app_, "Application needs to be initialized first");
		if (app_) {
			app_->pre_render()->mark_none();
		}
	}

	Copying::Type FillImage::type() const { return M_IMAGE; }
	Copying::Type FillGradientLinear::type() const { return M_GRADIENT_Linear; }
	Copying::Type FillGradientRadial::type() const { return M_GRADIENT_Radial; }
	Copying::Type BoxShadow::type() const { return M_SHADOW; }

	// ------------------------------ F i l l . I m a g e ------------------------------

	FillImage::FillImage(): _repeat(Repeat::REPEAT) {}
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

	Copying* FillImage::copy(Copying* to) {
		auto target = (to && to->type() == M_IMAGE) ?
				static_cast<FillImage*>(to) : new FillImage();
		target->set_next2(next());
		target->_repeat = _repeat;
		target->_position_x = _position_x;
		target->_position_y = _position_y;
		target->_size_x = _size_x;
		target->_size_y = _size_y;
		target->set_source(source());
		return target;
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
			case FillSizeKind::PIXEL: out = size.value; break;
			case FillSizeKind::RATIO: out = size.value * host; break;
		}
		return true;
	}

	float FillImage::compute_position(FillPosition pos, float host, float size) {
		float out = 0;
		switch (pos.kind) {
			default: break;
			//case FillPositionType::START: out = 0; break;
			case FillPositionKind::PIXEL: out = pos.value; break;
			case FillPositionKind::RATIO: out = pos.value * host; break;
			case FillPositionKind::END: out = host - size; break;
			case FillPositionKind::CENTER: out = (host - size) / 2; break;
		}
		return out;
	}

	// ------------------------------ F i l l . G r a d i e n t ------------------------------

	FillGradient::FillGradient(const Array<float>& pos, const Array<Color>& colors)
		: _pos(pos)
		, _colors(*reinterpret_cast<const Array<uint32_t>*>(&colors))
	{
	}

	FillGradient::~FillGradient() {}

	void FillGradient::set_positions(const Array<float>& pos) {
		_pos = pos;
		onChange();
	}

	void FillGradient::set_colors(const Array<Color>& colors) {
		_colors = *reinterpret_cast<const Array<uint32_t>*>(&colors);
		onChange();
	}

	FillGradientLinear::FillGradientLinear(float angle, const Array<float>& pos, const Array<Color>& colors)
		: FillGradient(pos, colors)
		, _angle(angle)
	{
		setRadian();
	}

	FillGradientRadial::FillGradientRadial(const Array<float>& pos, const Array<Color>& colors)
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

	Copying* FillGradientLinear::copy(Copying* to) {
		auto target = (to && to->type() == M_GRADIENT_Linear) ?
			static_cast<FillGradientLinear*>(to) : new FillGradientLinear(
				_angle, positions(), colors()
			);
		target->set_next2(next());
		target->_radian = _radian;
		target->_quadrant = _quadrant;
		return target;
	}

	Copying* FillGradientRadial::copy(Copying* to) {
		auto target = (to && to->type() == M_GRADIENT_Radial) ?
			static_cast<FillGradientRadial*>(to) : new FillGradientRadial(
				positions(), colors()
			);
		target->set_next2(next());
		return target;
	}

	// ------------------------------ B o x . S h a d o w ------------------------------

	BoxShadow::BoxShadow() {}
	BoxShadow::BoxShadow(Shadow value): _value(value) {}
	BoxShadow::BoxShadow(float x, float y, float s, Color color): _value{x,y,s,color} {}

	Copying* BoxShadow::copy(Copying* to) {
		auto target = (to && to->type() == M_SHADOW) ?
			static_cast<BoxShadow*>(to): new BoxShadow();
		target->set_next2(next());
		target->_value = _value;
		return target;
	}

}
