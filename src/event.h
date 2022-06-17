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

#ifndef __noug__event__
#define __noug__event__

#include "./util/event.h"
#include "./util/handle.h"
#include "./math.h"
#include "./value.h"
#include "./keyboard.h"

// all ui events / NAME, FLAG
#define N_UI_Events(F) \
	/* can bubble event */ \
	F(Click, CLICK, UI_EVENT_FLAG_BUBBLE) \
	F(Back, CLICK, UI_EVENT_FLAG_BUBBLE) \
	F(KeyDown, KEYBOARD, UI_EVENT_FLAG_BUBBLE) /* View */\
	F(KeyPress, KEYBOARD, UI_EVENT_FLAG_BUBBLE) \
	F(KeyUp, KEYBOARD, UI_EVENT_FLAG_BUBBLE) \
	F(KeyEnter, KEYBOARD, UI_EVENT_FLAG_BUBBLE) \
	F(TouchStart, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(TouchMove, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(TouchEnd, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(TouchCancel, TOUCH, UI_EVENT_FLAG_BUBBLE) \
	F(MouseOver, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MouseOut, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MouseLeave, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MouseEnter, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MouseMove, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MouseDown, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MouseUp, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(MouseWheel, MOUSE, UI_EVENT_FLAG_BUBBLE) \
	F(Focus, DEFAULT, UI_EVENT_FLAG_BUBBLE) \
	F(Blur, DEFAULT, UI_EVENT_FLAG_BUBBLE) \
	/* canno bubble event */ \
	F(Highlighted, HIGHLIGHTED, UI_EVENT_FLAG_NONE) /* normal / hover / down */ \
	F(ActionKeyframe, ACTION, UI_EVENT_FLAG_NONE) \
	F(ActionLoop, ACTION, UI_EVENT_FLAG_NONE) \
	F(FocusMove, FOCUS_MOVE, UI_EVENT_FLAG_NONE) /*Panel*/ \
	F(Scroll, DEFAULT, UI_EVENT_FLAG_NONE) /*BasicScroll*/\
	F(Change, DEFAULT, UI_EVENT_FLAG_NONE) /*Input*/ \
	F(Load, DEFAULT, UI_EVENT_FLAG_NONE) /* Image */ \
	/* player */ \
	F(Error, DEFAULT, UI_EVENT_FLAG_PLAYER | UI_EVENT_FLAG_ERROR) \
	F(Ready, DEFAULT, UI_EVENT_FLAG_PLAYER) /* AutoPlayer / Video */ \
	F(WaitBuffer, DEFAULT, UI_EVENT_FLAG_PLAYER | UI_EVENT_FLAG_FLOAT) \
	F(StartPlay, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(SourceEnd, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(Pause, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(Resume, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(Stop, DEFAULT, UI_EVENT_FLAG_PLAYER) \
	F(Seek, DEFAULT, UI_EVENT_FLAG_PLAYER | UI_EVENT_FLAG_UINT64) \


namespace noug {

	class Application;
	class View;
	class Action;

	// event category
	enum {
		UI_EVENT_CATEGORY_DEFAULT = 0,
		UI_EVENT_CATEGORY_KEYBOARD,
		UI_EVENT_CATEGORY_CLICK,
		UI_EVENT_CATEGORY_HIGHLIGHTED,
		UI_EVENT_CATEGORY_TOUCH,
		UI_EVENT_CATEGORY_MOUSE,
		UI_EVENT_CATEGORY_ACTION,
		UI_EVENT_CATEGORY_FOCUS_MOVE,
	};

	// event flags / cast
	enum {
		UI_EVENT_FLAG_NONE = 0,
		UI_EVENT_FLAG_ERROR,  // cast data
		UI_EVENT_FLAG_FLOAT,  // cast Float
		UI_EVENT_FLAG_UINT64, // cast Uint64
		UI_EVENT_FLAG_CAST = (255), // Event::data(), cast flag
		UI_EVENT_FLAG_BUBBLE = (1 << 8), // bubble, other flag
		UI_EVENT_FLAG_PLAYER = (1 << 9), // player
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

	class N_EXPORT UIEventName {
	public:
		UIEventName(cString& name, uint32_t category, uint32_t flag);
		N_DEFINE_PROP_READ(String, to_string);
		N_DEFINE_PROP_READ(uint32_t, category);
		N_DEFINE_PROP_READ(uint32_t, flag);
		N_DEFINE_PROP_READ(uint32_t, hash_code);
		inline bool equals(const UIEventName& v) const { return v.hash_code() == _hash_code; }
		inline bool operator==(const UIEventName& v) const { return v._hash_code == _hash_code; }
		inline bool operator!=(const UIEventName& v) const { return v._hash_code != _hash_code; }
		inline bool operator>(const UIEventName& v) const { return _hash_code > v._hash_code; }
		inline bool operator<(const UIEventName& v) const { return _hash_code < v._hash_code; }
	};

	// event names string => UIEventName
	N_EXPORT extern const Dict<String, UIEventName> UIEventNames;

	// define event names
#define N_FUN(NAME, C, F) \
	N_EXPORT extern const UIEventName UIEvent_##NAME;
	N_UI_Events(N_FUN)
#undef N_FUN

	// -----------------------------------

	/**
	* @func UIEvent gui event
	*/
	class N_EXPORT UIEvent: public Event<View, Object, View, int> {
	public:
		// inline UIEvent(cSendData& data): Event<View, Object, View>() { N_UNREACHABLE(); }
		UIEvent(View* origin);
		N_DEFINE_PROP_READ(uint64_t, timestamp);
		inline bool is_default() const { return return_value & RETURN_VALUE_MASK_DEFAULT; }
		inline bool is_bubble() const { return return_value & RETURN_VALUE_MASK_BUBBLE; }
		inline void cancel_default() { return_value &= ~RETURN_VALUE_MASK_DEFAULT; }
		inline void cancel_bubble() { return_value &= ~RETURN_VALUE_MASK_BUBBLE; }
	};

	/**
	* @class ActionEvent
	*/
	class N_EXPORT ActionEvent: public UIEvent {
	public:
		ActionEvent(Action* action, View* origin, uint64_t delay, uint32_t frame, uint32_t loop);
		N_DEFINE_PROP_READ(Action*, action);
		N_DEFINE_PROP_READ(uint64_t, delay);
		N_DEFINE_PROP_READ(uint32_t, frame);
		N_DEFINE_PROP_READ(uint32_t, loop);
		virtual void release();
	};

	/**
	* @func KeyEvent keyboard event
	*/
	class N_EXPORT KeyEvent: public UIEvent {
	public:
		KeyEvent(View* origin, uint32_t keycode,
						bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
						uint32_t repeat, int device, int source);
		N_DEFINE_PROP(View*, focus_move);
		N_DEFINE_PROP(uint32_t, keycode);
		N_DEFINE_PROP_READ(uint32_t, repeat);
		N_DEFINE_PROP_READ(uint32_t, device);
		N_DEFINE_PROP_READ(uint32_t, source);
		N_DEFINE_PROP_READ(uint32_t, shift);
		N_DEFINE_PROP_READ(uint32_t, ctrl);
		N_DEFINE_PROP_READ(uint32_t, alt);
		N_DEFINE_PROP_READ(uint32_t, command);
		N_DEFINE_PROP_READ(uint32_t, caps_lock);
		virtual void release();
	};

	/**
	* @class ClickEvent click event
	*/
	class N_EXPORT ClickEvent: public UIEvent {
	public:
		enum Type {
			TOUCH = 1, KEYBOARD = 2, MOUSE = 3
		};
		ClickEvent(View* origin, float x, float y, Type type, uint32_t count = 1);
		N_DEFINE_PROP_READ(float, x);
		N_DEFINE_PROP_READ(float, y);
		N_DEFINE_PROP_READ(uint32_t, count);
		N_DEFINE_PROP_READ(Type, type);
	};

	/**
	* @class UIMouseEvent mouse event
	*/
	class N_EXPORT MouseEvent: public KeyEvent {
	public:
		MouseEvent(View* origin, float x, float y, uint32_t keycode,
											bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
											uint32_t repeat = 0, int device = 0, int source = 0);
		N_DEFINE_PROP_READ(float, x);
		N_DEFINE_PROP_READ(float, y);
	};

	/**
	* @class HighlightedEvent status event
	*/
	class N_EXPORT HighlightedEvent: public UIEvent {
	public:
		HighlightedEvent(View* origin, HighlightedStatus status);
		N_DEFINE_PROP_READ(HighlightedStatus, status);
	};

	/**
	* @class TouchEvent touch event
	*/
	class N_EXPORT TouchEvent: public UIEvent {
	public:
		struct TouchPoint { // touch event point
			uint32_t id;
			float    start_x, start_y;
			float    x, y, force;
			bool     click_in;
			View*    view;
		};
		TouchEvent(View* origin, Array<TouchPoint>& touches);
		inline Array<TouchPoint>& changed_touches() { return _change_touches; }
	private:
		Array<TouchPoint> _change_touches;
	};

	typedef TouchEvent::TouchPoint TouchPoint;

	/**
	* @class FocusMoveEvent
	*/
	class N_EXPORT FocusMoveEvent: public UIEvent {
	public:
		FocusMoveEvent(View* origin, View* focus, View* focus_move);
		N_DEFINE_PROP_READ(View*, focus);
		N_DEFINE_PROP_READ(View*, focus_move);
		virtual void release();
	};

	/**
	* @class ITextInput
	*/
	class N_EXPORT ITextInput: public Protocol {
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
	class N_EXPORT EventDispatch: public Object {
	public:
		EventDispatch(Application* app);
		virtual ~EventDispatch();

		// handles
		void onTouchstart(List<TouchPoint>&& touches);
		void onTouchmove(List<TouchPoint>&& touches);
		void onTouchend(List<TouchPoint>&& touches);
		void onTouchcancel(List<TouchPoint>&& touches);
		void onMousemove(float x, float y);
		void onMousepress(KeyboardKeyName key, bool down);
		// ime
		void onImeDelete(int count);
		void onImeInsert(cString& text);
		void onImeMarked(cString& text);
		void onImeUnmark(cString& text);
		void onImeControl(KeyboardKeyName name);
		// keyboard main loop call
		void onKeyboard_down();
		void onKeyboard_up();

		N_DEFINE_PROP_READ(Application*, host);
		N_DEFINE_PROP_READ(KeyboardAdapter*, keyboard);
		N_DEFINE_PROP(ITextInput*, text_input);

	private:
		void touchstart_erase(View* view, List<TouchPoint>& in);
		void touchstart(View* view, List<TouchPoint>& in);
		void touchmove(List<TouchPoint>& in);
		void touchend(List<TouchPoint>& in, const UIEventName& type);
		void mousemove(View* view, Vec2 pos);
		void mousepress(KeyboardKeyName name, bool down, Vec2 pos);
		void mousewhell(KeyboardKeyName name, bool down, float x, float y);
		View* find_receive_event_view(Vec2 pos);
		Sp<MouseEvent> NewMouseEvent(View* view, float x, float y, uint32_t keycode = 0);
		class OriginTouche;
		class MouseHandle;
		Dict<View*, OriginTouche*> _origin_touches;
		MouseHandle*  _mouse_h;
	};

}

#endif
