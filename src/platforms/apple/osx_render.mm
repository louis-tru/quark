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
class AppleGLRender;

#define QK_USE_NSOpenGLView 1

@interface GLView
#if QK_USE_NSOpenGLView
:NSOpenGLView
#else
:UIView
#endif
{
	CVDisplayLinkRef _display_link;
}
@property (assign, nonatomic) qk::Application *host;
@property (assign, nonatomic) NSOpenGLContext *ctx;
@end

@interface GLLayer: NSOpenGLLayer
@end

@implementation GLLayer
	- (NSOpenGLPixelFormat *)openGLPixelFormatForDisplayMask:(uint32_t)mask {
		return ((GLView*)self.view).ctx.pixelFormat;
	}
	- (NSOpenGLContext*)openGLContextForPixelFormat:(NSOpenGLPixelFormat *)pixelFormat {
		return ((GLView*)self.view).ctx;
	}
	- (BOOL)canDrawInOpenGLContext:(NSOpenGLContext *)context
										 pixelFormat:(NSOpenGLPixelFormat *)pixelFormat
										forLayerTime:(CFTimeInterval)timeInterval
										 displayTime:(const CVTimeStamp *)timeStamp {
		return ((GLView*)self.view).host->display()->pre_render();
	}
	- (void)drawInOpenGLContext:(NSOpenGLContext *)context
									pixelFormat:(NSOpenGLPixelFormat *)pixelFormat
								 forLayerTime:(CFTimeInterval)timeInterval
									displayTime:(const CVTimeStamp *)timeStamp {
		CGLLockContext(context.CGLContextObj);
		((GLView*)self.view).host->display()->render();
		CGLUnlockContext(context.CGLContextObj);
	}
@end

@implementation GLView

#if QK_USE_NSOpenGLView
	- (id) initWithFrame:(CGRect)frameRect shareContext:(NSOpenGLContext*)context {
		if( (self = [super initWithFrame:frameRect pixelFormat:nil]) ) {
			_ctx = context;
			[self setOpenGLContext:context];
		}
		return self;
	}
	- (void) update {
		[super update];
	}
	- (void) reshape {
		[super reshape];
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
		
		if (v.host->display()->pre_render()) {
			[v.ctx makeCurrentContext];
			Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context");
			CGLLockContext(v.ctx.CGLContextObj);
			v.host->display()->render();
			CGLUnlockContext(v.ctx.CGLContextObj);
		}
		return kCVReturnSuccess;
	}

#else
	- (CALayer *)makeBackingLayer {
		auto layer = [[GLLayer alloc] init];
		// Layer should render when size changes.
		layer.needsDisplayOnBoundsChange = YES;
		// The layer should continuously call canDrawInOpenGLContext
		layer.asynchronous = YES;
		layer.shouldRasterize = YES;
		return layer;
	}
#endif
	- (void) viewDidChangeBackingProperties {
		[super viewDidChangeBackingProperties];
		// Need to propagate information about retina resolution
		self.layer.contentsScale = self.window.backingScaleFactor;
		[__appDelegate refresh_surface_region];
	}

@end

class AppleGLRender: public GLRender, public QkAppleRender {
public:
	AppleGLRender(Application* host, NSOpenGLContext *ctx, bool independentThread)
		: GLRender(host, independentThread), _ctx(ctx)
	{}

	~AppleGLRender() {
		[NSOpenGLContext clearCurrentContext];
	}

	void setRenderBuffer(int width, int height) override {
	}

	void setAntiAlias(int width, int height) override {
	}

	void begin() override {
		//[_ctx update];
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		//glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _aa_tex, 0);
	}

	void present() override {
		glFlush();

//		glBindFramebuffer(GL_READ_FRAMEBUFFER, _frame_buffer);
//		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//		auto region = _host->display()->surface_region();
//		auto w = region.size.x(), h = region.size.x();
//		glBlitFramebuffer(0, 0, w, h,
//											0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		[_ctx flushBuffer];
	}

	UIView* make_surface_view(CGRect rect) override {
		post_message(Cb([this](Cb::Data& e) { // set current context from render loop
			[_ctx makeCurrentContext];
			Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context 2");
		}));

#if QK_USE_NSOpenGLView
		_view = [[GLView alloc] initWithFrame:rect shareContext:_ctx];
#else
		_view = [[GLView alloc] initWithFrame:rect];
#endif
		_view.host = _host;
		_view.ctx = _ctx;
		_view.wantsBestResolutionOpenGLSurface = YES; // Enable retina-support
		_view.wantsLayer = YES; // Enable layer-backed drawing of view

		_ctx.view = _view;
		//[_ctx setFullScreen];

		GLint swapInterval = 1/*fDisplayParams.fDisableVsync*/ ? 0 : 1;
		[_ctx setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

		return _view;
	}

	Render* render() override {
		return this;
	}

private:
	GLView            *_view;
	NSOpenGLContext   *_ctx;
};

QkAppleRender* makeAppleGLRender(Application* host, bool independentThread) {
	
	//	generate the GL display mask for all displays
	CGDirectDisplayID		dspys[10];
	CGDisplayCount			count = 0;
	GLuint					    glDisplayMask = 0;

	if (CGGetActiveDisplayList(10, dspys, &count) == kCGErrorSuccess)	{
		for (int i = 0; i < count; i++)
			glDisplayMask = glDisplayMask | CGDisplayIDToOpenGLDisplayMask(dspys[i]);
	}
	
	NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFAAccelerated, // Choose a hardware accelerated renderer
		NSOpenGLPFAClosestPolicy,
		NSOpenGLPFADoubleBuffer, // use double buffering
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core, // OpenGL4.1
		NSOpenGLPFAColorSize, 24,  // color buffer bits
		NSOpenGLPFAAlphaSize, 8,   // alpha buffer size
		NSOpenGLPFADepthSize, 0,  // Depth Buffer Bit Depth
		NSOpenGLPFAStencilSize, 8, // Stencil buffer bit depth
		NSOpenGLPFAMultisample, 0, // use multisampling and Number of multisampled buffers to use
		//NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)4, // number of multisamples
		NSOpenGLPFANoRecovery, // Disable all failover systems
		NSOpenGLPFAScreenMask, glDisplayMask,
		//NSOpenGLPFAAllRenderers, // Choose from all available renderers
		//NSOpenGLPFAOffScreen, //
		//NSOpenGLPFAAllowOfflineRenderers, // Allow off-screen rendering
		0
	};
	auto format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	auto ctx = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];

	[ctx makeCurrentContext];
	Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context");
	
	//GLint stencilBits;
	//[ctx.pixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
	//GLint sampleCount;
	//[ctx.pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
	
	return new AppleGLRender(host, ctx, independentThread);
}

#endif
