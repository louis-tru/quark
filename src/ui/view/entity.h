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

#ifndef __quark__view__entity__
#define __quark__view__entity__

#include "./morph.h"
#include "../geometry.h"
#include "../../render/path.h"

namespace qk {
	class Agent;

	/**
	 * @class AgentStateChangeEvent
	 * @brief Event triggered when an Agent's state changes (e.g., velocity or following status).
	 */
	class Qk_EXPORT AgentStateEvent: public UIEvent {
	public:
		AgentStateEvent(Agent *origin);
		Qk_DEFINE_PROP_GET(Vec2, velocity, Const);  ///< Current velocity vector.
		Qk_DEFINE_PROP_GET(Vec2, heading, Const);  ///< Current heading vector of target or waypoints.
		Qk_DEFINE_PROP_GET(Vec2, target, Const);  ///< Current target position.
		Qk_DEFINE_PROP_GET(bool, moving, Const);  ///< Moving state.
	};

	/**
	 * @class ReachWaypointEvent
	 * @brief Event triggered when an Agent reaches a waypoint in its path.
	 */
	class Qk_EXPORT ReachWaypointEvent: public AgentStateEvent {
	public:
		ReachWaypointEvent(Agent *origin, Vec2 toNext, uint32_t waypoint_index);
		Qk_DEFINE_PROP_GET(Vec2, toNext, Const);   ///< current waypoint to next vector.
		Qk_DEFINE_PROP_GET(uint32_t, waypointIndex, Const); ///< Index of current waypoint.
	};

	/**
	 * @class AgentMovementEvent
	 * @brief Event triggered when an Agent starts, stops, or cancels movement.
	*/
	class Qk_EXPORT AgentMovementEvent: public AgentStateEvent {
	public:
		enum MovementState {
			Started = 1,  ///< Movement started
			Arrived,      ///< Arrived at target or waypoint
			Stopped,      ///< Arrived or naturally stopped
			Cancelled     ///< Interrupted (follow lost, new target etc.)
		};
		AgentMovementEvent(Agent *origin, MovementState state);
		Qk_DEFINE_PROP_GET(MovementState, movementState, Const);  ///< Movement state.
	};

	/**
	 * @class DiscoveryAgentEvent
	 * @brief Event triggered when one Agent discovers or loses another within a detection range.
	 */
	class Qk_EXPORT DiscoveryAgentEvent: public AgentStateEvent {
	public:
		/**
		 * @param origin Origin agent that detects the other.
		 * @param agent  The discovered or lost agent.
		 * @param location The relative location to the other agent.
		 * @param level Discovery level (range band index), 0xffffffff indicates completely lost.
		 * @param entering True if entering range, false if leaving.
		 */
		DiscoveryAgentEvent(Agent *origin, Agent* agent, Vec2 location, uint32_t level, bool entering);
		Qk_DEFINE_PROP_GET(Agent*, agent);     ///< The other discovered agent. is nullptr when remove from parent
		Qk_DEFINE_PROP_GET(Vec2, location, Const); ///< Relative position to the other agent.
		Qk_DEFINE_PROP_GET(uint32_t, level, Const); ///< Discovery level index.
		Qk_DEFINE_PROP_GET(bool, entering, Const); ///< True if entering range; false if leaving.

		void release() override; ///< Custom release logic.
	};

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
			kCircle,       ///< Represented as a circle
			kPolygon,      ///< Represented as a polygon
			kLineSegment,  ///< Represented as a line segment (with half thickness)
		};

		/**
		 * @struct Bounds
		 * @brief Stores geometric boundary data of an entity.
		 */
		struct Bounds {
			BoundsType type; ///< The type of boundary used for hit testing.
			Vec2 offset; ///< Offset applied to the bounds.
			union { 
				float radius;        ///< Used if type == kCircle
				float halfThickness; ///< Used if type == kLineSegment
			};
			std::atomic<Path*> pts; ///< Pointer to polygon or line-segment points.
		};

		/** Accessor for the entity’s bounding data. */
		Qk_DEFINE_ACCESSOR(const Bounds&, bounds, Const);

		/**
		 * Whether this entity participates in the world as a detectable/collidable object.
		 * 
		 * If false, other agents will ignore this entity (no collision, no detection),
		 * but this entity may still actively collide or detect others if it is an Agent.
		 *
		 * Default: true
		 */
		Qk_DEFINE_ACCESSOR(bool, participate, Const);

		Entity();
		~Entity();

		/** @override Return view type identifier. */
		ViewType view_type() const override;
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

		/** @override Return layout container for this entity. */
		const View::Container& layout_container() override;

		/** @override Return client area size in local coordinates. */
		Vec2 client_size() override;

		/** @override Return client area as region (used for hit testing). */
		Region client_region() override;

		/** @override Trigger listener count changes for this entity. */
		void trigger_listener_change(uint64_t name, int count, int change) override;

	private:
		bool test_entity_vs_entity(Entity *other, MTV *outMTV, Vec2 *outMtvVec, bool computeMTV);

		/** Draw debug outlines for development visualization. */
		void debugDraw(Painter *painter);

		/** Compute origin offset from user parameters. */
		Vec2 compute_origin_value(Vec2 from);

		/** Compute and cache geometric bounds (polygon or circle). */
		void solve_bounds();

		Bounds _bounds; ///< Entity’s local bounds description.

		/** Return a reference to internal bounds vertices for precise testing. */
		Array<Vec2>& ptsOfBounds();

		/** Cached polygonal bounds or line segment points. */
		Sp<Array<Vec2>> _ptsBounds;

		/** Cached circular bounds for quick rejection test. */
		Circle _circleBounds;

		/*
		 * Whether this entity participates in the world as a detectable/collidable object.
		*/
		bool _participate;

		friend class World;
		friend class Sprite;
		friend class Spine;
		friend class Agent;
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
		/**
		 * Whether the agent is currently active (moving or processing behavior). 
		 * Default: true
		 */
		Qk_DEFINE_PROPERTY(bool, active, Const);

		/**
		 * Whether the agent is currently moving toward a target or along waypoints or following another agent.
		*/
		Qk_DEFINE_PROP_GET(bool, moving, Const);

		/**
		 * Indicates if the agent’s standing position is floating (soft).
		 * 
		 * When enabled, the agent does not defend its current spot after arrival
		 * and can be displaced by nearby agents—useful for group formations or
		 * crowding around large targets.
		 * Default: false
		 */
		Qk_DEFINE_PROPERTY(bool, floatingStation, Const);

		/** Waypoints path for the agent to navigate. */
		Qk_DEFINE_ACCESSOR(Path*, waypoints, Const);

		/** Target position (for direct movement). */
		Qk_DEFINE_PROP_GET(Vec2, target, Const);

		/** Current avoidance velocity steering vector in world coordinates. */
		Qk_DEFINE_PROP_GET(Vec2, velocitySteer, Const);

		/** Current real velocity vector in world coordinates. */
		Qk_DEFINE_PROP_GET(Vec2, velocity, Const);

		/**
		 * Behavior heading direction (normalized).
		 * The agent's intended movement direction based on path/follow logic.
		 * Not necessarily equal to velocity direction and does not jitter.
		 */
		Qk_DEFINE_PROP_GET(Vec2, heading, Const);

		/** Maximum movement velocity. Default is 100.0f. */
		Qk_DEFINE_PROPERTY(float, velocityMax, Const);

		/** Current waypoint index when navigating along a path. */
		Qk_DEFINE_PROP_GET(uint32_t, currentWaypoint, Const);

		/** Accessor for discovery distance levels. Used for agent detection. */
		Qk_DEFINE_ACCESSOR(cArray<float>&, discoveryDistances, Const);

		/** Safety buffer for collision avoidance. Default 5.0f, range [0, 100]. */
		Qk_DEFINE_PROPERTY(float, safetyBuffer, Const);

		/**
		 * Avoidance strength multiplier during collision resolution.
		 * Default is 0.5f, recommended range [0.0, 1.0].
		*/
		Qk_DEFINE_PROPERTY(float, avoidanceFactor, Const);

		/**
		 * Maximum avoidance velocity applied when steering away from obstacles.
		 * Default is 1.0f, range [0.0, 3.0], recommended range [0.0, 1.0].
		*/
		Qk_DEFINE_PROPERTY(float, avoidanceVelocityFactor, Const);

		/**
		 * Distance min range maintained while following a target agent.
		*/
		Qk_DEFINE_PROPERTY(float, followMinDistance, Const);

		/**
		 * Distance max range maintained while following a target agent.
		*/
		Qk_DEFINE_PROPERTY(float, followMaxDistance, Const);

		/**
		 * The target agent to follow.  
		 * If non-null, follow behavior overrides waypoint or direct movement.
		 */
		Qk_DEFINE_ACCESSOR(Agent*, followTarget, Const);

		/** @override Destructor. */
		~Agent();

		/** @override Cast to Agent. */
		Agent* asAgent() override;

		/**
		 * Move agent toward a target position.
		 * Delete follow target if any.
		 * @param target Destination position.
		 * @param immediately If true, teleport or instantly set to target.
		 */
		void moveTo(Vec2 target, bool immediately = false);

		/**
		 * Assign waypoints for agent to traverse.
		 * Delete follow target if any.
		 * @param waypoints Path containing waypoint positions.
		 * @param immediately If true, move to nearest waypoint immediately.
		 */
		void setWaypoints(Path* waypoints, bool immediately = false);

		/**
		 * Assign waypoints using a raw array of positions.
		 * Delete follow target if any.
		 * @param waypoints List of 2D coordinates.
		 * @param immediately If true, move to nearest waypoint immediately.
		 */
		void setWaypoints(cArray<Vec2>& waypoints, bool immediately = false);

		/**
		 * Return to the closest path segment among current waypoints.
		 * Delete follow target if any.
		 * @param immediately Whether to jump to the path instantly.
		 */
		void returnToWaypoints(bool immediately = false);

		/**
		 * clear current movement (waypoints or follow target).
		 * and set target to current position.
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
		void reportDirectionChange(Vec2 dir);
		bool isSpeedBelowRatio(float ratio) const;
		Dict<Agent*, uint32_t> _discoverys_rt; ///< Runtime map of discovered agents → level index.
		std::atomic<Array<float>*> _discoveryDistances; ///< Discovery range levels.
		std::atomic<Path*> _waypoints; ///< Active waypoint path.
		std::atomic<Agent*> _followTarget; ///< Current follow target (weak reference).

		friend class World;
	};

	typedef Entity::Bounds Bounds; // typedef entity bounds
}
#endif
