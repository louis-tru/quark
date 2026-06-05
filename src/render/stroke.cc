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
#include "./raster/ft_path.h"
#define Qk_USE_FT_STROKE 1
#if Qk_USE_FT_STROKE
extern "C" {
#include "./raster/ft_math.c"
#include "./raster/ft_stroke.c"
}
#include "./raster/ft_path.cc"
#endif

namespace qk {
	// Tests if a point is inside a polygon using the ray-casting algorithm.
	bool test_overlap_from_polygon(cArray<Vec2>& polygon, Vec2 point);

	/**
	 * Determine if a point is inside a closed contour.
	*/
	static bool pointInContour(Vec2 p, const Vec2 *pts, int size) {
		return test_overlap_from_polygon(ArrayWeak<Vec2>(pts, size).buffer(), p);
	}

	// Duplicate filter used only by stroke/AASide generation. The tolerance is
	// intentionally wider than an exact equality test: repeated or almost
	// repeated vertices make vertex normals unstable and can create long spikes.
	static bool strokePointEquals(Vec2 a, Vec2 b) {
		return (a - b).lengthSq() < 0.01f;
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

	// `prev`/`next` are null at open caps. Closed contours receive wrapped
	// neighbors from strokeExec().
	// return false means skip the point, true means add the point
	typedef bool AddPoint(Vec2 *prev, Vec2 *from, Vec2 *next, int idx, void *ctx);

	// Runs once per subpath before point callbacks. AASide uses the raw point
	// buffer to calculate winding; strokePath does not need this hook.
	typedef void BeforeAdding(bool close, Vec2 *pts, int size, int subpath, void *ctx);

	// Runs once after every emitted subpath. strokePath uses it to seal the
	// generated left/right outline; AASide uses it to add the closing band.
	typedef void AfterDone(bool close, int size, int add, int subpath, void *ctx);

	// Walk a single subpath and expose each point with its previous/next neighbor.
	static int addPoints(bool close, Vec2* pts, int size, AddPoint add, void *ctx) {
		if (!add) return 0;
		int idx = 0;
		auto prev = close ? pts+size-1: nullptr;
		auto from = pts;
		for (int i = 1; i < size; i++) {
			// The add() callback can return false to skip target point.
			if (add(prev, from, ++pts, idx, ctx)) {
				idx++;
				prev = from;
				from = pts;
			}
		}
		if (idx) { // add last point if any points were added
			add(prev, from, close? pts-size+1: nullptr, idx++, ctx);
		}
		return idx;
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
	static void eachSubpath(
		const Path *self, Array<Vec2> *contourPts, // output buffer for raw contour points, used by AASide
		AddPoint add, BeforeAdding before, AfterDone after, bool closeAll, void *ctx
	) {
		Qk_ASSERT(self->isNormalized(), "Path::strokeExec requires normalized input");

		int subpath = 0;
		auto addSubpath = [&](Vec2 *pts, int size, bool close) {
			// Ignore empty/single-point subpaths. They do not have a stable edge
			// direction, so neither stroke nor AASide geometry can be generated.
			if (size < 2) { // size < 2
				return 0;
			}
			if (close) { // close path
				// Many path builders repeat the first point before close().
				// Keep one copy so wrapped neighbor lookup stays well-defined.
				if (strokePointEquals(*pts, pts[size-1])) { // start == end, exclude duplicates
					size--;
					if (size < 2)
						return 0;
				}
				// A closed contour needs at least 3 unique points. Two-point
				// inputs are still emitted as an open segment.
				close = size > 2; // Must have at least 3 vertices
			}
			if (before) {
				before(close, pts, size, subpath, ctx);
			}
			int addSize = addPoints(close, pts, size, add, ctx);
			if (after) {
				after(close, size, addSize, subpath, ctx);
			}
			subpath++;
			return size;
		};

		contourPts->extend(self->ptsLen()); // pre-allocate output buffer
		auto pts0 = &self->pts().front(); // input buffer
		auto pts1 = contourPts->val(); // output buffer
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
					pts1 += addSubpath(pts1, size, closeAll);
					pts1[0] = *pts0++;
					size = 1;
					break;
				case Path::kClose_Verb: // close
					pts1 += addSubpath(pts1, size, true);
					size = 0;
					break;
				default: Qk_Fatal("Path::strokePath");
			}
		}

		addSubpath(pts1, size, closeAll);
	}

	/**
	 * Walk the path by subpath and expose each point with its previous/next neighbor.
	 */
	static void eachContour(Array<PathContour> &contours, AddPoint add, BeforeAdding before, AfterDone after, void *ctx)
	{
		for (int i = 0; i < contours.length(); i++) {
			auto &c = contours[i];
			if (before) {
				before(c.close, c.pts, c.ptsNum, i, ctx);
			}
			int addSize = addPoints(c.close, c.pts, c.ptsNum, add, ctx);
			if (after) {
				after(c.close, c.ptsNum, addSize, i, ctx);
			}
		 }
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

	/**
	 * Collect raw contour points for each closed subpath and calculate contour
	 * properties used by AASide.
	 *
	 * AASide geometry already follows each contour's traversal direction: when
	 * a hole has the opposite winding, its generated normals are opposite too.
	 * Because of that, the CPU only needs one global sign calibration:
	 *
	 *   - inspect the first closed contour emitted by boundaryPath();
	 *   - decide whether the +normal side should be tagged as inside/outside;
	 *   - reuse that normalSide sign for all later contours.
	 *
	 * Do not recompute normalSide per contour in the default path, or holes will
	 * be compensated twice. `computeDepth` is kept as a diagnostic/alternate
	 * path for raw contours; it records containment depth but does not drive the
	 * default AASide side sign.
	*/
	static Array<PathContour> collectContours(const Path *self, Array<Vec2> *contourPts, bool computeDepth) {
		Array<PathContour> contours;
		struct Ctx {
			Array<PathContour> *contours;
			int start;
			float normalSide;
			bool computeDepth;
		} ctx = { &contours, 0, 10, computeDepth };

		eachSubpath(self, contourPts, nullptr, [](bool close, Vec2 *pts, int size, int, void *ctx) {
			auto c = static_cast<Ctx*>(ctx);
			if (close) { // ignore open subpaths
				PathContour contour = { c->start, size, 0.0f, -1.0f, pts, pts[0], 0, close };
				if (c->computeDepth) {
					contour.area = contourArea2(pts, size);
					// The sample is only needed by the slower containment pass;
					// keep it slightly off an edge so point-in-polygon does not
					// test exactly on the contour boundary.
					contour.sample = contourInteriorProbe(pts, size, contour.normalSide);
				}
				if (c->normalSide == 10) {
					// First closed contour calibrates the global aaSide sign.
					// With libtess2's automatic projection normal, CW/CCW can
					// flip relative to our screen-space intuition, so derive it
					// from the emitted boundary instead of hard-coding it.
					contour.area = c->computeDepth ? contour.area: contourArea2(pts, size);
					c->normalSide = contour.area < 0.0f ? 1.0f: -1.0f;
				}
				// Reuse the first contour's sign mapping. Later contours keep
				// their own traversal direction through their vertex normals.
				contour.normalSide = c->normalSide;
				c->contours->push(contour);
			}
			c->start += size;
		}, nullptr, true, &ctx);

		if (computeDepth) {
			for (auto &contour: contours) {
				if (!contour.close)
					continue;
				int depth = 0;
				for (auto &parent: contours) {
					if (&contour != &parent && parent.close) {
						if (pointInContour(contour.sample, parent.pts, parent.ptsNum))
							depth++;
					}
				}
				// Kept for inspection and possible non-boundaryPath callers.
				// The normal AASide path relies on tess boundary winding plus
				// the one-time normalSide calibration above.
				contour.depth = depth;
			}
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
		Path tmp;
		//auto self = normalized(&tmp, epsilon, false);
		// boundaryPath() asks libtess2 for TESS_BOUNDARY_CONTOURS using the
		// positive winding rule. Since tess uses `normal=nullptr`, it chooses
		// the projection normal itself; that can make the emitted CW/CCW meaning
		// differ from a hard-coded screen-space convention. collectContours()
		// therefore calibrates the aaSide sign once from the first contour.
		// Later contours use the same sign mapping while their own traversal
		// direction naturally flips normals for holes.
		auto self = boundaryPath(&tmp, epsilon);
		Path body;
		VertexData out;
		Array<Vec3> aaSide;
		Array<Vec2> ptsOut;
		auto contours = collectContours(self, &ptsOut, false);

		if (contours.length() == 0)
			return out; // no contour, return empty output

		// Tune this to adjust the generated band placement.
		// 0 means all expansion goes outward; 1 means a centered band.
		const float innerRatio = 0.5f;
		const float offset = width - Float32::clamp(innerRatio, 0, 1) * width;

		struct Ctx {
			Array<Vec3>        *out;
			Path               *body;
			Vec3               *ptr; // current write position in out
			float              aRadius,bRadius; // stroke radius
			float              normalSide; // sign assigned to the +normal side
			Vec3               prev_a, prev_b;
		} ctx = {
			&aaSide, &body, 0, width + offset, width - offset, contours.front().normalSide
		};

		// `a` is the +normal side and `b` is the -normal side. After the
		// first-contour calibration, a negative normalSide means +normal should
		// be tagged as inside, so swap the inner/outer radii once.
		if (ctx.normalSide < 0.0f) {
			std::swap(ctx.aRadius, ctx.bRadius);
		}

		eachContour(contours, [](Vec2 *prev, Vec2 *from, Vec2 *next, int idx, void *ctx) { // add
			Qk_ASSERT(prev && next, "eachContour should only call add() with valid neighbors");
			auto normals = from->normalline(prev, next); // normal line
			Qk_ASSERT(!normals.is_zero(), "degenerate points should have been removed by strokePointEquals");
			auto c = static_cast<Ctx*>(ctx);
			{
				// The averaged vertex normal must be lengthened by 1/sin(theta)
				// so both offset edges stay `width` away from their source
				// segments at the join.
				auto theta = normals.angleTo(*from - *next); // angle between normals
				auto sin = fabsf(sinf(theta));
				normals *= sin < 1e-2f ? 1.0f: 1.0f / sin;
			}
			// Store the calibrated sign on the +normal vertex and the opposite
			// sign on the -normal vertex. The shader places coverage around the
			// implicit zero crossing between these two vertices.
			Vec3 a(*from + normals * c->aRadius, c->normalSide);
			Vec3 b(*from - normals * c->bRadius, -c->normalSide);
			auto inner = a.z() < 0.0f ? a.xy(): b.xy();
			if (idx) {
				*(c->ptr++) = c->prev_b;
				*(c->ptr++) = c->prev_a;
				*(c->ptr++) = a;
				*(c->ptr++) = a;
				*(c->ptr++) = b;
				*(c->ptr++) = c->prev_b;
				c->body->lineTo(inner);
			} else {
				c->body->moveTo(inner);
			}
			c->prev_a = a;
			c->prev_b = b;
			return true;
		},
		[](bool close, Vec2 *pts, int size, int subpath, void *ctx) { // before
			Qk_ASSERT(close, "eachContour should only call before() with close=true");
			auto _ = static_cast<Ctx*>(ctx);
			auto len = _->out->length();
			// One pair of offset vertices per source point, plus one closing
			// pair emitted in after().
			_->out->extend(len + size * 6); // alloc memory space
			_->ptr = _->out->val() + len;
		},
		[](bool close, int size, int add, int subpath, void *ctx) { // after
			Qk_ASSERT(close, "eachContour should only call after() with close=true");
			Qk_ASSERT_EQ(size, add, "all points should have been added for closed contours");
			auto _ = static_cast<Ctx*>(ctx);
			// Close the final segment by connecting the last generated pair
			// back to the first generated pair in the current subpath.
			auto b = _->ptr - (add * 6 - 6), a = b + 1;
			*(_->ptr++) = _->prev_b;
			*(_->ptr++) = _->prev_a;
			*(_->ptr++) = *a;
			*(_->ptr++) = *a;
			*(_->ptr++) = *b;
			*(_->ptr++) = _->prev_b;
			_->body->close(); // close the body path
		}, &ctx);

		out = body.getTriangles(epsilon, -1.0f);
		out.vertex.write(aaSide.val(), aaSide.length()); // merge body and aaSide vertices
		out.vCount += aaSide.length();
		// debug;
		// out.vCount = aaSide.length();
		// out.vertex = std::move(aaSide);
		Qk_ReturnLocal(out);
	}

#if Qk_USE_FT_STROKE
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
		Qk_ASSERT(err==0, "Qk_FT_Stroker_New failed, err: %d", err);
		Qk_FT_Stroker_Set(stroker, FT_1616(width * 0.5), ft_cap, ft_join, FT_1616(miterLimit));

		Path tmp;
		// auto self = normalized(&tmp, 1, false);
		auto self = boundaryPath(&tmp, 1);

		auto outline = qk_ft_outline_from(self);
		err = Qk_FT_Stroker_ParseOutline(stroker, outline);
		Qk_ASSERT(err==0, "Qk_FT_Stroker_ParseOutline failed, err: %d", err);

		Path out;
		qk_ft_stroke_border_export(stroker->borders+0, &out); // left border
		qk_ft_stroke_border_export(stroker->borders+1, &out); // right border
		qk_ft_outline_destroy(outline);
		Qk_FT_Stroker_Done(stroker);

		Qk_ReturnLocal(out);
	}
#else
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

		for (int i = right.verbsLen() - 1; i >= 0; i--) {
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
		// auto self = normalized(&tmp, 1, false);
		auto self = boundaryPath(&tmp, 1);

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
		Array<Vec2> storePts;
		eachSubpath(self, &storePts, [](Vec2 *prev, Vec2 *_from, Vec2 *next, int idx, void *ctx) {
			#define Qk_addTo(l,r) left.lineTo(l),right.lineTo(r)
			auto _ = (Ctx*)ctx;
			auto &left = *_->left;
			auto &right = _->right;
			auto width = _->width;
			auto from = *_from;

			if (prev == NULL || next == NULL) { // prev == null or next == null
				auto normals = from.normalline(prev, next) * width; // normal line
				switch (_->cap) {
					case Path::Cap::kButt_Cap: // no stroke extension
						Qk_addTo(from + normals, from - normals);
						break;
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
						break;
					}
					default: {// adds square
						// Square caps extend half a stroke width beyond the
						// endpoint along the tangent direction.
						from += prev? normals.rotate270z(): normals.rotate90z();
						Qk_addTo(from + normals, from - normals);
						break;
					}
				}
				return true;
			}

			// Average the two adjacent edge normals to get the join bisector.
			Vec2 toFrom = *next - from;
			Vec2 fromPrev = from - *prev;
			Vec2 toNext90 = toFrom.rotate90z().normalized();
			Vec2 fromPrev90 = fromPrev.rotate90z().normalized();
			Vec2 normals = (toNext90 + fromPrev90).normalized(); // normal line

			if (normals.is_zero()) {
				// Repeated points will produce zero vectors.
				// maybe the from-to-prev is not 0, so we can use fromPrev90 as fallback
				normals = fromPrev90 * width;
				Qk_addTo(from + normals, from - normals);
				left.lineTo(from - normals);
				right.lineTo(from + normals);
				return true;
			}

			float angle    = normals.angle();
			float angleLen = angle - Vec2(-fromPrev[0],-fromPrev[1]).angle();

			if (angleLen < 0)
				angleLen += Qk_PI_2;
			float sinLen = fabsf(sinf(angleLen));
			float len = sinLen < 1e-2f ? _->miterLimit: width / sinLen;

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
							break;
						} // else goto kMiter_Join:
					} else { // inside
						auto aLen = Qk_PI_2_1 - angleLen;
						if (aLen > 0.075f) { // > 0.075f radian
							normals *= len;
							left .lineTo(from + normals);
							right.arc(from, width, Qk_PI-angle-aLen, aLen*2, false);
							break;
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
					break;
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
			return true;
			#undef Qk_addTo
		}, NULL, [](bool close, int size, int add, int subpath, void *ctx) {
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
