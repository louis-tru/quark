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

namespace ftr {

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
		return Vec2(-_scroll.x(), -_scroll.y());
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
	inline float scroll_x() const { return -_scroll.x(); }
	
	/**
	 * @func scroll_y get
	 */
	inline float scroll_y() const { return -_scroll.y(); }
	
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
	inline Vec2 scroll_size() const { return _scroll_size; }
	
	/**
	 * @func scroll_width
	 */
	inline float scroll_width() const { return _scroll_size.width(); }
	
	/**
	 * @func scroll_height
	 */
	inline float scroll_height() const { return _scroll_size.height(); }
	
	/**
	 * @func scrollbar get
	 */
	inline bool scrollbar() const { return _scrollbar; }
	
	/**
	 * @func set_scrollbar set
	 */
	inline void set_scrollbar(bool value) { _scrollbar = value; }
	
	/**
	 * @func resistance get
	 */
	inline float resistance() const { return _resistance; }
	
	/**
	 * @func set_resistance set
	 */
	void set_resistance(float value);
	
	/**
	 * @func bounce get
	 */
	inline bool bounce() const { return _bounce; }
	
	/**
	 * @func set_bounce set
	 */
	inline void set_bounce(bool value) { _bounce = value; }
	
	/**
	 * @func bounce_lock get
	 */
	inline bool bounce_lock() const { return _bounce_lock; }
	
	/**
	 * @func set_bounce_lock set
	 */
	inline void set_bounce_lock(bool value) { _bounce_lock = value; }
	
	/**
	 * @func momentum get
	 */
	inline bool momentum() const { return _momentum; }
	
	/**
	 * @func set_momentum set
	 */
	inline void set_momentum(bool value) { _momentum = value; }
	
	/**
	 * @func lock_direction get
	 */
	inline bool lock_direction() const { return _lock_direction; }
	
	/**
	 * @func set_lock_direction set
	 */
	inline void set_lock_direction(bool value) { _lock_direction = value; }
	
	/**
	 * @func catch_position get
	 */
	inline Vec2 catch_position() const { return _catch_position; }
	
	/**
	 * @func set_catch_position set
	 */
	inline void set_catch_position(Vec2 value) { _catch_position = value; }
	
	/**
	 * @func catch_position_x get
	 */
	inline float catch_position_x() const { return _catch_position.x(); }
	
	/**
	 * @func catch_position_x get
	 */
	inline float catch_position_y() const { return _catch_position.x(); }
	
	/**
	 * @func set_catch_position_x set
	 */
	inline void set_catch_position_x(float value) { _catch_position.x(value); }
	
	/**
	 * @func set_catch_position_y set
	 */
	inline void set_catch_position_y(float value) { _catch_position.y(value); }
	
	/**
	 * @func scrollbar_color
	 */
	inline Color scrollbar_color() const { return _scrollbar_color; }
	
	/**
	 * @func set_scrollbar_color
	 */
	inline void set_scrollbar_color(Color value) { _scrollbar_color = value; }
	
	/**
	 * @func h_scrollbar
	 */
	inline bool h_scrollbar() const { return _h_scrollbar; }
	
	/**
	 * @func v_scrollbar
	 */
	inline bool v_scrollbar() const { return _h_scrollbar; }
	
	/**
	 * @func scrollbar_width get
	 */
	float scrollbar_width() const;
	
	/**
	 * @func set_scrollbar_width set
	 */
	inline void set_scrollbar_width(float value) { _scrollbar_width = value; }
	
	/**
	 * @func scrollbar_margin get
	 */
	float scrollbar_margin() const;
	
	/**
	 * @func set_scrollbar_margin set
	 */
	inline void set_scrollbar_margin(float value) { _scrollbar_margin = value; }
	
	/**
	 * @func default_scroll_duration
	 */
	inline uint64 default_scroll_duration() const { return _default_scroll_duration; }
	
	/**
	 * @func set_default_scroll_duration
	 */
	inline void set_default_scroll_duration(uint64 value) { _default_scroll_duration = value; }
	
	/**
	 * @func default_scroll_curve
	 */
	inline cCurve& default_scroll_curve() const { return *_default_scroll_curve; }
	
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
	
	List<Task*> _tasks;
	
	Box* _box;
	Vec2 _raw_scroll;
	Vec2 _scroll;
	Vec2 _scroll_size;       /* scroll size */
	Vec2 _scroll_max;
	Vec2 _move_start_scroll;
	uint64 _move_start_time;
	Vec2 _move_point, _move_dist;
	Vec2 _catch_position;
	Vec2 _h_scrollbar_position, _v_scrollbar_position;
	uint _action_id;
	Color _scrollbar_color;
	float _scrollbar_width, _scrollbar_margin, _scrollbar_opacity;
	float _resistance;     /* 阻力 */
	uint64 _default_scroll_duration;
	Curve* _default_scroll_curve;
	bool _moved;           /* 受外力移动中 */
 protected: bool _h_scroll, _v_scroll;     /* 是否激活水平与垂直滚动 */
 private: bool _h_scrollbar, _v_scrollbar; /* 是否显示水平与垂直滚动条 */
	bool _bounce_lock;
	bool _lock_h, _lock_v;
	bool _lock_direction;  /* 锁定方向 */
	bool _bounce;          /* 使用反弹力 */
	bool _momentum;        /* 是否使用惯性 */
	bool _scrollbar;       /* 显示scrollbar */
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
	inline float focus_margin_left() const { return _focus_margin_left; }
	
	/**
	 * @func focus_margin_right get
	 */
	inline float focus_margin_right() const { return _focus_margin_right; }
	
	/**
	 * @func focus_margin_top get
	 */
	inline float focus_margin_top() const { return _focus_margin_top; }
	
	/**
	 * @func focus_margin_bottom get
	 */
	inline float focus_margin_bottom() const { return _focus_margin_bottom; }
	
	/**
	 * @func focus_align_x get
	 */
	inline Align focus_align_x() const { return _focus_align_x; }
	
	/**
	 * @func focus_align_y get
	 */
	inline Align focus_align_y() const { return _focus_align_y; }
	
	/**
	 * @func set_focus_margin_left set
	 */
	inline void set_focus_margin_left(float value) { _focus_margin_left = value; }
	
	/**
	 * @func set_focus_margin_right set
	 */
	inline void set_focus_margin_right(float value) { _focus_margin_right = value; }
	
	/**
	 * @func set_focus_margin_top set
	 */
	inline void set_focus_margin_top(float value) { _focus_margin_top = value; }
	
	/**
	 * @func set_focus_margin_bottom set
	 */
	inline void set_focus_margin_bottom(float value) { _focus_margin_bottom = value; }
	
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
	inline bool enable_focus_align() const { return _enable_focus_align; }
	
	/**
	 * @func set_enable_select_scroll set
	 */
	void set_enable_focus_align(bool value);
	
	/**
	 * @func enable_fixed_scroll_size
	 */
	inline bool enable_fixed_scroll_size() const { return _enable_fixed_scroll_size; }
	
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
	
	float _focus_margin_left;
	float _focus_margin_right;
	float _focus_margin_top;
	float _focus_margin_bottom;
	Align _focus_align_x;
	Align _focus_align_y;
	bool  _enable_focus_align;
	bool  _enable_fixed_scroll_size;
	
};

}
#endif
