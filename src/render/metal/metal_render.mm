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
 *MTLTexture
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

#include "./metal_render.h"
#include "../../app.h"
#include "../../display.h"

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

namespace qk {

	MetalRender::MetalRender(Options opts, Delegate *delegate)
		: Render(opts, delegate)
		, _queue(nil), _device(nil)
		, _view(nil), _layer(nil)
		, _drawable(nil), _pipelineArchive(nil) {
		_opts.colorType = kColor_Type_BGRA_8888; // metal mode can only use BGR
	}

	MetalRender::~MetalRender() {
		CFSafeRelease(_device); _device = nil;
		CFSafeRelease(_queue); _queue = nil;
		CFSafeRelease(_drawable); _drawable = nil;
	}

	static void test(id<CAMetalDrawable> drawable, id<MTLCommandBuffer> cmd) {
		MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		passDescriptor.colorAttachments[0].texture = drawable.texture;
		passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.5, 0, 1, 0.5);
		passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

		id<MTLRenderCommandEncoder> commandEncoder = [cmd renderCommandEncoderWithDescriptor:passDescriptor];
		[commandEncoder endEncoding];
	}

	void MetalRender::reload() {
		if (!_device) {
			_device = CFSafeRetain(MTLCreateSystemDefaultDevice());
			_queue = CFSafeRetain([_device newCommandQueue]);
			Qk_ASSERT(_device);

			if (_opts.msaa > 1) {
				while (![_device supportsTextureSampleCount:_opts.msaa])
					_opts.msaa /= 2;
			}
			
			if (_view) {
				_view.device = _device;
				_view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
				// _view.sampleCount = _opts.msaa;
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
		}

		auto region = _host->display()->display_region();

		// clean surface
		CFSafeRelease(_drawable); _drawable = nil;
		
		if (_view) {
			_view.drawableSize = CGRectMake(0, 0, region.width, region.height).size;
		}

		if (@available(iOS 13.0, *)) {
			if (_layer) {
				_layer.drawableSize = CGRectMake(0, 0, region.width, region.height).size;
			}
		}

		onReload();
	}

	void MetalRender::begin() {
		Qk_ASSERT(!_drawable);
		id<CAMetalDrawable> drawable;

		if (@available(iOS 13.0, *)) {
			drawable = ((CAMetalLayer*)_view.layer).nextDrawable;
		} else {
			drawable = _view.currentDrawable;
		}

		_drawable = CFSafeRetain(drawable);
		// Qk_DEBUG("CFGetRetainCount, %d", CFGetRetainCount(_drawableHandle));

		onBegin();
	}

	void MetalRender::submit() {
		onSubmit();

		id<MTLCommandBuffer> cmd = _queue.commandBuffer;
		//test(_drawable, cmd);

		//id<MTLTexture> mttex = ((__bridge id<CAMetalDrawable>)_drawable).texture;
		//Qk_DEBUG("sampleCount, %d, %d", mttex.sampleCount, _opts.msaa);

		[cmd presentDrawable:_drawable];
		[cmd commit];

		// ARC is off in sk_app, so we need to release the CF ref manually
		CFSafeRelease(_drawable); _drawable = nil;
	}

	void MetalRender::activate(bool isActive) {
		// serialize pipeline archive
	}

}