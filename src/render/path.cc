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
		addTo(Vec2(x2, y)); // center,top

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

		addTo(r.origin);
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
		if (radius.is_zero()) return;
		if (sweepAngle == 0) return;

		float rx = radius.x();
		float ry = radius.y();
		float cx = center.x();
		float cy = center.y();

		float n = ceilf(abs(sweepAngle) / Qk_PI_2_1);
		float sweep = sweepAngle / n;
		float magic =
			sweep == Qk_PI_2_1 ? magicCircle:
			sweep == -Qk_PI_2_1 ? -magicCircle:
			tanf(sweep / 4.0f) * 1.3333333333333333f/*4.0 / 3.0*/;

		startAngle = -startAngle;

		float x0 = cosf(startAngle);
		float y0 = sinf(startAngle);

		Vec2 start(x0 * rx + cx, y0 * ry + cy);

		if (useCenter) {
			addTo(Vec2(cx, cy));
			lineTo(start);
		} else {
			addTo(start);
		}

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
	
	void Path::addTo(Vec2 p) {
		if (_verbs.length() && _verbs.back() != kVerb_Close) {
			//if (*(Vec2*) (_pts.val() + _pts.length() - 2) != p)
			lineTo(p);
		} else {
			moveTo(p);
		}
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
		int  len = 0;
		Vec2 move,from;

		for (auto verb: self->_verbs) {
			switch(verb) {
				case kVerb_Move:
					move = from = *pts++;
					break;
				case kVerb_Line:
					edges.push(from);
					from = *pts++;
					edges.push(from); // edge 0
					break;
				case kVerb_Close: // close
					//Qk_ASSERT(verb == kVerb_Close);
					if (move != from) { // close path
						edges.push(from);
						edges.push(move);
					}
					move = from = Vec2();
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
						if (len == 0) {
							tmpV.push(Vec2(0)); len=1; // use Vec2(0,0) start point
						}
						tmpV.push(*pts++); len++;
						break;
					case kVerb_Close: // close
						// Qk_ASSERT(verb == kVerb_Close);
						if (len) {
							tmpV.push(tmpV[tmpV.length() - len++]);
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

		for (int i = 0, len = self->_verbs.length(); i < len;) {
			switch (self->_verbs[i++]) {
				case kVerb_Move:
					move = from = *pts++;
					break;
				case kVerb_Line:
					lineTo(from, *pts);
					from = *pts++;
					break;
				case kVerb_Close:
					if (from != move)
						lineTo(from, move);
					if (i < len)
						move = from = Vec2();
					break;
				default: Qk_FATAL("Path::dashPath");
			}
		}

		if (out.verbsLen() && useStage) {
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
					if (isZeor)
						add(Vec2(), kVerb_Move); // add zeor
					add(*pts++, kVerb_Line);
					isZeor = false;
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
		RectPath out;
		out.rect = rect;
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
		RectPath out;
		out.rect = rect;

		const float x1 = rect.origin.x(),    y1 = rect.origin.x();
		const float x2 = x1 + rect.size.x(), y2 = y1 + rect.size.y();
		const float x_5 = rect.size.x() * 0.5, y_5 = rect.size.y() * 0.5;

		const Vec2 leftTop = Vec2(Float::min(r.leftTop.x(), x_5), Float::min(r.leftTop.y(), y_5));
		const Vec2 rightTop = Vec2(-Float::min(r.rightTop.x(), x_5), Float::min(r.rightTop.y(), y_5));
		const Vec2 rightBottom = Vec2(-Float::min(r.rightBottom.x(), x_5), -Float::min(r.rightBottom.y(), y_5));
		const Vec2 leftBottom = Vec2(Float::min(r.leftBottom.x(), x_5), -Float::min(r.leftBottom.y(), y_5));

		out.path.path.moveTo(Vec2(x1, leftTop.is_zero_axis() ? y1: y1 + leftTop.y()));

		auto build = [](RectPath *out, Vec2 v, Vec2 v2, Vec2 radius, Vec2 radius2, float startAngle) {
			bool no_zero1  = radius[0] > 0 && radius[1] > 0;
			bool no_zero2 = radius2[0] > 0 && radius2[1] > 0;
			const Vec2 c(v + radius);

			if (no_zero1) {
				int   sample = getSampleFromRect(radius, 1); // |0|1| = sample = 3
				float angleStep = -Qk_PI_2_1 / (sample - 1);
				float angle = startAngle + Qk_PI_2_1;

				for (int i = 0; i < sample; i++) {
					const Vec2 p(c.x() - cosf(angle) * radius.x(), c.x() + sinf(angle) * radius.y());
					const Vec2 src[3] = { p,c,p };
					out->path.vertex.write(src, -1, i == 0 ? 2: 3); // add triangle vertex
					out->path.path.lineTo(p);
					angle += angleStep;
				}
			} else {
				out->path.path.lineTo(v);
			}

			if (no_zero2) { // no zero
				const Vec2 c2(v2 + radius2);
				const Vec2 p(c2.x() - cosf(startAngle) * radius2.x(), c2.y () + sinf(startAngle) * radius2.y());
				if (no_zero1) { // no zero
					const Vec2 src[4] = {p,p,c2,c};
					out->path.vertex.write(src, -1, 4);
				} else {
					const Vec2 src[3] = {v,p,c2};
					out->path.vertex.write(src, -1, 3);
				}
			} else if (no_zero1) { // v != zero and v2 == zero
				out->path.vertex.push(v2);
			}

			return c;
		};

		Vec2 a = build(&out, Vec2(x1, y1), Vec2(x2, y1), leftTop, rightTop, Qk_PI_2_1);
		Vec2 b = build(&out, Vec2(x2, y1), Vec2(x2, y2), rightTop, rightBottom, 0);
		Vec2 c = build(&out, Vec2(x2, y2), Vec2(x1, y2), rightBottom, leftBottom, -Qk_PI_2_1);
		Vec2 d = build(&out, Vec2(x1, x2), Vec2(x1, y1), leftBottom, leftTop, Qk_PI);

		const Vec2 src[6] = { a,b,c,c,d,a };
		out.path.vertex.write(src, -1, 6);
		out.path.path.close();

		Qk_ReturnLocal(out);
	}

	RectOutlinePath RectOutlinePath::MakeRectOutline(const Rect &o, const Rect &i) {
		RectOutlinePath rect;

		/* rect outline border
			._.______________._.
			|\|______________|/|
			|-|              |-|
			| |              | |
			|_|______________|_|
			|/|______________|\|
		*/
		const float o_x1 = o.origin.x(),      o_y1 = o.origin.y();
		const float i_x1 = i.origin.x(),      i_y1 = i.origin.y();
		const float o_x2 = o_x1 + o.size.x(), o_y2 = o_y1 + o.size.y();
		const float i_x2 = i_x1 + i.size.x(), i_y2 = i_y1 + i.size.y();

		const float border[6] = { // left,top,right,bottom,left,top
			i_x1 - o_x1, i_y1 - o_y1, o_x2 - i_x2, o_y2 - i_y2, i_x1 - o_x1, i_y1 - o_y1,
		};
		const float vertexfv[48] = {
			o_x1,o_y1,i_x1,o_y1,i_x2,o_y1,o_x2,o_y1,i_x2,i_y1,i_x1,i_y1,// vertex,top
			o_x2,o_y1,o_x2,i_y1,o_x2,i_y2,o_x2,o_y2,i_x2,i_y2,i_x2,i_y1,// vertex,right
			o_x2,o_y2,i_x2,o_y2,i_x1,o_y2,o_x1,o_y2,i_x1,i_y2,i_x1,i_y1,// vertex,bottom
			o_x1,o_y2,o_x1,i_y2,o_x1,i_y1,o_x1,o_y1,i_x1,i_y1,i_x1,i_y2,// vertex,left
		};
		const Vec2 *vertex = reinterpret_cast<const Vec2*>(vertexfv);

		//._.______________._.
		// \|______________|/
		auto build = [](Pathv *out, const float border[3], const Vec2 v[6]) {
			if (border[1] > 0) {
				// outside,outside,inside,inside,inside,outside
				const Vec2 src[6] = {v[0],v[3],v[5],v[4],v[5],v[3]};
				out->vertex.write(src, -1, 6);
				out->path.moveTo(v[0]);
				out->path.lineTo(v[3]);
				out->path.lineTo(v[4]);
				out->path.lineTo(v[5]);
				out->path.close();
			} else {
				out->path.moveTo(v[1]);
				out->path.lineTo(v[2]);
			}
		};

		for (int j = 0; j < 4; j++) {
			build(&rect.top+j,  border+j, vertex);
			vertex+=6;
		}

		Qk_ReturnLocal(rect);
	}

	RectOutlinePath RectOutlinePath::MakeRRectOutline(
		const Rect& o, const Rect &i, const Path::BorderRadius &r
	) {
		RectOutlinePath rect;

		const float o_x1 = o.origin.x(),      o_y1 = o.origin.y();
		const float i_x1 = i.origin.x(),      i_y1 = i.origin.y();
		const float o_x2 = o_x1 + o.size.x(), o_y2 = o_y1 + o.size.y();
		const float i_x2 = i_x1 + i.size.x(), i_y2 = i_y1 + i.size.y();
		// border width
		const float border[6] = { // left,top,right,bottom,left,top
			o_x1-i_x1, o_y1-i_y1, i_x2-o_x2, i_y2-o_y2, o_x1-i_x1, o_y1-i_y1,
		};
		const float vertexfv[48] = {
			o_x1,o_y1,i_x1,o_y1,i_x2,o_y1,o_x2,o_y1,i_x2,i_y1,i_x1,i_y1,// vertex,top
			o_x2,o_y1,o_x2,i_y1,o_x2,i_y2,o_x2,o_y2,i_x2,i_y2,i_x2,i_y1,// vertex,right
			o_x2,o_y2,i_x2,o_y2,i_x1,o_y2,o_x1,o_y2,i_x1,i_y2,i_x1,i_y1,// vertex,bottom
			o_x1,o_y2,o_x1,i_y2,o_x1,i_y1,o_x1,o_y1,i_x1,i_y1,i_x1,i_y2,// vertex,left
		};
		const Vec2 *vertex = reinterpret_cast<const Vec2*>(vertexfv);

		const float x_5  = o.size.x() * 0.5,  y_5  = o.size.y() * 0.5;
		// outside radius
		Vec2 R[5] = {
			Vec2(Float::min(r.leftTop.x(), x_5), Float::min(r.leftTop.y(), y_5)), // left top
			Vec2(Float::min(r.rightTop.x(), x_5), Float::min(r.rightTop.y(), y_5)),
			Vec2(Float::min(r.rightBottom.x(), x_5), Float::min(r.rightBottom.y(), y_5)),
			Vec2(Float::min(r.leftBottom.x(), x_5), Float::min(r.leftBottom.y(), y_5)),
		};
		// inside radius
		Vec2 iR[5] = {
			Vec2(R[0].x() - border[0], R[0].y() - border[1]), // left top
			Vec2(R[1].x() - border[2], R[1].y() - border[1]),
			Vec2(R[2].x() - border[2], R[2].y() - border[3]), Vec2(R[3].x() - border[0], R[3].y() - border[3]),
		};
		// center
		Vec2 center[5] = {
			{o_x1+R[0].x(),o_y1+R[0].y()}, // leftTop
			{o_x2-R[1].x(),o_y1+R[1].y()},
			{o_x2-R[2].x(),o_y2-R[2].y()}, {o_x1+R[3].x(),o_y2-R[3].y()},
		};

		R[4] = R[0];
		iR[4] = iR[0];
		center[4] = center[0];

		//._.______________._.
		// \|______________|/
		auto build = [](
			Pathv *out,
			const float border[3], const Vec2 v[6],
			const Vec2 radius[2], const Vec2 radius_i[2], const Vec2 center[2], float startAngle
		) {
			Path &path = out->path;
			auto sweep1 = border[1]/(border[0]+border[1]) * Qk_PI_2_1;
			auto sweep2 = border[2]/(border[2]+border[1]) * Qk_PI_2_1;

			if (radius[0].is_zero_axis()) {
				path.moveTo(v[0]);
			} else {
				path.arcTo(center[0],radius[0], startAngle + sweep1, -sweep1, false);
			}
			if (radius[1].is_zero_axis()) {
				path.lineTo(v[3]);
			} else {
				path.arcTo(center[1],radius[1], startAngle, -sweep2, false);
			}
			if (border[1] <= 0) return;

			if (radius_i[1].x() > 0 && radius_i[1].y() > 0) { // radius
				path.arcTo(center[1],radius_i[1], startAngle - sweep2, sweep2, false);
			} else {
				path.lineTo(v[4]);
			}
			if (radius_i[0].x() > 0 && radius_i[0].y() > 0) { // radius
				path.arcTo(center[0],radius_i[0], startAngle, sweep1, false);
			} else {
				path.lineTo(v[5]);
			}
			path.close();

			if (!path.isNormalized()) {
				out->path = path.normalizedPath();
			}
			out->vertex = out->path.getTriangles();
		};

		float angle = Qk_PI_2_1;
		for (int j = 0; j < 4; j++) {
			build(&rect.top + j, border+j, vertex, R+j, iR+j, center+j, angle);
			vertex+=6;
			angle -= Qk_PI_2_1;
		}

		Qk_ReturnLocal(rect);
	}

}
