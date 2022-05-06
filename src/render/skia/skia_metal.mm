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

#include "skia/gpu/mtl/GrMtlBackendContext.h"
// #include "skia/gpu/mtl/GrMtlTypes.h" //
// #include "skia/private/GrMtlTypesPriv.h"

#if F_APPLE
namespace noug {

	ViewVisitor* SkiaMetalRender::visitor() {
		return this;
	}

	void SkiaMetalRender::onReload() {
		if (!_direct) {
			GrMtlBackendContext backendContext = {};
			backendContext.fDevice.retain((__bridge void*)_device);
			backendContext.fQueue.retain((__bridge void*)_queue);

			_direct = GrDirectContext::MakeMetal(backendContext, {/*_opts.grContextOptions*/});
			F_ASSERT(_direct);
		}

		_surface.reset(); // clear curr surface
		_rasterSurface.reset();

		auto region = _host->display()->display_region();
		if (_raster) {
			_opts.stencilBits = 0;
			_opts.msaaSampleCnt = 0;
			auto info = SkImageInfo::Make(region.width, region.height,
																		SkColorType(_opts.colorType), kPremul_SkAlphaType, nullptr);
			_rasterSurface = SkSurface::MakeRaster(info);
			F_ASSERT(_rasterSurface);
			//_canvas = static_cast<SkiaCanvas*>(_rasterSurface->getCanvas());
		}
	}

	void SkiaMetalRender::onBegin() {
		id<MTLTexture> tex = _drawable.texture;
		
		GrMtlTextureInfo fbInfo;
		fbInfo.fTexture.retain((__bridge void*)tex);
		
		//auto region = _host->display()->surface_region();
		//F_DEBUG("width, %f==%d", region.width, tex.width);
		//F_DEBUG("height, %f==%d", region.height, tex.height);
		//tex.sampleCount = _opts.msaaSampleCnt;
		//F_DEBUG("%d, %d", tex.sampleCount, _opts.msaaSampleCnt);

		GrBackendRenderTarget backendRT((int)tex.width, (int)tex.height, _opts.msaaSampleCnt, fbInfo);
		SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

		_surface = SkSurface::MakeFromBackendRenderTarget(_direct.get(), backendRT,
														kTopLeft_GrSurfaceOrigin,
														kBGRA_8888_SkColorType, nullptr, &props);
		F_ASSERT(_surface);

		if (_raster) {
			_canvas = static_cast<SkiaCanvas*>(_rasterSurface->getCanvas());
		} else {
			_canvas = static_cast<SkiaCanvas*>(_surface->getCanvas());
		}
	}

	void SkiaMetalRender::onSubmit() {
		if (_raster)
			_rasterSurface->draw(_surface->getCanvas(), 0, 0);
		_surface->flushAndSubmit(); // commit sk
		_surface.reset();
		_canvas = nullptr;
	}

	SkiaMetalRender::SkiaMetalRender(Application* host, const Options& opts, bool raster)
	: MetalRender(host, opts) {
		_raster = raster;
	}

	SkiaCanvas* SkiaMetalRender::getCanvas() {
		if (!_canvas)
			begin();
		return _canvas;
	}

}
#endif
