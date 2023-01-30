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
// #include "../../render/skia/skia_render.h"

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
# if Qk_IOS
@implementation GLView
	+ (Class)layerClass {
		return CAEAGLLayer.class;
	}
@end
# else // #if Qk_IOS else osx
@implementation GLView
	+ (Class)layerClass {
		return CAEAGLLayer.class;
	}
@end
# endif // #if Qk_IOS
#endif

// namespace start

namespace quark {

	bool RenderApple::resize(CGRect rect) {
#if Qk_IOS
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

	// template<class RenderIMPL>
	class AppleMetalRender: public SkiaMetalRender, public RenderApple {
	public:
		AppleMetalRender(Application* host, bool raster): SkiaMetalRender(host, opts, raster)
		{}
		UIView* init(CGRect rect) override {
			_view = [[MTKView alloc] initWithFrame:rect device:nil];
			_view.layer.opaque = YES;
			return _view;
		}
		Render* render() override { return this; }
	};

	// ------------------- OpenGL ------------------

#if Qk_ENABLE_GL

	class AppleGLRenderBase: public RenderApple {
	public: 
		AppleGLRenderBase(EAGLContext* ctx): _ctx(ctx) {
			Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
			ctx.multiThreaded = NO;
		}
		~AppleGLRenderBase() {
			[EAGLContext setCurrentContext:nullptr];
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

		void renderbufferStorage(uint32_t target) {
			BOOL ok = [_ctx renderbufferStorage:target fromDrawable:_layer]; Qk_ASSERT(ok);
		}

		void swapBuffers() {
			// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
			// you present its contents by making it the current renderbuffer
			// and calling the presentRenderbuffer: method on your rendering context.
			[_ctx presentRenderbuffer:GL_FRAMEBUFFER];
		}

	private:
		EAGLContext *_ctx;
		CAEAGLLayer *_layer;
		GLView      *_view;
	};

	// template<class RenderIMPL>
	class AppleGLRender: public RenderIMPL, public AppleGLRenderBase {
	public:
		AppleGLRender(Application* host, EAGLContext* ctx, bool raster)
			: RenderIMPL(host, raster), AppleGLRenderBase(ctx)
		{
			//_is_support_multisampled = true;
		}
		Render* render() override { return this; }
		void onRenderbufferStorage(uint32_t target) override { renderbufferStorage(target); }
		void onSwapBuffers() override { swapBuffers(); }

		static AppleGLRender* New(Application* host, bool raster) {
			EAGLContext* ctx = [EAGLContext alloc];
			if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
				[EAGLContext setCurrentContext:ctx];
				return new AppleGLRender<RenderIMPL>(host, ctx, raster);
			}
			return nullptr;
		}
	};

#endif

#ifndef Qk_ENABLE_GPU
# define Qk_ENABLE_GPU 1
#endif
#ifndef Qk_ENABLE_METAL
# define Qk_ENABLE_METAL 1
#endif

	RenderApple* RenderApple::Make(Application* host) {
		RenderApple* r = nullptr;

		if (Qk_ENABLE_GPU) {
			if (@available(macOS 10.11, iOS 13.0, *)) {
				if (Qk_ENABLE_METAL)
					r = new AppleMetalRender<SkiaMetalRender>(host, false);
			}
#if Qk_ENABLE_GL
			if (!r) {
				r = AppleGLRender<SkiaGLRender>::New(host, false);
			}
#endif
		}

		if (!r) {
			if (@available(macOS 10.11, iOS 13.0, *)) {
				if (Qk_ENABLE_METAL)
					r = new AppleMetalRender<SkiaMetalRender>(host, true);
			}
#if Qk_ENABLE_GL
			if (!r) {
				r = AppleGLRender<SkiaGLRender>::New(host, true);
			}
#endif
		}

		Qk_ASSERT(r);
		return r;
	}

}
