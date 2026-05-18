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

#include "./gl_canvas.h"
#include "./gl_render.h"
#include "./gl_command.h"

#define isMoreSofterAA 1 // 1 is softer aa, 0 is more radical aa

namespace qk {
	GLenum gl_CheckFramebufferStatus(GLenum target);
	void  gl_set_framebuffer_renderbuffer(GLuint b, Vec2 s, GLenum f, GLenum at);
	void  gl_set_color_renderbuffer(GLuint rbo, GLuint orTex, ColorType type, Vec2 size);
	void  gl_set_aaclip_buffer(GLuint tex, Vec2 size);
	void  gl_set_tex_renderbuffer(GLuint tex, Vec2 size);
	GLuint gl_new_tex_stat();
	void clear_PathvCache(PathvCache *cache, int flags);
	void clearExec_PathvCache(PathvCache *cache);
	GLint gl_get_texture_format(ColorType type);
	GLint gl_get_texture_data_type(ColorType format);

	constexpr GLenum DrawBuffers[]{
		GL_COLOR_ATTACHMENT0/*main color out*/, GL_COLOR_ATTACHMENT1/*other out*/,
	};

	GLCanvas::GLCanvas(GLRender *render, Render::Options opts)
		: GPUCanvas(render, opts)
		, _render(render)
		, _fbo(0), _outTex(0), _outDepth(0), _outAaclipTex(0), _outTexA(0), _outTexB(0)
		, _matrixFlag(false)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kRGBA_8888_ColorType;
		_cmdPack = new GLC_CmdPack(render, this);
		_cmdPackFront = new GLC_CmdPack(render, this);
	}

	GLCanvas::~GLCanvas() {
		GLuint fbo = _fbo,
					rbo[] = { _outDepth },
					tex[] = { _outTex, _outAaclipTex, _outTexA, _outTexB };
		_render->post_message(Cb([render=_render,fbo,rbo,tex](auto &e) {
			glDeleteFramebuffers(1, &fbo);
			glDeleteRenderbuffers(1, rbo);
			glDeleteTextures(4, tex);
		}));
		_mutex.lock();
		Releasep(_cmdPackFront);
		Releasep(_cmdPack);
		_mutex.unlock();
	}

	// --------------------------------------------------------

	void GLCanvas::setBuffers(Vec2 surfaceSize) {
		auto w = surfaceSize.x(), h = surfaceSize.y();
		auto type = _opts.colorType;
		auto init = !_fbo;

		if (init) {
			// Create a color renderbuffer of texture
			_outTex = gl_new_tex_stat();
			// Create the framebuffer
			glGenFramebuffers(1, &_fbo);
			// Create depth buffer
			glGenRenderbuffers(1, &_outDepth);
			// gen aaclip buffer tex
			// gl_set_aaclip_buffer(_outAaclipTex, size);
		}
		// Bind framebuffer future OpenGL ES framebuffer commands are directed to it.
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		// Allocate storage for it, and attach it to the framebuffer.
		gl_set_color_renderbuffer(0, _outTex, type, surfaceSize);
		gl_set_framebuffer_renderbuffer(_outDepth, surfaceSize, GL_DEPTH24_STENCIL8, GL_DEPTH_ATTACHMENT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _outDepth);

		Qk_DLog("setBuffers: %f, %f", w, h);

		if (init) {
			Color4f color{0,0,0,0};
			glClearBufferfv(GL_COLOR, 0, color.val); // clear GL_COLOR_ATTACHMENT0
		}
		if (_outAaclipTex) {
			gl_set_aaclip_buffer(_outAaclipTex, surfaceSize);
		}
		if (_outTexA) {
			gl_set_tex_renderbuffer(_outTexA, surfaceSize);
		}
		if (_outTexB) {
			gl_set_tex_renderbuffer(_outTexB, surfaceSize);
		}

		glDrawBuffers(1, DrawBuffers);
		gl_CheckFramebufferStatus(GL_FRAMEBUFFER);

#if DEBUG
		int width, height;
		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DLog("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);
#endif
	}

	bool GLCanvas::swapBuffer() {
		_mutex.lock();
		// check if have cmds in front buffer, if have cmds, wait for next swap
		bool canSwap = _cmdPackFront->isEmpty();
		if (canSwap) {
			std::swap(_cmdPackFront, _cmdPack); // swap cmd buffer
			_cmdPack->savePipelineState(); // save initial pipeline state
			clear_PathvCache(_cache, 0); // tag: clear mark
		}
		_mutex.unlock();
		return canSwap;
	}

	void GLCanvas::flushBuffer() { // only can rendering thread call
		_mutex.lock();
		_cmdPackFront->flush(); // commit gl cmd
		_mutex.unlock();
		clearExec_PathvCache(_cache); // clear @clear mark
	}

	void GLCanvas::vportFullCopy(GLuint dstFBO) {
		if (!_outTex)
			return; // no output texture
		auto dest = _render->surfaceSize();
		auto chVport = _surfaceSize != dest;
		GLint filter = chVport ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_NEAREST;
		if (chVport) {
			glViewport(0, 0, dest.x(), dest.y());
		}
		glDisable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
		glUseProgram(_render->_shaders.vportFullCp.shader);
		glBindVertexArray(_render->_shaders.vportFullCp.vao);
		glActiveTexture(Qk_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _outTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo); // recover fbo
		glEnable(GL_BLEND);
		if (chVport) {
			glViewport(0, 0, _surfaceSize.x(), _surfaceSize.y());
		}
	}

	bool GLCanvas::readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) {
#if Qk_APPLE
		GLenum format = gl_get_texture_format(dst->type());
		GLenum type = gl_get_texture_data_type(dst->type());
		if (format && dst->bytes() != dst->buffer().length())
			return false;
		_render->lock();
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		flushBuffer(); // commit gl cmd first to ensure the data is ready for reading
		glReadPixels(srcX, srcY, dst->width(), dst->height(), format, type, dst->val());
		_render->unlock();
		return true;
#else
		// Disable non-macOS systems temporarily,
		// as the GL context cannot be bing to multiple threads on Linux.
		return false;
#endif
	}

	// --------------------------------------------------------------------

	void GLCanvas::checkMatrix() {
		if (_matrixFlag) {
			_cmdPack->setMatrix();
			_matrixFlag = false;
		}
	}

	void GLCanvas::setSurfaceCmd(bool changeSize) {
		_cmdPack->setSurface(changeSize);
	}

	void GLCanvas::setMatrixCmd() {
		_matrixFlag = true; // Defer the actual matrix update until the next draw call for better batching
	}

	void GLCanvas::setBlendModeCmd() {
		_cmdPack->setBlendMode();
	}

	void GLCanvas::enableStencilTestCmd(bool enable) {
		_cmdPack->switchState(GL_STENCIL_TEST, enable);
	}

	void GLCanvas::drawClipCmd(const GC_State::Clip &clip, uint32_t ref, bool revoke) {
		checkMatrix();
		_cmdPack->drawClip(clip, ref, _state->output.get(), revoke);
	}

	void GLCanvas::clearColorCmd(const Color4f &color, GC_ClearFlags flags) {
		checkMatrix();
		_cmdPack->clearColor(color, flags);
	}

	void GLCanvas::drawImageCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) {
		checkMatrix();
		_cmdPack->drawImage(vertex, paint, color);
	}

	void GLCanvas::drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) {
		checkMatrix();
		_cmdPack->drawGradient(vertex, paint, color);
	}

	void GLCanvas::drawImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) {
		checkMatrix();
		_cmdPack->drawImageMask(vertex, paint, color);
	}

	void GLCanvas::drawColorCmd(const VertexData &vertex, const Color4f &color) {
		checkMatrix();
		_cmdPack->drawColor(vertex, color);
	}

	void GLCanvas::drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) {
		checkMatrix();
		_cmdPack->drawRRectBlurColor(rect, radius, blur, color);
	}

	void GLCanvas::drawSDFImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
				const Color4f &strokeColor, float stroke) {
		checkMatrix();
		_cmdPack->drawSDFImageMask(vertex, paint, color, strokeColor, stroke);
	}

	void GLCanvas::blurFilterBeginCmd(Range bounds, float radius, float clearPad) {
		checkMatrix();
		_cmdPack->blurFilterBegin(bounds, radius, clearPad);
	}

	void GLCanvas::blurFilterEndCmd(Range bounds, float radius, float clearPad, int sample, int imageLod) {
		checkMatrix();
		_cmdPack->blurFilterEnd(bounds, radius, clearPad, sample, imageLod);
	}

	void GLCanvas::drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData) {
		checkMatrix();
		_cmdPack->drawTriangles(triangles, paint, color, copyData);
	}

	void GLCanvas::readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dest) {
		checkMatrix();
		_cmdPack->readImage(srcRect, src, dest);
	}

	void GLCanvas::outputImageBeginCmd(ImageSource* img) {
		checkMatrix();
		_cmdPack->outputImageBegin(img);
	}

	void GLCanvas::outputImageEndCmd(ImageSource* exit) {
		checkMatrix();
		_cmdPack->outputImageEnd(exit, _state->output.get());
	}
}
