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
    String VENDOR = (const char*)glGetString(GL_VENDOR);
    String RENDERER = (const char*)glGetString(GL_RENDERER);
		String VERSION = (const char*)glGetString(GL_VERSION);
    String EXTENSIONS = (const char*)glGetString(GL_EXTENSIONS);
		bool ok = false;

		Qk_DEBUG("OGL VENDOR: %s", *VENDOR);
		Qk_DEBUG("OGL RENDERER: %s", *RENDERER);
		Qk_DEBUG("OGL VERSION: %s", *VERSION);
		Qk_DEBUG("OGL EXTENSIONS: %s", *EXTENSIONS);
    
    String str = VENDOR +  " " + RENDERER + " " + VERSION + " " + EXTENSIONS;

		for (auto s : {"OpenGL ES ", "OpenGL Engine ", "OpenGL "}) {
			int idx = str.index_of(s);
			if (idx != -1) {
				int num = str.substr(idx + strlen(s), 1).trim().to_number<int>();
				if (num > 2)
					ok = true;
				else
					ok = str.index_of( "multisample" ) != -1;
				if (ok)
					break;
			}
		}

		if (!ok && str.index_of("Metal ") != -1)
			ok = true;
		
		return ok;
	}

	GLRender::GLRender(Application* host, bool independentThread)
		: Render(host, independentThread)
		, _frame_buffer(0), _msaa_frame_buffer(0)
		, _render_buffer(0), _msaa_render_buffer(0), _stencil_buffer(0), _depth_buffer(0),_aa_tex(0)
		,_is_support_multisampled(false), _raster(false)
	{
		_is_support_multisampled = checkIsSupportMultisampled();

		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(2, &_frame_buffer); // _frame_buffer,_msaa_frame_buffer
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(4, &_render_buffer); // _render_buffer,_msaa_render_buffer,_stencil_buffer,_depth_buffer
		// create anti alias texture
		glGenTextures(1, &_aa_tex);

		glEnable(GL_BLEND);
		setBlendMode(kSrcOver_BlendMode);

		glClearStencil(0);
		glStencilMask(0xffffffff);
		glColorMask(1,1,1,1);

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_DEPTH_TEST);

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
		
		if (!_is_support_multisampled || _raster) {
			_opts.msaaSampleCnt = 0;
		}
	}

	GLRender::~GLRender() {
		glDeleteFramebuffers(2, &_frame_buffer); // _frame_buffer,_msaa_frame_buffer
		glDeleteRenderbuffers(4, &_render_buffer); // _render_buffer,_msaa_render_buffer,_stencil_buffer,_depth_buffer
		glDeleteTextures(1, &_aa_tex);
	}

	Object* GLRender::asObject() {
		return this;
	}

	void GLRender::reload(Vec2 size, Mat4& root) {
		int width = size.x();
		int height = size.y();

		Qk_ASSERT(width, "Invalid viewport size width");
		Qk_ASSERT(height, "Invalid viewport size height");

		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer); // bind frame buffer
		setRenderBuffer(width, height);

		if (_opts.msaaSampleCnt > 1) {
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);

			do { // enable multisampling
				setMSAABuffer(width, height, _opts.msaaSampleCnt);
				// Test the framebuffer for completeness.
				if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ) {
					if ( _opts.msaaSampleCnt > 1 )
						_IsDeviceMsaa = true;
					break;
				}
				_opts.msaaSampleCnt >>= 1;
			} while (_opts.msaaSampleCnt > 1);
		}

		if (_opts.msaaSampleCnt <= 1) {
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
			setAntiAlias(width, height);
		}

		setStencilBuffer(width, height, _opts.msaaSampleCnt);

		const GLenum buffers[]{ GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);

		if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
			Qk_FATAL("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

		setRootMatrix(root);
	}

	void GLRender::setRenderBuffer(int width, int height) {
		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, glPixelInternalFormat(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);
	}

	void GLRender::setStencilBuffer(int width, int height, int MSAASample) { // set clip stencil buffer
		glBindRenderbuffer(GL_RENDERBUFFER, _stencil_buffer);
		if (MSAASample > 1) {
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASample, GL_STENCIL_INDEX8, width, height);
		} else {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _stencil_buffer);
	}

	void GLRender::setMSAABuffer(int width, int height, int MSAASample) {
		glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer); // render buffer
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASample, glPixelInternalFormat(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaa_render_buffer);
	}

	void GLRender::setAntiAlias(int width, int height) {
		// set anti alias texture buffer
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _aa_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _aa_tex, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		// set depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, _depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_buffer);
	}

	void GLRender::setRootMatrix(Mat4& root) {
#if DEBUG
		int width, height;
		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);
		Qk_ASSERT(width, "Invalid Renderbuffer size width");
		Qk_ASSERT(height, "Invalid Renderbuffer size height");
#endif
		
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// update all shader root matrix
		for (auto shader: _shaders) {
			glUseProgram(shader->shader());
			glUniformMatrix4fv( shader->root_matrix(), 1, GL_TRUE, root.val );
		}
	}

	void GLRender::begin() {
	}

	void GLRender::submit() {
		if (_opts.msaaSampleCnt > 1) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _msaa_frame_buffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frame_buffer);
			auto region = _host->display()->surface_region();
			auto w = region.size.x(), h = region.size.x();
			glBlitFramebuffer(0, 0, w, h,
												0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#if !Qk_OSX
			GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 3, attachments);
#endif
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
			present();
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
		} else {
#if !Qk_OSX
			GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
#endif
			present();
		}
	}

	GLuint GLRender::setTexture(cPixel *src, GLuint id) {
		return GLCanvas::setTexture(src, id, true);
	}

	void GLRender::deleteTextures(const GLuint *IDs, GLuint count) {
		GLCanvas::deleteTextures(IDs, count);
	}

}
