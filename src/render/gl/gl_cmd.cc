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


#include "./gl_cmd.h"
#include "./gl_render.h"
#include "./gl_canvas.h"

#define Qk_CGCmd_Option_Capacity 256
#define Qk_CGCmd_VertexBlock_Capacity 6555
#define Qk_CGCmd_OptBlock_Capacity 2048
#define Qk_CGCmd_CmdBlock_Capacity 65536
//#define Qk_useShaderProgram(shader, vertex) if (!useShaderProgram(shader, vertex)) return
#define Qk_useShaderProgram(shader, vertex) useShaderProgram(shader, vertex)

namespace qk {
	extern const float aa_fuzz_weight;
	extern const float zDepthNextUnit;
	void  gl_texture_barrier();
	void  gl_set_framebuffer_renderbuffer(GLuint b, Vec2 s, GLenum f, GLenum at);
	GLint gl_get_texture_internalformat(ColorType type);
	GLint gl_get_texture_format(ColorType type);
	GLint gl_get_texture_data_type(ColorType format);
	void setTex_SourceImage(RenderResource *res, ImageSource* s, cPixelInfo &i, const TexStat *tex);
	void gl_set_aaclip_buffer(GLuint tex, Vec2 size);
	void gl_set_tex_renderbuffer(GLuint tex, Vec2 size);
	void gl_set_texture_no_repeat(GLenum wrapdir);
	TexStat* gl_new_tex_stat();
	uint32_t alignUp(uint32_t ptr, uint32_t alignment = alignof(void*));

	constexpr float whiteColor[] = {1.0f,1.0f,1.0f,1.0f};
	constexpr float emptyColor[] = {0.0f,0.0f,0.0f,0.0f};
	constexpr GLenum DrawBuffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	struct PaintImageLock {
		inline PaintImageLock(const PaintImage *p): paint(p) {
			if (!paint->_isCanvas)
				paint->image->onState().lockShared();
		}
		inline ~PaintImageLock() {
			if (!paint->_isCanvas)
				paint->image->onState().unlockShared();
		}
		const PaintImage *paint;
	};

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
					cmd = (Cmd*)(((char*)cmd) + cmd->size);
				}
				i.size = 0;
			}
			_cmds.index = 0;
			_cmds.current = _cmds.blocks.val();
			_lastCmd = _cmds.current->val;
		}

		Cmd* allocCmd(uint32_t size) {
			Qk_ASSERT_EQ(alignUp(size), size); // Check if it is aligned
			auto cmds = _cmds.current;
			auto newSize = cmds->size + size;
			if (newSize > cmds->capacity) {
				if (++_cmds.index == _cmds.blocks.length()) {
					_cmds.blocks.push({ // init, alloc 64k memory
						(Cmd*)malloc(Qk_CGCmd_CmdBlock_Capacity),0,Qk_CGCmd_CmdBlock_Capacity
					});
				}
				cmds = _cmds.blocks.val() + _cmds.index;
				newSize = size;
				_cmds.current = cmds;
			}
			_lastCmd = (Cmd*)(((char*)cmds->val) + cmds->size);
			_lastCmd->size = size;
			cmds->size = newSize;
			return _lastCmd;
		}

		ColorsCmd* newColorsCmd() {
			auto cmd = (ColorsCmd*)allocCmd(sizeof(ColorsCmd));
			cmd->type = kColors_CmdType;

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

		ColorsCmd* getMColorCmd() {
			auto cmd = (ColorsCmd*)_lastCmd;
			if (cmd->type == kColors_CmdType) {
				if (_vertexBlocks.current->size != Qk_CGCmd_VertexBlock_Capacity &&
						_optionBlocks.current->size != Qk_CGCmd_OptBlock_Capacity &&
						cmd->subcmd != Qk_CGCmd_Option_Capacity
				) {
					return cmd;
				}
			}
			return newColorsCmd();
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
							blurFilterBeginCall(c->bounds, c->size, c->isClipState, c->depth);
							break;
						}
						case kBlurFilterEnd_CmdType: {
							auto c = (BlurFilterEndCmd*)cmd;
							blurFilterEndCall(c->bounds, c->size,
								*c->recover, c->backMode, c->n, c->lod, c->isClipState, c->depth);
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
							clearColorCall(c->color, c->range, c->fullClear, c->depth);
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
							drawColorCall(c->vertex, c->color, c->aafuzz, c->aaclip, c->depth);
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
							drawImageCall(c->vertex, &c->paint, c->allScale, c->color, c->aafuzz, c->aaclip, c->depth);
							c->~ImageCmd();
							break;
						}
						case kImageMask_CmdType: {
							auto c = (ImageMaskCmd*)cmd;
							drawImageMaskCall(c->vertex, &c->paint, c->allScale, c->color, c->aafuzz, c->aaclip, c->depth);
							c->~ImageMaskCmd();
							break;
						}
						case kSDFImageMask_CmdType: {
							auto c = (SDFImageMaskCmd*)cmd;
							drawSDFImageMaskCall(c->vertex, &c->paint, c->color, c->strokeColor,
									c->strokeWidth, c->aafuzz, c->aaclip, c->depth);
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
							drawGradientCall(c->vertex, &c->paint, c->color, c->aafuzz, c->aaclip, c->depth);
							c->~GradientCmd();
							break;
						}
						case kColors_CmdType: {
							auto c = (ColorsCmd*)cmd;
							auto s = c->aaclip ? &_render->_shaders.colorBatch_AACLIP: &_render->_shaders.colorBatch;
							glBindBuffer(GL_UNIFORM_BUFFER, _render->_optsBlock);
							glBufferData(GL_UNIFORM_BUFFER, sizeof(ColorsCmd::Option) * c->subcmd, c->opts,
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
							readImageCall(c->src, *c->img, c->canvasSize, c->surfaceSize, c->depth);
							c->~ReadImageCmd();
							break;
						}
						case kOutputImageBegin_CmdType: {
							auto c = (OutputImageBeginCmd*)cmd;
							outputImageBeginCall(*c->img);
							c->~OutputImageBeginCmd();
							break;
						}
						case kOutputImageEnd_CmdType: {
							auto c = (OutputImageEndCmd*)cmd;
							outputImageEndCall(*c->img);
							c->~OutputImageEndCmd();
							break;
						}
						case kFlushCanvas_CmdType: {
							auto c = (FlushCanvasCmd*)cmd;
							flushCanvasCall(c->srcC, c->srcCmd, c->root, c->mat, c->mode, root,
								curMat ? curMat->matrix: mat);
							c->~FlushCanvasCmd();
							break;
						}
						case kSetBuffers_CmdType: {
							auto c = (SetBuffersCmd*)cmd;
							setBuffersCall(c->size, c->recover.get(), c->chSize);
							c->~SetBuffersCmd();
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

		void useShaderProgram(GLSLShader *shader, const VertexData &vertex) {
			if (_cache->newVertexData(vertex.id)) {
				glBindVertexArray(vertex.id->vao); // use vao
				glUseProgram(shader->shader); // use shader program
			} else /*if (vertex.vertex.length())*/ {
				// copy vertex data to gpu and use shader
				Qk_ASSERT_EQ(vertex.vertex.length(), vertex.vCount, "useShaderProgram, vertex vCount != vertex.vertex.size()");
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
			Qk_useShaderProgram(s, vertex);
			glUniform1f(s->depth, depth);
			glUniform4fv(s->color, 1, color.val);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
		}

		void drawRRectBlurColorCall(const Rect& rect,
			const float *r, float s, const Color4f &color, bool aaclip, float depth
		) {
			s = Qk_Max(s, 0.5);
			float s1 = s * 1.15, s2 = s * 2.0;
			auto sh = aaclip ?
				&_render->_shaders.colorRrectBlur_AACLIP: &_render->_shaders.colorRrectBlur;
			float min_edge = Qk_Min(rect.size[0],rect.size[1]); // min w or h
			float rmax = 0.5 * min_edge; // max r
			Vec2 size = rect.size * 0.5;
			Vec2 c = rect.begin + size; // rect center
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
				glUniform3f(sh->consts, r1, n, 1.0/n);
				glUniform2f(sh->horn, p[0], p[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		bool setTextureSlot0(const PaintImage *paint) {
			if (paint->_isCanvas) { // flush canvas to current canvas
				auto srcC = static_cast<GLCanvas*>(paint->canvas);
				if (srcC != _canvas && srcC->isGpu()) { // now only supported gpu
					if (srcC->_render == _render) {
						if (srcC->_outTex) {
							_render->gl_set_texture_param(srcC->_outTex, 0, paint);
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
			const PaintImage *paint, float allScale, const Color4f &color, bool aafuzz, bool aaclip, float depth
		) {
			PaintImageLock lock(paint);
			if (setTextureSlot0(paint)) { // rgb or y
				GLSLImage *s;
				auto src = paint->image;

				if (kYUV420P_Y_8_ColorType == src->type()) { // yuv420p or yuv420sp
					auto yuv = aaclip ? &_render->_shaders.imageYuv_AACLIP: &_render->_shaders.imageYuv;
					s = (GLSLImage*)yuv;
					Qk_useShaderProgram(s, vertex);

					if (src->pixel(1)->type() == kYUV420P_U_8_ColorType) {
						_render->gl_set_texture(src, 2, paint); // v
						glUniform1i(yuv->format, 1); // yuv420p
					} else {
						glUniform1i(yuv->format, 0); // yuv420sp
					}
					_render->gl_set_texture(src, 1, paint); // u or uv
				} else {
					s = aaclip ? &_render->_shaders.image_AACLIP: &_render->_shaders.image;
					Qk_useShaderProgram(s, vertex);
				}
				glUniform1f(s->depth, depth);
				glUniform1f(s->allScale, allScale);
				glUniform4fv(s->color, 1, color.val);
				glUniform4fv(s->texCoords, 1, paint->coord.begin.val);
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawImageMaskCall(const VertexData &vertex,
			const PaintImage *paint, float allScale, const Color4f &color, bool aafuzz, bool aaclip, float depth
		) {
			PaintImageLock lock(paint);
			if (setTextureSlot0(paint)) {
				auto type = paint->image->type();
				auto s = aaclip ? &_render->_shaders.imageMask_AACLIP: &_render->_shaders.imageMask;
				Qk_useShaderProgram(s, vertex);
				glUniform1f(s->depth, depth);
				glUniform1i(s->alphaIndex, type == kAlpha_8_ColorType ? 0 :
						type == kLuminance_Alpha_88_ColorType ? 1 : 3); // alpha index
				glUniform1f(s->allScale, allScale);
				glUniform4fv(s->color, 1, color.val);
				glUniform4fv(s->texCoords, 1, paint->coord.begin.val);
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawSDFImageMaskCall(const VertexData &vertex,
				const PaintImage *paint, const Color4f &color, const Color4f &strokeColor,
				float stroke, bool aafuzz, bool aaclip, float depth) {
			PaintImageLock lock(paint);
			if (setTextureSlot0(paint)) {
				auto s = aaclip ? &_render->_shaders.imageSdfMask_AACLIP: &_render->_shaders.imageSdfMask;
				Qk_useShaderProgram(s, vertex);
				glUniform1f(s->depth, depth);
				glUniform4fv(s->color, 1, color.val);
				glUniform4fv(s->strokeColor, 1, stroke <= 0 ? color.val: strokeColor.val);
				glUniform1f(s->strokeWidth, stroke);
				glUniform4fv(s->texCoords, 1, paint->coord.begin.val);
				glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			}
		}

		void drawTrianglesCall(const Triangles &triangles, const PaintImage *paint, const Color4f &color, bool aaclip, float depth) {
			PaintImageLock lock(paint);
			if (setTextureSlot0(paint)) {
				auto isPre = paint->image->premultipliedAlpha();
				auto s = triangles.isDarkColor ?
					aaclip ? &_render->_shaders.triangles_DARK_COLOR_AACLIP: &_render->_shaders.triangles_DARK_COLOR:
					aaclip ? &_render->_shaders.triangles_AACLIP: &_render->_shaders.triangles;
				Qk_ASSERT_EQ(triangles.indexCount % 3, 0);
				s->use(triangles.vertCount * sizeof(V3F_T2F_C4B_C4B), triangles.verts);
				glUniform1f(s->depth, depth);
				glUniform4fv(s->color, 1, color.val);
				// glUniform1f(s->premultipliedAlpha, isPre ? 1.0f : 0.0f);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _render->_ebo); // restore ebo
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * triangles.indexCount, triangles.indices, GL_DYNAMIC_DRAW);
				glDrawElements(GL_TRIANGLES, triangles.indexCount, GL_UNSIGNED_SHORT, 0);
			}
		}

		void drawGradientCall(const VertexData &vertex, 
			const PaintGradient *paint, const Color4f &color, bool aafuzz, bool aaclip, float depth
		) {
			GLSLColorRadial *s;
			auto count = paint->count;

			if (paint->type == PaintGradient::kRadial_Type) {
				s = count == 2 ?
					aaclip ? &_render->_shaders.colorRadial_COUNT2_AACLIP: &_render->_shaders.colorRadial_COUNT2:
					aaclip ? &_render->_shaders.colorRadial_AACLIP: &_render->_shaders.colorRadial;
			} else {
				s = (GLSLColorRadial*)(count == 2 ?
					aaclip ? &_render->_shaders.colorLinear_COUNT2_AACLIP: &_render->_shaders.colorLinear_COUNT2:
					aaclip ? &_render->_shaders.colorLinear_AACLIP: &_render->_shaders.colorLinear);
			}

			Qk_useShaderProgram(s, vertex);
			glUniform1f(s->depth, depth);
			glUniform4fv(s->color, 1, color.val);
			glUniform4fv(s->range, 1, paint->origin.val);
			glUniform1i(s->count, count);
			glUniform4fv(s->colors, count, (const GLfloat*)paint->colors);
			glUniform1fv(s->positions, count, (const GLfloat*)paint->positions);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
			//glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex.length());
			//glDrawArrays(GL_LINES, 0, vertex.length());
		}

		void setColorbuffer(bool aaclip, ImageSource* elseOut) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				aaclip ? _canvas->_outAAClipTex: elseOut ? elseOut->texture(0)->id: _canvas->_outTex->id,
				0
			);
			// glDrawBuffers(1, DrawBuffers + (aaclip ? 1: 0));
		}

		void aaClipExec(float depth, const VertexData &vertex, ImageSource *recover,
			bool revoke, float W, float C, bool clearAA) 
		{
			auto _c = _canvas;
			auto _render = _c->_render;
			auto chMode = _render->_blendMode;
			if (!_c->_outAAClipTex) {
				glGenTextures(1, &_c->_outAAClipTex); // gen aaclip buffer tex
				gl_set_aaclip_buffer(_c->_outAAClipTex, _c->_surfaceSize);
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
			auto shader = revoke ? &_render->_shaders.aaclip_AACLIP_REVOKE: &_render->_shaders.aaclip;
			float aafuzzWeight = W * 0.1f;
			shader->use(vertex.vertex.size(), vertex.vertex.val());
			glUniform1f(shader->depth, depth);
			glUniform1f(shader->aafuzzWeight, aafuzzWeight); // Difference: -0.09
			glUniform1f(shader->aafuzzConst, C + 0.9f/aafuzzWeight); // C' = C + C1/W, Difference: -11
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount); // draw test
			setColorbuffer(false, recover);
			_render->gl_set_blend_mode(chMode); // revoke blend mode
		}

		void clipExec(float depth, const VertexData &vertex) {
			if (vertex.vCount == 0) return;
			auto shader = &_render->_shaders.color;
			shader->use(vertex.vertex.size(), vertex.vertex.val());
			glUniform4fv(shader->color, 1, emptyColor); // not output color buffer
			glUniform1f(shader->depth, depth);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount); // draw test
		}

		void drawClipCall(const GLC_State::Clip &clip, uint32_t ref, ImageSource *recover, bool revoke, float depth) {
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

		void clearColorCall(const Color4f &color, const Range &region, bool full, float depth) {
			if (full) {
				//glClearBufferfv(GL_DEPTH, 0, &depth); // depth = 0
				glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, 127); // depth=0, stencil = 127
				//glClearColor(color.r(), color.g(), color.b(), color.a());
				glClearBufferfv(GL_COLOR, 0, color.val); // clear GL_COLOR_ATTACHMENT0
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			} else {
				float x1 = region.begin.x(), y1 = region.begin.y();
				float x2 = region.end.x(), y2 = region.end.y();
				float data[] = {
					x1,y1,0,/*left top*/
					x2,y1,0,/*right top*/
					x1,y2,0, /*left bottom*/
					x2,y2,0, /*right bottom*/
				};
				_render->_shaders.color.use(sizeof(float) * 12, data);
				glUniform1f(_render->_shaders.color.depth, depth);
				glUniform4fv(_render->_shaders.color.color, 1, color.val);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		void clearRegion(const Range &region, float scale, float offsetY, float depth) {
			auto origin = region.begin * scale;
			auto end = region.end * scale;
			clearColorCall({0,0,0,0}, {
				{origin.x(), origin.y() + offsetY},
				{end.x(),    end.y()    + offsetY}
			}, false, depth);
		}

		void getBlurSampling(float size, int &n, int &lod) {
			const int N[] = { 3,3,3,3, 7,7,7,7, 13,13,13,13,13,13, 19,19,19,19,19,19 };
			size *= _canvas->_surfaceScale;
			n = ceilf(size); // sampling rate
			n = N[Qk_Min(n,19)];
			lod = ceilf(Float32::max(0,log2f(size/n)));
			// Qk_DLog("getBlurSampling %d, lod: %d", n, lod);
		}

		void blurFilterBeginCall(Range bounds, float size, bool isClipState, float depth) {
			if (!_canvas->_outA) {
				glGenTextures(1, &_canvas->_outA); // ready the blur buffer
				gl_set_tex_renderbuffer(_canvas->_outA, _canvas->_surfaceSize);
			}
			if (!_canvas->_outB) {
				glGenTextures(1, &_canvas->_outB); // ready the blur buffer
				gl_set_tex_renderbuffer(_canvas->_outB, _canvas->_surfaceSize);
			}
			if (_render->_blendMode != kSrc_BlendMode) {
				_render->gl_set_blend_mode(kSrc_BlendMode); // switch blend mode to src
			}
			if (isClipState) {
				glDisable(GL_STENCIL_TEST); // close clip
			}
			// output to texture buffer then do post processing
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_outA, 0);
			// clear pixels within bounds
			// clear more the x-axis regions, easy for fuzzy sampling without contamination,
			// but ignore the y-axis blurred regions
			/*
			|//////////////|
			|.|.| body |.|.|
			|//////////////|
			*/
			clearColorCall({0,0,0,0}, {
				{bounds.begin.x() - size, bounds.begin.y()},
				{bounds.end.x() + size, bounds.end.y()},
			}, false, depth);
			// glClearBufferfv(GL_COLOR, 0, emptyColor);

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
		void blurFilterEndCall(Range bounds, float size,
			ImageSource* recover, BlendMode backMode, int n, int lod, bool isClipState, float depth)
		{
			auto _c = _canvas;
			float x1 = bounds.begin.x(), y1 = bounds.begin.y() + size;
			float x2 = bounds.end.x(), y2 = bounds.end.y() - size;
			auto fullsize = size * _c->_surfaceScale;
			Vec2 R = _c->_surfaceSize;
			uint32_t oRw = R.x(), oRh = R.y();

			glActiveTexture(GL_TEXTURE0);
			glBindSampler(0, 0);
			glBindTexture(GL_TEXTURE_2D, _c->_outA);

			if (lod) { // copy image, gen mipmap texture
				if (isClipState)
					glDisable(GL_STENCIL_TEST); // close clip
				/* Copy more the x-axis regions, but ignore the y-axis blurred regions
				|//////////////|
				|.|.| body |.|.|
				|//////////////|
				*/
				int level = 0;
				auto &cp = _render->_shaders.vportCp;
				float x1_ = x1 - size, x2_ = x2 + size;
				float vertex[] = { x1_,y1,0, x2_,y1,0, x1_,y2,0, x2_,y2,0 };
				cp.use(sizeof(float) * 12, vertex);
				glUniform2f(cp.iResolution, R.x(), R.y());
				glUniform4f(cp.coord, 0, 0, 1, 1);
				do { // copy image level
					oRw >>= 1; oRh >>= 1;
					glUniform1f(cp.depth, depth);
					glUniform1f(cp.imageLod, level++);
					glUniform2f(cp.oResolution, oRw, oRh);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _c->_outA, level);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					depth += zDepthNextUnit;
				} while(level < lod && oRw && oRh);
			}

			// Setting target buffer B and flush blur texture buffer A
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _c->_outB, lod);

			// Choosing the right blur shader
			auto blur = &_render->_shaders.blur;
			/*switch (n) {
				case 3: blur += 1; break; // blur3
				case 7: blur += 2; break; // blur7
				case 13: blur += 3; break; // blur13
				case 19: blur += 4; break; // blur19
			}*/
			//Qk_DLog("blurFilterEndCall, n=%d", n);
			//Qk_DLog("size > lod, %f > %d", size, lod);

			/* The x-axis regions
			|//////////////|
			|/|.| body |.|/|
			|//////////////|
			*/
			float vertex_x[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			/* The y-axis regions
			|/|/|//////|/|/|
			|/|.|......|.|/|
			|/|.| body |.|/|
			|/|.|......|.|/|
			|/|/|//////|/|/|
			*/
			float y1_ = y1 - size, y2_ = y2 + size;
			float vertex_y[] = { x1,y1_,0, x2,y1_,0, x1,y2_,0, x2,y2_,0 };
			float oiScale = oRw / R.x(); // oResolution / iResolution
			float offsetY = (R.y() - oRh) / _c->_surfaceScale;
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
			y1_-=size; y2_+=size;
			clearRegion({{x1, y1_}, {x2, y1}}, oiScale, offsetY, depth); // clear top
			clearRegion({{x1, y2}, {x2, y2_}}, oiScale, offsetY, depth); // clear bottom
			clearRegion({{x1-3, y1_}, {x1, y2_}}, oiScale, offsetY, depth); // clear left
			clearRegion({{x2, y1_}, {x2+3, y2_}}, oiScale, offsetY, depth); // clear right
			// Making blur of the x-axis direction
			blur->use(sizeof(float) * 12, vertex_x);
			glUniform1f(blur->depth, depth);
			glUniform2f(blur->iResolution, R.x(), R.y());
			glUniform2f(blur->oResolution, oRw, oRh);
			glUniform1f(blur->imageLod, lod);
			glUniform1f(blur->detail, 1.0f/(n-1));
			glUniform2f(blur->size, fullsize / R.x(), 0); // horizontal blur
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur

			if (lod && isClipState) {
				glEnable(GL_STENCIL_TEST); // close clip
			}
			//!< r = s + (1-sa)*d
			//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			if (backMode != kSrc_BlendMode) {
				_render->gl_set_blend_mode(backMode); // restore blend mode
			}

			// recover output target
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				recover ? recover->texture(0)->id: _c->_outTex->id, 0
			);

			glBindTexture(GL_TEXTURE_2D, _c->_outB);

			// Making blur of the y-axis direction
			blur->use(sizeof(float) * 12, vertex_y);
			glUniform1f(blur->depth, depth + zDepthNextUnit);
			glUniform2f(blur->oResolution, R.x(), R.y());
			glUniform2f(blur->size, 0, fullsize / R.y()); // vertical blur
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur to main render buffer
		}

		void readImageCall(const Rect &src, ImageSource* img, Vec2 canvasSize, Vec2 surfaceSize, float depth) {
			auto tex = img->texture(0);
			auto o = src.begin, s = src.size;
			auto w = img->width(), h = img->height();
			auto iformat = gl_get_texture_internalformat(img->type());
			auto format = gl_get_texture_format(img->type());
			auto type = gl_get_texture_data_type(img->type());

			if (!tex) {
				tex = gl_new_tex_stat();
			}
			glActiveTexture(GL_TEXTURE0);
			glBindSampler(0, 0);
			glBindTexture(GL_TEXTURE_2D, tex->id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, w, h, 0, format, type, nullptr);

			// if (_canvas->_outTex) {
			Qk_ASSERT(_canvas->_outTex);
			float x2 = canvasSize[0], y2 = canvasSize[1];
			float data[] = { 0,0,0, x2,0,0, 0,y2,0, x2,y2,0 };
			auto &cp = _render->_shaders.vportCp;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->id, 0);
#if Qk_LINUX
			glClearBufferfv(GL_COLOR, 0, emptyColor); // clear image tex
#endif
			glBindTexture(GL_TEXTURE_2D, _canvas->_outTex->id); // read image source
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
			glUniform1f(cp.imageLod, 0);
			o /= surfaceSize; s /= surfaceSize;
			glUniform4f(cp.coord, o[0], o[1], s[0], s[1]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_outTex->id, 0);
			glBindTexture(GL_TEXTURE_2D, tex->id);
			// } else {
			// glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _render->_fbo);
			// glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->id, 0);
			// glBlitFramebuffer(o[0], o[1], o[0]+s[0], o[1]+s[1],
			// 	0, 0, w, h, GL_COLOR_BUFFER_BIT, s == Vec2(w,h) ? GL_NEAREST: GL_LINEAR);
			// glBindFramebuffer(GL_FRAMEBUFFER, _canvas->_fbo);
			// }
			if (img->isMipmap()) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			setTex_SourceImage(_render, img, img->info(), tex);
		}

		void outputImageBeginCall(ImageSource* img) {
			auto tex = img->texture(0);
			auto size = _canvas->_surfaceSize;
			auto iformat = gl_get_texture_internalformat(img->type());
			auto format = gl_get_texture_format(img->type());
			auto type = gl_get_texture_data_type(img->type());
			if (!tex) {
				tex = gl_new_tex_stat();
			}
			glActiveTexture(GL_TEXTURE0);
			glBindSampler(0, 0);
			glBindTexture(GL_TEXTURE_2D, tex->id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, size[0], size[1], 0, format, type, nullptr);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->id, 0);
#if Qk_LINUX
			glClearBufferfv(GL_COLOR, 0, emptyColor); // clear image tex
#endif
			setTex_SourceImage(_render, img, {
				int(size[0]),int(size[1]),img->type(),img->info().alphaType()
			}, tex);
		}

		void outputImageEndCall(ImageSource* img) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_outTex->id, 0);
			//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _canvas->_outColor);
			if (img->isMipmap()) {
				glActiveTexture(GL_TEXTURE0);
				glBindSampler(0, 0);
				glBindTexture(GL_TEXTURE_2D, img->texture(0)->id);
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
				glBindSampler(0, 0);
				glBindTexture(GL_TEXTURE_2D, srcC->_outTex->id);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		void flushCanvas(const PaintImage *paint) {
#if Qk_USE_GLC_CMD_QUEUE
			auto srcC = static_cast<GLCanvas*>(paint->canvas);
			if (!paint->_isCanvas || !srcC->isGpu())
				return; // now only supported gpu
			if (srcC->_render != _render || srcC == _canvas)
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

		void setBuffersCall(Vec2 size, ImageSource *recover, bool chSize) {
			auto _c = _canvas;
			auto w = size.x(), h = size.y();
			auto type = _c->_opts.colorType;

			Qk_ASSERT(w, "Invalid viewport size width");
			Qk_ASSERT(h, "Invalid viewport size height");

			if (chSize) {
				_c->setBuffers(size);
			}
			// update shader root matrix and clear all save state
			if (_render->_glcanvas == _c) { // main canvas
				glViewport(0, 0, size[0], size[1]);
				// init root matrix buffer
				setRootMatrixCall(_c->_rootMatrix);
				// init matrix buffer
				setMatrixCall(_c->_state->matrix);
			}
			if (_c->_outAAClipTex) { // clear aa clip tex buffer
				setColorbuffer(true, nullptr);
				glClearBufferfv(GL_COLOR, 0, whiteColor); // clear GL_COLOR_ATTACHMENT0
				// ensure clip texture clear can be executed correctly in sequence
				setColorbuffer(false, recover);
			}
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
	}

	GLC_CmdPack::FlushCanvasCmd::~FlushCanvasCmd() {
		srcC->release();
		delete srcCmd; // delete cmd pack
	}

#if Qk_USE_GLC_CMD_QUEUE

	bool GLC_CmdPack::isHaveCmds() {
		return _lastCmd->type != kEmpty_CmdType;
	}

	GLC_CmdPack::GLC_CmdPack(GLRender *render, GLCanvas *canvas)
		: _render(render), _canvas(canvas), _cache(canvas->getPathvCache())
		, _lastCmd(nullptr)
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

	void GLC_CmdPack::setMatrix() {
		auto cmd = (MatrixCmd*)_this->allocCmd(sizeof(MatrixCmd));
		cmd->type = kMatrix_CmdType;
		cmd->matrix = _canvas->_state->matrix;
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
#define Qk_USE_Colors (!Qk_LINUX)
#if Qk_USE_Colors
		if ( vertex.vertex.length() == 0 ) { // Maybe it's already cached
#endif
			auto cmd = new(_this->allocCmd(sizeof(ColorCmd))) ColorCmd;
			cmd->type = kColor_CmdType;
			cmd->vertex = vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->aafuzz = aafuzz;
			cmd->aaclip = _canvas->_state->aaclip;
			cmd->color = color;
#if Qk_USE_Colors
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

	void GLC_CmdPack::drawImage(const VertexData &vertex, const PaintImage *paint, const Color4f& color, bool aafuzz) {
		_this->flushCanvas(paint);
		auto cmd = new(_this->allocCmd(sizeof(ImageCmd))) ImageCmd;
		cmd->type = kImage_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aafuzz = aafuzz;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->allScale = _canvas->_allScale;
		cmd->color = color;
		cmd->paint = *paint;
		paint->image->retain(); // retain source image ref
	}

	void GLC_CmdPack::drawImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color, bool aafuzz) {
		_this->flushCanvas(paint);
		auto cmd = new(_this->allocCmd(sizeof(ImageMaskCmd))) ImageMaskCmd;
		cmd->type = kImageMask_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aafuzz = aafuzz;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->allScale = _canvas->_allScale;
		cmd->color = color;
		cmd->paint = *paint;
		paint->image->retain(); // retain source image ref
	}

	void GLC_CmdPack::drawSDFImageMask(const VertexData &vertex, const PaintImage *paint,
		const Color4f &color, const Color4f &strokeColor, float stroke, bool aafuzz) {
		auto cmd = new(_this->allocCmd(sizeof(SDFImageMaskCmd))) SDFImageMaskCmd;
		cmd->type = kSDFImageMask_CmdType;
		cmd->vertex = vertex;
		cmd->depth = _canvas->_zDepth;
		cmd->aafuzz = aafuzz;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->allScale = _canvas->_allScale;
		cmd->color = color;
		cmd->paint = *paint;
		cmd->strokeColor = strokeColor;
		cmd->strokeWidth = stroke;
		paint->image->retain();
	}

	void GLC_CmdPack::drawTriangles(const Triangles& triangles, const PaintImage *paint, const Color4f &color) {
		_this->flushCanvas(paint);
		auto cmd = new(_this->allocCmd(sizeof(TrianglesCmd))) TrianglesCmd;
		cmd->type = kTriangles_CmdType;
		cmd->triangles = triangles;
		cmd->depth = _canvas->_zDepth;
		cmd->paint = *paint;
		cmd->color = color;
		cmd->aaclip = _canvas->_state->aaclip;
		paint->image->retain();
	}

	void GLC_CmdPack::drawGradient(const VertexData &vertex, const PaintGradient *paint, const Color4f &color, bool aafuzz) {
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
		cmd->aafuzz = aafuzz;
		cmd->aaclip = _canvas->_state->aaclip;
		cmd->color = color;
		cmd->paint = *paint;
		cmd->paint.colors = colors;
		cmd->paint.positions = positions;
	}

	void GLC_CmdPack::drawClip(const GLC_State::Clip &clip, uint32_t ref, ImageSource *recover, bool revoke) {
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

	void GLC_CmdPack::clearColor(const Color4f &color, const Range &region, bool full) {
		auto cmd = new(_this->allocCmd(sizeof(ClearCmd))) ClearCmd;
		cmd->type = kClear_CmdType;
		cmd->color = color;
		cmd->range = region;
		cmd->depth = _canvas->_zDepth;
		cmd->fullClear = full;
	}

	void GLC_CmdPack::blurFilterBegin(Range bounds, float size) {
		auto cmd = new(_this->allocCmd(sizeof(BlurFilterBeginCmd))) BlurFilterBeginCmd;
		cmd->type = kBlurFilterBegin_CmdType;
		cmd->bounds = bounds;
		cmd->size = size;
		cmd->depth = _canvas->_zDepth;
		cmd->isClipState = _canvas->_isClipState;
	}

	int GLC_CmdPack::blurFilterEnd(Range bounds, float size, ImageSource *recover) {
		auto cmd = new(_this->allocCmd(sizeof(BlurFilterEndCmd))) BlurFilterEndCmd;
		cmd->type = kBlurFilterEnd_CmdType;
		cmd->bounds = bounds;
		cmd->depth = _canvas->_zDepth;
		cmd->backMode = _canvas->_blendMode;
		cmd->size = size;
		cmd->isClipState = _canvas->_isClipState;
		cmd->recover = recover;
		_this->getBlurSampling(size, cmd->n, cmd->lod);
		return cmd->lod + 2;
	}

	void GLC_CmdPack::readImage(const Rect &src, ImageSource* img) {
		auto cmd = new(_this->allocCmd(sizeof(ReadImageCmd))) ReadImageCmd;
		cmd->type = kReadImage_CmdType;
		cmd->src = src;
		cmd->img = img;
		cmd->depth = _canvas->_zDepth;
		cmd->canvasSize = _canvas->_size;
		cmd->surfaceSize = _canvas->_surfaceSize;
	}

	void GLC_CmdPack::outputImageBegin(ImageSource* img) {
		auto cmd = new(_this->allocCmd(sizeof(OutputImageBeginCmd))) OutputImageBeginCmd;
		cmd->type = kOutputImageBegin_CmdType;
		cmd->img = img;
	}

	void GLC_CmdPack::outputImageEnd(ImageSource* img) {
		auto cmd = new(_this->allocCmd(sizeof(OutputImageEndCmd))) OutputImageEndCmd;
		cmd->type = kOutputImageEnd_CmdType;
		cmd->img = img;
	}

	void GLC_CmdPack::setBuffers(Vec2 size, ImageSource *recover, bool chSize) {
		auto cmd = new(_this->allocCmd(sizeof(SetBuffersCmd))) SetBuffersCmd;
		cmd->type = kSetBuffers_CmdType;
		cmd->size = size;
		cmd->chSize = chSize;
		cmd->recover = recover;
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
		, _lastCmd(nullptr)
	{}
	GLC_CmdPack::~GLC_CmdPack() {
	}
	bool GLC_CmdPack::isHaveCmds() {
		return false;
	}
	void GLC_CmdPack::flush() {
	}
	void GLC_CmdPack::setMatrix() {
		_this->setMatrixCall(_canvas->_state->matrix);
	}
	void GLC_CmdPack::setBlendMode(BlendMode mode) {
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
	void GLC_CmdPack::drawImage(const VertexData &vertex, const PaintImage *paint, const Color4f &color, bool aafuzz) {
		_this->drawImageCall(vertex, paint, _canvas->_allScale, color, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color, bool aafuzz) {
		_this->drawImageMaskCall(vertex, paint, _canvas->_allScale, color, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}

	void GLC_CmdPack::drawSDFImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
		const Color4f &strokeColor, float stroke, bool aafuzz) {
		_this->drawSDFImageMaskCall(vertex, paint, color, strokeColor, stroke, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}

	void GLC_CmdPack::drawTriangles(const Triangles &triangles, const PaintImage *paint, const Color4f &color) {
		_this->drawTrianglesCall(triangles, paint, color, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawGradient(const VertexData &vertex, const PaintGradient *paint, const Color4f &color, bool aafuzz) {
		_this->drawGradientCall(vertex, paint, color, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
	}
	void GLC_CmdPack::drawClip(const GLC_State::Clip &clip, uint32_t ref, ImageSource *recover, bool revoke) {
		_this->drawClipCall(clip, ref, recover, revoke, _canvas->_zDepth);
	}
	void GLC_CmdPack::clearColor(const Color4f &color, const Region &region, bool full) {
		_this->clearColorCall(color, region, full, _canvas->_zDepth);
	}
	void GLC_CmdPack::blurFilterBegin(Region bounds, float size) {
		_this->blurFilterBeginCall(bounds, size, _canvas->_isClipState, _canvas->_zDepth);
	}
	int GLC_CmdPack::blurFilterEnd(Region bounds, float size, ImageSource* output) {
		int n,lod;
		_this->getBlurSampling(size, n, lod);
		_this->blurFilterEndCall(bounds, size, output,
			_canvas->_blendMode, n, lod, _canvas->_isClipState, _canvas->_zDepth
		);
		return lod + 2;
	}
	void GLC_CmdPack::readImage(const Rect &src, ImageSource* img) {
		_this->readImageCall(src, img,
			_canvas->_size, _canvas->_surfaceSize, _canvas->_zDepth);
	}
	void GLC_CmdPack::outputImageBegin(ImageSource* img) {
		_this->outputImageBeginCall(img);
	}
	void GLC_CmdPack::outputImageEnd(ImageSource* img) {
		_this->outputImageEndCall(img);
	}
	void GLC_CmdPack::setBuffers(Vec2 size, ImageSource *recover, bool chSize) {
		_this->setBuffersCall(size, recover, chSize);
	}
	void GLC_CmdPack::drawBuffers(GLsizei num, const GLenum buffers[2]) {
		_this->drawBuffersCall(num, buffers);
	}
#endif

}
