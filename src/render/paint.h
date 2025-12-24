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

#ifndef __quark_render_paint__
#define __quark_render_paint__

#include "./math.h"
#include "./blend.h"

namespace qk {
	class Canvas;
	class ImageSource;

	/**
	 * @struct PaintFilter
	 * @brief Image-space filter configuration applied to paint output.
	 *
	 * Filters affect how the painted content is post-processed, such as
	 * applying blur or motion blur effects.
	 */
	struct PaintFilter {
		/**
		 * @enum Type
		 * @brief Defines the filtering algorithm type.
		 */
		enum Type {
			kGray_Type,        //!< Grayscale filter.
			kBlur_Type,        //!< Gaussian blur filter.
			kMotionBlur_Type,  //!< Directional motion blur.
		};

		Type  type;  //!< Filter type.
		float val0;  //!< Main parameter (e.g., blur radius).
		float val1;  //!< Secondary parameter (e.g., motion blur direction, in radians).
	};

	/**
	 * @struct PaintGradient
	 * @brief Describes a gradient shader used for color interpolation.
	 *
	 * Gradients can be linear or radial, defined by a start point and end point
	 * (or radius for radial type), with a color array and normalized stop positions.
	 */
	struct PaintGradient {
		enum Type {
			kLinear_Type,  //!< Linear gradient from `origin` → `endOrRadius`.
			kRadial_Type,  //!< Radial gradient centered at `origin` with radius `endOrRadius.x`.
		};

		Type           type;          //!< Gradient type.
		Vec2           origin;        //!< Gradient origin position.
		Vec2           endOrRadius;   //!< End point (linear) or radius (radial).
		uint32_t       count;         //!< Number of color stops.
		const Color4f* colors;        //!< Pointer to color stop array.
		const float*   positions;     //!< Pointer to normalized position array [0,1].
	};

	/**
	 * @struct PaintImage
	 * @brief Represents an image or sub-canvas as a paint texture source.
	 *
	 * A `PaintImage` can reference either:
	 * - a static `ImageSource` (GPU texture), or
	 * - a `Canvas` whose current rendered content will be sampled as a texture.
	 *
	 * This abstraction allows dynamic composition, such as rendering a sublayer to
	 * an offscreen canvas, and then using that canvas as an image fill on another canvas.
	 */
	struct PaintImage {
		// ---------------------------------------------------------------------
		//  Tile / Filter / Mipmap Modes
		// ---------------------------------------------------------------------
		enum TileMode {
			kClamp_TileMode,   //!< Extend edge color beyond image bounds.
			kRepeat_TileMode,  //!< Repeat image in both directions.
			kMirror_TileMode,  //!< Mirror-repeat image, creating seamless edges.
			kDecal_TileMode,   //!< Clip strictly to image bounds (transparent outside).
		};

		enum FilterMode {
			kNearest_FilterMode, //!< Nearest neighbor sampling (fast, alias-prone).
			kLinear_FilterMode,  //!< Bilinear sampling (smooth).
		};

		enum MipmapMode {
			kNone_MipmapMode,          //!< No mipmaps, fastest, aliased.
			kLinearNearest_MipmapMode, //!< Linear filtering in one layer only.
			kNearestLinear_MipmapMode, //!< Nearest texel, blended between mip levels.
			kLinear_MipmapMode,        //!< Trilinear filtering (best quality).
		};

		union {
			uint32_t bitfields = (
				(kClamp_TileMode << 8) |
				(kClamp_TileMode << 10) |
				(kNone_MipmapMode << 12) |
				(kNearest_FilterMode << 14)
			);
			struct {
				uint8_t     srcIndex: 8;     //!< Source pixel offset (reserved, usually 0).
				TileMode    tileModeX: 2;    //!< Horizontal tile mode.
				TileMode    tileModeY: 2;    //!< Vertical tile mode.
				MipmapMode  mipmapMode: 2;   //!< Mipmap sampling mode.
				FilterMode  filterMode: 2;   //!< Texture sampling mode.
				bool       _isCanvas: 8;    //!< Whether the source is a canvas.
				uint8_t    _padding: 8;   //!< Padding for alignment.
			};
		};

		// ---------------------------------------------------------------------
		//  Binding sources
		// ---------------------------------------------------------------------

		/**
		 * @method setCanvas
		 * @brief Binds another `Canvas` as the texture source for this paint.
		 *
		 * When a `Canvas` is set as the image source, this method will **flush**
		 * that canvas to ensure all pending draw commands are finalized, and its
		 * underlying GPU texture is ready for sampling.
		 *
		 * ⚙️ **Key difference from `Canvas::outputImage()`:**
		 * - `Canvas::outputImage()` **creates or redirects** a canvas’s render target.
		 *   It changes *where* the drawing goes.
		 * - `PaintImage::setCanvas()` **uses** an existing canvas’s result as an input texture.
		 *   It samples *what has already been drawn*.
		 *
		 * In other words:
		 * - `outputImage()` → “**produce** an image target for drawing”
		 * - `setCanvas()` → “**consume** a finished canvas as an image source”
		 *
		 * ---
		 * ### Example
		 * ```cpp
		 * // Render into an offscreen canvas
		 * offscreen->save();
		 * auto tex = offscreen->outputImage(nullptr);
		 * drawScene(offscreen);
		 * offscreen->restore();
		 *
		 * // Use that offscreen canvas as an image source
		 * PaintImage img;
		 * img.setCanvas(offscreen, destRect);
		 * PaintStyle style;
		 * style.image = &img;
		 * rootCanvas->drawRect(destRect, style);
		 * ```
		 *
		 * ---
		 * ### Performance Notes
		 * - Using `setCanvas()` on a separate sub-canvas involves GPU command flush
		 *   and may incur pipeline sync if the sub-canvas is large or frequently updated.
		 * - For temporary offscreen usage within the same canvas, prefer
		 *   `outputImage()` + `restore()` instead, which avoids extra framebuffer setup.
		 */
		void setCanvas(Canvas *canvas, const Rect &dest, const Rect &src);
		void setCanvas(Canvas *canvas, const Rect &dest);

		/**
		 * @method setImage
		 * @brief Sets a static GPU `ImageSource` as the image source.
		 *
		 * This is used for normal texture drawing without involving a sub-canvas.
		 */
		void setImage(ImageSource *image, const Rect &dest, const Rect &src);
		void setImage(ImageSource *image, const Rect &dest);

		union {
			ImageSource *image;  //!< Image texture source (weak reference).
			Canvas      *canvas; //!< Canvas source (weak reference, flushed before sampling).
		};

		Range coord; //!< Normalized UV coordinate range [0,1] for sampling.
	};

	/**
	 * @struct PaintStyle
	 * @brief Defines fill or stroke style used by Paint.
	 *
	 * A PaintStyle may represent a solid color, a gradient, or an image shader.
	 * Only one of these is typically active per paint operation.
	 */
	struct PaintStyle {
		Color4f        color;                 //!< Solid color value.
		PaintImage*    image = nullptr;       //!< Image or sub-canvas source (weak).
		PaintGradient* gradient = nullptr;    //!< Gradient color definition (weak).
	};

	/**
	 * @struct Paint
	 * @brief Primary paint state controlling fill, stroke, color, blending, and filters.
	 *
	 * The `Paint` structure aggregates all attributes needed for a draw operation,
	 * including fill/stroke styles, blending mode, filters, stroke width, etc.
	 */
	struct Paint {
		/**
		 * @enum Style
		 * @brief Defines whether the geometry is filled, stroked, or both.
		 */
		enum Style {
			kFill_Style,          //!< Fill geometry interior.
			kStroke_Style,        //!< Stroke geometry outline.
			kStrokeAndFill_Style, //!< Both stroke and fill.
		};

		/**
		 * @enum Cap
		 * @brief Defines how stroke endpoints are rendered.
		 */
		enum Cap {
			kButt_Cap,   //!< Flat ends (no extension).
			kRound_Cap,  //!< Round caps at endpoints.
			kSquare_Cap, //!< Square caps extending half stroke width.
		};

		/**
		 * @enum Join
		 * @brief Defines how corners between stroke segments are rendered.
		 */
		enum Join {
			kMiter_Join, //!< Sharp corner with miter limit.
			kRound_Join, //!< Rounded corner.
			kBevel_Join, //!< Flat bevel corner.
		};

		PaintStyle    fill;           //!< Fill paint style (solid, image, or gradient).
		PaintStyle    stroke;         //!< Stroke paint style (solid, image, or gradient).
		BlendMode     blendMode = kSrcOverPre_BlendMode; //!< Compositing mode (default: premultiplied alpha src-over).
		Style         style = kFill_Style;               //!< Fill/stroke mode.
		Cap           cap = kButt_Cap;                   //!< Stroke cap style.
		Join          join = kMiter_Join;                //!< Stroke join style.
		bool          antiAlias = true;                  //!< Enable/disable anti-aliasing.
		float         strokeWidth = 0.0f;                //!< Stroke thickness in pixels.
		PaintFilter*  filter = nullptr;                  //!< Optional post-processing filter (weak reference).
		PaintImage*   mask = nullptr;                    //!< Optional mask image (weak reference).
	};

	/**
	 * @struct TexStat
	 * @brief Lightweight GPU texture handle used by RenderBackend.
	 *
	 * Represents a backend-managed texture resource identifier.
	 * May refer to a hardware texture (OpenGL ID, Vulkan image, etc.).
	 */
	struct TexStat {
		uint32_t id; //!< Backend texture identifier.
	};
}

#endif
