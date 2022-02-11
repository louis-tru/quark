/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "flare/app.h"
#include "flare/render/render.h"
#include "flare/display.h"

#include "skia/core/SkCanvas.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrBackendSurface.h"
#include "skia/gpu/GrDirectContext.h"

#if F_IOS
# include <OpenGLES/ES3/gl.h>
# include <OpenGLES/ES3/glext.h>
#elif F_ANDROID
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#elif F_OSX
# include <OpenGL/gl3.h>
# include <OpenGL/gl3ext.h>
#elif F_LINUX
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#else
# error "The operating system does not support"
#endif

namespace flare {

	static uint32_t glPixelInternalFormat(ColorType type) {
		switch (type) {
			default: return 0;
			case COLOR_TYPE_RGB_565: return GL_RGB565;
			case COLOR_TYPE_RGBA_8888: return GL_RGBA8;
			case COLOR_TYPE_RGB_888X: return GL_RGBA8;
			case COLOR_TYPE_RGBA_1010102: return GL_RGB10_A2;
			case COLOR_TYPE_RGB_101010X: return GL_RGB10_A2;
		}
	}

	OpenGLRender::OpenGLRender(Application* host, const Options& params)
		: Render(host, params)
		, _frame_buffer(0), _is_support_multisampled(false)
	{
		String extensions = (const char*)glGetString(GL_EXTENSIONS);
		String version = (const char*)glGetString(GL_VERSION);

		F_DEBUG("OGL Info: %s", glGetString(GL_VENDOR));
		F_DEBUG("OGL Info: %s", glGetString(GL_RENDERER));
		F_DEBUG("OGL Info: %s", *version);
		F_DEBUG("OGL Info: %s", *extensions);
		
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

	OpenGLRender::~OpenGLRender() {
		glDeleteRenderbuffers(1, &_render_buffer);
		glDeleteFramebuffers(1, &_frame_buffer);
		glDeleteFramebuffers(1, &_msaa_render_buffer);
		glDeleteRenderbuffers(1, &_msaa_frame_buffer);
	}

	SkSurface* OpenGLRender::surface() {
		if (!_direct)
			return nullptr;
		if (_surface)
			return _surface.get();

		auto region = _host->display()->surface_region();

		int width = region.width;
		int height = region.height;
		int MSAA = _opts.msaaSampleCnt;

		F_ASSERT(width && height);

		glViewport(0, 0, width, height);

		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		renderbufferStorage(GL_RENDERBUFFER);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);

		if ( MSAA > 1 ) { // 启用多重采样
			glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer); // render buffer
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA, glPixelInternalFormat(_opts.colorType), width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaa_render_buffer);
		}

		// Test the framebuffer for completeness.
		if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
			if ( MSAA > 1 ) {
				_opts.msaaSampleCnt /= 2;
				return OpenGLRender::surface();
			} else {
				F_ERR("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER) );
			}
			return nullptr;
		}
		
		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		F_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);

		glClearStencil(0);
		glStencilMask(0xffffffff);

		GrGLFramebufferInfo fbInfo = {
			_opts.msaaSampleCnt > 1 ? _msaa_frame_buffer : _frame_buffer,
			glPixelInternalFormat(_opts.colorType),
		};

		GrBackendRenderTarget backendRT(region.width, region.height, _opts.msaaSampleCnt, _opts.stencilBits, fbInfo);
		SkSurfaceProps props(_opts.flags, kUnknown_SkPixelGeometry);

		_surface = SkSurface::MakeFromBackendRenderTarget(
															_direct.get(), backendRT,
															kBottomLeft_GrSurfaceOrigin,
															SkColorType(_opts.colorType), /*_opts.colorSpace*/nullptr, &props);
		return _surface.get();
	}

	void OpenGLRender::renderbufferStorage(uint32_t target) {
		auto region = _host->display()->surface_region();
		::glRenderbufferStorage(target, glPixelInternalFormat(_opts.colorType), region.width, region.height);
	}

	void OpenGLRender::reload() {
		if (!_direct) {
			if (!_is_support_multisampled) {
				_opts.msaaSampleCnt = 0;
			}
			_direct = GrDirectContext::MakeGL(GrGLMakeNativeInterface(), {/*_opts.grContextOptions*/});
			F_ASSERT(_direct);
		}
		_surface.reset(); // clear surface
	}

	void OpenGLRender::submit() {
		_surface->flushAndSubmit(); // commit sk

		if (_opts.msaaSampleCnt > 1) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _msaa_frame_buffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frame_buffer);
			auto region = _host->display()->surface_region();
			glBlitFramebuffer(0, 0, region.width, region.height,
												0, 0, region.width, region.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 3, attachments);
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		} else {
			GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
		}

		swapBuffers();

		if ( _opts.msaaSampleCnt > 1 ) {
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer);
		}
	}

	RasterOpenGLRender::RasterOpenGLRender(Application* host, const Options& opts): OpenGLRender(host, opts) {
		glDisable(GL_BLEND); // disable color blend
	}

	SkSurface* RasterOpenGLRender::surface() {
		if (!_rasterSurface) {
			// make the offscreen image
			auto region = _host->display()->surface_region();
			auto info = SkImageInfo::Make(region.width, region.height,
																		SkColorType(_opts.colorType), kPremul_SkAlphaType, nullptr);
			_rasterSurface = SkSurface::MakeRaster(info);
		}
		return _rasterSurface.get();
	}

	void RasterOpenGLRender::reload() {
		_opts.stencilBits = 0;
		_opts.msaaSampleCnt = 0;
		OpenGLRender::reload();
		_rasterSurface.reset();
	}

	void RasterOpenGLRender::submit() {
		// draw to gl canvas
		_rasterSurface->draw(OpenGLRender::surface()->getCanvas(), 0, 0);
		OpenGLRender::submit();
	}

}   // namespace flare
