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

#import "./mac_app.h"
#import "../../display.h"
#import "../../render/gl/gl_render.h"
#import "../../render/gl/gl_cmd.h"

using namespace qk;

extern QkApplicationDelegate* __appDelegate;

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL && Qk_OSX

class OsxGLRender;

@interface GLView: NSOpenGLView
{
	CVDisplayLinkRef _displayLink;
	OsxGLRender      *_render;
	NSOpenGLContext  *_ctx;
	bool             _isInit; // start render
	Mutex            _mutexRender;
}
@property (assign, nonatomic) bool         isRun;
@property (assign, nonatomic) qk::ThreadID renderThreadId;
- (id) init:(CGRect)frameRect
		context:(NSOpenGLContext*)ctx render:(OsxGLRender*)r;
- (void) stopDisplay;
@end

// ----------------------------------------------------------------------------------------------

class OsxGLRender final: public GLRender, public QkMacRender {
public:
	OsxGLRender(Options opts, NSOpenGLContext *ctx)
		: GLRender(opts), _ctx(ctx)
	{}

	~OsxGLRender() override {
		Qk_STRICT_ASSERT(_message.length() == 0);
	}

	void release() override {
		lock();
		[_view stopDisplay]; // thread task must be forced to end
		unlock();

		GLRender::release(); // Destroy the pre object first

		// Perform the final message task
		_mutexMsg.lock();
		if (_message.length()) {
			lock();
			for ( auto &i : _message )
				i->resolve();
			_message.clear();
			unlock();
		}
		_mutexMsg.unlock();

		Object::release(); // final destruction
	}

	Render* render() override {
		return this;
	}

	bool isRenderThread() {
		return _view.renderThreadId == thread_current_id();
	}

	void lock() override {
		if (isRenderThread()) {
			if (_lockCount++ == 0) CGLLockContext(_ctx.CGLContextObj);
		} else {
			CGLLockContext(_ctx.CGLContextObj); [_ctx makeCurrentContext];
		}
	}

	void unlock() override {
		if (isRenderThread()) {
			if (--_lockCount == 0) CGLUnlockContext(_ctx.CGLContextObj);
		} else {
			CGLUnlockContext(_ctx.CGLContextObj);
		}
	}

	uint32_t post_message(Cb cb, uint64_t delay_us) override {
		if (isRenderThread()) {
			lock();
			cb->resolve();
			unlock();
		} else {
			_mutexMsg.lock();
			_message.pushBack(cb);
			_mutexMsg.unlock();
		}
		return 0;
	}

	Vec2 getSurfaceSize(float *defaultScaleOut) override {
		CGSize size = _view.frame.size;
		float defaultScale = _view.window.backingScaleFactor;
		if (defaultScaleOut)
			*defaultScaleOut = defaultScale;
		return Vec2(size.width * defaultScale, size.height * defaultScale);
	}

	void renderDisplay() {
		lock();
		if (!_view.isRun) return unlock();

		if (_message.length()) { //
			List<Cb> msg;
			_mutexMsg.lock();
			msg = std::move(_message);
			_mutexMsg.unlock();
			for ( auto &i : msg ) i->resolve();
		}

		if (_delegate->onRenderBackendDisplay()) {
			_glcanvas->flushBuffer(); // commit gl canvas cmd
			// copy pixels to default color buffer
			auto src = _glcanvas->surfaceSize();
			auto dest = _surfaceSize;
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, src[0], src[1],
				0, 0, dest[0], dest[1], GL_COLOR_BUFFER_BIT, src == dest ? GL_NEAREST: GL_LINEAR);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _glcanvas->fbo()); // bind frame buffer for main canvas
			// flush gl buffer
			glFlush(); // glFinish, glFenceSync, glWaitSync
			[_ctx flushBuffer]; // swap double buffer
		}
		unlock();
	}

	UIView* make_surface_view(CGRect rect) override {
		Qk_ASSERT(!_view);

		_view = [[GLView alloc] init:rect context:_ctx render:this];
		_view.wantsBestResolutionOpenGLSurface = YES; // Enable retina-support
		_view.wantsLayer = YES; // Enable layer-backed drawing of view

		[_ctx makeCurrentContext];
		//_ctx.view = _view;
		//[_ctx setFullScreen];

		GLint swapInterval = 1; // enable vsync
		[_ctx setValues:&swapInterval forParameter:NSOpenGLContextParameterSwapInterval];//NSOpenGLCPSwapInterval

		GLint sampleCount; // read msaa cnt
		[_ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];

		return _view;
	}

private:
	GLView            *_view;
	NSOpenGLContext   *_ctx;
	int              _lockCount;
	List<Cb>         _message;
	Mutex            _mutexMsg;
};

// ----------------------------------------------------------------------------------------------

@implementation GLView

- (id) init:(CGRect)frameRect
		context:(NSOpenGLContext*)ctx render:(OsxGLRender*)r
{
	if( (self = [super initWithFrame:frameRect pixelFormat:nil]) ) {
		_ctx = ctx;
		_isInit = false;
		_isRun = true;
		_displayLink = nil;
		_render = r;
		[self setOpenGLContext:ctx];
	}
	return self;
}

- (void) update {
	_render->reload();
	[super update];
}

static CVReturn displayLinkCallback(
	CVDisplayLinkRef displayLink, const CVTimeStamp* now,
	const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* view) {
	[((__bridge GLView*)view) renderDisplay];
	return kCVReturnSuccess;
}

- (void) prepareOpenGL {
	if (_render->options().fps == 0) {
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
	} else {
		_renderThreadId = thread_fork([](void* arg) { // fork render thread
			auto v = (__bridge GLView*)arg;
			while (v->_isRun) {
				[v renderDisplay];
			}
		}, (__bridge void*)self, "renderDisplay");
	}
	[super prepareOpenGL];
}

- (void) renderDisplay {
	if (!_isInit) {
		_isInit = true;
		_renderThreadId = thread_current_id();
		[_ctx makeCurrentContext];
	}
	_render->renderDisplay();
}

- (void) stopDisplay {
	if (_render->options().fps == 0) {
		CVDisplayLinkStop(_displayLink);
	} else {}
	_isRun = false;
}

@end

// ----------------------------------------------------------------------------------------------

QkMacRender* qk_make_mac_gl_render(Render::Options opts) {
	//	generate the GL display mask for all displays
	CGDirectDisplayID		dspys[10];
	CGDisplayCount			count = 0;
	GLuint					    glDisplayMask = 0;

	if (CGGetActiveDisplayList(10, dspys, &count) == kCGErrorSuccess)	{
		for (int i = 0; i < count; i++)
			glDisplayMask = glDisplayMask | CGDisplayIDToOpenGLDisplayMask(dspys[i]);
	}

	uint32_t MSAA = opts.msaaSample;
	uint32_t i = 0;
	NSOpenGLPixelFormatAttribute attrs[32] = {0};

	attrs[i++] = NSOpenGLPFAAccelerated; // Choose a hardware accelerated renderer
	attrs[i++] = NSOpenGLPFAClosestPolicy;
	// attrs[i++] = NSOpenGLPFADoubleBuffer; // use double buffering
	attrs[i++] = NSOpenGLPFAOpenGLProfile; // OpenGL version
	attrs[i++] = NSOpenGLProfileVersion3_2Core; // OpenGL3.2
	//attrs[i++] = NSOpenGLPFAColorSize; attrs[i++] = 24u; // color buffer bits
	//attrs[i++] = NSOpenGLPFAAlphaSize; attrs[i++] = 8u; // alpha buffer size
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
	auto ctx = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
	auto prevCtx = NSOpenGLContext.currentContext;

#if DEBUG
	GLint stencilBits;
	[ctx.pixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
	GLint depthSize;
	[ctx.pixelFormat getValues:&depthSize forAttribute:NSOpenGLPFADepthSize forVirtualScreen:0];
	GLint sampleCount;
	[ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
	Qk_DEBUG("stencilBits:%d,depthSize:%d,sampleCount:%d", stencilBits, depthSize, sampleCount);
#endif

	CGLLockContext(ctx.CGLContextObj);
	[ctx makeCurrentContext];
	Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context");
	auto render = new OsxGLRender(opts,ctx);
	CGLUnlockContext(ctx.CGLContextObj);
	[NSOpenGLContext clearCurrentContext]; // clear ctx

	if (prevCtx)
		[prevCtx makeCurrentContext];

	return render;
}

#endif
