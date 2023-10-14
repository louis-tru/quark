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

#define Qk_USE_GLC_CMD_QUEUE 1
#define Qk_MCCmd_Option_Capacity 1024
#define Qk_MCCmd_VertexBlock_Capacity 65535
#define Qk_MCCmd_OptBlock_Capacity 16384
#define Qk_MCCmd_CmdBlock_Capacity 65536

namespace qk {
	extern const float aa_fuzz_weight;
	extern const float DepthNextUnit;
	void gl_textureBarrier();

	Qk_DEFINE_INLINE_MEMBERS(GLC_CmdPack, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLC_CmdPack::Inl*>(self)

#if Qk_USE_GLC_CMD_QUEUE
		void clearCmds() {
			for (auto &i: cmds.blocks) {
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
						default: break;
					}
					cmd = (Cmd*)(((char*)cmd) + cmd->size);
				}
				i.size = 0;
			}
			cmds.current = cmds.blocks.val();
			cmds.current->size = sizeof(Cmd);
			cmds.index = 0;
			lastCmd = cmds.current->val;
		}

		Cmd* allocCmd(uint32_t size) {
			auto cmds_ = cmds.current;
			auto newSize = cmds_->size + size;
			if (newSize > cmds_->capacity) {
				if (++cmds.index == cmds.blocks.length()) {
					cmds.blocks.push({ // init, alloc 64k memory
						(Cmd*)malloc(Qk_MCCmd_CmdBlock_Capacity),0,Qk_MCCmd_CmdBlock_Capacity
					});
				}
				cmds.current = cmds_ = cmds.blocks.val() + cmds.index;
				newSize = size;
			}
			lastCmd = (Cmd*)(((char*)cmds_->val) + cmds_->size);
			lastCmd->size = size;
			cmds_->size = newSize;
			return lastCmd;
		}

		MultiColorCmd* newMColorCmd() {
			auto cmd = (MultiColorCmd*)allocCmd(sizeof(MultiColorCmd));
			cmd->type = kMultiColor_CmdType;

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
			cmd->aaclip = _canvas->_state->aaclip;

			return cmd;
		}

		MultiColorCmd* getMColorCmd() {
			auto cmd = (MultiColorCmd*)lastCmd;
			if (cmd->type == kMultiColor_CmdType) {
				if (vertexBlocks.current->size != Qk_MCCmd_VertexBlock_Capacity &&
						optionBlocks.current->size != Qk_MCCmd_OptBlock_Capacity &&
						cmd->subcmd != Qk_MCCmd_Option_Capacity
				) {
					return cmd;
				}
			}
			return newMColorCmd();
		}

		void checkMetrix() {
			if (_canvas->_chMatrix) { // check canvas matrix change state
				auto cmd = (MatrixCmd*)allocCmd(sizeof(MatrixCmd));
				cmd->type = kMatrix_CmdType;
				cmd->matrix = _canvas->_state->matrix;
				_canvas->_chMatrix = false;
			}
		}
#endif

		void useShaderProgram(GLSLShader *shader, const VertexData &vertex) {
			if (_cache->setGpuBufferData(vertex.id)) {
				glBindVertexArray(vertex.id->vao); // use vao
				glUseProgram(shader->shader); // use shader program
			} else {
				// copy vertex data to gpu and use shader
				shader->use(vertex.vertex.size(), vertex.vertex.val());
			}
		}

		void drawColor4fCall(const VertexData &vertex,
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
				float r0 = Float::min(Vec2(r[i], s1).length(), rmax); // len
				float r1 = Float::min(Vec2(r[i], s2).length(), rmax);
				float n = 2.0 * r1 / r0;
				glUniform3f(sh->__, r1, n, 1.0/n);
				glUniform2f(sh->horn, p[0], p[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		void drawImageCall(const VertexData &vertex,
			const ImagePaint *paint, float alpha, bool aafuzz, bool aaclip, float depth
		) {
			GLSLImage *s;
			auto src = paint->source;
			auto srcIndex = paint->srcIndex;

			if (kColor_Type_YUV420P_Y_8 == src->type()) { // yuv420p or yuv420sp
				auto yuv = aafuzz ?
					aaclip ? &_render->_shaders.imageYuv_AAFUZZ_AACLIP: &_render->_shaders.imageYuv_AAFUZZ:
					aaclip ? &_render->_shaders.imageYuv_AACLIP: &_render->_shaders.imageYuv;
				s = (GLSLImage*)yuv;
				useShaderProgram(s, vertex);

				if (src->pixels()[1].type() == kColor_Type_YUV420P_U_8) { // yuv420p
					glUniform1i(yuv->format, 1);
					_render->setTexture(src->pixels().val() + srcIndex + 2, 2, paint); // v
				} else { // yuv420sp
					glUniform1i(yuv->format, 0);
				}
				_render->setTexture(src->pixels().val() + srcIndex + 1, 1, paint); // u or uv
			} else {
				s = aafuzz ?
					aaclip ? &_render->_shaders.image_AAFUZZ_AACLIP: &_render->_shaders.image_AAFUZZ:
					aaclip ? &_render->_shaders.image_AACLIP: &_render->_shaders.image;
				useShaderProgram(s, vertex);
			}
			_render->setTexture(src->pixels().val() + srcIndex, 0, paint); // rgb or y

			glUniform1f(s->depth, depth);
			glUniform1f(s->alpha, alpha);
			glUniform4fv(s->coord, 1, paint->coord.origin.val);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
		}

		void drawImageMaskCall(const VertexData &vertex,
			const ImagePaint *paint, const Color4f &color, bool aafuzz, bool aaclip, float depth
		) {
			auto s = aafuzz ? 
				aaclip ? &_render->_shaders.imageMask_AAFUZZ_AACLIP: &_render->_shaders.imageMask_AAFUZZ:
				aaclip ? &_render->_shaders.imageMask_AACLIP: &_render->_shaders.imageMask;
			_render->setTexture(paint->source->pixels().val(), 0, paint);
			useShaderProgram(s, vertex);
			glUniform1f(s->depth, depth);
			glUniform4fv(s->color, 1, color.val);
			glUniform4fv(s->coord, 1, paint->coord.origin.val);
			glDrawArrays(GL_TRIANGLES, 0, vertex.vCount);
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

			auto aaClip = [](GLRender *_render, float depth, const GLC_State::Clip &clip, bool revoke, float W, float C) {
				auto chMode = _render->_blendMode;
				auto ch = chMode != kSrc_BlendMode && chMode != kSrcOver_BlendMode;
				if (ch)
					_render->setBlendMode(kSrc_BlendMode);
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
					_render->setBlendMode(chMode); // revoke blend mode
				gl_textureBarrier(); // ensure aa clip can be executed correctly in sequence
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
					aaClip(_render, depth, clip, revoke, aa_fuzz_weight, 0);
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
					aaClip(_render, depth, clip, revoke, -aa_fuzz_weight, -1);
				}
			}
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep
			glStencilFunc(GL_LEQUAL, ref, 0xFFFFFFFF); // Equality passes the test
		}

		void clearColor4fCall(const Color4f &color, const Region &region, bool full, float depth) {
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

		static void getBlurSampling(float size, int &n, int &lod) {
			n = ceilf(Float::clamp(size*1.25, 3, 13)); // sampling rate
			if (n % 2 == 0)
				n += 1; // keep singular
			lod = ceilf(Float::max(0,log2f(size/n)));
		}

		void blurFilterBeginCall(Region bounds, bool isClipState, float depth) {
			if (kSrc_BlendMode != _render->_blendMode)
				_render->setBlendMode(kSrc_BlendMode); // switch blend mode to src
			if (isClipState) glDisable(GL_STENCIL_TEST); // ignore clip
			// output to texture buffer then do post processing
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_blurBuffer, 0);
			clearColor4fCall({0,0,0,0}, bounds, false, depth); // clear pixels within bounds
			if (isClipState) glEnable(GL_STENCIL_TEST); // restore clip state
		}

		/**
		 * Fast gaussian blur reference https://www.shadertoy.com/view/ctjcWR
		 * Relevant information reference:
		 *   https://www.shadertoy.com/view/WtKfD3
		 *   https://blog.ivank.net/fastest-gaussian-blur.html
		 *   https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
		 *   https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
		 */
		void blurFilterEndCall(Region bounds, float size, BlendMode mode, int n, int lod, float depth) {
			float x1 = bounds.origin.x(), y1 = bounds.origin.y();
			float x2 = bounds.end.x(), y2 = bounds.end.y();
			float data[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			auto blur = _render->_shaders.blur; // blur shader
			size *= _canvas->_surfaceScale;
			auto R = _canvas->_surfaceSize;
			uint32_t oRw = R.x(), oRh = R.y();

			gl_textureBarrier();

			if (lod) { // copy image, gen mipmap texture
				int i = 0;
				auto &cp = _render->_shaders.imageCp;
				cp.use(sizeof(float) * 12, data);
				glUniform2f(cp.iResolution, R.x(), R.y());
				do { // copy image level
					oRw >>= 1; oRh >>= 1;
					glUniform1i(cp.depth, depth);
					glUniform1i(cp.imageLod, i);
					glUniform2f(cp.oResolution, oRw, oRh);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _canvas->_blurBuffer, i+1);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					gl_textureBarrier();
					depth += DepthNextUnit;
				} while(++i < lod);
			}

			blur.use(sizeof(float) * 12, data);
			// blur horizontal blur
			glUniform1i(blur.depth, depth);
			glUniform2f(blur.iResolution, R.x(), R.y());
			glUniform2f(blur.oResolution, oRw, oRh);
			glUniform1i(blur.imageLod, lod);
			glUniform1f(blur.step, 2.0/(n-1));
			glUniform2f(blur.size, size / R.x(), 0); // horizontal blur
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur
			//gl_textureBarrier(); // complete horizontal blur
			// blur vertical blur
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _canvas->_mainRenderBuff);
			glUniform1i(blur.depth, depth + DepthNextUnit);
			glUniform2f(blur.oResolution, R.x(), R.y());
			glUniform2f(blur.size, 0, size / R.y()); // vertical blur
			if (mode != kSrc_BlendMode)
				_render->setBlendMode(mode); // restore blend mode
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur to main render buffer
		}
	};

	// ---------------------------------------------------------------------------------------

	GLC_CmdPack::ImageCmd::~ImageCmd() {
		paint.source->release();
	}

	GLC_CmdPack::ImageMaskCmd::~ImageMaskCmd() {
		paint.source->release();
	}

	GLC_CmdPack::GLC_CmdPack(GLRender *render, GLCanvas *canvas)
		: _render(render), _canvas(canvas), _cache(canvas->_cache)
		, lastCmd(nullptr)
	{
#if Qk_USE_GLC_CMD_QUEUE
		vertexBlocks.blocks.push({
			(Vec4*)malloc(Qk_MCCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_MCCmd_VertexBlock_Capacity
		});
		vertexBlocks.current = vertexBlocks.blocks.val();
		vertexBlocks.index = 0;

		optionBlocks.blocks.push({
			(MCOpt*)malloc(Qk_MCCmd_OptBlock_Capacity * sizeof(MCOpt)),0,Qk_MCCmd_OptBlock_Capacity
		});
		optionBlocks.current = optionBlocks.blocks.val();
		optionBlocks.index = 0;

		cmds.blocks.push({ // init, alloc 64k memory
			(Cmd*)malloc(Qk_MCCmd_CmdBlock_Capacity),sizeof(Cmd),Qk_MCCmd_CmdBlock_Capacity
		});
		cmds.current = cmds.blocks.val();
		cmds.index = 0;
		lastCmd = cmds.current->val;
		lastCmd->size = sizeof(Cmd);
		lastCmd->type = kEmpty_CmdType;
#endif
	}

	GLC_CmdPack::~GLC_CmdPack() {
#if Qk_USE_GLC_CMD_QUEUE
		for (auto &i: vertexBlocks.blocks)
			free(i.val);
		for (auto &i: optionBlocks.blocks)
			free(i.val);
		for (auto &i: cmds.blocks)
			free(i.val);
		_this->clearCmds();
#endif
	}

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

		void GLC_CmdPack::drawColor4f(const VertexData &vertex, const Color4f &color, bool aafuzz) {
			if ( vertex.vertex.length() == 0 ) { // length == 0
				_this->checkMetrix(); // check matrix change
				auto cmd = new(_this->allocCmd(sizeof(ColorCmd))) ColorCmd;
				cmd->type = kColor_CmdType;
				cmd->vertex = vertex;
				cmd->depth = _canvas->_zDepth;
				cmd->aafuzz = aafuzz;
				cmd->aaclip = _canvas->_state->aaclip;
				cmd->color = color;
				return;
			}

			// add multi color subcmd
			auto vertexp = vertex.vertex.val();
			auto vertexLen = vertex.vCount;
			do {
				auto cmd = _this->getMColorCmd();
				cmd->opts[cmd->subcmd] = { // setting vertex option data
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

		void GLC_CmdPack::drawRRectBlurColor(
			const Rect& rect, const float *radius, float blur, const Color4f &color
		) {
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
			return;
		}

		void GLC_CmdPack::drawImage(const VertexData &vertex, const ImagePaint *paint, float alpha, bool aafuzz) {
			_this->checkMetrix(); // check matrix change
			auto cmd = new(_this->allocCmd(sizeof(ImageCmd))) ImageCmd;
			cmd->type = kImage_CmdType;
			cmd->vertex = vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->aafuzz = aafuzz;
			cmd->aaclip = _canvas->_state->aaclip;
			cmd->alpha = alpha;
			cmd->paint = *paint;
			paint->source->retain(); // retain source image ref
		}

		void GLC_CmdPack::drawImageMask(const VertexData &vertex, const ImagePaint *paint, const Color4f &color, bool aafuzz) {
			_this->checkMetrix(); // check matrix change
			auto cmd = new(_this->allocCmd(sizeof(ImageMaskCmd))) ImageMaskCmd;
			cmd->type = kImageMask_CmdType;
			cmd->vertex = vertex;
			cmd->depth = _canvas->_zDepth;
			cmd->aafuzz = aafuzz;
			cmd->aaclip = _canvas->_state->aaclip;
			cmd->color = color;
			cmd->paint = *paint;
			paint->source->retain(); // retain source image ref
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

		void GLC_CmdPack::clearColor4f(const Color4f &color, const Region &region, bool full) {
			auto cmd = new(_this->allocCmd(sizeof(ClearCmd))) ClearCmd;
			cmd->type = kClear_CmdType;
			cmd->color = color;
			cmd->region = region;
			cmd->depth = _canvas->_zDepth;
			cmd->fullClear = full;
		}

		void GLC_CmdPack::glFramebufferRenderbuffer(GLenum target, GLenum at, GLenum rbt, GLuint rb) {
			auto cmd = new(_this->allocCmd(sizeof(FramebufferRenderbufferCmd))) FramebufferRenderbufferCmd;
			cmd->type = kFramebufferRenderbuffer_CmdType;
			cmd->target = target;
			cmd->attachment = at;
			cmd->renderbuffertarget = rbt;
			cmd->renderbuffer = rb;
		}

		void GLC_CmdPack::glFramebufferTexture2D(GLenum target, GLenum at, GLenum tt, GLuint tex, GLint level) {
			auto cmd = new(_this->allocCmd(sizeof(FramebufferTexture2DCmd))) FramebufferTexture2DCmd;
			cmd->type = kFramebufferTexture2D_CmdType;
			cmd->target = target;
			cmd->attachment = at;
			cmd->textarget = tt;
			cmd->texture = tex;
			cmd->level = level;
		}

		void GLC_CmdPack::blurFilterBegin(Region bounds) {
			auto cmd = new(_this->allocCmd(sizeof(BlurFilterBeginCmd))) BlurFilterBeginCmd;
			cmd->type = kBlurFilterBegin_CmdType;
			cmd->bounds = bounds;
			cmd->depth = _canvas->_zDepth;
			cmd->isClipState = _canvas->_isClipState;
		}

		int GLC_CmdPack::blurFilterEnd(Region bounds, float size) {
			auto cmd = new(_this->allocCmd(sizeof(BlurFilterEndCmd))) BlurFilterEndCmd;
			cmd->type = kBlurFilterEnd_CmdType;
			cmd->bounds = bounds;
			cmd->depth = _canvas->_zDepth;
			cmd->mode = _canvas->_blendMode;
			cmd->size = size;
			_this->getBlurSampling(size, cmd->n, cmd->lod);
			return cmd->lod + 2;
		}

#else
		void GLC_CmdPack::setBlendMode(BlendMode mode) {
			_render->setBlendMode(mode);
		}
		void GLC_CmdPack::switchState(GLenum id, bool isEnable) {
			isEnable ? glEnable(id): glDisable(id);
		}
		void GLC_CmdPack::drawColor4f(const VertexData &vertex, const Color4f &color, bool aafuzz) {
			_this->drawColor4fCall(vertex, color, aafuzz, _canvas->_state->aaclip, _canvas->_zDepth);
		}
		void GLC_CmdPack::drawRRectBlurColor(
			const Rect& rect, const float *radius, float blur, const Color4f &color) {
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
		void GLC_CmdPack::clearColor4f(const Color4f &color, const Region &region, bool full) {
			_this->clearColor4fCall(color, region, full, _canvas->_zDepth);
		}
		void GLC_CmdPack::glFramebufferRenderbuffer(GLenum target, GLenum at, GLenum rbt, GLuint rb) {
			glFramebufferRenderbuffer(target, at, rbt, rb);
		}
		void GLC_CmdPack::glFramebufferTexture2D(GLenum target, GLenum at, GLenum tt, GLuint tex, GLint level) {
			glFramebufferTexture2D(target, at, tt, tex, level);
		}
		void GLC_CmdPack::blurFilterBegin(Region bounds) {
			_this->blurFilterBeginCall(bounds, _canvas->_isClipState, _canvas->_zDepth);
		}
		int GLC_CmdPack::blurFilterEnd(Region bounds, float size) {
			int n, lod;
			_this->getBlurSampling(size, n, lod);
			_this->blurFilterEndCall(bounds, size, _canvas->_blendMode, n, lod, _canvas->_zDepth);
			return lod + 2;
		}
#endif

	// ----------------------------------------------------------------------------------------

	void GLC_CmdPack::flush() {
#if Qk_USE_GLC_CMD_QUEUE
		// set canvas root matrix
		//glBindBuffer(GL_UNIFORM_BUFFER, _render->_rootMatrixBlock);
		//glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, _canvas->_rootMatrix.val, GL_DYNAMIC_DRAW);
		setMetrixUnifromBuffer(_canvas->_state->matrix); // maintain final status
		_render->setBlendMode(_canvas->_blendMode); // maintain final status

		for (auto &i: cmds.blocks) {
			if (i.size == 0) break;
			auto cmd = i.val;
			auto end = (Cmd*)(((char*)cmd) + i.size);
			Qk_ASSERT(cmd->size);

			while (cmd < end) {
				switch (cmd->type) {
					case kMatrix_CmdType:
						setMetrixUnifromBuffer(((MatrixCmd*)cmd)->matrix);
						break;
					case kBlend_CmdType:
						_render->setBlendMode(((BlendCmd*)cmd)->mode);
						break;
					case kFramebufferRenderbuffer_CmdType: {
						auto c = (FramebufferRenderbufferCmd*)cmd;
						glFramebufferRenderbuffer(c->target, c->attachment, c->renderbuffertarget, c->renderbuffer);
						break;
					}
					case kFramebufferTexture2D_CmdType: {
						auto c = (FramebufferTexture2DCmd*)cmd;
						glFramebufferTexture2D(c->target, c->attachment, c->textarget, c->texture, c->level);
						break;
					}
					case kBlurFilterBegin_CmdType: {
						auto c = (BlurFilterBeginCmd*)cmd;
						_this->blurFilterBeginCall(c->bounds, c->isClipState, c->depth);
					}
					case kBlurFilterEnd_CmdType: {
						auto c = (BlurFilterEndCmd*)cmd;
						_this->blurFilterEndCall(c->bounds, c->size, c->mode, c->n, c->lod, c->depth);
					}
					case kSwitch_CmdType: {
						auto c = (SwitchCmd*)cmd;
						c->isEnable ? glEnable(c->id): glDisable(c->id);
						break;
					}
					case kClear_CmdType: {
						auto c = (ClearCmd*)cmd;
						_this->clearColor4fCall(c->color, c->region, c->fullClear, c->depth);
						break;
					}
					case kClip_CmdType: {
						auto c = (ClipCmd*)cmd;
						_this->drawClipCall(c->clip, c->ref, c->revoke, c->depth);
						c->~ClipCmd();
						break;
					}
					case kColor_CmdType: {
						auto c = (ColorCmd*)cmd;
						_this->drawColor4fCall(c->vertex, c->color, c->aafuzz, c->aaclip, c->depth);
						break;
					}
					case kRRectBlurColor_CmdType: {
						auto c = (ColorRRectBlurCmd*)cmd;
						_this->drawRRectBlurColorCall(c->rect, c->radius, c->blur, c->color, c->aaclip, c->depth);
						break;
					}
					case kImage_CmdType: {
						auto c = (ImageCmd*)cmd;
						_this->drawImageCall(c->vertex, &c->paint, c->alpha, c->aafuzz, c->aaclip, c->depth);
						c->~ImageCmd();
						break;
					}
					case kImageMask_CmdType: {
						auto c = (ImageMaskCmd*)cmd;
						_this->drawImageMaskCall(c->vertex, &c->paint, c->color, c->aafuzz, c->aaclip, c->depth);
						c->~ImageMaskCmd();
						break;
					}
					case kGradient_CmdType: {
						auto c = (GradientCmd*)cmd;
						_this->drawGradientCall(c->vertex, &c->paint, c->alpha, c->aafuzz, c->aaclip, c->depth);
						break;
					}
					case kMultiColor_CmdType: {
						auto c = (MultiColorCmd*)cmd;
						auto s = c->aaclip ? &_render->_shaders.color1_AACLIP: &_render->_shaders.color1;
						glBindBuffer(GL_UNIFORM_BUFFER, _render->_optsBlock);
						glBufferData(GL_UNIFORM_BUFFER, sizeof(MultiColorCmd::Option) * c->subcmd, c->opts, GL_DYNAMIC_DRAW);
						glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
						glBufferData(GL_ARRAY_BUFFER, c->vCount * sizeof(Vec4), c->vertex, GL_DYNAMIC_DRAW);
						glBindVertexArray(s->vao);
						glUseProgram(s->shader);
						glDrawArrays(GL_TRIANGLES, 0, c->vCount);
						break;
					}
					default: break;
				}
				cmd = (Cmd*)(((char*)cmd) + cmd->size); // next cmd
			}
			i.size = 0;
		}

		for (int i = vertexBlocks.index; i >= 0; i--) {
			vertexBlocks.blocks[i].size = 0;
		}
		for (int i = optionBlocks.index; i >= 0; i--) {
			optionBlocks.blocks[i].size = 0;
		}

		vertexBlocks.current = vertexBlocks.blocks.val();
		vertexBlocks.index = 0;
		optionBlocks.current = optionBlocks.blocks.val();
		optionBlocks.index = 0;
		cmds.current       = cmds.blocks.val();
		cmds.current->size = sizeof(Cmd);
		cmds.index = 0;
		lastCmd = cmds.current->val;
#endif
	}

}
