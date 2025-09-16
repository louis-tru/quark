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
 * 
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

#define GL_SILENCE_DEPRECATION
#define GL3_PROTOTYPES

#import "./apple_render.h"
#import "../gl/gl_render.h"
#import "../gl/gl_cmd.h"

using namespace qk;

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL && Qk_MacOS

class MacGLRender;

@interface GLView: NSOpenGLView
{
	CVDisplayLinkRef _displayLink;
	MacGLRender      *_render;
}
@property (strong, nonatomic) NSOpenGLContext *ctx;
@property (assign, nonatomic) BOOL            isRun;
- (id)   init:(NSOpenGLContext*)ctx render:(MacGLRender*)r;
- (void) stopDisplay;
@end

// ----------------------------------------------------------------------------------------------

class MacGLRender final: public GLRender, public RenderSurface {
public:
	MacGLRender(Options opts, NSOpenGLContext *ctx)
		: GLRender(opts), _view(nil), _ctx(ctx)
	{
		//CFBridgingRetain(_ctx);
	}

	~MacGLRender() override {
		Qk_CHECK(_msg.length() == 0);
	}

	void release() override {
		lock();
		if (_view) {
			[_view stopDisplay]; // thread task must be forced to end
		}
		unlock();

		GLRender::release(); // Destroy the pre object first

		// Perform the final message task
		_mutexMsg.lock();
		if (_msg.length()) {
			lock();
			for (auto &i : _msg)
				i->resolve();
			_msg.clear();
			unlock();
		}
		_mutexMsg.unlock();

		//CFBridgingRelease((__bridge void*)_ctx);
		Object::release(); // final destruction
	}

	RenderSurface* surface() override {
		return this;
	}

	bool isRenderThread() {
		return _renderThreadId == thread_self_id();
	}

	void lock() override {
		if (!isRenderThread()) {
			CGLLockContext(_ctx.CGLContextObj);
			[_ctx makeCurrentContext];
		}
	}

	void unlock() override {
		if (!isRenderThread()) {
			CGLUnlockContext(_ctx.CGLContextObj);
		}
	}

	void post_message(Cb cb) override {
		if (_view && isRenderThread()) {
			cb->resolve();
		} else if (!_view.isRun) {
			if (_mutexMsg.try_lock()) {
				_msg.push(cb);
				_mutexMsg.unlock();
			} else {
				cb->resolve();
			}
		} else {
			_mutexMsg.lock();
			_msg.push(cb);
			_mutexMsg.unlock();
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
		_renderThreadId = thread_self_id();

		if (_msg.length()) { //
			_mutexMsg.lock();
			auto msg(std::move(_msg));
			_mutexMsg.unlock();
			for ( auto &i : msg ) i->resolve();
		}

		if (_delegate->onRenderBackendDisplay()) {
			_glcanvas->flushBuffer(); // commit gl canvas cmd
			auto src = _glcanvas->surfaceSize();
			auto dest = _surfaceSize;
			auto filter = src == dest ? GL_NEAREST: GL_LINEAR;
			// copy pixels to default color buffer
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, src[0], src[1], 0, 0, dest[0], dest[1], GL_COLOR_BUFFER_BIT, filter);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _glcanvas->fbo()); // bind frame buffer for main canvas
			glFlush(); // flush gl buffer, glFinish, glFenceSync, glWaitSync
			[_ctx flushBuffer]; // swap double buffer
		}
		_renderThreadId = qk::ThreadID();
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

		// [_ctx makeCurrentContext];
		// _ctx.view = _view;
		// [_ctx setFullScreen];

		GLint swapInterval = 1; // enable vsync
		[_ctx setValues:&swapInterval forParameter:NSOpenGLContextParameterSwapInterval];//NSOpenGLCPSwapInterval

		GLint sampleCount; // read msaa cnt
		[_ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];

		return _view;
	}

private:
	GLView            *_view;
	NSOpenGLContext   *_ctx;
	Array<Cb>        _msg;
	Mutex            _mutexMsg;
	qk::ThreadID     _renderThreadId;
};

// ----------------------------------------------------------------------------------------------

@implementation GLView

- (id)init:(NSOpenGLContext*)ctx render:(MacGLRender*)r {
	if ((self = [super initWithFrame:CGRectZero pixelFormat:nil])) {
		self.ctx = ctx;
		_isRun = true;
		_displayLink = nil;
		_render = r;
		[self setOpenGLContext:ctx];
	}
	return self;
}

static CVReturn displayLinkCallback(
	CVDisplayLinkRef displayLink, const CVTimeStamp* now,
	const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* view) {
	[((__bridge GLView*)view) renderDisplay];
	return kCVReturnSuccess;
}

- (BOOL)isOpaque { return NO; }

- (void)prepareOpenGL {
	[super prepareOpenGL];
	// Create a display link capable of being used with all active displays
	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	// Set the renderer output callback function
	CVDisplayLinkSetOutputCallback(_displayLink, &displayLinkCallback, (__bridge void*)self);
	// Set the display link for the current renderer
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(
		_displayLink, _ctx.CGLContextObj, _ctx.pixelFormat.CGLPixelFormatObj
	);
	// Activate the display link
	CVDisplayLinkStart(_displayLink);
}

- (void)drawRect:(NSRect)dirtyRect {
	if (_isRun) {
		CVDisplayLinkStop(_displayLink);
		if (NSOpenGLContext.currentContext != _ctx)
			[_ctx makeCurrentContext];
		_render->reload();
		_render->renderDisplay();
		CVDisplayLinkStart(_displayLink);
	}
}

- (void)renderDisplay {
	if (_isRun) {
		if (NSOpenGLContext.currentContext != _ctx)
			[_ctx makeCurrentContext];
		_render->renderDisplay();
	}
}

- (void)stopDisplay {
	CVDisplayLinkStop(_displayLink);
	_displayLink = nil;
	_isRun = false;
	_ctx = nil;
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
		auto render = new MacGLRender(opts,ctx);
		CGLUnlockContext(ctx.CGLContextObj);
		[NSOpenGLContext clearCurrentContext]; // clear ctx

		g_sharedRenderResource->post_message(Cb([](auto e) {
			[g_sharedRenderResource->ctx() makeCurrentContext];
		}));

		return render;
	}

}
#endif
