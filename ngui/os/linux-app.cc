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

#include "../base/loop.h"
#include "../app-1.h"
#include "../event.h"
#include "../display-port.h"
#include "./linux-gl-1.h"
#include "../base/loop.h"
#include <X11/Xlib.h>
#include <signal.h>

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
	, m_is_init(0)
	{
		XX_ASSERT(!application); application = this;
		m_dpy = XOpenDisplay(nullptr); 
		XX_CHECK(m_dpy, "Cannot connect to display");
		m_root = XDefaultRootWindow(m_dpy);

		XWindowAttributes attrs;
		XGetWindowAttributes(m_dpy, m_root, &attrs);
		m_win_width        = attrs.width;
		m_win_height       = attrs.height;
		m_wm_protocols     = XInternAtom(m_dpy, "WM_PROTOCOLS"    , False);
		m_wm_delete_window = XInternAtom(m_dpy, "WM_DELETE_WINDOW", False);
		
		m_xset.background_pixel = 0;
		m_xset.border_pixel = 0;
		m_xset.background_pixmap = None;
		m_xset.border_pixmap = None;
		m_xset.event_mask = (NoEventMask
			| ExposureMask
			| KeyPressMask
			| KeyRelease
			| ButtonPressMask
			| ButtonReleaseMask
			| PointerMotionMask // Motion
			| PointerMotionHintMask // Motion
			| Button1MotionMask // Motion
			| Button2MotionMask // Motion
			| Button3MotionMask // Motion
			| Button4MotionMask // Motion
			| Button5MotionMask // Motion
			| ButtonMotionMask // Motion
			| EnterWindowMask
			| LeaveWindowMask
			| KeymapStateMask // KeymapNotify
				// | VisibilityChangeMask	//
			| StructureNotifyMask 	// ConfigureNotify
				// | ResizeRedirectMask				//
				// | SubstructureNotifyMask		//
				// | SubstructureRedirectMask	//
			| FocusChangeMask // FocusIn, FocusOut
			| PropertyChangeMask // PropertyNotify
				// | ColormapChangeMask	//
				// | OwnerGrabButtonMask	//
		);
		m_xset.do_not_propagate_mask = NoEventMask;
	}

	~LinuxApplication() {
		if (m_dpy) {
			XCloseDisplay(m_dpy); m_dpy = nullptr;
		}
		application = nullptr;
		gl_draw_core = nullptr;
	}

	static void signal_handler(int signal) {
		switch(signal) {
			case SIGINT:
			case SIGTERM:
				application->destroy_app();
				break;
		}
	}

	void destroy_app() {
		m_host->onUnload();
		m_host->main_loop()->stop();
		XCloseDisplay(m_dpy); m_dpy = nullptr;  // disconnect x display
		SimpleThread::wait_end(m_main_tid); // wait main loop end
	}

	void run() {
		m_host = Inl_GUIApplication(app());
		m_dispatch = m_host->dispatch();
		m_render_looper = new RenderLooper(m_host);
		m_main_tid = m_host->main_loop()->thread_id();

		m_win = XCreateWindow(
			m_dpy,
			m_root,
			0,
			0,
			(uint)m_win_width,
			(uint)m_win_height,
			0,
			DefaultDepth(m_dpy, 0),
			InputOutput,
			DefaultVisual(m_dpy, 0),
			CWBackPixel | CWEventMask | CWBorderPixel | CWColormap,
			&m_xset
		);

		XStoreName(m_dpy, m_win, ""/*title*/);
		// XSelectInput(m_dpy, m_root, SubstructureNotifyMask);
		XMapWindow(m_dpy, m_win); //Map 窗口
		XSetWMProtocols(m_dpy, m_win, &m_wm_delete_window, True);

		signal(SIGTERM, signal_handler);
		signal(SIGINT, signal_handler);

		XEvent event;
		do {
			/*XEventsQueued(m_dpy, QueuedAfterFlush)*/
			XNextEvent(m_dpy, &event);
		} while(handle_events(event));
		
		destroy_app();
	}

	bool handle_events(XEvent& event) {

		switch(event.type) {
			case Expose: {
				XX_DEBUG("event, Expose");
				XWindowAttributes attrs;
				XGetWindowAttributes(m_dpy, m_win, &attrs);
				m_win_width = attrs.width;
				m_win_height = attrs.height;

				render_loop()->post_sync(Cb([this](Se &ev) {
					if (m_is_init) {
						CGRect rect = {Vec2(), get_window_size()};
						gl_draw_core->refresh_surface_size(&rect);
						m_host->refresh_display(); // 刷新显示
					} else {
						m_is_init = true;
						gl_draw_core->create_surface(m_win);
						gl_draw_core->initialize();
						m_host->onLoad();
						m_host->onForeground();
						m_render_looper->start();
					}
				}));
				break;
			}
			case MapNotify:
				if (m_is_init) {
					XX_DEBUG("event, MapNotify, onForeground");
					m_host->onForeground();
					m_render_looper->start();
				}
				break;
			case UnmapNotify:
				XX_DEBUG("event, UnmapNotify, onBackground");
				m_host->onBackground();
				m_render_looper->stop();
				break;
			case FocusIn:
				XX_DEBUG("event, FocusIn, onResume");
				m_host->onResume();
				break;
			case FocusOut:
				XX_DEBUG("event, FocusOut, onPause");
				m_host->onPause();
				break;
			case KeyPress:
				LOG("event, KeyPress");
				break;
			case KeyRelease:
				LOG("event, KeyRelease");
				break;
			case ButtonPress:
				LOG("event, ButtonPress, Mouse Down");
				break;
			case ButtonRelease:
				LOG("event, ButtonRelease, Mouse Up");
				break;
			case MotionNotify:
				LOG("event, MotionNotify");
				break;
			case EnterNotify:
				LOG("event, EnterNotify");
				break;
			case LeaveNotify:
				LOG("event, LeaveNotify");
				break;
			case KeymapNotify:
				LOG("event, KeymapNotify");
				break;
			case PropertyNotify:
				LOG("event, PropertyNotify");
				break;
			case ConfigureNotify:
				LOG("event, ConfigureNotify");
				break;
			case ClientMessage:
				if (event.xclient.message_type == m_wm_protocols && 
					(Atom)event.xclient.data.l[0] == m_wm_delete_window) {
					return false; // exit
				}
				break;
			default:
				LOG("event, %d", event.type);
				break;
		}

		// m_host->onMemorywarning();

		return true;
	}

 public:

	static void start(int argc, char* argv[]) {
		auto _ = new LinuxApplication();
		/**************************************************/
		/**************************************************/
		/************** Start GUI Application *************/
		/**************************************************/
		/**************************************************/
		ngui::AppInl::start(argc, argv);

		if ( app() ) {
			_->run();
		}
		delete _;
	}

	inline RunLoop* render_loop() {
		return m_host->render_loop();
	}

	inline void set_options(const Map<String, int>& options) {
		m_options = options;

		if (options.has("width")) {
			int v = options["width"];
			if (v > 0) {
				m_win_width = v;
			}
		}
		if (options.has("height")) {
			int v = options["height"];
			if (v > 0) {
				m_win_height = v;
			}
		}
		if (options.has("background")) {
			m_xset.background_pixel = options["background"];
		}

	}

	inline Vec2 get_window_size() {
		return Vec2(m_win_width, m_win_height);
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
	bool m_is_init;
	XSetWindowAttributes m_xset;
	Atom m_wm_protocols, m_wm_delete_window;
	ThreadID m_main_tid;
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

extern "C" int main(int argc, char* argv[]) {
	ngui::LinuxApplication::start(argc, argv);
	return 0;
}
