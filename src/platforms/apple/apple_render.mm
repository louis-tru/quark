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
#import "../../render/metal/metal_render.h"

using namespace qk;

uint32_t Render::post_message(Cb cb, uint64_t delay_us) {
	if (_renderLoop) {
		return _renderLoop->post(cb, delay_us);
	} else {
#if Qk_USE_DEFAULT_THREAD_RENDER
		auto core = cb.Handle::collapse();
		dispatch_async(dispatch_get_main_queue(), ^{
			core->resolve();
			core->release();
		});
		return 0;
	#else
		return _host->loop()->post(cb, delay_us);
	#endif
	}
}

// ------------------- Metal ------------------
#if Qk_ENABLE_METAL
@interface MTView: UIView
@end

@implementation MTView
+ (Class)layerClass {
	if (@available(iOS 13.0, *))
		return CAMetalLayer.class;
	return nil;
}
@end

class AppleMetalRender: public MetalRender, public QkAppleRender {
public:
	AppleMetalRender(Application* host, bool independentThread): MetalRender(host, independentThread)
	{}
	UIView* make_surface_view(CGRect rect) override {
		_view = [[MTKView alloc] initWithFrame:rect device:nil];
		_view.layer.opaque = YES;
		return _view;
	}
	Render* render() override {
		return this;
	}
};
#endif

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL
@interface GLView: UIView
@end

# if Qk_iOS
@implementation GLView
+ (Class)layerClass {
	return CAEAGLLayer.class;
}
@end
#else // #if Qk_iOS else osx
@implementation GLView
+ (Class)layerClass {
	return CAEAGLLayer.class;
}
@end
#endif // #if Qk_iOS

class AppleGLRender: public GLRender, public QkAppleRender {
public:
	static AppleGLRender* New(Application* host, bool independentThread) {
		EAGLContext* ctx = [EAGLContext alloc];
		if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
			[EAGLContext setCurrentContext:ctx];
			Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
			ctx.multiThreaded = NO;
			return new AppleGLRender(host, ctx, independentThread);
		}
		return nullptr;
	}

	AppleGLRender(Application* host, EAGLContext* ctx, bool independentThread)
		: GLRender(host, independentThread), _ctx(ctx) 
	{}

	~AppleGLRender() {
		[EAGLContext setCurrentContext:nullptr];
	}

	void onRenderbufferStorage(uint32_t target) override {
		if (! [_ctx renderbufferStorage:target fromDrawable:_layer] ) {
			Qk_FATAL();
		}
	}

	void onSwapBuffers() override {
		// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
		// you present its contents by making it the current renderbuffer
		// and calling the presentRenderbuffer: method on your rendering context.
		[_ctx presentRenderbuffer:GL_FRAMEBUFFER];
	}

	UIView* make_surface_view(CGRect rect) override {
		[EAGLContext setCurrentContext:_ctx];
		Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context 1");

		post_message(Cb([this](Cb::Data& e) { // set current context from render loop
			[EAGLContext setCurrentContext:_ctx];
			Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context 2");
		}));

		_view = [[GLView alloc] initWithFrame:rect];
		_layer = (CAEAGLLayer*)_view.layer;
		_layer.drawableProperties = @{
			kEAGLDrawablePropertyRetainedBacking : @NO,
			kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8
		};
		_layer.opaque = YES;
		//_layer.contentsGravity = kCAGravityTopLeft;

		return _view;
	}

	Render* render() override {
		return this;
	}

private:
	EAGLContext *_ctx;
	CAEAGLLayer *_layer;
	GLView      *_view;
};
#endif

Render* Render::Make(Application* host) {
	QkAppleRender* r = nullptr;
	bool independentThread = host->options().independentThread;

	if (independentThread) {
#if Qk_USE_DEFAULT_THREAD_RENDER
		independentThread = false; // use default thread render
#endif
	}

#if Qk_ENABLE_METAL
	if (@available(macOS 10.11, iOS 13.0, *))
		r = new AppleMetalRender(host, independentThread);
#endif
#if Qk_ENABLE_GL
	if (!r)
		r = AppleGLRender::New(host, independentThread);
#endif
	Qk_ASSERT(r);

	return r->render();
}
