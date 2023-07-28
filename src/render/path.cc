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

#include "tesselator.h"
#include "./path.h"
#include "./bezier.h"
#include "../util/handle.h"
#include <math.h>

namespace qk {

	Path Path::MakeOval(const Rect& r, bool ccw) {
		Path path;
		path.ovalTo(r, ccw);
		Qk_ReturnLocal(path);
	}

	Path Path::MakeArc(const Rect& r, float startAngle, float sweepAngle, bool useCenter, bool close) {
		Path path;
		path.arcTo(r, startAngle, sweepAngle, useCenter);
		if (close)
			path.close();
		Qk_ReturnLocal(path);
	}

	Path Path::MakeRect(const Rect& r, bool ccw) {
		Path path;
		path.rectTo(r,ccw);
		Qk_ReturnLocal(path);
	}

	Path Path::MakeCircle(Vec2 center, float radius, bool ccw) {
		return MakeOval({ Vec2(center.x() - radius, center.y() - radius), Vec2(radius) * 2 }, ccw);
	}

	static void setRRect(Path &path,
		const Rect& outside, const Rect *inside, const Path::BorderRadius& br)
	{
		if (outside.size.is_zero()) return;
		if (inside && inside->size.is_zero()) return;

		auto arc = [&](Vec2 origin, Vec2 radius, Vec2 dir, float startAngle, float sweepAngle) {
			if (radius.x() != 0 && radius.y() != 0) {
				Vec2 s = radius*2;
				path.arcTo({origin+s*dir, s}, startAngle, sweepAngle, false);
			} else {
				auto len = path.verbsLen();
				if (len && path.verbs()[len] != Path::kVerb_Close) {
					path.lineTo(origin);
				} else {
					path.moveTo(origin);
				}
			}
		};

		Vec2 a = br.leftTop;
		Vec2 b = br.rightTop;
		Vec2 c = br.rightBottom;
		Vec2 d = br.leftBottom;
		Vec2 origin = outside.origin;
		Vec2 end = origin + outside.size;

		// cw
		arc(origin, a, Vec2(0), Qk_PI, -Qk_PI_2_1); // left-top
		arc({end.x(), origin.y()}, b, Vec2(-1,0), Qk_PI_2_1, -Qk_PI_2_1); // right-top
		arc(end, c, Vec2(-1), 0, -Qk_PI_2_1); // right-bottom
		arc({origin.x(), end.y()}, d, Vec2(0,-1), -Qk_PI_2_1, -Qk_PI_2_1); // left-bottom

		path.close();

		if (!inside)
			return;

		Vec2 inside_origin = inside->origin;
		Vec2 inside_end = inside_origin + inside->size;

		float top    = inside_origin.y() - origin.y();
		float right  = end.x() - inside_end.x();
		float bottom = end.y() - inside_end.y();
		float left   = inside_origin.x() - origin.x();

		origin = inside_origin;
		end    = inside_end;

		a = { Float::max(a.x() - left, 0.0), Float::max(a.y() - top, 0.0) }; // left/top
		b = { Float::max(b.x() - right, 0.0), Float::max(b.y() - top, 0.0) }; // left/bottom
		c = { Float::max(c.x() - right, 0.0), Float::max(c.y() - bottom, 0.0) }; // right/bottom
		d = { Float::max(d.x() - left, 0.0), Float::max(d.y() - bottom, 0.0) }; // right/top

		// ccw
		arc(origin, a, Vec2(0), Qk_PI_2_1, Qk_PI_2_1); // left-top
		arc({origin.x(), end.y()}, d, Vec2(0,-1), Qk_PI, Qk_PI_2_1); // left-bottom
		arc(end, c, Vec2(-1), -Qk_PI_2_1, Qk_PI_2_1); // right-bottom
		arc({end.x(), origin.y()}, b, Vec2(-1,0), 0, Qk_PI_2_1); // right-top

		path.close();
	}

	Path Path::MakeRRect(const Rect& rect, const BorderRadius& br)
	{
		Path path;
		setRRect(path, rect, NULL, br);
		Qk_ReturnLocal(path);
	}

	Path Path::MakeRRectOutline(const Rect& outside, const Rect &inside, const BorderRadius &br) {
		Path path;
		setRRect(path, outside, &inside, br);
		Qk_ReturnLocal(path);
	}

	Path::Path(Vec2 move): _IsNormalized(true) {
		moveTo(move);
	}

	Path::Path(): _IsNormalized(true) {}

	void Path::moveTo(Vec2 to) {
		// _pts.push(to.x()); _pts.push(to.y());
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Move);
		_hash.updatefv2(to.val);
	}

	void Path::lineTo(Vec2 to) {
		//if (_pts.length() && *(uint64_t*)&_pts.lastIndexAt(1) == *(uint64_t*)to.val)
		//	return;
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Line);
		_hash.updatefv2(to.val);
	}

	void Path::quadTo(Vec2 control, Vec2 to) {
		_pts.write(control.val, -1, 2);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Quad);
		_IsNormalized = false;
		// _hash.update((&_pts.back()) - 4, sizeof(float) * 4);
		//_hash.update((uint32_t*)(&_pts.back()) - 4, 4);
		_hash.updatefv2(control.val);
		_hash.updatefv2(to.val);
	}

	void Path::cubicTo(Vec2 control1, Vec2 control2, Vec2 to) {
		//_pts.push(control1[0]); _pts.push(control1[1]);
		//_pts.push(control2[0]); _pts.push(control2[1]);
		//_pts.push(to[0]); _pts.push(to[1]);
		_pts.write(control1.val, -1, 2);
		_pts.write(control2.val, -1, 2);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Cubic);
		_IsNormalized = false;
		// _hash.update((uint32_t*)(&_pts.back()) - 6, 6);
		_hash.updatefv2(control1.val);
		_hash.updatefv2(control2.val);
		_hash.updatefv2(to.val);
	}

	constexpr float magicCircle = 0.551915024494f; // 0.552284749831f

	void Path::ovalTo(const Rect& r, bool ccw) {
		if (r.size.is_zero()) return;

		float w = r.size.x(), h = r.size.y();
		float x = r.origin.x(), y = r.origin.y();
		float x2 = x + w * 0.5, y2 = y + h * 0.5;
		float x3 = x + w, y3 = y + h;
		float cx = w * 0.5 * magicCircle, cy = h * 0.5 * magicCircle;
		lineTo(Vec2(x2, y)); // center,top

		if (ccw) {
			float d[] = {x2 - cx, y, x, y2 - cy, x, y2}; cubicTo2(d); // left,top
			float c[] = {x, y2 + cy, x2 - cx, y3, x2, y3}; cubicTo2(c); // bottom,left
			float b[] = {x2 + cx, y3, x3, y2 + cy, x3, y2}; cubicTo2(b); // right,bottom
			float a[] = {x3, y2 - cy, x2 + cx, y, x2, y}; cubicTo2(a); // top,right
		} else {
			float a[] = {x2 + cx, y, x3, y2 - cy, x3, y2}; cubicTo2(a); // top,right
			float b[] = {x3, y2 + cy, x2 + cx, y3, x2, y3}; cubicTo2(b); // right,bottom
			float c[] = {x2 - cx, y3, x, y2 + cy, x, y2}; cubicTo2(c); // bottom,left
			float d[] = {x, y2 - cy, x2 - cx, y, x2, y}; cubicTo2(d); // left,top
		}
	}

	void Path::rectTo(const Rect& r, bool ccw) {
		if (r.size.is_zero()) return;

		lineTo(r.origin);
		float x2 = r.origin.x() + r.size.x();
		float y2 = r.origin.y() + r.size.y();
		if (ccw) {
			lineTo(Vec2(r.origin.x(), y2)); // bottom left
			lineTo(Vec2(x2, y2)); // bottom right
			lineTo(Vec2(x2, r.origin.y())); // top right
		} else {
			lineTo(Vec2(x2, r.origin.y())); // top right
			lineTo(Vec2(x2, y2)); // bottom right
			lineTo(Vec2(r.origin.x(), y2)); // bottom left
		}
		lineTo(r.origin); // top left, origin point
	}

	void Path::arcTo(Vec2 center, Vec2 radius, float startAngle, float sweepAngle, bool useCenter) {
		float cx = center.x();
		float cy = center.y();

		if (radius.is_zero()) {
			lineTo(Vec2(cx, cy));
			return;
		}
		startAngle = -startAngle;
		float rx = radius.x();
		float ry = radius.y();
		float x0 = cosf(startAngle);
		float y0 = sinf(startAngle);

		Vec2 start(x0 * rx + cx, y0 * ry + cy);

		if (useCenter) {
			lineTo(Vec2(cx, cy));
			lineTo(start);
		} else {
			lineTo(start);
		}

		if (sweepAngle == 0) {
			return;
		}

		float n = ceilf(abs(sweepAngle) / Qk_PI_2_1);
		float sweep = sweepAngle / n;
		float magic =
			sweep == Qk_PI_2_1 ? magicCircle:
			sweep == -Qk_PI_2_1 ? -magicCircle:
			tanf(sweep / 4.0f) * 1.3333333333333333f/*4.0 / 3.0*/;

		for (int i = 0, j = n; i < j; i++) {
			startAngle -= sweep;
			float x3 = cosf(startAngle);
			float y3 = sinf(startAngle);
			float x1 = x0 + magic * y0;
			float y1 = y0 - magic * x0;
			float x2 = x3 - magic * y3;
			float y2 = y3 + magic * x3;
			float pts[] = {
				x1 * rx + cx, y1 * ry + cy, // p1
				x2 * rx + cx, y2 * ry + cy, // p2
				x3 * rx + cx, y3 * ry + cy  // p3
			};
			cubicTo2(pts);
			x0 = x3;
			y0 = y3;
		}

		if (useCenter) {
			lineTo(Vec2(cx, cy));
		}
	}

	void Path::arcTo(const Rect& r, float startAngle, float sweepAngle, bool useCenter) {
		auto radius = r.size * 0.5;
		arcTo(r.origin+radius, radius, startAngle, sweepAngle, useCenter);
	}

	void Path::quadTo2(float *p) {
		_pts.write(p, -1, 4);
		_verbs.push(kVerb_Quad);
		_IsNormalized = false;
		_hash.update((uint32_t*)p, 4);
	}

	void Path::cubicTo2(float *p) {
		_pts.write(p, -1, 6);
		_verbs.push(kVerb_Cubic);
		_IsNormalized = false;
		_hash.update((uint32_t*)p, 6);
	}

	void Path::close() {
		_verbs.push(kVerb_Close);
	}

	void Path::concat(const Path& path) {
		_verbs.write(path._verbs);
		_pts.write(path._pts);
		const uint64_t hash = path.hashCode();
		_hash.update(&hash, sizeof(uint64_t));
		_IsNormalized = _IsNormalized && path._IsNormalized;
	}

	Array<Vec2> Path::getEdgeLines(float epsilon) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp,epsilon,false);
		Array<Vec2> edges;
		auto pts = (const Vec2*)*self->_pts;
		bool isZero = true;
		Vec2 move, from;

		for (auto verb: self->_verbs) {
			switch(verb) {
				case kVerb_Move:
					move = from = *pts++;
					isZero = false;
					break;
				case kVerb_Line:
					if (isZero) {
						move = from = *pts++;
						isZero = false;
					} else {
						edges.push(from);
						from = *pts++;
						edges.push(from); // edge 0
					}
					break;
				case kVerb_Close: // close
					//Qk_ASSERT(verb == kVerb_Close);
					if (!isZero) {
						if (move != from) { // close path
							edges.push(from);
							edges.push(move);
						}
						isZero = true;
					}
					break;
				default: Qk_FATAL("Path::getEdgeLines");
			}
		}
		
		Qk_ReturnLocal(edges);
	}

	Array<Vec2> Path::getTriangles(float epsilon) const {
		return getPolygonsFromPaths(this, 1, 3, epsilon);
	}

	Array<Vec2> Path::getPolygonsFromPaths(const Path *paths, int pathsLen, int polySize, float epsilon) {
		auto tess = tessNewTess(nullptr); // TESStesselator*
		
		for (int i = 0; i < pathsLen; i++) {
			Path tmp;
			const Path *self = paths[i]._IsNormalized ?
				paths+i: paths[i].normalized(&tmp, epsilon,false);

			auto pts = (const Vec2*)*self->_pts;
			int len = 0;
			Array<Vec2> tmpV;

			for (auto verb: self->_verbs) {
				switch(verb) {
					case kVerb_Move:
						if (len > 1) { // auto close
							tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
						}
						tmpV.push(*pts++); len = 1;
						break;
					case kVerb_Line:
						//if (len == 0) {
						//	tmpV.push(Vec2(0)); len=1; // use Vec2(0,0) start point
						//}
						tmpV.push(*pts++);
						len++;
						break;
					case kVerb_Close: // close
						// Qk_ASSERT(verb == kVerb_Close);
						if (len) {
							// tmpV.push(tmpV[tmpV.length() - len++]);
							tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
							len = 0;
						}
						break;
					default: Qk_FATAL("Path::getVertexsFromPaths");
				}
			}

			if (len > 1) { // auto close
				tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
			}
		}

		Array<Vec2> vertexs;

		// Convert to convex contour vertex data
		if ( tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, polySize, 2, 0) ) {

			const int nelems = tessGetElementCount(tess);
			const TESSindex* elems = tessGetElements(tess);
			const Vec2* verts = (const Vec2*)tessGetVertices(tess);

			vertexs.extend(nelems * polySize);

			for (int i = 0; i < vertexs.length(); i++) {
				vertexs[i] = verts[*elems++];
			}
		}

		tessDeleteTess(tess);
		Qk_ReturnLocal(vertexs);
	}

	Path Path::dashPath(float *stageP, int stageCount, float offset) const {
		Path tmp, out;
		const Path *self = _IsNormalized ? this: normalized(&tmp, 1,false);
		auto pts = (const Vec2*)*self->_pts;
		int  stageIdx = -1;
		bool useStage = false; // no empty
		bool isZero = true;
		Vec2 move, from;
		float stage = 0;

		auto nextStage = [&]() {
			stageIdx = (stageIdx + 1) % stageCount;
			stage = stageP[stageIdx];
			Qk_ASSERT(stage > 0, "#Path.dashPath.nextStage() assert stage > 0");
			useStage = !useStage;
		};

		if (offset != 0) {
			int step = -1; // back
			if (offset < 0) {// advance
				offset = -offset;
				step = 1;
			} else { // back
				stageIdx = 0;
			}
			do {
				stageIdx = (stageIdx + step + stageCount) % stageCount;
				stage = stageP[stageIdx];
				auto use = Float::min(offset, stage);
				stage -= use;
				offset -= use;
			} while (offset > 0);

			if (step == -1) { // back
				stage = stageP[stageIdx] - stage;
			}
		}

		auto lineTo = [&](Vec2 from, Vec2 to) {
			auto  start = from;
			auto  point = to - from;
			float len = point.length(), useLen = 0;

			while (len > useLen) {
				if (useStage) out.lineTo(from);
				if (stage == 0) {
					nextStage();
					if (useStage) out.moveTo(from);
				}
				float use = Float::min(len - useLen, stage);
				useLen += use; stage -= use;
				from = start + point * Vec2(useLen / len);
			}
		};

		for (auto verb: self->_verbs) {
			switch (verb) {
				case kVerb_Move:
					move = from = *pts++;
					isZero = false;
					break;
				case kVerb_Line:
					if (isZero) {
						move = from = *pts++;
						isZero = false;
					} else {
						lineTo(from, *pts);
						from = *pts++;
					}
					break;
				case kVerb_Close:
					if (!isZero) {
						lineTo(from, move);
						isZero = true;
					}
					break;
				default: Qk_FATAL("Path::dashPath");
			}
		}

		if (out.verbsLen() && useStage) { // continue lineTo while(len > useLen)
			out.lineTo(from);
		}

		Qk_ReturnLocal(out);
	}

	Path Path::normalizedPath(float epsilon) const {
		if (_IsNormalized)
			return *this; // copy self
		Path line;
		normalized(&line, epsilon, true);
		Qk_ReturnLocal(line);
	}

	Path* Path::normalized(Path *out, float epsilon, bool updateHash) const {
		Path &line = *out;
		auto pts = ((const Vec2*)_pts.val());
		bool isZeor = true;

		auto add = [&](Vec2 to, PathVerb verb) {
			line._pts.write(to.val, -1, 2);
			line._verbs.push(verb);
			if (updateHash)
				line._hash.update((uint32_t*)&to, 2);
		};

		for (auto verb: _verbs) {
			switch(verb) {
				case kVerb_Move:
					add(*pts++, kVerb_Move);
					isZeor = false;
					break;
				case kVerb_Line:
					add(*pts++, isZeor ? (isZeor = false),kVerb_Move: kVerb_Line); // add move or line
					break;
				case kVerb_Quad: { // quadratic bezier
					if (isZeor)
						add(Vec2(), kVerb_Move);
					QuadraticBezier bezier(isZeor ? Vec2(): pts[-1], pts[0], pts[1]);
					pts+=2;
					int sample = Path::getQuadraticBezierSample(bezier, epsilon) - 1;
					// |0|1| = sample = 3
					int sampleSize  = sample * 2;
					line._pts.extend(line._pts.length() + sampleSize);
					auto points = &line._pts[line._pts.length() - sampleSize];
					bezier.sample_curve_points(sample+1, points - 2);
					if (updateHash)
						line._hash.update((uint32_t*)points, sampleSize); // update hash
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kVerb_Line, sample);
					isZeor = false;
					break;
				}
				case kVerb_Cubic: { // cubic bezier
					if (isZeor)
						add(Vec2(), kVerb_Move);
					CubicBezier bezier(isZeor ? Vec2(): pts[-1], pts[0], pts[1], pts[2]);
					pts+=3;
					int sample = Path::getCubicBezierSample(bezier, epsilon) - 1;
					// |0|1| = sample = 3
					int sampleSize = sample * 2;
					line._pts.extend(line._pts.length() + sampleSize);
					auto points = &line._pts[line._pts.length() - sampleSize];
					bezier.sample_curve_points(sample+1, points - 2);
					if (updateHash)
						line._hash.update((uint32_t*)points, sampleSize); // update hash
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kVerb_Line, sample);
					isZeor = false;
					break;
				}
				default: // close
					line._verbs.push(kVerb_Close);
					isZeor = true;
					break;
			}
		}
		
		return out;
	}

	void Path::transfrom(const Mat& matrix) {
		float* pts = *_pts;
		float* e = pts + _pts.length();
		while (pts < e) {
			*((Vec2*)pts) = matrix * (*(Vec2*)pts);
			pts += 2;
		}
	}

	void Path::scale(Vec2 scale) {
		float* pts = *_pts;
		float* e = pts + _pts.length();
		while (pts < e) {
			pts[0] *= scale[0];
			pts[1] *= scale[1];
			pts += 2;
		}
	}

	static float sqrt_sqrtf(int i) {
		static Array<float> num;
		if (!num.length()) {
			num.extend(5001);
#if DEBUG
			uint64_t t = qk::time_monotonic();
#endif
			for (int i = 0; i < 5001; i++) {
				num[i] = sqrtf(sqrtf(float(i)));
			}
			Qk_DEBUG("sqrt_sqrtf, %ld", qk::time_monotonic() - t);
		}
		if (i > 5000) {
			return num[5000] * i * 0.0002; // 5000.0
		}
		return num[i];
	}

	int Path::getQuadraticBezierSample(const QuadraticBezier& curve, float epsilon) {
		Vec2 A = curve.p0(), B = curve.p1(), C = curve.p2();

		// calculate triangle area by point cross multiplication

		float S_ABC = (A.x()*B.y() - A.y()*B.x()) + (B.x()*C.y() - B.y()*C.x()) + (C.x()*A.y() - C.y()*A.x());
		float S_2 = abs(S_ABC) * epsilon; // *0.5

		if (S_2 < 5000.0) {
			constexpr float count = 22.0 / 8.408964152537145;
			int i = Uint32::max(sqrt_sqrtf(S_2) * count, 2);
			return i;
		} else {
			return 22;
		}
	}

	int Path::getCubicBezierSample(const CubicBezier& curve, float epsilon) {
		/*
		function get_cubic_bezier_sample(A, B, C, D, epsilon = 1) {
			let S_ABC = (A.x*B.y - A.y*B.x) + (B.x*C.y - B.y*C.x);
			let S_CDA = (C.x*D.y - C.y*D.x) + (D.x*A.y - D.y*A.x);
			let S = (S_ABC + S_CDA) * 0.5 * epsilon;
			return S;
		}
		console.log(get_cubic_bezier_sample({x:0,y:0}, {x:10,y:0}, {x:10,y:10}, {x:0,y:10}));
		*/
		Vec2 A = curve.p0(), B = curve.p1(), C = curve.p2(), D = curve.p3();

		// calculate the area of two triangles by point cross multiplication

		float S_ABC = (A.x()*B.y() - A.y()*B.x()) + (B.x()*C.y() - B.y()*C.x());// + (C.x()*A.y() - C.y()*A.x());
		float S_CDA = (C.x()*D.y() - C.y()*D.x()) + (D.x()*A.y() - D.y()*A.x());// + (A.x()*C.y() - A.y()*C.x());
		float S_2 = abs(S_ABC + S_CDA) * epsilon; // S = S_2 * 0.5

		if (S_2 < 5000.0) { // circle radius < 80
			constexpr float count = 30.0 / 8.408964152537145;//sqrtf(sqrtf(5000.0));
			int i = Uint32::max(sqrt_sqrtf(S_2) * count, 2);
			return i;
		} else {
			return 30;
		}
	}

	static int getSampleFromRect(Vec2 rect, float epsilon, float ratio = 0.5) {
		float S_2 = abs(rect.x() * rect.y() * ratio) * epsilon; // width * height
		if (S_2 < 5000.0) { // circle radius < 80
			constexpr float count = 30.0 / 8.408964152537145;//sqrtf(sqrtf(5000.0));
			int i = Uint32::max(sqrt_sqrtf(S_2) * count, 2);
			return i;
		} else {
			return 30;
		}
	}

	// ------------------- R e c t . O u t l i n e . P a t h -------------------

	RectPath RectPath::MakeRect(const Rect &rect) {
		RectPath out{.rect=rect};
		float x2 = rect.origin.x() + rect.size.x();
		float y2 = rect.origin.y() + rect.size.y();
		// path
		out.path.path.moveTo(rect.origin);
		out.path.path.lineTo(Vec2(x2, rect.origin.y())); // top right
		out.path.path.lineTo(Vec2(x2, y2)); // bottom right
		out.path.path.lineTo(Vec2(rect.origin.x(), y2)); // bottom left
		out.path.path.close(); // top left, origin point
		// vertex
		out.path.vertex.extend(6);
		out.path.vertex[0] = rect.origin;
		out.path.vertex[1] = Vec2(x2, rect.origin.y());
		out.path.vertex[2] = Vec2(x2, y2);
		out.path.vertex[3] = Vec2(x2, y2);
		out.path.vertex[4] = Vec2(rect.origin.x(), y2);
		out.path.vertex[5] = rect.origin;

		Qk_ReturnLocal(out);
	}

	RectPath RectPath::MakeRRect(const Rect &rect, const Path::BorderRadius &r) {
		RectPath out{.rect=rect};

		float x1 = rect.origin.x(),      y1 = rect.origin.y();
		float x2 = x1 + rect.size.x(),   y2 = y1 + rect.size.y();

		/* vertex approximate location
		 ____________0__
		|      /      \ |
		3               |
		|\       /      1
		|__2____________|
		*/
		Vec2 vertex[6] = { // 0,3,2,1,0,2
			{r.rightTop[0]>0 && r.rightTop[1]>0 ? x2-r.rightTop.x(): x2,y1}, // 0
			{x1,y1+r.leftTop.y()}, // 3
			{x1+r.leftBottom.x(),y2}, // 2
			{x2,y2-r.rightBottom.y()}, // 1
		};
		vertex[4] = vertex[0];

		auto build = [](RectPath *out, Vec2 center, Vec2 radius, Vec2 v, Vec2 *v2, float angle) {
			if (radius[0] > 0 && radius[1] > 0) { // no zero
				center = center * radius + v;
				int   sample = getSampleFromRect(radius, 1); // |0|1| = sample = 3
				float angleStep = Qk_PI_2_1 / (sample - 1);
				auto p0 = *v2;
				for (int i = 0; i < sample; i++) {
					Vec2 p(center.x() + cosf(angle) * radius.x(), center.y() - sinf(angle) * radius.y());
					Vec2 src[3] = { p,p0,p };
					out->path.vertex.write(src, -1, i==0?1:3); // add triangle vertex
					out->path.path.lineTo(p);
					angle += angleStep;
				}
				out->path.vertex.pop();
			} else {
				v2[1] = v; // fix next border vertex
				out->path.path.lineTo(v);
			}
		};

		build(&out, {1    }, r.leftTop,     {x1,y1}, vertex + 0, Qk_PI_2_1); // top
		build(&out, {1, -1}, r.leftBottom,  {x1,y2}, vertex + 1, Qk_PI); // left
		build(&out, {-1   }, r.rightBottom, {x2,y2}, vertex + 2, -Qk_PI_2_1); // bottom
		build(&out, {-1, 1}, r.leftTop,     {x2,y1}, vertex + 3, 0); // right
		vertex[5] = vertex[2];

		out.path.vertex.write(vertex, -1, 6); // inl quadrilateral
		out.path.path.close();

		Qk_ReturnLocal(out);
	}

	RectOutlinePath RectOutlinePath::MakeRectOutline(const Rect &rect, const float border[4]) {
		RectOutlinePath outline;
		
		float o_x1 = rect.origin.x(),      o_y1 = rect.origin.y();
		float i_x1 = o_x1 + border[3],     i_y1 = o_y1 + border[0];
		float o_x2 = o_x1 + rect.size.x(), o_y2 = o_y1 + rect.size.y();
		float i_x2 = o_x2 - border[1],     i_y2 = o_y2 - border[2];

		float vertexfv[48] = {
			o_x1,o_y1,i_x1,o_y1,i_x2,o_y1,o_x2,o_y1,i_x2,i_y1,i_x1,i_y1,// vertex,top
			o_x2,o_y1,o_x2,i_y1,o_x2,i_y2,o_x2,o_y2,i_x2,i_y2,i_x2,i_y1,// vertex,right
			o_x2,o_y2,i_x2,o_y2,i_x1,o_y2,o_x1,o_y2,i_x1,i_y2,i_x2,i_y2,// vertex,bottom
			o_x1,o_y2,o_x1,i_y2,o_x1,i_y1,o_x1,o_y1,i_x1,i_y1,i_x1,i_y2,// vertex,left
		};
		Vec2 *v = reinterpret_cast<Vec2*>(vertexfv);

		//._.______________._.
		// \|______________|/
		for (int j = 0; j < 4; j++) {
			auto out = &outline.top+j;
			if (border[j] > 0) {
				out->path.moveTo(v[0]);
				out->path.lineTo(v[3]);
				out->path.moveTo(v[3]); // TODO: fix aa sdf stroke error
				out->path.lineTo(v[4]);
				out->path.lineTo(v[5]);
				out->path.lineTo(v[0]);
				// outside,outside,inside,inside,inside,outside
				Vec2 src[6] = {v[0],v[3],v[5],v[4],v[5],v[3]};
				out->vertex.write(src, -1, 6);
			} else {
				out->path.moveTo(v[1]);
				out->path.lineTo(v[2]);
			}
			v+=6;
		}

		Qk_ReturnLocal(outline);
	}

	RectOutlinePath RectOutlinePath::MakeRRectOutline(
		const Rect& rect, const float border[4], const Path::BorderRadius &r
	) {
		RectOutlinePath outline;

		float o_x1 = rect.origin.x(),      o_y1 = rect.origin.y();
		float i_x1 = o_x1 + border[3],     i_y1 = o_y1 + border[0];
		float o_x2 = o_x1 + rect.size.x(), o_y2 = o_y1 + rect.size.y();
		float i_x2 = o_x2 - border[1],     i_y2 = o_y2 - border[2];

		float vertexfv[48] = {
			o_x1,o_y1,i_x1,o_y1,i_x2,o_y1,o_x2,o_y1,i_x2,i_y1,i_x1,i_y1,// vertex,top
			o_x2,o_y1,o_x2,i_y1,o_x2,i_y2,o_x2,o_y2,i_x2,i_y2,i_x2,i_y1,// vertex,right
			o_x2,o_y2,i_x2,o_y2,i_x1,o_y2,o_x1,o_y2,i_x1,i_y2,i_x2,i_y2,// vertex,bottom
			o_x1,o_y2,o_x1,i_y2,o_x1,i_y1,o_x1,o_y1,i_x1,i_y1,i_x1,i_y2,// vertex,left
		};
		Vec2 *vertex = reinterpret_cast<Vec2*>(vertexfv);

		// border width
		float Bo[6] = { // left,top,right,bottom,left,top
			border[3], border[0], border[1], border[2], border[3], border[0],
		};
		Vec2 oR[5]{r.leftTop,r.rightTop,r.rightBottom,r.leftBottom,r.leftTop};
		// inside radius
		Vec2 iR[5]{
			{oR[0].x() - Bo[0], oR[0].y() - Bo[1]}, // left top
			{oR[1].x() - Bo[2], oR[1].y() - Bo[1]},
			{oR[2].x() - Bo[2], oR[2].y() - Bo[3]}, {oR[3].x() - Bo[0], oR[3].y() - Bo[3]},
		};
		iR[4] = iR[0];
		// center
		Vec2 Ce[5] = {
			{o_x1+oR[0].x(),o_y1+oR[0].y()}, // leftTop
			{o_x2-oR[1].x(),o_y1+oR[1].y()},
			{o_x2-oR[2].x(),o_y2-oR[2].y()}, {o_x1+oR[3].x(),o_y2-oR[3].y()},
		};
		Ce[4] = Ce[0];

		/* rect outline border
			0=A,1=B,5=C
			0____1______________________2____3
			| /| |                      |  / |
			|/_|_|                      | /  |
			|__|_5______________________4/___|
			|    |                      |    |
			|    |                      |    |
			|    |                      |    |
			|    |                      |    |
			|    |                 _____|____|
			|    |                |    /|   /|
			|    |                |   / |  / |
			|____|________________|__/__| /  |
			|\ | |                | /    /   |
			|_\|_|________________|/____/____|
		*/
		auto build = [](
			Pathv *out,
			const float border[3], const Vec2 v[6],
			const Vec2 radius[2], const Vec2 radius_i[2], const Vec2 center[2], float startAngle
		) {
			auto &path = out->path;
			auto noZeroB = border[1] != 0; // border is zero
			Array<Vec2> path2;
			Vec2 lastV;
			auto isRadiusZeroL = radius[0].is_zero_axis();
			auto isRadiusZeroR = radius[1].is_zero_axis();

			if (isRadiusZeroL) { // radius is zero
				if (noZeroB) {
					Vec2 src[]{v[0],v[5]}; // outside,inside
					out->vertex.write(src, -1, 2);
					path2.push(v[5]);
					lastV = v[5];
				}
				path.moveTo(v[0]);
			} else {
				auto borderSum = border[0] + border[1];
				auto sweep = borderSum == 0 ? (Qk_PI_2_1 * 0.5): border[1] / borderSum * Qk_PI_2_1;
				int  sample = getSampleFromRect(radius[0], 1); // |0|1| = sample = 3
				float angleStep = -sweep / (sample - 1);
				float angle = startAngle + sweep;
				bool isRadiusI = radius_i[0].x() > 0 && radius_i[0].y() > 0;

				for (int i = 0; i < sample; i++) {
					Vec2 xy(cosf(angle), -sinf(angle));
					Vec2 v0 = xy * radius[0] + center[0];

					if (noZeroB) {
						if (i == 0) {
							Vec2 v1 = isRadiusI ? xy * radius_i[0] + center[0]: v[5];
							Vec2 src[]{v0,v1}; // outside,inside
							out->vertex.write(src, -1, 2);
							path2.push(v1);
							lastV = v1;
						} else if (isRadiusI) {
							Vec2 v1 = xy * radius_i[0] + center[0];
							Vec2 src[]{v0,lastV,v1,v0,v0,v1};
							out->vertex.write(src, -1, 6);
							path2.push(v1);
							lastV = v1;
						} else { // inside radius is zero
							Vec2 src[]{v0,v0,v[5]}; // outside,outside,inside
							out->vertex.write(src, -1, 3);
						}
					}
					path.lineTo(v0);
					angle += angleStep;
				}
			}
		
			if (isRadiusZeroR) { // radius is zero
				if (noZeroB) {
					Vec2 src[]{v[3],v[3],out->vertex.back(),v[4]}; // outside,outside,inside,inside
					out->vertex.write(src, -1, 4);
					path2.push(v[4]);
				}
				path.lineTo(v[3]);
			} else {
				auto borderSum = border[2] + border[1];
				auto sweep = borderSum == 0 ? (Qk_PI_2_1 * 0.5): border[1] / borderSum * Qk_PI_2_1;
				int  sample = getSampleFromRect(radius[1], 1); // |0|1| = sample = 3
				float angleStep = -sweep / (sample - 1);
				float angle = startAngle;
				bool isRadiusI = radius_i[1].x() > 0 && radius_i[1].y() > 0;

				for (int i = 0; i < sample; i++) {
					Vec2 xy(cosf(angle), -sinf(angle));
					Vec2 v0 = xy * radius[1] + center[1], v1;

					if (noZeroB) {
						if (isRadiusI) {
							v1 = xy * radius_i[1] + center[1];
							RadiusI:
							// outside,inside,inside,outside,outside,inside
							Vec2 src[]{v0,lastV,v1,v0,v0,v1};
							out->vertex.write(src, -1, 6);
							path2.push(v1);
							lastV = v1;
							//Qk_DEBUG("v0: %f %f, v1: %f %f", v0[0], v0[1], v1[0], v1[1]);
							//Qk_DEBUG("ro: %f %f, ri: %f %f", radius[1][0], radius[1][1], radius_i[1][0], radius_i[1][1]);
						} else { // inside radius is zero
							if (i == 0) {
								v1 = v[4]; goto RadiusI;
							}
							Vec2 src[]{v0,v0,v[4]}; // outside,outside,inside
							out->vertex.write(src, -1, 3);
						}
					}
					path.lineTo(v0);
					angle += angleStep;
				}

				out->vertex.pop(2); // delete invalid vertices
			}
			
			if (noZeroB) {
				if (border[2] < 0.1 && !isRadiusZeroR) // fix aa sdf stroke error
					path.moveTo(path2.back());

				for (int i = path2.length() - 1; i >= 0; i--)
					path.lineTo(path2[i]);

				if (border[0] > 0.1 || isRadiusZeroL) // fix aa sdf stroke error
					path.lineTo(*path.pts()); // equivalent to close
			}
		};

		float angle = Qk_PI_2_1;
		for (int j = 0; j < 4; j++) {
			build(&outline.top + j, Bo+j, vertex, oR+j, iR+j, Ce+j, angle);
			vertex+=6;
			angle -= Qk_PI_2_1;
		}

		Qk_ReturnLocal(outline);
	}

}
