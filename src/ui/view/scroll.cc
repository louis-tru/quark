/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include "./scroll.h"
#include "../window.h"
#include "../app.h"
#include "../pre_render.h"
#include "../../util/numbers.h"
#include <math.h>

#define _async_call preRender().async_call

namespace qk {

	// ------------------------ S c r o l l . B a s e --------------------------

	static const Curve ease_in_out(Vec2{0.3f, 0.3f}, Vec2{0.3f, 1.0f});
	static const Curve ease_out(Vec2{0.0f, 0.0f}, Vec2{0.58f, 1.0f});

	constexpr uint32_t kScrollMark = View::kScroll | View::kRecursive_Transform;

	class ScrollBase::Task: public RenderTask {
	public:
		Task(ScrollBase* host, uint64_t duration, cCurve& curve = ease_out, ScrollBase::Task *next = 0)
			: m_host(host)
			, m_start_time(0)
			, m_duration(duration)
			, _next(next)
			, m_immediate_end_flag(false)
			, m_curve(curve)
			, m_is_inl_ease_out(&ease_out == &curve)
		{}

		virtual ~Task() {
			Releasep(_next);
		}

		inline void immediate_end_flag() {
			m_immediate_end_flag = true;
		}
		virtual void run(float y) = 0;
		virtual void end() = 0;
		virtual void immediate_end() = 0;

		void next();

		virtual bool run_task(int64_t sys_time) {
			if ( m_immediate_end_flag ) { // immediate end motion
				immediate_end();
			} else {
				int64_t now = time_monotonic();
				if (m_start_time == 0) {
					m_start_time = now;
				}
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
			return true;
		}

	protected:
		ScrollBase* m_host;
		uint64_t m_start_time;
		uint64_t m_duration;
		List<Task*>::Iterator m_id2;
		ScrollBase::Task *_next;
		bool   m_immediate_end_flag;
		cCurve m_curve;
		bool m_is_inl_ease_out;

		friend class ScrollBase::Inl;
	};

	Qk_DEFINE_INLINE_MEMBERS(ScrollBase, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<ScrollBase::Inl*>(static_cast<ScrollBase*>(self))

		struct Momentum {
			float dist;
			uint64_t time;
		};

		class ScrollMotionTask: public ScrollBase::Task {
		public:
			ScrollMotionTask(ScrollBase* host, uint64_t duration, Vec2 to, cCurve& curve = ease_out)
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

		class ScrollBarTask: public ScrollBase::Task {
		public:
			ScrollBarTask(
				ScrollBase* host, uint64_t duration,
				float from, float to, cCurve& curve = ease_out, ScrollBase::Task *next = 0
			)
				: Task(host, duration, curve, next)
				, m_from(from)
				, m_to(to)
			{}

			virtual void run(float y) {
				auto op = (m_to - m_from) * y + m_from;
				if (op != m_host->_scrollbar_opacity) {
					m_host->_scrollbar_opacity = op;
					m_host->_host->mark(0, true);
				}
				//Qk_DLog("run, %f, %p", m_host->_scrollbar_opacity, this);
			}

			virtual void end() {
				//Qk_DLog("end, %f, %p", m_to, this, this);
				m_host->_scrollbar_opacity = m_to;
				m_host->_host->mark(0, true);
				next();
				_inl(m_host)->termination_task(this);
			}

			virtual void immediate_end() {
				m_host->_scrollbar_opacity = m_to;
				//Qk_DLog("immediate_end, %f, %p", m_to, this);
				m_host->_host->mark(0, true);
				_inl(m_host)->termination_task(this);
			}

		protected:
			float m_from;
			float m_to;
		};
		
		class ScrollBarFadeInOutTask: public ScrollBarTask {
		public:
			ScrollBarFadeInOutTask(ScrollBase* host, uint64_t in, uint64_t fixed, uint64_t out)
				: ScrollBarTask(host, in, host->_scrollbar_opacity, 1, ease_out), _fixed(fixed), _out(out), _step(0)
			{}
			virtual void end() {
				_step++;
				if (_step == 1) {
					m_start_time = time_monotonic();
					m_from = m_to;
					m_duration = _fixed;
				} else if (_step == 2) {
					m_start_time = time_monotonic();
					m_to = 0;
					m_duration = _out;
				} else {
					ScrollBarTask::end();
				}
			}
		private:
			uint64_t _fixed, _out;
			int _step;
		};

		void register_task(Task* task) {
			if ( !task->is_register_task() ) {
				task->m_id2 = _tasks.pushBack(task);
				preRender().addtask(task);
				task->run_task(0);
			}
		}

		void termination_task(Task* task) {
			if ( task->is_register_task() ) {
				_tasks.erase( task->m_id2 );
				delete task;
			}
		}

		void termination_all_task_Rt() {
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

		inline PreRender& preRender() {
			return _host->window()->preRender();
		}

		// scroll
		// ------------------------------------------------------------------------

		Momentum get_momentum(uint64_t time, float dist, float max_dist_upper, float max_dist_lower, float size) {
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

		Vec2 get_optimal_display(Vec2 value) {
			auto scale = _host->window()->scale();
			value.set_x( round(value.x() * scale) / scale );
			value.set_y( round(value.y() * scale) / scale );
			return value;
		}

		Vec2 get_catch_valid_scroll(Vec2 scroll) {
			Vec2 valid = get_valid_scroll(scroll.x(), scroll.y());
			Vec2 catch_ = get_catch_value();

			if ( catch_.x() != 1 && catch_.y() != 1 ) { // 捕获位置
				valid.set_x( roundf(valid.x() / catch_.x()) * catch_.x() );
				if ( valid.x() < _scroll_max.x() ) {
					valid.set_x( valid.x() + catch_.x() );
				}
				valid.set_y( roundf(valid.y() / catch_.y()) * catch_.y() );
				if ( valid.y() < _scroll_max.y() ) {
					valid.set_y( valid.y() + catch_.y() );
				}
			}
			return get_optimal_display(valid);
		}

		void set_h_scrollbar_pos() {
			if ( ! _scrollbar_h ) return;

			float size_x = _host->content_size().x() + _host->padding_left() + _host->padding_right();
			float left_margin = scrollbar_margin();
			float right_margin = _scrollbar_v ? left_margin + scrollbar_width() : left_margin;
			float h_scrollbar_max_size = Float32::max(size_x - left_margin - right_margin, 0);

			float h_scrollbar_indicator_size = roundf(powf(h_scrollbar_max_size, 2) / _scroll_size.x());
			h_scrollbar_indicator_size = Float32::max(h_scrollbar_indicator_size, 8);
			float h_scrollbar_max_scroll = h_scrollbar_max_size - h_scrollbar_indicator_size;
			float h_scrollbar_prop = h_scrollbar_max_scroll / _scroll_max.x();

			// ------------------------------------------------------

			float pos = h_scrollbar_prop * _scroll.load().x();
			float size = h_scrollbar_indicator_size;

			if ( pos < 0 ) {
				size = h_scrollbar_indicator_size + roundf(pos * 3);
				size = Qk_Max(size, 8);
				pos = 0;
			} else if ( pos > h_scrollbar_max_scroll ) {
				size = h_scrollbar_indicator_size - roundf((pos - h_scrollbar_max_scroll) * 3);
				size = Qk_Max(size, 8);
				pos = h_scrollbar_max_scroll + h_scrollbar_indicator_size - size;
			}

			_scrollbar_position_h = Vec2(pos + left_margin, size);
		}

		void set_v_scrollbar_pos() {
			if ( ! _scrollbar_v ) return;

			float size_y = _host->content_size().y() + _host->padding_top() + _host->padding_bottom();
			float top_margin = scrollbar_margin();
			float bottom_margin = _scrollbar_h ? top_margin + scrollbar_width() : top_margin;
			float v_scrollbar_max_size = Float32::max(size_y - top_margin - bottom_margin, 0);

			float v_scrollbar_indicator_size = roundf(powf(v_scrollbar_max_size, 2) / _scroll_size.y());
			v_scrollbar_indicator_size = Float32::max(v_scrollbar_indicator_size, 8);

			float v_scrollbar_max_scroll = v_scrollbar_max_size - v_scrollbar_indicator_size;
			float v_scrollbar_prop = v_scrollbar_max_scroll / _scroll_max.y();

			// ------------------------------------------------------

			float pos = v_scrollbar_prop * _scroll.load().y();
			float size = v_scrollbar_indicator_size;

			if ( pos < 0 ) {
				size = v_scrollbar_indicator_size + roundf(pos * 3);
				size = Float32::max(size, 8);
				pos = 0;
			} else if ( pos > v_scrollbar_max_scroll ) {
				size = v_scrollbar_indicator_size - roundf((pos - v_scrollbar_max_scroll) * 3);
				size = Float32::max(size, 8);
				pos = v_scrollbar_max_scroll + v_scrollbar_indicator_size - size;
			}

			_scrollbar_position_v = Vec2(pos + top_margin, size);
		}

		void set_scroll_and_trigger_event(Vec2 scroll, bool forceTrigger = false) {
			scroll    = get_optimal_display(scroll);
			scroll[0] = _scroll_h ? scroll.x() : 0;
			scroll[1] = _scroll_v ? scroll.y() : 0;

			if (forceTrigger || _scroll.load() != scroll) {
				_scroll = scroll;
				set_h_scrollbar_pos();
				set_v_scrollbar_pos();
				_host->mark(kScrollMark, true); // mark

				_host->preRender().post(Cb([this, scroll](auto& e) {
					Sp<UIEvent> evt = new UIEvent(_host);
					_host->trigger(UIEvent_Scroll, **evt);
				}), _host);
			}
		}

		void scroll_to_valid_scroll(Vec2 valid_scroll, uint64_t duration, cCurve& curve = ease_out) {
			termination_all_task_Rt();
			if ( duration ) {
				motion_start(valid_scroll, duration, curve);
			} else {
				set_scroll_and_trigger_event(valid_scroll);
			}
		}

		void termination_recovery(uint64_t duration, cCurve& curve = ease_out) {
			termination_all_task_Rt();

			Vec2 scroll = get_catch_valid_scroll(_scroll);
			if ( scroll == _scroll ) {
				if ( duration ) {
					if ( _scrollbar_opacity != 0 ) {
						register_task( new ScrollBarTask(this, 3e5, _scrollbar_opacity, 0) );
					}
				} else {
					if ( _scrollbar_opacity != 0 ) {
						_scrollbar_opacity = 0;
						_host->mark(0, true);
					}
				}
			} else {
				scroll_to_valid_scroll(scroll, duration, curve);
			}
		}

		void motion_start(Vec2 scroll, uint64_t duration, cCurve& curve) {
			if ( !is_task() && ! _moved ) {
				if ( scroll != _scroll ) {
					register_task( new ScrollMotionTask(this, duration, scroll, curve) );
					if ( _scrollbar_opacity != 1 ) {
						register_task( new ScrollBarTask(this, 5e4, _scrollbar_opacity, 1) );
					}
				}
			}
		}

		void move_start(Vec2 point) {
			termination_all_task_Rt();
			_moved = false;
			_move_dist = Vec2();
			_move_point = point;
			_move_start_time = time_monotonic();
			_move_start_scroll = _scroll;
		}

		void move(Vec2 point) {
			float delta_x = point.x() - _move_point.x();
			float delta_y = point.y() - _move_point.y();
			Vec2 _scroll = this->_scroll.load();
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
					register_task( new ScrollBarTask(this, 2e5, _scrollbar_opacity, 1) );
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
			Vec2 _scroll = this->_scroll;
			float new_x = _scroll.x();
			float new_y = _scroll.y();

			_lock_h = false;
			_lock_v = false;

			// Calculate inertia
			if ( duration < 3e5 ) {
				if ( _momentum ) {
					auto size = _host->content_size();
					if ( new_x ) {
						momentum_x = get_momentum(duration, new_x - _move_start_scroll.x(),
																			-_scroll.x(), _scroll.x() - _scroll_max.x(),
																			_bounce ? size.x() / 2.0 : 0);
					}
					if ( new_y ) {
						momentum_y = get_momentum(duration, new_y - _move_start_scroll.y(),
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

				Vec2 Catch = get_catch_value(); // Capture location

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

					momentum_x.time = Qk_Max(Qk_Min(dist_x, 3e5), momentum_x.time);
				}

				if ( new_y < 0 && new_y > _scroll_max.y() && mod_y != 0 ) {
					if (_scroll.y() - _move_start_scroll.y() < 0) {
						dist_y = Catch.y() + mod_y;
					} else {
						dist_y = mod_y;
					}
					new_y -= dist_y;
					dist_y = fabsf(dist_y) * 1e4;

					momentum_y.time = Qk_Max(Qk_Min(dist_y, 3e5), momentum_y.time);
				}
			}

			_moved = false;

			//****************************************************************

			if ( momentum_x.time || momentum_y.time ) {
				uint64_t duration = Qk_Max(Qk_Max(momentum_x.time, momentum_y.time), 1e4);
				scroll_to_valid_scroll(Vec2(new_x, new_y), duration);
			} else {
				termination_recovery(3e5, ease_in_out);
			}
		}

		// event handles
		// ------------------------------------------------------------------------

		void handle_TouchStart(UIEvent& e) {
			auto evt = static_cast<TouchEvent*>(&e);
			auto args = new TouchEvent::TouchPoint(evt->changed_touches()[0]);
			_async_call([](auto ctx, auto arg) {
				Sp<TouchEvent::TouchPoint> handle(arg.arg);
				if ( !ctx->_action_id ) {
					ctx->_action_id = arg.arg->id;
					ctx->move_start(Vec2( arg.arg->x, arg.arg->y ));
				}
			}, this, args);
		}

		void handle_TouchMove(UIEvent& e) {
			if (_action_id && e.is_default()) {
				auto evt = static_cast<TouchEvent*>(&e);
				auto args = new Array<TouchEvent::TouchPoint>(evt->changed_touches());
				_async_call([](auto ctx, auto arg) {
					Sp<Array<TouchEvent::TouchPoint>> handle(arg.arg);
					if ( ctx->_action_id ) {
						for ( auto &i : *arg.arg ) {
							if (i.id == ctx->_action_id) {
								ctx->move(Vec2( i.x, i.y )); break;
							}
						}
					}
				}, this, args);
			}
		}

		void handle_TouchEnd(UIEvent& e) {
			auto evt = static_cast<TouchEvent*>(&e);
			auto args = new Array<TouchEvent::TouchPoint>(evt->changed_touches());
			_async_call([](auto ctx, auto arg) {
				if ( ctx->_action_id ) {
					for ( auto &i: *arg.arg ) {
						if (i.id == ctx->_action_id) {
							ctx->move_end(Vec2( i.x, i.y ));
							ctx->_action_id = 0;
							break;
						}
					}
				}
			}, this, args);
		}

		void handle_MouseDown(UIEvent& e) {
			auto evt = static_cast<MouseEvent*>(&e);
			_async_call([](auto ctx, auto arg) {
				if ( !ctx->_action_id ) {
					ctx->_action_id = 1;
					ctx->move_start(arg.arg);
				}
			}, this, Vec2( evt->x(), evt->y() ));
		}

		void handle_MouseMove(UIEvent& e) {
			if (_action_id && e.is_default()) {
				auto evt = static_cast<MouseEvent*>(&e);
				_async_call([](auto ctx, auto arg) {
					if ( ctx->_action_id ) {
						ctx->move(arg.arg);
					}
				}, this, Vec2( evt->x(), evt->y() ));
			}
		}

		void handle_MouseUp(UIEvent& e) {
			auto evt = static_cast<MouseEvent*>(&e);
			_async_call([](auto ctx, auto arg) {
				if ( ctx->_action_id ) {
					ctx->move_end(arg.arg);
					ctx->_action_id = 0;
				}
			}, this, Vec2( evt->x(), evt->y() ));
		}

		void handle_MouseWheel(UIEvent& e) {
			auto &evt = static_cast<MouseEvent&>(e);
			_async_call([](auto self, auto arg) {
				Vec2 v0 = self->_scroll.load() + (arg.arg * 10);
				Vec2 v = self->get_catch_valid_scroll(v0);
				if ( v != self->_scroll ) {
					self->scroll_to_valid_scroll(v, 0);
					self->register_task(new ScrollBarFadeInOutTask(self, 5e4, 1e6, 3e5));
				}
			}, this, Vec2(evt.x(), evt.y()));
		}

	};
	
	void ScrollBase::Task::next() {
		if (_next) {
			_inl(m_host)->register_task(_next);
			_next = nullptr;
		}
	}

	ScrollBase::ScrollBase(Box *host)
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
		, _host(host)
		, _scroll(Vec2())
		, _move_start_time(0)
		, _action_id(0)
		, _scrollbar_opacity(0)
		, _default_curve_Mt(ease_out)
		, _moved(false)
		, _scroll_h(false), _scroll_v(false)
		, _lock_h(false), _lock_v(false)
	{
		// bind touch event
		host->add_event_listener(UIEvent_TouchStart, &Inl::handle_TouchStart, _inl(this));
		host->add_event_listener(UIEvent_TouchMove, &Inl::handle_TouchMove, _inl(this));
		host->add_event_listener(UIEvent_TouchEnd, &Inl::handle_TouchEnd, _inl(this));
		host->add_event_listener(UIEvent_TouchCancel, &Inl::handle_TouchEnd, _inl(this));
		// bind mouse event
		//host->add_event_listener(UIEvent_MouseDown, &Inl::handle_MouseDown, _inl(this));
		//host->add_event_listener(UIEvent_MouseMove, &Inl::handle_MouseMove, _inl(this));
		//host->add_event_listener(UIEvent_MouseUp, &Inl::handle_MouseUp, _inl(this));
		host->add_event_listener(UIEvent_MouseWheel, &Inl::handle_MouseWheel, _inl(this));
	}

	ScrollBase::~ScrollBase() {
		_this->termination_all_task_Rt();
	}

	void ScrollBase::set_scrollbar(bool value, bool isRt) {
		_scrollbar = value;
		_host->mark(0, isRt);
	}

	void ScrollBase::set_resistance(float value, bool isRt) {
		_resistance = Qk_Max(0.5, value);
	}

	void ScrollBase::set_bounce(bool value, bool isRt) {
		_bounce = value;
	}

	void ScrollBase::set_bounce_lock(bool value, bool isRt) {
		_bounce_lock = value;
	}

	void ScrollBase::set_momentum(bool value, bool isRt) {
		_momentum = value;
	}

	void ScrollBase::set_lock_direction(bool value, bool isRt) {
		_lock_direction = value;
	}

	void ScrollBase::set_catch_position_x(float value, bool isRt) {
		_catch_position_x = value;
	}

	void ScrollBase::set_catch_position_y(float value, bool isRt) {
		_catch_position_y = value;
	}

	void ScrollBase::set_scrollbar_color(Color value, bool isRt) {
		_scrollbar_color = value;
		_host->mark(0, isRt);
	}

	void ScrollBase::set_scrollbar_width(float value, bool isRt) {
		_scrollbar_width = Float32::max(1.0, value);
		_host->mark(0, isRt);
	}

	void ScrollBase::set_scrollbar_margin(float value, bool isRt) {
		_scrollbar_margin = Float32::max(1.0, value);
		_host->mark(0, isRt);
	}

	void ScrollBase::set_scroll_duration(uint64_t value, bool isRt) {
		_scroll_duration = value;
	}

	cCurve& ScrollBase::default_curve() const {
		return _default_curve_Mt;
	}

	void ScrollBase::set_default_curve(cCurve& value, bool isRt) {
		_default_curve_Mt = value;
	}

	void ScrollBase::terminate() {
		_host->_async_call([](auto self, auto arg) {
			self->termination_recovery(0);
		}, _this, 0);
	}

	float ScrollBase::scroll_x() const {
		return -_scroll.load().x();
	}

	float ScrollBase::scroll_y() const {
		return -_scroll.load().y();
	}

	void ScrollBase::set_scroll_x(float value, bool isRt) {
		set_scroll({value, -_scroll.load()[1]}, isRt);
	}

	void ScrollBase::set_scroll_y(float value, bool isRt) {
		set_scroll({-_scroll.load()[0], value}, isRt);
	}

	Vec2 ScrollBase::scroll() const {
		Vec2 v = _scroll;
		return {-v[0],-v[1]};
	}

	void ScrollBase::set_scroll(Vec2 value, bool isRt) {
		if (isRt) {
			_this->set_scroll_and_trigger_event(_this->get_catch_valid_scroll({-value.x(), -value.y()}));
		} else if (_scroll_duration) {
			scrollTo(value, _scroll_duration, _default_curve_Mt);
		} else {
			value = _this->get_valid_scroll(-value[0], -value[1]);
			if (value != _scroll) {
				_scroll = value;
				_host->_async_call([](auto self, auto arg) {
					self->set_scroll_and_trigger_event(
						self->get_catch_valid_scroll({-arg.arg.x(), -arg.arg.y()}),
						true
					);
				}, _this, value);
			}
		}
	}

	void ScrollBase::scrollTo(Vec2 value, uint64_t duration, cCurve& curve) {
		struct Args { Vec2 value; uint64_t duration; Curve curve; };
		_host->_async_call([](auto self, auto val) {
			Vec2 v = self->get_catch_valid_scroll({-val.arg->value.x(), -val.arg->value.y()});
			if ( v != self->_scroll ) {
				self->scroll_to_valid_scroll(v, val.arg->duration, val.arg->curve);
			}
			delete val.arg;
		}, _this, new Args{value,duration,curve});
	}

	void ScrollBase::solve(uint32_t mark) {
		if ( mark & View::kScroll ) {
			if ( !_moved && !_this->is_task() ) {
				_this->set_scroll_and_trigger_event(_this->get_catch_valid_scroll(_scroll));
			}
			_host->unmark(View::kScroll);
		}
	}

	void ScrollBase::set_scroll_size_Rt(Vec2 size) {
		if (_scroll_size != size) {
			_scroll_size = size;
			_this->immediate_end_all_task(); // change size immediate task
		}
		auto cSize = _host->content_size();
		_scroll_max = Vec2(Float32::min(cSize.x() - size.x(), 0), Float32::min(cSize.y() - size.y(), 0));

		_scroll_h = _scroll_max.x() < 0;
		_scroll_v = ((!_bounce_lock && !_scroll_h) || _scroll_max.y() < 0);

		_scroll_h = _scroll_h && !_host->wrap_x(); // 非wrap的size才能滚动
		_scroll_v = _scroll_v && !_host->wrap_y();

		_scrollbar_h = (_scroll_h && _scrollbar);
		_scrollbar_v = (_scroll_v && _scrollbar && _scroll_max.y() < 0);

		_host->mark(kScrollMark, true);
	}

	// ------------------------ S c r o l l . L a y o u t --------------------------

	Scroll::Scroll(): Box(), ScrollBase(this)
	{
	}

	View* Scroll::init(Window *win) {
		View::init(win);
		set_clip(true);
		set_receive(true);
		return this;
	}

	Vec2 Scroll::layout_offset_inside() {
		Vec2 offset(
			padding_left() - scroll_x(),
			padding_top() - scroll_y()
		);
		auto _border = this->_border.load();
		if (_border) {
			offset.val[0] += _border->width[3]; // left
			offset.val[1] += _border->width[0]; // top
		}
		return offset;
	}

	void Scroll::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			set_scroll_size_Rt(layout_typesetting_float());
		}
	}

	void Scroll::solve_marks(const Mat &mat, uint32_t mark) {
		ScrollBase::solve(mark);
		Box::solve_marks(mat, mark_value());
	}

	ScrollBase* Scroll::asScrollBase() {
		return this;
	}

	ViewType Scroll::viewType() const {
		return kScroll_ViewType;
	}

}
