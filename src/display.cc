/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
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

#include "./display.h"
#include "./app.h"
#include "./pre_render.h"
#include "./layout/root.h"
#include "./render/render.h"
#include "./view_render.h"

#ifndef PRINT_RENDER_FRAME_TIME
# define PRINT_RENDER_FRAME_TIME 0
#endif

namespace qk {

	Display::Display(Application* host)
		: Qk_Init_Event(Change), Qk_Init_Event(Orientation)
		, _host(host)
		, _view_render(nullptr)
		, _lockSize()
		, _size(), _scale(1)
		, _atom_pixel(1)
		, _default_scale(0)
		, _fsp(0)
		, _nextFsp(0)
		, _nextFspTime(0), _surface_region()
	{
		_clipRegion.push({ Vec2{0,0},Vec2{0,0},Vec2{0,0} });
		_view_render = new ViewRender(this);
	}

	Display::~Display() {
		Release(_view_render); _view_render = nullptr;
	}

	void Display::push_clip_region(Region clip) {
		RegionSize re = {
			Vec2{clip.origin.x(), clip.origin.y()}, Vec2{clip.end.x(), clip.end.y()}, Vec2{0,0}
		};
		RegionSize dre = _clipRegion.back();
			
		// Compute an intersection area
			
		float x, x2, y, y2;
		
		y = dre.end.y() > re.end.y() ? re.end.y() : dre.end.y(); // choose a small
		y2 = dre.origin.y() > re.origin.y() ? dre.origin.y() : re.origin.y(); // choose a large
		x = dre.end.x() > re.end.x() ? re.end.x() : dre.end.x(); // choose a small
		x2 = dre.origin.x() > re.origin.x() ? dre.origin.x() : re.origin.x(); // choose a large

		if ( x > x2 ) {
			re.origin.set_x(x2);
			re.end.set_x(x);
		} else {
			re.origin.set_x(x);
			re.end.set_x(x2);
		}
		
		if ( y > y2 ) {
			re.origin.set_y(y2);
			re.end.set_y(y);
		} else {
			re.origin.set_y(y);
			re.end.set_y(y2);
		}

		re.size = Vec2(re.end.x() - re.origin.x(), re.end.y() - re.origin.y());

		_clipRegion.push(re);
	}

	void Display::pop_clip_region() {
		Qk_ASSERT( _clipRegion.length() > 1 );
		_clipRegion.pop();
	}

	void Display::next_frame(cCb& cb) {
		UILock lock(_host);
		_nextFrame.pushBack(cb);
	}

	void Display::solveNextFrame() {
		if (_nextFrame.length()) {
			List<Cb>* cb = new List<Cb>(std::move(_nextFrame));
			_host->loop()->post(Cb([this, cb](Cb::Data& e) {
				UILock lock(_host);
				Handle<List<Cb>> handle(cb);
				for ( auto& i : *cb ) {
					i->resolve();
				}
			}));
		}
	}

	void Display::set_size(Vec2 size) {
		float w = size.x(), h = size.y();
		if (w >= 0.0 && h >= 0.0) {
			UILock lock(_host);
			if (_lockSize.x() != w || _lockSize.y() != h) {
				_lockSize = { w, h };
				updateState();
				_host->render()->getCanvas()->setSurface(_surfaceMat, size, _scale);
			}
		} else {
			Qk_DEBUG("Lock size value can not be less than zero\n");
		}
	}

	void Display::updateState() { // Lock before calling
		Vec2 size = surface_size();
		float width = size.x();
		float height = size.y();

		if (_lockSize.x() == 0 && _lockSize.y() == 0) { // Use the system default most suitable size
			_size = { width / _default_scale, height / _default_scale };
		}
		else if (_lockSize.x() != 0) { // lock width
			_size = { _lockSize.x(), _lockSize.x() / width * height };
		}
		else if (_lockSize.y() != 0) { // lock height
			_size = { _lockSize.y() / height * width, _lockSize.y() };
		}
		else { // Use the system default most suitable size
			_size = { width / _default_scale, height / _default_scale };
		}

		_scale = (width + height) / (_size.x() + _size.y());
		_atom_pixel = 1.0f / _scale;

		// set default draw region
		_clipRegion.front() = {
			Vec2{0, 0},
			Vec2{_size.x(), _size.y()},
			Vec2{_size.x(), _size.y()},
		};

		_host->loop()->post(Cb([this](Cb::Data& e) { // main loop call
			Qk_Trigger(Change); // trigger display change
		}));

		auto region = _surface_region;
		Vec2 start = Vec2(-region.origin.x() / _scale, -region.origin.y() / _scale);
		Vec2 end   = Vec2(region.size.x() / _scale + start.x(), region.size.y() / _scale + start.y());
		_surfaceMat = Mat4::ortho(start.x(), end.x(), start.y(), end.y(), -1.0f, 1.0f);

		_view_render->set_render(_host->render());
		_host->root()->onDisplayChange();

		Qk_DEBUG("Display::updateState() %f, %f", region.size.x(), region.size.y());
	}

	void Display::onRenderBackendReload(Region region, Vec2 size, float defaultScale) {
		if (size.x() != 0 && size.y() != 0 && defaultScale != 0) {
			Qk_DEBUG("Display::onDeviceReload");
			UILock lock(_host);
			if ( _surface_region.origin != region.origin
				|| _surface_region.end != region.end
				|| _surface_region.size != size
				|| _default_scale != defaultScale
			) {
				_surface_region = { region.origin, region.end, size };
				_default_scale = defaultScale;
				updateState();
				_host->render()->getCanvas()->setSurface(_surfaceMat, size, _scale);
			} else {
				_host->root()->onDisplayChange();
			}
		}
	}

	bool Display::onRenderBackendDisplay() {
		UILock lock(_host); // ui main local

		if (!_host->pre_render()->solve()) {
			solveNextFrame();
			return false;
		}
		int64_t now_time = time_second();

		if (now_time - _nextFspTime >= 1) { // 1s
			_fsp = _nextFsp;
			_nextFsp = 0;
			_nextFspTime = now_time;
			Qk_DEBUG("fps: %d", _fsp);
		}
		_nextFsp++;

		_host->root()->accept(_view_render); // start drawing

		solveNextFrame(); // solve frame

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t st = time_micro();
#endif

		_host->render()->getCanvas()->swapBuffer();

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t ts2 = (time_micro() - st) / 1e3;
		if (ts2 > 16) {
			Qk_LOG("ts: %ld -------------- ", ts2);
		} else {
			Qk_LOG("ts: %ld", ts2);
		}
#endif

		return true;
	}

}
