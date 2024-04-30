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

#include "./view.h"
#include "../types.h"
#include "../../ui/action/action.h"
#include "../../ui/css/css.h"
#include "../../ui/view/button.h"

namespace qk { namespace js {

	template<class Event, class Self>
	static void addEventListener_Static(
		Wrap<Self>* wrap, const UIEventName* type, cString& func, int id, JsConverter *cData
	)
	{
		auto f = [wrap, func, cData](typename Self::EventType& evt) {
			auto worker = wrap->worker();
			Js_Callback_Scope();

			// arg event
			auto ev = WrapObject::wrap(static_cast<Event*>(&evt), Js_Typeid(Event));
			if (cData) 
				ev->setExternalData(cData); // set data cast func
			JSValue* args[2] = { ev->that(), worker->newInstance(true) };

			Qk_DEBUG("addEventListener_Static, %s, EventType: %s", *func, *evt.name());

			// call js trigger func
			JSValue* r = wrap->call( worker->newInstance(func, true), 2, args );
		};

		Self* self = wrap->self();
		self->add_event_listener(*type, f, id);
	}

	bool WrapView_Event::addEventListener(cString& name_, cString& func, int id)
	{
		const UIEventName *name;
		if ( UIEventNames.get(name_, name) ) {
			return false;
		}
		auto wrap = static_cast<Wrap<View>*>(static_cast<WrapObject*>(this));
		JsConverter* converter = nullptr;

		switch ( kTypes_UIEventFlags & name->flag() ) {
			case kError_UIEventFlags: converter = JsConverter::Instance<Error>(); break;
			case kFloat32_UIEventFlags: converter = JsConverter::Instance<Float32>(); break;
			case kUint64_UIEventFlags: converter = JsConverter::Instance<Uint64>(); break;
		}

		switch ( name->category() ) {
			case kClick_UIEventCategory:
				addEventListener_Static<ClickEvent>(wrap, name, func, id, converter); break;
			case kKeyboard_UIEventCategory:
				addEventListener_Static<KeyEvent>(wrap, name, func, id, converter); break;
			case kMouse_UIEventCategory:
			addEventListener_Static<MouseEvent>(wrap, name, func, id, converter); break;
			case kTouch_UIEventCategory:
				addEventListener_Static<TouchEvent>(wrap, name, func, id, converter); break;
			case kHighlighted_UIEventCategory:
				addEventListener_Static<HighlightedEvent>(wrap, name, func, id, converter); break;
			case kAction_UIEventCategory:
				addEventListener_Static<ActionEvent>(wrap, name, func, id, converter); break;
			default: // DEFAULT
				addEventListener_Static<UIEvent>(wrap, name, func, id, converter); break;
		}
		return true;
	}

	bool WrapView_Event::removeEventListener(cString& name_, int id) {
		const UIEventName *name;
		if ( UIEventNames.get(name_, name) ) {
			return false;
		}
		Qk_DEBUG("removeEventListener, name:%s, id:%d", *name_, id);
		
		auto wrap = reinterpret_cast<Wrap<View>*>(this);
		wrap->self()->remove_event_listener(*name, id); // off event listener
		return true;
	}

	class WrapView: public WrapView_Event {
	public:

		static void constructor(FunctionArgs args) {
			// New<WrapView>(args, new View(nullptr));
			// TODO ...
		}

		static void prepend(FunctionArgs args) {
			Js_Worker(args);
			if (args.length() < 1 || ! worker->instanceOf(args[0], kView_ViewType)) {
				Js_Throw(
					"* @method prepend(child)\n"
					"* @param child {View}\n"
				);
			}
			Js_Self(View);
			auto child = wrap<View>(args[0])->self();

			Js_Try_Catch({
				self->prepend(child);
			}, Error);
		}

		static void append(FunctionArgs args) {
			Js_Worker(args);
			if (args.length() < 1 || ! worker->instanceOf(args[0], kView_ViewType)) {
				Js_Throw(
					"* @method append(child)\n"
					"* @param child {View}\n"
				);
			}
			Js_Self(View);
			auto child = wrap<View>(args[0])->self();
			try { self->append(child); }
			catch (cError& err) { Js_Throw(err); }
		}

		static void append_text(FunctionArgs args) {
			Js_Worker(args);
			if ( args.length() < 1 ) {
				Js_Throw(
					"* @method appendText(text)\n"
					"* @param text {String}\n"
					"* @return {View}\n"
				);
			}
			Js_Self(View);
			View* view = nullptr;

			Js_Try_Catch({
				// TODO ...
				// view = self->appendText( args[0]->toString2Value(worker) );
			}, Error);

			if (view) {
				auto w = wrap<View>(view, view->viewType());
				Js_Return( w->that() );
			} else {
				Js_Return_Null();
			}
		}

		static void before(FunctionArgs args) {
			Js_Worker(args);
			if (args.length() < 1 || ! worker->instanceOf(args[0], kView_ViewType)) {
				Js_Throw(
					"* @method before(prev)\n"
					"* @param prev {View}\n"
				);
			}
			Js_Self(View);
			auto v = wrap<View>(args[0])->self();
			Js_Try_Catch({
				self->before(v);
			}, Error);
		}

		static void after(FunctionArgs args) {
			Js_Worker(args);
			if (args.length() < 1 || !worker->instanceOf(args[0], kView_ViewType)) {
				Js_Throw(
					"* @method after(next)\n"
					"* @param next {View}\n"
				);
			}
			Js_Self(View);
			auto v = wrap<View>(args[0])->self();
			Js_Try_Catch({
				self->after(v);
			}, Error);
		}

		static void remove(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			self->remove();
		}

		static void remove_all_child(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			self->remove_all_child();
		}

		static void focus(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->focus() );
		}

		static void blur(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->blur() );
		}

		static void layout_offset(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Vec2 rev = self->layout_offset();
			//Js_Return( worker->values()->New(rev) );
			// TODO ...
		}

		static void layout_offset_from(FunctionArgs args) {
			Js_Worker(args);
			View* target = nullptr;
			if ( args.length() > 0 && worker->instanceOf(args[0], kView_ViewType) ) {
				target = wrap<View>(args[0])->self();
			}
			Js_Self(View);
			// TODO ...
			//Vec2 rev = self->layout_offset_from(target);
			// Js_Return( worker->values()->New(rev) );
		}

		static void get_action(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			auto action = self->action();
			if ( action ) {
				Js_Return( wrap(action)->that() );
			} else {
				Js_Return_Null();
			}
		}

		static void set_action(FunctionArgs args) {
			Js_Worker(args);
			Action* action = nullptr;

			if ( args.length() > 0 ) {
				if (worker->instanceOf<Action>(args[0])) {
					action = wrap<Action>(args[0])->self();
				} else if ( !args[0]->isNull(worker) ) {
					Js_Throw(
										"* @method setAction([action])\n"
										"* @param [action=null] {Action}\n"
										);
				}
			}
			Js_Self(View);
			Js_Try_Catch({
				self->set_action(action);
			}, Error)
		}

		static void screen_rect(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			// Js_Return( worker->values()->New(self->screen_rect()) );
			// TODO ...
		}

		static void final_matrix(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			// Js_Return( worker->values()->New(self->final_matrix()) );
			// TODO ...
		}

		static void final_opacity(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			// Js_Return( self->final_opacity() );
			// TODO ...
		}

		static void position(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			//Js_Return( worker->values()->New(self->position()) );
			// TODO ...
		}

		static void overlap_test(FunctionArgs args) {
			Js_Worker(args);
			if ( args.length() < 1 ) {
				Js_Throw(
					"* @method overlapTest(point)\n"
					"* @param point {Vec2}\n"
					"* @return {bool}\n"
				);
			}
			// js_parse_value(Vec2, args[0], "View.overlapTest( %s )");
			// Js_Self(View);
			// Js_Return( self->overlap_test(out) );
			// TODO ...
		}

		static void add_class(FunctionArgs args) {
			Js_Worker(args);
			if ( args.length() < 1 || !args[0]->isString(worker) ) {
				Js_Throw(
					"* @method addClass(name)\n"
					"* @param name {String}\n"
				);
			}
			Js_Self(View);
			self->cssclass()->add( args[0]->toStringValue(worker) );
			// TODO ...
		}

		static void remove_class(FunctionArgs args) {
			Js_Worker(args);
			if ( args.length() < 1 || ! args[0]->isString(worker) ) {
				Js_Throw(
					"* @method removeClass(name)\n"
					"* @param name {String}\n"
				);
			}
			Js_Self(View);
			//self->remove_class( args[0]->toStringValue(worker) );
			// TODO ...
		}

		static void toggle_class(FunctionArgs args) {
			Js_Worker(args);
			if ( args.length() < 1 || ! args[0]->isString(worker) ) {
				Js_Throw(
					"* @method toggleClass(name)\n"
					"* @param name {String}\n"
				);
			}
			Js_Self(View);
			// self->toggle_class( args[0]->toStringValue(worker) );
			// TODO ...
		}
		
		// ----------------------------- get --------------------------------

		static void inner_text(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			// Js_Return( self->inner_text() );
			// TODO ...
		}

		static void parent(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			auto view = self->parent();
			if ( ! view) Js_Return_Null();
			auto w = wrap<View>(view, view->viewType());
			Js_Return( w->that() );
		}

		static void prev(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			auto view = self->prev();
			if ( ! view) Js_Return_Null();
			auto w = wrap<View>(view, view->viewType());
			Js_Return( w->that() );
		}

		static void next(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			auto view = self->next();
			if ( ! view) Js_Return_Null();
			auto w = wrap<View>(view, view->viewType());
			Js_Return( w->that() );
		}

		static void first(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			auto view = self->first();
			if ( ! view) Js_Return_Null();
			auto w = wrap<View>(view, view->viewType());
			Js_Return( w->that() );
		}

		static void last(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			auto view = self->last();
			if ( ! view) Js_Return_Null();
			auto w = wrap<View>(view, view->viewType());
			Js_Return( w->that() );
		}

		static void x(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->x() );
		}

		static void y(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->y() );
		}

		static void scale_x(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->scale_x() );
		}

		static void scale_y(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->scale_y() );
		}

		static void rotate_z(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->rotate_z() );
		}

		static void skew_x(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->skew_x() );
		}

		static void skew_y(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->skew_y() );
		}

		static void opacity(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->opacity() );
		}

		static void visible(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->visible() );
		}

		static void final_visible(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->final_visible() );
		}

		static void draw_visible(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->draw_visible() );
		}

		static void translate(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( worker->values()->New(self->translate()) );
		}

		static void scale(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( worker->values()->New(self->scale()) );
		}

		static void skew(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( worker->values()->New(self->skew()) );
		}

		static void origin_x(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->origin_x() );
		}

		static void origin_y(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->origin_y() );
		}

		static void origin(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( worker->values()->New(self->origin()) );
		}

		static void matrix(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( worker->values()->New(self->matrix()) );
		}

		static void level(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->level() );
		}

		static void need_draw(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->need_draw() );
		}

		static void receive(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->receive() );
		}

		static void is_focus(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->is_focus() );
		}

		static void view_type(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Js_Return( self->view_type() );
		}

		static void classs(JSString* name, PropertyArgs args) {
			Js_Worker(args);
			Js_Self(View);
			CSSViewClasss* classs = self->classs();
			if ( classs ) {
				// JSValue* rv = worker->NewObject();
				// rv.To<JSObject>()->Set(worker, worker->strs()->name(), worker->New(classs->name()) );
				Js_Return( classs->name() );
			} else {
				// Js_Return_Null();
				Js_Return( JSString::Empty(worker) );
			}
		}
		
		// ------------------------------------ set ----------------------------------

		static void set_x(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_x( value->ToNumberValue(worker) );
		}

		static void set_y(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_y( value->ToNumberValue(worker) );
		}

		static void set_scale_x(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_scale_x( value->ToNumberValue(worker) );
		}

		static void set_scale_y(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_scale_y( value->ToNumberValue(worker) );
		}

		static void set_rotate_z(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_rotate_z( value->ToNumberValue(worker) );
		}

		static void set_skew_x(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_skew_x( value->ToNumberValue(worker) );
		}

		static void set_skew_y(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_skew_y( value->ToNumberValue(worker) );
		}

		static void set_opacity(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_opacity( value->ToNumberValue(worker) );
		}

		static void set_translate(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			js_parse_value(Vec2, value, "View.translate = %s");
			Js_Self(View);
			self->set_translate( out );
		}

		static void set_scale(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			js_parse_value(Vec2, value, "View.scale = %s");
			Js_Self(View);
			self->set_scale( out );
		}

		static void set_skew(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			js_parse_value(Vec2, value, "View.skew = %s");
			Js_Self(View);
			self->set_skew( out );
		}

		static void set_origin_x(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_origin_x( value->ToNumberValue(worker) );
		}

		static void set_origin_y(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( ! value->IsNumber(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_origin_y( value->ToNumberValue(worker) );
		}

		static void set_origin(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			js_parse_value(Vec2, value, "View.origin = %s");
			Js_Self(View);
			self->set_origin( out );
		}

		static void set_visible(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			Js_Self(View);
			self->set_visible( value->ToBooleanValue(worker) );
		}

		static void set_need_draw(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			Js_Self(View);
			self->set_need_draw( value->ToBooleanValue(worker) );
		}

		static void set_receive(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			Js_Self(View);
			self->set_receive( value->ToBooleanValue(worker) );
		}

		static void set_is_focus(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			Js_Self(View);
			self->set_is_focus( value->ToBooleanValue(worker) );
		}

		static void set_class(JSString* name, JSValue* value, PropertySetArgs args) {
			Js_Worker(args);
			if ( !value->IsString(worker) ) {
				Js_Throw("Bad argument.");
			}
			Js_Self(View);
			self->set_class( value->toStringValue(worker) );
		}

		static void first_button(FunctionArgs args) {
			Js_Worker(args);
			Js_Self(View);
			Button* btn = self->first_button();
			if ( ! btn) Js_Return_Null();
			Wrap<Button>* wrap = Wrap<Button>::pack(btn, btn->view_type());
			Js_Return( wrap->that() );
		}

		static void has_child(FunctionArgs args) {
			Js_Worker(args);
			if ( args.length() == 0 ) {
				Js_Throw("Bad argument.");
			}
			if ( !worker->hasInstance(args[0], View::VIEW) ) {
				Js_Return_Null();
			}
			Js_Self(View);
			View* view = unpack<View>(args[0].To())->self();
			Js_Return( self->has_child(view) );
		}

		static void binding(JSObject* exports, Worker* worker) {
		//  #define SET_FIELD(enum, class, name) Js_Set_Property(enum, View::enum);
		// 	Qk_EACH_VIEWS(SET_FIELD)
		//  #undef SET_FIELD

			Js_New_Class(View, kView_ViewType, Local<JSClass>(), constructor, {
				Js_Set_Class_Method(prepend, prepend); // method
				Js_Set_Class_Method(append, append);
				Js_Set_Class_Method(appendText, append_text);
				Js_Set_Class_Method(before, before);
				Js_Set_Class_Method(after, after);
				Js_Set_Class_Method(remove, remove);
				Js_Set_Class_Method(removeAllChild, remove_all_child);
				Js_Set_Class_Method(focus, focus);
				Js_Set_Class_Method(blur, blur);
				Js_Set_Class_Method(layoutOffset, layout_offset);
				Js_Set_Class_Method(layoutOffsetFrom, layout_offset_from);
				Js_Set_Class_Method(getAction, get_action);
				Js_Set_Class_Method(_setAction, set_action);
				Js_Set_Class_Method(screenRect, screen_rect);
				Js_Set_Class_Method(finalMatrix, final_matrix);
				Js_Set_Class_Method(finalOpacity, final_opacity);
				Js_Set_Class_Method(position, position);
				Js_Set_Class_Method(overlapTest, overlap_test);
				Js_Set_Class_Method(addClass, add_class);
				Js_Set_Class_Method(removeClass, remove_class);
				Js_Set_Class_Method(toggleClass, toggle_class);
				Js_Set_Class_Method(firstButton, first_button);
				Js_Set_Class_Method(hasChild, has_child);
				Js_Set_Class_Accessor(innerText, inner_text); // property
				Js_Set_Class_Accessor(parent, parent);
				Js_Set_Class_Accessor(prev, prev);
				Js_Set_Class_Accessor(next, next);
				Js_Set_Class_Accessor(first, first);
				Js_Set_Class_Accessor(last, last);
				Js_Set_Class_Accessor(x, x, set_x);
				Js_Set_Class_Accessor(y, y, set_y);
				Js_Set_Class_Accessor(scaleX, scale_x, set_scale_x);
				Js_Set_Class_Accessor(scaleY, scale_y, set_scale_y);
				Js_Set_Class_Accessor(rotateZ, rotate_z, set_rotate_z);
				Js_Set_Class_Accessor(skewX, skew_x, set_skew_x);
				Js_Set_Class_Accessor(skewY, skew_y, set_skew_y);
				Js_Set_Class_Accessor(opacity, opacity, set_opacity);
				Js_Set_Class_Accessor(visible, visible, set_visible);
				Js_Set_Class_Accessor(finalVisible, final_visible);
				Js_Set_Class_Accessor(drawVisible, draw_visible);
				Js_Set_Class_Accessor(translate, translate, set_translate);
				Js_Set_Class_Accessor(scale, scale, set_scale);
				Js_Set_Class_Accessor(skew, skew, set_skew);
				Js_Set_Class_Accessor(originX, origin_x, set_origin_x);
				Js_Set_Class_Accessor(originY, origin_y, set_origin_y);
				Js_Set_Class_Accessor(origin, origin, set_origin);
				Js_Set_Class_Accessor(matrix, matrix);
				Js_Set_Class_Accessor(level, level);
				Js_Set_Class_Accessor(needDraw, need_draw, set_need_draw);
				Js_Set_Class_Accessor(receive, receive, set_receive);
				Js_Set_Class_Accessor(isFocus, is_focus, set_is_focus);
				Js_Set_Class_Accessor(viewType, view_type);
				Js_Set_Class_Accessor(class, classs, set_class);
			});
			cls->exports("View", exports);
		}
	};

	void binding_view(JSObject* exports, Worker* worker) {
		WrapView::binding(exports, worker);
	}

} }