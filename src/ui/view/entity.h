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
	 * @brief Entity is inherited from View and MorphView.
	 * 2d entity in the world, supports matrix transformation and bounding box calculation.
	 * Entities can participate in hit testing and rendering.
	*/
	class Qk_EXPORT Entity: public View, public MorphView {
	public:
		enum BoundsType { kDefault, kLineSegment, kCircle, kPolygon };
		struct Bounds {
			BoundsType type;
			union { float radius, halfThickness; }; // for circle or line segment
			Qk_DEFINE_PROP_GET_Atomic(cArray<Vec2>*, pts, Const); // polygon or line segment pts
			Bounds();
			Bounds(float radius); // for circle
			Bounds(cArray<Vec2>& pts, float halfThickness = 0.0f); // for polygon or line segment
			Bounds(const Bounds& bounds);
			~Bounds();
			friend class Entity;
		};
		/** The bounds of the entity */
		Qk_DEFINE_PROPERTY(Bounds, bounds, Const);

		Entity();
		virtual ViewType viewType() const override;
		virtual Entity* asEntity() override;
		virtual MorphView* asMorphView() override;
		virtual Vec2 layout_offset_inside() override;
		virtual void solve_marks(const Mat &mat, View *parent, uint32_t mark) override;
		virtual void solve_visible_area(const Mat &mat) override;
		virtual bool overlap_test(Vec2 point) override; // Check if the point overlaps with the sprite
		virtual void layout_reverse(uint32_t mark) override;
		virtual void draw(Painter *render) override;
		virtual Vec2 client_size() override;
		virtual Region client_region() override;
		virtual void trigger_listener_change(uint64_t name, int count, int change) override;
	private:
		void debugDraw(Painter *render); // draw debug bounds
		Vec2 solve_origin_value(Vec2 from); // compute origin value from parameter
		void solve_bounds(); // compute bounds pts and circle
		cArray<Vec2>& ptsOfBounds(); // get bounds pts for precise test
		Array<Vec2> *_ptsBounds; // The final polygon bounds or line segment for precise test
		Circle _circleBounds; // The final circle bounds for quick test

		friend class World;
		friend class Sprite;
		friend class Spine;
		friend class Agent;
	};

	/**
	 * @class Agent
	 * @brief Agent is inherited from Entity.
	 * An entity with agent properties for navigation and avoidance.
	 * abstract class, use Entity::asAgent() to get Agent pointer.
	*/
	class Qk_EXPORT Agent: public Entity {
	public:
		/** is active */
		Qk_DEFINE_PROPERTY(bool, active, Const);
		/** is following target */
		Qk_DEFINE_PROP_GET(bool, following, Const);
		/** has waypoints, priority comes second */
		Qk_DEFINE_ACCE_GET(bool, isWaypoints, Const);
		/** target position */
		Qk_DEFINE_PROP_GET(Vec2, target, Const);
		/** current velocity vector */
		Qk_DEFINE_PROP_GET(Vec2, velocity, Const);
		/** max velocity, default is 100.0f */
		Qk_DEFINE_PROPERTY(float, velocityMax, Const);
		/** current waypoint index */
		Qk_DEFINE_PROP_GET(uint32_t, currentWaypoint, Const);
		/** discovery distances levels squared, default is empty */
		Qk_DEFINE_ACCESSOR(cArray<float>&, discoveryDistancesSq, Const);
		/** safety buffer distance for avoidance, default is 5 units, range [0.0, 100.0] */
		Qk_DEFINE_PROPERTY(float, safetyBuffer, Const);
		/**
		 * follow target agent and follow distance is 0.0f, default is nullptr,
		 * priority is higher than waypoints and moveTo().
		 */
		Qk_DEFINE_ACCESSOR(Agent*, followTarget, Const);
		/** follow distance range, minimum and maximum values, default is [0.0f, 0.0f] */
		Qk_DEFINE_PROPERTY(Vec2, followDistanceRange, Const);

		/**
		 * Set discovery distances levels with not squared distances.
		*/
		void setDiscoveryDistances(cArray<float>& val);

		/**
		 * Destructor
		*/
		~Agent();

		/**
		 * @override
		*/
		Agent* asAgent() override;

		/**
		 * Clear waypoints and set target position for agent and set active to true
		 * @param target {Vec2} target position
		 * @param immediately {bool} immediately move to target position
		 */
		void moveTo(Vec2 target, bool immediately = false);

		/**
		 * set waypoints for agent, and move to closest waypoint, need at least 2 waypoints
		 * @param waypoints {Path*} waypoints for agent
		 * @param immediately {bool} immediately move to closest waypoint
		 */
		void setWaypoints(const Path& waypoints, bool immediately = false);

		/**
		 * set waypoints for agent, and move to closest waypoint, need at least 2 waypoints
		 * @param waypoints {cArray<Vec2>*} waypoints for agent
		 * @param immediately {bool} immediately move to closest waypoint
		 */
		void setWaypoints(cArray<Vec2>& waypoints, bool immediately = false);

		/**
		 * Return to the waypoints closest path from current position
		 * @param immediately {bool} immediately return to closest waypoints
		*/
		void returnToWaypoints(bool immediately = false);

		/**
		 * only set active to false, stop moving the agent
		*/
		void stop();

		/**
		 * @override
		*/
		void onActivate() override;

	protected:
		Agent(); // protected constructor
	private:
		Dict<Agent*, uint32_t> _discoverys_rt; // discovery agents set, Agent* -> dd level index
		std::atomic<Array<float>*> _discoveryDistancesSq; // discovery distances levels squared
		std::atomic<Array<Vec2>*> _waypoints; // waypoints for agent
		std::atomic<Agent*> _followTarget; // follow target agent, weak reference
		friend class World;
	};

	typedef Entity::Bounds EntityBounds; // typedef entity bounds
}
#endif
