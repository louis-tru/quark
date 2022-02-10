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

#include "../app.h"
#include "./metal.h"
#include "../display.h"

#include "skia/core/SkCanvas.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrBackendSurface.h"
#include "skia/gpu/GrDirectContext.h"
#include "skia/gpu/mtl/GrMtlBackendContext.h"
#include "skia/gpu/mtl/GrMtlTypes.h"
#include "skia/private/GrMtlTypesPriv.h"

#import <MetalKit/MTKView.h>
#import <GLKit/GLKView.h>

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
		, _queue(nil)
		, _view(nil)
		, _drawable(nil), _pipelineArchive(nil) {
	}

	MetalRender::~MetalRender() {
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
		
		auto region = _host->display()->surface_region();

		id<CAMetalDrawable> drawable = _view.currentDrawable;
		id<MTLTexture> texture = drawable.texture;

		GrMtlTextureInfo fbInfo;
		fbInfo.fTexture.retain((__bridge void*)texture);

		GrBackendRenderTarget backendRT(region.width, region.height, _opts.msaaSampleCnt, fbInfo);
		SkSurfaceProps props(_opts.flags, kUnknown_SkPixelGeometry);
		
		//texture.sampleCount = _opts.msaaSampleCnt;
		//F_DEBUG("%d, %d", texture.sampleCount, _opts.msaaSampleCnt);

		_surface = SkSurface::MakeFromBackendRenderTarget(_direct.get(), backendRT,
														kTopLeft_GrSurfaceOrigin,
														kBGRA_8888_SkColorType, nullptr, &props);

		_drawable = CFRetain((GrMTLHandle) drawable);
		// F_DEBUG("CFGetRetainCount, %d", CFGetRetainCount(_drawableHandle));

		return _surface.get();
	}

	NSURL* CacheURL() {
		 NSArray *paths = [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory
																														 inDomains:NSUserDomainMask];
		 NSURL* cachePath = [paths objectAtIndex:0];
		 return [cachePath URLByAppendingPathComponent:@"binaryArchive.metallib"];
	 }

	void MetalRender::reload() {
		auto region = _host->display()->surface_region();

		if (!_direct) {
			if (_opts.msaaSampleCnt > 1) {
				while (![_view.device supportsTextureSampleCount:_opts.msaaSampleCnt])
					_opts.msaaSampleCnt /= 2;
			}

			_queue = CFSafeRetain([_view.device newCommandQueue]);
			_view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
			_view.sampleCount = _opts.msaaSampleCnt;
			//_layer.displaySyncEnabled = _opts.disableVsync ? NO : YES;
			
			GrMtlBackendContext backendContext = {};
			backendContext.fDevice.retain((__bridge void*)_view.device);
			backendContext.fQueue.retain((__bridge void*)_queue);
			
			#if GR_METAL_SDK_VERSION >= 230
			if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
				if (_pipelineArchive) {
					// 'release' is unavailable: not available in automatic reference counting mode
					// [_pipelineArchive release];
				}
				if (_opts.msaaSampleCnt > 1) {
						auto desc = [MTLBinaryArchiveDescriptor new];
						desc.url = CacheURL(); // try to load
						NSError* error;
						_pipelineArchive = [_view.device newBinaryArchiveWithDescriptor:desc error:&error];
						if (!_pipelineArchive) {
							desc.url = nil; // create new
							NSError* error;
							_pipelineArchive = [_view.device newBinaryArchiveWithDescriptor:desc error:&error];
							if (!_pipelineArchive) {
								F_DEBUG("Error creating MTLBinaryArchive:\n%s", error.debugDescription.UTF8String);
							}
						}
					
				} else {
					_pipelineArchive = nil;
				}
				//backendContext.fBinaryArchive.retain((__bridge GrMTLHandle)_pipelineArchive);
			}
			#endif

			_direct = GrDirectContext::MakeMetal(backendContext, {/*_opts.grContextOptions*/});
			F_ASSERT(_direct);
		}

		// clean surface
		SkCFSafeRelease(_drawable); _drawable = nil;
		_surface.reset();

		_view.drawableSize = CGRectMake(0, 0, region.width, region.height).size;
	}

	void MetalRender::activate(bool isActive) {
		// serialize pipeline archive
		#if GR_METAL_SDK_VERSION >= 230
		if (!isActive) {
			if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
				if (_pipelineArchive) {
					NSError* error;
					[_pipelineArchive serializeToURL:CacheURL() error:&error];
					if (error) {
						SkDebugf("Error storing MTLBinaryArchive:\n%s\n",
								error.debugDescription.UTF8String);
					}
				}
			}
		}
		#endif
	}

	// ----------------------------- R a s t e r . M e t a l . R e n d e r -----------------------------


	RasterMetalRender::RasterMetalRender(Application* host, const Options& opts): MetalRender(host, opts) {
	}

	SkSurface* RasterMetalRender::surface() {
		if (!_rasterSurface) {
			auto region = _host->display()->surface_region();
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
