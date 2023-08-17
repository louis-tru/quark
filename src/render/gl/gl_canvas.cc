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

	float get_level_font_size(float fontSize);
	bool  gl_read_pixels(Pixel* dst, uint32_t srcX, uint32_t srcY);

	void test_color_fill_aa_lines(
		GLSLColor &color, const Path &path, const Paint &paint
	) {
		auto vertex = path.getTriangles();
		color.use(vertex);
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

	constexpr float aa_sdf_range[3] = {0.5,-0.25,0};
	constexpr float aa_sdf_width = 0.6;
	// constexpr float aa_sdf_range[3] = {1,0,0}; // test
	// constexpr float aa_sdf_width = 4; // test

	Qk_DEFINE_INLINE_MEMBERS(GLCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLCanvas::Inl*>(self)

		bool isStencilTest() {
			return _stencilRef != 127 || _stencilRefDecr != 127;
		}

		void setMatrixBuffer(const Mat& mat) {
			const float m4x4[16] = {
				mat[0], mat[3], 0.0, 0.0,
				mat[1], mat[4], 0.0, 0.0,
				0.0,    0.0,    1.0, 0.0,
				mat[2], mat[5], 0.0, 1.0
			}; // transpose matrix
			glBindBuffer(GL_UNIFORM_BUFFER, _matUbo);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, m4x4);

			auto mScale = Float::max(_state->matrix[0], _state->matrix[4]);
			if (_transfromScale != mScale) {
				_transfromScale = mScale;
				_scale = _surfaceScale * _transfromScale;
				_unitPixel = 2 / _scale;
			}
		}

		// const Path &path, const VertexData &vertex
		void clipv(const Path &path, const VertexData& vertex, ClipOp op, bool antiAlias) {
			if (!isStencilTest()) {
				glEnable(GL_STENCIL_TEST); // enable stencil test
			}
			Clip clip{.op=op,.aa=antiAlias};
			clip.path.path = path;
			_render->copyVertexData(vertex, &clip.path);
			if (drawClip(clip)) { // save clip state
				_state->clips.push(std::move(clip));
			}
		}

		bool drawClip(Clip &clip) {
			// ignore anti alias
			if (clip.op == kDifference_ClipOp) {
				if (_stencilRefDecr == 0) {
					Qk_WARN(" stencil ref decr value exceeds limit 0"); return false;
				}
				_stencilRefDecr--;
				glStencilOp(GL_KEEP, GL_DECR, GL_DECR); // Test success decr 1
			} else {
				if (_stencilRef == 255) {
					Qk_WARN(" stencil ref value exceeds limit 255"); return false;
				}
				_stencilRef++;
				glStencilOp(GL_KEEP, GL_INCR, GL_INCR); // Test success adds 1
			}

			glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
			_render->_clip.use(clip.path);
			glDrawArrays(GL_TRIANGLES, 0, clip.path.count); // draw test
			if (clip.aa) {
				// draw anti alias alpha..
			}
			glStencilFunc(GL_LEQUAL, _stencilRef, 0xFFFFFFFF); // Equality passes the test
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep

			return true;
		}

		void fillPathv(const Pathv &path, const Paint &paint, bool aa) {
			if (path.vertex.length()) {
				Qk_ASSERT(path.path.isNormalized());
				fillv(path, paint);
				if (aa) {
					drawAAStrokeSDF(path.path, paint, aa_sdf_range, aa_sdf_width);
				}
			}
		}

		void fillPath(const Path &path, const Paint &paint, bool aa) {
			Qk_ASSERT(path.isNormalized());
			auto &vertex = _render->getPathTriangles(path);
			if (vertex.vao) {
				fillv(vertex, paint);
				if (aa) {
					drawAAStrokeSDF(path, paint, aa_sdf_range, aa_sdf_width);
				}
			}
		}

		void fillv(const VertexData &vertex, const Paint &paint) {
			// __m128 tt;
			switch (paint.type) {
				case Paint::kColor_Type:
					drawColor(vertex, paint, GL_TRIANGLES);
					//test_color_fill_aa_lines(_render->_color, path, paint);
					break;
				case Paint::kGradient_Type:
					drawGradient(vertex, paint, GL_TRIANGLES);
					break;
				case Paint::kBitmap_Type:
					drawImage(vertex, paint, GL_TRIANGLES);
					break;
				case Paint::kBitmapMask_Type:
					drawImageMask(vertex, paint, GL_TRIANGLES);
					break;
			}
		}

		void drawStroke(const Path &path, const Paint& paint, bool aa) {
			if (aa) {
				auto width = paint.width - _unitPixel * 0.45f;
				if (width > 0) {
					fillPath(_render->getStrokePath(path, width, paint.cap, paint.join,0), paint, true);
				} else {
					// 5*5=25, 0.75
					width /= (_unitPixel * 0.65f); // range: -1 => 0
					width = powf(width*10, 3) * 0.005; // (width*10)^3 * 0.006
					const float stroke_sdf_range[3] = {0.5, width/*-0.25f*/, 0};
					drawAAStrokeSDF(path, paint, stroke_sdf_range, 0.5);
				}
			} else {
				fillPath(_render->getStrokePath(path, paint.width, paint.cap, paint.join,0), paint, false);
			}
		}

		void drawAAStrokeSDF(const Path& path, const Paint& paint, const float sdf_range[3], float sdf_width) {
			//Path newPath(path); newPath.transfrom(Mat(1,0,170,0,1,0));
			//auto &strip = _render->getSDFStrokeTriangleStripCache(newPath, _Scale);
			// _UnitPixel*0.6=1.2/_Scale, 2.4px
			auto &strip = _render->getSDFStrokeTriangleStrip(path, _unitPixel*sdf_width);
			// Qk_DEBUG("%p", &strip);
			switch (paint.type) {
				case Paint::kColor_Type:
					drawColorSDF(strip, paint.color, GL_TRIANGLE_STRIP, sdf_range);
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

		void drawColor(const VertexData &vertex, const Paint &paint, GLenum mode) {
			_render->_color.use(vertex);
			glUniform4fv(_render->_color.color, 1, paint.color.val);
			glDrawArrays(mode, 0, vertex.count);
		}

		void drawGradient(const VertexData &vertex, const Paint &paint, GLenum mode) {
			auto g = paint.gradient;
			auto shader = paint.gradientType ==
				Paint::kRadial_GradientType ? &_render->_radial:
				static_cast<GLSLColorRadial*>(static_cast<GLSLShader*>(&_render->_linear));
			auto count = Qk_MIN(g->colors->length(), 256);

			shader->use(vertex);
			glUniform1f(shader->opacity, paint.color.a());
			glUniform4fv(shader->range, 1, g->origin.val);
			glUniform1i(shader->count, count);
			glUniform4fv(shader->colors, count, (const GLfloat*)g->colors->val());
			glUniform1fv(shader->positions, count, (const GLfloat*)g->positions->val());
			glDrawArrays(mode, 0, vertex.count);
			//glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex.length());
			//glDrawArrays(GL_LINES, 0, vertex.length());
		}

		void drawImage(const VertexData &vertex, const Paint &paint, GLenum mode) {
			auto shader = &_render->_image;
			auto pixel = paint.image;
			auto type = pixel->type();
			auto texCount = 1;

			if (type == kColor_Type_YUV420P_Y_8) {
				shader = static_cast<GLSLImage*>(static_cast<GLSLShader*>(&_render->_yuv420p));
				texCount = 3;
			} else if (type == kColor_Type_YUV420SP_Y_8) {
				shader = static_cast<GLSLImage*>(static_cast<GLSLShader*>(&_render->_yuv420sp));
				texCount = 2;
			}
			for (int i = 0; i < texCount; i++) {
				_render->setTexture(pixel + i, i, paint);
			}
			shader->use(vertex);
			glUniform1f(shader->opacity, paint.color.a());
			glUniform4fv(shader->coord, 1, paint.region.origin.val);
			glDrawArrays(mode, 0, vertex.count);
		}

		void drawImageMask(const VertexData &vertex, const Paint &paint, GLenum mode) {
			auto shader = &_render->_colorMask;
			_render->setTexture(paint.image, 0, paint);
			shader->use(vertex);
			glUniform4fv(shader->color, 1, paint.color.val);
			glUniform4fv(shader->coord, 1, paint.region.origin.val);
			glDrawArrays(mode, 0, vertex.count);
		}

		// ----------------------------------------------------------------------------------------

		void drawColorSDF(const VertexData &vertex, const Color4f &color, GLenum mode, const float range[3]) {
			_render->_colorSdf.use(vertex);
			glUniform1fv(_render->_colorSdf.sdf_range, 3, range);
			glUniform4fv(_render->_colorSdf.color, 1, color.val);
			glDrawArrays(mode, 0, vertex.count);
			// glDrawArrays(GL_LINE_STRIP, 0, vertex.length());
		}

		void drawGradientSDF(const VertexData &vertex, const Paint& paint, GLenum mode, const float range[3]) {
			auto g = paint.gradient;
			auto shader = paint.gradientType ==
				Paint::kRadial_GradientType ? &_render->_radialSdf:
				static_cast<GLSLColorRadialSdf*>(static_cast<GLSLShader*>(&_render->_linearSdf));
			auto count = Qk_MIN(g->colors->length(), 256);
			shader->use(vertex);
			glUniform1f(shader->opacity, paint.color.a());
			glUniform1fv(shader->sdf_range, 3, range);
			glUniform4fv(shader->range, 1, g->origin.val);
			glUniform1i(shader->count, count);
			glUniform4fv(shader->colors, count, (const GLfloat*)g->colors->val());
			glUniform1fv(shader->positions, count, (const GLfloat*)g->positions->val());
			glDrawArrays(mode, 0, vertex.count);
			//glDrawArrays(GL_LINE_STRIP, 0, vertex.length());
		}

		void drawImageSDF(const VertexData &vertex, const Paint& paint, GLenum mode, const float range[3]) {
			auto shader = &_render->_imageSdf;
			auto type = paint.image->type();
			if (type == kColor_Type_YUV420P_Y_8 || type == kColor_Type_YUV420SP_Y_8) {
				return; // ignore
			}
			//setTexturePixel(paint.image, 0, paint);
			shader->use(vertex);
			glUniform1fv(shader->sdf_range, 3, range);
			glUniform1f(shader->opacity, paint.color.a());
			glUniform4fv(shader->coord, 1, paint.region.origin.val);
			glDrawArrays(mode, 0, vertex.count);
		}

		void drawImageMaskSDF(const VertexData &vertex, const Paint& paint, GLenum mode, const float range[3]) {
			auto shader = &_render->_colorMaskSdf;
			//setTexturePixel(paint.image, 0, paint);
			shader->use(vertex);
			glUniform1fv(shader->sdf_range, 3, range);
			glUniform4fv(shader->color, 1, paint.color.val);
			glUniform4fv(shader->coord, 1, paint.region.origin.val);
			glDrawArrays(mode, 0, vertex.count);
		}

		float drawTextImage(ImageSource *textImg, float imgTop, float scale, Vec2 origin, const Paint &paint) {
			auto &pix = textImg->pixels().front();
			auto scale_1 = 1.0 / scale;
			Paint p(paint);

			// default use baseline align
			Vec2 dst_start(origin.x(), origin.y() - imgTop * scale_1);
			Vec2 dst_size(pix.width() * scale_1, pix.height() * scale_1);

			p.setImage(&pix, {dst_start, dst_size});

			// Vec2 v1(dst_start.x() + dst_size.x(), dst_start.y());
			// Vec2 v2(dst_start.x(), dst_start.y() + dst_size.y());
			// Vec2 v3(dst_start + dst_size);

			// Array<Vec2> vertex{
			// 	// triangle 0
			// 	dst_start,
			// 	v1,
			// 	v2,
			// 	// triangle 1
			// 	v2,
			// 	v3,
			// 	v1,
			// };

			drawImageMask(_render->getRectPath({dst_start, dst_size}), p, GL_TRIANGLES);

			return scale_1;
		}
	};

	// ----------------------------------------------------------------------------------------

	GLCanvas::GLCanvas(GLRender *render)
		: _render(render)
		, _stencilRef(0), _stencilRefDecr(0)
		, _state(nullptr)
		, _surfaceScale(1), _transfromScale(1), _scale(1)
	{
		glGenBuffers(1, &_matUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, _matUbo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 32, NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _matUbo);

		_stateStack.push({ .matrix=Mat() }); // init state
		_state = &_stateStack.back();

		setMatrix(_state->matrix); // init shader matrix
	}

	GLCanvas::~GLCanvas() {
		glDeleteBuffers(1, &_matUbo);
	}

	int GLCanvas::save() {
		_stateStack.push({ _stateStack.back().matrix });
		_state = &_stateStack.back();
	}

	void GLCanvas::restore(uint32_t count) {
		if (!count || _stateStack.length() == 1)
			return;

		int clipOp = -1;
		count = Uint32::min(count, _stateStack.length() - 1);

		while (count > 0) {
			auto &state = _stateStack.back();
			for (auto &clip: state.clips) {
				// restore clip
				if (clipOp == -1)
					glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);

				if (clip.op == kDifference_ClipOp) {
					_stencilRefDecr++;
					Qk_ASSERT(_stencilRefDecr <= 127);
					if (clipOp != kDifference_ClipOp) {
						clipOp = kDifference_ClipOp;
						glStencilOp(GL_KEEP, GL_INCR, GL_INCR); // Test success adds 1
					}
				} else {
					_stencilRef--;
					Qk_ASSERT(_stencilRef >= 127);
					if (clipOp != kIntersect_ClipOp) {
						clipOp = kIntersect_ClipOp;
						glStencilOp(GL_KEEP, GL_DECR, GL_DECR); // Test success decr 1
					}
				}
				_render->_clip.use(clip.path);
				glDrawArrays(GL_TRIANGLES, 0, clip.path.count); // draw test
				_render->deleteVertexData(clip.path);
			}
			_state = &_stateStack.pop().back();
			_this->setMatrixBuffer(_state->matrix);
			count--;
		}

		if (!_this->isStencilTest()) { // not stencil test
			glDisable(GL_STENCIL_TEST); // disable stencil test
		}
		else if (clipOp != -1) {
			glStencilFunc(GL_LEQUAL, _stencilRef, 0xFFFFFFFF); // Equality passes the test
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep
		}
	}

	int GLCanvas::getSaveCount() const {
		return _stateStack.length() - 1;
	}

	const Mat& GLCanvas::getMatrix() const {
		return _state->matrix;
	}

	void GLCanvas::setRootMatrix(const Mat4& root, Vec2 surfaceScale) {
		// update all shader root matrix
		auto m4x4 = root.transpose(); // transpose matrix
		glBindBuffer(GL_UNIFORM_BUFFER, _matUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, m4x4.val);
		glClear(GL_STENCIL_BUFFER_BIT); // clear stencil buffer

		if (_this->isStencilTest()) { // current is have clips, restore clip stencil
			_stencilRef = _stencilRefDecr = 127;
			for (int i = 0; i < _stateStack.length(); i++) {
				_state = &_stateStack[i];
				_this->setMatrixBuffer(_state->matrix);
				for (auto &clip: _state->clips) {
					_this->drawClip(clip);
				}
			}
		}
		_surfaceScale = Float::max(surfaceScale[0], surfaceScale[1]);
		_transfromScale = Float::max(_state->matrix[0], _state->matrix[4]);
		_scale = _surfaceScale * _transfromScale;
		_unitPixel = 2 / _scale;
	}

	void GLCanvas::setMatrix(const Mat& mat) {
		_state->matrix = mat;
		_this->setMatrixBuffer(mat);
	}

	void GLCanvas::translate(float x, float y) {
		_state->matrix.translate({x, y});
		_this->setMatrixBuffer(_state->matrix);
	}

	void GLCanvas::scale(float x, float y) {
		_state->matrix.scale({x, y});
		_this->setMatrixBuffer(_state->matrix);
	}

	void GLCanvas::rotate(float z) {
		_state->matrix.rotate(z);
		_this->setMatrixBuffer(_state->matrix);
	}

	bool GLCanvas::readPixels(Pixel* dst, uint32_t srcX, uint32_t srcY) {
		return gl_read_pixels(dst, srcX, srcY);
	}

	void GLCanvas::clipPath(const Path& path, ClipOp op, bool antiAlias) {
		_this->clipv(path, _render->getPathTriangles(path), op, antiAlias);
	}

	void GLCanvas::clipPathv(const Pathv& path, ClipOp op, bool antiAlias) {
		_this->clipv(path.path, path, op, antiAlias);
	}

	void GLCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		auto& path = _render->getRectPath(rect);
		_this->clipv(path.path, path, op, antiAlias);
	}

	void GLCanvas::clearColor(const Color4f& color) {
		glClearColor(color.r(), color.g(), color.b(), color.a());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void GLCanvas::drawColor(const Color4f &color, BlendMode mode) {
		if (mode == kSrc_BlendMode) {
			clearColor(color);
		} else {
			float data[] = {
				-1,1,  1,1,
				-1,-1, 1,-1,
			};
			_render->setBlendMode(mode); // switch blend mode
			_render->_clear.use(sizeof(float) * 8, data);
			glUniform4fv(_render->_clear.color, 1, color.val);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	void GLCanvas::drawRect(const Rect& rect, const Paint& paint) {
		drawPathv(_render->getRectPath(rect), paint);
	}

	void GLCanvas::drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) {
		drawPathv(_render->getRRectPath(rect,radius), paint);
	}

	void GLCanvas::drawPathvColor(const Pathv& path, const Color4f &color, BlendMode mode) {
		if (path.vertex.length()) {
			bool aa = !_render->_IsDeviceMsaa; // Anti-aliasing using software
			_render->setBlendMode(mode); // switch blend mode
			_render->_color.use(path);
			//auto color4f = color.to_color4f_alpha(alpha);
			glUniform4fv(_render->_color.color, 1, color.val);
			glDrawArrays(GL_TRIANGLES, 0, path.vertex.length());
			//glDrawArrays(GL_LINES, 0, path.vertex.length());
			if (aa) {
				auto &strip = _render->getSDFStrokeTriangleStrip(path.path, _unitPixel*aa_sdf_width);
				//_render->setBlendMode(kSrc_BlendMode); // switch blend mode
				_this->drawColorSDF(strip, color, GL_TRIANGLE_STRIP, aa_sdf_range);
			}
		}
	}

	void GLCanvas::drawPath(const Path &path_, const Paint &paint) {
		_render->setBlendMode(paint.blendMode); // switch blend mode

		bool aa = paint.antiAlias && !_render->_IsDeviceMsaa; // Anti-aliasing using software
		auto path = &_render->getNormalizedPath(path_);

		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				_this->fillPath(*path, paint, aa);
				break;
			case Paint::kStrokeAndFill_Style:
				_this->fillPath(*path, paint, aa);
			case Paint::kStroke_Style: {
				_this->drawStroke(*path, paint, aa);
				break;
			}
		}
	}

	void GLCanvas::drawPathv(const Pathv& path, const Paint& paint) {
		_render->setBlendMode(paint.blendMode); // switch blend mode

		bool aa = paint.antiAlias && !_render->_IsDeviceMsaa; // Anti-aliasing using software
		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				_this->fillPathv(path, paint, aa);
				break;
			case Paint::kStrokeAndFill_Style:
				_this->fillPathv(path, paint, aa);
			case Paint::kStroke_Style: {
				_this->drawStroke(path.path, paint, aa);
				break;
			}
		}
	}

	float GLCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint)
	{
		_render->setBlendMode(paint.blendMode); // switch blend mode

		Sp<ImageSource> img;
		auto tf = glyphs.typeface();
		auto bound = tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _scale, offset, &img);
		img->mark_as_texture_unsafe(_render);
		auto scale_1 = _this->drawTextImage(*img, bound.y(), _scale, origin, paint);
		return scale_1 * bound.x();
	}

	void GLCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		_render->setBlendMode(paint.blendMode); // switch blend mode

		fontSize *= _transfromScale;
		auto levelSize = get_level_font_size(fontSize);
		auto levelScale = fontSize / levelSize;
		auto imageFontSize = levelSize * _surfaceScale;

		if (imageFontSize == 0.0)
			return;

		if (blob->imageFontSize != imageFontSize || !blob->image) { // fill text bolb
			auto tf = blob->typeface;
			auto offset = blob->offset.length() == blob->glyphs.length() ? &blob->offset: NULL;
			blob->imageBound = tf->getImage(blob->glyphs,imageFontSize, offset, &blob->image);
			blob->image->mark_as_texture_unsafe(_render);
		}

		_this->drawTextImage(*blob->image, blob->imageBound.y(), _scale * levelScale, origin, paint);
	}

}
