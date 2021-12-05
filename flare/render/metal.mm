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

	MetalRender::MetalRender(Application* host, const Options& params)
		: Render(host, params)
		, _device(nil), _queue(nil)
		, _layer(nil)
		, _drawableHandle(nil), _pipelineArchive(nil) {
	}

	MetalRender::~MetalRender() {
		CFSafeRelease(_device); _device = nil;
		CFSafeRelease(_queue); _queue = nil;
		SkCFSafeRelease(_drawableHandle); _drawableHandle = nil;
	}

	void MetalRender::commit() {
		id<CAMetalDrawable> currentDrawable = (__bridge id<CAMetalDrawable>)_drawableHandle;
		id<MTLCommandBuffer> commandBuffer([_queue commandBuffer]);
		commandBuffer.label = @"Present";

		[commandBuffer presentDrawable:currentDrawable];
		[commandBuffer commit];
		// ARC is off in sk_app, so we need to release the CF ref manually
		CFRelease(_drawableHandle); _drawableHandle = nil;
		_surface.reset();
	}

	SkSurface* MetalRender::surface() {
		if (_surface) return _surface.get();
		if (!_direct) return nullptr;
	
		auto region = _host->display()->surface_region();
		auto rect = CGRectMake(0, 0, region.width, region.height);

		_layer.drawableSize = rect.size;
		
		SkSurfaceProps props(SkSurfaceProps::Flags(_opts.surfaceFlags), SkPixelGeometry(_opts.surfacePixelGeometry));
		if (_opts.delayDrawableAcquisition) {
			_surface = SkSurface::MakeFromCAMetalLayer(_direct.get(),
														(__bridge GrMTLHandle)_layer,
														kTopLeft_GrSurfaceOrigin, _sample_count,
														kBGRA_8888_SkColorType,
														nullptr,
														&props, &_drawableHandle);
		} else {
			CALayer* lay = _layer;
			
			id<CAMetalDrawable> currentDrawable = [_layer nextDrawable];
			id<MTLTexture> mttex = currentDrawable.texture;

			GrMtlTextureInfo fbInfo;
			fbInfo.fTexture.retain((__bridge void*)mttex);

			auto region = _host->display()->surface_region();
			
			GrBackendRenderTarget backendRT(region.width, region.height, _sample_count, fbInfo);

			_surface = SkSurface::MakeFromBackendRenderTarget(_direct.get(), backendRT,
															kTopLeft_GrSurfaceOrigin,
															SkColorType(_opts.colorType),
															nullptr, &props);

			_drawableHandle = CFRetain((GrMTLHandle) currentDrawable);
		}

		return _surface.get();
	}

	void MetalRender::reload() {
		// clean surface
		if (_drawableHandle) {
			CFRelease(_drawableHandle); _drawableHandle = nil;
		}
		_surface.reset();

		if (_direct) return;

		_device = CFSafeRetain(MTLCreateSystemDefaultDevice());
		_queue = CFSafeRetain([_device newCommandQueue]);
		_sample_count = F_MAX(1, _opts.msaaSampleCount);
		_stencil_bits = 8;

		if (_sample_count > 1) {
			if (![_device supportsTextureSampleCount:_sample_count]) {
				_sample_count /= 2;
				reload();
				return;
			}
		}

		F_ASSERT(_layer);
		_layer.device = _device;
		_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		_layer.opaque = YES;
		//_layer.drawableSize = rect.size;
		//_layer.displaySyncEnabled = _opts.disableVsync ? NO : YES;

		GrMtlBackendContext backendContext = {};
		backendContext.fDevice.retain((__bridge void*)_device);
		backendContext.fQueue.retain((__bridge void*)_queue);
		
#if GR_METAL_SDK_VERSION >= 230
		if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
			if (_pipelineArchive) {
				// 'release' is unavailable: not available in automatic reference counting mode
				// [_PipelineArchive release];
			}
			if (_sample_count > 1) {
					auto desc = [MTLBinaryArchiveDescriptor new];
					desc.url = CacheURL(); // try to load
					NSError* error;
					_pipelineArchive = [_device newBinaryArchiveWithDescriptor:desc error:&error];
					if (!_pipelineArchive) {
						desc.url = nil; // create new
						NSError* error;
						_pipelineArchive = [_device newBinaryArchiveWithDescriptor:desc error:&error];
						if (!_pipelineArchive) {
							SkDebugf("Error creating MTLBinaryArchive:\n%s\n",
									error.debugDescription.UTF8String);
						}
					}
			} else {
				_pipelineArchive = nil;
			}
			backendContext.fBinaryArchive.retain((__bridge GrMTLHandle)_pipelineArchive);
		}
#endif

		_direct = GrDirectContext::MakeMetal(backendContext, {/*_opts.grContextOptions*/});
		F_ASSERT(_direct);
	}

	void MetalRender::activate(bool isActive) {
		// serialize pipeline archive
		if (!isActive) {
#if GR_METAL_SDK_VERSION >= 230
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
#endif
		}
	}

	NSURL* MetalRender::CacheURL() {
		NSArray *paths = [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory
																inDomains:NSUserDomainMask];
		NSURL* cachePath = [paths objectAtIndex:0];
		return [cachePath URLByAppendingPathComponent:@"binaryArchive.metallib"];
	}

}   //namespace flare
