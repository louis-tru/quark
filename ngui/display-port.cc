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

#include "display-port.h"
#include "app.h"
#include "draw.h"
#include "pre-render.h"
#include "root.h"
#include "action.h"
#include "base/loop-1.h"
#include "base/sys.h"

XX_NS(ngui)


XX_DEFINE_INLINE_MEMBERS(DisplayPort, Inl) {
public:
#define _inl(self) static_cast<DisplayPort::Inl*>(self)
	
	void handle_surface_size_change(Event<>& evt) {
		GUILock lock;
		m_phy_size = m_draw_ctx->selected_region().size;
		update_display_port();
	}
	
	void update_root_size() {
		Root* r = root();
		if (r) {
			r->set_width(m_size.width());
			r->set_height(m_size.height());
		}
	}
	
	void update_display_port() {
		
		if (m_lock_size.width() == 0 && m_lock_size.height() == 0) { // 使用系统默认的最合适的尺寸
			m_size = { m_phy_size.width() / m_draw_ctx->best_display_scale(),
				m_phy_size.height() / m_draw_ctx->best_display_scale() };
		}
		else if (m_lock_size.width() != 0 && m_lock_size.height() != 0) { // 尺寸全部锁定
			m_size = m_lock_size;
		}
		else if (m_lock_size.width() != 0) { // 只锁定宽度
			m_size.width(m_lock_size.width());
			m_size.height(m_size.width() / m_phy_size.width() * m_phy_size.height());
		}
		else { // m_lock_height == 0 // 只锁定高度
			m_size.height(m_lock_size.height());
			m_size.width(m_size.height() / m_phy_size.height() * m_phy_size.width());
		}
		
		m_scale_value[0] = m_phy_size.width() / m_size.width();
		m_scale_value[1] = m_phy_size.height() / m_size.height();
		m_scale = (m_scale_value[0] + m_scale_value[1]) / 2;
		
		m_atom_pixel = 1.0f / m_scale;
		
		// 计算2D视图变换矩阵
		
		CGRect region = m_draw_ctx->selected_region();
		Vec2 surface_size = m_draw_ctx->surface_size();
		
		Vec2 start = Vec2(-region.origin.x() / m_scale_value[0], -region.origin.y() / m_scale_value[1]);
		Vec2 end = Vec2(surface_size.width() / m_scale_value[0] + start.x(),
										surface_size.height() / m_scale_value[1] + start.y());
		
		m_root_matrix = Mat4::ortho(start[0], end[0], start[1], end[1], -1.0f, 1.0f);
		
		Mat4 test_root_matrix = // 测试着色器视图矩阵要大一圈
		Mat4::ortho(start[0]-5, end[0]+5, start[1]-5, end[1]+5, -1.0f, 1.0f);
		
		XX_CHECK(m_host->render_loop());
		
		update_root_size();

		m_host->render_loop()->post(Cb([this, test_root_matrix](Se& e) {
			m_draw_ctx->refresh_root_matrix(m_root_matrix, test_root_matrix);
			update_root_size();
		}));
		
		// set default draw region
		m_draw_region.first() = {
			0, 0,
			m_size.width(), m_size.height(),
			m_size.width(), m_size.height(),
		};
		
		m_host->main_loop()->post(Cb([this](Se& e){
			XX_TRIGGER(change); // 通知事件
		}));
	}
	
	/**
	 * @func solve_next_frame()
	 */
	void solve_next_frame() {
		if (m_next_frame.length()) {
			List<Callback>* cb = new List<Callback>(move(m_next_frame));
			m_host->main_loop()->post(Cb([cb](Se& e) {
				Handle<List<Callback>> handle(cb);
				for ( auto& i : *cb ) {
					i.value()->call();
				}
			}));
		}
	}
};

/**
 * @constructor
 */
DisplayPort::DisplayPort(GUIApplication* host)
: XX_INIT_EVENT(change)
, XX_INIT_EVENT(orientation)
, m_phy_size()
, m_lock_size()
, m_size()
, m_scale(1)
, m_scale_value(1)
, m_pre_render(new PreRender())
, m_draw_ctx(host->draw_ctx())
, m_root_matrix()
, m_atom_pixel(1)
, m_host(host)
, m_fsp(0)
, m_record_fsp(0)
, m_record_fsp_time(0)
{
	m_draw_region.push({ 0,0,0,0,0,0 });
	// 侦听视口尺寸变化
	XX_DEBUG("m_draw_ctx->XX_ON ...");
	m_draw_ctx->XX_ON(surface_size_change, &Inl::handle_surface_size_change, _inl(this));
	XX_DEBUG("m_draw_ctx->XX_ON ok");
}

/**
 * @destructor
 */
DisplayPort::~DisplayPort() {
	Release(m_pre_render);
	m_draw_ctx->XX_OFF(surface_size_change, &Inl::handle_surface_size_change, _inl(this));
}

float DisplayPort::best_scale() const {
	return m_draw_ctx->best_display_scale();
}

void DisplayPort::lock_size(float width, float height) {
	if (width >= 0.0 && height >= 0.0) {
		if (m_lock_size.width() != width || m_lock_size.height() != height) {
			m_lock_size = { width, height };
			_inl(this)->update_display_port();
		}
	} else {
		XX_WARN("Lock size value can not be less than zero\n");
	}
}

#define PRINT_RENDER_FRAME_TIME 0

/**
 * @func render_frame()
 */
void DisplayPort::render_frame() {
	Root* r = root();
	int64 now_time = sys::time_monotonic();
	m_host->action_center()->advance(now_time); // advance action
	
	if (r) {
		bool ok = m_pre_render->solve(now_time);
		if (ok || r->mark_value || r->m_child_change_flag) {
			
			if (now_time - m_record_fsp_time >= 1e6) {
				m_fsp = m_record_fsp;
				m_record_fsp = 0;
				m_record_fsp_time = now_time;
			}
			m_record_fsp++;
			
			m_draw_ctx->begin_render();
			r->draw(m_draw_ctx); // 开始绘图
			_inl(this)->solve_next_frame();
			
#if DEBUG && PRINT_RENDER_FRAME_TIME
			int64 st = sys::time();
#endif
			/*
			 * commit_render()非常耗时,渲染线程长时间占用`GUILock`会柱塞主线程。
			 * 所以这里释放`GUILock`，commit_render()主要是绘图相关的函数调用,
			 * 如果能够确保绘图函数的调用都在渲染线程,那就不会有安全问题。
			 */
			Inl2_RunLoop(m_host->render_loop())->independent_mutex_unlock();
			m_draw_ctx->commit_render();
			Inl2_RunLoop(m_host->render_loop())->independent_mutex_lock();
#if DEBUG && PRINT_RENDER_FRAME_TIME
			int64 ts2 = (sys::time() - st) / 1e3;
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
void DisplayPort::refresh() {
	// TODO 必须要渲染循环中调用
	Root* r = root();
	if ( r ) {
		m_pre_render->solve(sys::time_monotonic());
		m_draw_ctx->begin_render();
		r->draw(m_draw_ctx); // 开始绘图
		m_draw_ctx->commit_render();
	}
}

void DisplayPort::push_draw_region(Region re) {
	// 计算一个交集区域
	
	Region dre = m_draw_region.last();
	
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
	
	m_draw_region.push(re);
}

/**
 * @func next_frame
 */
void DisplayPort::next_frame(cCb& cb) {
	m_next_frame.push(cb);
}

XX_END

