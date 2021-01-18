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

namespace ftr {

static cCurve ease_in_out(0.3, 0.3, 0.3, 1);
static cCurve ease_out(0, 0, 0.58, 1);

/**
 * @class Scroll::Task
 */
class BasicScroll::Task: public PreRender::Task {
 public:
	
	Task(BasicScroll* host, uint64 duration, cCurve& curve = ease_out)
	: _host(host)
	, _start_time(sys::time_monotonic())
	, _duration(duration)
	, _immediate_end_flag(false)
	, _curve(curve)
	, _is_inl_ease_out(&ease_out == &curve)
	{
	
	}
	
	/**
	 * @func immediate_end_flag
	 */
	inline void immediate_end_flag() {
		_immediate_end_flag = true;
	}
	
	virtual void run(float y) = 0;
	virtual void end() = 0;
	virtual void immediate_end() = 0;
	
	/**
	 * @overwrite
	 */
	virtual bool run_task(int64 sys_time) {
		
		if ( _immediate_end_flag ) { // immediate end motion
			immediate_end();
		} else {
			int64 now = sys::time_monotonic();
			if ( now >= _start_time + _duration ) {
				end();
			} else {
				if ( _is_inl_ease_out ) {
					// ease_out
					float y = float(now - _start_time) / float(_duration);
					run( sqrtf(1 - powf(y - 1, 2)) );
				} else {
					run( _curve.solve(float(now - _start_time) / float(_duration), 0.001) );
				}
			}
		}
		return false;
	}
	
 protected:

	BasicScroll* _host;
	uint64 _start_time;
	uint64 _duration;
	List<Task*>::Iterator _id2;
	bool   _immediate_end_flag;
	cCurve _curve;
	bool _is_inl_ease_out;
	
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
		, _from(host->_scroll)
		, _to(to)
		{ }
		
		virtual void run(float y) {
			Vec2 scroll = Vec2((_to.x() - _from.x()) * y + _from.x(),
												 (_to.y() - _from.y()) * y + _from.y());
			_inl(_host)->set_scroll_and_trigger_event(scroll);
		}
		virtual void end() {
			_inl(_host)->set_scroll_and_trigger_event(_to);
			_inl(_host)->termination_recovery(3e5, ease_in_out);
		}
		virtual void immediate_end() {
			_inl(_host)->set_scroll_and_trigger_event(_to);
			_inl(_host)->termination_recovery(0, ease_in_out);
		}
		
	 private:
		Vec2  _from;
		Vec2  _to;
	};
	
	/**
	 * @class ScrollBarFadeInOutTask
	 */
	class ScrollBarFadeInOutTask: public Scroll::Task {
	 public:
		
		ScrollBarFadeInOutTask(BasicScroll* host, uint64 duration, float to, cCurve& curve = ease_out)
		: Task(host, duration, curve)
		, _from(host->_scrollbar_opacity)
		, _to(to)
		{ }
		
		virtual void run(float y) {
			_host->_scrollbar_opacity = (_to - _from) * y + _from;
			_host->_box->mark(View::M_SCROLL_BAR);
		}
		virtual void end() {
			_host->_scrollbar_opacity = _to;
			_host->_box->mark(View::M_SCROLL_BAR);
			_inl(_host)->termination_task(this);
		}
		virtual void immediate_end() {
			_host->_scrollbar_opacity = _to;
			_host->_box->mark(View::M_SCROLL_BAR);
			_inl(_host)->termination_task(this);
		}
		
	 private:
		float _from;
		float _to;
	};
	
	friend class BasicScroll::Task;
	friend class ScrollMotionTask;
	friend class ScrollBarFadeOutTask;
	
	/**
	 * @func register_task
	 */
	void register_task(Task* task) {
		if ( !task->is_register_task() ) {
			task->_id2 = _tasks.push(task);
			task->register_task();
			task->run_task(0);
		}
	}
	
	/**
	 * @func termination_task
	 */
	void termination_task(Task* task) {
		if ( task->is_register_task() ) {
			_tasks.del( task->_id2 );
			delete task;
		}
	}
	
	/**
	 * @func termination_task
	 */
	void termination_all_task() {
		for ( auto& i : _tasks ) {
			delete i.value();
		}
		_tasks.clear();
	}
	
	/**
	 * @func immediate_end_all_task
	 */
	void immediate_end_all_task() {
		for ( auto& i : _tasks ) {
			i.value()->immediate_end_flag();
		}
	}
	
	/**
	 * @func is_task
	 */
	inline bool is_task() {
		return _tasks.length();
	}
	
	/**
	 * @func momentum
	 */
	Momentum momentum(uint64 time, float dist,
										float max_dist_upper, float max_dist_lower, float size) {
		
		float deceleration = 0.001 * _resistance;
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
		float x = (_catch_position.x() < 1 ||
							 _catch_position.x() > _box->final_width()) ?
							 _box->final_width() : _catch_position.x();
		float y = (_catch_position.y() < 1 ||
							 _catch_position.y() > _box->final_width()) ?
							 _box->final_height() : _catch_position.y();
		if (x == 0.0) x = 1.0;
		if (y == 0.0) y = 1.0;
		return Vec2(x, y);
	}
	
	/**
	 * @func get_valid_scroll
	 */
	Vec2 get_valid_scroll(float x, float y) {
		x = x >= 0 ? 0 : x < _scroll_max.x() ? _scroll_max.x() : x;
		y = y >= 0 ? 0 : y < _scroll_max.y() ? _scroll_max.y() : y;
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
			if ( valid.x() < _scroll_max.x() ) {
				valid.x( valid.x() + Catch.x() );
			}
			valid.y( roundf(valid.y() / Catch.y()) * Catch.y() );
			if ( valid.y() < _scroll_max.y() ) {
				valid.y( valid.y() + Catch.y() );
			}
		}
		return optimal_display(valid);
	}
	
	/**
	 * @func set_h_scrollbar_pos
	 */
	void set_h_scrollbar_pos() {
		
		if ( ! _h_scrollbar ) {
			return;
		}
		
		float left_margin = scrollbar_margin();
		float right_margin = _v_scrollbar ? left_margin + scrollbar_width() : left_margin;
		float h_scrollbar_max_size = FX_MAX(_box->final_width() - left_margin - right_margin, 0);
		
		float h_scrollbar_indicator_size = roundf(powf(h_scrollbar_max_size, 2) / _scroll_size.x());
		h_scrollbar_indicator_size = FX_MAX(h_scrollbar_indicator_size, 8);
		float h_scrollbar_max_scroll = h_scrollbar_max_size - h_scrollbar_indicator_size;
		float h_scrollbar_prop = h_scrollbar_max_scroll / _scroll_max.x();
		
		// ------------------------------------------------------
		
		float pos = h_scrollbar_prop * _scroll.x();
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
		
		_h_scrollbar_position = Vec2(pos + left_margin, size);
	}
	
	/**
	 * @func set_v_scrollbar_pos
	 */
	void set_v_scrollbar_pos() {
		
		if ( ! _v_scrollbar ) {
			return;
		}
		
		float top_margin = scrollbar_margin();
		float bottom_margin = _h_scrollbar ? top_margin + scrollbar_width() : top_margin;
		float v_scrollbar_max_size = FX_MAX(_box->final_height() - top_margin - bottom_margin, 0);
		
		float v_scrollbar_indicator_size = roundf(powf(v_scrollbar_max_size, 2) / _scroll_size.y());
		v_scrollbar_indicator_size = FX_MAX(v_scrollbar_indicator_size, 8);
		
		float v_scrollbar_max_scroll = v_scrollbar_max_size - v_scrollbar_indicator_size;
		float v_scrollbar_prop = v_scrollbar_max_scroll / _scroll_max.y();
		
		// ------------------------------------------------------
		
		float pos = v_scrollbar_prop * _scroll.y();
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
		
		_v_scrollbar_position = Vec2(pos + top_margin, size);
	}
	
	/**
	 * @func set_scroll_and_trigger_event
	 */
	void set_scroll_and_trigger_event(Vec2 scroll) {
		
		scroll = optimal_display(scroll);
		scroll.x( _h_scroll ? scroll.x() : 0 );
		scroll.y( _v_scroll ? scroll.y() : 0 );
		
		if ( _scroll.x() != scroll.x() || _scroll.y() != scroll.y() ) {
			
			_scroll = scroll;
			_raw_scroll = scroll;
			
			set_h_scrollbar_pos();
			set_v_scrollbar_pos();
			
			_box->mark(View::M_SCROLL); // mark
			
			main_loop()->post(Cb([this](CbD& se) {
				Handle<GUIEvent> evt = New<GUIEvent>(_box);
				_box->trigger(GUI_EVENT_SCROLL, **evt); // trigger event
			}, _box));
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
		
		Vec2 scroll = catch_valid_scroll(_scroll);
		if ( scroll.x() == _scroll.x() && scroll.y() == _scroll.y() ) {
			if ( duration ) {
				if ( _scrollbar_opacity != 0 ) {
					register_task( new ScrollBarFadeInOutTask(this, 2e5, 0) );
				}
			} else {
				if ( _scrollbar_opacity != 0 ) {
					_scrollbar_opacity = 0;
					_box->mark(View::M_SCROLL_BAR);
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
		
		if ( !is_task() && ! _moved ) {
			
			if ( scroll.x() != _scroll.x() || scroll.y() != _scroll.y() ) {
				register_task( new ScrollMotionTask(this, duration, scroll, curve) );
				if ( _scrollbar_opacity != 1 ) {
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
		
		_moved = false;
		_move_dist = Vec2();
		_move_point = point;
		_move_start_time = sys::time_monotonic();
		_move_start_scroll = _scroll;
	}
	
	/**
	 * @func move
	 */
	void move(Vec2 point) {
		
		float delta_x = point.x() - _move_point.x();
		float delta_y = point.y() - _move_point.y();
		float new_x = _scroll.x() + delta_x;
		float new_y = _scroll.y() + delta_y;
		
		_move_point = point;
		
		// Slow down if outside of the boundaries
		if ( new_x > 0 || new_x < _scroll_max.x() ) {
			if ( _bounce ) {
				new_x = _scroll.x() + (delta_x / 2);
			} else {
				new_x = (new_x >= 0 || _scroll_max.x() >= 0 ? 0 : _scroll_max.x());
			}
		}
		if ( new_y > 0 || new_y < _scroll_max.y() ) {
			if ( _bounce ) {
				new_y = _scroll.y() + delta_y / 2;
			} else {
				new_y = (new_y >= 0 || _scroll_max.y() >= 0 ? 0 : _scroll_max.y());
			}
		}
		
		_move_dist.x( _move_dist.x() + delta_x );
		_move_dist.y( _move_dist.y() + delta_y );
		
		float dist_x = fabsf(_move_dist.x());
		float dist_y = fabsf(_move_dist.y());
		
		if ( !_moved ) {
			if ( dist_x < 3 && dist_y < 3 ) { // 距离小余3不处理
				return;
			}
			if ( _scrollbar_opacity != 1 ) {
				register_task( new ScrollBarFadeInOutTask(this, 2e5, 1) );
			}
			_moved = true;
		}
		
		// Lock direction
		if ( _lock_direction ) {
			
			if ( _lock_v ) {
				new_y = _scroll.y();
				delta_y = 0;
			} else if( _lock_h ) {
				new_x = _scroll.x();
				delta_x = 0;
			}
			else {
				if ( dist_x > dist_y + 2 ) {
					_lock_v = true;
				} else if ( dist_y > dist_x + 2 ) {
					_lock_h = true;
				}
			}
		}
		
		uint64 time = sys::time_monotonic();
		
		if (int64(time) - _move_start_time > 3e5) {
			_move_start_time = time;
			_move_start_scroll = _scroll;
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
		
		uint64 duration = int64(time) - _move_start_time;
		float new_x = _scroll.x();
		float new_y = _scroll.y();
		
		_lock_h = false;
		_lock_v = false;
		
		//计算惯性
		if ( duration < 3e5 ) {
			
			if ( _momentum ) {
				if ( new_x ) {
					momentum_x = momentum(duration, new_x - _move_start_scroll.x(),
																-_scroll.x(), _scroll.x() - _scroll_max.x(),
																_bounce ? _box->final_width() / 2.0 : 0);
				}
				if ( new_y ) {
					momentum_y = momentum(duration, new_y - _move_start_scroll.y(),
																-_scroll.y(), _scroll.y() - _scroll_max.y(),
																_bounce ? _box->final_height() / 2.0 : 0);
				}
				new_x = _scroll.x() + momentum_x.dist;
				new_y = _scroll.y() + momentum_y.dist;
				
				if ((_scroll.x() > 0 && new_x > 0) ||
						(_scroll.x() < _scroll_max.x() && new_x < _scroll_max.x())) {
					momentum_x = { 0, 0 };
				}
				if ((_scroll.y() > 0 && new_y > 0) ||
						(_scroll.y() < _scroll_max.y() && new_y < _scroll_max.y())) {
					momentum_y = { 0, 0 };
				}
			}
			
			//捕获位置
			Vec2 Catch = get_catch_value();
			
			float mod_x = int(roundf(new_x)) % uint(Catch.x());
			float mod_y = int(roundf(new_y)) % uint(Catch.y());
			float dist_x, dist_y;
			
			if ( new_x < 0 && new_x > _scroll_max.x() && mod_x != 0 ) {
				if ( _scroll.x() - _move_start_scroll.x() < 0 ) {
					dist_x = Catch.x() + mod_x;
				} else {
					dist_x = mod_x;
				}
				new_x -= dist_x;
				dist_x = fabsf(dist_x) * 1e4;
				
				momentum_x.time = FX_MAX(FX_MIN(dist_x, 3e5), momentum_x.time);
			}
			
			if ( new_y < 0 && new_y > _scroll_max.y() && mod_y != 0 ) {
				if (_scroll.y() - _move_start_scroll.y() < 0) {
					dist_y = Catch.y() + mod_y;
				} else {
					dist_y = mod_y;
				}
				new_y -= dist_y;
				dist_y = fabsf(dist_y) * 1e4;
				
				momentum_y.time = FX_MAX(FX_MIN(dist_y, 3e5), momentum_y.time);
			}
		}
		
		_moved = false;
		
		//****************************************************************
		
		if ( momentum_x.time || momentum_y.time ) {
			uint64 duration = FX_MAX(FX_MAX(momentum_x.time, momentum_y.time), 1e4);
			scroll_to_valid_scroll(Vec2(new_x, new_y), duration);
		} else {
			termination_recovery(3e5, ease_in_out);
		}
	}
	
	/**
	 * @func _touch_start_handle
	 */
	void _touch_start_handle(GUIEvent& e) {
		if ( !_action_id ) {
			GUITouchEvent* evt = static_cast<GUITouchEvent*>(&e);
			_action_id = evt->changed_touches()[0].id;
			move_start(Vec2( evt->changed_touches()[0].x, evt->changed_touches()[0].y ));
		}
	}

	/**
	 * @func _touch_move_handle
	 */
	void _touch_move_handle(GUIEvent& e) {
		if ( _action_id && e.return_value ) {
			GUITouchEvent* evt = static_cast<GUITouchEvent*>(&e);
			for ( auto& i : evt->changed_touches() ) {
				if (i.value().id == _action_id) {
					move(Vec2( i.value().x, i.value().y )); break;
				}
			}
		}
	}

	/**
	 * @func _touch_end_handle
	 */
	void _touch_end_handle(GUIEvent& e) {
		if ( _action_id ) {
			GUITouchEvent* evt = static_cast<GUITouchEvent*>(&e);
			for ( auto& i : evt->changed_touches() ) {
				if (i.value().id == _action_id) {
					move_end(Vec2( i.value().x, i.value().y ));
					_action_id = 0;
					break;
				}
			}
		}
	}
	
	void _mouse_down_handle(GUIEvent& e) {
		if ( !_action_id ) {
			GUIMouseEvent* evt = static_cast<GUIMouseEvent*>(&e);
			_action_id = 1;
			move_start(Vec2( evt->x(), evt->y() ));
		}
	}

	void _mouse_move_handle(GUIEvent& e) {
		if ( _action_id && e.return_value ) {
			GUIMouseEvent* evt = static_cast<GUIMouseEvent*>(&e);
			move(Vec2( evt->x(), evt->y() ));
		}
	}

	void _mouse_up_handle(GUIEvent& e) {
		if ( _action_id ) {
			GUIMouseEvent* evt = static_cast<GUIMouseEvent*>(&e);
			move_end(Vec2( evt->x(), evt->y() ));
			_action_id = 0;
		}
	}
	
};

BasicScroll::BasicScroll(Box* box)
: _box(box)
, _move_start_time(0)
, _catch_position(1,1)
, _action_id(0)
, _scrollbar_color(140, 140, 140, 200)
, _scrollbar_width(0)
, _scrollbar_margin(0)
, _scrollbar_opacity(0)
, _resistance(1)
, _default_scroll_duration(0)
, _default_scroll_curve(const_cast<Curve*>(&ease_out))
, _moved(false)
, _h_scroll(false)
, _v_scroll(false)
, _h_scrollbar(false)
, _v_scrollbar(false)
, _bounce_lock(true)
, _lock_h(false)
, _lock_v(false)
, _lock_direction(false)
, _bounce(true)
, _momentum(true)
, _scrollbar(true)
{
	// bind touch event
	box->add_event_listener(GUI_EVENT_TOUCH_START, &Inl::_touch_start_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_TOUCH_MOVE, &Inl::_touch_move_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_TOUCH_END, &Inl::_touch_end_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_TOUCH_CANCEL, &Inl::_touch_end_handle, _inl(this));
	// bind mouse event
	box->add_event_listener(GUI_EVENT_MOUSE_DOWN, &Inl::_mouse_down_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_MOUSE_MOVE, &Inl::_mouse_move_handle, _inl(this));
	box->add_event_listener(GUI_EVENT_MOUSE_UP, &Inl::_mouse_up_handle, _inl(this));
	
	FX_DEBUG("Scroll: %d, Panel: %d", sizeof(Scroll), sizeof(Panel));
}

BasicScroll::~BasicScroll() {
	_inl(this)->termination_all_task();
	if ( _default_scroll_curve != &ease_out ) {
		delete _default_scroll_curve;
	}
}

/**
 * @func scroll_to
 */
void BasicScroll::scroll_to(Vec2 value, uint64 duration) {
	scroll_to(value, duration, *_default_scroll_curve);
}

/**
 * @func scroll_to
 */
void BasicScroll::scroll_to(Vec2 value, uint64 duration, cCurve& curve) {
	_raw_scroll = Vec2(-value.x(), -value.y());
	Vec2 scroll = _inl(this)->catch_valid_scroll( Vec2(-value.x(), -value.y()) );
	if ( scroll.x() != _scroll.x() || scroll.y() != _scroll.y() ) {
		_inl(this)->scroll_to_valid_scroll(scroll, duration, curve);
	}
	_box->mark(View::M_SCROLL);
}

/**
 * @func scroll set
 */
void BasicScroll::set_scroll(Vec2 value) {
	if ( _default_scroll_duration ) {
		scroll_to(value, _default_scroll_duration, *_default_scroll_curve);
	} else {
		_raw_scroll = Vec2(-value.x(), -value.y());
		_scroll = _inl(this)->catch_valid_scroll( Vec2(-value.x(), -value.y()) );
		_box->mark(View::M_SCROLL);
	}
}

/**
 * @func set_scroll_x set
 */
void BasicScroll::set_scroll_x(float value) {
	_raw_scroll.x(-value);
	_scroll = _inl(this)->catch_valid_scroll( Vec2(-value, _raw_scroll.y()) );
	_box->mark(View::M_SCROLL);
}

/**
 * @func scroll_x set
 */
void BasicScroll::set_scroll_y(float value) {
	_raw_scroll.y(-value);
	_scroll = _inl(this)->catch_valid_scroll( Vec2(_raw_scroll.x(), -value) );
	_box->mark(View::M_SCROLL);
}

/**
 * @func resistance set
 */
void BasicScroll::set_resistance(float value) {
	_resistance = FX_MAX(0.5, value);
}

/**
 * @func get_scrollbar_width
 */
float BasicScroll::scrollbar_width() const {
	if ( _scrollbar_width < 1 ) {
		return 4 / display_port()->scale();
	} else {
		return _scrollbar_width;
	}
}

/**
 * @func scrollbar_margin
 */
float BasicScroll::scrollbar_margin() const {
	if ( _scrollbar_width < 1 ) {
		return 4 / display_port()->scale();
	} else {
		return _scrollbar_width;
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
	if ( _default_scroll_curve == &ease_out ) {
		_default_scroll_curve = new Curve();
	}
	*_default_scroll_curve = value;
}

/**
 * @func set_scroll_size
 */
void BasicScroll::set_scroll_size(Vec2 size) {
	if (_scroll_size != size) {
		_inl(this)->immediate_end_all_task(); // change size immediate task
		_scroll_size = size;
	}
	_scroll_max = Vec2(FX_MIN(_box->final_width() - size.width(), 0),
											FX_MIN(_box->final_height() - size.height(), 0));
	
	_h_scroll = _scroll_max.x() < 0;
	_v_scroll = ((!_bounce_lock && !_h_scroll) || _scroll_max.y() < 0);
	
	_h_scrollbar = (_h_scroll && _scrollbar);
	_v_scrollbar = (_v_scroll && _scrollbar && _scroll_max.y() < 0);
	
	//
	_box->mark(View::M_SCROLL);
}

void BasicScroll::solve() {
	if ( _box->mark_value & View::M_SCROLL ) {
		if ( !_moved && !_inl(this)->is_task() ) {
			// fix scroll value
			_scroll = _inl(this)->catch_valid_scroll(_raw_scroll);
			_raw_scroll = _scroll;
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
		
		if ( box && (_h_scroll || _v_scroll) ) {
			Vec2 offset = box->layout_offset_from(this);
			
			CGRect rect = {
				{ offset.x() - box->border_left().width, offset.y() - box->border_top().width },
				{//
					box->client_width() - box->final_margin_left() - box->final_margin_right(),
					box->client_height() - box->final_margin_top() - box->final_margin_bottom()
				}
			};
			
			Vec2 v;
			
			if ( _h_scroll ) {
				switch ( _focus_align_x ) {
					case Align::LEFT:
						v.x( rect.origin.x() - _focus_margin_left );
						break;
					case Align::RIGHT:
						v.x(rect.origin.x() - rect.size.x() - final_width() + _focus_margin_right);
						break;
					case Align::CENTER:
						v.x( rect.origin.x() - (rect.size.x() + final_width()) / 2.0 );
						break;
					default: // none float
						float height = _final_width -
						_focus_margin_left -
						_focus_margin_right;
						if ( rect.size.x() < height ) {
							float x = rect.origin.x() - _focus_margin_left;
							if ( x < scroll_x() ) { //
								v.x( x );
							} else {
								x = rect.origin.x() + rect.size.x() - _final_width + _focus_margin_right;
								if ( x > scroll_x() ) {
									v.x(x);
								}
							}
						} else {
							v.x( rect.origin.x() - (rect.size.x() + _final_width) / 2.0 ); // CENTER
						}
						break;
				}
			}
			
			if ( _v_scroll ) {
				switch ( _focus_align_y ) {
					case Align::TOP:
						v.y( rect.origin.y() - _focus_margin_top );
						break;
					case Align::BOTTOM:
						v.y(rect.origin.y() - rect.size.y() -
								_final_height + _focus_margin_bottom);
						break;
					case Align::CENTER:
						v.y( rect.origin.y() - (rect.size.y() + _final_height) / 2.0 );
						break;
					default: // none float
						float height = _final_height -
						_focus_margin_top -
						_focus_margin_bottom;
						if ( rect.size.y() < height ) {
							float y = rect.origin.y() - _focus_margin_top;
							if ( y < scroll_x() ) { //
								v.y( y );
							} else {
								y = rect.origin.y() + rect.size.y() -
								_final_height + _focus_margin_bottom;
								if ( y > scroll_y() ) {
									v.y(y);
								}
							}
						} else {
							v.y( rect.origin.y() - (rect.size.y() + _final_height) / 2.0 ); // CENTER
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
, _focus_margin_left(0)
, _focus_margin_right(0)
, _focus_margin_top(0)
, _focus_margin_bottom(0)
, _focus_align_x(Align::NONE)
, _focus_align_y(Align::NONE)
, _enable_focus_align(true)
, _enable_fixed_scroll_size(false)
{
	
}

Vec2 Scroll::layout_in_offset() {
	return Vec2( _origin.x() + scroll_x(), _origin.y() + scroll_y() );
}

/**
 * @func focus_align_x set
 */
void Scroll::set_focus_align_x(Align value) {
	if (value == Align::LEFT || value == Align::RIGHT ||
			value == Align::CENTER || value == Align::NONE ) {
		_focus_align_x = value;
	}
}

/**
 * @func focus_align_y set
 */
void Scroll::set_focus_align_y(Align value) {
	if (value == Align::TOP || value == Align::BOTTOM ||
			value == Align::CENTER || value == Align::NONE ) {
		_focus_align_y = value;
	}
}

/**
 * @func set_enable_focus_align set
 */
void Scroll::set_enable_focus_align(bool value) {
	if ( value != _enable_focus_align ) {
		if ( value ) {
			add_event_listener(GUI_EVENT_FOCUS_MOVE, &Inl::handle_panel_focus_mode, Inl_Scroll(this));
		} else {
			remove_event_listener(GUI_EVENT_FOCUS_MOVE, &Inl::handle_panel_focus_mode, Inl_Scroll(this));
		}
		_enable_focus_align = value;
	}
}

/**
 * @func enable_fixed_scroll_size
 */
void Scroll::set_enable_fixed_scroll_size(Vec2 size) {
	if ( size.width() > 0 && size.height() > 0 ) {
		_enable_fixed_scroll_size = true;
		set_scroll_size(size);
	} else {
		if ( _enable_fixed_scroll_size ) {
			_enable_fixed_scroll_size = false;
			if ( _explicit_width || _explicit_height ) { // 明确的宽度与高度才可以滚动
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
	if ( _visible ) {
		
		if ( mark_value ) {
			BasicScroll::solve();
			Panel::solve();
		}
		
		draw->draw(this);
		
		mark_value = M_NONE;
	}
}

void Scroll::set_layout_content_offset() {
	if (_final_visible) {
		
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
		if ( !_enable_fixed_scroll_size && (_explicit_width || _explicit_height) ) {
			set_scroll_size(squeeze);
		}
	}
}

}
