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

#include "../../util/macros.h"
#if Qk_ENABLE_GL && Qk_MacOS

#define GL_SILENCE_DEPRECATION
#define GL3_PROTOTYPES
#import "../plotforms.h"
#import "./gl_render.h"
#include "./gl_command.h"

using namespace qk;

class MacGLRender;
@interface GLView: NSOpenGLView
- (id) init:(NSOpenGLContext*)ctx render:(MacGLRender*)r;
@end

// ----------------------------------------------------------------------------------------------

class MacGLRender final: public GLRender, public RenderSurface {
 public:
	MacGLRender(Options opts, NSOpenGLContext *ctx)
		: GLRender(opts), _view(nil), _ctx(ctx), _displayLink(nil), _isRun(false)
	{
	}

	void release() override {
		lock();
		stopDisplay();
		unlock();
		GLRender::release();
		resolvedMsg(true);
		_view = nil;
		_ctx = nil;
		Object::release(); // final destruction
	}

	bool isRenderThread() {
		return _threadId == thread_self_id();
	}

	void lock() override {
		if (!isRenderThread()) { // avoid deadlock if already in render thread
			CGLLockContext(_ctx.CGLContextObj);
			[_ctx makeCurrentContext];
		}
	}

	void unlock() override {
		if (!isRenderThread()) {
			CGLUnlockContext(_ctx.CGLContextObj);
		}
	}

	RenderSurface* surface() override {
		return this;
	}

	void post_message(Cb cb) override {
		Qk_ASSERT(_ctx, "Render context is null. Cannot post message.");
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
		CGSize size = _view.frame.size;
		float defaultScale = _view.window.backingScaleFactor;
		return Vec2(size.width * defaultScale, size.height * defaultScale);
	}

	void renderDisplay() {
		CGLLockContext(_ctx.CGLContextObj);
		// Make the context current
		[_ctx makeCurrentContext];
		_threadId = thread_self_id();
		Qk_ASSERT_EQ(NSOpenGLContext.currentContext, _ctx, "Failed to set current OpenGL context");

		resolvedMsg(false); // resolve messages before display

		if (_delegate->onRenderBackendDisplay()) {
			_glcanvas->flushBuffer(); // commit gl canvas cmds
			_glcanvas->vportCopy(0); // copy pixels to default color buffer
			glFlush(); // flush gl buffer, glFinish, glFenceSync, glWaitSync
			// [_ctx flushBuffer]; // swap double buffer
		}
		_threadId = qk::ThreadID();
		CGLUnlockContext(_ctx.CGLContextObj);
	}

	UIView* surfaceView() override {
		if (_view)
			return _view;
		_view = [[GLView alloc] init:_ctx render:this];
		_view.wantsBestResolutionOpenGLSurface = YES; // Enable retina-support
		_view.wantsLayer = YES; // Enable layer-backed drawing of view
		_view.layer.opaque = NO;
		_view.layer.backgroundColor = CGColorCreateGenericRGB(0, 0, 0, 0);

		GLint swapInterval = 1; // enable vsync
		[_ctx setValues:&swapInterval forParameter:NSOpenGLContextParameterSwapInterval];//NSOpenGLCPSwapInterval
		GLint sampleCount; // read msaa cnt
		[_ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
		startDisplay(); // start display link
		return _view;
	}

	void onResize() {
		if (_isRun) {
			CVDisplayLinkStop(_displayLink);
			@autoreleasepool {
				reload();
				renderDisplay(); // immediately render display after resize
			}
			CVDisplayLinkStart(_displayLink);
		}
	}

private:
	void renderDisplayIf() {
		if (_isRun) {
			@autoreleasepool {
				renderDisplay();
			}
		}
	}

	void startDisplay() {
		if (_isRun) return;
		_isRun = true;
		// Create a display link capable of being used with all active displays
		CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
		// Set the renderer output callback function
		CVDisplayLinkSetOutputHandler(_displayLink, ^CVReturn(CVDisplayLinkRef, const CVTimeStamp *,
																													const CVTimeStamp *, CVOptionFlags,
																													CVOptionFlags *) {
			renderDisplayIf();
			return kCVReturnSuccess;
		});
		// Set the display link for the current renderer
		CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(
			_displayLink, _ctx.CGLContextObj, _ctx.pixelFormat.CGLPixelFormatObj
		);
		// Activate the display link
		CVDisplayLinkStart(_displayLink);
	}

	void stopDisplay() {
		if (!_isRun) return;
		_isRun = false;
		CVDisplayLinkStop(_displayLink);
		CVDisplayLinkRelease(_displayLink);
		_displayLink = nil;
	}

//fields:
	GLView           *_view;
	NSOpenGLContext  *_ctx;
	Array<Cb>        _msg;
	Mutex            _mutexMsg;
	qk::ThreadID     _threadId;
	bool            _isRun;
	CVDisplayLinkRef _displayLink;
};

// ----------------------------------------------------------------------------------------------

@implementation GLView {
	MacGLRender *_render;
}
- (id)init:(NSOpenGLContext*)ctx render:(MacGLRender*)r {
	if ((self = [super initWithFrame:CGRectZero pixelFormat:nil])) {
		[self setOpenGLContext:ctx];
		_render = r;
	}
	return self;
}

- (BOOL)isOpaque { return NO; }

- (void)drawRect:(NSRect)dirtyRect {
	_render->onResize();
}
@end

// ----------------------------------------------------------------------------------------------

namespace qk {

	class MacRenderResource: public GLRenderResource {
	public:
		Qk_DEFINE_PROP_GET(NSOpenGLContext*, ctx);
		MacRenderResource(NSOpenGLContext* ctx)
			: GLRenderResource(current_loop()), _ctx(ctx) {
		}
	};

	void* acquireRenderBackendStorage(size_t typeHash, size_t size);

	// A shared render resource for all render instances, used to share GL resources like textures, buffers, etc.
	static MacRenderResource* g_sharedRenderResource = nullptr;

	RenderResource* getSharedRenderResource() {
		return g_sharedRenderResource;
	}

	Render* make_gl_render(Render::Options opts) {
		//	generate the GL display mask for all displays
		CGDirectDisplayID		dspys[10];
		CGDisplayCount			count = 0;
		GLuint							glDisplayMask = 0;

		if (CGGetActiveDisplayList(10, dspys, &count) == kCGErrorSuccess)	{
			for (int i = 0; i < count; i++)
				glDisplayMask = glDisplayMask | CGDisplayIDToOpenGLDisplayMask(dspys[i]);
		}

		uint32_t MSAA = opts.msaaSample;
		uint32_t i = 0;
		NSOpenGLPixelFormatAttribute attrs[32] = {0};

		attrs[i++] = NSOpenGLPFAAccelerated; // Choose a hardware accelerated renderer
		attrs[i++] = NSOpenGLPFAClosestPolicy;
		//attrs[i++] = NSOpenGLPFADoubleBuffer; // use double buffering
		attrs[i++] = NSOpenGLPFAOpenGLProfile; // OpenGL version
		attrs[i++] = NSOpenGLProfileVersion3_2Core; // OpenGL3.2
		attrs[i++] = NSOpenGLPFAColorSize; attrs[i++] = 24u; // color buffer bits
		attrs[i++] = NSOpenGLPFAAlphaSize; attrs[i++] = 8u; // alpha buffer size
		//attrs[i++] = NSOpenGLPFAStencilSize; attrs[i++] = 8u; // Stencil buffer bit depth
		attrs[i++] = NSOpenGLPFANoRecovery; // Disable all failover systems
		attrs[i++] = NSOpenGLPFAScreenMask; attrs[i++] = glDisplayMask; // display
		//attrs[i++] = NSOpenGLPFAAllRenderers; // Choose from all available renderers
		//attrs[i++] = NSOpenGLPFAOffScreen;
		//attrs[i++] = NSOpenGLPFAAllowOfflineRenderers; // Allow off-screen rendering
		//attrs[i++] = NSOpenGLPFADepthSize; attrs[i++] = 24u;//MSAA <= 1 ? 24u: 0u; // number of multi sample buffers
		//if (MSAA > 1) { // use msaa
		//	attrs[i++] = NSOpenGLPFAMultisample; // choose multisampling
		//	attrs[i++] = NSOpenGLPFASampleBuffers; attrs[i++] = 1u; // number of multi sample buffers
		//	attrs[i++] = NSOpenGLPFASamples; attrs[i++] = MSAA; // number of multisamples
		//};
		auto format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
		if (!g_sharedRenderResource) {
			static std::once_flag flag;
			call_once(flag, [format]() {
				auto ctx = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
				g_sharedRenderResource = new MacRenderResource(ctx);
			});
		}
		auto ctx = [[NSOpenGLContext alloc] initWithFormat:format shareContext:g_sharedRenderResource->ctx()];

#if DEBUG
		GLint stencilBits;
		[ctx.pixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
		GLint depthSize;
		[ctx.pixelFormat getValues:&depthSize forAttribute:NSOpenGLPFADepthSize forVirtualScreen:0];
		GLint sampleCount;
		[ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
		Qk_DLog("stencilBits:%d,depthSize:%d,sampleCount:%d", stencilBits, depthSize, sampleCount);
#endif

		CGLLockContext(ctx.CGLContextObj);
		[ctx makeCurrentContext];
		Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context");

		auto mem = acquireRenderBackendStorage(typeid(MacGLRender).hash_code(), sizeof(MacGLRender));
		auto render = new(mem) MacGLRender(opts,ctx);

		CGLUnlockContext(ctx.CGLContextObj);
		[NSOpenGLContext clearCurrentContext]; // clear ctx

		g_sharedRenderResource->post_message(Cb([](auto e) {
			[g_sharedRenderResource->ctx() makeCurrentContext];
		}));

		return render;
	}

}
#endif
