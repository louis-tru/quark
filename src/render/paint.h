// @private head
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

#ifndef __quark_render_paint__
#define __quark_render_paint__

#include "../util/util.h"
#include "./source.h"
#include "./gradient.h"
#include "./blend.h"

namespace quark {

	struct Paint {

		enum Type {
			kColor_Type,
			kGradient_Type,
			kImage_Type,
		};

		enum Style {
			kFill_Style,          //!< set to fill geometry
			kStroke_Style,        //!< set to stroke geometry
			kStrokeAndFill_Style, //!< sets to stroke and fill geometry
		};

		enum Cap {
			kButt_Cap,                  //!< no stroke extension
			kRound_Cap,                 //!< adds circle
			kSquare_Cap,                //!< adds square
		};

		enum Join {
			kMiter_Join,                 //!< extends to miter limit
			kRound_Join,                 //!< adds circle
			kBevel_Join,                 //!< connects outside edges
		};

		enum TileMode {
			//!< Replicate the edge color if the shader draws outside of its original bounds.
			kClamp_TileMode,
			//!< Repeat the shader's image horizontally and vertically.
			kRepeat_TileMode,
			//!< Repeat the shader's image horizontally and vertically, alternating mirror images so that adjacent images always seam.
			kMirror_TileMode,
			//!< Only draw within the original domain, return transparent-black everywhere else.
			kDecal_TileMode,
		};

		Color4f           color;
		Sp<ImageSource>   image = nullptr;
		Sp<GradientPaint> gradient = nullptr;
		float             width = 1; // stroke width

		union {
			struct {
				Type          type: 2;// = kColor_Type;
				Style         style : 2;// = kFill_Style;
				Cap           cap: 2;// = kButt_Cap;
				Join          join : 2;// = kMiter_Join;
				TileMode      tileMode: 2; // = kClamp_TileMode
				BlendMode     blendMode: 8; // = kSrcOver_BlendMode
				bool          antiAlias : 1;// = false;
				bool          dither : 1;// = false;
				unsigned      padding : 12;  // 12 = 32-2-2-2-2-2-8-1-1
			};
			uint32_t        bitfields = (
				(kColor_Type << 0) |
				(kFill_Style << 2) |
				(kButt_Cap << 4) |
				(kMiter_Join << 6) |
				(kClamp_TileMode << 8) |
				(kSrcOver_BlendMode << 10) |
				(1 << 18) | // antiAlias
				(1 << 19) | // dither
				0
			);
		}; // size 32bit
	};

	void test() {
		Paint paint;
		paint.style = Paint::kFill_Style;
		GradientPaint::Linear({Color4f(0,0,0),Color4f(1,1,1)},{0,0.5,1},{0,0},{1,1});
	}
}

#endif