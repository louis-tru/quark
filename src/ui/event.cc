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

#include "./event.h"
#include "./app.h"
#include "./window.h"
#include "./view/view.h"
#include "./css/css.h"
#include "./view/root.h"
#include "./keyboard.h"
#include "./view/button.h"
#include "./action/action.h"

namespace qk {

	static inline HighlightedEvent::Status HOVER_or_NORMAL(View *view) {
		return view->is_focus() ? HighlightedEvent::kHover : HighlightedEvent::kNormal;
	}

	template<class T, typename... Args>
	inline static Handle<T> NewEvent(Args... args) { return new T(args...); }

	// -------------------------- V i e w --------------------------

	Qk_DEFINE_INLINE_MEMBERS(View, InlEvent) {
	public:
		#define _inl_view(self) static_cast<View::InlEvent*>(static_cast<View*>(self))

		void bubble_trigger(const NameType &name, UIEvent &evt) {
			View *view = this;
			do {
				if ( view->_receive ) {
					view->trigger(name, evt);
					if ( !evt.is_bubble() ) break; // Stop bubble
				}
				view = view->_parent;
			} while( view );
		}

		void trigger_click(UIEvent &evt) {
			bubble_trigger(UIEvent_Click, evt);
			if ( evt.is_default() ) {
				auto focus_view = _window->dispatch()->_focusView;
				auto root = _window->root();
				if (focus_view != evt.origin() && focus_view != root) {
					if (!focus_view->is_self_child(evt.origin())) {
						root->focus(); // root
					}
				}
			}
		}
		
		void trigger_highlightted(HighlightedEvent &evt) {
			bubble_trigger(UIEvent_Highlighted, evt);
			if ( evt.is_default() ) {
				/*
				// TODO: Can't running this code on iphone6
				auto execTest = [](){
					Qk_DLog("Test");
				};
				execTest();

				typedef void (*ExecFunc)();

				decltype(execTest) b = execTest;

				ExecFunc ex0 = execTest;
				std::function<void()> ex1 = std::function<void()>(execTest);

				ex1();

				auto ex2 = (void*)ex0;

				((ExecFunc)ex2)();
				*/

				preRender().async_call([](auto self, auto arg) {
					do {
						auto ss = self->_cssclass.load();
						if (ss)
							ss->setStatus_Rt(arg.arg);
						self = self->parent();
					} while(self);
				}, (View*)this, CSSType(evt.status()));

			}
		}

	};

	bool View::focus() {
		if ( is_focus() ) return true;

		auto dispatch = _window->dispatch();
		auto old = dispatch->focusView();

		if ( !dispatch->setFocusView(this) ) {
			return false;
		}

		if ( old ) {
			_inl_view(old)->bubble_trigger(UIEvent_Blur, **NewEvent<UIEvent>(old));
			_inl_view(old)->trigger_highlightted(
				**NewEvent<HighlightedEvent>(old, HighlightedStatus::kNormal)
			);
		}
		_inl_view(this)->bubble_trigger(UIEvent_Focus, **NewEvent<UIEvent>(this));
		_inl_view(this)->trigger_highlightted(
			**NewEvent<HighlightedEvent>(this, HighlightedStatus::kHover)
		);
		return true;
	}

	// -------------------------- E v e n t --------------------------

	UIEventName::UIEventName(cString& name, uint32_t category, uint32_t flag)
		: _toString(name), _hashCode((uint32_t)name.hashCode()), _category(category), _flag(flag)
	{}

	UIEvent::UIEvent(View *origin, SendData data)
		: Event(data, kAll_ReturnValueMask), _origin(origin), _timestamp(time_micro()) {
	}

	void UIEvent::release() {
		_sender = nullptr;
		_origin = nullptr;
		Object::release();
	}

	ActionEvent::ActionEvent(Action* action, View* origin, uint64_t delay, uint32_t frame, uint32_t looped)
		: UIEvent(origin), _action(action), _delay(delay), _frame(frame), _looped(looped)
	{
		action->retain();
	}

	void ActionEvent::release() {
		_action->release();
		_action = nullptr;
		UIEvent::release();
	}

	KeyEvent::KeyEvent(View* origin, KeyboardKeyCode keycode, int keypress,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
										uint32_t repeat, int device, int source)
		: UIEvent(origin), _keycode(keycode), _keypress(keypress)
		, _device(device), _source(source), _repeat(repeat), _shift(shift)
		, _ctrl(ctrl), _alt(alt), _command(command)
		, _caps_lock(caps_lock), _next_focus(nullptr)
	{}

	void KeyEvent::set_next_focus(View *view) {
		if (origin())
			_next_focus = view;
	}

	void KeyEvent::set_keycode(KeyboardKeyCode keycode) {
		_keycode = keycode;
	}

	void KeyEvent::release() {
		_next_focus = nullptr;
		UIEvent::release();
	}

	ClickEvent::ClickEvent(View* origin, float x, float y, Type type, uint32_t count)
		: UIEvent(origin), _x(x), _y(y), _count(count), _type(type)
	{}

	MouseEvent::MouseEvent(View* origin, float x, float y, KeyboardKeyCode keycode, int keypress,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
										uint32_t repeat, int device, int source)
		: KeyEvent(origin, keycode, keypress, shift, ctrl, alt, command, caps_lock
			, repeat, device, source
		), _x(x), _y(y)
	{}

	HighlightedEvent::HighlightedEvent(View* origin, HighlightedStatus status)
		: UIEvent(origin), _status(status)
	{}

	TouchEvent::TouchEvent(View* origin, Array<TouchPoint>& touches)
		: UIEvent(origin), _change_touches(touches)
	{}

	/**
	 * @class EventDispatch::OriginTouche
	 */
	class EventDispatch::OriginTouche {
	public:
		~OriginTouche() {
			_view->release();
		}
		static Vec2 position(View* view) {
			return view->position();
		}
		inline View* view() { return _view; }
		inline Vec2 view_start_position() { return _start_position; }
		inline bool is_click_invalid() { return _is_click_invalid; }
		inline bool is_click_down() { return _is_click_down; }
		inline void set_click_invalid() {
			_is_click_invalid = true;
			_is_click_down = false;
		}
		inline void set_is_click_down(bool value) {
			if ( !_is_click_invalid )
				_is_click_down = value;
		}
		inline Dict<uint32_t, TouchPoint>& values() { return _touches; }
		inline TouchPoint& operator[](uint32_t id) { return _touches[id]; }
		inline uint32_t count() { return _touches.length(); }
		inline bool has(uint32_t id) { return _touches.has(id); }
		inline void del(uint32_t id) { _touches.erase(id); }
		static OriginTouche* Make(View* view) {
			return view->tryRetain() ? new OriginTouche(view): nullptr;
		}
	private:
		OriginTouche(View* view)
			: _view(view)
			, _start_position(position(view))
			, _is_click_invalid(false), _is_click_down(false)
		{}
		View* _view;
		Dict<uint32_t, TouchPoint> _touches;
		Vec2  _start_position;
		bool  _is_click_invalid;
		bool  _is_click_down;
	};

	/**
	 * @class EventDispatch::MouseHandle
	 */
	class EventDispatch::MouseHandle {
	public:
		MouseHandle(): _view(nullptr), _click_view(nullptr) {}
		~MouseHandle() {
			Releasep(_view);
			Releasep(_click_view);
		}
		inline View* view() { return _view; }
		inline Vec2 view_start_position() { return _start_position; }
		inline Vec2 position() { return _position; }
		inline View* click_down_view() { return _click_view; }
		inline void set_position(Vec2 value) { _position = value; }
		void set_click_down_view_Mt(View* view) {
			Release(_click_view);
			if (view) {
				view->retain();
				_start_position = OriginTouche::position(view);
			}
			_click_view = view;
		}
		void set_view_Mt(View* view) {
			Release(_view);
			Retain(view);
			_view = view;
		}
	private:
		View *_view, *_click_view;
		Vec2 _start_position, _position;
	};

	#define _Fun(Name, C, Flag) \
	const UIEventName UIEvent_##Name(#Name, k##C##_UIEventCategory, Flag);
	Qk_UI_Events(_Fun)
	#undef _Fun

	const Dict<String, UIEventName> UIEventNames([]() {
		Dict<String, UIEventName> r;
		#define _Fun(Name, C, F) \
			r.set(UIEvent_##Name.toString(), UIEvent_##Name);
		Qk_UI_Events(_Fun)
		#undef _Fun
		Qk_ReturnLocal(r);
	}());

	// @EventDispatch
	// ----------------------------------------------------------------------------------------
	#define _pre _window->preRender()

	EventDispatch::EventDispatch(Window* win)
		: _window(win)
		, _host(win->host())
		, _text_input(nullptr), _focusView(nullptr)
	{
		_keyboard = KeyboardAdapter::create();
		_keyboard->_host = this;
		_mouse_handle = new MouseHandle();
	}

	EventDispatch::~EventDispatch() {
		for (auto& i : _origin_touches)
			delete i.value;
		if ( _focusView ) {
			_focusView->release();
			_focusView = nullptr;
		}
		Release(_keyboard);
		delete _mouse_handle;
	}

	Sp<View> EventDispatch::safe_focus_view() {
		ScopeLock lock(_focus_view_mutex);
		return Sp<View>(_focusView);
	}

	bool EventDispatch::setFocusView(View *view) {
		if ( _focusView != view ) {
			if ( view->_level && view->can_become_focus() ) {
				Lock lock(_focus_view_mutex);
				if ( _focusView ) {
					_focusView->release(); // unref
				}
				_focusView = view;
				_focusView->retain(); // strong ref
				lock.unlock();
				// set text input
				auto input = view->asTextInput();
				if ( _text_input != input ) {
					_text_input = input;
					if ( input ) {
						setImeKeyboardOpen({
							true,
							input->input_keyboard_type(),
							input->input_keyboard_return_type(),
							input->input_spot_rect(),
						});
					} else {
						setImeKeyboardClose();
					}
				} else if ( input ) {
					setImeKeyboardOpen({
						false,
						input->input_keyboard_type(),
						input->input_keyboard_return_type(),
						input->input_spot_rect(),
					});
				} // if ( _text_input != input ) {
			} else {
				return false;
			}
		}
		return true;
	}

	typedef Callback<List<TouchPoint>> TouchCb;

	// -------------------------- T o u c h --------------------------

	void EventDispatch::touchstartErase(View *view, List<TouchPoint> &in) {
		if ( view->_receive && in.length() ) {
			Array<TouchPoint> change_touches;

			for ( auto i = in.begin(), e = in.end(); i != e; ) {
				if ( view->overlap_test(Vec2(i->x, i->y)) ) {
					TouchPoint& touch = *i;
					touch.start_x = touch.x;
					touch.start_y = touch.y;
					touch.click_in = true;
					touch.view = view;

					OriginTouche *out = nullptr;
					if ( !_origin_touches.get(view, out) ) {
						if ((out = OriginTouche::Make(view))) {
							_origin_touches[view] = out;
						}
					}
					if (out) {
						out->operator[](touch.id) = touch;
						change_touches.push(touch);
						in.erase(i++);
					} else {
						i++;
					}
				} else {
					i++;
				}
			}

			if ( change_touches.length() ) { // notice
				auto clickDownNo = !_origin_touches[view]->is_click_down();
				if (clickDownNo) { // trigger click down
					_origin_touches[view]->set_is_click_down(true);
				}
				// post main thread
				_pre.post_main(Cb([view, clickDownNo, change_touches](auto& e) {
					auto evt = NewEvent<TouchEvent>(view, change_touches);
					_inl_view(view)->bubble_trigger(UIEvent_TouchStart, **evt); // emit event
					if ( clickDownNo ) {
						auto evt = NewEvent<HighlightedEvent>(view, HighlightedStatus::kActive);
						_inl_view(view)->trigger_highlightted(**evt); // emit event
					}
				}), view);
			}
		}
	}

	void EventDispatch::touchstart(View *view, List<TouchPoint> &in) {
		if ( view->_visible && in.length() ) {
			if ( view->_visible_region ) {
				if ( view->last() && view->is_clip() ) {
					List<TouchPoint> clipIn;

					for ( auto i = in.begin(), e = in.end(); i != e; ) {
						if ( view->overlap_test(Vec2(i->x, i->y)) ) {
							clipIn.pushBack(*i);
							in.erase(i++);
						} else {
							i++;
						}
					}
					auto v = view->last();
					while( v && clipIn.length() ) {
						touchstart(v, clipIn);
						v = v->prev();
					}
					touchstartErase(view, clipIn);

					if ( clipIn.length() ) {
						in.splice(in.end(), clipIn);
					}
				} else {
					auto v = view->last();
					while( v && in.length() ) {
						touchstart(v, in);
						v = v->prev();
					}
					touchstartErase(view, in);
				}
			}
		}
	}

	void EventDispatch::touchmove(List<TouchPoint>& in) {
		Dict<View*, Array<TouchPoint>> change_touches;

		for ( auto in_touch : in ) {
			for ( auto touches : _origin_touches ) {
				if ( touches.value->has(in_touch.id) ) {
					TouchPoint& touch = (*touches.value)[in_touch.id];
					touch.x = in_touch.x;
					touch.y = in_touch.y;
					touch.force = in_touch.force;
					if ( !touches.value->is_click_invalid() ) {
						touch.click_in = touch.view->overlap_test(Vec2(touch.x, touch.y));
					}
					change_touches[touch.view].push(touch);
					break;
				}
			}
		}

		for ( auto i : change_touches ) {
			auto& touchs = i.value;
			View* view = touchs[0].view;

			_pre.post_main(Cb([view, touchs](auto &e) {// emit event
				_inl_view(view)->bubble_trigger(UIEvent_TouchMove, **NewEvent<TouchEvent>(view, touchs));
			}), view);

			OriginTouche* origin_touche = _origin_touches[view];

			if ( !origin_touche->is_click_invalid() ) { // no invalid
				Vec2 position = OriginTouche::position(view);
				Vec2 start_position = origin_touche->view_start_position();

				float d = sqrtf(powf((position.x() - start_position.x()), 2) +
												powf((position.y() - start_position.y()), 2));

				// 视图位置移动超过2取消点击状态
				if ( d > 2 ) { // trigger invalid status
					if ( origin_touche->is_click_down() ) { // trigger style up
						_pre.post_main(Cb([view](auto &e){ // emit style status event
							auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
							_inl_view(view)->trigger_highlightted(**evt);
						}), view);
					}
					origin_touche->set_click_invalid();
				}
				else { // no invalid
					if ( origin_touche->is_click_down() ) { // May trigger click up
						bool trigger_event = true;
						for ( auto t : origin_touche->values() ) {
							if (t.value.click_in) {
								trigger_event = false;
								break;
							}
						}
						if ( trigger_event ) {
							origin_touche->set_is_click_down(false); // set up status
							_pre.post_main(Cb([view](auto &e) { // emit style status event
								auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
								_inl_view(view)->trigger_highlightted(**evt);
							}), view);
						}
					} else { // May trigger click down
						for ( uint32_t i = 0; i < touchs.length(); i++) {
							auto item = touchs[i];
							if ( item.click_in ) { // find range == true
								origin_touche->set_is_click_down(true); // set down status
								_pre.post_main(Cb([view](auto &e) { // emit style down event
									auto evt = NewEvent<HighlightedEvent>(view, HighlightedStatus::kActive);
									_inl_view(view)->trigger_highlightted(**evt);
								}), view);
								break;
							}
						}
					}
				} // no invalid end
			} // if end
		} // each end
	}

	void EventDispatch::touchend(List<TouchPoint>& in, const UIEventName& type) {
		Dict<View*, Array<TouchPoint>> change_touches;

		for ( auto& in_touch : in ) {
			for ( auto& item : _origin_touches ) {
				if ( item.value->has(in_touch.id) ) {
					TouchPoint& touch = (*item.value)[in_touch.id];
					touch.x = in_touch.x;
					touch.y = in_touch.y;
					touch.force = in_touch.force;
					change_touches[touch.view].push(touch);
					item.value->del(touch.id); // del touch point
					break;
				}
			}
		}

		for ( auto& i : change_touches ) { // views
			auto& touchs = i.value;
			View* view = touchs[0].view;
			auto origin_touche = _origin_touches[view];
			auto is_touches_zero = origin_touche->count() == 0;

			struct Core: CallbackCore<Object> {
				View *view;
				UIEventName type;
				Array<TouchPoint> touchs;
				OriginTouche *origin_touche;
				bool is_touches_zero;
				Core(View* v,const UIEventName& t,Array<TouchPoint> &to, OriginTouche* ot,bool is_t)
					: view(v), type(t), touchs(std::move(to)), origin_touche(ot), is_touches_zero(is_t) {}
				~Core() {
					if (is_touches_zero)
						delete origin_touche;
				}
				void call(Data& d) {
					bool is_click_down = is_touches_zero && origin_touche->is_click_down();
					auto evt0 = NewEvent<TouchEvent>(view, touchs);
					_inl_view(view)->bubble_trigger(type, **evt0); // emit touch end event

					if ( is_click_down ) { // trigger click
						for ( auto& item : touchs ) {
							if ( item.click_in ) { // find range == true
								auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
								_inl_view(view)->trigger_highlightted(**evt);

								if ( evt0->is_default() && type == UIEvent_TouchEnd && view->_level ) {
									auto evt = NewEvent<ClickEvent>(view, item.x, item.y, ClickEvent::kTouch);
									_inl_view(view)->trigger_click(**evt); // emit click event
								}
								break;
							}
						}
					}
				}
			};

			if (is_touches_zero) {
				_origin_touches.erase(view); // del
			}
			_pre.post_main(Cb(new Core(view, type, touchs, origin_touche, is_touches_zero)), view);
		}
	}

	void EventDispatch::onTouchstart(List<TouchPoint>&& list) {
		Qk_DLog("onTouchstart x: %f, y: %f", list.front().y, list.front().y);
		UILock lock(_window);
		auto r = _window->root();
		if (r) {
			touchstart(r, list);
		}
	}

	void EventDispatch::onTouchmove(List<TouchPoint>&& list) {
		Qk_DLog("onTouchmove x: %f, y: %f", list.front().y, list.front().y);
		UILock lock(_window);
		touchmove(list);
	}

	void EventDispatch::onTouchend(List<TouchPoint>&& list) {
		Qk_DLog("onTouchend x: %f, y: %f", list.front().y, list.front().y);
		UILock lock(_window);
		touchend(list, UIEvent_TouchEnd);
	}

	void EventDispatch::onTouchcancel(List<TouchPoint>&& list) {
		Qk_DLog("onTouchcancel x: %f, y: %f", list.front().y, list.front().y);
		UILock lock(_window);
		touchend(list, UIEvent_TouchCancel);
	}

// -------------------------- M o u s e --------------------------

	View* EventDispatch::find_receive_view_exec(View* view, Vec2 pos) {
		if ( view->visible() && view->visible_region() ) {
			auto v = view->last();
			if (v && view->is_clip() ) {
				if (view->overlap_test(pos)) {
					while (v) {
						auto r = find_receive_view_exec(v, pos);
						if (r) {
							return r;
						}
						v = v->prev();
					}
					if (view->_receive) {
						return view;
					}
				}
			} else {
				while (v) {
					auto r = find_receive_view_exec(v, pos);
					if (r) {
						return r;
					}
					v = v->prev();
				}
				if (view->_receive && view->overlap_test(pos)) {
					return view;
				}
			}
		}
		return nullptr;
	}

	View* EventDispatch::find_receive_view(Vec2 pos) {
		auto root = _window->root();
		if (root) {
			auto r = find_receive_view_exec(root, pos);
			return r ? r: root;
		} else {
			return nullptr;
		}
	}

	Sp<MouseEvent> EventDispatch::NewMouseEvent(View* view, float x, float y, KeyboardKeyCode keycode) {
		return NewEvent<MouseEvent>(view, x, y, keycode, 0,
			_keyboard->shift(),
			_keyboard->ctrl(), _keyboard->alt(),
			_keyboard->command(), _keyboard->caps_lock(), 0, 0, 0
		);
	}

	void EventDispatch::mousemove(View *view, Vec2 pos) {
		View* d_view = _mouse_handle->click_down_view();
		if ( d_view ) { // no invalid
			Vec2 position = OriginTouche::position(d_view);
			Vec2 start_position = _mouse_handle->view_start_position();
			float d = sqrtf(powf((position.x() - start_position.x()), 2) +
											powf((position.y() - start_position.y()), 2));
			// 视图位置移动超过2取消点击状态
			if ( d > 2 ) { // trigger invalid status
				if (view == d_view) {
					_inl_view(view)->trigger_highlightted( // emit style status event
						**NewEvent<HighlightedEvent>(view, HighlightedStatus::kHover));
				}
				_mouse_handle->set_click_down_view_Mt(nullptr);
			}
		}

		float x = pos[0], y = pos[1];

		View* old = _mouse_handle->view();

		if (old != view) {
			_mouse_handle->set_view_Mt(view);

			if (old) {
				auto evt = NewMouseEvent(old, x, y, KEYCODE_UNKNOWN);
				_inl_view(old)->bubble_trigger(UIEvent_MouseOut, **evt);

				if (evt->is_default()) {
					evt->return_value = kAll_ReturnValueMask;

					if (!view || !old->is_self_child(view)) {
						_inl_view(old)->bubble_trigger(UIEvent_MouseLeave, **evt);
					}
					_inl_view(old)->trigger_highlightted( // emit style status event
						**NewEvent<HighlightedEvent>(old, HighlightedStatus::kNormal)
					);
				}
			}
			if (view) {
				auto evt = NewMouseEvent(view, x, y, KEYCODE_UNKNOWN);
				_inl_view(view)->bubble_trigger(UIEvent_MouseOver, **evt);
				_window->setCursorStyle(view->cursor(), true);

				if (evt->is_default()) {
					evt->return_value = kAll_ReturnValueMask;

					if (!old || !view->is_self_child(old)) {
						_inl_view(view)->bubble_trigger(UIEvent_MouseEnter, **evt);
					}
					_inl_view(view)->trigger_highlightted( // emit style status event
						**NewEvent<HighlightedEvent>(view,
							view == d_view ? HighlightedStatus::kActive: HighlightedStatus::kHover)
					);
				}
			}
		}
		else if (view) {
			_inl_view(view)->bubble_trigger(UIEvent_MouseMove, **NewMouseEvent(view, x, y, KEYCODE_UNKNOWN));
		}
	}

	void EventDispatch::mousepress(View *view, Vec2 pos, KeyboardKeyCode code, bool down) {
		float x = pos[0], y = pos[1];

		if (_mouse_handle->view() != view) {
			mousemove(view, pos);
		}
		if (!view) return;

		auto evt = NewMouseEvent(view, x, y, code);

		Sp<View> raw_down_view = _mouse_handle->click_down_view();

		if (down) {
			_mouse_handle->set_click_down_view_Mt(view);
			_inl_view(view)->bubble_trigger(UIEvent_MouseDown, **evt);
			_window->setCursorStyle(view->cursor(), true);
		} else {
			_mouse_handle->set_click_down_view_Mt(nullptr);
			_inl_view(view)->bubble_trigger(UIEvent_MouseUp, **evt);
		}

		if (code != KEYCODE_MOUSE_LEFT || !evt->is_default())
			return;

		if (down) {
			_inl_view(view)->trigger_highlightted(
				**NewEvent<HighlightedEvent>(view, HighlightedStatus::kActive)); // emit style status event
		} else {
			_inl_view(view)->trigger_highlightted(
				**NewEvent<HighlightedEvent>(view, HighlightedStatus::kHover)); // emit style status event

			if (view == *raw_down_view) {
				_inl_view(view)->trigger_click(**NewEvent<ClickEvent>(view, x, y, ClickEvent::kMouse));
			}
		}
	}

	void EventDispatch::onMousemove(float x, float y) {
		UILock lock(_window);
		if (_window->root()) {
			Vec2 pos(x, y);
			// Qk_DLog("onMousemove x: %f, y: %f", x, y);
			_mouse_handle->set_position(pos); // set current mouse pos
			auto v = find_receive_view(pos);
			if (v) {
				_pre.post_main(Cb([this,v,pos](auto& e) { mousemove(v, pos); }),v);
			}
		}
	}

	void EventDispatch::onMousepress(KeyboardKeyCode code, bool isDown, const Vec2 *val) {
		Vec2 deltaDefault;
		switch(code) {
			case KEYCODE_MOUSE_LEFT:
			case KEYCODE_MOUSE_CENTER:
			case KEYCODE_MOUSE_RIGHT: {
				UILock lock(_window);
				auto pos = val ? *val: _mouse_handle->position(); // get current mouse pos
				auto v = find_receive_view(pos);
				//Qk_DLog("onMousepress code: %d, isDown: %i, v: %p", code, isDown, v);
				if (v)
					_pre.post_main(Cb([this,v,pos,code,isDown](auto& e) { mousepress(v, pos, code, isDown); }),v);
				break;
			}
			case KEYCODE_MOUSE_WHEEL_UP:
				deltaDefault = Vec2(0, 10); goto whell;
			case KEYCODE_MOUSE_WHEEL_DOWN:
				deltaDefault = Vec2(0, -10); goto whell;
			case KEYCODE_MOUSE_WHEEL_LEFT:
				deltaDefault = Vec2(10, 0); goto whell;
			case KEYCODE_MOUSE_WHEEL_RIGHT:
				deltaDefault = Vec2(-10, 0); whell:
				if (isDown) {
					auto delta = val ? *val: deltaDefault;
					_pre.post_main(Cb([=](auto e) {
						auto v = _mouse_handle->view();
						if (v) {
							_inl_view(v)->bubble_trigger(UIEvent_MouseWheel,
								**NewMouseEvent(v, delta.x(), delta.y(), code)
							);
						}
					}));
				}
				break;
			default: break;
		}
	}

	// -------------------------- k e y b o a r d --------------------------

	static KeyEvent* NewKeyEvent(View* view, KeyboardAdapter* _keyboard) {
		auto evt = new KeyEvent(view,
			_keyboard->keycode(),
			_keyboard->keypress(),
			_keyboard->shift(),
			_keyboard->ctrl(), _keyboard->alt(),
			_keyboard->command(), _keyboard->caps_lock(),
			_keyboard->repeat(), _keyboard->device(), _keyboard->source()
		);
		return evt;
	}

	void EventDispatch::onKeyboardDown() {
		auto safe_v = safe_focus_view();
		auto view = *safe_v;
		if (!view) view = _window->root();
		if (!view) return;

		auto code = _keyboard->keycode();
		auto btn = view->asButton();
		View *next_focus = nullptr;

		if (btn) {
			FindDirection dir;
			switch ( code ) {
				case KEYCODE_LEFT: dir = FindDirection::Left; break;  // left
				case KEYCODE_UP: dir = FindDirection::Top; break;     // top
				case KEYCODE_RIGHT: dir = FindDirection::Right; break; // right
				case KEYCODE_DOWN: dir = FindDirection::Bottom; break; // bottom
				default: dir = FindDirection::None; break;
			}
			if ( dir != FindDirection::None ) {
				auto view = btn->next_button(dir);
				if (view)
					next_focus = view->tryRetain(); // safe retain view
			}
		}

		auto evt = NewKeyEvent(view, _keyboard);
		evt->set_next_focus(next_focus);

		struct Core: CallbackCore<Object> {
			EventDispatch *host;
			View *view;
			Sp<View> next_focus;
			Sp<KeyEvent> evt;
			Core(EventDispatch *h, View *v, View *nf, KeyEvent *e): host(h), view(v), evt(e) {
				next_focus = Sp<View>::lazy(nf);
			}
			void call(Data& e) {
				auto code = evt->keycode();
				_inl_view(view)->bubble_trigger(UIEvent_KeyDown, **evt);

				if ( evt->is_default() ) {
					if ( code == KEYCODE_ENTER ) {
						_inl_view(view)->bubble_trigger(UIEvent_KeyEnter, **evt);
					} else if (code == KEYCODE_ESC) {
						host->window()->setFullscreen(false);
					} else if ( code == KEYCODE_VOLUME_UP ) {
						host->setVolumeUp();
					} else if ( code == KEYCODE_VOLUME_DOWN ) {
						host->setVolumeDown();
					}

					if ( evt->keypress() ) { // keypress
						_inl_view(view)->bubble_trigger(UIEvent_KeyPress, **evt);
					}

					if ( code == KEYCODE_CENTER && evt->repeat() == 0 ) {
						auto evt = NewEvent<HighlightedEvent>(view, HighlightedStatus::kActive);
						_inl_view(view)->trigger_highlightted(**evt); // emit click status event
					}

					if ( evt->next_focus() ) {
						evt->next_focus()->focus();
					}
				} // if ( evt->is_default() ) {
			}
		};

		_pre.post_main(Cb(new Core(this, view, next_focus, evt)), view); // async_resolve(
	}

	void EventDispatch::onKeyboardUp() {
		auto safe_v = safe_focus_view();
		View* view = *safe_v;
		if (!view) view = _window->root();
		if (!view) return;

		auto mat = view->matrix_view()->matrix();
		auto center = view->client_size() * 0.5f;
		auto matrix = view->asMatrixView();
		if (matrix)
			center -= matrix->origin_value();
		auto point = mat.mul_vec2_no_translate(center) + view->position();

		_pre.post_main(Cb([this,view,point](auto& e) {
			Sp<KeyEvent> evt(NewKeyEvent(view, _keyboard));

			_inl_view(view)->bubble_trigger(UIEvent_KeyUp, **evt);

			if ( evt->is_default() ) {
				if ( evt->keycode() == KEYCODE_BACK ) {
					auto evt = NewEvent<ClickEvent>(view, point.x(), point.y(), ClickEvent::kKeyboard);
					_inl_view(view)->bubble_trigger(UIEvent_Back, **evt); // emit back

					if ( evt->is_default() ) {
						_window->pending();
					}
				}
				else if ( evt->keycode() == KEYCODE_CENTER ) {
					auto evt = NewEvent<HighlightedEvent>(view, HighlightedStatus::kHover);
					_inl_view(view)->trigger_highlightted(**evt); // emit style status event

					auto evt2 = NewEvent<ClickEvent>(view, point.x(), point.y(), ClickEvent::kKeyboard);
					_inl_view(view)->trigger_click(**evt2);
				} //
			}
		}), view); // async_resolve(
	}

	// -------------------------- I M E --------------------------

	void EventDispatch::onImeDelete(int count) {
		UILock lock(_window);
		TextInput* input = _text_input;
		if ( input ) {
			input->input_delete(count);
			bool can_backspace = input->input_can_backspace();
			bool can_delete = input->input_can_delete();
			setImeKeyboardCanBackspace(can_backspace, can_delete);
		}
	}

	void EventDispatch::onImeInsert(cString& text) {
		UILock lock(_window);
		TextInput* input = _text_input;
		if ( input ) {
			input->input_insert(text);
		}
	}

	void EventDispatch::onImeMarked(cString& text) {
		UILock lock(_window);
		TextInput* input = _text_input;
		if ( input ) {
			input->input_marked(text);
		}
	}

	void EventDispatch::onImeUnmark(cString& text) {
		UILock lock(_window);
		TextInput* input = _text_input;
		if ( input ) {
			input->input_unmark(text);
		}
	}

	void EventDispatch::onImeControl(KeyboardKeyCode code) {
		UILock lock(_window);
		TextInput* input = _text_input;
		if ( input ) {
			input->input_control(code);
		}
	}

}
