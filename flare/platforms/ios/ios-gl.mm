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

#include "include/gpu/gl/GrGLInterface.h"
#include "../render/gl.h"

#import <OpenGLES/ES3/gl.h>
#import <UIKit/UIKit.h>

// using sk_app::window_context_factory::IOSWindowInfo;

@interface GLView : MainView
@end

@implementation GLView
+ (Class) layerClass {
	return [CAEAGLLayer class];
}
@end

namespace flare {

	struct IOSWindowInfo {
		UIViewController* fViewController;
	};

	class GLRender_ios : public GLRender {
	public:
		GLRender_ios(Application* host, const IOSWindowInfo&, const DisplayParams&);

		~GLRender_ios() override;

		void onSwapBuffers() override;

		sk_sp<const GrGLInterface> onInitializeContext() override;
		void onDestroyContext() override;

		void resize(Vec2 size, Region surface_region) override;

	private:
		GLView*              fGLView;
		EAGLContext*         fGLContext;
		GLuint               fFramebuffer;
		GLuint               fRenderbuffer;
	};

	GLRender_ios::GLRender_ios(const IOSWindowInfo& info, const DisplayParams& params)
		: INHERITED(params)
		// , fWindow(info.fWindow)
		// , fViewController(info.fViewController)
		, fGLContext(nil) {

		// any config code here (particularly for msaa)?

		this->initializeContext();
	}

	GLRender_ios::~GLRender_ios() {
		this->destroyContext();
		// [fGLView removeFromSuperview];
		// [fGLView release];
	}

	sk_sp<const GrGLInterface> GLRender_ios::onInitializeContext() {
		SkASSERT(fGLView);

		if (!fGLContext) {
			fGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];

			if (!fGLContext)
			{
				SkDebugf("Could Not Create OpenGL ES Context\n");
				return nullptr;
			}

			fGLContext.multiThreaded = NO;

			// Set up EAGLLayer
			CAEAGLLayer* eaglLayer = (CAEAGLLayer*)fGLView.layer;
			eaglLayer.drawableProperties = @{
				kEAGLDrawablePropertyRetainedBacking: @NO,
				kEAGLDrawablePropertyColorFormat    : kEAGLColorFormatRGBA8
			};

			eaglLayer.opaque = YES;
			// eaglLayer.frame = frameRect;
			// eaglLayer.contentsGravity = kCAGravityTopLeft;

		}

		if (![EAGLContext setCurrentContext:fGLContext]) {
			SkDebugf("Could Not Set OpenGL ES Context As Current\n");
			this->onDestroyContext();
			return nullptr;
		}

		// Set up framebuffer
		glGenFramebuffers(1, &fFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, fFramebuffer);

		glGenRenderbuffers(1, &fRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fRenderbuffer);

		[fGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			SkDebugf("Invalid Framebuffer\n");
			this->onDestroyContext();
			return nullptr;
		}

		glClearStencil(0);
		glClearColor(0, 0, 0, 255);
		glStencilMask(0xffffffff);
		glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		fStencilBits = 8;
		fSampleCount = 1; // TODO: handle multisampling

		fWidth = fViewController.view.frame.size.width;
		fHeight = fViewController.view.frame.size.height;

		glViewport(0, 0, fWidth, fHeight);

		return GrGLMakeNativeInterface();
	}

	void GLRender_ios::onDestroyContext() {
		glDeleteFramebuffers(1, &fFramebuffer);
		glDeleteRenderbuffers(1, &fRenderbuffer);
		[EAGLContext setCurrentContext:nil];
		// [fGLContext release];
		// fGLContext = nil;
	}

	void GLRender_ios::onSwapBuffers() {
		glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
		[fGLContext presentRenderbuffer:GL_RENDERBUFFER];
	}

	void GLRender_ios::resize(Vec2 size, Region surface_region) {
		// TODO: handle rotation
		// [fGLContext update];
		INHERITED::resize(size, surface_region);
	}

	Render* MakeGLForIOS(Application* host, const IOSWindowInfo& info,
												const DisplayParams& params) {
		Handle<WindowContext> ctx(new GLRender_ios(host, info, params));
		if (!ctx->isValid()) {
			return nullptr;
		}
		return ctx.collapse();
	}

}  // namespace flare
