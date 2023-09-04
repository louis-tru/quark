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


#include "./gl_cmd.h"
#include "./gl_render.h"
#include "./gl_canvas.h"

namespace qk {

	GLC_ImageCmd::~GLC_ImageCmd() {
		paint.source->release();
	}

	GLC_ImageMaskCmd::~GLC_ImageMaskCmd() {
		paint.source->release();
	}

	GLC_CmdPack::GLC_CmdPack(GLRender *render, GLCanvas *canvas)
		: _render(render), _canvas(canvas), _cache(canvas->_cache), _blendMode(kSrcOver_BlendMode)
	{
		cmds.size = sizeof(GLC_Cmd);
		cmds.capacity = 65536;
		cmds.val = (GLC_Cmd*)malloc(65536);
		cmds.val->size = cmds.size;
		cmds.val->type = kEmpty_GLC_CmdType;
		lastCmd = cmds.val;
		vertexBlocks.blocks.push({
			(Vec4*)malloc(Qk_MCCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_MCCmd_VertexBlock_Capacity
		});
		optionBlocks.blocks.push({
			(MCOpt*)malloc(Qk_MCCmd_OptBlock_Capacity * sizeof(MCOpt)),0,Qk_MCCmd_OptBlock_Capacity
		});
		vertexBlocks.current = vertexBlocks.blocks.val();
		vertexBlocks.index = 0;
		optionBlocks.current = optionBlocks.blocks.val();
		optionBlocks.index = 0;
	}

	GLC_CmdPack::~GLC_CmdPack() {
		for (auto &i: vertexBlocks.blocks)
			free(i.val);
		for (auto &i: optionBlocks.blocks)
			free(i.val);
		clear();
		free(cmds.val);
	}

	void GLC_CmdPack::clear() {
		auto cmd = cmds.val;

		while (cmd <= lastCmd) {
			switch (cmd->type) {
				case kClip_GLC_CmdType:
					((GLC_ClipCmd*)cmd)->~GLC_ClipCmd();
					break;
				case kImage_GLC_CmdType:
					((GLC_ImageCmd*)cmd)->~GLC_ImageCmd();
					break;
				case kImageMask_GLC_CmdType:
					((GLC_ImageMaskCmd*)cmd)->~GLC_ImageMaskCmd();
					break;
				default: break;
			}
			cmd = (GLC_Cmd*)(((char*)cmd) + cmd->size);
		}

		lastCmd = cmds.val;
		cmds.size = lastCmd->size;
	}

	GLC_Cmd* GLC_CmdPack::allocCmd(uint32_t size) {
		auto newSize = cmds.size + size;
		if (newSize > cmds.capacity) {
			cmds.capacity <<= 1;
			cmds.val = (GLC_Cmd*)realloc(cmds.val, cmds.capacity);
		}
		lastCmd = (GLC_Cmd*)(((char*)cmds.val) + cmds.size);
		lastCmd->size = size;
		cmds.size = newSize;
		return lastCmd;
	}

	GLC_MultiColorCmd* GLC_CmdPack::newMColorCmd() {
		auto cmd = (GLC_MultiColorCmd*)allocCmd(sizeof(GLC_MultiColorCmd));
		cmd->type = kMultiColor_GLC_CmdType;

		auto vertexs = vertexBlocks.current;
		auto opts    = optionBlocks.current;

		if (vertexs->size == Qk_MCCmd_VertexBlock_Capacity) {
			if (++vertexBlocks.index == vertexBlocks.blocks.length()) {
				vertexBlocks.blocks.push({
					(Vec4*)malloc(Qk_MCCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_MCCmd_VertexBlock_Capacity
				});
			}
			vertexBlocks.current = vertexs = vertexBlocks.blocks.val() + vertexBlocks.index;
		}

		if (opts->size == Qk_MCCmd_OptBlock_Capacity) {
			if (++optionBlocks.index == optionBlocks.blocks.length()) {
				optionBlocks.blocks.push({
					(MCOpt*)malloc(Qk_MCCmd_OptBlock_Capacity * sizeof(MCOpt)),0,Qk_MCCmd_OptBlock_Capacity
				});
			}
			optionBlocks.current = opts = optionBlocks.blocks.val() + optionBlocks.index;
		}

		cmd->vertex = vertexs->val + vertexs->size;
		cmd->opts    = opts->val    + opts->size;
		cmd->subcmd = 0;
		cmd->vCount = 0;

		return cmd;
	}

	GLC_MultiColorCmd* GLC_CmdPack::getMColorCmd() {
		auto cmd = (GLC_MultiColorCmd*)lastCmd;
		if (cmd->type == kMultiColor_GLC_CmdType) {
			if (vertexBlocks.current->size != Qk_MCCmd_VertexBlock_Capacity &&
					optionBlocks.current->size != Qk_MCCmd_OptBlock_Capacity &&
					cmd->subcmd != Qk_MCCmd_Option_Capacity
			) {
				return cmd;
			}
		}
		return newMColorCmd();
	}

// ---------------------------------------------------------------------------------------

	void GLC_CmdPack::setMetrixUnifromBuffer(const Mat &mat) {
		const float m4x4[16] = {
			mat[0], mat[3], 0.0, 0.0,
			mat[1], mat[4], 0.0, 0.0,
			0.0,    0.0,    1.0, 0.0,
			mat[2], mat[5], 0.0, 1.0
		}; // transpose matrix
		glBindBuffer(GL_UNIFORM_BUFFER, _render->_viewMatrixBlock);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, m4x4, GL_DYNAMIC_DRAW);
	}

#if Qk_USE_GLC_CMD_QUEUE

		void GLC_CmdPack::setMetrix() {
			if (_canvas->_chMatrix) {
				auto cmd = (GLC_MatrixCmd*)allocCmd(sizeof(GLC_MatrixCmd));
				cmd->type = kMatrix_GLC_CmdType;
				cmd->matrix = _canvas->_state->matrix;
				_canvas->_chMatrix = false;
			}
		}

		void GLC_CmdPack::setBlendMode(BlendMode mode) {
			if (_blendMode != mode) {
				auto cmd = (GLC_BlendCmd*)allocCmd(sizeof(GLC_BlendCmd));
				cmd->type = kBlend_GLC_CmdType;
				cmd->mode = mode;
				_blendMode = mode;
			}
		}

		void GLC_CmdPack::drawColor4f(const VertexData &vertex, const Color4f &color) {
			if ( vertex.vertex.length() ) {
				auto vertexp = vertex.vertex.val();
				auto vertexLen = vertex.vCount;
				do {
					auto cmd = getMColorCmd();
					// setting vertex option data
					cmd->opts[cmd->subcmd] = {
						.flags  = 0,                       .depth = _canvas->_zDepth,
						.matrix = _canvas->_state->matrix, .color = color,
					};
					auto vertexs = vertexBlocks.current;
					auto prevSize = vertexs->size;
					int  cpLen = Qk_MCCmd_VertexBlock_Capacity - prevSize;
					auto cpSrc = vertexp;

					optionBlocks.current->size++;

					if (cpLen < vertexLen) { // not enough space
						vertexs->size = Qk_MCCmd_VertexBlock_Capacity;
						vertexp += cpLen;
						vertexLen -= cpLen;
					} else {
						vertexs->size = prevSize + vertexLen;
						cpLen = vertexLen;
						vertexLen = 0;
					}

					const float subcmd = *((float*)&cmd->subcmd);
					auto p = vertexs->val + prevSize;
					auto p_1 = p + cpLen;

					// copy vertex data
					while (p < p_1) {
#if DEBUG
						*p = *((Vec4*)(cpSrc)); p->val[3] = subcmd;
						p++,cpSrc++;
						*p = *((Vec4*)(cpSrc)); p->val[3] = subcmd;
						p++,cpSrc++;
						*p = *((Vec4*)(cpSrc)); p->val[3] = subcmd;
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
					cmd->vCount += cpLen;
					cmd->subcmd++;
				} while(vertexLen);
			} else {
				setMetrix(); // check matrix change
				auto cmd = new(allocCmd(sizeof(GLC_ColorCmd))) GLC_ColorCmd;
				cmd->type = kColor_GLC_CmdType;
				cmd->vertex = &vertex;
				cmd->depth = _canvas->_zDepth;
				cmd->color = color;
			}
		}

		void GLC_CmdPack::drawImage(const VertexData &vertex, const ImagePaint *paint, float alpha) {
			setMetrix(); // check matrix change
			auto cmd = new(allocCmd(sizeof(GLC_ImageCmd))) GLC_ImageCmd;
			cmd->type = kImage_GLC_CmdType;
			cmd->vertex = &vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->alpha = alpha;
			cmd->paint = *paint;
			paint->source->retain(); // retain source image ref
		}

		void GLC_CmdPack::drawImageMask(const VertexData &vertex, const ImagePaint *paint, const Color4f &color) {
			setMetrix(); // check matrix change
			auto cmd = new(allocCmd(sizeof(GLC_ImageMaskCmd))) GLC_ImageMaskCmd;
			cmd->type = kImageMask_GLC_CmdType;
			cmd->vertex = &vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->color = color;
			cmd->paint = *paint;
			paint->source->retain(); // retain source image ref
		}

		void GLC_CmdPack::drawGradient(const VertexData &vertex, const GradientPaint *paint, float alpha) {
			setMetrix(); // check matrix change
			auto colorsSize = sizeof(Color4f) * paint->count;
			auto positionsSize = sizeof(float) * paint->count;
			auto cmdSize = sizeof(GLC_GradientCmd);
			auto cmd = new(allocCmd(cmdSize + colorsSize + positionsSize)) GLC_GradientCmd;
			auto cmdp = (char*)cmd;
			auto colors = reinterpret_cast<Color4f*>(cmdp + cmdSize);
			auto positions = reinterpret_cast<float*>(cmdp + cmdSize + colorsSize);
			memcpy(colors, paint->colors, colorsSize); // copy colors
			memcpy(positions, paint->positions, positionsSize); // copy positions
			cmd->type = kGradient_GLC_CmdType;
			cmd->vertex = &vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->alpha = alpha;
			cmd->paint = *paint;
			cmd->paint.colors = colors;
			cmd->paint.positions = positions;
		}

		void GLC_CmdPack::drawClip(GLC_State::Clip &clip, bool revoke) {
			setMetrix(); // check matrix change
			auto cmd = new(allocCmd(sizeof(GLC_ClipCmd))) GLC_ClipCmd;
			cmd->type = kClip_GLC_CmdType;
			cmd->clip = clip;
			cmd->depth = _canvas->_zDepth;
			cmd->revoke = revoke;
		}

		void GLC_CmdPack::clearColor4f(const Color4f &color, bool isBlend) {
			auto cmd = new(allocCmd(sizeof(GLC_ClearCmd))) GLC_ClearCmd;
			cmd->type = kClear_GLC_CmdType;
			cmd->color = color;
			cmd->depth = _canvas->_zDepth;
			cmd->isBlend = isBlend;
		}

#else
		void GLC_CmdPack::setBlendMode(BlendMode mode) {
			if (_blendMode != mode) {
				_render->setBlendMode(mode);
				_blendMode = mode;
			}
		}
		void GLC_CmdPack::drawColor4f(const VertexData &vertex, const Color4f &color) {
			drawColor4fCall(_canvas->_zDepth, vertex, color);
		}
		void GLC_CmdPack::drawImage(const VertexData &vertex, const ImagePaint *paint, float alpha) {
			drawImageCall(_canvas->_zDepth, vertex, paint, alpha);
		}
		void GLC_CmdPack::drawImageMask(const VertexData &vertex, const ImagePaint *paint, const Color4f &color) {
			drawImageMaskCall(_canvas->_zDepth, vertex, paint, color);
		}
		void GLC_CmdPack::drawGradient(const VertexData &vertex, const GradientPaint *paint, float alpha) {
			drawGradientCall(_canvas->_zDepth, vertex, paint, alpha);
		}
		void GLC_CmdPack::drawClip(GLC_State::Clip &clip, bool revoke) {
			drawClipCall(_canvas->_zDepth, clip, revoke);
		}
		void GLC_CmdPack::clearColor4f(const Color4f &color, bool isBlend) {
			clearColor4fCall(_canvas->_zDepth, color, isBlend);
		}
#endif

	// ----------------------------------------------------------------------------------------

	void GLC_CmdPack::useShader(GLSLShader *shader, const VertexData &vertex) {
		if (_cache->setGpuBufferData(vertex.id)) {
			glBindVertexArray(vertex.id->vao);
			glUseProgram(shader->shader);
		} else {
			shader->use(vertex.vertex.size(), vertex.vertex.val());
		}
	}

	void GLC_CmdPack::drawColor4fCall(float depth, const VertexData &vertex, const Color4f &color) {
		useShader(&_render->_color, vertex);
		glUniform1f(_render->_color.depth, _render->_zDepth + depth);
		glUniform4fv(_render->_color.color, 1, color.val);
		glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
	}

	void GLC_CmdPack::drawImageCall(float depth, const VertexData &vertex, const ImagePaint *paint, float alpha) {
		auto shader = &_render->_image;
		auto src = paint->source;
		if (kColor_Type_YUV420P_Y_8 == src->type()) { // yuv420p or yuv420sp
			shader = (GLSLImage*)&_render->_imageYuv;
			useShader(shader, vertex);
			_render->setTexture(src->pixels().val() + 1, 1, paint);
			if (src->pixels()[1].type() == kColor_Type_YUV420P_U_8) { // yuv420p
				glUniform1i(_render->_imageYuv.format, 1);
				_render->setTexture(src->pixels().val() + 2, 2, paint);
			} else { // yuv420sp
				glUniform1i(_render->_imageYuv.format, 0);
			}
		} else {
			useShader(shader, vertex);
		}
		_render->setTexture(src->pixels().val(), 0, paint);
		glUniform1f(shader->depth, _render->_zDepth + depth);
		glUniform1f(shader->opacity, alpha);
		glUniform4fv(shader->coord, 1, paint->coord.origin.val);
		glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
	}

	void GLC_CmdPack::drawImageMaskCall(float depth, const VertexData &vertex, const ImagePaint *paint, const Color4f &color) {
		auto shader = &_render->_imageMask;
		_render->setTexture(paint->source->pixels().val(), 0, paint);
		useShader(shader, vertex);
		glUniform1f(shader->depth, _render->_zDepth + depth);
		glUniform4fv(shader->color, 1, color.val);
		glUniform4fv(shader->coord, 1, paint->coord.origin.val);
		glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
	}

	void GLC_CmdPack::drawGradientCall(float depth, const VertexData &vertex, const GradientPaint *paint, float alpha) {
		auto shader = paint->type ==
			GradientPaint::kRadial_Type ? &_render->_radial:
			static_cast<GLSLColorRadial*>(static_cast<GLSLShader*>(&_render->_linear));
		auto count = paint->count;
		useShader(shader, vertex);
		glUniform1f(shader->depth, _render->_zDepth + depth);
		glUniform1f(shader->alpha, alpha);
		glUniform4fv(shader->range, 1, paint->origin.val);
		glUniform1i(shader->count, count);
		glUniform4fv(shader->colors, count, (const GLfloat*)paint->colors);
		glUniform1fv(shader->positions, count, (const GLfloat*)paint->positions);
		glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex.length());
		//glDrawArrays(GL_LINES, 0, vertex.length());
	}

	void GLC_CmdPack::drawClipCall(float depth, GLC_State::Clip &clip, bool revoke) {
		// TODO ..
	}

	void GLC_CmdPack::clearColor4fCall(float depth, const Color4f &color, bool isBlend) {
		if (isBlend) {
			float data[] = {
				-1,1,/*left top*/1,1,/*right top*/
				-1,-1, /*left bottom*/1,-1, /*right bottom*/
			};
			_render->_clear.use(sizeof(float) * 8, data);
			glUniform1f(_render->_color.depth, _render->_zDepth + depth);
			glUniform4fv(_render->_clear.color, 1, color.val);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		} else {
			glClearBufferfv(GL_DEPTH, 0, &_render->_zDepth); // clear GL_COLOR_ATTACHMENT0
			glClearBufferfv(GL_COLOR, 0, color.val);
			// glClearColor(color.r(), color.g(), color.b(), color.a());
			// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// drawColor(color, kSrcOver_BlendMode);
		}
	}

	void GLC_CmdPack::flush() {
#if Qk_USE_GLC_CMD_QUEUE
		auto cmdPack = this;
		auto cmd = cmds.val;
		auto _this = this;
		if (_render->_blendMode != _blendMode) {
			_render->setBlendMode(_blendMode); // maintain init status
		}
		setMetrixUnifromBuffer(_canvas->_state->matrix); // Maintain final status
		allocCmd(sizeof(GLC_Cmd))->type = kEmpty_GLC_CmdType;

		while(cmd != lastCmd) {
			Qk_ASSERT(cmd->size);
			cmd = (GLC_Cmd*)(((char*)cmd) + cmd->size); // skip first empty
			switch (cmd->type) {
				case kMatrix_GLC_CmdType:
					_this->setMetrixUnifromBuffer(((GLC_MatrixCmd*)cmd)->matrix);
					break;
				case kBlend_GLC_CmdType:
					_render->setBlendMode(((GLC_BlendCmd*)cmd)->mode);
					break;
				case kClear_GLC_CmdType: {
					auto cmd1 = (GLC_ClearCmd*)cmd;
					_this->clearColor4fCall(cmd1->depth, cmd1->color, cmd1->isBlend);
					break;
				}
				case kClip_GLC_CmdType: {
					auto cmd1 = (GLC_ClipCmd*)cmd;
					_this->drawClipCall(cmd1->depth, cmd1->clip, cmd1->revoke);
					cmd1->~GLC_ClipCmd();
					break;
				}
				case kColor_GLC_CmdType: {
					auto cmd1 = (GLC_ColorCmd*)cmd;
					_this->drawColor4fCall(cmd1->depth, *cmd1->vertex, cmd1->color);
					break;
				}
				case kImage_GLC_CmdType: {
					auto cmd1 = (GLC_ImageCmd*)cmd;
					_this->drawImageCall(cmd1->depth, *cmd1->vertex, &cmd1->paint, cmd1->alpha);
					cmd1->~GLC_ImageCmd();
					break;
				}
				case kImageMask_GLC_CmdType: {
					auto cmd1 = (GLC_ImageMaskCmd*)cmd;
					_this->drawImageMaskCall(cmd1->depth, *cmd1->vertex, &cmd1->paint, cmd1->color);
					cmd1->~GLC_ImageMaskCmd();
					break;
				}
				case kGradient_GLC_CmdType: {
					auto cmd1 = (GLC_GradientCmd*)cmd;
					_this->drawGradientCall(cmd1->depth, *cmd1->vertex, &cmd1->paint, cmd1->alpha);
					break;
				}
				case kMultiColor_GLC_CmdType: {
					auto cmd1 = (GLC_MultiColorCmd*)cmd;
					glBindBuffer(GL_UNIFORM_BUFFER, _render->_optsBlock);
					glBufferData(GL_UNIFORM_BUFFER,
						sizeof(GLC_MultiColorCmd::Option) * cmd1->subcmd, cmd1->opts, GL_DYNAMIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, _render->_color1.vbo);
					glBufferData(GL_ARRAY_BUFFER, cmd1->vCount * sizeof(Vec4), cmd1->vertex, GL_DYNAMIC_DRAW);
					glBindVertexArray(_render->_color1.vao);
					glUseProgram(_render->_color1.shader);
					glUniform1f(_render->_color1.depth, _render->_zDepth);
					glDrawArrays(GL_TRIANGLES, 0, cmd1->vCount);
					break;
				}
				default: break;
			}
		}

		cmdPack->vertexBlocks.current = cmdPack->vertexBlocks.blocks.val();
		cmdPack->optionBlocks.current = cmdPack->optionBlocks.blocks.val();
		cmdPack->cmds.size = sizeof(GLC_Cmd);
		cmdPack->lastCmd = cmdPack->cmds.val;

		for (int i = cmdPack->vertexBlocks.index; i >= 0; i--) {
			cmdPack->vertexBlocks.blocks[i].size = 0;
		}
		for (int i = cmdPack->optionBlocks.index; i >= 0; i--) {
			cmdPack->optionBlocks.blocks[i].size = 0;
		}
		cmdPack->vertexBlocks.index = 0;
		cmdPack->optionBlocks.index = 0;
#endif
	}

}