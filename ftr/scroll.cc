/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "sys.h"
#include "scroll.h"
#include "bezier.h"
#include "app.h"
#include "display-port.h"
#include "button.h"

FX_NS(ftr)

static cCurve ease_in_out(0.3, 0.3, 0.3, 1);
static cCurve ease_out(0, 0, 0.58, 1);

/**
 * @class Scroll::Task
 */
class BasicScroll::Task: public PreRender::Task {
 public:
	
	Task(BasicScroll* host, uint64 duration, cCurve& curve = ease_out)
	: m_host(host)
	, m_start_time(sys::time_monotonic())
	, m_duration(duration)
	, m_immediate_end_flag(false)
	, m_curve(curve)
	, m_is_inl_ease_out(&ease_out == &curve)
	{
	
	}
	
	/**
	 * @func immediate_end_flag
	 */
	inline void immediate_end_flag() {
		m_immediate_end_flag = true;
	}
	
	virtual void run(float y) = 0;
	virtual void end() = 0;
	virtual void immediate_end() = 0;
	
	/**
	 * @overwrite
	 */
	virtual bool run_task(int64 sys_time) {
		
		if ( m_immediate_end_flag ) { // immediate end motion
			immediate_end();
		} else {
			int64 now = sys::time_monotonic();
			if ( now >= m_start_time + m_duration ) {
				end();
			} else {
				if ( m_is_inl_ease_out ) {
					// ease_out
					float y = float(now - m_start_time) / float(m_duration);
					run( sqrtf(1 - powf(y - 1, 2)) );
				} else {
					run( m_curve.solve(float(now - m_start_time) / float(m_duration), 0.001) );
				}
			}
		}
		return false;
	}
	
 protected:

	BasicScroll* m_host;
	uint64 m_start_time;
	uint64 m_duration;
	List<Task*>::Iterator m_id2;
	bool   m_immediate_end_flag;
	cCurve m_curve;
	bool m_is_inl_ease_out;
	
	friend class BasicScroll::Inl;
};

/**
 * @class BasicScroll::Inl
 */
class BasicScroll::Inl: public BasicScroll {
 public:
 #define _inl(self) static_cast<BasicScroll::Inl*>(static_cast<BasicScroll*>(self))
	
	struct Momentum {
		float dist;
		uint64 time;
	};
	
	/**
	 * @class ScrollMotionTask
	 */
	class ScrollMotionTask: public BasicScroll::Task {
	 public:
		
		ScrollMotionTask(BasicScroll* host, uint64 duration, Vec2 to, cCurve& curve = ease_out)
		: Task(host, duration, curve)
		, m_from(host->m_scroll)
		, m_to(to)
		{ }
		
		virtual void run(float y) {
			Vec2 scroll = Vec2((m_to.x() - m_from.x()) * y + m_from.x(),
												 (m_to.y() - m_from.y()) * y + m_from.y());
			_inl(m_host)->set_scroll_and_trigger_event(scroll);
		}
		virtual void end() {
			_inl(m_host)->set_scroll_and_trigger_event(m_to);
			_inl(m_host)->termination_recovery(3e5, ease_in_out);
		}
		virtual void immediate_end() {
			_inl(m_host)->set_scroll_and_trigger_event(m_to);
			_inl(m_host)->termination_recovery(0, ease_in_out);
		}
		
	 private:
		Vec2  m_from;
		Vec2  m_to;
	};
	
	/**
	 * @class ScrollBarFadeInOutTask
	 */
	class ScrollBarFadeInOutTask: public Scroll::Task {
	 public:
		
		ScrollBarFadeInOutTask(BasicScroll* host, uint64 duration, float to, cCurve& curve = ease_out)
		: Task(host, duration, curve)
		, m_from(host->m_scrollbar_opacity)
		, m_to(to)
		{ }
		
		virtual void run(float y) {
			m_host->m_scrollbar_opacity = (m_to - m_from) * y + m_from;
			m_host->m_box->mark(View::M_SCROLL_BAR);
		}
		virtual void end() {
			m_host->m_scrollbar_opacity = m_to;
			m_host->m_box->mark(View::M_SCROLL_BAR);
			_inl(m_host)->termination_task(this);
		}
		virtual void immediate_end() {
			m_host->m_scrollbar_opacity = m_to;
			m_host->m_box->mark(View::M_SCROLL_BAR);
			_inl(m_host)->termination_task(this);
		}
		
	 private:
		float m_from;
		float m_to;
	};
	
	friend class BasicScroll::Task;
	friend class ScrollMotionTask;
	friend class ScrollBarFadeOutTask;
	
	/**
	 * @func register_task
	 */
	void register_task(Task* task) {
		if ( !task->is_register_task() ) {
			task->m_id2 = m_tasks.push(task);
			task->register_task();
			task->run_task(0);
		}
	}
	
	/**
	 * @func termination_task
	 */
	void termination_task(Task* task) {
		if ( task->is_register_task() ) {
			m_tasks.del( task->m_id2 );
			delete task;
		}
	}
	
	/**
	 * @func termination_task
	 */
	void termination_all_task() {
		for ( auto& i : m_tasks ) {
			delete i.value();
		}
		m_tasks.clear();
	}
	
	/**
	 * @func immediate_end_all_task
	 */
	void immediate_end_all_task() {
		for ( auto& i : m_tasks ) {
			i.value()->immediate_end_flag();
		}
	}
	
	/**
	 * @func is_task
	 */
	inline bool is_task() {
		return m_tasks.length();
	}
	
	/**
	 * @func momentum
	 */
	Momentum momentum(uint64 time, float dist,
										float max_dist_upper, float max_dist_lower, float size) {
		
		float deceleration = 0.001 * m_resistance;
		float speed = fabsf(dist) / float(time) * 1000.0;
		float new_dist = (speed * speed) / (2 * deceleration);
		float outside_dist = 0;
		
		// Proportinally reduce speed if we are outside of the boundaries
		if (dist > 0 && new_dist > max_dist_upper) {
			outside_dist = size / (6 / (new_dist / speed * deceleration));
			max_dist_upper = max_dist_upper + outside_dist;
			speed = speed * max_dist_upper / new_dist;
			new_dist = max_dist_upper;
		}
		else if (dist < 0 && new_dist > max_dist_lower) {
			outside_dist = size / (6 / (new_dist / speed * deceleration));
			max_dist_lower = max_dist_lower + outside_dist;
			speed = speed * max_dist_lower / new_dist;
			new_dist = max_dist_lower;
		}
		
		new_dist = new_dist * (dist < 0 ? -1 : 1);
		uint64 new_time = speed / deceleration * 1000;
		
		return { new_dist, new_time };
	}
	
	/**
	 * @func get_catch_value
	 */
	Vec2 get_catch_value() {
		float x = (m_catch_position.x() < 1 ||
							 m_catch_position.x() > m_box->final_width()) ?
							 m_box->final_width() : m_catch_position.x();
		float y = (m_catch_position.y() < 1 ||
							 m_catch_position.y() > m_box->final_width()) ?
							 m_box->final_height() : m_catch_position.y();
		if (x == 0.0) x = 1.0;
		if (y == 0.0) y = 1.0;
		return Vec2(x, y);
	}
	
	/**
	 * @func get_valid_scroll
	 */
	Vec2 get_valid_scroll(float x, float y) {
		x = x >= 0 ? 0 : x < m_scroll_max.x() ? m_scroll_max.x() : x;
		y = y >= 0 ? 0 : y < m_scroll_max.y() ? m_scroll_max.y() : y;
		return Vec2(x, y);
	}
	
	Vec2 optimal_display(Vec2 value) {
		Vec2 scale = display_port()->scale_value();
		value.x( round(value.x() * scale.x()) / scale.x() );
		value.y( round(value.y() * scale.y()) / scale.y() );
		return value;
	}
	
	/**
	 * @func catch_valid_scroll
	 */
	Vec2 catch_valid_scroll(Vec2 scroll) {
		Vec2 valid = get_valid_scroll(scroll.x(), scroll.y());
		Vec2 Catch = get_catch_value();
		
		if ( Catch.x() != 1 && Catch.y() != 1 ) { // 捕获位置
			valid.x( roundf(valid.x() / Catch.x()) * Catch.x() );
			if ( valid.x() < m_scroll_max.x() ) {
				valid.x( valid.x() + Catch.x() );
			}
			valid.y( roundf(valid.y() / Catch.y()) * Catch.y() );
			if ( valid.y() < m_scroll_max.y() ) {
				valid.y( valid.y() + Catch.y() );
			}
		}
		return optimal_display(valid);
	}
	
	/**
	 * @func set_h_scrollbar_pos
	 */
	void set_h_scrollbar_pos() {
		
		if ( ! m_h_scrollbar ) {
			return;
		}
		
		float left_margin = scrollbar_margin();
		float right_margin = m_v_scrollbar ? left_margin + scrollbar_width() : left_margin;
		float h_scrollbar_max_size = FX_MAX(m_box->final_width() - left_margin - right_margin, 0);
		
		float h_scrollbar_indicator_size = roundf(powf(h_scrollbar_max_size, 2) / m_scroll_size.x());
		h_scrollbar_indicator_size = FX_MAX(h_scrollbar_indicator_size, 8);
		float h_scrollbar_max_scroll = h_scrollbar_max_size - h_scrollbar_indicator_size;
		float h_scrollbar_prop = h_scrollbar_max_scroll / m_scroll_max.x();
		
		// ------------------------------------------------------
		
		float pos = h_scrollbar_prop * m_scroll.x();
		float size = h_scrollbar_indicator_size;
		
		if ( pos < 0 ) {
			size = h_scrollbar_indicator_size + roundf(pos * 3);
			size = FX_MAX(size, 8);
			pos = 0;
		} else if ( pos > h_scrollbar_max_scroll ) {
			size = h_scrollbar_indicator_size - roundf((pos - h_scrollbar_max_scroll) * 3);
			size = FX_MAX(size, 8);
			pos = h_scrollbar_max_scroll + h_scrollbar_indicator_size - size;
		}
		
		m_h_scrollbar_position = Vec2(pos + left_margin, size);
	}
	
	/**
	 * @func set_v_scrollbar_pos
	 */
	void set_v_scrollbar_pos() {
		
		if ( ! m_v_scrollbar ) {
			return;
		}
		
		float top_margin = scrollbar_margin();
		float bottom_margin = m_h_scrollbar ? top_margin + scrollbar_width() : top_margin;
		float v_scrollbar_max_size = FX_MAX(m_box->final_height() - top_margin - bottom_margin, 0);
		
		float v_scrollbar_indicator_size = roundf(powf(v_scrollbar_max_size, 2) / m_scroll_size.y());
		v_scrollbar_indicator_size = FX_MAX(v_scrollbar_indicator_size, 8);
		
		float v_scrollbar_max_scroll = v_scrollbar_max_size - v_scrollbar_indicator_size;
		float v_scrollbar_prop = v_scrollbar_max_scroll / m_scroll_max.y();
		
		// ------------------------------------------------------
		
		float pos = v_scrollbar_prop * m_scroll.y();
		float size = v_scrollbar_indicator_size;
		
		if ( pos < 0 ) {
			size = v_scrollbar_indicator_size + roundf(pos * 3);
			size = FX_MAX(size, 8);
			pos = 0;
		} else if ( pos > v_scrollbar_max_scroll ) {
			size = v_scrollbar_indicator_size - roundf((pos - v_scrollbar_max_scroll) * 3);
			size = FX_MAX(size, 8);
			pos = v_scrollbar_max_scroll + v_scrollbar_indicator_size - size;
		}
		
		m_v_scrollbar_position = Vec2(pos + top_margin, size);
	}
	
	/**
	 * @func set_scroll_and_trigger_event
	 */
	void set_scroll_and_trigger_event(Vec2 scroll) {
		
		scroll = optimal_display(scroll);
		scroll.x( m_h_scroll ? scroll.x() : 0 );
		scroll.y( m_v_scroll ? scroll.y() : 0 );
		
		if ( m_scroll.x() != scroll.x() || m_scroll.y() != scroll.y() ) {
			
			m_scroll = scroll;
			m_raw_scroll = scroll;
			
			set_h_scrollbar_pos();
			set_v_scrollbar_pos();
			
			m_box->mark(View::M_SCROLL); // mark
			
			main_loop()->post(Cb([this](CbD& se) {
				Handle<GUIEvent> evt = New<GUIEvent>(m_box);
				m_box->trigger(GUI_EVENT_SCROLL, **evt); // trigger event
			}, m_box));
		}
	}
	
	/**
	 * @func scroll_to_valid_scroll
	 */
	void scroll_to_valid_scroll(Vec2 valid_scroll, uint64 duration, cCurve& curve = ease_out) {
		termination_all_task();
		 if ( duration ) {
			motion_start(valid_scroll, duration, curve);
		} else {
			set_scroll_and_trigger_event(valid_scroll);
		}
	}
	
	/**
	 * @func termination_recovery scroll position
	 */
	void termination_recovery(uint64 duration, cCurve& curve = ease_out) {
		termination_all_task();
		
		Vec2 scroll = catch_valid_scroll(m_scroll);
		if ( scroll.x() == m_scroll.x() && scroll.y() == m_scroll.y() ) {
			if ( duration ) {
				if ( m_scrollbar_opacity != 0 ) {
					register_task( new ScrollBarFadeInOutTask(this, 2e5, 0) );
				}
			} else {
				if ( m_scrollbar_opacity != 0 ) {
					m_scrollbar_opacity = 0;
					m_box->mark(View::M_SCROLL_BAR);
				}
			}
		} else {
			scroll_to_valid_scroll(scroll, duration, curve);
		}
	}
	
	/**
	 * @func motion_start
	 */
	void motion_start(Vec2 scroll, uint64 duration, cCurve& curve) {
		
		if ( !is_task() && ! m_moved ) {
			
			if ( scroll.x() != m_scroll.x() || scroll.y() != m_scroll.y() ) {
				register_task( new ScrollMotionTask(this, duration, scroll, curve) );
				if ( m_scrollbar_opacity != 1 ) {
					register_task( new ScrollBarFadeInOutTask(this, 5e4, 1) );
				}
			}
		}
	}
	
	/**
	 * @func move_start
	 */
	void move_start(Vec2 point) {
		
		termination_all_task();
		
		m_moved = false;
		m_move_dist = Vec2();
		m_move_point = point;
		m_move_start_time = sys::time_monotonic();
		m_move_start_scroll = m_scroll;
	}
	
	/**
	 * @func move
	 */
	void move(Vec2 point) {
		
		float delta_x = point.x() - m_move_point.x();
		float delta_y = point.y() - m_move_point.y();
		float new_x = m_scroll.x() + delta_x;
		float new_y = m_scroll.y() + delta_y;
		
		m_move_point = point;
		
		// Slow down if outside of the boundaries
		if ( new_x > 0 || new_x < m_scroll_max.x() ) {
			if ( m_bounce ) {
				new_x = m_scroll.x() + (delta_x / 2);
			} else {
				new_x = (new_x >= 0 || m_scroll_max.x() >= 0 ? 0 : m_scroll_max.x());
			}
		}
		if ( new_y > 0 || new_y < m_scroll_max.y() ) {
			if ( m_bounce ) {
				new_y = m_scroll.y() + delta_y / 2;
			} else {
				new_y = (new_y >= 0 || m_scroll_max.y() >= 0 ? 0 : m_scroll_max.y());
			}
		}
		
		m_move_dist.x( m_move_dist.x() + delta_x );
		m_move_dist.y( m_move_dist.y() + delta_y );
		
		float dist_x = fabsf(m_move_dist.x());
		float dist_y = fabsf(m_move_dist.y());
		
		if ( !m_moved ) {
			if ( dist_x < 3 && dist_y < 3 ) { // 距离小余3不处理
				return;
			}
			if ( m_scrollbar_opacity != 1 ) {
				register_task( new ScrollBarFadeInOutTask(this, 2e5, 1) );
			}
			m_moved = true;
		}
		
		// Lock direction
		if ( m_lock_direction ) {
			
			if ( m_lock_v ) {
				new_y = m_scroll.y();
				delta_y = 0;
			} else if( m_lock_h ) {
				new_x = m_scroll.x();
				delta_x = 0;
			}
			else {
				if ( dist_x > dist_y + 2 ) {
					m_lock_v = true;
				} else if ( dist_y > dist_x + 2 ) {
					m_lock_h = true;
				}
			}
		}
		
		uint64 time = sys::time_monotonic();
		
		if (int64(time) - m_move_start_time > 3e5) {
			m_move_start_time = time;
			m_move_start_scroll = m_scroll;
		}
		
		set_scroll_and_trigger_event(Vec2(new_x, new_y));
	}
	
	/**
	 * @func move_end
	 */
	void move_end(Vec2 point) {
		
		uint64 time = sys::time_monotonic();
		
		Momentum momentum_x = { 0,0 };
		Momentum momentum_y = { 0,0 };
		
		uint64 duration = int64(time) - m_move_start_time;
		float new_x = m_scroll.x();
		float new_y = m_scroll.y();
		
		m_lock_h = false;
		m_lock_v = false;
		
		//计算惯性
		if ( duration < 3e5 ) {
			
			if ( m_momentum ) {
				if ( new_x ) {
					momentum_x = momentum(duration, new_x - m_move_start_scroll.x(),
																-m_scroll.x(), m_scroll.x() - m_scroll_max.x(),
																m_bounce ? m_box->final_width() / 2.0 : 0);
				}
				if ( new_y ) {
					momentum_y = momentum(duration, new_y - m_move_start_scroll.y(),
																-m_scroll.y(), m_scroll.y() - m_scroll_max.y(),
																m_bounce ? m_box->final_height() / 2.0 : 0);
				}
				new_x = m_scroll.x() + momentum_x.dist;
				new_y = m_scroll.y() + momentum_y.dist;
				
				if ((m_scroll.x() > 0 && new_x > 0) ||
						(m_scroll.x() < m_scroll_max.x() && new_x < m_scroll_max.x())) {
					momentum_x = { 0, 0 };
				}
				if ((m_scroll.y() > 0 && new_y > 0) ||
						(m_scroll.y() < m_scroll_max.y() && new_y < m_scroll_max.y())) {
					momentum_y = { 0, 0 };
				}
			}
			
			//捕获位置
			Vec2 Catch = get_catch_value();
			
			float mod_x = int(roundf(new_x)) % uint(Catch.x());
			float mod_y = int(roundf(new_y)) % uint(Catch.y());
			float dist_x, dist_y;
			
			if ( new_x < 0 && new_x > m_scroll_max.x() && mod_x != 0 ) {
				if ( m_scroll.x() - m_move_start_scroll.x() < 0 ) {
					dist_x = Catch.x() + mod_x;
				} else {
					dist_x = mod_x;
				}
				new_x -= dist_x;
				dist_x = fabsf(dist_x) * 1e4;
				
				momentum_x.time = FX_MAX(FX_MIN(dist_x, 3e5), momentum_x.time);
			}
			
			if ( new_y < 0 && new_y > m_scroll_max.y() && mod_y != 0 ) {
				if (m_scroll.y() - m_move_start_scroll.y() < 0) {
					dist_y = Catch.y() + mod_y;
				} else {
					dist_y = mod_y;
				}
				new_y -= dist_y;
				dist_y = fabsf(dist_y) * 1e4;
				
				momentum_y.time = FX_MAX(FX_MIN(dist_y, 3e5), momentum_y.time);
			}
		}
		
		m_moved = false;
		
		//****************************************************************
		
		if ( momentum_x.time || momentum_y.time ) {
			uint64 duration = FX_MAX(FX_MAX(momentum_x.time, momentum_y.time), 1e4);
			scroll_to_valid_scroll(Vec2(new_x, new_y), duration);
		} else {
			termination_recovery(3e5, ease_in_out);
		}
	}
	
	/**
	 * @func m_touch_start_handle
	 */
	void m_touch_start_handle(GUIEvent& e) {
		if ( !m_action_id ) {
			GUITouchEvent* evt = static_cast<GUITouchEvent*>(&e);
			m_action_id = evt->changed_touches()[0].id;
			move_start(Vec2( evt->changed_touches()[0].x, evt->changed_touches()[0].y ));
		}
	}

	/**
	 * @func m_touch_move_handle
	 */
	void m_touch_move_handle(GUIEvent& e) {
		if ( m_action_id && e.return_value ) {
			GUITouchEvent* evt = static_cast<GUITouchEvent*>(&e);
			for ( auto& i : evt->changed_touches() ) {
				if (i.value().id == m_action_id) {
					move(Vec2( i.value().x, i.value().y )); break;
				}
			}
		}
	}

	/**
	 * @func m_touch_end_handle
	 */
	void m_touch_end_handle(GUIEvent& e) {
		if ( m_action_id ) {
			GUITouchEvent* evt = static_cast<GUITouchEvent*>(&e);
			for ( auto& i : evt->changed_touches() ) {
				if (i.value().id == m_action_id) {
					move_end(Vec2( i.value().x, i.value().y ));
					m_action_id = 0;
					break;
				}
			}
		}
	}
	
	void m_mouse_down_handle(GUIEvent& e) {
		if ( !m_action_id ) {
			GUIMouseEvent* evt = static_cast<GUIMouseEvent*>(&e);
			m_action_id = 1;
			move_start(Vec2( evt->x(), evt->y() ));
		}
	}

	void m_mouse_move_handle(GUIEvent& e) {
		if ( m_action_id && e.return_value ) {
			GUIMouseEvent* evt = static_cast<GUIMouseEvent*>(&e);
			move(Vec2( evt->x(), evt->y() ));
		}
	}

	void m_mouse_up_handle(GUIEvent& e) {
		if ( m_action_id ) {
			GUIMouseEvent* evt = static_cast<GUIMouseEvent*>(&e);
			move_end(Vec2( evt->x(), evt->y() ));
			m_action_id = 0;
		}
	}
	
};

BasicScroll::BasicScroll(Box* box)
: m_box(box)
, m_move_start_time(0)
, m_catch_position(1,1)
, m_action_id(0)
, m_scrollbar_color(140, 140, 140, 200)
, m_scrollbar_width(0)
, m_scrollbar_margin(0)
, m_scrollbar_opacity(0)
, m_resistance(1)
, m_default_scroll_duration(0)
, m_default_scroll_curve(const_cast<Curve*>(&ease_out))
, m_moved(false)
, m_h_scroll(false)
, m_v_scroll(false)
, m_h_scrollbar(false)
, m_v_scrollbar(false)
, m_bounce_lock(true)
, m_lock_h(false)
, m_lock_v(false)
, m_lock_direction(false)
, m_bounce(true)
, m_momentum(true)
, m_scrollbar(true)
{
	// bind touch event
	box->add_event_listener(GUI_EVENT_TOUCH_START, &Inl::m_touch_start_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_TOUCH_MOVE, &Inl::m_touch_move_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_TOUCH_END, &Inl::m_touch_end_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_TOUCH_CANCEL, &Inl::m_touch_end_handle, _inl(this));
	// bind mouse event
	box->add_event_listener(GUI_EVENT_MOUSE_DOWN, &Inl::m_mouse_down_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_MOUSE_MOVE, &Inl::m_mouse_move_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_MOUSE_UP, &Inl::m_mouse_up_handle, _inl(this));
	
	FX_DEBUG("Scroll: %d, Panel: %d", sizeof(Scroll), sizeof(Panel));
}

BasicScroll::~BasicScroll() {
	_inl(this)->termination_all_task();
	if ( m_default_scroll_curve != &ease_out ) {
		delete m_default_scroll_curve;
	}
}

/**
 * @func scroll_to
 */
void BasicScroll::scroll_to(Vec2 value, uint64 duration) {
	scroll_to(value, duration, *m_default_scroll_curve);
}

/**
 * @func scroll_to
 */
void BasicScroll::scroll_to(Vec2 value, uint64 duration, cCurve& curve) {
	m_raw_scroll = Vec2(-value.x(), -value.y());
	Vec2 scroll = _inl(this)->catch_valid_scroll( Vec2(-value.x(), -value.y()) );
	if ( scroll.x() != m_scroll.x() || scroll.y() != m_scroll.y() ) {
		_inl(this)->scroll_to_valid_scroll(scroll, duration, curve);
	}
	m_box->mark(View::M_SCROLL);
}

/**
 * @func scroll set
 */
void BasicScroll::set_scroll(Vec2 value) {
	if ( m_default_scroll_duration ) {
		scroll_to(value, m_default_scroll_duration, *m_default_scroll_curve);
	} else {
		m_raw_scroll = Vec2(-value.x(), -value.y());
		m_scroll = _inl(this)->catch_valid_scroll( Vec2(-value.x(), -value.y()) );
		m_box->mark(View::M_SCROLL);
	}
}

/**
 * @func set_scroll_x set
 */
void BasicScroll::set_scroll_x(float value) {
	m_raw_scroll.x(-value);
	m_scroll = _inl(this)->catch_valid_scroll( Vec2(-value, m_raw_scroll.y()) );
	m_box->mark(View::M_SCROLL);
}

/**
 * @func scroll_x set
 */
void BasicScroll::set_scroll_y(float value) {
	m_raw_scroll.y(-value);
	m_scroll = _inl(this)->catch_valid_scroll( Vec2(m_raw_scroll.x(), -value) );
	m_box->mark(View::M_SCROLL);
}

/**
 * @func resistance set
 */
void BasicScroll::set_resistance(float value) {
	m_resistance = FX_MAX(0.5, value);
}

/**
 * @func get_scrollbar_width
 */
float BasicScroll::scrollbar_width() const {
	if ( m_scrollbar_width < 1 ) {
		return 4 / display_port()->scale();
	} else {
		return m_scrollbar_width;
	}
}

/**
 * @func scrollbar_margin
 */
float BasicScroll::scrollbar_margin() const {
	if ( m_scrollbar_width < 1 ) {
		return 4 / display_port()->scale();
	} else {
		return m_scrollbar_width;
	}
}

/**
 * @func terminate
 */
void BasicScroll::terminate() {
	_inl(this)->termination_recovery(0);
}

/**
 * @func default_scroll_curve
 */
void BasicScroll::set_default_scroll_curve(cCurve& value) {
	if ( m_default_scroll_curve == &ease_out ) {
		m_default_scroll_curve = new Curve();
	}
	*m_default_scroll_curve = value;
}

/**
 * @func set_scroll_size
 */
void BasicScroll::set_scroll_size(Vec2 size) {
	if (m_scroll_size != size) {
		_inl(this)->immediate_end_all_task(); // change size immediate task
		m_scroll_size = size;
	}
	m_scroll_max = Vec2(FX_MIN(m_box->final_width() - size.width(), 0),
											FX_MIN(m_box->final_height() - size.height(), 0));
	
	m_h_scroll = m_scroll_max.x() < 0;
	m_v_scroll = ((!m_bounce_lock && !m_h_scroll) || m_scroll_max.y() < 0);
	
	m_h_scrollbar = (m_h_scroll && m_scrollbar);
	m_v_scrollbar = (m_v_scroll && m_scrollbar && m_scroll_max.y() < 0);
	
	//
	m_box->mark(View::M_SCROLL);
}

void BasicScroll::solve() {
	if ( m_box->mark_value & View::M_SCROLL ) {
		if ( !m_moved && !_inl(this)->is_task() ) {
			// fix scroll value
			m_scroll = _inl(this)->catch_valid_scroll(m_raw_scroll);
			m_raw_scroll = m_scroll;
		}
	}
}

// -----------------------------------------------------------------------------------------------

/**
 * @class Scroll::Inl
 */
FX_DEFINE_INLINE_MEMBERS(Scroll, Inl) {
public:
	/**
	 * @func handle_panel_focus_mode
	 */
	void handle_panel_focus_mode(GUIEvent& e) {
		
		GUIFocusMoveEvent* evt = static_cast<GUIFocusMoveEvent*>(&e);
		View* view = evt->focus_move();
		Box* box = view ? view->as_box() : nullptr;
		
		if ( box && (m_h_scroll || m_v_scroll) ) {
			Vec2 offset = box->layout_offset_from(this);
			
			CGRect rect = {
				{ offset.x() - box->border_left().width, offset.y() - box->border_top().width },
				{//
					box->client_width() - box->final_margin_left() - box->final_margin_right(),
					box->client_height() - box->final_margin_top() - box->final_margin_bottom()
				}
			};
			
			Vec2 v;
			
			if ( m_h_scroll ) {
				switch ( m_focus_align_x ) {
					case Align::LEFT:
						v.x( rect.origin.x() - m_focus_margin_left );
						break;
					case Align::RIGHT:
						v.x(rect.origin.x() - rect.size.x() - final_width() + m_focus_margin_right);
						break;
					case Align::CENTER:
						v.x( rect.origin.x() - (rect.size.x() + final_width()) / 2.0 );
						break;
					default: // none float
						float height = m_final_width -
						m_focus_margin_left -
						m_focus_margin_right;
						if ( rect.size.x() < height ) {
							float x = rect.origin.x() - m_focus_margin_left;
							if ( x < scroll_x() ) { //
								v.x( x );
							} else {
								x = rect.origin.x() + rect.size.x() - m_final_width + m_focus_margin_right;
								if ( x > scroll_x() ) {
									v.x(x);
								}
							}
						} else {
							v.x( rect.origin.x() - (rect.size.x() + m_final_width) / 2.0 ); // CENTER
						}
						break;
				}
			}
			
			if ( m_v_scroll ) {
				switch ( m_focus_align_y ) {
					case Align::TOP:
						v.y( rect.origin.y() - m_focus_margin_top );
						break;
					case Align::BOTTOM:
						v.y(rect.origin.y() - rect.size.y() -
								m_final_height + m_focus_margin_bottom);
						break;
					case Align::CENTER:
						v.y( rect.origin.y() - (rect.size.y() + m_final_height) / 2.0 );
						break;
					default: // none float
						float height = m_final_height -
						m_focus_margin_top -
						m_focus_margin_bottom;
						if ( rect.size.y() < height ) {
							float y = rect.origin.y() - m_focus_margin_top;
							if ( y < scroll_x() ) { //
								v.y( y );
							} else {
								y = rect.origin.y() + rect.size.y() -
								m_final_height + m_focus_margin_bottom;
								if ( y > scroll_y() ) {
									v.y(y);
								}
							}
						} else {
							v.y( rect.origin.y() - (rect.size.y() + m_final_height) / 2.0 ); // CENTER
						}
						break;
				}
			}
			
			set_scroll(v);
		}
	}
	
};

Scroll::Scroll()
: BasicScroll(this)
, m_focus_margin_left(0)
, m_focus_margin_right(0)
, m_focus_margin_top(0)
, m_focus_margin_bottom(0)
, m_focus_align_x(Align::NONE)
, m_focus_align_y(Align::NONE)
, m_enable_focus_align(true)
, m_enable_fixed_scroll_size(false)
{
	
}

Vec2 Scroll::layout_in_offset() {
	return Vec2( m_origin.x() + scroll_x(), m_origin.y() + scroll_y() );
}

/**
 * @func focus_align_x set
 */
void Scroll::set_focus_align_x(Align value) {
	if (value == Align::LEFT || value == Align::RIGHT ||
			value == Align::CENTER || value == Align::NONE ) {
		m_focus_align_x = value;
	}
}

/**
 * @func focus_align_y set
 */
void Scroll::set_focus_align_y(Align value) {
	if (value == Align::TOP || value == Align::BOTTOM ||
			value == Align::CENTER || value == Align::NONE ) {
		m_focus_align_y = value;
	}
}

/**
 * @func set_enable_focus_align set
 */
void Scroll::set_enable_focus_align(bool value) {
	if ( value != m_enable_focus_align ) {
		if ( value ) {
			add_event_listener(GUI_EVENT_FOCUS_MOVE, &Inl::handle_panel_focus_mode, Inl_Scroll(this));
		} else {
			remove_event_listener(GUI_EVENT_FOCUS_MOVE, &Inl::handle_panel_focus_mode, Inl_Scroll(this));
		}
		m_enable_focus_align = value;
	}
}

/**
 * @func enable_fixed_scroll_size
 */
void Scroll::set_enable_fixed_scroll_size(Vec2 size) {
	if ( size.width() > 0 && size.height() > 0 ) {
		m_enable_fixed_scroll_size = true;
		set_scroll_size(size);
	} else {
		if ( m_enable_fixed_scroll_size ) {
			m_enable_fixed_scroll_size = false;
			if ( m_explicit_width || m_explicit_height ) { // 明确的宽度与高度才可以滚动
				Vec2 squeeze;
				set_div_content_offset(squeeze, Vec2());
				set_scroll_size(squeeze);
			} else {
				set_scroll_size(Vec2());
			}
		}
	}
}

void Scroll::draw(Draw* draw) {
	if ( m_visible ) {
		
		if ( mark_value ) {
			BasicScroll::solve();
			Panel::solve();
		}
		
		draw->draw(this);
		
		mark_value = M_NONE;
	}
}

void Scroll::set_layout_content_offset() {
	if (m_final_visible) {
		
		Vec2 squeeze;
		
		if ( set_div_content_offset(squeeze, Vec2()) ) { //
			mark(M_SHAPE);
			
			Layout* layout = parent()->as_layout();
			if (layout) {
				layout->mark_pre(M_CONTENT_OFFSET);
			} else { // 父视图只是个普通视图,默认将偏移设置为0
				set_default_offset_value();
			}
		}
		
		// 明确的宽度与高度才可以滚动
		if ( !m_enable_fixed_scroll_size && (m_explicit_width || m_explicit_height) ) {
			set_scroll_size(squeeze);
		}
	}
}

FX_END
