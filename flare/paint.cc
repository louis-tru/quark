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
#include "./paint.h"
#include "./display.h"
#include "./pre_render.h"
#include "./layout/box.h"
#include "./render/source.h"
#include "./render/render.h"
#include "skia/core/SkImage.h"

F_NAMESPACE_START

bool PaintBase::check_loop_reference(Paint value) {
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

Paint PaintBase::_Assign(Paint left, Paint right) {
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

void PaintBase::_Set_next(Paint value) {
	_next = assign(_next, value);
	if (_next) {
		_next->set_holder_mode(_holder_mode);
	}
	mark();
}

PaintBase::PaintBase()
	: _next(nullptr)
	, _holder_mode(M_INDEPENDENT)
{
}

PaintBase::~PaintBase() {
	if (_next) {
		_next->release();
		_next = nullptr;
	}
}

Paint PaintBase::set_next(Paint value) {
	if (value != _next) {
		if (check_loop_reference(value)) {
			F_ERR("Box background loop reference error");
		} else {
			_Set_next(value);
		}
	} else {
		mark();
	}
	return this;
}

Paint PaintBase::assign(Paint left, Paint right) {
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

bool PaintBase::retain() {
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
Paint PaintBase::set_holder_mode(HolderMode mode) {
	if (_holder_mode != mode) {
		_holder_mode = mode;
		if (_next) {
			_next->set_holder_mode(mode);
		}
	}
	return this;
}

void PaintBase::mark() {
	auto app_ = app();
	// F_ASSERT(app_, "Application needs to be initialized first");
	if (app_) {
		app_->pre_render()->mark_none();
	}
}

PaintBase::Type PaintBase::type() const { return M_INVALID; }
PaintBase::Type PaintColor::type() const { return M_COLOR; }
PaintBase::Type PaintImage::type() const { return M_IMAGE; }
PaintBase::Type PaintGradient::type() const { return M_GRADIENT; }
PaintBase::Type PaintShadow::type() const { return M_SHADOW; }
PaintBase::Type PaintBorder::type() const { return M_BORDER; }

// ------------------------------ F i l l C o l o r ------------------------------

PaintColor::PaintColor(Color color): _color(color) {}

void PaintColor::set_color(Color value) {
	if (_color != value) {
		_color = value;
		mark();
	}
}

Paint PaintColor::copy(Paint to) {
	auto target = (to && to->type() == M_COLOR) ?
		static_cast<PaintColor*>(to) : new PaintColor();
	target->_color = _color;
	_Set_next(_next);
	return target;
}

Paint PaintColor::WHITE( NewRetain<PaintColor>(Color(255,255,255,255))->set_holder_mode(M_SHARED) );
Paint PaintColor::BLACK( NewRetain<PaintColor>(Color(0,0,0,255))->set_holder_mode(M_SHARED) );
Paint PaintColor::BLUE( NewRetain<PaintColor>(Color(0,1,0,255))->set_holder_mode(M_SHARED) );

// ------------------------------ F i l l I m a g e ------------------------------

PaintImage::PaintImage(cString& src)
	: _repeat(Repeat::REPEAT)
{
	if (!src.is_empty()) {
		set_src(src);
	}
}

Paint PaintImage::copy(Paint to) {
	PaintImage* target = (to && to->type() == M_IMAGE) ?
			static_cast<PaintImage*>(to) : new PaintImage();
	target->_repeat = _repeat;
	target->_position_x = _position_x;
	target->_position_y = _position_y;
	target->_size_x = _size_x;
	target->_size_y = _size_y;
	target->set_source(source());
	_Set_next(_next);
	return target;
}

void PaintImage::set_repeat(Repeat value) {
	if (_repeat != value) {
		_repeat = value;
		mark();
	}
}

void PaintImage::set_position_x(PaintPosition value) {
	if (value != _position_x) {
		_position_x = value;
		mark();
	}
}

void PaintImage::set_position_y(PaintPosition value) {
	if (value != _position_y) {
		_position_y = value;
		mark();
	}
}

void PaintImage::set_size_x(PaintSize value) {
	if (value != _size_x) {
		_size_x = value;
		mark();
	}
}

void PaintImage::set_size_y(PaintSize value) {
	if (value != _size_y) {
		_size_y = value;
		mark();
	}
}

PaintGradient::PaintGradient()
{
}

Paint PaintGradient::copy(Paint to) {
	PaintGradient* target = (to && to->type() == M_GRADIENT) ?
		static_cast<PaintGradient*>(to) : new PaintGradient();
	// TODO ..
	_Set_next(_next);
	return target;
}

Paint PaintShadow::copy(Paint to) {
	return nullptr;
}

Paint PaintBorder::copy(Paint to) {
	return nullptr;
}

bool PaintImage::solve_size(FillSize size, float host, float& out) {
	switch (size.type) {
		default: return false; // AUTO
		case FillSizeType::PIXEL: out = size.value; break;
		case FillSizeType::RATIO: out = size.value * host; break;
	}
	return true;
}

float PaintImage::solve_position(FillPosition pos, float host, float size) {
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

F_NAMESPACE_END