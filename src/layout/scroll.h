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

#ifndef __noug__layout__scroll__
#define __noug__layout__scroll__

#include "./flow.h"
#include "../bezier.h"

namespace noug {

	class N_EXPORT BaseScroll {
	public:
		// define props
		N_DEFINE_PROP(bool, scrollbar); // 显示scrollbar
		N_DEFINE_PROP(bool, bounce);    // 使用回弹力
		N_DEFINE_PROP(bool, bounce_lock); // 使用回弹力锁定
		N_DEFINE_PROP(bool, momentum); // 是否使用惯性
		N_DEFINE_PROP(bool, lock_direction); // 锁定方向
		N_DEFINE_PROP_READ(bool, scrollbar_h); // 是否显示水平滚动条
		N_DEFINE_PROP_READ(bool, scrollbar_v); // 是否显示垂直滚动条
		N_DEFINE_ACCESSOR(float, scroll_x);
		N_DEFINE_ACCESSOR(float, scroll_y);
		N_DEFINE_ACCESSOR(Vec2,  scroll);
		N_DEFINE_PROP_READ(Vec2, scroll_size);
		N_DEFINE_PROP(float, resistance); // resistance default=1
		N_DEFINE_PROP(float, catch_position_x); // 停止后捕获位置
		N_DEFINE_PROP(float, catch_position_y);
		N_DEFINE_PROP(Color, scrollbar_color);
		N_DEFINE_PROP(float, scrollbar_width);
		N_DEFINE_PROP(float, scrollbar_margin);
		N_DEFINE_PROP(uint64_t, scroll_duration);
		N_DEFINE_PROP(cCurve*, scroll_curve);
		// constructor
		BaseScroll(Box *host);
		virtual ~BaseScroll();
		// define methods
		void scroll_to(Vec2 value, uint64_t duration);
		void scroll_to(Vec2 value, uint64_t duration, cCurve& curve);
		void terminate();
	protected:
		void set_scroll_size(Vec2 size);
		void solve();
	private:
		N_DEFINE_INLINE_CLASS(Inl);
		N_DEFINE_INLINE_CLASS(Task);
		Box *_host;
		List<Task*> _tasks;
		Vec2 _scroll_raw, _scroll, _scroll_max;
		Vec2 _move_start_scroll, _move_point, _move_dist;
		Vec2 _scrollbar_position_h, _scrollbar_position_v;
		uint64_t _move_start_time;
		uint32_t _action_id;
		float _scrollbar_opacity;
		bool _moved;               // 受外力移动中
		bool _scroll_h, _scroll_v; // 是否已激活水平与垂直滚动
		bool _lock_h, _lock_v;
	};

	class N_EXPORT Scroll: public FlowLayout, public BaseScroll {
		N_Define_View(Scroll);
	public:
	private:
	};

}
#endif
