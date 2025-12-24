/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "../../ui/ui.h"
#include "../../render/linux/linux_render.h"
#include "../../render/canvas.h"
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
		friend class Window;
		AppInl* _host = nullptr;
		ANativeActivity* _activity = nullptr;
		ANativeWindow* _window = nullptr;
		AInputQueue* _queue = nullptr;
		ALooper* _looper = nullptr;
		Orientation _currentOrientation = Orientation::kInvalid;
		List<Cb> _msg;
		Mutex _msgMutex;
		Window *_active = nullptr;
	public:
		Qk_DEFINE_PROP_GET(Range, displayRange);

		const ThreadID main_thread_id = thread_self_id();

		SharedWindowManager() {
			Qk_ASSERT_EQ(swm, nullptr);
			swm = this;
			_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
		}

		void addMsg(Cb& cb) {
			_msgMutex.lock();
			_msg.pushBack(cb);
			_msgMutex.unlock();
			Android_resolve_msg_onmain();
		}

		void resolveMsg() {
			if (_msg.length()) {
				List<Cb> msg;
				_msgMutex.lock();
				if (_msg.length()) {
					msg = std::move(_msg);
				}
				_msgMutex.unlock();

				for (auto& i: msg) {
					i->resolve();
				}
			}
		}

		void renderDisplay() {
			if (_window && _active) {
				_active->render()->surface()->renderDisplay(); // redrawing
			}
		}

		void windowEnter() {
			if (_window && _active) {
				auto render = _active->render();
				render->surface()->makeSurface(_window);
				render->reload();
				render->surface()->renderDisplay();
			}
		}

		void windowExit() {
			if (_active) {
				_active->render()->surface()->deleteSurface();
			}
		}

		static Window* activeWindow() {
			return swm->_active;
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

		static void onConfigurationChanged(ANativeActivity* activity) {
			Qk_DLog("onConfigurationChanged");
		}

		static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
			return nullptr;
		}

		void initPlatform_AppInl(AppInl* host) {
			_host = host;
			_host->triggerLoad();
		}

		static void onCreate(ANativeActivity* activity, void* saved_state, size_t saved_state_size) {
			if (!swm) {
				new SharedWindowManager();
				Application::runMain(0, nullptr, false); // run gui application
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
			memset(activity->callbacks, 0, sizeof(*activity->callbacks));
			swm->_activity = nullptr;
		}

		static void onStart(ANativeActivity* activity) {
			Qk_ASSERT_EQ(swm->_activity, activity);
			auto awin = activeWindow();
			if (awin)
				swm->_host->triggerForeground(awin);
			Qk_DLog("triggerForeground");
		}

		static void onStop(ANativeActivity* activity) {
			Qk_ASSERT_EQ(swm->_activity, activity);
			auto awin = activeWindow();
			if (awin)
				swm->_host->triggerBackground(awin);
			Qk_DLog("triggerBackground");
		}

		static void onResume(ANativeActivity* activity) {
			Qk_ASSERT_EQ(swm->_activity, activity);
			if (swm->_host)
				swm->_host->triggerResume();
			Qk_DLog("triggerResume");
		}

		static void onPause(ANativeActivity* activity) {
			Qk_ASSERT_EQ(swm->_activity, activity);
			if (swm->_host)
				swm->_host->triggerPause();
			Qk_DLog("triggerPause");
		}

		// ----------------------------------------------------------------------

		static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
			Qk_ASSERT_EQ(swm->_window, nullptr);
			// ANativeWindow_setBuffersGeometry(window, 800, 480, WINDOW_FORMAT_RGBX_8888);
			swm->_window = window;
			swm->windowEnter();
		}

		static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
			Qk_ASSERT_EQ(swm->_window, window);
			swm->windowExit();
			swm->_window = nullptr;
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
			swm->_displayRange = {Vec2(rect->left,rect->top),Vec2(rect->right,rect->bottom)};

			auto awin = activeWindow();
			if (awin) {
				awin->render()->reload();
			}

			auto ori = (Orientation)Android_get_orientation();
			if (ori != swm->_currentOrientation) {
				swm->_currentOrientation = ori;
				if (swm->_host)
					swm->_host->triggerOrientation();
			}
		}
	};

	void post_messate_main(Cb cb, bool sync) {
		Qk_ASSERT_NE(swm, nullptr);
		if (swm->main_thread_id == thread_self_id()) {
			cb->resolve();
		} else if (sync) {
			CondMutex mutex;
			Cb cb1([&cb, &mutex](auto e) {
				cb->resolve();
				mutex.lock_and_notify_one();
			});
			swm->addMsg(cb1);
			mutex.lock_and_wait_for(); // wait
		} else {
			swm->addMsg(cb);
		}
	}

	static bool convertToTouch(AInputEvent* motionEvent, int pointerIndex, TouchPoint* out) {
		Vec2 scale = swm->activeWindow()->scale();
		float left = swm->displayRange().begin.x();
		int id = AMotionEvent_getPointerId(motionEvent, pointerIndex);
		float x = AMotionEvent_getX(motionEvent, pointerIndex) - left;
		float y = AMotionEvent_getY(motionEvent, pointerIndex);
		float pressure = AMotionEvent_getPressure(motionEvent, pointerIndex);
		float h_x = AMotionEvent_getHistoricalX(motionEvent, pointerIndex, 0);
		float h_y = AMotionEvent_getHistoricalY(motionEvent, pointerIndex, 0);
		*out = {
			uint32_t(id + 20170820),
			{0, 0},
			{x / scale.x(),
			y / scale.y()},
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
		if (!awin)
			return;
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

	void AppInl::initPlatform() {
		swm->initPlatform_AppInl(this);
	}

	void Window::openImpl(Options &opts) {
		post_messate_main(Cb([&](auto e) {
			_impl = swm;
			set_backgroundColor(opts.backgroundColor);
			activate();
		}), true);
	}

	void Window::beforeClose() {
		post_messate_main(Cb([this](auto e) {
			Qk_ASSERT_NE(_impl, nullptr);
			swm->windowExit();
			swm->_active = nullptr;
			_impl = nullptr;
		}), true);
	}

	void Window::closeImpl() {
		if (!_host->activeWindow()) {
			pending();
		}
	}

	float Window::getDefaultScale() {
		return Android_get_display_scale();
	}

	Range Window::getDisplayRange(Vec2 size) {
		auto range = swm->displayRange();
		auto rangeNew = _opts.navigationColor.a() == 0 ? Range{0,size}: range;
		float scale = getDefaultScale();

		if (_lockSize.x() != 0) { // lock width
			scale = (rangeNew.end.x() - rangeNew.begin.x()) / _lockSize.x();
		}
		else if (_lockSize.y() != 0) { // lock height
			scale = (rangeNew.end.y() - rangeNew.begin.y()) / _lockSize.y();
		}

		// Navigation button range for Android
		if (range.end.y() != size.y()) { // bottom
			_navigationRect = {
				Vec2{0,range.end.y()}/scale,
				Vec2{size.x(), size.y()-range.end.y()}/scale
			};
		}
		else if (range.begin.x() != 0) { // left
			_navigationRect = {
				Vec2{0,0},
				Vec2{range.begin.x(), size.y()}/scale
			};
		}
		else if (range.end.x() != size.x()) { // right
			_navigationRect = {
				Vec2{range.end.x(),0}/scale,
				Vec2{size.x()-range.end.x(), size.y()}/scale
			};
		} else {
			_navigationRect = {};
		}

		return rangeNew;
	}

	void Window::afterDisplay() {
		if (_opts.navigationColor.a() != 0 && !_navigationRect.size.is_zero_axis()) {
			Paint paint;
			paint.fill.color = _opts.navigationColor.to_color4f();
			paint.antiAlias = false;
			_render->getCanvas()->drawRect(_navigationRect, paint);
		}
	}

	void Window::set_backgroundColor(Color val) {
		// Noop action
		_backgroundColor = val;
	}

	void Window::activate() {
		auto awin = _host->activeWindow();
		if (awin == this) return;

		post_messate_main(Cb([awin](auto e) {
			swm->windowExit();
			swm->_active = nullptr;
		}, awin), false);

		post_messate_main(Cb([this](auto e) {
			swm->_active = this;
			swm->windowEnter();
		}, this), false);

		Inl_Application(_host)->setActiveWindow(this);
		if (awin)
			Inl_Application(_host)->triggerBackground(awin);
		Inl_Application(_host)->triggerForeground(this);
	}

	void Window::pending() {
		post_messate_main(Cb([](auto e) {
			ANativeActivity_finish(swm->_activity);
		}), false);
	}

	void Window::setFullscreen(bool fullscreen) {
		Android_set_fullscreen(fullscreen);
	}

	void Window::setCursorStyle(CursorStyle cursor, bool low) {
		// Noop
	}
}

extern "C" {

	Qk_EXPORT void Java_org_quark_Android_onResolveMsg(JNIEnv* env, jclass clazz) {
		Qk_ASSERT_NE(qk::swm, nullptr);
		qk::swm->resolveMsg();
	}

	Qk_EXPORT void Java_org_quark_Activity_onRenderDisplay(JNIEnv* env, jclass clazz) {
		Qk_ASSERT_NE(qk::swm, nullptr);
		qk::swm->renderDisplay();
	}

	Qk_EXPORT void Java_org_quark_Activity_onStatucBarVisibleChange(JNIEnv* env, jclass clazz) {
		// Noop
	}

	Qk_EXPORT void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
	{
		qk::SharedWindowManager::onCreate(activity, savedState, savedStateSize);
	}
}
