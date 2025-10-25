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

#include <limits>
#include "../src/util/util.h"
#include "../src/ui/geometry.h"

using namespace qk;

struct Obstacle {
	enum Type { kLineSegment, kCircle, kPolygon } type;
	union {
		LineSegment line;
		Circle circle;
		struct { std::vector<Vec2> poly; } polygon;
	};
	~Obstacle() {
		if (type == kPolygon) {
			polygon.poly.~vector<Vec2>(); // manually call destructor
		}
	}
};

constexpr size_t size = sizeof(Obstacle);

struct Agent {
	Vec2 velocity; // 当前速度向量
	Vec2 target;
	Vec2 pos;
	float radius;
	float velocityScalar; // 速度大小
	Array<Vec2> poly; // 用于多边形避障检测
	int currentWaypoint = 0;
};

// ------------------------ 局部避让合成：结合 SAT MTV（即时）与 预测 (T_pred) ------------------------
// 返回避让向量（零向量表示不需要避让）
Vec2 computeAvoidanceForAgent(Agent& a, cArray<Obstacle>& obs, float T_pred, float safetyBuf = 0.1f) {
	Vec2 dirToTarget = (a.target - a.pos).normalized();
	Vec2 dir = (a.velocity.lengthSq() > 1e-6f) ? a.velocity.normalized() : dirToTarget;
	Vec2 avoidanceTotal{0,0};
	float maxDist = a.velocityScalar * T_pred;

	for (const auto& o : obs) {
		Vec2 mtv;
		if (o.type == Obstacle::kLineSegment) { // 1) 处理线段障碍（使用圆 vs 厚线段检测）
			if (test_circle_vs_line_segment({a.pos, a.radius}, o.line, &mtv)) {
				avoidanceTotal += mtv; // 推开优先：按深度加权
			} else {
				// 预测：如果在 T 时间内可能会到达墙体
				float d = predict_forward_distance_to_seg({a.pos, a.radius}, dir, o.line, safetyBuf);
				if (d < maxDist) {
					// 将避让朝向设置为远离线段中心的方向
					Vec2 cent = (o.line.a + o.line.b) * 0.5f;
					Vec2 away = (a.pos - cent).normalized(); // 远离方向
					avoidanceTotal += away * (1.0f + (maxDist - d) / maxDist); // 加权避让 1->2 away
				}
			}
		}
		else if (o.type == Obstacle::kCircle) { // 3) 处理圆形障碍（使用圆 vs 圆检测）
			if (test_circle_vs_circle({a.pos, a.radius}, o.circle, &mtv)) {
				avoidanceTotal += mtv; // 推开优先：按深度加权
			} else {
				float d = predict_forward_distance_to_circle({a.pos, a.radius}, dir, o.circle, safetyBuf);
				if (d < maxDist) {
					// 远离障碍中心方向避让
					avoidanceTotal += (a.pos - o.circle.center).normalized() * (1.0f + (maxDist - d) / maxDist);
				}
			}
		}
		else if (o.type == Obstacle::kPolygon) { // 2) 处理多边形障碍（使用 SAT 精确分离向量）
			if (test_polygon_vs_polygon(a.poly, o.polygon.poly, &mtv)) {
				avoidanceTotal += mtv; // 相交：MTV 已返回（注意：上面传入 computeDistanceWhenSeparated=false）
			} else {
				float d = predict_forward_distance_to_polygon({a.pos, a.radius}, dir, o.polygon.poly, safetyBuf);
				if (d < maxDist) {
					Vec2 cent{0,0};
					for (auto &v : o.polygon.poly) cent += v;
					cent = cent * (1.0f / o.polygon.poly.size()); // 质心
					// 远离障碍中心方向避让
					avoidanceTotal += (a.pos - cent).normalized() * (1.0f + (maxDist - d) / maxDist);
				}
			}
		} // end obstacle type check
	}

	return avoidanceTotal; // 返回总避让向量
}

// ------------------------ 更新单个 agent 的主函数（包含子步迭代）
void updateAgentWithAvoidance(
	Agent& a, cArray<Obstacle>& obs, float deltaTime, int nSubSteps = 2, float safetyBuf = 0.05f)
{
	float T_pred = 0.1; // 预测时间窗口
	Qk_ASSERT(a.velocityScalar > 0.0f); // 防止除零

	// 1. desired velocity towards current waypoint/target
	Vec2 dirToTarget = (a.target - a.pos).normalized();

	// 2. sub-step integration
	float stepDt = deltaTime / std::max(1, nSubSteps);
	for (int step = 0; step < nSubSteps; ++step) {
		// recompute avoidance (could be expensive; can be throttled)
		Vec2 avoidance = computeAvoidanceForAgent(a, obs, T_pred, safetyBuf);

		Vec2 newDir;
		// 如果 avoidance 几乎为 0，则不干预
		if (avoidance.lengthSq() < 1e-6f) {
			newDir = dirToTarget; // no avoidance
		} else {
			// 滑动优先：尝试合成目标与避让
			Vec2 aDir = avoidance.normalized();
			Vec2 sum = dirToTarget + aDir * 0.8f; // avoidFactor ~0.8
			if (sum.lengthSq() < 1e-6f) {
				// 特殊情况：被正面阻挡，取法线（沿障碍边滑动）
				sum = aDir.rotate90z(); // 任意法线
			}
			newDir = sum.normalized();
		}

		// limit velocity
		a.velocity = newDir * a.velocityScalar;

		// tentative move
		Vec2 move = a.velocity * stepDt;

		bool collided = false;
		Vec2 totalMTV{0,0};
		for (const auto& o : obs) {
			Vec2 mtv;
			bool collision = false;
			if (o.type == Obstacle::kLineSegment) {
				// collision prevention: do a quick circle vs walls check for move
				// 移动到新位置的检测（简单：检测目标位置）
				collision = test_circle_vs_line_segment({a.pos + move, a.radius}, o.line, &mtv);
			} else if (o.type == Obstacle::kCircle) {
				// circle vs circle
				collision = test_circle_vs_circle({a.pos + move, a.radius}, o.circle, &mtv);
			} else if (o.type == Obstacle::kPolygon) {
				// polygon check (approx circle->poly using SAT code) - optionally enable
				collision = test_polygon_vs_polygon(a.poly, o.polygon.poly, &mtv);
			}
			if (collision) {
				collided = true;
				totalMTV += mtv;
			}
		}

		if (!collided) {
			a.pos += move;
		} else {
			// Resolve by sliding: remove component of move along normal, keep tangential
			Vec2 mtvN = totalMTV.normalized();
			Vec2 moveTang = move - mtvN * move.dot(mtvN);
			// small step along tangential direction
			a.pos += moveTang * 0.5f; // scale to avoid jitter
			// optionally, also push out by mtv to remove overlap
			a.pos += mtvN * (totalMTV.length() + 1e-3f);
		}

		// update dirToTarget for next substep
		dirToTarget = (a.target - a.pos).normalized();
	}
}

// ------------------------ 批量更新（示例） ------------------------
void updateAgents(Array<Agent>& agents, cArray<Obstacle>& obs, float deltaTime) {
	// Simple loop; for many agents use spatial partitioning to cull walls/polys per-agent
	for (auto &a : agents) {
		if (a.poly.length() == 0) {
			// initialize agent polygon if needed
			a.poly = circle_to_octagon({a.pos, a.radius});
		}
		updateAgentWithAvoidance(a, obs, deltaTime, 2, 0.05f);
	}
}