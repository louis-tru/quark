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

namespace qk {

	struct Paint {

		enum Type {
			kColor_Type,          //!< set to color paint type
			kGradient_Type,       //!< set to gradient paint type
			kImage_Type,          //!< set to image paint type
		};

		enum Style {
			kFill_Style,          //!< set to fill geometry
			kStroke_Style,        //!< set to stroke geometry
			kStrokeAndFill_Style, //!< sets to stroke and fill geometry
		};

		enum Cap {              //!< point style
			kButt_Cap,            //!< no stroke extension
			kRound_Cap,           //!< adds circle
			kSquare_Cap,          //!< adds square
		};

		enum Join {             //!< stroke style
			kMiter_Join,          //!< extends to miter limit
			kRound_Join,          //!< adds circle
			kBevel_Join,          //!< connects outside edges
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

		enum FilterMode {
			kNearest_FilterMode,   //!< single sample point (nearest neighbor)
			kLinear_FilterMode,    //!< interporate between 2x2 sample points (bilinear interpolation)
		};

		enum MipmapMode {
			kNone_MipmapMode,      //!< ignore mipmap levels, sample from the "base"
			kNearest_MipmapMode,   //!< sample from the nearest level
			kLinear_MipmapMode,    //!< interpolate between the two nearest levels
		};

		inline Vec2 offset() const {
			return Vec2(color.b(), color.g());
		}

		inline Vec2 scale() const {
			return Vec2(color.r(), color.a());
		}

		inline void setOffset(Vec2 offset) {
			color.b(offset.x());
			color.g(offset.y());
		}

		inline void setScale(Vec2 scale) {
			color.r(scale.x());
			color.a(scale.y());
		}

		inline const GradientPaint* gradient() const {
			return reinterpret_cast<const GradientPaint*>(image);
		}

		void setImage(ImageSource* image, const Rect& dest, const Rect& src);
		void setImage(ImageSource* image, const Rect& dest); // src = {Vec2(0,0),Vec2(w,h)}
		void setGradient(GradientPaint* gradient);

		union {
			uint32_t        bitfields = (
				(kColor_Type << 0) |
				(kFill_Style << 2) |
				(kSrcOver_BlendMode << 4) |
				(kButt_Cap << 12) |
				(kMiter_Join << 14) |
				(kClamp_TileMode << 16) |
				(kNearest_FilterMode << 18) |
				(kNone_MipmapMode << 20) |
				(1 << 22) | // antiAlias = true
				0
			);
			struct {
				Type          type: 2;// default kColor_Type;
				Style         style : 2;// default kFill_Style;
				BlendMode     blendMode: 8; // default kSrcOver_BlendMode
				Cap           cap: 2;// default kButt_Cap;
				Join          join : 2;// default kMiter_Join;
				TileMode      tileMode: 2; // default kClamp_TileMode
				FilterMode    filterMode: 2;// default kNearest_FilterMode, image source filter mode
				MipmapMode    mipmapMode: 2;// default kNone_MipmapMode, image source mipmap mode
				bool          antiAlias : 1;// default true;
				unsigned      padding : 9;  // 32 = 2+2+8+2+2+2+2+2+1+9
			};
		}; // size 32bit

		float             opacity; // image opacity
		Color4f           color; // color or image source uv coord
		ImageSource      *image; // storage image source or gradient paint
		float             width; // stroke width
	};

}

#endif