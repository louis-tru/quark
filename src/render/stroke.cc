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
		const Path *self = _IsNormalized ? this: normalized(&tmp, 1,false);

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

	// concat paths, left += reverse(right)
	static void reverseConcatPath(Path &left, const Path &right) {
		auto verbs = right.verbs();
		auto pts = right.pts() + right.ptsLen() - 1;

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

	typedef void AddPoint(const Vec2 *prev, Vec2 from, const Vec2 *next, int idx, void *ctx);
	typedef void BeforeAdding(bool close, int size, int subpath, void *ctx);
	typedef void AfterDone(bool close, int size, int subpath, void *ctx);

	static void strokeExec(
		const Path *self, AddPoint add,
		BeforeAdding before, AfterDone after, bool closeAll, void *ctx
	) {
		int subpath = 0;
		auto addSubpath = [&](const Vec2 *pts, int size, bool close) {
			if (size > 1) { // size > 1
				if (close) { // close path
					if (*pts == pts[size-1]) { // start == end, exclude duplicates
						size--;
					}
					close = size > 2; // Must have at least 3 vertices
				}
				if (before) {
					before(close, size, subpath, ctx);
				}
				add(close ? pts+size-1: NULL, *pts, pts+1, 0, ctx); pts++;

				for (int i = 1, l = size-1; i < l; i++, pts++) {
					add(pts-1, *pts, pts+1, i, ctx);
				}
				add(pts-1, *pts, close? pts-size+1: NULL, size-1, ctx);

				if (after) {
					after(close, size, subpath, ctx);
				}
				subpath++;
			}
		};

		Array<Vec2> pts(self->ptsLen());
		auto pts0 = self->pts();
		auto pts1 = pts.val();
		int  size = 0;
		auto verbs = self->verbs();

		for (int i = 0, l = self->verbsLen(); i < l; i++) {
			switch(verbs[i]) {
				case Path::kLine_Verb:
					if (size != 0) {
						if (pts1[size-1] != *pts0++) // exclude duplicates
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
	 * using offset vertex normals mode
	 * TODO: When the included angle is extremely small, the normal will be shifted too much, 
	 *       which will cause the image to appear glitchy
	 * @method getSDFStrokeTriangleStrip() returns sdf stroke triangle vertices
	 * @return {Array<Vec3>} points { x, y, sdf value renge 0.5 to -0.5 }[]
	*/
	VertexData Path::getAAFuzzStrokeTriangle(float width, float epsilon) const {
		Path tmp;
		auto self = _IsNormalized ? this: normalized(&tmp, epsilon, false);
		VertexData out{0};
		struct Ctx {
			Array<Vec3> *out;
			Vec3        *ptr;
			float       width;
			Vec3        prev_a, prev_b;
		} ctx = { &out.vertex,0,width,0 };

		strokeExec(self, [](const Vec2 *prev, Vec2 from, const Vec2 *next, int idx, void *ctx) { // add
			auto normals = from.normalline(prev, next); // normal line
			auto _ = (Ctx*)ctx;

			if (prev == NULL || next == NULL) { // prev == null or next == null
				normals *= _->width;
			} else if (normals.is_zero()) {
				// Returns zero when the previous is on the same side and on the same line as the next
				auto fromPrev = from - *prev;
				normals = fromPrev.rotate90z().normalized() * _->width;
			} else {
				auto angleLen = normals.angleTo(*prev - from);
				auto len = _->width / sinf(angleLen);
				normals *= len;
			}
			Vec3 a(from + normals, -1), b(from - normals, 1);
			if (idx) {
				*(_->ptr++) = _->prev_b;
				*(_->ptr++) = _->prev_a;
				*(_->ptr++) = a;
				*(_->ptr++) = a;
				*(_->ptr++) = b;
				*(_->ptr++) = _->prev_b;
			}
			_->prev_a = a;
			_->prev_b = b;
		},
		[](bool close, int size, int subpath, void *ctx) { // before
			auto _ = static_cast<Ctx*>(ctx);
			auto len = _->out->length();
			size = (close ? size: size - 1) * 6;
			_->out->extend(len + size); // alloc memory space
			_->ptr = _->out->val() + len;
		},
		[](bool close, int size, int subpath, void *ctx) { // after
			if (close) {
				auto _ = static_cast<Ctx*>(ctx);
				auto b = _->ptr - (size * 6 - 6), a = b + 1;
				*(_->ptr++) = _->prev_b;
				*(_->ptr++) = _->prev_a;
				*(_->ptr++) = *a;
				*(_->ptr++) = *a;
				*(_->ptr++) = *b;
				*(_->ptr++) = _->prev_b;
			}
		}, false, &ctx);

		out.vCount = out.vertex.length();

		Qk_ReturnLocal(out);
	}

#if !Qk_USE_FT_STROKE

	// modification to stroke path
	Path Path::strokePath(float width, Cap cap, Join join, float miterLimit) const {
		if (miterLimit == 0)
			miterLimit = 1024.0;

		miterLimit = Float32::min(miterLimit, 1024);
		width *= 0.5;

		Path tmp,out;
		auto self = _IsNormalized ? this: normalized(&tmp, 1, false);

		struct Ctx {
			float width,miterLimit;
			Cap cap; Join join;
			Path *left,right;
		} ctx = { width, miterLimit, cap, join, &out };

		/*
			1.An unclosed path produces a closed path
			2.closed path produces two closed paths
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
						from += prev? normals.rotate270z(): normals.rotate90z();
						Qk_addTo(from + normals, from - normals);
						return;
					}
				}
			}

			// Calculate normals
			Vec2 toFrom = *next - from;
			Vec2 fromPrev = from - *prev;
			Vec2 toNext90 = toFrom.rotate90z().normalized();
			Vec2 fromPrev90 = fromPrev.rotate90z().normalized();
			Vec2 normals = (toNext90 + fromPrev90).normalized(); // normal line
			// Calculate normals end

			// Returns zero when the previous is on the same side and on the same line as the next
			if (normals.is_zero()) {
				normals = fromPrev90 * width;
				Qk_addTo(from + normals, from - normals);
				left.lineTo(from - normals);
				right.lineTo(from + normals);
				return;
			}

			float angle    = normals.angle();
			float angleLen = angle - Vec2(-fromPrev[0],-fromPrev[1]).angle();
			float len = width / sinf(angleLen);

			if (angleLen < 0)
				angleLen += Qk_PI_2;

			switch (_->join) {
				case Path::Join::kRound_Join: {// adds circle
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
