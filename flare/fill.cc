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
#include "./fill.h"
#include "./source.h"
#include "./display.h"
#include "./pre_render.h"
#include "./layout/box.h"
#include "./render/render.h"
#include "skia/core/SkImage.h"

namespace flare {

	bool FillBox::check_loop_reference(Fill value) {
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
	
	Fill FillBox::_Assign(Fill left, Fill right) {
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
	
	void FillBox::_Set_next(Fill value) {
		_next = assign(_next, value);
		if (_next) {
			_next->set_holder_mode(_holder_mode);
		}
		mark();
	}

	FillBox::FillBox()
		: _next(nullptr)
		, _holder_mode(M_INDEPENDENT)
	{
	}

	FillBox::~FillBox() {
		if (_next) {
			_next->release();
			_next = nullptr;
		}
	}

	Fill FillBox::set_next(Fill value) {
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

	Fill FillBox::assign(Fill left, Fill right) {
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

	bool FillBox::retain() {
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
	Fill FillBox::set_holder_mode(HolderMode mode) {
		if (_holder_mode != mode) {
			_holder_mode = mode;
			if (_next) {
				_next->set_holder_mode(mode);
			}
		}
		return this;
	}

	void FillBox::mark() {
		auto app_ = app();
		// F_ASSERT(app_, "Application needs to be initialized first");
		if (app_) {
			app_->pre_render()->mark_none();
		}
	}

	FillBox::Type FillBox::type() const { return M_INVALID; }
	FillBox::Type FillColor::type() const { return M_COLOR; }
	FillBox::Type FillImage::type() const { return M_IMAGE; }
	FillBox::Type FillGradient::type() const { return M_GRADIENT; }
	FillBox::Type FillShadow::type() const { return M_SHADOW; }
	FillBox::Type FillBorder::type() const { return M_BORDER; }

	// ------------------------------ F i l l C o l o r ------------------------------

	FillColor::FillColor(Color color): _color(color) {}

	void FillColor::set_color(Color value) {
		if (_color != value) {
			_color = value;
			mark();
		}
	}

	Fill FillColor::copy(Fill to) {
		auto target = (to && to->type() == M_COLOR) ?
			static_cast<FillColor*>(to) : new FillColor();
		target->_color = _color;
		_Set_next(_next);
		return target;
	}

	Fill FillColor::WHITE( NewRetain<FillColor>(Color(255,255,255,255))->set_holder_mode(M_SHARED) );
	Fill FillColor::BLACK( NewRetain<FillColor>(Color(0,0,0,255))->set_holder_mode(M_SHARED) );
	Fill FillColor::BLUE( NewRetain<FillColor>(Color(0,1,0,255))->set_holder_mode(M_SHARED) );

	// ------------------------------ F i l l I m a g e ------------------------------

	FillImage::FillImage(cString& src)
		: _repeat(Repeat::NONE)
	{
		if (!src.is_empty()) {
			set_src(src);
		}
	}

	Fill FillImage::copy(Fill to) {
		FillImage* target = (to && to->type() == M_IMAGE) ?
				static_cast<FillImage*>(to) : new FillImage();
		target->_repeat = _repeat;
		target->_position_x = _position_x;
		target->_position_y = _position_y;
		target->_size_x = _size_x;
		target->_size_y = _size_y;
		target->set_source(source());
		_Set_next(_next);
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

	FillGradient::FillGradient()
	{
	}

	Fill FillGradient::copy(Fill to) {
		FillGradient* target = (to && to->type() == M_GRADIENT) ?
			static_cast<FillGradient*>(to) : new FillGradient();
		// TODO ..
		_Set_next(_next);
		return target;
	}

	// ------------------------------ d r a w ------------------------------

	SkImage* CastSkImage(ImageSource* img);

	SkRect MakeSkRectFrom(Box *host) {
		auto begin = host->transform_origin(); // begin
		auto end = host->client_size() - begin; // end
		return {-begin.x(), -begin.y(), end.x(), end.y()};
	}

	void FillColor::draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) {
		if (full && _color.a()) {
			SkPaint paint;
			paint.setColor(_color.to_uint32_argb_from(alpha));
			if (host->is_radius()) {
				// TODO ...
			} else {
				canvas->drawRect(MakeSkRectFrom(host), paint);
			}
		}
		if (_next)
			_next->draw(host, canvas, alpha, full);
	}

	bool FillImage::solve_size(FillSize size, float host, float& out) {
		switch (size.type) {
			default: return false; // AUTO
			case FillSizeType::PIXEL: out = size.value; break;
			case FillSizeType::RATIO: out = size.value * host; break;
		}
		return true;
	}

	float FillImage::solve_position(FillPosition pos, float host, float size) {
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

	void FillImage::draw(Box *host, Canvas *canvas, uint8_t alpha, bool full) {
		if (0 && full && source() && source()->ready()) {
			auto src = source();
			auto img = CastSkImage(src);
			SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

			auto ori = host->transform_origin(); // begin
			auto cli = host->client_size();
			auto src_w = src->width(), src_h = src->height();
			auto cli_x = cli.x(), cli_y = cli.y();
			float w, h, x, y;
			
			if (solve_size(_size_x, cli_x, w)) { // ok x
				if (!solve_size(_size_y, cli_y, h)) { // auto y
					h = w / src_w * src_h;
				}
			} else if (solve_size(_size_y, cli_y, h)) { // auto x, ok y
				w = h / src_h * src_w;
			} else { // auto x,y
				w = src_w;
				h = src_h;
			}
			
			auto min = [] (float a, float b) { return a < b ? a: b; };
			auto max = [] (float a, float b) { return a > b ? a: b; };
			auto scale_x = w / src_w;
			auto scale_y = h / src_h;
			
			x = solve_position(_position_x, cli_x, w) - ori.x();
			y = solve_position(_position_y, cli_y, h) - ori.y();
			
			cli_x -= ori.x();
			cli_y -= ori.y();

			if (_repeat == Repeat::NONE) {
				
				if (x < cli_x) {
					if (y < cli_y) {
						auto __x = max(x, -ori.x()), __y = max(y, -ori.y());
						auto __x2 = min(x + w, cli_x), __y2 = min(y + h, cli_y);
						// auto src_x = __x < 0
						canvas->drawImageRect(img, { 0, 0, w, h}, {__x,__y,__x2,__y2}, opts, nullptr, Canvas::kFast_SrcRectConstraint);
					}
				}
				
			} else {
				bool repeat_x = _repeat == Repeat::REPEAT || _repeat == Repeat::REPEAT_X;
				bool repeat_y = _repeat == Repeat::REPEAT || _repeat == Repeat::REPEAT_Y;
				
				for (auto _x = x; _x < cli_x; _x += w) {
					for (auto _y = y; _y < cli_y; _y += h) {
						auto __x = max(_x, -ori.x()), __y = max(_y, -ori.y());
						auto __x2 = min(_x + w, cli_x), __y2 = min(_y + h, cli_y);

						canvas->drawImageRect(img, {__x,__y,__x2,__y2}, opts);

						if (!repeat_y) break;
					}
					if (!repeat_x) break;
				}
			}

			// canvas->drawImageRect(img, rect, opts);
		}

		if (_next)
			_next->draw(host, canvas, alpha, full);
	}

}
