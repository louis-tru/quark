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

namespace qk {

	Display::Display(Application* host)
		: Qk_Init_Event(Change), Qk_Init_Event(Orientation)
		, _host(host)
		, _set_size()
		, _size(), _scale(1)
		, _atom_pixel(1)
		, _fsp(0)
		, _next_fsp(0)
		, _next_fsp_time(0), _surface_region()
	{
		_clip_region.push({ Vec2{0,0},Vec2{0,0},Vec2{0,0} });
	}

	Display::~Display() {
	}

	/**
		* @thread render
		*/
	void Display::updateState() { // Called in render loop
		UILock lock(_host);

		Vec2 size = surface_size();
		float width = size.x();
		float height = size.y();

		if (_set_size.x() == 0 && _set_size.y() == 0) { // 使用系统默认的最合适的尺寸
			_size = { width / _default_scale, height / _default_scale };
		}
		else if (_set_size.x() != 0) { // 锁定宽度
			_size = { _set_size.x(), _set_size.x() / width * height };
		}
		else if (_set_size.y() != 0) { // 锁定高度
			_size = { _set_size.y() / height * width, _set_size.y() };
		}
		else { // 使用系统默认的最合适的尺寸
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

		lock.unlock();

		_host->loop()->post(Cb([this](Cb::Data& e) { // main loop call
			_host->root()->onDisplayChange(); // update root
			Qk_Trigger(Change); // 通知事件
		}));
		_host->render()->reload();
	}

	void Display::set_size(float width, float height) {
		if (width >= 0.0 && height >= 0.0) {
			UILock lock(_host);
			if (_set_size.x() != width || _set_size.y() != height) {
				_set_size = { width, height };
				_host->render()->post_message(Cb([this](Cb::Data& e) {
					updateState();
				}));
			}
		} else {
			Qk_WARN("Lock size value can not be less than zero\n");
		}
	}

	#ifndef PRINT_RENDER_FRAME_TIME
	# define PRINT_RENDER_FRAME_TIME 0
	#endif

	void Display::render() { // Must be called in the render loop
		UILock lock(_host); // ui main local
		Root* root = _host->root();
		int64_t now_time = time_monotonic();
		// _host->action_direct()->advance(now_time); // advance action TODO ...

		if (_host->pre_render()->solve(now_time)) {
			if (now_time - _next_fsp_time >= 1e6) { // 1s
				_fsp = _next_fsp;
				_next_fsp = 0;
				_next_fsp_time = now_time;
				Qk_DEBUG("fps: %d", _fsp);
			}
			_next_fsp++;

			auto render = _host->render();

			render->begin(); // ready render
			root->accept(render); // 开始绘图
			solve_next_frame();

			#if DEBUG && PRINT_RENDER_FRAME_TIME
				int64_t st = time_micro();
			#endif
			/*
			* swapBuffers()非常耗时,渲染线程长时间占用`UILock`会柱塞主线程。
			* 所以这里释放`UILock`commit()主要是绘图相关的函数调用,
			* 如果能够确保绘图函数的调用都在渲染线程,那就不会有安全问题。
			*/
			lock.unlock(); //

			render->submit(); // commit render cmd

			#if DEBUG && PRINT_RENDER_FRAME_TIME
				int64_t ts2 = (time_micro() - st) / 1e3;
				if (ts2 > 16) {
					Qk_LOG("ts: %ld -------------- ", ts2);
				} else {
					Qk_LOG("ts: %ld", ts2);
				}
			#endif
		} else {
			solve_next_frame();
		}
	}

	void Display::push_clip_region(Region clip) {
		RegionSize re = {
			Vec2{clip.origin.x(), clip.origin.y()}, Vec2{clip.end.x(), clip.end.y()}, Vec2{0,0}
		};
		RegionSize dre = _clip_region.back();
			
		// 计算一个交集区域
			
		float x, x2, y, y2;
		
		y = dre.end.y() > re.end.y() ? re.end.y() : dre.end.y(); // 选择一个小的
		y2 = dre.origin.y() > re.origin.y() ? dre.origin.y() : re.origin.y(); // 选择一个大的
		x = dre.end.x() > re.end.x() ? re.end.x() : dre.end.x(); // 选择一个小的
		x2 = dre.origin.x() > re.origin.x() ? dre.origin.x() : re.origin.x(); // 选择一个大的
		
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
			_host->loop()->post(Cb([cb](Cb::Data& e) {
				Handle<List<Cb>> handle(cb);
				for ( auto& i : *cb ) {
					i->resolve();
				}
			}));
		}
	}

	void Display::set_default_scale(float value) {
		UILock lock(_host);
		_default_scale = value;
	}

	bool Display::set_surface_region(RegionSize region) {
		if (region.size.x() != 0 && region.size.y() != 0) {
			UILock lock(_host);
			if (  _surface_region.origin.x() != region.origin.x()
				||	_surface_region.origin.y() != region.origin.y()
				||	_surface_region.end.x() != region.end.x()
				||	_surface_region.end.y() != region.end.y()
				||	_surface_region.size.x() != region.size.x()
				||	_surface_region.size.y() != region.size.y()
			) {
				_surface_region = region;
				_host->render()->post_message(Cb([this](Cb::Data& e) {
					updateState();
				}));
				return true;
			}
		}
		return false;
	}

}
