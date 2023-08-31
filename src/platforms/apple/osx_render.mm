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
 
// #define GL_SILENCE_DEPRECATION

#import "./apple_app.h"
#import "../../display.h"
#import "../../render/gl/gl_render.h"

using namespace qk;

extern QkApplicationDelegate* __appDelegate;

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL && Qk_OSX

@interface GLView: NSOpenGLView
{
	CVDisplayLinkRef _displayLink;
	List<Cb>         _message;
	Mutex            _mutexMsg;
	bool             _isLockRender;
	qk::Render      *_render;
	NSOpenGLContext *_ctx;
	qk::ThreadID     _renderThreadID;
}
@end

@implementation GLView

	- (id) initWithFrame:(CGRect)frameRect
							 context:(NSOpenGLContext*)context
							  render:(qk::Render*)render
	{
		if( (self = [super initWithFrame:frameRect pixelFormat:nil]) ) {
			_ctx = context;
			_isLockRender = false;
			_displayLink = nil;
			_render = render;
			[self setOpenGLContext:context];
		}
		return self;
	}
	- (void) update {
		[super update];
		_render->reload();
		Qk_DEBUG("NSOpenGLView::update");
	}
	- (void) prepareOpenGL {
		[super prepareOpenGL];
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
			_renderThreadID = thread_fork([](void* arg) { // fork render thread
				auto v = (__bridge GLView*)arg;
				while (v->_renderThreadID != qk::ThreadID()) {
					[v renderDisplay];
				}
			}, (__bridge void*)self, "renderDisplay");
		}
	}
	- (void) lockRender {
		if (!_isLockRender) {
			_isLockRender = true;
			CGLLockContext(_ctx.CGLContextObj);
			[_ctx makeCurrentContext];
			Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context 3");
		}
	}
	- (void) unlockRender {
		if (_isLockRender) {
			_isLockRender = false;
			CGLUnlockContext(_ctx.CGLContextObj);
		}
	}
	- (void) renderDisplay {
		List<Cb> msg;
		if (_message.length()) { //
			_mutexMsg.lock();
			msg = std::move(_message);
			_mutexMsg.unlock();
		}
		if (msg.length()) {
			[self lockRender];
			for ( auto &i : msg )
				i->resolve();
			_render->delegate()->onRenderBackendDisplay();
			[self unlockRender];
		} else {
			_render->delegate()->onRenderBackendDisplay();
		}
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

	-(void) stopDisplay {
		if (_render->options().fps == 0) {
			CVDisplayLinkStop(_displayLink);
		} else {}
		_renderThreadID = qk::ThreadID();
	}

	- (uint32_t) post_message:(Cb) cb delay_us:(uint64_t)delayUs {
		if (_isLockRender) {
			cb->resolve();
		} else {
			_mutexMsg.lock();
			_message.pushBack(cb);
			_mutexMsg.unlock();
		}
	}

@end

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

	uint32_t post_message(Cb cb, uint64_t delay_us) override {
		return [_view post_message:cb delay_us:delay_us];
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
		Mat4 mat;
		Vec2 surfaceScale;
		if (!_delegate->onRenderBackendReload({ Vec2{0,0},size}, size, _defaultScale, &mat, &surfaceScale))
			return;

		CGLLockContext(_ctx.CGLContextObj);
		[_ctx makeCurrentContext];

		glViewport(0, 0, size.x(), size.y());
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // default frame buffer

		setClipAABuffer(size.x(), size.y(), _opts.msaa);

		_glcanvas.setRootMatrix(mat, surfaceScale);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		[NSOpenGLContext clearCurrentContext]; // clear ctx
		CGLUnlockContext(_ctx.CGLContextObj);
	}

	void begin() override {
		[_view lockRender];
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void submit() override {
		flushBuffer();
		glFlush(); // glFenceSync, glWaitSync
		[_ctx flushBuffer]; // swap double buffer, glFinish
		[_view unlockRender];
	}

	UIView* make_surface_view(CGRect rect) override {
		Qk_ASSERT(!_view);

		_view = [[GLView alloc] initWithFrame:rect context:_ctx render:this];
		_view.wantsBestResolutionOpenGLSurface = YES; // Enable retina-support
		_view.wantsLayer = YES; // Enable layer-backed drawing of view

		_ctx.view = _view;
		//[_ctx setFullScreen];

		GLint swapInterval = 1; // enable vsync
		[_ctx setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

		GLint sampleCount;
		[_ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
		
		if (sampleCount > 1) {
			_IsDeviceMsaa = true;
		}
		_opts.msaa = sampleCount;
		return _view;
	}

private:
	GLView            *_view;
	NSOpenGLContext   *_ctx;
};

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
	attrs[i++] = NSOpenGLPFAStencilSize; attrs[i++] = 8u; // Stencil buffer bit depth
	attrs[i++] = NSOpenGLPFANoRecovery; // Disable all failover systems
	attrs[i++] = NSOpenGLPFAScreenMask; attrs[i++] = glDisplayMask; // display
	//attrs[i++] = NSOpenGLPFAAllRenderers; // Choose from all available renderers
	//attrs[i++] = NSOpenGLPFAOffScreen;
	//attrs[i++] = NSOpenGLPFAAllowOfflineRenderers; // Allow off-screen rendering
	attrs[i++] = NSOpenGLPFADepthSize; attrs[i++] = 24u;//MSAA <= 1 ? 24u: 0u; // number of multi sample buffers

	if (MSAA > 1) { // use msaa
		attrs[i++] = NSOpenGLPFAMultisample; // choose multisampling
		attrs[i++] = NSOpenGLPFASampleBuffers; attrs[i++] = 1u; // number of multi sample buffers
		attrs[i++] = NSOpenGLPFASamples; attrs[i++] = MSAA; // number of multisamples
	};

	auto format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	auto ctx = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
	auto prev = NSOpenGLContext.currentContext;

	//GLint stencilBits;
	//[ctx.pixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
	//GLint sampleCount;
	//[ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];

	CGLLockContext(ctx.CGLContextObj);
	[ctx makeCurrentContext];
	Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context");
	auto render = new AppleGLRender(opts,ctx);
	CGLUnlockContext(ctx.CGLContextObj);
	[NSOpenGLContext clearCurrentContext]; // clear ctx

	if (prev)
		[prev makeCurrentContext];

	return render;
}

#endif
