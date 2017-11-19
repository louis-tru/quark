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

#ifndef __ngui__event__
#define __ngui__event__

#include "base/event.h"
#include "base/array.h"
#include "base/sys.h"
#include "mathe.h"
#include "value.h"
#include "keyboard.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

class GUIApplication;
class View;
class Action;
class Activity;
class Button;

// ---------------------------------------- EVENT TYPE ----------------------------------------

enum {
  GUI_EVENT_CATEGORY_DEFAULT,
  GUI_EVENT_CATEGORY_KEYBOARD,
  GUI_EVENT_CATEGORY_CLICK,
  GUI_EVENT_CATEGORY_HIGHLIGHTED,
  GUI_EVENT_CATEGORY_TOUCH,
  GUI_EVENT_CATEGORY_ACTION,
  GUI_EVENT_CATEGORY_FOCUS_MOVE,
};

class XX_EXPORT GUIEventName {
 public:
  inline GUIEventName() { XX_UNREACHABLE(); }
  inline GUIEventName(cString& n, uint category)
  : name_(n), code_(n.hash_code()), category_(category) { }
  inline uint hash_code() const { return code_; }
  inline bool equals(const GUIEventName& o) const { return o.hash_code() == code_; }
  inline String to_string() const { return name_; }
  inline uint category() const { return category_; }
  inline bool operator==(const GUIEventName& type) const { return type.code_ == code_; }
  inline bool operator!=(const GUIEventName& type) const { return type.code_ != code_; }
 private:
  String  name_;
  uint  code_, category_;
};

// can bubble event
XX_EXPORT extern const GUIEventName GUI_EVENT_CLICK;
XX_EXPORT extern const GUIEventName GUI_EVENT_BACK;
XX_EXPORT extern const GUIEventName GUI_EVENT_KEYDOWN;
XX_EXPORT extern const GUIEventName GUI_EVENT_KEYPRESS;
XX_EXPORT extern const GUIEventName GUI_EVENT_KEYUP;
XX_EXPORT extern const GUIEventName GUI_EVENT_KEYENTER;
XX_EXPORT extern const GUIEventName GUI_EVENT_TOUCHSTART;
XX_EXPORT extern const GUIEventName GUI_EVENT_TOUCHMOVE;
XX_EXPORT extern const GUIEventName GUI_EVENT_TOUCHEND;
XX_EXPORT extern const GUIEventName GUI_EVENT_TOUCHCANCEL;
XX_EXPORT extern const GUIEventName GUI_EVENT_FOCUS;
XX_EXPORT extern const GUIEventName GUI_EVENT_BLUR;
// canno bubble event
XX_EXPORT extern const GUIEventName GUI_EVENT_HIGHLIGHTED; // normal / hover / down
XX_EXPORT extern const GUIEventName GUI_EVENT_REMOVE_VIEW;
XX_EXPORT extern const GUIEventName GUI_EVENT_ACTION_KEYFRAME;
XX_EXPORT extern const GUIEventName GUI_EVENT_ACTION_LOOP;
XX_EXPORT extern const GUIEventName GUI_EVENT_FOCUS_MOVE;
XX_EXPORT extern const GUIEventName GUI_EVENT_SCROLL;
XX_EXPORT extern const GUIEventName GUI_EVENT_CHANGE;
//
XX_EXPORT extern const Map<String, GUIEventName> GUI_EVENT_TABLE;

// --------------------------------------------------------------------------------

enum HighlightedStatus {
  HIGHLIGHTED_NORMAL = 1,
  HIGHLIGHTED_HOVER,
  HIGHLIGHTED_DOWN,
};

enum ReturnValueMask {
  RETURN_VALUE_MASK_DEFAULT = (1 << 0),
  RETURN_VALUE_MASK_BUBBLE = (1 << 1),
  RETURN_VALUE_MASK_ALL = (RETURN_VALUE_MASK_DEFAULT | RETURN_VALUE_MASK_BUBBLE),
};

struct XX_EXPORT GUITouch { // touch event point
  uint    id;
  float   start_x, start_y;
  float   x, y, force;
  bool    click_in;
  View*   view;
};

/**
 * @func GUIEvent gui event
 */
class XX_EXPORT GUIEvent: public Event<Object, View> {
 public:
  inline GUIEvent(cSendData data): Event<Object, View>() { XX_UNREACHABLE(); }
  inline GUIEvent(View* origin, cSendData data = SendData())
  : Event(data), origin_(origin), time_(sys::time()), valid_(true) {
    return_value = RETURN_VALUE_MASK_ALL;
  }
  inline View* origin() const { return origin_; }
  inline uint64 timestamp() const { return time_; }
  inline void cancel_default() { return_value &= ~RETURN_VALUE_MASK_DEFAULT; }
  inline void cancel_bubble() { return_value &= ~RETURN_VALUE_MASK_BUBBLE; }
  inline bool is_default() const { return return_value & RETURN_VALUE_MASK_DEFAULT; }
  inline bool is_bubble() const { return return_value & RETURN_VALUE_MASK_BUBBLE; }
  virtual void release() {
    valid_ = false;
    origin_ = nullptr;
    Event<Object, View>::release();
  }
 protected:
  View*   origin_;
  uint64  time_;
  bool    valid_;
};

/**
 * @class GUIActionEvent
 */
class XX_EXPORT GUIActionEvent: public GUIEvent {
 public:
  inline GUIActionEvent(Action* action, View* view, uint64 delay, uint frame, uint loop)
  : GUIEvent(view), action_(action), delay_(delay), frame_(frame), loop_(loop) { }
  inline Action* action() const { return action_; }
  inline uint64 delay() const { return delay_; }
  inline uint frame() const { return frame_; }
  inline uint loop() const { return loop_; }
  virtual void release() { action_ = nullptr; GUIEvent::release(); }
 private:
  Action* action_;
  uint64  delay_;
  uint  frame_, loop_;
};

/**
 * @func GUIKeyEvent keyboard event
 */
class XX_EXPORT GUIKeyEvent: public GUIEvent {
 public:
  inline GUIKeyEvent(View* origin, uint keycode,
                     bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
                     uint repeat, int device, int source)
  : GUIEvent(origin), keycode_(keycode)
  , repeat_(repeat), device_(device), source_(source), shift_(shift)
  , ctrl_(ctrl), alt_(alt), command_(command), caps_lock_(caps_lock), focus_move_(nullptr) { }
  inline int  keycode() const { return keycode_; }
  inline int  repeat() const { return repeat_; }
  inline int  device() const { return device_; }
  inline int  source() const { return source_; }
  inline bool shift() const { return shift_; }
  inline bool ctrl() const { return ctrl_; }
  inline bool alt() const { return alt_; }
  inline bool command() const { return command_; }
  inline bool caps_lock() const { return caps_lock_; }
  inline void set_keycode(int value) { keycode_ = value; }
  inline View* focus_move() const { return focus_move_; }
  inline void set_focus_move(View* view) { if (valid_) focus_move_ = view; }
  virtual void release() { focus_move_ = nullptr; GUIEvent::release(); }
 private:
  int  keycode_;
  int  device_, source_, repeat_;
  bool shift_, ctrl_, alt_;
  bool command_, caps_lock_;
  View* focus_move_;
};

/**
 * @class GUIClickEvent click event
 */
class XX_EXPORT GUIClickEvent: public GUIEvent {
 public:
  inline GUIClickEvent(View* origin, float x, float y, bool keyboard = false, uint count = 1)
  : GUIEvent(origin), x_(x), y_(y), keyboard_(keyboard), count_(count) { }
  inline float x() const { return x_; }
  inline float y() const { return y_; }
  inline uint count() const { return count_; }
  inline bool keyboard() const { return keyboard_; }
 private:
  float x_, y_;
  uint count_;
  bool keyboard_;
};

/**
 * @class GUIMouseEvent mouse event
 */
class XX_EXPORT GUIMouseEvent: public GUIKeyEvent {
 public:
  inline GUIMouseEvent(View* origin, float x, float y, uint keycode,
                       bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
                       uint repeat = 0, int device = 0, int source = 0)
  : GUIKeyEvent(origin, keycode,shift, ctrl, alt,
                command, caps_lock, repeat, device, source), x_(x), y_(y) { }
  inline float x() const { return x_; }
  inline float y() const { return y_; }
 private:
  float x_, y_;
};

/**
 * @class GUIHighlightedEvent status event
 */
class XX_EXPORT GUIHighlightedEvent: public GUIEvent {
 public:
  inline GUIHighlightedEvent(View* origin, HighlightedStatus status)
  : GUIEvent(origin), _status(status) { }
  inline HighlightedStatus status() const { return _status; }
 private:
  HighlightedStatus _status;
};

/**
 * @class GUITouchEvent touch event
 */
class XX_EXPORT GUITouchEvent: public GUIEvent {
 public:
  inline GUITouchEvent(View* origin, Array<GUITouch>& touches)
    : GUIEvent(origin), m_change_touches(touches) { }
  inline Array<GUITouch>& changed_touches() { return m_change_touches; }
 private:
  Array<GUITouch> m_change_touches;
};

/**
 * @class GUIFocusMoveEvent
 */
class XX_EXPORT GUIFocusMoveEvent: public GUIEvent {
 public:
  inline GUIFocusMoveEvent(View* origin, View* focus, View* focus_move)
  : GUIEvent(origin), m_focus(focus), m_focus_move(focus_move) { }
  inline View* focus() { return m_focus; }
  inline View* focus_move() { return m_focus_move; }
  virtual void release() {
    m_focus = nullptr;
    m_focus_move = nullptr; GUIEvent::release();
  }
 private:
  View* m_focus;
  View* m_focus_move;
};

/**
 * @class TextInputProtocol
 */
class XX_EXPORT TextInputProtocol {
 public:
  typedef ProtocolTraits Traits;
  
  virtual void input_delete_text(int count) = 0;
  virtual void input_insert_text(cString& text) = 0;
  virtual void input_marked_text(cString& text) = 0;
  virtual void input_unmark_text(cString& text) = 0;
  virtual bool input_can_delete() = 0;
  virtual bool input_can_backspace() = 0;
  virtual KeyboardType keyboard_type() = 0;
  virtual KeyboardReturnType keyboard_return_type() = 0;
};

/**
 * @class GUIEventDispatch
 */
class XX_EXPORT GUIEventDispatch: public Object {
 public:
  GUIEventDispatch(GUIApplication* app);
  
  virtual ~GUIEventDispatch();
  
  void dispatch_touchstart(List<GUITouch>&& touches);   // touch
  void dispatch_touchmove(List<GUITouch>&& touches);
  void dispatch_touchend(List<GUITouch>&& touches);
  void dispatch_touchcancel(List<GUITouch>&& touches);
  void dispatch_ime_delete(int count);     // ime input
  void dispatch_ime_insert(cString& text);
  void dispatch_ime_marked(cString& text);
  void dispatch_ime_unmark(cString& text);
  
  /**
   * @func make_text_input
   */
  void make_text_input(TextInputProtocol* input);
  
  /**
   * @func keyboard_adapter
   */
  inline KeyboardAdapter* keyboard_adapter() {
    return m_keyboard;
  }
  
 private:
  class OriginTouche; typedef Map<PrtKey<View>, OriginTouche*> OriginTouches;
  
  GUIApplication*     app_;
  OriginTouches       m_origin_touches;
  KeyboardAdapter*    m_keyboard;
  TextInputProtocol*  m_text_input;
  
  XX_DEFINE_INLINE_CLASS(Inl);
};

XX_END
#endif
