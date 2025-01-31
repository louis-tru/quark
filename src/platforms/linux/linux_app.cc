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

#include "quark/util/loop.h"
#include "quark/app.h"
#include "quark/event.h"
#include "quark/display.h"
#include "quark/util/loop.h"
#include "quark/util/http.h"
#include "linux_gl.h"
#include "linux_ime_helper.h"
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>
#include <signal.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

namespace qk {

	class UnixApplication;
	static UnixApplication* application = nullptr;
	static GLDrawProxy* gl_draw_context = nullptr;
	typedef Display::Orientation Orientation;

	#if DEBUG
		cChar* MOUSE_KEYS[] = {
			"left",
			"second (or middle)",
			"right",
			"pull_up",
			"pull_down",
			"pull_left",
			"pull_right",
		};
	#endif

	void exit(int rc, bool forceExit);

	/**
	* @class UnixApplication
	*/
	class UnixApplication {
	public:
		typedef NonObjectTraits Traits;

		UnixApplication()
		: _host(nullptr)
		, _dpy(nullptr)
		, _root(0)
		, _win(0)
		, _render_looper(nullptr)
		, _dispatch(nullptr)
		, _current_orientation(Orientation::ORIENTATION_INVALID)
		, _screen(0), _s_width(0), _s_height(0)
		, _x(0), _y(0), _width(1), _height(1)
		, _w_width(1), _w_height(1)
		, _is_init(0), _exit(0)
		, _xft_dpi(96.0)
		, _xwin_scale(1.0)
		, _main_loop(0)
		, _ime(nullptr)
		, _mixer(nullptr)
		, _multitouch_device(nullptr)
		, _element(nullptr)
		, _JSONfullscreen(0)
		{
			Qk_ASSERT(!application); application = this;
		}

		~UnixApplication() {
			if (_mixer) {
				snd_mixer_free(_mixer);
				snd_mixer_detach(_mixer, "default");
				snd_mixer_close(_mixer);
				_mixer = nullptr;
				_element = nullptr;
			}
			if (_win) {
				XDestroyWindow(_dpy, _win); _win = 0;
			}
			if (_dpy) {
				XCloseDisplay(_dpy); _dpy = nullptr;
			}
			if (_ime) {
				delete _ime; _ime = nullptr;
			}
			application = nullptr;
			gl_draw_context = nullptr;
		}

		void post_message(cCb& cb) {
			Qk_ASSERT(_win);
			{
				ScopeLock lock(_queue_mutex);
				_queue.push(cb);
			}
			// XExposeEvent event;
			// event.type = Expose;
			// event.display = _dpy;
			// event.window = _win;
			// event.x = 0;
			// event.y = 0;
			// event.width = 1;
			// event.height = 1;
			// event.count = 0;
			// Status status = XSendEvent(_dpy, _win, true, ExposureMask, (XEvent*)&event);

			XCirculateEvent event;
			event.type = CirculateNotify;
			event.display = _dpy;
			event.window = _win;
			event.place = PlaceOnTop;
			XSendEvent(_dpy, _win, false, NoEventMask, (XEvent*)&event);
		}

		/**
		* create x11 window
		*/
		Window create_xwindow () {

			_xset.event_mask = NoEventMask
				| KeyPressMask
				| KeyReleaseMask
				| EnterWindowMask   // EnterNotify
				| LeaveWindowMask   // LeaveNotify
				| KeymapStateMask
				| ExposureMask
				| FocusChangeMask   // FocusIn, FocusOut
			;

			if (!_multitouch_device) {
				_xset.event_mask |= NoEventMask
					| ButtonPressMask
					| ButtonReleaseMask
					| PointerMotionMask // MotionNotify
					| Button1MotionMask // Motion
					| Button2MotionMask // Motion
					| Button3MotionMask // Motion
					| Button4MotionMask // Motion
					| Button5MotionMask // Motion
					| ButtonMotionMask  // Motion
				;
			}

			Qk_DLog("XCreateWindow, x:%d, y:%d, w:%d, h:%d", _x, _y, _width, _height);

			Window win = XCreateWindow(
				_dpy, _root,
				_x, _y,
				_width, _height, 0,
				DefaultDepth(_dpy, 0),
				InputOutput,
				DefaultVisual(_dpy, 0),
				CWBackPixel | CWEventMask | CWBorderPixel | CWColormap, &_xset
			);

			// Qk_DLog("_xset.background_pixel 3, %d", _xset.background_pixel);

			Qk_ASSERT(win, "Cannot create XWindow");

			if (_multitouch_device) {
				Qk_DLog("_multitouch_device");

				XIEventMask eventmask;
				uint8_t mask[3] = { 0,0,0 };

				eventmask.deviceid = XIAllMasterDevices;
				eventmask.mask_len = sizeof(mask);
				eventmask.mask = mask;

				XISetMask(mask, XI_TouchBegin);
				XISetMask(mask, XI_TouchUpdate);
				XISetMask(mask, XI_TouchEnd);

				XISelectEvents(_dpy, win, &eventmask, 1);
			}

			XStoreName(_dpy, win, *_title); // set window title name
			XSetWMProtocols(_dpy, win, &_wm_delete_window, True);

			return win;
		}

		void run() {
			_host = Inl_Application(shared_app());
			_dispatch = _host->dispatch();
			_render_looper = new RenderLooper(_host);
			_main_loop = _host->main_loop();
			_win = create_xwindow();
			_ime = new UnixIMEHelper(_host, _dpy, _win);

			XMapWindow(_dpy, _win); // Activate window

			if (_is_fullscreen) {
				request_fullscreen(true);
			}

			XEvent event;

			// run message main_loop
			do {
				XNextEvent(_dpy, &event);

				resolved_queue(); // resolved message queue

				if (XFilterEvent(&event, None)) continue;

				switch (event.type) {
					case Expose:
						handle_expose(event);
						break;
					case MapNotify:
						if (_is_init) {
							Qk_DLog("event, MapNotify, Window onForeground");
							_host->triggerForeground();
							_render_looper->start();
						}
						break;
					case UnmapNotify:
						Qk_DLog("event, UnmapNotify, Window onBackground");
						_host->triggerBackground();
						_render_looper->stop();
						break;
					case FocusIn:
						Qk_DLog("event, FocusIn, Window onResume");
						_ime->focus_in();
						_host->triggerResume();
						break;
					case FocusOut:
						Qk_DLog("event, FocusOut, Window onPause");
						_ime->focus_out();
						_host->triggerPause();
						break;
					case KeyPress:
						Qk_DLog("event, KeyDown, keycode: %ld", event.xkey.keycode);
						_ime->key_press(&event.xkey);
						_dispatch->keyboard_adapter()->dispatch(event.xkey.keycode, 0, 1);
						break;
					case KeyRelease:
						Qk_DLog("event, KeyUp, keycode: %d", event.xkey.keycode);
						_dispatch->keyboard_adapter()->dispatch(event.xkey.keycode, 0, 0);
						break;
					case ButtonPress:
						Qk_DLog("event, MouseDown, button: %s", MOUSE_KEYS[event.xbutton.button - 1]);
						_dispatch->dispatch_mousepress(KeyboardKeyCode(event.xbutton.button), true);
						break;
					case ButtonRelease:
						Qk_DLog("event, MouseUp, button: %s", MOUSE_KEYS[event.xbutton.button - 1]);
						_dispatch->dispatch_mousepress(KeyboardKeyCode(event.xbutton.button), false);
						break;
					case MotionNotify: {
						Vec2 scale = _host->display_port()->scale();
						Qk_DLog("event, MouseMove: [%d, %d]", event.xmotion.x, event.xmotion.y);
						_dispatch->dispatch_mousemove(event.xmotion.x / scale[0], event.xmotion.y / scale[1]);
						break;
					}
					case ClientMessage:
						if (event.xclient.message_type == _wm_protocols && 
							(Atom)event.xclient.data.l[0] == _wm_delete_window) {
							_exit = 1; // exit
						}
						break;
					case GenericEvent:
						/* event is a union, so cookie == &event, but this is type safe. */
						if (XGetEventData(_dpy, &event.xcookie)) {
							XHandleGenericEvent(&event.xcookie);
							XFreeEventData(_dpy, &event.xcookie);
						}
						break;
					default:
						Qk_DLog("event, %d", event.type);
						break;
				}
			} while(!_exit);

			destroy();
		}

		void XHandleGenericEvent(XGenericEventCookie* cookie) {

			XIDeviceEvent* xev = (XIDeviceEvent*)cookie->data;
			Vec2 scale = _host->display_port()->scale();

			List<UITouch> touchs = {
				{
					uint(xev->detail + 20170820),
					0, 0,
					float(xev->event_x / scale.x()),
					float(xev->event_y / scale.y()),
					0,
					false,
					nullptr,
				}
			};

			switch(cookie->evtype) {
				case XI_TouchBegin:
					Qk_DLog("event, XI_TouchBegin, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f", 
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					_dispatch->dispatch_touchstart( move(touchs) );
					break;
				case XI_TouchEnd:
					Qk_DLog("event, XI_TouchEnd, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f", 
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					_dispatch->dispatch_touchend( move(touchs) );
					break;
				case XI_TouchUpdate:
					Qk_DLog("event, XI_TouchUpdate, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f", 
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					_dispatch->dispatch_touchmove( move(touchs) );
					break;
			}
		}

		void handle_expose(XEvent& event) {
			Qk_DLog("event, Expose");
			XWindowAttributes attrs;
			XGetWindowAttributes(_dpy, _win, &attrs);

			_w_width = attrs.width;
			_w_height = attrs.height;

			typedef Callback<RunLoop::PostSyncData> Cb;
			_host->render_loop()->post_sync(Cb([this](Cb::Data &ev) {
				if (_is_init) {
					Rect rect = {Vec2(), get_window_size()};
					gl_draw_context->refresh_surface_size(&rect);
					_host->display()->render_frame(); // 刷新显示
				} else {
					_is_init = 1;
					Qk_ASSERT(gl_draw_context->create_surface(_win));
					gl_draw_context->initialize();
					_host->triggerLoad();
					_host->triggerForeground();
					_render_looper->start();
				}
				ev.data->complete();
			}));
		}

		inline UnixIMEHelper* ime() {
			return _ime;
		}

		void initialize_display() {
			if (!_dpy) {
				_dpy = XOpenDisplay(nullptr);
				Qk_ASSERT(_dpy, "Error: Can't open display");
				_xft_dpi = get_monitor_dpi();
				_xwin_scale = _xft_dpi / 96.0;
			}
		}

		inline float xwin_scale() {
			initialize_display(); // init display
			return _xwin_scale;
		}

		void initialize(cJSON& options) {
			Qk_ASSERT(XInitThreads(), "Error: Can't init X threads");

			cJSON& o_x = options["x"];
			cJSON& o_y = options["y"];
			cJSON& o_w = options["width"];
			cJSON& o_h = options["height"];
			cJSON& o_b = options["background"];
			cJSON& o_t = options["title"];
			cJSON& o_fc = options["fullScreen"];
			cJSON& o_et = options["enableTouch"];
			
			int is_enable_touch = 0;

			if (o_t.is_string()) _title = o_t.to_string();
			if (o_fc.is_bool()) _is_fullscreen = o_fc.to_bool();
			if (o_fc.is_int()) _is_fullscreen = o_fc.to_int();
			if (o_et.is_bool()) is_enable_touch = o_et.to_bool();
			if (o_et.is_int()) is_enable_touch = o_et.to_int();

			initialize_display(); // init display

			_root = XDefaultRootWindow(_dpy);
			_screen = DefaultScreen(_dpy);

			_w_width = _width = _s_width   = XDisplayWidth(_dpy, _screen);
			_w_height = _height = _s_height = XDisplayHeight(_dpy, _screen);
			_wm_protocols     = XInternAtom(_dpy, "WM_PROTOCOLS"    , False);
			_wm_delete_window = XInternAtom(_dpy, "WM_DELETE_WINDOW", False);

			APP, "screen width: %d, height: %d, dpi scale: %f", _s_width, _s_height, _xwin_scale);
			
			_xset.background_pixel = 0;
			_xset.border_pixel = 0;
			_xset.background_pixmap = None;
			_xset.border_pixmap = None;
			_xset.event_mask = NoEventMask;
			_xset.do_not_propagate_mask = NoEventMask;

			// APP, "_xset.background_pixel 1, %d", _xset.background_pixel);

			if (o_b.is_uint()) _xset.background_pixel = o_b.to_uint();
			if (o_w.is_uint()) _width = Qk_Max(1, o_w.to_uint()) * _xwin_scale;
			if (o_h.is_uint()) _height = Qk_Max(1, o_h.to_uint()) * _xwin_scale;
			if (o_x.is_uint()) _x = o_x.to_uint() * _xwin_scale; else _x = (_s_width - _width) / 2;
			if (o_y.is_uint()) _y = o_y.to_uint() * _xwin_scale; else _y = (_s_height - _height) / 2;

			// Qk_DLog("_xset.background_pixel 2, %d", _xset.background_pixel);

			if (is_enable_touch)
				initialize_multitouch();

			initialize_master_volume_control();
		}

		void initialize_multitouch() {
			Qk_ASSERT(!_multitouch_device);

			Atom touchAtom = XInternAtom(_dpy, "TOUCHSCREEN", true);
			if (touchAtom == None) {
				touchAtom = XInternAtom(_dpy, XI_TOUCHSCREEN, false);
				if (touchAtom == None) return;
			}

			int inputDeviceCount = 0;
			XDeviceInfo* devices = XListInputDevices(_dpy, &inputDeviceCount);
			XDeviceInfo* touchInfo = nullptr;

			for (int i = 0; i < inputDeviceCount; i++) {
				if (devices[i].type == touchAtom) {
					touchInfo = devices + i;
					break;
				}
			}
			if (!touchInfo) {
				return;
			}

			_multitouch_device =  XOpenDevice(_dpy, touchInfo->id);
			if (!_multitouch_device)
				return;

			Qk_DLog("X11 Touch enable active for device «%s»", touchInfo->name);

			Atom enabledAtom = XInternAtom(_dpy, "Device Enabled", false);

			uint8_t enabled = 1;
			XChangeDeviceProperty(_dpy, _multitouch_device,
				enabledAtom, XA_INTEGER, 8, PropModeReplace, &enabled, 1);
		}

		int get_master_volume() {
			long volume = 0;
			if (_element) {
				snd_mixer_selem_get_playback_volume(_element, SND_MIXER_SCHN_FRONT_LEFT, &volume);
			}
			return volume;
		}

		void set_master_volume(int volume) {
			volume = Qk_Max(0, volume);
			volume = Qk_Min(100, volume);

			const snd_mixer_selem_channel_id_t chs[] = {
				SND_MIXER_SCHN_FRONT_LEFT,
				SND_MIXER_SCHN_FRONT_RIGHT,
				SND_MIXER_SCHN_REAR_LEFT,
				SND_MIXER_SCHN_REAR_RIGHT,
				SND_MIXER_SCHN_FRONT_CENTER,
				SND_MIXER_SCHN_WOOFER,
				SND_MIXER_SCHN_SIDE_LEFT,
				SND_MIXER_SCHN_SIDE_RIGHT,
				SND_MIXER_SCHN_REAR_CENTER,
			};
			int len = sizeof(snd_mixer_selem_channel_id_t) / sizeof(chs);

			for (int i = 0; i < len; i++) {
				snd_mixer_selem_set_playback_volume(_element, chs[i], volume);
			}
		}

		void request_fullscreen(bool fullscreen) {
			int mask = CWBackPixel | CWEventMask | CWBorderPixel | CWColormap;
			int x, y, width, height;
			if (fullscreen) {
				XWindowAttributes attrs;
				XGetWindowAttributes(_dpy, _win, &attrs);
				_x = attrs.x;
				_y = attrs.y;
				x = 0; y = 0;
				_width = _w_width;
				_height = _w_height;
				width = XDisplayWidth(_dpy, _screen);
				height = XDisplayHeight(_dpy, _screen);
				mask |= CWOverrideRedirect;
				_xset.override_redirect = True;
			} else {
				x = _x; y = _y;
				width = _width;
				height = _height;
				_xset.override_redirect = False;
			}
			XChangeWindowAttributes(_dpy, _win, mask, &_xset);
			XMoveResizeWindow(_dpy, _win, x, y, width, height);
		}

		inline Vec2 get_window_size() {
			return Vec2(_w_width, _w_height);
		}

		inline Display* get_x11_display() {
			return _dpy;
		}

		void initialize_master_volume_control() {
			Qk_ASSERT(!_mixer);
			snd_mixer_open(&_mixer, 0);
			snd_mixer_attach(_mixer, "default");
			snd_mixer_selem_register(_mixer, NULL, NULL);
			snd_mixer_load(_mixer);

			/* 取得第一個 element，也就是 Master */
			_element = snd_mixer_first_elem(_mixer); Qk_DLog("element,%p", _element);

			/* 設定音量的範圍 0 ~ 100 */
			if (_element) {
				snd_mixer_selem_set_playback_volume_range(_element, 0, 100);
			}
		}

		void resolved_queue() {
			List<Cb> queue;
			{
				ScopeLock lock(_queue_mutex);
				if (_queue.length()) {
					queue = move(_queue);
				}
			}
			if (queue.length()) {
				for (auto& i: queue) {
					Cb::Data data = { 0, _host, 0 };
					i.value()->call(data);
				}
			}
		}

		void destroy() {
			_render_looper->stop();
			thread_exit(0);
			XDestroyWindow(_dpy, _win); _win = 0;
			XCloseDisplay(_dpy); _dpy = nullptr; // disconnect x display
			Qk_DLog("UnixApplication Exit");
		}

		float get_monitor_dpi() {
			Char* ms = XResourceManagerString(_dpy);
			float dpi = 96.0;
			if (ms) {
				Qk_DLog("Entire DB:\n%s", ms);
				XrmDatabase db = XrmGetStringDatabase(ms);
				XrmValue value;
				Char* type = nullptr;
				if (XrmGetResource(db, "Xft.dpi", "String", &type, &value) == True) {
					if (value.addr) {
						dpi = atof(value.addr);
					}
				}
			}
			Qk_DLog("DPI: %f", dpi);
			return dpi;
		}

	private:
		// @methods data:
		AppInl* _host;
		Display* _dpy;
		Window _root, _win;
		RenderLooper* _render_looper;
		EventDispatch* _dispatch;
		Orientation _current_orientation;
		int _screen, _s_width, _s_height;
		int _x, _y;
		uint32_t _width, _height;
		std::atomic_int _w_width, _w_height;
		bool _is_init, _exit;
		float _xft_dpi, _xwin_scale;
		XSetWindowAttributes _xset;
		Atom _wm_protocols, _wm_delete_window;
		RunLoop* _main_loop;
		String _title;
		UnixIMEHelper* _ime;
		List<Cb> _queue;
		Mutex _queue_mutex;
		snd_mixer_t* _mixer;
		XDevice* _multitouch_device;
		snd_mixer_elem_t* _element;
		bool _is_fullscreen;
	};

	Vec2 __get_window_size() {
		Qk_ASSERT(application);
		return application->get_window_size();
	}

	Display* __get_x11_display() {
		Qk_ASSERT(application);
		return application->get_x11_display();
	}

	// sync to x11 main message loop
	void __dispatch_x11_async(cCb& cb) {
		Qk_ASSERT(application);
		return application->post_message(cb);
	}

	/**
	* @func pending() 挂起应用进程
	*/
	void Application::pending() {
		// exit(0);
	}

	/**
	* @func open_url()
	*/
	void Application::open_url(cString& url) {
		if (vfork() == 0) {
			execlp("xdg-open", "xdg-open", *url, NULL);
		}
	}

	/**
	* @func send_email
	*/
	void Application::send_email(cString& recipient,
																	cString& subject,
																	cString& cc, cString& bcc, cString& body) 
	{
		String arg = "mailto:";

		arg += recipient + '?';

		if (!cc.is_empty()) {
			arg += "cc=";
			arg += cc + '&';
		}
		if (!bcc.is_empty()) {
			arg += "bcc=";
			arg += bcc + '&';
		}

		arg += "subject=";
		arg += URI::encode(subject) + '&';

		if (!body.is_empty()) {
			arg += "body=";
			arg += URI::encode(body);
		}

		// xdg-open
		// mailto:haorooms@126.com?cc=name2@rapidtables.com&bcc=name3@rapidtables.com&
		// subject=The%20subject%20of%20the%20email&body=The%20body%20of%20the%20email

		if (vfork() == 0) {
			execlp("xdg-open", "xdg-open", *arg, NULL);
		}
	}

	/**
	* @func initialize(options)
	*/
	void AppInl::initialize(cJSON& options) {
		Qk_DLog("AppInl::initialize");
		Qk_ASSERT(!gl_draw_context);
		application->initialize(options);
		gl_draw_context = GLDrawProxy::create(this, options);
		_draw_ctx = gl_draw_context->host();
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
		application->set_master_volume(application->get_master_volume() + 1);
	}

	/**
	* @func set_volume_down()
	*/
	void AppInl::set_volume_down() {
		application->set_master_volume(application->get_master_volume() - 1);
	}

	/**
	* @func default_atom_pixel
	*/
	float Display::default_atom_pixel() {
		// Qk_Log());
		return 1.0 / application->xwin_scale();
	}

	/**
	* @func keep_screen(keep)
	*/
	void Display::keep_screen(bool keep) {
		// TODO ..
	}

	/**
	* @func status_bar_height()
	*/
	float Display::status_bar_height() {
		return 0;
	}

	/**
	* @func default_status_bar_height
	*/
	float Display::default_status_bar_height() {
		return 0;
	}

	/**
	* @func set_visible_status_bar(visible)
	*/
	void Display::set_visible_status_bar(bool visible) {
	}

	/**
	* @func set_status_bar_text_color(color)
	*/
	void Display::set_status_bar_style(StatusBarStyle style) {
	}

	/**
	* @func request_fullscreen(fullscreen)
	*/
	void Display::request_fullscreen(bool fullscreen) {
		__dispatch_x11_async(Cb([fullscreen](Cb::Data& e) {
			application->request_fullscreen(fullscreen);
		}));
	}

	/**
	* @func orientation()
	*/
	Orientation Display::orientation() {
		return ORIENTATION_INVALID;
	}

	/**
	* @func set_orientation(orientation)
	*/
	void Display::set_orientation(Orientation orientation) {
	}

}

using namespace qk;

extern "C" Qk_Export int main(int argc, Char* argv[]) {
	Handle<UnixApplication> h = new UnixApplication();

	/**************************************************/
	/**************************************************/
	/************** Start UI Application *************/
	/**************************************************/
	/**************************************************/
	AppInl::runMain(argc, argv);

	if ( shared_app() ) {
		h->run();
	}

	return 0;
}
