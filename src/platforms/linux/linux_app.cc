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

#include "../../render/linux/linux_render.h"

#undef Status
#undef Bool
#undef None

#include "./linux_app.h"
#include "../../util/thread.h"
#include "../../util/http.h"
#include "../../ui/event.h"
#include "../../ui/app.h"
#include "../../ui/window.h"

namespace qk
{
	struct MainLooper;
	static MainLooper* looper = nullptr;
	static ThreadID main_thread_id(thread_self_id());

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

	struct MainLooper {
		XDisplay *_xdpy;
		XWindow _xwinTmp;
		App::Inl *_app;
		List<Cb> _msg;
		Mutex _msgMutex;
		Dict<XWindow, WindowImpl*> _winImpl;

		MainLooper(): _xdpy(0), _xwinTmp(0) {
			Qk_ASSERT_EQ(looper, nullptr);
			looper = this;
		}

		~MainLooper() {
			Qk_DLog("~MainLooper()");
			looper = nullptr;
		}

		XWindow newXWindow() {
			XSetWindowAttributes xset = {
				.event_mask = NoEventMask,
				.do_not_propagate_mask = NoEventMask,
				.override_redirect = False,
			};
			auto xwin = XCreateWindow(
				_xdpy, XDefaultRootWindow(_xdpy),
				0, 0, 1, 1, 0, 0,
				InputOutput,
				nullptr,
				0, &xset
			);
			Qk_ASSERT_RAW(xwin, "Cannot create XWindow");

			return xwin;
		}

		void runLoop() {
			_xdpy = openXDisplay();
			_xwinTmp = newXWindow();
			_app = Inl_Application(shared_app());
			_app->triggerLoad();

			Atom wmProtocols    = XInternAtom(_xdpy, "WM_PROTOCOLS"    , False);
			Atom wmDeleteWindow = XInternAtom(_xdpy, "WM_DELETE_WINDOW", False);

			WindowImpl* impl;
			XEvent event;
			do {
				XNextEvent(_xdpy, &event);
				resolvedMsg();

				if (XFilterEvent(&event, 0)) {
					continue;
				}

				if (!event.xany.window || !_winImpl.get(event.xany.window, impl)) {
					continue;
				}
				switch (event.type) {
					case Expose:
						Qk_DLog("event, Expose");
						impl->win()->render()->reload();
						break;
					case MapNotify:
						Qk_DLog("event, MapNotify, Window onForeground");
						_app->triggerForeground(impl->win());
						impl->win()->render()->surface()->renderLoopRun();
						break;
					case UnmapNotify:
						Qk_DLog("event, UnmapNotify, Window onBackground");
						_app->triggerBackground(impl->win());
						impl->win()->render()->surface()->renderLoopStop();
						break;
					case FocusIn:
						Qk_DLog("event, FocusIn, Window onResume");
						impl->ime()->focus_in();
						break;
					case FocusOut:
						Qk_DLog("event, FocusOut, Window onPause");
						impl->ime()->focus_out();
						break;
					case KeyPress:
						Qk_DLog("event, KeyDown, keycode: %ld", event.xkey.keycode);
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
						impl->win()->dispatch()->onMousepress(
							KeyboardKeyCode(KEYCODE_MOUSE_LEFT + event.xbutton.button - 1), true, nullptr);
						break;
					case ButtonRelease:
						Qk_DLog("event, MouseUp, button: %s", MOUSE_KEYS[event.xbutton.button - 1]);
						impl->win()->dispatch()->onMousepress(
							KeyboardKeyCode(KEYCODE_MOUSE_LEFT + event.xbutton.button - 1), false, nullptr);
						break;
					case MotionNotify: {
						// Qk_DLog("event, MouseMove: [%d, %d]", event.xmotion.x, event.xmotion.y);
						impl->win()->dispatch()->onMousemove(
							event.xmotion.x / impl->win()->scale(), event.xmotion.y / impl->win()->scale()
						);
						break;
					}
					case ClientMessage:
						if (event.xclient.message_type == wmProtocols && 
							(Atom)event.xclient.data.l[0] == wmDeleteWindow) { // close
							Qk_DLog("event, ClientMessage: Close");
							impl->win()->close();

							if (_winImpl.length() == 0) { // Exit process
								XDestroyWindow(_xdpy, _xwinTmp); _xwinTmp = 0;
								thread_exit(0);
							}
						}
						break;
					case GenericEvent:
						/* event is a union, so cookie == &event, but this is type safe. */
						if (XGetEventData(_xdpy, &event.xcookie)) {
							handleGenericEvent(event, impl);
							XFreeEventData(_xdpy, &event.xcookie);
						}
						break;
					default:
						//Qk_DLog("event, %d, %d", event.type, time_second());
						break;
				}
			} while(!is_process_exit());
		}

		void handleGenericEvent(XEvent& event, WindowImpl* impl) {
			XGenericEventCookie* cookie = &event.xcookie;
			XIDeviceEvent* xev = (XIDeviceEvent*)cookie->data;
			auto dispatch = impl->win()->dispatch();

			List<TouchEvent::TouchPoint> touchs = {{
				xev->detail,
				0, 0,
				float(xev->event_x) / impl->win()->scale(),
				float(xev->event_y) / impl->win()->scale(),
				0,
				false,
				nullptr,
			}};
			switch(cookie->evtype) {
				case XI_TouchBegin:
					Qk_DLog("event, XI_TouchBegin, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f",
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					dispatch->onTouchstart( std::move(touchs) );
					break;
				case XI_TouchEnd:
					Qk_DLog("event, XI_TouchEnd, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f",
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					dispatch->onTouchend( std::move(touchs) );
					break;
				case XI_TouchUpdate:
					Qk_DLog("event, XI_TouchUpdate, deviceid: %d, sourceid: %d, detail: %d, x: %f, y: %f",
						xev->deviceid, xev->sourceid, xev->detail, float(xev->event_x), float(xev->event_y));
					dispatch->onTouchmove( std::move(touchs) );
					break;
			}
		}

		void addMsg(Cb& cb) {
			_msgMutex.lock();
			_msg.pushBack(cb);
			_msgMutex.unlock();

			XEvent event;
			event.type = CirculateNotify;
			event.xcirculate.display = _xdpy;
			// event.xcirculate.window = _xwinTmp;
			event.xcirculate.place = PlaceOnTop;
			Qk_ASSERT_EQ(
				True,
				XSendEvent(_xdpy, _xwinTmp, False, NoEventMask, &event)
			);
			XFlush(_xdpy);
		}

		void resolvedMsg() {
			List<Cb> msg;
			_msgMutex.lock();
			if (_msg.length())
				msg = std::move(_msg);
			_msgMutex.unlock();

			if (msg.length()) {
				for (auto& i: msg)
					i->resolve();
			}
		}
	};

	void addImplToGlobal(XWindow xwin, WindowImpl* impl) {
		looper->_winImpl.set(xwin, impl);
	}

	void deleteImplFromGlobal(XWindow xwin) {
		looper->_winImpl.erase(xwin);
	}

	// sync to x11 main message loop
	void post_messate_main(Cb cb, bool sync) {
		Qk_ASSERT(looper);
		if (main_thread_id == thread_self_id()) {
			cb->resolve();
		} else if (sync) {
			CondMutex mutex;
			Cb cb1([&cb, &mutex](auto e) {
				cb->resolve();
				mutex.lock_and_notify_one();
			});
			looper->addMsg(cb1);
			mutex.lock_and_wait_for(); // wait
		} else {
			looper->addMsg(cb);
		}
	}

	void Application::openURL(cString& url) {
		if (vfork() == 0) {
			execlp("xdg-open", "xdg-open", *url, nullptr);
		}
	}

	void Application::sendEmail(cString& recipient,
			cString& subject, cString& body, cString& cc, cString& bcc)
	{
		String uri(String::format("mailto:%s?cc=%s&bcc=%s&subject=%s&body=%s",
			*recipient,
			*URI::encode(cc,true),
			*URI::encode(bcc,true),
			*URI::encode(subject,true), *URI::encode(body, true)
		));
		// xdg-open
		// mailto:haorooms@126.com?cc=name2@rapidtables.com&bcc=name3@rapidtables.com&
		// subject=The%20subject%20of%20the%20email&body=The%20body%20of%20the%20email

		if (vfork() == 0) {
			execlp("xdg-open", "xdg-open", *uri, nullptr);
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
			volume = Int32::clamp(volume, 0, 100);
			// const snd_mixer_selem_channel_id_t chs[] = {
			// 	SND_MIXER_SCHN_FRONT_LEFT,
			// 	SND_MIXER_SCHN_FRONT_RIGHT,
			// 	SND_MIXER_SCHN_REAR_LEFT,
			// 	SND_MIXER_SCHN_REAR_RIGHT,
			// 	SND_MIXER_SCHN_FRONT_CENTER,
			// 	SND_MIXER_SCHN_WOOFER,
			// 	SND_MIXER_SCHN_SIDE_LEFT,
			// 	SND_MIXER_SCHN_SIDE_RIGHT,
			// 	SND_MIXER_SCHN_REAR_CENTER,
			// };
			// int len = sizeof(snd_mixer_selem_channel_id_t) / sizeof(chs);

			// for (int i = 0; i < len; i++) {
			// 	snd_mixer_selem_set_playback_volume(_element, chs[i], volume);
			// }
			snd_mixer_selem_set_playback_volume_all(_element, volume);
		}
	};

	static Snd* snd() {
		static Sp<Snd> _snd = nullptr;
		if (!_snd)
			_snd = new Snd;
		return _snd.get();
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
