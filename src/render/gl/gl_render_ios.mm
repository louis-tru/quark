/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "../../util/macros.h"
#if Qk_ENABLE_GL && Qk_iOS

#import "../plotforms.h"
#import "./gl_render.h"

using namespace qk;

namespace qk {
	void post_message_main(Cb cb, bool sync);
}

class IosGLRender;
@interface GLView: UIView {
	CADisplayLink *_displayLink;
	IosGLRender   *_render;
}
@property (assign, nonatomic) bool isRun;
- (id)   init:(IosGLRender*)render;
- (void) stopDisplay;
@end

// ----------------------------------------------------------------------------------------------

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
		Qk_ASSERT_EQ(_fbo_0, 0);
	}

	void release() override {
		lock();
		if (_view)
			[_view stopDisplay]; // thread task must be forced to end
		unlock();

		GLRender::release(); // Destroy the pre object first

		GLuint
			fbo = _fbo_0,
			rbo = _rbo_0;
		_fbo_0 = _rbo_0 = 0; // clear
		post_message(Cb([fbo,rbo](auto &e) {
			glDeleteFramebuffers(1, &fbo);
			glDeleteRenderbuffers(1, &rbo);
		}));

		_ctx = nil;
		_layer = nil;
		_view = nil;
		Object::release(); // final destruction
	}

	bool isRenderThread() {
		return _threadId == thread_self_id();
	}

	void lock() override {
		if (!isRenderThread()) { // avoid deadlock if already in render thread
			_mutex.lock();
			[EAGLContext setCurrentContext:_ctx];
		}
	}

	void unlock() override {
		if (!isRenderThread()) {
			_mutex.unlock();
		}
	}

	RenderSurface* surface() override {
		return this;
	}

	void post_message(Cb cb) override {
		Qk_ASSERT(_ctx, "Render context is null. Cannot post message.");
		// Send it directly to the main thread,
		// because iOS renders everything on the main thread.
		post_message_main(cb, false);
	}

	Vec2 getSurfaceSize() override {
		if (!_view) return {};
		CGSize size = _view.frame.size;
		float defaultScale = UIScreen.mainScreen.scale;
		return Vec2(size.width * defaultScale, size.height * defaultScale);
	}

	void renderDisplay() {
		_mutex.lock();
		_threadId = thread_self_id();
		Qk_ASSERT_EQ(EAGLContext.currentContext, _ctx, "Failed to set current OpenGL context");

		if (_delegate->onRenderBackendDisplay()) {
			_glcanvas->flushBuffer(); // commit gl canvas cmd

			auto src = _glcanvas->surfaceSize();
			auto dest = _surfaceSize;
			auto filter = src == dest ? GL_NEAREST: GL_LINEAR;

			GLenum attachments[] = {GL_COLOR_ATTACHMENT1,GL_STENCIL_ATTACHMENT,GL_DEPTH_ATTACHMENT};
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, sizeof(attachments)/sizeof(GLenum), attachments);

			if (_rbo_0_size != _surfaceSize) { // re-allocate render buffer if surface size changed
				_rbo_0_size = _surfaceSize;
				glBindFramebuffer(GL_FRAMEBUFFER, _fbo_0);
				glBindRenderbuffer(GL_RENDERBUFFER, _rbo_0);
				// allocate render buffer storage from layer
				[_ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable: _layer];
				// attach render buffer to framebuffer
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo_0);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, _glcanvas->fbo()); // restore default
			}
 #if 0
			// copy pixels to fob_0/rbo_0 color buffer
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_0);
			glBlitFramebuffer(0, 0, src[0], src[1], 0, 0, dest[0], dest[1], GL_COLOR_BUFFER_BIT, filter);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _glcanvas->fbo()); // restore default framebuffer
 #else
			_glcanvas->vportCopy(_fbo_0); // copy pixels to fob_0/rbo_0 color buffer
 #endif
			glFlush(); // flush gl buffer, glFinish, glFenceSync, glWaitSync

			// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
			// you present its contents by making it the current renderbuffer
			// and calling the presentRenderbuffer: method on your rendering context.;
			glBindRenderbuffer(GL_RENDERBUFFER, _rbo_0);
			[_ctx presentRenderbuffer: GL_RENDERBUFFER];
		}
		_threadId = qk::ThreadID();
		_mutex.unlock();
	}

	UIView* surfaceView() override {
		if (_view) return _view;

		[EAGLContext setCurrentContext:_ctx];
		Qk_ASSERT_EQ(EAGLContext.currentContext, _ctx, "Failed to set current OpenGL context");

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
	Vec2       _rbo_0_size;
	GLuint     _fbo_0, _rbo_0;
	Mutex      _mutex;
	qk::ThreadID _threadId;
};

// ----------------------------------------------------------------------------------------------

@implementation GLView

	+ (Class) layerClass {
		return CAEAGLLayer.class;
	}

	- (id) init:(IosGLRender*)render {
		self = [super init];
		if (self) {
			_isRun = true;
			_render = render;
			_displayLink = [CADisplayLink displayLinkWithTarget:self
																									selector:@selector(renderDisplay:)];
			//_displayLink.frameInterval = 0;
			//_displayLink.preferredFramesPerSecond = 0;
			[_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
		}
		return self;
	}

	- (void) renderDisplay:(CADisplayLink*)displayLink {
		if (_isRun) {
			@autoreleasepool {
				_render->renderDisplay();
			}
		}
	}

	- (void) stopDisplay {
		_isRun = false;
		[_displayLink removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	}

@end

// ----------------------------------------------------------------------------------------------

namespace qk {

	static IosGLRender* g_sharedRenderResource = nullptr;

	void* acquireRenderBackendStorage(size_t typeHash, size_t size);

	RenderResource* getSharedRenderResource() {
		return g_sharedRenderResource;
	}

	Render* make_gl_render(Render::Options opts) {
		// iOS only allows one window and one drawing context
		Qk_CHECK(!g_sharedRenderResource,
			"The iOS system only allows one window and one drawing context"
		);

		auto ctx = [EAGLContext alloc];
		if (![ctx initWithAPI:kEAGLRenderingAPIOpenGLES3])
			return nullptr;
		[EAGLContext setCurrentContext:ctx];
		Qk_ASSERT(EAGLContext.currentContext, "Failed to set current OpenGL context");

		// disable multithreaded for iOS
		ctx.multiThreaded = NO;

		// allocate render backend storage
		auto mem = acquireRenderBackendStorage(typeid(IosGLRender).hash_code(), sizeof(IosGLRender));
		// iOS only create one window and one drawing context
		// so we use a render backend as shared render resource
		g_sharedRenderResource = new(mem) IosGLRender(opts, ctx);

		g_sharedRenderResource->post_message(Cb([ctx](auto e) {
			// Ensure the GL context is current on the render thread.
			// (Context binding is thread-local on iOS)
			[EAGLContext setCurrentContext:ctx];
		}));

		return g_sharedRenderResource;
	}
}
#endif // #if Qk_ENABLE_GL
