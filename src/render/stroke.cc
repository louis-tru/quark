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

namespace qk {

	static void Path_reverse_concat(Path &left, const Path &right) {
		auto verbs = right.verbs();
		auto pts = right.pts() + right.ptsLen() - 1;

		for (int i = right.verbsLen() - 1; i >= 0; i--) {
			if (verbs[i] == Path::kVerb_Cubic) {
				left.addTo(*pts); pts--;
				do {
					left.cubicTo(pts[0], pts[-1], pts[-2]); pts-=3;
					i--;
				} while(verbs[i] == Path::kVerb_Cubic);
			} else if (verbs[i] == Path::kVerb_Close) {
				// ignore
			} else {
				Qk_ASSERT(verbs[i] == Path::kVerb_Line || verbs[i] == Path::kVerb_Move);
				left.addTo(*pts--);
			}
		}
	}

	Array<Vec3> Path::getAntiAliasStrokeTriangles(float epsilon) {
		// TODO ...
	}

#if Qk_USE_FT_STROKE

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

#else

	Path Path::strokePath(float width, Cap cap, Join join, float miterLimit) const {
		if (miterLimit == 0)
			miterLimit = 1024.0;

		miterLimit = Float::min(miterLimit, 1024);
		width *= 0.5;

		Path tmp,left,right;
		auto self = _IsNormalized ? this: normalized(&tmp, 1, false);

		auto pts_ = (const Vec2*)self->_pts.val();
		int  size = 0;
		/*
			1.An unclosed path produces a closed path
			2.closed path produces two closed paths
		*/

		auto add = [&](const Vec2 *prev, Vec2 from, const Vec2 *next) {
			#define Qk_addTo(l,r) \
				right.ptsLen() ? (left.lineTo(l),right.lineTo(r)): (left.moveTo(l), right.moveTo(r))

			if (!prev || !next) {
				auto nline = from.normalline(prev, next); // normal line
				nline *= width;

				switch (cap) {
					case Path::Cap::kButt_Cap: // no stroke extension
						Qk_addTo(from + nline, from - nline);
						return;
					case Path::Cap::kRound_Cap: { //adds circle
						float angle = nline.angle();
						if (prev) {
							left.arcTo({from-width,width*2}, -angle, -Qk_PI, false);
							right.addTo(from - nline);
						} else {
							left.addTo(from + nline);
							right.arcTo({from-width,width*2}, -angle, Qk_PI, false);
						}
						return;
					}
					default: {// adds square
						from += prev? nline.rotate270z(): nline.rotate90z();
						Qk_addTo(from + nline, from - nline);
						return;
					}
				}
			}

			Vec2 toFrom = *next - from;
			Vec2 fromPrev = from - *prev;
			Vec2 toNext90   = toFrom.rotate90z().normalized();
			Vec2 fromPrev90 = fromPrev.rotate90z().normalized();
			Vec2 nline = (toNext90 + fromPrev90).normalized(); // normal line

			if (nline.is_zero()) {
				nline = fromPrev90 * width;
				Qk_addTo(from + nline, from - nline);
				left.lineTo(from - nline);
				right.lineTo(from + nline);
				return;
			}

			float angle    = nline.angle();
			float angleLen = angle - Vec2(-fromPrev[0],-fromPrev[1]).angle();
			float len = width / sinf(angleLen);

			if (angleLen < 0)
				angleLen += Qk_PI_2;

			switch (join) {
				case Path::Join::kMiter_Join: { // extends to miter limit
					if (len > miterLimit) {
						float lenL = len - miterLimit;
						float y = tanf(angleLen) * lenL;
						auto a = nline.rotate90z() * y;
						auto nLineL = nline * miterLimit;
						nline *= len;

						if (angleLen > Qk_PI_2_1) {
							Qk_addTo(from + nLineL - a, from - nline);
							left.lineTo(from + nLineL + a);
						} else {
							Qk_addTo(from + nline, from - nLineL + a);
							right.lineTo(from - nLineL - a);
						}
					} else {
						nline *= len;
						Qk_addTo(from + nline, from - nline);
					}
					return;
				}
				case Path::Join::kRound_Join: {// adds circle
					auto a = fromPrev90 * width;
					auto b = toNext90 * width;
					nline *= len;
					if (angleLen > Qk_PI_2_1) {
						angleLen = Qk_PI_2_1 - Qk_PI + angleLen;
						left.arcTo({from-width,Vec2(width*2)}, Qk_PI_2-angle+angleLen, -angleLen*2, false);
						right.addTo(from - nline);
					} else {
						angleLen = Qk_PI_2_1 - angleLen;
						left.addTo(from + nline);
						right.arcTo({from-width,Vec2(width*2)}, Qk_PI-angle-angleLen, angleLen*2, false);
					}
					return;
				}
				default: {// connects outside edges
					auto a = fromPrev90 * width;
					auto b = toNext90 * width;
					nline *= len;
					if (angleLen > Qk_PI_2_1) {
						Qk_addTo(from + a, from - nline);
						left.lineTo(from + b);
					} else {
						Qk_addTo(from + nline, from - a);
						right.lineTo(from - b);
					}
				}
			}
			#undef Qk_addTo
		};

		auto subpath = [&](const Vec2 *pts, int size, bool close) {
			if (size > 1) { // size > 1
				close = close && size > 2;

				add(close ? pts+size-1: NULL, *pts, pts+1); pts++;

				for (int i = 1; i < size-1; i++, pts++) {
					add(pts-1, *pts, pts+1);
				}
				add(pts-1, *pts, close? pts-size+1: NULL);

				if (close)
					left.close();

				Path_reverse_concat(left, right);

				left.close();
			}
			right = Path(); // clear right path
		};

		for (auto verb: self->_verbs) {
			switch(verb) {
				case Path::kVerb_Move:
					subpath(pts_, size, false);
					pts_ += size;
					size = 1;
					break;
				case Path::kVerb_Line:
					size++;
					break;
				case Path::kVerb_Close: // close
					subpath(pts_, size, true);
					pts_ += size;
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
