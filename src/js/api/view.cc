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

#include "./ui.h"
#include "../../ui/window.h"
#include "../../ui/action/action.h"
#include "../../ui/css/css.h"

namespace qk { namespace js {

	template<class Event, class Self>
	static void addEventListener_Static(
		Wobj<Self>* wrap, const UIEventName* type, cString& func, int id, JsConverter *cData
	)
	{
		auto f = [wrap, func, cData](typename Self::EventType& e) {
			auto worker = wrap->worker();
			Js_Handle_Scope(); // Callback Scope
			// arg event
			auto ev = WrapObject::wrap(static_cast<Event*>(&e), Js_Typeid(Event));
			if (cData) {
				// cData->as(worker, e.data());
				ev->set(worker->strs()->_data(), cData->cast(worker, e.data()));
			}
			JSValue* args[2] = { ev->that(), worker->newInstance(true) };

			// Qk_DEBUG("addEventListener_Static, %s, EventType: %s", *func, *e.name());
			// call js trigger func
			JSValue* r = wrap->call( func, 2, args );
		};

		Self* self = wrap->self();
		self->add_event_listener(*type, f, id);
	}

	bool WrapViewObject::addEventListener(cString& name_, cString& func, int id) {
		const UIEventName *name;
		if ( UIEventNames.get(name_, name) ) {
			return false;
		}
		auto wrap = static_cast<Wobj<View>*>(static_cast<WrapObject*>(this));
		JsConverter* converter = nullptr;

		switch ( kTypesMask_UIEventFlags & name->flag() ) {
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

	bool WrapViewObject::removeEventListener(cString& name_, int id) {
		const UIEventName *name;
		if ( UIEventNames.get(name_, name) ) {
			return false;
		}
		Qk_DEBUG("removeEventListener, name:%s, id:%d", *name_, id);

		auto wrap = reinterpret_cast<Wobj<View>*>(this);
		wrap->self()->remove_event_listener(*name, id); // off event listener
		return true;
	}
	
	void WrapViewObject::init() {
		that()->defineOwnProperty(worker(), worker()->strs()->window(),
			wrap<Window>(self<View>()->window())->that(), JSObject::ReadOnly | JSObject::DontDelete
		);
	}

	Window* WrapViewObject::checkNewView(FunctionArgs args) {
		Js_Worker(args);
		if (!args.length() || !Js_IsWindow(args[0])) {
			Js_Throw("\
				Call view constructor() error, param window object no match. \n\
				@constructor(window) \n\
				@param window {Window} \n\
			"), nullptr;
		}
		return wrap<Window>(args[0])->self();
	}

	class WrapView: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(View, 0, {
				Js_NewView(View);
			});

			Js_Set_Class_Accessor_Get(cssclass, {
				Js_Self(View);
				Js_Return(self->cssclass());
			});

			Js_Set_Class_Accessor_Get(parent, {
				Js_Self(View);
				Js_Return(self->parent());
			});

			Js_Set_Class_Accessor_Get(prev, {
				Js_Self(View);
				Js_Return( self->prev() );
			});

			Js_Set_Class_Accessor_Get(next, {
				Js_Self(View);
				Js_Return( self->next() );
			});

			Js_Set_Class_Accessor_Get(first, {
				Js_Self(View);
				Js_Return( self->first() );
			});

			Js_Set_Class_Accessor_Get(last, {
				Js_Self(View);
				Js_Return( self->last() );
			});

			Js_Set_Class_Accessor(action_, {
				Js_Self(View);
				Js_Return( self->action() );
			}, {
				if (val->isNull()) {
					Js_Self(View);
					self->set_action(nullptr);
				} else {
					if (!worker->template instanceOf<Action>(val))
						Js_Throw("@prop set_action {Action}\n");
					Js_Self(View);
					self->set_action(wrap<Action>(val)->self());
				}
			});

			Js_Set_Class_Accessor_Get(matrix, {
				Js_Self(View);
				Js_Return( args.worker()->types()->jsvalue(self->matrix()) );
			});

			Js_Set_Class_Accessor_Get(level, {
				Js_Self(View);
				Js_Return( self->level() );
			});

			Js_Set_Class_Accessor(opacity, {
				Js_Self(View);
				Js_Return( self->opacity() );
			}, {
				if (!val->isNumber())
					Js_Throw("@prop View.set_opacity {float}\n");
				Js_Self(View);
				self->set_opacity(val->toFloatValue(worker).unsafe());
			});

			Js_Set_Class_Accessor(cursor, {
				Js_Self(View);
				Js_Return( uint32_t(self->cursor()) );
			}, {
				Js_Parse_Type(CursorStyle, val, "@prop View.set_cursor = %s");
				Js_Self(View);
				self->set_cursor(out);
			});

			Js_Set_Class_Accessor(visible, {
				Js_Self(View);
				Js_Return( self->visible() );
			}, {
				Js_Self(View);
				self->set_visible(val->toBooleanValue(worker));
			});

			Js_Set_Class_Accessor_Get(visibleRegion, {
				Js_Self(View);
				Js_Return( self->visible_region() );
			});

			Js_Set_Class_Accessor(receive, {
				Js_Self(View);
				Js_Return( self->receive() );
			}, {
				Js_Self(View);
				self->set_receive(val->toBooleanValue(worker));
			});

			Js_Set_Class_Accessor_Get(isFocus, {
				Js_Self(View);
				Js_Return( self->is_focus() );
			});

			Js_Set_Class_Method(focus, {
				Js_Self(View);
				self->focus();
			});

			Js_Set_Class_Method(blur, {
				Js_Self(View);
				self->blur();
			});

			Js_Set_Class_Method(isSelfChild, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.is_self_child(child)\n"
						"@param child {View}\n"
						"@return {bool}\n"
					);
				}
				Js_Self(View);
				auto v = wrap<View>(args[0])->self();
				Js_Return( self->is_self_child(v) );
			});

			Js_Set_Class_Method(before, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.before(prev)\n"
						"@param prev {View}\n"
					);
				}
				Js_Self(View);
				auto v = wrap<View>(args[0])->self();
				Js_Try_Catch({ self->before(v); }, Error);
			});

			Js_Set_Class_Method(after, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.after(next)\n"
						"@param next {View}\n"
					);
				}
				Js_Self(View);
				auto v = wrap<View>(args[0])->self();
				Js_Try_Catch({ self->after(v); }, Error);
			});

			Js_Set_Class_Method(prepend, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.prepend(child)\n"
						"@param child {View}\n"
					);
				}
				Js_Self(View);
				auto v = wrap<View>(args[0])->self();
				Js_Try_Catch({ self->prepend(v); }, Error);
			});

			Js_Set_Class_Method(append, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.append(child)\n"
						"@param child {View}\n"
					);
				}
				Js_Self(View);
				auto v = wrap<View>(args[0])->self();
				Js_Try_Catch({ self->append(v); }, Error);
			});

			Js_Set_Class_Method(remove, {
				Js_Self(View);
				self->remove();
			});

			Js_Set_Class_Method(removeAllChild, {
				Js_Self(View);
				self->remove_all_child();
			});

			Js_Set_Class_Accessor_Get(layoutWeight, {
				Js_Self(View);
				Js_Return( self->layout_weight() );
			});

			Js_Set_Class_Accessor_Get(layoutAlign, {
				Js_Self(View);
				Js_Return( uint32_t(self->layout_align()) );
			});

			Js_Set_Class_Accessor_Get(isClip, {
				Js_Self(View);
				Js_Return( self->is_clip() );
			});

			Js_Set_Class_Accessor_Get(viewType, {
				Js_Self(View);
				Js_Return( self->viewType() );
			});

			// -----------------------------------------------------------------------------
			// @safe Rt
			Js_Set_Class_Method(overlapTest, {
				if (!args.length()) {
					Js_Throw(
						"@method View.overlapTest(point)\n"
						"@param point {Vec2}\n"
						"@return bool\n"
					);
				}
				Js_Parse_Type(Vec2, args[0], "@method View.overlapTest(point = %s)");
				Js_Self(View);
				Js_Return(self->overlap_test(out));
			});
			Js_Set_Class_Accessor_Get(position, {
				Js_Self(View);
				Js_Return( args.worker()->types()->jsvalue(self->position()) );
			});
			Js_Set_Class_Accessor_Get(layoutOffset, {
				Js_Self(View);
				Js_Return( args.worker()->types()->jsvalue(self->layout_offset()) );
			});
			Js_Set_Class_Accessor_Get(center, {
				Js_Self(View);
				Js_Return( args.worker()->types()->jsvalue(self->center()) );
			});
			// -----------------------------------------------------------------------------
			cls->exports("View", exports);
		}
	};

	void binding_view(JSObject* exports, Worker* worker) {
		WrapView::binding(exports, worker);
	}
} }
