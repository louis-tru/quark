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
#include "./views2/root.h"
#include "./action/action.h"
#include "./util/_working.h"
#include "./util/os.h"

namespace flare {

	FX_DEFINE_INLINE_MEMBERS(Display, Inl) {
	public:
		#define _inl(self) static_cast<Display::Inl*>(self)
		
		void handle_surface_size_change(Event<>& evt) {
			_phy_size = _draw_ctx->selected_region().size;
			update_display_port_rl();
		}
		
		void update_root_size() {
			Root* r = _host->root();
			if (r) {
				r->set_width(_size.width());
				r->set_height(_size.height());
			}
		}
		
		void update_display_port_rl() {
			
			if (_lock_size.width() == 0 && _lock_size.height() == 0) { // 使用系统默认的最合适的尺寸
				_size = { _phy_size.width() / _draw_ctx->best_display_scale(),
					_phy_size.height() / _draw_ctx->best_display_scale() };
			}
			else if (_lock_size.width() != 0 && _lock_size.height() != 0) { // 尺寸全部锁定
				_size = _lock_size;
			}
			else if (_lock_size.width() != 0) { // 只锁定宽度
				_size.width(_lock_size.width());
				_size.height(_size.width() / _phy_size.width() * _phy_size.height());
			}
			else { // _lock_height == 0 // 只锁定高度
				_size.height(_lock_size.height());
				_size.width(_size.height() / _phy_size.height() * _phy_size.width());
			}
			
			_scale[0] = _phy_size.width() / _size.width();
			_scale[1] = _phy_size.height() / _size.height();

			float scale = (_scale[0] + _scale[1]) / 2;
			
			_atom_pixel = 1.0f / scale;
			
			// 计算2D视图变换矩阵
			
			Rect region = _render_ctx->selected_region();
			Vec2 surface_size = _render_ctx->surface_size();
			
			Vec2 start = Vec2(-region.origin.x() / _scale[0], -region.origin.y() / _scale[1]);
			Vec2 end   = Vec2(surface_size.width() / _scale[0] + start.x(),
												surface_size.height() / _scale[1] + start.y());
			
			_root_matrix = Mat4::ortho(start[0], end[0], start[1], end[1], -1.0f, 1.0f);
			
			Mat4 test_root_matrix = // 测试着色器视图矩阵要大一圈
			Mat4::ortho(start[0]-5, end[0]+5, start[1]-5, end[1]+5, -1.0f, 1.0f);
			
			// _draw_ctx->refresh_root_matrix(_root_matrix, test_root_matrix);
			
			update_root_size(); // update root
			
			// set default draw region
			_draw_region.front() = {
				0, 0,
				_size.width(), _size.height(),
				_size.width(), _size.height(),
			};
			
			_host->main_loop()->post(Cb([this](CbData& e){
				FX_Trigger(change); // 通知事件
			}));
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

	/**
	* @constructor
	*/
	Display::Display(GUIApplication* host)
	: FX_Init_Event(change), FX_Init_Event(orientation)
	, _phy_size(), _lock_size()
	, _size(), _scale(1, 1)
	, _draw_ctx(host->draw_ctx())
	, _root_matrix()
	, _atom_pixel(1)
	, _host(host)
	, _fsp(0)
	, _record_fsp(0)
	, _record_fsp_time(0)
	{
		_draw_region.push_back({ 0,0,0,0,0,0 });
		// 侦听视口尺寸变化
		FX_DEBUG("_draw_ctx->FX_On ...");
		_draw_ctx->FX_On(surface_size_change_r, &Inl::handle_surface_size_change, _inl(this));
		FX_DEBUG("_draw_ctx->FX_On ok");
	}

	/**
	* @destructor
	*/
	Display::~Display() {
		_draw_ctx->FX_Off(surface_size_change_r, &Inl::handle_surface_size_change, _inl(this));
	}

	float Display::best_scale() const {
		return _draw_ctx->best_display_scale();
	}

	void Display::lock_size(float width, float height) {
		if (width >= 0.0 && height >= 0.0) {
			if (_lock_size.width() != width || _lock_size.height() != height) {
				_lock_size = { width, height };
				ASSERT(_host->render_loop());
				_host->render_loop()->post(Cb([this](CbData& e) {
					_inl(this)->update_display_port_rl();
				}));
			}
		} else {
			FX_WARN("Lock size value can not be less than zero\n");
		}
	}

	#ifndef PRINT_RENDER_FRAME_TIME
	# define PRINT_RENDER_FRAME_TIME 0
	#endif

	/**
	* @func render_frame()
	*/
	void Display::render_frame() {
		Root* r = _host->root();
		int64_t now_time = os::time_monotonic();
		// TODO ...
		// _host->action_center()->advance(now_time); // advance action
		
		if (r) {
			bool redraw = _host->_pre_render->solve(now_time);
			if (redraw || r->mark_value || r->_child_change_flag) {
				
				if (now_time - _record_fsp_time >= 1e6) {
					_fsp = _record_fsp;
					_record_fsp = 0;
					_record_fsp_time = now_time;
				}
				_record_fsp++;
				
				_draw_ctx->begin_render();
				r->draw(_draw_ctx); // 开始绘图
				_inl(this)->solve_next_frame();
				
				#if DEBUG && PRINT_RENDER_FRAME_TIME
					int64_t st = os::time();
				#endif
				/*
				* commit_render()非常耗时,渲染线程长时间占用`GUILock`会柱塞主线程。
				* 所以这里释放`GUILock`，commit_render()主要是绘图相关的函数调用,
				* 如果能够确保绘图函数的调用都在渲染线程,那就不会有安全问题。
				*/
				Inl2_RunLoop(_host->render_loop())->independent_mutex_unlock();
				_draw_ctx->commit_render();
				Inl2_RunLoop(_host->render_loop())->independent_mutex_lock();
				#if DEBUG && PRINT_RENDER_FRAME_TIME
					int64_t ts2 = (os::time() - st) / 1e3;
					if (ts2 > 16) {
						LOG("ts: %ld -------------- ", ts2);
					} else {
						LOG("ts: %ld", ts2);
					}
				#endif
				return;
			}
		}
		_inl(this)->solve_next_frame();
	}

	/**
	* @func refresh()
	*/
	void Display::refresh() {
		// TODO 必须要渲染循环中调用
		Root* r = _host->root();
		if ( r ) {
			_host->_pre_render->solve(os::time_monotonic());
			_draw_ctx->begin_render();
			r->draw(_draw_ctx); // 开始绘图
			_draw_ctx->commit_render();
		}
	}

	void Display::push_draw_region(Region re) {
		// 计算一个交集区域
		
		Region dre = _draw_region.back();
		
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
		
		_draw_region.push_back(re);
	}

	/**
	* @func next_frame
	*/
	void Display::next_frame(cCb& cb) {
		_next_frame.push_back(cb);
	}

	bool Display::set_surface_size(Vec2 surface_size, Rect* surface_region) {
		Rect region = surface_region ? 
			*surface_region : Rect({ Vec2(), surface_size });
			
		if (_surface_size != surface_size ||
				_surface_region.origin != region.origin ||
				_surface_region.size != region.size
		) {
			_surface_size = surface_size;
			_surface_region = region;
			// refresh_buffer();
			// FX_Trigger(surface_size_change_from_render);
			return true;
		}
		return false;
	}


}
