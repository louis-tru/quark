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
#include "./blend.h"

namespace qk {
	class Canvas;
	class ImageSource;

	struct PaintFilter {
		enum Type {
			kGra_Type, //!< gra filter
			kBlur_Type, //!< blur filter
			kMotionBlur_Type, //!< motion blur
		};
		Type  type; //!< paint filter type
		float val0; //!< blur value radius
		float val1; //!< motion blur direction
	};

	struct PaintGradient {
		enum Type {
			kLinear_Type,  //!< linear gradient type
			kRadial_Type,  //!< radial gradient type
		};
		Type           type; // gradient color type
		Vec2           origin,endOrRadius;
		uint32_t       count;
		const Color4f  *colors;
		const float    *positions;
	};

	struct PaintImage {
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
			kNone_MipmapMode,      //!< Nearest point sampling + recently mipmap layer (fast speed, but strong aliasing).
			kLinearNearest_MipmapMode, //!< Linear sampling + nearest mipmap layer (with smoother transitions).
			kNearestLinear_MipmapMode, //!< Take the nearest point samples from two adjacent mipmap layers and mix them linearly.
			kLinear_MipmapMode, //!< Linear interpolation is performed in two adjacent mipmap layers,
				// followed by linear mixing between layers (commonly known as trilinear filtering,
				// which performs the best but consumes the most performance).
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
				bool          _flushCanvas: 1;// default false, flush canvas and draw canvas
				unsigned short _padding: 16;
			};
		}; // size 32bit

		/**
		 * @method setCanvas flush and draw canvas
		*/
		void setCanvas(Canvas *canvas, const Rect &dest, const Rect &src);
		void setCanvas(Canvas *canvas, const Rect &dest);
		/**
		 * @method setImage set image draw region
		*/
		void setImage(ImageSource *image, const Rect &dest, const Rect &src);
		void setImage(ImageSource *image, const Rect &dest);
		union {
			ImageSource     *image; // image source, weak ref
			Canvas          *canvas; // flush canvas, weak ref
		};
		Region            coord; // bitmap uv coord
	};

	struct PaintStyle {
		Color4f        color; // color
		PaintImage*    image = nullptr; // image source, weak ref
		PaintGradient* gradient = nullptr; // gradient color, weak ref
	};

	struct Paint {
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

		PaintStyle    fill; // fill paint style
		PaintStyle    stroke; // stroke paint style
		BlendMode     blendMode = kSrcOver_BlendMode; // blend mode
		Style         style = kFill_Style; // paint style
		Cap           cap = kButt_Cap; // stroke cap
		Join          join = kMiter_Join; // stroke join
		bool          antiAlias = true; // is anti aliasing
		float         strokeWidth = 1.0f; // stroke width
		PaintFilter*  filter = nullptr; // filter, weak ref
		PaintImage*   mask = nullptr; // mask image, weak ref
	};

	/**
	 * @struct render backend pixel texture stat
	*/
	struct TexStat {
		uint32_t id;
	};

}

#endif
