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

#ifndef __quark_render_metal_metalcanvas__
#define __quark_render_metal_metalcanvas__

#include "../canvas.h"

namespace qk {

	class MetalCanvas: public Canvas {
	public:
		virtual int  save() override;
		virtual void restore() override;
		virtual int  getSaveCount() const override;
		virtual void restoreToCount(int saveCount) override;
		virtual bool readPixels(Pixel* dstPixels, int srcX, int srcY) override;
		virtual void clipRect(const Rect& rect, ClipOp op, bool doAntiAlias) override;
		virtual void clipPath(const Path& path, ClipOp op, bool doAntiAlias) override;
		virtual void drawColor(const Color4f& color, BlendMode mode) override;
		virtual void drawPaint(const Paint& paint) override;
		virtual void drawRect(const Rect& rect, const Paint& paint) override;
		virtual void drawPath(const Path& path, const Paint& paint) override;
		virtual void drawOval(const Rect& oval, const Paint& paint) override;
		virtual void drawCircle(Vec2 center, float radius, const Paint& paint) override;
		virtual void drawArc(const Rect& oval, float startAngle, float sweepAngle,
			bool useCenter, const Paint& paint) override;
		virtual void drawGlyphs(const Array<GlyphID>& glyphs, const Array<Vec2>& positions,
			Vec2 origin, float fontSize, Typeface* typeface, const Paint& paint) override;
		virtual void drawTextBlob(TextBlob* blob, Vec2 origin, float floatSize, const Paint& paint) override;
	};

}
#endif
