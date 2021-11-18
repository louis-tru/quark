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

#include "../_js.h"
#include "../_view.h"
#include "../../event.h"
#include "../../action/action.h"
#include "../../views2/button.h"
#include "../../views2/panel.h"
#include "../../keyboard.h"
#include "../str.h"
#include "./_event.h"
#include <native-inl-js.h>

/**
 * @ns flare::js
 */

JS_BEGIN

using namespace native_js;

Cast::Cast(CastFunc func): _cast_func(func) { }

Local<JSValue> Cast::cast(const Object& object, Worker* worker) {
	return _cast_func ? _cast_func(object, worker): worker->NewNull();
}

// ------------------------------------------------------------------------

typedef Event<> NativeEvent;

class WrapNativeEvent: public WrapObject {
	public:
	typedef Event<> Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	static void noticer(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_UNPACK(Type);
		JS_RETURN( wrap->get( worker->strs()->_noticer() ) );
	}
	
	static void sender(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_UNPACK(Type);
		Local<JSValue> noticer = wrap->get(worker->strs()->_noticer());
		
		if ( !noticer.IsEmpty() && noticer->IsObject(worker) ) {
			Local<JSValue> sender = noticer.To<JSObject>()->Get(worker, worker->strs()->sender());
			if ( !sender.IsEmpty() ) {
				JS_RETURN(sender);
			}
		} else {
			if ( wrap->self()->noticer() ) {
				Wrap<Object>* sender = Wrap<Object>::pack( wrap->self()->sender() );
				if ( sender ) {
					JS_RETURN( sender->that() );
				}
			}
		}
		JS_RETURN_NULL();
	}
	
	static void name(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_UNPACK(Type);
		Local<JSValue> noticer = wrap->get(worker->strs()->_noticer());
		
		if ( !noticer.IsEmpty() && noticer->IsObject(worker) ) {
			Local<JSValue> name = noticer.To<JSObject>()->Get(worker, worker->strs()->name());
			if ( !name.IsEmpty() ) {
				JS_RETURN(name);
			}
		} else {
			if ( wrap->self()->noticer() ) {
				JS_RETURN( wrap->self()->name() );
			} else {
				JS_RETURN_NULL();
			}
		}
	}
	
	static void data(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_UNPACK(Type);
		Cast* cast = static_cast<Cast*>(wrap->privateData());
		if ( cast ) {
			JS_RETURN( cast->cast(*wrap->self()->data(), worker) );
		}
		JS_RETURN_NULL();
	}

	static void origin(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_RETURN_NULL();
	}
	
	static void return_value(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->return_value );
	}
	
	static void set_return_value(Local<JSString> name,
															 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args);
		if ( !value->IsBoolean(worker) && !value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(Type);
		self->return_value = value->ToInt32Value(worker);
	}
	
	public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_NEW_CLASS(NativeEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(noticer, noticer);
			JS_SET_CLASS_ACCESSOR(sender, sender);
			JS_SET_CLASS_ACCESSOR(data, data);
			JS_SET_CLASS_ACCESSOR(name, name);
			JS_SET_CLASS_ACCESSOR(origin, origin);
			JS_SET_CLASS_ACCESSOR(returnValue, return_value, set_return_value);
		}, exports->GetProperty(worker, "Event").To<JSFunction>());
		
		cls->Export(worker, "NativeEvent", exports);
	}
};


/**
 * @class WrapUIEvent
 */
class WrapUIEvent: public WrapObject {
	public:
	typedef UIEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	/**
	 * @get origin {View*}
	 */
	static void origin(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		if ( self->origin() ) {
			Wrap<View>* wrap = Wrap<View>::pack(self->origin());
			JS_RETURN( wrap->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @get timestamp {uint64}
	 */
	static void timestamp(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->timestamp() );
	}
	
	/**
	 * @func cancel_default()
	 */
	static void cancel_default(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		self->cancel_default();
	}
	
	/**
	 * @func cancel_bubble()
	 */
	static void cancel_bubble(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		self->cancel_bubble();
	}
	
	/**
	 * @get is_default {bool}
	 */
	static void is_default(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->is_default() );
	}
	
	/**
	 * @get is_bubble {bool}
	 */
	static void is_bubble(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->is_bubble() );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UIEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(origin, origin);
			JS_SET_CLASS_ACCESSOR(timestamp, timestamp);
			JS_SET_CLASS_METHOD(cancelDefault, cancel_default);
			JS_SET_CLASS_METHOD(cancelBubble, cancel_bubble);
			JS_SET_CLASS_ACCESSOR(isDefault, is_default);
			JS_SET_CLASS_ACCESSOR(isBubble, is_bubble);
		}, Event<>);
	}
};

/**
 * @class WrapUIActionEvent
 */
class WrapUIActionEvent: public WrapObject {
	public:
	typedef UIActionEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	/**
	 * @get action {Action*}
	 */
	static void action(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIActionEvent);
		if ( self->action() ) {
			Wrap<Action>* wrap = Wrap<Action>::pack(self->action());
			JS_RETURN( wrap->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @get delay {uint64}
	 */
	static void delay(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIActionEvent);
		JS_RETURN( self->delay() / 1000 );
	}
	
	/**
	 * @get frame {uint}
	 */
	static void frame(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIActionEvent);
		JS_RETURN( self->frame() );
	}
	
	/**
	 * @get loop {uint}
	 */
	static void loop(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIActionEvent);
		JS_RETURN( self->loop() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UIActionEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(action, action);
			JS_SET_CLASS_ACCESSOR(delay, delay);
			JS_SET_CLASS_ACCESSOR(frame, frame);
			JS_SET_CLASS_ACCESSOR(loop, loop);
		}, UIEvent);
	}
};

/**
 * @class WrapUIKeyEvent
 */
class WrapUIKeyEvent: public WrapObject {
	public:
	typedef UIKeyEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	/**
	 * @get keycode {KeyboardKeyName}
	 */
	static void keycode(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->keycode() );
	}
	
	/**
	 * @get repeat {int}
	 */
	static void repeat(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->repeat() );
	}
	
	/**
	 * @get shift {bool}
	 */
	static void shift(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->shift() );
	}
	
	/**
	 * @get ctrl {bool}
	 */
	static void ctrl(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->ctrl() );
	}
	
	/**
	 * @get alt {bool}
	 */
	static void alt(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->alt() );
	}
	
	/**
	 * @get command {bool}
	 */
	static void command(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->command() );
	}
	
	/**
	 * @get caps_lock {bool}
	 */
	static void caps_lock(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->caps_lock() );
	}
	
	/**
	 * @get device {int}
	 */
	static void device(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->device() );
	}
	
	/**
	 * @get source {int}
	 */
	static void source(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		JS_RETURN( self->source() );
	}
	
	/**
	 * @get focus_move {View*}
	 */
	static void focus_move(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		if ( self->focus_move() ) {
			JS_RETURN( Wrap<View>::pack(self->focus_move(), View::VIEW)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @set focus_move {View*}
	 */
	static void set_focus_move(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args);
		JS_SELF(Type);
		View* view = nullptr;
		if ( worker->hasInstance(value, View::VIEW) ) {
			view = WrapObject::unpack<Button>(value.To<JSObject>())->self();
		} else if ( ! value->IsNull(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		self->set_focus_move(view);
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UIKeyEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(keycode, keycode);
			JS_SET_CLASS_ACCESSOR(repeat, repeat);
			JS_SET_CLASS_ACCESSOR(shift, shift);
			JS_SET_CLASS_ACCESSOR(ctrl, ctrl);
			JS_SET_CLASS_ACCESSOR(alt, alt);
			JS_SET_CLASS_ACCESSOR(command, command);
			JS_SET_CLASS_ACCESSOR(capsLock, caps_lock);
			JS_SET_CLASS_ACCESSOR(device, device);
			JS_SET_CLASS_ACCESSOR(source, source);
			JS_SET_CLASS_ACCESSOR(focusMove, focus_move, set_focus_move);
		}, UIEvent);
	}
};

/**
 * @class WrapUIClickEvent
 */
class WrapUIClickEvent: public WrapObject {
	public:
	typedef UIClickEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	/**
	 * @get x {float}
	 */
	static void x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIClickEvent);
		JS_RETURN( self->x() );
	}
	
	/**
	 * @get y {float}
	 */
	static void y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIClickEvent);
		JS_RETURN( self->y() );
	}
	
	/**
	 * @get count {uint}
	 */
	static void count(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIClickEvent);
		JS_RETURN( self->count() );
	}
	
	/**
	 * @get mode {int}
	 */
	static void type(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIClickEvent);
		JS_RETURN( int(self->type()) );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UIClickEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(x, x);
			JS_SET_CLASS_ACCESSOR(y, y);
			JS_SET_CLASS_ACCESSOR(type, type);
			JS_SET_CLASS_ACCESSOR(count, count);
		}, UIEvent);
	}
};

/**
 * @class WrapUIHighlightedEvent
 */
class WrapUIHighlightedEvent: public WrapObject {
	public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	/**
	 * @get status {HighlightedStatus}
	 */
	static void status(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIHighlightedEvent);
		JS_RETURN( self->status() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UIHighlightedEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(status, status);
		}, UIEvent);
	}
};

/**
 * @class UIMouseEvent
 */
class WrapUIMouseEvent: public WrapObject {
	public:
	typedef UIMouseEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	static void x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIMouseEvent);
		JS_RETURN( self->x() );
	}
	
	static void y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIMouseEvent);
		JS_RETURN( self->y() );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UIMouseEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(x, x);
			JS_SET_CLASS_ACCESSOR(y, y);
		}, UIKeyEvent);
	}
};

/**
 * @class WrapUITouchEvent
 */
class WrapUITouchEvent: public WrapObject {
	public:
	typedef UITouchEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	/**
	 * @get changedTouches {Array}
	 */
	static void changedTouches(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_UNPACK(UITouchEvent);
		JS_HANDLE_SCOPE();
		
		Local<JSValue> r = wrap->get(worker->strs()->_change_touches());
		
		if (r.IsEmpty()) { // js error
			return;
		}
		
		if ( !r->IsArray(worker) ) {
			Local<JSArray> arr = worker->NewArray();
			int j = 0;
			
			for ( auto& i : wrap->self()->changed_touches() ) {
				Local<JSObject> item = worker->NewObject();
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
		JS_RETURN( r );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UITouchEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(changedTouches, changedTouches);
		}, UIEvent);
	}
};

/**
 * @class WrapUISwitchEvent
 */
class WrapUIFocusMoveEvent: public WrapObject {
	public:
	typedef UIFocusMoveEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	/**
	 * @get focus {View*}
	 */
	static void focus(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIFocusMoveEvent);
		
		if ( self->focus() ) {
			JS_RETURN( Wrap<View>::pack(self->focus(), View::VIEW)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @get focusMove {View*}
	 */
	static void focusMove(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(UIFocusMoveEvent);
		
		if ( self->focus_move() ) {
			JS_RETURN( Wrap<View>::pack(self->focus_move(), View::VIEW)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(UIFocusMoveEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(focus, focus);
			JS_SET_CLASS_ACCESSOR(focusMove, focusMove);
		}, UIEvent);
	}
};

class BindingNativeEvent {
	public:
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->runNativeScript(WeakBuffer((Char*)
															INL_native_js_code__event_,
															INL_native_js_code__event_count_), "_event.js", exports);
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

JS_REG_MODULE(_event, BindingNativeEvent)
JS_END
