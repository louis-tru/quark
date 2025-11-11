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

#include "./paint.h"
#include "./canvas.h"
#include "./source.h"

namespace qk {

	void PaintImage_setImage(PaintImage *paint, const Rect &dest, const Rect &src, Vec2 srcSize) {
		Vec2 scale(dest.size.x() / src.size.x(), dest.size.y() / src.size.y());
		paint->coord = {
			Vec2(
				src.begin.x() * scale.x() - dest.begin.x(),
				src.begin.y() * scale.y() - dest.begin.y()
			),
			Vec2(scale.x() * srcSize[0], scale.y() * srcSize[1]), // shader tex scale
		};
	}

	void PaintImage::setCanvas(Canvas *canvas, const Rect &dest, const Rect &src) {
		this->canvas = canvas;
		_isCanvas = true;
		PaintImage_setImage(this, dest, src, canvas->size());
	}

	void PaintImage::setCanvas(Canvas *canvas, const Rect &dest) {
		setCanvas(canvas, dest, { 0, canvas->size() });
	}

	void PaintImage::setImage(ImageSource *image, const Rect& dest, const Rect& src) {
		this->image = image;
		_isCanvas = false;
		PaintImage_setImage(this, dest, src, Vec2(image->width(), image->height()));
	}

	void PaintImage::setImage(ImageSource *image, const Rect &dest) {
		setImage(image, dest, { 0, Vec2(image->width(), image->height()) });
	}

	// -------------------------------------------------------

	void paint_test() {
		Paint paint;
		Array<Color4f> colors{Color4f(0,0,0),Color4f(1,1,1)};
		Array<float>   pos{0,0.5,1};
		PaintGradient  gr{
			PaintGradient::kLinear_Type, Vec2{0,0}, Vec2{1,1},
			(uint32_t)colors.length(), colors.val(), pos.val()
		};

		paint.antiAlias = true;
		paint.fill.gradient = &gr;
	}

}
