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

	Path Path::stroke(float width, Cap cap, Join join, float miterLimit) const {
		if (miterLimit == 0)
			miterLimit = 1024.0;

		width *= 0.5;

		Vec2 s(width, width);

		Path tmp, out;
		Array<Vec2> right;
		auto self = _IsNormalized ? this: normalized(&tmp, 1,false);
		auto pts_ = (const Vec2*)self->_pts.val();
		int  size = 0;

		/*
			1.未闭合路径产生一条封闭路径
			2.已闭合路径产生两条封闭路径
		*/

		auto add = [&](const Vec2 *prev, const Vec2 *from, const Vec2 *to) {
			auto l = from
				->normalline(prev, to, false)
				.normalized() * s;
			
			if (right.length()) {
				out.lineTo(l);
			} else {
				out.moveTo(l);
			}
			right.push({ -1 * l[0], -1 * l[1] });
		};

		auto subpath = [&](const Vec2 *pts, int size, bool close) {
			if (size > 1) { // size > 1
				bool isClose = close && size > 2;

				if (isClose) {
					add(pts+size-1, pts, pts+1);
				} else { // no close
					add(NULL, pts, pts+1);
				}
				pts++;

				for (int i = 1; i < size-1; i++) {
					add(pts-1, pts, pts+1);
					pts++;
				}

				if (isClose) {
					add(pts-1, pts, pts-size+1);
				} else { // no close
					add(pts-1, pts, NULL);
				}

				if (close) {
					out.close();
				}

				// right.reverse();
			}
			pts_ += size;
			right.clear();
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

		Qk_ReturnLocal(out);
	}

}