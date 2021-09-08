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

#include "./display-port.h"
#include "./app.h"
#include "./draw.h"
#include "./_pre-render.h"
#include "./layout/root.h"
#include "./action/action.h"
#include "./util/_working.h"
#include "./util/os.h"

namespace flare {

	FX_DEFINE_INLINE_MEMBERS(Display, Inl) {
	public:
		#define _inl(self) static_cast<Display::Inl*>(self)

		void update_from_render_loop() {
			ScopeLock lock(_Mutex);

			if (_lock_size.x() == 0 && _lock_size.y() == 0) { // 使用系统默认的最合适的尺寸
				_size = {
					_surface_region.size.x() / _best_display_scale,
					_surface_region.size.y() / _best_display_scale,
				};
			}
			else if (_lock_size.x() != 0 && _lock_size.y() != 0) { // 尺寸全部锁定
				_size = _lock_size;
			}
			else if (_lock_size.x() != 0) { // 只锁定宽度
				_size.y(_lock_size.x());
				_size.y(_size.x() / _surface_region.size.x() * _surface_region.size.y());
			}
			else { // _lock_height == 0 // 只锁定高度
				_size.y(_lock_size.y());
				_size.x(_size.y() / _surface_region.size.y() * _surface_region.size.x());
			}
			
			_scale.x(_surface_region.size.x() / _size.x());
			_scale.y(_surface_region.size.y() / _size.y());

			float scale = (_scale.x() + _scale.y()) / 2;
			
			_atom_pixel = 1.0f / scale;
			
			Rect region = _surface_region;
			
			Vec2 start = Vec2(-region.origin.x() / _scale.x(), -region.origin.y() / _scale.y());
			Vec2 end   = Vec2(region.size.x() / _scale.x() + start.x(), region.size.y() / _scale.y() + start.y());
			
			_root_matrix = Mat4::ortho(start.x(), end.x(), start.y(), end.y(), -1.0f, 1.0f); // 计算2D视图变换矩阵

			// update root
			Root* r = _host->root();
			if (r) {
				r->mark(Layout::M_LAYOUT_SIZE_WIDTH | Layout::M_LAYOUT_SIZE_HEIGHT);
			}
			
			// set default draw region
			_display_region.front() = {
				0, 0,
				_size.x(), _size.y(),
				_size.x(), _size.y(),
			};
			
			_host->main_loop()->post(Cb([this](CbData& e){
				FX_Trigger(change); // 通知事件
			}));

			_render->resize(_size, _surface_region);
		}
		
		/**
		* @func solve_next_frame()
		*/
		void solve_next_frame() {
			if (_next_frame.length()) {
				List<Cb>* cb = new List<Cb>(std::move(_next_frame));
				_host->main_loop()->post(Cb([cb](CbData& e) {
					Handle<List<Cb>> handle(cb);
					for ( auto& i : *cb ) {
						i->resolve();
					}
				}));
			}
		}
	};

	Display::Display(Application* host)
		: FX_Init_Event(change), FX_Init_Event(orientation)
		, _host(host)
		, _lock_size()
		, _size(), _scale(1, 1)
		, _root_matrix()
		, _atom_pixel(1)
		, _fsp(0)
		, _record_fsp(0)
		, _record_fsp_time(0), _surface_region()
	{
		_display_region.push_back({ 0,0,0,0,0,0 });
	}

	Display::~Display() {
	}

	void Display::lock_size(float width, float height) {
		if (width >= 0.0 && height >= 0.0) {
			ScopeLock lock(_Mutex);
			if (_lock_size.width() != width || _lock_size.height() != height) {
				_lock_size = { width, height };
				ASSERT(_host->render_loop());
				_host->render_loop()->post(Cb([this](CbData& e) {
					_inl(this)->update_from_render_loop();
				}));
			}
		} else {
			FX_WARN("Lock size value can not be less than zero\n");
		}
	}

	#ifndef PRINT_RENDER_FRAME_TIME
	# define PRINT_RENDER_FRAME_TIME 0
	#endif

	void Display::render_frame() {// 必须要渲染循环中调用
		Root* root = _host->root();
		int64_t now_time = os::time_monotonic();
		_host->action_center()->advance(now_time); // advance action
		
		if (root && _host->_pre_render->solve(now_time)) {
			if (now_time - _record_fsp_time >= 1e6) {
				_fsp = _record_fsp;
				_record_fsp = 0;
				_record_fsp_time = now_time;
			}
			_record_fsp++;

			auto render = _host->render();
			
			render->begin_render();
			root->draw(render->canvas()); // 开始绘图
			_inl(this)->solve_next_frame();
			
			#if DEBUG && PRINT_RENDER_FRAME_TIME
				int64_t st = os::time();
			#endif
			/*
			* swapBuffers()非常耗时,渲染线程长时间占用`GUILock`会柱塞主线程。
			* 所以这里释放`GUILock`swapBuffers()主要是绘图相关的函数调用,
			* 如果能够确保绘图函数的调用都在渲染线程,那就不会有安全问题。
			*/
			Inl2_RunLoop(_host->render_loop())->independent_mutex_unlock();
			render->swapBuffers();
			Inl2_RunLoop(_host->render_loop())->independent_mutex_lock();
			#if DEBUG && PRINT_RENDER_FRAME_TIME
				int64_t ts2 = (os::time() - st) / 1e3;
				if (ts2 > 16) {
					LOG("ts: %ld -------------- ", ts2);
				} else {
					LOG("ts: %ld", ts2);
				}
			#endif
		} else {
			_inl(this)->solve_next_frame();
		}
	}

	void Display::refresh() {
		// 必须要渲染循环中调用
		auto root = _host->root();
		if ( root ) {
			auto render = _host->render();
			_host->_pre_render->solve(os::time_monotonic());
			render->begin_render();
			root->draw(render->canvas()); // draw
			render->swapBuffers();
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
		
		re.w = re.x2 - re.x;
		re.h = re.y2 - re.y;
		
		_display_region.push_back(re);
	}

	void Display::next_frame(cCb& cb) {
		_next_frame.push_back(cb);
	}

	void Display::set_surface_region(Rect region) {
		if (_surface_region.origin != region.origin || _surface_region.size != region.size) {
			_surface_region = region;
			_inl(this)->update_from_render_loop();
		}
	}

}
