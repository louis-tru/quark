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

#include "./apple_render.h"
#include "../../display.h"
#include "../../render/gl/gl_render.h"
#include "../../render/metal/metal_render.h"

@interface MTView: UIView @end

@implementation MTView
	+ (Class)layerClass {
		if (@available(iOS 13.0, *))
			return CAMetalLayer.class;
		return nil;
	}
@end

#if Qk_ENABLE_GL
@interface GLView: UIView @end
# if Qk_iOS
@implementation GLView
	+ (Class)layerClass { return CAEAGLLayer.class; }
@end
# else // #if Qk_iOS else osx
@implementation GLView
	+ (Class)layerClass { return CAEAGLLayer.class; }
@end
# endif // #if Qk_iOS
#endif

// namespace start

namespace qk {

	bool AppleRender::resize(CGRect rect) {
#if Qk_iOS
		float scale = UIScreen.mainScreen.scale;
#else
		float scale = UIScreen.mainScreen.backingScaleFactor;
#endif
		float x = rect.size.width * scale;
		float y = rect.size.height * scale;
		return render()->host()->display()->set_display_region({ 0,0,x,y,x,y });
	}

	uint32_t Render::post_message(Cb cb, uint64_t delay_us) {
#if Qk_USE_DEFAULT_THREAD_RENDER
		auto core = cb.Handle::collapse();
		dispatch_async(dispatch_get_main_queue(), ^{
			Cb cb(core);
			cb->resolve();
		});
		return 0;
#else
		return render()->host()->loop()->post(cb, delay_us);
#endif
	}

	// ------------------- Metal ------------------

#if Qk_ENABLE_METAL

	class AppleMetalRender: public MetalRender, public AppleRender {
	public:
		AppleMetalRender(Application* host): MetalRender(host)
		{}
		UIView* init(CGRect rect) override {
			_view = [[MTKView alloc] initWithFrame:rect device:nil];
			_view.layer.opaque = YES;
			return _view;
		}
		Render* render() override { return this; }
	};

#endif

	// ------------------- OpenGL ------------------

#if Qk_ENABLE_GL

	class AppleGLRender: public GLRender, public AppleRender {
	public:
		static AppleGLRender* New(Application* host) {
			EAGLContext* ctx = [EAGLContext alloc];
			if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
				[EAGLContext setCurrentContext:ctx];
				return new AppleGLRender(host, ctx);
			}
			return nullptr;
		}

		AppleGLRender(Application* host, EAGLContext* ctx)
			: GLRender(host), _ctx(ctx) 
		{
			Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
			// ctx.multiThreaded = NO;
		}

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

		UIView* init(CGRect rect) override {
			[EAGLContext setCurrentContext:_ctx];
			Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
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

		Render* render() override { return this; }

	private:
		EAGLContext *_ctx;
		CAEAGLLayer *_layer;
		GLView      *_view;
	};

#endif

	Render* Render::Make(Application* host) {
		RenderApple* r = nullptr;

		if (!r) {
#if Qk_ENABLE_METAL
			if (@available(macOS 10.11, iOS 13.0, *)) {
				if (Qk_ENABLE_METAL)
					r = new AppleMetalRender(host);
			}
#endif
#if Qk_ENABLE_GL
			if (!r) {
				r = AppleGLRender::New(host);
			}
#endif
		}
		Qk_ASSERT(r);

		return r->render();
	}

}
