/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
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

#ifndef __quark__view__scroll__
#define __quark__view__scroll__

#include "./box.h"
#include "../../render/bezier.h"

namespace qk {
	/**
	 * Scroll view support class
	 *
	 * A physics-driven scroll controller that manages scrolling state,
	 * momentum, bounce, direction lock and scrollbar visualization.
	 *
	 * This class does NOT depend on the event system directly. Input can be
	 * injected manually via begin_drag/drag/end_drag/wheel, making it usable
	 * as a pure scrolling core for complex components (e.g. editors).
	 *
	 * ScrollView itself does not render. It operates on a host Box and is
	 * used internally by the Scroll view.
	 */
	class Qk_EXPORT ScrollView {
	public:
		/** Enable/disable scrollbars (both directions) */
		Qk_DEFINE_PROPERTY(bool, scrollbar, Const);

		/** Enable edge bounce effect when reaching scroll limits */
		Qk_DEFINE_PROPERTY(bool, bounce, Const);

		/**
		 * Lock bounce direction once engaged.
		 * Prevents oscillation between axes when hitting corners.
		 */
		Qk_DEFINE_PROPERTY(bool, bounce_lock, Const);

		/** Enable inertial scrolling after drag release */
		Qk_DEFINE_PROPERTY(bool, momentum, Const);

		/**
		 * Lock scrolling direction once movement exceeds threshold.
		 * Prevents diagonal jitter when user intends single-axis scroll.
		 */
		Qk_DEFINE_PROPERTY(bool, lock_direction, Const);

		/** Whether horizontal scrollbar is currently active/visible */
		Qk_DEFINE_PROP_GET(bool, scrollbar_h, Const);

		/** Whether vertical scrollbar is currently active/visible */
		Qk_DEFINE_PROP_GET(bool, scrollbar_v, Const);

		/** Current horizontal scroll offset (pixels) */
		Qk_DEFINE_ACCESSOR(float, scroll_left, Const);

		/** Current vertical scroll offset (pixels) */
		Qk_DEFINE_ACCESSOR(float, scroll_top, Const);

		/** Current scroll offset vector */
		Qk_DEFINE_ACCESSOR(Vec2, scroll, Const);

		/**
		 * Scrollable content size.
		 * Represents the maximum scroll range derived from layout.
		 */
		Qk_DEFINE_PROP_GET(Vec2, scroll_size, Const);

		/**
		 * Drag resistance factor.
		 * Higher values make movement feel "heavier".
		 * Default = 1.
		 */
		Qk_DEFINE_PROPERTY(float, resistance, Const);

		/**
		 * Snap step on X axis when scrolling stops.
		 *
		 * If > 0, the final scroll position will be snapped to the nearest
		 * multiple of this value.
		 *
		 * Example:
		 *   catch_position_x = 10
		 *   → scrollLeft will settle at 0, 10, 20, 30, ...
		 *
		 * Set to 0 to disable snapping.
		 */
		Qk_DEFINE_PROPERTY(float, catch_position_x, Const);

		/**
		 * Snap step on Y axis when scrolling stops.
		 *
		 * Works the same as catch_position_x but for vertical scrolling.
		 */
		Qk_DEFINE_PROPERTY(float, catch_position_y, Const);

		/** Scrollbar color */
		Qk_DEFINE_VIEW_PROPERTY(Color, scrollbar_color, Const);

		/** Scrollbar thickness in pixels */
		Qk_DEFINE_VIEW_PROPERTY(float, scrollbar_width, Const);

		/** Margin between scrollbar and edge */
		Qk_DEFINE_VIEW_PROPERTY(float, scrollbar_margin, Const);

		/**
		 * Default programmatic scroll duration (ms).
		 * Used by scrollTo() when duration is not specified externally.
		 */
		Qk_DEFINE_PROPERTY(uint64_t, scroll_duration, Const);

		/**
		 * Default animation curve used by scrollTo().
		 */
		Qk_DEFINE_ACCESSOR(cCurve&, default_curve, Const);

		/**
		 * Host box that provides layout size and acts as scroll container.
		 * Scroll offsets are applied relative to this box.
		 */
		Qk_DEFINE_PROP_GET(Box*, host);

		// ------------------------------------------------------------------
		// Core control methods
		// ------------------------------------------------------------------

		/**
		 * Scroll to a target position using animation.
		 *
		 * @param value    Target scroll offset
		 * @param duration Animation duration (ms)
		 * @param curve    Easing curve
		 */
		void scrollTo(Vec2 value, uint64_t duration, cCurve& curve);

		/**
		 * Scroll using default curve.
		 */
		void scrollTo(Vec2 value, uint64_t duration) {
			scrollTo(value, duration, _default_curve_Wt);
		}

		/**
		 * Immediately stop all ongoing scroll animations and momentum.
		 */
		void terminate();

		// ------------------------------------------------------------------
		// External input injection (physics entry points)
		// ------------------------------------------------------------------

		/**
		 * Begin drag interaction at given absolute position.
		 * Starts tracking movement and prepares velocity calculation.
		 */
		void begin_drag(Vec2 pos);

		/**
		 * Continue drag movement.
		 * Position should be continuous absolute coordinates.
		 */
		void drag(Vec2 pos);

		/**
		 * End drag interaction.
		 * Velocity will be calculated and momentum may start.
		 */
		void end_drag(Vec2 pos);

		/**
		 * Wheel / trackpad scroll input.
		 *
		 * @param delta Scroll delta in pixels
		 */
		void wheel(Vec2 delta);

	protected:
		ScrollView(Box *host);
		~ScrollView();

		/** Layout solve entry (render thread) */
		void solve(uint32_t mark); // @thread Rt

		/** Update scrollable size (render thread) */
		void set_scroll_size_rt(Vec2 size); // @thread Rt

		/** Apply scroll value (render thread) */
		void set_scroll_rt(Vec2 value); // @thread Rt

	private:
		Qk_DEFINE_INLINE_CLASS(Inl);
		Qk_DEFINE_INLINE_CLASS(Task);
		friend class Painter;

		/** Active animation/momentum tasks */
		List<Task*> _tasks;

		/** Current scroll position (thread-safe) */
		std::atomic<Vec2> _scroll;

		/** Maximum allowed scroll range */
		Vec2 _scroll_max;

		/** Drag start state */
		Vec2 _move_start_scroll, _move_point, _move_dist;

		/** Scrollbar visual positions */
		Vec2 _scrollbar_position_h, _scrollbar_position_v;

		/** Drag start timestamp */
		uint64_t _move_start_time;

		/** Current animation/action id */
		uint32_t _action_id;

		/** Default animation curve instance */
		Curve _default_curve_Wt;

		/** Scrollbar opacity animation state */
		float _scrollbar_opacity;

		/** Currently being moved by external force (dragging) */
		bool _moved;

		/** Axis activation state */
		bool _scroll_h, _scroll_v;

		/** Direction lock flags */
		bool _lock_h, _lock_v;
	};

	/**
	 * Scroll view
	 *
	 * A concrete View that embeds ScrollView behavior.
	 *
	 * Responsibilities:
	 * - Acts as layout container
	 * - Applies scroll offset to children
	 * - Renders scrollbars
	 * - Bridges UI lifecycle with ScrollView physics
	 */
	class Qk_EXPORT Scroll: public Box, public ScrollView {
	public:
		Scroll();

		/** Initialize view with window context */
		virtual View* init(Window *win) override;

		/** Return view type identifier */
		virtual ViewType view_type() const override;

		/** Cast helper */
		virtual ScrollView* asScrollView() override;

		/** Offset applied to child layout based on scroll position */
		virtual Vec2 layout_offset_inside() override;

		/** Layout reverse pass */
		virtual void layout_reverse(uint32_t mark) override;

		/** Layout solving entry */
		virtual void solve_marks(const Mat &mat, View *parent, uint32_t mark) override;

		/** Render scrollbars and content */
		virtual void draw(Painter *render) override;
	};

}
#endif
