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
#include <OpenGLES/ES3/gl.h>

namespace flare {

	bool RenderApple::resize(::CGRect rect) {
		float scale = UIScreen.mainScreen.scale;
		float x = rect.size.width * scale;
		float y = rect.size.height * scale;
		return render()->host()->display()->set_surface_region({ 0,0,x,y,x,y });
	}

	uint32_t Render::post_message(Cb cb, uint64_t delay_us) {
		auto core = cb.Handle::collapse();
		dispatch_async(dispatch_get_main_queue(), ^{
			Cb cb(core);
			cb->resolve();
		});
		return 0;
	}

	class AppleGLRender_IMPL: public RenderApple {
		public:

			AppleGLRender_IMPL(EAGLContext* ctx): _ctx(ctx) {
				F_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
				ctx.multiThreaded = NO;
			}

			~AppleGLRender_IMPL() {
				[EAGLContext setCurrentContext:nullptr];
			}

			void setView(UIView* view) override {
				F_ASSERT(!_view);
				[EAGLContext setCurrentContext:_ctx];
				F_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
				_view = view;
				_layer = (CAEAGLLayer*)view.layer;
				_layer.drawableProperties = @{
					kEAGLDrawablePropertyRetainedBacking : @NO,
					kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8
				};
				_layer.opaque = YES;
				//_layer.frame = frameRect;
				//_layer.contentsGravity = kCAGravityTopLeft;
			}

			Class layerClass() override { return [CAEAGLLayer class]; }

			void glRenderbufferStorage() {
				BOOL ok = [_ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer];
				F_ASSERT(ok);
			}

			void eglSwapBuffers() {
				// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
				// you present its contents by making it the current renderbuffer
				// and calling the presentRenderbuffer: method on your rendering context.
				[_ctx presentRenderbuffer:GL_FRAMEBUFFER];
			}
			
		private:
			EAGLContext* _ctx;
			CAEAGLLayer* _layer;
			UIView* _view;
	};

	class AppleGLRender: public GLRender, public AppleGLRender_IMPL {
		public:
			AppleGLRender(Application* host, const Options& params, EAGLContext* ctx)
				: GLRender(host, params), AppleGLRender_IMPL(ctx)
			{
				_is_support_multisampled = true;
			}
			Render* render() override { return this; }
			void glRenderbufferStorage() override { AppleGLRender_IMPL::glRenderbufferStorage(); }
			void eglSwapBuffers() override { AppleGLRender_IMPL::eglSwapBuffers(); }
	};

	class AppleRasterRender: public RasterRender, public AppleGLRender_IMPL {
		public:
			AppleRasterRender(Application* host, const Options& params, EAGLContext* ctx)
				: RasterRender(host, params), AppleGLRender_IMPL(ctx)
			{}
			Render* render() override { return this; }
			void glRenderbufferStorage() override { AppleGLRender_IMPL::glRenderbufferStorage(); }
			void eglSwapBuffers() override { AppleGLRender_IMPL::eglSwapBuffers(); }
	};

	template<class IMPL>
	RenderApple* MakeRenderApple(Application* host, const Render::Options& parems) {
		EAGLContext* ctx = [EAGLContext alloc];
		if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
			[EAGLContext setCurrentContext:ctx];
			return new IMPL(host, parems, ctx);
		}
		return nullptr;
	}

	RenderApple* RenderApple::Make(Application* host, const Render::Options& opts) {
		RenderApple* r = nullptr;

		if (0/*opts.enableGpu*/) {
			r = MakeRenderApple<AppleGLRender>(host, opts);
		}
		if (r) {
			return r;
		}
		r = MakeRenderApple<AppleRasterRender>(host, opts);
		F_ASSERT(r);

		return r;
	}

}  // namespace flare
