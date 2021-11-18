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

#ifndef __flare__event__
#define __flare__event__

#include "./util/event.h"
#include "./util/os.h"
#include "./math.h"
#include "./value.h"
#include "./keyboard.h"


// all ui events / NAME, STR_NAME, CATEGORY, FLAG
#define FX_UI_EVENT_TABLE(F) \
	/* can bubble event */ \
	F(CLICK, Click, CLICK, UI_EVENT_FLAG_BUBBLE) \
	F(BACK, Back, CLICK, UI_EVENT_FLAG_BUBBLE) \
	F(KEY_DOWN, KeyDown, KEYBOARD, UI_EVENT_FLAG_BUBBLE) /* View */\
	F(KEY_PRESS, KeyPress, KEYBOARD, UI_EVENT_FLAG_BUBBLE) \
	F(KEY_UP, KeyUp, KEYBOARD, UI_EVENT_FLAG_BUBBLE) \
	F(KEY_ENTER, KeyEnter, KEYBOARD, UI_EVENT_FLAG_BUBBLE) \
	F(TOUCH_START, TouchStart, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(TOUCH_MOVE, TouchMove, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(TOUCH_END, TouchEnd, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(TOUCH_CANCEL, TouchCancel, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_OVER, MouseOver, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_OUT, MouseOut, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_LEAVE, MouseLeave, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_ENTER, MouseEnter, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_MOVE, MouseMove, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_DOWN, MouseDown, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_UP, MouseUp, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MOUSE_WHEEL, MouseWheel, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(FOCUS, Focus, DEFAULT, UI_EVENT_FLAG_BUBBLE) \
	F(BLUR, Blur, DEFAULT, UI_EVENT_FLAG_BUBBLE) \
	/* canno bubble event */ \
	F(HIGHLIGHTED, Highlighted, HIGHLIGHTED, UI_EVENT_FLAG_NONE) /* normal / hover / down */ \
	F(ACTION_KEYFRAME, ActionKeyframe, ACTION, UI_EVENT_FLAG_NONE) \
	F(ACTION_LOOP, ActionLoop, ACTION, UI_EVENT_FLAG_NONE) \
	F(FOCUS_MOVE, FocusMove, FOCUS_MOVE, UI_EVENT_FLAG_NONE) /*Panel*/ \
	F(SCROLL, Scroll, DEFAULT, UI_EVENT_FLAG_NONE) /*BasicScroll*/\
	F(CHANGE, Change, DEFAULT, UI_EVENT_FLAG_NONE) /*Input*/ \
	F(LOAD, Load, DEFAULT, UI_EVENT_FLAG_NONE) /* Image */ \
	F(ERROR, Error, ERROR, UI_EVENT_FLAG_PLAYER) \
	F(READY, Ready, DEFAULT, UI_EVENT_FLAG_PLAYER) /* AutoPlayer / Video */ \
	F(WAIT_BUFFER, WaitBuffer, FLOAT, UI_EVENT_FLAG_PLAYER) \
	F(START_PLAY, StartPlay, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(SOURCE_END, SourceEnd, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(PAUSE, Pause, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(RESUME, Resume, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(STOP, Stop, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(SEEK, Seek, UINT64, UI_EVENT_FLAG_PLAYER) \



namespace flare {

	class Application;
	class View;
	class Action;

	// event category
	enum {
		UI_EVENT_CATEGORY_DEFAULT,
		UI_EVENT_CATEGORY_KEYBOARD,
		UI_EVENT_CATEGORY_CLICK,
		UI_EVENT_CATEGORY_HIGHLIGHTED,
		UI_EVENT_CATEGORY_TOUCH,
		UI_EVENT_CATEGORY_MOUSE,
		UI_EVENT_CATEGORY_ACTION,
		UI_EVENT_CATEGORY_FOCUS_MOVE,
		UI_EVENT_CATEGORY_ERROR,
		UI_EVENT_CATEGORY_FLOAT,
		UI_EVENT_CATEGORY_UINT64,
	};

	// event flags
	enum {
		UI_EVENT_FLAG_NONE = 0,
		UI_EVENT_FLAG_BUBBLE = (1 << 0),
		UI_EVENT_FLAG_PLAYER = (1 << 1),
	};

	// event returl value mask
	enum ReturnValueMask {
		RETURN_VALUE_MASK_DEFAULT = (1 << 0),
		RETURN_VALUE_MASK_BUBBLE = (1 << 1),
		RETURN_VALUE_MASK_ALL = (RETURN_VALUE_MASK_DEFAULT | RETURN_VALUE_MASK_BUBBLE),
	};

	// event highlighed status
	enum HighlightedStatus {
		HIGHLIGHTED_NORMAL = 1,
		HIGHLIGHTED_HOVER,
		HIGHLIGHTED_DOWN,
	};

	// event name
	class FX_EXPORT UIEventName {
	 public:
		inline UIEventName() { FX_UNREACHABLE(); }
		inline UIEventName(cString& n, uint32_t category, int flag)
			: name_(n), code_((uint32_t)n.hash_code()), category_(category), flag_(flag) {}
		inline uint32_t hash_code() const { return code_; }
		inline bool equals(const UIEventName& o) const { return o.hash_code() == code_; }
		inline String to_string() const { return name_; }
		inline uint32_t category() const { return category_; }
		inline int flag() const { return flag_; }
		inline bool operator==(const UIEventName& type) const { return type.code_ == code_; }
		inline bool operator!=(const UIEventName& type) const { return type.code_ != code_; }
		inline bool operator>(const UIEventName& type) const { return code_ > type.code_; }
		inline bool operator<(const UIEventName& type) const { return code_ < type.code_; }
	 private:
		String  name_;
		uint32_t code_, category_;
		int  flag_;
	};

	// event names string => UIEventName
	FX_EXPORT extern const Dict<String, UIEventName> UIEventNames;

	// define event names
	#define FX_FUN(NAME, STR, CATEGORY, FLAG) \
		FX_EXPORT extern const UIEventName UI_EVENT_##NAME;
	FX_UI_EVENT_TABLE(FX_FUN)
	#undef FX_FUN

	// -----------------------------------

	/**
	* @func UIEvent gui event
	*/
	class FX_EXPORT UIEvent: public Event<Object, View> {
	 public:
		inline UIEvent(cSendData& data): Event<Object, View>() { FX_UNREACHABLE(); }
		inline UIEvent(View* origin, cSendData& data = SendData())
			: Event(data), return_value(RETURN_VALUE_MASK_ALL), origin_(origin), time_(os::time()), valid_(true) {
		}
		inline View* origin() const { return origin_; }
		inline uint64_t timestamp() const { return time_; }
		inline void cancel_default() { return_value &= ~RETURN_VALUE_MASK_DEFAULT; }
		inline void cancel_bubble() { return_value &= ~RETURN_VALUE_MASK_BUBBLE; }
		inline bool is_default() const { return return_value & RETURN_VALUE_MASK_DEFAULT; }
		inline bool is_bubble() const { return return_value & RETURN_VALUE_MASK_BUBBLE; }
		virtual void release();
		int return_value;
	 protected:
		View*    origin_;
		uint64_t time_;
		bool     valid_;
	};

	/**
	* @class ActionEvent
	*/
	class FX_EXPORT ActionEvent: public UIEvent {
	 public:
		inline ActionEvent(Action* action, View* view, uint64_t delay, uint32_t frame, uint32_t loop)
			: UIEvent(view), action_(action), delay_(delay), frame_(frame), loop_(loop) {}
		inline Action* action() const { return action_; }
		inline uint64_t delay() const { return delay_; }
		inline uint32_t frame() const { return frame_; }
		inline uint32_t loop() const { return loop_; }
		virtual void release();
	 private:
		Action* action_;
		uint64_t  delay_;
		uint32_t  frame_, loop_;
	};

	/**
	* @func KeyEvent keyboard event
	*/
	class FX_EXPORT KeyEvent: public UIEvent {
	 public:
		inline KeyEvent(View* origin, uint32_t keycode,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
										uint32_t repeat, int device, int source)
			: UIEvent(origin), keycode_(keycode)
			, device_(device), source_(source), repeat_(repeat), shift_(shift)
			, ctrl_(ctrl), alt_(alt), command_(command), caps_lock_(caps_lock), focus_move_(nullptr) {}
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
		virtual void release();
	 private:
		int  keycode_;
		int  device_, source_, repeat_;
		bool shift_, ctrl_, alt_;
		bool command_, caps_lock_;
		View* focus_move_;
	};

	/**
	* @class ClickEvent click event
	*/
	class FX_EXPORT ClickEvent: public UIEvent {
	 public:
		enum Type { TOUCH = 1, KEYBOARD = 2, MOUSE = 3 };
		inline ClickEvent(View* origin, float x, float y, Type type, uint32_t count = 1)
			: UIEvent(origin), x_(x), y_(y), count_(count), type_(type) {}
		inline float x() const { return x_; }
		inline float y() const { return y_; }
		inline uint32_t count() const { return count_; }
		inline Type type() const { return type_; }
	 private:
		float x_, y_;
		uint32_t count_;
		Type type_;
	};

	/**
	* @class UIMouseEvent mouse event
	*/
	class FX_EXPORT MouseEvent: public KeyEvent {
	 public:
		inline MouseEvent(View* origin, float x, float y, uint32_t keycode,
											bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
											uint32_t repeat = 0, int device = 0, int source = 0)
			: KeyEvent(origin, keycode, shift, ctrl, alt,
				command, caps_lock, repeat, device, source), x_(x), y_(y) {}
		inline float x() const { return x_; }
		inline float y() const { return y_; }
	 private:
		float x_, y_;
	};

	/**
	* @class HighlightedEvent status event
	*/
	class FX_EXPORT HighlightedEvent: public UIEvent {
	 public:
		inline HighlightedEvent(View* origin, HighlightedStatus status)
			: UIEvent(origin), _status(status) {}
		inline HighlightedStatus status() const { return _status; }
	 private:
		HighlightedStatus _status;
	};

	/**
	* @class TouchEvent touch event
	*/
	class FX_EXPORT TouchEvent: public UIEvent {
	 public:
		struct TouchPoint { // touch event point
			uint32_t id;
			float    start_x, start_y;
			float    x, y, force;
			bool     click_in;
			View*    view;
		};
		inline TouchEvent(View* origin, Array<TouchPoint>& touches)
			: UIEvent(origin), _change_touches(touches) {}
		inline Array<TouchPoint>& changed_touches() { return _change_touches; }
	 private:
		Array<TouchPoint> _change_touches;
	};

	typedef TouchEvent::TouchPoint TouchPoint;

	/**
	* @class FocusMoveEvent
	*/
	class FX_EXPORT FocusMoveEvent: public UIEvent {
	 public:
		inline FocusMoveEvent(View* origin, View* old_focus, View* new_focus)
			: UIEvent(origin), _old_focus(old_focus), _new_focus(new_focus) {}
		inline View* old_focus() { return _old_focus; }
		inline View* new_focus() { return _new_focus; }
		inline View* focus() { return _old_focus; }
		inline View* focus_move() { return _new_focus; }
		virtual void release();
	 private:
		View* _old_focus;
		View* _new_focus;
	};

	/**
	* @class ITextInput
	*/
	class FX_EXPORT ITextInput: public Protocol {
	 public:
		virtual void input_delete(int count) = 0;
		virtual void input_insert(cString& text) = 0;
		virtual void input_marked(cString& text) = 0;
		virtual void input_unmark(cString& text) = 0;
		virtual void input_control(KeyboardKeyName name) = 0;
		virtual bool input_can_delete() = 0;
		virtual bool input_can_backspace() = 0;
		virtual Vec2 input_spot_location() = 0;
		virtual KeyboardType input_keyboard_type() = 0;
		virtual KeyboardReturnType input_keyboard_return_type() = 0;
	};

	/**
	* @class EventDispatch
	*/
	class FX_EXPORT EventDispatch: public Object {
	 public:
		EventDispatch(Application* app);
		virtual ~EventDispatch();
		// touch
		void onTouchstart(List<TouchPoint>&& touches);
		void onTouchmove(List<TouchPoint>&& touches);
		void onTouchend(List<TouchPoint>&& touches);
		void onTouchcancel(List<TouchPoint>&& touches);
		// mouse
		void onMousemove(float x, float y);
		void onMousepress(KeyboardKeyName key, bool down);
		// ime
		void onIme_delete(int count);
		void onIme_insert(cString& text);
		void onIme_marked(cString& text);
		void onIme_unmark(cString& text);
		void onIme_control(KeyboardKeyName name);

		/**
		* @func make_text_input
		*/
		void make_text_input(ITextInput* input);
		
		/**
		*mapunc keyboard_adapter
		*/
		inline KeyboardAdapter* keyboard_adapter() {
			return _keyboard;
		}
		
	 private:
		class OriginTouche;
		class MouseHandle;
		typedef Dict<View*, OriginTouche*> OriginTouches;
		
		Application*        _host;
		OriginTouches       _origin_touches;
		MouseHandle*        _mouse_h;
		KeyboardAdapter*    _keyboard;
		ITextInput*         _text_input;
		
		FX_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif
