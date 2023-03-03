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

#include "../math.h"
#include "./source.h"
#include "./blend.h"

namespace qk {

	struct GradientColor {
		Array<Color4f> colors;
		Array<float>   positions;
	};

	struct Paint {

		enum Type {
			kColor_Type,          //!< set to color paint type
			kGradient_Type,       //!< set to gradient paint type
			kBitmap_Type,         //!< set to bitmap image paint type
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
			//!< Repeat the shader's image horizontally.
			kRepeat_X_TileMode,
			//!< Repeat the shader's image vertically.
			kRepeat_Y_TileMode,
			//!< Repeat the shader's image horizontally and vertically, alternating mirror images so that adjacent images always seam.
			kMirror_TileMode,
			//!< Repeat the shader's image horizontally, alternating mirror images so that adjacent images always seam.
			kMirror_X_TileMode,
			//!< Repeat the shader's image vertically, alternating mirror images so that adjacent images always seam.
			kMirror_Y_TileMode,
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

		enum GradientType {
			kLinear_GradientType,  //!< linear gradient type
			kRadial_GradientType,  //!< radial gradient type
		};

		inline Vec2 bitmapOffset() const { return Vec2(color[0], color[1]); }
		inline Vec2 bitmapScale() const { return Vec2(color[2], color[3]); }
		inline Vec2 linearStart() const { return Vec2(color[0], color[1]); }
		inline Vec2 linearEnd() const { return Vec2(color[2], color[3]); }
		inline Vec2 radialCenter() const { return Vec2(color[0], color[1]); }
		inline Vec2 radialRadius() const { return Vec2(color[2], color[3]); }

		inline const GradientColor* gradientColor() const {
			return reinterpret_cast<const GradientColor*>(image);
		}
		inline const BitmapPixel* bitmapPixel() const {
			return reinterpret_cast<cPixel*>(image);
		}

		void setBitmapPixel(cPixel *image, const Rect& dest, const Rect& src);
		void setBitmapPixel(cPixel *image, const Rect& dest); // src = {Vec2(0,0),Vec2(w,h)}
		void setLinearGradient(const GradientColor *colors, Vec2 start, Vec2 end);
		void setRadialGradient(const GradientColor *colors, Vec2 center, Vec2 radius);

		union {
			uint32_t        bitfields = (
				(kColor_Type << 0) | // 2 bits
				(kFill_Style << 2) | // 2 bits
				(kSrcOver_BlendMode << 4) | // 6 bits
				(kButt_Cap << 10) | // 2 bits
				(kMiter_Join << 12) | // 2 bits
				(kClamp_TileMode << 14) | // 3 bits
				(kNearest_FilterMode << 17) | // 1 bits
				(kNone_MipmapMode << 18) | // 2 bits
				(kLinear_GradientType << 20) | // 1 bits
				(1 << 21) | // 1 bits, default antiAlias = true
				0
			);
			struct {
				Type          type: 2;// default kColor_Type;
				Style         style : 2;// default kFill_Style;
				BlendMode     blendMode: 6; // default kSrcOver_BlendMode
				Cap           cap: 2;// default kButt_Cap;
				Join          join : 2;// default kMiter_Join;
				TileMode      tileMode: 3; // default kClamp_TileMode
				FilterMode    filterMode: 1;// default kNearest_FilterMode, image source filter mode
				MipmapMode    mipmapMode: 2;// default kNone_MipmapMode, image source mipmap mode
				GradientType  gradientType: 1;// default kLinear_GradientType, gradient color type
				bool          antiAlias : 1;// default true;
				unsigned      padding : 10;
			};
		}; // size 32bit

		float             opacity; // bitmap opacity
		// color or bitmap uv coord or start/end or center/radius for gradient color range
		Color4f           color;
		const void       *image; // bitmap or gradient color, weak ref
		float             width; // stroke width
	};

}

#endif