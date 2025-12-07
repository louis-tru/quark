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
#include "../../ui/ui.h"

namespace qk {
	typedef const Window::Options cOptions;

	struct X11 {
		XDisplay* xdpy;
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
		XFreeDeviceList(devices);

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
			XrmDestroyDatabase(db);
		}
		Qk_DLog("DPI: %f", dpi);
		return dpi;
	}

	static X11& x11() {
		static X11 x([](){
			auto xdpy = openXDisplay();
			return X11{
				xdpy, openXDevice(xdpy), getXDisplayDpi(xdpy)
			};
		}());
		return x;
	}

	float dpiForXDisplay() {
		return x11().xdpyDpi;
	}

	void addImplToGlobal(XWindow xwin, WindowImpl* win);
	void deleteImplFromGlobal(XWindow xwin);

	class WindowPlatform: public WindowImpl {
		#define _platform(_impl) static_cast<WindowPlatform*>(_impl)
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
			_xset.background_pixmap = XNone;
			_xset.background_pixel = opts.backgroundColor.to_uint32_rgba() >> 8,
			_xset.border_pixmap = XNone;
			_xset.border_pixel = 0;
			_xset.colormap = XNone;
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

			if (!x11().xdevice) { // It's not a multipoint device
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
				select(opts.frame.begin[0] * _xwin_scale, (xdpyW - width) / 2),
				select(opts.frame.begin[1] * _xwin_scale, (xdpyH - height) / 2), width, height, 0,
				DefaultDepth(_xdpy, 0),
				InputOutput,
				DefaultVisual(_xdpy, 0),
				CWBackPixel | CWEventMask | CWBorderPixel | CWColormap, &_xset
			);

			// Qk_DLog("_xset.background_pixel, %d", _xset.background_pixel);

			Qk_CHECK(xwin, "Cannot create XWindow");

			if (x11().xdevice) { // It's a Multipoint device
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
			_xset.background_pixel = val.to_uint32_rgba() >> 8; // delete alpha quantity
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
			_render->surface()->makeSurface((EGLNativeWindowType)_impl->xwin());
			_render->reload();
			_render->surface()->renderLoopRun();
			activate();
		}), true);
	}

	void Window::closeImpl() {
		post_messate_main(Cb([this](auto e) {
			Qk_ASSERT_NE(_impl, nullptr);
			delete _platform(_impl);
			_impl = nullptr;
			if (!_host->activeWindow())
				thread_exit(0); // Exit process
		}, this), false);
	}

	void Window::beforeClose() {}

	void Window::set_backgroundColor(Color val) {
		post_messate_main(Cb([this, val](auto e) {
			if (!_impl) return;
			_platform(_impl)->setBackgroundColor(val);
		}, this), false);
		_backgroundColor = val;
	}

	void Window::activate() {
		post_messate_main(Cb([this](auto e) {
			if (!_impl) return;
			XMapWindow(_platform(_impl)->xdpy(), _platform(_impl)->xwin());
		}, this), false);
		Inl_Application(_host)->setActiveWindow(this);
	}

	float Window::getDefaultScale() {
		return _platform(_impl)->_xwin_scale;
	}

	Range Window::getDisplayRange(Vec2 size) {
		// begin,end
		return {{0}, size};
	}

	void Window::afterDisplay() {
		// Noop
	}

	void Window::pending() {
	}

	void Window::setFullscreen(bool fullscreen) {
		post_messate_main(Cb([this, fullscreen](auto e) {
			if (!_impl) return;
			_platform(_impl)->setFullscreen(fullscreen);
		}, this), false);
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
			if (!_impl) return;
			_platform(_impl)->setCursor(cursor);
		}, this), false);
	}
}
