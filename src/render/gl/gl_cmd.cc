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

#define Qk_CGCmd_Option_Capacity 1024
#define Qk_CGCmd_VertexBlock_Capacity 6555
#define Qk_CGCmd_OptBlock_Capacity 2048
#define Qk_CGCmd_CmdBlock_Capacity 65536

namespace qk {
	extern const float aa_fuzz_weight;
	extern const float DepthNextUnit;
	void  gl_texture_barrier();
	void  gl_set_framebuffer_renderbuffer(GLuint b, Vec2 s, GLenum f, GLenum at);
	GLint gl_get_texture_pixel_format(ColorType type);
	GLint gl_get_texture_data_type(ColorType format);
	void  setTex_SourceImage_RT(ImageSource* s, cPixelInfo &i, const TexStat *tex, bool isMipmap);
	void  gl_set_aaclip_buffer(GLuint tex, Vec2 size);
	void  gl_set_blur_renderbuffer(GLuint tex, Vec2 size);
	TexStat* gl_new_texture();

	Qk_DEFINE_INLINE_MEMBERS(GLC_CmdPack, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLC_CmdPack::Inl*>(self)

#if Qk_USE_GLC_CMD_QUEUE
		void clearCmds() {
			for (auto &i: _cmds.blocks) {
				auto cmd = i.val;
				auto end = (Cmd*)(((char*)cmd) + i.size);
				while (cmd < end) {
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
						case kReadImage_CmdType:
							((ReadImageCmd*)cmd)->~ReadImageCmd();
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
							auto c = ((FlushCanvasCmd*)cmd);
							c->srcC->release();
							delete c->srcCmd; // delete cmd pack
							break;
						}
						default: break;
					}
					cmd = (Cmd*)(((char*)cmd) + cmd->size);
				}
				i.size = 0;
			}
			_cmds.index = 0;
			_cmds.current = _cmds.blocks.val();
			_lastCmd = _cmds.current->val;
		}

		Cmd* allocCmd(uint32_t size) {
			auto cmds = _cmds.current;
			auto newSize = cmds->size + size;
			if (newSize > cmds->capacity) {
				if (++_cmds.index == _cmds.blocks.length()) {
					_cmds.blocks.push({ // init, alloc 64k memory
						(Cmd*)malloc(Qk_CGCmd_CmdBlock_Capacity),0,Qk_CGCmd_CmdBlock_Capacity
					});
				}
				_cmds.current = cmds = _cmds.blocks.val() + _cmds.index;
				newSize = size;
			}
			_lastCmd = (Cmd*)(((char*)cmds->val) + cmds->size);
			_lastCmd->size = size;
			cmds->size = newSize;
			return _lastCmd;
		}

		ColorGroupCmd* newColorGroupCmd() {
			auto cmd = (ColorGroupCmd*)allocCmd(sizeof(ColorGroupCmd));
			cmd->type = kColorGroup_CmdType;

			auto vertexs = _vertexBlocks.current;
			auto opts    = _optionBlocks.current;

			if (vertexs->size == Qk_CGCmd_VertexBlock_Capacity) {
				if (++_vertexBlocks.index == _vertexBlocks.blocks.length()) {
					_vertexBlocks.blocks.push({
						(Vec4*)malloc(Qk_CGCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_CGCmd_VertexBlock_Capacity
					});
				}
				_vertexBlocks.current = vertexs = _vertexBlocks.blocks.val() + _vertexBlocks.index;
			}

			if (opts->size == Qk_CGCmd_OptBlock_Capacity) {
				if (++_optionBlocks.index == _optionBlocks.blocks.length()) {
					_optionBlocks.blocks.push({
						(CGOpt*)malloc(Qk_CGCmd_OptBlock_Capacity * sizeof(CGOpt)),0,Qk_CGCmd_OptBlock_Capacity
					});
				}
				_optionBlocks.current = opts = _optionBlocks.blocks.val() + _optionBlocks.index;
			}

			cmd->vertex = vertexs->val + vertexs->size;
			cmd->opts   = opts->val    + opts->size;
			cmd->subcmd = 0;
			cmd->vCount = 0;
			cmd->aaclip = _canvas->_state->aaclip;

			return cmd;
		}

		ColorGroupCmd* getMColorCmd() {
			auto cmd = (ColorGroupCmd*)_lastCmd;
			if (cmd->type == kColorGroup_CmdType) {
				if (_vertexBlocks.current->size != Qk_CGCmd_VertexBlock_Capacity &&
						_optionBlocks.current->size != Qk_CGCmd_OptBlock_Capacity &&
						cmd->subcmd != Qk_CGCmd_Option_Capacity
				) {
					return cmd;
				}
			}
			return newColorGroupCmd();
		}

		void checkMetrix() {
			if (_chMatrix) { // check canvas matrix change state
				auto cmd = (MatrixCmd*)allocCmd(sizeof(MatrixCmd));
				cmd->type = kMatrix_CmdType;
				cmd->matrix = _canvas->_state->matrix;
				_chMatrix = false;
			}
		}
#endif

		void callCmds(const Mat4& root, const Mat& mat, BlendMode mode) {
#if Qk_USE_GLC_CMD_QUEUE
			MatrixCmd *curMat = nullptr;

			for (auto &i: _cmds.blocks) {
				if (i.size == 0) break;
				auto cmd = i.val;
				auto end = (Cmd*)(((char*)cmd) + i.size);
				Qk_ASSERT(cmd->size);

				while (cmd < end) {
					switch (cmd->type) {
						case kMatrix_CmdType: {
							curMat = (MatrixCmd*)cmd;
							setMatrixCall(curMat->matrix);
							break;
						}
						case kBlend_CmdType:
							setBlendModeCall(((BlendCmd*)cmd)->mode);
							break;
						case kBlurFilterBegin_CmdType: {
							auto c = (BlurFilterBeginCmd*)cmd;
							blurFilterBeginCall(c->bounds, c->isClipState, c->depth);
							break;
						}
						case kBlurFilterEnd_CmdType: {
							auto c = (BlurFilterEndCmd*)cmd;
							blurFilterEndCall(c->bounds, c->size,
								*c->output, c->mode, c->n, c->lod, c->isClipState, c->depth);
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
							clearColorCall(c->color, c->region, c->fullClear, c->depth);
							break;
						}
						case kClip_CmdType: {
							auto c = (ClipCmd*)cmd;
							drawClipCall(c->clip, c->ref, c->revoke, c->depth);
							c->~ClipCmd();
							break;
						}
						case kColor_CmdType: {
							auto c = (ColorCmd*)cmd;
							drawColorCall(c->vertex, c->color, c->aafuzz, c->aaclip, c->depth);
							break;
						}
						case kRRectBlurColor_CmdType: {
							auto c = (ColorRRectBlurCmd*)cmd;
							drawRRectBlurColorCall(c->rect, c->radius, c->blur, c->color, c->aaclip, c->depth);
							break;
						}
						case kImage_CmdType: {
							auto c = (ImageCmd*)cmd;
							drawImageCall(c->vertex, &c->paint, c->alpha, c->aafuzz, c->aaclip, c->depth);
							c->~ImageCmd();
							break;
						}
						case kImageMask_CmdType: {
							auto c = (ImageMaskCmd*)cmd;
							drawImageMaskCall(c->vertex, &c->paint, c->color, c->aafuzz, c->aaclip, c->depth);
							c->~ImageMaskCmd();
							break;
						}
						case kGradient_CmdType: {
							auto c = (GradientCmd*)cmd;
							drawGradientCall(c->vertex, &c->paint, c->alpha, c->aafuzz, c->aaclip, c->depth);
							break;
						}
						case kColorGroup_CmdType: {
							auto c = (ColorGroupCmd*)cmd;
							auto s = c->aaclip ? &_render->_shaders.color1_AACLIP: &_render->_shaders.color1;
							glBindBuffer(GL_UNIFORM_BUFFER, _render->_optsBlock);
							glBufferData(GL_UNIFORM_BUFFER, sizeof(ColorGroupCmd::Option) * c->subcmd, c->opts,
								GL_DYNAMIC_DRAW);
							glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
							glBufferData(GL_ARRAY_BUFFER, c->vCount * sizeof(Vec4), c->vertex, GL_DYNAMIC_DRAW);
							glBindVertexArray(s->vao);
							glUseProgram(s->shader);
							glDrawArrays(GL_TRIANGLES, 0, c->vCount);
							break;
						}
						case kReadImage_CmdType: {
							auto c = (ReadImageCmd*)cmd;
							readImageCall(c->src, *c->img, c->isMipmap, c->canvasSize, c->surfaceSize, c->depth);
							c->~ReadImageCmd();
							break;
						}
						case kOutputImageBegin_CmdType: {
							auto c = (OutputImageBeginCmd*)cmd;
							outputImageBeginCall(*c->img, c->isMipmap);
							c->~OutputImageBeginCmd();
							break;
						}
						case kOutputImageEnd_CmdType: {
							auto c = (OutputImageEndCmd*)cmd;
							outputImageEndCall(*c->img, c->isMipmap);
							c->~OutputImageEndCmd();
							break;
						}
						case kFlushCanvas_CmdType: {
							auto c = (FlushCanvasCmd*)cmd;
							flushCanvasCall(c->srcC, c->srcCmd, c->root, c->mat, c->mode, root,
								curMat ? curMat->matrix: mat);
							c->srcC->release();
							delete c->srcCmd; // delete gl cmd pack
							break;
						}
						case kSetBuffers_CmdType: {
							auto c = (SetBuffersCmd*)cmd;
							setBuffersCall(c->size, c->chSize, c->isClip);
							break;
						}
						case kDrawBuffers_CmdType: {
							auto c = (DrawBuffersCmd*)cmd;
							drawBuffersCall(c->num, c->buffers);
							break;
						}
						default: break;
					}
					cmd = (Cmd*)(((char*)cmd) + cmd->size); // next cmd
				}
				i.size = 0;
			}

			_cmds.index = 0;
			_cmds.current = _cmds.blocks.val();
			_lastCmd = _cmds.current->val;

			for (int i = _vertexBlocks.index; i >= 0; i--) {
				_vertexBlocks.blocks[i].size = 0;
			}
			for (int i = _optionBlocks.index; i >= 0; i--) {
				_optionBlocks.blocks[i].size = 0;
			}

			_vertexBlocks.current = _vertexBlocks.blocks.val();
			_vertexBlocks.index = 0;
			_optionBlocks.current = _optionBlocks.blocks.val();
			_optionBlocks.index = 0;
#endif
		}

		void flushAAClipBuffer() {
			//gl_texture_barrier();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _canvas->_aaclipTex, 0);
		}

		void useShaderProgram(GLSLShader *shader, const VertexData &vertex) {
			if (_cache->makeVertexData(vertex.id)) {
				glBindVertexArray(vertex.id->vao); // use vao
				glUseProgram(shader->shader); // use shader program
			} else {
				// copy vertex data to gpu and use shader
				shader->use(vertex.vertex.size(), vertex.vertex.val());
			}
		}

		void setRootMatrixCall(const Mat4 &root) {
			glBindBuffer(GL_UNIFORM_BUFFER, _render->_rootMatrixBlock);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, root.val, GL_DYNAMIC_DRAW);
		}

		void setMatrixCall(const Mat &mat) {
			const float m4x4[16] = {
				mat[0], mat[3], 0.0, 0.0,
				mat[1], mat[4], 0.0, 0.0,
				0.0,    0.0,    1.0, 0.0,
				mat[2], mat[5], 0.0, 1.0
			}; // transpose matrix
			glBindBuffer(GL_UNIFORM_BUFFER, _render->_viewMatrixBlock);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, m4x4, GL_DYNAMIC_DRAW);
		}

		void switchStateCall(GLenum id, bool isEnable) {
			isEnable ? glEnable(id): glDisable(id);
		}

		void setBlendModeCall(BlendMode mode) {
			_render->gl_set_blend_mode(mode);
		}

		void drawColorCall(const VertexData &vertex,
			const Color4f &color, bool aafuzz, bool aaclip, float depth
		) {
			auto s = aafuzz ? 
				aaclip ? &_render->_shaders.color_AAFUZZ_AACLIP: &_render->_shaders.color_AAFUZZ:
				aaclip ? &_render->_shaders.color_AACLIP: &_render->_shaders.color;
			useShaderProgram(s, vertex);
			glUniform1f(s->depth, depth);
			glUniform4fv(s->color, 1, color.val);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
		}

		void drawRRectBlurColorCall(const Rect& rect,
			const float *r, float s, const Color4f &color, bool aaclip, float depth
		) {
			s = Qk_MAX(s, 0.5);
			float s1 = s * 1.15, s2 = s * 2.0;
			auto sh = aaclip ?
				&_render->_shaders.colorRrectBlur_AACLIP: &_render->_shaders.colorRrectBlur;
			float min_edge = Qk_MIN(rect.size[0],rect.size[1]); // min w or h
			float rmax = 0.5 * min_edge; // max r
			Vec2 size = rect.size * 0.5;
			Vec2 c = rect.origin + size; // rect center
			float w = size[0] + s, h = size[1] + s;
			float x1 = c[0] - w, x2 = c[0] + w;
			float y1 = c[1] - h, y2 = c[1] + h;
			Vec2 p_[] = { {x1,y1}, {x2,y1}, {x2,y2}, {x1,y2} };

			glUseProgram(sh->shader); // use shader program
			glUniform4fv(sh->color, 1, color.val);
			glUniform1f(sh->min_edge, min_edge);
			glUniform1f(sh->depth, depth);
			glUniform1f(sh->s_inv, 1.0/s); // 1/s blur size reciprocal
			glBindBuffer(GL_ARRAY_BUFFER, sh->vbo);
			glBindVertexArray(sh->vao);

			for (int i = 0; i < 4; i++) {
				auto p = p_[i];
				float v[] = { c[0],c[1],p[0],c[1],p[0],p[1],c[0],p[1] };
				float r0 = Float32::min(Vec2(r[i], s1).length(), rmax); // len
				float r1 = Float32::min(Vec2(r[i], s2).length(), rmax);
				float n = 2.0 * r1 / r0;
				glUniform3f(sh->__, r1, n, 1.0/n);
				glUniform2f(sh->horn, p[0], p[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		bool setTextureSlot0(const ImagePaint *paint) {
			if (paint->_flushCanvas) { // flush canvas to current canvas
				auto srcC = static_cast<GLCanvas*>(paint->canvas);
				if (srcC != _canvas && srcC->isGpu()) { // now only supported gpu
					if (srcC->_render == _render) {
						if (srcC->_isTexRender) {
							_render->gl_set_texture_param(srcC->_t_rbo, 0, paint);
							return true;
						}
					}
				}
				return false;
			} else {
				return _render->gl_set_texture(paint->image, 0, paint);
			}
		}

		void drawImageCall(const VertexData &vertex,
			const ImagePaint *paint, float alpha, bool aafuzz, bool aaclip, float depth
		) {
			if (setTextureSlot0(paint)) { // rgb or y
				GLSLImage *s;
				auto src = paint->image;
				auto srcIndex = paint->srcIndex;

				if (kColor_Type_YUV420P_Y_8 == src->type()) { // yuv420p or yuv420sp
					auto yuv = aafuzz ?
						aaclip ? &_render->_shaders.imageYuv_AAFUZZ_AACLIP: &_render->_shaders.imageYuv_AAFUZZ:
						aaclip ? &_render->_shaders.imageYuv_AACLIP: &_render->_shaders.imageYuv;
					s = (GLSLImage*)yuv;
					useShaderProgram(s, vertex);

					if (src->pixels()[1].type() == kColor_Type_YUV420P_U_8) { // yuv420p
						glUniform1i(yuv->format, 1);
						_render->gl_set_texture(src, 2, paint); // v
					} else { // yuv420sp
						glUniform1i(yuv->format, 0);
					}
					_render->gl_set_texture(src, 1, paint); // u or uv
				} else {
					s = aafuzz ?
						aaclip ? &_render->_shaders.image_AAFUZZ_AACLIP: &_render->_shaders.image_AAFUZZ:
						aaclip ? &_render->_shaders.image_AACLIP: &_render->_shaders.image;
					useShaderProgram(s, vertex);
				}
				glUniform1f(s->depth, depth);
				glUniform1f(s->alpha, alpha);
				glUniform4fv(s->coord, 1, paint->coord.origin.val);
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawImageMaskCall(const VertexData &vertex,
			const ImagePaint *paint, const Color4f &color, bool aafuzz, bool aaclip, float depth
		) {
			if (setTextureSlot0(paint)) {
				auto s = aafuzz ? 
					aaclip ? &_render->_shaders.imageMask_AAFUZZ_AACLIP: &_render->_shaders.imageMask_AAFUZZ:
					aaclip ? &_render->_shaders.imageMask_AACLIP: &_render->_shaders.imageMask;
				useShaderProgram(s, vertex);
				glUniform1f(s->depth, depth);
				glUniform4fv(s->color, 1, color.val);
				glUniform4fv(s->coord, 1, paint->coord.origin.val);
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawGradientCall(const VertexData &vertex, 
			const GradientPaint *paint, float alpha, bool aafuzz, bool aaclip, float depth
		) {
			GLSLColorRadial *s;
			auto count = paint->count;

			if (paint->type == GradientPaint::kRadial_Type) {
				s = count == 2 ?
					aaclip ? &_render->_shaders.colorRadial_COUNT2_AACLIP: &_render->_shaders.colorRadial_COUNT2:
					aaclip ? &_render->_shaders.colorRadial_AACLIP: &_render->_shaders.colorRadial;
			} else {
				s = (GLSLColorRadial*)(count == 2 ?
					aaclip ? &_render->_shaders.colorLinear_COUNT2_AACLIP: &_render->_shaders.colorLinear_COUNT2:
					aaclip ? &_render->_shaders.colorLinear_AACLIP: &_render->_shaders.colorLinear);
			}

			useShaderProgram(s, vertex);
			glUniform1f(s->depth, depth);
			glUniform1f(s->alpha, alpha);
			glUniform4fv(s->range, 1, paint->origin.val);
			glUniform1i(s->count, count);
			glUniform4fv(s->colors, count, (const GLfloat*)paint->colors);
			glUniform1fv(s->positions, count, (const GLfloat*)paint->positions);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			//glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex.length());
			//glDrawArrays(GL_LINES, 0, vertex.length());
		}

		void drawClipCall(const GLC_State::Clip &clip, uint32_t ref, bool revoke, float depth) {
			auto _c = _canvas;

			if (!_c->_stencilBuffer) {
				glGenRenderbuffers(1, &_c->_stencilBuffer); // gen stencil buffer
				gl_set_framebuffer_renderbuffer(
					_c->_stencilBuffer, _c->_surfaceSize, GL_STENCIL_INDEX8, GL_STENCIL_ATTACHMENT
				);
				glClear(GL_STENCIL_BUFFER_BIT); // clear stencil buffer
			}

			auto aaClip = [](Inl *self, float depth, const GLC_State::Clip &clip, bool revoke, float W, float C) {
				auto _c = self->_canvas;
				auto _render = _c->_render;
				auto chMode = _render->_blendMode;
				auto ch = chMode != kSrc_BlendMode && chMode != kSrcOver_BlendMode;

				if (!_c->_aaclipTex) {
					glGenTextures(1, &_c->_aaclipTex); // gen aaclip buffer tex
					gl_set_aaclip_buffer(_c->_aaclipTex, _c->_surfaceSize);
					float color[] = {1.0f,1.0f,1.0f,1.0f};
					glClearBufferfv(GL_COLOR, 1, color); // clear GL_COLOR_ATTACHMENT1
					// ensure clip texture clear can be executed correctly in sequence
					self->flushAAClipBuffer();
				}
				if (ch)
					_render->gl_set_blend_mode(kSrc_BlendMode);
				auto shader = revoke ? &_render->_shaders.clipAa_AACLIP_REVOKE: &_render->_shaders.clipAa;
				float aafuzzWeight = W * 0.1f;
				// float aafuzzWeight = W;
				shader->use(clip.aafuzz.vertex.size(), clip.aafuzz.vertex.val());
				glUniform1f(shader->depth, depth);
				glUniform1f(shader->aafuzzWeight, aafuzzWeight);
				glUniform1f(shader->aafuzzConst, C + 0.9f/aafuzzWeight); // C' = C + C1/W
				// glUniform1f(shader->aafuzzConst, C);
				glDrawArrays(GL_TRIANGLES, 0, clip.aafuzz.vCount); // draw test
				if (ch)
					_render->gl_set_blend_mode(chMode); // revoke blend mode
				// ensure aa clip can be executed correctly in sequence
				self->flushAAClipBuffer();
			};

			if (clip.op == Canvas::kDifference_ClipOp) { // difference clip
				glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
				glStencilOp(GL_KEEP/*fail*/, GL_KEEP/*zfail*/, revoke ? GL_INCR: GL_DECR/*zpass*/); // test success op
				_render->_shaders.clipTest.use(clip.vertex.vertex.size(), clip.vertex.vertex.val()); // only stencil fill test
				glUniform1f(_render->_shaders.clipTest.depth, depth);
				glDrawArrays(GL_TRIANGLES, 0, clip.vertex.vCount); // draw test

				if (clip.aafuzz.vCount) { // draw anti alias alpha
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep
					glStencilFunc(GL_LEQUAL, ref, 0xFFFFFFFF); // Equality passes the test
					aaClip(this, depth, clip, revoke, aa_fuzz_weight, 0);
					return;
				}
			} else { // intersect clip
				auto shader = ref == 128 && !revoke ?
					(GLSLClipTest*)&_render->_shaders.clipTest_CLIP_FILL: &_render->_shaders.clipTest; // init fill
				glStencilOp(GL_KEEP, GL_KEEP, revoke ? GL_DECR: GL_INCR); // test success op
				shader->use(clip.vertex.vertex.size(), clip.vertex.vertex.val()); // only stencil fill test
				glUniform1f(shader->depth, depth);
				glDrawArrays(GL_TRIANGLES, 0, clip.vertex.vCount); // draw test

				if (clip.aafuzz.vCount) { // draw anti alias alpha
					aaClip(this, depth, clip, revoke, -aa_fuzz_weight, -1);
				}
			}
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep
			glStencilFunc(GL_LEQUAL, ref, 0xFFFFFFFF); // Equality passes the test
		}

		void clearColorCall(const Color4f &color, const Region &region, bool full, float depth) {
			if (full) {
				glClearBufferfv(GL_DEPTH, 0, &depth); // depth = 0
				glClearBufferfv(GL_COLOR, 0, color.val); // clear GL_COLOR_ATTACHMENT0
				// glClearColor(color.r(), color.g(), color.b(), color.a());
				// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			} else {
				float x1 = region.origin.x(), y1 = region.origin.y();
				float x2 = region.end.x(), y2 = region.end.y();
				float data[] = {
					x1,y1,0,/*left top*/
					x2,y1,0,/*right top*/
					x1,y2,0, /*left bottom*/
					x2,y2,0, /*right bottom*/
				};
				_render->_shaders.clear.use(sizeof(float) * 12, data);
				glUniform1f(_render->_shaders.clear.depth, depth);
				glUniform4fv(_render->_shaders.clear.color, 1, color.val);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		void getBlurSampling(float size, int &n, int &lod) {
			const int N[] = { 3,3,3,3, 7,7,7,7, 13,13,13,13,13,13, 19,19,19,19,19,19 };
			size *= _canvas->_surfaceScale;
			n = ceilf(size); // sampling rate
			n = N[Qk_MIN(n,19)];
			lod = ceilf(Float32::max(0,log2f(size/n)));
			// Qk_DEBUG("getBlurSampling %d, lod: %d", n, lod);
		}

		void blurFilterBeginCall(Region bounds, bool isClipState, float depth) {
			if (!_canvas->_blurTex) {
				glGenTextures(1, &_canvas->_blurTex);
				gl_set_blur_renderbuffer(_canvas->_blurTex, _canvas->_surfaceSize);
			}
			if (kSrc_BlendMode != _render->_blendMode) {
				_render->gl_set_blend_mode(kSrc_BlendMode); // switch blend mode to src
			}
			if (isClipState) {
				glDisable(GL_STENCIL_TEST); // close clip
			}
			// output to texture buffer then do post processing
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_blurTex, 0);
			clearColorCall({0,0,0,0}, bounds, false, depth); // clear pixels within bounds

			if (isClipState) {
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
		void blurFilterEndCall(Region bounds, float size,
			ImageSource* output, BlendMode mode, int n, int lod, bool isClipState, float depth)
		{
			float x1 = bounds.origin.x(), y1 = bounds.origin.y();
			float x2 = bounds.end.x(), y2 = bounds.end.y();
			float data[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			auto fullSize = size * _canvas->_surfaceScale;
			auto R = _canvas->_surfaceSize;
			uint32_t oRw = R.x(), oRh = R.y();
			auto _c = _canvas;
			auto blur = &_render->_shaders.blur;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _canvas->_blurTex);

			if (lod) { // copy image, gen mipmap texture
				if (isClipState) {
					glDisable(GL_STENCIL_TEST); // close clip
				}
				int level = 0;
				auto &cp = _render->_shaders.vportCp;
				cp.use(sizeof(float) * 12, data);
				glUniform2f(cp.iResolution, R.x(), R.y());
				glUniform4f(cp.coord, 0, 0, 1, 1);
				do { // copy image level
					oRw >>= 1; oRh >>= 1;
					glUniform1f(cp.depth, depth);
					glUniform1i(cp.imageLod, level);
					glUniform2f(cp.oResolution, oRw, oRh);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						_c->_blurTex, level+1);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					depth += DepthNextUnit;
				} while(++level < lod);
			}

			/*switch(n) {
				case 3: blur = blur + 1; break;
				case 7: blur = blur + 2; break;
				case 13: blur = blur + 3; break;
				case 19: blur = blur + 4; break;
			}*/

			// flush blur texture buffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _c->_blurTex, lod);

			if (size > lod) {
				size -= lod; // reduce blank areas, fault tolerance
				y1 += size; y2 -= size; // reduce horizontal blank areas
			}
			float datax[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			blur->use(sizeof(float) * 12, datax);
			// blur horizontal blur
			glUniform1f(blur->depth, depth);
			glUniform2f(blur->iResolution, R.x(), R.y());
			glUniform2f(blur->oResolution, oRw, oRh);
			glUniform1i(blur->imageLod, lod);
			glUniform1f(blur->detail, 1.0f/(n-1));
			glUniform2f(blur->size, fullSize / R.x(), 0); // horizontal blur
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur

			if (output) { // output target
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output->texture_RT(0)->id, 0);
			} else if (_canvas->_isTexRender) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _c->_t_rbo->id, 0);
			} else {
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _c->_rbo);
			}

			if (lod && isClipState) {
				glEnable(GL_STENCIL_TEST); // close clip
			}
			//!< r = s + (1-sa)*d
			//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			if (mode != kSrc_BlendMode) {
				_render->gl_set_blend_mode(mode); // restore blend mode
			}

			blur->use(sizeof(float) * 12, data);
			// blur vertical blur
			glUniform1f(blur->depth, depth + DepthNextUnit);
			glUniform2f(blur->oResolution, R.x(), R.y());
			glUniform2f(blur->size, 0, fullSize / R.y()); // vertical blur
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur to main render buffer
		}

		void readImageCall(const Rect &src, ImageSource* img,
			bool isMipmap, Vec2 canvasSize, Vec2 surfaceSize, float depth
		){
			auto tex = img->texture_RT(0);
			auto o = src.origin, s = src.size;
			auto w = img->width(), h = img->height();
			auto iformat = gl_get_texture_pixel_format(img->type());
			auto type = gl_get_texture_data_type(img->type());

			if (!tex) {
				tex = gl_new_texture();
			} else {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tex->id);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, w, h, 0, iformat, type, nullptr);

			if (_canvas->_isTexRender) {
				float x2 = canvasSize[0], y2 = canvasSize[1];
				float data[] = { 0,0,0, x2,0,0, 0,y2,0, x2,y2,0 };
				auto &cp = _render->_shaders.vportCp;
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->id, 0);
				glBindTexture(GL_TEXTURE_2D, _canvas->_t_rbo->id); // read image source
				if (s == Vec2(w,h)) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				} else {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				}
				cp.use(sizeof(float) * 12, data);
				glUniform1f(cp.depth, depth);
				glUniform2f(cp.iResolution, surfaceSize.x(), surfaceSize.y());
				glUniform2f(cp.oResolution, w, h);
				glUniform1i(cp.imageLod, 0);
				o /= surfaceSize; s /= surfaceSize;
				glUniform4f(cp.coord, o[0], o[1], s[0], s[1]);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_t_rbo->id, 0);
				glBindTexture(GL_TEXTURE_2D, tex->id);
			} else {
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _render->_fbo);
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->id, 0);
				glBlitFramebuffer(o[0], o[1], o[0]+s[0], o[1]+s[1],
					0, 0, w, h, GL_COLOR_BUFFER_BIT, s == Vec2(w,h) ? GL_NEAREST: GL_LINEAR);
				glBindFramebuffer(GL_FRAMEBUFFER, _canvas->_t_rbo->id);
			}

			if (isMipmap) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			setTex_SourceImage_RT(img, img->info(), tex, isMipmap);
		}

		void outputImageBeginCall(ImageSource* img, bool isMipmap) {
			auto tex = img->texture_RT(0);
			auto size = _canvas->_surfaceSize;
			auto iformat = gl_get_texture_pixel_format(img->type());
			auto type = gl_get_texture_data_type(img->type());
			if (!tex) {
				tex = gl_new_texture();
			} else {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tex->id);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, size[0], size[1], 0, iformat, type, nullptr);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->id, 0);
			setTex_SourceImage_RT(img, {int(size[0]),int(size[1]),img->type(),img->info().alphaType()}, tex, isMipmap);
		}

		void outputImageEndCall(ImageSource* img, bool isMipmap) {
			if (_canvas->_isTexRender)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_t_rbo->id, 0);
			else
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _canvas->_rbo);
			if (isMipmap) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, img->texture_RT(0)->id);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		void flushCanvasCall(GLCanvas* srcC, GLC_CmdPack* srcCmd,
			const Mat4 &root, const Mat &mat, BlendMode mode, const Mat4 &parentRoot, const Mat &parentMat
		) {
			auto parentMode = _render->_blendMode;
			auto chPort = srcC->_surfaceSize != _canvas->_surfaceSize;
			auto chMode = mode != parentMode;

				// switch canvas status
			if (chMode) {
				_render->gl_set_blend_mode(mode);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, srcC->_fbo);
			setRootMatrixCall(root);
			setMatrixCall(mat);

			if (chPort) {
				glViewport(0, 0, srcC->_surfaceSize[0], srcC->_surfaceSize[1]);
			}
			// call subcanvas subs
			_inl(srcCmd)->callCmds(root, mat, mode);

			// * Restore to current state value *
			if (chPort) {
				glViewport(0, 0, _canvas->_surfaceSize[0], _canvas->_surfaceSize[1]);
			}
			if (chMode) {
				_render->gl_set_blend_mode(parentMode); // ch mode
			}
			setRootMatrixCall(parentRoot); // ch root matrix
			setMatrixCall(parentMat); // ch matrix
			glBindFramebuffer(GL_FRAMEBUFFER, _canvas->_fbo); // bind top fbo

			if (srcC->_opts.isMipmap) { // gen mipmap texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, srcC->_t_rbo->id);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		void flushCanvas(const ImagePaint *paint) {
#if Qk_USE_GLC_CMD_QUEUE
			auto srcC = static_cast<GLCanvas*>(paint->canvas);
			if (!paint->_flushCanvas || !srcC->isGpu())
				return; // now only supported gpu
			if (srcC->_render != _render || srcC == _canvas)
				return;
			if (!srcC->_isTexRender)
				return;

			GLC_CmdPack *srcCmd = nullptr;
			srcC->_mutex.mutex.lock();
			if (srcC->_cmdPackFront->isHaveCmds()) { // use canvas cmd pack
				srcCmd = srcC->_cmdPackFront;
				srcC->_cmdPackFront = new GLC_CmdPack(_render, srcC);
			}
			srcC->_mutex.mutex.unlock(); // unlock

			if (srcCmd) {
				auto cmd = new(_this->allocCmd(sizeof(FlushCanvasCmd))) FlushCanvasCmd;
				cmd->type = kFlushCanvas_CmdType;
				cmd->srcC = static_cast<GLCanvas*>(paint->canvas);
				cmd->srcCmd = srcCmd;
				cmd->root = srcC->_rootMatrix;
				cmd->mat = srcC->_state->matrix;
				cmd->mode = srcC->_blendMode;
				srcC->retain();
			}
#endif
		}

		void setBuffersCall(Vec2 size, bool chSize, bool isClip) {
			auto _c = _canvas;
			auto w = size.x(), h = size.y();
			auto type = _c->_opts.colorType;

			Qk_ASSERT(w, "Invalid viewport size width");
			Qk_ASSERT(h, "Invalid viewport size height");

			chSize = chSize && size == _c->_surfaceSize;

			if (chSize) {
				_c->setBuffers();
			}
			// update shader root matrix and clear all save state
			if (_render->_glcanvas == _canvas) { // main canvas
				glViewport(0, 0, size[0], size[1]);
				// init root matrix buffer
				setRootMatrixCall(_c->_rootMatrix);
				// init matrix buffer
				setMatrixCall(_c->_state->matrix);
			}
			if (chSize || isClip) { // is clear clip buffer
				if (_c->_aaclipTex) { // clear aa clip tex buffer
					float color[] = {1.0f,1.0f,1.0f,1.0f};
					glClearBufferfv(GL_COLOR, 1, color); // clear GL_COLOR_ATTACHMENT1
					// ensure clip texture clear can be executed correctly in sequence
					flushAAClipBuffer();
				}
				if (_c->_stencilBuffer) {
					glClear(GL_STENCIL_BUFFER_BIT); // clear stencil buffer
					glDisable(GL_STENCIL_TEST); // disable stencil test
				}
			}
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

#if Qk_USE_GLC_CMD_QUEUE

	bool GLC_CmdPack::isHaveCmds() {
		return _lastCmd->type != kEmpty_CmdType;
	}

	GLC_CmdPack::GLC_CmdPack(GLRender *render, GLCanvas *canvas)
		: _render(render), _canvas(canvas), _cache(canvas->getPathvCache())
		, _lastCmd(nullptr), _chMatrix(true)
	{
		_cmds.blocks.push({ // init, alloc 64k memory
			(Cmd*)malloc(Qk_CGCmd_CmdBlock_Capacity),sizeof(Cmd),Qk_CGCmd_CmdBlock_Capacity // 65k
		});
		_cmds.index = 0;
		_cmds.current = _cmds.blocks.val();
		_lastCmd = _cmds.current->val;
		_lastCmd->size = sizeof(Cmd);
		_lastCmd->type = kEmpty_CmdType;

		// color group cmd storage
		_vertexBlocks.blocks.push({
			(Vec4*)malloc(Qk_CGCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_CGCmd_VertexBlock_Capacity // 104k
		});
		_vertexBlocks.current = _vertexBlocks.blocks.val();
		_vertexBlocks.index = 0;

		_optionBlocks.blocks.push({
			(CGOpt*)malloc(Qk_CGCmd_OptBlock_Capacity * sizeof(CGOpt)),0,Qk_CGCmd_OptBlock_Capacity // 98k
		});
		_optionBlocks.current = _optionBlocks.blocks.val();
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
		if (isHaveCmds())
			_this->callCmds(_canvas->_rootMatrix, _canvas->_state->matrix, _canvas->_blendMode);
	}

	void GLC_CmdPack::setMetrix() {
		_chMatrix = true; // mark matrix change
	}

	void GLC_CmdPack::setBlendMode(BlendMode mode) {
		auto cmd = (BlendCmd*)_this->allocCmd(sizeof(BlendCmd));
		cmd->type = kBlend_CmdType;
		cmd->mode = mode;
	}

	void GLC_CmdPack::switchState(GLenum id, bool isEnable) {
		auto cmd = (SwitchCmd*)_this->allocCmd(sizeof(SwitchCmd));
		cmd->type = kSwitch_CmdType;
		cmd->id = id;
		cmd->isEnable = isEnable;
	}

	void GLC_CmdPack::drawColor(const VertexData &vertex, const Color4f &color, bool aafuzz) {
		if ( vertex.vertex.length() == 0 ) { // length == 0
			_this->checkMetrix(); // check matrix change
			auto cmd = new(_this->allocCmd(sizeof(ColorCmd))) ColorCmd;
			cmd->type = kColor_CmdType;
			cmd->vertex = vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->aafuzz = aafuzz;
			cmd->aaclip = _canvas->_state->aaclip;
			cmd->color = color;
		} else {
			// add multi color subcmd
			auto vertexp = vertex.vertex.val();
			auto vertexLen = vertex.vCount;
			do {
				auto cmd = _this->getMColorCmd();
				cmd->opts[cmd->subcmd] = { // setting vertex option data
					.flags  = 0,                       .depth = _canvas->_zDepth,
					.matrix = _canvas->_state->matrix, .color = color,
				};
				auto vertexs = _vertexBlocks.current;
				auto prevSize = vertexs->size;
				int  cpLen = Qk_CGCmd_VertexBlock_Capacity - prevSize;
				auto cpSrc = vertexp;

				_optionBlocks.current->size++;

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
	}

	void GLC_CmdPack::drawRRectBlurColor(const Rect& rect, const float *radius, float blur, const Color4f &color) {
		_this->checkMetrix(); // check matrix change
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

	void GLC_CmdPack::drawImage(const VertexData &vertex, const ImagePaint *paint, float alpha, bool aafuzz) {
		_this->flushCanvas(paint);
		_this->checkMetrix(); // check matrix change
		auto cmd = new(_this->allocCmd(sizeof(ImageCmd))) ImageCmd;
		cmd->type = kImage_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aafuzz = aafuzz;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->alpha = alpha;
		cmd->paint = *paint;
		paint->image->retain(); // retain source image ref
	}

	void GLC_CmdPack::drawImageMask(const VertexData &vertex, const ImagePaint *paint, const Color4f &color, bool aafuzz) {
		_this->flushCanvas(paint);
		_this->checkMetrix(); // check matrix change
		auto cmd = new(_this->allocCmd(sizeof(ImageMaskCmd))) ImageMaskCmd;
		cmd->type = kImageMask_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aafuzz = aafuzz;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->color = color;
		cmd->paint = *paint;
		paint->image->retain(); // retain source image ref
	}

	void GLC_CmdPack::drawGradient(const VertexData &vertex, const GradientPaint *paint, float alpha, bool aafuzz) {
		_this->checkMetrix(); // check matrix change
		auto colorsSize = sizeof(Color4f) * paint->count;
		auto positionsSize = sizeof(float) * paint->count;
		auto cmdSize = sizeof(GradientCmd);
		auto cmd = new(_this->allocCmd(cmdSize + colorsSize + positionsSize)) GradientCmd;
		auto cmdp = (char*)cmd;
		auto colors = reinterpret_cast<Color4f*>(cmdp + cmdSize);
		auto positions = reinterpret_cast<float*>(cmdp + cmdSize + colorsSize);
		memcpy(colors, paint->colors, colorsSize); // copy colors
		memcpy(positions, paint->positions, positionsSize); // copy positions
		cmd->type = kGradient_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aafuzz = aafuzz;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->alpha = alpha;
		cmd->paint = *paint;
		cmd->paint.colors = colors;
		cmd->paint.positions = positions;
	}

	void GLC_CmdPack::drawClip(const GLC_State::Clip &clip, uint32_t ref, bool revoke) {
		_this->checkMetrix(); // check matrix change
		auto cmd = new(_this->allocCmd(sizeof(ClipCmd))) ClipCmd;
		cmd->type = kClip_CmdType;
		cmd->clip = clip; // copy clip
		cmd->depth = _canvas->_zDepth;
		cmd->ref = ref;
		cmd->revoke = revoke;
	}

	void GLC_CmdPack::clearColor(const Color4f &color, const Region &region, bool full) {
		auto cmd = new(_this->allocCmd(sizeof(ClearCmd))) ClearCmd;
		cmd->type = kClear_CmdType;
		cmd->color = color;
		cmd->region = region;
		cmd->depth = _canvas->_zDepth;
		cmd->fullClear = full;
	}

	void GLC_CmdPack::blurFilterBegin(Region bounds) {
		auto cmd = new(_this->allocCmd(sizeof(BlurFilterBeginCmd))) BlurFilterBeginCmd;
		cmd->type = kBlurFilterBegin_CmdType;
		cmd->bounds = bounds;
		cmd->depth = _canvas->_zDepth;
		cmd->isClipState = _canvas->_isClipState;
	}

	int GLC_CmdPack::blurFilterEnd(Region bounds, float size, ImageSource* output) {
		auto cmd = new(_this->allocCmd(sizeof(BlurFilterEndCmd))) BlurFilterEndCmd;
		cmd->type = kBlurFilterEnd_CmdType;
		cmd->bounds = bounds;
		cmd->depth = _canvas->_zDepth;
		cmd->mode = _canvas->_blendMode;
		cmd->size = size;
		cmd->isClipState = _canvas->_isClipState;
		cmd->output = output;
		_this->getBlurSampling(size, cmd->n, cmd->lod);
		return cmd->lod + 2;
	}

	void GLC_CmdPack::readImage(const Rect &src, ImageSource* img, bool isMipmap) {
		auto cmd = new(_this->allocCmd(sizeof(ReadImageCmd))) ReadImageCmd;
		cmd->type = kReadImage_CmdType;
		cmd->src = src;
		cmd->img = img;
		cmd->isMipmap = isMipmap;
		cmd->depth = _canvas->_zDepth;
		cmd->canvasSize = _canvas->_size;
		cmd->surfaceSize = _canvas->_surfaceSize;
	}

	void GLC_CmdPack::outputImageBegin(ImageSource* img, bool isMipmap) {
		auto cmd = new(_this->allocCmd(sizeof(OutputImageBeginCmd))) OutputImageBeginCmd;
		cmd->type = kOutputImageBegin_CmdType;
		cmd->img = img;
		cmd->isMipmap = isMipmap;
	}

	void GLC_CmdPack::outputImageEnd(ImageSource* img, bool isMipmap) {
		auto cmd = new(_this->allocCmd(sizeof(OutputImageEndCmd))) OutputImageEndCmd;
		cmd->type = kOutputImageEnd_CmdType;
		cmd->img = img;
		cmd->isMipmap = isMipmap;
	}

	void GLC_CmdPack::setBuffers(Vec2 size, bool chSize, bool isClip) {
		auto cmd = new(_this->allocCmd(sizeof(SetBuffersCmd))) SetBuffersCmd;
		cmd->type = kSetBuffers_CmdType;
		cmd->size = size;
		cmd->chSize = chSize;
		cmd->isClip = isClip;
	}

	void GLC_CmdPack::drawBuffers(GLsizei num, const GLenum buffers[2]) {
		auto cmd = new(_this->allocCmd(sizeof(DrawBuffersCmd))) DrawBuffersCmd;
		cmd->type = kDrawBuffers_CmdType;
		cmd->num = num;
		cmd->buffers[0] = buffers[0];
		cmd->buffers[1] = buffers[1];
	}

#else
	GLC_CmdPack::GLC_CmdPack(GLRender *render, GLCanvas *canvas)
		: _render(render), _canvas(canvas), _cache(canvas->getPathvCache())
		, lastCmd(nullptr), _chMatrix(true)
	{}
	GLC_CmdPack::~GLC_CmdPack() {
	}
	bool GLC_CmdPack::isHaveCmds() {
		return false;
	}
	void GLC_CmdPack::flush() {
	}
	void GLC_CmdPack::setMetrix() {
		_this->setMatrixCall(_canvas->_state->matrix);
	}
	void GLC_CmdPack::gl_set_blend_mode(BlendMode mode) {
		_render->gl_set_blend_mode(mode);
	}
	void GLC_CmdPack::switchState(GLenum id, bool isEnable) {
		_this->switchStateCall(id, isEnable);
	}
	void GLC_CmdPack::drawColor(const VertexData &vertex, const Color4f &color, bool aafuzz) {
		_this->drawColorCall(vertex, color, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawRRectBlurColor(const Rect& rect, const float *radius, float blur, const Color4f &color) {
		_this->drawRRectBlurColorCall(rect, radius, blur, color, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawImage(const VertexData &vertex, const ImagePaint *paint, float alpha, bool aafuzz) {
		_this->drawImageCall(vertex, paint, alpha, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawImageMask(const VertexData &vertex, const ImagePaint *paint, const Color4f &color, bool aafuzz) {
		_this->drawImageMaskCall(vertex, paint, color, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawGradient(const VertexData &vertex, const GradientPaint *paint, float alpha, bool aafuzz) {
		_this->drawGradientCall(vertex, paint, alpha, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawClip(const GLC_State::Clip &clip, uint32_t ref, bool revoke) {
		_this->drawClipCall(clip, ref, revoke, _canvas->_zDepth);
	}
	void GLC_CmdPack::clearColor(const Color4f &color, const Region &region, bool full) {
		_this->clearColorCall(color, region, full, _canvas->_zDepth);
	}
	void GLC_CmdPack::blurFilterBegin(Region bounds) {
		_this->blurFilterBeginCall(bounds, _canvas->_isClipState, _canvas->_zDepth);
	}
	int GLC_CmdPack::blurFilterEnd(Region bounds, float size, ImageSource* output) {
		int n,lod;
		_this->getBlurSampling(size, n, lod);
		_this->blurFilterEndCall(bounds, size, output,
			_canvas->_blendMode, n, lod, _canvas->_isClipState, _canvas->_zDepth
		);
		return lod + 2;
	}
	void GLC_CmdPack::readImage(const Rect &src, ImageSource* img, bool isMipmap) {
		_this->readImageCall(src, img, isMipmap,
			_canvas->_size, _canvas->_surfaceSize, _canvas->_zDepth);
	}
	void GLC_CmdPack::outputImageBegin(ImageSource* img, bool isMipmap) {
		_this->outputImageBeginCall(img, isMipmap);
	}
	void GLC_CmdPack::outputImageEnd(ImageSource* img, bool isMipmap) {
		_this->outputImageEndCall(img, isMipmap);
	}
	void GLC_CmdPack::setBuffers(Vec2 size, bool chSize, bool isClip) {
		_this->setBuffersCall(size, chSize, isClip);
	}
	void GLC_CmdPack::drawBuffers(GLsizei num, const GLenum buffers[2]) {
		_this->drawBuffersCall(num, buffers);
	}
#endif

}
