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

#ifndef __quark__view__world__
#define __quark__view__world__

#include "./morph.h"
#include "./entity.h"

namespace qk {

	/**
	 * @class World
	 * @brief A 2D physical world container for agents and entities.
	 *
	 * `World` is a specialized `Morph` view that maintains and updates a set of
	 * `Entity` and `Agent` objects under a common coordinate system.
	 * It acts as the simulation root for navigation, discovery, and avoidance logic.
	 *
	 * Features:
	 * - Provides time-stepped updates (with sub-steps) for stable agent movement.
	 * - Handles discovery events between agents based on distance thresholds.
	 * - Computes local avoidance using predictive collision detection.
	 * - Integrates with the pre-render task system (`PreRender::Task`) for update scheduling.
	 *
	 * The `playing` flag controls whether the simulation is active.  
	 * When stopped, agents remain visible but do not move or interact.
	 */
	class Qk_EXPORT World: public Morph, public PreRender::Task {
	public:
		/** Whether the world is actively updating physics and agent movement. Default: false. */
		Qk_DEFINE_PROPERTY(bool, playing, Const);

		/** 
		 * The number of internal simulation sub-steps per frame.
		 * Higher values increase stability for fast-moving agents.
		 * Suggested range: 1–5. Default: 1.
		 */
		Qk_DEFINE_PROPERTY(int, subSteps, Const);

		/** 
		 * Time scale multiplier.  
		 * 1.0 = real-time speed, <1 = slow motion, >1 = fast forward.
		 * Range: [0.0, 10.0].
		 */
		Qk_DEFINE_PROPERTY(float, timeScale, Const);

		/** 
		 * Prediction horizon (seconds) used for avoidance calculations.  
		 * Agents will predict future positions within this window to plan separation.
		 * Default: 0.1s, Range: [0.05, 2.0].
		 */
		Qk_DEFINE_PROPERTY(float, predictionTime, Const);

		/** 
		 * Maximum avoidance steering factor.  
		 * Defines how strongly agents deviate to avoid collisions.  
		 * Default: 0.8, Range: [0.0, 1.0].
		 */
		Qk_DEFINE_PROPERTY(float, avoidanceFactor, Const);

		/**
		 * Buffer distance added to discovery thresholds to prevent flickering
		 * enter/leave events.  
		 * Default: 5.0f, Range: [0.0, 100.0].
		 */
		Qk_DEFINE_PROPERTY(float, discoveryThresholdBuffer, Const);

		/** Constructor — initializes default parameters and world state. */
		World();

		/** @override Return the view type identifier. */
		ViewType viewType() const override;

		/**
		 * @override
		 * Called by the View layout system when the world is activated or deactivated.  
		 * Used to start or stop physics scheduling in the PreRender system.
		 */
		void onActivate() override;

		/**
		 * @override
		 * Invoked when a child view (typically an `Agent`) is removed, replaced, or has
		 * a layout transform change that affects its lifecycle within the World.
		 *
		 * This method is primarily used for **cleanup**:
		 * - Removes any references in `Agent::_discoverys_rt` related to the child agent.
		 * - Clears follow-target links if the removed child was being followed.
		 * - Emits the corresponding cleanup events:
		 *   - `FollowTargetEvent::kCancel` when the follow target is lost.
		 *   - `DiscoveryAgentEvent` with `level = -1` when an agent completely loses sight of another.
		 *
		 * In short, this keeps the World’s runtime relationship graph safe and consistent
		 * when children (Agents or Entities) are detached or destroyed.
		 *
		 * @param child The child view whose layout or existence changed.
		 * @param mark  Change flags describing the modification.
		 */
		void onChildLayoutChange(View* child, uint32_t mark) override;

		/**
		 * @override
		 * Main update loop executed as part of the PreRender task system.  
		 * Performs physics integration, discovery tests, and movement updates.
		 * @param time  Current absolute time (ns).
		 * @param delta Time elapsed since last frame (ns).
		 * @return true if the world requires continued updates; false to pause.
		 */
		bool run_task(int64_t time, int64_t delta) override;

	private:
		/**
		 * Handle discovery and visibility events between two agents.
		 * @param agent The source agent performing discovery.
		 * @param other The other agent being tested.
		 * @param mtv   Minimum translation vector from last overlap check.
		 * @param bufferSq Squared buffer distance to avoid event flicker.
		 */
		void handleDiscoveryEvents(Agent* agent, Agent* other, MTV mtv, float bufferSq);

		/**
		 * Compute avoidance steering direction for a given agent.
		 * @param agent The agent to update.
		 * @param obs List of nearby obstacles (entities or agents).
		 * @param circ The agent’s collision circle.
		 * @param pts  Temporary buffer for intersection test points.
		 * @param dirToTarget Current movement direction toward target.
		 * @return Steering vector to apply for avoidance.
		 */
		Vec2 computeAvoidanceForAgent(Agent* agent, cArray<Entity*>& obs, Circle circ,
																	Array<Vec2>& pts, Vec2 dirToTarget);

		/**
		 * Integrate agent movement with avoidance applied.
		 * @param agent The agent to update.
		 * @param obs Nearby obstacles.
		 * @param deltaTime Delta time for this step (seconds).
		 * @return Updated remaining distance to target.
		 */
		float updateAgentWithAvoidance(Agent* agent, cArray<Entity*>& obs, float deltaTime);

		/**
		 * Process standard movement and waypoint following.
		 * @param agent The agent to update.
		 * @param obs Nearby obstacles.
		 * @param deltaTime Delta time for this step.
		 * @param update Output flag set to true if agent moved or changed state.
		 */
		void updateAgentWithMovement(Agent* agent, cArray<Entity*>& obs,
																float deltaTime, bool &update);

		/**
		 * Process target-following behavior for agents that track another agent.
		 * @param agent The follower agent.
		 * @param obs Nearby obstacles.
		 * @param deltaTime Delta time for this step.
		 * @param update Output flag set to true if agent moved or changed state.
		 */
		void updateAgentWithFollow(Agent* agent, cArray<Entity*>& obs,
															float deltaTime, bool &update);
	};

} // namespace qk


#endif // __quark__view__world__
