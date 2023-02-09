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

#include "./gl_canvas.h"

namespace quark {

	GLCanvas::GLCanvas()
		: _blendMode(kClear_BlendMode)
		, _IsDeviceAntiAlias(false)
	{
	}

	int  GLCanvas::save() {
		// TODO ...
	}

	void GLCanvas::restore() {
		// TODO ...
	}

	int  GLCanvas::getSaveCount() const {
		// TODO ...
	}

	void GLCanvas::restoreToCount(int saveCount) {
		// TODO ...
	}

	bool GLCanvas::readPixels(Pixel* dstPixels, int srcX, int srcY) {
		// TODO ...
	}

	void GLCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		// TODO ...
	}

	void GLCanvas::clipPath(const Path& path, ClipOp op, bool antiAlias) {
		// TODO ...
	}

	void GLCanvas::drawPaint(const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::drawPath(const Path& path, const Paint& paint) {

		bool antiAlias = paint.antiAlias && !_IsDeviceAntiAlias; // Anti-aliasing using software

		Array<Vec3> polygons;

		if (_blendMode != paint.blendMode) {
			setBlendMode(paint.blendMode); // switch blend mode
		}

		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				polygons = path.to_polygons(3, antiAlias);
				break;
			case Paint::kStroke_Style:
				polygons = path.genStrokePath(paint.width, paint.join, false)
					.to_polygons(3, antiAlias);
				break;
			case Paint::kStrokeAndFill_Style:
				polygons = path.genStrokePath(paint.width, paint.join, true)
					.to_polygons(3, antiAlias);
				break;
		}

		// fill polygons
		switch (paint.type) {
			case Paint::kColor_Type:
				fillColor(polygons, paint); break;
			case Paint::kGradient_Type:
				fillGradient(polygons, paint); break;
			case Paint::kImage_Type:
				fillImage(polygons, paint); break;
		}
	}

	void GLCanvas::drawGlyphs(const Array<GlyphID>& glyphs, const Array<Vec2>& positions,
		Vec2 origin, float fontSize, Typeface* typeface, const Paint& paint) 
	{
		// TODO ...
	}

	void GLCanvas::drawTextBlob(TextBlob* blob, Vec2 origin, float floatSize, const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::fillColor(const Array<Vec3>& triangles, const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::fillGradient(const Array<Vec3>& triangles, const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::fillImage(const Array<Vec3>& triangles, const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::setBlendMode(BlendMode blendMode) {

		switch (blendMode) {
			case kClear_BlendMode:         //!< r = 0
				break;
			case kSrc_BlendMode:           //!< r = s
				break;
			case kDst_BlendMode:           //!< r = d
				break;
			case kSrcOver_BlendMode:       //!< r = s + (1-sa)*d
				break;
			case kDstOver_BlendMode:       //!< r = d + (1-da)*s
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kDstIn_BlendMode:         //!< r = d * sa
				break;
			case kDstOut_BlendMode:        //!< r = d * (1-sa)
				break;
			case kSrcIn_BlendMode:         //!< r = s * da
				break;
			case kSrcOut_BlendMode:        //!< r = s * (1-da)
				break;
			case kSrcATop_BlendMode:       //!< r = s*da + d*(1-sa)
				break;
			case kDstATop_BlendMode:       //!< r = d*sa + s*(1-da)
				break;
			case kXor_BlendMode:           //!< r = s*(1-da) + d*(1-sa)
				break;
			case kPlus_BlendMode:          //!< r = min(s + d, 1)
				break;
			case kModulate_BlendMode:      //!< r = s*d
				break;
			case kScreen_BlendMode:        //!< r = s + d - s*d
				break;
		}

		_blendMode = blendMode;
	}

}
