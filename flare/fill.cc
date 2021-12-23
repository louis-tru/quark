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
#include "./image_source.h"
#include "./display.h"
#include "./pre_render.h"
#include "./layout/box.h"
#include "./render/render.h"
#include "./util/fs.h"
#include "skia/core/SkImage.h"

namespace flare {

	F_DEFINE_INLINE_MEMBERS(FillBox, Inl) {
		#define _inl(self) static_cast<FillBox::Inl*>(static_cast<Fill>(self))
	 public:
		
		bool check_loop_reference(Fill value) {
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
		
		static Fill assign(Fill left, Fill right) {
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
		
		void set_next(Fill value) {
			_next = assign(_next, value);
			if (_next) {
				_next->set_holder_mode(_holder_mode);
			}
			mark();
		}

	};

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
			if (_inl(this)->check_loop_reference(value)) {
				F_ERR("Box background loop reference error");
			} else {
				_inl(this)->set_next(value);
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
			if (left && right && _inl(left)->check_loop_reference(right->_next)) {
				F_ERR("Box background loop reference error");
				return left;
			} else {
				return Inl::assign(left, right);
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
	FillBox::Type FillBorderRadius::type() const { return M_BORDER_RADIUS; }

	FillColor::FillColor(Color color): _color(color) {
	}

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
		_inl(target)->set_next(_next);
		return target;
	}

	Fill FillColor::WHITE( NewRetain<FillColor>(Color(255,255,255,255))->set_holder_mode(M_SHARED) );
	Fill FillColor::BLACK( NewRetain<FillColor>(Color(0,0,0,255))->set_holder_mode(M_SHARED) );
	Fill FillColor::BLUE( NewRetain<FillColor>(Color(0,1,0,255))->set_holder_mode(M_SHARED) );

	F_DEFINE_INLINE_MEMBERS(FillImage, Inl) {
	 public:
		#define _inl_img(self) static_cast<FillImage::Inl*>(self)

		void source_change_handle(Event<ImageSource, ImageSource::State>& evt) { // 收到图像变化通知
			if (*evt.data() & ImageSource::STATE_DECODE_COMPLETE) {
				mark();
			}
		}
	};


	FillImage::FillImage(cString& src)
		: _repeat(Repeat::REPEAT)
	{
		if (!src.is_empty()) {
			set_src(src);
		}
	}

	FillImage::~FillImage() {
		if (_source) {
			_source->F_Off(State, &Inl::source_change_handle, _inl_img(this));
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
		target->_source = _source;
		_inl(target)->set_next(_next);
		return target;
	}

	String FillImage::src() const {
		return _source ? _source->id(): String();
	}

	void FillImage::set_src(cString& value) {
		auto source = new ImageSource(value); // TODO ...
		set_source(source);
	}

	void FillImage::set_source(ImageSource* source) {
		if (_source.value() != source) {
			if (_source) {
				_source->F_Off(State, &Inl::source_change_handle, _inl_img(this));
			}
			if (source) {
				source->F_On(State, &Inl::source_change_handle, _inl_img(this));
			}
			_source = Handle<ImageSource>(source);
		}
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
		_inl(target)->set_next(_next);
		return target;
	}

	// ------------------------------ draw ------------------------------

	static SkRect MakeSkRectFrom(Box *host) {
		auto o = host->transform_origin();
		auto s = host->client_size();
		return {o[0], o[1], s[0], s[1]};
	}

	void FillColor::draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) {
		if (_color.a()) {
 			SkPaint paint;
			paint.setColor(_color.to_uint32_argb_from(alpha));
			if (radius) {
				// TODO ...
			} else {
				canvas->drawRect(MakeSkRectFrom(host), paint);
			}
		}
		if (_next)
			_next->draw(host, canvas, alpha, radius);
	}

	SkImage* CastSkImage(ImageSource* img);

	void FillImage::draw(Box *host, Canvas *canvas, uint8_t alpha, FillBorderRadius *radius) {
		if (_source && _source->ready()) {
			if (radius) {
				// TODO ...
			} else {
				auto skimg = CastSkImage(_source.value());
				canvas->drawImageRect(skimg, MakeSkRectFrom(host), {});
			}
		}
		if (_next)
			_next->draw(host, canvas, alpha, radius);
	}

	void FillBorderRadius::draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) {
		if (_next)
			_next->draw(host, canvas, alpha, this);
	}

}
