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
#include "./view/entity.h"

namespace qk {

	static inline HighlightedEvent::Status HOVER_or_NORMAL(View *view) {
		return view->is_focus() ? HighlightedEvent::kHover : HighlightedEvent::kNormal;
	}

	template<class T, typename... Args>
	inline static Handle<T> NewEvent(Args... args) { return new T(args...); }

	#define _Fun(Name, C, Flag) \
	cUIEventName UIEvent_##Name(#Name, k##C##_UIEventCategory, Flag);
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
			View *view = this;
			do {
				if ( view->_receive ) {
					view->trigger(name, evt);
					if ( !evt.is_bubble() )
						break; // Stop bubble
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
				preRender().async_call([](auto self, auto arg) {
					do {
						auto ss = self->_cssclass.load();
						if (ss)
							ss->setStatus_rt(arg.arg);
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
		: _string(name), _hashCode(name.hashCode()), _category(category), _flag(flag)
	{}

	UIEvent::UIEvent(View *origin, SendData data)
		: Event(data, kAll_ReturnValueMask), _origin(origin), _timestamp(time_micro() / 1000) {
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

	ClickEvent::ClickEvent(View* origin, Vec2 position, Type type, uint32_t count)
		: UIEvent(origin), _position(position), _count(count), _type(type)
	{}

	MouseEvent::MouseEvent(View* origin, Vec2 position, KeyboardKeyCode keycode, int keypress,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
										uint32_t repeat, int device, int source)
		: KeyEvent(origin, keycode, keypress, shift, ctrl, alt, command, caps_lock
			, repeat, device, source
		), _position(position)
	{}

	HighlightedEvent::HighlightedEvent(View* origin, HighlightedStatus status)
		: UIEvent(origin), _status(status)
	{}

	TouchEvent::TouchEvent(View* origin, Array<TouchPoint>& touches)
		: UIEvent(origin), _change_touches(touches)
	{}

	void TouchEvent::release() {
		for (auto& touch : _change_touches)
			touch.view = nullptr; // clear weak reference
		UIEvent::release();
	}

	class EventDispatch::OriginTouche {
	public:
		~OriginTouche() {
			_origin->release(); // it must release here
		}
		View* origin() { return _origin; }
		// bool is_click_valid() { return _is_click_valid; }
		// void set_click_invalid() { _is_click_valid = false; }
		Dict<uint32_t, TouchPoint>& values() { return _touches; }
		TouchPoint& operator[](uint32_t id) { return _touches[id]; }
		uint32_t count() { return _touches.length(); }
		bool has(uint32_t id) { return _touches.has(id); }
		void del(uint32_t id) { _touches.erase(id); }
		int click_valid_count() const { return _click_valid_count; }
		void set_click_valid_count(int delta) { _click_valid_count += delta;
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
		void set_click_down_view_Wt(View* view) {
			Release(_click_view);
			if (view) {
				view->retain();
				_start_position = view->position();
			}
			_click_view = view;
		}
		void set_view_Wt(View* view) {
			Release(_view);
			Retain(view);
			_view = view;
		}
	private:
		View *_view, *_click_view;
		Vec2 _start_position, _position;
	};

	// @EventDispatch
	// ----------------------------------------------------------------------------------------
	#define _loop _window->loop()

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
			delete i.second;
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
						(*out)[touch.id] = touch;
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
				auto origin = _origin_touches[view];
				auto highlighted = origin->click_valid_count() == 0; // is need highlight
				origin->set_click_valid_count(change_touches.length());
				// post main thread
				_loop->post(Cb([view, highlighted, change_touches](auto& e) {
					auto evt = NewEvent<TouchEvent>(view, change_touches);
					_inl_view(view)->bubble_trigger(UIEvent_TouchStart, **evt); // emit event
					if (highlighted) {
						auto evt = NewEvent<HighlightedEvent>(view, HighlightedStatus::kActive);
						_inl_view(view)->trigger_highlightted(**evt); // emit event
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
						if (d > 4) { // 2^2 pt range
							touch.click_valid = false; // set invalid
							origin.second->set_click_valid_count(-1); // decrease count
							if (origin.second->click_valid_count() == 0) {
								_loop->post(Cb([view = touch.view](auto &e){ // emit style status event
									auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
									_inl_view(view)->trigger_highlightted(**evt);
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
					break;
				}
			}
		}

		for ( auto& i : change_touches ) { // views
			auto& touchs = i.second;
			View* view = touchs[0].view;
			auto origin_touche = _origin_touches[view];
			auto is_end = origin_touche->count() == 0;
			auto is_click = origin_touche->click_valid_count() == 0;

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

					if (is_click) { // trigger click
						for (auto& touch : touchs) {
							if (touch.click_valid) {
								auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
								_inl_view(view)->trigger_highlightted(**evt); // emit style status event
								if (evt0->is_default() && !is_cancel && view->_level) {
									auto evt = NewEvent<ClickEvent>(view, touch.position, ClickEvent::kTouch);
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
		Qk_DLog("onTouchstart id: %d, x: %f, y: %f, c: %d", id, pos.x(), pos.y(), list.length());
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
		Qk_DLog("onTouchend x: %f, y: %f, c: %d", pos.x(), pos.y(), list.length());
		UILock lock(_window);
		touchend(list, false);
	}

	void EventDispatch::onTouchcancel(List<TouchPoint>&& list) {
		auto pos = list.front().position;
		Qk_DLog("onTouchcancel x: %f, y: %f", pos.x(), pos.y());
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

	Sp<MouseEvent> EventDispatch::NewMouseEvent(View* view, Vec2 pos, KeyboardKeyCode keycode) {
		return NewEvent<MouseEvent>(view, pos, keycode, 0,
			_keyboard->shift(),
			_keyboard->ctrl(), _keyboard->alt(),
			_keyboard->command(), _keyboard->caps_lock(), 0, 0, 0
		);
	}

	void EventDispatch::mousemove(View *view, Vec2 pos) {
		View* d_view = _mouse_handle->click_down_view();
		if ( d_view ) { // no invalid
			Vec2 position = d_view->position();
			Vec2 start_position = _mouse_handle->view_start_position();
			float d = (start_position - position).lengthSq() * _window->scale() / _window->defaultScale();
			// 视图位置移动超过4平方距离取消点击状态
			if ( d > 4 ) { // trigger invalid status
				if (view == d_view) {
					_inl_view(view)->trigger_highlightted( // emit style status event
						**NewEvent<HighlightedEvent>(view, HighlightedStatus::kHover));
				}
				_mouse_handle->set_click_down_view_Wt(nullptr);
			}
		}

		View* old = _mouse_handle->view();

		if (old != view) {
			_mouse_handle->set_view_Wt(view);

			if (old) {
				auto evt = NewMouseEvent(old, pos, KEYCODE_UNKNOWN);
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
				auto evt = NewMouseEvent(view, pos, KEYCODE_UNKNOWN);
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
			_inl_view(view)->bubble_trigger(UIEvent_MouseMove, **NewMouseEvent(view, pos, KEYCODE_UNKNOWN));
		}
	}

	void EventDispatch::mousepress(View *view, Vec2 pos, KeyboardKeyCode code, bool down) {
		if (_mouse_handle->view() != view) {
			mousemove(view, pos);
		}
		if (!view) return;

		auto evt = NewMouseEvent(view, pos, code);

		Sp<View> raw_down_view = _mouse_handle->click_down_view();

		if (down) {
			_mouse_handle->set_click_down_view_Wt(view);
			_inl_view(view)->bubble_trigger(UIEvent_MouseDown, **evt);
			_window->setCursorStyle(view->cursor(), true);
		} else {
			_mouse_handle->set_click_down_view_Wt(nullptr);
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
				_inl_view(view)->trigger_click(**NewEvent<ClickEvent>(view, pos, ClickEvent::kMouse));
			}
		}
	}

	void EventDispatch::onMousemove(float x, float y) {
		UILock lock(_window);
		if (_window->root()) {
			Vec2 pos(x, y);
			// Qk_DLog("onMousemove x: %f, y: %f", x, y);
			_mouse_handle->set_position(pos); // set current mouse pos
			auto v = find_receive_view_and_retain(pos);
			if (v) {
				_loop->post(Cb([this,v,pos](auto& e) {
					mousemove(v, pos);
					v->release(); // it has to release
				}),v);
			}
		}
	}

	void EventDispatch::onMousepress(KeyboardKeyCode code, bool isDown, const Vec2 *val) {
		Vec2 deltaDefault;
		switch(code) {
			case KEYCODE_MOUSE_LEFT:
			case KEYCODE_MOUSE_CENTER:
			case KEYCODE_MOUSE_RIGHT: {
				UILock lock(_window); // Lock ui render
				auto pos = val ? *val: _mouse_handle->position(); // get current mouse pos
				auto v = find_receive_view_and_retain(pos);
				//Qk_DLog("onMousepress code: %d, isDown: %i, v: %p", code, isDown, v);
				if (v)
					_loop->post(Cb([this,v,pos,code,isDown](auto& e) {
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
						auto v = _mouse_handle->view();
						if (v) {
							_inl_view(v)->bubble_trigger(UIEvent_MouseWheel,
								**NewMouseEvent(v, delta, code)
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
						auto evt = NewEvent<HighlightedEvent>(view, HighlightedStatus::kActive);
						_inl_view(view)->trigger_highlightted(**evt); // emit click status event
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
		if (!view) view = _window->root();
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
					auto evt = NewEvent<ClickEvent>(view, point, ClickEvent::kKeyboard);
					_inl_view(view)->bubble_trigger(UIEvent_Back, **evt); // emit back

					if ( evt->is_default() ) {
						_window->pending();
					}
				}
				else if ( evt->keycode() == KEYCODE_CENTER ) {
					auto evt = NewEvent<HighlightedEvent>(view, HighlightedStatus::kHover);
					_inl_view(view)->trigger_highlightted(**evt); // emit style status event

					auto evt2 = NewEvent<ClickEvent>(view, point, ClickEvent::kKeyboard);
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
