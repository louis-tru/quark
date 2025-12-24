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

#ifndef __quark_render_canvas__
#define __quark_render_canvas__

#include "../util/util.h"
#include "./font/families.h"
#include "./pathv_cache.h"
#include "./pixel.h"
#include "./paint.h"

namespace qk {

	/**
	 * @class Canvas
	 * @brief Abstract base class defining all 2D drawing APIs.
	 * 
	 * `Canvas` represents a rendering context for drawing vector paths, images,
	 * text, and color fills. It abstracts GPU/CPU rendering backends while providing
	 * consistent state management (matrix, clipping, paint).
	 * 
	 * This is an abstract type — concrete implementations (e.g. GLCanvas, VkCanvas)
	 * must override all pure virtual methods.
	 */
	class Qk_EXPORT Canvas: public Reference {
		Qk_DISABLE_COPY(Canvas);
	public:
		/**
		 * @enum ClipOp
		 * @brief Defines how new clipping regions combine with existing ones.
		 */
		enum ClipOp {
			kDifference_ClipOp,  ///< Subtract new region from current clip.
			kIntersect_ClipOp,   ///< Intersect new region with current clip.
		};

		/**
		 * @struct TextBlob
		 * @brief Represents precomputed text glyph data for efficient reuse.
		 *
		 * A text blob bundles glyph IDs, positions, and an optional cached image,
		 * allowing multiple draw calls without re-shaping or re-layout.
		 */
		struct TextBlob {
			Sp<Typeface>    typeface;   ///< Typeface used to render the text.
			Array<GlyphID>  glyphs;     ///< List of glyph indices.
			Array<Vec2>     offset;     ///< Glyph offsets (length = glyph count + 1).
			Typeface::TextImage img;    ///< Cached rasterized image (optional).
		};

		/**
		 * @struct V3F_T2F_C4B_C4B
		 * @brief Vertex structure with position, texture coordinate, and two colors.
		 *
		 * Used for low-level triangle drawing (e.g. gradients, lighting, or two-tone effects).
		 */
		struct V3F_T2F_C4B_C4B {
			Vec3  vertices;   ///< Vertex position (x, y, z).
			Vec2  texCoords;  ///< Texture coordinates (u, v).
			Color lightColor; ///< Primary color (usually lit color).
			Color darkColor;  ///< Secondary color (usually shadow/dark tone).
		};

		/**
		 * @struct Triangles
		 * @brief Encapsulates a triangle mesh submission.
		 *
		 * When using `drawTriangles()`, ensure vertex and index buffers remain valid
		 * until rendering completes.
		 */
		struct Triangles {
			V3F_T2F_C4B_C4B *verts = nullptr; ///< Vertex array.
			uint16_t *indices = nullptr;      ///< Index array.
			uint32_t vertCount = 0;           ///< Number of vertices.
			uint32_t indexCount = 0;          ///< Number of indices.
			float zDepthTotal = 0;            ///< Combined Z-depth for batching.
			bool isDarkColor = false;         ///< Whether darkColor is active.
		};

		// ---------------------------------------------------------------------
		//  State control
		// ---------------------------------------------------------------------

		/** Saves the current drawing state (matrix, clip, paint, etc.). */
		virtual int save() = 0;

		/** Restores one or more previously saved states. */
		virtual void restore(uint32_t count = 1) = 0;

		/** Returns the current save stack depth. */
		virtual int getSaveCount() const = 0;

		// ---------------------------------------------------------------------
		//  Matrix transformation
		// ---------------------------------------------------------------------

		/** Returns the current transformation matrix. */
		virtual const Mat& getMatrix() const = 0;

		/** Replaces the current transformation matrix. */
		virtual void setMatrix(const Mat& mat) = 0;

		/** Sets absolute translation transform. */
		virtual void setTranslate(Vec2 val) = 0;

		/** Applies relative translation transform. */
		virtual void translate(Vec2 val) = 0;

		/** Applies relative scaling transform. */
		virtual void scale(Vec2 val) = 0;

		/** Applies rotation around the origin (Z-axis). */
		virtual void rotate(float z) = 0;

		// ---------------------------------------------------------------------
		//  Clipping
		// ---------------------------------------------------------------------

		/** Clips to a vector path. */
		virtual void clipPath(const Path& path, ClipOp op, bool antiAlias) = 0;

		/** Clips to a pre-tessellated path variant. */
		virtual void clipPathv(const Pathv& path, ClipOp op, bool antiAlias) = 0;

		/** Clips to a rectangular region. */
		virtual void clipRect(const Rect& rect, ClipOp op, bool antiAlias);

		// ---------------------------------------------------------------------
		//  Basic drawing
		// ---------------------------------------------------------------------

		/** Clears the surface to the specified color. */
		virtual void clearColor(const Color4f& color) = 0;

		/** Fills the surface with a color using the specified blend mode. */
		virtual void drawColor(const Color4f& color, BlendMode mode) = 0;

		/** Draws a vector path with the given paint parameters. */
		virtual void drawPath(const Path& path, const Paint& paint) = 0;

		/** Draws a pre-tessellated path variant. */
		virtual void drawPathv(const Pathv& path, const Paint& paint) = 0;

		/** Draws a solid color path (non-painted) for simple fills. */
		virtual void drawPathvColor(const Pathv &path, const Color4f &color, BlendMode mode, bool antiAlias);

		/** Draws multiple colored paths at once for batching. */
		virtual void drawPathvColors(const Pathv* path[], int count, const Color4f &color, BlendMode mode, bool antiAlias) = 0;

		/** Draws an axis-aligned rectangle. */
		virtual void drawRect(const Rect& rect, const Paint& paint);

		/** Draws a rounded rectangle. */
		virtual void drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint);

		/** Draws an oval within the given bounding rect. */
		virtual void drawOval(const Rect& oval, const Paint& paint);

		/** Draws a circle at the specified center and radius. */
		virtual void drawCircle(Vec2 center, float radius, const Paint& paint);

		/** Draws positioned glyphs; returns total text advance. */
		virtual float drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint& paint) = 0;

		// ---------------------------------------------------------------------
		//  Advanced drawing
		// ---------------------------------------------------------------------

		/**
		 * Draws a triangle mesh.
		 * 
		 * @note Vertex/index data must remain alive until GPU submission finishes.
		 */
		virtual void drawTriangles(const Triangles& triangles, const Paint& paint) = 0;

		/**
		 * Optimized rounded-rect blur drawing using solid color fill.
		 * 
		 * @param rect Target bounds
		 * @param radius Corner radius per-corner
		 * @param blur   Blur radius
		 * @param color  Fill color
		 * @param mode   Blend mode
		 */
		virtual void drawRRectBlurColor(const Rect& rect,
			const float radius[4], float blur, const Color4f &color, BlendMode mode) = 0;

		// ---------------------------------------------------------------------
		//  Readback
		// ---------------------------------------------------------------------

		/**
		 * @method readPixels
		 * 
		 * Synchronously reads pixel data from the current render surface into `dst`.
		 * 
		 * ⚠️ **Warning — blocks the rendering pipeline:**  
		 * This call forces a full GPU→CPU sync and stalls the graphics command queue,
		 * which can severely impact performance, especially when invoked per frame.
		 * 
		 * Use only for debugging or one-time readback.  
		 * For general use, prefer the asynchronous `readImage()` method instead.
		 */
		virtual bool readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) = 0;

		/**
		 * @brief Asynchronously reads a rectangular region into a GPU-side ImageSource.
		 *
		 * Unlike `readPixels()`, this method does **not** stall the rendering pipeline.
		 * It schedules a non-blocking GPU→GPU copy operation that transfers the specified
		 * source region from the current render target into a new or existing GPU texture.
		 *
		 * The returned `ImageSource` object is fully initialized and can be used
		 * **immediately** as a texture input in subsequent draw calls or shader bindings.  
		 * The underlying GPU copy will complete asynchronously in the graphics queue.
		 *
		 * @param src   Source rectangle in current render target coordinates.
		 * @param dest  Destination offset within the new image.
		 * @param type  Color format for the image (default: RGBA_8888).
		 * @param mode  Blend mode used when compositing into the image.
		 * @param isMipmap  Whether to generate mipmaps for the resulting texture.
		 * @return A shared pointer to the resulting GPU-resident `ImageSource`.
		 *
		 * @note This operation is non-blocking on the CPU, but the GPU copy cost still
		 *       depends on backend implementation and queue synchronization.
		 */
		virtual Sp<ImageSource> readImage(const Rect &src, Vec2 dest, 
			ColorType type = kRGBA_8888_ColorType,
			BlendMode mode = kSrcOverPre_BlendMode, bool isMipmap = false) = 0;

		/**
		 * @brief Creates or binds a render target image as current output.
		 * 
		 * Returns an ImageSource representing the active target.  
		 * All subsequent draw calls will render into this image until `restore()` is invoked.
		 * 
		 * @param dest Existing image target (optional).
		 * @param isMipmap Whether to generate mipmaps for the render target.
		 */
		virtual Sp<ImageSource> outputImage(ImageSource* dest, bool isMipmap = false) = 0;

		/**
		 * @brief Draws a pre-computed text blob with baseline alignment.
		 */
		virtual void drawTextBlob(TextBlob* blob, Vec2 origin, float fontSize, const Paint& paint) = 0;

		// ---------------------------------------------------------------------
		//  System / utility
		// ---------------------------------------------------------------------

		/** Returns true if this canvas is backed by GPU rendering. */
		virtual bool isGpu();

		/** Swaps internal draw command buffers (multi-queue renderers). */
		virtual void swapBuffer() = 0;

		/** Returns the cached path vectorizer for path reuse. */
		virtual PathvCache* getPathvCache() = 0;

		/**
		 * @brief Updates surface transform and scaling parameters.
		 * 
		 * ⚠️ Not thread-safe — must be called from the same thread as draw operations,
		 * or guarded by external synchronization.
		 */
		virtual void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) = 0;

		/** Simplified overload of setSurface(). */
		void setSurface(Vec2 surfaceSize, float scale = 1);

		/** Returns the drawable surface size in pixels. */
		virtual Vec2 size() = 0;

	protected:
		Canvas() = default;
	};
}

#endif
