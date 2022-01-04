/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
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

#include "./display.h"
#include "./app.h"
#include "./pre_render.h"
#include "./layout/root.h"
#include "./render/render.h"
// #include "./action/action.h"

namespace flare {

	/**
		* @thread render
		*/
	void Display::update_state() { // Called in render loop
		UILock lock(_host);

		Vec2 _phy_size = phy_size();
		float width = _phy_size.x();
		float height = _phy_size.y();

		if (_lock_size.x() == 0 && _lock_size.y() == 0) { // 使用系统默认的最合适的尺寸
			_size = {
				width / _default_scale,
				height / _default_scale,
			};
		}
		else if (_lock_size.x() != 0 && _lock_size.y() != 0) { // 尺寸全部锁定
			_size = _lock_size;
		}
		else if (_lock_size.x() != 0) { // 只锁定宽度
			_size.y(_lock_size.x());
			_size.y(_size.x() / width * height);
		}
		else { // _lock_height == 0 // 只锁定高度
			_size.y(_lock_size.y());
			_size.x(_size.y() / height * width);
		}
		
		_scale.x(width / _size.x());
		_scale.y(height / _size.y());

		float scale = (_scale.x() + _scale.y()) / 2;
		
		_atom_pixel = 1.0f / scale;
		
		// update root
		Root* r = _host->root();
		if (r) {
			r->mark_layout_change();
		}
		
		// set default draw region
		_display_region.front() = {
			0, 0,
			_size.x() * _scale.x(), _size.y() * _scale.y(),
			_size.x() * _scale.x(), _size.y() * _scale.y(),
		};

		lock.unlock();
		
		_host->loop()->post(Cb([this](CbData& e){
			F_Trigger(Change); // 通知事件
		}));

		_host->render()->reload();
	}
	
	/**
	* @func solve_next_frame()
	*/
	void Display::solve_next_frame() {
		if (_next_frame.length()) {
			List<Cb>* cb = new List<Cb>(std::move(_next_frame));
			_host->loop()->post(Cb([cb](CbData& e) {
				Handle<List<Cb>> handle(cb);
				for ( auto& i : *cb ) {
					i->resolve();
				}
			}));
		}
	}

	Vec2 Display::phy_size() const {
		return Vec2(_surface_region.x2 - _surface_region.x, _surface_region.y2 - _surface_region.y);
	}

	Display::Display(Application* host)
		: F_Init_Event(Change), F_Init_Event(Orientation)
		, _host(host)
		, _lock_size()
		, _size(), _scale(1, 1)
		, _atom_pixel(1)
		, _fsp(0)
		, _next_fsp(0)
		, _next_fsp_time(0), _surface_region()
	{
		_display_region.push_back({ 0,0,0,0,0,0 });
	}

	Display::~Display() {
	}

	void Display::lock_size(float width, float height) {
		if (width >= 0.0 && height >= 0.0) {
			UILock lock(_host);
			if (_lock_size.width() != width || _lock_size.height() != height) {
				_lock_size = { width, height };
				_host->render()->post_message(Cb([this](CbData& e) {
					update_state();
				}));
			}
		} else {
			F_WARN("Lock size value can not be less than zero\n");
		}
	}

	#ifndef PRINT_RENDER_FRAME_TIME
	# define PRINT_RENDER_FRAME_TIME 0
	#endif

	void Display::render(bool need) {// 必须要渲染循环中调用
		UILock lock(_host); // ui main local
		Root* root = _host->root();
		int64_t now_time = time_monotonic();
		// _host->action_direct()->advance(now_time); // advance action TODO ...
		
		if (root && (_host->pre_render()->solve(now_time) || need)) {
			if (now_time - _next_fsp_time >= 1e6) { // 1s
				_fsp = _next_fsp;
				_next_fsp = 0;
				_next_fsp_time = now_time;
			}
			_next_fsp++;

			auto render = _host->render();
			
			root->draw(render->canvas(), 1); // 开始绘图
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
			render->commit();
			#if DEBUG && PRINT_RENDER_FRAME_TIME
				int64_t ts2 = (time_micro() - st) / 1e3;
				if (ts2 > 16) {
					F_LOG("ts: %ld -------------- ", ts2);
				} else {
					F_LOG("ts: %ld", ts2);
				}
			#endif
		} else {
			solve_next_frame();
		}
	}

	void Display::push_display_region(Region re) {
		// 计算一个交集区域
		Region dre = _display_region.back();
		
		float x, x2, y, y2;
		
		y = dre.y2 > re.y2 ? re.y2 : dre.y2; // 选择一个小的
		y2 = dre.y > re.y ? dre.y : re.y; // 选择一个大的
		x = dre.x2 > re.x2 ? re.x2 : dre.x2; // 选择一个小的
		x2 = dre.x > re.x ? dre.x : re.x; // 选择一个大的
		
		if ( x > x2 ) {
			re.x = x2;
			re.x2 = x;
		} else {
			re.x = x;
			re.x2 = x2;
		}
		
		if ( y > y2 ) {
			re.y = y2;
			re.y2 = y;
		} else {
			re.y = y;
			re.y2 = y2;
		}
		
		re.x *= _scale.x();
		re.x2 *= _scale.x();
		re.y *= _scale.y();
		re.y2 *= _scale.y();

		re.width = re.x2 - re.x;
		re.height = re.y2 - re.y;
		
		_display_region.push_back(re);
	}

	void Display::pop_display_region() {
		F_ASSERT( _display_region.length() > 1 );
		_display_region.pop_back();
	}

	void Display::next_frame(cCb& cb) {
		UILock lock(_host);
		_next_frame.push_back(cb);
	}

	void Display::set_default_scale(float value) {
		UILock lock(_host);
		_default_scale = value;
	}

	bool Display::set_surface_region(Region region) {
		bool ok = false;
		if (region.width != 0 && region.height != 0) {
			UILock lock(_host);
			if (  _surface_region.x != region.x
				||	_surface_region.y != region.y
				||	_surface_region.x2 != region.x2
				||	_surface_region.y2 != region.y2
				||	_surface_region.width != region.width
				||	_surface_region.height != region.height
			) {
				_surface_region = region;
				ok = true;
			}
		}
		if (ok) {
			update_state();
		}
		return ok;
	}

}
