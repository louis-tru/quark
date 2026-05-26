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

namespace qk {
	void post_message_main(Cb cb, bool sync);
}

class AppleMetalRender;

@interface MTLSurfaceView: UIView<CALayerDelegate>
@property (nonatomic,assign) AppleMetalRender *render;
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
		lock();
		stopDisplay();
		unlock();
		MetalRender::release();
		resolvedMsg(true);
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

	void lock() override {
		_mutex.lock();
	}

	void unlock() override {
		_mutex.unlock();
	}

	void post_message(Cb cb) override {
#if Qk_iOS
		post_message_main(cb, false);
#else
		if (_view && isRenderThread()) {
			cb->resolve(); // immediately resolve
		} else if (!_isRun) { // is not running
			if (_mutexMsg.try_lock()) { // releaseing render, try to lock msg mutex
				_msg.push(cb);
				_mutexMsg.unlock();
			} else {
				cb->resolve(); // if failed to lock, immediately resolve the message
			}
		} else {
			_mutexMsg.lock();
			_msg.push(cb);
			_mutexMsg.unlock();
		}
#endif
	}

	void resolvedMsg(bool destroy) {
		if (destroy) {
			_mutexMsg.lock();
			if (_msg.length()) {
				lock();
				for (auto &i : _msg) i->resolve();
				_msg.clear();
				unlock();
			}
			_mutexMsg.unlock();
		} else if (_msg.length()) {
			_mutexMsg.lock();
			auto msg(std::move(_msg));
			_mutexMsg.unlock();
			for ( auto &i : msg ) i->resolve();
		}
	}

	Vec2 getSurfaceSize() override {
		if (!_view) return {};
#if Qk_iOS
		CGSize size  = _view.bounds.size;
		float  scale = UIScreen.mainScreen.scale;
#else
		CGSize size   = _view.frame.size;
		float  scale  = _view.window ? _view.window.backingScaleFactor: UIScreen.mainScreen.backingScaleFactor;
#endif
		return Vec2(size.width * scale, size.height * scale);
	}

	void renderDisplay() {
		lock();
		_threadId = thread_self_id();

		resolvedMsg(false);

		if (_delegate->onRenderBackendDisplay() && _mtlcanvas->isRecorded()) {
			Qk_ASSERT(_metalLayer, "Metal layer is null");
			// update drawable size before rendering
			_metalLayer.drawableSize = CGSizeMake(_mtlcanvas->surfaceSize().x(), _mtlcanvas->surfaceSize().y());
			// get next drawable for current frame
			id<CAMetalDrawable> drawable = _metalLayer.nextDrawable;
			// flush command buffers and present drawable if available
			if (drawable) {
				auto cmds = _mtlcanvas->flushBuffer();
				if (cmds.length()) {
					_mtlcanvas->vportCopy(cmds.back(), drawable);
					for (auto cmd: cmds) {
						[cmd commit];
					}
				} // if (cmds.length())
			} // if (drawable)
		}
		_threadId = qk::ThreadID();
		unlock();
	}

	UIView* surfaceView() override {
		if (_view) return _view;
		_view = [[MTLSurfaceView alloc] initWithFrame:CGRectZero];
		_view.render = this;
#if Qk_iOS
		_metalLayer = (CAMetalLayer*)_view.layer;
#else
		_view.wantsLayer = YES;
		_view.layer = [CAMetalLayer layer];
		_metalLayer = (CAMetalLayer*)_view.layer;
#endif
		_metalLayer.device      = _device;
		_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		_metalLayer.opaque      = YES;
		_metalLayer.framebufferOnly = YES;
		_metalLayer.drawableSize = CGSizeMake(1, 1);
		startDisplay();
		return _view;
	}

	void onResize() {
		if (_isRun) {
#if Qk_MacOS
			CVDisplayLinkStop(_displayLink);
#endif
			@autoreleasepool {
				reload();
				renderDisplay();
			}
#if Qk_MacOS
			CVDisplayLinkStart(_displayLink);
#endif
		}
	}

private:
	void startDisplay() {
		if (_isRun) return;
		_isRun = true;
		auto self = this;
#if Qk_iOS
		_displayLink = [CADisplayLink displayLinkWithTarget:
			[NSBlockOperation blockOperationWithBlock:^{
				@autoreleasepool {
					if (self->_isRun)
						self->renderDisplay();
				}
			}]
			selector:@selector(main)];
		[_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
#else
		CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
		CVDisplayLinkSetOutputHandler(_displayLink, ^CVReturn(CVDisplayLinkRef, const CVTimeStamp *,
																													const CVTimeStamp *, CVOptionFlags,
																													CVOptionFlags *) {
			@autoreleasepool {
				if (self->_isRun)
					self->renderDisplay();
			}
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
	Array<Cb>       _msg;
	Mutex           _mutexMsg;
	Mutex           _mutex;
	qk::ThreadID    _threadId;
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
- (void)layoutSubviews {
	[super layoutSubviews];
}
#else // macOS
- (CALayer*)makeBackingLayer {
	return [CAMetalLayer layer];
}
- (BOOL)wantsUpdateLayer {
	return YES;
}
- (void)setFrameSize:(CGSize)newSize {
	[super setFrameSize:newSize];
	self.render->onResize();
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
