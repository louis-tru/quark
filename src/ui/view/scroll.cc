/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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

#define _async_call(block, param) _host->async_call([](auto self, auto arg) block, _this, param)

namespace qk {

	// ------------------------ S c r o l l . B a s e --------------------------

	static const Curve ease_in_out(Vec2{0.3f, 0.3f}, Vec2{0.3f, 1.0f});
	static const Curve ease_out(Vec2{0.0f, 0.0f}, Vec2{0.58f, 1.0f});

	constexpr float kScrollBaseDeceleration = 2.6f; // exponential decay rate per second
	constexpr float kScrollBaseRubberBandCoefficient = 0.55f;
	constexpr float kScrollBaseSpringStiffness = 180.0f;
	constexpr float kScrollBaseSpringDamping = 24.0f;
	constexpr float kScrollBaseMaxVelocity = 3500.0f; // logical pixels per second
	constexpr float kScrollMinimumPhysicsScale = 0.1f;
	constexpr float kScrollPhysicsMaxDelta = 1.0f / 30.0f;
	constexpr float kScrollSettleVelocity = 3.0f;
	constexpr float kScrollSettleDistance = 0.25f;
	constexpr uint64_t kScrollVelocitySampleWindow = 120000; // microseconds
	constexpr uint64_t kScrollVelocityMinDuration = 8000; // microseconds
	constexpr uint64_t kScrollPhysicsRecoveryDuration = 200000; // microseconds

	constexpr uint32_t kScrollMark = View::kScroll | View::kTransform;

	Vec2 free_typesetting(View* view, const View::Container &container);

	class ScrollView::Task: public RenderTask {
	public:
		Task(ScrollView* host, uint64_t duration, cCurve& curve = ease_out, ScrollView::Task *next = 0)
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

		virtual bool run_task(int64_t time, int64_t deltaTime) {
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
		ScrollView* m_host;
		uint64_t m_start_time;
		uint64_t m_duration;
		List<Task*>::Iterator m_id2;
		ScrollView::Task *_next;
		bool   m_immediate_end_flag;
		cCurve m_curve;
		bool m_is_inl_ease_out;

		friend class ScrollView::Inl;
	};

	Qk_DEFINE_INLINE_MEMBERS(ScrollView, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<ScrollView::Inl*>(static_cast<ScrollView*>(self))

		class ScrollMotionTask: public ScrollView::Task {
		public:
			ScrollMotionTask(ScrollView* host, uint64_t duration, Vec2 to, cCurve& curve = ease_out)
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

		class ScrollPhysicsTask: public ScrollView::Task {
		public:
			ScrollPhysicsTask(ScrollView *host, Vec2 velocity)
				: Task(host, 0), _position(host->_scroll.load()), _velocity(velocity) {}

			bool run_task(int64_t time, int64_t deltaTime) override {
				if (m_immediate_end_flag) {
					immediate_end();
					return true;
				}
				float dt = F32::clamp(float(deltaTime) * 1e-6f, 0.0f, kScrollPhysicsMaxDelta);
				if (dt == 0.0f)
					return true;
				bool settledX = update_axis(_position[0], _velocity[0], m_host->_scroll_max[0], dt);
				bool settledY = update_axis(_position[1], _velocity[1], m_host->_scroll_max[1], dt);
				_inl(m_host)->set_scroll_and_trigger_event(_position);

				if (settledX && settledY) {
					_inl(m_host)->termination_recovery(kScrollPhysicsRecoveryDuration, ease_in_out);
				}
				return true;
			}

			void run(float) override {}
			void end() override { immediate_end(); }
			void immediate_end() override {
				_inl(m_host)->set_scroll_and_trigger_event(
					_inl(m_host)->get_valid_scroll(m_host->_scroll.load().x(), m_host->_scroll.load().y())
				);
				_inl(m_host)->termination_recovery(0);
			}

		private:
			bool update_axis(float &position, float &velocity, float min, float dt) {
				float bound = position > 0.0f ? 0.0f : position < min ? min : position;
				bool outside = bound != position;
				if (outside) {
					// Damped spring preserving release velocity across the boundary.
					velocity += ((bound - position) * kScrollBaseSpringStiffness * m_host->_bounce_stiffness -
						velocity * kScrollBaseSpringDamping * m_host->_bounce_damping) * dt;
				} else {
					// Frame-rate independent inertial decay.
					velocity *= expf(-kScrollBaseDeceleration * m_host->_resistance * dt);
				}
				position += velocity * dt;
				if (!m_host->_bounce) {
					if (position > 0.0f) {
						position = 0.0f;
						velocity = 0.0f;
					} else if (position < min) {
						position = min;
						velocity = 0.0f;
					}
				}

				bound = position > 0.0f ? 0.0f : position < min ? min : position;
				if (fabsf(velocity) < kScrollSettleVelocity &&
					fabsf(position - bound) < kScrollSettleDistance) {
					position = bound;
					velocity = 0.0f;
					return true;
				}
				return false;
			}

			Vec2 _position; // continuous position, independent of display pixel snapping
			Vec2 _velocity; // pixels per second
		};

		class ScrollBarTask: public ScrollView::Task {
		public:
			ScrollBarTask(
				ScrollView* host, uint64_t duration,
				float from, float to, cCurve& curve = ease_out, ScrollView::Task *next = 0
			)
				: Task(host, duration, curve, next)
				, m_from(from)
				, m_to(to)
			{}

			virtual void run(float y) {
				auto op = (m_to - m_from) * y + m_from;
				if (op != m_host->_scrollbar_opacity) {
					m_host->_scrollbar_opacity = op;
					m_host->_host->mark<true>(0);
				}
				//Qk_DLog("run, %f, %p", m_host->_scrollbar_opacity, this);
			}

			virtual void end() {
				//Qk_DLog("end, %f, %p", m_to, this, this);
				m_host->_scrollbar_opacity = m_to;
				m_host->_host->mark<true>(0);
				next();
				_inl(m_host)->termination_task(this);
			}

			virtual void immediate_end() {
				m_host->_scrollbar_opacity = m_to;
				//Qk_DLog("immediate_end, %f, %p", m_to, this);
				m_host->_host->mark<true>(0);
				_inl(m_host)->termination_task(this);
			}

		protected:
			float m_from;
			float m_to;
		};
		
		class ScrollBarFadeInOutTask: public ScrollBarTask {
		public:
			ScrollBarFadeInOutTask(ScrollView* host, uint64_t in, uint64_t fixed, uint64_t out)
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
				pre_render().addtask(task);
				task->run_task(0, 0);
			}
		}

		void termination_task(Task* task) {
			if ( task->is_register_task() ) {
				_tasks.erase( task->m_id2 );
				delete task;
			}
		}

		void termination_all_task_rt() {
			for ( auto i : _tasks ) {
				delete i;
			}
			_tasks.clear();
		}

		void immediate_end_all_task() {
			for ( auto i : _tasks ) {
				i->immediate_end_flag();
			}
		}

		inline bool is_task() {
			return _tasks.length();
		}

		inline PreRender& pre_render() {
			return _host->window()->pre_render();
		}

		// scroll
		// ------------------------------------------------------------------------

		float rubber_band(float distance, float dimension) {
			if (dimension <= 0.0f)
				return 0.0f;
			float coefficient = kScrollBaseRubberBandCoefficient / _bounce_resistance;
			return distance * coefficient * dimension /
				(dimension + coefficient * distance);
		}

		float drag_axis(float value, float min, float dimension) {
			if (!_bounce)
				return value > 0.0f ? 0.0f : value < min ? min : value;
			if (value > 0.0f)
				return rubber_band(value, dimension);
			if (value < min)
				return min - rubber_band(min - value, dimension);
			return value;
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
			float h_scrollbar_max_size = F32::max(size_x - left_margin - right_margin, 0);

			float h_scrollbar_indicator_size = roundf(powf(h_scrollbar_max_size, 2) / _scroll_size.x());
			h_scrollbar_indicator_size = F32::max(h_scrollbar_indicator_size, 8);
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
			float v_scrollbar_max_size = F32::max(size_y - top_margin - bottom_margin, 0);

			float v_scrollbar_indicator_size = roundf(powf(v_scrollbar_max_size, 2) / _scroll_size.y());
			v_scrollbar_indicator_size = F32::max(v_scrollbar_indicator_size, 8);

			float v_scrollbar_max_scroll = v_scrollbar_max_size - v_scrollbar_indicator_size;
			float v_scrollbar_prop = v_scrollbar_max_scroll / _scroll_max.y();

			// ------------------------------------------------------

			float pos = v_scrollbar_prop * _scroll.load().y();
			float size = v_scrollbar_indicator_size;

			if ( pos < 0 ) {
				size = v_scrollbar_indicator_size + roundf(pos * 3);
				size = F32::max(size, 8);
				pos = 0;
			} else if ( pos > v_scrollbar_max_scroll ) {
				size = v_scrollbar_indicator_size - roundf((pos - v_scrollbar_max_scroll) * 3);
				size = F32::max(size, 8);
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
				_host->mark<true>(kScrollMark); // mark

				_host->pre_render().post(Cb([this, scroll](auto& e) {
					Sp<UIEvent> evt = new UIEvent(_host);
					_host->trigger(UIEvent_Scroll, **evt);
				}), _host);
			}
		}

		void scroll_to_valid_scroll(Vec2 valid_scroll, uint64_t duration, cCurve& curve = ease_out) {
			termination_all_task_rt();
			if ( duration ) {
				motion_start(valid_scroll, duration, curve);
			} else {
				set_scroll_and_trigger_event(valid_scroll);
			}
		}

		void termination_recovery(uint64_t duration, cCurve& curve = ease_out) {
			termination_all_task_rt();

			Vec2 scroll = get_catch_valid_scroll(_scroll);
			if ( scroll == _scroll ) {
				if ( duration ) {
					if ( _scrollbar_opacity != 0 ) {
						register_task( new ScrollBarTask(this, 3e5, _scrollbar_opacity, 0) );
					}
				} else {
					if ( _scrollbar_opacity != 0 ) {
						_scrollbar_opacity = 0;
						_host->mark<true>(0);
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
			termination_all_task_rt();
			_moved = false;
			_move_dist = Vec2();
			_move_point = point;
			_move_raw_scroll = _scroll.load();
		}

		void move(Vec2 point) {
			float delta_x = point.x() - _move_point.x();
			float delta_y = point.y() - _move_point.y();
			_move_point = point;

			_move_dist.set_x( _move_dist.x() + delta_x );
			_move_dist.set_y( _move_dist.y() + delta_y );

			float dist_x = fabsf(_move_dist.x());
			float dist_y = fabsf(_move_dist.y());

			if ( !_moved ) {
				if ( dist_x < 0.5 && dist_y < 0.5 ) { // 小于阈值的移动距离不处理
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
					delta_y = 0;
				} else if( _lock_h ) {
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

			_move_raw_scroll += Vec2(delta_x, delta_y);
			auto size = _host->content_size();
			set_scroll_and_trigger_event({
				drag_axis(_move_raw_scroll.x(), _scroll_max.x(), size.x()),
				drag_axis(_move_raw_scroll.y(), _scroll_max.y(), size.y()),
			});
		}

		void move_end(Vec2 velocity) {
			if (_lock_h)
				velocity.set_x(0);
			if (_lock_v)
				velocity.set_y(0);
			_lock_h = false;
			_lock_v = false;
			_moved = false;
			if (!_momentum)
				velocity = Vec2();
			if (!_scroll_h)
				velocity.set_x(0);
			if (!_scroll_v)
				velocity.set_y(0);

			auto scroll = _scroll.load();
			bool outside = scroll.x() > 0 || scroll.x() < _scroll_max.x() ||
				scroll.y() > 0 || scroll.y() < _scroll_max.y();
			if (outside || velocity != Vec2()) {
				register_task(new ScrollPhysicsTask(this, velocity));
			} else {
				termination_recovery(3e5, ease_in_out);
			}
		}

		// event handles
		// ------------------------------------------------------------------------

		void handle_TouchStart(UIEvent& e) {
			if ( !_action_id ) {
				auto& pos = static_cast<TouchEvent*>(&e)->changed_touches()[0];
				_action_id = pos.id;
				begin_drag(pos.position, pos.time);
			}
		}

		void handle_TouchMove(UIEvent& e) {
			if (_action_id && e.is_default()) {
				for (auto &i : static_cast<TouchEvent*>(&e)->changed_touches()) {
					if (i.id == _action_id) {
						drag(i.position, i.time);
						break;
					}
				}
			}
		}

		void handle_TouchEnd(UIEvent& e) {
			if (_action_id) {
				for ( auto &i: static_cast<TouchEvent*>(&e)->changed_touches() ) {
					if (i.id == _action_id) {
						_action_id = 0;
						end_drag(i.position, i.time);
						break;
					}
				}
			}
		}

		void handle_MouseDown(UIEvent& e) {
			begin_drag(static_cast<MouseEvent*>(&e)->position());
		}

		void handle_MouseMove(UIEvent& e) {
			if (e.is_default()) {
				drag(static_cast<MouseEvent*>(&e)->position());
			}
		}

		void handle_MouseUp(UIEvent& e) {
			end_drag(static_cast<MouseEvent*>(&e)->position());
		}

		void handle_MouseWheel(UIEvent& e) {
			wheel(static_cast<MouseEvent&>(e).delta());
		}

	};
	
	void ScrollView::Task::next() {
		if (_next) {
			_inl(m_host)->register_task(_next);
			_next = nullptr;
		}
	}

	ScrollView::ScrollView(Box *host)
		: _scrollbar(true)
		, _bounce(true)
		, _bounce_lock(true)
		, _momentum(true)
		, _lock_direction(false)
		, _scrollbar_h(false)
		, _scrollbar_v(false)
		, _resistance(1)
		, _bounce_resistance(1)
		, _bounce_stiffness(1)
		, _bounce_damping(1)
		, _momentum_velocity(1)
		, _catch_position_x(1)
		, _catch_position_y(1)
		, _scrollbar_color(140, 140, 140, 200)
		, _scrollbar_width(4.0)
		, _scrollbar_margin(2.0)
		, _scroll_duration(0)
		, _host(host)
		, _scroll(Vec2())
		, _dragSampleCount(0)
		, _action_id(0)
		, _scrollbar_opacity(0)
		, _default_curve_Wt(ease_out)
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

	ScrollView::~ScrollView() {
		_this->termination_all_task_rt();
	}

	void ScrollView::add_drag_sample(Vec2 position, uint64_t time) {
		if (!time)
			time = time_monotonic();
		if (_dragSampleCount && time < _dragSamples[_dragSampleCount - 1].time)
			_dragSampleCount = 0;
		if (_dragSampleCount == sizeof(_dragSamples) / sizeof(_dragSamples[0])) {
			for (uint32_t i = 1; i < _dragSampleCount; i++)
				_dragSamples[i - 1] = _dragSamples[i];
			_dragSampleCount--;
		}
		if (_dragSampleCount && time == _dragSamples[_dragSampleCount - 1].time) {
			_dragSamples[_dragSampleCount - 1].position = position;
		} else {
			_dragSamples[_dragSampleCount++] = { position, time };
		}
	}

	Vec2 ScrollView::drag_velocity() const {
		if (_dragSampleCount < 2)
			return {};
		auto &last = _dragSamples[_dragSampleCount - 1];
		uint32_t first = _dragSampleCount - 1;
		for (uint32_t i = _dragSampleCount - 1; i > 0; i--) {
			auto age = last.time - _dragSamples[i - 1].time;
			if (age > kScrollVelocitySampleWindow)
				break;
			first = i - 1;
		}
		auto duration = last.time - _dragSamples[first].time;
		if (duration < kScrollVelocityMinDuration)
			return {};
		Vec2 velocity = (last.position - _dragSamples[first].position) *
			(1e6f / float(duration));
		float maxVelocity = kScrollBaseMaxVelocity * _momentum_velocity;
		velocity.set_x(F32::max(-maxVelocity, F32::min(maxVelocity, velocity.x())));
		velocity.set_y(F32::max(-maxVelocity, F32::min(maxVelocity, velocity.y())));
		return velocity;
	}

	void ScrollView::begin_drag(Vec2 pos, uint64_t time) {
		_dragSampleCount = 0;
		add_drag_sample(pos, time);
		_async_call({
			self->move_start(arg);
		}, pos);
	}

	void ScrollView::drag(Vec2 pos, uint64_t time) {
		add_drag_sample(pos, time);
		_async_call({
			self->move(arg);
		}, pos);
	}

	void ScrollView::end_drag(Vec2 pos, uint64_t time) {
		add_drag_sample(pos, time);
		auto velocity = drag_velocity();
		_dragSampleCount = 0;
		_async_call({
			self->move_end(arg);
		}, velocity);
	}

	void ScrollView::wheel(Vec2 delta) {
		_async_call({
			Vec2 v0 = self->_scroll.load() - arg;
			Vec2 v = self->get_catch_valid_scroll(v0);
			if ( v != self->_scroll ) {
				self->scroll_to_valid_scroll(v, 0);
				self->register_task(new Inl::ScrollBarFadeInOutTask(self, 5e4, 1e6, 3e5));
			}
		}, delta);
	}

	void ScrollView::set_scrollbar(bool value) {
		_scrollbar = value;
		_host->mark(0);
	}

	void ScrollView::set_resistance(float value) {
		_resistance = Qk_Max(kScrollMinimumPhysicsScale, value);
	}

	void ScrollView::set_bounce_resistance(float value) {
		_bounce_resistance = Qk_Max(kScrollMinimumPhysicsScale, value);
	}

	void ScrollView::set_bounce_stiffness(float value) {
		_bounce_stiffness = Qk_Max(kScrollMinimumPhysicsScale, value);
	}

	void ScrollView::set_bounce_damping(float value) {
		_bounce_damping = Qk_Max(kScrollMinimumPhysicsScale, value);
	}

	void ScrollView::set_momentum_velocity(float value) {
		_momentum_velocity = Qk_Max(kScrollMinimumPhysicsScale, value);
	}

	void ScrollView::set_bounce(bool value) {
		_bounce = value;
	}

	void ScrollView::set_bounce_lock(bool value) {
		_bounce_lock = value;
	}

	void ScrollView::set_momentum(bool value) {
		_momentum = value;
	}

	void ScrollView::set_lock_direction(bool value) {
		_lock_direction = value;
	}

	void ScrollView::set_catch_position_x(float value) {
		_catch_position_x = value;
	}

	void ScrollView::set_catch_position_y(float value) {
		_catch_position_y = value;
	}

	void ScrollView::set_scrollbar_color(Color value) {
		_host->mark_style_flag(kSCROLLBAR_COLOR_CssProp);
		_async_call({ self->set_scrollbar_color_direct(arg, true); }, value);
	}

	void ScrollView::set_scrollbar_width(float value) {
		_host->mark_style_flag(kSCROLLBAR_WIDTH_CssProp);
		_async_call({ self->set_scrollbar_width_direct(arg, true); }, value);
	}

	void ScrollView::set_scrollbar_margin(float value) {
		_host->mark_style_flag(kSCROLLBAR_MARGIN_CssProp);
		_async_call({ self->set_scrollbar_margin_direct(arg, true); }, value);
	}

	void ScrollView::set_scrollbar_color_direct(Color value, bool isRT) {
		_scrollbar_color = value;
		_host->mark_rerender();
	}

	void ScrollView::set_scrollbar_width_direct(float value, bool isRT) {
		_scrollbar_width = F32::max(1.0, value);
		_host->mark_rerender();
	}

	void ScrollView::set_scrollbar_margin_direct(float value, bool isRT) {
		_scrollbar_margin = F32::max(1.0, value);
		_host->mark_rerender();
	}

	void ScrollView::set_scroll_duration(uint64_t value) {
		_scroll_duration = value;
	}

	cCurve& ScrollView::default_curve() const {
		return _default_curve_Wt;
	}

	void ScrollView::set_default_curve(cCurve& value) {
		_default_curve_Wt = value;
	}

	void ScrollView::terminate() {
		_async_call({
			self->termination_recovery(0);
		}, 0);
	}

	float ScrollView::scroll_left() const {
		return -_scroll.load().x();
	}

	float ScrollView::scroll_top() const {
		return -_scroll.load().y();
	}

	void ScrollView::set_scroll_left(float value) {
		set_scroll({value, -_scroll.load().y()});
	}

	void ScrollView::set_scroll_top(float value) {
		set_scroll({-_scroll.load().x(), value});
	}

	Vec2 ScrollView::scroll() const {
		Vec2 v = _scroll;
		return {-v[0],-v[1]};
	}

	void ScrollView::set_scroll(Vec2 value) {
		if (_scroll_duration) {
			scrollTo(value, _scroll_duration, _default_curve_Wt);
		} else {
			value = _this->get_valid_scroll(-value[0], -value[1]);
			if (value != _scroll) {
				_scroll = value;
				_async_call({
					self->set_scroll_and_trigger_event(
						self->get_catch_valid_scroll({arg.x(), arg.y()}),
						true
					);
				}, value);
			}
		}
	}

	void ScrollView::set_scroll_rt(Vec2 value) {
		_this->set_scroll_and_trigger_event(_this->get_catch_valid_scroll({-value.x(), -value.y()}));
	}

	void ScrollView::scrollTo(Vec2 value, uint64_t duration, cCurve& curve) {
		struct Args { Vec2 value; uint64_t duration; Curve curve; };
		_async_call({
			Vec2 v = self->get_catch_valid_scroll({-arg->value.x(), -arg->value.y()});
			if ( v != self->_scroll ) {
				self->scroll_to_valid_scroll(v, arg->duration, arg->curve);
			}
			delete arg;
		}, new Args({value,duration,curve}));
	}

	void ScrollView::solve(const Mat &mat, View *parent, uint32_t mark) {
		if ( mark & View::kScroll ) {
			if ( !_moved && !_this->is_task() ) {
				// fix scroll position and catch position and trigger event
				_this->set_scroll_and_trigger_event(_this->get_catch_valid_scroll(_scroll));
			}
			_host->unmark(View::kScroll);
		}

		if (mark & View::kTransform) {
			auto v = parent->layout_offset_inside();
			v += _host->layout_offset();
			v += Vec2(_host->margin_left(), _host->margin_top());
			v -= Vec2(scroll_left(), scroll_top());
			auto scrollPos =
				mat.mul_vec2_no_translate(v) + parent->position();
			_scrollMatrix = Mat(mat).set_translate(scrollPos);
		}
	}

	void ScrollView::set_scroll_size_rt(Vec2 size) {
		if (_scroll_size != size) {
			_scroll_size = size;
			_this->immediate_end_all_task(); // change size immediate task
		}
		auto cSize = _host->content_size();
		_scroll_max = Vec2(F32::min(cSize.x() - size.x(), 0), F32::min(cSize.y() - size.y(), 0));

		_scroll_h = _scroll_max.x() < 0;
		_scroll_v = ((!_bounce_lock && !_scroll_h) || _scroll_max.y() < 0);

		_scrollbar_h = (_scroll_h && _scrollbar);
		_scrollbar_v = (_scroll_v && _scrollbar && _scroll_max.y() < 0);

		_host->mark<true>(kScrollMark);
	}

	// ------------------------ S c r o l l . L a y o u t --------------------------

	Scroll::Scroll(): ScrollView(this)
	{
	}

	View* Scroll::init(Window *win) {
		View::init(win);
		set_clip(true);
		set_receive(true);
		return this;
	}

	Vec2 Scroll::layout_offset_inside() {
		return Box::layout_offset_inside() - Vec2(scroll_left(), scroll_top());
	}

	void Scroll::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			set_scroll_size_rt(layout_typesetting_float());
		}
	}

	void Scroll::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		ScrollView::solve(mat, parent, mark);
		Box::solve_marks(mat, parent, mark);
	}

	ScrollView* Scroll::asScrollView() {
		return this;
	}

	ViewType Scroll::view_type() const {
		return kScroll_ViewType;
	}

}
