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

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/mtl/GrMtlBackendContext.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "./metal.h"
#include "../display.h"

namespace flare {

	MetalRender::MetalRender(Application* host, const DisplayParams& params)
		: Render(host, params)
		, _Device(nil), _Queue(nil),
		, _layer(nil)
		, _DrawableHandle(nil), _PipelineArchive(nil) {
	}

	void MetalRender::initialize() {
	}

	void MetalRender::start() {
	}

	void MetalRender::commit() {
		id<CAMetalDrawable> currentDrawable = (id<CAMetalDrawable>)_DrawableHandle;

		id<MTLCommandBuffer> commandBuffer([*_Queue commandBuffer]);
		commandBuffer.label = @"Present";

		[commandBuffer presentDrawable:currentDrawable];
		[commandBuffer commit];
		// ARC is off in sk_app, so we need to release the CF ref manually
		CFRelease(_DrawableHandle);
		_DrawableHandle = nil;
		_Surface.reset();
	}

	sk_sp<SkSurface> MetalRender::getSurface() {
		if (!_Surface) {
			if (_Context) {
				if (_DisplayParams.fDelayDrawableAcquisition) {
					_Surface = SkSurface::MakeFromCAMetalLayer(_Context.get(),
																(__bridge GrMTLHandle)_layer,
																kTopLeft_GrSurfaceOrigin, _SampleCount,
																kBGRA_8888_SkColorType,
																_DisplayParams.fColorSpace,
																&_DisplayParams.fSurfaceProps,
																&_DrawableHandle);
				} else {
					id<CAMetalDrawable> currentDrawable = [_layer nextDrawable];

					GrMtlTextureInfo fbInfo;
					fbInfo.fTexture.retain(currentDrawable.texture);

					auto size = _host->display()->size();
					float width = size.x();
					float height = size.y();

					GrBackendRenderTarget backendRT(width,
													height,
													_SampleCount,
													fbInfo);

					_Surface = SkSurface::MakeFromBackendRenderTarget(_Context.get(), backendRT,
																	kTopLeft_GrSurfaceOrigin,
																	kBGRA_8888_SkColorType,
																	_DisplayParams.fColorSpace,
																	&_DisplayParams.fSurfaceProps);

					_DrawableHandle = CFRetain((GrMTLHandle) currentDrawable);
				}
			}
		}

		return _Surface;
	}

	void MetalRender::reload() {
		if (_Context) {
			// in case we have outstanding refs to this (lua?)
			_Context->abandonContext();
			_Context.reset();
		}

	#if GR_METAL_SDK_VERSION >= 230
		if (@available(macOS 11.0, iOS 14.0, *)) {
			[_PipelineArchive release];
		}
	#endif
		if (_DrawableHandle) {
			CFRelease(_DrawableHandle);
			_DrawableHandle = nil;
		}
		_Surface.reset();

		// -------------------------------
		if (!_Device) {
			_Device.reset(MTLCreateSystemDefaultDevice());
			_Queue.reset([*_Device newCommandQueue]);
		}

		if (_DisplayParams.fMSAASampleCount > 1) {
			if (@available(macOS 10.11, iOS 9.0, *)) {
				if (![*_Device supportsTextureSampleCount:_DisplayParams.fMSAASampleCount]) {
					_DisplayParams.fMSAASampleCount /= 2;
					reload();
					return;
				}
			} else {
				return;
			}
		}

		ASSERT(_layer);

		auto region = _host->display()->surface_region();
		auto rect = CGRectMake(0, 0, region.width, region.height);

		_layer.device = _Device.get();
		_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		_layer.drawableSize = rect.size;
		// _layer.frame = rect;
		// _layer.displaySyncEnabled = _DisplayParams.fDisableVsync ? NO : YES;
		// _layer.contentsGravity = kCAGravityTopLeft;

		_SampleCount = _DisplayParams.fMSAASampleCount;
		_StencilBits = 8;

	#if GR_METAL_SDK_VERSION >= 230
		if (_DisplayParams.fEnableBinaryArchive) {
			if (@available(macOS 11.0, iOS 14.0, *)) {
				sk_cfp<MTLBinaryArchiveDescriptor*> desc([MTLBinaryArchiveDescriptor new]);
				(*desc).url = CacheURL(); // try to load
				NSError* error;
				_PipelineArchive = [*_Device newBinaryArchiveWithDescriptor:*desc error:&error];
				if (!_PipelineArchive) {
					(*desc).url = nil; // create new
					NSError* error;
					_PipelineArchive = [*_Device newBinaryArchiveWithDescriptor:*desc error:&error];
					if (!_PipelineArchive) {
						SkDebugf("Error creating MTLBinaryArchive:\n%s\n",
								error.debugDescription.UTF8String);
					}
				}
			}
		} else {
			if (@available(macOS 11.0, iOS 14.0, *)) {
				_PipelineArchive = nil;
			}
		}
	#endif

		GrMtlBackendContext backendContext = {};
		backendContext.fDevice.retain((__bridge GrMTLHandle)_Device.get());
		backendContext.fQueue.retain((__bridge GrMTLHandle)_Queue.get());
	#if GR_METAL_SDK_VERSION >= 230
		if (@available(macOS 11.0, iOS 14.0, *)) {
			backendContext.fBinaryArchive.retain((__bridge GrMTLHandle)_PipelineArchive);
		}
	#endif

		_Context = GrDirectContext::MakeMetal(backendContext, _DisplayParams.fGrContextOptions);

		ASSERT(_Context);
	}

	void MetalRender::activate(bool isActive) {
		// serialize pipeline archive
		if (!isActive) {
	#if GR_METAL_SDK_VERSION >= 230
			if (@available(macOS 11.0, iOS 14.0, *)) {
				if (_PipelineArchive) {
					NSError* error;
					[_PipelineArchive serializeToURL:CacheURL() error:&error];
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
