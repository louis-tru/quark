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

#include "../js_.h"
#include "./ui.h"
#include "../../ui/action/action.h"
#include "../../ui/view/view.h"
#include "../../ui/event.h"
#include "../../../out/native-inl-js.h"

namespace qk { namespace js {
	typedef Event<> NativeEvent;

	struct WrapNativeEvent: WrapObject {
		typedef Event<> Type;

		static void binding(JSObject* exports, Worker* worker) {
			auto Event = exports->get(worker, worker->newStringOneByte("Event"))
				->as<JSFunction>();

			Js_New_Class(NativeEvent, Js_Typeid(NativeEvent), Event, _Js_Fun({
				Js_Throw("Access forbidden.");
			}));

			Js_Set_Class_Accessor_Get(sender, {
				Js_Self(Type);
				Js_Return(self->sender());
			});

			Js_Set_Class_Accessor(returnValue, {
				Js_Self(Type);
				Js_Return( self->return_value );
			}, {
				if ( !val->isInt32() )
					Js_Throw("Bad argument.");
				Js_Self(Type);
				self->return_value = val->toInt32Value(worker).unsafe();
			});

			cls->exports("NativeEvent", exports);
		}
	};

	struct WrapUIEvent: WrapObject {
		typedef UIEvent Type;

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIEvent, NativeEvent, {
				Js_Throw("Access forbidden.");
			});

			Js_Set_Class_Accessor_Get(origin, {
				Js_Self(Type);
				Js_Return(self->origin());
			});

			Js_Set_Class_Accessor_Get(timestamp, {
				Js_Self(Type);
				Js_Return( self->timestamp() );
			});

			Js_Set_Class_Accessor_Get(isDefault, {
				Js_Self(Type);
				Js_ReturnBool( self->is_default() );
			});

			Js_Set_Class_Accessor_Get(isBubble, {
				Js_Self(Type);
				Js_ReturnBool( self->is_bubble() );
			});

			Js_Set_Class_Method(cancelDefault, {
				Js_Self(Type);
				self->cancel_default();
			});

			Js_Set_Class_Method(cancelBubble, {
				Js_Self(Type);
				self->cancel_bubble();
			});

			cls->exports("UIEvent", exports);
		}
	};

	struct WrapActionEvent: WrapObject {
		typedef ActionEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(ActionEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Set_Class_Accessor_Get(action, {
				Js_Self(ActionEvent);
				Js_Return( self->action() );
			});
			Js_Set_Class_Accessor_Get(delay, {
				Js_Self(ActionEvent);
				Js_Return( self->delay() / 1000 );
			});
			Js_Set_Class_Accessor_Get(frame, {
				Js_Self(ActionEvent);
				Js_Return( self->frame() );
			});
			Js_Set_Class_Accessor_Get(loop, {
				Js_Self(ActionEvent);
				Js_Return( self->loop() );
			});
			cls->exports("ActionEvent", exports);
		}
	};

	struct WrapKeyEvent: WrapObject {
		typedef KeyEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(KeyEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});

			Js_Set_Class_Accessor_Get(keycode, {
				Js_Self(Type);
				Js_Return( self->keycode() );
			});
			Js_Set_Class_Accessor_Get(repeat, {
				Js_Self(Type);
				Js_Return( self->repeat() );
			});
			Js_Set_Class_Accessor_Get(shift, {
				Js_Self(Type);
				Js_ReturnBool( self->shift() );
			});
			Js_Set_Class_Accessor_Get(ctrl, {
				Js_Self(Type);
				Js_ReturnBool( self->ctrl() );
			});
			Js_Set_Class_Accessor_Get(alt, {
				Js_Self(Type);
				Js_ReturnBool( self->alt() );
			});
			Js_Set_Class_Accessor_Get(command, {
				Js_Self(Type);
				Js_ReturnBool( self->command() );
			});
			Js_Set_Class_Accessor_Get(capsLock, {
				Js_Self(Type);
				Js_ReturnBool( self->caps_lock() );
			});
			Js_Set_Class_Accessor_Get(device, {
				Js_Self(Type);
				Js_Return( self->device() );
			});
			Js_Set_Class_Accessor_Get(source, {
				Js_Self(Type);
				Js_Return( self->source() );
			});
			Js_Set_Class_Accessor(nextFocus, {
				Js_Self(Type);
				Js_Return( self->next_focus() );
			}, {
				Js_Self(Type);
				View* view = nullptr;
				if ( Js_IsView(val) ) {
					view = wrap<View>(val)->self();
				} else if ( !val->isNull() ) {
					Js_Throw("Bad argument.");
				}
				self->set_next_focus(view);
			});
			cls->exports("KeyEvent", exports);
		}
	};

	struct WrapClickEvent: WrapObject {
		typedef ClickEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(ClickEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Set_Class_Accessor_Get(x, {
				Js_Self(ClickEvent);
				Js_Return( self->x() );
			});
			Js_Set_Class_Accessor_Get(y, {
				Js_Self(ClickEvent);
				Js_Return( self->y() );
			});
			Js_Set_Class_Accessor_Get(count, {
				Js_Self(ClickEvent);
				Js_Return( self->count() );
			});
			Js_Set_Class_Accessor_Get(type, {
				Js_Self(ClickEvent);
				Js_Return( int(self->type()) );
			});
			cls->exports("ClickEvent", exports);
		}
	};

	struct WrapHighlightedEvent: WrapObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(HighlightedEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Set_Class_Accessor_Get(status, {
				Js_Self(HighlightedEvent);
				Js_Return(self->status());
			});
			cls->exports("HighlightedEvent", exports);
		}
	};

	struct WrapMouseEvent: WrapObject {
		typedef MouseEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(MouseEvent, KeyEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Set_Class_Accessor_Get(x, {
				Js_Self(MouseEvent);
				Js_Return( self->x() );
			});
			Js_Set_Class_Accessor_Get(y, {
				Js_Self(MouseEvent);
				Js_Return( self->y() );
			});
			cls->exports("MouseEvent", exports);
		}
	};

	struct WrapTouchEvent: WrapObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(TouchEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});

			Js_Set_Class_Accessor_Get(changedTouches, {
				Js_Wrap(TouchEvent);

				auto r = wrap->get(worker->strs()->_change_touches());
				if (!r) return; // js error
				if (!r->isArray()) {
					r = worker->types()->jsvalue(wrap->self()->changed_touches());
					wrap->set(worker->strs()->_change_touches(), r);
				}
				Js_Return(r);
			});

			cls->exports("TouchEvent", exports);
		}
	};

	struct WrapEvent {
		static void binding(JSObject* exports, Worker* worker) {
			worker->runNativeScript(WeakBuffer((Char*)
				native_js::INL_native_js_code__event_,
				native_js::INL_native_js_code__event_count_).buffer(), "_event.js", exports);
			WrapNativeEvent::binding(exports, worker);
			WrapUIEvent::binding(exports, worker);
			WrapActionEvent::binding(exports, worker);
			WrapKeyEvent::binding(exports, worker);
			WrapClickEvent::binding(exports, worker);
			WrapMouseEvent::binding(exports, worker);
			WrapTouchEvent::binding(exports, worker);
			WrapHighlightedEvent::binding(exports, worker);
		}
	};

	Js_Set_Module(_event, WrapEvent)
} }
