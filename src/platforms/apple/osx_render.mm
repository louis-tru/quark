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

@interface GLLayer: NSOpenGLLayer
@property (assign, nonatomic) NSOpenGLContext *ctx;
@property (assign, nonatomic) qk::Application *host;
@end

@implementation GLLayer
	- (NSOpenGLPixelFormat *)openGLPixelFormatForDisplayMask:(uint32_t)mask {
		return self.ctx.pixelFormat;
	}
	- (NSOpenGLContext*)openGLContextForPixelFormat:(NSOpenGLPixelFormat *)pixelFormat {
		return self.ctx;
	}
	- (BOOL)canDrawInOpenGLContext:(NSOpenGLContext *)context
										 pixelFormat:(NSOpenGLPixelFormat *)pixelFormat
										forLayerTime:(CFTimeInterval)timeInterval
										 displayTime:(const CVTimeStamp *)timeStamp {
		return self.host->display()->pre_render();
	}
	- (void)drawInOpenGLContext:(NSOpenGLContext *)context
									pixelFormat:(NSOpenGLPixelFormat *)pixelFormat
								 forLayerTime:(CFTimeInterval)timeInterval
									displayTime:(const CVTimeStamp *)timeStamp {
		// CGLLockContext(context.CGLContextObj);
		// CGLUnlockContext(context.CGLContextObj);
		//glClearColor(1, 1, 0, 1);
		//glClear(GL_COLOR_BUFFER_BIT);
		self.host->display()->render();
	}
@end

@interface GLView: UIView
@end

@implementation GLView
	- (CALayer *)makeBackingLayer {
		auto layer = [[GLLayer alloc] init];
		// Layer should render when size changes.
		layer.needsDisplayOnBoundsChange = YES;
		// The layer should continuously call canDrawInOpenGLContext
		layer.asynchronous = YES;
		return layer;
	}
	- (void)viewDidChangeBackingProperties {
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

	void onRenderbufferStorage(uint32_t target) override {
		GLRender::onRenderbufferStorage(target);
	}

	void onSwapBuffers() override {
		[_ctx flushBuffer];
	}

	UIView* make_surface_view(CGRect rect) override {
		[_ctx makeCurrentContext];
		Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context 1");

		post_message(Cb([this](Cb::Data& e) { // set current context from render loop
			[_ctx makeCurrentContext];
			Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context 2");
		}));
		
		_view = [[GLView alloc] initWithFrame:rect];
		// Enable retina-support
		_view.wantsBestResolutionOpenGLSurface = YES;
		// Enable layer-backed drawing of view
		_view.wantsLayer = YES;
		_layer = (GLLayer*)_view.layer;
		_layer.ctx = _ctx;
		_layer.host = _host;
		return _view;
	}

	Render* render() override {
		return this;
	}

private:
	GLView            *_view;
	GLLayer           *_layer;
	NSOpenGLContext   *_ctx;
};

QkAppleRender* makeAppleGLRender(Application* host, bool independentThread) {
	NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFANoRecovery, // 禁用所有故障恢复系统
		NSOpenGLPFAAccelerated, // 选择硬件加速渲染器
		NSOpenGLPFAColorSize, 24, // 颜色缓冲位数
		// NSOpenGLPFADoubleBuffer,   // 双缓冲
		// NSOpenGLPFADepthSize, 24,  // 深度缓冲位深
		// NSOpenGLPFAStencilSize, 8, // 模板缓冲位深
		// NSOpenGLPFAMultisample,    // 多重采样
		// NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1, //多重采样buffer
		// NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)4, // 多重采样数
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core, // OpenGL4.1
		//NSOpenGLPFAAllRenderers,NSOpenGLPFAOffScreen,NSOpenGLPFAAllowOfflineRenderers, 0
		0
	};
	auto format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	auto ctx = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];

	[ctx makeCurrentContext];
	Qk_ASSERT(NSOpenGLContext.currentContext, "Failed to set current OpenGL context");
	
	return new AppleGLRender(host, ctx, independentThread);
}

#endif
