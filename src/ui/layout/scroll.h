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

#ifndef __quark__layout__scroll__
#define __quark__layout__scroll__

#include "./float.h"
#include "../../render/bezier.h"

namespace qk {
	class ScrollLayout;

	class Qk_EXPORT ScrollLayoutBase {
		Qk_DEFINE_INLINE_CLASS(Inl);
		Qk_DEFINE_INLINE_CLASS(Task);
	public:
		Qk_DEFINE_PROP(bool, scrollbar); // 显示scrollbar
		Qk_DEFINE_PROP(bool, bounce);    // 使用回弹力
		Qk_DEFINE_PROP(bool, bounce_lock); // 使用回弹力锁定
		Qk_DEFINE_PROP(bool, momentum); // 是否使用惯性
		Qk_DEFINE_PROP(bool, lock_direction); // 锁定方向
		Qk_DEFINE_PROP_GET(bool, scrollbar_h); // 是否显示水平滚动条
		Qk_DEFINE_PROP_GET(bool, scrollbar_v); // 是否显示垂直滚动条
		Qk_DEFINE_PROP_ACC(float, scroll_x);
		Qk_DEFINE_PROP_ACC(float, scroll_y);
		Qk_DEFINE_PROP_ACC(Vec2,  scroll);
		Qk_DEFINE_PROP_GET(Vec2, scroll_size);
		Qk_DEFINE_PROP(float, resistance); // resistance default=1
		Qk_DEFINE_PROP(float, catch_position_x); // 停止后捕获位置
		Qk_DEFINE_PROP(float, catch_position_y);
		Qk_DEFINE_PROP(Color, scrollbar_color);
		Qk_DEFINE_PROP(float, scrollbar_width);
		Qk_DEFINE_PROP(float, scrollbar_margin);
		Qk_DEFINE_PROP(uint64_t, scroll_duration);
		Qk_DEFINE_PROP(cCurve*, scroll_curve);
		void terminate();
		void scroll_to(Vec2 value, uint64_t duration);
		void scroll_to(Vec2 value, uint64_t duration, cCurve& curve);
	protected:
		ScrollLayoutBase(BoxLayout *host);
		~ScrollLayoutBase();
		void set_scroll_size(Vec2 size);
		void solve(uint32_t mark);
	private:
		friend class UIRender;
		friend class ScrollLayoutBaseAsync;
		BoxLayout *_host;
		List<Task*> _tasks;
		Vec2 _scroll, _scroll_max, _scroll_for_main_t;
		Vec2 _move_start_scroll, _move_point, _move_dist;
		Vec2 _scrollbar_position_h, _scrollbar_position_v;
		uint64_t _move_start_time;
		uint32_t _action_id;
		float _scrollbar_opacity;
		bool _moved;               // 受外力移动中
		bool _scroll_h, _scroll_v; // 是否已激活水平与垂直滚动
		bool _lock_h, _lock_v;
	};

	class Qk_EXPORT ScrollLayoutBaseAsync {
	public:
		Qk_DEFINE_PROP_ACC(bool, scrollbar);
		Qk_DEFINE_PROP_ACC(bool, bounce);   
		Qk_DEFINE_PROP_ACC(bool, bounce_lock);
		Qk_DEFINE_PROP_ACC(bool, momentum);
		Qk_DEFINE_PROP_ACC(bool, lock_direction);
		Qk_DEFINE_PROP_ACC_GET(bool, scrollbar_h);
		Qk_DEFINE_PROP_ACC_GET(bool, scrollbar_v);
		Qk_DEFINE_PROP_ACC(float, scroll_x);
		Qk_DEFINE_PROP_ACC(float, scroll_y);
		Qk_DEFINE_PROP_ACC(Vec2,  scroll);
		Qk_DEFINE_PROP_ACC_GET(Vec2, scroll_size);
		Qk_DEFINE_PROP_ACC(float, resistance);
		Qk_DEFINE_PROP_ACC(float, catch_position_x);
		Qk_DEFINE_PROP_ACC(float, catch_position_y);
		Qk_DEFINE_PROP_ACC(Color, scrollbar_color);
		Qk_DEFINE_PROP_ACC(float, scrollbar_width);
		Qk_DEFINE_PROP_ACC(float, scrollbar_margin);
		Qk_DEFINE_PROP_ACC(uint64_t, scroll_duration);
		Qk_DEFINE_PROP_ACC(cCurve*, scroll_curve);
		void scroll_to(Vec2 value, uint64_t duration);
		void scroll_to(Vec2 value, uint64_t duration, cCurve& curve);
		void terminate();
		virtual ScrollLayoutBase* getScrollLayoutBase() const = 0;
		virtual PreRender& getPreRender() = 0;
	protected:
		ScrollLayoutBaseAsync(ScrollLayoutBase *layout, View *v);
	};

	// -------------------------------

	class Qk_EXPORT ScrollLayout: public FloatLayout, public ScrollLayoutBase {
	public:
		ScrollLayout(Window *win);
		virtual Vec2 layout_offset_inside() override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void solve_marks(const Mat &mat, uint32_t mark) override;
		virtual void draw(UIRender *render) override;
		virtual ScrollLayoutBase* asScrollLayoutBase() override;
		virtual ViewType viewType() const override;
	};

	class Qk_EXPORT Scroll: public Float, public ScrollLayoutBaseAsync {
	public:
		typedef ScrollLayout Layout;
		Scroll(ScrollLayout *layout);
		virtual ScrollLayoutBase* getScrollLayoutBase() const override;
		virtual PreRender& getPreRender() override;
	};

}
#endif
