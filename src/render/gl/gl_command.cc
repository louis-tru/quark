/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */


#include "./gl_command.h"
#include "./gl_render.h"
#include "./gl_canvas.h"

#define Qk_CGCmd_Option_Capacity 256
#define Qk_CGCmd_VertexBlock_Capacity 6555
#define Qk_CGCmd_OptBlock_Capacity 2048
#define Qk_CGCmd_CmdBlock_Capacity 65536
#define Qk_FLAG_AACLIP (1u << 0)
#define Qk_FLAG_Y_FLIPPED (1u << 1)
#define Qk_AACLIP(clip) (clip ? Qk_FLAG_AACLIP: 0)

namespace qk {
	extern const float aa_fuzz_weight;
	extern const float zDepthNextUnit;
	void  gl_texture_barrier();
	void gl_set_blend_mode(BlendMode mode);
	void  gl_set_framebuffer_renderbuffer(GLuint b, Vec2 s, GLenum f, GLenum at);
	GLint gl_get_texture_internalformat(ColorType type);
	GLint gl_get_texture_format(ColorType type);
	GLint gl_get_texture_data_type(ColorType format);
	void setTex_SourceImage(ImageSource* s, cPixelInfo &i, const TexStat *tex);
	void gl_set_aaclip_buffer(GLuint tex, Vec2 size);
	void gl_set_tex_renderbuffer(GLuint tex, Vec2 size);
	void gl_set_texture_no_repeat(GLenum wrapdir);
	GLuint gl_new_tex_stat();
	uint32_t alignUp(uint32_t ptr, uint32_t alignment = alignof(void*));
	void clearExec_PathvCache(PathvCache *cache);

	constexpr float whiteColor[] = {1.0f,1.0f,1.0f,1.0f};
	constexpr float emptyColor[] = {0.0f,0.0f,0.0f,0.0f};
	constexpr GLenum DrawBuffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	Qk_DEFINE_INLINE_MEMBERS(GLC_CmdPack, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLC_CmdPack::Inl*>(self)

		Cmd* allocCmd(uint32_t size) {
			Qk_ASSERT_EQ(alignUp(size), size); // Check if it is aligned
			auto block = _cmds.currentBlock;
			auto newSize = block->size + size;
			if (newSize > block->capacity) {
				if (++_cmds.index == _cmds.blocks.length()) {
					_cmds.blocks.push({ // init, alloc 64k memory
						(Cmd*)malloc(Qk_CGCmd_CmdBlock_Capacity),0,Qk_CGCmd_CmdBlock_Capacity
					});
				}
				block = _cmds.blocks.val() + _cmds.index;
				newSize = size;
				_cmds.currentBlock = block;
			}
			_lastCmd = (Cmd*)(((char*)block->val) + block->size);
			_lastCmd->size = size;
			block->size = newSize;
			return _lastCmd;
		}

		ColorBatchCmd* newColorBatchCmd() {
			auto cmd = (ColorBatchCmd*)allocCmd(sizeof(ColorBatchCmd));
			cmd->type = kColorBatch_CmdType;

			auto vertexs = _vertexBlocks.currentBlock;
			auto opts    = _optionBlocks.currentBlock;

			if (vertexs->size == Qk_CGCmd_VertexBlock_Capacity) {
				if (++_vertexBlocks.index == _vertexBlocks.blocks.length()) {
					_vertexBlocks.blocks.push({
						(Vec4*)malloc(Qk_CGCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_CGCmd_VertexBlock_Capacity
					});
				}
				_vertexBlocks.currentBlock = vertexs = _vertexBlocks.blocks.val() + _vertexBlocks.index;
			}

			if (opts->size == Qk_CGCmd_OptBlock_Capacity) {
				if (++_optionBlocks.index == _optionBlocks.blocks.length()) {
					_optionBlocks.blocks.push({
						(CGOpt*)malloc(Qk_CGCmd_OptBlock_Capacity * sizeof(CGOpt)),0,Qk_CGCmd_OptBlock_Capacity
					});
				}
				_optionBlocks.currentBlock = opts = _optionBlocks.blocks.val() + _optionBlocks.index;
			}

			cmd->vertex = vertexs->val + vertexs->size;
			cmd->opts   = opts->val    + opts->size;
			cmd->subcmd = 0;
			cmd->vCount = 0;
			cmd->aaclip = _canvas->_state->aaclip;

			return cmd;
		}

		ColorBatchCmd* getColorBatchCmd() {
			auto cmd = (ColorBatchCmd*)_lastCmd;
			if (cmd->type == kColorBatch_CmdType) {
				if (_vertexBlocks.currentBlock->size != Qk_CGCmd_VertexBlock_Capacity &&
						_optionBlocks.currentBlock->size != Qk_CGCmd_OptBlock_Capacity &&
						cmd->subcmd != Qk_CGCmd_Option_Capacity
				) {
					return cmd;
				}
			}
			return newColorBatchCmd();
		}

		void clearCmds() {
			for (auto &block: _cmds.blocks) {
				if (block.size == 0) break;
				auto cmd = block.val;
				auto end = (Cmd*)(((char*)cmd) + block.size);
				while (cmd < end) { // is end of block
					switch (cmd->type) {
						case kClip_CmdType:
							((ClipCmd*)cmd)->~ClipCmd();
							break;
						case kColor_CmdType:
							((ColorCmd*)cmd)->~ColorCmd();
							break;
						case kGradient_CmdType:
							((GradientCmd*)cmd)->~GradientCmd();
							break;
						case kImage_CmdType:
							((ImageCmd*)cmd)->~ImageCmd();
							break;
						case kImageMask_CmdType:
							((ImageMaskCmd*)cmd)->~ImageMaskCmd();
							break;
						case kSDFImageMask_CmdType:
							((SDFImageMaskCmd*)cmd)->~SDFImageMaskCmd();
							break;
						case kReadImage_CmdType:
							((ReadImageCmd*)cmd)->~ReadImageCmd();
							break;
						case kTriangles_CmdType:
							((TrianglesCmd*)cmd)->~TrianglesCmd();
							break;
						case kBlurFilterEnd_CmdType:
							((BlurFilterEndCmd*)cmd)->~BlurFilterEndCmd();
							break;
						case kOutputImageBegin_CmdType:
							((OutputImageBeginCmd*)cmd)->~OutputImageBeginCmd();
							break;
						case kOutputImageEnd_CmdType:
							((OutputImageEndCmd*)cmd)->~OutputImageEndCmd();
							break;
						case kFlushCanvas_CmdType: {
							((FlushCanvasCmd*)cmd)->~FlushCanvasCmd();
							break;
						}
						default: break;
					}
					cmd = (Cmd*)(((char*)cmd) + cmd->size); // next cmd
				}
				block.size = 0;
			}
			for (int i = _vertexBlocks.index; i >= 0; i--) {
				_vertexBlocks.blocks[i].size = 0; // reset vertex block size
			}
			for (int i = _optionBlocks.index; i >= 0; i--) {
				_optionBlocks.blocks[i].size = 0; // reset option block size
			}
			_cmds.currentBlock = _cmds.blocks.val();
			_cmds.index = 0;
			_lastCmd = _cmds.currentBlock->val;
			_lastCmd->type = kEmpty_CmdType; // set empty cmd first

			_vertexBlocks.currentBlock = _vertexBlocks.blocks.val();
			_vertexBlocks.index = 0;
			_optionBlocks.currentBlock = _optionBlocks.blocks.val();
			_optionBlocks.index = 0;
		}

		void callCmds() {
			for (auto &block: _cmds.blocks) {
				if (block.size == 0) break;
				auto cmd = block.val;
				auto end = (Cmd*)(((char*)cmd) + block.size);
				Qk_ASSERT(cmd->size, "Invalid cmd size 0");
				while (cmd < end) {
					switch (cmd->type) {
						case kMatrix_CmdType: {
							setMatrixCall(((MatrixCmd*)cmd)->matrix);
							break;
						}
						case kBlend_CmdType:
							setBlendModeCall(((BlendCmd*)cmd)->mode);
							break;
						case kBlurFilterBegin_CmdType: {
							auto c = (BlurFilterBeginCmd*)cmd;
							blurFilterBeginCall(c);
							break;
						}
						case kBlurFilterEnd_CmdType: {
							auto c = (BlurFilterEndCmd*)cmd;
							blurFilterEndCall(c);
							c->~BlurFilterEndCmd();
							break;
						}
						case kSwitch_CmdType: {
							auto c = (SwitchCmd*)cmd;
							switchStateCall(c->id, c->isEnable);
							break;
						}
						case kClear_CmdType: {
							auto c = (ClearCmd*)cmd;
							clearColorCall(c->color, c->flags, c->depth);
							break;
						}
						case kClip_CmdType: {
							auto c = (ClipCmd*)cmd;
							drawClipCall(c->clip, c->ref, c->recover.get(), c->revoke, c->depth);
							c->~ClipCmd();
							break;
						}
						case kColor_CmdType: {
							auto c = (ColorCmd*)cmd;
							drawColorCall(c->vertex, c->color, c->aaclip, c->depth);
							c->~ColorCmd();
							break;
						}
						case kRRectBlurColor_CmdType: {
							auto c = (ColorRRectBlurCmd*)cmd;
							drawRRectBlurColorCall(c->rect, c->radius, c->blur, c->color, c->aaclip, c->depth);
							break;
						}
						case kImage_CmdType: {
							auto c = (ImageCmd*)cmd;
							drawImageCall(c->vertex, &c->paint, c->allScale, c->color, c->aaclip, c->depth);
							c->~ImageCmd();
							break;
						}
						case kImageMask_CmdType: {
							auto c = (ImageMaskCmd*)cmd;
							drawImageMaskCall(c->vertex, &c->paint, c->allScale, c->color, c->aaclip, c->depth);
							c->~ImageMaskCmd();
							break;
						}
						case kSDFImageMask_CmdType: {
							auto c = (SDFImageMaskCmd*)cmd;
							drawSDFImageMaskCall(c->vertex, &c->paint, c->color, c->strokeColor,
									c->strokeWidth, c->aaclip, c->depth);
							c->~SDFImageMaskCmd();
							break;
						}
						case kTriangles_CmdType: {
							auto c = (TrianglesCmd*)cmd;
							drawTrianglesCall(c->triangles, &c->paint, c->color, c->aaclip, c->depth);
							c->~TrianglesCmd();
							break;
						}
						case kGradient_CmdType: {
							auto c = (GradientCmd*)cmd;
							drawGradientCall(c->vertex, &c->paint, c->color, c->aaclip, c->depth);
							c->~GradientCmd();
							break;
						}
						case kColorBatch_CmdType: {
							auto c = (ColorBatchCmd*)cmd;
							auto s = &_render->_shaders.colorBatch;
							glBindBuffer(GL_UNIFORM_BUFFER, _render->_ubo2);
							glBufferData(GL_UNIFORM_BUFFER, sizeof(ColorBatchCmd::Option) * c->subcmd, c->opts,
								GL_DYNAMIC_DRAW);
							glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
							glBufferData(GL_ARRAY_BUFFER, c->vCount * sizeof(Vec4), c->vertex, GL_DYNAMIC_DRAW);
							glBindVertexArray(s->vao);
							glUseProgram(s->shader);
							glUniform1ui(s->pc_flags, Qk_AACLIP(c->aaclip));
							glDrawArrays(GL_TRIANGLES, 0, c->vCount);
							break;
						}
						case kReadImage_CmdType: {
							auto c = (ReadImageCmd*)cmd;
							readImageCall(c->srcRect, c->src.get(), c->dest.get(), c->canvasSize, c->surfaceSize, c->depth);
							c->~ReadImageCmd();
							break;
						}
						case kOutputImageBegin_CmdType: {
							auto c = (OutputImageBeginCmd*)cmd;
							outputImageBeginCall(*c->dst);
							c->~OutputImageBeginCmd();
							break;
						}
						case kOutputImageEnd_CmdType: {
							auto c = (OutputImageEndCmd*)cmd;
							outputImageEndCall(*c->exit, *c->next);
							c->~OutputImageEndCmd();
							break;
						}
						case kFlushCanvas_CmdType: {
							auto c = (FlushCanvasCmd*)cmd;
							flushCanvasCall(c->srcC, c->srcCmdPack, c->restoreState);
							c->~FlushCanvasCmd();
							break;
						}
						case kSetSurface_CmdType: {
							auto c = (SetSurfaceCmd*)cmd;
							setSurfaceCall(c->surfaceSize, c->rootMatrix, c->changeSize);
							c->~SetSurfaceCmd();
							break;
						}
						case kDrawBuffers_CmdType: {
							auto c = (DrawBuffersCmd*)cmd;
							drawBuffersCall(c->num, c->buffers);
							break;
						}
						default:
							break;
					}
					cmd = (Cmd*)(((char*)cmd) + cmd->size); // next cmd
				}
				block.size = 0;
			}
			clearCmds();
		}

		void useShaderProgram(GLSLShader *shader, const VertexData &vertex) {
			if (Render::useVertexData(vertex.id)) {
				glBindVertexArray(vertex.id->a); // use vao
				glUseProgram(shader->shader); // use shader program
			} else /*if (vertex.vertex.length())*/ {
				// copy vertex data to gpu and use shader
				Qk_ASSERT_EQ(vertex.vertex.length(), vertex.vCount, "useShaderProgram, vertex vCount != vertex.vertex.size()");
				shader->use(vertex.vertex.size(), vertex.vertex.val());
			}
		}

		void setRootMatrixCall(const Mat4 &root) {
			glBindBuffer(GL_UNIFORM_BUFFER, _render->_ubo0);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, root.val, GL_DYNAMIC_DRAW);
		}

		void setMatrixCall(const Mat &mat) {
			const float m4x4[16] = {
				mat[0], mat[3], 0.0, 0.0,
				mat[1], mat[4], 0.0, 0.0,
				0.0,    0.0,    1.0, 0.0,
				mat[2], mat[5], 0.0, 1.0
			}; // transpose matrix
			glBindBuffer(GL_UNIFORM_BUFFER, _render->_ubo1);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, m4x4, GL_DYNAMIC_DRAW);
			// glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, m4x4); // for sync
		}

		void switchStateCall(GLenum id, bool isEnable) {
			isEnable ? glEnable(id): glDisable(id);
		}

		void setBlendModeCall(BlendMode mode) {
			_render->set_blend_mode(mode);
		}

		void drawColorCall(const VertexData &vertex, const Color4f &color, bool aaclip, float depth) {
			auto s = &_render->_shaders.color;
			useShaderProgram(s, vertex);
			glUniform1f(s->pc_depth, depth);
			glUniform4fv(s->pc_color, 1, color.val);
			glUniform1ui(s->pc_flags, Qk_AACLIP(aaclip));
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
		}

		void drawRRectBlurColorCall(const Rect& rect,
			const float *r, float s, const Color4f &color, bool aaclip, float depth
		) {
			s = Qk_Max(s, 0.5);
			float s1 = s * 1.15, s2 = s * 2.0;
			auto sh = &_render->_shaders.colorRrectBlur;
			float min_edge = Qk_Min(rect.size[0],rect.size[1]); // min w or h
			float rmax = 0.5 * min_edge; // max r
			Vec2 size = rect.size * 0.5;
			Vec2 c = rect.begin + size; // rect center
			float w = size[0] + s, h = size[1] + s;
			float x1 = c[0] - w, x2 = c[0] + w;
			float y1 = c[1] - h, y2 = c[1] + h;
			Vec2 horns[] = { {x1,y1}, {x2,y1}, {x2,y2}, {x1,y2} };

			glUseProgram(sh->shader); // use shader program
			glUniform4fv(sh->pc_color, 1, color.val);
			glUniform1f(sh->pc_min_edge, min_edge);
			glUniform1f(sh->pc_depth, depth);
			glUniform1f(sh->pc_s_inv, 1.0/s); // 1/s blur size reciprocal
			glUniform1ui(sh->pc_flags, Qk_AACLIP(aaclip));
			glBindBuffer(GL_ARRAY_BUFFER, sh->vbo);
			glBindVertexArray(sh->vao);

			for (int i = 0; i < 4; i++) {
				auto horn = horns[i];
				float v[] = { c[0],c[1],horn[0],c[1],horn[0],horn[1],c[0],horn[1] };
				float r0 = Float32::min(Vec2(r[i], s1).length(), rmax); // len
				float r1 = Float32::min(Vec2(r[i], s2).length(), rmax);
				float n = 2.0 * r1 / r0;
				glUniform3f(sh->pc_consts, r1, n, 1.0/n);
				glUniform2f(sh->pc_horn, horn[0], horn[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		bool useTextureSlot0(const PaintImage *paint) {
			if (paint->_isCanvas) { // flush canvas to current canvas
				auto srcC = static_cast<GLCanvas*>(paint->canvas);
				if (srcC != _canvas && srcC->isGpu()) { // now only supported gpu
					if (srcC->_render == _render) {
						if (srcC->_outTex) {
							_render->set_texture_param(srcC->_outTex, 0, paint);
							return true;
						}
					}
				}
				return false;
			} else {
				return _render->use_texture(paint->image, 0, paint);
			}
		}

		void drawImageCall(const VertexData &vertex,
			const PaintImage *paint, float allScale, const Color4f &color, bool aaclip, float depth
		) {
			if (useTextureSlot0(paint)) { // rgb or y
				auto src = paint->image;

				if (!paint->_isCanvas && kYUV420P_Y_8_ColorType == src->type()) { // yuv420p or yuv420sp
					auto yuv = &_render->_shaders.imageYuv;
					useShaderProgram(yuv, vertex);
					if (!_render->use_texture(src, 1, paint))
						return; // u or uv

					if (src->pixel(1)->type() == kYUV420P_U_8_ColorType) {
						if (!_render->use_texture(src, 2, paint)) return; // v
						glUniform1i(yuv->pc_format, 1); // yuv420p
					} else {
						glUniform1i(yuv->pc_format, 0); // yuv420sp
					}

					glUniform1ui(yuv->pc_flags, Qk_AACLIP(aaclip));
					glUniform1f(yuv->pc_depth, depth);
					glUniform1f(yuv->pc_allScale, allScale);
					glUniform4fv(yuv->pc_color, 1, color.val);
					glUniform4fv(yuv->pc_texCoords, 1, paint->coord.begin.val);
				} else {
					auto s = &_render->_shaders.image;
					useShaderProgram(s, vertex);
					glUniform1ui(s->pc_flags, Qk_AACLIP(aaclip));
					glUniform1f(s->pc_depth, depth);
					glUniform1f(s->pc_allScale, allScale);
					glUniform4fv(s->pc_color, 1, color.val);
					glUniform4fv(s->pc_texCoords, 1, paint->coord.begin.val);
				}
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawImageMaskCall(const VertexData &vertex,
			const PaintImage *paint, float allScale, const Color4f &color, bool aaclip, float depth
		) {
			if (useTextureSlot0(paint)) {
				auto type = paint->_isCanvas ? kRGBA_8888_ColorType: paint->image->type();
				auto s = &_render->_shaders.imageMask;
				useShaderProgram(s, vertex);
				glUniform1ui(s->pc_flags, Qk_AACLIP(aaclip));
				glUniform1f(s->pc_depth, depth);
				glUniform1i(s->pc_alphaIndex, type == kAlpha_8_ColorType ? 0 :
						type == kLuminance_Alpha_88_ColorType ? 1 : 3); // alpha index
				glUniform1f(s->pc_allScale, allScale);
				glUniform4fv(s->pc_color, 1, color.val);
				glUniform4fv(s->pc_texCoords, 1, paint->coord.begin.val);
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawSDFImageMaskCall(const VertexData &vertex,
				const PaintImage *paint, const Color4f &color, const Color4f &strokeColor,
				float stroke, bool aaclip, float depth) {
			if (useTextureSlot0(paint)) {
				auto s = &_render->_shaders.imageSdfMask;
				useShaderProgram(s, vertex);
				glUniform1ui(s->pc_flags, Qk_AACLIP(aaclip));
				glUniform1f(s->pc_depth, depth);
				glUniform4fv(s->pc_color, 1, color.val);
				glUniform4fv(s->pc_strokeColor, 1, stroke <= 0 ? color.val: strokeColor.val);
				glUniform1f(s->pc_strokeWidth, stroke);
				glUniform4fv(s->pc_texCoords, 1, paint->coord.begin.val);
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawTrianglesCall(const Triangles &triangles, const PaintImage *paint, const Color4f &color, bool aaclip, float depth) {
			if (useTextureSlot0(paint)) {
				auto isPre = paint->image->premultipliedAlpha();
				auto s = &_render->_shaders.triangles;
				Qk_ASSERT_EQ(triangles.indexCount % 3, 0, "drawTrianglesCall, indexCount must be a multiple of 3");
				s->use(triangles.vertCount * sizeof(V3F_T2F_C4B_C4B), triangles.verts);
				glUniform1ui(s->pc_flags, Qk_AACLIP(aaclip) | (triangles.isDarkColor ? (1u << 1): 0));
				glUniform1f(s->pc_depth, depth);
				glUniform4fv(s->pc_color, 1, color.val);
				// glUniform1f(s->premultipliedAlpha, isPre ? 1.0f : 0.0f);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _render->_ebo); // restore ebo
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * triangles.indexCount, triangles.indices, GL_DYNAMIC_DRAW);
				glDrawElements(GL_TRIANGLES, triangles.indexCount, GL_UNSIGNED_SHORT, 0);
			}
		}

		void drawGradientCall(const VertexData &vertex, 
			const PaintGradient *paint, const Color4f &color, bool aaclip, float depth
		) {
			// colorRadial and colorLinear have the same uniform layout
			GLSLColorRadial *s = paint->type == PaintGradient::kRadial_Type ?
				&_render->_shaders.colorRadial: (GLSLColorRadial*)&_render->_shaders.colorLinear;
			int count = Qk_Min(64, paint->count); // max 64 stops

			useShaderProgram(s, vertex);
			glUniform1ui(s->pc_flags, Qk_AACLIP(aaclip) | (count == 2 ? (1u << 1): 0));
			glUniform1f(s->pc_depth, depth);
			glUniform4fv(s->pc_color, 1, color.val);
			glUniform4fv(s->pc_range, 1, paint->origin.val);
			glUniform1i(s->pc_count, count);
			glBindBuffer(GL_UNIFORM_BUFFER, _render->_ubo2);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(Color4f) * count, (const GLfloat*)paint->colors, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, _render->_ubo3);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * count, (const GLfloat*)paint->positions, GL_DYNAMIC_DRAW);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			//glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex.length());
			//glDrawArrays(GL_LINES, 0, vertex.length());
		}

		void setColorbuffer(bool aaclip, ImageSource* elseOut) {
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				aaclip ? _canvas->_outAaclipTex:
				elseOut ? elseOut->texture(0)->id(): _canvas->_outTex,
				0
			);
			// glDrawBuffers(1, DrawBuffers + (aaclip ? 1: 0));
		}

		void aaClipExec(float depth, const VertexData &vertex, ImageSource *recover,
			bool revoke, float W, float C, bool clearAA) 
		{
			auto _c = _canvas;
			if (!_c->_outAaclipTex) {
				glGenTextures(1, &_c->_outAaclipTex); // gen aaclip buffer tex
				gl_set_aaclip_buffer(_c->_outAaclipTex, _c->_surfaceSize);
				setColorbuffer(true, nullptr);
				glClearBufferfv(GL_COLOR, 0, whiteColor); // clear aa clip
			}
			if (clearAA) {
				glBlendFunc(GL_ONE, GL_ZERO); // src
			} else if (revoke) {
				glBlendFunc(GL_DST_COLOR, GL_ONE); // src * dst + dst
			} else {
				glBlendFunc(GL_DST_COLOR, GL_ZERO); // src * dst
			}
			setColorbuffer(true, nullptr);
			auto shader = &_render->_shaders.aaclip;
			float aafuzzWeight = W * 0.1f;
			shader->use(vertex.vertex.size(), vertex.vertex.val());
			glUniform1ui(shader->pc_flags, revoke ? 1u << 1 : 0u);
			glUniform1f(shader->pc_depth, depth);
			glUniform1f(shader->pc_aafuzzWeight, aafuzzWeight); // Difference: -0.09
			glUniform1f(shader->pc_aafuzzConst, C + 0.9f/aafuzzWeight); // C' = C + C1/W, Difference: -11
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount); // draw test
			setColorbuffer(false, recover);
			gl_set_blend_mode(_render->_blendMode); // revoke blend mode
		}

		void clipExec(float depth, const VertexData &vertex) {
			if (vertex.vCount == 0) return;
			auto shader = &_render->_shaders.color;
			shader->use(vertex.vertex.size(), vertex.vertex.val());
			glUniform4fv(shader->pc_color, 1, emptyColor); // not output color buffer
			glUniform1f(shader->pc_depth, depth);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount); // draw test
		}

		void drawClipCall(const GC_State::Clip &clip, uint32_t ref, ImageSource *recover, bool revoke, float depth) {
			if (clip.op == Canvas::kDifference_ClipOp) { // difference clip
				glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
				glStencilOp(GL_KEEP/*fail*/, GL_KEEP/*zfail*/, revoke ? GL_INCR: GL_DECR/*zpass*/); // test success op
				clipExec(depth, clip.vertex);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep
				glStencilFunc(GL_LEQUAL, ref, 0xFFFFFFFF); // Equality passes the test
				if (clip.aafuzz.vCount) { // draw anti alias alpha
					aaClipExec(depth, clip.aafuzz, recover, revoke, aa_fuzz_weight, 0, false);
				}
			} else { // intersect clip
				glStencilOp(GL_KEEP, GL_KEEP, revoke ? GL_DECR: GL_INCR); // test success op
				if (clip.aafuzz.vCount) { // only draw anti alias alpha because already merged
					bool clear = ref == 128 && !revoke;
					aaClipExec(depth, clip.aafuzz, recover, revoke, -aa_fuzz_weight, -1, clear);
				} else {
					clipExec(depth, clip.vertex);
				}
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep
				glStencilFunc(GL_LEQUAL, ref, 0xFFFFFFFF); // Equality passes the test
			}
		}

		void clearColorCall(const Color4f &color, GC_ClearFlags flags, float depth) {
			if (flags) {
				if (flags == kClearAll_ClearFlags) { // clear stencil and depth
					//glClearBufferfv(GL_DEPTH, 0, &depth); // depth = 0
					glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, 127); // depth=0, stencil = 127
				}
				//glClearColor(color.r(), color.g(), color.b(), color.a());
				glClearBufferfv(GL_COLOR, 0, color.val); // clear GL_COLOR_ATTACHMENT0
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			} else {
				glUseProgram(_render->_shaders.clear.shader);
				glBindVertexArray(_render->_shaders.clear.vao); // vertex data uses shader constants
				glUniform1f(_render->_shaders.clear.pc_depth, depth);
				glUniform4fv(_render->_shaders.clear.pc_color, 1, color.val);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
		}

		void drawRegion(const Color4f &color, const Range &region, float depth) {
			float x1 = region.begin.x(), y1 = region.begin.y();
			float x2 = region.end.x(), y2 = region.end.y();
			float data[] = {
				x1,y1,0,/*left top*/
				x2,y1,0,/*right top*/
				x1,y2,0, /*left bottom*/
				x2,y2,0, /*right bottom*/
			};
			_render->_shaders.color.use(sizeof(float) * 12, data);
			glUniform1f(_render->_shaders.color.pc_depth, depth);
			glUniform4fv(_render->_shaders.color.pc_color, 1, color.val);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		void clearRegion(const Range &region, float scale, float offsetY, float depth) {
			auto origin = region.begin * scale;
			auto end = region.end * scale;
			drawRegion({0,0,0,0}, {
				{origin.x(), origin.y() + offsetY},
				{end.x(),    end.y()    + offsetY}
			}, depth);
		}

		void blurFilterBeginCall(BlurFilterBeginCmd* cmd) {
			// Range bounds, float radius, int imageLod, float allScale, bool clipState, float depth
			if (!_canvas->_outTexA) {
				glGenTextures(1, &_canvas->_outTexA); // ready the blur buffer
				gl_set_tex_renderbuffer(_canvas->_outTexA, _canvas->_surfaceSize);
			}
			if (!_canvas->_outTexB) {
				glGenTextures(1, &_canvas->_outTexB); // ready the blur buffer
				gl_set_tex_renderbuffer(_canvas->_outTexB, _canvas->_surfaceSize);
			}

			_render->set_blend_mode(kSrc_BlendMode); // switch blend mode to src
			if (cmd->clipState) {
				glDisable(GL_STENCIL_TEST); // close clip
			}
			// output to texture buffer then do post processing
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_outTexA, 0);

			/*clear pixels within bounds
				bounds already includes the blur radius; keep an extra guard band for
				scaled/mipmapped blur samples near the edge of the temporary texture.
			|.|.|......|.|.|
			|.|.|.body.|.|.|
			|.|.|......|.|.|
			*/
			drawRegion({0,0,0,0}, {
				{cmd->bounds.begin.x() - cmd->radius, cmd->bounds.begin.y()},
				{cmd->bounds.end.x() + cmd->radius, cmd->bounds.end.y()},
			}, cmd->depth);

			if (cmd->clipState) {
				glEnable(GL_STENCIL_TEST); // restore clip state
			}
		}

		/**
		 * Fast gaussian blur reference https://www.shadertoy.com/view/ctjcWR
		 * Relevant information reference:
		 *   https://www.shadertoy.com/view/WtKfD3
		 *   https://blog.ivank.net/fastest-gaussian-blur.html
		 *   https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
		 *   https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
		 */
		void blurFilterEndCall(BlurFilterEndCmd *cmd)
		{
			float radius = cmd->radius, depth = cmd->depth;
			float x1 = cmd->bounds.begin.x(), y1 = cmd->bounds.begin.y() + radius;
			float x2 = cmd->bounds.end.x(), y2 = cmd->bounds.end.y() - radius;
			auto fullRadius = radius * cmd->surfaceScale;
			Vec2 R = cmd->surfaceSize;
			uint32_t oRw = R.x(), oRh = R.y();

			glActiveTexture(Qk_TEXTURE0);
			Qk_BindSampler(0, 0);
			glBindTexture(GL_TEXTURE_2D, _canvas->_outTexA);

			if (cmd->clipState)
				glDisable(GL_STENCIL_TEST); // close clip

			if (cmd->imageLod) { // copy image, gen mipmap texture
				/* Copy more the x-axis regions, but ignore the y-axis blurred regions
				|/|/|//////|/|/|
				|.|.|.body.|.|.|
				|/|/|//////|/|/|
				*/
				int level = 0;
				auto &cp = _render->_shaders.vportCp;
				float x1_ = x1 - radius, x2_ = x2 + radius;
				float vertex[] = { x1_,y1,0, x2_,y1,0, x1_,y2,0, x2_,y2,0 };
				cp.use(sizeof(float) * 12, vertex);
				glUniform2f(cp.pc_iResolution, R.x(), R.y());
				glUniform4f(cp.pc_coord, 0, 0, 1, 1);
				do { // copy image level
					oRw >>= 1; oRh >>= 1;
					glUniform1f(cp.pc_depth, depth);
					glUniform1f(cp.pc_imageLod, level++);
					glUniform2f(cp.pc_oResolution, oRw, oRh);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_outTexA, level);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					depth += zDepthNextUnit;
				} while(level < cmd->imageLod && oRw && oRh);
			}

			// Setting target buffer B and flush blur texture buffer A
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_outTexB, cmd->imageLod);

			// Choosing the right blur shader
			auto blur = &_render->_shaders.blur;
			/*switch (n) {
				case 3: blur += 1; break; // blur3
				case 7: blur += 2; break; // blur7
				case 13: blur += 3; break; // blur13
				case 19: blur += 4; break; // blur19
			}*/

			/* The x-axis regions
			|/|/|//////|/|/|
			|/|.|.body.|.|/|
			|/|/|//////|/|/|
			*/
			float vertex_x[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			/* The y-axis regions
			|/|/|//////|/|/|
			|/|.|......|.|/|
			|/|.|.body.|.|/|
			|/|.|......|.|/|
			|/|/|//////|/|/|
			*/
			float y1_ = y1 - radius, y2_ = y2 + radius;
			float vertex_y[] = { x1,y1_,0, x2,y1_,0, x1,y2_,0, x2,y2_,0 };
			float oiScale = oRw / R.x(); // oResolution / iResolution
			float offsetY = (R.y() - oRh) / cmd->surfaceScale;
			/* First clean the y-axis of buffer B
			|.|.|......|.|.|
			|.|.|......|.|.|
			|.|/|//////|/|.|
			|.|.|......|.|.|
			|.|.|......|.|.|
			*/
			// Qk_DLog("------------- oRw:%d, oRh:%d, Rw:%f, Rh:%f, offsetY:%f, oiScale:%f",
			//	oRw, oRh, R.x(), R.y(), offsetY, oiScale);
			//glClearBufferfv(GL_COLOR, 0, emptyColor);
			y1_-=radius; y2_+=radius;
			clearRegion({{x1, y1_}, {x2, y1}}, oiScale, offsetY, depth); // clear top
			clearRegion({{x1, y2}, {x2, y2_}}, oiScale, offsetY, depth); // clear bottom
			clearRegion({{x1-3, y1_}, {x1, y2_}}, oiScale, offsetY, depth); // clear left
			clearRegion({{x2, y1_}, {x2+3, y2_}}, oiScale, offsetY, depth); // clear right
			// Making blur of the x-axis direction
			blur->use(sizeof(float) * 12, vertex_x);
			glUniform1f(blur->pc_depth, depth);
			glUniform2f(blur->pc_iResolution, R.x(), R.y());
			glUniform2f(blur->pc_oResolution, oRw, oRh);
			glUniform1f(blur->pc_imageLod, cmd->imageLod);
			glUniform1f(blur->pc_detail, 1.0f/(cmd->sample-1));
			glUniform2f(blur->pc_size, fullRadius / R.x(), 0); // horizontal blur
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur

			if (cmd->clipState) {
				glEnable(GL_STENCIL_TEST); // recover clip state
			}
			//!< r = s + (1-sa)*d
			//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			_render->set_blend_mode(cmd->backMode); // restore blend mode

			// recover output target
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				cmd->recover ? cmd->recover->texture(0)->id(): _canvas->_outTex, 0
			);

			glBindTexture(GL_TEXTURE_2D, _canvas->_outTexB);

			// Making blur of the y-axis direction
			blur->use(sizeof(float) * 12, vertex_y);
			glUniform1f(blur->pc_depth, depth + zDepthNextUnit);
			glUniform2f(blur->pc_oResolution, R.x(), R.y());
			glUniform2f(blur->pc_size, 0, fullRadius / R.y()); // vertical blur
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur to main render buffer
		}

		void readImageCall(const Rect &srcRect, ImageSource* src, ImageSource* dst,
				Vec2 canvasSize, Vec2 surfaceSize, float depth)
		{
			auto tex = dst->texture(0);
			auto w = dst->width(), h = dst->height();
			auto iformat = gl_get_texture_internalformat(dst->type());
			auto format = gl_get_texture_format(dst->type());
			auto type = gl_get_texture_data_type(dst->type());

			TexStat newTex;
			GLuint id = tex->id();
			if (!id) {
				tex = &newTex;
				id = gl_new_tex_stat();
				newTex.set_id(id);
			}
			glActiveTexture(Qk_TEXTURE0);
			Qk_BindSampler(0, 0);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, w, h, 0, format, type, nullptr);

			auto origin = srcRect.begin, size = srcRect.size;
			auto srcTex = src ? src->texture(0)->id(): _canvas->_outTex;
			Qk_ASSERT(srcTex, "readImageCall src texture is null");

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
			glBindTexture(GL_TEXTURE_2D, srcTex); // read image source
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, size == Vec2(w,h) ? GL_NEAREST: GL_LINEAR);
			// always use nearest filter because scTex levels 1 only
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			// use shader and set vertex data to draw a rect
			float x2 = canvasSize[0], y2 = canvasSize[1];
			float vertex[] = { 0,0,0, x2,0,0, 0,y2,0, x2,y2,0 };
			auto &cp = _render->_shaders.vportCp;
			cp.use(sizeof(float) * 12, vertex);
			glUniform1f(cp.pc_depth, depth);
			glUniform2f(cp.pc_iResolution, surfaceSize.x(), surfaceSize.y());
			glUniform2f(cp.pc_oResolution, w, h);
			glUniform1f(cp.pc_imageLod, 0);
			origin /= surfaceSize;
			size /= surfaceSize;
			glUniform4f(cp.pc_coord, origin[0], origin[1], size[0], size[1]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0);

			// generate mipmap if needed
			if (dst->mipmap()) {
				glBindTexture(GL_TEXTURE_2D, id);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			setTex_SourceImage(dst, dst->info(), tex);
		}

		void outputImageBeginCall(ImageSource* dst) {
			auto s = _canvas->_surfaceSize; // surface size
			auto tex = dst->texture(0);
			GLuint id = tex->id();
			TexStat newTex;
			if (id) {
				if (s == dst->info().size()) {
					glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
					return; // already allocated with right size, just return
				}
			} else {
				tex = &newTex;
				id = gl_new_tex_stat();
				newTex.set_id(id);
			}
			auto iformat = gl_get_texture_internalformat(dst->type());
			auto format = gl_get_texture_format(dst->type());
			auto type = gl_get_texture_data_type(dst->type());
			glActiveTexture(Qk_TEXTURE0);
			Qk_BindSampler(0, 0);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, s[0], s[1], 0, format, type, nullptr);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
			setTex_SourceImage(dst, {int(s[0]),int(s[1]),dst->type(),dst->info().alphaType()}, tex);
		}

		void outputImageEndCall(ImageSource* exit, ImageSource* next) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				next ? next->texture(0)->id(): _canvas->_outTex, 0);
			Qk_ASSERT(exit, "outputImageEndCall exit image is null");
			if (exit->mipmap()) {
				glActiveTexture(Qk_TEXTURE0);
				Qk_BindSampler(0, 0);
				glBindTexture(GL_TEXTURE_2D, exit->texture(0)->id());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		void flushCanvasCall(GLCanvas* srcC, GLC_CmdPack* srcCmdPack, PipelineState &restoreState) {
			// initial pipeline state for flush
			auto &state = srcCmdPack->_pipelineState;
			glBindFramebuffer(GL_FRAMEBUFFER, srcC->_fbo);
			setRootMatrixCall(state.rootMatrix); // init root matrix
			setMatrixCall(state.matrix); // init matrix
			_render->set_blend_mode(state.mode); // init blend mode
			_render->set_viewport(state.surfaceSize);

			// call subcanvas subs
			_inl(srcCmdPack)->callCmds();

			// Restore pipeline state after flush
			_render->set_viewport(restoreState.surfaceSize); // ch viewport
			_render->set_blend_mode(restoreState.mode); // ch mode
			setRootMatrixCall(restoreState.rootMatrix); // ch root matrix
			setMatrixCall(restoreState.matrix); // ch matrix
			glBindFramebuffer(GL_FRAMEBUFFER, _canvas->_fbo); // bind top fbo

			if (srcC->_opts.mipmap) { // gen mipmap texture
				glActiveTexture(Qk_TEXTURE0);
				Qk_BindSampler(0, 0);
				glBindTexture(GL_TEXTURE_2D, srcC->_outTex);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			clearExec_PathvCache(srcC->_cache); // clear @clear mark
		}

		void flushCanvas(const PaintImage *paint) {
			auto srcC = static_cast<GLCanvas*>(paint->canvas);
			if (!paint->_isCanvas)
				return; // return if not canvas image
			if (srcC->_render != _render || srcC == _canvas)
				return; // only support flush sub canvas to parent canvas

			GLC_CmdPack *srcCmdPack = nullptr;
			srcC->_mutex.lock();
			if (!srcC->_cmdPackFront->isEmpty()) { // use canvas cmd pack
				srcCmdPack = srcC->_cmdPackFront;
				srcC->_cmdPackFront = new GLC_CmdPack(_render, srcC);
			}
			srcC->_mutex.unlock(); // unlock

			if (srcCmdPack) {
				auto cmd = new(_this->allocCmd(sizeof(FlushCanvasCmd))) FlushCanvasCmd;
				cmd->type = kFlushCanvas_CmdType;
				cmd->srcC = static_cast<GLCanvas*>(paint->canvas);
				cmd->srcCmdPack = srcCmdPack;
				// save current state for restore after flush
				cmd->restoreState = {
					_canvas->_surfaceSize,
					_canvas->_rootMatrix,
					_canvas->_state->matrix, _canvas->_blendMode
				};
				srcC->retain(); // retain canvas for async flush, release in cmd destructor
			}
		}

		void setSurfaceCall(Vec2 surfaceSize, Mat4 rootMatrix, bool chSize) {
			auto _c = _canvas;
			auto w = surfaceSize.x(), h = surfaceSize.y();

			Qk_ASSERT(w, "Invalid viewport surface size width");
			Qk_ASSERT(h, "Invalid viewport surface size height");

			// update shader root matrix and clear all save state buffers
			_render->set_viewport(surfaceSize);
			if (chSize)
				_c->setBuffers(surfaceSize);
			// root matrix buffer
			setRootMatrixCall(rootMatrix);
			// clear aa clip tex buffer
			if (_c->_outAaclipTex) {
				setColorbuffer(true, nullptr); // set output to aa clip buffer
				glClearBufferfv(GL_COLOR, 0, whiteColor); // clear aaclip buffer
			}
			setColorbuffer(false, nullptr); // set output to normal buffer
			glClearBufferfi(GL_DEPTH_STENCIL, 0, 0, 127); // clear depth and stencil
			glDisable(GL_STENCIL_TEST); // disable stencil test
		}

		void drawBuffersCall(GLsizei num, const GLenum buffers[2]) {
			glDrawBuffers(num, buffers);
		}
	};

	// ---------------------------------------------------------------------------------------

	GLC_CmdPack::ImageCmd::~ImageCmd() {
		paint.image->release();
	}

	GLC_CmdPack::ImageMaskCmd::~ImageMaskCmd() {
		paint.image->release();
	}

	GLC_CmdPack::TrianglesCmd::~TrianglesCmd() {
		paint.image->release();
		if (copyData) {
			free(triangles.verts);
		}
	}

	GLC_CmdPack::FlushCanvasCmd::~FlushCanvasCmd() {
		srcC->release();
		delete srcCmdPack; // delete cmd pack
	}

	void GLC_CmdPack::savePipelineState() {
		_pipelineState = {
			_canvas->_surfaceSize,
			_canvas->_rootMatrix,
			_canvas->_state->matrix, _canvas->_blendMode
		};
	}

	bool GLC_CmdPack::isEmpty() {
		return _lastCmd->type == kEmpty_CmdType;
	}

	GLC_CmdPack::GLC_CmdPack(GLRender *render, GLCanvas *canvas)
		: _render(render), _canvas(canvas), _cache(canvas->getPathvCache())
		, _lastCmd(nullptr)
	{
		_cmds.blocks.push({ // init, alloc 64k memory
			(Cmd*)malloc(Qk_CGCmd_CmdBlock_Capacity),0,Qk_CGCmd_CmdBlock_Capacity // 65k
		});
		_cmds.index = 0;
		_cmds.currentBlock = _cmds.blocks.val();
		_lastCmd = _cmds.currentBlock->val;
		_lastCmd->type = kEmpty_CmdType;

		// color group cmd storage
		_vertexBlocks.blocks.push({
			(Vec4*)malloc(Qk_CGCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_CGCmd_VertexBlock_Capacity // 104k
		});
		_vertexBlocks.currentBlock = _vertexBlocks.blocks.val();
		_vertexBlocks.index = 0;

		_optionBlocks.blocks.push({
			(CGOpt*)malloc(Qk_CGCmd_OptBlock_Capacity * sizeof(CGOpt)),0,Qk_CGCmd_OptBlock_Capacity // 98k
		});
		_optionBlocks.currentBlock = _optionBlocks.blocks.val();
		_optionBlocks.index = 0;
	}

	GLC_CmdPack::~GLC_CmdPack() {
		_this->clearCmds();
		for (auto &i: _vertexBlocks.blocks)
			free(i.val);
		for (auto &i: _optionBlocks.blocks)
			free(i.val);
		for (auto &i: _cmds.blocks)
			free(i.val);
	}

	void GLC_CmdPack::flush() {
		if (!isEmpty())
			_this->callCmds();
	}

	void GLC_CmdPack::setMatrix() {
		auto cmd = (MatrixCmd*)_this->allocCmd(sizeof(MatrixCmd));
		cmd->type = kMatrix_CmdType;
		cmd->matrix = _canvas->_state->matrix;
	}

	void GLC_CmdPack::setBlendMode() {
		auto cmd = (BlendCmd*)_this->allocCmd(sizeof(BlendCmd));
		cmd->type = kBlend_CmdType;
		cmd->mode = _canvas->_blendMode;
	}

	void GLC_CmdPack::switchState(GLenum id, bool isEnable) {
		auto cmd = (SwitchCmd*)_this->allocCmd(sizeof(SwitchCmd));
		cmd->type = kSwitch_CmdType;
		cmd->id = id;
		cmd->isEnable = isEnable;
	}

	void GLC_CmdPack::drawColor(const VertexData &vertex, const Color4f &color) {
#define Qk_USE_ColorBatch (!Qk_LINUX)
#if Qk_USE_ColorBatch
		if ( vertex.vertex.length() == 0 ) { // Maybe it's already cached
#endif
			auto cmd = new(_this->allocCmd(sizeof(ColorCmd))) ColorCmd;
			cmd->type = kColor_CmdType;
			cmd->vertex = vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->aaclip = _canvas->_state->aaclip;
			cmd->color = color;
#if Qk_USE_ColorBatch
		} else {
			// add multi color subcmd
			auto vertexp = vertex.vertex.val();
			auto vertexLen = vertex.vCount;
			do {
				auto cmd = _this->getColorBatchCmd();
				cmd->opts[cmd->subcmd] = { // setting vertex option data
					.flags  = 0,                       .depth = _canvas->_zDepth,
					.matrix = _canvas->_state->matrix, .color = color,
				};
				auto vertexs = _vertexBlocks.currentBlock;
				auto prevSize = vertexs->size;
				int  cpLen = Qk_CGCmd_VertexBlock_Capacity - prevSize;
				auto cpSrc = vertexp;

				_optionBlocks.currentBlock->size++;

				if (cpLen < vertexLen) { // not enough space
					vertexs->size = Qk_CGCmd_VertexBlock_Capacity;
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
					#define Qk_CopyVec() *p = \
						*((Vec4*)(cpSrc)); p->val[3] = subcmd; p++,cpSrc++
#else
					#define Qk_CopyVec() *p = {\
						cpSrc->val[0],cpSrc->val[1],cpSrc->val[2],subcmd\
					}; p++,cpSrc++
#endif
					Qk_CopyVec();
					Qk_CopyVec();
					Qk_CopyVec();
					Qk_CopyVec();
					#undef Qk_CopyVec
				}
				cmd->vCount += cpLen;
				cmd->subcmd++;
			} while(vertexLen);
		}
#endif
	}

	void GLC_CmdPack::drawRRectBlurColor(const Rect& rect, const float *radius, float blur, const Color4f &color) {
		auto cmd = new(_this->allocCmd(sizeof(ColorRRectBlurCmd))) ColorRRectBlurCmd;
		cmd->type = kRRectBlurColor_CmdType;
		cmd->depth = _canvas->_zDepth;
		cmd->rect = rect;
		cmd->radius[0] = radius[0];
		cmd->radius[1] = radius[1];
		cmd->radius[2] = radius[2];
		cmd->radius[3] = radius[3];
		cmd->color = color;
		cmd->blur = blur;
		cmd->aaclip = _canvas->_state->aaclip;
	}

	void GLC_CmdPack::drawImage(const VertexData &vertex, const PaintImage *paint, const Color4f& color) {
		_this->flushCanvas(paint);
		auto cmd = new(_this->allocCmd(sizeof(ImageCmd))) ImageCmd;
		cmd->type = kImage_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->allScale = _canvas->_allScale;
		cmd->color = color;
		cmd->paint = *paint;
		paint->image->retain(); // retain source image ref
	}

	void GLC_CmdPack::drawImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color) {
		_this->flushCanvas(paint);
		auto cmd = new(_this->allocCmd(sizeof(ImageMaskCmd))) ImageMaskCmd;
		cmd->type = kImageMask_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->allScale = _canvas->_allScale;
		cmd->color = color;
		cmd->paint = *paint;
		paint->image->retain(); // retain source image ref
	}

	void GLC_CmdPack::drawSDFImageMask(const VertexData &vertex, const PaintImage *paint,
		const Color4f &color, const Color4f &strokeColor, float stroke) 
	{
		auto cmd = new(_this->allocCmd(sizeof(SDFImageMaskCmd))) SDFImageMaskCmd;
		cmd->type = kSDFImageMask_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->allScale = _canvas->_allScale;
		cmd->color = color;
		cmd->paint = *paint;
		cmd->strokeColor = strokeColor;
		cmd->strokeWidth = stroke;
		paint->image->retain();
	}

	void GLC_CmdPack::drawTriangles(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData) {
		if (!triangles.verts || !triangles.indices || !triangles.vertCount || !triangles.indexCount)
			return;
		_this->flushCanvas(paint);
		auto cmd = new(_this->allocCmd(sizeof(TrianglesCmd))) TrianglesCmd;
		cmd->type = kTriangles_CmdType;
		cmd->triangles = triangles;
		cmd->depth = _canvas->_zDepth;
		cmd->paint = *paint;
		cmd->color = color;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->copyData = copyData;
		if (copyData) {
			auto vertsSize = triangles.vertCount * sizeof(V3F_T2F_C4B_C4B);
			auto indicesSize = triangles.indexCount * sizeof(uint16_t);
			auto memPtr = (char*)malloc(vertsSize + indicesSize);
			cmd->triangles.verts = (V3F_T2F_C4B_C4B*)memPtr;
			cmd->triangles.indices = (uint16_t*)(memPtr + vertsSize);
			memcpy(cmd->triangles.verts, triangles.verts, vertsSize);
			memcpy(cmd->triangles.indices, triangles.indices, indicesSize);
		}
		paint->image->retain();
	}

	void GLC_CmdPack::drawGradient(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) {
		auto colorsSize = (uint32_t)sizeof(Color4f) * paint->count;
		auto positionsSize = (uint32_t)sizeof(float) * paint->count;
		auto cmdSize = (uint32_t)sizeof(GradientCmd);
		auto cmd = new(_this->allocCmd(alignUp(cmdSize + colorsSize + positionsSize))) GradientCmd;
		auto cmdp = (char*)cmd;
		auto colors = reinterpret_cast<Color4f*>(cmdp + cmdSize);
		auto positions = reinterpret_cast<float*>(cmdp + cmdSize + colorsSize);
		memcpy(colors, paint->colors, colorsSize); // copy colors
		memcpy(positions, paint->positions, positionsSize); // copy positions
		cmd->type = kGradient_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->color = color;
		cmd->paint = *paint;
		cmd->paint.colors = colors;
		cmd->paint.positions = positions;
	}

	void GLC_CmdPack::drawClip(const GC_State::Clip &clip, uint32_t ref, ImageSource *recover, bool revoke) {
		auto cmd = new(_this->allocCmd(sizeof(ClipCmd))) ClipCmd;
		cmd->type = kClip_CmdType;
		cmd->clip = { .op=clip.op };
		// Canvas::kDifference_ClipOp need combine vertex and aafuzz vertex
		if (clip.aafuzz.vCount && clip.op != Canvas::kDifference_ClipOp) {
			// combine clip vertex and aafuzz vertex
			cmd->clip.aafuzz.vCount = clip.vertex.vCount + clip.aafuzz.vCount;
			cmd->clip.aafuzz.vertex.extend(cmd->clip.aafuzz.vCount);
			cmd->clip.aafuzz.vertex.write(clip.vertex.vertex.val(), clip.vertex.vCount, 0);
			cmd->clip.aafuzz.vertex.write(clip.aafuzz.vertex.val(), clip.aafuzz.vCount, clip.vertex.vCount);
		} else {
			cmd->clip.vertex = clip.vertex;
			cmd->clip.aafuzz = clip.aafuzz;
		}
		cmd->depth = _canvas->_zDepth;
		cmd->ref = ref;
		cmd->revoke = revoke;
		cmd->recover = recover;
	}

	void GLC_CmdPack::clearColor(const Color4f &color, GC_ClearFlags flags) {
		auto cmd = new(_this->allocCmd(sizeof(ClearCmd))) ClearCmd;
		cmd->type = kClear_CmdType;
		cmd->color = color;
		cmd->depth = _canvas->_zDepth;
		cmd->flags = flags;
	}

	void GLC_CmdPack::blurFilterBegin(Range bounds, float radius, float clearPad) {
		auto cmd = new(_this->allocCmd(sizeof(BlurFilterBeginCmd))) BlurFilterBeginCmd;
		cmd->type = kBlurFilterBegin_CmdType;
		cmd->bounds = bounds;
		cmd->radius = radius;
		cmd->clearPad = clearPad;
		cmd->depth = _canvas->_zDepth;
		cmd->clipState = _canvas->_clipState;
	}

	void GLC_CmdPack::blurFilterEnd(Range bounds, float radius, float clearPad, int sample, int imageLod) {
		auto cmd = new(_this->allocCmd(sizeof(BlurFilterEndCmd))) BlurFilterEndCmd;
		cmd->type = kBlurFilterEnd_CmdType;
		cmd->bounds = bounds;
		cmd->depth = _canvas->_zDepth;
		cmd->backMode = _canvas->_blendMode;
		cmd->radius = radius;
		cmd->clearPad = clearPad;
		cmd->surfaceScale = _canvas->_allScale;
		cmd->surfaceSize = _canvas->_surfaceSize;
		cmd->clipState = _canvas->_clipState;
		cmd->recover = _canvas->_state->output;
		cmd->sample = sample;
		cmd->imageLod = imageLod;
	}

	void GLC_CmdPack::readImage(const Rect &srcRect, ImageSource* src, ImageSource* dst) {
		auto cmd = new(_this->allocCmd(sizeof(ReadImageCmd))) ReadImageCmd;
		cmd->type = kReadImage_CmdType;
		cmd->srcRect = srcRect;
		cmd->src = src;
		cmd->dest = dst;
		cmd->canvasSize = _canvas->_size;
		cmd->surfaceSize = _canvas->_surfaceSize;
		cmd->depth = _canvas->_zDepth;
	}

	void GLC_CmdPack::outputImageBegin(ImageSource* dst) {
		auto cmd = new(_this->allocCmd(sizeof(OutputImageBeginCmd))) OutputImageBeginCmd;
		cmd->type = kOutputImageBegin_CmdType;
		cmd->dst = dst;
	}

	void GLC_CmdPack::outputImageEnd(ImageSource* exit, ImageSource* next) {
		auto cmd = new(_this->allocCmd(sizeof(OutputImageEndCmd))) OutputImageEndCmd;
		cmd->type = kOutputImageEnd_CmdType;
		cmd->exit = exit;
		cmd->next = next;
	}

	void GLC_CmdPack::setSurface(bool changeSize) {
		auto cmd = new(_this->allocCmd(sizeof(SetSurfaceCmd))) SetSurfaceCmd;
		cmd->type = kSetSurface_CmdType;
		cmd->surfaceSize = _canvas->_surfaceSize;
		cmd->rootMatrix = _canvas->_rootMatrix;
		cmd->changeSize = changeSize;
	}

	void GLC_CmdPack::drawBuffers(GLsizei num, const GLenum buffers[2]) {
		auto cmd = new(_this->allocCmd(sizeof(DrawBuffersCmd))) DrawBuffersCmd;
		cmd->type = kDrawBuffers_CmdType;
		cmd->num = num;
		cmd->buffers[0] = buffers[0];
		cmd->buffers[1] = buffers[1];
	}
}
