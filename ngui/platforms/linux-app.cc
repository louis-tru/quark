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

#include "../utils/loop.h"
#include "../app-1.h"
#include "../event.h"
#include "../display-port.h"
#include "./linux-gl-1.h"
#include "../utils/loop.h"
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <signal.h>
#include "linux-ime-helper-1.h"

XX_NS(ngui)

class LinuxApplication;
static LinuxApplication* application = nullptr;
static LinuxGLDrawCore* gl_draw_core = nullptr;
typedef DisplayPort::Orientation Orientation;

#if DEBUG
cchar* MOUSE_KEYS[] = {
	"left",
	"second (or middle)",
	"right",
	"pull_up",
	"pull_down",
	"pull_left",
	"pull_right",
};
#endif

/**
 * @class LinuxApplication
 */
class LinuxApplication {
 public:
	typedef NonObjectTraits Traits;

	LinuxApplication()
	: m_host(nullptr)
	, m_dpy(nullptr)
	, m_root(0)
	, m_win(0)
	, m_render_looper(nullptr)
	, m_dispatch(nullptr)
	, m_current_orientation(Orientation::ORIENTATION_INVALID)
	, m_r_width(0), m_r_height(0)
	, m_win_width(1), m_win_height(1)
	, m_is_init(0), m_exit(0)
	, m_xft_dpi(96.0)
	, m_xwin_scale(1.0)
	, m_ime(nullptr)
	{
		XX_ASSERT(!application); application = this;
	}

	~LinuxApplication() {
		if (m_win) {
			XDestroyWindow(m_dpy, m_win); m_win = 0;
		}
		if (m_dpy) {
			XCloseDisplay(m_dpy); m_dpy = nullptr;
		}
		if (m_ime) {
			delete m_ime; m_ime = nullptr;
		}
		application = nullptr;
		gl_draw_core = nullptr;
	}

	void post_message(cCb& cb) {
		XX_ASSERT(m_win);
		{
			ScopeLock lock(m_queue_mutex);
			m_queue.push(cb);
		}
		// XExposeEvent event;
		// event.type = Expose;
		// event.display = m_dpy;
		// event.window = m_win;
		// event.x = 0;
		// event.y = 0;
		// event.width = 1;
		// event.height = 1;
		// event.count = 0;
		// Status status = XSendEvent(m_dpy, m_win, true, ExposureMask, (XEvent*)&event);

		XCirculateEvent event;
		event.type = CirculateNotify;
		event.display = m_dpy;
		event.window = m_win;
		// event.parent = m_root;
		event.place = PlaceOnTop;
		XSendEvent(m_dpy, m_win, false, NoEventMask, (XEvent*)&event);
	}

	void run() {
		m_host = Inl_GUIApplication(app());
		m_dispatch = m_host->dispatch();
		m_render_looper = new RenderLooper(m_host);
		m_main_tid = m_host->main_loop()->thread_id();

		// create x11 window
		m_win = XCreateWindow(
			m_dpy, m_root,
			(m_r_width - m_win_width) / 2,
			(m_r_height - m_win_height) / 2,
			uint(m_win_width),
			uint(m_win_height), 0,
			DefaultDepth(m_dpy, 0),
			InputOutput,
			DefaultVisual(m_dpy, 0),
			CWBackPixel | CWEventMask | CWBorderPixel | CWColormap, &m_xset
		);

		XX_CHECK(m_win, "Cannot create XWindow");

		m_ime = new LINUXIMEHelper(m_host, m_dpy, m_win);

		// XSelectInput(m_dpy, m_win, PointerMotionMask);
		XStoreName(m_dpy, m_win, *m_title); // set window title name
		XSetWMProtocols(m_dpy, m_win, &m_wm_delete_window, True);
		XMapWindow(m_dpy, m_win); // Activate window

		XEvent event;

		do {
			XNextEvent(m_dpy, &event);

			resolved_queue(); // resolved message queue

			if (XFilterEvent(&event, None)) {
				continue;
			}

			switch(event.type) {
				case Expose:
					handle_expose(event);
					break;
				case MapNotify:
					if (m_is_init) {
						DLOG("event, MapNotify, Window onForeground");
						m_host->onForeground();
						m_render_looper->start();
					}
					break;
				case UnmapNotify:
					DLOG("event, UnmapNotify, Window onBackground");
					m_host->onBackground();
					m_render_looper->stop();
					break;
				case FocusIn:
					DLOG("event, FocusIn, Window onResume");
					m_ime->focus_in();
					m_host->onResume();
					break;
				case FocusOut:
					DLOG("event, FocusOut, Window onPause");
					m_ime->focus_out();
					m_host->onPause();
					break;
				case KeyPress:
					DLOG("event, KeyDown, keycode: %ld", event.xkey.keycode);
					m_ime->key_press(&event.xkey);
					m_dispatch->keyboard_adapter()->dispatch(event.xkey.keycode, 0, 1);
					break;
				case KeyRelease:
					DLOG("event, KeyUp, keycode: %d", event.xkey.keycode);
					m_dispatch->keyboard_adapter()->dispatch(event.xkey.keycode, 0, 0);
					break;
				case ButtonPress:
					DLOG("event, MouseDown, button: %s", MOUSE_KEYS[event.xbutton.button - 1]);
					m_dispatch->dispatch_mousepress(KeyboardKeyName(event.xbutton.button), true);
					break;
				case ButtonRelease:
					DLOG("event, MouseUp, button: %s", MOUSE_KEYS[event.xbutton.button - 1]);
					m_dispatch->dispatch_mousepress(KeyboardKeyName(event.xbutton.button), false);
					break;
				case MotionNotify: {
					Vec2 scale = m_host->display_port()->scale();
					DLOG("event, MouseMove: [%d, %d]", event.xmotion.x, event.xmotion.y);
					m_dispatch->dispatch_mousemove(event.xmotion.x / scale[0], event.xmotion.y / scale[1]);
					break;
				}
				case EnterNotify:
					DLOG("event, EnterNotify");
					break;
				case LeaveNotify:
					DLOG("event, LeaveNotify");
					break;
				case CirculateNotify:
					DLOG("event, CirculateNotify");
					break;
				case CirculateRequest:
					DLOG("event, CirculateRequest");
					break;
				case ClientMessage:
					if (event.xclient.message_type == m_wm_protocols && 
						(Atom)event.xclient.data.l[0] == m_wm_delete_window) {
						m_exit = 1; // exit
					}
					break;
				default:
					DLOG("event, %d", event.type);
					break;
			}
		} while(!m_exit);

		destroy();
	}

	inline LINUXIMEHelper* ime() {
		return m_ime;
	}

	inline float xwin_scale() {
		return m_xwin_scale;
	}

	void initialize(cJSON& options) {
		XX_CHECK(XInitThreads(), "Cannot init X threads");

		m_dpy = XOpenDisplay(nullptr);
		XX_CHECK(m_dpy, "Cannot connect to display");
		m_root = XDefaultRootWindow(m_dpy);

		XWindowAttributes attrs;
		XGetWindowAttributes(m_dpy, m_root, &attrs);
		m_win_width = m_r_width   = attrs.width;
		m_win_height = m_r_height = attrs.height;
		m_wm_protocols     = XInternAtom(m_dpy, "WM_PROTOCOLS"    , False);
		m_wm_delete_window = XInternAtom(m_dpy, "WM_DELETE_WINDOW", False);
		m_xft_dpi = get_monitor_dpi();
		m_xwin_scale = m_xft_dpi / 96.0;

		DLOG("width: %d, height: %d, dpi scale: %f", m_r_width, m_r_height, m_xwin_scale);
		
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
			| PointerMotionMask // MotionNotify
			| Button1MotionMask // Motion
			| Button2MotionMask // Motion
			| Button3MotionMask // Motion
			| Button4MotionMask // Motion
			| Button5MotionMask // Motion
			| ButtonMotionMask  // Motion
			| EnterWindowMask   // EnterNotify
			| LeaveWindowMask   // LeaveNotify
			| FocusChangeMask   // FocusIn, FocusOut
		);
		m_xset.do_not_propagate_mask = NoEventMask;

		cJSON& o_w = options["width"];
		cJSON& o_h = options["height"];
		cJSON& o_b = options["background"];
		cJSON& o_t = options["title"];

		if (o_w.is_uint()) m_win_width = XX_MAX(1, o_w.to_uint()) * m_xwin_scale;
		if (o_h.is_uint()) m_win_height = XX_MAX(1, o_h.to_uint()) * m_xwin_scale;
		if (o_w.is_uint()) m_xset.background_pixel = o_b.to_uint();
		if (o_t.is_string()) m_title = o_t.to_string();
	}

	inline Vec2 get_window_size() {
		return Vec2(m_win_width, m_win_height);
	}

	inline Display* get_x11_display() {
		return m_dpy;
	}

 private:

	void resolved_queue() {
		List<Callback> queue;
		{
			ScopeLock lock(m_queue_mutex);
			if (m_queue.length()) {
				queue = move(m_queue);
			}
		}
		if (queue.length()) {
			for (auto& i: queue) {
				SimpleEvent data = { 0, m_host, 0 };
				i.value()->call(data);
			}
		}
	}

	void handle_expose(XEvent& event) {
		DLOG("event, Expose");
		XWindowAttributes attrs;
		XGetWindowAttributes(m_dpy, m_win, &attrs);

		m_win_width = attrs.width;
		m_win_height = attrs.height;

		m_host->render_loop()->post_sync(Cb([this](Se &ev) {
			if (m_is_init) {
				CGRect rect = {Vec2(), get_window_size()};
				gl_draw_core->refresh_surface_size(&rect);
				m_host->refresh_display(); // 刷新显示
			} else {
				m_is_init = 1;
				gl_draw_core->create_surface(m_win);
				gl_draw_core->initialize();
				m_host->onLoad();
				m_host->onForeground();
				m_render_looper->start();
			}
		}));
	}

	void destroy() {
		m_render_looper->stop();
		m_host->exit();
		SimpleThread::wait_end(m_main_tid); // wait main loop end
		XDestroyWindow(m_dpy, m_win); m_win = 0;
		XCloseDisplay(m_dpy); m_dpy = nullptr;  // disconnect x display
	}

	float get_monitor_dpi() {
		char* ms = XResourceManagerString(m_dpy);
		float dpi = 96.0;
		if (ms) {
			DLOG("Entire DB:\n%s", ms);
			XrmDatabase db = XrmGetStringDatabase(ms);
			XrmValue value;
			char* type = nullptr;
			if (XrmGetResource(db, "Xft.dpi", "String", &type, &value) == True) {
				if (value.addr) {
					dpi = atof(value.addr);
				}
			}
		}
		DLOG("DPI: %f", dpi);
		return dpi;
	}

	AppInl* m_host;
	Display* m_dpy;
	Window m_root, m_win;
	RenderLooper* m_render_looper;
	GUIEventDispatch* m_dispatch;
	Orientation m_current_orientation;
	int m_r_width, m_r_height;
	std::atomic_int m_win_width, m_win_height;
	bool m_is_init, m_exit;
	float m_xft_dpi, m_xwin_scale;
	XSetWindowAttributes m_xset;
	Atom m_wm_protocols, m_wm_delete_window;
	ThreadID m_main_tid;
	String m_title;
	LINUXIMEHelper* m_ime;
	List<Callback> m_queue;
	Mutex m_queue_mutex;
};

Vec2 __get_window_size() {
	XX_CHECK(application);
	return application->get_window_size();
}

Display* __get_x11_display() {
	XX_CHECK(application);
	return application->get_x11_display();
}

// sync to x11 main message loop
void __dispatch_x11_async(cCb& cb) {
	XX_ASSERT(application);
	return application->post_message(cb);
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
void AppInl::initialize(cJSON& options) {
	DLOG("AppInl::initialize");
	XX_ASSERT(!gl_draw_core);
	application->initialize(options);
	gl_draw_core = LinuxGLDrawCore::create(this, options);
	m_draw_ctx = gl_draw_core->host();
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
	application->ime()->open(options);
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	application->ime()->set_keyboard_can_backspace(can_backspace, can_delete);
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
	application->ime()->close();
}

/**
 * @func ime_keyboard_spot_location
 */
void AppInl::ime_keyboard_spot_location(Vec2 location) {
	application->ime()->set_spot_location(location);
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
	return 1.0 / application->xwin_scale();
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
 * @func default_status_bar_height
 */
float DisplayPort::default_status_bar_height() {
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

using namespace ngui;

extern "C" int main(int argc, char* argv[]) {
	Handle<LinuxApplication> h = new LinuxApplication();

	/**************************************************/
	/**************************************************/
	/************** Start GUI Application *************/
	/**************************************************/
	/**************************************************/
	AppInl::start(argc, argv);

	if ( app() ) {
		h->run();
	}

	return 0;
}
