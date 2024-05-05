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

#ifndef __quark__view__scroll__
#define __quark__view__scroll__

#include "./float.h"
#include "../../render/bezier.h"

namespace qk {
	class Scroll;

	class Qk_EXPORT ScrollBase {
	public:
		Qk_DEFINE_VIEW_PROP(bool, scrollbar, Const); // 显示scrollbar
		Qk_DEFINE_VIEW_PROP(bool, bounce, Const); // 使用回弹力
		Qk_DEFINE_VIEW_PROP(bool, bounce_lock, Const); // 使用回弹力锁定
		Qk_DEFINE_VIEW_PROP(bool, momentum, Const); // 是否使用惯性
		Qk_DEFINE_VIEW_PROP(bool, lock_direction, Const); // 锁定方向
		Qk_DEFINE_VIEW_PROP_GET(bool, scrollbar_h, Const); // 是否显示水平滚动条
		Qk_DEFINE_VIEW_PROP_GET(bool, scrollbar_v, Const); // 是否显示垂直滚动条
		Qk_DEFINE_VIEW_PROP_ACC(float, scroll_x, Const);
		Qk_DEFINE_VIEW_PROP_ACC(float, scroll_y, Const);
		Qk_DEFINE_VIEW_PROP_ACC(Vec2,  scroll, Const);
		Qk_DEFINE_VIEW_PROP_GET(Vec2, scroll_size, Const);
		Qk_DEFINE_VIEW_PROP(float, resistance, Const); // resistance default=1
		Qk_DEFINE_VIEW_PROP(float, catch_position_x, Const); // 停止后捕获位置
		Qk_DEFINE_VIEW_PROP(float, catch_position_y, Const);
		Qk_DEFINE_VIEW_PROP(Color, scrollbar_color, Const);
		Qk_DEFINE_VIEW_PROP(float, scrollbar_width, Const);
		Qk_DEFINE_VIEW_PROP(float, scrollbar_margin, Const);
		Qk_DEFINE_VIEW_PROP(uint64_t, scroll_duration, Const);
		Qk_DEFINE_VIEW_PROP_ACC(cCurve&, default_curve, Const); // default scroll curve
		// define methods
		void scrollTo(Vec2 value, uint64_t duration, cCurve& curve);
		void scrollTo(Vec2 value, uint64_t duration) {
			scrollTo(value, duration, _default_curve_Mt);
		}
		void terminate();
	protected:
		ScrollBase(Box *host);
		~ScrollBase();
		void solve(uint32_t mark); // @safe Rt
		void set_scroll_size_Rt(Vec2 size); // @safe Rt
	private:
		Qk_DEFINE_INLINE_CLASS(Inl);
		Qk_DEFINE_INLINE_CLASS(Task);
		void scroll_to_Rt(Vec2 value, uint64_t duration, cCurve& curve);
		friend class UIRender;
		Box *_host;
		List<Task*> _tasks;
		Vec2 _scroll, _scroll_max, _scroll_for_Mt;
		Vec2 _move_start_scroll, _move_point, _move_dist;
		Vec2 _scrollbar_position_h, _scrollbar_position_v;
		uint64_t _move_start_time;
		uint32_t _action_id;
		Curve _default_curve_Mt; // default curve
		float _scrollbar_opacity;
		bool _moved;               // 受外力移动中
		bool _scroll_h, _scroll_v; // 是否已激活水平与垂直滚动
		bool _lock_h, _lock_v;
	};

	class Qk_EXPORT Scroll: public Float, public ScrollBase {
	public:
		Scroll();
		virtual View* init(Window *win) override;
		virtual ViewType viewType() const override;
		virtual ScrollBase* asScrollBase() override;
		virtual Vec2 layout_offset_inside() override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void solve_marks(const Mat &mat, uint32_t mark) override;
		virtual void draw(UIRender *render) override;
	};

}
#endif
