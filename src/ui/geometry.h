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

#ifndef __quark__ui__geometry__
#define __quark__ui__geometry__

#include "../render/math.h"
#include "../util/array.h"

namespace qk {

	struct LineSegment {
		Vec2 a, b;
		float halfTh;
	};

	struct Circle {
		Vec2 center; // center point
		float radius;
	};

	// MinimumTranslationVector
	struct MTV {
		Vec2 axis;
		float overlap;
	};

	/**
	* @note Determine if a point is inside a convex quadrilateral using the
	* cross product method. The quadrilateral is defined by its four vertices,
	* which should be provided in either clockwise or counterclockwise order. The
	* function calculates the cross products of vectors formed by the edges of
	* the quadrilateral and the vector from each vertex to the point in question. If
	* the signs of all cross products are the same (all positive or all negative),
	* the point is inside the quadrilateral; otherwise, it is outside.
	*/
	Qk_EXPORT bool test_overlap_from_convex_quadrilateral(Vec2 quadrilateral[4], Vec2 point);

	/**
	 * @note Determine if a point is inside a convex polygon using the
	 * cross product method. The polygon is defined by its vertices,
	 * which should be provided in either clockwise or counterclockwise order. The
	 * function calculates the cross products of vectors formed by the edges of
	 * the polygon and the vector from each vertex to the point in question. If
	 * the signs of all cross products are the same (all positive or all negative),
	 * the point is inside the polygon; otherwise, it is outside.
	 */
	Qk_EXPORT bool test_overlap_from_convex_polygon(cArray<Vec2>& polygon, Vec2 point);

	/**
	 * @note Determine if a point is inside a polygon (convex or concave)
	 * using the ray-casting algorithm. The polygon is defined by its vertices,
	 * which should be provided in either clockwise or counterclockwise order.
	 * The function counts how many times a horizontal ray, extending to the right
	 * from the point in question, intersects with the edges of the polygon.
	 * If the count of intersections is odd, the point is inside the polygon;
	 * if even, the point is outside.
	 */
	Qk_EXPORT bool test_overlap_from_polygon(cArray<Vec2>& polygon, Vec2 point);

	/**
	 * @note Determine if two convex polygons overlap using the Separating Axis Theorem (SAT).
	 * The function takes two convex polygons defined by their vertices and checks for overlap
	 * by projecting the polygons onto potential separating axes derived from the edges of both polygons.
	 * If a separating axis is found where the projections do not overlap, the polygons do not collide.
	 * If no such axis exists, the polygons overlap. Optionally, the function can return the minimum
	 * translation vector needed to separate the polygons if they are found to be overlapping.
	 * @param poly1 The vertices of the first convex polygon.
	 * @param poly2 The vertices of the second convex polygon.
	 * @param outMTV Optional output parameter to receive the minimum translation vector
	 *               needed to separate the polygons if they overlap.
	 * @return true if the polygons overlap, false otherwise.
	*/
	Qk_EXPORT bool test_polygon_vs_polygon_sat(cArray<Vec2>& poly1, cArray<Vec2>& poly2, MTV* outMTV = nullptr);

	/**
	 * @brief GJK closest-distance / overlap test for convex polygons.
	 *
	 * If polygons overlap, returns MTV=zero (use SAT for penetration resolution).
	 * If polygons do NOT overlap, returns the **shortest separation vector**
	 * that moves A to touch B.
	 *
	 * ⚠️ Semantics:
	 * - Convex polygons only
	 * - Designed for **separated** shapes
	 * - Returns real geometric distance (not projection depth)
	 * - MTV.axis always points **from A to B**
	 *
	 * Recommended use:
	 * - Use SAT first for penetration resolution
	 * - Use GJK only when SAT reports "no overlap"
	 *
	 * @param A     Polygon A
	 * @param B     Polygon B
	 * @param mtv   Output MTV {axis, distance} if separated
	 * @return true Overlapping (MTV = {0,0})
	 * @return false Separated (MTV valid)
	 */
	Qk_EXPORT bool test_polygon_vs_polygon_gjk(cArray<Vec2>& A, cArray<Vec2>& B, MTV* mtv = nullptr);

	/**
	 * @brief Fast approximate closest-distance MTV between convex polygons.
	 *
	 * Computes a separation vector using point-to-edge distance tests.
	 * Faster than GJK, less robust numerically.
	 *
	 * ⚠️ Semantics:
	 * - Convex polygons only
	 * - Only valid when shapes are **not overlapping**
	 * - Returns smallest distance to separate A from B
	 * - MTV.axis always points **from A to B**
	 *
	 * Use when:
	 * - You need very fast steering / avoidance / wall-sliding
	 * - You don't need exact GJK precision
	 *
	 * @param A     Polygon A
	 * @param B     Polygon B
	 * @param mtv   Output MTV {axis, distance}
	 */
	Qk_EXPORT void test_polygon_vs_polygon_fast(cArray<Vec2>& A, cArray<Vec2>& B, MTV* mtv);

	/**
	 * @method region_from_convex_quadrilateral
	 * @note Calculate the axis-aligned bounding box (AABB) that fully contains
	 * the given convex quadrilateral. The AABB is defined by its minimum and
	 * maximum corners, which are returned as a Range object.
	 * @param quadrilateral An array of four Vec2 points representing the vertices
	 *                      of the convex quadrilateral.
	 * @return A Range object representing the axis-aligned bounding box of the quadrilateral.
	*/
	Qk_EXPORT Range region_aabb_from_convex_quadrilateral(Vec2 quadrilateral[4]);

	/**
	 * @method region_from_polygon
	 * @note Calculate the axis-aligned bounding box (AABB) that fully contains
	 * the given polygon. The AABB is defined by its minimum and maximum corners,
	 * which are returned as a Range object.
	 * @param poly1 A cArray of Vec2 points representing the vertices of the polygon.
	 * @return A Range object representing the axis-aligned bounding box of the polygon.
	*/
	Qk_EXPORT Range region_aabb_from_polygon(cArray<Vec2>& poly);

	// ------------------------ 圆转四边形 ------------------------
	Qk_EXPORT Array<Vec2> circle_to_quad(Circle circ);
	// ------------------------ 圆转八边形 ------------------------
	Qk_EXPORT Array<Vec2> circle_to_octagon(Circle circ);
	// ------------------------ 线段转四边形 ------------------------
	Qk_EXPORT Array<Vec2> segment_to_quad(const LineSegment& seg);
	// ------------------------ 最近点到线段 ------------------------
	Qk_EXPORT Vec2 closest_point_on_segment(Vec2 p, Vec2 a, Vec2 b);
	// ------------------------ 点到直线的垂线向量 ------------------------
	Qk_EXPORT Vec2 perpendicular_vector_to_line(Vec2 p, Vec2 a, Vec2 b);
	// ------------------------ 射线 vs 线段相交检测 ------------------------
	Qk_EXPORT bool ray_segment_intersection(Vec2 p, Vec2 dir, Vec2 a, Vec2 b, Vec2& outPoint);
	// ------------------------ 圆 vs 厚线段检测（MTV） ------------------------
	Qk_EXPORT bool test_circle_vs_line_segment(const Circle& circ, const LineSegment& seg, MTV* out = nullptr, bool requestSeparationMTV = false);
	// ------------------------ 多边形 vs 线段检测（MTV） ------------------------
	Qk_EXPORT bool test_polygon_vs_line_segment(cArray<Vec2>& poly, const LineSegment& seg, MTV* out = nullptr, bool requestSeparationMTV = false);
	// ------------------------ 圆 vs 圆检测（MTV） ------------------------
	Qk_EXPORT bool test_circle_vs_circle(const Circle& circ, const Circle& otherCirc, MTV* out = nullptr, bool requestSeparationMTV = false);
	// ------------------------ 圆 vs 多边形检测（MTV） ------------------------
	Qk_EXPORT bool test_circle_vs_polygon(const Circle& circ, cArray<Vec2>& polyB, MTV* out = nullptr, bool requestSeparationMTV = false);
	// ------------------------ 多边形 vs 多边形检测（MTV） ------------------------
	Qk_EXPORT bool test_polygon_vs_polygon(cArray<Vec2>& polyA, cArray<Vec2>& polyB, MTV* out = nullptr, bool requestSeparationMTV = false);
	// ------------------------ 预测前方距离（基于方向 dir）（如果不会阻挡，返回 +inf）------------------------
	Qk_EXPORT float predict_forward_distance_circ_to_seg_fast(const Circle& circ, Vec2 dir, const LineSegment& seg, float safetyBuf = 0.0f);
	// ------------------------ 预测前方距离到线段障碍 ------------------------
	Qk_EXPORT float predict_forward_distance_circ_to_seg(const Circle& circ, Vec2 dir, const LineSegment& seg, float safetyBuf = 0.0f);
	// ------------------------ 多边形 vs 线段预测距离 ------------------------
	Qk_EXPORT float predict_forward_distance_poly_to_seg(cArray<Vec2>& poly, Vec2 dir, const LineSegment& seg, float safetyBuf = 0.0f, Vec2* outCentroid = nullptr);
	// ------------------------ 预测前方距离到圆形障碍 ------------------------
	Qk_EXPORT float predict_forward_distance_circ_to_circ(const Circle& circ, Vec2 dir, const Circle& obsCirc, float safetyBuf = 0.0f);
	// ------------------------ 预测前方距离到多边形障碍 ------------------------
	Qk_EXPORT float predict_forward_distance_circ_to_poly(const Circle& circ, Vec2 dir, cArray<Vec2>& poly, float safetyBuf = 0.0f, Vec2* outCentroid = nullptr);
	// ------------------------ 多边形 vs 多边形预测距离（简化质心版） ------------------------
	Qk_EXPORT float predict_forward_distance_poly_to_poly(cArray<Vec2>& polyA, Vec2 dir, cArray<Vec2>& polyB, float safetyBuf = 0.0f, Vec2* outCentroidA = nullptr, Vec2* outCentroidB = nullptr);
}

#endif
