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

#include "./path.h"
#include <math.h>
#define Qk_USE_FT_STROKE 0

#if Qk_USE_FT_STROKE
extern "C" {
#include "./raster/ft_math.c"
#include "./raster/ft_stroke.c"
}
#include "./raster/ft_path.cc"

namespace qk {

	/*
	 * Alternative FreeType-based stroker kept behind Qk_USE_FT_STROKE.
	 *
	 * The active code below uses Quark's smaller hand-written stroker, but this
	 * path documents the equivalent FT flow: normalize to lines, convert to an
	 * FT outline, ask FT to expand it, then convert the result back to Path.
	 */
	Path Path::strokePath(float width, Cap cap, Join join, float miterLimit) const {
		Qk_FT_Stroker stroker;
		Qk_FT_Stroker_LineCap ft_cap = Qk_FT_Stroker_LineCap(cap);
		Qk_FT_Stroker_LineJoin ft_join =
			Join::kMiter_Join == join ? Qk_FT_Stroker_LineJoin::Qk_FT_STROKER_LINEJOIN_MITER: 
			Join::kRound_Join == join ? Qk_FT_Stroker_LineJoin::Qk_FT_STROKER_LINEJOIN_ROUND: 
			Qk_FT_STROKER_LINEJOIN_BEVEL;
		Qk_FT_Error err;

		if (miterLimit == 0)
			miterLimit = 1024;

		err = Qk_FT_Stroker_New(&stroker);
		Qk_ASSERT(err==0);
		Qk_FT_Stroker_Set(stroker, FT_1616(width * 0.5), ft_cap, ft_join, FT_1616(miterLimit));
		
		Path tmp;
		const Path *self = normalized(&tmp, 1,false);

		auto from_outline = qk_ft_outline_convert(self);
		err = Qk_FT_Stroker_ParseOutline(stroker, from_outline);
		Qk_ASSERT(err==0);

		Qk_FT_UInt anum_points, anum_contours;
		err = Qk_FT_Stroker_GetCounts(stroker, &anum_points, &anum_contours);
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
}

#endif

namespace qk {
	bool test_overlap_from_polygon(cArray<Vec2>& polygon, Vec2 point);

	/*
	 * Append `right` to `left` in reverse drawing order.
	 *
	 * strokePath() builds two offset contours while walking the source path:
	 * `left` follows one side forward, and `right` follows the other side
	 * forward. A filled stroke outline needs to go forward on one side and come
	 * back on the other, so this helper appends reverse(right) to left.
	 *
	 * This helper is only for strokePath() output. strokePath() first normalizes
	 * the source path and only creates line segments plus cubic arcs, so `right`
	 * is expected to contain Move/Line/Cubic/Close verbs, not Quad.
	 *
	 * Cubics need special handling because reversing a cubic swaps endpoint and
	 * control-point order. Close verbs are ignored here; the caller owns the
	 * final close() once both sides have been joined.
	 */
	static void reverseConcatPath(Path &left, const Path &right) {
		auto verbs = right.verbs();
		auto pts = &right.pts().back();

		for (int i = Qk_Minus(right.verbsLen(), 1); i >= 0; i--) {
			if (verbs[i] == Path::kCubic_Verb) {
				left.lineTo(*pts); pts--;
				do {
					left.cubicTo(pts[0], pts[-1], pts[-2]); pts-=3;
					i--;
				} while(verbs[i] == Path::kCubic_Verb);
			} else if (verbs[i] == Path::kClose_Verb) {
				// ignore
				//Qk_DLog("Close");
			} else {
				Qk_ASSERT(verbs[i] == Path::kLine_Verb || verbs[i] == Path::kMove_Verb);
				left.lineTo(*pts--);
			}
		}
	}

	// `prev`/`next` are null at open caps. Closed contours receive wrapped
	// neighbors from strokeExec().
	typedef void AddPoint(const Vec2 *prev, Vec2 from, const Vec2 *next, int idx, void *ctx);

	// Runs once per subpath before point callbacks. AASide uses the raw point
	// buffer to calculate winding; strokePath does not need this hook.
	typedef void BeforeAdding(bool close, const Vec2 *pts, int size, int subpath, void *ctx);

	// Runs once after every emitted subpath. strokePath uses it to seal the
	// generated left/right outline; AASide uses it to add the closing band.
	typedef void AfterDone(bool close, int size, int subpath, void *ctx);

	// Tiny duplicate filter used only by stroke generation. The tolerance is in
	// path units and intentionally small: it removes accidental repeated points
	// without trying to simplify real geometry.
	static bool strokePointEquals(Vec2 a, Vec2 b) {
		return (a - b).lengthSq() < 0.0001f;
	}

	/*
	 * Walk normalized line-only paths by subpath and expose each point with its
	 * previous/next neighbor. This is shared by stroke outline generation and
	 * AA side-band generation, which both need local edge normals.
	 *
	 * closeAll controls how implicit endings are handled:
	 * - false keeps open subpaths open unless the source has kClose_Verb.
	 * - true treats every subpath as closed when it has enough points. AASide
	 *   uses this because edge-side winding and coverage need a closed contour
	 *   even when the original path omitted close().
	 *
	 * Callback order for each valid subpath is:
	 *   before(close, pts, size, subpath)
	 *   add(prev, from, next, idx) for every point
	 *   after(close, size, subpath)
	 *
	 * When close is true, duplicate start/end points are removed before
	 * callbacks, and add() receives wrapped neighbors for the first/last point.
	 */
	static void strokeExec(
		const Path *self, AddPoint add, BeforeAdding before, AfterDone after, bool closeAll, void *ctx
	) {
		Qk_ASSERT(self->isNormalized(), "Path::strokeExec requires normalized input");
		int subpath = 0;
		auto addSubpath = [&](const Vec2 *pts, int size, bool close) {
			// Ignore empty/single-point subpaths. They do not have a stable edge
			// direction, so neither stroke nor AASide geometry can be generated.
			if (size > 1) { // size > 1
				if (close) { // close path
					// Many path builders repeat the first point before close().
					// Keep one copy so wrapped neighbor lookup stays well-defined.
					if (strokePointEquals(*pts, pts[size-1])) { // start == end, exclude duplicates
						size--;
						if (size < 2)
							return;
					}
					// A closed contour needs at least 3 unique points. Two-point
					// inputs are still emitted as an open segment.
					close = size > 2; // Must have at least 3 vertices
				}
				if (before) {
					before(close, pts, size, subpath, ctx);
				}

				if (add) {
					// First and last vertices receive null neighbors for open
					// subpaths, or wrapped neighbors for closed contours.
					add(close ? pts+size-1: nullptr, *pts, pts+1, 0, ctx); pts++;

					for (int i = 1, l = size-1; i < l; i++, pts++) {
						add(pts-1, *pts, pts+1, i, ctx);
					}
					add(pts-1, *pts, close? pts-size+1: nullptr, size-1, ctx);
				}

				if (after) {
					after(close, size, subpath, ctx);
				}
				subpath++;
			}
		};

		Array<Vec2> pts(self->ptsLen());
		auto pts0 = &self->pts().front(); // input buffer
		auto pts1 = pts.val(); // output buffer
		int  size = 0;

		for (auto v: self->verbs()) {
			switch(v) {
				case Path::kLine_Verb:
					if (size != 0) {
						if (!strokePointEquals(pts1[size-1], *pts0++)) // exclude duplicates
							pts1[size++] = pts0[-1];
						break;
					}
				case Path::kMove_Verb:
					addSubpath(pts1, size, closeAll);
					pts1[0] = *pts0++;
					size = 1;
					break;
				case Path::kClose_Verb: // close
					addSubpath(pts1, size, true);
					size = 0;
					break;
				default: Qk_Fatal("Path::strokePath");
			}
		}

		addSubpath(pts1, size, closeAll);
	}

	/**
	 * Calculate the signed area of a closed contour using the shoelace formula.
	 * Positive area indicates counter-clockwise winding, negative area indicates
	 * clockwise winding.
	 */
	static float contourArea2(const Vec2 *pts, int size) {
		float area = 0.0f;
		for (int i = 0; i < size; i++) {
			auto a = pts[i], b = pts[(i + 1) % size];
			area += a.x() * b.y() - a.y() * b.x();
		}
		return area;
	}

	/**
	 * Determine if a point is inside a closed contour.
	*/
	static bool pointInContour(Vec2 p, const Vec2 *pts, int size) {
		return test_overlap_from_polygon(ArrayWeak<Vec2>(pts, size).buffer(), p);
	}

	/**
	 * Find a point that is just inside the contour.
	 */
	static Vec2 contourInteriorProbe(const Vec2 *pts, int size, float normalSide) {
		auto a = pts[size-1];
		for (int i = 0; i < size; i++) {
			auto b = pts[i];
			auto edge = b - a;
			float edgeLenSq = edge.lengthSq();

			if (edgeLenSq > 1e-5f) {
				auto normal = edge.normalized().rotate90z();
				auto mid = (a + b) * 0.5f;
				float probe = Float32::clamp(edgeLenSq * 1e-4f, 1e-5f, 0.25f);

				// `normalSide` is the sign assigned to the +normal side. For an
				// isolated contour, the filled side is negative, so this steps
				// just inside the contour instead of sampling exactly on an edge.
				auto sample = mid + normal * (normalSide < 0.0f ? probe: -probe);

				if (pointInContour(sample, pts, size))
					return sample;

				sample = mid - normal * (normalSide < 0.0f ? probe: -probe);
				if (pointInContour(sample, pts, size))
					return sample;

				return mid;
			}
			a = b; // advance to next edge
		}

		return pts[0];
	}

	struct AASideContour {
		uint32_t start;
		int      size;
		bool     close;
		float    area;
		float    normalSide;
		Vec2     sample;
		int      nesting;
	};

	static Array<AASideContour> collectAASideContours(const Path *self, Array<Vec2> *contourPts) {
		Array<AASideContour> contours;

		struct Ctx {
			Array<AASideContour> *contours;
			Array<Vec2>         *pts;
		} ctx = { &contours, contourPts };

		strokeExec(self, nullptr, [](bool close, const Vec2 *pts, int size, int, void *ctx) {
			auto _ = static_cast<Ctx*>(ctx);
			AASideContour contour = { _->pts->length(), size, close, 0.0f, -1.0f, pts[0], 0 };

			if (close) {
				contour.area = contourArea2(pts, size);
				contour.normalSide = contour.area < 0.0f ? 1.0f: -1.0f;
				contour.sample = contourInteriorProbe(pts, size, contour.normalSide);
			}
			_->pts->write(pts, size);
			_->contours->push(contour);
		}, nullptr, true, &ctx);

		for (auto &contour: contours) {
			if (!contour.close)
				continue;

			int nesting = 0;
			for (auto &parent: contours) {
				if (&contour != &parent && parent.close) {
					auto pts = contourPts->val() + parent.start;
					if (pointInContour(contour.sample, pts, parent.size))
						nesting++;
				}
			}
			contour.nesting = nesting;
			if (nesting & 1)
				contour.normalSide = -contour.normalSide;
		}

		Qk_ReturnLocal(contours);
	}

	/**
	 * Build the conservative AA side band for a path.
	 *
	 * Each source edge produces two offset vertices. The third component is not
	 * a physical distance; it is a signed side coordinate:
	 *   - negative means the filled side of the contour
	 *   - positive means the outside side of the contour
	 *
	 * The shader can use this sign to place the 0 edge between inner and outer
	 * coverage. AASide always treats eligible subpaths as closed, because
	 * winding and inside/outside are not well-defined for open contours.
	 *
	 * TODO: When the included angle is extremely small, the normal can be
	 * shifted too far and produce visual spikes.
	 *
	 * @method getAASideTriangle() returns signed aa side stroke triangle vertices and body triangles
	 * @return {Array<Vec3>} points { x, y, aaSide }, aaSide < 0 inside, aaSide > 0 outside
	*/
	VertexData Path::getAASideTriangle(float width, float epsilon) const {
		Path boundary;
		auto self = boundaryPath(&boundary, epsilon);
		Path bodyPath;
		Array<Vec3> aaSide;
		Array<Vec2> contourPts;
		auto contours = collectAASideContours(self, &contourPts);

		struct Ctx {
			Array<Vec3>        *out;
			Array<AASideContour> *contours;
			Path               *body;
			Vec3               *ptr;
			float              width;
			float              normalSide; // sign assigned to the +normal side
			Vec3               prev_a, prev_b;
		} ctx = { &aaSide, &contours, &bodyPath, 0, width, -1.0f, 0 };

		strokeExec(self, [](const Vec2 *prev, Vec2 from, const Vec2 *next, int idx, void *ctx) { // add
			auto normals = from.normalline(prev, next); // normal line
			auto _ = (Ctx*)ctx;

			if (prev == NULL || next == NULL) { // prev == null or next == null
				// Open endpoints use the single adjacent segment normal.
				normals *= _->width;
			} else if (normals.is_zero()) {
				// 180-degree turns produce a zero averaged normal. Fall back to
				// the previous segment normal so the band remains continuous.
				auto fromPrev = from - *prev;
				normals = fromPrev.rotate90z().normalized() * _->width;
			} else {
				// The averaged vertex normal must be lengthened by 1/sin(theta)
				// so both offset edges stay `width` away from their source
				// segments at the join.
				auto angleLen = normals.angleTo(*prev - from);
				auto sinLen = fabsf(sinf(angleLen));
				normals *= sinLen < 1e-4f ? _->width: _->width / sinLen;
			}
			Vec3 a(from + normals, _->normalSide);
			Vec3 b(from - normals, -_->normalSide);
			auto inner = a.z() < 0.0f ? a: b;
			auto innerPt = Vec2(inner.x(), inner.y());
			if (idx) {
				_->body->lineTo(innerPt);
				*(_->ptr++) = _->prev_b;
				*(_->ptr++) = _->prev_a;
				*(_->ptr++) = a;
				*(_->ptr++) = a;
				*(_->ptr++) = b;
				*(_->ptr++) = _->prev_b;
			} else {
				_->body->moveTo(innerPt);
			}
			_->prev_a = a;
			_->prev_b = b;
		},
		[](bool close, const Vec2 *pts, int size, int subpath, void *ctx) { // before
			auto _ = static_cast<Ctx*>(ctx);
			if (subpath < _->contours->length()) {
				// The local contour winding tells which side is inside for a
				// single isolated polygon. Nested contours flip that meaning at
				// every containment level: nesting 0 normal, nesting 1 reversed,
				// nesting 2 normal, and so on.
				_->normalSide = (*_->contours)[subpath].normalSide;
			} else {
				_->normalSide = -1.0f;
			}
			auto len = _->out->length();
			size = (close ? size: size - 1) * 6;
			_->out->extend(len + size); // alloc memory space
			_->ptr = _->out->val() + len;
		},
		[](bool close, int size, int subpath, void *ctx) { // after
			if (close) {
				auto _ = static_cast<Ctx*>(ctx);
				// Close the final segment by connecting the last generated pair
				// back to the first generated pair in the current subpath.
				auto b = _->ptr - (size * 6 - 6), a = b + 1;
				*(_->ptr++) = _->prev_b;
				*(_->ptr++) = _->prev_a;
				*(_->ptr++) = *a;
				*(_->ptr++) = *a;
				*(_->ptr++) = *b;
				*(_->ptr++) = _->prev_b;
				_->body->close();
			}
		}, true, &ctx);

		auto out = bodyPath.getTriangles(epsilon, -1.0f);
		out.vertex.write(aaSide.val(), aaSide.length());
		out.vCount += aaSide.length();

		Qk_ReturnLocal(out);
	}

#if !Qk_USE_FT_STROKE

	/*
	 * Convert a normalized path into a filled path representing its stroke.
	 *
	 * The algorithm walks each subpath once and builds two offset outlines:
	 * `left` and `right`. For open paths, cap handling joins the ends. For
	 * closed paths, each original contour becomes a ring: left side forward,
	 * right side reversed, then close().
	 *
	 * This is intentionally a geometric path expansion, not raster AA. It feeds
	 * the normal fill tessellator after the stroke outline is built.
	 */
	Path Path::strokePath(float width, Cap cap, Join join, float miterLimit) const {
		if (miterLimit == 0)
			miterLimit = 1024.0;

		// The public stroke width is full width; all offset calculations use
		// half width from the source centerline to either side.
		miterLimit = Float32::min(miterLimit, 1024);
		width *= 0.5;

		Path tmp,out;
		auto self = normalized(&tmp, 1, false);

		struct Ctx {
			float width,miterLimit;
			Cap cap; Join join;
			Path *left,right;
		} ctx = { width, miterLimit, cap, join, &out };

		/*
		 * Result shape rules:
		 * 1. An open source subpath produces one closed stroke outline.
		 * 2. A closed source subpath produces one closed ring outline. Multiple
		 *    source contours therefore produce multiple closed outlines.
		 */

		strokeExec(self, [](const Vec2 *prev, Vec2 from, const Vec2 *next, int idx, void *ctx) {
			#define Qk_addTo(l,r) left.lineTo(l),right.lineTo(r)

			auto _ = (Ctx*)ctx;
			auto &left = *_->left;
			auto &right = _->right;
			auto width = _->width;

			if (prev == NULL || next == NULL) { // prev == null or next == null
				auto normals = from.normalline(prev, next) * width; // normal line
				switch (_->cap) {
					case Path::Cap::kButt_Cap: // no stroke extension
						Qk_addTo(from + normals, from - normals);
						return;
					case Path::Cap::kRound_Cap: { //adds circle
						// The start cap is emitted on `right`, the end cap on
						// `left`, so reverseConcatPath() can join the full
						// outline without crossing edges.
						float angle = normals.angle();
						if (prev) {
							left.arcTo({from-width,width*2}, -angle, -Qk_PI, false);
							right.lineTo(from - normals);
						} else {
							left.lineTo(from + normals);
							right.arcTo({from-width,width*2}, -angle, Qk_PI, false);
						}
						return;
					}
					default: {// adds square
						// Square caps extend half a stroke width beyond the
						// endpoint along the tangent direction.
						from += prev? normals.rotate270z(): normals.rotate90z();
						Qk_addTo(from + normals, from - normals);
						return;
					}
				}
			}

			// Average the two adjacent edge normals to get the join bisector.
			Vec2 toFrom = *next - from;
			Vec2 fromPrev = from - *prev;
			Vec2 toNext90 = toFrom.rotate90z().normalized();
			Vec2 fromPrev90 = fromPrev.rotate90z().normalized();
			Vec2 normals = (toNext90 + fromPrev90).normalized(); // normal line

			if (normals.is_zero()) {
				// Straight 180-degree reversal: emit a tiny bridge between the
				// two sides so the generated outline remains fillable.
				normals = fromPrev90 * width;
				Qk_addTo(from + normals, from - normals);
				left.lineTo(from - normals);
				right.lineTo(from + normals);
				return;
			}

			float angle    = normals.angle();
			float angleLen = angle - Vec2(-fromPrev[0],-fromPrev[1]).angle();

			if (angleLen < 0)
				angleLen += Qk_PI_2;
			float sinLen = fabsf(sinf(angleLen));
			float len = sinLen < 1e-4f ? _->miterLimit: width / sinLen;

			switch (_->join) {
				case Path::Join::kRound_Join: {// adds circle
					// Emit an arc only when the bend is large enough to matter.
					// Very small arcs fall through to the miter path below.
					if (angleLen > Qk_PI_2_1) { // outside
						auto aLen = angleLen - Qk_PI_2_1;
						if (aLen > 0.075f) { // > 0.075 radian
							normals *= len;
							left .arc(from, width, Qk_PI_2-angle+aLen, -aLen*2, false);
							right.lineTo(from - normals);
							return;
						} // else goto kMiter_Join:
					} else { // inside
						auto aLen = Qk_PI_2_1 - angleLen;
						if (aLen > 0.075f) { // > 0.075f radian
							normals *= len;
							left .lineTo(from + normals);
							right.arc(from, width, Qk_PI-angle-aLen, aLen*2, false);
							return;
						} // else goto kMiter_Join:
					}
				}
				case Path::Join::kMiter_Join: { // extends to miter limit
					if (len > _->miterLimit) { // > miter limit or default 1024
						// Clamp long miters by cutting the spike at miterLimit
						// and adding a short connecting edge.
						auto lenL = len - _->miterLimit;
						auto y = tanf(angleLen) * lenL;
						auto a = normals.rotate90z() * y;
						auto normalsL = normals * _->miterLimit;
						normals *= len;

						if (angleLen > Qk_PI_2_1) {
							Qk_addTo(from + normalsL - a, from - normals);
							left.lineTo(from + normalsL + a);
						} else {
							Qk_addTo(from + normals, from - normalsL + a);
							right.lineTo(from - normalsL - a);
						}
					} else {
						normals *= len;
						Qk_addTo(from + normals, from - normals);
					}
					return;
				}
				default: {// connects outside edges
					// Bevel join: connect the two segment offsets directly on
					// the outside of the bend.
					auto a = fromPrev90 * width;
					auto b = toNext90 * width;
					normals *= len;
					if (angleLen > Qk_PI_2_1) {
						Qk_addTo(from + a, from - normals);
						left.lineTo(from + b);
					} else {
						Qk_addTo(from + normals, from - a);
						right.lineTo(from - b);
					}
				}
			}
			#undef Qk_addTo
		}, NULL, [](bool close, int size, int subpath, void *ctx) {
			auto _ = static_cast<Ctx*>(ctx);
			if (close) {
				// For closed source contours, close the first side before
				// appending the reversed opposite side.
				_->left->close();
			}
			reverseConcatPath(*_->left, _->right); // concat paths, left += reverse(right)
			_->left->close(); // close path
			_->right = Path(); // clear right path
		}, false, &ctx);

		Qk_ReturnLocal(out);
	}

#endif

}
