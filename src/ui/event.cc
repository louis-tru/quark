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

#include "./event.h"
#include "./app.h"
#include "./window.h"
#include "./view/view.h"
#include "./css/css.h"
#include "./view/root.h"
#include "./keyboard.h"
#include "./view/button.h"
#include "./action/action.h"
#include "./view/entity.h"

namespace qk {

	static inline UIState HOVER_or_NORMAL(View *view) {
		return view->is_focus() ? kHover_UIState : kNormal_UIState;
	}

	template<class T, typename... Args>
	inline static Handle<T> NewEvent(Args... args) { return new T(args...); }

	#define _Fun(Name, C, Flags) \
	cUIEventName UIEvent_##Name(#Name, k##C##_UIEventCategory, Flags);
	Qk_UI_Events(_Fun)
	#undef _Fun

	const Dict<String, UIEventName> UIEventNames([]() {
		Dict<String, UIEventName> r;
		#define _Fun(Name, C, F) r.set(UIEvent_##Name.string(), UIEvent_##Name);
		Qk_UI_Events(_Fun)
		#undef _Fun
		Qk_ReturnLocal(r);
	}());

	// -------------------------- V i e w --------------------------

	Qk_DEFINE_INLINE_MEMBERS(View, InlEvent) {
	public:
		#define _inl_view(self) static_cast<View::InlEvent*>(static_cast<View*>(self))

		void bubble_trigger(const NameType &name, UIEvent &evt) {
			View *view = this, *lastTrigger = nullptr;
			do {
				if ( view->_receive ) {
					lastTrigger = view;
					view->trigger(name, evt);
					if ( !evt.is_bubble() ) // Stop bubble
						break; // Stop bubble
				}
				view = view->_parent;
			} while( view );

			if ((name.flags() & kSystem_UIEventFlags) && lastTrigger != _window->root()) {
				// Ensure that the final root can receive system events.
				// and ignore the receive flag of root.
				_window->root()->trigger(name, evt);
			}
		}

		void trigger_click(UIEvent &evt) {
			bubble_trigger(UIEvent_Click, evt);
			if ( evt.is_default() ) {
				auto focus_view = _window->dispatch()->_focusView;
				auto root = _window->root();
				if (focus_view != evt.origin() && focus_view != root) {
					if (!focus_view->is_child(evt.origin())) {
						root->focus(); // root
					}
				}
			}
		}
		
		void trigger_UIStateChange(UIStateEvent &evt) {
			bubble_trigger(UIEvent_UIStateChange, evt);
			if ( evt.is_default() ) {
				preRender().async_call([](auto self, auto arg) {
					do {
						auto ss = self->_cssclass.load();
						if (ss)
							ss->setState_rt(arg.arg);
						self = self->parent();
					} while(self);
				}, (View*)this, evt.state());

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
			_inl_view(old)->trigger_UIStateChange(
				**NewEvent<UIStateEvent>(old, kNormal_UIState)
			);
		}
		_inl_view(this)->bubble_trigger(UIEvent_Focus, **NewEvent<UIEvent>(this));
		_inl_view(this)->trigger_UIStateChange(
			**NewEvent<UIStateEvent>(this, kHover_UIState)
		);
		return true;
	}

	// -------------------------- E v e n t --------------------------

	UIEventName::UIEventName(cString& name, uint32_t category, uint32_t flags)
		: _string(name), _hashCode(name.hashCode()), _category(category), _flags(flags)
	{}

	UIEvent::UIEvent(View *origin, SendData data)
		: Event(data, kAll_ReturnValueMask), _origin(origin), _timestamp(time_millisecond()) {
	}

	void UIEvent::release() {
		_sender = nullptr; // clear weak reference
		_origin = nullptr; // clear weak reference
		Object::release();
	}

	ActionEvent::ActionEvent(Action* action, View* origin, uint64_t delay, uint32_t frame, uint32_t looped)
		: UIEvent(origin), _action(action), _delay(delay), _frame(frame), _looped(looped)
	{
		action->retain(); // retain action
	}

	void ActionEvent::release() {
		Releasep(_action); // clear weak reference
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
		_next_focus = nullptr; // clear weak reference
		UIEvent::release();
	}

	ClickEvent::ClickEvent(View* origin, Vec2 position, Type type, uint32_t count,
			KeyboardKeyCode keycode,
			bool shift, bool ctrl, bool alt, bool command, bool caps_lock)
		: KeyEvent(origin, keycode, 0, shift, ctrl, alt, command, caps_lock
			, 0, 0, 0
		), _position(position), _count(count), _type(type)
	{}

	MouseEvent::MouseEvent(View* origin, Vec2 position, KeyboardKeyCode keycode,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock)
		: KeyEvent(origin, keycode, 0, shift, ctrl, alt, command, caps_lock, 0, 0, 0
		), _position(position), _level(origin->level())
	{}

	Sp<ClickEvent> NewClick(View* view, Vec2 pos,
			ClickEvent::Type type, KeyboardKeyCode keycode = KEYCODE_UNKNOWN, uint32_t count = 1) {
		auto dispatch = view->window()->dispatch();
		auto keyboard = dispatch->keyboard();
		return NewEvent<ClickEvent>(view, pos, type, count, keycode,
			keyboard->shift(),
			keyboard->ctrl(), keyboard->alt(),
			keyboard->command(), keyboard->caps_lock()
		);
	}

	Sp<MouseEvent> NewMouseEvent(View* view, Vec2 pos, KeyboardKeyCode keycode) {
		auto dispatch = view->window()->dispatch();
		auto keyboard = dispatch->keyboard();
		return NewEvent<MouseEvent>(view, pos, keycode,
			keyboard->shift(),
			keyboard->ctrl(), keyboard->alt(),
			keyboard->command(), keyboard->caps_lock()
		);
	}

	UIStateEvent::UIStateEvent(View* origin, UIState state)
		: UIEvent(origin), _state(state)
	{}

	TouchEvent::TouchEvent(View* origin, Array<TouchPoint>& touches)
		: UIEvent(origin), _change_touches(touches)
	{}

	void TouchEvent::release() {
		for (auto& touch : _change_touches)
			touch.view = nullptr; // clear weak reference
		UIEvent::release();
	}

	// O r i g i n T o u c h e
	class EventDispatch::OriginTouche {
	public:
		~OriginTouche() {
			_origin->release(); // it must release here
		}
		View* origin() { return _origin; }
		Dict<uint32_t, TouchPoint>& values() { return _touches; }
		TouchPoint& operator[](uint32_t id) { return _touches[id]; }
		uint32_t count() { return _touches.length(); }
		bool has(uint32_t id) { return _touches.has(id); }
		void del(uint32_t id) { _touches.erase(id); }
		int click_valid_count() const { return _click_valid_count; }
		void set_click_valid_count(int delta) {
			_click_valid_count += delta;
			Qk_ASSERT(_click_valid_count >= 0);
		}
		static OriginTouche* Make(View* view) {
			return view->tryRetain_rt() ? new OriginTouche(view): nullptr;
		}
	private:
		OriginTouche(View* origin)
			: _origin(origin), _click_valid_count(0) {}
		View* _origin;
		Dict<uint32_t, TouchPoint> _touches;
		int _click_valid_count; // click valid count
	};

	// M o u s e H a n d l e r
	class EventDispatch::MouseHandler {
	public:
		MouseHandler(): _view(nullptr), _down_view(nullptr) {}
		~MouseHandler() {
			Releasep(_view);
			Releasep(_down_view);
		}
		View* view() { return _view; }
		View* down_view() { return _down_view; }
		Vec2 position() { return _position; }
		Vec2 down_view_pos() { return _down_v_pos; }
		Vec2 down_pos() { return _down_pos; }
		void set_position(Vec2 value) { _position = value; }
		void set_down_view_mt(View* view) {
			Release(_down_view);
			if (view) {
				view->retain();
				_down_v_pos = view->position();
				_down_pos = _position;
			}
			_down_view = view;
		}
		void set_view_mt(View* view) {
			Release(_view);
			Retain(view);
			_view = view;
		}
	private:
		View *_view, *_down_view;
		Vec2 _position;
		Vec2 _down_v_pos, _down_pos;
	};

	// ----------------------------------------------------------------------------------------
	#define _loop _window->loop()

	EventDispatch::EventDispatch(Window* win)
		: _window(win)
		, _host(win->host())
		, _text_input(nullptr), _focusView(nullptr)
	{
		_keyboard = KeyboardAdapter::create();
		_keyboard->_host = this;
		_mouse = new MouseHandler();
	}

	EventDispatch::~EventDispatch() {
		for (auto& i : _origin_touches)
			delete i.second;
		if ( _focusView ) {
			_focusView->release();
			_focusView = nullptr;
		}
		Release(_keyboard);
		delete _mouse;
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

#if DEBUG
	static int testTouchsCount = 0;
#endif
	void EventDispatch::touchstart_consume(View *view, List<TouchPoint> &in) {
		if ( in.length() && view->_receive ) {
			Array<TouchPoint> touches;

			for (auto i = in.begin(), e = in.end(); i != e;) {
				if (view->overlap_test(i->position)) { // hit test
					TouchPoint& touch = *i;
					touch.start_position = touch.position;
					touch.view = view;
					touch.click_valid = true; // is valid for click

					OriginTouche *out = nullptr;
					if ( !_origin_touches.get(view, out) ) {
						out = OriginTouche::Make(view); // Make origin touch and retain view
						if (out) {
							_origin_touches[view] = out;
						}
					}
					if (out) {
						Qk_ASSERT_EQ(out->has(touch.id), false, "TouchPoint id conflict");
						out->operator[](touch.id) = touch;
						touches.push(touch);
						in.erase(i++);
					} else {
						i++;
					}
				} else {
					i++;
				}
			}

			if ( touches.length() ) { // notice
				auto origin = _origin_touches[view];
				auto highlighted = origin->click_valid_count() == 0; // is need highlight
				origin->set_click_valid_count(touches.length());
				// post main thread
				_loop->post(Cb([view, highlighted, touches](auto e) {
					auto evt = NewEvent<TouchEvent>(view, touches);
					_inl_view(view)->bubble_trigger(UIEvent_TouchStart, **evt); // emit event
					Qk_DEBUGCODE({
						// Qk_DLog("TouchStart %d", testTouchsCount);
						testTouchsCount += touches.length();
						Qk_ASSERT_LE(testTouchsCount, 10, "Too many touch points active");
					});
					if (highlighted) {
						auto evt = NewEvent<UIStateEvent>(view, kActive_UIState);
						_inl_view(view)->trigger_UIStateChange(**evt); // emit event
					}
				}), view);
			}
		}
	}

	void EventDispatch::touchstart(View *view, List<TouchPoint> &in) {
		if ( view->_visible && in.length() ) {
			if ( view->_visible_area ) {
				if ( view->last() && view->is_clip() ) {
					List<TouchPoint> clipIn;

					for ( auto i = in.begin(), e = in.end(); i != e; ) {
						if ( view->overlap_test(i->position) ) {
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
					touchstart_consume(view, clipIn);

					if ( clipIn.length() ) {
						in.splice(in.end(), clipIn);
					}
				} else {
					auto v = view->last();
					while( v && in.length() ) {
						touchstart(v, in);
						v = v->prev();
					}
					touchstart_consume(view, in);
				}
			}
		}
	}

	void EventDispatch::touchmove(List<TouchPoint>& in) {
		Dict<View*, Array<TouchPoint>> change_touches_for_view;

		for (auto& in_touch : in) {
			for ( auto origin : _origin_touches ) {
				if ( origin.second->has(in_touch.id) ) {
					auto& touch = (*origin.second)[in_touch.id];
					touch.position = in_touch.position;
					touch.force = in_touch.force;
					if (touch.click_valid) {
						float d = (touch.start_position - touch.position).lengthSq() *
								_window->scale() / _window->defaultScale();
						if (d > 9) { // 3^2 pt range
							touch.click_valid = false; // set invalid
							origin.second->set_click_valid_count(-1); // decrease count
							if (origin.second->click_valid_count() == 0) {
								_loop->post(Cb([view = touch.view](auto &e){ // emit style status event
									auto evt = NewEvent<UIStateEvent>(view, HOVER_or_NORMAL(view));
									_inl_view(view)->trigger_UIStateChange(**evt);
								}), touch.view);
							}
						}
					}
					change_touches_for_view[touch.view].push(touch);
					break;
				}
			}
		}

		for (auto pair: change_touches_for_view) {
			_loop->post(Cb([pair](auto e) {// emit event
				_inl_view(pair.first)->bubble_trigger(UIEvent_TouchMove,
						**NewEvent<TouchEvent>(pair.first, pair.second));
			}), pair.first);
		} // each end
	}

	void EventDispatch::touchend(List<TouchPoint>& in, bool isCancel) {
		Dict<View*, Array<TouchPoint>> change_touches;
		int count = 0;

		for ( auto& in_touch : in ) {
			for ( auto& origin : _origin_touches ) {
				if ( origin.second->has(in_touch.id) ) {
					TouchPoint& touch = (*origin.second)[in_touch.id];
					touch.position = in_touch.position;
					touch.force = in_touch.force;
					if (touch.click_valid)
						origin.second->set_click_valid_count(-1);
					change_touches[touch.view].push(touch);
					origin.second->del(touch.id); // del touch point
					count++;
					break;
				}
			}
		}
		Qk_ASSERT_EQ(in.length(), count, "Some touch points not found in origin touches");

		for ( auto& i : change_touches ) { // views
			auto& touchs = i.second;
			View* view = touchs[0].view;
			auto origin_touche = _origin_touches[view];
			auto is_end = origin_touche->count() == 0; // End of all touchs
			auto is_click = origin_touche->click_valid_count() == 0; // is click event

			struct Core: CallbackCore<Object> {
				View *view;
				bool is_cancel;
				Array<TouchPoint> touchs;
				OriginTouche *origin_touche;
				bool is_end, is_click;

				Core(View* v, bool isCancel, Array<TouchPoint> &to, OriginTouche* ot,
						bool is_end, bool is_click)
					: view(v), is_cancel(isCancel), touchs(std::move(to))
					, origin_touche(ot), is_end(is_end), is_click(is_click) {}
				~Core() {
					if (is_end)
						delete origin_touche;
				}
				void call(Data& d) {
					auto evt0 = NewEvent<TouchEvent>(view, touchs);
					// emit touch end event
					_inl_view(view)->bubble_trigger(is_cancel ? UIEvent_TouchCancel: UIEvent_TouchEnd, **evt0);
					Qk_DEBUGCODE({
						testTouchsCount -= touchs.length();
						// Qk_DLog("TouchEnd %d", testTouchsCount);
						Qk_ASSERT_GE(testTouchsCount, 0, "Too less touch points active");
					});
					if (is_click) { // trigger click
						for (auto& touch : touchs) {
							if (touch.click_valid) {
								auto evt = NewEvent<UIStateEvent>(view, HOVER_or_NORMAL(view));
								_inl_view(view)->trigger_UIStateChange(**evt); // emit style status event
								if (evt0->is_default() && !is_cancel && view->_level) {
									auto evt = NewClick(view, touch.position, ClickEvent::kTouch);
									_inl_view(view)->trigger_click(**evt); // emit click event
								}
								break;
							}
						}
					} // if (is_click) {
				}
			};

			if (is_end) {
				_origin_touches.erase(view); // del
			}
			_loop->post(Cb(new Core(view, isCancel, touchs, origin_touche, is_end, is_click)), view);
		}
	}

	void EventDispatch::onTouchstart(List<TouchPoint>&& list) {
		auto id = list.front().id;
		auto pos = list.front().position;
		// Qk_DLog("onTouchstart id: %d, x: %f, y: %f, c: %d", id, pos.x(), pos.y(), list.length());
		UILock lock(_window);
		auto r = _window->root();
		if (r) {
			touchstart(r, list);
		}
	}

	void EventDispatch::onTouchmove(List<TouchPoint>&& list) {
		auto id = list.front().id;
		auto loc = list.front().position;
		// Qk_DLog("onTouchmove id: %d, x: %f, y: %f, c: %d", id, loc.x(), loc.y(), list.length());
		UILock lock(_window);
		touchmove(list);
	}

	void EventDispatch::onTouchend(List<TouchPoint>&& list) {
		auto pos = list.front().position;
		// Qk_DLog("onTouchend x: %f, y: %f, c: %d", pos.x(), pos.y(), list.length());
		UILock lock(_window);
		touchend(list, false);
	}

	void EventDispatch::onTouchcancel(List<TouchPoint>&& list) {
		auto pos = list.front().position;
		// Qk_DLog("onTouchcancel x: %f, y: %f", pos.x(), pos.y());
		UILock lock(_window);
		touchend(list, true);
	}

	// -------------------------- M o u s e --------------------------

	View* EventDispatch::find_receive_view_exec(View* view, Vec2 pos) {
		if ( view->visible() && view->visible_area() ) {
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

	View* EventDispatch::find_receive_view_and_retain(Vec2 pos) {
		auto root = _window->root();
		if (root) {
			auto r = find_receive_view_exec(root, pos);
			r = r ? r : root;
			return r->tryRetain_rt();
		} else {
			return nullptr;
		}
	}

	void EventDispatch::mousemove(View *view, Vec2 pos) {
		// always trigger mouse move that is to ensure the continuity of move events, even view first enters
		auto evt = NewMouseEvent(view, pos, KEYCODE_UNKNOWN);
		_inl_view(view)->bubble_trigger(UIEvent_MouseMove, **evt);

		View* v_down = _mouse->down_view();
		if ( v_down && evt->is_default() ) { // test down view valid if not default prevented
			float scale_factor = _window->scale() / _window->defaultScale();
			Vec2 down_pos = _mouse->down_pos();
			Vec2 down_v_pos = _mouse->down_view_pos();
			Vec2 v_pos = v_down->position();
			// 视图位置与光标位置都移动超过3^2平方距离取消点击状态
			if ((down_v_pos - v_pos).lengthSq() * scale_factor > 9 && 
					(down_pos - pos).lengthSq() * scale_factor > 9) {
				// trigger invalid status
				if (view == v_down) {
					_inl_view(v_down)->trigger_UIStateChange( // emit style status event
						**NewEvent<UIStateEvent>(v_down, kHover_UIState));
				}
				v_down = nullptr;
				_mouse->set_down_view_mt(nullptr);
			}
		}

		View* old = _mouse->view();
		if (old != view) {
			_mouse->set_view_mt(view);
			if (old) {
				// Can trigger the MouseOut event here.
				if (!old->is_child(view)) {
					auto evt = NewMouseEvent(old, pos, KEYCODE_UNKNOWN);
					_inl_view(old)->bubble_trigger(UIEvent_MouseLeave, **evt);
				}
				_inl_view(old)->trigger_UIStateChange( // emit style status event
					**NewEvent<UIStateEvent>(old, kNormal_UIState)
				);
			}
			// Can trigger the MouseOver event here.
			_window->setCursorStyle(view->cursor_style_exec(), true);

			if (!old || !view->is_child(old)) {
				auto evt = NewMouseEvent(view, pos, KEYCODE_UNKNOWN);
				_inl_view(view)->bubble_trigger(UIEvent_MouseEnter, **evt);
			}
			auto status = view == v_down || view->is_child(v_down) ?
				kActive_UIState: kHover_UIState;
			_inl_view(view)->trigger_UIStateChange(**NewEvent<UIStateEvent>(view, status));
		}
	}

	void EventDispatch::mousepress(View *view, Vec2 pos, KeyboardKeyCode code, bool down) {
		if (_mouse->view() != view) {
			mousemove(view, pos); // ensure mouse move to this view first
		}
		Sp<MouseEvent> evt = NewMouseEvent(view, pos, code);
		Sp<View> v_down = _mouse->down_view();

		if (down) {
			_mouse->set_down_view_mt(view);
			_inl_view(view)->bubble_trigger(UIEvent_MouseDown, **evt);
			_window->setCursorStyle(view->cursor_style_exec(), true);
		} else {
			_mouse->set_down_view_mt(nullptr);
			_inl_view(view)->bubble_trigger(UIEvent_MouseUp, **evt);
		}

		if (code != KEYCODE_MOUSE_LEFT || !evt->is_default())
			return;

		if (down) {
			_inl_view(view)->trigger_UIStateChange(
				**NewEvent<UIStateEvent>(view, kActive_UIState)); // emit style status event
		} else {
			_inl_view(view)->trigger_UIStateChange(
				**NewEvent<UIStateEvent>(view, kHover_UIState)); // emit style status event

			if (view == *v_down || view->is_child(*v_down)) {
				_inl_view(view)->trigger_click(**NewClick(view, pos, ClickEvent::kMouse, code));
			}
		}
	}

	void EventDispatch::onMousemove(float x, float y) {
		UILock lock(_window);
		Vec2 pos(x, y);
		// Qk_DLog("onMousemove x: %f, y: %f", x, y);
		_mouse->set_position(pos); // set current mouse pos
		auto v = find_receive_view_and_retain(pos);
		if (v) {
			_loop->post(Cb([this,v,pos](auto e) {
				// static int64_t lastTime = 0;
				// int64_t time = time_monotonic();
				// int64_t diff = time - lastTime;
				// lastTime = time;
				// Qk_DLog("mouse move delay: %lld ms", diff / 1000);
				mousemove(v, pos);
				v->release(); // it has to release
			}),v);
		}
	}

	void EventDispatch::onMousepress(KeyboardKeyCode code, bool isDown, const Vec2 *val) {
		Vec2 deltaDefault;
		switch(code) {
			case KEYCODE_MOUSE_LEFT:
			case KEYCODE_MOUSE_CENTER:
			case KEYCODE_MOUSE_RIGHT: {
				UILock lock(_window); // Lock ui render
				auto pos = val ? *val: _mouse->position(); // get current mouse pos
				auto v = find_receive_view_and_retain(pos);
				//Qk_DLog("onMousepress code: %d, isDown: %i, v: %p", code, isDown, v);
				if (v)
					_loop->post(Cb([this,v,pos,code,isDown](auto e) {
						mousepress(v, pos, code, isDown);
						v->release(); // it has to release
					}),v);
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
					_loop->post(Cb([=](auto e) {
						auto v = _mouse->view();
						if (v)
							_inl_view(v)->bubble_trigger(UIEvent_MouseWheel, **NewMouseEvent(v, delta, code));
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
		if (!view) return;

		auto code = _keyboard->keycode();
		auto btn = view->asButton();
		View *next_focus = nullptr;

		if (btn) {
			Direction dir;
			switch ( code ) {
				case KEYCODE_LEFT: dir = Direction::Left; break;  // left
				case KEYCODE_UP: dir = Direction::Top; break;     // top
				case KEYCODE_RIGHT: dir = Direction::Right; break; // right
				case KEYCODE_DOWN: dir = Direction::Bottom; break; // bottom
				default: dir = Direction::None; break;
			}
			if ( dir != Direction::None ) {
				auto view = btn->next_button(dir);
				if (view)
					next_focus = view->tryRetain_rt(); // safe retain view
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
						auto evt = NewEvent<UIStateEvent>(view, kActive_UIState);
						_inl_view(view)->trigger_UIStateChange(**evt); // emit click status event
					}

					if ( evt->next_focus() ) {
						evt->next_focus()->focus();
					}
				} // if ( evt->is_default() ) {
			}
		};

		_loop->post(Cb(new Core(this, view, next_focus, evt)), view); // async_resolve(
	}

	void EventDispatch::onKeyboardUp() {
		auto safe_v = safe_focus_view();
		View* view = *safe_v;
		if (!view) return;

		auto mat = view->morph_view()->matrix();
		auto center = view->client_size() * 0.5f;
		auto morph = view->asMorphView();
		if (morph)
			center -= morph->origin_value();
		auto point = mat.mul_vec2_no_translate(center) + view->position();

		_loop->post(Cb([this,view,point](auto& e) {
			Sp<KeyEvent> evt(NewKeyEvent(view, _keyboard));

			_inl_view(view)->bubble_trigger(UIEvent_KeyUp, **evt);

			if ( evt->is_default() ) {
				if ( evt->keycode() == KEYCODE_BACK ) {
					auto evt = NewClick(view, point, ClickEvent::kKeyboard, KEYCODE_BACK);
					_inl_view(view)->bubble_trigger(UIEvent_Back, **evt); // emit back

					if ( evt->is_default() ) {
						_window->pending();
					}
				}
				else if ( evt->keycode() == KEYCODE_CENTER ) {
					auto evt = NewEvent<UIStateEvent>(view, kHover_UIState);
					_inl_view(view)->trigger_UIStateChange(**evt); // emit style status event

					auto evt2 = NewClick(view, point, ClickEvent::kKeyboard, KEYCODE_CENTER);
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
