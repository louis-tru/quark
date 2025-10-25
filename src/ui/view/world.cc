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

	World::World()
		: _playing(false)
		, _subSteps(1), _timeScale(1.0f)
		, _predictionTime(0.1f), _avoidanceFactor(0.8f), _discoveryThresholdBuffer(5.0f) {
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

	void World::set_subSteps(int value) {
		_subSteps = Int32::clamp(value, 1, 5);
	}

	void World::set_timeScale(float value) {
		_timeScale = Float32::clamp(value, 0.1f, 10.0f);
	}

	void World::set_predictionTime(float value) {
		_predictionTime = Float32::clamp(value, 0.05f, 2.0f);
	}

	void World::set_avoidanceFactor(float value) {
		_avoidanceFactor = Float32::clamp(value, 0.0f, 1.0f);
	}

	void World::set_discoveryThresholdBuffer(float value) {
		_discoveryThresholdBuffer = Float32::clamp(value, 0.0f, 100.0f);
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

	template<class T, typename... Args>
	static void onEvent(cUIEventName& name, Agent* agent, Args... args) {
		agent->preRender().post(Cb([agent,name,args...](auto e) {
			Sp<T> h(new T(agent, args...));
			agent->trigger(name, **h);
		}), agent);
	}

	static void onDiscoveryAgent(Agent* agent, Agent* other, Vec2 mtv, int level, bool entering, bool removeOther) {
		Agent* otherPtr = nullptr;
		if (!removeOther) { // check agent validity
			otherPtr = static_cast<Agent*>(other->tryRetain_rt());
			if (!otherPtr)
				return; // agent already deleted
		}
		struct Wrap { Sp<Agent> other; Vec2 mtv; uint32_t id; int level; bool entering; };
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

	void World::onChildLayoutChange(View* child, uint32_t mark) {
		auto other = child->asAgent();
		if (other && (mark & kChild_Layout_Visible)) {
			auto v = first();
			while (v) {
				auto agent = v->asAgent();
				if (agent && other != agent) {
					// Remove from discovery set if no longer visible
					if (agent->_discoverys_rt.erase(other)) { // erase success
						onDiscoveryAgent(agent, other, Vec2(), -1, false, true); // lose discovery, remove agent
					}
					if (agent->_followTarget == other) {
						agent->_following = false;
						agent->_followTarget = nullptr; // cancel follow target
						// notify cancel follow target
						onEvent<FollowTargetEvent>(UIEvent_FollowStateChange, agent, FollowTargetEvent::kCancel);
					}
				}
				v = v->next();
			}
		}
		View::onChildLayoutChange(child, mark);
	}

	bool World::run_task(int64_t time, int64_t delta) {
		// World per-frame update logic can be added here
		Array<Agent*> agents, follows;
		Array<Entity*> entities;
		auto v = first();
		while (v) {
			auto entity = v->asEntity();
			if (entity && entity->visible()) { // only process visible entities
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
				entities.push(entity);
			}
			v = v->next();
		}

		if (agents.isNull() && follows.isNull()) {
			return false; // no active agents
		}

		auto update = false;
		auto deltaTime = delta / 1e6f * _timeScale; // convert to seconds

		// Update normal agents
		for (auto agent : agents) {
			updateAgentWithMovement(agent, entities, deltaTime, update);
		}
		// Update follow agents
		for (auto agent : follows) {
			updateAgentWithFollow(agent, entities, deltaTime, update);
		}

		return update;
	}

	void World::updateAgentWithMovement(Agent* agent, cArray<Entity*>& obs, float delta, bool &update) {
		auto waypoints = agent->_waypoints.load(); // waypoints array
		auto current = agent->_currentWaypoint; // current waypoint index
		if (waypoints) {
			while (agent->_active) { // still active
				// Move towards current waypoint
				float remaining = updateAgentWithAvoidance(agent, obs, delta);
				if (remaining != delta) update = true;
				if (remaining == 0.0f) break; // no remaining, done for this frame
				delta = remaining; // continue with remaining time
				if (agent->_target != waypoints->at(current)) {
					// Set initial target waypoint if target changed
					agent->_target = waypoints->at(current);
					continue; // continue moving towards target
				}
				auto target = agent->_target; // current target waypoint
				current++; // next waypoint index
				agent->_currentWaypoint = current; // advance to next waypoint
				auto isNext = current < waypoints->length();
				// vector to next waypoint
				auto toNext = isNext ? waypoints->at(current) - target : Vec2();
				// Reached current waypoint event
				onEvent<ArrivePositionEvent>(UIEvent_ReachWaypoint, agent, target, toNext, current);
				if (isNext) {
					agent->_target = waypoints->at(current); // set new target
				} else {
					onEvent<ArrivePositionEvent>(UIEvent_ArriveDestination, agent, target, Vec2(), 0);
					agent->_active = false; // reached final waypoint, stop
					break;
				}
			}
		} else if (agent->_active) {
			float remaining = updateAgentWithAvoidance(agent, obs, delta);
			if (remaining != delta) update = true;
			if (remaining != 0.0f) { // Already at target, ended movement
				onEvent<ArrivePositionEvent>(UIEvent_ArriveDestination, agent, agent->_target, Vec2(), 0);
				agent->_active = false;
			}
		}
	}

	void World::updateAgentWithFollow(Agent* agent, cArray<Entity*>& obs, float delta, bool &update) {
		auto target = agent->_followTarget.load();
		if (!target)
			return;
		if (target->_bounds.type == Entity::kLineSegment)
			return;
		Qk_ASSERT(agent->_bounds.type != Entity::kLineSegment);

		bool isPoly = agent->_bounds.type == Entity::kPolygon;
		bool collided;
		Vec2 pos = agent->_circleBounds.center; // current position
		MTV mtv; // minimum translation vector for avoidance

		if (target->_bounds.type == Entity::kPolygon) {
			collided = isPoly ?
				test_polygon_vs_polygon(agent->ptsOfBounds(), target->ptsOfBounds(), &mtv, true):
				test_circle_vs_polygon(agent->_circleBounds, target->ptsOfBounds(), &mtv, true);
		} else {
			collided = isPoly ?
				test_polygon_vs_polygon(agent->ptsOfBounds(), target->ptsOfBounds(), &mtv, true):
				test_circle_vs_circle(agent->_circleBounds, target->_circleBounds, &mtv, true);
		}

		if (collided) {
			agent->_target = pos + mtv.axis * mtv.overlap; // set target to avoid collision
		} else {
			Qk_ASSERT(mtv.overlap >= 0.0f);
			float mtvLen = mtv.overlap - agent->_safetyBuffer; // minus safety buffer
			float minBuf = mtvLen - agent->_followDistanceRange[0], maxBuf;
			if (minBuf < 0.0f) {
				agent->_target = pos + mtv.axis * minBuf; // too close, move away
			} else if ((maxBuf = mtvLen - agent->_followDistanceRange[1]) > GEOM_EPS) {
				agent->_target = pos + mtv.axis * maxBuf; // too far, move closer
			} else {
				// Already at follow distance
				if (agent->_following) {
					agent->_following = false;
					onEvent<FollowTargetEvent>(UIEvent_FollowStateChange, agent, FollowTargetEvent::kStop);
				}
				return;
			}
		}

		if (!agent->_following) {
			agent->_following = true;
			onEvent<FollowTargetEvent>(UIEvent_FollowStateChange, agent, FollowTargetEvent::kStart);
		}

		if (updateAgentWithAvoidance(agent, obs, delta) != delta)
			update = true;
	} 

	/**
	 * discovery event processing
	 */
	void World::handleDiscoveryEvents(Agent* agent, Agent* other, MTV mtv, float bufferSq) {
		auto ddsSq = agent->_discoveryDistancesSq.load();
		auto mtvLenSq = mtv.overlap * mtv.overlap;
		auto level = ddsSq->length();
		for (int i = level - 1; i >= 0; i--) {
			if (mtvLenSq < ddsSq->at(i)) {
				level = i; // found level
			} else {
				break;
			}
		}
		Vec2 mtvVec = mtv.axis * mtv.overlap;
		// process discovery levels
		if (level < ddsSq->length()) { // within some level
			uint32_t *oldLevel;
			if (agent->_discoverys_rt.get(other, oldLevel)) { // found
				if (level > *oldLevel) { // leave
					if (mtvLenSq > ddsSq->at(level) + bufferSq) { // buffered leave
						*oldLevel = level; // update level
						onDiscoveryAgent(agent, other, mtvVec, level, false, false); // leave
					}
				} else if (level < *oldLevel) { // entering
					if (mtvLenSq < ddsSq->at(level) - bufferSq) {
						*oldLevel = level; // update level
						onDiscoveryAgent(agent, other, mtvVec, level, true, false); // entering
					}
				}
			} else { // new discovery
				agent->_discoverys_rt.set(other, level);
				onDiscoveryAgent(agent, other, mtvVec, level, true, false); // entering
			}
		} else { // out of all levels
			if (agent->_discoverys_rt.erase(other)) { // erase success
				onDiscoveryAgent(agent, other, mtvVec, -1, false, false); // lose discovery
			}
		}
	}

	// ------------------------ 局部避让合成：结合 SAT MTV（即时）与 预测 (T) ------------------------
	Vec2 World::computeAvoidanceForAgent(Agent *agent, cArray<Entity*>& obs, Circle circ, Array<Vec2>& pts, Vec2 dirToTarget) {
		bool isPoly = agent->_bounds.type == Entity::kPolygon;
		Vec2 pos = circ.center;
		Vec2 dir = (agent->_velocity.lengthSq() > GEOM_EPS) ? agent->_velocity.normalized() : dirToTarget;
		Vec2 avoidanceTotal; // total avoidance vector
		float maxDist = agent->_velocityMax * _predictionTime;

		for (auto o : obs) {
			if (o == agent)
				continue; // skip self
			// select safety buffer
			float safetyBuf = agent->_safetyBuffer;
			if (agent->_followTarget == o) // following target
				safetyBuf += agent->_followDistanceRange[0]; // add follow distance
			MTV mtv; // avoidance vector for this obstacle

			if (o->_bounds.type == Entity::kDefault || o->_bounds.type == Entity::kCircle) { // 1) 处理圆形障碍
				Circle circB = o->_circleBounds;
				if (isPoly ? test_polygon_vs_polygon(pts, o->ptsOfBounds(), &mtv): test_circle_vs_circle(circ, circB, &mtv)) {
					avoidanceTotal += mtv.axis * mtv.overlap; // 推开优先：按深度加权
				} else {
					float d = isPoly ?
						predict_forward_distance_circ_to_poly(circB, -dir, pts, safetyBuf, &pos):
						predict_forward_distance_circ_to_circ(circ, dir, circB, safetyBuf);
					if (d < maxDist) {
						// 远离障碍中心方向避让
						avoidanceTotal += (pos - circB.center).normalized() * (1.0f + (maxDist - d) / maxDist);
					}
				}
			}
			else if (o->_bounds.type == Entity::kLineSegment) { // 2) 处理线段障碍
				auto halfWidth = o->_bounds.halfThickness;
				auto &pts = o->ptsOfBounds();
				auto A = pts.lastAt(0);
				for (auto B: pts) {
					LineSegment seg{A, B, halfWidth};
					if (isPoly ? test_poly_vs_line_segment(pts, seg, &mtv) : test_circle_vs_line_segment(circ, seg, &mtv)) {
						avoidanceTotal += mtv.axis * mtv.overlap; // 推开优先：按深度加权
					} else {
						// 预测：如果在 T 时间内可能会到达墙体
						float d = isPoly ?
							predict_forward_distance_poly_to_seg(pts, dir, seg, safetyBuf, &pos) :
							predict_forward_distance_circ_to_seg(circ, dir, seg, safetyBuf);
						if (d < maxDist) {
							// 将避让朝向设置为远离线段中心的方向
							Vec2 cent = (seg.a + seg.b) * 0.5f;
							Vec2 away = (pos - cent).normalized(); // 远离方向
							avoidanceTotal += away * (1.0f + (maxDist - d) / maxDist); // 加权避让 1->2 away
						}
					}
					A = B;
				}
			}
			else if (o->_bounds.type == Entity::kPolygon) { // 3) 处理多边形障碍（使用 SAT 精确分离向量）
				auto &ploy = o->ptsOfBounds();
				if (test_polygon_vs_polygon(pts, ploy, &mtv)) {
					avoidanceTotal += mtv.axis * mtv.overlap; // 相交：MTV 已返回
				} else {
					Vec2 cent; // 质心
					float d = isPoly ?
						predict_forward_distance_poly_to_poly(pts, dir, ploy, safetyBuf, &pos, &cent) :
						predict_forward_distance_circ_to_poly(circ, dir, ploy, safetyBuf, &cent);
					if (d < maxDist) {
						// 远离障碍中心方向避让
						avoidanceTotal += (pos - cent).normalized() * (1.0f + (maxDist - d) / maxDist);
					}
				}
			} // end obstacle type check
		}

		return avoidanceTotal; // 返回总避让向量
	}

	// ------------------------ 更新单个 agent 的主函数（包含子步迭代）
	float World::updateAgentWithAvoidance(Agent* agent, cArray<Entity*>& obs, float deltaTime) {
		Qk_ASSERT(agent->_velocityMax > 0.0f); // must have max velocity
		Qk_ASSERT(agent->_bounds.type != Entity::kLineSegment); // line segment agent does not move

		bool isPoly = agent->_bounds.type == Entity::kPolygon;
		Circle circ = agent->_circleBounds;
		Vec2 pos = circ.center;
		Vec2 toTarget = agent->_target - pos;

		if (toTarget.lengthSq() < GEOM_EPS) {
			// Already at target
			agent->_velocity = Vec2{0,0};
			return deltaTime; // overflow all time
		}

		cArray<Vec2>* pts = nullptr;
		Array<Vec2> prePts;
		// precompute polygon points at current position
		if (isPoly) {
			pts = &agent->ptsOfBounds();
			prePts = *pts;
		}

		Vec2 moveTotal; // total move this update

		// load discovery distances
		auto ddsSq = agent->_discoveryDistancesSq.load();
		// compute discovery distance squared
		auto bufferSq = _discoveryThresholdBuffer * _discoveryThresholdBuffer;

		// sub-step integration
		float stepDt = deltaTime / std::max(1, _subSteps);
		for (int step = 0; step < _subSteps; ++step) {
			Vec2 dirToTarget = toTarget.normalized(); // recompute each step
			// recompute avoidance (could be expensive; can be throttled)
			Vec2 avoidance = computeAvoidanceForAgent(agent, obs, circ, prePts, dirToTarget);

			Vec2 newDir;
			// 如果 avoidance 几乎为 0，则不干预
			if (avoidance.lengthSq() < GEOM_EPS) {
				newDir = dirToTarget; // no avoidance
			} else {
				// 滑动优先：尝试合成目标与避让
				Vec2 aDir = avoidance.normalized();
				Vec2 sum = dirToTarget + aDir * _avoidanceFactor; // avoidFactor ~0.8
				if (sum.lengthSq() < GEOM_EPS) {
					// 特殊情况：被正面阻挡，取法线（沿障碍边滑动）
					sum = aDir.rotate90z(); // 任意法线
				}
				newDir = sum.normalized();
			}

			// limit velocity
			agent->_velocity = newDir * agent->_velocityMax;

			// tentative move
			Vec2 move = agent->_velocity * stepDt;
			circ.center = pos + move; // precompute circle at new position

			// precompute polygon points at new position
			if (isPoly) {
				Vec2 off = moveTotal + move; // offset from original position
				for (int i = 0; i < prePts.length(); ++i)
					prePts[i] = pts->at(i) + off;
			}

			bool collided = false;
			Vec2 totalMTV;
			for (auto o : obs) {
				if (o == agent)
					continue; // skip self
				MTV mtv;
				auto other = o->asAgent();
				bool isEvent = ddsSq && other;
				bool collision = false;
				if (o->_bounds.type == Entity::kDefault || o->_bounds.type == Entity::kCircle) {
					// circle vs circle
					collision = isPoly ?
						test_polygon_vs_polygon(prePts, o->ptsOfBounds(), &mtv, isEvent):
						test_circle_vs_circle(circ, o->_circleBounds, &mtv, isEvent);
				}
				else if (o->_bounds.type == Entity::kLineSegment) {
					// collision prevention: do a quick circle vs walls check for move
					auto halfWidth = o->_bounds.halfThickness;
					auto &ptsB = o->ptsOfBounds();
					auto a = ptsB.lastAt(0); // get last point
					for (auto b: ptsB) {
						if (isPoly ? 
								test_poly_vs_line_segment(prePts, {a, b, halfWidth}, &mtv): 
								test_circle_vs_line_segment(circ, {a, b, halfWidth}, &mtv)) {
							collided = true;
							totalMTV += mtv.axis * mtv.overlap;
						}
						a = b;
					}
					continue; // already handled above
				} else if (o->_bounds.type == Entity::kPolygon) {
					// polygon check (approx circle->poly using SAT code) - optionally enable
					collision = isPoly ?
						test_polygon_vs_polygon(prePts, o->ptsOfBounds(), &mtv, isEvent):
						test_circle_vs_polygon(circ, o->ptsOfBounds(), &mtv, isEvent);
				}

				if (collision) {
					collided = true;
					totalMTV += mtv.axis * mtv.overlap;
				}

				// handle discovery events
				if (isEvent) {
					handleDiscoveryEvents(agent, other, mtv, bufferSq);
				}
			}

			if (collided) {
				// Resolve by sliding: remove component of move along normal, keep tangential
				Vec2 mtvN = totalMTV.normalized();
				Vec2 moveTang = move - mtvN * move.dot(mtvN);
				// small step along tangential direction
				move = moveTang * 0.5f; // scale to avoid jitter
				// optionally, also push out by mtv to remove overlap
				move += mtvN * (totalMTV.length() + 1e-3f);
			}

			float movedLenSq = move.lengthSq();
			float toTargetLenSq = toTarget.lengthSq();
			// check if reached target
			if (movedLenSq >= toTargetLenSq) {
				float scalar = sqrtf(toTargetLenSq) / sqrtf(movedLenSq); // 0-1 scale
				Vec2 finalMove = move * scalar; // scale move to exactly
				if ((toTarget - finalMove).lengthSq() < GEOM_EPS) {
					// reached target this step
					agent->set_translate(agent->translate() + finalMove, true); // update entity position
					agent->_velocity = Vec2{0,0}; // stop at target
					return deltaTime - stepDt * (step + 2.0f - scalar); // return overflow time
				}
			}

			// apply move
			pos += move;
			moveTotal += move; // accumulate total move
			circ.center = pos; // update circle center

			// precompute polygon points for next step
			if (isPoly && step < _subSteps - 1) {
				for (int i = 0; i < prePts.length(); ++i)
					prePts[i] = pts->at(i) + moveTotal;
			}
			agent->set_translate(agent->translate() + move, true); // update entity position

			// update toTarget for next substep
			toTarget = agent->_target - pos;
		}

		return 0; // all time used
	}

} // namespace qk
