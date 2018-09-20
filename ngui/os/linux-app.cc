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

#include "ngui/base/loop.h"
#include "../app-1.h"
#include "../event.h"
#include "../display-port.h"
#include "linux-gl-1.h"
#include "ngui/base/loop.h"
#include <X11/Xlib.h>

XX_NS(ngui)

class LinuxApplication;
static LinuxApplication* application = nullptr;
static LinuxGLDrawCore* gl_draw_core = nullptr;
typedef DisplayPort::Orientation Orientation;

/**
 * @class LinuxApplication
 */
class LinuxApplication {
	typedef NonObjectTraits Traits;

	LinuxApplication()
	: m_host(nullptr)
	, m_dpy(nullptr)
	, m_root(0)
	, m_win(0)
	, m_render_looper(nullptr)
	, m_dispatch(nullptr)
	, m_current_orientation(Orientation::ORIENTATION_INVALID)
	, m_win_width(1)
	, m_win_height(1)
	{
		XX_ASSERT(!application); application = this;
		m_dpy = XOpenDisplay(nullptr);
	}

	void initialize() {

		m_host = Inl_GUIApplication(app());
		m_dispatch = m_host->dispatch();
		m_render_looper = new RenderLooper(m_host);
		m_root = XRootWindow(m_dpy, 0);

		XKeyEvent event;
		XSetWindowAttributes set;
		set.background_pixel = XBlackPixel(m_dpy, 0);
		// set.background_pixel = XWhitePixel(m_dpy, 0);
		XWindowAttributes attrs;
		XGetWindowAttributes(m_dpy, m_root, &attrs);

		if (m_options.has("width")) {
			int v = m_options["width"];
			if (v > 0) {
				attrs.width = v;
			}
		}

		if (m_options.has("height")) {
			int v = m_options["height"];
			if (v > 0) {
				attrs.height = v;
			}
		}

		m_win = XCreateWindow(
			m_dpy,
			m_root,
			0,
			0,
			attrs.width,
			attrs.height,
			0,
			DefaultDepth(m_dpy, 0),
			InputOutput,
			DefaultVisual(m_dpy, 0),
			CWBackPixel,
			&set
		);
		
		XSelectInput(m_dpy, m_win, ExposureMask | KeyPressMask); // 选择输入事件。
		XMapWindow(m_dpy, m_win); //Map 窗口

		render_loop()->post(Cb([this](Se &ev) {
			gl_draw_core->create_surface(m_win);
			gl_draw_core->initialize();
			m_host->onLoad();
			m_host->onForeground();
			m_host->onResume();
			// application->m_host->onBackground();
			// application->m_host->onPause();
			// application->m_host->onMemorywarning();
			m_render_looper->start();
		}));

		while(1) {
			XNextEvent(m_dpy,(XEvent*)&event);

			switch(event.type) {
				case Expose:
					XGetWindowAttributes(m_dpy, m_win, &attrs);
					m_win_width = attrs.width;
					m_win_height = attrs.height;
					render_loop()->post_sync(Cb([this](Se &ev) {
						CGRect rect = { Vec2(), get_window_size() };
						gl_draw_core->refresh_surface_size(&rect);
						m_host->refresh_display(); // 刷新显示
					}));
					break;
				case KeyPress:
				LOG("event, %d", event.type);
					break;
				default:
					LOG("event, %d", event.type);
					break;
			}
		}

		XCloseDisplay(m_dpy); m_dpy = nullptr;
	}

 public:

	static void start(int argc, char* argv[]) {
		/**************************************************/
		/**************************************************/
		/************** Start GUI Application *************/
		/**************************************************/
		/**************************************************/
		new LinuxApplication();
		ngui::AppInl::start(argc, argv);
		if ( app() ) {
			application->initialize();
		}
	}

	inline RunLoop* render_loop() {
		return m_host->render_loop();
	}

	inline void set_options(const Map<String, int>& options) {
		m_options = options;
	}

	inline Vec2 get_window_size() {
		return Vec2(application->m_win_width, application->m_win_height);
	}

	inline Display* get_x11_display() {
		return m_dpy;
	}

 private:
	AppInl* m_host;
	Display* m_dpy;
	Window m_root;
	Window m_win;
	RenderLooper* m_render_looper;
	GUIEventDispatch* m_dispatch;
	Orientation m_current_orientation;
	std::atomic_int m_win_width;
	std::atomic_int m_win_height;
	Map<String, int> m_options;
};

Vec2 __get_window_size() {
	return application->get_window_size();
}

Display* __get_x11_display() {
	return application->get_x11_display();
}

/**
 * @func pending() 挂起应用进程
 */
void GUIApplication::pending() {
	// exit(0);
}

/**
 * @func open_url()
 */
void GUIApplication::open_url(cString& url) {
	// TODO ...
}

/**
 * @func send_email
 */
void GUIApplication::send_email(cString& recipient,
																cString& subject,
																cString& cc, cString& bcc, cString& body) {
	// TODO ...
}

/**
 * @func initialize(options)
 */
void AppInl::initialize(const Map<String, int>& options) {
	XX_DEBUG("AppInl::initialize");
	XX_ASSERT(!gl_draw_core);
	application->set_options(options);
	gl_draw_core = LinuxGLDrawCore::create(this, options);
	m_draw_ctx = gl_draw_core->host();
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
	// TODO...
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	// TODO...
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
	// TODO...
}

/**
 * @func set_volume_up()
 */
void AppInl::set_volume_up() {
	// TODO ..
}

/**
 * @func set_volume_down()
 */
void AppInl::set_volume_down() {
	// TODO ..
}

/**
 * @func default_atom_pixel
 */
float DisplayPort::default_atom_pixel() {
	return 1.0;
}

/**
 * @func keep_screen(keep)
 */
void DisplayPort::keep_screen(bool keep) {
	// TODO ..
}

/**
 * @func status_bar_height()
 */
float DisplayPort::status_bar_height() {
	return 0;
}

/**
 * @func set_visible_status_bar(visible)
 */
void DisplayPort::set_visible_status_bar(bool visible) {
	// TODO ..
}

/**
 * @func set_status_bar_text_color(color)
 */
void DisplayPort::set_status_bar_style(StatusBarStyle style) {
	// TODO ..
}

/**
 * @func request_fullscreen(fullscreen)
 */
void DisplayPort::request_fullscreen(bool fullscreen) {
	// TODO ..
}

/**
 * @func orientation()
 */
Orientation DisplayPort::orientation() {
	return ORIENTATION_INVALID;
}

/**
 * @func set_orientation(orientation)
 */
void DisplayPort::set_orientation(Orientation orientation) {
	// TODO ..
}

XX_END

extern "C" {

	int main(int argc, char* argv[]) {
		ngui::LinuxApplication::start(argc, argv);
		return 0;
	}
	
}
