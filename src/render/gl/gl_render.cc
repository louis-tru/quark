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

#include "../../app.h"
#include "../../display.h"
#include "./gl_render.h"

namespace qk {

	uint32_t glPixelInternalFormat(ColorType type) {
		switch (type) {
			case kColor_Type_RGB_565: return GL_RGB565;
			case kColor_Type_RGBA_8888: return GL_RGBA8;
			case kColor_Type_RGB_888X: return GL_RGBA8;
			case kColor_Type_RGBA_1010102: return GL_RGB10_A2;
			case kColor_Type_RGB_101010X: return GL_RGB10_A2;
			default: return 0;
		}
	}

	bool checkIsSupportMultisampled() {
		String extensions = (const char*)glGetString(GL_EXTENSIONS);
		String version = (const char*)glGetString(GL_VERSION);

		bool ok = false;

		Qk_DEBUG("OGL Info: %s", glGetString(GL_VENDOR));
		Qk_DEBUG("OGL Info: %s", glGetString(GL_RENDERER));
		Qk_DEBUG("OGL Info: %s", *version);
		Qk_DEBUG("OGL Info: %s", *extensions);

		for (auto s : {"OpenGL ES ", "OpenGL "}) {
			int idx = version.index_of(s);
			if (idx != -1) {
				int num = version.substr(idx + 10, 1).to_number<int>();
				if (num > 2) {
					ok = true;
				} else {
					ok = extensions.index_of( "multisample" ) != -1;
				}
				if (ok)
					break;
			}
		}
		return ok;
	}

	GLRender::GLRender(Application* host, bool raster)
		: Render(host)
		, _frame_buffer(0), _is_support_multisampled(false), _raster(raster)
	{
		_is_support_multisampled = checkIsSupportMultisampled();

		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(1, &_frame_buffer);
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(1, &_render_buffer);
		// Create multisample buffers
		glGenFramebuffers(1, &_msaa_frame_buffer);
		glGenRenderbuffers(1, &_msaa_render_buffer);

		glEnable(GL_BLEND);
		setBlendMode(kSrcOver_BlendMode);

		glClearStencil(0);
		glStencilMask(0xffffffff);

		glDisable(GL_DEPTH);
		glDisable(GL_STENCIL);

		switch(_opts.colorType) {
			case kColor_Type_BGRA_8888:
				_opts.colorType = kColor_Type_RGBA_8888; break;
			case kColor_Type_BGRA_1010102:
				_opts.colorType = kColor_Type_RGBA_1010102; break;
			case kColor_Type_BGR_101010X:
				_opts.colorType = kColor_Type_RGB_101010X; break;
			default: break;
		}

		if (_raster) {
			// new raster canvas
		}
		_canvas = this;
	}

	GLRender::~GLRender() {
		glDeleteRenderbuffers(1, &_render_buffer);
		glDeleteFramebuffers(1, &_frame_buffer);
		glDeleteFramebuffers(1, &_msaa_render_buffer);
		glDeleteRenderbuffers(1, &_msaa_frame_buffer);
	}

	void GLRender::onRenderbufferStorage(GLuint target) {
		auto region = _host->display()->surface_region();
		int w = region.size.x(), h = region.size.y();
		::glRenderbufferStorage(target, glPixelInternalFormat(_opts.colorType), w, h);
	}

	void GLRender::reload() {
		if (!_is_support_multisampled || _raster) {
			_opts.msaaSampleCnt = 0;
		}

		auto region = _host->display()->surface_region();

		int width = region.size.x();
		int height = region.size.y();

		Qk_ASSERT(width, "Invalid viewport size width");
		Qk_ASSERT(height, "Invalid viewport size height");

		glViewport(0, 0, width, height);

		// --------------------- Init render and frame buffers ---------------------
		glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		onRenderbufferStorage(GL_RENDERBUFFER); // create main buffer storage
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);

		do {
			int MSAA = _opts.msaaSampleCnt;
			if ( MSAA > 1 ) { // 启用多重采样
				glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
				glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer); // render buffer
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA, glPixelInternalFormat(_opts.colorType), width, height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaa_render_buffer);
			}
			// Test the framebuffer for completeness.
			if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ) {
				_IsDeviceAntiAlias = true;
				break;
			} else if ( MSAA > 1 ) {
				_opts.msaaSampleCnt >>= 1;
			} else {
				Qk_FATAL("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
			}
		} while(1);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);

		// ---------------------------------------------------------------

		// update all shader root matrix
		auto scale = _host->display()->scale();
		Vec2 start = Vec2(-region.origin.x() / scale, -region.origin.y() / scale);
		Vec2 end   = Vec2(region.size.x() / scale + start.x(), region.size.y() / scale + start.y());
		auto matrix = Mat4::ortho(start.x(), end.x(), start.y(), end.y(), -1.0f, 1.0f);
		// root_matrix.transpose();

		for (auto shader: _shaders) {
			glUseProgram(shader->shader());
			glUniformMatrix4fv( shader->root_matrix(), 1, GL_TRUE, matrix.val );
		}
	}

	void GLRender::begin() {
		// noop
	}

	void GLRender::submit() {
		if (_opts.msaaSampleCnt > 1) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _msaa_frame_buffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frame_buffer);
			auto region = _host->display()->surface_region();
			glBlitFramebuffer(0, 0, region.size.x(), region.size.y(),
												0, 0, region.size.x(), region.size.y(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
#if !Qk_OSX
			GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 3, attachments);
#endif
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
			// glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
			onSwapBuffers();
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
			// glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer);
		} else {
#if !Qk_OSX
			GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
#endif
			onSwapBuffers();
		}
	}

	GLuint GLRender::setTexture(cPixel *src, GLuint id) {
		return GLCanvas::setTexture(src, id, true);
	}

	void GLRender::deleteTextures(const GLuint *IDs, GLuint count) {
		GLCanvas::deleteTextures(IDs, count);
	}

}
