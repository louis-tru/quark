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
#include "./geometry.h"

namespace qk {

	// 全局几何比较容差
	constexpr float GEOM_EPS = 1e-6f;


	// 判断点是否在凸四边形内
	// quadrilateral: 凸四边形的四个顶点，按顺时针或逆时针顺序排列
	// point: 待测试的点
	// 返回值: true - 在四边形内, false - 在四边形外
	bool test_overlap_from_convex_quadrilateral(Vec2* quadrilateral, Vec2 point) {
		/*
		* 直线方程：(x-x1)(y2-y1)-(y-y1)(x2-x1)=0
		* 平面座标系中凸四边形内任一点是否存在：
		* [(x-x1)(y2-y1)-(y-y1)(x2-x1)][(x-x4)(y3-y4)-(y-y4)(x3-x4)] < 0  and
		* [(x-x2)(y3-y2)-(y-y2)(x3-x2)][(x-x1)(y4-y1)-(y-y1)(x4-x1)] < 0
		*/

		float x = point.x();
		float y = point.y();

		#define x1 quadrilateral[0].x()
		#define y1 quadrilateral[0].y()
		#define x2 quadrilateral[1].x()
		#define y2 quadrilateral[1].y()
		#define x3 quadrilateral[2].x()
		#define y3 quadrilateral[2].y()
		#define x4 quadrilateral[3].x()
		#define y4 quadrilateral[3].y()

		if (((x-x1)*(y2-y1)-(y-y1)*(x2-x1))*((x-x4)*(y3-y4)-(y-y4)*(x3-x4)) < 0 &&
				((x-x2)*(y3-y2)-(y-y2)*(x3-x2))*((x-x1)*(y4-y1)-(y-y1)*(x4-x1)) < 0
		) {
			return true;
		}

		#undef x1
		#undef y1
		#undef x2
		#undef y2
		#undef x3
		#undef y3
		#undef x4
		#undef y4

		return false;
	}

	// 判断点是否在多边形内
	// polygon: 多边形的所有顶点，按顺时针或逆时针顺序排列
	// point: 待测试的点
	// 返回值: true - 在多边形内, false - 在多边形外
	bool test_overlap_from_convex_polygon(cArray<Vec2>& polygon, Vec2 point) {
		float x = point.x();
		float y = point.y();
		// 使用容差并适配任意顺时针/逆时针顶点顺序。
		// 如果所有非零叉积符号一致（或为零），则点在凸多边形内。
		int n = polygon.length();
		if (n < 3) return false;

		int sign = 0;
		for (int i = 0; i < n; ++i) {
			int next = (i + 1) % n;
			float x1 = polygon[i].x();
			float y1 = polygon[i].y();
			float x2 = polygon[next].x();
			float y2 = polygon[next].y();

			float cross = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
			if (fabsf(cross) <= GEOM_EPS) {
				// 点在边附近（视为在边上），当作在内
				continue;
			}
			int s = cross > 0 ? 1 : -1;
			if (sign == 0) sign = s;
			else if (s != sign) return false;
		}
		return true;
	}

	// 判断点是否在任意多边形内
	// polygon_vertex: 多边形的所有顶点，按顺时针或逆时针顺序排列
	// vertex_count: 多边形顶点的数量
	// point: 待测试的点
	// 返回值: true - 在多边形内, false - 在多边形外
	bool test_overlap_from_polygon(cArray<Vec2>& polygon, Vec2 point) {
		float x = point.x();
		float y = point.y();
		
		// 初始化交点计数
		int count = 0;

		// 遍历每条边
		for (int i = 0; i < polygon.length(); ++i) {
			// 获取当前边的两个端点
			int next = (i + 1) % polygon.length();
			float x1 = polygon[i].x();
			float y1 = polygon[i].y();
			float x2 = polygon[next].x();
			float y2 = polygon[next].y();

			// 检查点是否在线段的水平射线与边相交
			if (((y1 > y) != (y2 > y)) &&  // 点的y值在边的y范围内
					(x < (x2 - x1) * (y - y1) / (y2 - y1) + x1)) {  // 射线与边的交点
				count++;
			}
		}

		// 如果交点数是奇数，则点在多边形内；如果是偶数，则在多边形外
		return (count % 2 == 1);
	}

	// SAT 检测两个凸多边形是否相交。
	// poly1, poly2: 凸多边形的顶点数组，按
	// origin1, origin2: 多边形的原点位置
	// outSeparation: 输出最小穿透向量或分离向量（可选）
	// 返回值: true - 相交, false - 不相交
	// - computeSeparated = true : 计算不相交时的最短距离向量
	// - computeSeparated = false: 不相交时提前退出，提高性能
	bool test_polygon_vs_polygon_sat(cArray<Vec2>& poly1, cArray<Vec2>& poly2, MTV* mtv) {
		if (poly1.length() < 3 || poly2.length() < 3)
			return false;
		Vec2 bestAxis{0, 0};
		float minDepth = std::numeric_limits<float>::max();

		auto checkAxes = [&](cArray<Vec2>& A, cArray<Vec2>& B) {
			for (int i = 0; i < A.length(); i++) {
				const Vec2& p1 = A[i];
				const Vec2& p2 = A[(i + 1) % A.length()];
				Vec2 edge = p2 - p1;
				Vec2 axis = edge.rotate270z().normalized(); // 单位法线

				// 投影 A
				float minA = A[0].dot(axis), maxA = minA;
				for (int j = 1; j < A.length(); j++) {
					float proj = A[j].dot(axis);
					if (proj < minA) minA = proj;
					else if (proj > maxA) maxA = proj;
				}
				// 投影 B
				float minB = B[0].dot(axis), maxB = minB;
				for (int j = 1; j < B.length(); j++) {
					float proj = B[j].dot(axis);
					if (proj < minB) minB = proj;
					else if (proj > maxB) maxB = proj;
				}

				float overlap = std::min(maxA, maxB) - std::max(minA, minB);
				if (overlap < 0.0f) // 分离轴
					return false;
				// 有重叠（相交情况），记录最小穿透深度
				if (overlap < minDepth) {
					minDepth = overlap;
					bestAxis = axis;
				}
			}
			return true;
		};

		if (!checkAxes(poly1, poly2) || !checkAxes(poly2, poly1)) {
			return false; // 不相交
		}

		if (mtv) {
			// ✅ 统一 MTV 方向
			if ((poly2[0] - poly1[0]).dot(bestAxis) < 0)
				bestAxis = -bestAxis;
			*mtv = { bestAxis, minDepth };
			Qk_ASSERT(mtv->overlap >= 0.0f);
		}

		return true; // 相交
	}

	// GJK 算法检测两个凸多边形是否相交。
	// A, B: 凸多边形的顶点数组，按顺时针或逆时针顺序排列
	// outMTV: 输出最小穿透向量（可选）
	// 返回值: true - 相交, false - 不相交
	bool test_polygon_vs_polygon_gjk(cArray<Vec2>& A, cArray<Vec2>& B, MTV* mtv) {
		auto support = [&](const cArray<Vec2>& P, const Vec2& d) {
			float best = std::numeric_limits<float>::lowest();
			Vec2 res;
			for (uint32_t i = 0; i < P.length(); i++) {
				float v = P[i].dot(d);
				if (v > best) best = v, res = P[i];
			}
			return res;
		};

		auto supportM = [&](const Vec2& d) {
			return support(A, d) - support(B, -d);
		};

		Vec2 dir = {1, 0};
		Vec2 simplex[3];
		int n = 0;

		simplex[n++] = supportM(dir);
		dir = -simplex[0];

		for (int iter = 0; iter < 20; iter++) {
			Vec2 a = supportM(dir);
			if (a.dot(dir) < 0) {
				// no intersection — distance is |dir|
				float dist = dir.length();
				Vec2 axis = dir / dist;
				if (mtv) *mtv = {axis, dist};
				return false; // no overlap
			}

			simplex[n++] = a;

			if (n == 2) {
				Vec2 a = simplex[1];
				Vec2 b = simplex[0];
				Vec2 ab = b - a;
				Vec2 ao = -a;

				// perpendicular toward origin
				dir = {ab.y(), -ab.x()};
				if (dir.dot(ao) < 0) dir = -dir;
			}
			else if (n == 3) {
				Vec2 a = simplex[2];
				Vec2 b = simplex[1];
				Vec2 c = simplex[0];
				Vec2 ao = -a;
				Vec2 ab = b - a;
				Vec2 ac = c - a;

				Vec2 abp = {ab.y(), -ab.x()};
				if (abp.dot(c - b) > 0) abp = -abp;

				if (abp.dot(ao) > 0) {
						simplex[0] = b; simplex[1] = a; n = 2; dir = abp; continue;
				}

				Vec2 acp = {ac.y(), -ac.x()};
				if (acp.dot(b - c) > 0) acp = -acp;

				if (acp.dot(ao) > 0) {
						simplex[1] = a; simplex[0] = c; n = 2; dir = acp; continue;
				}

				// theoretically intersect → but SAT already resolved this case
				if (mtv) *mtv = {{0,0}, 0};
				return true;
			}
		}

		// fallback — shouldn't reach here after SAT precheck
		float dist = dir.length();
		Vec2 axis = dir / dist;
		if (mtv) *mtv = {axis, dist};
		return false;
	}

	// 计算两个凸多边形之间的最短距离及对应的最小移动向量 (MTV)
	// A, B: 凸多边形的顶点数组，按顺时针或逆时针顺序排列
	// mtv: 输出最小移动向量
	// 返回值: true - 多边形重叠（MTV 无效）， false - 多边形不重叠（MTV 有效）
	void test_polygon_vs_polygon_fast(cArray<Vec2>& A, cArray<Vec2>& B, MTV* mtv) {
		float minDist2 = std::numeric_limits<float>::infinity();
		Vec2 bestA, bestB;

		// A 点 -> B 边
		for (uint32_t i=0;i<A.length();i++) {
			const Vec2& p = A[i];
			for (uint32_t j=0;j<B.length();j++) {
				Vec2 a = B[j];
				Vec2 b = B[(j+1)%B.length()];
				Vec2 c = closest_point_on_segment(p, a, b);
				float d2 = (p-c).lengthSq();// dot(p-c, p-c);
				if (d2 < minDist2) minDist2 = d2, bestA = p, bestB = c;
			}
		}
		// B 点 -> A 边
		for (uint32_t i=0;i<B.length();i++) {
			const Vec2& p = B[i];
			for (uint32_t j=0;j<A.length();j++) {
				Vec2 a = A[j];
				Vec2 b = A[(j+1)%A.length()];
				Vec2 c = closest_point_on_segment(p, a, b);
				float d2 = (p-c).lengthSq(); //dot(p-c, p-c);
				if (d2 < minDist2) minDist2 = d2, bestA = c, bestB = p;
			}
		}

		float dist = std::sqrt(minDist2);
		Vec2 dir = (bestB - bestA).normalized(); // A->B
		*mtv = { dir, dist };
	}

	// 由凸四边形的四个顶点计算其轴对齐边界框范围盒 (AABB)
	// quadrilateral: 凸四边形的四个顶点，按顺时针或逆时针顺序排列
	// 返回值: 边界框范围 {min, max}
	Range region_aabb_from_convex_quadrilateral(Vec2* quadrilateral) {
		#define A quadrilateral[0]
		#define B quadrilateral[1]
		#define C quadrilateral[2]
		#define D quadrilateral[3]

		Vec2 min, max;//, size;

		float w1 = fabs(A.x() - C.x());
		float w2 = fabs(B.x() - D.x());

		if (w1 > w2) {
			if ( A.x() > C.x() ) {
				max.set_x( A.x() ); min.set_x( C.x() );
			} else {
				max.set_x( C.x() ); min.set_x( A.x() );
			}
			if ( B.y() > D.y() ) {
				max.set_y( B.y() ); min.set_y( D.y() );
			} else {
				max.set_y( D.y() ); min.set_y( B.y() );
			}
			//size = Vec2(w1, max.y() - min.y());
		} else {
			if ( B.x() > D.x() ) {
				max.set_x( B.x() ); min.set_x( D.x() );
			} else {
				max.set_x( D.x() ); min.set_x( B.x() );
			}
			if ( A.y() > C.y() ) {
				max.set_y( A.y() ); min.set_y( C.y() );
			} else {
				max.set_y( C.y() ); min.set_y( A.y() );
			}
			//size = Vec2(w2, max.y() - min.y());
		}

		#undef A
		#undef B
		#undef C
		#undef D

		return {
			min, max
		};
	}

	// 由多边形的所有顶点计算其轴对齐边界框范围盒 (AABB)
	// poly1: 多边形的所有顶点，按顺时针或逆时针顺序排列
	// 返回值: 边界框范围 {min, max}
	Range region_aabb_from_polygon(cArray<Vec2>& poly) {
		if (poly.length() == 0)
			return {}; // 空多边形返回空范围
		Vec2 min = poly[0], max = poly[0];
		for (uint32_t i = 1; i < poly.length(); i++) {
			const Vec2& p = poly[i];
			if (p.x() < min.x()) min.set_x(p.x());
			if (p.y() < min.y()) min.set_y(p.y());
			if (p.x() > max.x()) max.set_x(p.x());
			if (p.y() > max.y()) max.set_y(p.y());
		}
		return { min, max };
	}

	// ------------------------ 圆近似为四边形 ------------------------
	Array<Vec2> circle_to_quad(Circle circ) {
		// agent approximated as small polygon (circle->approx)
		return Array<Vec2>{
			{circ.center.x() + circ.radius, circ.center.y()},
			{circ.center.x(), circ.center.y() + circ.radius},
			{circ.center.x() - circ.radius, circ.center.y()},
			{circ.center.x(), circ.center.y() - circ.radius}
		};
	}

	// ------------------------ 圆近似为八边形 ------------------------
	Array<Vec2> circle_to_octagon(Circle circ) {
		// 八边形（每隔45°取点）。使用 cos(45°)=sin(45°)=sqrt(2)/2
		const float r = circ.radius;
		const float k = 0.7071067811865476f * r; // sqrt(2)/2 * r
		return Array<Vec2>{
			{circ.center.x() + r,    circ.center.y()       }, // 0°
			{circ.center.x() + k,    circ.center.y() + k   }, // 45°
			{circ.center.x(),        circ.center.y() + r   }, // 90°
			{circ.center.x() - k,    circ.center.y() + k   }, // 135°
			{circ.center.x() - r,    circ.center.y()       }, // 180°
			{circ.center.x() - k,    circ.center.y() - k   }, // 225°
			{circ.center.x(),        circ.center.y() - r   }, // 270°
			{circ.center.x() + k,    circ.center.y() - k   }  // 315°
		};
	}

	// ------------------------ 线段转四边形 ------------------------
	Array<Vec2> segment_to_quad(const LineSegment& seg) {
		// 方向向量
		Vec2 dir = seg.b - seg.a;
		// 归一化方向
		Vec2 n = dir.normalized();
		// 法线向量（垂直方向）
		Vec2 normal = n.rotate270z() * seg.halfTh;

		// 四个顶点：顺时针或逆时针都可以
		Vec2 p1 = seg.a + normal;
		Vec2 p2 = seg.b + normal;
		Vec2 p3 = seg.b - normal;
		Vec2 p4 = seg.a - normal;

		return { p1, p2, p3, p4 };
	}

	// ------------------------ 最近点到线段 ------------------------
	// closestPointOnSegment
	Vec2 closest_point_on_segment(Vec2 p, const Vec2 a, const Vec2 b) {
		Vec2 ab = b - a;
		float denom = ab.lengthSq();
		if (denom <= GEOM_EPS) return a;
		float t = (p - a).dot(ab) / denom;
		if (t < 0.0f) t = 0.0f;
		else if (t > 1.0f) t = 1.0f;
		return a + ab * t;
	}

	// ------------------------ 点到直线的垂线向量 ------------------------
	Vec2 perpendicular_vector_to_line(Vec2 p, Vec2 a, Vec2 b) {
		Vec2 ab = b - a;
		float denom = ab.lengthSq();
		if (denom <= GEOM_EPS) return a;
		float t = (p - a).dot(ab) / denom;  // 投影比例（可能小于0或大于1）
		Vec2 proj = a + ab * t;             // 投影点（垂足）
		return proj - p;                    // 从P指向直线的垂线向量
	}

	// ------------------------ 射线 vs 线段相交检测 ------------------------
	bool ray_segment_intersection(Vec2 p, Vec2 dir, Vec2 a, Vec2 b, Vec2& outPoint) {
		Vec2 v1 = dir; // 线段1的方向向量
		Vec2 v2 = b - a; // 线段2的方向向量
		// 计算方向向量的交叉乘积（分母部分）
		float denom = v1.det(v2);
		if (fabs(denom) < GEOM_EPS) {
			return false; // 如果平行或共线，返回 false
		}
		Vec2 r = a - p; // 线段A到射线起点的向量
		// 计算参数 t 和 u
		float invDenom = 1.0f / denom;
		float t = r.det(v2) * invDenom;
		float u = r.det(v1) * invDenom;
		// 判断交点是否在范围内, 对于射线仅 t >= 0
		if (t >= 0 /*&& t <= 1*/ && u >= 0 && u <= 1) {
			// 计算交点：t 参数化线段1，u 参数化线段2
			outPoint = p + v1 * t;
			return true;
		}
		return false;
	}

	// ------------------------ 圆 vs 厚线段检测（MTV） ------------------------
	// 圆 vs 厚线段检测，返回是否相交，必要时计算分离向量（MTV）
	// circ:   圆体 (center, radius)
	// seg:    线段 (a, b, halfTh)
	// mtv: 最小平移向量 (圆心方向)
	// requestSeparationMTV: 若为 true，则在未相交时返回间距向量
	// axis 始终从圆指向线段，overlap 始终为正
	bool test_circle_vs_line_segment(const Circle& circ, const LineSegment& seg, MTV* mtv, bool requestSeparationMTV) {
		Vec2 closest = closest_point_on_segment(circ.center, seg.a, seg.b);
		Vec2 delta = closest - circ.center; // 指向线段
		float distSq = delta.lengthSq();
		float minDist = circ.radius + seg.halfTh;
		bool collided = (distSq < minDist * minDist);

		if (!collided && (!requestSeparationMTV || !mtv))
			return false;

		float dist = sqrtf(distSq);
		Vec2 axis;
		if (dist > GEOM_EPS)
			// 相交时seg指向circ, 否则 circ指向seg
			axis = delta * (collided ? -1.0f : 1.0f) / dist;
		else {
			// 圆心正好在线段上或极接近：取线段方向的法线
			Vec2 _seg = seg.b - seg.a;
			Vec2 segN = _seg.normalized();
			axis = segN.rotate90z();
			// dist = 0.0f;
		}

		if (mtv) {
			*mtv = { axis, fabsf(minDist - dist) };
		}
		return collided;
	}

	// ------------------------ 多边形 vs 厚线段检测（MTV） ------------------------
	// 多边形 vs 厚线段检测，返回是否相交，必要时计算分离向量（MTV）
	// poly:   多边形顶点数组
	// seg:    线段 (a, b, halfTh)
	// outMTV: 最小平移向量
	// requestSeparationMTV: 若为 true，则在未相交时返回间距向量
	bool test_poly_vs_line_segment(cArray<Vec2>& poly, const LineSegment& seg, MTV* outMTV, bool requestSeparationMTV) {
		// 将线段近似为四边形进行检测
		Array<Vec2> segPoly = segment_to_quad(seg);
		// return test_polygon_vs_polygon_sat(poly, segPoly, outMTV);
		return test_polygon_vs_polygon(poly, segPoly, outMTV, requestSeparationMTV);
	}

	// ------------------------ 圆 vs 圆检测（MTV） ------------------------
	// 其中 mtv->axis 始终指向 otherCirc，mtv->overlap 始终为正数
	bool test_circle_vs_circle(const Circle& circ, const Circle& otherCirc, MTV* mtv, bool requestSeparationMTV) {
		Vec2 delta = otherCirc.center - circ.center; // 指向 otherCirc
		float distSq = delta.lengthSq();
		float minDist = circ.radius + otherCirc.radius;
		bool collided = (distSq < minDist * minDist);

		if (!collided && (!requestSeparationMTV || !mtv))
			return false; // 不相交，且不要求输出MTV

		float overlap;
		float dist = sqrtf(distSq);
		Vec2 axis;
		if (dist > GEOM_EPS)
			// 相交时otherCirc指向circ（推开）, 否则 circ指向otherCirc（拉近）
			axis = delta * (collided ? -1.0f : 1.0f) / dist; 
		else {
			// 圆心重合或极近，选任意方向，这里选X轴正方向（推开）
			axis = Vec2{1.0f, 0.0f};
			// dist = 0.0f;
		}

		if (mtv)
			*mtv = { axis, fabsf(minDist - dist)  };
		return collided;
	}

	bool test_circle_vs_polygon(const Circle& circ, cArray<Vec2>& polyB, MTV* outMTV, bool requestSeparationMTV) {
		// 将圆近似为八边形进行检测
		Array<Vec2> circPoly = circle_to_octagon(circ);
		return test_polygon_vs_polygon(circPoly, polyB, outMTV, requestSeparationMTV);
	}

	// ------------------------ 多边形 vs 多边形检测（MTV） ------------------------
	// 返回是否相交，并输出最小分离向量（MTV）
	bool test_polygon_vs_polygon(cArray<Vec2>& polyA, cArray<Vec2>& polyB, MTV* outMTV, bool requestSeparationMTV) {
		bool collided = test_polygon_vs_polygon_sat(polyA, polyB, outMTV);
		if (collided) {
			return true;
		} else if (requestSeparationMTV && outMTV) {
			// 计算分离向量
			test_polygon_vs_polygon_fast(polyA, polyB, outMTV);
			//bool gjkResult = test_polygon_vs_polygon_gjk(polyA, polyB, outMTV);
			//Qk_ASSERT_EQ(gjkResult, false); // 已经确认不相交
		}
		return false;
	}

	// ------------------------ 预测前方距离（基于方向 dir）
	// 返回到障碍边缘的沿方向的可用距离（如果不会阻挡，返回 +inf）
	// 简化版：使用线段中点估计
	float predict_forward_distance_circ_to_seg_fast(const Circle& circ, Vec2 dir, const LineSegment& seg, float safetyBuf) {
		Vec2 toObs = (seg.a + seg.b) * 0.5f - circ.center; // 近似使用中点估计（更精确可投影多点）
		float forward = toObs.dot(dir);
		if (forward <= 0.0f)
			return std::numeric_limits<float>::infinity();
		// lateral distance from line (exact)
		Vec2 closest = closest_point_on_segment(circ.center, seg.a, seg.b);
		Vec2 rel = closest - circ.center; // pos到最近点的向量
		float lateral = sqrtf(std::max(0.0f, rel.lengthSq() - forward*forward)); // 或直接 (rel - dir*forward).len()
		float safe = circ.radius + seg.halfTh + safetyBuf;
		// better: compute perpendicular distance directly:
		Vec2 along = dir * forward;
		Vec2 lateralVec = closest - (circ.center + along);
		float lat = lateralVec.length();
		if (lat < safe) {
			// distance along path to obstacle edge:
			// distanceFromPosToClosestAlongDir = forward - projection of seg thickness along dir => approximate:
			return forward - safe;
		}
		return std::numeric_limits<float>::infinity();
	}

	// 更精确版：使用射线与线段交点计算，基于垂线长度比例缩放计算
	float predict_forward_distance_circ_to_seg(const Circle& circ, Vec2 dir, const LineSegment& seg, float safetyBuf) {
		Vec2 obs;
		if (!ray_segment_intersection(circ.center, dir, seg.a, seg.b, obs))
			return std::numeric_limits<float>::infinity(); // 无交点
		Vec2 toPerp = perpendicular_vector_to_line(circ.center, seg.a, seg.b); // 垂线向量
		float perpLen = toPerp.length(); // 垂线长度
		float perpSafe = perpLen - (circ.radius + seg.halfTh + safetyBuf); // 安全垂线长度
		if (perpSafe <= 0.0f)
			return perpSafe; // 已相交或非常接近
		Vec2 toObs = obs - circ.center; // 从位置到交点的向量
		float safeDist = toObs.length() * perpSafe / perpLen; // 比例缩放得到安全距离
		return safeDist;
	}

	// 更精确版2：直接计算沿 dir 的安全距离，基于垂线长度与方向投影计算
	float predict_forward_distance_circ_to_seg2(const Circle& circ, Vec2 dir, const LineSegment& seg, float safetyBuf) {
		Vec2 obs;
		if (!ray_segment_intersection(circ.center, dir, seg.a, seg.b, obs))
			return std::numeric_limits<float>::infinity(); // 无交点
		Vec2 toPerp = perpendicular_vector_to_line(circ.center, seg.a, seg.b); // 垂线向量
		float perpLenSq = toPerp.lengthSq(); // 垂线长度平方
		float perpLen = sqrtf(perpLenSq); // 垂线长度
		float safe = circ.radius + seg.halfTh + safetyBuf; // 安全距离
		if (perpLen < safe)
			return perpLen - safe; // 已经在墙内或紧贴
		float safeDist = (perpLenSq - safe * perpLen) / toPerp.dot(dir); // 计算沿 dir 的安全距离
		return safeDist;
	}

	// ------------------------ 多边形 vs 线段预测距离 ------------------------
	// 返回到障碍边缘的沿方向的可用距离（如果不会阻挡，返回 +inf）
	// 简化版：使用质心估计
	float predict_forward_distance_poly_to_seg(cArray<Vec2>& poly, Vec2 dir, const LineSegment& seg, float safetyBuf, Vec2* outCentroid) {
		// 将线段近似为四边形进行检测
		Array<Vec2> segPoly = segment_to_quad(seg);
		return predict_forward_distance_poly_to_poly(poly, dir, segPoly, safetyBuf, outCentroid, nullptr);
	}

	// ------------------------ 预测前方距离到圆形障碍 ------------------------
	// 返回到障碍边缘的沿方向的可用距离（如果不会阻挡，返回 +inf）
	float predict_forward_distance_circ_to_circ(const Circle& circ, Vec2 dir, const Circle& obsCirc, float safetyBuf) {
		Vec2 toObs = obsCirc.center - circ.center;
		float forward = toObs.dot(dir);
		if (forward <= 0.0f)
			return std::numeric_limits<float>::infinity();
		float lateralSq = toObs.lengthSq() - forward * forward;
		float safe = circ.radius + obsCirc.radius + safetyBuf;
		if (lateralSq < safe * safe) {
			// distance along path to obstacle edge:
			float d = forward - sqrtf(safe * safe - lateralSq);
			return d;
		}
		return std::numeric_limits<float>::infinity();
	}

	// ------------------------ 预测前方距离到多边形障碍 ------------------------
	// 简化版：使用质心估计
	float predict_forward_distance_circ_to_poly(const Circle& circ, Vec2 dir, cArray<Vec2>& poly, float safetyBuf, Vec2* outCentroid) {
		// Use polygon centroid as heuristic, and compute lateral distance to centroid projection.
		// (For precise use you can project all polygon vertices or use Minkowski-based method.)
		Vec2 centroid{0,0};
		for (auto v : poly) centroid += v;
		centroid = centroid * (1.0f / poly.length());
		if (outCentroid) *outCentroid = centroid;
		Vec2 toObs = centroid - circ.center;
		float forward = toObs.dot(dir);
		if (forward <= 0.0f) return std::numeric_limits<float>::infinity();
		Vec2 projPoint = circ.center + dir * forward;
		// compute distance from projPoint to polygon (approx by vertex-to-proj min)
		float minDist = std::numeric_limits<float>::infinity();
		for (auto v : poly) {
			float d = (v - projPoint).length();
			if (d < minDist) minDist = d;
		}
		float safe = circ.radius + safetyBuf;
		if (minDist < safe) return forward - safe;
		return std::numeric_limits<float>::infinity();
	}

	// ------------------------ 多边形 vs 多边形预测距离（简化质心版） ------------------------
	float predict_forward_distance_poly_to_poly(cArray<Vec2>& polyA, Vec2 dir,
			cArray<Vec2>& polyB, float safetyBuf, Vec2* outCentroidA, Vec2* outCentroidB)
	{
		// 1. 计算两个多边形质心
		Vec2 centroidA{0,0}, centroidB{0,0};
		for (auto v : polyA) centroidA += v;
		for (auto v : polyB) centroidB += v;
		centroidA = centroidA * (1.0f / polyA.length());
		centroidB = centroidB * (1.0f / polyB.length());
		if (outCentroidA) *outCentroidA = centroidA;
		if (outCentroidB) *outCentroidB = centroidB;

		// 2. 方向上是否在前方
		Vec2 toObs = centroidB - centroidA;
		float forward = toObs.dot(dir);
		if (forward <= 0.0f)
			return std::numeric_limits<float>::infinity(); // 障碍在后方

		// 3. 估计当前横向距离（最小顶点-顶点距离近似）
		float minDist = std::numeric_limits<float>::infinity();
		for (auto vA : polyA) {
			for (auto vB : polyB) {
				float d = (vA - vB).length();
				if (d < minDist) minDist = d;
			}
		}

		// 4. 如果横向距离小于安全距离，则可能会碰到
		float safe = safetyBuf;
		if (minDist < safe)
			return forward - safe; // 提前量（可为负）

		return std::numeric_limits<float>::infinity();
	}

} // namespace qk
