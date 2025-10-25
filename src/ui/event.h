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
#include "./events.h"

namespace qk {
	class Application;
	class View;
	class Agent;
	class Action;
	class TextInput;
	class Window;
	class UIEventName;
	typedef const UIEventName cUIEventName;

	enum {
		kDefault_UIEventCategory,
		kClick_UIEventCategory,
		kTouch_UIEventCategory,
		kMouse_UIEventCategory,
		kHighlighted_UIEventCategory,
		kKeyboard_UIEventCategory,
		kAction_UIEventCategory,
		kPlayer_UIEventCategory,
		kSpine_UIEventCategory,
		kAgent_UIEventCategory,
	};

	// event flags / cast
	enum {
		kNone_UIEventFlags, // none flag
		kError_UIEventFlags,   // type Error
		kFloat32_UIEventFlags, // type Float
		kUint64_UIEventFlags,  // type Uint64
		kTypesMask_UIEventFlags = (255), // Event::data(), data types
		kBubble_UIEventFlags = (1 << 8), // is bubble
	};

	// event returl value mask
	enum ReturnValueMask {
		kDefault_ReturnValueMask = (1 << 0),
		kBubble_ReturnValueMask = (1 << 1),
		kAll_ReturnValueMask = (kDefault_ReturnValueMask | kBubble_ReturnValueMask),
	};

	class Qk_EXPORT UIEventName {
	public:
		Qk_DEFINE_PROP_GET(uint32_t, category, Const);
		Qk_DEFINE_PROP_GET(uint32_t, flag, Const);
		Qk_DEFINE_PROP_GET(uint64_t, hashCode, Const);
		Qk_DEFINE_PROP_GET(String, string, Const);
		UIEventName(cString& name, uint32_t category, uint32_t flag);
		inline bool equals(cUIEventName& v) const { return v._hashCode == _hashCode; }
		inline bool operator==(cUIEventName& v) const { return v._hashCode == _hashCode; }
		inline bool operator!=(cUIEventName& v) const { return v._hashCode != _hashCode; }
		inline bool operator>(cUIEventName& v) const { return _hashCode > v._hashCode; }
		inline bool operator<(cUIEventName& v) const { return _hashCode < v._hashCode; }
	};

	// event names string => UIEventName
	Qk_EXPORT extern const Dict<String, UIEventName> UIEventNames;

	// define event names
#define _Fun(Name, C, F) \
	Qk_EXPORT extern cUIEventName UIEvent_##Name;
	Qk_UI_Events(_Fun)
#undef _Fun
	// -----------------------------------

	/**
	* @class UIEvent gui event
	*/
	class Qk_EXPORT UIEvent: public Event<> {
		Qk_DISABLE_COPY(UIEvent);
	public:
		UIEvent(View *origin, SendData data = nullptr);
		Qk_DEFINE_PROP_GET(View*, origin);
		Qk_DEFINE_PROP_GET(int64_t, timestamp, Const); // Unit milliseconds
		inline bool is_default() const { return return_value & kDefault_ReturnValueMask; }
		inline bool is_bubble() const { return return_value & kBubble_ReturnValueMask; }
		inline void cancel_default() { return_value &= ~kDefault_ReturnValueMask; }
		inline void cancel_bubble() { return_value &= ~kBubble_ReturnValueMask; }
		void release() override;
	};

	/**
	* @class ActionEvent
	*/
	class Qk_EXPORT ActionEvent: public UIEvent {
	public:
		ActionEvent(Action* action, View* origin, uint64_t delay, uint32_t frame, uint32_t looped);
		Qk_DEFINE_PROP_GET(Action*, action);
		Qk_DEFINE_PROP_GET(uint64_t, delay, Const);
		Qk_DEFINE_PROP_GET(uint32_t, frame, Const);
		Qk_DEFINE_PROP_GET(uint32_t, looped, Const);
		void release() override;
	};

	/**
	* @func KeyEvent keyboard event
	*/
	class Qk_EXPORT KeyEvent: public UIEvent {
	public:
		KeyEvent(View* origin, KeyboardKeyCode keycode, int keypress,
						bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
						uint32_t repeat, int device, int source);
		Qk_DEFINE_PROPERTY(View*, next_focus);
		Qk_DEFINE_PROPERTY(KeyboardKeyCode, keycode, Const);
		Qk_DEFINE_PROPERTY(int, keypress, Const);
		Qk_DEFINE_PROP_GET(uint32_t, repeat, Const);
		Qk_DEFINE_PROP_GET(uint32_t, device, Const);
		Qk_DEFINE_PROP_GET(uint32_t, source, Const);
		Qk_DEFINE_PROP_GET(bool, shift, Const);
		Qk_DEFINE_PROP_GET(bool, ctrl, Const);
		Qk_DEFINE_PROP_GET(bool, alt, Const);
		Qk_DEFINE_PROP_GET(bool, command, Const);
		Qk_DEFINE_PROP_GET(bool, caps_lock, Const);
		void release() override;
	};

	/**
	* @class ClickEvent click event
	*/
	class Qk_EXPORT ClickEvent: public UIEvent {
	public:
		enum Type {
			kTouch = 1, kKeyboard, kMouse
		};
		ClickEvent(View* origin, Vec2 position, Type type, uint32_t count = 1);
		Qk_DEFINE_PROP_GET(Vec2, position, Const);
		Qk_DEFINE_PROP_GET(uint32_t, count, Const);
		Qk_DEFINE_PROP_GET(Type, type, Const);
	};

	/**
	* @class UIMouseEvent mouse event
	*/
	class Qk_EXPORT MouseEvent: public KeyEvent {
	public:
		MouseEvent(View* origin, Vec2 position, KeyboardKeyCode keycode, int keypress,
											bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
											uint32_t repeat = 0, int device = 0, int source = 0);
		Qk_DEFINE_PROP_GET(Vec2, position, Const);
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
			Vec2   start_position; // start position
			Vec2   position; // current position
			float  force; // touch force
			bool   click_valid; // is valid for click
			View   *view; // init view when touchstart
		};
		TouchEvent(View* origin, Array<TouchPoint>& touches);
		cArray<TouchPoint>& changed_touches() const {
			return _change_touches;
		}
		void release() override;
	private:
		Array<TouchPoint> _change_touches;
	};

	typedef TouchEvent::TouchPoint TouchPoint;

	/**
	 * @class ArrivePositionEvent arrive position event
	*/
	class Qk_EXPORT ArrivePositionEvent: public UIEvent {
	public:
		ArrivePositionEvent(View *origin, Vec2 position, Vec2 nextLocation, uint32_t waypoint_index);
		Qk_DEFINE_PROP_GET(Vec2, position, Const);
		Qk_DEFINE_PROP_GET(Vec2, nextLocation, Const); // vector to next waypoint
		Qk_DEFINE_PROP_GET(uint32_t, waypointIndex, Const);
	};

	/**
	 * @class DiscoveryAgentEvent discovery agent event
	 */
	class Qk_EXPORT DiscoveryAgentEvent: public UIEvent {
	public:
		DiscoveryAgentEvent(View *origin, Agent* agent, Vec2 location, uint32_t id, int level, bool entering);
		Qk_DEFINE_PROP_GET(Agent*, agent); // other agent
		Qk_DEFINE_PROP_GET(Vec2, location, Const); // to agent location
		Qk_DEFINE_PROP_GET(uint32_t, agentId, Const); // usually use agent's pointer address low 32 bits
		Qk_DEFINE_PROP_GET(int, level, Const); // discovery level, -1 means lost all levels
		Qk_DEFINE_PROP_GET(bool, entering, Const); // is leaving or entering
		void release() override;
	};

	class Qk_EXPORT FollowTargetEvent: public UIEvent {
	public:
		enum State { kStart = 1, kStop, kCancel };
		FollowTargetEvent(View *origin, State state);
		Qk_DEFINE_PROP_GET(State, state, Const);
	};

	class Qk_EXPORT EventDispatch: public Object {
	public:
		struct KeyboardOptions {
			bool               clear;
			KeyboardType       type;
			KeyboardReturnType return_type;
			Rect               spot_rect;
		};
		Qk_DEFINE_PROP_GET(Application*, host);
		Qk_DEFINE_PROP_GET(Window*, window);
		Qk_DEFINE_PROP_GET(KeyboardAdapter*, keyboard);
		Qk_DEFINE_PROP_GET(View*, focusView);

		EventDispatch(Window* win);
		~EventDispatch();
		// handles
		void onTouchstart(List<TouchPoint>&& touches);
		void onTouchmove(List<TouchPoint>&& touches);
		void onTouchend(List<TouchPoint>&& touches);
		void onTouchcancel(List<TouchPoint>&& touches);
		void onMousemove(float x, float y);
		void onMousepress(KeyboardKeyCode key, bool isDown, const Vec2 *vec);
		// ime
		void onImeDelete(int count);
		void onImeInsert(cString& text);
		void onImeMarked(cString& text);
		void onImeUnmark(cString& text);
		void onImeControl(KeyboardKeyCode code);
		// keyboard work loop call
		void onKeyboardDown();
		void onKeyboardUp();
		// setting state
		void setVolumeUp();
		void setVolumeDown();
		void setImeKeyboardOpen(KeyboardOptions options);
		void setImeKeyboardCanBackspace(bool can_back_space, bool can_delete);
		void setImeKeyboardClose();
		void setImeKeyboardSpotRect(Rect rect);
		bool setFocusView(View *view); // set focus from main thread
	private:
		void touchstartErase(View *view, List<TouchPoint>& in);
		void touchstart(View* view, List<TouchPoint>& in);
		void touchmove(List<TouchPoint>& in);
		void touchend(List<TouchPoint>& in, bool isCancel);
		void mousemove(View* view, Vec2 pos);
		void mousepress(View* view, Vec2 pos, KeyboardKeyCode code, bool down);
		View* find_receive_view_exec(View *view, Vec2 pos);
		View* find_receive_view_and_retain(Vec2 pos);
		Sp<MouseEvent> NewMouseEvent(View *view, Vec2 pos, KeyboardKeyCode keycode);
		Sp<View> safe_focus_view();

		class OriginTouche;
		class MouseHandle;
		Dict<View*, OriginTouche*> _origin_touches;
		MouseHandle *_mouse_handle;
		std::atomic<TextInput*> _text_input;
		Mutex _focus_view_mutex; // get set focus view mutex for main and render thread
		friend class View;
		friend class View;
	};

}

#endif
