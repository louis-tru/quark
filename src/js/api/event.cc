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
			auto Event = exports->get(worker, worker->newStringOneByte("Event"))->cast<JSFunction>();

			Js_New_Class(NativeEvent, Js_Typeid(NativeEvent), Event, _Js_Fun(,{
				Js_Throw("Access forbidden.");
			}));
			Js_Class_Accessor_Get(sender, { Js_Return(self->sender()); });

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
			Js_Define_Class(UIEvent, NativeEvent, { Js_Throw("Access forbidden.");});
			Js_MixObject_Acce_Get(UIEvent, int64_t, timestamp, timestamp);
			Js_Class_Accessor_Get(origin, { Js_Return(self->origin()); });
			Js_Class_Accessor_Get(isDefault, { Js_ReturnBool( self->is_default() ); });
			Js_Class_Accessor_Get(isBubble, { Js_ReturnBool( self->is_bubble() ); });
			Js_Class_Method(cancelDefault, { self->cancel_default(); });
			Js_Class_Method(cancelBubble, { self->cancel_bubble(); });
			cls->exports("UIEvent", exports);
		}
	};

	struct MixActionEvent: MixObject {
		typedef ActionEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(ActionEvent, UIEvent, { Js_Throw("Access forbidden."); });
			Js_Class_Accessor_Get(action, { Js_Return( self->action() ); });
			Js_Class_Accessor_Get(delay, { Js_Return( self->delay() / 1000 ); });
			Js_MixObject_Acce_Get(UIEvent, uint32_t, frame, frame);
			Js_MixObject_Acce_Get(UIEvent, uint32_t, looped, looped);
			cls->exports("ActionEvent", exports);
		}
	};

	struct MixKeyEvent: MixObject {
		typedef KeyEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(KeyEvent, UIEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(KeyEvent, int, keycode, keycode);
			Js_MixObject_Acce_Get(KeyEvent, uint32_t, repeat, repeat);
			Js_MixObject_Acce_Get(KeyEvent, bool, shift, shift);
			Js_MixObject_Acce_Get(KeyEvent, bool, ctrl, ctrl);
			Js_MixObject_Acce_Get(KeyEvent, bool, alt, alt);
			Js_MixObject_Acce_Get(KeyEvent, bool, command, command);
			Js_MixObject_Acce_Get(KeyEvent, bool, caps_lock, capsLock);
			Js_MixObject_Acce_Get(KeyEvent, uint32_t, device, device);
			Js_MixObject_Acce_Get(KeyEvent, uint32_t, source, source);
			// Js_MixObject_Acce_Get(KeyEvent, View*, next_focus, nextFocus);
			Js_Class_Accessor(nextFocus, {
				Js_Return(self->next_focus());
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
			Js_Define_Class(ClickEvent, KeyEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(ClickEvent, Vec2, position, position);
			Js_MixObject_Acce_Get(ClickEvent, uint32_t, count, count);
			Js_MixObject_Acce_Get(ClickEvent, int, type, type);
			cls->exports("ClickEvent", exports);
		}
	};

	struct MixUIStateEvent: MixObject {
		typedef UIStateEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(UIStateEvent, UIEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(UIStateEvent, int, state, state);
			cls->exports("UIStateEvent", exports);
		}
	};

	struct MixMouseEvent: MixObject {
		typedef MouseEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(MouseEvent, KeyEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(MouseEvent, Vec2, position, position);
			Js_MixObject_Acce_Get(MouseEvent, uint32_t, level, level);
			cls->exports("MouseEvent", exports);
		}
	};

	struct MixTouchEvent: MixObject {
		typedef TouchEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(TouchEvent, UIEvent, { Js_Throw("Access forbidden."); });

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
			Js_Define_Class(SpineEvent, UIEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(SpineEvent, int, type, type);
			Js_MixObject_Acce_Get(SpineEvent, int, trackIndex, trackIndex);
			Js_MixObject_Acce_Get(SpineEvent, String, animationName, animationName);
			Js_MixObject_Acce_Get(SpineEvent, float, trackTime, trackTime);
			cls->exports("SpineEvent", exports);
		}
	};

	struct MixSpineExtEvent: MixObject {
		typedef SpineExtEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(SpineExtEvent, SpineEvent, { Js_Throw("Access forbidden."); });
			Js_Class_Accessor_Get(staticData, {
				auto _staticData = worker->strs()->_staticData();
				auto data = mix->handle()->template get<JSObject>(worker, _staticData);
				if (data->isUndefined()) {
					auto sData = self->static_data();
					if (!sData)
						Js_Return_Null();
					Js_Handle_Scope();
					data = worker->newObject();
					data->set(worker, "name", String(sData->getName().buffer()));
					data->set(worker, "intValue", sData->getIntValue());
					data->set(worker, "floatValue", sData->getFloatValue());
					data->set(worker, "stringValue", String(sData->getStringValue().buffer()));
					data->set(worker, "audioPath", String(sData->getAudioPath().buffer()));
					data->set(worker, "volume", sData->getVolume());
					data->set(worker, "balance", sData->getBalance());
					mix->handle()->set(worker, _staticData, data);
				}
				Js_Return(data);
			});
			Js_MixObject_Acce_Get(SpineExtEvent,float, time, time);
			Js_MixObject_Acce_Get(SpineExtEvent, int, int_value, intValue);
			Js_MixObject_Acce_Get(SpineExtEvent, float, float_value, floatValue);
			Js_MixObject_Acce_Get(SpineExtEvent, String, string_value, stringValue);
			Js_MixObject_Acce_Get(SpineExtEvent, float, volume, volume);
			Js_MixObject_Acce_Get(SpineExtEvent, float, balance, balance);
			cls->exports("SpineExtEvent", exports);
		}
	};

	struct MixAgentStateEvent: MixObject {
		typedef AgentStateEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(AgentStateEvent, UIEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(AgentStateEvent, Vec2, velocity, velocity);
			Js_MixObject_Acce_Get(AgentStateEvent, Vec2, heading, heading);
			Js_MixObject_Acce_Get(AgentStateEvent, Vec2, target, target);
			Js_MixObject_Acce_Get(AgentStateEvent, bool, moving, moving);
			cls->exports("AgentStateEvent", exports);
		}
	};

	struct MixReachWaypointEvent: MixObject {
		typedef ReachWaypointEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(ReachWaypointEvent, AgentStateEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(ReachWaypointEvent, Vec2, toNext, toNext);
			Js_MixObject_Acce_Get(ReachWaypointEvent, uint32_t, waypointIndex, waypointIndex);
			cls->exports("ReachWaypointEvent", exports);
		}
	};

	struct MixAgentMovementEvent: MixObject {
		typedef AgentMovementEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(AgentMovementEvent, AgentStateEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(AgentMovementEvent, int, movementState, movementState);
			cls->exports("AgentMovementEvent", exports);
		}
	};

	struct MixDiscoveryAgentEvent: MixObject {
		typedef DiscoveryAgentEvent Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(DiscoveryAgentEvent, AgentStateEvent, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(DiscoveryAgentEvent, Vec2, location, location);
			Js_MixObject_Acce_Get(DiscoveryAgentEvent, Agent, agent, agent);
			Js_MixObject_Acce_Get(DiscoveryAgentEvent, uint32_t, level, level);
			Js_MixObject_Acce_Get(DiscoveryAgentEvent, bool, entering, entering);
			cls->exports("DiscoveryAgentEvent", exports);
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
			MixUIStateEvent::binding(exports, worker);
			MixSpineEvent::binding(exports, worker);
			MixSpineExtEvent::binding(exports, worker);
			MixAgentStateEvent::binding(exports, worker);
			MixReachWaypointEvent::binding(exports, worker);
			MixAgentMovementEvent::binding(exports, worker);
			MixDiscoveryAgentEvent::binding(exports, worker);
		}
	};

	Js_Module(_event, MixEvent)
} }
