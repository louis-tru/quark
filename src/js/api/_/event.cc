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

#include "../types.h"
#include "./view.h"
#include "../../ui/event.h"
#include "../../ui/action/action.h"
#include "../../ui/view/button.h"
#include "../../ui/keyboard.h"
#include <native-inl-js.h>

namespace qk { namespace js {
	typedef Event<> NativeEvent;

	class WrapNativeEvent: public WrapObject {
	public:
		typedef Event<> Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}
		
		static void noticer(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_UNPACK(Type);
			Js_Return( wrap->get( worker->strs()->_noticer() ) );
		}
		
		static void sender(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_UNPACK(Type);
			JSValue* noticer = wrap->get(worker->strs()->_noticer());
			
			if ( !noticer.IsEmpty() && noticer->IsObject(worker) ) {
				JSValue* sender = noticer.To<JSObject>()->Get(worker, worker->strs()->sender());
				if ( !sender.IsEmpty() ) {
					Js_Return(sender);
				}
			} else {
				if ( wrap->self()->noticer() ) {
					Wrap<Object>* sender = Wrap<Object>::pack( wrap->self()->sender() );
					if ( sender ) {
						Js_Return( sender->that() );
					}
				}
			}
			Js_Return_Null();
		}
		
		static void name(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_UNPACK(Type);
			JSValue* noticer = wrap->get(worker->strs()->_noticer());
			
			if ( !noticer.IsEmpty() && noticer->IsObject(worker) ) {
				JSValue* name = noticer.To<JSObject>()->Get(worker, worker->strs()->name());
				if ( !name.IsEmpty() ) {
					Js_Return(name);
				}
			} else {
				if ( wrap->self()->noticer() ) {
					Js_Return( wrap->self()->name() );
				} else {
					Js_Return_Null();
				}
			}
		}
		
		static void data(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_UNPACK(Type);
			Cast* cast = static_cast<Cast*>(wrap->privateData());
			if ( cast ) {
				Js_Return( cast->cast(*wrap->self()->data(), worker) );
			}
			Js_Return_Null();
		}

		static void origin(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Return_Null();
		}
		
		static void return_value(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->return_value );
		}
		
		static void set_return_value(JSString* name,
																JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( !value->IsBoolean(worker) && !value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(Type);
			self->return_value = value->ToInt32Value(worker);
		}
		
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_NEW_CLASS(NativeEvent, constructor, {
				Js_Set_Class_Accessor(noticer, noticer);
				Js_Set_Class_Accessor(sender, sender);
				Js_Set_Class_Accessor(data, data);
				Js_Set_Class_Accessor(name, name);
				Js_Set_Class_Accessor(origin, origin);
				Js_Set_Class_Accessor(returnValue, return_value, set_return_value);
			}, exports->GetProperty(worker, "Event").To<JSFunction>());
			
			cls->Export(worker, "NativeEvent", exports);
		}
	};

	class WrapUIEvent: public WrapObject {
		public:
		typedef UIEvent Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}

		static void origin(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			if ( self->origin() ) {
				Wrap<View>* wrap = Wrap<View>::pack(self->origin());
				Js_Return( wrap->that() );
			} else {
				Js_Return_Null();
			}
		}

		static void timestamp(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->timestamp() );
		}

		static void cancel_default(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			self->cancel_default();
		}

		static void cancel_bubble(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			self->cancel_bubble();
		}

		static void is_default(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->is_default() );
		}

		static void is_bubble(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->is_bubble() );
		}
		
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIEvent, constructor, {
				Js_Set_Class_Accessor(origin, origin);
				Js_Set_Class_Accessor(timestamp, timestamp);
				Js_Set_Class_Method(cancelDefault, cancel_default);
				Js_Set_Class_Method(cancelBubble, cancel_bubble);
				Js_Set_Class_Accessor(isDefault, is_default);
				Js_Set_Class_Accessor(isBubble, is_bubble);
			}, Event<>);
		}
	};

	class WrapUIActionEvent: public WrapObject {
		public:
		typedef UIActionEvent Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}

		static void action(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIActionEvent);
			if ( self->action() ) {
				Wrap<Action>* wrap = Wrap<Action>::pack(self->action());
				Js_Return( wrap->that() );
			} else {
				Js_Return_Null();
			}
		}

		static void delay(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIActionEvent);
			Js_Return( self->delay() / 1000 );
		}

		static void frame(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIActionEvent);
			Js_Return( self->frame() );
		}

		static void loop(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIActionEvent);
			Js_Return( self->loop() );
		}
		
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIActionEvent, constructor, {
				Js_Set_Class_Accessor(action, action);
				Js_Set_Class_Accessor(delay, delay);
				Js_Set_Class_Accessor(frame, frame);
				Js_Set_Class_Accessor(loop, loop);
			}, UIEvent);
		}
	};

	class WrapUIKeyEvent: public WrapObject {
		public:
		typedef UIKeyEvent Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}

		static void keycode(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->keycode() );
		}

		static void repeat(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->repeat() );
		}

		static void shift(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->shift() );
		}

		static void ctrl(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->ctrl() );
		}

		static void alt(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->alt() );
		}

		static void command(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->command() );
		}

		static void caps_lock(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->caps_lock() );
		}

		static void device(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->device() );
		}

		static void source(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			Js_Return( self->source() );
		}

		static void focus_move(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			if ( self->focus_move() ) {
				Js_Return( Wrap<View>::pack(self->focus_move(), View::VIEW)->that() );
			} else {
				Js_Return_Null();
			}
		}

		static void set_focus_move(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			Js_Self(Type);
			View* view = nullptr;
			if ( worker->hasInstance(value, View::VIEW) ) {
				view = WrapObject::unpack<Button>(value.To<JSObject>())->self();
			} else if ( ! value->IsNull(worker) ) {
				Js_Throw("Bad argument.");
			}
			self->set_focus_move(view);
		}
		
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIKeyEvent, constructor, {
				Js_Set_Class_Accessor(keycode, keycode);
				Js_Set_Class_Accessor(repeat, repeat);
				Js_Set_Class_Accessor(shift, shift);
				Js_Set_Class_Accessor(ctrl, ctrl);
				Js_Set_Class_Accessor(alt, alt);
				Js_Set_Class_Accessor(command, command);
				Js_Set_Class_Accessor(capsLock, caps_lock);
				Js_Set_Class_Accessor(device, device);
				Js_Set_Class_Accessor(source, source);
				Js_Set_Class_Accessor(focusMove, focus_move, set_focus_move);
			}, UIEvent);
		}
	};

	class WrapUIClickEvent: public WrapObject {
		public:
		typedef UIClickEvent Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}

		static void x(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIClickEvent);
			Js_Return( self->x() );
		}

		static void y(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIClickEvent);
			Js_Return( self->y() );
		}

		static void count(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIClickEvent);
			Js_Return( self->count() );
		}

		static void type(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIClickEvent);
			Js_Return( int(self->type()) );
		}
		
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIClickEvent, constructor, {
				Js_Set_Class_Accessor(x, x);
				Js_Set_Class_Accessor(y, y);
				Js_Set_Class_Accessor(type, type);
				Js_Set_Class_Accessor(count, count);
			}, UIEvent);
		}
	};

	class WrapUIHighlightedEvent: public WrapObject {
		public:
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}
		
		/**
		 * @get status {HighlightedStatus}
		 */
		static void status(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIHighlightedEvent);
			Js_Return( self->status() );
		}
		
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIHighlightedEvent, constructor, {
				Js_Set_Class_Accessor(status, status);
			}, UIEvent);
		}
	};

	class WrapUIMouseEvent: public WrapObject {
		public:
		typedef UIMouseEvent Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}
		
		static void x(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIMouseEvent);
			Js_Return( self->x() );
		}
		
		static void y(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIMouseEvent);
			Js_Return( self->y() );
		}
		
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIMouseEvent, constructor, {
				Js_Set_Class_Accessor(x, x);
				Js_Set_Class_Accessor(y, y);
			}, UIKeyEvent);
		}
	};

	class WrapUITouchEvent: public WrapObject {
		public:
		typedef UITouchEvent Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}

		static void changedTouches(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_UNPACK(UITouchEvent);
			Js_Handle_Scope();
			
			JSValue* r = wrap->get(worker->strs()->_change_touches());
			
			if (r.IsEmpty()) { // js error
				return;
			}
			
			if ( !r->IsArray(worker) ) {
				Local<JSArray> arr = worker->NewArray();
				int j = 0;
				
				for ( auto& i : wrap->self()->changed_touches() ) {
					JSObject* item = worker->NewObject();
					//
					Wrap<View>* view = Wrap<View>::pack(i.value().view);
					item->Set(worker,worker->strs()->id(), worker->New(i.value().id));
					item->Set(worker,worker->strs()->startX(), worker->New(i.value().start_x));
					item->Set(worker,worker->strs()->startY(), worker->New(i.value().start_y));
					item->Set(worker,worker->strs()->x(), worker->New(i.value().x));
					item->Set(worker,worker->strs()->y(), worker->New(i.value().y));
					item->Set(worker,worker->strs()->force(), worker->New(i.value().force));
					item->Set(worker,worker->strs()->clickIn(), worker->New(i.value().click_in));
					item->Set(worker,worker->strs()->view(), view->that());
					//
					arr->Set(worker, j, item);
					j++;
				}
				
				r = arr.To<JSValue>();
				
				wrap->set(worker->strs()->_change_touches(), r);
			}
			Js_Return( r );
		}
		
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UITouchEvent, constructor, {
				Js_Set_Class_Accessor(changedTouches, changedTouches);
			}, UIEvent);
		}
	};

	class WrapUIFocusMoveEvent: public WrapObject {
		public:
		typedef UIFocusMoveEvent Type;
		
		static void constructor(FunctionArgs args) {
			Js_ATTACH(args);
			Js_Worker(args);
			Js_Throw("Access forbidden.");
		}

		static void focus(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIFocusMoveEvent);
			
			if ( self->focus() ) {
				Js_Return( Wrap<View>::pack(self->focus(), View::VIEW)->that() );
			} else {
				Js_Return_Null();
			}
		}

		static void focusMove(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(UIFocusMoveEvent);
			
			if ( self->focus_move() ) {
				Js_Return( Wrap<View>::pack(self->focus_move(), View::VIEW)->that() );
			} else {
				Js_Return_Null();
			}
		}
		
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIFocusMoveEvent, constructor, {
				Js_Set_Class_Accessor(focus, focus);
				Js_Set_Class_Accessor(focusMove, focusMove);
			}, UIEvent);
		}
	};

	class BindingNativeEvent {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			worker->runNativeScript(WeakBuffer((Char*)
															native_js::INL_native_js_code__event_,
															native_js::INL_native_js_code__event_count_).buffer(), "_event.js", exports);
			WrapNativeEvent::binding(exports, worker);
			WrapUIEvent::binding(exports, worker);
			WrapUIActionEvent::binding(exports, worker);
			WrapUIKeyEvent::binding(exports, worker);
			WrapUIClickEvent::binding(exports, worker);
			WrapUIMouseEvent::binding(exports, worker);
			WrapUITouchEvent::binding(exports, worker);
			WrapUIFocusMoveEvent::binding(exports, worker);
			WrapUIHighlightedEvent::binding(exports, worker);
		}
	};

	Js_Set_Module(_event, BindingNativeEvent)
} }