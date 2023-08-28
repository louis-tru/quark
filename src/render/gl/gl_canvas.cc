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

	const    static Region ZeroRegion;
	constexpr float aa_fuzz_weight = 0.9;
	constexpr float aa_fuzz_width = 0.5;

	GLC_ImageCmd::~GLC_ImageCmd() {
		paint.source->release();
	}

	GLC_GenericeCmd::~GLC_GenericeCmd() {
		for (auto i = 0; i < images; i++) {
			(&image)[i].source->release();
		}
	}

	GLC_CmdPack::GLC_CmdPack(): gc_vertexsIdx(0), gc_optsIdx(0) {
		cmdSize = sizeof(GLC_Cmd);
		cmdCapacity = 1024;
		cmd = (GLC_Cmd*)malloc(1024);
		cmd->size = cmdSize;
		gc_vertexs.push({(Vec3*)malloc(65535 * sizeof(Vec3)),0,65535});
		gc_optidxs.push({(int*)malloc(65535 * sizeof(int)),0,65535});
		gc_opts.push({(GCOpt*)malloc(1024 * sizeof(GCOpt)),0,1024});
	}

	GLC_CmdPack::~GLC_CmdPack() {
		for (auto &i: gc_vertexs) free(i.val);
		for (auto &i: gc_optidxs) free(i.val);
		for (auto &i: gc_opts) free(i.val);
		free(cmd);
	}

	GLC_Cmd* GLC_CmdPack::allocCmd(uint32_t size) {
		auto newSize = cmdSize + size;
		if (newSize > cmdCapacity) {
			cmdCapacity <<= 1;
			cmd = (GLC_Cmd*)realloc(cmd, cmdCapacity);
		}
		GLC_Cmd* newCmd = (GLC_Cmd*)(((char*)cmd) + cmdSize);
		newCmd->size = size;
		cmdSize = newSize;
		return newCmd;
	}

	GLC_GenericeCmd* GLC_CmdPack::newGenericeCmd() {
		// TODO ...
	}

	Qk_DEFINE_INLINE_MEMBERS(GLCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLCanvas::Inl*>(self)

		bool isStencilTest() {
			return _stencilRef != 127 || _stencilRefDecr != 127;
		}

		float zDepth() {
			return _render->_zDepth;
		}

		void zDepthNext() {
			_render->_zDepth += 0.000000125f; // 1/8000000
		}

		void setMatrixBuffer(const Mat& mat) {
			const float m4x4[16] = {
				mat[0], mat[3], 0.0, 0.0,
				mat[1], mat[4], 0.0, 0.0,
				0.0,    0.0,    1.0, 0.0,
				mat[2], mat[5], 0.0, 1.0
			}; // transpose matrix
			glBindBuffer(GL_UNIFORM_BUFFER, _render->_matrixBlock);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, m4x4);

			auto mScale = Float::max(_state->matrix[0], _state->matrix[4]);
			if (_transfromScale != mScale) {
				_transfromScale = mScale;
				_scale = _surfaceScale * _transfromScale;
				_unitPixel = 2 / _scale;
			}
		}

		void clipv(const Path &path, const Array<Vec3> &vertex, ClipOp op, bool antiAlias) {
			if (!isStencilTest()) {
				glEnable(GL_STENCIL_TEST); // enable stencil test
			}
			GLC_State::Clip clip{
				.op=op,
				.aa=antiAlias&&!_render->_IsDeviceMsaa,
				.path={vertex,path}
			};
			if (drawClip(clip)) { // save clip state
				_state->clips.push(std::move(clip));
			}
		}

		bool drawClip(GLC_State::Clip &clip) {
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

			if (clip.aa) { // aa
				// draw anti alias alpha
				// drawAAStrokeSDF(path.path, paint, aa_sdf_range, aa_sdf_width);
			} else {
				_render->_clip.use(clip.path.vertex.size(), clip.path.vertex.val());
				glDrawArrays(GL_TRIANGLES, 0, clip.path.vertex.length()); // draw test
			}
			glStencilFunc(GL_LEQUAL, _stencilRef, 0xFFFFFFFF); // Equality passes the test
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep

			return true;
		}

		void fillPathv(const Pathv &path, const Paint &paint, bool aa) {
			if (path.vertex.length()) {
				Qk_ASSERT(path.path.isNormalized());
				fillv(path.vertex, paint);
				if (aa) {
					drawAAFuzzStroke(path.path, paint, aa_fuzz_weight, aa_fuzz_width);
				}
				_render->_zDepth++;
			}
		}

		void fillPath(const Path &path, const Paint &paint, bool aa) {
			Qk_ASSERT(path.isNormalized());
			auto &vertex = _render->getPathTriangles(path);
			if (vertex.length()) {
				fillv(vertex, paint);
				if (aa) {
					drawAAFuzzStroke(path, paint, aa_fuzz_weight, aa_fuzz_width);
				}
			}
			_render->_zDepth++;
		}

		void fillv(const Array<Vec3> &vertex, const Paint &paint) {
			switch (paint.type) {
				case Paint::kColor_Type:
					drawColor(vertex, paint.color); break;
				case Paint::kGradient_Type:
					drawGradient(vertex, paint, paint.color.a()); break;
				case Paint::kBitmap_Type:
					drawImage(vertex, paint, paint.color.a()); break;
				case Paint::kBitmapMask_Type:
					drawImageMask(vertex, paint, 1); break;
			}
		}

		void strokePath(const Path &path, const Paint& paint, bool aa) {
			if (aa) {
				auto width = paint.width - _unitPixel * 0.45f;
				if (width > 0) {
					fillPath(_render->getStrokePath(path, width, paint.cap, paint.join,0), paint, true);
				} else {
					width /= (_unitPixel * 0.65f); // range: -1 => 0
					width = powf(width*10, 3) * 0.005; // (width*10)^3 * 0.006
					drawAAFuzzStroke(path, paint, 0.5 / (0.5 - width), 0.5);
					_render->_zDepth++;
				}
			} else {
				fillPath(_render->getStrokePath(path, paint.width, paint.cap, paint.join,0), paint, false);
			}
		}

		void drawAAFuzzStroke(const Path& path, const Paint& paint, float aaFuzzWeight, float aaFuzzWidth) {
			//Path newPath(path); newPath.transfrom(Mat(1,0,170,0,1,0));
			//auto &vertex = _render->getSDFStrokeTriangleStripCache(newPath, _Scale);
			// _UnitPixel*0.6=1.2/_Scale, 2.4px
			auto &vertex = _render->getAAFuzzStrokeTriangle(path, _unitPixel*aaFuzzWidth);
			// Qk_DEBUG("%p", &vertex);
			switch (paint.type) {
				case Paint::kColor_Type:
					drawColor(vertex, paint.color.to_color4f_alpha(aaFuzzWeight)); break;
				case Paint::kGradient_Type:
					drawGradient(vertex, paint, aaFuzzWeight * paint.color.a()); break;
				case Paint::kBitmap_Type:
					drawImage(vertex, paint, aaFuzzWeight * paint.color.a()); break;
				case Paint::kBitmapMask_Type:
					drawImageMask(vertex, paint, aaFuzzWeight); break;
			}
		}

		float drawTextImage(ImageSource *textImg, float imgTop, float scale, Vec2 origin, const Paint &paint) {
			auto &pix = textImg->pixels().front();
			auto scale_1 = 1.0 / scale;
			ImagePaint p;
			// default use baseline align
			Vec2 dst_start(origin.x(), origin.y() - imgTop * scale_1);
			Vec2 dst_size(pix.width() * scale_1, pix.height() * scale_1);

			p.setImage(textImg, {dst_start, dst_size});

			Vec2 v1(dst_start.x() + dst_size.x(), dst_start.y());
			Vec2 v2(dst_start.x(), dst_start.y() + dst_size.y());
			Vec2 v3(dst_start + dst_size);
			Array<Vec3> vertex{
				dst_start, v1, v2, // triangle 0
				v2, v3, v1, // triangle 1
			};
			// _render->getRectPath({dst_start, dst_size}).vertex;
			
			Paint p0(paint);
			p0.image = &p;

			drawImageMask(vertex, p0, 1);
			_render->_zDepth++;

			return scale_1;
		}

		// ----------------------------------------------------------------------------------------

		GLC_GenericeCmd* newGenericeCmd() {
			auto mem = malloc(sizeof(GLC_GenericeCmd) +
				sizeof(ImagePaintBase) * (_render->_maxTextureImageUnits - 1)
			);
			auto cmd = new(mem) GLC_GenericeCmd;
			cmd->options.extend(128);
			cmd->subcmd = 0;
			cmd->images = 0;
			cmd->type = kGenerice_GLC_CmdType;
			// cmd->depth = 0;
			cmd->next = nullptr;
			_cmdEnd->next = cmd;
			_cmdEnd = cmd;
			return cmd;
		}

		GLC_GenericeCmd* getGenericeCmd() {
			if (_cmdEnd->type == kGenerice_GLC_CmdType) {
				auto cmd = static_cast<GLC_GenericeCmd*>(_cmdEnd);
				if (cmd->subcmd == cmd->options.length()) {
					if (cmd->subcmd == 1024) {
						return newGenericeCmd();
					} else {
						cmd->options.extend(cmd->subcmd + 128);
					}
				}
				return cmd;
			}
			return newGenericeCmd();
		}

		int addGenericeSubcmd(GLC_GenericeCmd* cmd, int flags,
			const Array<Vec3> &vertex, const Color4f &color, const Region &coord
		) {
			// add new subcmd
			auto optinx = cmd->subcmd++;
			auto vertexLen = vertex.length();
 			// setting vertex option data
			cmd->options[optinx] = {
				.flags  = flags,           .depth = zDepth(),
				.matrix = _state->matrix,  .color = color,
				.coord  = coord,
			};
			cmd->vertex.write(*vertex, -1, vertexLen); // copy vertex data
			cmd->optidxs.extend(cmd->vertex.length());
			// set vertex option index
			memset_pattern4(&cmd->optidxs.back() - vertexLen, &optinx, vertexLen);
			return optinx;
		}

		inline void drawColor(const Array<Vec3> &vertex, const Color4f &color) {
			addGenericeSubcmd(getGenericeCmd(), GLC_GenericeCmd::kColor, vertex, color, ZeroRegion);
		}

		void drawImage(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			auto img = paint.image;
			auto type = img->source->type();
			auto texCount = type == kColor_Type_YUV420P_Y_8 ? 3:
											type == kColor_Type_YUV420SP_Y_8 ? 2: 1;
			if (texCount == 1) { // use generice cmd
				auto cmd = getGenericeCmd();
				if (cmd->images == _render->_maxTextureImageUnits)
					cmd = newGenericeCmd();
				auto optinx = addGenericeSubcmd(
					cmd, GLC_GenericeCmd::kImage, vertex, Color4f(0,0,0,alpha), paint.image->coord
				);
				auto cmd = static_cast<GLC_GenericeCmd*>(_cmdEnd);
				(&cmd->image)[cmd->images++] = *paint.image;
				cmd->options[optinx].flags |= cmd->images << 2;
				paint.image->source->retain(); // retain source image ref
			} else {
				// TODO check matrix change ..
				auto cmd = new(_cmdPack->allocCmd(sizeof(GLC_ImageCmd))) GLC_ImageCmd;
				cmd->type = kImage_GLC_CmdType;
				cmd->alpha = paint.color.a();
				cmd->format = GLC_ImageCmd::Format(texCount - 1);
				cmd->paint = *img;
				img->source->retain(); // retain source image ref
			}
		}

		void drawImageMask(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			auto cmd = getGenericeCmd();
			if (cmd->images == _render->_maxTextureImageUnits)
				cmd = newGenericeCmd();
			auto color = paint.color.to_color4f_alpha(alpha);
			auto optinx = addGenericeSubcmd(
				cmd, GLC_GenericeCmd::kColorMask, vertex, color, paint.image->coord
			);
			auto cmd = static_cast<GLC_GenericeCmd*>(_cmdEnd);
			(&cmd->image)[cmd->images++] = *paint.image;
			cmd->options[optinx].flags |= cmd->images << 2;
			paint.image->source->retain(); // retain source image ref
		}

		void drawGradient(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			// TODO check matrix change ..
			auto g = paint.gradient;
			auto colors = sizeof(Color4f) * g->count;
			auto positions = sizeof(float) * g->count;
			auto cmdSize = sizeof(GLC_GradientCmd);
			auto cmd = new(_cmdPack->allocCmd(cmdSize + colors + positions)) GLC_GradientCmd;
			auto cmdp = (char*)cmd;
			auto colors_p = reinterpret_cast<Color4f*>(cmdp + cmdSize);
			auto positions_p = reinterpret_cast<float*>(cmdp + cmdSize + colors);
			memcpy(colors_p, g->colors, colors); // copy colors
			memcpy(positions_p, g->positions, positions); // copy positions
			cmd->type = kGenerice_GLC_CmdType;
			cmd->vertex.write(*vertex, -1, vertex.length());
			cmd->depth = zDepth();
			cmd->alpha = alpha;
			cmd->paint = *g;
			cmd->paint.colors = colors_p;
			cmd->paint.positions = positions_p;
		}

	};

	// ----------------------------------------------------------------------------------------

	GLCanvas::GLCanvas(GLRender *render)
		: _render(render)
		, _stencilRef(0), _stencilRefDecr(0)
		, _state(nullptr)
		, _surfaceScale(1), _transfromScale(1), _scale(1)
	{
		_cmdPack = _cmdPacks;
		_stateStack.push({ .matrix=Mat() }); // init state
		_state = &_stateStack.back();
		setMatrix(_state->matrix); // init shader matrix
	}

	GLCanvas::~GLCanvas() {
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
				_render->_clip.use(clip.path.vertex.size(), clip.path.vertex.val());
				glDrawArrays(GL_TRIANGLES, 0, clip.path.vertex.length()); // draw test
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
		// update shader root matrix and clear all save state
		auto m4x4 = root.transpose(); // transpose matrix
		glBindBuffer(GL_UNIFORM_BUFFER, _render->_matrixBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, m4x4.val);
		glClear(GL_STENCIL_BUFFER_BIT); // clear stencil buffer
		glDisable(GL_STENCIL_TEST); // disable stencil test

		if (!_render->_IsDeviceMsaa) { // clear clip aa buffer
			const float color[] = {0.0f,0.0f,0.0f,0.0f};
			glClearBufferfv(GL_COLOR, 1, color); // clear GL_COLOR_ATTACHMENT1
		}
		_stateStack.clear();
		_stencilRef = _stencilRefDecr = 127; // clear clip state
		_stateStack.push({ .matrix=Mat() }); // init state
		_state = &_stateStack.back();
		_this->setMatrixBuffer(_state->matrix); // init shader matrix

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
		_this->clipv(path.path, path.vertex, op, antiAlias);
	}

	void GLCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		auto &path = _render->getRectPath(rect);
		_this->clipv(path.path, path.vertex, op, antiAlias);
	}

	void GLCanvas::clearColor(const Color4f& color) {
		const float depth = 0.0f;
		glClearBufferfv(GL_DEPTH, 0, &depth); // clear GL_COLOR_ATTACHMENT0
		glClearBufferfv(GL_COLOR, 0, color.val);
		// glClearColor(color.r(), color.g(), color.b(), color.a());
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// drawColor(color, kSrcOver_BlendMode);
		_render->_zDepth = 0;
	}

	void GLCanvas::drawColor(const Color4f &color, BlendMode mode) {
		if (mode == kSrc_BlendMode) {
			clearColor(color);
		} else {
			float data[] = {
				-1,1,/*left top*/1,1,/*right top*/
				-1,-1, /*left bottom*/1,-1, /*right bottom*/
			};
			_render->setBlendMode(mode); // switch blend mode
			_render->_clear.use(sizeof(float) * 8, data);
			glUniform1f(_render->_color.depth, _this->zDepth());
			glUniform4fv(_render->_clear.color, 1, color.val);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			_render->_zDepth++;
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
			_render->setBlendMode(mode); // switch blend mode
			_this->drawColor(path.vertex, color);
			if (!_render->_IsDeviceMsaa) { // Anti-aliasing using software
				auto &vertex = _render->getAAFuzzStrokeTriangle(path.path, _unitPixel*aa_fuzz_width);
				_this->drawColor(vertex, color.to_color4f_alpha(aa_fuzz_weight));
			}
			_render->_zDepth++;
		}
	}

	void GLCanvas::drawPath(const Path &path_, const Paint &paint) {
		_render->setBlendMode(paint.blendMode); // switch blend mode

		bool aa = paint.antiAlias && !_render->_IsDeviceMsaa; // Anti-aliasing using software
		auto path = &_render->getNormalizedPath(path_);

		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				_this->fillPath(*path, paint, aa); break;
			case Paint::kStrokeAndFill_Style:
				_this->fillPath(*path, paint, aa);
			case Paint::kStroke_Style: {
				_this->strokePath(*path, paint, aa); break;
			}
		}
	}

	void GLCanvas::drawPathv(const Pathv& path, const Paint& paint) {
		_render->setBlendMode(paint.blendMode); // switch blend mode

		bool aa = paint.antiAlias && !_render->_IsDeviceMsaa; // Anti-aliasing using software
		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				_this->fillPathv(path, paint, aa); break;
			case Paint::kStrokeAndFill_Style:
				_this->fillPathv(path, paint, aa);
			case Paint::kStroke_Style: {
				_this->strokePath(path.path, paint, aa); break;
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
