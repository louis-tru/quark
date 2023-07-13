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
#include "./gl_render.h"

namespace qk {

	bool gl_read_pixels(Pixel* dst, uint32_t srcX, uint32_t srcY);

	GLCanvas::GLCanvas(GLRender *backend)
		: _backend(backend)
		, _stencil_ref(0), _stencil_ref_decr(0)
		, _texTmp{0,0,0}
		, _curState(nullptr)
		, _surfaceScale(1,1), _surfaceScalef1(1), _transfromScale(1), _Scale(1)
	{
		glGenBuffers(1, &_mat_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, _mat_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 32, NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _mat_ubo);

		_state.push({ .matrix=Mat() }); // init state
		_curState = &_state.back();

		setMatrix(_curState->matrix); // init shader matrix
	}

	GLCanvas::~GLCanvas() {}

	int GLCanvas::save() {
		_state.push({ _state.back().matrix });
		_curState = &_state.back();
	}

	void GLCanvas::restore(uint32_t count) {
		if (!count || _state.length() == 1)
			return;

		int clipOp = -1;

		count = Uint32::min(count, _state.length() - 1);

		while (count > 0) {
			auto &state = _state.back();
			for (auto &clip: state.clips) {
				// restore clip
				if (clipOp == -1)
					glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);

				if (clip.op == kDifference_ClipOp) {
					_stencil_ref_decr++;
					Qk_ASSERT(_stencil_ref_decr <= 127);
					if (clipOp != kDifference_ClipOp) {
						clipOp = kDifference_ClipOp;
						glStencilOp(GL_KEEP, GL_INCR, GL_INCR); // Test success adds 1
					}
				} else {
					_stencil_ref--;
					Qk_ASSERT(_stencil_ref >= 127);
					if (clipOp != kIntersect_ClipOp) {
						clipOp = kIntersect_ClipOp;
						glStencilOp(GL_KEEP, GL_DECR, GL_DECR); // Test success decr 1
					}
				}

				_backend->_clip.use(clip.vertex.size(), *clip.vertex);
				glDrawArrays(GL_TRIANGLES, 0, clip.vertex.length()); // draw test
			}
			_curState = &_state.pop().back();
			setMatrixBuffer(_curState->matrix);
			count--;
		}

		if (isStencilRefDefaultValue()) {
			glDisable(GL_STENCIL_TEST); // disable stencil test
		}
		else if (clipOp != -1) {
			glStencilFunc(GL_LEQUAL, _stencil_ref, 0xFFFFFFFF); // Equality passes the test
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep
		}
	}

	int GLCanvas::getSaveCount() const {
		return _state.length() - 1;
	}

	const Mat& GLCanvas::getMatrix() const {
		return _curState->matrix;
	}

	bool GLCanvas::isStencilRefDefaultValue() {
		return _stencil_ref == 127 && _stencil_ref_decr == 127;
	}

	void GLCanvas::setRootMatrixBuffer(const Mat4& root) {
		// update all shader root matrix
		auto m4x4 = root.transpose(); // transpose matrix
		glBindBuffer(GL_UNIFORM_BUFFER, _mat_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, m4x4.val);

		// restore clip stencil
		glClear(GL_STENCIL_BUFFER_BIT);

		if (_stencil_ref != 0 && !isStencilRefDefaultValue()) {
			_stencil_ref = _stencil_ref_decr = 127;

			for (int i = 0; i < _state.length(); i++) {
				_curState = &_state[i];
				setMatrixBuffer(_curState->matrix);

				for (auto &clip: _curState->clips) {
					drawClip(&clip);
				}
			}
		}

		_surfaceScalef1 = Float::max(_surfaceScale[0], _surfaceScale[1]);
		_transfromScale = Float::max(_curState->matrix[0], _curState->matrix[4]);
		_Scale = _surfaceScalef1 * _transfromScale;
		_UnitPixel = 2 / _Scale;
	}

	void GLCanvas::setMatrixBuffer(const Mat& mat) {
		const float m4x4[16] = {
			mat[0], mat[3], 0.0, 0.0,
			mat[1], mat[4], 0.0, 0.0,
			0.0,    0.0,    1.0, 0.0,
			mat[2], mat[5], 0.0, 1.0
		}; // transpose matrix
		glBindBuffer(GL_UNIFORM_BUFFER, _mat_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, m4x4);

		auto mScale = Float::max(_curState->matrix[0], _curState->matrix[4]);
		if (_transfromScale != mScale) {
			_transfromScale = mScale;
			_Scale = _surfaceScalef1 * _transfromScale;
			_UnitPixel = 2 / _Scale;
		}
	}

	void GLCanvas::setMatrix(const Mat& mat) {
		_curState->matrix = mat;
		setMatrixBuffer(mat);
	}

	void GLCanvas::translate(float x, float y) {
		_curState->matrix.translate(x, y);
		setMatrixBuffer(_curState->matrix);
	}

	void GLCanvas::scale(float x, float y) {
		_curState->matrix.scale(x, y);
		setMatrixBuffer(_curState->matrix);
	}

	void GLCanvas::rotate(float z) {
		_curState->matrix.rotate(z);
		setMatrixBuffer(_curState->matrix);
	}

	bool GLCanvas::readPixels(Pixel* dst, uint32_t srcX, uint32_t srcY) {
		return gl_read_pixels(dst, srcX, srcY);
	}

	void GLCanvas::clipPath(const Path& path, ClipOp op, bool antiAlias) {
		if (_stencil_ref == 0) { // start enable stencil test
			_stencil_ref = _stencil_ref_decr = 127;
			glClear(GL_STENCIL_BUFFER_BIT); // clear stencil
		}
		if (isStencilRefDefaultValue()) {
			glEnable(GL_STENCIL_TEST); // enable stencil test
		}

		Clip clip{_backend->getPathTrianglesCache(path), op, antiAlias};

		if (drawClip(&clip)) {
			// save clip state
			_curState->clips.push(std::move(clip));
		}
	}

	bool GLCanvas::drawClip(Clip *clip) {
		// ignore anti alias
		if (clip->op == kDifference_ClipOp) {
			if (_stencil_ref_decr == 0) {
				Qk_WARN(" stencil ref decr value exceeds limit 0");
				return false;
			}
			_stencil_ref_decr--;
			glStencilOp(GL_KEEP, GL_DECR, GL_DECR); // Test success decr 1
		} else {
			if (_stencil_ref == 255) {
				Qk_WARN(" stencil ref value exceeds limit 255");
				return false;
			}
			_stencil_ref++;
			glStencilOp(GL_KEEP, GL_INCR, GL_INCR); // Test success adds 1
		}

		glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
		_backend->_clip.use(clip->vertex.size(), *clip->vertex);
		glDrawArrays(GL_TRIANGLES, 0, clip->vertex.length() >> 1); // draw test
		glStencilFunc(GL_LEQUAL, _stencil_ref, 0xFFFFFFFF); // Equality passes the test
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep

		return true;
	}

	void GLCanvas::clearColor(const Color4f& color) {
		glClearColor(color.r(), color.g(), color.b(), color.a());
		if (isStencilRefDefaultValue()) {
			glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		} else {
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	void GLCanvas::drawColor(const Color4f &color, BlendMode mode) {
		if (mode == kSrc_BlendMode) {
			clearColor(color);
		} else {
			_backend->setBlendMode(mode); // switch blend mode

			float data[] = {
				-1,1,  1,1,
				-1,-1, 1,-1,
			};
			_backend->_clear.use(sizeof(float) * 8, data);
			glUniform4fv(_backend->_clear.color, 1, color.val);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	void test_color_fill_aa_lines(
		GLSLColor &color,
		GLSLColorDotted &colorDotted, const Path &path, const Paint &paint
	) {
		Array<Vec2> vertex = path.getTriangles();
		color.use(vertex.size(), *vertex);
		glUniform4fv(color.color, 1, paint.color.val);
		// glDrawArrays(GL_TRIANGLES, 0, vertex.length());
		//glDrawArrays(GL_LINES, 0, vertex.length());

		Array<Vec2> lines = path.getEdgeLines();
		color.use(lines.size(), *lines);
		//Path p = path.normalizedPath();
		//color.use(p.ptsLen()*8, p.pts());

		//colorStroke.use(lines.size(), *lines);
		//glUniform4fv(colorStroke.color, 1, paint.color.val);
		//glUniform4fv(colorStroke.color, 1, Color4f(0,0,0).val);
		
		//glUniform4fv(color.color, 1,  Color4f(0,0,1,0.7).val);

#if Qk_OSX
		glEnable(GL_LINE_SMOOTH);
#endif

		glLineWidth(1);
		glDrawArrays(GL_LINES, 0, lines.length());
		//glDrawArrays(GL_LINE_STRIP, 0, p.ptsLen());
	}

	void GLCanvas::drawPath(const Path &path_, const Paint &paint) {
		_backend->setBlendMode(paint.blendMode); // switch blend mode

		bool antiAlias = paint.antiAlias && !_backend->_IsDeviceMsaa; // Anti-aliasing using software
		auto path = &_backend->getNormalizedPathCache(path_);

		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				fillPath(*path, paint, antiAlias);
				break;
			case Paint::kStrokeAndFill_Style:
				fillPath(*path, paint, antiAlias);
			case Paint::kStroke_Style: {
				if (antiAlias) {
					auto width = paint.width - _UnitPixel;
					if (width > 0) {
						fillPath(_backend->getStrokePathCache(*path, width, paint.cap, paint.join), paint, true);
					} else {
						// 5*5=25, 0.75
						width /= _UnitPixel; // range: -1 => 0
						width = powf(width*10, 3) * 0.006; // (width*10)^3 * 0.006
						const float sdf_range[3] = {0.5, width-0.25f, 0};
						drawAAStrokeSDF(*path, paint, sdf_range);
					}
				} else {
					fillPath(_backend->getStrokePathCache(*path, paint.width, paint.cap, paint.join), paint, false);
				}
				break;
			}
		}
	}

	void GLCanvas::fillPath(const Path &path, const Paint &paint, bool antiAlias) {
		Qk_ASSERT(path.isNormalized());

		auto &triangles = _backend->getPathTrianglesCache(path);
		switch (paint.type) {
			case Paint::kColor_Type:
				drawColor(triangles, paint, GL_TRIANGLES);
				//test_color_fill_aa_lines(_backend->_color, _backend->_colorDotted, path, paint);
				break;
			case Paint::kGradient_Type:
				drawGradient(triangles, paint, GL_TRIANGLES);
				break;
			case Paint::kBitmap_Type:
				drawImage(triangles, paint, GL_TRIANGLES);
				break;
			case Paint::kBitmapMask_Type:
				drawImageMask(triangles, paint, GL_TRIANGLES);
				break;
		}

		if (antiAlias) {
			constexpr float sdf_range[3] = {0.5,-0.25,0};
			drawAAStrokeSDF(path, paint, sdf_range);
		}
	}

	void GLCanvas::drawAAStrokeSDF(const Path& path, const Paint& paint, const float sdf_range[3]) {
		//Path newPath(path); newPath.transfrom(Mat(1,0,170,0,1,0));
		//auto &strip = _backend->getSDFStrokeTriangleStripCache(newPath, _Scale);
		// _UnitPixel*0.6=1.2/_Scale, 2.4px
		auto &strip = _backend->getSDFStrokeTriangleStripCache(path, _UnitPixel*0.6);
		// Qk_DEBUG("%p", &strip);
		switch (paint.type) {
			case Paint::kColor_Type:
				drawColorSDF(strip, paint, GL_TRIANGLE_STRIP, sdf_range);
				break;
			case Paint::kGradient_Type:
				drawGradientSDF(strip, paint, GL_TRIANGLE_STRIP, sdf_range);
				break;
			case Paint::kBitmap_Type:
				drawImageSDF(strip, paint, GL_TRIANGLE_STRIP, sdf_range);
				break;
			case Paint::kBitmapMask_Type:
				drawImageMaskSDF(strip, paint, GL_TRIANGLE_STRIP, sdf_range);
				break;
		}
	}

	// ----------------------------------------------------------------------------------------

	void GLCanvas::drawColor(const Array<Vec2> &vertex, const Paint &paint, GLenum mode) {
		_backend->_color.use(vertex.size(), *vertex);
		glUniform4fv(_backend->_color.color, 1, paint.color.val);
		glDrawArrays(mode, 0, vertex.length());
	}

	void GLCanvas::drawGradient(const Array<Vec2> &vertex, const Paint &paint, GLenum mode) {
		auto g = paint.gradient;
		auto shader = paint.gradientType ==
			Paint::kRadial_GradientType ? &_backend->_radial:
			static_cast<GLSLColorRadial*>(static_cast<GLSLShader*>(&_backend->_linear));
		auto count = Qk_MIN(g->colors.length(), 256);
		shader->use(vertex.size(), *vertex);
		glUniform4fv(shader->range, 1, paint.color.val);
		glUniform1i(shader->count, count);
		glUniform4fv(shader->colors, count, (const GLfloat*)g->colors.val());
		glUniform1fv(shader->positions, count, (const GLfloat*)g->positions.val());
		glDrawArrays(mode, 0, vertex.length());
	}

	void GLCanvas::drawImage(const Array<Vec2> &vertex, const Paint &paint, GLenum mode) {
		auto shader = &_backend->_image;
		auto pixel = paint.image;
		auto type = pixel->type();
		auto texCount = 1;

		if (type == kColor_Type_YUV420P_Y_8) {
			shader = static_cast<GLSLImage*>(static_cast<GLSLShader*>(&_backend->_yuv420p));
			texCount = 3;
		} else if (type == kColor_Type_YUV420SP_Y_8) {
			shader = static_cast<GLSLImage*>(static_cast<GLSLShader*>(&_backend->_yuv420sp));
			texCount = 2;
		}
		for (int i = 0; i < texCount; i++) {
			_backend->setTexture(pixel + i, i, paint);
		}
		shader->use(vertex.size(), *vertex);
		glUniform1f(shader->opacity, paint.color.a());
		glUniform4fv(shader->coord, 1, paint.region.origin.val);
		glDrawArrays(mode, 0, vertex.length());
	}

	void GLCanvas::drawImageMask(const Array<Vec2> &vertex, const Paint &paint, GLenum mode) {
		auto shader = &_backend->_colorMask;
		_backend->setTexture(paint.image, 0, paint);
		shader->use(vertex.size(), *vertex);
		glUniform4fv(shader->color, 1, paint.color.val);
		glUniform4fv(shader->coord, 1, paint.region.origin.val);
		glDrawArrays(mode, 0, vertex.length());
	}

	// ----------------------------------------------------------------------------------------

	void GLCanvas::drawColorSDF(const Array<Vec3> &vertex, const Paint& paint, GLenum mode, const float range[2]) {
		_backend->_colorSdf.use(vertex.size(), *vertex);
		glUniform1fv(_backend->_colorSdf.sdf_range, 3, range);
		glUniform4fv(_backend->_colorSdf.color, 1, paint.color.val);
		glDrawArrays(mode, 0, vertex.length());
	}

	void GLCanvas::drawGradientSDF(const Array<Vec3> &vertex, const Paint& paint, GLenum mode, const float range[3]) {
		auto g = paint.gradient;
		auto shader = paint.gradientType ==
			Paint::kRadial_GradientType ? &_backend->_radialSdf:
			static_cast<GLSLColorRadialSdf*>(static_cast<GLSLShader*>(&_backend->_linearSdf));
		auto count = Qk_MIN(g->colors.length(), 256);
		shader->use(vertex.size(), *vertex);
		glUniform1fv(shader->sdf_range, 2, range);
		glUniform4fv(shader->range, 3, paint.color.val);
		glUniform1i(shader->count, count);
		glUniform4fv(shader->colors, count, (const GLfloat*)g->colors.val());
		glUniform1fv(shader->positions, count, (const GLfloat*)g->positions.val());
		glDrawArrays(mode, 0, vertex.length());
	}

	void GLCanvas::drawImageSDF(const Array<Vec3> &vertex, const Paint& paint, GLenum mode, const float range[3]) {
		auto shader = &_backend->_imageSdf;
		auto type = paint.image->type();
		if (type == kColor_Type_YUV420P_Y_8 || type == kColor_Type_YUV420SP_Y_8) {
			return; // ignore
		}
		//setTexturePixel(paint.image, 0, paint);
		shader->use(vertex.size(), *vertex);
		glUniform1fv(shader->sdf_range, 3, range);
		glUniform1f(shader->opacity, paint.color.a());
		glUniform4fv(shader->coord, 1, paint.region.origin.val);
		glDrawArrays(mode, 0, vertex.length());
	}

	void GLCanvas::drawImageMaskSDF(const Array<Vec3> &vertex, const Paint& paint, GLenum mode, const float range[3]) {
		auto shader = &_backend->_colorMaskSdf;
		//setTexturePixel(paint.image, 0, paint);
		shader->use(vertex.size(), *vertex);
		glUniform1fv(shader->sdf_range, 3, range);
		glUniform4fv(shader->color, 1, paint.color.val);
		glUniform4fv(shader->coord, 1, paint.region.origin.val);
		glDrawArrays(mode, 0, vertex.length());
	}

	// ----------------------------------------------------------------------------------------

	float GLCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint)
	{
		_backend->setBlendMode(paint.blendMode); // switch blend mode

		Sp<ImageSource> img;
		auto tf = glyphs.typeface();
		auto bound = tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _Scale, offset, &img);
		auto scale_1 = drawTextImage(*img, bound.y(), _Scale, origin, paint);
		return scale_1 * bound.x();
	}

	void GLCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		_backend->setBlendMode(paint.blendMode); // switch blend mode

		fontSize *= _transfromScale;
		auto levelSize = get_level_font_size(fontSize);
		auto levelScale = fontSize / levelSize;
		auto imageFontSize = levelSize * _surfaceScalef1;

		if (imageFontSize == 0.0)
			return;

		if (blob->imageFontSize != imageFontSize || !blob->image) {
			auto tf = blob->typeface;
			auto offset = blob->offset.length() == blob->glyphs.length() ? &blob->offset: NULL;
			blob->imageBound = tf->getImage(blob->glyphs,imageFontSize, offset, &blob->image);
			blob->image->mark_as_texture_unsafe(_backend);
		}

		drawTextImage(*blob->image, blob->imageBound.y(), _Scale * levelScale, origin, paint);
	}

	float GLCanvas::drawTextImage(ImageSource *textImg, float imgTop, float scale, Vec2 origin, const Paint &paint) {
		auto &pix = textImg->pixels().front();
		auto scale_1 = 1.0 / scale;
		Paint p(paint);

		// default use baseline align
		Vec2 dst_start(origin.x(), origin.y() - imgTop * scale_1);
		Vec2 dst_size(pix.width() * scale_1, pix.height() * scale_1);

		p.setBitmapPixel(&pix, {dst_start, dst_size});

		Vec2 v1(dst_start.x() + dst_size.x(), dst_start.y());
		Vec2 v2(dst_start.x(), dst_start.y() + dst_size.y());
		Vec2 v3(dst_start + dst_size);

		Array<Vec2> vertex{
			// triangle 0
			dst_start,
			v1,
			v2,
			// triangle 1
			v2,
			v3,
			v1,
		};

		drawImageMask(vertex, p, GL_TRIANGLES);

		return scale_1;
	}

}
