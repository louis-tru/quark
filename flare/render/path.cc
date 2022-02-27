/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "tesselator.h"
#include "./path.h"
#include "../util/handle.h"
#include "../util/bezier.h"

namespace flare {

	PathLine PathLine::Oval(Rect r) {
		Vec2 top(r.origin.x() + r.size.x() / 2, r.origin.y());

		PathLine path(top); // start

		path.add_conic(
			Vec2(r.origin.x() + r.size.x(), r.origin.y()),
			Vec2(r.origin.x() + r.size.x(), r.origin.y() + r.size.y() / 2)); // top,right

		path.add_conic(
			r.origin + r.size,
			Vec2(r.origin.x() + r.size.x() / 2, r.origin.y() + r.size.y())); // right,bottom

		path.add_conic(
			Vec2(r.origin.x(), r.origin.y() + r.size.y()),
			Vec2(r.origin.x(), r.origin.y() + r.size.y() / 2)); // bottom,left

		path.add_conic(r.origin, top); // left,top

		return path;
	}

	PathLine PathLine::Rect(Rect r) {
		PathLine path(r.origin);
		float x2 = r.origin.x() + r.size.x();
		float y2 = r.origin.y() + r.size.y();
		path.add_quad(
			Vec2(x2, r.origin.y()),
			Vec2(x2, y2), Vec2(r.origin.x(), y2)
		);
		return path;
	}

	PathLine PathLine::Circle(Vec2 center, float radius) {
		return Oval({ Vec2(center.x() - radius / 2, center.y() - radius / 2), Vec2(radius) * 2 });
	}

	PathLine::PathLine(Vec2 move) {
		add_move(move);
	}

	PathLine::PathLine(Vec2* pts, int len, PathVerb* verbs, int verbsLen) {
		F_ASSERT(verbs[0] == kVerb_Move);
		_pts.write((float*)pts, -1, len * 2);
		_verbs.write((uint8_t*)verbs, -1, verbsLen);
	}

	PathLine::PathLine() {}

	void PathLine::add_move(Vec2 to) {
		_pts.push(to);
		_verbs.push(kVerb_Move);
	}

	void PathLine::add_line(Vec2 to) {
		// _pts.push(to);
		_pts.write(p1.val, -1, 2);
		_verbs.push(kVerb_Line);
	}

	void PathLine::add_quad(Vec2 p1, Vec2 p2, Vec2 p3) {
		// _pts.push(p1);
		// _pts.push(p2);
		// _pts.push(p3);
		// _pts.push(_pts[_pts.length() - 4]); // close
		_pts.write(p1.val, -1, 6);
		_pts.write(_pts.val()[_pts.length() - 8], -1, 2); // close
		_verbs.push(kVerb_Quad);
	}

	void PathLine::add_conic(Vec2 p1, Vec2 to) {
		//_pts.push(p1);
		//_pts.push(to);
		// _pts.write(p1.val, -1, 4);
		// _verbs.push(kVerb_Conic);
	}

	void PathLine::add_quadratic(Vec2 control, Vec2 to) {
		_pts.write(control.val, -1, 4);
		_verbs.push(kVerb_Quadratic);
	}

	void PathLine::add_cubic(Vec2 control1, Vec2 control2, Vec2 to) {
		//_pts.push(control1[0]); _pts.push(control1[1]);
		//_pts.push(control2[0]); _pts.push(control2[1]);
		//_pts.push(to[0]); _pts.push(to[1]);
		_pts.write(control1.val, -1, 6);
		_verbs.push((uint8_t)kVerb_Cubic);
	}

	Array<Vec2> PathLine::to_polygon(int polySize) const {
		F_ASSERT(_verbs[0] == kVerb_Move);

		TESStesselator* tess = tessNewTess(nullptr);
		ClearScope clear([tess]() { tessDeleteTess(tess); });

		const Vec2* pts = (const Vec2*)*_pts;
		Array<Vec2> polygons, tmpV;
		int len = 0;

		for (auto verb: _verbs) {
			switch(verb) {
				case kVerb_Move:
					if (len) {
						tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
						len = 1;
					}
					tmp.push(*pts++);
					break;
				case kVerb_Line:
					tmp.push(*pts++);
					len++;
					break;
				case kVerb_Quad: {
					tmp.push(*pts++);
					tmp.push(*pts++);
					tmp.push(*pts++);
					tmp.push(*pts++); // close quad
					len += 4;
					break;
				}
				case kVerb_Conic: {
					// #define SK_ScalarRoot2Over2         0.707106781f
					break;
				}
				case kVerb_Quadratic: { // quadratic
					// F_DEBUG("conic_to:%f,%f|%f,%f", pts[0].x(), pts[0].y(), pts[1].x(), to[1].y());
					QuadraticBezier bezier(pts[-1], *pts++, *pts++);
					int sample = PathLine::get_quadratic_bezier_sample(bezier);
					tmpV.extend(tmpV.length() + sample - 1);
					bezier.sample_curve_points(sample, (float*)&tmpV[tmpV.length() - sample]);
					len += sample - 1;
					break;
				}
				case kVerb_Cubic: {// cubic
					//  F_DEBUG("cubic_to:%f,%f|%f,%f|%f,%f",
					//           pts[0].x(), pts[0].y(), pts[1].x(), to[1].y(), pts[2].x(), to[2].y());
					CubicBezier bezier(pts[-1], *pts++, *pts++, *pts++);
					int sample = PathLine::get_cubic_bezier_sample(bezier);
					tmpV.extend(tmpV.length() + sample - 1);
					bezier.sample_curve_points(sample, (float*)&tmpV[tmpV.length() - sample]);
					len += sample - 1;
					break;
				}
				default: // close
					break;
			}
		}

		if (len) { // closure
			tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
		}

		// Convert to convex contour vertex data
		if ( tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_CONNECTED_POLYGONS, polySize, 2, 0) ) {
			const int nelems = tessGetElementCount(tess);
			const TESSindex* elems = tessGetElements(tess);
			const TESSreal* verts = tessGetVertices(tess);
			for (int i = 0; i < nelems * polySize; i++) {
				float x = verts[elems[0]];
				float y = verts[elems[1]];
				polygons.push(Vec2(x, y));
				elems += 2;
			}
		}

		return polygons;
	}

	Array<Vec2> PathLine::to_edge_line() const {
		F_ASSERT(_verbs[0] == kVerb_Move);

		const Vec2* pts = ((const Vec2*)*_pts) - 1;
		Array<Vec2> edges;

		for (auto verb: _verbs) {
			switch(verb) {
				case kVerb_Move:
					pts++;
					break;
				case kVerb_Line:
					edges.push(*pts++); edges.push(*pts); // edge 0
					break;
				case kVerb_Quad: {
					Vec2 p0 = pts[0];
					edges.push(*pts++); edges.push(*pts); // edge 0
					edges.push(*pts++); edges.push(*pts); // edge 1
					edges.push(*pts++); edges.push(*pts); // edge 2
					edges.push(*pts++); edges.push(*pts); // edge 3
					break;
				}
				case kVerb_Quadratic: { // Quadratic
					//  F_DEBUG("conic_to:%f,%f|%f,%f", pts[0].x(), pts[0].y(), pts[1].x(), to[1].y());
					QuadraticBezier bezier(*pts++, *pts++, *pts++);
					int sample = PathLine::get_quadratic_bezier_sample(bezier);
					auto points = bezier.sample_curve_points(sample);
					for (int i = 0; i < sample - 1; i++) {
						edges.push(points[i]); edges.push(points[i + 1]); // add edge line
					}
					break;
				}
				case kVerb_Cubic: { // cubic
					//  F_DEBUG("cubic_to:%f,%f|%f,%f|%f,%f",
					//           pts[0].x(), pts[0].y(), pts[1].x(), to[1].y(), pts[2].x(), to[2].y());
					CubicBezier bezier(*pts++, *pts++, *pts++, *pts++);
					int sample = PathLine::get_cubic_bezier_sample(bezier);
					auto points = bezier.sample_curve_points(sample);
					for (int i = 0; i < sample - 1; i++) {
						edges.push(points[i]); edges.push(points[i + 1]); // add edge line
					}
					break;
				}
				default: // close
					break;
			}
		}

		return edges;
	}

	Array<Vec2i> PathLine::to_edge_line_i() const {
		Array<Vec2i> edges;
		for (auto it: to_edge_line())
			edges.push(Vec2i(int(it[0]), int(it[1])));
		return edges;
	}

	void PathLine::transfrom(const Mat& matrix) {
		float* pts = *_pts;
		float* e = pts + _pts.length();
		while (pts < e) {
			*((Vec2*)pts) = matrix * (*(Vec2*)pts);
			pts += 2;
		}
	}

	void PathLine::scale(Vec2 scale) {
		float* pts = *_pts;
		float* e = pts + _pts.length();
		while (pts < e) {
			pts[0] *= scale[0];
			pts[1] *= scale[1];
			pts += 2;
		}
	}

	PathLine PathLine::reduce() const {
		F_ASSERT(_verbs[0] == kVerb_Move);

		const Vec2* pts = ((const Vec2*)*_pts) - 1;
		PathLine line;

		for (auto verb: _verbs) {
			switch(verb) {
				case kVerb_Move:
					line.add_move(*pts++);
					break;
				case kVerb_Line:
					line.add_line(*pts++);
					break;
				case kVerb_Quad:
					line.add_quad(*pts++, *pts++, *pts++); pts++;
					break;
				case kVerb_Quadratic: { // quadratic
					QuadraticBezier bezier(*pts++, *pts++, *pts++);
					int sample = PathLine::get_quadratic_bezier_sample(bezier) - 1;
					line._pts.extend(line._pts.length() + sample * 2);
					bezier.sample_curve_points(sample, &line._pts[line._pts.length() - sample * 2 - 2]);
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kVerb_Line, sample);
					break;
				}
				case kVerb_Cubic: { // cubic
					CubicBezier bezier(*pts++, *pts++, *pts++, *pts++);
					int sample = PathLine::get_cubic_bezier_sample(bezier) - 1;
					line._pts.extend(line._pts.length() + sample * 2);
					bezier.sample_curve_points(sample, &line._pts[line._pts.length() - sample * 2 - 2]);
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kVerb_Line, sample);
					break;
				}
				default: // close
					line._verbs.push(kVerb_Close);
					break;
			}
		}

		return line;
	}

	PathLine PathLine::clip(const PathLine& path) const {
		// TODO ...
		return PathLine();
	}

	int PathLine::get_quadratic_bezier_sample(const QuadraticBezier& curve) const {
		// TODO ...
		return 10;
	}

	int PathLine::get_cubic_bezier_sample(const CubicBezier& curve) const {
		// TODO ...
		return 10;
	}

}