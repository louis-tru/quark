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

extern QkApplicationDelegate *__appDelegate;

@interface GLView: UIView
{
	CADisplayLink *_display_link;
}
@property (assign, nonatomic) qk::Render *render;
@end

@implementation GLView

	+ (Class) layerClass {
		return CAEAGLLayer.class;
	}

	- (id) initWithFrame:(CGRect)frame {
		self = [super initWithFrame:frame];
		if (self) {
			_display_link = [CADisplayLink displayLinkWithTarget:self
																									selector:@selector(display:)];
			[_display_link addToRunLoop:[NSRunLoop mainRunLoop]
													forMode:NSDefaultRunLoopMode];
		}
		return self;
	}

	- (void) display:(CADisplayLink*)displayLink {
		 // auto _ = __appDelegate.host;
		 static int _fps = 0;
		 if (_fps == 0) { // 3 = 15, 1 = 30
			Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context 2");

			auto del = self.render->delegate();
			 if (del->onRenderBackendPreDisplay())
				 del->onRenderBackendDisplay();
			 _fps = 0;
		 } else {
			 _fps++;
		 }
	 }

	- (void) stopDisplay {
		[_display_link removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	}

@end

class AppleGLRender: public GLRender, public QkAppleRender {
public:
	AppleGLRender(Options opts, EAGLContext* ctx)
		: GLRender(opts), _ctx(ctx)
	{}

	~AppleGLRender() {
		[_view stopDisplay];
		[EAGLContext setCurrentContext:nullptr];
	}

	Vec2 getSurfaceSize() override {
		CGSize size = _view.frame.size;
		_default_scale = UIScreen.mainScreen.scale;
		float w = size.width * _default_scale;
		float h = size.height * _default_scale;
		_surface_size = Vec2(w, h);
		return _surface_size;
	}

	float getDefaultScale() override {
		_default_scale = UIScreen.mainScreen.scale;
		return _default_scale;
	}

	
	void setAntiAlias(int width, int height) override {
	}
	
	void setDepthBuffer(int width, int height) override {
	}

	void setRenderBuffer(int width, int height) override {
		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		[_ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer];
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);
	}

	void begin() override {
		if (_IsDeviceMsaa) {
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);
		} else {
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		}
	}

	void submit() override {
		if (_IsDeviceMsaa) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _msaa_frame_buffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frame_buffer);

			auto w = _surface_size.x(), h = _surface_size.y();
			glBlitFramebuffer(0, 0, w, h,
												0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 3, attachments);
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		} else {
			GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
		}

		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
		// you present its contents by making it the current renderbuffer
		// and calling the presentRenderbuffer: method on your rendering context.
		[_ctx presentRenderbuffer:GL_RENDERBUFFER];
	}

	UIView* make_surface_view(CGRect rect) override {
		[EAGLContext setCurrentContext:_ctx];
		Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context 2");

		_view = [[GLView alloc] initWithFrame:rect];
		_view.render = this;
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

QkAppleRender* qk_make_apple_gl_render(Render::Options opts) {
	EAGLContext* ctx = [EAGLContext alloc];
	if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
		[EAGLContext setCurrentContext:ctx];
		Qk_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
		ctx.multiThreaded = NO;
		return new AppleGLRender(opts, ctx);
	}
	return nullptr;
}

#endif // #if Qk_ENABLE_GL

