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

#include "./event.h"
#include "./_app.h"
#include "./layout/root.h"
#include "./util/os.h"
#include "./keyboard.h"

namespace flare {

	#define FX_FUN(NAME, FLAG) \
		const UIEventName UIEvent_##NAME(#NAME, FLAG);
	FX_UI_EVENTs(FX_FUN)
	#undef FX_FUN

	const Dict<String, UIEventName> UIEventNames([]() -> Dict<String, UIEventName> {
		Dict<String, UIEventName> r;
		#define FX_FUN(NAME, FLAG) \
			r[UIEvent_##NAME.to_string()] = UIEvent_##NAME;
		FX_UI_EVENTs(FX_FUN)
		#undef FX_FUN
		return r;
	}());

	UIEvent::UIEvent(View* origin)
		: Event(SendData(), origin), time_(os::time()) {
		return_value = RETURN_VALUE_MASK_ALL;
	}

	void ActionEvent::release() {
		action_ = nullptr;
		UIEvent::release();
	}

	void KeyEvent::release() {
		focus_move_ = nullptr;
		UIEvent::release();
	}

	void FocusMoveEvent::release()  {
		_old_focus = nullptr;
		_new_focus = nullptr;
		UIEvent::release();
	}

	static inline HighlightedStatus HOVER_or_NORMAL(View* view) {
		return view->is_focus() ? HIGHLIGHTED_HOVER : HIGHLIGHTED_NORMAL;
	}

	template<class T, typename... Args>
	inline static Handle<T> NewEvent(Args... args) { return new T(args...); }

	FX_DEFINE_INLINE_MEMBERS(View, InlEvent) {
	 public:
		#define _inl_view(self) static_cast<View::InlEvent*>(static_cast<View*>(self))

		/**
		 * @func trigger_highlightted
		 */
		void trigger_highlightted(HighlightedEvent& evt) {
			View* view = this;
			if ( view ) {
				if ( view->_receive ) {
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
		
		/**
		 * @func bubble_trigger
		 */
		void bubble_trigger(const NameType& name, UIEvent& evt) {
			View* view = this;
			while( view ) {
				if ( view->_receive ) {
					view->trigger(name, evt);
					if ( !evt.is_bubble() ) {
						break; // Stop bubble
					}
				}
				view = view->parent();
			}
		}

		void trigger(const NameType& name, UIEvent& evt) {
			if ( _receive ) {
				Notification::trigger(name, evt);
			}
		}
	};

	/**
	 * @func focus()
	 */
	bool View::focus() {
		if ( is_focus() ) return true;
		
		View* old = app()->focus_view();
		Handle<FocusMoveEvent> evt;

		// TODO Keyboard navigation ...
		// Panel* panel = reinterpret_cast<Button*>(this)->panel();
		// if ( panel ) {
		// 	evt = NewEvent<FocusMoveEvent>(panel, old, this);
		// 	_inl_view(panel)->trigger(UIEvent_FocusMove, **evt );
		// }
		
		if ( (*evt && !evt->is_default()) || !_inl_app(app())->set_focus_view(this) ) {
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
	
	/**
	 * @func blur()
	 */
	bool View::blur() {
		if ( is_focus() ) {
			View* v = app()->root();
			if ( v ) {
				if ( v != this ) {
					return v->focus();
				}
			}
			return false;
		}
		return true;
	}
	
	/**
	 * @class EventDispatch::OriginTouche
	 */
	class EventDispatch::OriginTouche {
	 public:
		OriginTouche() { FX_UNREACHABLE(); }
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
			return Vec2(view->_matrix[2], view->_matrix[5]);
		}
		inline View* view() { return _view; }
		inline Vec2 view_start_position() { return _start_position; }
		inline bool is_click_invalid() { return _is_click_invalid; }
		inline bool is_click_down() { return _is_click_down; }
		inline void set_click_invalid() {
			_is_click_invalid = true;
			_is_click_down = false;
		}
		inline void set_click_down(bool value) {
			if ( !_is_click_invalid )
				_is_click_down = value;
		}
		inline Dict<uint32_t, TouchPoint>& values() { return _touches; }
		inline TouchPoint& operator[](uint32_t id) { return _touches[id]; }
		inline uint32_t count() { return _touches.length(); }
		inline bool has(uint32_t id) { return _touches.count(id); }
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

	/**
	 * @class EventDispatch::Inl
	 */
	FX_DEFINE_INLINE_MEMBERS(EventDispatch, Inl) {
	 public:
		#define _inl(self) static_cast<EventDispatch::Inl*>(self)
		
		// -------------------------- touch --------------------------

		void touchstart(View* view, List<TouchPoint>& in) {
			if ( view->_receive && in.length() ) {
				Array<TouchPoint> change_touches;
				
				for ( auto i = in.begin(), e = in.end(); i != e; ) {

					if ( view->overlap_test(Vec2(i->x, i->y)) ) {
						TouchPoint& touch = *i;
						touch.start_x = touch.x;
						touch.start_y = touch.y;
						touch.click_in = true;
						touch.view = view;
						
						if ( !_origin_touches.count(view) ) {
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
						_origin_touches[view]->set_click_down(true);
						auto evt = NewEvent<HighlightedEvent>(view, HIGHLIGHTED_DOWN);
						_inl_view(view)->trigger_highlightted(**evt); // emit event
					}
				}
			}
		}

		void onTouchstart(View* view, List<TouchPoint>& in) {
			
			if ( view->_visible && in.length() ) {
				if ( view->_region_visible /*|| view->_need_draw*/ ) {
					
					if ( view->_last /*&& view->as_box()*/ /*&& static_cast<Box*>(view)->clip()*/ ) {
						List<TouchPoint> in2;
						
						for ( auto i = in.begin(), e = in.end(); i != e; ) {
							if ( view->overlap_test(Vec2(i->x, i->y)) ) {
								in2.push_back(*i);
								in.erase(i++);
							} else {
								i++;
							}
						}
						
						View* v = view->_last;
						while( v && in2.length() ) {
							onTouchstart(v, in2);
							v = v->_prev;
						}
						
						touchstart(view, in2);
						
						if ( in2.length() ) {
							in.splice(in.end(), in2);
						}
					} else {
						View* v = view->_last;
						while( v && in.length() ) {
							onTouchstart(v, in);
							v = v->_prev;
						}
						touchstart(view, in);
					}
					
				}
			}
		}

		void onTouchmove(List<TouchPoint>& in) {
			Dict<View*, Array<TouchPoint>> change_touches;
			
			for ( auto in_touch : in ) {
				// TouchPoint& in_touch = i;
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
									trigger_event = false; break;
								}
							}
							if ( trigger_event ) {
								origin_touche->set_click_down(false); // set up status
								// emit style status event
								auto evt = NewEvent<HighlightedEvent>(view, HOVER_or_NORMAL(view));
								_inl_view(view)->trigger_highlightted(**evt);
							}
						} else { // May trigger click down
							for ( int i = 0; i < touchs.length(); i++) {
								auto item = touchs[i];
								if ( item.click_in ) { // find range == true
									origin_touche->set_click_down(true); // set down status
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
		
		void onTouchend(List<TouchPoint>& in, const UIEventName& type) {
			Dict<View*, Array<TouchPoint>> change_touches;
			
			for ( auto& i : in ) {
				TouchPoint& in_touch = i;
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
								
								if ( type == UIEvent_TouchEnd && view->final_visible() ) {
									auto evt = NewEvent<ClickEvent>(view, item.x, item.y, ClickEvent::TOUCH);
									_inl_view(view)->bubble_trigger(UIEvent_Click, **evt); // emit click event
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

		// -------------------------- mouse --------------------------

		Handle<MouseEvent> NewMouseEvent(View* view, float x, float y, uint32_t keycode = 0) {
			return NewEvent<MouseEvent>(view, x, y, keycode,
				_keyboard->shift(),
				_keyboard->ctrl(), _keyboard->alt(),
				_keyboard->command(), _keyboard->caps_lock(), 0, 0, 0
			);
		}

		inline static bool test_view_level(View* parent, View* subview) {
			return parent->has_child(subview);
		}

		static View* find_receive_event_view_2(View* view, Vec2 pos) {
			if ( view->_visible ) {
				if ( view->_region_visible/* || view->_need_draw*/ ) {
					View* v = view->_last;

					if (v /*&& view->as_box()*/ /*&& static_cast<Box*>(view)->clip()*/ ) {
						if (view->overlap_test(pos)) {
							while (v) {
								auto r = find_receive_event_view_2(v, pos);
								if (r) {
									return r;
								}
								v = v->_prev;
							}
							if (view->_receive) {
								return view;
							}
						}
					} else {
						while (v) {
							auto r = find_receive_event_view_2(v, pos);
							if (r) {
								return r;
							}
							v = v->_prev;
						}
						if (view->_receive && view->overlap_test(pos)) {
							return view;
						}
					}
				}
			}
			return nullptr;
		}

		inline View* find_receive_event_view(Vec2 pos) {
			return app_->root() ? find_receive_event_view_2(app_->root(), pos) : nullptr;
		}

		void onMousemove(View* view, Vec2 pos) {
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

						if (!view || !test_view_level(old, view)) {
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
						
						if (!old || !test_view_level(view, old)) {
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

		void onMousepress(KeyboardKeyName name, bool down, Vec2 pos) {
			float x = pos[0], y = pos[1];
			Handle<View> view(find_receive_event_view(pos));

			if (_mouse_h->view() != *view) {
				mousemove(*view, pos);
			}

			if (view.is_null()) return;

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
					_inl_view(*view)->bubble_trigger(UIEvent_Click,
						**NewEvent<ClickEvent>(*view, x, y, ClickEvent::MOUSE));
				}
			}
		}

		void onMousewhell(KeyboardKeyName name, bool down, float x, float y) {
			if (down) {
				auto view = _mouse_h->view();
				if (view) {
					_inl_view(view)->bubble_trigger(UIEvent_MouseWheel, **NewMouseEvent(view, x, y, name));
				}
			}
		}

		// -------------------------- keyboard --------------------------

		void onKeyboard_down() {

			View* view = app_->focus_view();
			if ( !view )
				view = app_->root();

			if ( view ) {
				auto name = _keyboard->keyname();
				View* focus_move = nullptr;

				// TODO Keyboard navigation ...
				// Panel* panel = nullptr;
				// Direction direction = Direction::NONE;
				// switch ( name ) {
				// 	case KEYCODE_LEFT: direction = Direction::LEFT; break;  // left
				// 	case KEYCODE_UP: direction = Direction::TOP; break;     // top
				// 	case KEYCODE_RIGHT: direction = Direction::RIGHT; break; // right
				// 	case KEYCODE_DOWN: direction = Direction::BOTTOM; break; // bottom
				// 	default: break;
				// }
				// if ( direction != Direction::NONE ) {
				// 	Button* button = view->as_button();
				// 	if ( button ) {
				// 		if ( (panel = button->panel()) && panel->enable_select() ) {
				// 			focus_move = button->find_next_button(direction);
				// 		}
				// 	}
				// }
				
				auto evt = NewEvent<KeyEvent>(view, name,
					_keyboard->shift(),
					_keyboard->ctrl(), _keyboard->alt(),
					_keyboard->command(), _keyboard->caps_lock(),
					_keyboard->repeat(), _keyboard->device(), _keyboard->source()
				);
				
				evt->set_focus_move(focus_move);
				
				_inl_view(view)->bubble_trigger(UIEvent_KeyDown, **evt);
				
				if ( evt->is_default() ) {
					
					if ( name == KEYCODE_ENTER ) {
						_inl_view(view)->bubble_trigger(UIEvent_KeyEnter, **evt);
					} else if ( name == KEYCODE_VOLUME_UP ) {
						_inl_app(app_)->set_volume_up();
					} else if ( name == KEYCODE_VOLUME_DOWN ) {
						_inl_app(app_)->set_volume_down();
					}
					
					int keypress_code = _keyboard->keypress();
					if ( keypress_code ) { // keypress
						evt->set_keycode( keypress_code );
						_inl_view(view)->bubble_trigger(UIEvent_KeyPress, **evt);
					}

					if ( name == KEYCODE_CENTER && _keyboard->repeat() == 0 ) {
						// Rect rect = view->screen_rect();
						auto evt = NewEvent<HighlightedEvent>(view, HIGHLIGHTED_DOWN);
						_inl_view(view)->trigger_highlightted(**evt); // emit click status event
					}
					
					if ( evt->focus_move() ) {
						evt->focus_move()->focus();
					}
					
				} // if ( evt->is_default() ) {
			} // if ( view )
		}
		
		void onKeyboard_up() {

			View* view = app_->focus_view();
			if ( !view )
				view = app_->root();

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
						Rect rect;// = view->screen_rect(); // TODO ...
						auto evt = NewEvent<ClickEvent>(view, rect.origin.x() + rect.size.x() / 2,
																								rect.origin.y() + rect.size.y() / 2,
																								ClickEvent::KEYBOARD);
						_inl_view(view)->bubble_trigger(UIEvent_Back, **evt); // emit back
						
						if ( evt->is_default() ) {
							// pending gui application (挂起应用)
							app_->pending();
						}
					}
					else if ( name == KEYCODE_CENTER ) {
						auto evt = NewEvent<HighlightedEvent>(view, HIGHLIGHTED_HOVER);
						_inl_view(view)->trigger_highlightted(**evt); // emit style status event
						
						Rect rect;// = view->screen_rect(); // TODO ...
						auto evt2 = NewEvent<ClickEvent>(view, rect.origin.x() + rect.size.x() / 2,
																							 rect.origin.y() + rect.size.y() / 2,
																							 ClickEvent::KEYBOARD);
						_inl_view(view)->bubble_trigger(UIEvent_Click, **evt2);
					} //
				}
			}
		}
		
	};

	EventDispatch::EventDispatch(Application* app): app_(app), _text_input(nullptr) {
		_keyboard = KeyboardAdapter::create();
		_mouse_h = new MouseHandle();
	}

	EventDispatch::~EventDispatch() {
		for (auto& i : _origin_touches) {
			delete i.value;
		}
		Release(_keyboard);
		delete _mouse_h;
	}

	#define _loop static_cast<PostMessage*>(app_->main_loop())

	typedef Callback<List<TouchPoint>> TouchCb;

	void EventDispatch::onTouchstart(List<TouchPoint>&& list) {
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock;
			Root* r = app_->root();
			if (r) {
				_inl(this)->onTouchstart(r, *evt.data);
			}
		}), std::move(list), _loop);
	}

	void EventDispatch::onTouchmove(List<TouchPoint>&& list) {
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock;
			_inl(this)->onTouchmove(*evt.data);
		}), std::move(list), _loop);
	}

	void EventDispatch::onTouchend(List<TouchPoint>&& list) {
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock;
			_inl(this)->onTouchend(*evt.data, UIEvent_TouchEnd);
		}), std::move(list), _loop);
	}

	void EventDispatch::onTouchcancel(List<TouchPoint>&& list) {
		async_resolve(TouchCb([this](TouchCb::Data& evt) {
			UILock lock;
			_inl(this)->onTouchend(*evt.data, UIEvent_TouchCancel);
		}), std::move(list), _loop);
	}

	void EventDispatch::onMousemove(float x, float y) {
		async_resolve(Cb([=](CbData& evt) {
			UILock lock;
			Vec2 pos(x, y);
			// set current mouse pos
			_mouse_h->set_position(pos);

			if (app_->root()) {
				Handle<View> v(_inl(this)->find_receive_event_view(pos));
				_inl(this)->onMousemove(*v, pos);
			}
		}), _loop);
	}

	void EventDispatch::onMousepress(KeyboardKeyName name, bool down) {
		async_resolve(Cb([=](CbData& evt) {
			UILock lock;
			switch(name) {
				case KEYCODE_MOUSE_LEFT:
				case KEYCODE_MOUSE_CENTER:
				case KEYCODE_MOUSE_RIGHT:
					_inl(this)->onMousepress(name, down, _mouse_h->position());
					break;
				case KEYCODE_MOUSE_WHEEL_UP:
					_inl(this)->onMousewhell(name, down, 0, -53); break;
				case KEYCODE_MOUSE_WHEEL_DOWN:
					_inl(this)->onMousewhell(name, down, 0, 53); break;
				case KEYCODE_MOUSE_WHEEL_LEFT:
					_inl(this)->onMousewhell(name, down, -53, 0); break;
				case KEYCODE_MOUSE_WHEEL_RIGHT:
					_inl(this)->onMousewhell(name, down, 53, 0); break;
				default: break;
			}
		}), _loop);
	}

	void KeyboardAdapter::dispatch(uint32_t keycode, bool unicode,
																 bool down, int repeat, int device, int source)
	{
		async_resolve(Cb([=](CbData& evt) {
			UILock lock;
			repeat_ = repeat; device_ = device;
			source_ = source;

			bool is_clear = transformation(keycode, unicode, down);
			
			if ( down ) {
				_inl(_inl_app(app_)->dispatch())->onKeyboard_down();
			} else {
				_inl(_inl_app(app_)->dispatch())->onKeyboard_up();
			}

			if ( is_clear ) {
				shift_ = alt_ = false;
				ctrl_ = command_ = false;
			}
		}), _loop);
	}

	void EventDispatch::onIme_delete(int count) {
		async_resolve(Cb([=](CbData& d) {
			UILock lock;
			if ( _text_input ) {
				_text_input->input_delete(count);
				bool can_backspace = _text_input->input_can_backspace();
				bool can_delete = _text_input->input_can_delete();
				_inl_app(app_)->ime_keyboard_can_backspace(can_backspace, can_delete);
			}
		}), _loop);
	}

	void EventDispatch::onIme_insert(cString& text) {
		async_resolve(Cb([=](CbData& d) {
			UILock lock;
			if ( _text_input ) {
				_text_input->input_insert(text);
			}
		}), _loop);
	}

	void EventDispatch::onIme_marked(cString& text) {
		async_resolve(Cb([=](CbData& d) {
			UILock lock;
			if ( _text_input ) {
				_text_input->input_marked(text);
			}
		}), _loop);
	}

	void EventDispatch::onIme_unmark(cString& text) {
		async_resolve(Cb([=](CbData& d) {
			UILock lock;
			if ( _text_input ) {
				_text_input->input_unmark(text);
			}
		}), _loop);
	}

	void EventDispatch::onIme_control(KeyboardKeyName name) {
		async_resolve(Cb([=](CbData& d) {
			UILock lock;
			if ( _text_input ) {
				_text_input->input_control(name);
			}
		}), _loop);
	}

	void EventDispatch::make_text_input(ITextInput* input) {
		DLOG("make_text_input");
		if ( input != _text_input ) {
			_text_input = input;
			
			if ( input ) {
				_inl_app(app_)->ime_keyboard_open({
					true, input->input_keyboard_type(),
					input->input_keyboard_return_type(),
					input->input_spot_location(),
				});
			} else {
				_inl_app(app_)->ime_keyboard_close();
			}
		} else {
			if ( input ) {
				_inl_app(app_)->ime_keyboard_open({
					false, input->input_keyboard_type(),
					input->input_keyboard_return_type(),
					input->input_spot_location(),
				});
			}
		}
	}

}
