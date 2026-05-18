/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

// ------------------- Metal ------------------

#if Qk_ENABLE_METAL
#import "../plotforms.h"
#import "../metal/mtl_render.h"
#import "../metal/mtl_canvas.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#if Qk_MacOS
#import <QuartzCore/CVDisplayLink.h>
#endif

using namespace qk;

@interface MTLSurfaceView: UIView
@end

class AppleMetalRender final: public MetalRender, public RenderSurface {
public:
	AppleMetalRender(Options opts)
		: MetalRender(opts)
		, _view(nil), _metalLayer(nil), _isRun(false)
		, _displayLink(nil)
	{}

	~AppleMetalRender() {
		Qk_CHECK(!_isRun);
	}

	void release() override {
		stopDisplay();
		MetalRender::release();
		_view       = nil;
		_metalLayer = nil;
		Object::release();
	}

	RenderSurface* surface() override {
		return this;
	}

	void lock() override {
		_mutex.lock();
	}

	void unlock() override {
		_mutex.unlock();
	}

	void post_message(Cb cb) override {
		post_message_main(cb, false);
	}

	Vec2 getSurfaceSize() override {
		if (!_view) return {};
#if Qk_iOS
		CGSize size  = _view.bounds.size;
		float  scale = UIScreen.mainScreen.scale;
#else
		CGRect frame  = _view.frame;
		CGSize size   = frame.size;
		float  scale  = _view.window ? _view.window.backingScaleFactor: UIScreen.mainScreen.backingScaleFactor;
#endif
		return Vec2(size.width * scale, size.height * scale);
	}

	void renderDisplay() {
		lock();
		if (_delegate->onRenderBackendDisplay()) {
			_mtlcanvas->flushBuffer();

			// Present canvas content to the CAMetalLayer drawable
			if (!_metalLayer) return;
			auto canvas = static_cast<MTLCanvas*>(mtlCanvas());
			if (!canvas || !canvas->colorTexture()) return;

#if Qk_iOS
			auto scale = UIScreen.mainScreen.scale;
#else
			auto scale = _view.window ? _view.window.backingScaleFactor : UIScreen.mainScreen.backingScaleFactor;
#endif
			_metalLayer.drawableSize = CGSizeMake(_view.bounds.size.width * scale, _view.bounds.size.height * scale);

			id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
			if (!drawable) return;

			// Copy from offscreen color texture to drawable using vportFullCp shader
			auto cb  = [_commandQueue commandBuffer];
			auto rpd = [MTLRenderPassDescriptor new];
			rpd.colorAttachments[0].texture     = drawable.texture;
			rpd.colorAttachments[0].loadAction  = MTLLoadActionDontCare;
			rpd.colorAttachments[0].storeAction = MTLStoreActionStore;

			auto enc = [cb renderCommandEncoderWithDescriptor:rpd];
			auto pso = blitPipeline(surfacePixelFormat());
			if (pso) {
				[enc setRenderPipelineState:pso];
				// vportFullCp vert: uses vertex_id only (no buffers)
				// frag: tex(0) = image (no PcArgs)
				[enc setFragmentTexture:canvas->colorTexture() atIndex:0];
				[enc setFragmentSamplerState:mtlSampler() atIndex:0];
				[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
			}
			[enc endEncoding];
			[cb presentDrawable:drawable];
			[cb commit];
		}
		unlock();
	}

	UIView* surfaceView() override {
		if (_view) return _view;
		_view = [[MTLSurfaceView alloc] initWithFrame:CGRectZero];
#if Qk_iOS
		_metalLayer = (CAMetalLayer*)_view.layer;
#else
		_view.wantsLayer = YES;
		_view.layer = [CAMetalLayer layer];
		_metalLayer = (CAMetalLayer*)_view.layer;
#endif
		_metalLayer.device      = _device;
		_metalLayer.pixelFormat = (MTLPixelFormat)surfacePixelFormat();
		_metalLayer.opaque      = YES;
		_metalLayer.framebufferOnly = YES;
		_metalLayer.drawableSize = CGSizeMake(1, 1);
		startDisplay();
		return _view;
	}

private:
	void startDisplay() {
		if (_isRun) return;
		_isRun = true;
		auto self = this;
#if Qk_iOS
		_displayLink = [CADisplayLink displayLinkWithTarget:
			[NSBlockOperation blockOperationWithBlock:^{
				if (self->_isRun)
					self->renderDisplay();
			}]
			selector:@selector(main)];
		[_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
#else
		CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
		CVDisplayLinkSetOutputHandler(_displayLink, ^CVReturn(CVDisplayLinkRef, const CVTimeStamp *,
																													const CVTimeStamp *, CVOptionFlags,
																													CVOptionFlags *) {
			// dispatch_async(dispatch_get_main_queue(), ^{
			if (self->_isRun)
				self->renderDisplay();
			// });
			return kCVReturnSuccess;
		});
		CVDisplayLinkStart(_displayLink);
#endif
	}

	void stopDisplay() {
		if (!_isRun) return;
		_isRun = false;
#if Qk_iOS
		[_displayLink invalidate];
		_displayLink = nil;
#else
		CVDisplayLinkStop(_displayLink);
		CVDisplayLinkRelease(_displayLink);
		_displayLink = nullptr;
#endif
	}

	MTLSurfaceView *_view;
	CAMetalLayer   *_metalLayer;
	bool            _isRun;
	Mutex           _mutex;
#if Qk_iOS
	CADisplayLink  *_displayLink;
#else
	CVDisplayLinkRef _displayLink;
#endif
};

// -----------------------------------------------------------------------
// MTLSurfaceView implementation
// -----------------------------------------------------------------------
@implementation MTLSurfaceView
#if Qk_iOS
+ (Class)layerClass { return CAMetalLayer.class; }
#else // macOS
- (CALayer*)makeBackingLayer { return [CAMetalLayer layer]; }
- (BOOL)wantsUpdateLayer { return YES; }
#endif
@end

// -----------------------------------------------------------------------
namespace qk {
	void* acquireRenderBackendStorage(size_t typeHash, size_t size);

	Render* make_metal_render(Render::Options opts) {
		auto mem = acquireRenderBackendStorage(typeid(AppleMetalRender).hash_code(), sizeof(AppleMetalRender));
		auto render = new(mem) AppleMetalRender(opts);
		return render;
	}
} // namespace qk
#endif // Qk_ENABLE_METAL
