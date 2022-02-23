/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __flare__render__canvas__
#define __flare__render__canvas__

#include "../util/util.h"
#include "../math.h"
#include "../value.h"
#include "./pixel.h"

namespace flare {

	class Render;

	class F_EXPORT Canvas: public Object {
		F_HIDDEN_ALL_COPY(Canvas);
		public:
			struct Options {
				ColorType colorType;
				uint32_t  width;
				uint32_t  height;
				// gpu opts:
				int   msaaSampleCount;
				bool  enableDepthTest;
				bool  enableStencilTest;
				bool  ebableColorBlend;
				bool  alwaysOpenTest;
			};
			Canvas(Render* host);
			virtual ~Canvas();
			virtual bool settings(Options opts);
			inline Render* host() { return _host; }
			inline const Options& options() const { return _opts; }
			virtual void begin() = 0;
			virtual void submit() = 0;
			virtual Sp<ImageSource> makeImageSnapshot() = 0;
			// bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY);
			// bool writePixels(const SkImageInfo& info, const void* pixels, size_t rowBytes, int x, int y);
			// int save();
			// void restore();
			// int getSaveCount() const;
			// void restoreToCount(int saveCount);
			virtual void setMatrix(const Mat& mat) = 0;
			// void clipRect(const SkRect& rect, SkClipOp op, bool doAntiAlias);
			// void clipRRect(const SkRRect& rrect, SkClipOp op, bool doAntiAlias);
			// void clipPath(const SkPath& path, SkClipOp op, bool doAntiAlias);
			// void clipShader(sk_sp<SkShader>, SkClipOp = SkClipOp::kIntersect);
			// void clipRegion(const SkRegion& deviceRgn, SkClipOp op = SkClipOp::kIntersect);
			// bool quickReject(const SkRect& rect) const;
			// bool quickReject(const SkPath& path) const;
			// SkRect getLocalClipBounds() const;
			// void drawColor(const SkColor4f& color, SkBlendMode mode = SkBlendMode::kSrcOver);
			virtual void clear(Color color) = 0;
			// void drawPaint(const SkPaint& paint);
			// void drawPoints(PointMode mode, size_t count, const Vec2 pts[], const SkPaint& paint);
			// void drawPoint(float x, float y, const SkPaint& paint);
			// void drawLine(float x0, float y0, float x1, float y1, const SkPaint& paint);
			// void drawRect(const Rect& rect, const SkPaint& paint);
			// void drawRegion(const SkRegion& region, const SkPaint& paint);
			// void drawOval(const Rect& oval, const SkPaint& paint);
			// void drawRRect(const SkRRect& rrect, const SkPaint& paint);
			// void drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint);
			// void drawCircle(Vec2 center, float radius, const SkPaint& paint);
			// void drawArc(const Rect& oval, float startAngle, float sweepAngle, bool useCenter, const SkPaint& paint);
			// void drawRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry, const SkPaint& paint);
			// void drawPath(const SkPath& path, const SkPaint& paint);
			virtual void drawImage(ImageSource* source, float x, float y) = 0;
			//virtual void drawImageRect(ImageSource* image, const Rect& src, const Rect& dst,
			//	const SkSamplingOptions& sampling, const SkPaint* paint, SrcRectConstraint constraint) = 0;
			// void drawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst, 
			//	SkFilterMode filter, const SkPaint* paint = nullptr);
			// void drawImageLattice(ImageSource* image, const Lattice& lattice, const SkRect& dst,
			// 	SkFilterMode filter, const SkPaint* paint = nullptr);
			// void drawSimpleText(const char* text, size_t byteLength, SkTextEncoding encoding,
			// 							SkScalar x, SkScalar y, const SkFont& font, const SkPaint& paint);
			// void drawGlyphs(int count, const SkGlyphID glyphs[], const SkPoint positions[],
			//                 const uint32_t clusters[], int textByteCount, const char utf8text[],
			//                 SkPoint origin, const SkFont& font, const SkPaint& paint);
			// void drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint);
			// void drawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint);
			// void drawVertices(const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint);
			// void drawPatch(const SkPoint cubics[12], const SkColor colors[4],
			//              const SkPoint texCoords[4], SkBlendMode mode, const SkPaint& paint);
			// void drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
			// 							const SkColor colors[], int count, SkBlendMode mode,
			// 							const SkSamplingOptions& sampling, const SkRect* cullRect, const SkPaint* paint);
			// void drawDrawable(SkDrawable* drawable, const SkMatrix* matrix = nullptr);
			// void drawAnnotation(const SkRect& rect, const char key[], SkData* value);
		private:
			Render* _host;
			Options _opts;
	};
}

#endif
