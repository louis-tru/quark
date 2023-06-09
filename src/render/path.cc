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
#include "./ft/ft_path.h"

namespace qk {

	Path Path::MakeOval(const Rect& r, bool ccw) {
		Path path;
		path.ovalTo(r, ccw);
		path.close();
		Qk_ReturnLocal(path);
	}

	Path Path::MakeArc(const Rect& r, float startAngle, float sweepAngle, bool useCenter) {
		Path path;
		path.arcTo(r, startAngle, sweepAngle, useCenter);
		path.close();
		Qk_ReturnLocal(path);
	}

	Path Path::MakeRect(const Rect& r, bool ccw) {
		Path path;
		path.rectTo(r,ccw);
		path.close();
		Qk_ReturnLocal(path);
	}

	Path Path::MakeCircle(Vec2 center, float radius, bool ccw) {
		return MakeOval({ Vec2(center.x() - radius, center.y() - radius), Vec2(radius) * 2 }, ccw);
	}

	static void setRRect(Path &path,
		const Rect& outside, const Rect *inside, const Path::BorderRadius& br)
	{
		auto arc = [&](Vec2 origin, Vec2 r, Vec2 dir, float startAngle, float sweepAngle) {
			if (r.x() != 0 && r.y() != 0) {
				Vec2 s = r*2;
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
		arc(origin, a, Vec2(0), Qk_PI, -Qk_PI2); // left-top
		arc({end.x(), origin.y()}, b, Vec2(-1,0), Qk_PI2, -Qk_PI2); // right-top
		arc(end, c, Vec2(-1), 0, -Qk_PI2); // right-bottom
		arc({origin.x(), end.y()}, d, Vec2(0,-1), -Qk_PI2, -Qk_PI2); // left-bottom

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
		arc(origin, a, Vec2(0), Qk_PI2, Qk_PI2); // left-top
		arc({origin.x(), end.y()}, d, Vec2(0,-1), Qk_PI, Qk_PI2); // left-bottom
		arc(end, c, Vec2(-1), -Qk_PI2, Qk_PI2); // right-bottom
		arc({end.x(), origin.y()}, b, Vec2(-1,0), 0, Qk_PI2); // right-top

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

	Array<Vec2> Path::getEdgeLines(bool close, float epsilon) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp,epsilon,false);
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
					if (close)
						closeLine();
					prev = *pts++;
					len = 0;
					break;
				case kVerb_Line:
					edges.push(prev);
					prev = *pts++;
					edges.push(prev); // edge 0
					len+=2;
					break;
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
		
		Qk_ReturnLocal(edges);
	}

	Array<Vec2> Path::getVertexsFromPaths(const Path *paths, int pathsLen, int polySize, float epsilon) {
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

	Path Path::dashPath(float *stage_p, int stage_count) const {
		Path tmp, out;
		const Path *self = _IsNormalized ? this: normalized(&tmp, 1,false);
		auto pts = (const Vec2*)*self->_pts;
		int  stage_idx = -1;
		bool useStage = false; // no empty
		Vec2 move, from;
		float stage = 0;

		auto nextStage = [&]() {
			stage_idx = (stage_idx + 1) % stage_count;
			stage = stage_p[stage_idx];
			Qk_ASSERT(stage != 0, "#Path.dashPath.nextStage() assert stage != 0");
			useStage = stage > 0;
			if (!useStage) { // empty
				stage = -stage;
			}
		};

		auto lineTo = [&](Vec2 from, Vec2 to) {
			auto  start = from;
			auto  point = to - from;
			float len = point.length(), useLen = 0;

			while (len > useLen) {
				if (useStage) {
					out.lineTo(from);
				}
				if (stage == 0) {
					nextStage();
					if (useStage) {
						out.moveTo(from);
					}
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
					break;
				case kVerb_Line:
					lineTo(from, *pts);
					from = *pts++;
					break;
				default:
					Qk_ASSERT(verb == kVerb_Close);
					if (from != move) {
						lineTo(from, move);
					}
					move = from = Vec2();
					break;
			}
		}

		if (out.verbsLen() && useStage) {
			out.lineTo(from);
		}

		Qk_ReturnLocal(out);
	}

	Path Path::strokePath(float width, Cap cap, Join join, float miter_limit) const {
		Qk_FT_Stroker stroker;
		Qk_FT_Stroker_LineCap ft_cap = Qk_FT_Stroker_LineCap(cap);
		Qk_FT_Stroker_LineJoin ft_join =
			Join::kMiter_Join == join ? Qk_FT_Stroker_LineJoin::Qk_FT_STROKER_LINEJOIN_MITER: 
			Join::kRound_Join == join ? Qk_FT_Stroker_LineJoin::Qk_FT_STROKER_LINEJOIN_ROUND: 
			Qk_FT_STROKER_LINEJOIN_BEVEL;
		Qk_FT_Error err;

		if (miter_limit == 0)
			miter_limit = 1024;

		err = Qk_FT_Stroker_New(&stroker);
		Qk_ASSERT(err==0);
		Qk_FT_Stroker_Set(stroker, FT_1616(width * 0.5), ft_cap, ft_join, FT_1616(miter_limit));

		auto from_outline = qk_ft_outline_convert(this);
		err = Qk_FT_Stroker_ParseOutline(stroker, from_outline);
		Qk_ASSERT(err==0);

		Qk_FT_UInt anum_points, anum_contours;
		err =
		Qk_FT_Stroker_GetCounts(stroker, &anum_points, &anum_contours);
		Qk_ASSERT(err==0);

		auto to_outline = qk_ft_outline_create(anum_points, anum_contours);
		Qk_FT_Stroker_Export(stroker, to_outline);
		Path out;
		err = qk_ft_path_convert(to_outline, &out);
		Qk_ASSERT(err==0);

		qk_ft_outline_destroy(from_outline);
		qk_ft_outline_destroy(to_outline);
		Qk_FT_Stroker_Done(stroker);

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
		float S_2 = abs(S_ABC + S_CDA); // S = S_2 * 0.5

		if (S_2 < 5000.0) { // circle radius < 80
			constexpr float count = 30.0 / 8.408964152537145;//sqrtf(sqrtf(5000.0));
			int i = Uint32::max(sqrt_sqrtf(S_2) * count * epsilon, 2);
			return i;
		} else {
			return 30;
		}
	}

	static int getSampleFromRect(Vec2 rect, float epsilon, float ratio = 0.5) {
		float S_2 = abs(rect.x() * rect.y() * ratio); // width * height
		if (S_2 < 5000.0) { // circle radius < 80
			constexpr float count = 30.0 / 8.408964152537145;//sqrtf(sqrtf(5000.0));
			int i = Uint32::max(sqrt_sqrtf(S_2) * count * epsilon, 2);
			return i;
		} else {
			return 30;
		}
	}

	// ------------------- R e c t . O u t l i n e . P a t h -------------------

	RectPath RectPath::MakeRect(const Rect &r) {
		RectPath rect;
		float x2 = r.origin.x() + r.size.x();
		float y2 = r.origin.y() + r.size.y();
		// path
		rect.path.moveTo(r.origin);
		rect.path.lineTo(Vec2(x2, r.origin.y())); // top right
		rect.path.lineTo(Vec2(x2, y2)); // bottom right
		rect.path.lineTo(Vec2(r.origin.x(), y2)); // bottom left
		rect.path.close(); // top left, origin point
		// vertex
		rect.vertex.extend(6);
		rect.vertex[0] = r.origin;
		rect.vertex[1] = Vec2(x2, r.origin.y());
		rect.vertex[2] = Vec2(x2, y2);
		rect.vertex[3] = Vec2(x2, y2);
		rect.vertex[4] = Vec2(r.origin.x(), y2);
		rect.vertex[5] = r.origin;

		Qk_ReturnLocal(rect);
	}

	RectPath RectPath::MakeRRect(const Rect &rect, const Path::BorderRadius &r) {
		RectPath path;

		const float x1 = rect.origin.x(),    y1 = rect.origin.x();
		const float x2 = x1 + rect.size.x(), y2 = y1 + rect.size.y();
		const float x_5 = rect.size.x() * 0.5, y_5 = rect.size.y() * 0.5;

		const Vec2 leftTop = Vec2(Float::min(r.leftTop.x(), x_5), Float::min(r.leftTop.y(), y_5));
		const Vec2 rightTop = Vec2(-Float::min(r.rightTop.x(), x_5), Float::min(r.rightTop.y(), y_5));
		const Vec2 rightBottom = Vec2(-Float::min(r.rightBottom.x(), x_5), -Float::min(r.rightBottom.y(), y_5));
		const Vec2 leftBottom = Vec2(Float::min(r.leftBottom.x(), x_5), -Float::min(r.leftBottom.y(), y_5));

		path.path.moveTo(Vec2(x1, leftTop.is_zero_or() ? y1: y1 + leftTop.y()));

		auto build = [](
			RectPath *out, Vec2 v, Vec2 v2,
			Vec2 radius, Vec2 radius2, float startAngle
		) {
			bool is_zero  = radius.is_zero_or();
			bool is_zero2 = radius2.is_zero_or();
			const Vec2 c(v + radius);

			if (!is_zero) {
				int   sample = getSampleFromRect(radius, 1); // |0|1| = sample = 3
				float angleStep = -Qk_PI2 / (sample - 1);
				float angle = startAngle + Qk_PI2;

				for (int i = 0; i < sample; i++) {
					const Vec2 p(
						c.x() - cosf(angle) * radius.x(), c.x() + sinf(angle) * radius.y()
					);
					const Vec2 src[3] = { p,c,p };
					out->vertex.write(src, -1, i == 0 ? 2: 3); // add triangle vertex
					out->path.lineTo(p);
					angle += angleStep;
				}
			} else {
				out->path.lineTo(v);
			}

			if (!is_zero2) { // no zero
				const Vec2 c2(v2 + radius2);
				const Vec2 p(
					c2.x() - cosf(startAngle) * radius2.x(), c2.y () + sinf(startAngle) * radius2.y()
				);
				if (!is_zero) { // no zero
					const Vec2 src[4] = {p,p,c2,c};
					out->vertex.write(src, -1, 4);
				} else {
					const Vec2 src[3] = {v,p,c2};
					out->vertex.write(src, -1, 3);
				}
			} else if (!is_zero) { // v != zero and v2 == zero
				out->vertex.push(v2);
			}

			return c;
		};

		Vec2 a = build(&path, Vec2(x1, y1), Vec2(x2, y1), leftTop, rightTop, Qk_PI2);
		Vec2 b = build(&path, Vec2(x2, y1), Vec2(x2, y2), rightTop, rightBottom, 0);
		Vec2 c = build(&path, Vec2(x2, y2), Vec2(x1, y2), rightBottom, leftBottom, -Qk_PI2);
		Vec2 d = build(&path, Vec2(x1, x2), Vec2(x1, y1), leftBottom, leftTop, Qk_PI);

		const Vec2 src[6] = { a,b,c,c,d,a };
		path.vertex.write(src, -1, 6);
		path.path.close();

		Qk_ReturnLocal(path);
	}

	RectOutlinePath RectOutlinePath::MakeRectOutline(const Rect &o, const Rect &i) {
		RectOutlinePath rect;
		// contain outline length offset and width offset and border direction
		// of ext data item

		const float o_x1 = o.origin.x(),      o_y1 = o.origin.y();
		const float i_x1 = i.origin.x(),      i_y1 = i.origin.y();
		const float o_x2 = o_x1 + o.size.x(), o_y2 = o_y1 + o.size.y();
		const float i_x2 = i_x1 + i.size.x(), i_y2 = i_y1 + i.size.y();

		// outside path
		rect.outside.moveTo(o.origin);
		rect.outside.lineTo(Vec2(o_x2, o_y1)); // top right
		rect.outside.lineTo(Vec2(o_x2, o_y2)); // bottom right
		rect.outside.lineTo(Vec2(o_x1, o_y2)); // bottom left
		rect.outside.close(); // top left, origin point
		// inside path,ccw
		rect.inside.moveTo(i.origin);
		rect.inside.lineTo(Vec2(i_x1, i_y2)); // bottom left
		rect.inside.lineTo(Vec2(i_x2, i_y2)); // bottom right
		rect.inside.lineTo(Vec2(i_x2, i_y1)); // top right
		rect.inside.close(); // top left, origin point

		/* rect outline border
			._.______________._.
			|\|______________|/|
			|-|              |-|
			| |              | |
			|_|______________|_|
			|/|______________|\|
		*/
		const float border[6] = {
			i_x1 - o_x1, // left
			i_y1 - o_y1, // top
			o_x2 - i_x2, // right
			o_y2 - i_y2, // bottom
			i_x1 - o_x1, // left
			i_y1 - o_y1, // top
		};
		const float vertex[48] = {
			o_x1,o_y1,i_x1,o_y1,i_x2,o_y1,o_x2,o_y1,i_x2,i_y1,i_x1,i_y1,// vertex,right
			o_x2,o_y1,o_x2,i_y1,o_x2,i_y2,o_x2,o_y2,i_x2,i_y2,i_x2,i_y1,// vertex,right
			o_x2,o_y2,i_x2,o_y2,i_x1,o_y2,o_x1,o_y2,i_x1,i_y2,i_x1,i_y1,// vertex,bottom
			o_x1,o_y2,o_x1,i_y2,o_x1,i_y1,o_x1,o_y1,i_x1,i_y1,i_x1,i_y2,// vertex,left
		};
		const Vec2 *vertex_p = reinterpret_cast<const Vec2*>(vertex);


		//._.______________._.
		// \|______________|/
		auto build = [](
			Array<float> *out,
			const float border[3], const Vec2 v[6], // vertex
			float offset, float inside_border_length, float direction
		) {
			if (border[1] <= 0) return offset + inside_border_length;

			if (border[0] > 0) { // if left > 0 then add triangle top,left
				const float src[15] = {
					// {x,y,length-offset,width-offset,border-direction}
					v[0].x(), v[0].y(), offset,             0, direction, // vertex 0
					v[1].x(), v[1].y(), offset + border[0], 0, direction, // vertex 1
					v[5].x(), v[5].y(), offset + border[0], 1, direction, // vertex 2
				};
				out->write(src, -1, 15);
				offset += border[0];
			}
			{
				const float src[30] = {
					// {x,y,length-offset,width-offset,border-direction}
					v[1].x(), v[1].y(), offset,                        0, direction, // vertex 0
					v[2].x(), v[2].y(), offset + inside_border_length, 0, direction, // vertex 1
					v[5].x(), v[5].y(), offset + inside_border_length, 1, direction, // vertex 2
					v[4].x(), v[4].y(), offset + inside_border_length, 1, direction, // vertex 3
					v[5].x(), v[5].y(), offset,                        1, direction, // vertex 4
					v[1].x(), v[1].y(), offset,                        0, direction, // vertex 5
				};
				out->write(src, -1, 30);
				offset += inside_border_length;
			}
			if (border[2] > 1) {
				const float src[15] = {
					// {x,y,length-offset,width-offset,border-direction}
					v[2].x(), v[2].y(), offset,              0, direction, // vertex 0
					v[3].x(), v[3].y(), offset + border[2],  0, direction, // vertex 1
					v[4].x(), v[4].y(), offset,              1, direction, // vertex 2
				};
				out->write(src, -1, 15);
				offset += border[2];
			}

			return offset;
		};

		float offset = 0; // length-offset

		for (int j = 0; j < 4; j++) {
			offset = build(
				&rect.vertex, border + j,
				vertex_p,
				offset, j % 2 ? i.size.y(): i.size.x(), j
			);
			vertex_p+=6;
		}

		Qk_ReturnLocal(rect);
	}

	static float MakeRRectOutlinePart(
		RectOutlinePath *out,
		const float border[3], const Vec2 v[6],
		const Vec2  radius[2], const Vec2 radius_inside[2],
		const float inside_radius_overflow[2],
		float offset, float inside_border_length, float direction
	) {
		// no border
		if (border[1] <= 0) return offset + inside_border_length;

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
		const int   axis = int(direction) % 2; // axis == 0 then is horizontal
		const float startAngle = Qk_PI2 - (direction * Qk_PI2);
		const float averageRadius_a = (radius[0].x() + radius[0].y()) * 0.5;
		const float averageRadius_b = (radius[1].x() + radius[1].y()) * 0.5;
		const float overflow_a = inside_radius_overflow[0];
		const float overflow_b = inside_radius_overflow[1];
		const bool  is_overflow_a = overflow_a > 0;
		const bool  is_overflow_b = overflow_b > 0;

		Vec2 v1 = v[1], v5 = v[5]; // inside radius v1 and v5 point, prev point a
		Vec2 v2 = v[2], v4 = v[4]; // inside radius v2 and v4 point, prev point b

		// ----------------------------- make left radius -----------------------------
		if (!radius[0].is_zero_or()) { // radius not zero
			Vec2 c(v[0] + radius[0]);

			float angleRatio = border[1] / (border[0] + border[1]);
			int   sample = getSampleFromRect(radius[0], 1, 0.5 * angleRatio);
			// |0|1| = sample = 3
			float sweepAngle = angleRatio * Qk_PI2; // make angle size
			float angleStep = -sweepAngle / (sample - 1);
			float angleStepLen = angleStep * averageRadius_a;
			float angleLen = angleStepLen * (sample - 1);
			float angle = startAngle + sweepAngle; // start from 0

			auto lineTo = [](Path &path, Vec2 p) { path.ptsLen() ? path.lineTo(p): path.moveTo(p); };

			if (radius_inside[0].is_zero_or()) { // no inside radius
				const float offset_v5 = angleLen + (
					!is_overflow_a ? -overflow_a: // no overflow
					atanf(overflow_a/border[1]) * averageRadius_a // overflow
				);
				for (int i = 0; i < sample; i++) {
					const Vec2 p = Vec2(
						c.x() - cosf(angle) * radius[0].x(), c.x() + sinf(angle) * radius[0].y()
					);
					const float src[15] = {
						// {x,y,length-offset,width-offset,border-direction}
						p.x(),  p.y(),  offset,    0, direction, // vertex outside
						v5.x(), v5.y(), offset_v5, 1, direction, // vertex inside
						p.x(),  p.y(),  offset,    0, direction, // vertex outside
					};
					out->vertex.write(src, -1, i == 0 ? 10: 15); // add triangle vertex
					lineTo(out->outside, p);
					angle += angleStep;
					offset += angleStepLen;
				}
				out->inside.startTo(v5);

				if (is_overflow_a) {
					// update v1 and v5 point
					v1 = Vec2(out->vertex.lastIndexAt(4), out->vertex.lastIndexAt(3));
					v5[axis] = v1[axis];
					inside_border_length -= overflow_a;
					const float src[15] = {
						v5.x(), v5.y(), offset, 1, direction, // vertex inside
					};
					out->vertex.write(src, -1, 5); // add triangle end vertex
					out->inside.lineTo(v5);
				} else {
					offset -= overflow_a;
					const float src[5] = {
						v1.x(), v1.y(), offset, 0, direction, // vertex outside
					};
					out->vertex.write(src, -1, 5); // add triangle end vertex
					out->outside.lineTo(v1);
				}
			} else {
				for (int i = 1; i <= sample; i++) {
					const float x = cosf(angle), y = sinf(angle);
					const Vec2 p[2] = {
						{c.x() - x * radius       [0].x(), c.y() + y * radius       [0].y()}, // outside
						{c.x() - x * radius_inside[0].x(), c.y() + y * radius_inside[0].y()}, // inside
					};
					const float src[30] = {
						// triangle 0 complete
						p[1].x(), p[1].y(), offset, 1, direction, // vertex inside
						// triangle 1
						v1.x(),   v1.y(), offset - angleStepLen, 0, direction, // vertex outside
						p[0].x(), p[0].y(), offset, 0, direction, // vertex outside
						p[1].x(), p[1].y(), offset, 1, direction, // vertex inside
						// start next triangle 0
						p[1].x(), p[1].y(), offset, 1, direction, // vertex inside
						p[0].x(), p[0].y(), offset, 0, direction, // vertex outside
					};

					v1 = {p[0].x(), p[0].y()};
					v5 = {p[1].x(), p[1].y()};

					if (i == 1) {
						out->vertex.write(src+20, -1, 10);
					} else if (i == sample) {
						out->vertex.write(src, -1, 20);
					} else {
						out->vertex.write(src, -1, 30);
					}
					lineTo(out->outside, p[0]);
					lineTo(out->inside, p[1]);
					angle += angleStep;
					offset += angleStepLen;
				}
				inside_border_length -= overflow_a;
			}
		} else if (border[0] > 0) { // not zero
			const float src[15] = {
				v5.x(),   v5.y(),   offset, 1, direction, // vertex inside
				v[0].x(), v[0].y(), offset, 0, direction, // vertex outside
				v1.x(),   v1.y(),   offset, 0, direction, // vertex outside
			};
			out->vertex.write(src, -1, 15); // add triangle end vertex
			out->outside.startTo(v[0]);
			out->outside.lineTo(v1);
			out->inside.lineTo(v5);
			offset += border[0];
		} else { // radius equal zero and left border equal zero
			out->outside.startTo(v1);
			out->inside.startTo(v5);
		}

		if (is_overflow_b) { // update v2 and v4 point
			v2[axis] += direction < 2 ? -overflow_b: overflow_b;
			v4[axis] = v2[axis];
			inside_border_length -= overflow_b;
		}

		// ----------------------------- make center rect -----------------------------
		{
			const float src[30] = {
				v5.x(), v5.y(), offset, 1, direction, // vertex inside
				v1.x(), v1.y(), offset, 0, direction, // vertex outside
				v2.x(), v2.y(), offset + inside_border_length, 0, direction, // vertex outside
				v2.x(), v2.y(), offset + inside_border_length, 0, direction, // vertex outside
				v4.x(), v4.y(), offset + inside_border_length, 1, direction, // vertex inside
				v5.x(), v5.y(), offset, 1, direction, // vertex inside
			};
			out->vertex.write(src, -1, 30); // add triangle end vertex
			out->outside.lineTo(v2);
			out->inside.lineTo(v4);
			offset += inside_border_length;
		}

		// ----------------------------- make right radius -----------------------------
		if (!radius[1].is_zero_or()) { // outside radius no zero
			Vec2 c(v[3] + radius[1]);

			float angleRatio = border[1] / (border[2] + border[1]);
			int   sample = getSampleFromRect(radius[1], 1, 0.5 * angleRatio);
			// |0|1| = sample = 3
			float sweepAngle = angleRatio * Qk_PI2; // make angle size
			float angleStep = -sweepAngle / (sample - 1);
			float angleStepLen = angleStep * averageRadius_b;
			//float angleLen = angleStepLen * (sample - 1);
			float angle = startAngle; // start from 0

			if (radius_inside[1].is_zero_or()) { // no inside radius
				float offset_v4 = offset;

				if (is_overflow_b) {
					offset_v4 = offset + overflow_b;
					const float src[15] = {
						v4.x(),   v4.y(),   offset, 1, direction, // vertex inside
						v2.x(),   v2.y(),   offset, 0, direction, // vertex outside
						v[4].x(), v[4].y(), offset_v4, 1, direction, // vertex inside
					};
					out->vertex.write(src, -1, 15); // add triangle end vertex
					out->inside.lineTo(v[4]);
					v4 = v[4];
				} else {
					Vec2 v2_(v2);
					v2_[axis] -= overflow_b;
					const float src[15] = {
						v4.x(),   v4.y(),   offset, 1, direction, // vertex inside
						v2.x(),   v2.y(),   offset, 0, direction, // vertex outside
						v2_.x(),  v2_.y(),  offset, 0, direction, // vertex outside
					};
					out->vertex.write(src, -1, 15); // add triangle end vertex
					out->outside.lineTo(v2_);
					offset -= overflow_b;
				}
				
				for (int i = 1; i <= sample; i++) {
					const Vec2 p = Vec2(
						c.x() - cosf(angle) * radius[0].x(), c.x() + sinf(angle) * radius[0].y()
					);
					const float src[15] = {
						// {x,y,length-offset,width-offset,border-direction}
						p.x(),  p.y(),  offset,    0, direction, // vertex outside
						v4.x(), v4.y(), offset_v4, 1, direction, // vertex inside
						p.x(),  p.y(),  offset,    0, direction, // vertex outside
					};
					out->vertex.write(src, -1, i == 1 ? 10: i == sample ? 5: 15); // add triangle vertex
					out->outside.lineTo(p);
					angle += angleStep;
					offset += angleStepLen;
				}
			} else {
				for (int i = 1; i <= sample; i++) {
					const float x = cosf(angle), y = sinf(angle);
					const Vec2 p[2] = {
						{c.x() - x * radius       [0].x(), c.y() + y * radius       [0].y()}, // outside
						{c.x() - x * radius_inside[0].x(), c.y() + y * radius_inside[0].y()}, // inside
					};
					const float src[30] = {
						// triangle 0 complete
						p[1].x(), p[1].y(), offset, 1, direction, // vertex inside
						// triangle 1
						v2.x(),   v2.y(),   offset - angleStepLen, 0, direction, // vertex outside
						p[0].x(), p[0].y(), offset, 0, direction, // vertex outside
						p[1].x(), p[1].y(), offset, 1, direction, // vertex inside
						// start next triangle 0
						p[1].x(), p[1].y(), offset, 1, direction, // vertex inside
						p[0].x(), p[0].y(), offset, 0, direction, // vertex outside
					};

					v2 = {p[0].x(), p[0].y()};
					v4 = {p[1].x(), p[1].y()};

					if (i == 1) {
						out->vertex.write(src+20, -1, 10);
					} else if (i == sample) {
						out->vertex.write(src, -1, 20);
					} else {
						out->vertex.write(src, -1, 30);
					}
					out->outside.lineTo(p[0]);
					out->inside.lineTo(p[1]);
					angle += angleStep;
					offset += angleStepLen;
				}
				// inside_border_length -= overflow_b;
			}
		} else if (border[2] > 0) {
			const float src[15] = {
				v4.x(),   v4.y(),   offset, 1, direction, // vertex inside
				v2.x(),   v2.y(),   offset, 0, direction, // vertex outside
				v[3].x(), v[3].y(), offset + border[2], 0, direction, // vertex outside
			};
			out->vertex.write(src, -1, 15); // add triangle end vertex
			out->outside.lineTo(v[3]);
			out->inside.lineTo(v5);
			offset += border[2];
		}

		return offset;
	}

	RectOutlinePath RectOutlinePath::MakeRRectOutline(
		const Rect& o, const Rect &i, const Path::BorderRadius &r
	) {
		RectOutlinePath rect;
		// contain outline length offset and width offset and border direction
		// of ext data item

		const float o_x1 = o.origin.x(),      o_y1 = o.origin.y();
		const float i_x1 = i.origin.x(),      i_y1 = i.origin.y();
		const float o_x2 = o_x1 + o.size.x(), o_y2 = o_y1 + o.size.y();
		const float i_x2 = i_x1 + i.size.x(), i_y2 = i_y1 + i.size.y();
		const float x_5  = o.size.x() * 0.5,  y_5  = o.size.y() * 0.5;
		// border width
		const float border[6] = {
			i_x1 - o_x1, i_y1 - o_y1, o_x2 - i_x2, // left,top,right
			o_y2 - i_y2, i_x1 - o_x1, i_y1 - o_y1, // bottom,left,top
		};
		// limit for outside radius
		const Vec2 leftTop = Vec2(Float::min(r.leftTop.x(), x_5), Float::min(r.leftTop.y(), y_5));
		const Vec2 rightTop = Vec2(Float::min(r.rightTop.x(), x_5), Float::min(r.rightTop.y(), y_5));
		const Vec2 rightBottom = Vec2(Float::min(r.rightBottom.x(), x_5), Float::min(r.rightBottom.y(), y_5));
		const Vec2 leftBottom = Vec2(Float::min(r.leftBottom.x(), x_5), Float::min(r.leftBottom.y(), y_5));
		// inside radius
		const Vec2 inside_leftTop = Vec2(leftTop.x() - border[0], leftTop.y() - border[1]);
		const Vec2 inside_rightTop = Vec2(rightTop.x() - border[2], rightTop.y() - border[1]);
		const Vec2 inside_rightBottom = Vec2(rightBottom.x() - border[2], rightBottom.y() - border[3]);
		const Vec2 inside_leftBottom = Vec2(leftBottom.x() - border[0], leftBottom.y() - border[3]);
		// radius
		const Vec2 radius[10] = {
			{leftTop}, // outside radius
			{-rightTop.x(), rightTop.y()},
			{-rightBottom.x(), -rightBottom.y()},
			{leftBottom.x(), -leftBottom.y()},
			{leftTop},
			// inside radius
			{ Float::max(inside_leftTop.x(),0),      Float::max(inside_leftTop.y(),0)},
			{-Float::max(inside_rightTop.x(),0),     Float::max(inside_rightTop.y(),0)},
			{-Float::max(inside_rightBottom.x(),0), -Float::max(inside_rightBottom.y(),0)},
			{ Float::max(inside_leftBottom.x(),0),  -Float::max(inside_leftBottom.y(),0)},
			{ Float::max(inside_leftTop.x(),0),      Float::max(inside_leftTop.y(),0)},
		};
		const float inside_radius_overflow[] = { // inside radius overflow for main axis
			inside_leftTop.x(),inside_rightTop.x(), // top
			inside_rightTop.y(),inside_rightBottom.y(), // right
			inside_rightBottom.x(),inside_leftBottom.x(), // bottom
			inside_leftBottom.y(),inside_leftTop.y(), // left
		};
		const float vertex[48] = { // Vec2[6] {0,1,2,3,4,5}
			o_x1,o_y1,i_x1,o_y1,i_x2,o_y1,o_x2,o_y1,i_x2,i_y1,i_x1,i_y1,// vertex,right
			o_x2,o_y1,o_x2,i_y1,o_x2,i_y2,o_x2,o_y2,i_x2,i_y2,i_x2,i_y1,// vertex,right
			o_x2,o_y2,i_x2,o_y2,i_x1,o_y2,o_x1,o_y2,i_x1,i_y2,i_x1,i_y1,// vertex,bottom
			o_x1,o_y2,o_x1,i_y2,o_x1,i_y1,o_x1,o_y1,i_x1,i_y1,i_x1,i_y2,// vertex,left
		};
		const Vec2 *vertex_p = reinterpret_cast<const Vec2*>(vertex);
		const float *inside_radius_overflow_p = inside_radius_overflow;

		float offset = 0; // length-offset

		for (int j = 0; j < 4; j++) {
			offset = MakeRRectOutlinePart(
				&rect, border + j, vertex_p,
				radius + j, radius + j + 5, inside_radius_overflow_p,
				offset, j % 2 ? i.size.y(): i.size.x(), j
			);
			vertex_p+=6;
			inside_radius_overflow_p+=2;
		}

		Qk_ReturnLocal(rect);
	}

}
