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
#include "android/android.h"
#include "../app-1.h"
#include "../display-port.h"
#include "../event.h"
#include "android-ogl-1.h"
#include <android/native_activity.h>
#include "ngui/base/os/android-jni.h"

XX_NS(ngui)

class AndroidApplication;
static AndroidApplication* android_app = nullptr;
static AndroidGLDrawCore* android_draw_core = nullptr;
typedef GUIApplication::Inl AppInl;
typedef DisplayPort::Orientation Orientation;

class AndroidApplication {
public:
  typedef NonObjectTraits Traits;

  AndroidApplication()
  : m_activity(nullptr)
  , m_window(nullptr)
  , m_host(nullptr)
  , m_queue(nullptr)
  , m_looper(nullptr)
  , m_dispatch( nullptr )
  , m_render_loop_id(0)
  , m_current_orientation(Orientation::ORIENTATION_INVALID)
  , m_is_init_ok(false)
  {
    XX_ASSERT(!android_app);
    android_app = this;
    m_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  }

  // ----------- Render ----------

  static void render_task(Se& ev, Uint* id) {
    auto self = android_app;
    if ( id->value == self->m_render_loop_id ) {
      self->m_host->render_loop()->
        post(self->m_render_task_cb, 1000.0 / 60.0 * 1000); // 60fsp
      self->m_host->onRender();
    } else {
      Release(id);
    }
  }

  void start_render_task() {
    if ( Android::is_screen_on() ) {
      m_host->render_loop()->post(Cb([this](Se &ev) {
        ScopeLock scope(m_mutex);
        if (m_window && !m_render_loop_id) {
          Uint *id = new Uint(iid32());
          m_render_loop_id = id->value;
          m_render_task_cb = Cb(&render_task, id);
          render_task(ev, id);
        }
      }));
    }
  }

  void stop_render_task(bool immediately = false /*如果要立即停止,必须在在渲染循环调用*/) {
    if (immediately) {
      m_render_loop_id = 0;
    } else {
      m_host->render_loop()->post(Cb([this](Se& ev) {
        m_render_loop_id = 0;
      }));      
    }
  }

  inline Orientation orientation() const { return m_current_orientation; }

  inline AppInl* host() const { return m_host; }

  // ---------- static ----------

  static void onCreate(ANativeActivity* activity, void* saved_state, size_t saved_state_size) {
    if ( !android_app ) {
      new AndroidApplication();
    }
    // ANativeActivity_setWindowFlags(activity, 0x00000400, 0);

    activity->callbacks->onDestroy                  = &AndroidApplication::onDestroy;
    activity->callbacks->onStart                    = &AndroidApplication::onStart;
    activity->callbacks->onResume                   = &AndroidApplication::onResume;
    activity->callbacks->onSaveInstanceState        = &AndroidApplication::onSaveInstanceState;
    activity->callbacks->onPause                    = &AndroidApplication::onPause;
    activity->callbacks->onStop                     = &AndroidApplication::onStop;
    activity->callbacks->onConfigurationChanged     = &AndroidApplication::onConfigurationChanged;
    activity->callbacks->onLowMemory                = &AndroidApplication::onLowMemory;
    activity->callbacks->onWindowFocusChanged       = &AndroidApplication::onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated      = &AndroidApplication::onNativeWindowCreated;
    activity->callbacks->onNativeWindowResized      = &AndroidApplication::onNativeWindowResized;
    activity->callbacks->onNativeWindowRedrawNeeded = &AndroidApplication::onNativeWindowRedrawNeeded;
    activity->callbacks->onNativeWindowDestroyed    = &AndroidApplication::onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated        = &AndroidApplication::onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed      = &AndroidApplication::onInputQueueDestroyed;
    activity->callbacks->onContentRectChanged       = &AndroidApplication::onContentRectChanged;
    activity->instance = android_app;

    android_app->m_activity = activity;
  }

  void pending() {
    ANativeActivity_finish(m_activity);
  }

  static void onDestroy(ANativeActivity* activity) {
    XX_ASSERT(android_app->m_activity);

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

    android_app->m_activity = nullptr;

    // android_app->m_app->onUnload();
    // delete android_app; android_app = nullptr;
  }

  static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    ScopeLock scope(android_app->m_mutex);
    android_app->m_window = window;
    android_app->m_host->render_loop()->post(Cb([window](Se &ev) {
      bool ok = false;
      { //
        ScopeLock scope(android_app->m_mutex);
        if ( window == android_app->m_window ) {
          ok = android_draw_core->create_surface(android_app->m_window);
        }
      }
      if ( ok ) {
        if ( android_app->m_is_init_ok ) {
          android_draw_core->refresh_surface_size(nullptr);
          android_app->m_host->refresh_display(); // 更新画面
        } else {
          android_app->m_is_init_ok = true;
          android_draw_core->initialize();
          android_app->m_host->onLoad();
        }
        android_app->start_render_task();
      }
    }));
  }
  
  static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    ScopeLock scope(android_app->m_mutex);
    android_app->m_window = nullptr;
    android_app->m_host->render_loop()->post(Cb([window](Se& ev) {
      android_draw_core->destroyed_surface(window);
      android_app->stop_render_task(true);
    }));
  }

  static void onStart(ANativeActivity* activity) {
    
    if ( android_app->m_host == nullptr ) { // start gui
      AppInl::run_gui_application(0, nullptr); // run gui application

      android_app->m_host = Inl_GUIApplication(app());
      android_app->m_dispatch = android_app->m_host->dispatch();

      XX_ASSERT(android_app->m_activity);
      XX_ASSERT(android_app->m_host);
      XX_ASSERT(android_app->m_host->render_loop());
    }
    android_app->m_host->onForeground();
    android_app->stop_render_task();
  }
  
  static void onResume(ANativeActivity* activity) {
    android_app->m_host->onResume();
  }
  
  static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    return nullptr;
  }
  
  static void onPause(ANativeActivity* activity) {
    android_app->m_host->onPause();
  }
  
  static void onStop(ANativeActivity* activity) {
    android_app->m_host->onBackground();
    android_app->stop_render_task();
  }
  
  static void onConfigurationChanged(ANativeActivity* activity) {
    XX_DEBUG("onConfigurationChanged");
  }
  
  static void onLowMemory(ANativeActivity* activity) {
    XX_DEBUG("onLowMemory");
    android_app->m_host->onMemorywarning();
  }
  
  static void onWindowFocusChanged(ANativeActivity* activity, int hasFocus) {
    XX_DEBUG("onWindowFocusChanged");
    android_app->start_render_task();
  }
  
  static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
    XX_DEBUG("onNativeWindowResized");
  }

  static void onContentRectChanged(ANativeActivity* activity, const ARect* rect) {
    /*
     * 这里做Rect选择绘制，主要因为android的虚拟导航按键，
     * 虚拟导航按键区域绘制黑色背景颜色填充
     */
    android_app->m_rect = {
      Vec2(rect->left, 0),
      Vec2(rect->right - rect->left, rect->bottom)
    };

    // 屏幕旋转都会触发这个事件，所以不做更多的监听工作
    Orientation orientation = (Orientation)Android::get_orientation();
    int targger_orientation = (orientation != android_app->m_current_orientation);
    if ( targger_orientation ) { // 屏幕方向改变
      android_app->m_current_orientation = orientation;
    }

    android_app->m_host->render_loop()->post(Cb([targger_orientation](Se &ev) {
      // 这里有点奇怪，因为绘图表面反应迟钝，
      // 也就是说 `ANativeWindow_getWidth()` 返回值可能与当前真实值不相同，
      // 但调用eglSwapBuffers()会刷新绘图表面。
      android_app->m_host->refresh_display(); // 刷新绘图表面
      android_draw_core->refresh_surface_size(&android_app->m_rect);

      if ( targger_orientation ) { // 触发方向变化事件
        android_app->m_host->main_loop()->post(Cb([](Se& e) {
          android_app->m_host->display_port()->XX_TRIGGER(orientation);
        }));
      }
    }));
  }

  static void onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
    XX_DEBUG("onNativeWindowRedrawNeeded");
    android_app->start_render_task();
  }

  // --------------------------- Dispatch event ---------------------------

  static bool get_gui_touch(AInputEvent* motion_event, int pointer_index, GUITouch* out) {
    Vec2 scale = android_app->m_host->display_port()->scale();
    float left = android_app->m_rect.origin.x();

    int id = AMotionEvent_getPointerId(motion_event, pointer_index);
    float x = AMotionEvent_getX(motion_event, pointer_index) - left;
    float y = AMotionEvent_getY(motion_event, pointer_index);
    float pressure = AMotionEvent_getPressure(motion_event, pointer_index);

    *out = {
            uint(id + 20170820),
            0, 0,
            x / scale.x(),
            y / scale.y(),
            pressure,
            false,
            nullptr,
    };

    float h_x = AMotionEvent_getHistoricalX(motion_event, pointer_index, 0);
    float h_y = AMotionEvent_getHistoricalY(motion_event, pointer_index, 0);
    return x != h_x || y != h_y;
  }

  static List<GUITouch> to_gui_touchs(AInputEvent* motion_event, bool filter) {
    List<GUITouch> rv;
    GUITouch touch;
    int count = AMotionEvent_getPointerCount(motion_event);

    for (int i = 0; i < count; i++) {
      if ( filter ) {
        if ( get_gui_touch(motion_event, i, &touch) ) {
          rv.push(touch);
        }
      } else {
        get_gui_touch(motion_event, i, &touch);
        rv.push(touch);
      }
    }
    return rv;
  }

  static void dispatch_event(AInputEvent* event) {
    GUIEventDispatch* dispatch = android_app->m_dispatch;

    int type = AInputEvent_getType(event);
    int device = AInputEvent_getDeviceId(event);
    int source = AInputEvent_getSource(event);

    if (type == AINPUT_EVENT_TYPE_KEY)
    {
      int code = AKeyEvent_getKeyCode(event);
      int repeat = AKeyEvent_getRepeatCount(event);
      int action = AKeyEvent_getAction(event);

      XX_DEBUG("code:%d, repeat:%d, action:%d, "
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
          XX_DEBUG("AKEY_EVENT_ACTION_MULTIPLE");
          break;
      }
    }
    else
    { // AINPUT_EVENT_TYPE_MOTION
      int action = AMotionEvent_getAction(event);
      int pointer_index = action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

      List<GUITouch> touchs;
      GUITouch touch;

      switch (action & AMOTION_EVENT_ACTION_MASK) {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
          get_gui_touch(event, pointer_index, &touch);
          touchs.push(touch);
          dispatch->dispatch_touchstart( move(touchs) );
          break;
        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_POINTER_UP:
          get_gui_touch(event, pointer_index, &touch);
          touchs.push(touch);
          dispatch->dispatch_touchend( move(touchs) );
          break;
        case AMOTION_EVENT_ACTION_MOVE:
          touchs = to_gui_touchs(event, true);
          if ( touchs.length() ) {
            // XX_DEBUG("AMOTION_EVENT_ACTION_MOVE, %d", touchs.length());
            dispatch->dispatch_touchmove( move(touchs) );
          }
          break;
        case AMOTION_EVENT_ACTION_CANCEL:
          dispatch->dispatch_touchcancel( to_gui_touchs(event, false) );
          break;
      }
    }
  }

  static int onInputEvent_callback(int fd, int events, AInputQueue* queue) {
    AInputEvent* event = nullptr;

    while( AInputQueue_getEvent(queue, &event) >= 0 ) {
      if (AInputQueue_preDispatchEvent(queue, event) == 0) {
        dispatch_event(event);
        AInputQueue_finishEvent(queue, event, fd);
      } else {
        XX_DEBUG("AInputQueue_preDispatchEvent(queue, event) != 0");
      }
    }
    return 1;
  }

  static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    if ( queue != android_app->m_queue ) {
      if ( android_app->m_queue ) {
        AInputQueue_detachLooper(android_app->m_queue);
      }
      AInputQueue_attachLooper(queue, android_app->m_looper,
                               ALOOPER_POLL_CALLBACK,
                               (ALooper_callbackFunc) &onInputEvent_callback, queue);
      android_app->m_queue = queue;
    }
  }
  
  static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    AInputQueue_detachLooper(queue);
    android_app->m_queue = nullptr;
  }

private:
  ANativeActivity* m_activity;
  ANativeWindow* m_window;
  AppInl* m_host;
  AInputQueue* m_queue;
  ALooper* m_looper;
  GUIEventDispatch* m_dispatch;
  Callback m_render_task_cb;
  uint m_render_loop_id;
  Orientation m_current_orientation;
  CGRect m_rect;
  Mutex m_mutex;
  bool m_is_init_ok;
};

/**
 * @func open_url()
 */
void GUIApplication::open_url(cString& url) {
  Android::open_url(url);
}

/**
 * @func send_email
 */
void GUIApplication::send_email(cString& recipient,
                                cString& subject,
                                cString& cc, cString& bcc, cString& body) {
  Android::send_email(recipient, subject, cc, bcc, body);
}


void AppInl::initialize(const Map<String, int>& options) {
  XX_ASSERT(!android_draw_core);
  android_draw_core = AndroidGLDrawCore::create(this, options);
  m_draw_ctx = android_draw_core->host();
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
   Android::ime_keyboard_open(options.is_clear, int(options.type), int(options.return_type));
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
   Android::ime_keyboard_can_backspace(can_backspace, can_delete);
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
   Android::ime_keyboard_close();
}

/**
 * @func pending() 挂起应用进程
 */
void GUIApplication::pending() {
  if ( android_app ) {
    android_app->pending();
  }
}

/**
 * @func set_volume_up()
 */
void AppInl::set_volume_up() {
  Android::set_volume_up();
}

/**
 * @func set_volume_down()
 */
void AppInl::set_volume_down() {
  Android::set_volume_down();
}

/**
* @func keep_screen(keep)
*/
void DisplayPort::keep_screen(bool keep) {
  Android::keep_screen(keep);
}

/**
* @func status_bar_height()
*/
float DisplayPort::status_bar_height() {
  return Android::get_status_bar_height() / m_scale_value[1];
}

/**
* @func set_visible_status_bar(visible)
*/
void DisplayPort::set_visible_status_bar(bool visible) {
  Android::set_visible_status_bar(visible);
}

/**
* @func set_status_bar_text_color(color)
*/
void DisplayPort::set_status_bar_style(StatusBarStyle style) {
  Android::set_status_bar_style(style);
}

/**
* @func request_fullscreen(fullscreen)
*/
void DisplayPort::request_fullscreen(bool fullscreen) {
  Android::request_fullscreen(fullscreen);
}

/**
* @func orientation()
*/
Orientation DisplayPort::orientation() {
  Orientation r = android_app->orientation();
  if ( r == ORIENTATION_INVALID )
    r = (Orientation)Android::get_orientation();
  if ( r >= ORIENTATION_INVALID && r <= ORIENTATION_REVERSE_LANDSCAPE ) {
    return r;
  }
  return ORIENTATION_INVALID;
}

/**
 * @func set_orientation(orientation)
 */
void DisplayPort::set_orientation(Orientation orientation) {
  Android::set_orientation(int(orientation));
}

extern "C" {

  XX_EXPORT void Java_org_ngui_IMEReceiver_dispatchIMEDelete(JNIEnv* env, jclass clazz, jint count) {
    _inl_app(app())->dispatch()->dispatch_ime_delete(count);
  }

  XX_EXPORT void Java_org_ngui_IMEReceiver_dispatchIMEInsert(JNIEnv* env, jclass clazz, jstring text) {
    _inl_app(app())->dispatch()->dispatch_ime_insert(JNI::jstring_to_string(text));
  }

  XX_EXPORT void Java_org_ngui_IMEReceiver_dispatchIMEMarked(JNIEnv* env, jclass clazz, jstring text) {
    _inl_app(app())->dispatch()->dispatch_ime_marked(JNI::jstring_to_string(text));
  }

  XX_EXPORT void Java_org_ngui_IMEReceiver_dispatchIMEUnmark(JNIEnv* env, jclass clazz, jstring text) {
    _inl_app(app())->dispatch()->dispatch_ime_unmark(JNI::jstring_to_string(text));
  }
  
  XX_EXPORT void Java_org_ngui_IMEReceiver_dispatchKeyboardInput(JNIEnv* env, jclass clazz,
    jint keycode, jboolean ascii, jboolean down, jint repeat, jint device, jint source) {
    _inl_app(app())->dispatch()->keyboard_adapter()->dispatch(keycode, ascii,
                                                              down, repeat, device, source);
  }

  XX_EXPORT void Java_org_ngui_NGUIActivity_onStatucBarVisibleChange(JNIEnv* env, jclass clazz) {
    android_app->host()->main_loop()->post(Cb([](Se& ev){
      android_app->host()->display_port()->XX_TRIGGER(change);
    }));
  }

  XX_EXPORT void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
  {
    AndroidApplication::onCreate(activity, savedState, savedStateSize);
  }
}

XX_END