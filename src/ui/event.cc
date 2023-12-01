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
#include "./view.h"
#include "./layout/root.h"
#include "./layout/button.h"
#include "./keyboard.h"

namespace qk {

	static inline HighlightedStatus HOVER_or_NORMAL(View* view) {
		return view->is_focus() ? HIGHLIGHTED_HOVER : HIGHLIGHTED_NORMAL;
	}

	template<class T, typename... Args>
	inline static Handle<T> NewEvent(Args... args) { return new T(args...); }

 // -------------------------- V i e w --------------------------

	Qk_DEFINE_INLINE_MEMBERS(View, InlEvent) {
	public:
		#define _inl_view(self) static_cast<View::InlEvent*>(static_cast<View*>(self))

		void trigger_highlightted(HighlightedEvent& evt) {
			View* view = this;
			if ( view ) {
				if ( view->_layout->receive() ) {
					view->trigger(UIEvent_Highlighted, evt);
					if ( evt.is_default() ) {
						// auto classs = view->classs();
						// if ( classs ) { // 切换样式表状态
						// 	classs->set_style_pseudo_status( CSSPseudoClass(evt.status()) );
						// }
					}
				}
			}
		}

		void trigger_click(UIEvent& evt) {
			View* view = this;
			do {
				if ( view->_layout->receive() ) {
					view->trigger(UIEvent_Click, evt);
					if ( !evt.is_bubble() ) {
						break; // Stop bubble
					}
				}
				if (view->parent()) { // root
					view = view->parent();
				} else {
					if (evt.is_default()) {
						auto win = _layout->window();
						if (evt.origin() != win->dispatch()->focus())
							view->focus(); // root
					}
					break;
				}
			} while(true);
		}

		void bubble_trigger(const NameType& name, UIEvent& evt) {
			View* view = this;
			while( view ) {
				if ( view->_layout->receive() ) {
					view->trigger(name, evt);
					if ( !evt.is_bubble() ) {
						break; // Stop bubble
					}
				}
				view = view->parent();
			}
		}

		void trigger(const NameType& name, UIEvent& evt) {
			if ( _layout->receive() ) {
				Notification::trigger(name, evt);
			}
		}
	};

	bool View::focus() {
		if ( is_focus() ) return true;

		auto dispatch = _layout->window()->dispatch();
		View* old = dispatch->focus();

		if ( !dispatch->set_focus(this) ) {
			return false;
		}

		if ( old ) {
			_inl_view(old)->bubble_trigger(UIEvent_Blur, **NewEvent<UIEvent>(old));
			_inl_view(old)->trigger_highlightted(
				**NewEvent<HighlightedEvent>(old, HIGHLIGHTED_NORMAL)
			);
		}
		_inl_view(this)->bubble_trigger(UIEvent_Focus, **NewEvent<UIEvent>(this));
		_inl_view(this)->trigger_highlightted(
				**NewEvent<HighlightedEvent>(this, HIGHLIGHTED_HOVER)
		);
		return true;
	}

	// -------------------------- E v e n t --------------------------

#define Qk_FUN(NAME, C, FLAG) \
	const UIEventName UIEvent_##NAME(#NAME, UI_EVENT_CATEGORY_##C, FLAG);
	Qk_UI_Events(Qk_FUN)
#undef Qk_FUN

	const Dict<String, UIEventName> UIEventNames([]() -> Dict<String, UIEventName> {
		Dict<String, UIEventName> r;
#define Qk_FUN(NAME, C, F) r.set(UIEvent_##NAME.toString(), UIEvent_##NAME);
		Qk_UI_Events(Qk_FUN)
#undef Qk_FUN
		return r;
	}());

	UIEventName::UIEventName(cString& name, uint32_t category, uint32_t flag)
		: _toString(name), _hashCode((uint32_t)name.hashCode()), _category(category), _flag(flag)
	{}

	UIEvent::UIEvent(View* origin)
		: Event(SendData(), origin), _timestamp(time_micro()) {
		return_value = RETURN_VALUE_MASK_ALL;
	}

	ActionEvent::ActionEvent(Action* action, View* origin, uint64_t delay, uint32_t frame, uint32_t loop)
		: UIEvent(origin), _action(action), _delay(delay), _frame(frame), _loop(loop)
	{}

	void ActionEvent::release() {
		_action = nullptr;
		UIEvent::release();
	}

	KeyEvent::KeyEvent(View* origin, uint32_t keycode,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
										uint32_t repeat, int device, int source)
		: UIEvent(origin), _keycode(keycode)
		, _device(device), _source(source), _repeat(repeat), _shift(shift)
		, _ctrl(ctrl), _alt(alt), _command(command), _caps_lock(caps_lock), _focus_move(nullptr)
	{}

	void KeyEvent::set_focus_move(View* view) {
		if (origin())
			_focus_move = view;
	}

	void KeyEvent::set_keycode(uint32_t keycode) {
		_keycode = keycode;
	}

	void KeyEvent::release() {
		_focus_move = nullptr;
		UIEvent::release();
	}

	ClickEvent::ClickEvent(View* origin, float x, float y, Type type, uint32_t count)
		: UIEvent(origin), _x(x), _y(y), _count(count), _type(type)
	{}

	MouseEvent::MouseEvent(View* origin, float x, float y, uint32_t keycode,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
										uint32_t repeat, int device, int source)
		: KeyEvent(origin, keycode, shift, ctrl, alt, command, caps_lock, repeat, device, source), _x(x), _y(y)
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
		OriginTouche() { Qk_UNREACHABLE(); }
		OriginTouche(View* view)
			: _view(view)
			, _start_position(view_position(view))
			, _is_click_invalid(false), _is_click_down(false)
		{
			_view->retain();
		}
		~OriginTouche() {
			_view->release();
		}
		static Vec2 view_position(View* view) {
			return Vec2(view->_layout->matrix()[2], view->_layout->matrix()[5]);
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
	private:
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
		~MouseHandle() { Release(_view); }
		inline View* view() { return _view; }
		inline Vec2 view_start_position() { return _start_position; }
		inline Vec2 position() { return _position; }
		inline View* click_down_view() { return _click_view; }
		inline void set_position(Vec2 value) { _position = value; }
		void set_click_down_view(View* view) {
			Release(_click_view);
			if (view) {
				view->retain();
				_start_position = OriginTouche::view_position(view);
			}
			_click_view = view;
		}
		void set_view(View* view) {
			Release(_view);
			Retain(view);
			_view = view;
		}
	private:
		View* _view, *_click_view;
		Vec2 _start_position, _position;
	};

	EventDispatch::EventDispatch(Window* win)
		: _window(win)
		, _host(win->host())
		, _text_input(nullptr), _focus(nullptr) 
	{
		_keyboard = KeyboardAdapter::create();
		_keyboard->_host = this;
		_mouse_h = new MouseHandle();
	}

	EventDispatch::~EventDispatch() {
		for (auto& i : _origin_touches)
			delete i.value;
		if ( _focus ) {
			_focus->release();
			_focus = nullptr;
		}
		Release(_keyboard);
		delete _mouse_h;
	}

	bool EventDispatch::set_focus(View* view) {
		if ( _focus != view ) {
			if ( view->_layout->_level && view->_layout->can_become_focus() ) {
				if ( _focus ) {
					_focus->release(); // unref
				}
				_focus = view;
				_focus->retain(); // strong ref
				set_text_input(view->_layout->as_text_input());
			} else {
				return false;
			}
		}
		return true;
	}

	#define _loop static_cast<PostMessage*>(_host->loop())

	typedef Callback<List<TouchPoint>> TouchCb;

	// -------------------------- T o u c h --------------------------

	void EventDispatch::touchstart_erase(View* view, List<TouchPoint>& in) {
		if ( /*view->receive() &&*/ in.length() ) {
			Array<TouchPoint> change_touches;
			
			for ( auto i = in.begin(), e = in.end(); i != e; ) {

				if ( view->_layout->overlap_test(Vec2(i->x, i->y)) ) {
					TouchPoint& touch = *i;
					touch.start_x = touch.x;
					touch.start_y = touch.y;
					touch.click_in = true;
					touch.view = view;
					
					if ( !_origin_touches.has(view) ) {
						_origin_touches[view] = new OriginTouche(view);
					}
					(*_origin_touches[view])[touch.id] = touch;
					
					change_touches.push( touch );
					in.erase(i++);
				} else {
					i++;
				}
			}
			
			if ( change_touches.length() ) { // notice
				auto evt = NewEvent<TouchEvent>(view, change_touches);
				_inl_view(view)->bubble_trigger(UIEvent_TouchStart, **evt); // emit event
				
				if ( !_origin_touches[view]->is_click_down() ) { // trigger click down
					_origin_touches[view]->set_is_click_down(true);
					auto evt = NewEvent<HighlightedEvent>(view, HIGHLIGHTED_DOWN);
					_inl_view(view)->trigger_highlightted(**evt); // emit event
				}
			}
		}
	}

	void EventDispatch::touchstart(View* view, List<TouchPoint>& in) {
		
		if ( view->visible() && in.length() ) {
			if ( view->_layout->visible_region() ) {
				
				if ( view->last() && view->_layout->clip() ) {
					List<TouchPoint> in2;

					for ( auto i = in.begin(), e = in.end(); i != e; ) {
						if ( view->_layout->overlap_test(Vec2(i->x, i->y)) ) {
							in2.pushBack(*i);
							in.erase(i++);
						} else {
							i++;
						}
					}
					
					View* v = view->last();
					while( v && in2.length() ) {
						touchstart(v, in2);
						v = v->prev();
					}
					
					touchstart_erase(view, in2);
					
					if ( in2.length() ) {
						in.splice(in.end(), in2);
					}
				} else {
					View* v = view->last();
					while( v && in.length() ) {
						touchstart(v, in);
						v = v->prev();
					}
					touchstart_erase(view, in);
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
						touch.click_in = touch.view->_layout->overlap_test(Vec2(touch.x, touch.y));
					}
					change_touches[touch.view].push(touch);
					break;
				}
			}
		}
		
		for ( auto i : change_touches ) {
			
			Array<TouchPoint>& touchs = i.value;
			View* view = touchs[0].view;
			// emit event
			_inl_view(view)->bubble_trigger(
				UIEvent_TouchMove,
				**NewEvent<TouchEvent>(view, i.value)
			);
			
			OriginTouche* origin_touche = _origin_touches[view];
			
			if ( !origin_touche->is_click_invalid() ) { // no invalid
				Vec2 position = OriginTouche::view_position(view);
				Vec2 start_position = origin_touche->view_start_position();
				
				float d = sqrtf(powf((position.x() - start_position.x()), 2) +
												powf((position.y() - start_position.y()), 2));
				// 视图位置移动超过2取消点击状态
				if ( d > 2 ) { // trigger invalid status
					if ( origin_touche->is_click_down() ) { // trigger style up
						// emit style status event
						auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
						_inl_view(view)->trigger_highlightted(**evt);
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
							// emit style status event
							auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
							_inl_view(view)->trigger_highlightted(**evt);
						}
					} else { // May trigger click down
						for ( int i = 0; i < touchs.length(); i++) {
							auto item = touchs[i];
							if ( item.click_in ) { // find range == true
								origin_touche->set_is_click_down(true); // set down status
								// emit style down event
								auto evt = NewEvent<HighlightedEvent>(view, HIGHLIGHTED_DOWN);
								_inl_view(view)->trigger_highlightted(**evt);
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
			Array<TouchPoint>& touchs = i.value;
			View* view = touchs[0].view;
			_inl_view(view)->bubble_trigger(type, **NewEvent<TouchEvent>(view, touchs)); // emit touch end event
			
			OriginTouche* origin_touche = _origin_touches[view];
			
			if ( origin_touche->count() == 0 ) {
				if ( origin_touche->is_click_down() ) { // trigger click
					for ( auto& item : touchs ) {
						// find range == true
						if ( item.click_in ) {
							// emit style up event
							auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
							_inl_view(view)->trigger_highlightted(**evt);
							
							if ( type == UIEvent_TouchEnd && view->_layout->_level ) {
								auto evt = NewEvent<ClickEvent>(view, item.x, item.y, ClickEvent::TOUCH);
								_inl_view(view)->trigger_click(**evt); // emit click event
							}
							break;
						}
					}
				}
				delete origin_touche;
				_origin_touches.erase(view); // del
			}
			//
		}
	}

	void EventDispatch::onTouchstart(List<TouchPoint>&& list) {
		Qk_DEBUG("onTouchstart x: %f, y: %f", list.front().y, list.front().y);
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock(_window);
			auto r = _window->root();
			if (r) {
				touchstart(r, *evt.data);
			}
		}), std::move(list), _loop);
	}

	void EventDispatch::onTouchmove(List<TouchPoint>&& list) {
		Qk_DEBUG("onTouchmove x: %f, y: %f", list.front().y, list.front().y);
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock(_window);
			touchmove(*evt.data);
		}), std::move(list), _loop);
	}

	void EventDispatch::onTouchend(List<TouchPoint>&& list) {
		Qk_DEBUG("onTouchend x: %f, y: %f", list.front().y, list.front().y);
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock(_window);
			touchend(*evt.data, UIEvent_TouchEnd);
		}), std::move(list), _loop);
	}

	void EventDispatch::onTouchcancel(List<TouchPoint>&& list) {
		Qk_DEBUG("onTouchcancel x: %f, y: %f", list.front().y, list.front().y);
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock(_window);
			touchend(*evt.data, UIEvent_TouchCancel);
		}), std::move(list), _loop);
	}

// -------------------------- M o u s e --------------------------

	View* EventDispatch::find_receive_view(Vec2 pos) {
		return _window->root() ? find_receive_view_rec(_window->root(), pos) : nullptr;
	}

	View* EventDispatch::find_receive_view_rec(View* view, Vec2 pos) {
		if ( view->visible() ) {
			if ( view->_layout->visible_region() ) {
				View* v = view->last();

				if (v && view->_layout->clip() ) {
					if (view->_layout->overlap_test(pos)) {
						while (v) {
							auto r = find_receive_view_rec(v, pos);
							if (r) {
								return r;
							}
							v = v->prev();
						}
						if (view->_layout->receive()) {
							return view;
						}
					}
				} else {
					while (v) {
						auto r = find_receive_view_rec(v, pos);
						if (r) {
							return r;
						}
						v = v->prev();
					}
					if (view->_layout->receive() && view->_layout->overlap_test(pos)) {
						return view;
					}
				}
			}
		}
		return nullptr;
	}

	Sp<MouseEvent> EventDispatch::NewMouseEvent(View* view, float x, float y, uint32_t keycode) {
		return NewEvent<MouseEvent>(view, x, y, keycode,
			_keyboard->shift(),
			_keyboard->ctrl(), _keyboard->alt(),
			_keyboard->command(), _keyboard->caps_lock(), 0, 0, 0
		);
	}

	void EventDispatch::mousemove(View* view, Vec2 pos) {
		View* d_view = _mouse_h->click_down_view();

		if ( d_view ) { // no invalid
			Vec2 position = OriginTouche::view_position(d_view);
			Vec2 start_position = _mouse_h->view_start_position();
			float d = sqrtf(powf((position.x() - start_position.x()), 2) +
											powf((position.y() - start_position.y()), 2));
			// 视图位置移动超过2取消点击状态
			if ( d > 2 ) { // trigger invalid status
				if (view == d_view) {
					_inl_view(view)->trigger_highlightted( // emit style status event
						**NewEvent<HighlightedEvent>(view, HIGHLIGHTED_HOVER));
				}
				_mouse_h->set_click_down_view(nullptr);
			}
		}

		float x = pos[0], y = pos[1];

		View* old = _mouse_h->view();

		if (old != view) {
			_mouse_h->set_view(view);

			if (old) {
				auto evt = NewMouseEvent(old, x, y);
				_inl_view(old)->bubble_trigger(UIEvent_MouseOut, **evt);

				if (evt->is_default()) {
					evt->return_value = RETURN_VALUE_MASK_ALL;

					if (!view || !old->is_self_child(view)) {
						_inl_view(old)->bubble_trigger(UIEvent_MouseLeave, **evt);
					}

					_inl_view(old)->trigger_highlightted( // emit style status event
						**NewEvent<HighlightedEvent>(old, HIGHLIGHTED_NORMAL));
				}
			}
			if (view) {
				auto evt = NewMouseEvent(view, x, y);
				_inl_view(view)->bubble_trigger(UIEvent_MouseOver, **evt);

				if (evt->is_default()) {
					evt->return_value = RETURN_VALUE_MASK_ALL;
					
					if (!old || !view->is_self_child(old)) {
						_inl_view(view)->bubble_trigger(UIEvent_MouseEnter, **evt);
					}

					_inl_view(view)->trigger_highlightted( // emit style status event
						**NewEvent<HighlightedEvent>(view,
							view == d_view ? HIGHLIGHTED_DOWN: HIGHLIGHTED_HOVER)
					);
				}
			}
		}
		else if (view) {
			_inl_view(view)->bubble_trigger(UIEvent_MouseMove, **NewMouseEvent(view, x, y));
		}
	}

	void EventDispatch::mousepress(KeyboardKeyName name, bool down, Vec2 pos) {
		float x = pos[0], y = pos[1];
		Handle<View> view(find_receive_view(pos));

		if (_mouse_h->view() != *view) {
			mousemove(*view, pos);
		}

		if (!view) return;

		auto evt = NewMouseEvent(*view, x, y, name);

		Handle<View> raw_down_view = _mouse_h->click_down_view();

		if (down) {
			_mouse_h->set_click_down_view(*view);
			_inl_view(*view)->bubble_trigger(UIEvent_MouseDown, **evt);
		} else {
			_mouse_h->set_click_down_view(nullptr);
			_inl_view(*view)->bubble_trigger(UIEvent_MouseUp, **evt);
		}

		if (name != KEYCODE_MOUSE_LEFT || !evt->is_default()) return;

		if (down) {
			_inl_view(*view)->trigger_highlightted(
				**NewEvent<HighlightedEvent>(*view, HIGHLIGHTED_DOWN)); // emit style status event
		} else {
			_inl_view(*view)->trigger_highlightted(
				**NewEvent<HighlightedEvent>(*view, HIGHLIGHTED_HOVER)); // emit style status event

			if (*view == *raw_down_view) {
				_inl_view(*view)->trigger_click(**NewEvent<ClickEvent>(*view, x, y, ClickEvent::MOUSE));
			}
		}
	}

	void EventDispatch::mousewhell(KeyboardKeyName name, bool down, float x, float y) {
		if (down) {
			auto view = _mouse_h->view();
			if (view) {
				_inl_view(view)->bubble_trigger(UIEvent_MouseWheel, **NewMouseEvent(view, x, y, name));
			}
		}
	}

	void EventDispatch::onMousemove(float x, float y) {
		async_resolve(Cb([=](Cb::Data& evt) {
			UILock lock(_window);
			Vec2 pos(x, y);
			// set current mouse pos
			_mouse_h->set_position(pos);

			if (_window->root()) {
				Handle<View> v(find_receive_view(pos));
				mousemove(*v, pos);
			}
		}), _loop);
	}

	void EventDispatch::onMousepress(KeyboardKeyName name, bool down) {
		async_resolve(Cb([=](Cb::Data& evt) {
			UILock lock(_window);
			switch(name) {
				case KEYCODE_MOUSE_LEFT:
				case KEYCODE_MOUSE_CENTER:
				case KEYCODE_MOUSE_RIGHT:
					mousepress(name, down, _mouse_h->position());
					break;
				case KEYCODE_MOUSE_WHEEL_UP:
					mousewhell(name, down, 0, -53); break;
				case KEYCODE_MOUSE_WHEEL_DOWN:
					mousewhell(name, down, 0, 53); break;
				case KEYCODE_MOUSE_WHEEL_LEFT:
					mousewhell(name, down, -53, 0); break;
				case KEYCODE_MOUSE_WHEEL_RIGHT:
					mousewhell(name, down, 53, 0); break;
				default: break;
			}
		}), _loop);
	}

	// -------------------------- k e y b o a r d --------------------------

	void EventDispatch::onKeyboard_down() {

		auto view = _focus;
		if ( !view )
			view = _window->root();

		if ( view ) {
			auto name = _keyboard->keyname();
			auto btn = view->_layout->as_button();
			Layout *focus_move = nullptr;

			if (btn) {
				FindDirection dir;
				switch ( name ) {
					case KEYCODE_LEFT: dir = FindDirection::kLeft; break;  // left
					case KEYCODE_UP: dir = FindDirection::kTop; break;     // top
					case KEYCODE_RIGHT: dir = FindDirection::kRight; break; // right
					case KEYCODE_DOWN: dir = FindDirection::kBottom; break; // bottom
					default: dir = FindDirection::kNone; break;
				}
				if ( dir != FindDirection::kNone ) {
					focus_move = btn->next_button(dir);
				}
			}

			auto evt = NewEvent<KeyEvent>(view, name,
				_keyboard->shift(),
				_keyboard->ctrl(), _keyboard->alt(),
				_keyboard->command(), _keyboard->caps_lock(),
				_keyboard->repeat(), _keyboard->device(), _keyboard->source()
			);

			evt->set_focus_move(focus_move->_view);

			_inl_view(view)->bubble_trigger(UIEvent_KeyDown, **evt);
			
			if ( evt->is_default() ) {
				
				if ( name == KEYCODE_ENTER ) {
					_inl_view(view)->bubble_trigger(UIEvent_KeyEnter, **evt);
				} else if ( name == KEYCODE_VOLUME_UP ) {
					set_volume_up();
				} else if ( name == KEYCODE_VOLUME_DOWN ) {
					set_volume_down();
				}
				
				int keypress_code = _keyboard->keypress();
				if ( keypress_code ) { // keypress
					evt->set_keycode( keypress_code );
					_inl_view(view)->bubble_trigger(UIEvent_KeyPress, **evt);
				}

				if ( name == KEYCODE_CENTER && _keyboard->repeat() == 0 ) {
					auto evt = NewEvent<HighlightedEvent>(view, HIGHLIGHTED_DOWN);
					_inl_view(view)->trigger_highlightted(**evt); // emit click status event
				}
				
				if ( evt->focus_move() ) {
					evt->focus_move()->focus();
				}
			} // if ( evt->is_default() ) {
		} // if ( view )
	}
	
	void EventDispatch::onKeyboard_up() {

		View* view = _focus;
		if ( !view )
			view = _window->root();

		if ( view ) {
			auto name = _keyboard->keyname();
			auto evt = NewEvent<KeyEvent>(view, name,
				_keyboard->shift(),
				_keyboard->ctrl(), _keyboard->alt(),
				_keyboard->command(), _keyboard->caps_lock(),
				_keyboard->repeat(), _keyboard->device(), _keyboard->source()
			);

			_inl_view(view)->bubble_trigger(UIEvent_KeyUp, **evt);
			
			if ( evt->is_default() ) {
				if ( name == KEYCODE_BACK ) {
					auto point = view->_layout->position();
					auto evt = NewEvent<ClickEvent>(view, point.x(), point.y(), ClickEvent::KEYBOARD);
					_inl_view(view)->bubble_trigger(UIEvent_Back, **evt); // emit back
					
					if ( evt->is_default() ) {
						_window->pending();
					}
				}
				else if ( name == KEYCODE_CENTER ) {
					auto evt = NewEvent<HighlightedEvent>(view, HIGHLIGHTED_HOVER);
					_inl_view(view)->trigger_highlightted(**evt); // emit style status event
					
					auto point = view->_layout->position();
					auto evt2 = NewEvent<ClickEvent>(view, point.x(), point.y(), ClickEvent::KEYBOARD);
					_inl_view(view)->trigger_click(**evt2);
				} //
			}
		}
	}

	// -------------------------- I M E --------------------------

	void EventDispatch::onImeDelete(int count) {
		async_resolve(Cb([=](Cb::Data& d) {
			UILock lock(_window);
			if ( _text_input ) {
				_text_input->input_delete(count);
				bool can_backspace = _text_input->input_can_backspace();
				bool can_delete = _text_input->input_can_delete();
				set_ime_keyboard_can_backspace(can_backspace, can_delete);
			}
		}), _loop);
	}

	void EventDispatch::onImeInsert(cString& text) {
		async_resolve(Cb([=](Cb::Data& d) {
			UILock lock(_window);
			if ( _text_input ) {
				_text_input->input_insert(text);
			}
		}), _loop);
	}

	void EventDispatch::onImeMarked(cString& text) {
		async_resolve(Cb([=](Cb::Data& d) {
			UILock lock(_window);
			if ( _text_input ) {
				_text_input->input_marked(text);
			}
		}), _loop);
	}

	void EventDispatch::onImeUnmark(cString& text) {
		async_resolve(Cb([=](Cb::Data& d) {
			UILock lock(_window);
			if ( _text_input ) {
				_text_input->input_unmark(text);
			}
		}), _loop);
	}

	void EventDispatch::onImeControl(KeyboardKeyName name) {
		async_resolve(Cb([=](Cb::Data& d) {
			UILock lock(_window);
			if ( _text_input ) {
				_text_input->input_control(name);
			}
		}), _loop);
	}

	void EventDispatch::set_text_input(TextInput* input) {
		Qk_DEBUG("set_text_input");
		if ( input != _text_input ) {
			_text_input = input;

			if ( input ) {
				set_ime_keyboard_open({
					true,
					input->input_keyboard_type(),
					input->input_keyboard_return_type(),
					input->input_spot_location(),
				});
			} else {
				set_ime_keyboard_close();
			}
		} else {
			if ( input ) {
				set_ime_keyboard_open({
					false,
					input->input_keyboard_type(),
					input->input_keyboard_return_type(),
					input->input_spot_location(),
				});
			}
		}
	}

}
