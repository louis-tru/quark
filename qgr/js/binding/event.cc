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

#include "qgr/js/js-1.h"
#include "qgr/js/qgr.h"
#include "qgr/event.h"
#include "qgr/action.h"
#include "qgr/button.h"
#include "qgr/panel.h"
#include "qgr/keyboard.h"
#include "qgr/js/str.h"
#include "event-1.h"
#include "native-ext-js.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

using namespace native_js;

Cast::Cast(CastFunc func): m_cast_func(func) { }

Local<JSValue> Cast::cast(const Object& object, Worker* worker) {
	return m_cast_func ? m_cast_func(object, worker): worker->NewNull();
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
		JS_RETURN( wrap->get( worker->strs()->m_noticer() ) );
	}
	
	static void sender(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_UNPACK(Type);
		Local<JSValue> noticer = wrap->get(worker->strs()->m_noticer());
		
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
		Local<JSValue> noticer = wrap->get(worker->strs()->m_noticer());
		
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
		Cast* cast = static_cast<Cast*>(wrap->private_data());
		if ( cast ) {
			JS_RETURN( cast->cast(*wrap->self()->data(), worker) );
		}
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
			JS_SET_CLASS_ACCESSOR(returnValue, return_value, set_return_value);
		}, exports->GetProperty(worker, "Event").To<JSFunction>());
		
		cls->Export(worker, "NativeEvent", exports);
	}
};


/**
 * @class WrapGUIEvent
 */
class WrapGUIEvent: public WrapObject {
 public:
	typedef GUIEvent Type;
	
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
		JS_DEFINE_CLASS(GUIEvent, constructor, {
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
 * @class WrapGUIActionEvent
 */
class WrapGUIActionEvent: public WrapObject {
 public:
	typedef GUIActionEvent Type;
	
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
		JS_SELF(GUIActionEvent);
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
		JS_SELF(GUIActionEvent);
		JS_RETURN( self->delay() / 1000 );
	}
	
	/**
	 * @get frame {uint}
	 */
	static void frame(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(GUIActionEvent);
		JS_RETURN( self->frame() );
	}
	
	/**
	 * @get loop {uint}
	 */
	static void loop(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(GUIActionEvent);
		JS_RETURN( self->loop() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(GUIActionEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(action, action);
			JS_SET_CLASS_ACCESSOR(delay, delay);
			JS_SET_CLASS_ACCESSOR(frame, frame);
			JS_SET_CLASS_ACCESSOR(loop, loop);
		}, GUIEvent);
	}
};

/**
 * @class WrapGUIKeyEvent
 */
class WrapGUIKeyEvent: public WrapObject {
 public:
	typedef GUIKeyEvent Type;
	
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
		if ( worker->has_instance(value, View::VIEW) ) {
			view = WrapObject::unpack<Button>(value.To<JSObject>())->self();
		} else if ( ! value->IsNull(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		self->set_focus_move(view);
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(GUIKeyEvent, constructor, {
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
		}, GUIEvent);
	}
};

/**
 * @class WrapGUIClickEvent
 */
class WrapGUIClickEvent: public WrapObject {
 public:
	typedef GUIClickEvent Type;
	
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
		JS_SELF(GUIClickEvent);
		JS_RETURN( self->x() );
	}
	
	/**
	 * @get y {float}
	 */
	static void y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(GUIClickEvent);
		JS_RETURN( self->y() );
	}
	
	/**
	 * @get count {uint}
	 */
	static void count(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(GUIClickEvent);
		JS_RETURN( self->count() );
	}
	
	/**
	 * @get mode {int}
	 */
	static void mode(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(GUIClickEvent);
		JS_RETURN( int(self->mode()) );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(GUIClickEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(x, x);
			JS_SET_CLASS_ACCESSOR(y, y);
			JS_SET_CLASS_ACCESSOR(mode, mode);
			JS_SET_CLASS_ACCESSOR(count, count);
		}, GUIEvent);
	}
};

/**
 * @class WrapGUIHighlightedEvent
 */
class WrapGUIHighlightedEvent: public WrapObject {
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
		JS_SELF(GUIHighlightedEvent);
		JS_RETURN( self->status() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(GUIHighlightedEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(status, status);
		}, GUIEvent);
	}
};

/**
 * @class GUIMouseEvent
 */
class WrapGUIMouseEvent: public WrapObject {
 public:
	typedef GUIMouseEvent Type;
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Access forbidden.");
	}
	
	static void x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(GUIMouseEvent);
		JS_RETURN( self->x() );
	}
	
	static void y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(GUIMouseEvent);
		JS_RETURN( self->y() );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(GUIMouseEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(x, x);
			JS_SET_CLASS_ACCESSOR(y, y);
		}, GUIKeyEvent);
	}
};

/**
 * @class WrapGUITouchEvent
 */
class WrapGUITouchEvent: public WrapObject {
 public:
	typedef GUITouchEvent Type;
	
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
		JS_UNPACK(GUITouchEvent);
		JS_HANDLE_SCOPE();
		
		Local<JSValue> r = wrap->get(worker->strs()->m_change_touches());
		
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
				item->Set(worker,worker->strs()->view(), view->that());
				//
				arr->Set(worker, j, item);
				j++;
			}
			
			r = arr.To<JSValue>();
			
			wrap->set(worker->strs()->m_change_touches(), r);
		}
		JS_RETURN( r );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(GUITouchEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(changedTouches, changedTouches);
		}, GUIEvent);
	}
};

/**
 * @class WrapGUISwitchEvent
 */
class WrapGUIFocusMoveEvent: public WrapObject {
 public:
	typedef GUIFocusMoveEvent Type;
	
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
		JS_SELF(GUIFocusMoveEvent);
		
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
		JS_SELF(GUIFocusMoveEvent);
		
		if ( self->focus_move() ) {
			JS_RETURN( Wrap<View>::pack(self->focus_move(), View::VIEW)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(GUIFocusMoveEvent, constructor, {
			JS_SET_CLASS_ACCESSOR(focus, focus);
			JS_SET_CLASS_ACCESSOR(focusMove, focusMove);
		}, GUIEvent);
	}
};

class BindingNativeEvent {
 public:
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->run_native_script(WeakBuffer((char*)
															EXT_native_js_code_event_,
															EXT_native_js_code_event_count_), "event.js", exports);
		// Enum: HighlightedStatus
		JS_SET_PROPERTY(HIGHLIGHTED_NORMAL, HIGHLIGHTED_NORMAL);
		JS_SET_PROPERTY(HIGHLIGHTED_HOVER, HIGHLIGHTED_HOVER);
		JS_SET_PROPERTY(HIGHLIGHTED_DOWN, HIGHLIGHTED_DOWN);
		
		// Enum: ReturnValueMask
		JS_SET_PROPERTY(RETURN_VALUE_MASK_DEFAULT, RETURN_VALUE_MASK_DEFAULT);
		JS_SET_PROPERTY(RETURN_VALUE_MASK_BUBBLE, RETURN_VALUE_MASK_BUBBLE);
		JS_SET_PROPERTY(RETURN_VALUE_MASK_ALL, RETURN_VALUE_MASK_ALL);
		
		// Emun: KeyboardKeyName
	 #define def_enum_keyboard_key_name(Name, Code) JS_SET_PROPERTY(Name, Name);
		xx_each_keyboard_key_name_table(def_enum_keyboard_key_name)
	 #undef def_enum_keyboard_key_name
		
		WrapNativeEvent::binding(exports, worker);
		WrapGUIEvent::binding(exports, worker);
		WrapGUIActionEvent::binding(exports, worker);
		WrapGUIKeyEvent::binding(exports, worker);
		WrapGUIClickEvent::binding(exports, worker);
		WrapGUIMouseEvent::binding(exports, worker);
		WrapGUITouchEvent::binding(exports, worker);
		WrapGUIFocusMoveEvent::binding(exports, worker);
		WrapGUIHighlightedEvent::binding(exports, worker);
	}
};

JS_REG_MODULE(_event, BindingNativeEvent)
JS_END
