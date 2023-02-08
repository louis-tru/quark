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

#ifndef __quark_render_canvas__
#define __quark_render_canvas__

#include "../util/util.h"
#include "./path.h"
#include "./pixel.h"
#include "./paint.h"
#include "./font/metrics.h"
#include "../text/text_blob.h"

namespace quark {

	/**
	 * @class Canvas base abstract type, define all draw apis
	 */
	class Qk_EXPORT Canvas: public Object {
		Qk_HIDDEN_ALL_COPY(Canvas);
	public:
		enum ClipOp {
			kDifference_ClipOp,
			kIntersect_ClipOp,
		};
		virtual int  save() = 0;
		virtual void restore() = 0;
		virtual int  getSaveCount() const = 0;
		virtual void restoreToCount(int saveCount) = 0;
		virtual bool readPixels(Pixel* dstPixels, int srcX, int srcY) = 0;
		virtual void clipRect(const Rect& rect, ClipOp op, bool doAntiAlias) = 0;
		virtual void clipPath(const Path& path, ClipOp op, bool doAntiAlias) = 0;
		virtual void drawColor(const Color4f& color, BlendMode mode = kSrcOver_BlendMode) = 0;
		virtual void drawPaint(const Paint& paint) = 0;
		virtual void drawRect(const Rect& rect, const Paint& paint);
		virtual void drawPath(const Path& path, const Paint& paint) = 0;
		virtual void drawOval(const Rect& oval, const Paint& paint);
		virtual void drawCircle(Vec2 center, float radius, const Paint& paint);
		virtual void drawGlyphs(const Array<GlyphID>& glyphs, const Array<Vec2>& positions,
			Vec2 origin, float fontSize, Typeface* typeface, const Paint& paint) = 0;
		virtual void drawTextBlob(TextBlob* blob, Vec2 origin, float floatSize, const Paint& paint) = 0;
	protected:
		Canvas() = default;
	};

}

#endif