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

#include "../app.h"
#include "../display.h"
#include "./gl.h"

namespace quark {

	uint32_t glPixelInternalFormat(ColorType type) {
		switch (type) {
			default: return 0;
			case kColor_Type_RGB_565: return GL_RGB565;
			case kColor_Type_RGBA_8888: return GL_RGBA8;
			case kColor_Type_RGB_888X: return GL_RGBA8;
			case kColor_Type_RGBA_1010102: return GL_RGB10_A2;
			case kColor_Type_RGB_101010X: return GL_RGB10_A2;
		}
	}

	GLRender::GLRender(Application* host, const Options& opts)
		: Render(host, opts)
		, _frame_buffer(0), _is_support_multisampled(false)
	{

		switch(_opts.colorType) {
			case kColor_Type_BGRA_8888: _opts.colorType = kColor_Type_RGBA_8888; break;
			case kColor_Type_BGRA_1010102: _opts.colorType = kColor_Type_RGBA_1010102; break;
			case kColor_Type_BGR_101010X: _opts.colorType = kColor_Type_RGB_101010X; break;
			default: break;
		}

		String extensions = (const char*)glGetString(GL_EXTENSIONS);
		String version = (const char*)glGetString(GL_VERSION);

		Qk_DEBUG("OGL Info: %s", glGetString(GL_VENDOR));
		Qk_DEBUG("OGL Info: %s", glGetString(GL_RENDERER));
		Qk_DEBUG("OGL Info: %s", *version);
		Qk_DEBUG("OGL Info: %s", *extensions);
		
		if (!_is_support_multisampled) {
			for (auto s : {"OpenGL ES ", "OpenGL "}) {
				int idx = version.index_of(s);
				if (idx != -1) {
					int num = version.substr(idx + 10, 1).to_number<int>();
					if (num > 2) {
						_is_support_multisampled = true;
					} else {
						_is_support_multisampled = extensions.index_of( "multisample" ) != -1;
					}
					if (_is_support_multisampled)
						break;
				}
			}
		}

		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(1, &_frame_buffer);
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(1, &_render_buffer);
		// Create multisample buffers
		glGenFramebuffers(1, &_msaa_frame_buffer);
		glGenRenderbuffers(1, &_msaa_render_buffer);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	GLRender::~GLRender() {
		glDeleteRenderbuffers(1, &_render_buffer);
		glDeleteFramebuffers(1, &_frame_buffer);
		glDeleteFramebuffers(1, &_msaa_render_buffer);
		glDeleteRenderbuffers(1, &_msaa_frame_buffer);
	}

	void GLRender::onRenderbufferStorage(uint32_t target) {
		auto region = _host->display()->display_region();
		::glRenderbufferStorage(target, glPixelInternalFormat(_opts.colorType), region.width, region.height);
	}

	void GLRender::reload() {
		if (!_is_support_multisampled) {
			_opts.msaaSampleCnt = 0;
		}

		auto region = _host->display()->display_region();

		int width = region.width;
		int height = region.height;

		Qk_ASSERT(width && height);

		glViewport(0, 0, width, height);

		do {
			glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
			onRenderbufferStorage(GL_RENDERBUFFER);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);

			int MSAA = _opts.msaaSampleCnt;
			if ( MSAA > 1 ) { // 启用多重采样
				glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer); // render buffer
				glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA, glPixelInternalFormat(_opts.colorType), width, height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaa_render_buffer);
			}

			// Test the framebuffer for completeness.
			if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ) {
				break;
			} else {
				if ( MSAA > 1 ) {
					_opts.msaaSampleCnt /= 2;
				} else {
					Qk_ERR("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER) );
					return;
				}
			}
		} while(1);

		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);

		glClearStencil(0);
		glStencilMask(0xffffffff);

		onReload();
	}

	void GLRender::begin() {
		//glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		//glBindRenderbuffer(GL_RENDERBUFFER, _frame_buffer);
	}

	void GLRender::submit() {
		onSubmit();

		if (_opts.msaaSampleCnt > 1) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _msaa_frame_buffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frame_buffer);
			auto region = _host->display()->display_region();
			glBlitFramebuffer(0, 0, region.width, region.height,
												0, 0, region.width, region.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#if !Qk_OSX
			GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 3, attachments);
#endif
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		} else {
#if !Qk_OSX
			GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
#endif
		}

		onSwapBuffers();

		if ( _opts.msaaSampleCnt > 1 ) {
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer);
		}
	}

}
