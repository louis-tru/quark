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

#define Qk_GL_USE_CMD_PACK 1
#define Qk_GCmdOptionCapacity 1024
#define Qk_GCmdVertexsMemBlockCapacity 65535
#define Qk_GCmdOptMemBlockCapacity 16384

namespace qk {

	float get_level_font_size(float fontSize);
	bool  gl_read_pixels(Pixel* dst, uint32_t srcX, uint32_t srcY);

	extern uint32_t GL_MaxTextureImageUnits;
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

	GLC_CmdPack::GLC_CmdPack()
	{
		cmd.size = sizeof(GLC_Cmd);
		cmd.capacity = 65536;
		cmd.val = (GLC_Cmd*)malloc(65536);
		cmd.val->size = cmd.size;
		cmd.val->type = kEmpty_GLC_CmdType;
		lastCmd = cmd.val;
		vertexsBlocks.blocks.push({
			(Vec4*)malloc(Qk_GCmdVertexsMemBlockCapacity * sizeof(Vec4)),0,Qk_GCmdVertexsMemBlockCapacity
		});
		vertexsBlocks.current = vertexsBlocks.blocks.val();
		vertexsBlocks.index = 0;
		// optidxsBlocks.blocks.push({
		// 	(int*)malloc(Qk_GCmdVertexsMemBlockCapacity * sizeof(int)),0,Qk_GCmdVertexsMemBlockCapacity
		// });
		// optidxsBlocks.current = optidxsBlocks.blocks.val();
		// optidxsBlocks.index = 0;
		optsBlocks.blocks.push({
			(GCOpt*)malloc(Qk_GCmdOptMemBlockCapacity * sizeof(GCOpt)),0,Qk_GCmdOptMemBlockCapacity
		});
		optsBlocks.current = optsBlocks.blocks.val();
		optsBlocks.index = 0;
	}

	GLC_CmdPack::~GLC_CmdPack() {
		for (auto &i: vertexsBlocks.blocks) free(i.val);
		// for (auto &i: optidxsBlocks.blocks) free(i.val);
		for (auto &i: optsBlocks.blocks) free(i.val);
		free(cmd.val);
	}

	GLC_Cmd* GLC_CmdPack::allocCmd(uint32_t size) {
		auto newSize = cmd.size + size;
		if (newSize > cmd.capacity) {
			cmd.capacity <<= 1;
			cmd.val = (GLC_Cmd*)realloc(cmd.val, cmd.capacity);
		}
		lastCmd = (GLC_Cmd*)(((char*)cmd.val) + cmd.size);
		lastCmd->size = size;
		cmd.size = newSize;
		return lastCmd;
	}

	GLC_GenericeCmd* GLC_CmdPack::newGenericeCmd() {
		auto cmd = (GLC_GenericeCmd*)allocCmd(
			sizeof(GLC_GenericeCmd) + sizeof(ImagePaintBase) * (GL_MaxTextureImageUnits - 1)
		);
		cmd->type = kGenerice_GLC_CmdType;

		auto vertexs = vertexsBlocks.current;
		// auto optidxs = optidxsBlocks.current;
		auto opts    = optsBlocks.current;

		if (vertexs->size == Qk_GCmdVertexsMemBlockCapacity) {
			if (++vertexsBlocks.index == vertexsBlocks.blocks.length()) {
				vertexsBlocks.blocks.push({
					(Vec4*)malloc(Qk_GCmdVertexsMemBlockCapacity * sizeof(Vec4)),0,Qk_GCmdVertexsMemBlockCapacity
				});
				// optidxsBlocks.blocks.push({
				// 	(int*)malloc(Qk_GCmdVertexsMemBlockCapacity * sizeof(int)),0,Qk_GCmdVertexsMemBlockCapacity
				// });
			}
			vertexsBlocks.current = vertexs = vertexsBlocks.blocks.val() + vertexsBlocks.index;
			// optidxsBlocks.current = optidxs = optidxsBlocks.blocks.val() + vertexsBlocks.index;
		}

		if (opts->size == Qk_GCmdOptMemBlockCapacity) {
			if (++optsBlocks.index == optsBlocks.blocks.length()) {
				optsBlocks.blocks.push({
					(GCOpt*)malloc(Qk_GCmdOptMemBlockCapacity * sizeof(GCOpt)),0,Qk_GCmdOptMemBlockCapacity
				});
			}
			optsBlocks.current = opts = optsBlocks.blocks.val() + optsBlocks.index;
		}

		cmd->vertexs = vertexs->val + vertexs->size;
		// cmd->optidxs = optidxs->val + vertexs->size;
		cmd->opts    = opts->val    + opts->size;
		cmd->subcmd = 0;
		cmd->images = 0;

		return cmd;
	}

	Qk_DEFINE_INLINE_MEMBERS(GLCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLCanvas::Inl*>(self)

		void zDepthNext() {
			_zDepth += 0.000000125f; // 1/8000000
		}

		void setSurfaceScale(const Mat& mat) {
			auto mScale = Float::max(_state->matrix[0], _state->matrix[4]);
			if (_transfromScale != mScale) {
				_transfromScale = mScale;
				_scale = _surfaceScale * _transfromScale;
				_unitPixel = 2 / _scale;
			}
		}

		void clipv(const Path &path, const Array<Vec3> &vertex, ClipOp op, bool antiAlias) {
			GLC_State::Clip clip{
				.op=op,
				.aa=antiAlias&&!_render->_IsDeviceMsaa,
				.path={vertex,path}
			};
			drawClip(clip, false);
			_state->clips.push(std::move(clip));
		}

		void fillPathv(const Pathv &path, const Paint &paint, bool aa) {
			if (path.vertex.length()) {
				Qk_ASSERT(path.path.isNormalized());
				fillv(path.vertex, paint);
				if (aa) {
					drawAAFuzzStroke(path.path, paint, aa_fuzz_weight, aa_fuzz_width);
				}
				zDepthNext();
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
			zDepthNext();
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
					zDepthNext();
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
			
			Paint p0(paint);
			p0.image = &p;

			drawImageMask(vertex, p0, 1);
			zDepthNext();

			return scale_1;
		}

		// ----------------------------------------------------------------------------------------

#if Qk_GL_USE_CMD_PACK

		void setMetrixCmd() {
			if (_chMatrix) {
				auto cmd = (GLC_MatrixCmd*)_cmdPack->allocCmd(sizeof(GLC_MatrixCmd));
				cmd->type = kMatrix_GLC_CmdType;
				cmd->matrix = _state->matrix;
				_chMatrix = false;
			}
		}

		void setBlendMode(BlendMode mode) {
			if (_blendMode != mode) {
				auto cmd = (GLC_BlendCmd*)_cmdPack->allocCmd(sizeof(GLC_BlendCmd));
				cmd->type = kBlend_GLC_CmdType;
				cmd->mode = mode;
				_blendMode = mode;
			}
		}

		GLC_GenericeCmd* getGenericeCmd() {
			auto cmd = (GLC_GenericeCmd*)_cmdPack->lastCmd;
			if (cmd->type == kGenerice_GLC_CmdType) {
				if (_cmdPack->vertexsBlocks.current->size != Qk_GCmdVertexsMemBlockCapacity &&
						_cmdPack->optsBlocks.current->size    != Qk_GCmdOptMemBlockCapacity &&
						cmd->subcmd != Qk_GCmdOptionCapacity
				) {
					return cmd;
				}
			}
			return _cmdPack->newGenericeCmd();
		}

		int32_t addGenericeSubcmd(GLC_GenericeCmd* cmd, int flags,
			const Vec3 *&vertex, int32_t vertexLen, const Color4f &color, const Region &coord
		) {
 			// setting vertex option data
			cmd->opts[cmd->subcmd] = {
				.flags  = flags,           .depth = _zDepth,
				.matrix = _state->matrix,  .color = color,
				.coord  = coord,
			};
			auto vertexs = _cmdPack->vertexsBlocks.current;
			auto prevSize = vertexs->size;
			int  cpLen = Qk_GCmdVertexsMemBlockCapacity - prevSize;
			auto cpSrc = vertex;
			int32_t retv = 0;

			_cmdPack->optsBlocks.current->size++;

			if (cpLen < vertexLen) { // not enough space
				vertexs->size = Qk_GCmdVertexsMemBlockCapacity;
				vertex += cpLen;
				retv = vertexLen - cpLen;
			} else {
				vertexs->size = prevSize + vertexLen;
				cpLen = vertexLen;
			}

			const float subcmd = *((float*)&cmd->subcmd);
			auto p = vertexs->val + prevSize;
			auto p_1 = p + cpLen;

			// memcpy(p, cpSrc, sizeof(Vec4) * cpLen);

			// copy vertex data
			while (p < p_1) {
#if DEBUG
				*p = *((Vec4*)(cpSrc));
				p->val[3] = subcmd;
				p++,cpSrc++;
#else
				*p = {cpSrc->val[0],cpSrc->val[1],cpSrc->val[2],subcmd};
				p++,cpSrc++;
				*p = {cpSrc->val[0],cpSrc->val[1],cpSrc->val[2],subcmd};
				p++,cpSrc++;
				*p = {cpSrc->val[0],cpSrc->val[1],cpSrc->val[2],subcmd};
				p++,cpSrc++;
#endif
			}

			return retv;
		}

		void drawColor(const Array<Vec3> &vertex, const Color4f &color) {
			auto vertexp = vertex.val();
			auto vertexLen = vertex.length();
			do {
				auto cmd = getGenericeCmd();
				vertexLen = addGenericeSubcmd(cmd,
					GLC_GenericeCmd::kColor, vertexp, vertexLen, color, ZeroRegion
				);
				cmd->subcmd++;
			} while(vertexLen);
		}

		void drawImage(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			auto img = paint.image;
			auto type = img->source->type();
			auto texCount = type == kColor_Type_YUV420P_Y_8 ? 3:
											type == kColor_Type_YUV420SP_Y_8 ? 2: 1;
			if (texCount == 1) { // use generice cmd
				auto vertexp = vertex.val();
				auto vertexLen = vertex.length();
				do {
					auto cmd = getGenericeCmd();
					if (cmd->images == _render->_maxTextureImageUnits)
						cmd = _cmdPack->newGenericeCmd();
					vertexLen = addGenericeSubcmd(
						cmd, GLC_GenericeCmd::kImage,
						vertexp, vertexLen, Color4f(0,0,0,alpha), paint.image->coord
					);
					(&cmd->image)[cmd->images++] = *paint.image;
					cmd->opts[cmd->subcmd++].flags |= cmd->images << 2;
					paint.image->source->retain(); // retain source image ref
				} while(vertexLen);
			} else {
				setMetrixCmd(); // check matrix change
				auto cmd = new(_cmdPack->allocCmd(sizeof(GLC_ImageCmd))) GLC_ImageCmd;
				cmd->type = kImage_GLC_CmdType;
				cmd->depth = _zDepth;
				cmd->alpha = paint.color.a();
				cmd->format = GLC_ImageCmd::Format(texCount - 1);
				cmd->paint = *img;
				img->source->retain(); // retain source image ref
			}
		}

		void drawImageMask(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			auto vertexp = vertex.val();
			auto vertexLen = vertex.length();
			do {
				auto cmd = getGenericeCmd();
				if (cmd->images == _render->_maxTextureImageUnits)
					cmd = _cmdPack->newGenericeCmd();
				vertexLen = addGenericeSubcmd(
					cmd, GLC_GenericeCmd::kColorMask, vertexp, vertexLen,
					paint.color.to_color4f_alpha(alpha), paint.image->coord
				);
				(&cmd->image)[cmd->images++] = *paint.image;
				cmd->opts[cmd->subcmd++].flags |= cmd->images << 2;
				paint.image->source->retain(); // retain source image ref
			} while(vertexLen);
		}

		void drawGradient(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			setMetrixCmd(); // check matrix change
			auto g = paint.gradient;
			auto colorsSize = sizeof(Color4f) * g->count;
			auto positionsSize = sizeof(float) * g->count;
			auto cmdSize = sizeof(GLC_GradientCmd);
			auto cmd = new(_cmdPack->allocCmd(cmdSize + colorsSize + positionsSize)) GLC_GradientCmd;
			auto cmdp = (char*)cmd;
			auto colors = reinterpret_cast<Color4f*>(cmdp + cmdSize);
			auto positions = reinterpret_cast<float*>(cmdp + cmdSize + colorsSize);
			memcpy(colors, g->colors, colorsSize); // copy colors
			memcpy(positions, g->positions, positionsSize); // copy positions
			cmd->type = kGenerice_GLC_CmdType;
			cmd->vertex.write(*vertex, -1, vertex.length());
			cmd->depth = _zDepth;
			cmd->alpha = alpha;
			cmd->paint = *g;
			cmd->paint.colors = colors;
			cmd->paint.positions = positions;
		}

		void drawClip(GLC_State::Clip &clip, bool revoke) {
			auto cmd = new(_cmdPack->allocCmd(sizeof(GLC_ClipCmd))) GLC_ClipCmd;
			cmd->type = kClip_GLC_CmdType;
			cmd->clip = clip;
			cmd->depth = _zDepth;
			cmd->revoke = revoke;
		}

#else

		// ----------------------------------------------------------------------------------------

		void setMetrixCmd() {
			if (_chMatrix) {
				auto &mat = _state->matrix;
				const float m4x4[16] = {
					mat[0], mat[3], 0.0, 0.0,
					mat[1], mat[4], 0.0, 0.0,
					0.0,    0.0,    1.0, 0.0,
					mat[2], mat[5], 0.0, 1.0
				}; // transpose matrix
				glBindBuffer(GL_UNIFORM_BUFFER, _render->_matrixBlock);
				glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, m4x4);
				_chMatrix = false;
			}
		}

		void setBlendMode(BlendMode mode) {
			if (_blendMode != mode) {
				_render->setBlendMode(mode);
			}
		}

		void drawColor(const Array<Vec3> &vertex, const Color4f &color) {
			setMetrixCmd();
			_render->_color.use(vertex.size(), vertex.val());
			glUniform1f(_render->_color.depth, _zDepth);
			glUniform4fv(_render->_color.color, 1, color.val);
			glDrawArrays(GL_TRIANGLES, 0, vertex.length());
		}

		void drawImage(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			setMetrixCmd(); // check matrix change
			auto shader = &_render->_image;
			auto pixel = paint.image->source;
			auto type = pixel->type();
			auto texCount = type == kColor_Type_YUV420P_Y_8 ? 3:
											type == kColor_Type_YUV420SP_Y_8 ? 2: 1;
			for (int i = 0; i < texCount; i++) {
				_render->setTexture(pixel->pixels().val() + i, i, paint);
			}
			shader->use(vertex.size(), vertex.val());
			glUniform1f(shader->depth, _zDepth);
			glUniform1i(shader->format, texCount - 1);
			glUniform1f(shader->opacity, alpha);
			glUniform4fv(shader->coord, 1, paint.image->coord.origin.val);
			glUniform1i(shader->format, GLC_ImageCmd::Format(texCount - 1));
			glDrawArrays(GL_TRIANGLES, 0, vertex.length());
		}

		void drawImageMask(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			setMetrixCmd(); // check matrix change
			auto shader = &_render->_colorMask;
			_render->setTexture(paint.image->source->pixels().val(), 0, paint);
			shader->use(vertex.size(), vertex.val());
			glUniform1f(shader->depth, _zDepth);
			glUniform4f(shader->color, paint.color[0], paint.color[1], paint.color[2], paint.color[3]*alpha);
			glUniform4fv(shader->coord, 1, paint.image->coord.origin.val);
			glDrawArrays(GL_TRIANGLES, 0, vertex.length());
		}

		void drawGradient(const Array<Vec3> &vertex, const Paint &paint, float alpha) {
			setMetrixCmd(); // check matrix change
			auto g = paint.gradient;
			auto shader = paint.gradient->type ==
				GradientPaint::kRadial_Type ? &_render->_radial:
				static_cast<GLSLColorRadial*>(static_cast<GLSLShader*>(&_render->_linear));
			auto count = g->count;
			shader->use(vertex.size(), vertex.val());
			glUniform1f(shader->depth, _zDepth);
			glUniform1f(shader->alpha, alpha);
			glUniform4fv(shader->range, 1, g->origin.val);
			glUniform1i(shader->count, count);
			glUniform4fv(shader->colors, count, (const GLfloat*)g->colors);
			glUniform1fv(shader->positions, count, (const GLfloat*)g->positions);
			glDrawArrays(GL_TRIANGLES, 0, vertex.length());
			//glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex.length());
			//glDrawArrays(GL_LINES, 0, vertex.length());
		}

		void drawClip(GLC_State::Clip &clip, bool revoke) {
		}

#endif

	};

	// ----------------------------------------------------------------------------------------

	GLCanvas::GLCanvas(GLRender *render, bool isMultiThreading)
		: _render(render)
		, _zDepth(0)
		, _state(nullptr)
		, _surfaceScale(1), _transfromScale(1), _scale(1), _chMatrix(true)
		, _blendMode(kClear_BlendMode)
	{
		_cmdPack = _cmdPacks;
		_cmdPackSubmit = _cmdPacks + (isMultiThreading ? 1: 0);
		_stateStack.push({ .matrix=Mat() }); // init state
		_state = &_stateStack.back();
		setMatrix(_state->matrix); // init shader matrix
	}

	GLCanvas::~GLCanvas() {}

	void GLCanvas::submit() {
#if Qk_GL_USE_CMD_PACK
		_mutex.lock();
		auto swap = _cmdPackSubmit;
		_cmdPackSubmit = _cmdPack;
		_cmdPack = swap;
		_mutex.unlock();
#else
#endif
	}

	int GLCanvas::save() {
		_stateStack.push({ _stateStack.back() });
		_state = &_stateStack.back();
	}

	void GLCanvas::restore(uint32_t count) {
		if (!count || _stateStack.length() == 1)
			return;
		count = Uint32::min(count, _stateStack.length() - 1);

		if (count > 0) {
			do {
				auto &state = _stateStack.back();
				for (auto &clip: state.clips) {
					_this->drawClip(clip, true);
				}
				_state = &_stateStack.pop().back();
				count--;
			} while (count > 0);
			_chMatrix = true; // mark matrix change
			_this->setSurfaceScale(_state->matrix);
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
		_stateStack.push({ .matrix=Mat() }); // init state
		_state = &_stateStack.back();
		// set surface scale
		_surfaceScale = Float::max(surfaceScale[0], surfaceScale[1]);
		_transfromScale = Float::max(_state->matrix[0], _state->matrix[4]);
		_scale = _surfaceScale * _transfromScale;
		_unitPixel = 2 / _scale;
	}

	void GLCanvas::setMatrix(const Mat& mat) {
		_state->matrix = mat;
		_chMatrix = true; // mark matrix change
		_this->setSurfaceScale(mat);
	}

	void GLCanvas::translate(float x, float y) {
		_state->matrix.translate({x, y});
		_chMatrix = true;
		_this->setSurfaceScale(_state->matrix);
	}

	void GLCanvas::scale(float x, float y) {
		_state->matrix.scale({x, y});
		_chMatrix = true;
		_this->setSurfaceScale(_state->matrix);
	}

	void GLCanvas::rotate(float z) {
		_state->matrix.rotate(z);
		_chMatrix = true;
		_this->setSurfaceScale(_state->matrix);
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
		drawColor(color, kSrc_BlendMode);
	}

	void GLCanvas::drawColor(const Color4f &color, BlendMode mode) {
		auto isBlend = mode != kSrc_BlendMode || color.a() != 1;
#if Qk_GL_USE_CMD_PACK
		if (isBlend) {
			_this->setBlendMode(mode);
		}
		auto cmd = new(_cmdPack->allocCmd(sizeof(GLC_ClearCmd))) GLC_ClearCmd;
		cmd->type = kClear_GLC_CmdType;
		cmd->color = color;
		cmd->depth = _zDepth;

		if (isBlend) {
			Vec3 vectex[] = {
				{-1,1},/*left top*/{1,1},/*right top*/{-1,-1},
				{-1,-1}, /*left bottom*/{1,-1}, /*right bottom*/{1,1}
			};
			cmd->vertex.write(vectex, -1, 6);
			_this->zDepthNext();
		} else {
			_zDepth = 0;
		}
#else
		if (isBlend) {
			_this->setBlendMode(mode);
			float data[] = {
				-1,1,/*left top*/1,1,/*right top*/
				-1,-1, /*left bottom*/1,-1, /*right bottom*/
			};
			_render->setBlendMode(mode); // switch blend mode
			_render->_clear.use(sizeof(float) * 8, data);
			glUniform1f(_render->_color.depth, _zDepth);
			glUniform4fv(_render->_clear.color, 1, color.val);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			_render->_zDepth++;
		} else {
			_zDepth = 0.0f;
			glClearBufferfv(GL_DEPTH, 0, &_zDepth); // clear GL_COLOR_ATTACHMENT0
			glClearBufferfv(GL_COLOR, 0, color.val);
			// glClearColor(color.r(), color.g(), color.b(), color.a());
			// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// drawColor(color, kSrcOver_BlendMode);
			_zDepth = 0;
		}
#endif
	}

	void GLCanvas::drawRect(const Rect& rect, const Paint& paint) {
		drawPathv(_render->getRectPath(rect), paint);
	}

	void GLCanvas::drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) {
		drawPathv(_render->getRRectPath(rect,radius), paint);
	}

	void GLCanvas::drawPathvColor(const Pathv& path, const Color4f &color, BlendMode mode) {
		if (path.vertex.length()) {
			_this->setBlendMode(mode); // switch blend mode
			_this->drawColor(path.vertex, color);
			if (!_render->_IsDeviceMsaa) { // Anti-aliasing using software
				auto &vertex = _render->getAAFuzzStrokeTriangle(path.path, _unitPixel*aa_fuzz_width);
				_this->drawColor(vertex, color.to_color4f_alpha(aa_fuzz_weight));
			}
			_this->zDepthNext();
		}
	}

	void GLCanvas::drawPath(const Path &path_, const Paint &paint) {
		_this->setBlendMode(paint.blendMode); // switch blend mode

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
		_this->setBlendMode(paint.blendMode); // switch blend mode

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
		_this->setBlendMode(paint.blendMode); // switch blend mode

		Sp<ImageSource> img;
		auto tf = glyphs.typeface();
		auto bound = tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _scale, offset, &img);
		img->mark_as_texture(_render);
		auto scale_1 = _this->drawTextImage(*img, bound.y(), _scale, origin, paint);
		return scale_1 * bound.x();
	}

	void GLCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		_this->setBlendMode(paint.blendMode); // switch blend mode

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
			blob->image->mark_as_texture(_render);
		}

		_this->drawTextImage(*blob->image, blob->imageBound.y(), _scale * levelScale, origin, paint);
	}

}
