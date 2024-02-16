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

#ifndef __quark__ui__event__
#define __quark__ui__event__

#include "../util/event.h"
#include "../util/handle.h"
#include "../util/loop.h"
#include "./math.h"
#include "./types.h"
#include "./text/text_input.h"

// all ui events / Name, Flag
#define Qk_UI_Events(F) \
/* can bubble event */ \
F(Click, Click, kBubble_UIEventFlags) \
F(Back, Click, kBubble_UIEventFlags) \
F(KeyDown, Keyboard, kBubble_UIEventFlags) /* View */\
F(KeyPress, Keyboard, kBubble_UIEventFlags) \
F(KeyUp, Keyboard, kBubble_UIEventFlags) \
F(KeyEnter, Keyboard, kBubble_UIEventFlags) \
F(TouchStart, Touch, kBubble_UIEventFlags) \
F(TouchMove, Touch, kBubble_UIEventFlags) \
F(TouchEnd, Touch, kBubble_UIEventFlags) \
F(TouchCancel, Touch, kBubble_UIEventFlags) \
F(MouseOver, Mouse, kBubble_UIEventFlags) \
F(MouseOut, Mouse, kBubble_UIEventFlags) \
F(MouseLeave, Mouse, kBubble_UIEventFlags) \
F(MouseEnter, Mouse, kBubble_UIEventFlags) \
F(MouseMove, Mouse, kBubble_UIEventFlags) \
F(MouseDown, Mouse, kBubble_UIEventFlags) \
F(MouseUp, Mouse, kBubble_UIEventFlags) \
F(MouseWheel, Mouse, kBubble_UIEventFlags) \
F(Focus, Default, kBubble_UIEventFlags) \
F(Blur, Default, kBubble_UIEventFlags) \
F(Highlighted, Highlighted, kBubble_UIEventFlags) /* normal / hover / down */ \
/* canno bubble event */ \
F(ActionKeyframe, Action, kNone_UIEventFlags) \
F(ActionLoop, Action, kNone_UIEventFlags) \
F(Scroll, Default, kNone_UIEventFlags) /*ScrollBase*/\
F(Change, Default, kNone_UIEventFlags) /*Input*/ \
F(Load, Default, kNone_UIEventFlags) /* Image */ \
/* player */ \
F(Error, Player, kError_UIEventFlags) \
F(Ready, Player, kNone_UIEventFlags) /* AutoPlayer / Video */ \
F(WaitBuffer, Player, kFloat_UIEventFlags) \
F(StartPlay, Player, kNone_UIEventFlags) \
F(SourceEnd, Player, kNone_UIEventFlags) \
F(Pause, Player, kNone_UIEventFlags) \
F(Resume, Player, kNone_UIEventFlags) \
F(Stop, Player, kNone_UIEventFlags) \
F(Seek, Player, kUint64_UIEventFlags) \

namespace qk {
	class Application;
	class Layout;
	class View;
	class Action;
	class TextInput;
	class Window;

	enum {
		kDefault_UIEventCategory,
		kKeyboard_UIEventCategory,
		kClick_UIEventCategory,
		kTouch_UIEventCategory,
		kMouse_UIEventCategory,
		kFocus_MOVE_UIEventCategory,
		kAction_UIEventCategory,
		kHighlighted_UIEventCategory,
		kPlayer_UIEventCategory,
	};

	// event flags / cast
	enum {
		kNone_UIEventFlags,
		kError_UIEventFlags,  // cast data
		kFloat_UIEventFlags,  // cast Float
		kUint64_UIEventFlags, // cast Uint64
		kCast_UIEventFlags = (255), // Event::data(), cast flag
		kBubble_UIEventFlags = (1 << 8), // bubble, other flag
	};

	// event returl value mask
	enum ReturnValueMask {
		kDefault_ReturnValueMask = (1 << 0),
		kBubble_ReturnValueMask = (1 << 1),
		kAll_ReturnValueMask = (kDefault_ReturnValueMask | kBubble_ReturnValueMask),
	};

	class Qk_EXPORT UIEventName {
	public:
		UIEventName(cString& name, uint32_t category, uint32_t flag);
		Qk_DEFINE_PROP_GET(String, toString, Const);
		Qk_DEFINE_PROP_GET(uint32_t, category, Const);
		Qk_DEFINE_PROP_GET(uint32_t, flag, Const);
		Qk_DEFINE_PROP_GET(uint32_t, hashCode, Const);
		inline bool equals(const UIEventName& v) const { return v.hashCode() == _hashCode; }
		inline bool operator==(const UIEventName& v) const { return v._hashCode == _hashCode; }
		inline bool operator!=(const UIEventName& v) const { return v._hashCode != _hashCode; }
		inline bool operator>(const UIEventName& v) const { return _hashCode > v._hashCode; }
		inline bool operator<(const UIEventName& v) const { return _hashCode < v._hashCode; }
	};

	// event names string => UIEventName
	Qk_EXPORT extern const Dict<String, UIEventName> UIEventNames;
	// define event names
	#define _Fun(Name, C, F) \
	Qk_EXPORT extern const UIEventName UIEvent_##Name;
	Qk_UI_Events(_Fun)
	#undef _Fun

	// -----------------------------------

	/**
	* @func UIEvent gui event
	*/
	class Qk_EXPORT UIEvent: public Event<View, Object, int> {
		Qk_HIDDEN_ALL_COPY(UIEvent);
	public:
		UIEvent(View *origin);
		Qk_DEFINE_PROP_GET(View*, origin);
		Qk_DEFINE_PROP_GET(uint64_t, timestamp, Const);
		inline bool is_default() const { return return_value & kDefault_ReturnValueMask; }
		inline bool is_bubble() const { return return_value & kBubble_ReturnValueMask; }
		inline void cancel_default() { return_value &= ~kDefault_ReturnValueMask; }
		inline void cancel_bubble() { return_value &= ~kBubble_ReturnValueMask; }
		virtual void release() override;
	};

	/**
	* @class ActionEvent
	*/
	class Qk_EXPORT ActionEvent: public UIEvent {
	public:
		ActionEvent(Action* action, View* origin, uint64_t delay, uint32_t frame, uint32_t loop);
		Qk_DEFINE_PROP_GET(Action*, action);
		Qk_DEFINE_PROP_GET(uint64_t, delay, Const);
		Qk_DEFINE_PROP_GET(uint32_t, frame, Const);
		Qk_DEFINE_PROP_GET(uint32_t, loop, Const);
		virtual void release() override;
	};

	/**
	* @func KeyEvent keyboard event
	*/
	class Qk_EXPORT KeyEvent: public UIEvent {
	public:
		KeyEvent(View* origin, uint32_t keycode,
						bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
						uint32_t repeat, int device, int source);
		Qk_DEFINE_PROP(View*, focus_move);
		Qk_DEFINE_PROP(uint32_t, keycode, Const);
		Qk_DEFINE_PROP_GET(uint32_t, repeat, Const);
		Qk_DEFINE_PROP_GET(uint32_t, device, Const);
		Qk_DEFINE_PROP_GET(uint32_t, source, Const);
		Qk_DEFINE_PROP_GET(uint32_t, shift, Const);
		Qk_DEFINE_PROP_GET(uint32_t, ctrl, Const);
		Qk_DEFINE_PROP_GET(uint32_t, alt, Const);
		Qk_DEFINE_PROP_GET(uint32_t, command, Const);
		Qk_DEFINE_PROP_GET(uint32_t, caps_lock, Const);
		virtual void release() override;
	};

	/**
	* @class ClickEvent click event
	*/
	class Qk_EXPORT ClickEvent: public UIEvent {
	public:
		enum Type {
			kTouch = 1, kKeyboard = 2, kMouse = 3
		};
		ClickEvent(View* origin, float x, float y, Type type, uint32_t count = 1);
		Qk_DEFINE_PROP_GET(float, x, Const);
		Qk_DEFINE_PROP_GET(float, y, Const);
		Qk_DEFINE_PROP_GET(uint32_t, count, Const);
		Qk_DEFINE_PROP_GET(Type, type, Const);
	};

	/**
	* @class UIMouseEvent mouse event
	*/
	class Qk_EXPORT MouseEvent: public KeyEvent {
	public:
		MouseEvent(View* origin, float x, float y, uint32_t keycode,
											bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
											uint32_t repeat = 0, int device = 0, int source = 0);
		Qk_DEFINE_PROP_GET(float, x, Const);
		Qk_DEFINE_PROP_GET(float, y, Const);
	};

	/**
	* @class HighlightedEvent status event
	*/
	class Qk_EXPORT HighlightedEvent: public UIEvent {
	public:
		enum Status {
			kNormal = 1, kHover, kActive,
		};
		HighlightedEvent(View* origin, Status status);
		Qk_DEFINE_PROP_GET(Status, status, Const);
	};

	typedef HighlightedEvent::Status HighlightedStatus;

	/**
	* @class TouchEvent touch event
	*/
	class Qk_EXPORT TouchEvent: public UIEvent {
	public:
		struct TouchPoint { // touch event point
			uint32_t id;
			float    start_x, start_y;
			float    x, y, force;
			bool     click_in;
			View     *view;
		};
		TouchEvent(View* origin, Array<TouchPoint>& touches);
		Array<TouchPoint>& changed_touches() { return _change_touches; }
	private:
		Array<TouchPoint> _change_touches;
	};

	typedef TouchEvent::TouchPoint TouchPoint;

	class Qk_EXPORT EventDispatch: public Object {
	public:
		struct KeyboardOptions {
			bool               is_clear;
			KeyboardType       type;
			KeyboardReturnType return_type;
			Vec2               spot_location;
		};
		Qk_DEFINE_PROP_GET(Application*, host);
		Qk_DEFINE_PROP_GET(Window*, window);
		Qk_DEFINE_PROP_GET(KeyboardAdapter*, keyboard);
		Qk_DEFINE_PROP_GET(View*, focus_view);

		EventDispatch(Window* win);
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
		void onKeyboardDown();
		void onKeyboardUp();
		// setting state
		void set_volume_up();
		void set_volume_down();
		void set_ime_keyboard_open(KeyboardOptions options);
		void set_ime_keyboard_can_backspace(bool can_back_space, bool can_delete);
		void set_ime_keyboard_close();
		void set_ime_keyboard_spot_location(Vec2 location);
		bool set_focus_view(View *view); // set focus from main thread
	private:
		void touchstartErase(View *view, List<TouchPoint>& in);
		void touchstart(Layout* layout, List<TouchPoint>& in);
		void touchmove(List<TouchPoint>& in);
		void touchend(List<TouchPoint>& in, const UIEventName& type);
		void mousemove(View* view, Vec2 pos);
		void mousepress(View* view, Vec2 pos, KeyboardKeyName name, bool down);
		void mousewhell(KeyboardKeyName name, bool down, float x, float y);
		View* find_receive_view_exec(Layout *view, Vec2 pos);
		Sp<View, ReferenceTraits> find_receive_view(Vec2 pos);
		Sp<MouseEvent> NewMouseEvent(View *view, float x, float y, uint32_t keycode = 0);
		Sp<View, ReferenceTraits> get_focus_view();

		class OriginTouche;
		class MouseHandle;
		Dict<View*, OriginTouche*> _origin_touches;
		MouseHandle *_mouse_handle;
		std::atomic<TextInput*> _text_input;
		RecursiveMutex _view_mutex; // view layout mutex for main and render thread
		friend class View;
		friend class Layout;
	};

}

#endif
