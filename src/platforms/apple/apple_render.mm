/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
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
#include "../../render/skia/skia_render.h"

@interface MTView: UIView @end
@implementation MTView
	+ (Class)layerClass {
		if (@available(iOS 13.0, *))
			return CAMetalLayer.class;
		return nil;
	}
@end

#if F_ENABLE_GL
@interface GLView: UIView @end
# if F_IOS
@implementation GLView
	+ (Class)layerClass {
		return CAEAGLLayer.class;
	}
@end
# else // #if F_IOS else osx
@implementation GLView
	+ (Class)layerClass {
		return CAEAGLLayer.class;
	}
@end
# endif // #if F_IOS
#endif


// namespace start

namespace noug {

	bool RenderApple::resize(CGRect rect) {
#if F_IOS
		float scale = UIScreen.mainScreen.scale;
#else
		float scale = UIScreen.mainScreen.backingScaleFactor;
#endif
		float x = rect.size.width * scale;
		float y = rect.size.height * scale;
		return render()->host()->display()->set_display_region({ 0,0,x,y,x,y });
	}

	uint32_t Render::post_message(Cb cb, uint64_t delay_us) {
		auto core = cb.Handle::collapse();
		dispatch_async(dispatch_get_main_queue(), ^{
			Cb cb(core);
			cb->resolve();
		});
		return 0;
	}

	// ------------------- Metal ------------------

	template<class RenderIMPL>
	class AppleMetalRender: public RenderIMPL, public RenderApple {
	public:
		AppleMetalRender(Application* host, const Render::Options& opts, bool raster): RenderIMPL(host, opts, raster)
		{}
		UIView* init(CGRect rect) override {
			MTKView* view = this->_view = [[MTKView alloc] initWithFrame:rect device:nil];
			//UIView* view = [[MTKView alloc] initWithFrame:rect];
			view.layer.opaque = YES;
			return view;
		}
		Render* render() override { return this; }
	};

	// ------------------- OpenGL ------------------

#if F_ENABLE_GL

	class AppleGLRenderBase: public RenderApple {
	public: 
		AppleGLRenderBase(EAGLContext* ctx): _ctx(ctx) {
			F_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
			ctx.multiThreaded = NO;
		}
		~AppleGLRenderBase() {
			[EAGLContext setCurrentContext:nullptr];
		}

		UIView* init(CGRect rect) override {
			[EAGLContext setCurrentContext:_ctx];
			F_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
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
			BOOL ok = [_ctx renderbufferStorage:target fromDrawable:_layer]; F_ASSERT(ok);
		}

		void swapBuffers() {
			// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
			// you present its contents by making it the current renderbuffer
			// and calling the presentRenderbuffer: method on your rendering context.
			[_ctx presentRenderbuffer:GL_FRAMEBUFFER];
		}
		
	private:
		EAGLContext* _ctx;
		CAEAGLLayer* _layer;
		GLView* _view;
	};

	template<class RenderIMPL>
	class AppleGLRender: public RenderIMPL, public AppleGLRenderBase {
	public:
		AppleGLRender(Application* host, const Render::Options& params, EAGLContext* ctx, bool raster)
			: RenderIMPL(host, params, raster), AppleGLRenderBase(ctx)
		{
			//_is_support_multisampled = true;
		}
		Render* render() override { return this; }
		void onRenderbufferStorage(uint32_t target) override { renderbufferStorage(target); }
		void onSwapBuffers() override { swapBuffers(); }

		static AppleGLRender* New(Application* host, const Render::Options& parems, bool raster) {
			EAGLContext* ctx = [EAGLContext alloc];
			if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
				[EAGLContext setCurrentContext:ctx];
				return new AppleGLRender<RenderIMPL>(host, parems, ctx, raster);
			}
			return nullptr;
		}
	};

#endif

#ifndef F_ENABLE_GPU
# define F_ENABLE_GPU 1
#endif
#ifndef F_ENABLE_METAL
# define F_ENABLE_METAL 1
#endif

	RenderApple* RenderApple::Make(Application* host, const Render::Options& opts) {
		RenderApple* r = nullptr;

		if (F_ENABLE_GPU) {
			if (@available(macOS 10.11, iOS 13.0, *)) {
				if (F_ENABLE_METAL)
					r = new AppleMetalRender<SkiaMetalRender>(host, opts, false);
			}
#if F_ENABLE_GL
			if (!r) {
				r = AppleGLRender<SkiaGLRender>::New(host, opts, false);
			}
#endif
		}

		if (!r) {
			if (@available(macOS 10.11, iOS 13.0, *)) {
				if (F_ENABLE_METAL)
					r = new AppleMetalRender<SkiaMetalRender>(host, opts, true);
			}
#if F_ENABLE_GL
			if (!r) {
				r = AppleGLRender<SkiaGLRender>::New(host, opts, true);
			}
#endif
		}

		F_ASSERT(r);
		return r;
	}

}
