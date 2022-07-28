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

#include "./scroll.h"
#include "../pre_render.h"
#include "../display.h"
#include "../app.h"
#include "../util/numbers.h"
#include <math.h>

namespace noug {

	// ------------------------ B a s e . S c r o l l --------------------------

	static const Curve ease_in_out(0.3, 0.3, 0.3, 1);
	static const Curve ease_out(0, 0, 0.58, 1);

	class BaseScroll::ScrollBox: public Box {
	public:

		void triggerScroll() {
			Sp<UIEvent> evt = New<UIEvent>(this);
			Notification<UIEvent, UIEventName, Layout>::trigger(UIEvent_Scroll, **evt);
		}

		void mark_none(uint32_t mark = kLayout_None) {
			Layout::mark_none(mark);
		}

		void unmark(uint32_t mark = (~View::kLayout_None)) {
			Layout::unmark(mark);
		}

		inline bool layout_wrap_x() const {
			return Box::layout_wrap_x();
		}

		inline bool layout_wrap_y() const {
			return Box::layout_wrap_y();
		}
	};

	class BaseScroll::Task: public PreRender::Task {
	public:

		Task(BaseScroll* host, uint64_t duration, cCurve& curve = ease_out)
			: m_host(host)
			, m_start_time(time_monotonic())
			, m_duration(duration)
			, m_immediate_end_flag(false)
			, m_curve(curve)
			, m_is_inl_ease_out(&ease_out == &curve)
		{}

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
		virtual bool run_task(int64_t sys_time) {
			
			if ( m_immediate_end_flag ) { // immediate end motion
				immediate_end();
			} else {
				int64_t now = time_monotonic();
				if ( now >= m_start_time + m_duration ) {
					end();
				} else {
					if ( m_is_inl_ease_out ) {
						// ease_out
						float y = float(now - m_start_time) / float(m_duration);
						run( sqrtf(1 - powf(y - 1, 2)) );
					} else {
						run( m_curve.solve_t(float(now - m_start_time) / float(m_duration), 0.001) );
					}
				}
			}
			return false;
		}

	protected:
		BaseScroll* m_host;
		uint64_t m_start_time;
		uint64_t m_duration;
		List<Task*>::Iterator m_id2;
		bool   m_immediate_end_flag;
		cCurve m_curve;
		bool m_is_inl_ease_out;

		friend class BaseScroll::Inl;
	};

	N_DEFINE_INLINE_MEMBERS(BaseScroll, Inl) {
	public:
		#define _inl(self) static_cast<BaseScroll::Inl*>(static_cast<BaseScroll*>(self))

		struct Momentum {
			float dist;
			uint64_t time;
		};

		class ScrollMotionTask: public BaseScroll::Task {
		public:
			
			ScrollMotionTask(BaseScroll* host, uint64_t duration, Vec2 to, cCurve& curve = ease_out)
				: Task(host, duration, curve)
				, m_from(host->_scroll)
				, m_to(to)
			{}
			
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

		class ScrollBarFadeInOutTask: public BaseScroll::Task {
		public:
			
			ScrollBarFadeInOutTask(BaseScroll* host, uint64_t duration, float to, cCurve& curve = ease_out)
				: Task(host, duration, curve)
				, m_from(host->_scrollbar_opacity)
				, m_to(to)
			{}

			virtual void run(float y) {
				m_host->_scrollbar_opacity = (m_to - m_from) * y + m_from;
				m_host->_host->mark_none();
			}

			virtual void end() {
				m_host->_scrollbar_opacity = m_to;
				m_host->_host->mark_none();
				_inl(m_host)->termination_task(this);
			}

			virtual void immediate_end() {
				m_host->_scrollbar_opacity = m_to;
				m_host->_host->mark_none();
				_inl(m_host)->termination_task(this);
			}

		private:
			float m_from;
			float m_to;
		};

		friend class BaseScroll::Task;
		friend class ScrollMotionTask;
		friend class ScrollBarFadeOutTask;

		void register_task(Task* task) {
			if ( !task->is_register_task() ) {
				task->m_id2 = _tasks.push_back(task);
				task->register_task();
				task->run_task(0);
			}
		}
		
		void termination_task(Task* task) {
			if ( task->is_register_task() ) {
				_tasks.erase( task->m_id2 );
				delete task;
			}
		}

		void termination_all_task() {
			for ( auto &i : _tasks ) {
				delete i;
			}
			_tasks.clear();
		}

		void immediate_end_all_task() {
			for ( auto &i : _tasks ) {
				i->immediate_end_flag();
			}
		}

		inline bool is_task() {
			return _tasks.length();
		}

		Momentum momentum(uint64_t time, float dist, float max_dist_upper, float max_dist_lower, float size) {
			
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
			uint64_t new_time = speed / deceleration * 1000;
			
			return { new_dist, new_time };
		}

		Vec2 get_catch_value() {
			auto size = _host->content_size();
			float x = (_catch_position_x < 1 || _catch_position_x > size.x()) ? size.x() : _catch_position_x;
			float y = (_catch_position_y < 1 || _catch_position_y > size.y()) ? size.y() : _catch_position_y;
			if (x == 0.0)
				x = 1.0;
			if (y == 0.0)
				y = 1.0;
			return Vec2(x, y);
		}

		Vec2 get_valid_scroll(float x, float y) {
			x = x >= 0 ? 0 : x < _scroll_max.x() ? _scroll_max.x() : x;
			y = y >= 0 ? 0 : y < _scroll_max.y() ? _scroll_max.y() : y;
			return Vec2(x, y);
		}

		Vec2 optimal_display(Vec2 value) {
			auto scale = _host->pre_render()->host()->display()->scale();
			value.set_x( round(value.x() * scale) / scale );
			value.set_y( round(value.y() * scale) / scale );
			return value;
		}

		Vec2 catch_valid_scroll(Vec2 scroll) {
			Vec2 valid = get_valid_scroll(scroll.x(), scroll.y());
			Vec2 Catch = get_catch_value();
			
			if ( Catch.x() != 1 && Catch.y() != 1 ) { // 捕获位置
				valid.set_x( roundf(valid.x() / Catch.x()) * Catch.x() );
				if ( valid.x() < _scroll_max.x() ) {
					valid.set_x( valid.x() + Catch.x() );
				}
				valid.set_y( roundf(valid.y() / Catch.y()) * Catch.y() );
				if ( valid.y() < _scroll_max.y() ) {
					valid.set_y( valid.y() + Catch.y() );
				}
			}
			return optimal_display(valid);
		}

		void set_h_scrollbar_pos() {
			if ( ! _scrollbar_h ) {
				return;
			}
			
			float size_x = _host->content_size().x() + _host->padding_left() + _host->padding_right();
			float left_margin = scrollbar_margin();
			float right_margin = _scrollbar_v ? left_margin + scrollbar_width() : left_margin;
			float h_scrollbar_max_size = Float::max(size_x - left_margin - right_margin, 0);

			float h_scrollbar_indicator_size = roundf(powf(h_scrollbar_max_size, 2) / _scroll_size.x());
			h_scrollbar_indicator_size = Float::max(h_scrollbar_indicator_size, 8);
			float h_scrollbar_max_scroll = h_scrollbar_max_size - h_scrollbar_indicator_size;
			float h_scrollbar_prop = h_scrollbar_max_scroll / _scroll_max.x();
			
			// ------------------------------------------------------
			
			float pos = h_scrollbar_prop * _scroll.x();
			float size = h_scrollbar_indicator_size;
			
			if ( pos < 0 ) {
				size = h_scrollbar_indicator_size + roundf(pos * 3);
				size = N_MAX(size, 8);
				pos = 0;
			} else if ( pos > h_scrollbar_max_scroll ) {
				size = h_scrollbar_indicator_size - roundf((pos - h_scrollbar_max_scroll) * 3);
				size = N_MAX(size, 8);
				pos = h_scrollbar_max_scroll + h_scrollbar_indicator_size - size;
			}
			
			_scrollbar_position_h = Vec2(pos + left_margin, size);
		}

		void set_v_scrollbar_pos() {
			if ( ! _scrollbar_v ) {
				return;
			}

			float size_y = _host->content_size().y() + _host->padding_top() + _host->padding_bottom();
			float top_margin = scrollbar_margin();
			float bottom_margin = _scrollbar_h ? top_margin + scrollbar_width() : top_margin;
			float v_scrollbar_max_size = Float::max(size_y - top_margin - bottom_margin, 0);
			
			float v_scrollbar_indicator_size = roundf(powf(v_scrollbar_max_size, 2) / _scroll_size.y());
			v_scrollbar_indicator_size = Float::max(v_scrollbar_indicator_size, 8);
			
			float v_scrollbar_max_scroll = v_scrollbar_max_size - v_scrollbar_indicator_size;
			float v_scrollbar_prop = v_scrollbar_max_scroll / _scroll_max.y();
			
			// ------------------------------------------------------
			
			float pos = v_scrollbar_prop * _scroll.y();
			float size = v_scrollbar_indicator_size;
			
			if ( pos < 0 ) {
				size = v_scrollbar_indicator_size + roundf(pos * 3);
				size = Float::max(size, 8);
				pos = 0;
			} else if ( pos > v_scrollbar_max_scroll ) {
				size = v_scrollbar_indicator_size - roundf((pos - v_scrollbar_max_scroll) * 3);
				size = Float::max(size, 8);
				pos = v_scrollbar_max_scroll + v_scrollbar_indicator_size - size;
			}
			
			_scrollbar_position_v = Vec2(pos + top_margin, size);
		}

		void set_scroll_and_trigger_event(Vec2 scroll) {
			
			scroll = optimal_display(scroll);
			scroll.set_x( _scroll_h ? scroll.x() : 0 );
			scroll.set_y( _scroll_v ? scroll.y() : 0 );
			
			if ( _scroll.x() != scroll.x() || _scroll.y() != scroll.y() ) {
				
				_scroll = scroll;
				_scroll_raw = scroll;
				
				set_h_scrollbar_pos();
				set_v_scrollbar_pos();
				
				_host->mark_none(View::kScroll); // mark
				
				_host->pre_render()->host()->loop()->post(Cb([this](CbData& se) {
					_host->triggerScroll(); // trigger event
				}, _host));
			}
		}

		void scroll_to_valid_scroll(Vec2 valid_scroll, uint64_t duration, cCurve& curve = ease_out) {
			termination_all_task();
			if ( duration ) {
				motion_start(valid_scroll, duration, curve);
			} else {
				set_scroll_and_trigger_event(valid_scroll);
			}
		}

		void termination_recovery(uint64_t duration, cCurve& curve = ease_out) {
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
						_host->mark_none();
					}
				}
			} else {
				scroll_to_valid_scroll(scroll, duration, curve);
			}
		}

		void motion_start(Vec2 scroll, uint64_t duration, cCurve& curve) {
			
			if ( !is_task() && ! _moved ) {
				
				if ( scroll.x() != _scroll.x() || scroll.y() != _scroll.y() ) {
					register_task( new ScrollMotionTask(this, duration, scroll, curve) );
					if ( _scrollbar_opacity != 1 ) {
						register_task( new ScrollBarFadeInOutTask(this, 5e4, 1) );
					}
				}
			}
		}

		void move_start(Vec2 point) {
			termination_all_task();
			_moved = false;
			_move_dist = Vec2();
			_move_point = point;
			_move_start_time = time_monotonic();
			_move_start_scroll = _scroll;
		}

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
			
			_move_dist.set_x( _move_dist.x() + delta_x );
			_move_dist.set_y( _move_dist.y() + delta_y );
			
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

			uint64_t time = time_monotonic();

			if (int64_t(time) - _move_start_time > 3e5) {
				_move_start_time = time;
				_move_start_scroll = _scroll;
			}
			
			set_scroll_and_trigger_event(Vec2(new_x, new_y));
		}

		void move_end(Vec2 point) {
			uint64_t time = time_monotonic();
			
			Momentum momentum_x = { 0,0 };
			Momentum momentum_y = { 0,0 };
			
			uint64_t duration = int64_t(time) - _move_start_time;
			float new_x = _scroll.x();
			float new_y = _scroll.y();
			
			_lock_h = false;
			_lock_v = false;
			
			//计算惯性
			if ( duration < 3e5 ) {
				
				if ( _momentum ) {
					auto size = _host->content_size();
					if ( new_x ) {
						momentum_x = momentum(duration, new_x - _move_start_scroll.x(),
																	-_scroll.x(), _scroll.x() - _scroll_max.x(),
																	_bounce ? size.x() / 2.0 : 0);
					}
					if ( new_y ) {
						momentum_y = momentum(duration, new_y - _move_start_scroll.y(),
																	-_scroll.y(), _scroll.y() - _scroll_max.y(),
																	_bounce ? size.y() / 2.0 : 0);
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
				
				float mod_x = int(roundf(new_x)) % uint32_t(Catch.x());
				float mod_y = int(roundf(new_y)) % uint32_t(Catch.y());
				float dist_x, dist_y;
				
				if ( new_x < 0 && new_x > _scroll_max.x() && mod_x != 0 ) {
					if ( _scroll.x() - _move_start_scroll.x() < 0 ) {
						dist_x = Catch.x() + mod_x;
					} else {
						dist_x = mod_x;
					}
					new_x -= dist_x;
					dist_x = fabsf(dist_x) * 1e4;
					
					momentum_x.time = N_MAX(N_MIN(dist_x, 3e5), momentum_x.time);
				}
				
				if ( new_y < 0 && new_y > _scroll_max.y() && mod_y != 0 ) {
					if (_scroll.y() - _move_start_scroll.y() < 0) {
						dist_y = Catch.y() + mod_y;
					} else {
						dist_y = mod_y;
					}
					new_y -= dist_y;
					dist_y = fabsf(dist_y) * 1e4;
					
					momentum_y.time = N_MAX(N_MIN(dist_y, 3e5), momentum_y.time);
				}
			}
			
			_moved = false;

			//****************************************************************

			if ( momentum_x.time || momentum_y.time ) {
				uint64_t duration = N_MAX(N_MAX(momentum_x.time, momentum_y.time), 1e4);
				scroll_to_valid_scroll(Vec2(new_x, new_y), duration);
			} else {
				termination_recovery(3e5, ease_in_out);
			}
		}

		void touch_start_handle(UIEvent& e) {
			if ( !_action_id ) {
				TouchEvent* evt = static_cast<TouchEvent*>(&e);
				_action_id = evt->changed_touches()[0].id;
				move_start(Vec2( evt->changed_touches()[0].x, evt->changed_touches()[0].y ));
			}
		}

		void touch_move_handle(UIEvent& e) {
			if ( _action_id && e.return_value ) {
				TouchEvent* evt = static_cast<TouchEvent*>(&e);
				for ( auto& i : evt->changed_touches() ) {
					if (i.id == _action_id) {
						move(Vec2( i.x, i.y )); break;
					}
				}
			}
		}

		void touch_end_handle(UIEvent& e) {
			if ( _action_id ) {
				TouchEvent* evt = static_cast<TouchEvent*>(&e);
				for ( auto& i : evt->changed_touches() ) {
					if (i.id == _action_id) {
						move_end(Vec2( i.x, i.y ));
						_action_id = 0;
						break;
					}
				}
			}
		}

		void mouse_down_handle(UIEvent& e) {
			if ( !_action_id ) {
				MouseEvent* evt = static_cast<MouseEvent*>(&e);
				_action_id = 1;
				move_start(Vec2( evt->x(), evt->y() ));
			}
		}

		void mouse_move_handle(UIEvent& e) {
			if ( _action_id && e.return_value ) {
				MouseEvent* evt = static_cast<MouseEvent*>(&e);
				move(Vec2( evt->x(), evt->y() ));
			}
		}

		void mouse_up_handle(UIEvent& e) {
			if ( _action_id ) {
				MouseEvent* evt = static_cast<MouseEvent*>(&e);
				move_end(Vec2( evt->x(), evt->y() ));
				_action_id = 0;
			}
		}

	};

	BaseScroll::BaseScroll(Box *host)
		: _scrollbar(true)
		, _bounce(true)
		, _bounce_lock(true)
		, _momentum(true)
		, _lock_direction(false)
		, _scrollbar_h(false)
		, _scrollbar_v(false)
		, _resistance(1)
		, _catch_position_x(1)
		, _catch_position_y(1)
		, _scrollbar_color(140, 140, 140, 200)
		, _scrollbar_width(2.0)
		, _scrollbar_margin(2.0)
		, _scroll_duration(0)
		, _host(static_cast<ScrollBox*>(host))
		, _move_start_time(0)
		, _action_id(0)
		, _scrollbar_opacity(0)
		, _scroll_curve(&ease_out)
		, _moved(false)
		, _scroll_h(false), _scroll_v(false)
		, _lock_h(false), _lock_v(false)
	{
		// bind touch event
		host->add_event_listener(UIEvent_TouchStart, &Inl::touch_start_handle, _inl(this));
		host->add_event_listener(UIEvent_TouchMove, &Inl::touch_move_handle, _inl(this));
		host->add_event_listener(UIEvent_TouchEnd, &Inl::touch_end_handle, _inl(this));
		host->add_event_listener(UIEvent_TouchCancel, &Inl::touch_end_handle, _inl(this));
		// bind mouse event
		host->add_event_listener(UIEvent_MouseDown, &Inl::mouse_down_handle, _inl(this));
		host->add_event_listener(UIEvent_MouseMove, &Inl::mouse_move_handle, _inl(this));
		host->add_event_listener(UIEvent_MouseUp, &Inl::mouse_up_handle, _inl(this));
		// N_DEBUG("BaseScroll: %d", sizeof(BaseScroll));
	}

	BaseScroll::~BaseScroll() {
		_inl(this)->termination_all_task();
		if ( _scroll_curve != &ease_out ) {
			delete _scroll_curve;
		}
	}

	void BaseScroll::scroll_to(Vec2 value, uint64_t duration) {
		scroll_to(value, duration, *_scroll_curve);
	}

	void BaseScroll::scroll_to(Vec2 value, uint64_t duration, cCurve& curve) {
		_scroll_raw = Vec2(-value.x(), -value.y());
		Vec2 scroll = _inl(this)->catch_valid_scroll( Vec2(-value.x(), -value.y()) );
		if ( scroll.x() != _scroll.x() || scroll.y() != _scroll.y() ) {
			_inl(this)->scroll_to_valid_scroll(scroll, duration, curve);
		}
		_host->mark_none(View::kScroll);
	}

	void BaseScroll::set_scroll(Vec2 value) {
		if ( _scroll_duration ) {
			scroll_to(value, _scroll_duration, *_scroll_curve);
		} else {
			_scroll_raw = Vec2(-value.x(), -value.y());
			_scroll = _inl(this)->catch_valid_scroll( Vec2(-value.x(), -value.y()) );
			_host->mark_none();
		}
	}

	void BaseScroll::set_scroll_x(float value) {
		_scroll_raw.set_x(-value);
		_scroll = _inl(this)->catch_valid_scroll( Vec2(-value, _scroll_raw.y()) );
		_host->mark_none(View::kScroll);
	}

	void BaseScroll::set_scroll_y(float value) {
		_scroll_raw.set_y(-value);
		_scroll = _inl(this)->catch_valid_scroll( Vec2(_scroll_raw.x(), -value) );
		_host->mark_none(View::kScroll);
	}

	Vec2 BaseScroll::scroll() const {
		return  Vec2(-_scroll.x(), -_scroll.y());
	}

	float BaseScroll::scroll_x() const {
		return -_scroll.y();
	}

	float BaseScroll::scroll_y() const {
		return -_scroll.y();
	}

	void BaseScroll::set_scrollbar(bool value) {
		_scrollbar = value;
	}

	void BaseScroll::set_resistance(float value) {
		_resistance = N_MAX(0.5, value);
	}

	void BaseScroll::set_bounce(bool value) {
		_bounce = value;
	}

	void BaseScroll::set_bounce_lock(bool value) {
		_bounce_lock = value;
	}

	void BaseScroll::set_momentum(bool value) {
		_momentum = value;
	}

	void BaseScroll::set_lock_direction(bool value) {
		_lock_direction = value;
	}

	void BaseScroll::set_catch_position_x(float value) {
		_catch_position_x = value;
	}

	void BaseScroll::set_catch_position_y(float value) {
		_catch_position_y = value;
	}

	void BaseScroll::set_scrollbar_color(Color value) {
		_scrollbar_color = value;
	}

	void BaseScroll::set_scrollbar_width(float value) {
		_scrollbar_width = Float::max(1.0, value);
	}

	void BaseScroll::set_scrollbar_margin(float value) {
		_scrollbar_margin = Float::max(1.0, value);
	}

	void BaseScroll::set_scroll_duration(uint64_t value) {
		_scroll_duration = value;
	}

	void BaseScroll::terminate() {
		_inl(this)->termination_recovery(0);
	}

	void BaseScroll::set_scroll_curve(cCurve* value) {
		if ( _scroll_curve == &ease_out )
			_scroll_curve = new Curve();
		*const_cast<Curve*>(_scroll_curve) = *value;
	}

	void BaseScroll::set_scroll_size(Vec2 size) {
		if (_scroll_size != size) {
			_inl(this)->immediate_end_all_task(); // change size immediate task
			_scroll_size = size;
		}
		auto content_size = _host->content_size();
		_scroll_max = Vec2(Float::min(content_size.x() - size.x(), 0), Float::min(content_size.y() - size.y(), 0));

		_scroll_h = _scroll_max.x() < 0;
		_scroll_v = ((!_bounce_lock && !_scroll_h) || _scroll_max.y() < 0);

		_scroll_h = _scroll_h && !_host->layout_wrap_x(); // 非wrap的size才能滚动
		_scroll_v = _scroll_v && !_host->layout_wrap_y();
		
		_scrollbar_h = (_scroll_h && _scrollbar);
		_scrollbar_v = (_scroll_v && _scrollbar && _scroll_max.y() < 0);
		//
		_host->mark_none(View::kScroll);
	}

	void BaseScroll::solve(uint32_t mark) {
		if ( mark & View::kScroll ) {
			if ( !_moved && !_inl(this)->is_task() ) {
				// fix scroll value
				_scroll = _inl(this)->catch_valid_scroll(_scroll_raw);
				_scroll_raw = _scroll;
			}
			_host->unmark(View::kScroll);
			_host->mark_none(View::kRecursive_Transform);
		}
	}

	// ------------------------ S c r o l l --------------------------

	Scroll::Scroll(): FloatLayout(), BaseScroll(this)
	{}

	Vec2 Scroll::layout_offset_inside() {
		auto origin = origin_value();
		Vec2 offset(
			padding_left() - origin.x() - scroll_x(),
			padding_top() - origin.y() - scroll_y()
		);
		if (_border) {
			offset.val[0] += _border->width_left;
			offset.val[1] += _border->width_top;
		}
		return offset;
	}

	bool Scroll::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (!is_ready_layout_typesetting()) return true; // continue iteration
			auto full_size = layout_typesetting_float(); // return full content size
			set_scroll_size(full_size);
		}
		return false; // stop iteration
	}

	void Scroll::solve_marks(uint32_t mark) {
		BaseScroll::solve(mark);
		View::solve_marks(mark);
	}

}
