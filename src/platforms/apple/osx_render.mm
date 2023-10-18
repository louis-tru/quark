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

#import "./apple_app.h"
#import "../../display.h"
#import "../../render/gl/gl_render.h"
#import "../../render/gl/gl_cmd.h"

using namespace qk;

extern QkApplicationDelegate* __appDelegate;

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL && Qk_OSX

class AppleGLRender;

@interface GLView: NSOpenGLView
{
	CVDisplayLinkRef _displayLink;
	AppleGLRender    *_render;
	NSOpenGLContext  *_ctx;
	bool             _isStart; // start render
}
@property (assign, nonatomic) qk::ThreadID renderThreadId;
- (id) init:(CGRect)frameRect
		context:(NSOpenGLContext*)ctx render:(AppleGLRender*)r;
- (void) stopDisplay;
@end

// ----------------------------------------------------------------------------------------------

class AppleGLRender: public GLRender, public QkAppleRender {
public:
	AppleGLRender(Options opts, NSOpenGLContext *ctx)
		: GLRender(opts), _ctx(ctx)
	{}

	~AppleGLRender() {
		[_view stopDisplay];
	}

	Render* render() override {
		return this;
	}

	qk::ThreadID threadId() override {
		return _view.renderThreadId;
	}

	void lock() override {
		if (_view.renderThreadId == thread_current_id()) {
			if (_lockCount++ == 0)
				CGLLockContext(_ctx.CGLContextObj);
		} else {
			CGLLockContext(_ctx.CGLContextObj);
			[_ctx makeCurrentContext];
		}
	}

	void unlock() override {
		if (_view.renderThreadId == thread_current_id()) {
			if (--_lockCount == 0)
				CGLUnlockContext(_ctx.CGLContextObj);
		} else {
			CGLUnlockContext(_ctx.CGLContextObj);
		}
	}

	uint32_t post_message(Cb cb, uint64_t delay_us) override {
		if (_view.renderThreadId == thread_current_id()) {
			cb->resolve();
		} else {
			_mutexMsg.lock();
			_message.pushBack(cb);
			_mutexMsg.unlock();
		}
		return 0;
	}

	Vec2 getSurfaceSize() override {
		CGSize size = _view.frame.size;
		_defaultScale = _view.window.backingScaleFactor;
		float w = size.width * _defaultScale;
		float h = size.height * _defaultScale;
		_surfaceSize = Vec2(w,h);
		return _surfaceSize;
	}

	float getDefaultScale() override {
		_defaultScale = _view.window.backingScaleFactor;
		return _defaultScale;
	}

	void reload() override {
		auto size = getSurfaceSize();
		_delegate->onRenderBackendReload({ Vec2{0,0},size}, size, _defaultScale);
	}

	void renderDisplay() {
		List<Cb> msg;
		if (_message.length()) { //
			_mutexMsg.lock();
			msg = std::move(_message);
			_mutexMsg.unlock();
		}
		if (msg.length()) {
			lock();
			for ( auto &i : msg ) i->resolve();
			unlock();
		}
		//if (!_isActive) return;

#if !Qk_USE_GLC_CMD_QUEUE
		lock();
#endif

		if (_delegate->onRenderBackendDisplay()) {
#if Qk_USE_GLC_CMD_QUEUE
			lock();
#endif
			_glCanvas.flushBuffer(); // commit gl canvas cmd
			// copy pixels to default color buffer
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			auto src = _glCanvas.surfaceSize();
			auto dest = _surfaceSize;
			glBlitFramebuffer(0, 0, src[0], src[1],
				0, 0, dest[0], dest[1], GL_COLOR_BUFFER_BIT, src == dest ? GL_NEAREST: GL_LINEAR);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _glCanvas.fbo()); // bind frame buffer for main canvas
			// flush gl buffer
			glFlush(); // glFinish, glFenceSync, glWaitSync
			[_ctx flushBuffer]; // swap double buffer
			unlock();
		} else {
#if !Qk_USE_GLC_CMD_QUEUE
			unlock();
#endif
		}
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
		context:(NSOpenGLContext*)ctx render:(AppleGLRender*)r
{
	if( (self = [super initWithFrame:frameRect pixelFormat:nil]) ) {
		_ctx = ctx;
		_isStart = false;
		_displayLink = nil;
		_render = r;
		[self setOpenGLContext:ctx];
	}
	return self;
}

//-(void) reshape {}
- (void) update {
	_render->reload();
	[super update];
}

- (void) prepareOpenGL {
	if (_render->options().fps == 0) {
		// Create a display link capable of being used with all active displays
		CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
		// Set the renderer output callback function
		CVDisplayLinkSetOutputCallback(_displayLink, &displayLinkCallback, (__bridge void*)self);
		// Set the display link for the current renderer
		CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink,
																											_ctx.CGLContextObj,
																											_ctx.pixelFormat.CGLPixelFormatObj);
		// Activate the display link
		CVDisplayLinkStart(_displayLink);
	} else {
		_renderThreadId = thread_fork([](void* arg) { // fork render thread
			auto v = (__bridge GLView*)arg;
			while (v->_renderThreadId != qk::ThreadID()) {
				[v renderDisplay];
			}
		}, (__bridge void*)self, "renderDisplay");
	}
	[super prepareOpenGL];
}

static CVReturn displayLinkCallback(CVDisplayLinkRef displayLink,
																		const CVTimeStamp* now,
																		const CVTimeStamp* outputTime,
																		CVOptionFlags flagsIn,
																		CVOptionFlags* flagsOut, void* view)
{
	[((__bridge GLView*)view) renderDisplay];
	return kCVReturnSuccess;
}

- (void) renderDisplay {
	if (!_isStart) {
		_isStart = true;
		_renderThreadId = thread_current_id();
		[_ctx makeCurrentContext];
	}
	_render->renderDisplay();
}

-(void) stopDisplay {
	if (_render->options().fps == 0) {
		CVDisplayLinkStop(_displayLink);
	} else {}
	_renderThreadId = qk::ThreadID();
}

@end

// ----------------------------------------------------------------------------------------------

QkAppleRender* qk_make_apple_gl_render(Render::Options opts) {
	//	generate the GL display mask for all displays
	CGDirectDisplayID		dspys[10];
	CGDisplayCount			count = 0;
	GLuint					    glDisplayMask = 0;

	if (CGGetActiveDisplayList(10, dspys, &count) == kCGErrorSuccess)	{
		for (int i = 0; i < count; i++)
			glDisplayMask = glDisplayMask | CGDisplayIDToOpenGLDisplayMask(dspys[i]);
	}
	
	uint32_t MSAA = opts.msaa;
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
	auto render = new AppleGLRender(opts,ctx);
	CGLUnlockContext(ctx.CGLContextObj);
	[NSOpenGLContext clearCurrentContext]; // clear ctx

	if (prevCtx)
		[prevCtx makeCurrentContext];

	return render;
}

#endif
