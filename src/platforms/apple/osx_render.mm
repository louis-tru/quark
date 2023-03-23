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

#import "./apple_app.h"
#import "../../display.h"
#import "../../render/gl/gl_render.h"

using namespace qk;

extern QkApplicationDelegate* __appDelegate;

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL && Qk_OSX

@interface GLView: NSOpenGLView
{
	CVDisplayLinkRef _display_link;
	List<Cb>         _message;
	Mutex            _mutex;
}
@property (assign, nonatomic) qk::Render *render;
@property (assign, nonatomic) NSOpenGLContext *ctx;
@end

@implementation GLView

	- (id) initWithFrame:(CGRect)frameRect shareContext:(NSOpenGLContext*)context {
		if( (self = [super initWithFrame:frameRect pixelFormat:nil]) ) {
			_ctx = context;
			[self setOpenGLContext:context];
		}
		return self;
	}
	- (void) update {
		[super update];
		if (self.render)
			self.render->reload();
    Qk_DEBUG("NSOpenGLView::update");
	}
	- (void) prepareOpenGL {
		[super prepareOpenGL];

		// Create a display link capable of being used with all active displays
		CVDisplayLinkCreateWithActiveCGDisplays(&_display_link);

		// Set the renderer output callback function
		CVDisplayLinkSetOutputCallback(_display_link, &display_link_callback, (__bridge void*)self);

		// Set the display link for the current renderer
		CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_display_link,
																											_ctx.CGLContextObj,
																											_ctx.pixelFormat.CGLPixelFormatObj);
		// Activate the display link
		CVDisplayLinkStart(_display_link);
	}
	static CVReturn display_link_callback(CVDisplayLinkRef displayLink,
																				const CVTimeStamp* now,
																				const CVTimeStamp* outputTime,
																				CVOptionFlags flagsIn,
																				CVOptionFlags* flagsOut,
																				void* view)
	{
		auto v = (__bridge GLView*)view;
		auto lock = [&]() {
			CGLLockContext(v.ctx.CGLContextObj);
			[v.ctx makeCurrentContext];
			Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context 3");
		};
		auto unlock = [&](){
			CGLUnlockContext(v.ctx.CGLContextObj);
		};

		List<Cb> msg;
		{ //
			v->_mutex.lock();
			if (v->_message.length())
				msg = std::move(v->_message);
			v->_mutex.unlock();
		}
		
		auto del = v.render->delegate();

		if (msg.length()) {
			lock();
			for ( auto& i : msg )
				i->resolve();
			
			if (del->onRenderDevicePreDisplay())
				del->onRenderDeviceDisplay();
			unlock();
		}
		else if (del->onRenderDevicePreDisplay()) {
			lock();
			del->onRenderDeviceDisplay();
			unlock();
		}

		return kCVReturnSuccess;
	}

	-(void) stopDisplay {
		CVDisplayLinkStop(_display_link);
	}

	- (uint32_t) post_message:(Cb) cb delay_us:(uint64_t)delay_us {
		ScopeLock lock(_mutex);
		_message.push_back(cb);
	}

@end

class AppleGLRender: public GLRender, public QkAppleRender {
public:
	AppleGLRender(Options opts, NSOpenGLContext *ctx, Delegate *delegate)
		: GLRender(opts, delegate), _ctx(ctx)
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
		_default_scale = _view.window.backingScaleFactor;
		float w = size.width * _default_scale;
		float h = size.height * _default_scale;
		_surface_size = Vec2(w,h);
		return _surface_size;
	}

	float getDefaultScale() override {
		_default_scale = _view.window.backingScaleFactor;
		return _default_scale;
	}

	void reload() override {
		auto size = getSurfaceSize();
		Mat4 mat;
		if (!_delegate->onRenderDeviceReload({ Vec2{0,0},size}, size, _default_scale, &mat))
			return;

		CGLLockContext(_ctx.CGLContextObj);
		[_ctx makeCurrentContext];

		glViewport(0, 0, size.x(), size.y());

		if (!_IsDeviceMsaa) { // no device msaa
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // default frame buffer
			setAntiAlias(size.x(), size.y());
		}
		const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(_IsDeviceMsaa ? 1: 2, buffers);
		
		setRootMatrix(mat);

		[NSOpenGLContext clearCurrentContext]; // clear ctx
		CGLUnlockContext(_ctx.CGLContextObj);
  }

	void begin() override {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void submit() override {
		glFlush();
		[_ctx flushBuffer]; // swap double buffer
	}

	UIView* make_surface_view(CGRect rect) override {
		Qk_ASSERT(!_view);

		_view = [[GLView alloc] initWithFrame:rect shareContext:_ctx];
		_view.render = this;
		_view.ctx = _ctx;
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

		return _view;
	}

private:
	GLView            *_view;
	NSOpenGLContext   *_ctx;
};

QkAppleRender* qk_make_apple_gl_render(Render::Options opts, Render::Delegate *delegate) {

	//	generate the GL display mask for all displays
	CGDirectDisplayID		dspys[10];
	CGDisplayCount			count = 0;
	GLuint					    glDisplayMask = 0;

	if (CGGetActiveDisplayList(10, dspys, &count) == kCGErrorSuccess)	{
		for (int i = 0; i < count; i++)
			glDisplayMask = glDisplayMask | CGDisplayIDToOpenGLDisplayMask(dspys[i]);
	}
	
	uint32_t MSAA = opts.msaaSampleCnt;
	uint32_t i = 0;
	NSOpenGLPixelFormatAttribute attrs[32] = {0};
	
	attrs[i++] = NSOpenGLPFAAccelerated; // Choose a hardware accelerated renderer
	attrs[i++] = NSOpenGLPFAClosestPolicy;
	//attrs[i++] = NSOpenGLPFADoubleBuffer; // use double buffering
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
	auto render = new AppleGLRender(opts, ctx, delegate);
	CGLUnlockContext(ctx.CGLContextObj);
	[NSOpenGLContext clearCurrentContext]; // clear ctx

	if (prev)
		[prev makeCurrentContext];

	return render;
}

#endif
