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

#ifndef __quark_render_canvas__
#define __quark_render_canvas__

#include "../util/util.h"
#include "./font/families.h"
#include "./pathv_cache.h"
#include "./pixel.h"
#include "./paint.h"

namespace qk {

	/**
	 * @class Canvas base abstract type, define all draw apis
	 */
	class Qk_EXPORT Canvas: public Reference {
		Qk_HIDDEN_ALL_COPY(Canvas);
	public:
		enum ClipOp {
			kDifference_ClipOp,
			kIntersect_ClipOp,
		};

		struct TextBlob {
			Sp<Typeface>    typeface;
			Array<GlyphID>  glyphs;
			Array<Vec2>     offset; // length = glyphs count + 1
			Typeface::ImageOut out;// output image cache
		};

		virtual int  save() = 0;
		virtual void restore(uint32_t count = 1) = 0;
		virtual int  getSaveCount() const = 0;
		virtual const Mat& getMatrix() const = 0;
		virtual void setMatrix(const Mat& mat) = 0;
		virtual void setTranslate(Vec2 val) = 0;
		virtual void translate(Vec2 val) = 0;
		virtual void scale(Vec2 val) = 0;
		virtual void rotate(float z) = 0; // arc rotation
		virtual void clipPath(const Path& path, ClipOp op, bool antiAlias) = 0;
		virtual void clipPathv(const Pathv& path, ClipOp op, bool antiAlias) = 0;
		virtual void clipRect(const Rect& rect, ClipOp op, bool antiAlias);
		virtual void clearColor(const Color4f& color) = 0;
		virtual void drawColor(const Color4f& color, BlendMode mode = kSrcOver_BlendMode) = 0;
		virtual void drawPath(const Path& path, const Paint& paint) = 0;
		virtual void drawPathv(const Pathv& path, const Paint& paint) = 0;
		virtual void drawPathvColor(const Pathv &path, const Color4f &color, BlendMode mode);
		virtual void drawPathvColors(const Pathv* path[], int count, const Color4f &color, BlendMode mode) = 0;
		virtual void drawRect(const Rect& rect, const Paint& paint);
		virtual void drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint);
		virtual void drawOval(const Rect& oval, const Paint& paint);
		virtual void drawCircle(Vec2 center, float radius, const Paint& paint);
		virtual float drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint& paint) = 0;

		/**
		 * Optimized rounded rect blur color drawing command
		*/
		virtual void drawRRectBlurColor(const Rect& rect,
			const float radius[4], float blur, const Color4f &color, BlendMode mode) = 0;

		/**
		 * @method readPixels() sync read pixels caution use
		*/
		virtual bool readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) = 0;

		/**
		 * Return an image source does not plunger threads, async read image
		*/
		virtual Sp<ImageSource> readImage(const Rect &src, Vec2 dest, ColorType type, bool isMipmap = false) = 0;

		/**
		 * Returns an image buffer and switches canvas output to dest image buffer, share the current canvas state.
		 * all drawing commands target the returned image until the end of calling 'restore()'
		 * 
		 * @param dest {ImageSource*} Create a new image if the image is empty
		*/
		virtual Sp<ImageSource> outputImage(ImageSource* dest, bool isMipmap = false) = 0;

		/**
		 * @dev drawTextBlob() Draw with text baseline aligned
		*/
		virtual void drawTextBlob(TextBlob* blob, Vec2 origin, float fontSize, const Paint& paint) = 0;

		/**
		 * return is gpu implement
		*/
		virtual bool isGpu();

		/**
		 * swap draw commands buffer, if there are multiple command queues
		*/
		virtual void swapBuffer() = 0;

		/**
		 * @method getPathvCache() Returns pathv cache object
		*/
		virtual PathvCache* getPathvCache() = 0;

		/**
		 * @method setSurface() Surface params change reload
		 * 
		 * Note that it is not thread safe and must be used in the same thread as the drawing method
		 * 
		 * or displayed thread mutual exclusion measures
		*/
		virtual void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) = 0;

		/**
		 * @method setSurface() Surface params change reload, Simplified approach
		*/
		void setSurface(Vec2 surfaceSize, float scale = 1);

		/**
		 * @method get this canvas draw region size
		*/
		virtual Vec2 size() = 0;

	protected:
		Canvas() = default;
	};

}

#endif
