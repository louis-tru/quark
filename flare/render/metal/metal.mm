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
 *MTLTexture
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

#include "./metal.h"
#include "../../app.h"
#include "../../display.h"

#include "skia/core/SkCanvas.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrBackendSurface.h"
#include "skia/gpu/GrDirectContext.h"
#include "skia/gpu/mtl/GrMtlBackendContext.h"
#include "skia/gpu/mtl/GrMtlTypes.h"
#include "skia/private/GrMtlTypesPriv.h"

#import <MetalKit/MTKView.h>

template <typename T> static inline T CFSafeRetain(T obj) {
	if (obj) {
		CFRetain((__bridge void*)obj);
	}
	return obj;
}

template <typename T> static inline void CFSafeRelease(T obj) {
	if (obj) {
		CFRelease((__bridge void*)obj);
	}
}

namespace flare {

	MetalRender::MetalRender(Application* host, const Options& opts)
		: Render(host, opts)
		, _queue(nil), _device(nil)
		, _view(nil), _layer(nil)
		, _drawable(nil), _pipelineArchive(nil) {
	}

	MetalRender::~MetalRender() {
		CFSafeRelease(_device); _device = nil;
		CFSafeRelease(_queue); _queue = nil;
		SkCFSafeRelease(_drawable); _drawable = nil;
	}

	static void test(GrMTLHandle drawable, id<MTLCommandBuffer> cmd) {
		 MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		 passDescriptor.colorAttachments[0].texture = ((__bridge id<CAMetalDrawable>)drawable).texture;
		 passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.5, 0, 1, 0.5);
		 passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		 passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

		 id<MTLRenderCommandEncoder> commandEncoder = [cmd renderCommandEncoderWithDescriptor:passDescriptor];
		 [commandEncoder endEncoding];
	}

	void MetalRender::submit() {
		id<MTLCommandBuffer> cmd = _queue.commandBuffer;
		//test(_drawable, cmd);

		//id<MTLTexture> mttex = ((__bridge id<CAMetalDrawable>)_drawable).texture;
		//F_DEBUG("sampleCount, %d, %d", mttex.sampleCount, _opts.msaaSampleCnt);

		_surface->flushAndSubmit(); // commit sk

		[cmd presentDrawable:(__bridge id<CAMetalDrawable>)_drawable];
		[cmd commit];

		_surface.reset();
		// ARC is off in sk_app, so we need to release the CF ref manually
		SkCFSafeRelease(_drawable); _drawable = nil;
	}

	SkSurface* MetalRender::surface() {
		if (_surface)
			return _surface.get();
		if (!_direct)
			return nullptr;

		id<CAMetalDrawable> drawable;
		
		if (@available(iOS 13.0, *)) {
			drawable = ((CAMetalLayer*)_view.layer).nextDrawable;
		} else {
			drawable = _view.currentDrawable;
		}

		id<MTLTexture> tex = drawable.texture;
		
		GrMtlTextureInfo fbInfo;
		fbInfo.fTexture.retain((__bridge void*)tex);
		
		//auto region = _host->display()->surface_region();
		//F_DEBUG("width, %f==%d", region.width, tex.width);
		//F_DEBUG("height, %f==%d", region.height, tex.height);
		//tex.sampleCount = _opts.msaaSampleCnt;
		//F_DEBUG("%d, %d", tex.sampleCount, _opts.msaaSampleCnt);

		GrBackendRenderTarget backendRT((int)tex.width, (int)tex.height, _opts.msaaSampleCnt, fbInfo);
		SkSurfaceProps props(_opts.flags, kUnknown_SkPixelGeometry);
		
		_surface = SkSurface::MakeFromBackendRenderTarget(_direct.get(), backendRT,
														kTopLeft_GrSurfaceOrigin,
														kBGRA_8888_SkColorType, nullptr, &props);

		_drawable = CFRetain((GrMTLHandle) drawable);
		// F_DEBUG("CFGetRetainCount, %d", CFGetRetainCount(_drawableHandle));

		return _surface.get();
	}

	void MetalRender::reload() {
		auto region = _host->display()->display_region();

		if (!_direct) {
			_device = CFSafeRetain(MTLCreateSystemDefaultDevice());
			_queue = CFSafeRetain([_device newCommandQueue]);

			if (_opts.msaaSampleCnt > 1) {
				while (![_device supportsTextureSampleCount:_opts.msaaSampleCnt])
					_opts.msaaSampleCnt /= 2;
			}
			
			if (_view) {
				_view.device = _device;
				_view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
				// _view.sampleCount = _opts.msaaSampleCnt;
			}
			
			if (@available(iOS 13.0, *)) {
				if (_view) {
					_layer = (CAMetalLayer*)_view.layer;
				} else {
					_layer.device = _device;
					_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
					//_layer.displaySyncEnabled = _opts.disableVsync ? NO : YES;
				}
			}
			
			GrMtlBackendContext backendContext = {};
			backendContext.fDevice.retain((__bridge void*)_device);
			backendContext.fQueue.retain((__bridge void*)_queue);

			_direct = GrDirectContext::MakeMetal(backendContext, {/*_opts.grContextOptions*/});
			F_ASSERT(_direct);
		}

		// clean surface
		SkCFSafeRelease(_drawable); _drawable = nil;
		_surface.reset();
		
		if (_view) {
			_view.drawableSize = CGRectMake(0, 0, region.width, region.height).size;
		}

		if (@available(iOS 13.0, *)) {
			if (_layer) {
				_layer.drawableSize = CGRectMake(0, 0, region.width, region.height).size;
			}
		}
	}

	void MetalRender::activate(bool isActive) {
		// serialize pipeline archive
	}

	// ----------------------------- R a s t e r . M e t a l . R e n d e r -----------------------------


	RasterMetalRender::RasterMetalRender(Application* host, const Options& opts): MetalRender(host, opts) {
	}

	SkSurface* RasterMetalRender::surface() {
		if (!_rasterSurface) {
			auto region = _host->display()->display_region();
			auto info = SkImageInfo::Make(region.width, region.height,
																		SkColorType(_opts.colorType), kPremul_SkAlphaType, nullptr);
			_rasterSurface = SkSurface::MakeRaster(info);
		}
		return _rasterSurface.get();
	}

	void RasterMetalRender::reload() {
		_opts.stencilBits = 0;
		_opts.msaaSampleCnt = 0;
		MetalRender::reload();
		_rasterSurface.reset();
	}

	void RasterMetalRender::submit() {
		// draw to gl canvas
		_rasterSurface->draw(MetalRender::surface()->getCanvas(), 0, 0);
		MetalRender::submit();
	}

}   //namespace flare
