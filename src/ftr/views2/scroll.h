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

#ifndef __ftr__scroll__
#define __ftr__scroll__

#include "ftr/panel.h"
#include "ftr/pre-render.h"
#include "ftr/bezier.h"

/**
 * @ns ftr
 */

FX_NS(ftr)

/**
 * @class BasicScroll
 */
class FX_EXPORT BasicScroll: public Protocol {
 public:

	BasicScroll(Box* box);
	
	/**
	 * @destructor
	 */
	virtual ~BasicScroll();
		
	/**
	 * @func scroll get
	 */
	inline Vec2 scroll() const {
		return Vec2(-m_scroll.x(), -m_scroll.y());
	}
	
	/**
	 * @func scroll
	 */
	void scroll_to(Vec2 value, uint64 duration);
	
	/**
	 * @func scroll
	 */
	void scroll_to(Vec2 value, uint64 duration, cCurve& curve);
	
	/**
	 * @func set_scroll set
	 */
	void set_scroll(Vec2 value);
	
	/**
	 * @func scroll_x get
	 */
	inline float scroll_x() const { return -m_scroll.x(); }
	
	/**
	 * @func scroll_y get
	 */
	inline float scroll_y() const { return -m_scroll.y(); }
	
	/**
	 * @func set_scroll_x set
	 */
	void set_scroll_x(float value);
	
	/**
	 * @func set_scroll_x set
	 */
	void set_scroll_y(float value);
	
	/**
	 * @func scroll_size
	 */
	inline Vec2 scroll_size() const { return m_scroll_size; }
	
	/**
	 * @func scroll_width
	 */
	inline float scroll_width() const { return m_scroll_size.width(); }
	
	/**
	 * @func scroll_height
	 */
	inline float scroll_height() const { return m_scroll_size.height(); }
	
	/**
	 * @func scrollbar get
	 */
	inline bool scrollbar() const { return m_scrollbar; }
	
	/**
	 * @func set_scrollbar set
	 */
	inline void set_scrollbar(bool value) { m_scrollbar = value; }
	
	/**
	 * @func resistance get
	 */
	inline float resistance() const { return m_resistance; }
	
	/**
	 * @func set_resistance set
	 */
	void set_resistance(float value);
	
	/**
	 * @func bounce get
	 */
	inline bool bounce() const { return m_bounce; }
	
	/**
	 * @func set_bounce set
	 */
	inline void set_bounce(bool value) { m_bounce = value; }
	
	/**
	 * @func bounce_lock get
	 */
	inline bool bounce_lock() const { return m_bounce_lock; }
	
	/**
	 * @func set_bounce_lock set
	 */
	inline void set_bounce_lock(bool value) { m_bounce_lock = value; }
	
	/**
	 * @func momentum get
	 */
	inline bool momentum() const { return m_momentum; }
	
	/**
	 * @func set_momentum set
	 */
	inline void set_momentum(bool value) { m_momentum = value; }
	
	/**
	 * @func lock_direction get
	 */
	inline bool lock_direction() const { return m_lock_direction; }
	
	/**
	 * @func set_lock_direction set
	 */
	inline void set_lock_direction(bool value) { m_lock_direction = value; }
	
	/**
	 * @func catch_position get
	 */
	inline Vec2 catch_position() const { return m_catch_position; }
	
	/**
	 * @func set_catch_position set
	 */
	inline void set_catch_position(Vec2 value) { m_catch_position = value; }
	
	/**
	 * @func catch_position_x get
	 */
	inline float catch_position_x() const { return m_catch_position.x(); }
	
	/**
	 * @func catch_position_x get
	 */
	inline float catch_position_y() const { return m_catch_position.x(); }
	
	/**
	 * @func set_catch_position_x set
	 */
	inline void set_catch_position_x(float value) { m_catch_position.x(value); }
	
	/**
	 * @func set_catch_position_y set
	 */
	inline void set_catch_position_y(float value) { m_catch_position.y(value); }
	
	/**
	 * @func scrollbar_color
	 */
	inline Color scrollbar_color() const { return m_scrollbar_color; }
	
	/**
	 * @func set_scrollbar_color
	 */
	inline void set_scrollbar_color(Color value) { m_scrollbar_color = value; }
	
	/**
	 * @func h_scrollbar
	 */
	inline bool h_scrollbar() const { return m_h_scrollbar; }
	
	/**
	 * @func v_scrollbar
	 */
	inline bool v_scrollbar() const { return m_h_scrollbar; }
	
	/**
	 * @func scrollbar_width get
	 */
	float scrollbar_width() const;
	
	/**
	 * @func set_scrollbar_width set
	 */
	inline void set_scrollbar_width(float value) { m_scrollbar_width = value; }
	
	/**
	 * @func scrollbar_margin get
	 */
	float scrollbar_margin() const;
	
	/**
	 * @func set_scrollbar_margin set
	 */
	inline void set_scrollbar_margin(float value) { m_scrollbar_margin = value; }
	
	/**
	 * @func default_scroll_duration
	 */
	inline uint64 default_scroll_duration() const { return m_default_scroll_duration; }
	
	/**
	 * @func set_default_scroll_duration
	 */
	inline void set_default_scroll_duration(uint64 value) { m_default_scroll_duration = value; }
	
	/**
	 * @func default_scroll_curve
	 */
	inline cCurve& default_scroll_curve() const { return *m_default_scroll_curve; }
	
	/**
	 * @func set_default_scroll_curve
	 */
	void set_default_scroll_curve(cCurve& value);
	
	/**
	 * @func terminate
	 */
	void terminate();
	
 protected:
	
	/**
	 * @func set_scroll_size
	 */
	void set_scroll_size(Vec2 size);
	
	/**
	 * @func solve
	 */
	void solve();
	
 private:
	
	FX_DEFINE_INLINE_CLASS(Inl);
	FX_DEFINE_INLINE_CLASS(Task);
	
	friend class GLDraw;
	
	List<Task*> m_tasks;
	
	Box* m_box;
	Vec2 m_raw_scroll;
	Vec2 m_scroll;
	Vec2 m_scroll_size;       /* scroll size */
	Vec2 m_scroll_max;
	Vec2 m_move_start_scroll;
	uint64 m_move_start_time;
	Vec2 m_move_point, m_move_dist;
	Vec2 m_catch_position;
	Vec2 m_h_scrollbar_position, m_v_scrollbar_position;
	uint m_action_id;
	Color m_scrollbar_color;
	float m_scrollbar_width, m_scrollbar_margin, m_scrollbar_opacity;
	float m_resistance;     /* 阻力 */
	uint64 m_default_scroll_duration;
	Curve* m_default_scroll_curve;
	bool m_moved;           /* 受外力移动中 */
 protected: bool m_h_scroll, m_v_scroll;     /* 是否激活水平与垂直滚动 */
 private: bool m_h_scrollbar, m_v_scrollbar; /* 是否显示水平与垂直滚动条 */
	bool m_bounce_lock;
	bool m_lock_h, m_lock_v;
	bool m_lock_direction;  /* 锁定方向 */
	bool m_bounce;          /* 使用反弹力 */
	bool m_momentum;        /* 是否使用惯性 */
	bool m_scrollbar;       /* 显示scrollbar */
};

/**
 * @class Scroll
 */
class FX_EXPORT Scroll: public Panel, public BasicScroll {
	FX_DEFINE_GUI_VIEW(SCROLL, Scroll, scroll);
 public:

	typedef ReferenceTraits Traits;
	
	Scroll();

	/**
	 * @overwrite
	 */
	virtual Vec2 layout_in_offset();
	// virtual bool is_clip() { return true; }
	virtual BasicScroll* as_basic_scroll() { return this; }
	virtual Object* to_object() { return this; }

	/**
	 * @func focus_margin_left get
	 */
	inline float focus_margin_left() const { return m_focus_margin_left; }
	
	/**
	 * @func focus_margin_right get
	 */
	inline float focus_margin_right() const { return m_focus_margin_right; }
	
	/**
	 * @func focus_margin_top get
	 */
	inline float focus_margin_top() const { return m_focus_margin_top; }
	
	/**
	 * @func focus_margin_bottom get
	 */
	inline float focus_margin_bottom() const { return m_focus_margin_bottom; }
	
	/**
	 * @func focus_align_x get
	 */
	inline Align focus_align_x() const { return m_focus_align_x; }
	
	/**
	 * @func focus_align_y get
	 */
	inline Align focus_align_y() const { return m_focus_align_y; }
	
	/**
	 * @func set_focus_margin_left set
	 */
	inline void set_focus_margin_left(float value) { m_focus_margin_left = value; }
	
	/**
	 * @func set_focus_margin_right set
	 */
	inline void set_focus_margin_right(float value) { m_focus_margin_right = value; }
	
	/**
	 * @func set_focus_margin_top set
	 */
	inline void set_focus_margin_top(float value) { m_focus_margin_top = value; }
	
	/**
	 * @func set_focus_margin_bottom set
	 */
	inline void set_focus_margin_bottom(float value) { m_focus_margin_bottom = value; }
	
	/**
	 * @func set_focus_align_x set
	 */
	void set_focus_align_x(Align value);
	
	/**
	 * @func set_focus_align_y set
	 */
	void set_focus_align_y(Align value);
	
	/**
	 * @func enable_focus_align get
	 */
	inline bool enable_focus_align() const { return m_enable_focus_align; }
	
	/**
	 * @func set_enable_select_scroll set
	 */
	void set_enable_focus_align(bool value);
	
	/**
	 * @func enable_fixed_scroll_size
	 */
	inline bool enable_fixed_scroll_size() const { return m_enable_fixed_scroll_size; }
	
	/**
	 * @func set_enable_fixed_scroll_size
	 */
	void set_enable_fixed_scroll_size(Vec2 size);
	
 protected:
	
	/**
	 * @overwrite
	 */
	virtual void draw(Draw* draw);
	virtual void set_layout_content_offset();
	
 private:
	
	FX_DEFINE_INLINE_CLASS(Inl);
	
	float m_focus_margin_left;
	float m_focus_margin_right;
	float m_focus_margin_top;
	float m_focus_margin_bottom;
	Align m_focus_align_x;
	Align m_focus_align_y;
	bool  m_enable_focus_align;
	bool  m_enable_fixed_scroll_size;
	
};

FX_END
#endif
