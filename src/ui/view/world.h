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
	 * @brief A 2D basic world view container.
	*/
	class Qk_EXPORT World: public Morph, public PreRender::Task {
	public:
		/** is active update world physics simulation, default is false */
		Qk_DEFINE_PROPERTY(bool, playing, Const);
	 	/** The number of sub-steps for each update, default is 1, suggested value is 1~5 */
		Qk_DEFINE_PROPERTY(int, subSteps, Const);
		/** time scale factor, default is 1.0f, range [0.0, 10.0] */
		Qk_DEFINE_PROPERTY(float, timeScale, Const);
		/** prediction time for avoidance, default is 0.1 second, range [0.05, 2.0] */
		Qk_DEFINE_PROPERTY(float, predictionTime, Const);
		/** max avoidance force factor, default is 0.8 units, range [0.0, 1.0] */
		Qk_DEFINE_PROPERTY(float, avoidanceFactor, Const);
		/** buffer distance for discovery threshold, default is 5.0f, range [0.0, 100.0] */
		Qk_DEFINE_PROPERTY(float, discoveryThresholdBuffer, Const);

		World();
		ViewType viewType() const override;
		void onActivate() override;
		void onChildLayoutChange(View* child, uint32_t mark) override;
		bool run_task(int64_t time, int64_t delta) override;
	private:
		void handleDiscoveryEvents(Agent* agent, Agent* other, MTV mtv, float bufferSq);
		Vec2 computeAvoidanceForAgent(Agent* agent, cArray<Entity*>& obs, Circle circ, Array<Vec2>& pts, Vec2 dirToTarget);
		float updateAgentWithAvoidance(Agent* agent, cArray<Entity*>& obs, float deltaTime);
		void updateAgentWithMovement(Agent* agent, cArray<Entity*>& obs, float deltaTime, bool &update);
		void updateAgentWithFollow(Agent* agent, cArray<Entity*>& obs, float deltaTime, bool &update);
	};

} // namespace qk


#endif // __quark__view__world__
