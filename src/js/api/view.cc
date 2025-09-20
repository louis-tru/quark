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
#include "../../ui/view/matrix.h"

namespace qk { namespace js {

	NotificationBasic* MixViewObject::asNotificationBasic() {
		return static_cast<View*>(self());
	}

	Window* MixViewObject::checkNewView(FunctionArgs args) {
		Js_Worker(args);
		if (!args.length() || !Js_IsWindow(args[0])) {
			Js_Throw("\
				Call view constructor() error, param window object no match. \n\
				@constructor(window) \n\
				@param window {Window} \n\
			"), nullptr;
		}
		return mix<Window>(args[0])->self();
	}

	class MixView: public MixViewObject {
	public:
		typedef View Type;

		static void getWindow(Worker *worker, PropertyArgs args) {
			Js_Self(View);
			auto win = self->window();
			// TODO: win pre safe check ..
			Js_Return( win );
		}
	
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(View, 0, {
				Js_NewView(View);
			});

			Js_Class_Accessor_Get(window, {
				getWindow(worker, args);
			});

			Js_Class_Accessor_Get(cssclass, {
				Js_Return(self->cssclass());
			});

			Js_Class_Accessor_Get(parent, {
				Js_Return(self->parent());
			});

			Js_Class_Accessor_Get(prev, {
				Js_Return( self->prev() );
			});

			Js_Class_Accessor_Get(next, {
				Js_Return( self->next() );
			});

			Js_Class_Accessor_Get(first, {
				Js_Return( self->first() );
			});

			Js_Class_Accessor_Get(last, {
				Js_Return( self->last() );
			});

			Js_Class_Accessor(action_, {
				Js_Return( self->action() );
			}, {
				if (val->isNull()) {
					self->set_action(nullptr);
				} else {
					if (!worker->template instanceOf<Action>(val))
						Js_Throw("@prop set_action {Action}\n");
					self->set_action(MixObject::mix<Action>(val)->self());
				}
			});

			Js_Class_Accessor_Get(matrixView, {
				auto view = self->matrix_view();
				Js_Return( view ? view->host() : nullptr );
			});

			Js_Class_Accessor_Get(level, {
				Js_Return( self->level() );
			});

			Js_MixObject_Accessor(View, Color, color, color);
			Js_MixObject_Accessor(View, CascadeColor, cascade_color, cascadeColor);
			Js_MixObject_Accessor(View, CursorStyle, cursor, cursor);
			Js_MixObject_Accessor(View, float, opacity, opacity);
			Js_MixObject_Accessor(View, bool, visible, visible);
			Js_MixObject_Accessor(View, bool, test_visible_region, testVisibleRegion);

			Js_Class_Accessor_Get(visibleRegion, {
				Js_ReturnBool( self->visible_region() );
			});

			Js_MixObject_Accessor(View, bool, receive, receive);
			Js_MixObject_Accessor(View, bool, aa, aa);

			Js_Class_Accessor_Get(isFocus, {
				Js_ReturnBool( self->is_focus() );
			});

			Js_Class_Method(focus, {
				Js_ReturnBool( self->focus() );
			});

			Js_Class_Method(blur, {
				Js_ReturnBool( self->blur() );
			});

			Js_Class_Method(isSelfChild, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.is_self_child(child)\n"
						"@param child {View}\n"
						"@return {bool}\n"
					);
				}
				auto v = MixObject::mix<View>(args[0])->self();
				Js_ReturnBool( self->is_self_child(v) );
			});

			Js_Class_Method(before, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.before(prev)\n"
						"@param prev {View}\n"
					);
				}
				auto v = MixObject::mix<View>(args[0])->self();
				Js_Try_Catch({ self->before(v); }, Error);
			});

			Js_Class_Method(after, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.after(next)\n"
						"@param next {View}\n"
					);
				}
				auto v = MixObject::mix<View>(args[0])->self();
				Js_Try_Catch({ self->after(v); }, Error);
			});

			Js_Class_Method(prepend, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.prepend(child)\n"
						"@param child {View}\n"
					);
				}
				auto v = MixObject::mix<View>(args[0])->self();
				Js_Try_Catch({ self->prepend(v); }, Error);
			});

			Js_Class_Method(append, {
				if (!args.length() || !Js_IsView(args[0])) {
					Js_Throw(
						"@method View.append(child)\n"
						"@param child {View}\n"
					);
				}
				auto v = MixObject::mix<View>(args[0])->self();
				Js_Try_Catch({ self->append(v); }, Error);
			});

			Js_Class_Method(remove, {
				self->remove();
			});

			Js_Class_Method(removeAllChild, {
				self->remove_all_child();
			});

			Js_Class_Accessor_Get(layoutWeight, {
				Js_Return( worker->types()->jsvalue( self->layout_weight()) );
			});

			Js_Class_Accessor_Get(layoutAlign, {
				Js_Return( uint32_t(self->layout_align()) );
			});

			Js_Class_Accessor_Get(isClip, {
				Js_ReturnBool( self->is_clip());
			});

			Js_Class_Accessor_Get(viewType, {
				Js_Return( self->viewType() );
			});

			// -----------------------------------------------------------------------------
			// @thread Rt
			Js_Class_Method(overlapTest, {
				if (!args.length()) {
					Js_Throw(
						"@method View.overlapTest(point)\n"
						"@param point {Vec2}\n"
						"@return bool\n"
					);
				}
				Js_Parse_Type(Vec2, args[0], "@method View.overlapTest(point = %s)");
				Js_ReturnBool(self->overlap_test(out));
			});
			Js_Class_Accessor_Get(position, {
				Js_Return( worker->types()->jsvalue(self->position()) );
			});
			Js_Class_Accessor_Get(layoutOffset, {
				Js_Return( worker->types()->jsvalue(self->layout_offset()) );
			});
			Js_Class_Accessor_Get(layoutSize, {
				Js_Return( worker->types()->jsvalue(self->layout_size()) );
			});
			Js_Class_Accessor_Get(clientSize, {
				Js_Return( worker->types()->jsvalue(self->client_size()) );
			});
			// -----------------------------------------------------------------------------
			cls->exports("View", exports);
		}
	};

	void binding_view(JSObject* exports, Worker* worker) {
		MixView::binding(exports, worker);
	}
} }
