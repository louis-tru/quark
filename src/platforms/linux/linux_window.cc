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

#include <X11/Xresource.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/cursorfont.h>
#include <EGL/eglplatform.h>
#define XNone 0
#undef Status
#undef Bool
#undef None

#include "./linux_app.h"
#include "../../render/linux/linux_render.h"
#include "../../ui/window.h"

namespace qk {
	typedef const Window::Options cOptions;

	static void closeXDisplay(XDisplay* dpy){ XCloseDisplay(dpy); }

	typedef Sp<XDisplay, object_traits_from<XDisplay, closeXDisplay>> XDisplayAuto;

	struct XInit {
		XDisplayAuto xdpy;
		XDevice* xdevice;
		float xdpyDpi;
	};

	static XDevice* openXDevice(XDisplay *xdpy) {
		Atom touchAtom = XInternAtom(xdpy, "TOUCHSCREEN", true);
		if (touchAtom == XNone) {
			touchAtom = XInternAtom(xdpy, XI_TOUCHSCREEN, false);
			if (touchAtom == XNone) return nullptr;
		}

		int inputDeviceCount = 0;
		XDeviceInfo* devices = XListInputDevices(xdpy, &inputDeviceCount);
		XDeviceInfo* touchInfo = nullptr;

		for (int i = 0; i < inputDeviceCount; i++) {
			if (devices[i].type == touchAtom) {
				touchInfo = devices + i;
				break;
			}
		}
		if (!touchInfo) {
			return nullptr;
		}

		auto xdevice = XOpenDevice(xdpy, touchInfo->id);
		if (!xdevice)
			return nullptr;

		Qk_DLog("X11 Touch enable active for device «%s»", touchInfo->name);

		Atom enabledAtom = XInternAtom(xdpy, "Device Enabled", false);

		uint8_t enabled = 1;
		XChangeDeviceProperty(xdpy, xdevice,
			enabledAtom, XA_INTEGER, 8, PropModeReplace, &enabled, 1);

		return xdevice;
	}

	static float getXDisplayDpi(Display* xdpy) {
		char* ms = XResourceManagerString(xdpy);
		float dpi = 96.0;
		if (ms) {
			Qk_DLog("Entire DB:\n%s", ms);
			XrmDatabase db = XrmGetStringDatabase(ms);
			XrmValue value;
			char* type = nullptr;
			if (XrmGetResource(db, "Xft.dpi", "String", &type, &value) == True) {
				if (value.addr) {
					dpi = atof(value.addr);
				}
			}
		}
		Qk_DLog("DPI: %f", dpi);
		return dpi;
	}

	static XInit& xinit() {
		static XInit x([](){
			Qk_ASSERT(XInitThreads(), "Error: Can't init X threads");
			auto xdpy = XOpenDisplay(nullptr);
			Qk_ASSERT_RAW(xdpy, "Can't open display");
			return XInit{
				xdpy, openXDevice(xdpy), getXDisplayDpi(xdpy)
			};
		}());
		return x;
	}

	XDisplay* openXDisplay() {
		return xinit().xdpy.get();
	}

	float dpiForXDisplay() {
		return xinit().xdpyDpi;
	}

	void addImplToGlobal(XWindow xwin, WindowImpl* win);
	void deleteImplFromGlobal(XWindow xwin);

	class WindowPlatform: public WindowImpl {
	public:
		float _xft_dpi, _xwin_scale;
		XSetWindowAttributes _xset;
		XWindowAttributes _attrs;
		XWindow _root;
		Cursor _noneCursor;

		WindowPlatform(Window* win, cOptions &opts) {
			_win = win;
			_xdpy = openXDisplay();
			_root = XDefaultRootWindow(_xdpy);
			_xft_dpi = dpiForXDisplay();
			_xwin_scale = _xft_dpi / 96.0;
			_xwin = newXWindow(opts);
			_ime = LinuxIMEHelper::Make(this);
			_noneCursor = XNone;

			addImplToGlobal(_xwin, this);
		}

		~WindowPlatform() {
			Qk_ASSERT(_xwin);
			deleteImplFromGlobal(_xwin);
			XDestroyWindow(_xdpy, _xwin); _xwin = 0;
			if (_ime) {
				delete _ime; _ime = nullptr;
			}
			if (_noneCursor != XNone) {
				XFreeCursor(_xdpy, _noneCursor);
			}
		}

		XWindow newXWindow(cOptions &opts) {
			_xset.background_pixel = opts.backgroundColor.to_uint32();
			_xset.background_pixmap = 0;
			_xset.border_pixel = 0;
			_xset.border_pixmap = XNone;
			_xset.event_mask = NoEventMask;
			_xset.do_not_propagate_mask = NoEventMask;
			_xset.override_redirect = False;

			_xset.event_mask = NoEventMask
				| KeyPressMask      // KeyPress
				| KeyReleaseMask    // KeyRelease
				| EnterWindowMask   // EnterNotify
				| LeaveWindowMask   // LeaveNotify
				| KeymapStateMask   // MapNotify
				| ExposureMask      // Expose
				| FocusChangeMask   // FocusIn, FocusOut
				| VisibilityChangeMask
				| StructureNotifyMask
			;

			if (!xinit().xdevice) { // It's not a multipoint device
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

			auto select = [](int a, int b){ return a > 0 ? a: b; };
			auto screen = DefaultScreen(_xdpy);
			int xdpyW = XDisplayWidth(_xdpy, screen);
			int xdpyH = XDisplayHeight(_xdpy, screen);
			int width = select(opts.frame.size[0] * _xwin_scale, xdpyW / 2);
			int height = select(opts.frame.size[1] * _xwin_scale, xdpyH / 2);

			auto xwin = XCreateWindow(
				_xdpy, _root,
				select(opts.frame.origin[0] * _xwin_scale, (xdpyW - width) / 2),
				select(opts.frame.origin[1] * _xwin_scale, (xdpyH - height) / 2), width, height, 0,
				DefaultDepth(_xdpy, 0),
				InputOutput,
				DefaultVisual(_xdpy, 0),
				CWBackPixel | CWEventMask | CWBorderPixel | CWColormap, &_xset
			);

			// Qk_DLog("_xset.background_pixel, %d", _xset.background_pixel);

			Qk_ASSERT_RAW(xwin, "Cannot create XWindow");

			if (xinit().xdevice) { // It's a Multipoint device
				XIEventMask eventmask;
				uint8_t mask[3] = { 0,0,0 };

				eventmask.deviceid = XIAllMasterDevices;
				eventmask.mask_len = sizeof(mask);
				eventmask.mask = mask;

				XISetMask(mask, XI_TouchBegin);
				XISetMask(mask, XI_TouchUpdate);
				XISetMask(mask, XI_TouchEnd);

				XISelectEvents(_xdpy, xwin, &eventmask, 1);
			}

			auto wmDeleteWindow = XInternAtom(_xdpy, "WM_DELETE_WINDOW", False);

			if (!opts.title.isEmpty()) {
				XStoreName(_xdpy, xwin, opts.title.c_str());
			}
			XSetWMProtocols(_xdpy, xwin, &wmDeleteWindow, True);
			XGetWindowAttributes(_xdpy, xwin, &_attrs);

			return xwin;
		}

		void setXwindowAttributes() {
			int mask = CWBackPixel | CWEventMask | CWBorderPixel | CWColormap;
			if (_xset.override_redirect == True) {
				XChangeWindowAttributes(_xdpy, _xwin, mask | CWOverrideRedirect, &_xset);
			} else {
				XChangeWindowAttributes(_xdpy, _xwin, mask, &_xset);
			}
		}

		void setBackgroundColor(Color val) {
			_xset.background_pixel = val.to_uint32();
			setXwindowAttributes();
		}

		void setFullscreen(bool fullscreen) {
			if (fullscreen) {
				if (_xset.override_redirect == False) {
					auto screen = DefaultScreen(_xdpy);
					_xset.override_redirect = True;
					XGetWindowAttributes(_xdpy, _xwin, &_attrs); // save attrs
					setXwindowAttributes();
					XMoveResizeWindow(_xdpy, _xwin, _attrs.x, _attrs.y,
						XDisplayWidth(_xdpy, screen), XDisplayHeight(_xdpy, screen)
					);
				}
			} else {
				if (_xset.override_redirect == True) {
					_xset.override_redirect = False;
					setXwindowAttributes();
					XMoveResizeWindow(_xdpy, _xwin, _attrs.x, _attrs.y, _attrs.width, _attrs.height);
				}
			}
		}

		void setCursor(CursorStyle cursor) {

			auto hideCursor = [&]() {
				if (_noneCursor == XNone) {
					static char noData[] = { 0,0,0,0,0,0,0,0 };
					XColor black = { .red = 0, .green = 0, .blue = 0 };
					Pixmap bitmap = XCreateBitmapFromData(_xdpy, _xwin, noData, 1, 1);
					_noneCursor = XCreatePixmapCursor(_xdpy, bitmap, bitmap, &black, &black, 0, 0);
					XFreePixmap(_xdpy, bitmap);
				}
				XDefineCursor(_xdpy, _xwin, _noneCursor);
			};

			if (cursor == CursorStyle::None) {
				hideCursor();
			} else if (cursor == CursorStyle::NoneUntilMouseMoves) {
				hideCursor();
			} else {
				// Ref file X11/cursorfont.h
				Cursor c;
				switch (cursor) {
					default:
					case CursorStyle::Normal:
					case CursorStyle::Arrow:
						c = XNone;
						break;
					case CursorStyle::Ibeam:
						c = XcursorLibraryLoadCursor(_xdpy, "xterm");
						break;
					case CursorStyle::PointingHand:
						c = XcursorLibraryLoadCursor(_xdpy, "hand1");
						break;
					case CursorStyle::ClosedHand:
						c = XcursorLibraryLoadCursor(_xdpy, "fleur");
						break;
					case CursorStyle::OpenHand:
						c = XcursorLibraryLoadCursor(_xdpy, "fleur");
						break;
					case CursorStyle::ResizeLeft:
						c = XcursorLibraryLoadCursor(_xdpy, "left_side");
						break;
					case CursorStyle::ResizeRight:
						c = XcursorLibraryLoadCursor(_xdpy, "right_side");
						break;
					case CursorStyle::ResizeLeftRight:
						c = XcursorLibraryLoadCursor(_xdpy, "sb_h_double_arrow");
						break;
					case CursorStyle::ResizeUp:
						c = XcursorLibraryLoadCursor(_xdpy, "top_side");
						break;
					case CursorStyle::ResizeDown:
						c = XcursorLibraryLoadCursor(_xdpy, "bottom_side");
						break;
					case CursorStyle::ResizeUpDown:
						c = XcursorLibraryLoadCursor(_xdpy, "sb_v_double_arrow");
						break;
					case CursorStyle::Crosshair:
						c = XcursorLibraryLoadCursor(_xdpy, "crosshair");
						break;
					case CursorStyle::DisappearingItem:
						c = XcursorLibraryLoadCursor(_xdpy, "X_cursor");
						break;
					case CursorStyle::OperationNotAllowed:
						c = XcursorLibraryLoadCursor(_xdpy, "X_cursor");
						break;
					case CursorStyle::DragLink:
						c = XcursorLibraryLoadCursor(_xdpy, "center_ptr");
						break;
					case CursorStyle::DragCopy:
						c = XcursorLibraryLoadCursor(_xdpy, "circle");
						break;
					case CursorStyle::ContextualMenu:
						c = XcursorLibraryLoadCursor(_xdpy, "arrow");
						break;
					case CursorStyle::IbeamForVertical:
						c = XcursorLibraryLoadCursor(_xdpy, "xterm");
						break;
					case CursorStyle::Cross:
						c = XcursorLibraryLoadCursor(_xdpy, "cross");
						break;
				}
				XDefineCursor(_xdpy, _xwin, c);
			}
		}
	};

	void Window::openImpl(Options &opts) {
		post_messate_main(Cb([&opts,this](auto e) {
			Qk_ASSERT_EQ(_impl, nullptr);
			_impl = new WindowPlatform(this, opts);
			_backgroundColor = opts.backgroundColor;
			_render->surface()->makeSurface(_impl->xwin());
			_render->reload();
			_render->surface()->renderLoopRun();
			activate();
		}), true);
	}

	void Window::closeImpl() {
		auto impl = static_cast<WindowPlatform*>(_impl);
		if (impl) {
			_impl = nullptr;
			post_messate_main(Cb([impl](auto e) {
				delete impl;
			}), false);
		}
	}

	void Window::set_backgroundColor(Color val) {
		post_messate_main(Cb([this, val](auto e) {
			auto impl = static_cast<WindowPlatform*>(_impl);
			impl->setBackgroundColor(val);
		}), false);
		_backgroundColor = val;
	}

	void Window::activate() {
		post_messate_main(Cb([this](auto e) {
			auto impl = static_cast<WindowPlatform*>(_impl);
			XMapWindow(impl->xdpy(), impl->xwin());
		}), false);
	}
 
	float Window::getDefaultScale() {
		return static_cast<WindowPlatform*>(_impl)->_xwin_scale;
	}

	void Window::pending() {
	}

	void Window::setFullscreen(bool fullscreen) {
		post_messate_main(Cb([this, fullscreen](auto e) {
			auto impl = static_cast<WindowPlatform*>(_impl);
			impl->setFullscreen(fullscreen);
		}), false);
	}

	void Window::setCursorStyle(CursorStyle cursor, bool isBase) {
		static CursorStyle current_cursor_base = CursorStyle::Arrow;
		static CursorStyle current_cursor_user = CursorStyle::Normal;

		if (isBase) {
			current_cursor_base = cursor;
		} else {
			current_cursor_user = cursor;
		}
		cursor = current_cursor_user == CursorStyle::Normal ? current_cursor_base: current_cursor_user;

		post_messate_main(Cb([this, cursor](auto e) {
			auto impl = static_cast<WindowPlatform*>(_impl);
			impl->setCursor(cursor);
		}), false);
	}
}
