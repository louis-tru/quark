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

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <signal.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "./linux_app.h"
#include "../../util/thread.h"
#include "../../util/http.h"
#include "../../ui/event.h"
#include "../../ui/app.h"
#include "../../ui/window.h"

namespace qk
{
	struct Mainlooper;
	static Mainlooper* looper = nullptr;

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

	static WindowImpl* windowBy(XEvent& event, XWindowAttributes* attrs = nullptr) {
		if (event.xany.window) {
			static XWindowAttributes attrsStatic;
			if (!attrs) {
				attrs = &attrsStatic;
			}
			Qk_ASSERT_EQ(True, XGetWindowAttributes(_xdpy, event.xany.window, &attrs));
			if (attrs.visual && attrs.visual->private_data) {
				auto impl = reinterpret_cast<WindowImpl*>(attrs.visual->ext_data->private_data);
				Qk_ASSERT_EQ(impl->isValid(), true);
				if (impl->isValid()) {
					return impl;
				}
			}
		}
		return nullptr;
	}

	struct MainLooper {
		MainLooper(): _xdpy(nullptr) {
			Qk_ASSERT_EQ(looper, nullptr);
			looper = this;
		}

		~MainLooper() {
			Qk_DLog("~MainLooper()");
			looper = nullptr;
		}

		void runLoop() {
			_xdpy = openXDisplay();
			_app = Inl_Application(shared_app());
			_app->triggerLoad();

			bool appActive = false;
			Atom wmProtocols    = XInternAtom(_xdpy, "WM_PROTOCOLS"    , False);
			Atom wmDeleteWindow = XInternAtom(_xdpy, "WM_DELETE_WINDOW", False);

			WindowImpl* impl;
			XEvent event;
			do {
				XNextEvent(_xdpy, &event);

				resolvedMsg();

				if (XFilterEvent(&event, None))
					continue;

				switch (event.type) {
					case Expose:
						Qk_DLog("event, Expose");
						impl = windowBy(event);
						impl->win()->render()->reload();
						break;
					case MapNotify:
						Qk_DLog("event, MapNotify, Window onForeground");
						impl = windowBy(event);
						_app->triggerForeground(impl->win());
						// _render_looper->start();
						break;
					case UnmapNotify:
						Qk_DLog("event, UnmapNotify, Window onBackground");
						impl = windowBy(event);
						_app->triggerBackground(impl->win());
						// _render_looper->stop();
						break;
					case FocusIn:
						Qk_DLog("event, FocusIn, Window onResume");
						impl = windowBy(event);
						impl->ime()->focus_in();
						// _app->triggerResume();
						// _app->triggerUnload();
						// _app->triggerMemorywarning();
						break;
					case FocusOut:
						Qk_DLog("event, FocusOut, Window onPause");
						impl = windowBy(event);
						impl->ime()->focus_out();
						// _host->triggerPause();
						break;
					case KeyPress:
						Qk_DLog("event, KeyDown, keycode: %ld", event.xkey.keycode);
						impl = windowBy(event);
						impl->ime()->key_press(&event.xkey);
						impl->win()->dispatch()->keyboard()->dispatch(event.xkey.keycode, false, true,
							false, false, 0, 0);
						break;
					case KeyRelease:
						Qk_DLog("event, KeyUp, keycode: %d", event.xkey.keycode);
						impl->win()->dispatch()->keyboard()->dispatch(event.xkey.keycode, false, false,
							false, false, 0, 0);
						break;
					case ButtonPress:
						Qk_DLog("event, MouseDown, button: %s", MOUSE_KEYS[event.xbutton.button - 1]);
						impl = windowBy(event);
						impl->win()->dispatch()->onMousepress(
							KeyboardKeyCode(KEYCODE_MOUSE_LEFT + event.xbutton.button - 1), true, nullptr);
						break;
					case ButtonRelease:
						Qk_DLog("event, MouseUp, button: %s", MOUSE_KEYS[event.xbutton.button - 1]);
						impl = windowBy(event);
						impl->win()->dispatch()->onMousepress(
							KeyboardKeyCode(KEYCODE_MOUSE_LEFT + event.xbutton.button - 1), false, nullptr);
						break;
					case MotionNotify: {
						Qk_DLog("event, MouseMove: [%d, %d]", event.xmotion.x, event.xmotion.y);
						impl = windowBy(event);
						impl->win()->dispatch()->onMousemove(
							event.xmotion.x / impl->win()->scale(), event.xmotion.y / impl->win()->scale()
						);
						break;
					}
					case ClientMessage:
						if (event.xclient.message_type == wmProtocols && 
							(Atom)event.xclient.data.l[0] == wmDeleteWindow) {
							Qk_DLog("event, ClientMessage: Close");
							// return;
						}
						break;
					case GenericEvent:
						/* event is a union, so cookie == &event, but this is type safe. */
						if (XGetEventData(_xdpy, &event.xcookie)) {
							handleGenericEvent(event);
							XFreeEventData(_xdpy, &event.xcookie);
						}
						break;
					default:
						Qk_DLog("event, %d", event.type);
						break;
				}
			} while(!is_process_exit());
		}

		void handleGenericEvent(XEvent& event) {
			XGenericEventCookie* cookie = &event.xcookie;
			XIDeviceEvent* xev = (XIDeviceEvent*)cookie->data;
			auto win = windowBy(event)->win();
			auto dispatch = win->dispatch();

			List<TouchEvent::TouchPoint> touchs = {{
				xev->detail,
				0, 0,
				float(xev->event_x) / win->scale(),
				float(xev->event_y) / win->scale(),
				0,
				false,
				nullptr,
			}};
			switch(cookie->evtype) {
				case XI_TouchBegin:
					Qk_DLog("event, XI_TouchBegin, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f",
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					dispatch->onTouchstart( move(touchs) );
					break;
				case XI_TouchEnd:
					Qk_DLog("event, XI_TouchEnd, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f",
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					dispatch->onTouchend( move(touchs) );
					break;
				case XI_TouchUpdate:
					Qk_DLog("event, XI_TouchUpdate, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f",
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					dispatch->onTouchmove( move(touchs) );
					break;
			}
		}

		void postMessateMain(Cb& cb, bool sync) {
			_msgMutex.lock();
			_msg.pushBack(cb);
			_msgMutex.unlock();

			XCirculateEvent event;
			event.type = CirculateNotify;
			event.display = _xdpy;
			event.window = _win;
			event.place = PlaceOnTop;
			Qk_ASSERT_EQ(
				TRUE,
				XSendEvent(_xdpy, nullptr, false, NoEventMask, (XEvent*)&event)
			);
		}

		void resolvedMsg() {
			List<Cb> msg;
			_msgMutex.lock();
			if (msg.length())
				msg = move(msg);
			_msgMutex.unlock();

			if (msg.length()) {
				for (auto& i: msg)
					i->resolve();
			}
		}

		XDisplay *_xdpy;
		App::Inl *_app;
		List<Cb> _msg;
		Mutex _msgMutex;
	};

	// sync to x11 main message loop
	void post_messate_main(Cb cb, bool sync) {
		Qk_ASSERT(application);
		return application->postMessateMain(cb);
	}

	void Application::openUrl(cString& url) {
		if (vfork() == 0) {
			execlp("xdg-open", "xdg-open", *url, NULL);
		}
	}

	void Application::sendEmail(cString& recipient,
			cString& subject, cString& body, cString& cc, cString& bcc)
	{
		String arg = "mailto:";

		arg += recipient + '?';

		if (!cc.isEmpty()) {
			arg += "cc=";
			arg += cc + '&';
		}
		if (!bcc.isEmpty()) {
			arg += "bcc=";
			arg += bcc + '&';
		}

		arg += "subject=";
		arg += URI::encode(subject) + '&';

		if (!body.isEmpty()) {
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

	// ***************** E v e n t . D i s p a t c h *****************

	struct Snd {
		snd_mixer_t* _mixer;
		snd_mixer_elem_t* _element;

		Snd(): _mixer(nullptr), _element(nullptr) {
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

		~Snd() {
			if (_mixer) {
				snd_mixer_free(_mixer);
				snd_mixer_detach(_mixer, "default");
				snd_mixer_close(_mixer);
				_mixer = nullptr;
				_element = nullptr;
			}
		}

		int master_volume() {
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
	};

	static Snd* snd() {
		static Sp<Snd> _snd = nullptr;
		if (!_snd)
			_snd = new Snd;
		return snd.get();
	}

	void EventDispatch::setVolumeUp() {
		snd()->set_master_volume(snd()->master_volume() + 1);
	}

	void EventDispatch::setVolumeDown() {
		snd()->set_master_volume(snd()->master_volume() - 1);
	}

}

extern "C" Qk_EXPORT int main(int argc, char* argv[]) {
	using namespace qk;

	MainLooper looper;
	Application::runMain(argc, argv);

	if ( shared_app() ) {
		looper.runLoop();
	}
	return 0;
}
