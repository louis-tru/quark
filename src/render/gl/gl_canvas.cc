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

namespace qk {

	GLCanvas::GLCanvas()
		: _blendMode(kClear_BlendMode)
		, _IsDeviceAntiAlias(false)
		, _linear(GradientPaint::kLinear)
		, _radial(GradientPaint::kRadial)
	{
		glGenBuffers(1, &_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _ubo);

		GLSLShader *shaders[] = {
			&_color, &_image, &_linear, &_radial
		};
		for (auto shader: shaders) {
			shader->build();
		}

		glUseProgram(_image.shader());
		glUniform1i(_image.image(), 0);
		glUseProgram(0);
	}

	void GLCanvas::setMatrix(const Mat& mat) {
		float mat4[16] = {
			mat[0], mat[3], 0.0, 0.0,
			mat[1], mat[4], 0.0, 0.0,
			0.0,    0.0,    1.0, 0.0,
			mat[2], mat[5], 0.0, 1.0
		};
		// glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, mat4);
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
				polygons = path.getPolygons(3, antiAlias);
				break;
			case Paint::kStroke_Style:
				polygons = path.strokePath(paint.width, paint.join).getPolygons(3, antiAlias);
				break;
			case Paint::kStrokeAndFill_Style:
				polygons = path.extendPath(paint.width * 0.5, paint.join).getPolygons(3, antiAlias);
				break;
		}

		// fill polygons
		switch (paint.type) {
			case Paint::kColor_Type:
				drawColor(polygons, paint); break;
			case Paint::kGradient_Type:
				drawGradient(polygons, paint); break;
			case Paint::kImage_Type:
				drawImage(polygons, paint); break;
		}
	}

	void GLCanvas::drawColor(const Array<Vec3>& triangles, const Paint& paint) {
		glUseProgram(_color.shader());
		glUniform4fv(_color.color(), 1, paint.color.val);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangles.val());
		glDrawArrays(GL_TRIANGLES, 0, triangles.length());
	}

	void GLCanvas::drawGradient(const Array<Vec3>& triangles, const Paint& paint) {
		const GradientPaint *g = paint.gradient();
		auto shader = g->type() == GradientPaint::kLinear ? &_linear: &_radial;
		glUseProgram(shader->shader());
		glUniform4fv(shader->range(), 1, g->range());
		glUniform1i(shader->count(), g->colors().length());
		glUniform4fv(shader->colors(), g->colors().length(), (const GLfloat*)g->colors().val());
		glUniform1fv(shader->positions(), g->colors().length(), (const GLfloat*)g->positions().val());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangles.val());
		glDrawArrays(GL_TRIANGLES, 0, triangles.length());
	}

	void GLCanvas::drawImage(const Array<Vec3>& triangles, const Paint& paint) {
		auto image = *paint.image;
		glUseProgram(_image.shader());
		glUniform1f(_image.opacity(), paint.opacity);
		glUniform4fv(_image.coord(), 1, paint.color.val);
		// set image texture data
		// ..
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangles.val());
		glDrawArrays(GL_TRIANGLES, 0, triangles.length());
	}

	void GLCanvas::drawGlyphs(const Array<GlyphID>& glyphs, const Array<Vec2>& positions,
		Vec2 origin, float fontSize, Typeface* typeface, const Paint& paint) 
	{
		// TODO ...
	}

	void GLCanvas::drawTextBlob(TextBlob* blob, Vec2 origin, float floatSize, const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::setBlendMode(BlendMode blendMode) {

		switch (blendMode) {
			case kClear_BlendMode:         //!< r = 0
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kSrc_BlendMode:           //!< r = s
				glBlendFunc(GL_ONE, GL_ZERO);
				break;
			case kDst_BlendMode:           //!< r = d
				glBlendFunc(GL_ZERO, GL_ONE);
				break;
			case kSrcOver_BlendMode:       //!< r = s + (1-sa)*d
				/** [Sa + (1 - Sa)*Da, Rc = Sc + (1 - Sa)*Dc] */
				// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kDstOver_BlendMode:       //!< r = (1-da)*s + d
				/** [Sa + (1 - Sa)*Da, Rc = Dc + (1 - Da)*Sc] */
				// glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
				glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
				break;
			case kSrcIn_BlendMode:         //!< r = da*s
				glBlendFunc(GL_DST_ALPHA, GL_ZERO);
				break;
			case kDstIn_BlendMode:         //!< r = sa*d
				glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
				break;
			case kSrcOut_BlendMode:        //!< r = (1-da)*s
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
				break;
			case kDstOut_BlendMode:        //!< r = (1-sa)*d
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kSrcATop_BlendMode:       //!< r = da*s + (1-sa)*d
				glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kDstATop_BlendMode:       //!< r = (1-da)*s + sa*d
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
				break;
			case kXor_BlendMode:           //!< r = (1-da)*s + (1-sa)*d
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kPlus_BlendMode:          //!< r = min(s + d, 1)
				glBlendFunc(GL_ONE, GL_ONE);
				break;
			case kModulate_BlendMode:      //!< r = s*d
				glBlendFunc(GL_ZERO, GL_SRC_COLOR);
				break;
			case kScreen_BlendMode:        //!< r = s + d - s*d
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				break;
		}

		_blendMode = blendMode;
	}

}
