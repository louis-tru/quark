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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT viewS AND CONTRIBUTORS "AS IS" AND
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
#include "./view/box.h"
#include "./app.h"
#include "./window.h"
#include "../render/source.h"
#include "../render/render.h"
#include <math.h>

namespace qk {

	template<typename T>
	inline T transition_value(T v1, T v2, float t) {
		auto v = v1 - (v1 - v2) * t;
		return v;
	}

	template<>
	Repeat transition_value(Repeat v1, Repeat v2, float t) {
		return t < 1.0 ? v1: v2;
	}

	template<>
	FillPosition transition_value(FillPosition v1, FillPosition v2, float t) {
		if ( v1.kind == v2.kind ) {
			return { transition_value(v1.value, v2.value, t), v2.kind };
		} else {
			return  t < 1.0 ? v1 : v2;
		}
	}

	template<>
	FillSize transition_value(FillSize v1, FillSize v2, float t) {
		if ( v1.kind == v2.kind ) {
			return { transition_value(v1.value, v2.value, t), v2.kind };
		} else {
			return  t < 1.0 ? v1 : v2;
		}
	}

	template<>
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

	static bool check_loop_ref(BoxFilter *self, BoxFilter *value) {
		auto next = value->next();
		while (next) {
			if (self == next) {
				return Qk_ERR("check_loop_ref(), Box filter loop reference error"), true;
			}
			next = next->next();
		}
		return false;
	}

	static View* safe_view(View *view) {
		//return dynamic_cast<View*>(view);
		return view;
	}

	static void mark(BoxFilter *self, bool isRt) {
		auto view = self->view();
		if (view)
			view->mark(0,isRt);
	}

	BoxFilter* BoxFilter::safe_filter(BoxFilter *filter) {
		//return filter && filter->_safe_mark == 0xff00ffaa ? filter: nullptr;
		return filter;
	}

	BoxFilter::BoxFilter()
		: _view(nullptr), _next(nullptr), _safe_mark(0xff00ffaa), _isHolder(false)
	{}

	void BoxFilter::release() {
		if (_next) {
			_next->release();
			_next = nullptr;
		}
		if (_view) {
			// Ensure that the handle is held safely by the main thread,
			// Only release on the main thread when there is a view
			_view->preRender().post(Cb([this](auto&e) {
				Object::release();
			}));
			_view = nullptr; // clear view
		}
	}

	BoxFilter* BoxFilter::assign_Rt(BoxFilter *left, BoxFilter *right, View *holder) {
		if (right) {
			if (left != right) {
				if (right->_view && right->_view != holder) {
					return Qk_ERR("BoxFilter#assign, right->_view != holder arg"), left;
				}
				auto new_left = right;
				if (right->_isHolder) {
					new_left = right->copy_Rt(left); // copy
				}
				if (new_left != left) {
					Release(left);
				}
				left = new_left;
			}
			left->set_view_Rt(holder);
		} else { // right nullptr
			Release(left);
			left = nullptr;
		}
		if (holder) {
			holder->mark(0,true);
		}
		return left;
	}

	void BoxFilter::set_next_Rt(BoxFilter *next) {
		_next = assign_Rt(_next, next, _view);
	}

	BoxFilter* BoxFilter::next() {
		return safe_filter(_next);
	}

	void BoxFilter::set_next(BoxFilter *next) {
		auto view = safe_view(_view); // safe get
		if (view) {
			if (next && next->_view) {
				if (next->_view != view) // disable
					return Qk_ERR("BoxFilter#set_next, next->_view != _view");
			}
			view->preRender().async_call([](auto ctx, auto arg) {
				if (!check_loop_ref(ctx, arg.arg))
					ctx->set_next_Rt(arg.arg);
			}, this, next);
		} else { // view nullptr
			if (next && !next->_view) {
				if (check_loop_ref(this, next))
					return;
			}
			_next = assign_Rt(_next, next, nullptr);
		}
	}

	void BoxFilter::set_view_Rt(View* value) {
		if (value != _view) {
			_view = value;
			if (_next)
				_next->set_view_Rt(value);
		}
		_isHolder = true;
	}

	BoxFilter::Type FillImage::type() const {
		return kImage;
	}
	BoxFilter::Type FillGradientLinear::type() const {
		return kGradientLinear;
	}
	BoxFilter::Type FillGradientRadial::type() const {
		return kGradientRadial;
	}
	BoxFilter::Type BoxShadow::type() const {
		return kShadow;
	}

	// ------------------------------ F i l l . I m a g e ------------------------------

	FillImage::FillImage(cString& src, Init init)
		: _size_x(init.size_x)
		, _size_y(init.size_y)
		, _position_x(init.position_x)
		, _position_y(init.position_y)
		, _repeat(init.repeat)
	{
		if (!src.isEmpty()) {
			ImageSourceHolder::set_src(src);
		}
	}

	BoxFilter* FillImage::copy_Rt(BoxFilter* dest) {
		auto dest1 = (dest && dest->type() == kImage) ?
				static_cast<FillImage*>(dest) : new FillImage(String());
		dest1->_repeat = _repeat;
		dest1->_position_x = _position_x;
		dest1->_position_y = _position_y;
		dest1->_size_x = _size_x;
		dest1->_size_y = _size_y;
		dest1->set_source(source());
		dest1->set_next_Rt(next());
		return dest1;
	}

	BoxFilter* FillImage::transition_Rt(BoxFilter* dest, BoxFilter *to, float t) {
		if (to && to->type() == kImage) {
			if (!dest || dest->type() != kImage) {
				dest = new FillImage(String());
			}
			auto dest1 = static_cast<FillImage*>(dest);
			auto to1 = static_cast<FillImage*>(to);
			dest1->_repeat = transition_value(_repeat, to1->_repeat, t);
			dest1->_position_x = transition_value(_position_x, to1->_position_x, t);
			dest1->_position_y = transition_value(_position_y, to1->_position_y, t);
			dest1->_size_x = transition_value(_size_x, to1->_size_x, t);
			dest1->_size_y = transition_value(_size_y, to1->_size_y, t);
			dest1->set_source(t < 1.0 ? source(): to1->source());
			mark(dest1, true);
			auto n = next();
			if (n)
				dest1->set_next_Rt(n->transition_Rt(dest->next(), to->next(), t));
		}
		return dest;
	}

	void FillImage::set_repeat(Repeat value, bool isRt) {
		if (_repeat != value) {
			_repeat = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_position_x(FillPosition value, bool isRt) {
		if (value != _position_x) {
			_position_x = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_position_y(FillPosition value, bool isRt) {
		if (value != _position_y) {
			_position_y = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_size_x(FillSize value, bool isRt) {
		if (value != _size_x) {
			_size_x = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_size_y(FillSize value, bool isRt) {
		if (value != _size_y) {
			_size_y = value;
			mark(this, isRt);
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
		float out;
		switch (pos.kind) {
			default:
			case FillPositionKind::kStart: out = 0; break;
			case FillPositionKind::kPixel: out = pos.value; break;
			case FillPositionKind::kRatio: out = pos.value * host; break;
			case FillPositionKind::kEnd: out = host - size; break;
			case FillPositionKind::kCenter: out = (host - size) / 2; break;
		}
		return out;
	}

	String FillImage::src() const {
		return ImageSourceHolder::src();
	}

	ImageSource* FillImage::source() {
		return ImageSourceHolder::source();
	}

	void FillImage::set_src(String src, bool isRt) {
		auto pool = imgPool();
		set_source(pool ? pool->get(src): *ImageSource::Make(src), isRt);
	}

	void FillImage::set_source(ImageSource* val, bool isRt) {
		auto v = safe_view(view()); // safe get
		if (isRt || !v) {
			ImageSourceHolder::set_source(val);
		} else {
			Retain(val); // temp retain
			v->preRender().async_call([](auto self, auto arg) {
				self->ImageSourceHolder::set_source(arg.arg);
				Release(arg.arg); // temp release
			}, this, val);
		}
	}

	ImagePool* FillImage::imgPool() {
		return shared_app() ? shared_app()->imgPool(): nullptr;
	}

	void FillImage::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (*evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
			mark(this, false);
		}
	}

	// ------------------------------ F i l l . G r a d i e n t ------------------------------

	FillGradient::FillGradient(cArray<float>& pos, cArray<Color4f>& colors)
		: _pos(pos), _colors(colors)
	{}

	void FillGradient::transition_g_Rt(BoxFilter* dest, BoxFilter *to, float t) {
		auto dest1 = static_cast<FillGradient*>(dest);
		auto to1 = static_cast<FillGradient*>(to);
		dest1->_pos = positions();
		dest1->_colors = colors();

		auto posLen = Uint32::min(
			dest1->_pos.length(),
			Uint32::min(_pos.length(), to1->_pos.length())
		);
		auto colorLen = Uint32::min(
			dest1->_colors.length(),
			Uint32::min(_colors.length(), to1->_colors.length())
		);
		for (auto i = 0; i < posLen; i++) {
			dest1->_pos[i] = transition_value(_pos[i], to1->_pos[i], t);
		}
		for (auto i = 0; i < colorLen; i++) {
			dest1->_colors[i] = transition_value(_colors[i], to1->_colors[i], t);
		}
		mark(dest1, true);
		auto n = next();
		if (n)
			dest1->set_next_Rt(n->transition_Rt(dest->next(), to->next(), t));
	}

	FillGradientLinear::FillGradientLinear(float angle, cArray<float>& pos, cArray<Color4f>& colors)
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

	void FillGradientLinear::set_angle(float val, bool isRt) {
		if (val != _angle) {
			_angle = val;
			setRadian();
			mark(this, isRt);
		}
	}

	BoxFilter* FillGradientLinear::copy_Rt(BoxFilter* dest) {
		auto dest1 = (dest && dest->type() == kGradientLinear) ?
			static_cast<FillGradientLinear*>(dest): new FillGradientLinear(_angle, positions(), colors());
		dest1->_radian = _radian;
		dest1->_quadrant = _quadrant;
		dest1->set_next_Rt(next());
		return dest1;
	}

	BoxFilter* FillGradientLinear::transition_Rt(BoxFilter* dest, BoxFilter *to, float t) {
		if (to && to->type() == type()) {
			if (!dest || dest->type() != type())
				dest = new FillGradientLinear(0, {},{});
			FillGradient::transition_g_Rt(dest, to, t);
			auto to1 = static_cast<FillGradientLinear*>(to);
			auto dest1 = static_cast<FillGradientLinear*>(dest);
			dest1->_angle = transition_value(_angle, to1->_angle, t);
			dest1->setRadian();
		}
		return dest;
	}

	BoxFilter* FillGradientRadial::copy_Rt(BoxFilter* dest) {
		auto dest1 = (dest && dest->type() == kGradientRadial) ?
			static_cast<FillGradientRadial*>(dest): new FillGradientRadial(positions(), colors());
		dest1->set_next_Rt(next());
		return dest1;
	}

	BoxFilter* FillGradientRadial::transition_Rt(BoxFilter* dest, BoxFilter *to, float t) {
		if (to && to->type() == type()) {
			if (!dest || dest->type() != type())
				dest = new FillGradientRadial({},{});
			FillGradient::transition_g_Rt(dest, to, t);
		}
		return dest;
	}

	// ------------------------------ B o x . S h a d o w ------------------------------

	BoxShadow::BoxShadow() {}
	BoxShadow::BoxShadow(Shadow value): _value(value) {}
	BoxShadow::BoxShadow(float x, float y, float s, Color color): _value{x,y,s,color} {}

	BoxFilter* BoxShadow::copy_Rt(BoxFilter* dest) {
		auto dest1 = (dest && dest->type() == kShadow) ?
			static_cast<BoxShadow*>(dest): new BoxShadow();
		dest1->_value = _value;
		dest1->set_next_Rt(next());
		return dest1;
	}

	BoxFilter* BoxShadow::transition_Rt(BoxFilter* dest, BoxFilter *to, float t) {
		if (to && to->type() == kShadow) {
			if (!dest || dest->type() != kShadow) dest = new BoxShadow();
			auto dest1 = static_cast<BoxShadow*>(dest);
			auto to1 = static_cast<BoxShadow*>(to);
			dest1->_value = transition_value(_value, to1->_value, t);
			mark(dest1, true);
			auto n = next();
			if (n)
				dest1->set_next_Rt(n->transition_Rt(dest->next(), to->next(), t));
		}
		return dest;
	}
}
