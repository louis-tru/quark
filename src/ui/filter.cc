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

#define _async_call _window->preRender().async_call

namespace qk {

	template<typename T>
	inline T transition_value(T v1, T v2, float t) {
		return v1 - (v1 - v2) * t;
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
		if (value) {
			auto next = value->next();
			while (next) {
				if (self == next) {
					return Qk_ERR("check_loop_ref(), Box filter loop reference error"), true;
				}
				next = next->next();
			}
		}
		return false;
	}

	static void mark(BoxFilter *self, bool isRt) {
		auto view = self->view();
		if (view)
			view->mark(0,isRt);
	}

	BoxFilter* BoxFilter::link(const std::initializer_list<BoxFilter*>& list) {
		if (list.size()) {
			auto it = list.begin();
			auto prev = *it;
			while (++it != list.end()) {
				prev->set_next(*it);
				prev = *it;
			}
			return *list.begin();
		}
		return nullptr;
	}

	BoxFilter::BoxFilter()
		: _window(nullptr), _view(nullptr), _next(nullptr), _isIndependent(false)
	{
	}

	void BoxFilter::destroy() {
		auto next = _next.load();
		if (next) {
			next->release();
		}
		if (_window && _window->loop()->thread_id() == thread_self_id()) { // is main thread
			_async_call([](auto self, auto arg) {
				// To ensure safety and efficiency,
				// it should be Completely destroyed in RT (render thread)
				self->Object::destroy();
			}, this, 0);
			_window = nullptr;
		} else {
			Object::destroy();
		}
	}

	BoxFilter* BoxFilter::assign(BoxFilter *left, BoxFilter *right, View *view, bool isRt) {
		if (right) {
			if (left != right) {
				auto new_left = right;
				if (right->_isIndependent) {
					new_left = right->copy(left, isRt); // copy
				}
				if (new_left != left) {
					if (left) {
						left->set_view(nullptr); // clear view
						left->release();
					}
					left = new_left;
					left->retain();
					left->set_view(view);
				}
			}
		} else if (left) { // right nullptr
			left->set_view(nullptr); // clear view
			left->release();
			left = nullptr;
		}
		if (view) {
			view->mark(0, isRt);
		}
		return left;
	}

	void BoxFilter::set_next(BoxFilter *next, bool isRt) {
		if (check_loop_ref(this, next)) {
			set_next_no_check(next, isRt);
		}
	}

	void BoxFilter::set_next_no_check(BoxFilter *next, bool isRt) {
		auto left = _next.load();
		_next.store(
			assign(left, next, left ? left->view(): nullptr, isRt)
		);
	}

	void BoxFilter::set_view(View *value) {
		if (value != _view.load()) {
			Qk_Assert_Eq(_isIndependent, false);
			_view.store(value);
			if (!_window) // bind window
				_window = value->window();
			auto next = _next.load();
			if (next)
				next->set_view(value);
		}
		_isIndependent = true;
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
		: _width(init.width)
		, _height(init.height)
		, _x(init.x)
		, _y(init.y)
		, _repeat(init.repeat)
	{
		if (!src.isEmpty()) {
			set_src(src);
		}
	}

	BoxFilter* FillImage::copy(BoxFilter* dest, bool isRt) {
		auto dest1 = (dest && dest->type() == kImage) ?
				static_cast<FillImage*>(dest) : new FillImage(String());
		dest1->_repeat = _repeat;
		dest1->_x = _x;
		dest1->_y = _y;
		dest1->_width = _width;
		dest1->_height = _height;
		dest1->set_source(source());
		dest1->set_next_no_check(next(), isRt);
		return dest1;
	}

	BoxFilter* FillImage::transition(BoxFilter* dest, BoxFilter *to, float t, bool isRt) {
		if (to && to->type() == kImage) {
			if (!dest || dest->type() != kImage) {
				dest = new FillImage(String());
			}
			auto dest1 = static_cast<FillImage*>(dest);
			auto to1 = static_cast<FillImage*>(to);
			dest1->_repeat = transition_value(_repeat, to1->_repeat, t);
			dest1->_x = transition_value(_x, to1->_x, t);
			dest1->_y = transition_value(_y, to1->_y, t);
			dest1->_width = transition_value(_width, to1->_width, t);
			dest1->_height = transition_value(_height, to1->_height, t);
			dest1->set_source(t < 1.0 ? source(): to1->source());
			mark(dest1, isRt);
			auto n = next();
			if (n)
				dest1->set_next_no_check(n->transition(dest->next(), to->next(), t, isRt), isRt);
		}
		return dest;
	}

	void FillImage::set_repeat(Repeat value, bool isRt) {
		if (_repeat != value) {
			_repeat = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_x(FillPosition value, bool isRt) {
		if (value != _x) {
			_x = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_y(FillPosition value, bool isRt) {
		if (value != _y) {
			_y = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_width(FillSize value, bool isRt) {
		if (value != _width) {
			_width = value;
			mark(this, isRt);
		}
	}

	void FillImage::set_height(FillSize value, bool isRt) {
		if (value != _height) {
			_height = value;
			mark(this, isRt);
		}
	}

	bool FillImage::compute_size(FillSize size, float host, float& out) {
		switch (size.kind) {
			default: return false; // AUTO
			case FillSizeKind::Value: out = size.value; break;
			case FillSizeKind::Ratio: out = size.value * host; break;
		}
		return true;
	}

	float FillImage::compute_position(FillPosition pos, float host, float size) {
		float out;
		switch (pos.kind) {
			default:
			case FillPositionKind::Start: out = 0; break;
			case FillPositionKind::Value: out = pos.value; break;
			case FillPositionKind::Ratio: out = pos.value * host; break;
			case FillPositionKind::End: out = host - size; break;
			case FillPositionKind::Center: out = (host - size) / 2; break;
		}
		return out;
	}

	String FillImage::src() const {
		return ImageSourceHold::src();
	}

	void FillImage::set_src(String src, bool isRt) {
		ImageSourceHold::set_src(src);
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

	FillGradientRadial::FillGradientRadial(cArray<float>& pos, cArray<Color4f>& colors)
		: _pos(pos), _colors(colors)
	{}

	void FillGradientRadial::transition_g(BoxFilter* dest, BoxFilter *to, float t, bool isRt) {
		auto dest1 = static_cast<FillGradientRadial*>(dest);
		auto to1 = static_cast<FillGradientRadial*>(to);
		dest1->_pos = _pos;
		dest1->_colors = _colors;

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
		mark(dest1, isRt);
		auto n = next();
		if (n)
			dest1->set_next_no_check(n->transition(dest->next(), to->next(), t, isRt), isRt);
	}

	BoxFilter* FillGradientRadial::copy(BoxFilter* dest, bool isRt) {
		auto dest1 = (dest && dest->type() == kGradientRadial) ?
			static_cast<FillGradientRadial*>(dest): new FillGradientRadial(positions(), colors());
		dest1->set_next_no_check(next(), isRt);
		return dest1;
	}

	BoxFilter* FillGradientRadial::transition(BoxFilter* dest, BoxFilter *to, float t, bool isRt) {
		if (to && to->type() == type()) {
			if (!dest || dest->type() != type())
				dest = new FillGradientRadial({},{});
			transition_g(dest, to, t, isRt);
		}
		return dest;
	}

	FillGradientLinear::FillGradientLinear(cArray<float>& pos, cArray<Color4f>& colors, float angle)
		: FillGradientRadial(pos, colors)
		, _angle(angle)
	{
		setRadian();
	}

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

	BoxFilter* FillGradientLinear::copy(BoxFilter* dest, bool isRt) {
		auto dest1 = (dest && dest->type() == kGradientLinear) ?
			static_cast<FillGradientLinear*>(dest): new FillGradientLinear({},{},0);
		dest1->_pos = _pos;
		dest1->_colors = _colors;
		dest1->_angle = _angle;
		dest1->_radian = _radian;
		dest1->_quadrant = _quadrant;
		dest1->set_next_no_check(next(), isRt);
		return dest1;
	}

	BoxFilter* FillGradientLinear::transition(BoxFilter* dest, BoxFilter *to, float t, bool isRt) {
		if (to && to->type() == type()) {
			if (!dest || dest->type() != type()) dest = new FillGradientLinear({},{},0);
			transition_g(dest, to, t, isRt);
			auto to1 = static_cast<FillGradientLinear*>(to);
			auto dest1 = static_cast<FillGradientLinear*>(dest);
			dest1->_angle = transition_value(_angle, to1->_angle, t);
			dest1->setRadian();
		}
		return dest;
	}

	// ------------------------------ B o x . S h a d o w ------------------------------

	BoxShadow::BoxShadow(Shadow value): _value(value) {}
	BoxShadow::BoxShadow(float x, float y, float s, Color color): _value{x,y,s,color} {}

	void BoxShadow::set_value(Shadow value, bool isRt) {
		if (value != _value) {
			_value = value;
			mark(this, isRt);
		}
	}

	BoxFilter* BoxShadow::copy(BoxFilter* dest, bool isRt) {
		auto dest1 = (dest && dest->type() == kShadow) ?
			static_cast<BoxShadow*>(dest): new BoxShadow({});
		dest1->_value = _value;
		dest1->set_next_no_check(next(), isRt);
		return dest1;
	}

	BoxFilter* BoxShadow::transition(BoxFilter* dest, BoxFilter *to, float t, bool isRt) {
		if (to && to->type() == kShadow) {
			if (!dest || dest->type() != kShadow) dest = new BoxShadow({});
			auto dest1 = static_cast<BoxShadow*>(dest);
			auto to1 = static_cast<BoxShadow*>(to);
			dest1->_value = transition_value(_value, to1->_value, t);
			mark(dest1, isRt);
			auto n = next();
			if (n)
				dest1->set_next_no_check(n->transition(dest->next(), to->next(), t, isRt), isRt);
		}
		return dest;
	}
}
