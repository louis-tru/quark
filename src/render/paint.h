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

#include "./math.h"
#include "./source.h"
#include "./blend.h"

namespace qk {

	struct GradientPaint {
		enum Type {
			kLinear_Type,  //!< linear gradient type
			kRadial_Type,  //!< radial gradient type
		};
		Type type; // gradient color type
		Vec2 origin,endOrRadius;
		uint32_t       count;
		const Color4f *colors;
		const float   *positions;
	};

	struct ImagePaint {
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

		union {
			uint32_t bitfields = (
				// 0 | // src index default zero
				(kClamp_TileMode << 8) | // 2 bits
				(kClamp_TileMode << 10) | // 2 bits
				(kNone_MipmapMode << 12) | // 2 bits
				(kNearest_FilterMode << 14) | // 1 bits
				0
			);
			struct {
				uint8_t       srcIndex: 8; // default zero, image source pixel index offset
				TileMode      tileModeX: 2; // default kClamp_TileMode
				TileMode      tileModeY: 2; // default kClamp_TileMode
				MipmapMode    mipmapMode: 2;// default kNone_MipmapMode, image source mipmap mode
				FilterMode    filterMode: 1;// default kNearest_FilterMode, image source filter mode
				unsigned      padding: 17;
			};
		}; // size 32bit

		void setImage(ImageSource *image, const Rect &dest, const Rect &src);
		void setImage(ImageSource *image, const Rect &dest) {
			setImage(image, dest, { Vec2(0,0), Vec2(image->width(), image->height()) });
		}
		ImageSource      *source; // image source, weak ref
		Region            coord; // bitmap uv coord
	};

	struct PaintFilter {
		enum Type {
			kBlur_Type, //!< blur type
			kBackdropBlur_Type, //!< backdrop blur type
		};
		Type  type; //!< paint filter type
		float value; //!< blur value radius
	};

	struct Paint {

		enum Type {
			kColor_Type,          //!< set to color paint type
			kGradient_Type,       //!< set to gradient paint type
			kBitmap_Type,         //!< set to bitmap image paint type
			kBitmapMask_Type,     //!< set to bitmap image mask paint type
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

		union {
			uint32_t        bitfields = (
				(kColor_Type << 0) | // 2 bits
				(kFill_Style << 2) | // 2 bits
				(kButt_Cap << 4) | // 2 bits
				(kMiter_Join << 6) | // 2 bits
				(kSrcOver_BlendMode << 8) | // 6 bits
				(1 << 14) | // 2 bits, default antiAlias = true
				0
			);
			struct {
				Type          type: 2;// default kColor_Type;
				Style         style : 2;// default kFill_Style;
				Cap           cap: 2;// default kButt_Cap;
				Join          join: 2;// default kMiter_Join;
				BlendMode     blendMode: 6; // default kSrcOver_BlendMode
				bool          antiAlias: 2;// default true;
				bool          padding: 16;
			};
		}; // size 32bit

		float             width; // stroke width
		Color4f           color; // color
		ImagePaint        *image; // image source, weak ref
		GradientPaint     *gradient; // gradient color, weak ref
		PaintFilter       *filter = nullptr; // filter
	};

}

#endif
