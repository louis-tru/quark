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

#include "tesselator.h"
#include "./path.h"
#include "./bezier.h"
#include <math.h>

namespace qk {

	Path Path::MakeOval(const Rect& r, bool ccw) {
		Path path;
		path.ovalTo(r, ccw);
		path.close();
		Qk_ReturnLocal(path);
	}

	Path Path::MakeCircle(Vec2 center, float radius, bool ccw) {
		return MakeOval({ Vec2(center.x() - radius, center.y() - radius), Vec2(radius) * 2 }, ccw);
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

	static void setRRect(Path &path,
		const Rect& outside, const Rect *inside, const Path::BorderRadius& br)
	{
		if (outside.size.is_zero_axis()) return;
		if (inside && inside->size.is_zero_axis()) return;

		auto arc = [&](Vec2 origin, Vec2 radius, Vec2 dir, float startAngle, float sweepAngle) {
			if (radius.x() != 0 && radius.y() != 0) {
				Vec2 s = radius*2;
				path.arcTo({origin+s*dir, s}, startAngle, sweepAngle, false);
			} else {
				auto len = path.verbsLen();
				if (len && path.verbs()[len] != Path::kClose_Verb) {
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
		Vec2 origin = outside.begin;
		Vec2 end = origin + outside.size;

		// cw
		arc(origin, a, Vec2(0), Qk_PI, -Qk_PI_2_1); // left-top
		arc({end.x(), origin.y()}, b, Vec2(-1,0), Qk_PI_2_1, -Qk_PI_2_1); // right-top
		arc(end, c, Vec2(-1), 0, -Qk_PI_2_1); // right-bottom
		arc({origin.x(), end.y()}, d, Vec2(0,-1), -Qk_PI_2_1, -Qk_PI_2_1); // left-bottom

		path.close();

		if (!inside)
			return;

		Vec2 inside_origin = inside->begin;
		Vec2 inside_end = inside_origin + inside->size;

		float top    = inside_origin.y() - origin.y();
		float right  = end.x() - inside_end.x();
		float bottom = end.y() - inside_end.y();
		float left   = inside_origin.x() - origin.x();

		origin = inside_origin;
		end    = inside_end;

		a = { F32::max(a.x() - left, 0.0), F32::max(a.y() - top, 0.0) }; // left/top
		b = { F32::max(b.x() - right, 0.0), F32::max(b.y() - top, 0.0) }; // left/bottom
		c = { F32::max(c.x() - right, 0.0), F32::max(c.y() - bottom, 0.0) }; // right/bottom
		d = { F32::max(d.x() - left, 0.0), F32::max(d.y() - bottom, 0.0) }; // right/top

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

	Path::Path(Vec2 move): _IsNormalized(true), _sealed(false), _isBoundaryPath(false) {
		moveTo(move);
	}

	Path::Path(): _IsNormalized(true), _sealed(false), _isBoundaryPath(false) {}

	Path::Path(const Path& path)
		: _IsNormalized(path._IsNormalized), _sealed(false), _isBoundaryPath(path._isBoundaryPath)
		, _verbs(path._verbs), _pts(path._pts), _hash(path._hash) {}

	Path& Path::operator=(const Path& path) {
		if (this != &path) {
			_IsNormalized = path._IsNormalized;
			_sealed = false;
			_isBoundaryPath = path._isBoundaryPath;
			_verbs = path._verbs;
			_pts = path._pts;
			_hash = path._hash;
		}
		return *this;
	}

	void Path::moveTo(Vec2 to) {
		if (_sealed) return;
		// _pts.push(to.x()); _pts.push(to.y());
		_pts.push(to);
		_verbs.push(kMove_Verb);
		_hash.update2f(to.val);
	}

	void Path::lineTo(Vec2 to) {
		if (_sealed) return;
		//if (_pts.length() && *(uint64_t*)&_pts.lastIndexAt(1) == *(uint64_t*)to.val)
		//	return;
		_pts.push(to);
		_verbs.push(kLine_Verb);
		_hash.update2f(to.val);
	}

	void Path::quadTo(Vec2 control, Vec2 to) {
		if (_sealed) return;
		_pts.push(control);
		_pts.push(to);
		_verbs.push(kQuad_Verb);
		_IsNormalized = false;
		// _hash.update((&_pts.back()) - 4, sizeof(float) * 4);
		//_hash.update((uint32_t*)(&_pts.back()) - 4, 4);
		_hash.update2f(control.val);
		_hash.update2f(to.val);
	}

	void Path::cubicTo(Vec2 control1, Vec2 control2, Vec2 to) {
		if (_sealed) return;
		//_pts.push(control1[0]); _pts.push(control1[1]);
		//_pts.push(control2[0]); _pts.push(control2[1]);
		//_pts.push(to[0]); _pts.push(to[1]);
		_pts.push(control1);
		_pts.push(control2);
		_pts.push(to);
		_verbs.push(kCubic_Verb);
		_IsNormalized = false;
		// _hash.update((uint32_t*)(&_pts.back()) - 6, 6);
		_hash.update2f(control1.val);
		_hash.update2f(control2.val);
		_hash.update2f(to.val);
	}

	constexpr float magicCircle = 0.551915024494f; // 0.552284749831f

	void Path::ovalTo(const Rect& r, bool ccw) {
		if (_sealed) return;
		if (r.size.is_zero_axis()) return;

		float w = r.size.x(), h = r.size.y();
		float x = r.begin.x(), y = r.begin.y();
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
		if (_sealed) return;
		if (r.size.is_zero_axis()) return;

		lineTo(r.begin);
		float x2 = r.begin.x() + r.size.x();
		float y2 = r.begin.y() + r.size.y();
		if (ccw) {
			lineTo(Vec2(r.begin.x(), y2)); // bottom left
			lineTo(Vec2(x2, y2)); // bottom right
			lineTo(Vec2(x2, r.begin.y())); // top right
		} else {
			lineTo(Vec2(x2, r.begin.y())); // top right
			lineTo(Vec2(x2, y2)); // bottom right
			lineTo(Vec2(r.begin.x(), y2)); // bottom left
		}
		lineTo(r.begin); // top left, origin point
	}

	void Path::arc(Vec2 center, Vec2 radius, float startAngle, float sweepAngle, bool useCenter) {
		if (_sealed) return;
		float cx = center.x();
		float cy = center.y();

		if (radius.is_zero_axis()) {
			lineTo(Vec2(cx, cy));
			return;
		}
		sweepAngle = F32::clamp(sweepAngle, -Qk_PI_2, Qk_PI_2);
		startAngle = -startAngle;
		float rx = radius.x();
		float ry = radius.y();
		float x0 = cosf(startAngle);
		float y0 = sinf(startAngle);

		Vec2 start(x0 * rx + cx, y0 * ry + cy);

		if (abs(sweepAngle) == Qk_PI_2)
			useCenter = false;

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
		arc(r.begin+radius, radius, startAngle, sweepAngle, useCenter);
	}

	void Path::quadTo2(float *p) {
		Qk_ASSERT(!_sealed, "Path::quadTo2() sealed path can not be modified");
		_pts.write((Vec2*)p, 2);
		_verbs.push(kQuad_Verb);
		_IsNormalized = false;
		// _hash.updateu32v((uint32_t*)p, 4);
		// _hash.updateu32v((uint32_t*)p, 4);
		_hash.update2f(p);
		_hash.update2f(p+2);
	}

	void Path::cubicTo2(float *p) {
		Qk_ASSERT(!_sealed, "Path::cubicTo2() sealed path can not be modified");
		_pts.write((Vec2*)p, 3);
		_verbs.push(kCubic_Verb);
		_IsNormalized = false;
		//_hash.update((uint32_t*)p, 6);
		_hash.update2f(p);
		_hash.update2f(p+2);
		_hash.update2f(p+3);
	}

	void Path::close() {
		if (_sealed) return;
		_verbs.push(kClose_Verb);
	}

	void Path::concat(const Path& path) {
		if (_sealed) return;
		_verbs.write(path._verbs.val(), path._verbs.length());
		_pts.write(path._pts.val(), path._pts.length());
		_hash.updateu64(path.hashCode());
		_IsNormalized = _IsNormalized && path._IsNormalized;
	}

	cArray<Path::PathVerb>& Path::verbs() const {
		return (cArray<PathVerb>&)_verbs;
	}

	Array<Vec2> Path::getEdgeLines(float precision, const Mat* matrix) const {
		Path tmp;
		const Path *self = normalized(&tmp, precision, false);
		self = matrix ? self->transformPath(&tmp, *matrix) : self; // transform path
		Array<Vec2> edges;
		auto pts = (const Vec2*)*self->_pts;
		bool isZero = true;
		Vec2 move, from;

		for (auto verb: self->_verbs) {
			switch(verb) {
				case kLine_Verb:
					if (!isZero) {
						edges.push(from);
						from = *pts++;
						edges.push(from); // edge 0
						break;
					}
				case kMove_Verb:
					move = from = *pts++;
					isZero = false;
					break;
				case kClose_Verb: // close
					if (!isZero) {
						if (move != from) { // close path
							edges.push(from);
							edges.push(move);
						}
						isZero = true;
					}
					break;
				default:
					Qk_Fatal("Path::getEdgeLines() invalid verb");
			}
		}

		Qk_ReturnLocal(edges);
	}

	bool tessAddPathContours(TESStesselator *tess, const Path *path, float z = 0) {
		auto pts = (const Vec2*)*path->pts();
		int len = 0;
		Array<Vec3> vertex; // temporary vertex buffer for tessAddContour, Vec3: x,y,z
		bool added = false;

		auto addContour = [&]() {
			if (len) {
				auto *point = &vertex[vertex.length() - len];
				tessAddContour(tess, 3, (float*)point, sizeof(Vec3), len);
				len = 0;
				added = true;
			}
		};

		for (auto verb: path->verbs()) {
			switch(verb) {
				case Path::kMove_Verb:
					addContour();
					vertex.push(Vec3(*pts++, z)); len = 1;
					break;
				case Path::kLine_Verb:
					vertex.push(Vec3(*pts++, z));
					len++;
					break;
				case Path::kClose_Verb:
					addContour();
					break;
				default:
					Qk_Fatal("Path::tessAddPathContours() invalid verb");
			}
		}
		addContour();

		return added;
	}

	const Path* Path::boundaryPath(Path *out, float precision) const {
		Qk_ASSERT(!out->_sealed, "Path::boundaryPath() requires a non-sealed output");
		Qk_ASSERT(out != this, "Path::boundaryPath() requires a separate output");
		if (_isBoundaryPath)
			return this;

		Path tmp;
		auto self = normalized(&tmp, precision, false);
		auto tess = tessNewTess(nullptr);

		if (tessAddPathContours(tess, self) &&
			tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_BOUNDARY_CONTOURS, 0, 2, 0)
		) {
			const int nelems = tessGetElementCount(tess);
			const TESSindex* elems = tessGetElements(tess);
			const Vec2* verts = (const Vec2*)tessGetVertices(tess);

			for (int i = 0; i < nelems; i++) {
				const TESSindex base = elems[i * 2];
				const TESSindex count = elems[i * 2 + 1];
				if (count < 3)
					continue;

				out->moveTo(verts[base]);
				for (int j = 1; j < count; j++) {
					out->lineTo(verts[base + j]);
				}
				out->close();
			}
		}

		tessDeleteTess(tess);
		out->_isBoundaryPath = true;
		out->_sealed = true;
		return out;
	}

	VertexData Path::getTriangles(float precision, float z) const {
		Path tmp;
		auto *self = normalized(&tmp, precision, false);
		VertexData out;

		int polySize = 3;
		auto tess = tessNewTess(nullptr); // TESStesselator*
		tessAddPathContours(tess, self);

		// Convert to convex contour vertex data
		if ( tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, polySize, 2, 0) ) {
			const int nelems = tessGetElementCount(tess);
			const TESSindex* elems = tessGetElements(tess);
			const Vec2* verts = (const Vec2*)tessGetVertices(tess);

			out.vCount = nelems * polySize;
			out.vertex.extend(out.vCount);

			for (int i = 0; i < out.vCount; i++) {
				out.vertex[i] = { verts[*elems++], z };
			}
		}

		tessDeleteTess(tess);

		Qk_ReturnLocal(out);
	}

	Path Path::dashPath(float *stageP, int stageCount, float offset) const {
		Path tmp, out;
		const Path *self = normalized(&tmp, 1,false);
		auto pts = (const Vec2*)*self->_pts;
		int  stageIdx = -1;
		bool useStage = false; // false: gap, true: dash
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
				auto use = F32::min(offset, stage);
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
				float use = F32::min(len - useLen, stage);
				useLen += use; stage -= use;
				from = start + point * Vec2(useLen / len);
			}
		};

		for (auto verb: self->_verbs) {
			switch (verb) {
				case kLine_Verb:
					if (!isZero) {
						lineTo(from, *pts);
						from = *pts++;
						break;
					}
				case kMove_Verb:
					move = from = *pts++;
					isZero = false;
					break;
				case kClose_Verb:
					if (!isZero) {
						lineTo(from, move);
						isZero = true;
					}
					break;
				default: Qk_Fatal("Path::dashPath");
			}
		}

		if (out.verbsLen() && useStage) { // continue lineTo while(len > useLen)
			out.lineTo(from);
		}

		Qk_ReturnLocal(out);
	}

	Path& Path::normalizedPath(float precision) {
		if (_sealed) return *this;
		if (!_IsNormalized) {
			Path tmp;
			normalized(&tmp, precision, true);
			*this = std::move(tmp);
		}
		return *this;
	}

	void Path::transform(const Mat& matrix) {
		if (_sealed)
			return;
		transformPath(this, matrix);
	}

	void Path::scale(Vec2 scale) {
		if (_sealed)
			return;
		if (scale == Vec2(1.0f))
			return;
		Vec2* pts = *_pts;
		Vec2* e = pts + _pts.length();
		while (pts < e) {
			pts[0] *= scale[0];
			pts[1] *= scale[1];
			pts++;
		}
	}

	void Path::seal() {
		_sealed = true;
	}

	Range Path::getBounds(const Mat* mat) const {
		mat = mat && !mat->is_identity_matrix() ? mat: nullptr;
		return getBoundsFromPoints((const Vec2*)*_pts, ptsLen(), mat);
	}

	// get rect bounds from pts
	Range Path::getBoundsFromPoints(const Vec2 *pts, uint32_t ptsLen, const Mat* mat) {
		if (ptsLen == 0) {
			return {0,0}; // empty range
		}

		bool isMul = false;
		if (mat) {
			// if not only translate, need mul_vec2_no_translate
			isMul = !mat->is_translation_matrix();
		}
		const Vec2* e = pts + ptsLen;
		auto begin = isMul ? mat->mul_vec2_no_translate(*pts): *pts;
		auto end = begin;

		while (++pts < e) {
			auto p = isMul ? mat->mul_vec2_no_translate(*pts): *pts;
			auto x = p.x(), y = p.y();
			if (x < begin[0]) {
				begin[0] = x;
			} else if (x > end[0]) {
				end[0] = x;
			}
			if (y < begin[1]) {
				begin[1] = y;
			} else if (y > end[1]) {
				end[1] = y;
			}
		}
		if (mat) {
			Vec2 offset{mat->val[2], mat->val[5]}; // translate
			begin += offset; // apply translate to bounds
			end += offset;
		}
		return {begin,end};
	}

	// estimate sample rate
	static int getQuadraticBezierSample(const QuadraticBezier& curve, float precision);
	static int getCubicBezierSample(const CubicBezier& curve, float precision);

	/**
	 * The connections point between multiple different paths may not be standard, 
	 * so grid points are used here to make points that are very close to each other become the same.
	*/
	static void grid_point(Vec2 &p) {
		p[0] = roundf(p[0] * 32.0) / 32.0;
		p[1] = roundf(p[1] * 32.0) / 32.0;
	}

	static bool isPointEquals(Vec2 a, Vec2 b) {
		return (a - b).lengthSq() < 0.01f;
	}

	const Path* Path::normalized(Path *out, float precision, bool updateHash) const {
		Qk_ASSERT(!out->_sealed, "Path::normalized() sealed path can not be modified");
		Qk_ASSERT(out != this, "Path::normalized() output path must be different from input path");
		if (_IsNormalized)
			return this;
		Path &line = *out;

		auto pts = ((Vec2*)_pts.val());
		bool isZero = true;
		static Vec2 Zero;

		auto add = [&](Vec2 &to, PathVerb verb) {
			grid_point(to);
			line._pts.push(to);
			line._verbs.push(verb);

			if (updateHash)
				line._hash.update2f(to.val);
		};

		for (auto verb: _verbs) {
			switch(verb) {
				case kMove_Verb:
					add(*pts++, kMove_Verb);
					isZero = false;
					break;
				case kLine_Verb:
					add(*pts++, isZero ? (isZero = false), kMove_Verb: kLine_Verb); // add move or line
					break;
				case kQuad_Verb: { // quadratic bezier
					if (isZero)
						add(Zero, kMove_Verb);
					QuadraticBezier bezier(isZero ? Vec2(): pts[-1], pts[0], pts[1]);
					pts+=2;
					int sample = getQuadraticBezierSample(bezier, precision) - 1;
					// |0|1| = sample = 3
					line._pts.extend(line._pts.length() + sample);
					auto points = &line._pts[line._pts.length() - sample];
					bezier.sample_curve_points(sample+1, (float*)(points - 1));
					grid_point(line._pts.back());
					if (updateHash)
						line._hash.updateu64v((uint64_t*)points, sample); // update hash
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kLine_Verb, sample);
					isZero = false;
					break;
				}
				case kCubic_Verb: { // cubic bezier
					if (isZero)
						add(Zero, kMove_Verb);
					CubicBezier bezier(isZero ? Vec2(): pts[-1], pts[0], pts[1], pts[2]);
					pts+=3;
					int sample = getCubicBezierSample(bezier, precision) - 1;
					// |0|1| = sample = 3
					line._pts.extend(line._pts.length() + sample);
					auto points = &line._pts[line._pts.length() - sample];
					bezier.sample_curve_points(sample+1, (float*)(points - 1));
					grid_point(line._pts.back());
					if (updateHash)
						line._hash.updateu64v((uint64_t*)points, sample); // update hash
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kLine_Verb, sample);
					isZero = false;
					break;
				}
				default: // close
					line._verbs.push(kClose_Verb);
					isZero = true;
					break;
			}
		}
		
		/*if (out->_pts.length() > 1) {
			if (out->_pts.front() == out->_pts.back()) {
				out->_pts.pop();
				out->_verbs.pop();
			}
		}*/

		return out;
	}

	const Path* Path::transformPath(Path *out, const Mat& matrix) const {
		if (matrix.is_identity_matrix())
			return this;
		if (out != this)
			*out = *this; // copy first, then transform
		Vec2* pts = *out->_pts;
		Vec2* e = pts + out->_pts.length();
		if (matrix.is_translation_matrix()) { // only translate, no multiply
			float tx = matrix.val[2],
						ty = matrix.val[5];
			while (pts < e) {
				pts[0][0] += tx;
				pts[0][1] += ty;
				pts++;
			}
		} else {
			while (pts < e) {
				*pts = matrix * (*pts);
				pts++;
			}
		}
		return out;
	}

	static constexpr float bezierBasePrecision = 16.0f;

	static int getQuadraticBezierSample(const QuadraticBezier& curve, float precision) {
		Vec2 v = curve.p0() - curve.p1() * 2.0f + curve.p2();
		precision *= bezierBasePrecision;
		float segmentsP4 = v.lengthSq() * precision * precision * 0.0625f;
		int segments = ceilf(sqrtf(sqrtf(segmentsP4)));
		return std::max(segments, 2);
	}

	static int getCubicBezierSample(const CubicBezier& curve, float precision) {
		Vec2 A = curve.p0(), B = curve.p1(), C = curve.p2(), D = curve.p3();
		Vec2 v0 = A - B * 2.0f + C;
		Vec2 v1 = B - C * 2.0f + D;
		precision *= bezierBasePrecision;
		float segmentsP4 =
			std::max(v0.lengthSq(), v1.lengthSq()) *
			precision * precision * 0.5625f;
		int segments = ceilf(sqrtf(sqrtf(segmentsP4)));
		return std::max(segments, 3);
	}

	/**
	 * @param radian {float} maximum PI/2
	*/
	static int getRadianSample(Vec2 radius, float radian) {
		float S_2 = abs(radius.x() * radius.y() * radian * 0.25); // width * height
		if (S_2 < 5000.0) { // circle radius < 80
			constexpr float count = 30.0 / 8.408964152537145;//sqrtf(sqrtf(5000.0));
			int i = std::max(sqrtf(sqrtf(S_2)) * count, 3.0f);
			return i;
		} else {
			return 30;
		}
	}

	// ------------------- R e c t . O u t l i n e . P a t h -------------------

	constexpr float Z = -1.0f; // defaultAASideZ = -1.0f;

	RectPath RectPath::MakeRect(const Rect &rect) {
		RectPath out;
		out.rect = rect;
		out.flags = 0;
		if (rect.size.x() <= 0 || rect.size.y() <= 0) {
			Qk_ReturnLocal(out);
		}
		float x2 = rect.begin.x() + rect.size.x();
		float y2 = rect.begin.y() + rect.size.y();
		// path
		out.moveTo(rect.begin);
		out.lineTo(Vec2(x2, rect.begin.y())); // top right
		out.lineTo(Vec2(x2, y2)); // bottom right
		out.lineTo(Vec2(rect.begin.x(), y2)); // bottom left
		out.close(); // top left, origin point
		Qk_ReturnLocal(out);
	}

	RectPath RectPath::MakeRRect(const Rect &rect, const Path::BorderRadius &r) {
		RectPath out;
		out.rect = rect;
		out.flags = 0;

		if (rect.size.x() <= 0 || rect.size.y() <= 0) {
			Qk_ReturnLocal(out);
		}

		float x1 = rect.begin.x(),      y1 = rect.begin.y();
		float x2 = x1 + rect.size.x(),  y2 = y1 + rect.size.y();

		/* vertex approximate location
		 ____________0__
		|      /      \ |
		3               |
		|\       /      1
		|__2____________|
		*/
		static auto equals = [](Path& path, Vec2 &p) {
			return path.ptsLen() && isPointEquals(path._pts.back(), p);
		};

		auto build = [](RectPath *out, Vec2 center, Vec2 radius, Vec2 v, float angle, int mask) {
			if (radius[0] > 0 && radius[1] > 0) { // no zero
				center = center * radius + v;
				int   sample = getRadianSample(radius, Qk_PI_2_1); // |0|1| = sample = 3
				float angleStep = Qk_PI_2_1 / (sample - 1);
				for (int i = 0; i < sample; i++) {
					Vec2 p(
						center.x() + cosf(angle) * radius.x(),
						center.y() - sinf(angle) * radius.y()
					);
					// First point can ignore, because it is repeat
					if (i || !equals(*out, p)) {
						out->lineTo(p);
					}
					angle += angleStep;
				}
				out->flags |= mask;
			} else {
				out->lineTo(v);
			}
		};

		build(&out, {1    }, r.leftTop,     {x1,y1}, Qk_PI_2_1, 1 << 0); // top
		build(&out, {1, -1}, r.leftBottom,  {x1,y2}, Qk_PI, 1 << 3); // left
		build(&out, {-1   }, r.rightBottom, {x2,y2}, -Qk_PI_2_1, 1 << 2); // bottom
		build(&out, {-1, 1}, r.rightTop,    {x2,y1}, 0, 1 << 1); // right

		out.close();

		Qk_ReturnLocal(out);
	}

	RectOutlinePath RectOutlinePath::MakeRectOutline(const Rect &rect, const float border[4]) {
		RectOutlinePath outline;
		outline.flags = 0;

		float o_x1 = rect.begin.x(),       o_y1 = rect.begin.y();
		float i_x1 = o_x1 + border[3],     i_y1 = o_y1 + border[0];
		float o_x2 = o_x1 + rect.size.x(), o_y2 = o_y1 + rect.size.y();
		float i_x2 = o_x2 - border[1],     i_y2 = o_y2 - border[2];

		float vertexfv[] = {
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
			if (border[j] > 0) { // have border
				out->moveTo(v[0]);
				out->lineTo(v[3]);
				// out->moveTo(v[3]); // TODO: fix aa sdf stroke error
				out->lineTo(v[4]);
				out->lineTo(v[5]);
				out->close(); //out->lineTo(v[0]);
				outline.flags |= 1 << j;
			} else {
				out->moveTo(v[1]);
				out->lineTo(v[2]);
			}
			v+=6;
		}

		Qk_ReturnLocal(outline);
	}

	RectOutlinePath RectOutlinePath::MakeRRectOutline(const Rect& rect, const float border[4],
			const Path::BorderRadius &r)
	{
		RectOutlinePath outline;
		outline.flags = 0;

		float o_x1 = rect.begin.x(),       o_y1 = rect.begin.y();
		float i_x1 = o_x1 + border[3],     i_y1 = o_y1 + border[0];
		float o_x2 = o_x1 + rect.size.x(), o_y2 = o_y1 + rect.size.y();
		float i_x2 = o_x2 - border[1],     i_y2 = o_y2 - border[2];

		float vertexfv[] = {
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
			Path &path, const float border[3], const Vec2 v[6],
			const Vec2 radius[2], const Vec2 radius_i[2], const Vec2 center[2], float startAngle
		) {
			auto isBorder = border[1] > 0; // border is zero
			Array<Vec2> pathInside; // border path
			auto isRadiusZeroL = radius[0].is_zero_axis();
			auto isRadiusZeroR = radius[1].is_zero_axis();

			if (isRadiusZeroL) { // radius is zero
				if (isBorder) {
					pathInside.push(v[5]);
				}
				path.moveTo(v[0]);
			} else {
				auto borderSum = border[0] + border[1];
				auto sweep = borderSum == 0 ? (Qk_PI_2_1 * 0.5): border[1] / borderSum * Qk_PI_2_1;
				int  sample = getRadianSample(radius[0], sweep); // |0|1| = sample = 3
				float angleStep = -sweep / (sample - 1);
				float angle = startAngle + sweep;
				bool isRadiusI = radius_i[0].x() > 0 && radius_i[0].y() > 0;

				for (int i = 0; i < sample; i++) {
					Vec2 xy(cosf(angle), -sinf(angle));
					Vec2 v0 = xy * radius[0] + center[0];

					if (isBorder) {
						if (i == 0) {
							Vec2 v1 = isRadiusI ? xy * radius_i[0] + center[0]: v[5];
							pathInside.push(v1);
						} else if (isRadiusI) {
							Vec2 v1 = xy * radius_i[0] + center[0];
							pathInside.push(v1);
						}
					}
					path.lineTo(v0);
					angle += angleStep;
				}
			}

			if (isRadiusZeroR) { // radius is zero
				if (isBorder && !isPointEquals(pathInside.back(), v[4])) {
					pathInside.push(v[4]);
				}
				if (!isPointEquals(path.pts().back(), v[3])) {
					path.lineTo(v[3]);
				}
			} else {
				auto borderSum = border[2] + border[1];
				auto sweep = borderSum == 0 ? (Qk_PI_2_1 * 0.5): border[1] / borderSum * Qk_PI_2_1;
				int  sample = getRadianSample(radius[1], sweep); // |0|1| = sample = 3
				float angleStep = -sweep / (sample - 1);
				float angle = startAngle;
				bool isRadiusI = radius_i[1].x() > 0 && radius_i[1].y() > 0;

				for (int i = 0; i < sample; i++) {
					Vec2 xy(cosf(angle), -sinf(angle));
					Vec2 v0 = xy * radius[1] + center[1], v1;

					if (isBorder) {
						if (isRadiusI) {
							v1 = xy * radius_i[1] + center[1];
							RadiusI:
							if (i != 0 || !isPointEquals(pathInside.back(), v1)) {
								pathInside.push(v1);
							}
						} else { // inside radius is zero
							if (i == 0) {
								v1 = v[4];
								goto RadiusI;
							}
						}
					}
					if (i != 0 || !isPointEquals(path.pts().back(), v0)) {
						path.lineTo(v0);
					}
					angle += angleStep;
				}
			}

			if (isBorder) {
				// if (border[2] < 0.1f && !isRadiusZeroR) // fix aa sdf stroke error
				// 	path.moveTo(pathInside.back());
				for (int i = pathInside.length() - 1; i >= 0; i--)
					path.lineTo(pathInside[i]);
				// if (border[0] > 0.1f || isRadiusZeroL) // fix aa sdf stroke error
				// 	path.lineTo(path.pts().front()); // equivalent to close
			}
		};

		float angle = Qk_PI_2_1;
		for (int j = 0; j < 4; j++) {
			auto out = &outline.top + j;
			outline.flags |= border[j] > 0 ? 1 << j : 0; // have border
			build(*out, Bo+j, vertex, oR+j, iR+j, Ce+j, angle);
			vertex+=6;
			angle -= Qk_PI_2_1;
		}

		Qk_ReturnLocal(outline);
	}

}
