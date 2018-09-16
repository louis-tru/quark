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
	: m_window(0), m_host(nullptr)
	, m_render_looper(nullptr), m_dispatch(nullptr)
	, m_current_orientation(Orientation::ORIENTATION_INVALID)
	{
		XX_ASSERT(!application); application = this;
		m_host = Inl_GUIApplication(app());
		m_dispatch = m_host->dispatch();
		m_render_looper = new RenderLooper(m_host);
	}

 public:

	static void start(int argc, char* argv[]) {
		ngui::AppInl::start(argc, argv);
		if ( !ngui::app() ) return;
		new LinuxApplication(); // create linux application object

		// TODO ...

		Display* dpy = XOpenDisplay(nullptr); // 连接到 X Server，创建到 X Server 的套接字连接

		XSetWindowAttributes attrs;
		// attrs.background_pixel = XWhitePixel(dpy, 0);

		Window win = XCreateWindow(
			dpy,
			XRootWindow(dpy, 0),
			0,
			0,
			500,
			500,
			0,
			DefaultDepth(dpy, 0),
			InputOutput,
			DefaultVisual(dpy, 0),
			CWBackPixel,
			&attrs
		);
		
		XSelectInput(dpy, win, ExposureMask | KeyPressMask); // 选择输入事件。
		XMapWindow(dpy, win); //Map 窗口

		// 事件主循环。主要处理 Expose 事件和 KeyPress 事件
		while(1) {
			XKeyEvent event;
			XNextEvent(dpy,(XEvent*)&event);

			switch(event.type) {
				case Expose:
					XWindowAttributes attrs;
					XGetWindowAttributes(dpy, win, &attrs);
					LOG("%s,width: %d, height: %d\n", "draw", attrs.width, attrs.height);
					break;
				case KeyPress:
					XCloseDisplay(dpy);
					exit(0);
					break;
				default: break;
			}
		}

	}

 private:
	Window m_window;
	AppInl* m_host;
	RenderLooper* m_render_looper;
	GUIEventDispatch* m_dispatch;
	Orientation m_current_orientation;
};

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
		/**************************************************/
		/**************************************************/
		/************** Start GUI Application *************/
		/**************************************************/
		/**************************************************/
		ngui::LinuxApplication::start(argc, argv);
		return 0;
	}
	
}
