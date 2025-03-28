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

#include "../../ui/app.h"
#include "../../ui/window.h"
#include "../../ui/screen.h"
#include "../../ui/event.h"
#include "../../render/linux/linux_render.h"
#include "./android.h"

#include <android/native_activity.h>
#include <android/native_window.h>

namespace qk {

	typedef TouchEvent::TouchPoint TouchPoint;
	typedef Screen::Orientation Orientation;
	class WindowImpl {};
	class SharedWindowManager;
	static SharedWindowManager *swm = nullptr;

	static void dispatchEvent(AInputEvent* event);

	class SharedWindowManager: public WindowImpl {
		Application::Inl* _host = nullptr;
		ANativeActivity* _activity = nullptr;
		ANativeWindow* _window = nullptr;
		AInputQueue* _queue = nullptr;
		ALooper* _looper = nullptr;
		Orientation _currentOrientation = Orientation::kInvalid;
		friend class Window;
	public:
		Qk_DEFINE_PROP_GET(Rect, displayRect);

		SharedWindowManager() {
			Qk_ASSERT_EQ(swm, nullptr);
			swm = this;
			_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
		}

		inline RunLoop* loop() {
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

		static Window* activeWindow() {
			return swm->_host->activeWindow();
		}

		static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
			if ( queue != swm->_queue ) {
				if ( swm->_queue ) {
					AInputQueue_detachLooper(swm->_queue);
				}
				AInputQueue_attachLooper(queue, swm->_looper, ALOOPER_POLL_CALLBACK,
				(ALooper_callbackFunc)[](int fd, int events, void* q) -> int {
					AInputQueue* queue = (AInputQueue*)q;
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
				swm->_queue = queue;
			}
		}

		static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
			AInputQueue_detachLooper(queue);
			swm->_queue = nullptr;
		}

		static void onLowMemory(ANativeActivity* activity) {
			Qk_DLog("onLowMemory");
			swm->_host->triggerMemorywarning();
		}

		static void onResume(ANativeActivity* activity) {
			swm->_host->triggerResume();
		}

		static void onPause(ANativeActivity* activity) {
			swm->_host->triggerPause();
		}

		static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
			return nullptr;
		}
		
		static void onConfigurationChanged(ANativeActivity* activity) {
			Qk_DLog("onConfigurationChanged");
		}

		static void onCreate(ANativeActivity* activity, void* saved_state, size_t saved_state_size) {
			if ( !swm ) {
				new SharedWindowManager();
			}
			Qk_ASSERT_EQ(swm->_activity, nullptr);
			// ANativeActivity_setWindowFlags(activity, 0x00000400, 0);

			activity->callbacks->onDestroy                  = &SharedWindowManager::onDestroy;
			activity->callbacks->onStart                    = &SharedWindowManager::onStart;
			activity->callbacks->onResume                   = &SharedWindowManager::onResume;
			activity->callbacks->onSaveInstanceState        = &SharedWindowManager::onSaveInstanceState;
			activity->callbacks->onPause                    = &SharedWindowManager::onPause;
			activity->callbacks->onStop                     = &SharedWindowManager::onStop;
			activity->callbacks->onConfigurationChanged     = &SharedWindowManager::onConfigurationChanged;
			activity->callbacks->onLowMemory                = &SharedWindowManager::onLowMemory;
			activity->callbacks->onWindowFocusChanged       = &SharedWindowManager::onWindowFocusChanged;
			activity->callbacks->onNativeWindowCreated      = &SharedWindowManager::onNativeWindowCreated;
			activity->callbacks->onNativeWindowResized      = &SharedWindowManager::onNativeWindowResized;
			activity->callbacks->onNativeWindowRedrawNeeded = &SharedWindowManager::onNativeWindowRedrawNeeded;
			activity->callbacks->onNativeWindowDestroyed    = &SharedWindowManager::onNativeWindowDestroyed;
			activity->callbacks->onInputQueueCreated        = &SharedWindowManager::onInputQueueCreated;
			activity->callbacks->onInputQueueDestroyed      = &SharedWindowManager::onInputQueueDestroyed;
			activity->callbacks->onContentRectChanged       = &SharedWindowManager::onContentRectChanged;
			activity->instance = swm;

			swm->_activity = activity;
		}

		static void onDestroy(ANativeActivity* activity) {
			Qk_ASSERT_NE(swm->_activity, nullptr);

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

			swm->_activity = nullptr;
		}

		static void onStart(ANativeActivity* activity) {
			if ( swm->_host == nullptr ) { // start gui
				Application::runMain(0, nullptr); // run gui application

				swm->_host = Inl_Application(shared_app());

				Qk_ASSERT(swm->_host);
				Qk_ASSERT_EQ(swm->_activity, activity);

				swm->_host->triggerLoad();
			}

			//auto awin = activeWindow();
			//if (awin)
			//	swm->_host->triggerForeground(awin);
		}

		static void onStop(ANativeActivity* activity) {
			Qk_DLog("onStop");
			//auto awin = activeWindow();
			//if (awin)
			//	swm->_host->triggerBackground(awin);
		}

		// ----------------------------------------------------------------------

		static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
			Qk_ASSERT_NE(swm->_window, nullptr);
			// ANativeWindow_setBuffersGeometry(window, 800, 480, WINDOW_FORMAT_RGBX_8888);
			swm->_window = window;
			swm->enterWindow(activeWindow());
		}

		static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
			swm->_window = nullptr;
			swm->exitWindow(activeWindow());
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
			swm->_displayRect = {
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
			if ((ori != swm->_currentOrientation)) {
				swm->_currentOrientation = ori;
				swm->_host->triggerOrientation();
			}
		}
	};

	static bool convertToTouch(AInputEvent* motionEvent, int pointerIndex, TouchPoint* out) {
		Vec2 scale = swm->activeWindow()->scale();
		float left = swm->displayRect().origin.x();
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

	static List<TouchPoint> convertToTouchList(AInputEvent* motionEvent, bool filter) {
		List<TouchPoint> rv;
		TouchPoint touch;
		int count = AMotionEvent_getPointerCount(motionEvent);

		for (int i = 0; i < count; i++) {
			if ( filter ) {
				if ( convertToTouch(motionEvent, i, &touch) ) {
					rv.pushBack(touch);
				}
			} else {
				convertToTouch(motionEvent, i, &touch);
				rv.pushBack(touch);
			}
		}
		Qk_ReturnLocal(rv);
	}

	static void dispatchEvent(AInputEvent* event) {
		auto awin = SharedWindowManager::activeWindow();
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
						dispatch->keyboard()->dispatch(code, 0, 1, false, repeat, device, source);
					break;
				case AKEY_EVENT_ACTION_UP:
					if (code)
						dispatch->keyboard()->dispatch(code, 0, 0, false, repeat, device, source);
					break;
				case AKEY_EVENT_ACTION_MULTIPLE:
					Qk_DLog("AKEY_EVENT_ACTION_MULTIPLE");
					break;
			}
		}
		else { // AINPUT_EVENT_TYPE_MOTION
			int action = AMotionEvent_getAction(event);
			int pointerIndex = action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

			List<TouchPoint> touchs;
			TouchPoint touch;

			switch (action & AMOTION_EVENT_ACTION_MASK) {
				case AMOTION_EVENT_ACTION_DOWN:
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
					convertToTouch(event, pointerIndex, &touch);
					touchs.pushBack(touch);
					dispatch->onTouchstart( std::move(touchs) );
					break;
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_POINTER_UP:
					convertToTouch(event, pointerIndex, &touch);
					touchs.pushBack(touch);
					dispatch->onTouchend( std::move(touchs) );
					break;
				case AMOTION_EVENT_ACTION_MOVE:
					touchs = convertToTouchList(event, true);
					if ( touchs.length() ) {
						// Qk_DLog("AMOTION_EVENT_ACTION_MOVE, %d", touchs.length());
						dispatch->onTouchmove( std::move(touchs) );
					}
					break;
				case AMOTION_EVENT_ACTION_CANCEL:
					dispatch->onTouchcancel(convertToTouchList(event, false));
					break;
			}
		}
	}

	void Window::openImpl(Options &opts) {
		swm->post_messate_main(Cb([&](auto e) {
			set_backgroundColor(opts.backgroundColor);
			activate();
		}), true);
		_impl = swm;
	}

	void Window::closeImpl() {
		if (!_host->activeWindow()) {
			ANativeActivity_finish(swm->_activity);
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
		swm->post_messate_main(Cb([awin,this](auto e) {
			swm->exitWindow(awin);
			swm->enterWindow(this);
		}), true);
		Inl_Application(_host)->setActiveWindow(this);
	}

	void Window::pending() {
		ANativeActivity_finish(swm->_activity);
	}

	void Window::setFullscreen(bool fullscreen) {
		Android::set_fullscreen(fullscreen);
	}

	void Window::setCursorStyle(CursorStyle cursor, bool low) {
		// Noop
	}
}

extern "C" {

	Qk_EXPORT void Java_org_quark_Activity_onStatucBarVisibleChange(JNIEnv* env, jclass clazz) {
		// Noop
	}

	Qk_EXPORT void ANativeActivity_onCreate(ANativeActivity* activity, 
																					void* savedState, size_t savedStateSize)
	{
		qk::SharedWindowManager::onCreate(activity, savedState, savedStateSize);
	}
}
