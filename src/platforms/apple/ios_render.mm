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

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL && Qk_iOS

@interface GLView: UIView
@end

@implementation GLView
+ (Class)layerClass {
	return CAEAGLLayer.class;
}
@end

class AppleGLRender: public GLRender, public QkAppleRender {
public:
	AppleGLRender(Application* host, EAGLContext* ctx, bool independentThread)
		: GLRender(host, independentThread), _ctx(ctx)
	{}

	~AppleGLRender() {
		[EAGLContext setCurrentContext:nullptr];
	}
	
	void setAntiAlias(int width, int height) override {
	}

	void setRenderBuffer(int width, int height) override {
		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		[_ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer];
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);
	}

	void present() override {
		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
		// you present its contents by making it the current renderbuffer
		// and calling the presentRenderbuffer: method on your rendering context.
		[_ctx presentRenderbuffer:GL_RENDERBUFFER];
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
		//_layer.presentsWithTransaction = YES; // sync or async draw
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

QkAppleRender* makeAppleGLRender(Application* host, bool independentThread) {
	EAGLContext* ctx = [EAGLContext alloc];
	if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
		[EAGLContext setCurrentContext:ctx];
		Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
		ctx.multiThreaded = NO;
		return new AppleGLRender(host, ctx, independentThread);
	}
	return nullptr;
}

#endif // #if Qk_ENABLE_GL

