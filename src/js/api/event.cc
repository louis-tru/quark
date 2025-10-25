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
#include "../../ui/view/spine.h"
#include "../../../out/native-inl-js.h"
#include <spine/EventData.h>

namespace qk { namespace js {
	typedef Event<> NativeEvent;

	struct MixNativeEvent: MixObject {
		typedef Event<> Type;

		static void binding(JSObject* exports, Worker* worker) {
			auto Event = exports->get(worker, worker->newStringOneByte("Event"))
				->cast<JSFunction>();

			Js_New_Class(NativeEvent, Js_Typeid(NativeEvent), Event, _Js_Fun(,{
				Js_Throw("Access forbidden.");
			}));

			Js_Class_Accessor_Get(sender, {
				Js_Return(self->sender());
			});

			Js_Class_Accessor(returnValue, {
				Js_Return( self->return_value );
			}, {
				if ( !val->isInt32() )
					Js_Throw("Bad argument.");
				auto num = val->template cast<JSInt32>()->value();
				self->return_value = num;
			});
			// cls->exports("NativeEvent", exports);
		}
	};

	struct MixUIEvent: MixObject {
		typedef UIEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIEvent, NativeEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Accessor_Get(timestamp, {
				Js_Return( self->timestamp() );
			});

			Js_Class_Accessor_Get(origin, {
				Js_Return(self->origin());
			});

			Js_Class_Accessor_Get(isDefault, {
				Js_ReturnBool( self->is_default() );
			});

			Js_Class_Accessor_Get(isBubble, {
				Js_ReturnBool( self->is_bubble() );
			});

			Js_Class_Method(cancelDefault, {
				self->cancel_default();
			});

			Js_Class_Method(cancelBubble, {
				self->cancel_bubble();
			});

			cls->exports("UIEvent", exports);
		}
	};

	struct MixActionEvent: MixObject {
		typedef ActionEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(ActionEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Accessor_Get(action, {
				Js_Return( self->action() );
			});
			Js_Class_Accessor_Get(delay, {
				Js_Return( self->delay() / 1000 );
			});
			Js_Class_Accessor_Get(frame, {
				Js_Return( self->frame() );
			});
			Js_Class_Accessor_Get(looped, {
				Js_Return( self->looped() );
			});
			cls->exports("ActionEvent", exports);
		}
	};

	struct MixKeyEvent: MixObject {
		typedef KeyEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(KeyEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});

			Js_Class_Accessor_Get(keycode, {
				Js_Return( self->keycode() );
			});
			Js_Class_Accessor_Get(repeat, {
				Js_Return( self->repeat() );
			});
			Js_Class_Accessor_Get(shift, {
				Js_ReturnBool( self->shift() );
			});
			Js_Class_Accessor_Get(ctrl, {
				Js_ReturnBool( self->ctrl() );
			});
			Js_Class_Accessor_Get(alt, {
				Js_ReturnBool( self->alt() );
			});
			Js_Class_Accessor_Get(command, {
				Js_ReturnBool( self->command() );
			});
			Js_Class_Accessor_Get(capsLock, {
				Js_ReturnBool( self->caps_lock() );
			});
			Js_Class_Accessor_Get(device, {
				Js_Return( self->device() );
			});
			Js_Class_Accessor_Get(source, {
				Js_Return( self->source() );
			});
			Js_Class_Accessor(nextFocus, {
				Js_Return( self->next_focus() );
			}, {
				View* view = nullptr;
				if ( Js_IsView(val) ) {
					view = MixObject::mix<View>(val)->self();
				} else if ( !val->isNull() ) {
					Js_Throw("Bad argument.");
				}
				self->set_next_focus(view);
			});
			cls->exports("KeyEvent", exports);
		}
	};

	struct MixClickEvent: MixObject {
		typedef ClickEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(ClickEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Accessor_Get(position, {
				Js_Return( worker->types()->jsvalue(self->position()) );
			});
			Js_Class_Accessor_Get(count, {
				Js_Return( self->count() );
			});
			Js_Class_Accessor_Get(type, {
				Js_Return( int(self->type()) );
			});
			cls->exports("ClickEvent", exports);
		}
	};

	struct MixHighlightedEvent: MixObject {
		typedef HighlightedEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(HighlightedEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Accessor_Get(status, {
				Js_Return(self->status());
			});
			cls->exports("HighlightedEvent", exports);
		}
	};

	struct MixMouseEvent: MixObject {
		typedef MouseEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(MouseEvent, KeyEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Accessor_Get(position, {
				Js_Return( worker->types()->jsvalue(self->position()) );
			});
			cls->exports("MouseEvent", exports);
		}
	};

	struct MixTouchEvent: MixObject {
		typedef TouchEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(TouchEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});

			Js_Class_Accessor_Get(changedTouches, {
				auto r = mix->handle()->get(worker, worker->strs()->_change_touches());
				if (!r) return; // js error
				if (!r->isArray()) {
					r = worker->types()->jsvalue(mix->self()->changed_touches());
					mix->handle()->set(worker, worker->strs()->_change_touches(), r);
				}
				Js_Return(r);
			});

			cls->exports("TouchEvent", exports);
		}
	};

	struct MixSpineEvent: MixObject {
		typedef SpineEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(SpineEvent, UIEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Accessor_Get(type, {
				Js_Return(self->type());
			});
			Js_Class_Accessor_Get(trackIndex, {
				Js_Return(self->trackIndex());
			});
			Js_Class_Accessor_Get(animationName, {
				Js_Return(self->animationName());
			});
			Js_Class_Accessor_Get(trackTime, {
				Js_Return(self->trackTime());
			});
			cls->exports("SpineEvent", exports);
		}
	};

	struct MixSpineKeyEvent: MixObject {
		typedef SpineKeyEvent Type;
		static void binding(JSObject* exports, Worker* worker) {

			Js_Define_Class(SpineKeyEvent, SpineEvent, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Accessor_Get(staticData, {
				auto _staticData = worker->strs()->_staticData();
				auto data = mix->handle()->template get<JSObject>(worker, _staticData);
				if (data->isUndefined()) {
					auto sData = self->static_data();
					if (!sData)
						Js_Return_Null();
					Js_Handle_Scope();
					data = worker->newObject();
					data->setFor(worker, "name", String(sData->getName().buffer()));
					data->setFor(worker, "intValue", sData->getIntValue());
					data->setFor(worker, "floatValue", sData->getFloatValue());
					data->setFor(worker, "stringValue", String(sData->getStringValue().buffer()));
					data->setFor(worker, "audioPath", String(sData->getAudioPath().buffer()));
					data->setFor(worker, "volume", sData->getVolume());
					data->setFor(worker, "balance", sData->getBalance());
					mix->handle()->set(worker, _staticData, data);
				}
				Js_Return(data);
			});
			Js_Class_Accessor_Get(time, {
				Js_Return( self->time() );
			});
			Js_Class_Accessor_Get(intValue, {
				Js_Return( self->int_value() );
			});
			Js_Class_Accessor_Get(float_value, {
				Js_Return( self->float_value() );
			});
			Js_Class_Accessor_Get(string_value, {
				Js_Return( self->string_value() );
			});
			Js_Class_Accessor_Get(volume, {
				Js_Return( self->volume() );
			});
			Js_Class_Accessor_Get(balance, {
				Js_Return( self->balance() );
			});
			cls->exports("SpineKeyEvent", exports);
		}
	};

	struct MixEvent {
		static void binding(JSObject* exports, Worker* worker) {
			worker->runNativeScript((Char*)
				native_js::INL_native_js_code__event_,
				native_js::INL_native_js_code__event_count_, "_event.js", exports);
			MixNativeEvent::binding(exports, worker);
			MixUIEvent::binding(exports, worker);
			MixActionEvent::binding(exports, worker);
			MixKeyEvent::binding(exports, worker);
			MixClickEvent::binding(exports, worker);
			MixMouseEvent::binding(exports, worker);
			MixTouchEvent::binding(exports, worker);
			MixHighlightedEvent::binding(exports, worker);
			MixSpineEvent::binding(exports, worker);
			MixSpineKeyEvent::binding(exports, worker);
		}
	};

	Js_Module(_event, MixEvent)
} }
