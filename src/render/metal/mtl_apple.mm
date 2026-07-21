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

#include "src/util/macros.h"
#if Qk_ENABLE_METAL
#import "../plotforms.h"
#import "../metal/mtl_render.h"
#import "../metal/mtl_canvas.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#if Qk_MacOS
#import <QuartzCore/CVDisplayLink.h>
#import <QuartzCore/CAMetalDisplayLink.h>
#endif

using namespace qk;

inline CGSize CGSizeFromVec2(const Vec2 &vec2) {
	return CGSizeMake(vec2.x(), vec2.y());
}

inline Vec2 Vec2FromCGSize(const CGSize &size) {
	return Vec2(size.width, size.height);
}

namespace qk {
	void post_message_main(Cb cb, bool sync);
	MTLTextureID mtl_get_texture_from(const ImageSource* src, MTLTextureID _else = nil);
}

class AppleMetalRender;

@interface MTLSurfaceView: UIView
#if Qk_MacOS
<CAMetalDisplayLinkDelegate>
#endif
@property (nonatomic,assign) AppleMetalRender *render;
@property (nonatomic,readonly) Vec2 surfaceSize;
@end

class AppleMetalRender final: public MetalRender, public RenderSurface {
public:
	AppleMetalRender(Options opts)
		: MetalRender(opts)
		, _view(nil), _metalLayer(nil), _displayLink(nil), _isRun(false)
#if Qk_MacOS
		, _metalDisplayLink(nil), _metalDisplayThread(nil), _mainDisplayTimer(nil)
#endif
	{}

	void release() override {
		lock();
		stopDisplay();
		unlock();
		MetalRender::release();
		_view       = nil;
		_metalLayer = nil;
		Object::release();
	}

	bool isRenderThread() {
		return _threadId == thread_self_id();
	}

	RenderSurface* surface() override {
		return this;
	}

	void lock() {
		_mutex.lock();
	}

	void unlock() {
		_mutex.unlock();
	}

	Vec2 getSurfaceSize() override {
		Vec2 size = _view ? _view.surfaceSize : Vec2();
		Qk_ASSERT(!size.is_zero_axis());
		return size;
	}

	void setMainCanvasDrawable(id<CAMetalDrawable> drawable) {
		if (!drawable)
			return;
		_mtlcanvas->_outTex = drawable ? drawable.texture : nil;
		if (_mtlcanvas->_outColorTex != qk::mtl_get_texture_from(_mtlcanvas->_state->output.get())) {
			_mtlcanvas->_outColorTex = _mtlcanvas->_outTex;
		}
	}

	void renderDisplay(id<CAMetalDrawable> drawable = nil) {
		if (!_isRun)
			return;
		lock();
		_threadId = thread_self_id();

		Qk_ASSERT(_metalLayer, "Metal layer is null");

		// get next drawable for current frame
		if (!drawable) {
			// update drawable size before rendering
			_metalLayer.drawableSize = CGSizeFromVec2(_mtlcanvas->surfaceSize());
			drawable = _metalLayer.nextDrawable;
		}

		if (!drawable)
			return; // if drawable is nil, skip this frame

			// set the main canvas drawable for rendering
		setMainCanvasDrawable(drawable);

		if (_delegate->onRenderBackendDisplay() && _mtlcanvas->isRecorded()) {
			auto cmds = _mtlcanvas->flushBuffer();
			if (cmds.length()) {
				// _mtlcanvas->vportCopy(cmds.back(), drawable);
				[cmds.back() presentDrawable:drawable];
				for (auto cmd: cmds) {
					[cmd commit];
				}
			}
		}
		_threadId = qk::ThreadID();
		unlock();
	}

	UIView* surfaceView() override {
		if (_view) return _view;
		_view = [MTLSurfaceView new];
		_view.render = this;
		_metalLayer = (CAMetalLayer*)_view.layer;
		_metalLayer.device      = _device;
		_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		_metalLayer.opaque      = YES;
		_metalLayer.framebufferOnly = NO; // allow sampling from the drawable texture
		_metalLayer.drawableSize = CGSizeMake(1, 1);
		startDisplay();
		return _view;
	}

	void reload() override {
		lock();
		_surfaceSize = getSurfaceSize();
		_metalLayer.drawableSize = CGSizeFromVec2(_surfaceSize);
		_delegate->onRenderBackendReload(_surfaceSize);
		unlock();
	}

#if Qk_MacOS
	void onResize() {
		if (!_isRun) return;
		pauseDisplay();
		@autoreleasepool {
			reload();
			if (!_metalDisplayThread) {
				renderDisplay(); // immediately render display after resize
			}
		}
		resumeDisplay();
	}
#endif

private:
	void startDisplay() {
		if (_isRun) return;
		_isRun = true;
#if Qk_iOS
		_displayLink = [CADisplayLink displayLinkWithTarget:
			[NSBlockOperation blockOperationWithBlock:^{
				@autoreleasepool { renderDisplay(); }
			}]
			selector:@selector(main)];
		[_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
#else
		// if (@available(macOS 14.0, *)) {
		// 	startMetalDisplayLink();
		// 	return;
		// }
		// startMainThreadDisplayLoop(); // Debug Xcode Metal Capture on main thread.
		// return;
		CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
		CVDisplayLinkSetOutputHandler(_displayLink, ^CVReturn(CVDisplayLinkRef, const CVTimeStamp *,
																													const CVTimeStamp *, CVOptionFlags,
																													CVOptionFlags *) {
			@autoreleasepool { renderDisplay(); }
			return kCVReturnSuccess;
		});
		CVDisplayLinkStart(_displayLink);
#endif
	}

#if Qk_MacOS
	void pauseDisplay() {
		if (_mainDisplayTimer) {
			[_mainDisplayTimer setFireDate:[NSDate distantFuture]];
		} else if (_metalDisplayThread) {
			if (@available(macOS 14.0, *))
				((CAMetalDisplayLink*)_metalDisplayLink).paused = YES;
		} else {
			CVDisplayLinkStop(_displayLink);
		}
	}

	void resumeDisplay() {
		if (_mainDisplayTimer) {
			[_mainDisplayTimer setFireDate:[NSDate date]];
		} else if (_metalDisplayThread) {
			if (@available(macOS 14.0, *))
				((CAMetalDisplayLink*)_metalDisplayLink).paused = NO;
		} else {
			CVDisplayLinkStart(_displayLink);
		}
	}

	void startMainThreadDisplayLoop() {
		Qk_ASSERT([NSThread isMainThread], "Main thread display loop should start on main thread");
		auto timer = [NSTimer timerWithTimeInterval:(1.0 / 60.0) repeats:YES block:^(NSTimer*) {
			@autoreleasepool { renderDisplay(); }
		}];
		_mainDisplayTimer = timer;
		[[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
	}

	void startMetalDisplayLink() API_AVAILABLE(macos(14.0)) {
		_metalDisplayThread = [[NSThread alloc] initWithBlock:^{
			@autoreleasepool {
				auto link = [[CAMetalDisplayLink alloc] initWithMetalLayer:_metalLayer];
				link.delegate = _view;
				link.preferredFrameLatency = 1;
				_metalDisplayLink = link;
				[link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
				while (_metalDisplayLink/*![NSThread currentThread].cancelled*/) {
					@autoreleasepool {
						[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
							beforeDate:[NSDate dateWithTimeIntervalSinceNow:1.0]];
					}
				}
			}
		}];
		[_metalDisplayThread start];
	}
#endif

	void stopDisplay() {
		if (!_isRun) return;
		_isRun = false;
#if Qk_iOS
		[_displayLink invalidate];
#else
		if (_mainDisplayTimer) {
			[_mainDisplayTimer invalidate];
			_mainDisplayTimer = nil;
		} else if (_metalDisplayThread) {
			if (@available(macOS 14.0, *))
				[(CAMetalDisplayLink*)_metalDisplayLink invalidate];
			_metalDisplayLink = nil;
			_metalDisplayThread = nil;
		} else {
			CVDisplayLinkStop(_displayLink);
			CVDisplayLinkRelease(_displayLink);
		}
#endif
		_displayLink = nil;
	}

//fields:
	MTLSurfaceView *_view;
	CAMetalLayer   *_metalLayer;
	Mutex           _mutex;
	qk::ThreadID    _threadId;
	bool            _isRun;
#if Qk_iOS
	CADisplayLink  *_displayLink;
#else
	CVDisplayLinkRef _displayLink;
	id          _metalDisplayLink;
	NSThread   *_metalDisplayThread;
	NSTimer    *_mainDisplayTimer;
#endif
};

// -----------------------------------------------------------------------
// MTLSurfaceView implementation
// -----------------------------------------------------------------------

@implementation MTLSurfaceView
#if Qk_iOS
+ (Class)layerClass {
	return CAMetalLayer.class;
}
-(Vec2)surfaceSize {
	return Vec2FromCGSize(self.frame.size) * UIScreen.mainScreen.scale;
}
- (void)layoutSubviews {
	[super layoutSubviews];
}
#else // macOS
- (id)init {
	self = [super init];
	self.wantsLayer = YES;
	return self;
}
-(Vec2)surfaceSize {
	float scale = self.window ? self.window.backingScaleFactor : NSScreen.mainScreen.backingScaleFactor;
	return Vec2FromCGSize(self.frame.size) * scale;
}
- (CALayer*)makeBackingLayer {
	return [CAMetalLayer layer];
}
- (void)setFrameSize:(CGSize)size {
	[super setFrameSize:size];
	self.render->onResize();
}
- (void)metalDisplayLink:(CAMetalDisplayLink *)link
						 needsUpdate:(CAMetalDisplayLinkUpdate *)update API_AVAILABLE(macos(14.0)) {
	self.render->renderDisplay(update.drawable);
}
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
