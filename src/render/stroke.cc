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
#include "./ft/ft_path.h"
#include <math.h>

#define Qk_USE_FT_STROKE 0

#if Qk_USE_FT_STROKE

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

	Array<Vec3> Path::getAntiAliasStrokeTriangles(float epsilon) {
		// TODO ...
	}

#if !Qk_USE_FT_STROKE

	Path Path::strokePath(float width, Cap cap, Join join, float miterLimit) const {
		if (miterLimit == 0)
			miterLimit = 8;//1024.0;

		width *= 0.5;

		Path tmp, left, right;
		auto self = _IsNormalized ? this: normalized(&tmp, 1, false);
		auto pts_ = (const Vec2*)self->_pts.val();
		int  size = 0;

		/*
			1.An unclosed path produces a closed path
			2.closed path produces two closed paths
		*/

		#define Qk_StartTo(l,r) \
			right.ptsLen() ? (left.lineTo(l),right.lineTo(r)): (left.moveTo(l), right.moveTo(r))

		auto add = [&](const Vec2 *prev, Vec2 from, const Vec2 *next) {

			if (!prev || !next) {
				auto nline = from.normalline(prev, next); // normal line
				nline *= width;

				switch (cap) {
					case Cap::kButt_Cap: // no stroke extension
						break;
					case Cap::kRound_Cap: //adds circle
						break;
					default: // adds square
						break;
				}

				Qk_StartTo(from + nline, from - nline);
				return;
			}

			Vec2 toFrom = *next - from;
			Vec2 fromPrev = from - *prev;
			Vec2 toNext90   = toFrom.rotate90z().normalized();
			Vec2 fromPrev90 = fromPrev.rotate90z().normalized();
			Vec2 nline = (toNext90 + fromPrev90).normalized(); // normal line

			if (nline.is_zero()) {
				nline = fromPrev90 * width;
				Qk_StartTo(from + nline, from - nline);
				left.lineTo(from - nline);
				right.lineTo(from + nline);
				return;
			}

			float angle = nline.angleTo(Vec2(-fromPrev[0],-fromPrev[1]));
			float len = width / sinf(angle);

			if (angle < 0)
				angle += Qk_PI_2;

			switch (join) {
				case Join::kMiter_Join: { // extends to miter limit

					if (len > miterLimit) {
						float lenL = len - miterLimit;
						float y = tanf(angle) * lenL;
						auto a = nline.rotate90z() * y;
						auto nLineL = nline * miterLimit;
						nline *= len;

						if (angle > Qk_PI_2_1) {
							Qk_StartTo(from + nLineL - a, from - nline);
							left.lineTo(from + nLineL + a);
						} else {
							Qk_StartTo(from + nline, from - nLineL + a);
							right.lineTo(from - nLineL - a);
						}

					} else {
						nline *= len;
						Qk_StartTo(from + nline, from - nline);
					}
					break;
				}
				case Join::kRound_Join: {// adds circle
					auto a = fromPrev90 * width;
					auto b = toNext90 * width;

					nline *= len;

					if (angle > Qk_PI_2_1) {
						Qk_StartTo(from + a, from - nline);
						left.lineTo(from + b);
					} else {
						Qk_StartTo(from + nline, from - a);
						right.lineTo(from - b);
					}

					return;
				}
				default: {// connects outside edges
					auto a = fromPrev90 * width;
					auto b = toNext90 * width;

					nline *= len;

					if (angle > Qk_PI_2_1) {
						Qk_StartTo(from + a, from - nline);
						left.lineTo(from + b);
					} else {
						Qk_StartTo(from + nline, from - a);
						right.lineTo(from - b);
					}
					return;
				}
			}
			#undef Qk_StartTo
		};

		auto subpath = [&](const Vec2 *pts, int size, bool close) {
			if (size > 1) { // size > 1
				close = close && size > 2;

				add(close ? pts+size-1: NULL, *pts, pts+1); pts++;

				for (int i = 1; i < size-1; i++, pts++) {
					add(pts-1, *pts, pts+1);
				}
				add(pts-1, *pts, close? pts-size+1: NULL);

				auto verbs = right.verbs();
				auto pts = right.pts() + right.ptsLen() - 1;

				if (close) {
					left.close();
					left.moveTo(*pts);
				} else {
					left.lineTo(*pts);
				}
				pts--;

				for (int i = right.verbsLen() - 2; i >= 0; i--) {
					if (verbs[i] == kVerb_Cubic) {
						left.cubicTo(pts[0], pts[-1], pts[-2]); pts-=3;
					} else {
						Qk_ASSERT(verbs[i] == kVerb_Line || verbs[i] == kVerb_Move);
						left.lineTo(*pts--);
					}
				}

				left.close();
			}
			pts_ += size;
			right = Path(); // clear right path
		};

		for (auto verb: self->_verbs) {
			switch(verb) {
				case kVerb_Move:
					subpath(pts_, size, false);
					size = 1;
					break;
				case kVerb_Line:
					size++;
					break;
				case kVerb_Close: // close
					subpath(pts_, size, true);
					size = 0;
					break;
				default: Qk_FATAL("Path::strokePath");
			}
		}

		subpath(pts_, size, false);

		Qk_ReturnLocal(left);
	}

#endif

}
