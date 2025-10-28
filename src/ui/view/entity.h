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

#ifndef __quark__view__entity__
#define __quark__view__entity__

#include "./morph.h"
#include "../geometry.h"
#include "../../render/path.h"

namespace qk {
	class Agent;

	/**
	 * @class Entity
	 * @brief Base class for all 2D world entities.
	 * 
	 * `Entity` is derived from both `View` and `MorphView`.  
	 * It represents a drawable and interactive 2D object in the world.  
	 * 
	 * Features:
	 * - Supports matrix-based spatial transformations.
	 * - Maintains its own geometric bounds (circle, line segment, or polygon).
	 * - Can participate in rendering and hit testing.
	 * - Provides automatic visible area and layout resolution.
	 */
	class Qk_EXPORT Entity: public View, public MorphView {
	public:
		enum BoundsType { 
			kDefault,      ///< Default bounds (usually rectangular or automatic)
			kLineSegment,  ///< Represented as a line segment (with half thickness)
			kCircle,       ///< Represented as a circle
			kPolygon       ///< Represented as a polygon
		};

		/**
		 * @struct Bounds
		 * @brief Stores geometric boundary data of an entity.
		 */
		struct Bounds {
			BoundsType type; ///< The type of boundary used for hit testing.
			union { 
				float radius;        ///< Used if type == kCircle
				float halfThickness; ///< Used if type == kLineSegment
			};
			std::atomic<Path*> pts; ///< Pointer to polygon or line-segment points.
		};

		/** Accessor for the entity’s bounding data. */
		Qk_DEFINE_ACCESSOR(const Bounds&, bounds, Const);

		Entity();
		~Entity();

		/** @override Return view type identifier. */
		ViewType viewType() const override;
		/** @override Cast to Entity. */
		Entity* asEntity() override;
		/** @override Cast to MorphView. */
		MorphView* asMorphView() override;

		/** @override Return offset used in layout calculations. */
		Vec2 layout_offset_inside() override;

		/** @override Resolve marks and transform states. */
		void solve_marks(const Mat &mat, View *parent, uint32_t mark) override;

		/** @override Compute visible area for rendering and interaction. */
		void solve_visible_area(const Mat &mat) override;

		/**
		 * @override
		 * Check if a given world-space point overlaps the entity’s bounds.
		 * @param point Point in world coordinates.
		 * @return true if overlaps, false otherwise.
		 */
		bool overlap_test(Vec2 point) override;

		/** @override Perform reverse layout calculations if necessary. */
		void layout_reverse(uint32_t mark) override;

		/** @override Draw the entity using a given Painter. */
		void draw(Painter *render) override;

		/** @override Return client area size in local coordinates. */
		Vec2 client_size() override;

		/** @override Return client area as region (used for hit testing). */
		Region client_region() override;

		/** @override Trigger listener count changes for this entity. */
		void trigger_listener_change(uint64_t name, int count, int change) override;

	private:
		/** Draw debug outlines for development visualization. */
		void debugDraw(Painter *render);

		/** Compute origin offset from user parameters. */
		Vec2 solve_origin_value(Vec2 from);

		/** Compute and cache geometric bounds (polygon or circle). */
		void solve_bounds();

		Bounds _bounds; ///< Entity’s local bounds description.

		/** Return a reference to internal bounds vertices for precise testing. */
		cArray<Vec2>& ptsOfBounds();

		/** Cached polygonal bounds or line segment points. */
		Sp<Array<Vec2>> _ptsBounds;

		/** Cached circular bounds for quick rejection test. */
		Circle _circleBounds;

		friend class World;
		friend class Sprite;
		friend class Spine;
		friend class Agent;
	};

	/**
	 * @class ArrivePositionEvent
	 * @brief Event triggered when an Agent arrives at a waypoint or target position.
	 */
	class Qk_EXPORT ArrivePositionEvent: public UIEvent {
	public:
		ArrivePositionEvent(Agent *origin, Vec2 position, Vec2 nextLocation, uint32_t waypoint_index);

		Qk_DEFINE_PROP_GET(Vec2, position, Const);       ///< Current arrived position.
		Qk_DEFINE_PROP_GET(Vec2, nextLocation, Const);   ///< Vector direction to next waypoint.
		Qk_DEFINE_PROP_GET(uint32_t, waypointIndex, Const); ///< Index of current waypoint.
	};

	/**
	 * @class DiscoveryAgentEvent
	 * @brief Event triggered when one Agent discovers or loses another within a detection range.
	 */
	class Qk_EXPORT DiscoveryAgentEvent: public UIEvent {
	public:
		/**
		 * @param origin Origin agent that detects the other.
		 * @param agent  The discovered or lost agent.
		 * @param location The relative location to the other agent.
		 * @param id Unique identifier for the discovered agent.
		 * @param level Discovery level (range band index), -1 indicates completely lost.
		 * @param entering True if entering range, false if leaving.
		 */
		DiscoveryAgentEvent(Agent *origin, Agent* agent, Vec2 location, uint32_t id, int level, bool entering);

		Qk_DEFINE_PROP_GET(Agent*, agent);     ///< The other discovered agent.
		Qk_DEFINE_PROP_GET(Vec2, location, Const); ///< Relative position to the agent.
		Qk_DEFINE_PROP_GET(uint32_t, agentId, Const); ///< Usually low 32 bits of pointer address.
		Qk_DEFINE_PROP_GET(int, level, Const); ///< Discovery level index.
		Qk_DEFINE_PROP_GET(bool, entering, Const); ///< True if entering range; false if leaving.

		void release() override; ///< Custom release logic.
	};

	/**
	 * @class FollowTargetEvent
	 * @brief Event for target following state transitions.
	 */
	class Qk_EXPORT FollowTargetEvent: public UIEvent {
	public:
		enum State { 
			kStart = 1, ///< Following started.
			kStop,      ///< Following stopped normally.
			kCancel     ///< Following cancelled (e.g., target removed in parent view).
		};

		FollowTargetEvent(Agent *origin, State state);

		Qk_DEFINE_PROP_GET(State, state, Const); ///< Current follow state.
	};

	/**
	 * @class Agent
	 * @brief Abstract class for mobile world entities with navigation, avoidance, and target following.
	 *
	 * `Agent` extends `Entity` and introduces AI movement logic.
	 * Agents can:
	 * - Move toward target positions or follow predefined waypoints.
	 * - Detect and react to nearby agents using discovery levels.
	 * - Follow a specific target agent within a distance range.
	 * - Perform avoidance behaviors and emit events such as ArrivePositionEvent, DiscoveryAgentEvent, etc.
	 */
	class Qk_EXPORT Agent: public Entity {
	public:
		/** Whether the agent is currently active (moving or processing behavior). */
		Qk_DEFINE_PROPERTY(bool, active, Const);

		/** Whether the agent is currently following a target. */
		Qk_DEFINE_PROP_GET(bool, following, Const);

		/** Whether the agent currently has valid waypoints assigned. */
		Qk_DEFINE_ACCE_GET(bool, isWaypoints, Const);

		/** Target position (for direct movement). */
		Qk_DEFINE_PROP_GET(Vec2, target, Const);

		/** Current velocity vector in world coordinates. */
		Qk_DEFINE_PROP_GET(Vec2, velocity, Const);

		/** Maximum movement velocity. Default is 100.0f. */
		Qk_DEFINE_PROPERTY(float, velocityMax, Const);

		/** Current waypoint index when navigating along a path. */
		Qk_DEFINE_PROP_GET(uint32_t, currentWaypoint, Const);

		/** Accessor for discovery distance levels (squared). Used for agent detection. */
		Qk_DEFINE_ACCESSOR(cArray<float>&, discoveryDistancesSq, Const);

		/** Safety buffer for collision avoidance. Default 5.0f, range [0, 100]. */
		Qk_DEFINE_PROPERTY(float, safetyBuffer, Const);

		/**
		 * The target agent to follow.  
		 * If non-null, follow behavior overrides waypoint or direct movement.
		 */
		Qk_DEFINE_ACCESSOR(Agent*, followTarget, Const);

		/** Distance range [min, max] maintained while following a target agent. */
		Qk_DEFINE_PROPERTY(Vec2, followDistanceRange, Const);

		/**
		 * Set discovery distance levels using real (non-squared) values.
		 * @param val Array of distance thresholds (ascending order).
		 */
		void setDiscoveryDistances(cArray<float>& val);

		~Agent();

		/** @override Cast to Agent. */
		Agent* asAgent() override;

		/**
		 * Move agent toward a target position.
		 * @param target Destination position.
		 * @param immediately If true, teleport or instantly set to target.
		 */
		void moveTo(Vec2 target, bool immediately = false);

		/**
		 * Assign waypoints for agent to traverse.
		 * @param waypoints Path containing waypoint positions.
		 * @param immediately If true, move to nearest waypoint immediately.
		 */
		void setWaypoints(Path* waypoints, bool immediately = false);

		/**
		 * Assign waypoints using a raw array of positions.
		 * @param waypoints List of 2D coordinates.
		 * @param immediately If true, move to nearest waypoint immediately.
		 */
		void setWaypoints(cArray<Vec2>& waypoints, bool immediately = false);

		/**
		 * Return to the closest path segment among current waypoints.
		 * @param immediately Whether to jump to the path instantly.
		 */
		void returnToWaypoints(bool immediately = false);

		/**
		 * Stop agent movement (set `active=false`).
		 */
		void stop();

		/**
		 * @override
		 * Called by the View layout system when this view (Agent) is activated
		 * or deactivated in the scene graph.
		 *
		 * Agent overrides this to keep its internal state safe when it is
		 * attached to or removed from a World. In particular, this is where
		 * we clean up or update state that depends on World membership
		 * (such as weak references like followTarget, discovery sets, etc.)
		 * so that they don't become unsafe after removal.
		 *
		 * This is part of the generic View lifecycle; it is not an AI
		 * movement start/stop callback.
		 */
		void onActivate() override;

	protected:
		Agent(); ///< Protected constructor; use derived class instantiation.

	private:
		Dict<Agent*, uint32_t> _discoverys_rt; ///< Runtime map of discovered agents → level index.
		std::atomic<Array<float>*> _discoveryDistancesSq; ///< Squared discovery range levels.
		std::atomic<Path*> _waypoints; ///< Active waypoint path.
		std::atomic<Agent*> _followTarget; ///< Current follow target (weak reference).

		friend class World;
	};

	typedef Entity::Bounds Bounds; // typedef entity bounds
}
#endif
