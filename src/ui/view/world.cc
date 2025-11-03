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

#include "./world.h"
#include "./entity.h"

namespace qk {
	// Geometric epsilon for float comparisons
	constexpr float GEOM_EPS = 1e-6f;

	static float frandf(float min, float max) {
		constexpr float randMaxInv = 1.0f / float(RAND_MAX);
		return min + rand() * randMaxInv * (max - min);
	}

	void onUIEvent(cUIEventName& name, Agent* agent, UIEvent *evt) {
		struct CbCore: CallbackCore<Object> {
			CbCore(cUIEventName& name, UIEvent* evt) : name(name), evt(evt) {}
			void call(Data& e) { static_cast<Agent*>(e.data)->trigger(name, **evt); }
			Sp<UIEvent> evt;
			cUIEventName& name;
		};
		auto core = new CbCore(name, evt);
		agent->preRender().post(Cb(core), agent);
	}

	template<class T, typename... Args>
	static void onEvent(cUIEventName& name, Agent* agent, Args... args) {
		onUIEvent(name, agent, new T(agent, args...));
	}

	static void onDiscoveryAgent(Agent* agent, Agent* other, Vec2 mtv, uint32_t level, bool entering) {
		Agent* otherPtr = static_cast<Agent*>(other->tryRetain_rt());
		if (!otherPtr) // check agent validity
			return; // other agent is being destroyed
		struct Wrap { Sp<Agent> other; Vec2 mtv; uint32_t id; uint32_t level; bool entering; };
		struct CbCore: CallbackCore<Object> {
			void call(Data& e) {
				auto agent = static_cast<Agent*>(e.data);
				auto evt = new DiscoveryAgentEvent(agent, w.other.get(), w.mtv, w.id, w.level, w.entering);
				Sp<DiscoveryAgentEvent> h(evt);
				agent->trigger(UIEvent_DiscoveryAgent, **h);
			}
			Wrap w;
		};
		auto core = new CbCore;
		core->w = { Sp<Agent>::lazy(otherPtr), mtv, uint32_t(uintptr_t(other)), level, entering };
		agent->preRender().post(Cb(core), agent); // post to main thread
	}

	World::World()
		: _playing(false)
		, _subSteps(1), _timeScale(1.0f)
		, _predictionTime(0.1f)
		, _discoveryThresholdBuffer(5.0f)
		, _waypointRadius(0.0f) {
	}

	View* World::init(Window *win) {
		View::init(win);
		set_free(true);
		return this;
	}

	void World::set_playing(bool value) {
		if (_playing != value) {
			_playing = value;
			if (_playing) {
				preRender().addtask(this); // add task
			} else {
				preRender().untask(this); // remove task
			}
		}
	}

	void World::set_discoveryThresholdBuffer(float value) {
		_discoveryThresholdBuffer = Float32::clamp(value, 0.0f, 100.0f);
	}

	void World::set_subSteps(int value) {
		_subSteps = Int32::clamp(value, 1, 5);
	}

	void World::set_timeScale(float value) {
		_timeScale = Float32::clamp(value, 0.1f, 10.0f);
	}

	void World::set_predictionTime(float value) {
		_predictionTime = Float32::clamp(value, 0.05f, 2.0f);
	}

	void World::set_waypointRadius(float value) {
		_waypointRadius = Float32::max(value, 0.0f);
	}

	ViewType World::viewType() const {
		return kWorld_ViewType;
	}

	void World::onActivate() {
		if (level() == 0 || !_playing) {
			preRender().untask(this); // remove task
		} else {
			preRender().addtask(this); // add task
		}
	}

	void World::onChildLayoutChange(View* child, uint32_t mark) {
		auto other = child->asAgent();
		if (other && (!child->parent() || !child->visible())) { // removed from world or invisible
			auto v = first();
			while (v) {
				auto agent = v->asAgent();
				if (agent && agent != other) {
					// Remove from discovery set if no longer visible
					if (agent->_discoverys_rt.erase(other)) { // erase success
						onDiscoveryAgent(agent, other, Vec2(), 0xffffffff, false); // lose discovery, remove agent
					}
					if (agent->_followTarget == other) {
						agent->_following = false;
						agent->_followTarget = nullptr; // cancel follow target
						// notify cancel follow target
						onEvent<FollowStateEvent>(UIEvent_FollowStateChange, agent, FollowStateEvent::kCancel);
					}
				}
				v = v->next();
			}
		}
		View::onChildLayoutChange(child, mark);
	}

	/**
	 * discovery event processing
	 */
	void World::handleDiscoveryEvents(Agent* agent, Agent* other, MTV mtv) {
		// compute discovery distance squared
		auto buffer = _discoveryThresholdBuffer; // buffer distance to prevent flickering
		auto dds = agent->_discoveryDistances.load();
		auto mtvLen = mtv.overlap;
		auto level = dds->length();
		for (int i = level - 1; i >= 0; i--) {
			if (mtvLen < dds->at(i))
				level = i; // found level
			else break;
		}
		Vec2 mtvVec = mtv.axis * mtv.overlap;
		// process discovery levels
		if (level < dds->length()) { // within some level
			uint32_t *lastLevel;
			if (agent->_discoverys_rt.get(other, lastLevel)) { // found
				if (level > *lastLevel) { // leave
					if (mtvLen > dds->at(*lastLevel) + buffer) { // buffered leave
						*lastLevel = level; // update level
						onDiscoveryAgent(agent, other, mtvVec, level, false); // leave
					}
				} else if (level < *lastLevel) { // entering
					if (mtvLen < dds->at(*lastLevel) - buffer) {
						*lastLevel = level; // update level
						onDiscoveryAgent(agent, other, mtvVec, level, true); // entering
					}
				}
			} else { // new discovery
				agent->_discoverys_rt.set(other, level);
				onDiscoveryAgent(agent, other, mtvVec, level, true); // entering
			}
		} else { // out of all levels
			if (agent->_discoverys_rt.erase(other)) { // erase success
				onDiscoveryAgent(agent, other, mtvVec, 0xffffffff, false); // lose discovery
			}
		}
	}

	bool World::run_task(int64_t time, int64_t delta) {
		// World per-frame update logic can be added here
		Array<Agent*> agents, follows;
		Array<Entity*> entities;
		auto v = first();
		while (v) {
			auto entity = v->asEntity();
			// only process visible entities
			if (entity && entity->_circleBounds.radius != 0.0f && entity->visible()) {
				auto agent = entity->asAgent();
				if (agent && agent->_active && agent->_velocityMax) {
					// Skip line segment agents for movement
					if (agent->_bounds.type != Entity::kLineSegment) {
						if (agent->_followTarget) {
							follows.push(agent); // process follow targets later
						} else {
							agents.push(agent); // normal agent
						}
					}
				}
				if (entity->_isObstacle)
					entities.push(entity);
			}
			v = v->next();
		}

		if (agents.isNull() && follows.isNull()) {
			return false; // no active agents
		}

		float nsTime = 1.0f / 1e6f; // convert ns to seconds
		float timeFloat = time * nsTime; // convert to seconds
		auto deltaTime = delta * nsTime * _timeScale; // convert to seconds

		// Update normal agents
		for (auto agent : agents) {
			updateAgentWithMovement(agent, entities, timeFloat, deltaTime);
		}
		// Update follow agents
		for (auto agent : follows) {
			updateAgentWithFollow(agent, entities, timeFloat, deltaTime);
		}

		return false;
	}

	void World::updateAgentWithMovement(Agent* agent, cArray<Entity*>& obs, float time, float delta) {
		auto waypoints = agent->_waypoints.load(); // waypoints array
		auto current = agent->_currentWaypoint; // current waypoint index
		auto lastToTarget = agent->_target - agent->_translate;
		if (waypoints) {
			while (agent->_active && delta > 0.0f) { // still active
				// Move towards current waypoint
				delta = updateAgentWithAvoidance(agent, obs, time, delta);
				if (delta == 0.0f) {
					auto velocitySq = agent->_velocity.lengthSq();
					auto velocityMaxSq = agent->_velocityMax * agent->_velocityMax; // for optimization
					if (velocitySq / velocityMaxSq < 0.3f) {
						// Stopped moving due to avoidance
						// Check if the target has been approximately reached
						auto toTarget = agent->_target - agent->_translate;
						auto buffer = agent->_circleBounds.radius + _waypointRadius;
						auto toTargetLen = toTarget.length();
						if (toTargetLen <= buffer)
							goto tag1; // consider reached, advance to next waypoint
					}
					break; // no remaining, done for this frame
				}
			 tag1:
				if (current >= waypoints->ptsLen()) break;
				if (agent->_target != waypoints->atPt(current)) {
					// Set initial target waypoint if target changed
					agent->_target = waypoints->atPt(current);
					continue; // continue moving towards target
				}
				auto target = agent->_target; // current target waypoint
				current++; // next waypoint index
				agent->_currentWaypoint = current; // advance to next waypoint
				auto isNext = current < waypoints->ptsLen();
				// vector to next waypoint
				auto toNext = isNext ? waypoints->atPt(current) - target : Vec2();
				// Reached current waypoint event
				onEvent<ArrivePositionEvent>(UIEvent_ReachWaypoint, agent, target, toNext, current);
				if (isNext) {
					agent->_target = waypoints->atPt(current); // set new target
				} else {
					agent->_active = false; // reached final waypoint, stop
					onEvent<ArrivePositionEvent>(UIEvent_ArriveDestination, agent, target, Vec2(), 0);
					break;
				}
			}
		} else if (agent->_active) {
			delta = updateAgentWithAvoidance(agent, obs, time, delta);
			if (delta != 0.0f) { // Already at target, ended movement
				agent->_active = false;
				onEvent<ArrivePositionEvent>(UIEvent_ArriveDestination, agent, agent->_target, Vec2(), 0);
			}
		}
		agent->reportDirectionChange(lastToTarget.normalized());
	}

	void World::updateAgentWithFollow(Agent* agent, cArray<Entity*>& obs, float time, float delta) {
		auto other = agent->_followTarget.load();
		if (!other || !agent->_active)
			return;
		if (other->_bounds.type == Entity::kLineSegment)
			return;
		Qk_ASSERT(agent->_bounds.type != Entity::kLineSegment);

		Vec2 pos = agent->_translate; // current position
		MTV mtv; // minimum translation vector for avoidance
		Vec2 mtvVec;
		float safetyBuffer = agent->_safetyBuffer;
		constexpr float EPS = 1e-2f;//GEOM_EPS;

		bool collision = agent->test_entity_vs_entity(other, &mtv, &mtvVec, true);
		if (collision) {
			agent->_target = pos + mtvVec; // set target to avoid collision
		} else {
			float mtvLen = mtv.overlap; // distance to target
			float minBuf = mtvLen - agent->_followMinDistance;
			float maxBuf = mtvLen - agent->_followMaxDistance;
			float safetyBufferEPS = safetyBuffer + EPS;
			maxBuf = std::min(maxBuf, minBuf - safetyBufferEPS - safetyBufferEPS);

			// if (minBuf <= 0.0f || (agent->_following && minBuf < safetyBuffer)) {
			if (minBuf <= safetyBuffer) {
				agent->_target = pos + mtv.axis * (minBuf - safetyBuffer - EPS); // too close, move away
				if (!agent->_following && minBuf > 0.0f) {
					goto update; // still within buffer range, do not trigger follow start event
				}
			} else if (maxBuf >= 0.0f || (agent->_following && maxBuf > -safetyBuffer)) {
				agent->_target = pos + mtv.axis * (maxBuf + safetyBufferEPS); // too far, move closer
			} else {
				agent->_target = pos; // set target to current position
				// Already at follow distance
				if (agent->_following) { // stable for 0.2s
					//Qk_DLog("FollowStateEvent::kStop, %f %f", minBuf, maxBuf);
					agent->_following = false;
					onEvent<FollowStateEvent>(UIEvent_FollowStateChange, agent, FollowStateEvent::kStop);
				}
				goto update;
			}
		}

		if (!agent->_following) {
			//Qk_DLog("FollowStateEvent::kStart");
			agent->_following = true;
			onEvent<FollowStateEvent>(UIEvent_FollowStateChange, agent, FollowStateEvent::kStart);
		}
		update:
		updateAgentWithAvoidance(agent, obs, time, delta);
		// report direction change
		if (!collision) {
			agent->reportDirectionChange(mtv.axis);
			// is left
			// float dot = mtv.axis.dot(Vec2(-1.0f, 0.0f));
			// if (dot < 0.8f && dot > 0) {
			// 	Qk_DLog("is left");
			// }
		}
	}

	// ------------------------ 局部避让合成：结合 SAT MTV（即时）与 预测 (T) ------------------------
	Vec2 World::computeAvoidanceForAgent(Agent *agent, cArray<Entity*>& obs, Vec2 dirToTarget) {
		bool isPoly = agent->_bounds.type == Entity::kPolygon;
		auto &pts = agent->ptsOfBounds();
		Circle &circ = agent->_circleBounds;
		Vec2 pos = circ.center;
		Vec2 dir = (agent->_velocity.lengthSq() > GEOM_EPS) ? agent->_velocity.normalized() : dirToTarget;
		Vec2 avoidanceTotal; // total avoidance vector
		float maxDist = agent->_velocityMax * _predictionTime;
		float safetyBuf = agent->_safetyBuffer;

		#define USE_tangent 1

		for (auto o : obs) {
			if (o == agent) continue; // skip self
			bool isFollowingTarget = agent->_followTarget == o;
			MTV mtv; // avoidance vector for this obstacle

			if (o->_bounds.type == Entity::kDefault || o->_bounds.type == Entity::kCircle) { // 1) 处理圆形障碍
				Circle circB = o->_circleBounds;
				if (isPoly ? test_polygon_vs_polygon(pts, o->ptsOfBounds(), &mtv):
						test_circle_vs_circle(circ, circB, &mtv)) {
					avoidanceTotal += mtv.axis * mtv.overlap; // 推开优先：按深度加权
				} else if (!isFollowingTarget) { // 如果是跟随目标不需要预测避让，应该直接靠近
					float d = isPoly ?
						predict_forward_distance_circ_to_poly(circB, -dir, pts, safetyBuf, &pos):
						predict_forward_distance_circ_to_circ(circ, dir, circB, safetyBuf);
					if (d < maxDist) {
						// 远离障碍中心切线方向避让
						auto away = (pos - circB.center).normalized();
						#if USE_tangent
						away = away.det(dirToTarget) < GEOM_EPS ? away.rotate90z() : away.rotate270z();
						#endif
						avoidanceTotal += away * (1.0f + (maxDist - d) / maxDist);
					}
				}
			}
			else if (o->_bounds.type == Entity::kPolygon) { // 2) 处理多边形障碍（使用 SAT 精确分离向量）
				auto &ploy = o->ptsOfBounds();
				if (test_polygon_vs_polygon(pts, ploy, &mtv)) {
					avoidanceTotal += mtv.axis * mtv.overlap; // 相交：MTV 已返回
				} else if (!isFollowingTarget) { // 如果是跟随目标不需要预测避让
					Vec2 cent; // 质心
					float d = isPoly ?
						predict_forward_distance_poly_to_poly(pts, dir, ploy, safetyBuf, &pos, &cent) :
						predict_forward_distance_circ_to_poly(circ, dir, ploy, safetyBuf, &cent);
					if (d < maxDist) {
						// 远离障碍中心切线方向避让
						Vec2 away = (pos - cent).normalized();
						#if USE_tangent
						away = away.det(dirToTarget) < GEOM_EPS ? away.rotate90z() : away.rotate270z();
						#endif
						avoidanceTotal += away * (1.0f + (maxDist - d) / maxDist);
					}
				}
			} else { // 3) 处理线段障碍
				auto halfWidth = o->_bounds.halfThickness;
				auto &pts = o->ptsOfBounds();
				auto a = pts.at(0);
				for (int i = 1; i < pts.length(); ++i) {
					auto b = pts.at(i);
					LineSegment seg{a, b, halfWidth};
					if (isPoly ? test_poly_vs_line_segment(pts, seg, &mtv) :
							test_circle_vs_line_segment(circ, seg, &mtv)) {
						avoidanceTotal += mtv.axis * mtv.overlap; // 推开优先：按深度加权
					} else {
						// 预测：如果在 T 时间内可能会到达墙体
						float d = isPoly ?
							predict_forward_distance_poly_to_seg(pts, dir, seg, safetyBuf, &pos) :
							predict_forward_distance_circ_to_seg_fast(circ, dir, seg, safetyBuf);
						if (d < maxDist) {
							// 将避让朝向设置为远离线段中心切线的方向
							Vec2 cent = (seg.a + seg.b) * 0.5f;
							Vec2 away = (pos - cent).normalized(); // 远离方向
							#if USE_tangent
							away = away.det(dirToTarget) < GEOM_EPS ? away.rotate90z() : away.rotate270z();
							#endif
							avoidanceTotal += away * (1.0f + (maxDist - d) / maxDist); // 加权避让 1->2 away
						}
					}
					a = b;
				}
			}
		}

		return avoidanceTotal; // 返回总避让向量
	}

	// 计算 agent 的新速度向量
	void World::updateVelocityForAgent(Agent* agent, cArray<Entity*>& obs, Vec2 toTarget) {
		Vec2 dirToTarget = toTarget.normalized(); // recompute each step
		// recompute avoidance (could be expensive; can be throttled)
		Vec2 avoidance = computeAvoidanceForAgent(agent, obs, dirToTarget);

		Vec2 newDir;
		// 如果 avoidance 几乎为 0，则不干预
		if (avoidance.lengthSq() < GEOM_EPS) {
			newDir = dirToTarget; // no avoidance
		} else {
			// 滑动优先：尝试合成目标与避让
			Vec2 aDir = avoidance.normalized();
			float oppose = aDir.dot(dirToTarget); // [-1,1]
			// 检查是否几乎互相抵消（正面硬顶）取法线（沿障碍边滑动）
			if (oppose < GEOM_EPS - 1.0f) {
				// 判断 dir 与 aDir 的相对朝向（通过叉积符号）
				newDir = aDir.det(dirToTarget) < GEOM_EPS ? aDir.rotate90z() : aDir.rotate270z();
			} else {
				// 根据对抗程度，自动调低避让比重
				// float weight = 0.8f + (-oppose) * 0.3f; // weight 避让权重 ~0.5->1.1
				float weight = 0.8f;
				Vec2 sum = dirToTarget + aDir * weight;
				newDir = sum.normalized();
			}
		}

		// limit velocity change
		Vec2 lastVelocity = agent->_velocity;
		// First order low-pass filtering, smooth velocity change
		auto velocity = lastVelocity * 0.85f + newDir * agent->_velocityMax * 0.15f;
		//auto velocity = newDir * agent->_velocityMax;
		float vLen = velocity.length();
		float maxV = agent->_velocityMax;
		if (vLen > maxV) // clamp to max velocity
			velocity *= maxV / vLen;
		agent->_velocity = velocity; // update agent velocity
	}

	// ------------------------ 更新单个 agent 的主函数（包含子步迭代）
	float World::updateAgentWithAvoidance(Agent* agent, cArray<Entity*>& obs, float time, float deltaTime) {
		Qk_ASSERT(agent->_velocityMax > 0.0f); // must have max velocity
		Qk_ASSERT(agent->_bounds.type != Entity::kLineSegment); // line segment agent does not move

		bool isPoly = agent->_bounds.type == Entity::kPolygon;
		bool isFollow = agent->_followTarget.load();
		Circle &circ = agent->_circleBounds;
		Vec2 pos = agent->_translate; // current position
		// Vec2 pos = circ.center; // current position
		Vec2 toTarget = agent->_target - pos;

		if (time - agent->_lastUpdateTime > 1.0f) { // 1 second without update
			// If last update was long ago, reset velocity to avoid sudden jumps
			agent->_velocity = {}; // reset velocity
		}
		agent->_lastUpdateTime = time;

		// precompute polygon points at current position
		auto* pts = isPoly ? &agent->ptsOfBounds() : nullptr;
		// load discovery distances
		auto dds = agent->_discoveryDistances.load();

		// sub-step integration
		float stepDt = deltaTime / std::max(1, _subSteps);
		for (int step = 0; step < _subSteps; ++step) {
			Vec2 move; // movement this sub-step
			float movedLenSq = 0;
			float toTargetLenSq = toTarget.lengthSq();
			bool hasTarget = toTargetLenSq > GEOM_EPS;
			if (hasTarget) {
				updateVelocityForAgent(agent, obs, toTarget);
				move = agent->_velocity * stepDt;
				if (pts) // precompute polygon points at new position
					for (auto& pt : *pts) pt += move;
				circ.center += move; // precompute circle at new position
				movedLenSq = move.lengthSq();
			} else {
				agent->_velocity *= 0.85f; // slow down when near target
				if (!isFollow) { // not following, can stop
					// reached target, return remaining time
					return deltaTime - stepDt * step;
				}
			}

			bool collided = false;
			Vec2 totalMTV;
			for (auto o : obs) {
				if (o == agent) continue; // skip self
				MTV mtv;
				auto other = o->asAgent();
				bool isEvent = dds && other; // only agents have discovery events
				if (agent->test_entity_vs_entity(o, &mtv, &totalMTV, isEvent)) {
					collided = true;
				}
				if (isEvent) { // handle discovery events
					handleDiscoveryEvents(agent, other, mtv);
				}
			}

			if (collided) {
				// apply total MTV to move out of collision
				float overlap = totalMTV.length() + 1e-3f; // add small epsilon to avoid precision issues
				float factor = agent->_avoidanceFactor * 0.5f; // scale down avoid factor on collision
				float near = std::min(sqrtf(toTargetLenSq) / circ.radius, 1.0f);
				factor *= (0.8f + near * 0.2f); // reduce avoid factor when near target
				Vec2 axis = totalMTV.normalized(); // MTV points from this agent outward
				auto avoidance = axis * (overlap * factor);

				Vec2 &v = agent->_velocity;
				float vn = v.dot(axis);
				// if velocity is pushing into the wall, remove that component
				if (vn > GEOM_EPS) {
					v = v - axis * vn; // keep only tangential velocity
				}
				v *= agent->_avoidanceVelocityFactor; // reduce velocity on collision

				// precompute polygon points for next step
				if (pts && step < _subSteps - 1)
					for (auto &pt: *pts) pt += avoidance;
				move += avoidance;
				circ.center += avoidance; // update circle center
			}

			// check if reached target
			if (!isFollow && movedLenSq >= toTargetLenSq) {
				float scalar = sqrtf(toTargetLenSq) / sqrtf(movedLenSq); // 0-1 scale
				Vec2 finalMove = move * scalar; // scale move to exactly
				float lenSq = (toTarget - finalMove).lengthSq();
				if (lenSq < 1) {
					// reached target this step
					agent->set_translate(agent->translate() + finalMove, true); // update entity position
					return deltaTime - (step + 1.0f - scalar) * stepDt; // return overflow time
				}
			}

			// apply move
			pos += move;
			agent->set_translate(agent->translate() + move, true); // update entity position

			// update toTarget for next substep
			toTarget = agent->_target - pos;
		}

		return 0; // all time used
	}

} // namespace qk
