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

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <locale.h>
#undef Status
#undef Bool
#undef None

#include "./linux_app.h"
#include "../../ui/keyboard.h"
#include "../../ui/window.h"
#include "../../ui/event.h"

namespace qk {

	class LinuxIMEHelperImpl: public LinuxIMEHelper {
	public:

		LinuxIMEHelperImpl(WindowImpl* impl, int inputStyle)
			: _win(impl->win())
			, _xwin(impl->xwin())
			, _xdpy(impl->xdpy())
			, _im(nullptr), _ic(nullptr)
			, _has_open(false)
			, _input_style(inputStyle)
			, _fontSet(nullptr)
		{
			_spot_location = {1,1};

			// load fontset
			char **missing_list = nullptr;
			int missing_count = 0;
			char *default_string = nullptr;
			_fontSet = XCreateFontSet(_xdpy, "*,*", 
				&missing_list, &missing_count, &default_string);
			if (_fontSet == nullptr) {
				Qk_ELog("Can't create font set: %s", "*,*");
			}
		}

		~LinuxIMEHelperImpl() override {
			if (_fontSet) {
				XFreeFontSet(_xdpy, _fontSet);
			}
			close();
		}

		void open() {
			Qk_DLog("IME open");
			if (!_has_open) {
				_has_open = true;
				registerInstantiateCallback();
			}
		}

		void close() {
			Qk_DLog("IME close");
			_has_open = false;
			destroyIC();
			closeIM();
		}

		void clear() {
			Qk_DLog("IME clear");
			if (_has_open && _ic) {
				if (!_preedit_string.is_empty()) {
					_preedit_string = String();
					_win->dispatch()->dispatch_ime_unmark(String());
				}
				XUnsetICFocus(_ic);
				Xutf8ResetIC(_ic);
				XSetICFocus(_ic);
			}
		}

		void set_keyboard_can_backspace(bool can_backspace, bool can_delete) {}

		void set_keyboard_type(KeyboardType type) {}

		void set_keyboard_return_type(KeyboardReturnType type) {}

		void set_spot_rect(Rect rect) {
			auto location = rect.origin;
			Qk_DLog("set_spot_rect, x=%f,y=%f", location[0], location[1]);
			if (location[0] != 0 || location[1] != 0) {
				Vec2 scale = _win->display_port()->scale_value();
				_spot_location = {
					int16(location.x() * scale.x()), int16(location.y() * scale.y())
				};
				updateSpotLocation();
			}
		}

		void key_press(XKeyPressedEvent *event) override
		{
			if (_im == nullptr)
				return;

			char buf[256] = { '\0', };
			KeySym keysym = 0;
			Status status = XLookupNone;

			if (_ic == nullptr) {
				XLookupString(event, buf, sizeof(buf), &keysym, nullptr);
				status = XLookupchars;
			} else {
				Xutf8LookupString(_ic, event, buf, 256, &keysym, &status);
			}

			Qk_DLog("onKeyPress %lu\n", keysym);

			if (status == XLookupchars || 
				status == XLookupKeySym || status == XLookupBoth) {

				if (keysym == XK_Return) {
					onKeyReturn();
				} else if (keysym == XK_Delete) {
					onKeyDelete();
				} else if (keysym == XK_BackSpace) {
					onKeyBackspace();
				} else if (keysym == XK_Escape) {
					// esc
				} else if (keysym == XK_Left) {
					onKeyControl(KEYCODE_LEFT);
				} else if (keysym == XK_Right) {
					onKeyControl(KEYCODE_RIGHT);
				} else if (keysym == XK_Up) {
					onKeyControl(KEYCODE_UP);
				} else if (keysym == XK_Down) {
					onKeyControl(KEYCODE_DOWN);
				} else if (keysym == XK_Home) {
					onKeyControl(KEYCODE_MOVE_HOME);
				} else if (keysym == XK_End) {
					onKeyControl(KEYCODE_MOVE_END);
				} else {
					if ((status == XLookupchars || status == XLookupBoth) &&
						((event->state & ControlMask) != ControlMask) &&
						((event->state & Mod1Mask) != Mod1Mask)) 
					{
						insert(buf);
					}
				}
			}
		}

		void focus_in() override
		{
			if (_ic == nullptr)
				return;
			XSetICFocus(_ic);
		}

		void focus_out() override
		{
			if (_ic == nullptr)
				return;

			char* str = Xutf8ResetIC(_ic);

			if (str != nullptr) {
				setPreeditString(str, 0, 0);
			}
			setPreeditString(nullptr, 0, 0);

			XUnsetICFocus(_ic);
		}

	private:
		// for XIM
		void registerInstantiateCallback() 
		{
			XRegisterIMInstantiateCallback(_xdpy, nullptr, nullptr, nullptr,
				IMInstantiateCallback, (XPointer)this);
		}

		// IM instantiate callback
		static void IMInstantiateCallback(Display *display, XPointer client_data, XPointer data)
		{
			if (client_data == nullptr)
				return;
			Qk_DLog("XIM is available now");
			auto self = reinterpret_cast<Inl*>(client_data);
			self->openIM();
		}

		// IM destroy callbacks
		static void IMDestroyCallback(XIM im, XPointer client_data, XPointer data)
		{
			Qk_DLog("xim is destroyed");

			if (client_data == nullptr)
				return;

			auto self = reinterpret_cast<Inl*>(client_data);

			self->_im = nullptr;
			self->_ic = nullptr;

			if (self->_has_open)
				self->registerInstantiateCallback();
		}

		// on the spot callbacks
		static void preeditStartCallback(XIM xim, XPointer user_data, XPointer data)
		{
			Qk_DLog("preedit start");
		}

		static void preeditDoneCallback(XIM xim, XPointer user_data, XPointer data)
		{
			Qk_DLog("preedit done");

			if (user_data == nullptr)
				return;

			auto self = reinterpret_cast<Inl*>(user_data);
			self->setPreeditString(nullptr, 0, 0);
		}

		static void preeditDrawCallback(XIM xim, XPointer user_data, XPointer data)
		{
			if (user_data == nullptr || data == nullptr)
				return;

			auto self = reinterpret_cast<Inl*>(user_data);

			XIMPreeditDrawCallbackStruct *draw_data = 
				reinterpret_cast<XIMPreeditDrawCallbackStruct*>(data);

			self->setPreeditCaret(draw_data->caret);

			if (draw_data->text == nullptr) {
				self->setPreeditString(nullptr,
					draw_data->chg_first, draw_data->chg_length);
			} else {
				if (draw_data->text->encoding_is_wchar) {
					String str = wchar_t_to_string(draw_data->text->string.wide_char);
					self->setPreeditString(*str,
							draw_data->chg_first, draw_data->chg_length);
				} else {
					self->setPreeditString(draw_data->text->string.multi_byte,
						draw_data->chg_first, draw_data->chg_length);
				}
			}
		}

		static void preeditCaretCallback(XIM xim, XPointer user_data, XPointer data)
		{
			if (user_data == nullptr || data == nullptr)
			return;

			auto self = reinterpret_cast<Inl*>(user_data);
			XIMPreeditCaretCallbackStruct *caret_data = 
				reinterpret_cast<XIMPreeditCaretCallbackStruct*>(data);

			switch (caret_data->direction) {
				case XIMForwardchar:
					self->onKeyControl(KEYCODE_RIGHT);
					break;
				case XIMBackwardchar:
					self->onKeyControl(KEYCODE_LEFT);
					break;
				case XIMDontChange:
					break;
				default:
					Qk_DLog("preedit caret: %d", caret_data->direction);
					break;
			}
		}

		static void statusStartCallback(XIM xim, XPointer user_data, XPointer data)
		{
			Qk_DLog("status start");
		}

		static void statusDoneCallback(XIM xim, XPointer user_data, XPointer data)
		{
			Qk_DLog("status done");
		}

		static void statusDrawCallback(XIM xim, XPointer user_data, XPointer data)
		{
			Qk_DLog("status draw");
		}

		// string conversion callback
		static void stringConversionCallback(XIM xim, XPointer client_data, XPointer data)
		{
			Qk_DLog("string conversion");
		}

		// for XIM interaction
		void openIM()
		{
			Qk_ASSERT(!_im);

			_im = XOpenIM(_xdpy, nullptr, nullptr, nullptr);
			if (_im  == nullptr) {
				Qk_DLog("Can't open XIM");
				return;
			}

			Qk_DLog("XIM is opened");
			XUnregisterIMInstantiateCallback(_xdpy, nullptr, nullptr, nullptr,
							IMInstantiateCallback, (XPointer)this);

			// register destroy callback
			XIMCallback destroy;
			destroy.callback = IMDestroyCallback;
			destroy.client_data = (XPointer)this;
			XSetIMValues(_im, XNDestroyCallback, &destroy, nullptr);

			bool useStringConversion = false;

			XIMValuesList* ic_values = nullptr;
			XGetIMValues(_im,
				XNQueryICValuesList, &ic_values,
				nullptr);
			
			if (ic_values != nullptr) {
				for (int i = 0; i < ic_values->count_values; i++) {
					Qk_DLog("%s", ic_values->supported_values[i]);
					if (strcmp(ic_values->supported_values[i],
								XNStringConversionCallback) == 0) {
						useStringConversion = true;
						break;
					}
				}
			}

			createIC(useStringConversion);
		}

		void closeIM(void)
		{
			if (_im == nullptr)
				return;

			XCloseIM(_im);
			_im = nullptr;

			Qk_DLog("XIM is closed");
		}

		void createIC(bool useStringConversion)
		{
			if (_im == nullptr)
				return;
			
			Qk_ASSERT(!_ic);

			if ((_input_style & XIMPreeditPosition) && _fontSet) {
				XRectangle area = { 0,0,1,1 };
				XVaNestedList attr = XVaCreateNestedList(0,
								XNSpotLocation, &_spot_location,
								XNArea, &area,
								XNFontSet, _fontSet,
								nullptr);

				_ic = XCreateIC(_im,
								XNInputStyle, XIMPreeditPosition,
								XNClientWindow, _xwin,
								XNPreeditAttributes, attr,
								nullptr);

				XFree(attr);
			} else {
				XIMCallback preedit_start, preedit_done, preedit_draw, preedit_caret;

				preedit_start.callback = preeditStartCallback;
				preedit_start.client_data = (XPointer)this;
				preedit_done.callback = preeditDoneCallback;
				preedit_done.client_data = (XPointer)this;
				preedit_draw.callback = preeditDrawCallback;
				preedit_draw.client_data = (XPointer)this;
				preedit_caret.callback = preeditCaretCallback;
				preedit_caret.client_data = (XPointer)this;

				XIMCallback status_start, status_done, status_draw;

				status_start.callback = statusStartCallback;
				status_start.client_data = (XPointer)this;
				status_done.callback = statusDoneCallback;
				status_done.client_data = (XPointer)this;
				status_draw.callback = statusDrawCallback;
				status_draw.client_data = (XPointer)this;

				XVaNestedList preedit_attr = XVaCreateNestedList(0,
								XNPreeditStartCallback, &preedit_start,
								XNPreeditDoneCallback,  &preedit_done,
								XNPreeditDrawCallback,  &preedit_draw,
								XNPreeditCaretCallback, &preedit_caret,
								nullptr);

				XVaNestedList status_attr = XVaCreateNestedList(0,
								XNStatusStartCallback, &status_start,
								XNStatusDoneCallback,  &status_done,
								XNStatusDrawCallback,  &status_draw,
								nullptr);

				_ic = XCreateIC(_im,
								XNInputStyle, XIMPreeditCallbacks,
								XNClientWindow, _xwin,
								XNPreeditAttributes, preedit_attr,
								XNStatusAttributes, status_attr,
								nullptr);
				XFree(preedit_attr);
				XFree(status_attr);
			}

			if (_ic != nullptr) {
				if (useStringConversion) {
					XIMCallback strconv;
					strconv.callback    = stringConversionCallback;
					strconv.client_data = (XPointer)this;
					XSetICValues(_ic, XNStringConversionCallback, &strconv, nullptr);
				}
				Qk_DLog("XIC is created");
			} else {
				Qk_DLog("cannot create XIC");
			}
		}

		void destroyIC()
		{
			if (_ic == nullptr)
				return;

			char* str = Xutf8ResetIC(_ic);

			if (str != nullptr) {
				setPreeditString(str, 0, 0);
			}
			setPreeditString(nullptr, 0, 0);

			XDestroyIC(_ic);
			_ic = nullptr;
			Qk_DLog("XIC is destroyed");
		}

		static String wchar_t_to_string(const wchar_t *str)
		{
			if (sizeof(wchar_t) == 2) {
				String2 ustr = (const uint16*)str;
				return ustr.toString();
			} else {
				Ucs4String ustr = (const uint*)str;
				return ustr.toString();
			}
		}

		void updateSpotLocation() {
			if (_ic) {
				XVaNestedList attr = XVaCreateNestedList(0,
					XNSpotLocation, &_spot_location, nullptr);
				XSetICValues(_ic, XNPreeditAttributes, attr, nullptr);
				XFree(attr);
			}
		}

		void insert(cchar* str)
		{
			Qk_DLog("insert, %s", str);
			_win->dispatch()->onImeInsert(str);
		}

		void setPreeditString(cchar* str, int pos, int length)
		{
			Qk_DLog("setPreeditString, %s, %d, %d", str, pos, length);
			if (str == nullptr) {
				_win->dispatch()->onImeUnmark(String());
				_preedit_string = String();
			} else {
				_preedit_string = str;
				_win->dispatch()->onImeMarked(_preedit_string);
			}
		}

		void onKeyReturn()
		{
			_win->dispatch()->onImeInsert("\n");
		}

		void onKeyDelete()
		{
			_win->dispatch()->onImeDelete(1);
		}

		void onKeyBackspace()
		{
			_win->dispatch()->onImeDelete(-1);
		}

		void setPreeditCaret(int pos)
		{
			// TODO ...
		}

		void onKeyControl(KeyboardKeyCode name) {
			_win->dispatch()->onImeControl(name);
		}

		Window* _win;
		XWindow _xwin;
		XDisplay *_xdpy;
		XIM _im;
		XIC _ic;
		String _preedit_string;
		bool _has_open;
		int _input_style;
		XPoint _spot_location;
		XFontSet _fontSet;
	};

	LinuxIMEHelper* LinuxIMEHelper::Make(WindowImpl* impl, int inputStyle) {
		char *locale = setlocale(LC_CTYPE, "");
		if (locale == nullptr) {
			Qk_ELog("Can't set locale");
			return nullptr;
		}
		Qk_DLog("locale: %s", locale);

		if (!XSupportsLocale()) {
			Qk_ELog("X does not support locale");
			return nullptr;
		}

		char *modifiers = XSetLocaleModifiers("");
		if (modifiers == nullptr) {
			Qk_ELog("Can't set locale modifiers");
			return nullptr;
		}
		Qk_DLog("modifiers: %s", modifiers);

		return new LinuxIMEHelperImpl(impl, inputStyle);
	}

	// ***************** E v e n t . D i s p a t c h *****************

	void EventDispatch::setImeKeyboardOpen(KeyboardOptions opts) {
		auto impl = window()->impl();
		post_messate_main(Cb([impl, opts](auto e) {
			auto ime = static_cast<LinuxIMEHelperImpl*>(impl->ime());
			ime->set_keyboard_type(opts.type);
			ime->set_keyboard_return_type(opts.return_type);
			ime->set_spot_rect(opts.spot_rect);
			if (opts.clear) {
				ime->clear();
			}
			ime->open();
		}));
	}

	void EventDispatch::setImeKeyboardClose() {
		auto impl = window()->impl();
		post_messate_main(Cb([=](auto e) {
			static_cast<LinuxIMEHelperImpl*>(impl->ime())->close();
		}));
	}

	void EventDispatch::setImeKeyboardCanBackspace(bool can_backspace, bool can_delete) {
		auto impl = window()->impl();
		post_messate_main(Cb([impl, can_backspace, can_delete](auto e) {
			static_cast<LinuxIMEHelperImpl*>(impl->ime())->
				set_keyboard_can_backspace(can_backspace, can_delete);
		}));
	}

	void EventDispatch::setImeKeyboardSpotRect(Rect rect) {
		auto impl = window()->impl();
		post_messate_main(Cb([impl, rect](auto e) {
			static_cast<LinuxIMEHelperImpl*>(impl->ime())->set_spot_rect(rect);
		}));
	}

}
