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
#include "../../render/gl/gl.h"
#include "../../render/metal/metal.h"

#if F_IOS
#include <OpenGLES/ES3/gl.h>
#else
#include <OpenGL/gl3.h>
#endif

#if F_IOS
@interface MTView: UIView @end
@interface GLView: UIView @end

@implementation GLView
	+ (Class)layerClass { return CAEAGLLayer.class; }
@end
@implementation MTView
+ (Class)layerClass {
	if (@available(iOS 13.0, *))
		return CAMetalLayer.class;
	return nil;
}
@end
#else
@interface MTView: UIView @end
@implementation MTView
+ (Class)layerClass {
	return CAMetalLayer.class;
}
@end
#endif

namespace flare {

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

	template<class BASE>
	class AppleMetalRender: public BASE, public RenderApple {
		public:
			AppleMetalRender(Application* host, const Render::Options& opts): BASE(host, opts)
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
	
#if F_IOS

	class AppleGLRender: public RenderApple {
		public: 

			AppleGLRender(EAGLContext* ctx): _ctx(ctx) {
				F_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
				ctx.multiThreaded = NO;
			}
			~AppleGLRender() {
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

	template<class BASE>
	class AppleGLRender_IMPL: public BASE, public AppleGLRender {
		public:
			AppleGLRender_IMPL(Application* host, const Render::Options& params, EAGLContext* ctx)
				: BASE(host, params), AppleGLRender(ctx)
			{
				//_is_support_multisampled = true;
			}
			Render* render() override { return this; }
			void renderbufferStorage(uint32_t target) override { AppleGLRender::renderbufferStorage(target); }
			void swapBuffers() override { AppleGLRender::swapBuffers(); }
	};

	template<class BASE>
	RenderApple* MakeAppleGLRender(Application* host, const Render::Options& parems) {
		EAGLContext* ctx = [EAGLContext alloc];
		if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
			[EAGLContext setCurrentContext:ctx];
			return new AppleGLRender_IMPL<BASE>(host, parems, ctx);
		}
		return nullptr;
	}
	
#endif

	RenderApple* RenderApple::Make(Application* host, const Render::Options& opts) {
		RenderApple* r = nullptr;

		if (opts.enableGpu) {
			if (@available(macOS 10.11, iOS 13.0, *)) {
				if (!opts.disableMetal)
					r = new AppleMetalRender<MetalRender>(host, opts);
			}
#if F_IOS
			if (!r) {
				r = MakeAppleGLRender<GLRender>(host, opts);
			}
#endif
		}

		if (!r) {
			if (@available(macOS 10.11, iOS 13.0, *)) {
				if (!opts.disableMetal)
					r = new AppleMetalRender<RasterMetalRender>(host, opts);
			}
#if F_IOS
			if (!r) {
				r = MakeAppleGLRender<RasterGLRender>(host, opts);
			}
#endif
		}

		F_ASSERT(r);
		return r;
	}

}  // namespace flare
