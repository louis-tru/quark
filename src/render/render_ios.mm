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

#include "./render_mac.h"
#import "./gl/gl_render.h"

using namespace qk;

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL && defined(Qk_iOS)

class IosGLRender;

@interface GLView: UIView
{
	CADisplayLink *_displayLink;
	IosGLRender   *_render;
}
@property (assign, nonatomic) bool isRun;
- (id)   init:(IosGLRender*)render;
- (void) stopDisplay;
@end

// ----------------------------------------------------------------------------------------------

void qk_post_messate_main(Cb cb, bool sync);

class IosGLRender final: public GLRender, public RenderSurface {
public:
	IosGLRender(Options opts, EAGLContext* ctx)
		: GLRender(opts), _ctx(ctx), _layer(nil), _view(nil), _fbo_0(0), _rbo_0(0)
	{
		glGenFramebuffers(1, &_fbo_0);
		glGenRenderbuffers(1, &_rbo_0);
	}

	~IosGLRender() {
		[EAGLContext setCurrentContext:nil];
		Qk_Assert_Eq(_fbo_0, 0);
	}

	void release() override {
		lock();
		if (_view) {
			[_view stopDisplay]; // thread task must be forced to end
		}
		unlock();

		GLRender::release(); // Destroy the pre object first

		GLuint fbo = _fbo_0, rbo = _rbo_0;
		_fbo_0 = _rbo_0 = 0;
		post_message(Cb([fbo,rbo](auto &e) {
			glDeleteFramebuffers(1, &fbo);
			glDeleteRenderbuffers(1, &rbo);
		}));

		Object::release(); // final destruction
	}

	RenderSurface* surface() override {
		return this;
	}

	void reload() override {
		GLRender::reload();
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo_0);
		glBindRenderbuffer(GL_RENDERBUFFER, _rbo_0);
		[_ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable: _layer];
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo_0);
	}

	void lock() override {
		_mutex.lock();
	}

	void unlock() override {
		_mutex.unlock();
	}

	void post_message(Cb cb) override {
		qk_post_messate_main(cb, false);
	}

	Vec2 getSurfaceSize(float *defaultScaleOut) override {
		if (!_view) return {};
		CGSize size = _view.frame.size;
		float defaultScale = UIScreen.mainScreen.scale;
		if (defaultScaleOut)
			*defaultScaleOut = defaultScale;
		return Vec2(size.width * defaultScale, size.height * defaultScale);
	}

	void renderDisplay() {
		lock();

		if (_delegate->onRenderBackendDisplay()) {
			_glcanvas->flushBuffer(); // commit gl canvas cmd

			auto src = _glcanvas->surfaceSize();
			auto dest = _surfaceSize;
			auto filter = src == dest ? GL_NEAREST: GL_LINEAR;

			GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT };
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 4, attachments);
			// copy pixels to default color buffer
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_0);
			glBlitFramebuffer(0, 0, src[0], src[1], 0, 0, dest[0], dest[1], GL_COLOR_BUFFER_BIT, filter);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _glcanvas->fbo()); // bind frame buffer for main canvas
			glFlush(); // flush gl buffer, glFinish, glFenceSync, glWaitSync

			glBindRenderbuffer(GL_RENDERBUFFER, _rbo_0);
			// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
			// you present its contents by making it the current renderbuffer
			// and calling the presentRenderbuffer: method on your rendering context.;
			//[_ctx presentRenderbuffer: GL_FRAMEBUFFER];
			[_ctx presentRenderbuffer: GL_RENDERBUFFER];
		}
		unlock();
	}

	UIView* surfaceView() override {
		if (_view) return _view;

		[EAGLContext setCurrentContext:_ctx];
		Qk_Assert_Eq(EAGLContext.currentContext, _ctx, "Failed to set current OpenGL context");

		_view = [[GLView alloc] init: this];
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

private:
	EAGLContext *_ctx;
	CAEAGLLayer *_layer;
	GLView      *_view;
	GLuint     _fbo_0, _rbo_0;
	RecursiveMutex _mutex;
};

@implementation GLView

+ (Class) layerClass {
	return CAEAGLLayer.class;
}

- (id) init:(IosGLRender*)render {
	self = [super initWithFrame:UIScreen.mainScreen.bounds];
	if (self) {
		_isRun = true;
		_render = render;
		_displayLink = [CADisplayLink displayLinkWithTarget:self
																								selector:@selector(renderDisplay:)];
		[_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
	}
	return self;
}

- (void) renderDisplay:(CADisplayLink*)displayLink {
	if (_isRun) {
		Qk_Assert(EAGLContext.currentContext, "Failed to set current OpenGL context");
		_render->renderDisplay();
	}
}

- (void) stopDisplay {
	[_displayLink removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	_isRun = false;
}

@end

// ----------------------------------------------------------------------------------------------

namespace qk {

	Render* make_mac_gl_render(Render::Options opts) {
		auto ctx = [EAGLContext alloc];
		if ([ctx initWithAPI:kEAGLRenderingAPIOpenGLES3]) {
			[EAGLContext setCurrentContext:ctx];
			Qk_Assert(EAGLContext.currentContext, "Failed to set current OpenGL context");
			ctx.multiThreaded = NO;
			return new IosGLRender(opts, ctx);
		}
		return nullptr;
	}
}
#endif // #if Qk_ENABLE_GL
