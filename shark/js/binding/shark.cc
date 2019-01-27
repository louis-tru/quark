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

#include "shark/js/shark.h"
#include "event-1.h"

/**
 * @ns shark::js
 */

JS_BEGIN

void WrapViewBase::destroy() {
	GUILock lock;
	delete this;
}

template<class T, class Self>
static void add_event_listener_1(Wrap<Self>* wrap, const GUIEventName& type, 
																 cString& func, int id, Cast* cast = nullptr) 
{
	auto f = [wrap, func, cast](typename Self::EventType& evt) {
		// if (worker()->is_terminate()) return;
		HandleScope scope(wrap->worker());
		// arg event
		Wrap<T>* ev = Wrap<T>::pack(static_cast<T*>(&evt), JS_TYPEID(T));

		if (cast)
			ev->set_private_data(cast); // set data cast func
		
		Local<JSValue> args[2] = { ev->that(), wrap->worker()->New(true) };
		// call js trigger func
		Local<JSValue> r = wrap->call( wrap->worker()->New(func,1), 2, args );
		
		// test:
		//if (r->IsNumber(worker)) {
		//  LOG("--------------number,%s", *r->ToStringValue(wrap->worker()));
		//} else {
		//  LOG("--------------string,%s", *r->ToStringValue(wrap->worker()));
		//}
	};
	
	Self* self = wrap->self();
	self->add_event_listener(type, f, id);
}

bool WrapViewBase::add_event_listener(cString& name_s, cString& func, int id) {
	auto i = GUI_EVENT_TABLE.find(name_s);
	if ( i.is_null() ) {
		return false;
	}
	
	GUIEventName name = i.value();
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
	
	switch ( name.category() ) {
		case GUI_EVENT_CATEGORY_CLICK:
			add_event_listener_1<GUIClickEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_KEYBOARD:
			add_event_listener_1<GUIKeyEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_MOUSE:
		 add_event_listener_1<GUIMouseEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_TOUCH:
			add_event_listener_1<GUITouchEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_HIGHLIGHTED:
			add_event_listener_1<GUIHighlightedEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_ACTION:
			add_event_listener_1<GUIActionEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_FOCUS_MOVE:
			add_event_listener_1<GUIFocusMoveEvent>(wrap, name, func, id); break;
		case GUI_EVENT_CATEGORY_ERROR:
			add_event_listener_1<GUIEvent>(wrap, name, func, id, Cast::entity<Error>()); break;
		case GUI_EVENT_CATEGORY_FLOAT:
			add_event_listener_1<GUIEvent>(wrap, name, func, id, Cast::entity<Float>()); break;
		case GUI_EVENT_CATEGORY_UINT64:
			add_event_listener_1<GUIEvent>(wrap, name, func, id, Cast::entity<Uint64>()); break;
		case GUI_EVENT_CATEGORY_DEFAULT:
			add_event_listener_1<GUIEvent>(wrap, name, func, id); break;
		default:
			return false;
	}
	return true;
}

bool WrapViewBase::remove_event_listener(cString& name, int id) {
	auto i = GUI_EVENT_TABLE.find(name);
	if ( i.is_null() ) {
		return false;
	}
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
	wrap->self()->remove_event_listener(i.value(), id); // off event listener
	return true;
}

void binding_app(Local<JSObject> exports, Worker* worker);
void binding_display(Local<JSObject> exports, Worker* worker);
void binding_view(Local<JSObject> exports, Worker* worker);
void binding_sprite(Local<JSObject> exports, Worker* worker);
void binding_layout(Local<JSObject> exports, Worker* worker);
void binding_box(Local<JSObject> exports, Worker* worker);
void binding_div(Local<JSObject> exports, Worker* worker);
void binding_hybrid(Local<JSObject> exports, Worker* worker);
void binding_span(Local<JSObject> exports, Worker* worker);
void binding_text_node(Local<JSObject> exports, Worker* worker);
void binding_image(Local<JSObject> exports, Worker* worker);
void binding_indep_div(Local<JSObject> exports, Worker* worker);
void binding_root(Local<JSObject> exports, Worker* worker);
void binding_label(Local<JSObject> exports, Worker* worker);
void binding_limit(Local<JSObject> exports, Worker* worker);
void binding_action(Local<JSObject> exports, Worker* worker);
void binding_button(Local<JSObject> exports, Worker* worker);
void binding_panel(Local<JSObject> exports, Worker* worker);
void binding_scroll(Local<JSObject> exports, Worker* worker);
void binding_text(Local<JSObject> exports, Worker* worker);
void binding_input(Local<JSObject> exports, Worker* worker);

/**
 * @class NativeShark
 */
class NativeShark {
	
	static void lock(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsFunction(worker)) {
			JS_THROW_ERR("Bad argument");
		}
		GUILock lock;
		Local<JSValue> r = args[0].To<JSFunction>()->Call(worker);
		if (!r.IsEmpty()) {
			JS_RETURN(r);
		}
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->binding_module("shark_value");	XX_DEBUG("binding shark_value ok");
		worker->binding_module("shark_event");	XX_DEBUG("binding shark_event ok");
		binding_app(exports, worker); 			XX_DEBUG("binding app ok");
		binding_display(exports, worker); 	XX_DEBUG("binding display ok");
		binding_view(exports, worker); 			XX_DEBUG("binding view ok");
		binding_sprite(exports, worker); 		XX_DEBUG("binding sprite ok");
		binding_layout(exports, worker); 		XX_DEBUG("binding layout ok");
		binding_box(exports, worker); 			XX_DEBUG("binding box ok");
		binding_div(exports, worker); 			XX_DEBUG("binding div ok");
		binding_panel(exports, worker); 		XX_DEBUG("binding panel ok");
		binding_hybrid(exports, worker); 		XX_DEBUG("binding hybrid ok");
		binding_span(exports, worker); 			XX_DEBUG("binding span ok");
		binding_text_node(exports, worker); XX_DEBUG("binding text_node ok");
		binding_image(exports, worker); 		XX_DEBUG("binding image ok");
		binding_indep_div(exports, worker); XX_DEBUG("binding indep_div ok");
		binding_root(exports, worker); 			XX_DEBUG("binding root ok");
		binding_label(exports, worker); 		XX_DEBUG("binding label ok");
		binding_limit(exports, worker); 		XX_DEBUG("binding limit ok");
		binding_scroll(exports, worker); 		XX_DEBUG("binding scroll ok");
		binding_text(exports, worker); 			XX_DEBUG("binding text ok");
		binding_button(exports, worker); 		XX_DEBUG("binding button ok");
		binding_input(exports, worker); 		XX_DEBUG("binding input ok");
		JS_SET_METHOD(lock, lock); 					XX_DEBUG("binding lock ok");
	}
};

JS_REG_MODULE(shark, NativeShark);
JS_END
