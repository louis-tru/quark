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

#include "../../app.h"
#include "../../display.h"
#include "./skia_render.h"

#include "skia/gpu/gl/GrGLInterface.h"
#include "skia/core/SkImageInfo.h"

#if !F_APPLE || F_ENABLE_GL

namespace flare {

	uint32_t glPixelInternalFormat(ColorType type);

	// --------------- S k i a . G L . R e n d e r ---------------

	ViewVisitor* SkiaGLRender::visitor() {
		return this;
	}

	void SkiaGLRender::onReload() {
		if (!_direct) {
			_direct = GrDirectContext::MakeGL(GrGLMakeNativeInterface(), {/*_opts.grContextOptions*/});
			F_ASSERT(_direct);
		}
		_surface.reset(); // clear curr surface
		_rasterSurface.reset();

		auto region = _host->display()->display_region();
		if (_raster) {
			glDisable(GL_BLEND); // disable color blend
			_opts.stencilBits = 0;
			_opts.msaaSampleCnt = 0;
			auto info = SkImageInfo::Make(region.width, region.height,
																		SkColorType(_opts.colorType), kPremul_SkAlphaType, nullptr);
			_rasterSurface = SkSurface::MakeRaster(info);
			F_ASSERT(_rasterSurface);
		}

		GrGLFramebufferInfo fbInfo = {
			_opts.msaaSampleCnt > 1 ? _msaa_frame_buffer : _frame_buffer,
			glPixelInternalFormat(_opts.colorType),
		};

		GrBackendRenderTarget backendRT(region.width, region.height, _opts.msaaSampleCnt, _opts.stencilBits, fbInfo);
		SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

		_surface = SkSurface::MakeFromBackendRenderTarget(
															_direct.get(), backendRT,
															kBottomLeft_GrSurfaceOrigin,
															SkColorType(_opts.colorType), /*_opts.colorSpace*/nullptr, &props);
		F_ASSERT(_surface);
		if (_raster) {
			_canvas = static_cast<SkiaCanvas*>(_rasterSurface->getCanvas());
		} else {
			_canvas = static_cast<SkiaCanvas*>(_surface->getCanvas());
		}
	}

	void SkiaGLRender::onSubmit() {
		if (_raster)
			_rasterSurface->draw(_surface->getCanvas(), 0, 0);
		_surface->flushAndSubmit(); // commit sk
	}

	SkiaGLRender::SkiaGLRender(Application* host, const Options& opts, bool raster)
		: GLRender(host, opts) {
		_raster = raster;
	}

}
#endif
