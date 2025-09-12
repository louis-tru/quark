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

#include "../../util/macros.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#if Qk_ANDROID
# include <android/native_window.h>
#else
# include <X11/Xlib.h>
# undef Status
# undef Bool
# undef None
#endif

#include "./linux_render.h"
#include "../gl/gl_render.h"
#include "../gl/gl_cmd.h"

#if (Qk_LINUX || Qk_ANDROID) && Qk_ENABLE_GL

#define GL_ETC1_RGB8_OES  0x8D64
#define EGL_NO_NATIVE_WINDOW 0

namespace qk {

	typedef Render::Options Options;

	typedef std::remove_pointer_t<EGLDisplay> EGLDisplayType;

	static ThreadID emptyThreadID;

	static void closeEGLDisplay(EGLDisplay dpy){ eglTerminate(dpy); }

	typedef Sp<EGLDisplayType, object_traits_from<EGLDisplayType, closeEGLDisplay>> EGLDisplayAuto;

#ifndef Qk_ANDROID
	static void closeXDisplay(Display* dpy){ XCloseDisplay(dpy); }

	typedef Sp<Display, object_traits_from<Display, closeXDisplay>> XDisplayAuto;

	Display* openXDisplay() {
		static XDisplayAuto xdpy([]() {
			Qk_ASSERT(XInitThreads(), "Error: Can't init X threads");
			auto xdpy = XOpenDisplay(nullptr);
			Qk_CHECK(xdpy, "Can't open display");
			return xdpy;
		}());
		return xdpy.get();
	}
#endif

	static EGLDisplay egl_display() {
		static EGLDisplayAuto display = EGL_NO_DISPLAY;
		if (display.get() == EGL_NO_DISPLAY) { // get display and init it
			display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
			Qk_DLog("eglGetDisplay, %p", display.get());
			Qk_CHECK(display.get() != EGL_NO_DISPLAY);
			EGLBoolean displayState = eglInitialize(display.get(), nullptr, nullptr);
			Qk_CHECK(displayState, "Cannot initialize EGL");
		}
		return display.get();
	}

	static EGLConfig egl_config(EGLDisplay display, Options opts) {
		EGLConfig config = nullptr;
		EGLint MSAA = 0; // opts.msaaSample;

		// choose configuration
		EGLint attribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_RED_SIZE,   8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE,  8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 0,
			EGL_STENCIL_SIZE, 8,
			EGL_SAMPLE_BUFFERS, MSAA > 1 ? 1 : 0,
			EGL_SAMPLES, MSAA > 1 ? MSAA : 0,
			EGL_NONE
		};

		EGLint numConfigs = 0; // number of frame buffer configurations
		EGLBoolean chooseConfigState;

		// first we get size of all configurations
		eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

		if ( numConfigs == 0 ) {
			// attempt disable multi sample
			attribs[17] = 0;
			attribs[19] = 0;
			MSAA = 0;

			eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

			Qk_CHECK(numConfigs > 0, "We can't have EGLConfig array with zero size!");
		}

		Qk_DLog("numConfigs,%d", numConfigs);

		// then we create array large equarkh to store all configs
		Array<EGLConfig> supportedConfigs(numConfigs);

		// and load them
		chooseConfigState = eglChooseConfig(display, attribs, 
																				*supportedConfigs, numConfigs, &numConfigs);
		Qk_ASSERT(chooseConfigState);
		Qk_CHECK(numConfigs > 0, "Value of `numConfigs` must be positive");

		EGLint configIndex = 0;
		while (configIndex < numConfigs) {

			EGLConfig& cfg = supportedConfigs[configIndex];

			EGLint r, g ,b, a, s, samples;

			bool hasMatch =
					eglGetConfigAttrib(display, cfg, EGL_RED_SIZE,   &r) && r == 8
				&& eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) && g == 8
				&& eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE,  &b) && b == 8
				&& eglGetConfigAttrib(display, cfg, EGL_ALPHA_SIZE, &a) && a == 8
				&& eglGetConfigAttrib(display, cfg, EGL_STENCIL_SIZE, &s) && s == 8
				&& eglGetConfigAttrib(display, cfg, EGL_SAMPLES, &samples)
				&& (MSAA <= 1 || samples >= MSAA)
			;
			if ( hasMatch ) {
				Qk_DLog("hasMatch,%d", configIndex);
				config = supportedConfigs[configIndex];
				break;
			}
			configIndex++;
		}

		// if we don't find anything choose first one
		if ( configIndex == numConfigs ) {
			config = supportedConfigs[0];
		}

		// eglGetConfigAttrib(display, config, EGL_SAMPLES, &MSAA);

		return config;
	}

	class LinuxGLRender final: public GLRender, public RenderSurface {
	public:
		LinuxGLRender(Options opts, EGLDisplay display, EGLConfig config, EGLContext ctx)
			: GLRender(opts)
			, _display(display)
			, _config(config)
			, _context(ctx)
			, _surface(EGL_NO_SURFACE)
			, _win(EGL_NO_NATIVE_WINDOW)
		{}

		~LinuxGLRender() override {
			Qk_CHECK(_msg.length() == 0);
		}

		void release() override {
			lock();
			renderLoopStop();
			unlock();

			GLRender::release(); // Destroy the pre object first

			// Perform the final message task
			_mutexMsg.lock();
			if (_msg.length()) {
				if (_display != EGL_NO_DISPLAY && _context != EGL_NO_CONTEXT)
					Qk_CHECK(eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, _context));
				lock();
				for (auto &i : _msg)
					i->resolve();
				_msg.clear();
				unlock();
			}
			_mutexMsg.unlock();

			if (_display != EGL_NO_DISPLAY) {
				eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
				if (_context != EGL_NO_CONTEXT) eglDestroyContext(_display, _context);
				if (_surface != EGL_NO_SURFACE) eglDestroySurface(_display, _surface);
			}
			_display = EGL_NO_DISPLAY;
			_context = EGL_NO_CONTEXT;
			_surface = EGL_NO_SURFACE;

			Object::release(); // final destruction
		}

		RenderSurface* surface() override {
			return this;
		}

		bool isRenderThread() {
			return _renderThreadId == thread_self_id();
		}

		void lock() override {
			_mutex.lock();
		}

		void unlock() override {
			_mutex.unlock();
		}

		void post_message(Cb cb) override {
			if (isRenderThread()) {
				lock();
				cb->resolve();
				unlock();
			} else if (_renderThreadId == emptyThreadID) { // No run
				if (_mutexMsg.try_lock()) {
					_msg.push(cb);
					_mutexMsg.unlock();
				} else {
					cb->resolve();
				}
			} else {
				_mutexMsg.lock();
				_msg.push(cb);
				_mutexMsg.unlock();
			}
		}

		Vec2 getSurfaceSize() override {
			if (!_win) return {};
#if Qk_ANDROID
			Vec2 size(ANativeWindow_getWidth(_win), ANativeWindow_getHeight(_win));
#else
		XWindowAttributes attrs;
		auto dpy = openXDisplay();
		Qk_ASSERT_EQ(1, XGetWindowAttributes(dpy, _win, &attrs));
		// Qk_DLog("attrs.width: %d, attrs.height: %d", attrs.width, attrs.height);
		Vec2 size(attrs.width, attrs.height);
#endif
			return Vec2(size.width(), size.height());
		}

		void renderDisplay() override {
			lock();
			makeCurrent();

			if (_msg.length()) { //
				_mutexMsg.lock();
				auto msg(std::move(_msg));
				_mutexMsg.unlock();
				for (auto &i : msg) i->resolve();
			}

			if (_delegate->onRenderBackendDisplay()) {
				_glcanvas->flushBuffer(); // commit gl canvas cmd
				auto src = _glcanvas->surfaceSize();
				auto dest = _surfaceSize;
				auto filter = src == dest ? GL_NEAREST: GL_LINEAR;

				// copy pixels to default color buffer
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glBlitFramebuffer(0, 0, src[0], src[1], 0, 0, dest[0], dest[1], GL_COLOR_BUFFER_BIT, filter);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _glcanvas->fbo()); // bind frame buffer for main canvas

				glFlush(); // flush gl buffer, glFinish, glFenceSync, glWaitSync
				// GLuint fence = glCreateSyncTokens(1);
				// glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
				// glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, TIMEOUT)
				eglSwapBuffers(_display, _surface);
			}
			unlock();
		}

		void makeSurface(EGLNativeWindowType win) override {
			if (!_surface) {
				EGLSurface surface = eglCreateWindowSurface(_display, _config, win, nullptr);

				Qk_CHECK(surface, "Unable to create a drawing surface");

				_win = win;
				_surface = surface;
			}
		}

		void deleteSurface() override {
			if (_surface) {
				if (_renderThreadId == thread_self_id()) {
					_renderThreadId = ThreadID();
					eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr);
				}
				eglDestroySurface(_display, _surface);
				_surface = EGL_NO_SURFACE;
			}
		}

		void makeCurrent() {
			if (_renderThreadId == ThreadID()) {
				_renderThreadId = thread_self_id();
				Qk_CHECK(eglMakeCurrent(_display, _surface, _surface, _context),
					"Unable to create a drawing surface");
			}
			Qk_ASSERT_EQ(_renderThreadId, thread_self_id());
		}

		void renderLoopRun() {
			if (_renderThreadId != ThreadID())
				return;

			_renderThreadId = thread_new([this](cThread* t) {
				Qk_CHECK(eglMakeCurrent(_display, _surface, _surface, _context),
					"Unable to create a drawing surface");
				const int64_t intervalUs = 1e6 / 60; // 60 frames
				while (!t->abort) {
					auto timeUs = time_monotonic();
					renderDisplay();
					timeUs += intervalUs - time_monotonic();
					if (timeUs >= 0) {
						thread_sleep(timeUs);
					}
				}
				Qk_CHECK(eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr));
				_renderThreadId = ThreadID();
			}, "linux_render_Thread");
		}

		void renderLoopStop() {
			if (_renderThreadId != ThreadID()) {
				thread_try_abort(_renderThreadId);
				thread_join_for(_renderThreadId);
				_renderThreadId = ThreadID();
			}
		}

	private:
		EGLDisplay _display;
		EGLConfig _config;
		EGLContext _context;
		EGLSurface _surface;
		EGLNativeWindowType _win;
		Mutex _mutexMsg;
		Array<Cb> _msg;
		ThreadID _renderThreadId;
		RecursiveMutex _mutex;
	};

	// ------------------------------------------

	class LinuxRenderResource: public GLRenderResource {
	public:
		Qk_DEFINE_PROP_GET(EGLDisplay, dpy);
		Qk_DEFINE_PROP_GET(EGLContext, ctx);
		LinuxRenderResource(EGLDisplay dpy, EGLContext ctx)
			: GLRenderResource(current_loop()), _dpy(dpy), _ctx(ctx) {
		}
		~LinuxRenderResource() {
			eglMakeCurrent(_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroyContext(_dpy, _ctx);
		}
	};

	static LinuxRenderResource* g_sharedRenderResource = nullptr;

	RenderResource* getSharedRenderResource() {
		return g_sharedRenderResource;
	}

	Render* make_gl_render(Options opts) {
		Render* r = nullptr;

		EGLDisplay dpy = egl_display();
		EGLConfig cfg = egl_config(dpy, opts);

		static EGLint attrs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 3, // opengl es 3
			EGL_NONE
		};

		if (!g_sharedRenderResource) {
			auto ctx = eglCreateContext(dpy, cfg, nullptr, attrs);
			if (!ctx) return nullptr;
			g_sharedRenderResource = new LinuxRenderResource(dpy, ctx);
		}

		auto ctx = eglCreateContext(dpy, cfg, g_sharedRenderResource->ctx(), attrs);
		if ( ctx ) {
			Qk_CHECK(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx));
			Qk_ASSERT_EQ(eglGetCurrentContext(), ctx, "eglGetCurrentContext()");
			r = new LinuxGLRender(opts, dpy, cfg, ctx);
			Qk_CHECK(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr));
		}

		g_sharedRenderResource->post_message(Cb([dpy](auto e) {
			eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, g_sharedRenderResource->ctx());
		}));

		return r;
	}
}
#endif