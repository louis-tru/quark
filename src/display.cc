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

#ifndef PRINT_RENDER_FRAME_TIME
# define PRINT_RENDER_FRAME_TIME 0
#endif

namespace qk {

	Display::Display(Application* host)
		: Qk_Init_Event(Change), Qk_Init_Event(Orientation)
		, _host(host)
		, _lock_size()
		, _size(), _scale(1)
		, _atom_pixel(1)
		, _default_scale(0)
		, _fsp(0)
		, _next_fsp(0)
		, _next_fsp_time(0), _surface_region(), _lock_size_mark(false)
	{
		_clip_region.push({ Vec2{0,0},Vec2{0,0},Vec2{0,0} });
	}

	Display::~Display() {}

	void Display::updateState(void *lock, Mat4 *surfaceMat) { // Lock before calling
    auto _lock = static_cast<UILock*>(lock);
		Vec2 size = surface_size();
		float width = size.x();
		float height = size.y();

		if (_lock_size.x() == 0 && _lock_size.y() == 0) { // Use the system default most suitable size
			_size = { width / _default_scale, height / _default_scale };
		}
		else if (_lock_size.x() != 0) { // lock width
			_size = { _lock_size.x(), _lock_size.x() / width * height };
		}
		else if (_lock_size.y() != 0) { // lock height
			_size = { _lock_size.y() / height * width, _lock_size.y() };
		}
		else { // Use the system default most suitable size
			_size = { width / _default_scale, height / _default_scale };
		}

		_scale = (width + height) / (_size.x() + _size.y());
		_atom_pixel = 1.0f / _scale;

		// set default draw region
		_clip_region.front() = {
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
    *surfaceMat = Mat4::ortho(start.x(), end.x(), start.y(), end.y(), -1.0f, 1.0f);

    _host->root()->onDisplayChange();

    Qk_DEBUG("Display::updateState() %f, %f", region.size.x(), region.size.y());
	}

	void Display::set_size(float width, float height) {
		if (width >= 0.0 && height >= 0.0) {
			UILock lock(_host);
			if (_lock_size.x() != width || _lock_size.y() != height) {
				_lock_size = { width, height };
				_lock_size_mark = true;
				_host->render()->reload();
			}
		} else {
			Qk_DEBUG("Lock size value can not be less than zero\n");
		}
	}

	void Display::push_clip_region(Region clip) {
		RegionSize re = {
			Vec2{clip.origin.x(), clip.origin.y()}, Vec2{clip.end.x(), clip.end.y()}, Vec2{0,0}
		};
		RegionSize dre = _clip_region.back();
			
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

		_clip_region.push(re);
	}

	void Display::pop_clip_region() {
		Qk_ASSERT( _clip_region.length() > 1 );
		_clip_region.pop();
	}

	void Display::next_frame(cCb& cb) {
		UILock lock(_host);
		_next_frame.push_back(cb);
	}

	void Display::solve_next_frame() {
		if (_next_frame.length()) {
			List<Cb>* cb = new List<Cb>(std::move(_next_frame));
			_host->loop()->post(Cb([this, cb](Cb::Data& e) {
				UILock lock(_host);
				Handle<List<Cb>> handle(cb);
				for ( auto& i : *cb ) {
					i->resolve();
				}
			}));
		}
	}

	bool Display::onRenderBackendReload(Region region, Vec2 size, float defaultScale, Mat4 *mat) {
		if (size.x() != 0 && size.y() != 0 && defaultScale != 0) {
			Qk_DEBUG("Display::onDeviceReload");
			UILock lock(_host);
			if ( _lock_size_mark
			  || _surface_region.origin.x() != region.origin.x()
				||	_surface_region.origin.y() != region.origin.y()
				||	_surface_region.end.x() != region.end.x()
				||	_surface_region.end.y() != region.end.y()
				||	_surface_region.size.x() != size.x()
				||	_surface_region.size.y() != size.y()
				||  _default_scale != defaultScale
			) {
				_surface_region = { region.origin, region.end, size };
				_default_scale = defaultScale;
				updateState(&lock, mat);
				return true;
			} else {
				_host->root()->onDisplayChange();
			}
		}
		return false;
	}

	bool Display::onRenderBackendPreDisplay() {
		UILock lock(_host); // ui main local
		if (_host->pre_render()->solve())
			return true;
		solve_next_frame();
		return false;
	}

	void Display::onRenderBackendDisplay() {
		UILock lock(_host); // ui main local
		int64_t now_time = time_monotonic();
		
		Qk_DEBUG("Display::render()");

		if (now_time - _next_fsp_time >= 1e6) { // 1s
			_fsp = _next_fsp;
			_next_fsp = 0;
			_next_fsp_time = now_time;
			Qk_DEBUG("fps: %d", _fsp);
		}
		_next_fsp++;

		_host->render()->begin(); // ready render
		_host->root()->accept(_host->render()); // start drawing

		solve_next_frame(); // solve frame

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t st = time_micro();
#endif
		/*
		 * submit() is very time-consuming, and the rendering thread occupying `UILock` for a long time will plunge the main thread.
		 * So the release of `UILock`submit() here is mainly a function call related to drawing,
		 * If you can ensure that the drawing function calls are all in the rendering thread, then there will be no security issues.
		 */
		lock.unlock(); //

		_host->render()->submit(); // commit render cmd

#if DEBUG && PRINT_RENDER_FRAME_TIME
		int64_t ts2 = (time_micro() - st) / 1e3;
		if (ts2 > 16) {
			Qk_LOG("ts: %ld -------------- ", ts2);
		} else {
			Qk_LOG("ts: %ld", ts2);
		}
#endif
	}

}
