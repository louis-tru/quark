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
#include "../../ui/action/action.h"
#include "../../ui/action/keyframe.h"

namespace qk { namespace js {

	uint64_t kAction_Typeid(Js_Typeid(Action));
	uint64_t kKeyframe_Typeid(Js_Typeid(Keyframe));

	static Window* NewActionCheck(FunctionArgs args, cChar *name) {
		Js_Worker(args);
		if (!args.length() || !Js_IsWindow(args[0])) {
			Js_Throw("\
				* Call constructor error, param window object no match. \n\
				* @constructor %s(Window *window) \n\
			", name), nullptr;
		}
		return MixObject::mix<Window>(args[0])->self();
	}

	struct MixAction: MixObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Action, 0, { Js_Throw("Access forbidden."); });

			// Qk_DEFINE_PROP_GET(Window*, window, Protected);

			Js_MixObject_Accessor(Action, uint32_t, loop, loop);

			Js_Class_Accessor_Get(duration, {
				Js_Self(Action);
				Js_Return(self->duration());
			});

			Js_MixObject_Accessor(Action, float, speed, speed);
			Js_MixObject_Accessor(Action, bool, playing, playing);

			Js_Class_Method(play, {
				Js_Self(Action);
				self->play();
				Js_Return(args.thisObj());
			});

			Js_Class_Method(stop, {
				Js_Self(Action);
				self->stop();
				Js_Return(args.thisObj());
			});

			Js_Class_Method(seek, {
				if (!args.length() || !args[0]->isUint32())
					Js_Throw("@method Action.seek(uint32_t timeMs)");
				Js_Self(Action);
				self->seek(args[0]->toUint32(worker)->value());
				Js_Return(args.thisObj());
			});

			Js_Class_Method(seekPlay, {
				if (!args.length() || !args[0]->isUint32())
					Js_Throw("@method Action.seek_play(uint32_t timeMs)");
				Js_Self(Action);
				self->seek_play(args[0]->toUint32(worker)->value());
				Js_Return(args.thisObj());
			});

			Js_Class_Method(seekStop, {
				if (!args.length() || !args[0]->isUint32())
					Js_Throw("@method Action.seek_stop(uint32_t timeMs)");
				Js_Self(Action);
				self->seek_stop(args[0]->toUint32(worker)->value());
				Js_Return(args.thisObj());
			});

			Js_Class_Method(before, {
				if (!args.length() || !worker->instanceOf(args[0], kAction_Typeid))
					Js_Throw("@method Action.before(Action *act)");
				Js_Self(Action);
				self->before(mix<Action>(args[0])->self());
			});

			Js_Class_Method(after, {
				if (!args.length() || !worker->instanceOf(args[0], kAction_Typeid))
					Js_Throw("@method Action.after(Action *act)");
				Js_Self(Action);
				self->after(mix<Action>(args[0])->self());
			});

			Js_Class_Method(remove, {
				Js_Self(Action);
				self->remove();
			});

			Js_Class_Method(append, {
				if (!args.length() || !worker->instanceOf(args[0], kAction_Typeid))
					Js_Throw("@method Action.append(Action *child)");
				Js_Self(Action);
				Js_Try_Catch({
					self->append(mix<Action>(args[0])->self());
				}, Error);
			});

			Js_Class_Method(clear, {
				Js_Self(Action);
				self->clear();
			});

			cls->exports("Action", exports);
		}
	};

	struct MixSpawnAction: MixObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(SpawnAction, Action, {
				auto win = NewActionCheck(args, "SpawnAction");
				if (win)
					New<MixSpawnAction>(args, new SpawnAction(win));
			});
			cls->exports("SpawnAction", exports);
		}
	};

	struct MixSequenceAction: MixObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(SequenceAction, Action, {
				auto win = NewActionCheck(args, "SequenceAction");
				if (win)
					New<MixSequenceAction>(args, new SequenceAction(win));
			});
			cls->exports("SequenceAction", exports);
		}
	};

	struct MixKeyframeAction: MixObject {
		typedef KeyframeAction Type;

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(KeyframeAction, Action, {
				auto win = NewActionCheck(args, "KeyframeAction");
				if (win)
					New<MixKeyframeAction>(args, new KeyframeAction(win));
			});

			Js_Class_Accessor_Get(time, {
				Js_Self(Type);
				Js_Return(self->time());
			});

			Js_Class_Accessor_Get(frame, {
				Js_Self(Type);
				Js_Return(self->frame());
			});

			Js_Class_Accessor_Get(length, {
				Js_Self(Type);
				Js_Return(self->length());
			});

			Js_Class_Indexed_Get({
				Js_Self(Type);
				if (key >= self->length()) {
					Js_Throw("@method KeyframeAction[](uint32_t index) Frame array index overflow.");
				}
				auto wobj = mix<Keyframe>(self->operator[](key), kKeyframe_Typeid);
				Js_Return( wobj->handle() );
			});

			// bool hasProperty(ViewProp name);

			Js_Class_Method(addFrame, {
				if (!args.length() || !args[0]->isUint32()) {
					Js_Throw("\
						Param timeMs cannot be empty \n\
						@method KeyframeAction.addFrame(uint32_t timeMs, cCurve& curve = EASE) \n\
					");
				}
				auto timeMs = args[0]->toUint32(worker)->value();
				auto curve = EASE;
				if (args.length() > 1 && !args[1]->isUndefined()) {
					Js_Parse_Type(Curve, args[1], "@method KeyframeAction.addFrame() curve = %s");
					curve = out;
				}
				Js_Self(Type);
				Js_Return( self->addFrame(timeMs, curve) );
			});

			Js_Class_Method(addFrameWithCss, {
				if (!args.length() || !args[0]->isString()) {
					Js_Throw("\
						Bad argument cssExp \n\
						@method KeyframeAction.addFrameWithCss(cString& cssExp, uint32_t *timeMs, cCurve *curve) \n\
					");
				}
				String cssExp = args[0]->toString(worker)->value(worker);
				uint32_t time;
				uint32_t *time_p = nullptr;
				Curve curve;
				Curve *curve_p = nullptr;

				if (args.length() > 1) {
					Js_Parse_Type(uint32_t, args[1], "@method KeyframeAction.addFrameWithCss() timeMs = %s");
					time = out; time_p = &time;
				}
				if (args.length() > 2) {
					Js_Parse_Type(Curve, args[2], "@method KeyframeAction.addFrameWithCss() curve = %s");
					curve = out; curve_p = &curve;
				}
				Js_Self(Type);
				Js_Return( self->addFrameWithCss(cssExp, time_p, curve_p) );
			});

			cls->exports("KeyframeAction", exports);
		}
	};

	struct MixKeyframe: MixObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Keyframe, StyleSheets, { Js_Throw("Access forbidden."); });

			Js_Class_Accessor_Get(index, {
				Js_Self(Keyframe);
				Js_Return(self->index());
			});

			Js_Class_Accessor_Get(time, {
				Js_Self(Keyframe);
				Js_Return(self->time());
			});

			Js_Class_Accessor_Get(curve, {
				Js_Self(Keyframe);
				Js_Return( worker->types()->jsvalue(self->curve()) );
			});

			cls->exports("Keyframe", exports);
		};
	};

	struct NativeAction {
		static void binding(JSObject* exports, Worker* worker) {
			worker->bindingModule("_css");
			MixAction::binding(exports, worker);
			MixSpawnAction::binding(exports, worker);
			MixSequenceAction::binding(exports, worker);
			MixKeyframeAction::binding(exports, worker);
			MixKeyframe::binding(exports, worker);
		}
	};

	Js_Module(_action, NativeAction);
} }
