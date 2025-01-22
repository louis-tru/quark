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

#include "./android.h"
#include "../../ui/app.h"
#include "../../ui/window.h"
#include "../../ui/screen.h"
#include "../../ui/event.h"
#include "../../render/linux/linux_render.h"
#include <android/native_activity.h>
#include <android/native_window.h>

namespace qk {

	typedef TouchEvent::TouchPoint TouchPoint;
	typedef Screen::Orientation Orientation;
	class WindowImpl {};
	class SharedWindowmanager;
	static SharedWindowmanager *manager = nullptr;

	static void dispatchEvent(AInputEvent* event);

	class SharedWindowmanager: public WindowImpl {
	public:

		SharedWindowmanager()
		{
			Qk_ASSERT_EQ(manager, nullptr);
			manager = this;
			_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
		}

		inline void RunLoop* loop() {
			return _host->loop();
		}

		void post_messate_main(Cb cb, bool sync) {
			// TODO ...
		}

		void enterWindow(Window* win) {
			if (_window && win) {
				auto render = win->render();
				render->surface()->makeSurface(_window);
				render->reload();
				// render->surface()->renderDisplay(); // redrawing
			}
		}

		void exitWindow(Window* win) {
			if (win)
				win->render()->surface()->deleteSurface();
		}

		static void Window* activeWindow() {
			return manager->_host->activeWindow();
		}

		static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
			if ( queue != manager->_queue ) {
				if ( manager->_queue ) {
					AInputQueue_detachLooper(manager->_queue);
				}
				AInputQueue_attachLooper(queue, manager->_looper, ALOOPER_POLL_CALLBACK, (ALooper_callbackFunc)
				[](int fd, int events, AInputQueue* queue) -> int {
					AInputEvent* event = nullptr;
					while( AInputQueue_getEvent(queue, &event) >= 0 ) {
						if (AInputQueue_preDispatchEvent(queue, event) == 0) {
							dispatchEvent(event);
							AInputQueue_finishEvent(queue, event, fd);
						} else {
							Qk_DLog("AInputQueue_preDispatchEvent(queue, event) != 0");
						}
					}
					return 1;
				}, queue);
				manager->_queue = queue;
			}
		}

		static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
			AInputQueue_detachLooper(queue);
			manager->_queue = nullptr;
		}

		static void onLowMemory(ANativeActivity* activity) {
			Qk_DLog("onLowMemory");
			manager->_host->triggerMemorywarning();
		}

		static void onResume(ANativeActivity* activity) {
			manager->_host->triggerResume();
		}

		static void onPause(ANativeActivity* activity) {
			manager->_host->triggerPause();
		}

		static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
			return nullptr;
		}
		
		static void onConfigurationChanged(ANativeActivity* activity) {
			Qk_DLog("onConfigurationChanged");
		}

		static void onCreate(ANativeActivity* activity, void* saved_state, size_t saved_state_size) {
			if ( !manager ) {
				new manager();
			}
			Qk_ASSERT_EQ(manager->_activity, nullptr);
			// ANativeActivity_setWindowFlags(activity, 0x00000400, 0);

			activity->callbacks->onDestroy                  = &manager::onDestroy;
			activity->callbacks->onStart                    = &manager::onStart;
			activity->callbacks->onResume                   = &manager::onResume;
			activity->callbacks->onSaveInstanceState        = &manager::onSaveInstanceState;
			activity->callbacks->onPause                    = &manager::onPause;
			activity->callbacks->onStop                     = &manager::onStop;
			activity->callbacks->onConfigurationChanged     = &manager::onConfigurationChanged;
			activity->callbacks->onLowMemory                = &manager::onLowMemory;
			activity->callbacks->onWindowFocusChanged       = &manager::onWindowFocusChanged;
			activity->callbacks->onNativeWindowCreated      = &manager::onNativeWindowCreated;
			activity->callbacks->onNativeWindowResized      = &manager::onNativeWindowResized;
			activity->callbacks->onNativeWindowRedrawNeeded = &manager::onNativeWindowRedrawNeeded;
			activity->callbacks->onNativeWindowDestroyed    = &manager::onNativeWindowDestroyed;
			activity->callbacks->onInputQueueCreated        = &manager::onInputQueueCreated;
			activity->callbacks->onInputQueueDestroyed      = &manager::onInputQueueDestroyed;
			activity->callbacks->onContentRectChanged       = &manager::onContentRectChanged;
			activity->instance = manager;

			manager->_activity = activity;
		}

		static void onDestroy(ANativeActivity* activity) {
			Qk_ASSERT_NE(manager->_activity, nullptr);

			activity->callbacks->onDestroy                  = nullptr;
			activity->callbacks->onStart                    = nullptr;
			activity->callbacks->onResume                   = nullptr;
			activity->callbacks->onSaveInstanceState        = nullptr;
			activity->callbacks->onPause                    = nullptr;
			activity->callbacks->onStop                     = nullptr;
			activity->callbacks->onConfigurationChanged     = nullptr;
			activity->callbacks->onLowMemory                = nullptr;
			activity->callbacks->onWindowFocusChanged       = nullptr;
			activity->callbacks->onNativeWindowCreated      = nullptr;
			activity->callbacks->onNativeWindowResized      = nullptr;
			activity->callbacks->onNativeWindowRedrawNeeded = nullptr;
			activity->callbacks->onNativeWindowDestroyed    = nullptr;
			activity->callbacks->onInputQueueCreated        = nullptr;
			activity->callbacks->onInputQueueDestroyed      = nullptr;
			activity->callbacks->onContentRectChanged       = nullptr;

			manager->_activity = nullptr;
		}

		static void onStart(ANativeActivity* activity) {
			if ( manager->_host == nullptr ) { // start gui
				Application::runMain(0, nullptr); // run gui application

				manager->_host = Inl_Application(shared_app());

				Qk_ASSERT(manager->_host);
				Qk_ASSERT_EQ(manager->_activity, activity);

				manager->_host->triggerLoad();
			}

			//auto awin = activeWindow();
			//if (awin)
			//	manager->_host->triggerForeground(awin);
		}

		static void onStop(ANativeActivity* activity) {
			Qk_DLog("onStop");
			//auto awin = activeWindow();
			//if (awin)
			//	manager->_host->triggerBackground(awin);
		}

		// ----------------------------------------------------------------------

		static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
			Qk_ASSERT_NE(manager->_window, nullptr);
			// ANativeWindow_setBuffersGeometry(window, 800, 480, WINDOW_FORMAT_RGBX_8888);
			manager->_window = window;
			manager->enterWindow(activeWindow());
		}

		static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
			manager->_window = nullptr;
			manager->exitWindow(activeWindow());
		}

		static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
			Qk_DLog("onNativeWindowResized");
		}

		static void onWindowFocusChanged(ANativeActivity* activity, int hasFocus) {
			Qk_DLog("onWindowFocusChanged");
		}

		static void onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
			Qk_DLog("onNativeWindowRedrawNeeded");
		}

		// ----------------------------------------------------------------------

		static void onContentRectChanged(ANativeActivity* activity, const ARect* rect) {
			/*
			* Rect选择绘制，主要因为android的虚拟导航按键，虚拟导航按键区域绘制黑色背景颜色填充
			*/
			manager->_rect = {
				Vec2(rect->left, 0),
				Vec2(rect->right - rect->left, rect->bottom)
			};

			auto awin = activeWindow();
			if (awin) {
				awin->render()->reload();
				// Note: 这里很奇怪，因为绘图表面反应迟钝，`ANativeWindow_getWidth()` 返回值可能有延时，
				// 但调用eglSwapBuffers()会刷新绘图表面。
				// awin->render()->surface()->renderDisplay(); // redrawing
			}

			auto ori = (Orientation)Android::get_orientation();
			if ((ori != manager->_currentOrientation)) {
				manager->_currentOrientation = ori;
				manager->_host->triggerOrientation();
			}
		}

	private:
		Application::Inl* _host = nullptr;
		ANativeActivity* _activity = nullptr;
		ANativeWindow* _window = nullptr;
		AInputQueue* _queue = nullptr;
		ALooper* _looper = nullptr;
		Orientation _currentOrientation = Orientation::ORIENTATION_INVALID;
		Rect _rect;
		friend class Window;
	};

	static bool toTouch(AInputEvent* motionEvent, int pointerIndex, TouchPoint* out) {
		Vec2 scale = manager->activeWindow()->scale();
		float left = manager->_rect.origin.x();
		int id = AMotionEvent_getPointerId(motionEvent, pointerIndex);
		float x = AMotionEvent_getX(motionEvent, pointerIndex) - left;
		float y = AMotionEvent_getY(motionEvent, pointerIndex);
		float pressure = AMotionEvent_getPressure(motionEvent, pointerIndex);
		float h_x = AMotionEvent_getHistoricalX(motionEvent, pointerIndex, 0);
		float h_y = AMotionEvent_getHistoricalY(motionEvent, pointerIndex, 0);
		*out = {
			uint32_t(id + 20170820),
			0, 0,
			x / scale.x(),
			y / scale.y(),
			pressure,
			false,
			nullptr,
		};
		return x != h_x || y != h_y;
	}

	static List<TouchPoint> toTouchList(AInputEvent* motionEvent, bool filter) {
		List<TouchPoint> rv;
		TouchPoint touch;
		int count = AMotionEvent_getPointerCount(motionEvent);

		for (int i = 0; i < count; i++) {
			if ( filter ) {
				if ( toTouch(motionEvent, i, &touch) ) {
					rv.push(touch);
				}
			} else {
				toTouch(motionEvent, i, &touch);
				rv.push(touch);
			}
		}
		Qk_ReturnLocal(rv);
	}

	static void dispatchEvent(AInputEvent* event) {
		auto awin = Manager::activeWindow();
		if (!awin) return;
		auto dispatch = awin->dispatch();
		int type = AInputEvent_getType(event);
		int device = AInputEvent_getDeviceId(event);
		int source = AInputEvent_getSource(event);

		if (type == AINPUT_EVENT_TYPE_KEY) {
			int code = AKeyEvent_getKeyCode(event);
			int repeat = AKeyEvent_getRepeatCount(event);
			int action = AKeyEvent_getAction(event);

			Qk_DLog("code:%d, repeat:%d, action:%d, "
											"flags:%d, scancode:%d, metastate:%d, downtime:%ld, time:%ld",
							code, repeat, action,
							AKeyEvent_getFlags(event),
							AKeyEvent_getScanCode(event),
							AKeyEvent_getMetaState(event),
							AKeyEvent_getDownTime(event),
							AKeyEvent_getEventTime(event)
			);

			switch (action) {
				case AKEY_EVENT_ACTION_DOWN:
					if (code)
						dispatch->keyboard_adapter()->dispatch(code, 0, 1, repeat, device, source);
					break;
				case AKEY_EVENT_ACTION_UP:
					if (code)
						dispatch->keyboard_adapter()->dispatch(code, 0, 0, repeat, device, source);
					break;
				case AKEY_EVENT_ACTION_MULTIPLE:
					Qk_DLog("AKEY_EVENT_ACTION_MULTIPLE");
					break;
			}
		}
		else { // AINPUT_EVENT_TYPE_MOTION
			int action = AMotionEvent_getAction(event);
			int pointerIndex = action >> AmotionEvent_ACTION_pointerIndex_SHIFT;

			List<TouchPoint> touchs;
			TouchPoint touch;

			switch (action & AmotionEvent_ACTION_MASK) {
				case AmotionEvent_ACTION_DOWN:
				case AmotionEvent_ACTION_POINTER_DOWN:
					toTouch(event, pointerIndex, &touch);
					touchs.push(touch);
					dispatch->dispatch_touchstart( std::move(touchs) );
					break;
				case AmotionEvent_ACTION_UP:
				case AmotionEvent_ACTION_POINTER_UP:
					toTouch(event, pointerIndex, &touch);
					touchs.push(touch);
					dispatch->dispatch_touchend( std::move(touchs) );
					break;
				case AmotionEvent_ACTION_MOVE:
					touchs = toTouchList(event, true);
					if ( touchs.length() ) {
						// Qk_DLog("AmotionEvent_ACTION_MOVE, %d", touchs.length());
						dispatch->dispatch_touchmove( std::move(touchs) );
					}
					break;
				case AmotionEvent_ACTION_CANCEL:
					dispatch->dispatch_touchcancel(toTouchList(event, false));
					break;
			}
		}
	}

	void Window::openImpl(Options &opts) {
		manager->post_messate_main(Cb([this](auto e) {
			set_backgroundColor(opts.backgroundColor);
			activate();
		}), true);
		_impl = manager;
	}

	void Window::closeImpl() {
		if (!_host->activeWindow()) {
			ANativeActivity_finish(manager->_activity);
		}
		_impl = nullptr;
	}

	float Window::getDefaultScale() {
		float defaultScale = Android::get_display_scale();
		return defaultScale;
	}

	void Window::set_backgroundColor(Color val) {
		// TODO ...
		// _backgroundColor = val;
	}

	void Window::activate() {
		auto awin = _host->activeWindow();
		if (awin == this)
			return;
		manager->post_messate_main(Cb([awin,this](auto e) {
			manager->exitWindow(awin);
			manager->enterWindow(this);
		}), true);
		_host->setActiveWindow(this);
	}

	void Window::pending() {
		ANativeActivity_finish(manager->_activity);
	}

	void Window::setFullscreen(bool fullscreen) {
		Android::set_fullscreen(fullscreen);
	}

	void Window::setCursorStyle(CursorStyle cursor, bool low) {
		// Noop
	}
}

extern "C" {

	Qk_Export void Java_org_quark_Activity_onStatucBarVisibleChange(JNIEnv* env, jclass clazz) {
		// Noop
	}

	Qk_Export void ANativeActivity_onCreate(ANativeActivity* activity, 
																					void* savedState, size_t savedStateSize)
	{
		Manager::onCreate(activity, savedState, savedStateSize);
	}
}
