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
		path.close();
		return std::move(path);
	}

	Path Path::MakeArc(const Rect& r, float startAngle, float sweepAngle, bool useCenter) {
		Path path;
		path.arcTo(r, startAngle, sweepAngle, useCenter);
		path.close();
		return std::move(path);
	}

	Path Path::MakeRect(const Rect& r, bool ccw) {
		Path path;
		path.rectTo(r,ccw);
		path.close();
		return std::move(path);
	}

	Path Path::MakeCircle(Vec2 center, float radius, bool ccw) {
		return MakeOval({ Vec2(center.x() - radius, center.y() - radius), Vec2(radius) * 2 }, ccw);
	}

	void setRrectOutline(Path &path,
		const Rect& outside, const Rect *inside, const Path::BorderRadius& br)
	{
		auto arc = [&](Vec2 origin, Vec2 r, Vec2 dir, float startAngle, float sweepAngle) {
			if (r.x() != 0 && r.y() != 0) {
				Vec2 s = r*2;
				path.arcTo({origin+s*dir, s}, startAngle, sweepAngle, false);
			} else {
				path.moveTo(origin);
			}
		};

		Vec2 a = br.leftTop;
		Vec2 b = br.rightTop;
		Vec2 c = br.rightBottom;
		Vec2 d = br.leftBottom;
		Vec2 origin = outside.origin;
		Vec2 end = origin + outside.size;

		// cw
		arc(origin, a, Vec2(0), Qk_PI, -Qk_PI2); // left-top
		arc({end.x(), origin.y()}, b, Vec2(-1,0), Qk_PI2, -Qk_PI2); // right-top
		arc(end, c, Vec2(-1), 0, -Qk_PI2); // right-bottom
		arc({origin.x(), end.y()}, d, Vec2(0,-1), -Qk_PI2, -Qk_PI2); // left-bottom

		path.close();

		if (!inside)
			return;

		Vec2 inside_origin = inside->origin;
		Vec2 inside_end = inside_origin + inside->size;

		float top = inside_origin.y() - origin.y();
		float right = end.x() - inside_end.x();
		float bottom = end.y() - inside_end.y();
		float left = inside_origin.x() - origin.x();

		origin = inside_origin;
		end = inside_end;

		a = { Float::max(a.x() - left, 0.0), Float::max(a.y() - top, 0.0) };
		b = { Float::max(b.x() - right, 0.0), Float::max(b.y() - top, 0.0) };
		c = { Float::max(c.x() - right, 0.0), Float::max(c.y() - bottom, 0.0) };
		d = { Float::max(d.x() - left, 0.0), Float::max(d.y() - bottom, 0.0) };

		// ccw
		arc(origin, a, Vec2(0), Qk_PI2, Qk_PI2); // left-top
		arc({origin.x(), end.y()}, d, Vec2(0,-1), Qk_PI, Qk_PI2); // left-bottom
		arc(end, c, Vec2(-1), -Qk_PI2, Qk_PI2); // right-bottom
		arc({end.x(), origin.y()}, b, Vec2(-1,0), 0, Qk_PI2); // right-top

		path.close();
	}

	Path Path::MakeRRect(const Rect& rect, const BorderRadius& br)
	{
		Path path;
		setRrectOutline(path, rect, NULL, br);
		return std::move(path);
	}

	Path Path::MakeRRectOutline(
		const Rect& outside, const Rect &inside, const BorderRadius& br) {
		Path path;
		setRrectOutline(path, outside, &inside, br);
		// TODO ... add ext data
		// Array<float> extData(20);
		
		return std::move(path);
	}
	
	Path Path::MakeRectOutline(const Rect &outside, const Rect &inside) {
		Path path;
		path.rectTo(outside);
		path.close();
		path.rectTo(inside, true);
		path.close();

		Array<float> extData(10);
		Vec2 end = outside.size + outside.origin;

		// outside
		float length = 0;
		extData[0] = length; // left/top
		extData[1] = (length+=end.x()); // right/top
		extData[2] = (length+=end.y()); // right/bottom
		extData[3] = (length+=end.x()); // left/bottom
		extData[4] = (length+=end.y()); // left/top
		// inside
		extData[5] = length; // left/top
		extData[6] = (length-=end.y()); // left/bottom
		extData[7] = (length-=end.x()); // right/bottom
		extData[8] = (length-=end.y()); // right/top
		extData[9] = (length-=end.x()); // left/top

		path.setExtData(std::move(extData));

		return std::move(path);
	}

	Path::Path(Vec2 move): _IsNormalized(true) {
		moveTo(move);
	}

	Path::Path(): _IsNormalized(true) {}

	void Path::moveTo(Vec2 to) {
		// _pts.push(to.x()); _pts.push(to.y());
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Move);
		_hash.update((uint32_t*)&to, 2);
	}

	void Path::lineTo(Vec2 to) {
		// _pts.push(to);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Line);
		_hash.update((uint32_t*)&to, 2);
	}

	void Path::quadTo(Vec2 control, Vec2 to) {
		_pts.write(control.val, -1, 2);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Quad);
		_IsNormalized = false;
		// _hash.update((&_pts.back()) - 4, sizeof(float) * 4);
		_hash.update((uint32_t*)(&_pts.back()) - 4, 4);
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
		_hash.update((uint32_t*)(&_pts.back()) - 6, 6);
	}

	constexpr float magicCircle = 0.551915024494f; // 0.552284749831f

	void Path::ovalTo(const Rect& r, bool ccw) {
		float w = r.size.x(), h = r.size.y();
		float x = r.origin.x(), y = r.origin.y();
		float x2 = x + w * 0.5, y2 = y + h * 0.5;
		float x3 = x + w, y3 = y + h;
		float cx = w * 0.5 * magicCircle, cy = h * 0.5 * magicCircle;
		startTo(Vec2(x2, y)); // center,top

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
		startTo(r.origin);
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

	void Path::arcTo(const Rect& r, float startAngle, float sweepAngle, bool useCenter) {

		float rx = r.size.x() * 0.5f;
		float ry = r.size.y() * 0.5f;
		float cx = r.origin.x() + rx;
		float cy = r.origin.y() + ry;

		float n = ceilf(abs(sweepAngle) / Qk_PI2);
		float sweep = sweepAngle / n;
		float magic =
			sweep == Qk_PI2 ? magicCircle:
			sweep == -Qk_PI2 ? -magicCircle:
			tanf(sweep / 4.0f) * 1.3333333333333333f/*4.0 / 3.0*/;

		startAngle = -startAngle;

		float x0 = cosf(startAngle);
		float y0 = sinf(startAngle);

		Vec2 start(x0 * rx + cx, y0 * ry + cy);

		if (useCenter) {
			startTo(Vec2(cx, cy));
			lineTo(start);
		} else {
			startTo(start);
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
	
	void Path::startTo(Vec2 p) {
		if (_verbs.length() && _verbs.back() != kVerb_Close) {
			if (*(Vec2*) (_pts.val() + _pts.length() - 2) != p)
				lineTo(p);
		} else {
			moveTo(p);
		}
	}

	void Path::close() {
		_verbs.push(kVerb_Close);
	}
	
	void Path::setExtData(Array<float> &&extData) {
		_ptsExt = std::move(extData);
	}

	Array<Vec2> Path::getEdgeLines(bool close, float epsilon) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp, false,epsilon);
		Array<Vec2> edges;
		auto pts = (const Vec2*)*self->_pts;
		int  len = 0;
		Vec2 prev;

		auto closeLine = [&]() {
		if (len) {
				auto vertex = edges[edges.length() - len];
				edges.push(prev);
				edges.push(vertex);
			}
		};

		for (auto verb: self->_verbs) {
			switch(verb) {
				case kVerb_Move:
					if (close) {
						closeLine();
					}
					prev = *pts++;
					len = 0;
					break;
				case kVerb_Line: {
					edges.push(prev);
					prev = *pts++;
					edges.push(prev); // edge 0
					len+=2;
					break;
				}
				default: // close
					Qk_ASSERT(verb == kVerb_Close);
					closeLine(); // add close edge line
					prev = Vec2();
					len = 0;
					break;
			}
		}

		if (close) {
			closeLine();
		}
		
		return std::move(edges);
	}

	Array<float> Path::getPolygons(int polySize, float epsilon, bool isExt) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp, false,epsilon);
		auto tess = tessNewTess(nullptr); // TESStesselator*
		auto pts = (const Vec2*)*self->_pts;
		int len = 0;
		Array<float> polygons;
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
				default: // close
					Qk_ASSERT(verb == kVerb_Close);
					if (len) {
						tmpV.push(tmpV[tmpV.length() - len++]);
						tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
						len = 0;
					}
					break;
			}
		}

		if (len > 1) { // auto close
			tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
		}

		// Convert to convex contour vertex data
		if ( tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, polySize, 2, 0) ) {

			const int nelems = tessGetElementCount(tess);
			const TESSindex* elems = tessGetElements(tess);
			const Vec2* verts = (const Vec2*)tessGetVertices(tess);

			polygons.extend(nelems * polySize * 2);

			for (int i = 0; i < polygons.length(); i+=2) {
				// polygons[i] = verts[*elems++];
				*reinterpret_cast<Vec2*>(polygons.val() + i) = verts[*elems++];
			}
		}

		tessDeleteTess(tess);
		return std::move(polygons);
	}

	Path Path::dashPath(float offset, float phase, float interval) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp, false, 1);
		// TODO ...
	}
	
	Path Path::strokePath(float width, Cap cap, Join join, float offset) const {
		// TODO ...
		return *this;
	}

	Path Path::normalizedPath(float epsilon) const {
		if (_IsNormalized)
			return *this; // copy self
		Path line;
		normalized(&line, true, epsilon);
		return std::move(line);
	}

	Path* Path::normalized(Path *out, bool updateHash, float epsilon) const {
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
					QuadraticBezier bezier(pts[-1], pts[0], pts[1]);
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
					CubicBezier bezier(pts[-1], pts[0], pts[1], pts[2]);
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
		float S_2 = abs(S_ABC); // *0.5

		if (S_2 < 5000.0) {
			constexpr float count = 22.0 / 8.408964152537145;
			int i = Uint32::max(sqrt_sqrtf(S_2) * count * epsilon, 2);
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
		float S_2 = abs(S_ABC + S_CDA); // S = S_2 * 0.5

		if (S_2 < 5000.0) { // circle radius < 80
			constexpr float count = 30.0 / 8.408964152537145;//sqrtf(sqrtf(5000.0));
			int i = Uint32::max(sqrt_sqrtf(S_2) * count * epsilon, 2);
			return i;
		} else {
			return 30;
		}
	}

}
