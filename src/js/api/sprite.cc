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

#include "./ui.h"
#include "../../ui/view/sprite.h"
#include "../../ui/view/world.h"

namespace qk { namespace js {
	const uint64_t kAgent_Typeid(Js_Typeid(Agent));

	class MixEntity: public MixViewObject {
	public:
		typedef Entity Type;
		virtual MorphView* asMorphView() { return self<Entity>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Entity, View, { Js_NewView(Entity); });
			inheritMorphView(cls, worker);
			Js_MixObject_Accessor(Entity, Bounds, bounds, bounds);
			Js_MixObject_Accessor(Entity, bool, participate, participate);
			cls->exports("Entity", exports);
		}
	};

	class MixAgent: public MixViewObject {
	public:
		typedef Agent Type;
		virtual MorphView* asMorphView() { return self<Agent>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Agent, Entity, { Js_Throw("Access forbidden."); });
			Js_MixObject_Accessor(Agent, bool, active, active);
			Js_MixObject_Acce_Get(Agent, bool, moving, moving);
			Js_MixObject_Accessor(Agent, bool, floatingStation, floatingStation);
			Js_MixObject_Accessor(Agent, PathPtr, waypoints, waypoints);
			Js_MixObject_Acce_Get(Agent, Vec2, target, target);
			Js_MixObject_Acce_Get(Agent, Vec2, velocitySteer, velocitySteer);
			Js_MixObject_Acce_Get(Agent, Vec2, velocity, velocity);
			Js_MixObject_Acce_Get(Agent, Vec2, heading, heading);
			Js_MixObject_Accessor(Agent, float, velocityMax, velocityMax);
			Js_MixObject_Acce_Get(Agent, uint32_t, currentWaypoint, currentWaypoint);
			Js_MixObject_Accessor(Agent, ArrayFloat, discoveryDistances, discoveryDistances);
			Js_MixObject_Accessor(Agent, float, safetyBuffer, safetyBuffer);
			Js_MixObject_Accessor(Agent, float, avoidanceFactor, avoidanceFactor);
			Js_MixObject_Accessor(Agent, float, avoidanceVelocityFactor, avoidanceVelocityFactor);
			Js_MixObject_Accessor(Agent, float, followMinDistance, followMinDistance);
			Js_MixObject_Accessor(Agent, float, followMaxDistance, followMaxDistance);

			Js_Class_Accessor(followTarget, { Js_Return( self->followTarget() ); }, {
				if (val->isNull()) {
					self->set_followTarget(nullptr);
				} else {
					if (!worker->instanceOf(val, kAgent_Typeid))
						Js_Throw("Agent.followTarget:Agent");
					self->set_followTarget(MixObject::mix<Agent>(val)->self());
				}
			});

			Js_Class_Method(moveTo, {
				Js_Parse_Args(Vec2, 0, "target = %s");
				Js_Parse_Args(bool, 1, "immediately = %s", (false));
				self->moveTo(arg0, arg1);
			});

			Js_Class_Method(setWaypoints, {
				Js_Parse_Args(PathPtr, 0, "waypoints = %s");
				Js_Parse_Args(bool, 1, "immediately = %s", (false));
				self->setWaypoints(arg0, arg1);
			});

			Js_Class_Method(returnToWaypoints, {
				Js_Parse_Args(bool, 0, "immediately = %s", (false));
				self->returnToWaypoints(arg0);
			});

			Js_Class_Method(stop, { self->stop(); });

			cls->exports("Agent", exports);
		}
	};

	class MixWorld: public MixViewObject {
	public:
		typedef World Type;
		virtual MorphView* asMorphView() { return self<World>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(World, Morph, { Js_NewView(World); });
			Js_MixObject_Accessor(World, bool, playing, playing);
			Js_MixObject_Accessor(World, int, subSteps, subSteps);
			Js_MixObject_Accessor(World, float, timeScale, timeScale);
			Js_MixObject_Accessor(World, float, predictionTime, predictionTime);
			Js_MixObject_Accessor(World, float, discoveryThresholdBuffer, discoveryThresholdBuffer);
			Js_MixObject_Accessor(World, float, waypointRadius, waypointRadius);
			cls->exports("World", exports);
		}
	};

	class MixSprite: public MixViewObject {
	public:
		typedef Sprite Type;
		virtual MorphView* asMorphView() { return self<Sprite>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Sprite, Agent, { Js_NewView(Sprite); });
			Js_MixObject_Accessor(Sprite, String, src, src);
			Js_MixObject_Accessor(Sprite, float, width, width);
			Js_MixObject_Accessor(Sprite, float, height, height);
			Js_MixObject_Accessor(Sprite, uint32_t, frame, frame);
			Js_MixObject_Accessor(Sprite, uint32_t, frames, frames);
			Js_MixObject_Accessor(Sprite, uint32_t, set, set);
			Js_MixObject_Accessor(Sprite, uint32_t, sets, sets);
			Js_MixObject_Accessor(Sprite, uint32_t, spacing, spacing);
			Js_MixObject_Accessor(Sprite, uint32_t, frequency, frequency);
			Js_MixObject_Accessor(Sprite, Direction, direction, direction);
			Js_MixObject_Accessor(Sprite, bool, playing, playing);
			Js_Class_Method(play, { self->play(); });
			Js_Class_Method(stop, { self->stop(); });
			cls->exports("Sprite", exports);
		}
	};

	void binding_sprite(JSObject* exports, Worker* worker) {
		MixEntity::binding(exports, worker);
		MixAgent::binding(exports, worker);
		MixSprite::binding(exports, worker);
		MixWorld::binding(exports, worker);
	}
} }
