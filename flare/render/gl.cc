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
#include "flare/render/gl.h"
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

	GLRender::GLRender(Application* host, const Options& params)
		: Render(host, params)
		, _interface(nullptr)
		, _frame_buffer(0), _is_support_multisampled(false)
	{
		String extensions = (const char*)glGetString(GL_EXTENSIONS);
		String version = (const char*)glGetString(GL_VERSION);

		F_DEBUG("OGL Info: %s", glGetString(GL_VENDOR));
		F_DEBUG("OGL Info: %s", glGetString(GL_RENDERER));
		F_DEBUG("OGL Info: %s", *version);
		F_DEBUG("OGL Info: %s", *extensions);
		
		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(1, &_frame_buffer);
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(1, &_render_buffer);
		// Create multisample buffers
		glGenFramebuffers(1, &_msaa_frame_buffer);
		glGenRenderbuffers(1, &_msaa_render_buffer);

		// initializ gl status
		if (is_gpu()) {
			/*
			* @开启颜色混合
			*
			* 如果设置了glBlendFunc(GL_ONE, GL_ZERO);
			* 则表示完全使用源颜色，完全不使用目标颜色，因此画面效果和不使用混合的时候一致（当然效率可能会低一点点）。
			* 如果没有设置源因子和目标因子，则默认情况就是这样的设置。
			*
			* 如果设置了glBlendFunc(GL_ZERO, GL_ONE);
			* 则表示完全不使用源颜色，因此无论你想画什么，最后都不会被画上去了。
			*（但这并不是说这样设置就没有用，有些时候可能有特殊用途）
			*
			* 如果设置了glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			* 则表示源颜色乘以自身的alpha 值，目标颜色乘以1.0减去源颜色的alpha值，这样一来，源颜色的alpha值越大，
			* 则产生的新颜色中源颜色所占比例就越大，而目标颜色所占比例则减 小。这种情况下，我们可以简单的将源颜色的alpha值
			* 理解为“不透明度”。这也是混合时最常用的方式。
			*
			* 如果设置了glBlendFunc(GL_ONE, GL_ONE);
			* 则表示完全使用源颜色和目标颜色，最终的颜色实际上就是两种颜色的简单相加。
			* 例如红色(1, 0, 0)和绿色(0, 1, 0)相加得到(1, 1, 0)，结果为黄色。
			*/
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

	GLRender::~GLRender() {
		glDeleteRenderbuffers(1, &_render_buffer);
		glDeleteFramebuffers(1, &_frame_buffer);
		glDeleteFramebuffers(1, &_msaa_render_buffer);
		glDeleteRenderbuffers(1, &_msaa_frame_buffer);
	}

	int GLRender::msaa_sample() {
		if ( _opts.MSAASampleCount > 1 && _is_support_multisampled && is_gpu()) {
			return _opts.MSAASampleCount;
		}
		return 0;
	}

	SkSurface* GLRender::surface() {
		if (!_direct) return nullptr;
		if (_surface) return _surface.get();

		auto region = _host->display()->surface_region();

		if (region.width == 0 || region.height == 0)
			return nullptr;

		int width = region.width;
		int height = region.height;
		auto MSAA = _opts.MSAASampleCount;

		glViewport(0, 0, width, height);

		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		gl_renderbuffer_storage();
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);

		if ( msaa_sample() ) { // 启用多重采样
			glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer); // render buffer
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA, GL_RGBA8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaa_render_buffer);
		}
		// glBindRenderbuffer(GL_RENDERBUFFER, _frame_buffer);

		// Test the framebuffer for completeness.
		if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
			if ( msaa_sample() ) {
				_opts.MSAASampleCount /= 2;
				return surface();
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
		
		GrGLFramebufferInfo fbInfo;
		fbInfo.fFBOID = msaa_sample() ? _msaa_frame_buffer : _frame_buffer;
		fbInfo.fFormat = GL_RGBA8;

		GrBackendRenderTarget backendRT(region.width, region.height, _sample_count, _stencil_bits, fbInfo);
		
		SkSurfaceProps props(SkSurfaceProps::Flags(_opts.flags), kUnknown_SkPixelGeometry);

		_surface = SkSurface::MakeFromBackendRenderTarget(
															_direct.get(), backendRT,
															kBottomLeft_GrSurfaceOrigin,
															SkColorType(_opts.colorType),
															/*_opts.colorSpace*/nullptr, &props);
		return _surface.get();
	}

	void GLRender::gl_renderbuffer_storage() {
		auto region = _host->display()->surface_region();
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, region.width, region.height);
	}

	void GLRender::reload() {
		_surface.reset(); // clear surface
		if (_direct) return;

		_interface.reset(nullptr);
		_interface = GrGLMakeNativeInterface();
		_stencil_bits = 8;
		_sample_count = 1;

		if ( msaa_sample() ) {
			_sample_count = _opts.MSAASampleCount;
		}
		_direct = GrDirectContext::MakeGL(_interface, {/*_opts.grContextOptions*/});
		F_ASSERT(_direct);
	}

}   // namespace flare
