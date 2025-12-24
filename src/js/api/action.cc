/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
	const uint64_t kAction_Typeid(Js_Typeid(Action));
	const uint64_t kKeyframe_Typeid(Js_Typeid(Keyframe));

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
		typedef Action Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Action, 0, { Js_Throw("Access forbidden."); });
			// Qk_DEFINE_PROP_GET(Window*, window, Protected);
			Js_MixObject_Accessor(Action, uint32_t, loop, loop);
			Js_MixObject_Acce_Get(Action, uint32_t, duration, duration);
			Js_MixObject_Accessor(Action, float, speed, speed);
			Js_MixObject_Accessor(Action, bool, playing, playing);

			Js_Class_Method(play, {
				self->play();
				Js_Return(args.thisObj());
			});

			Js_Class_Method(stop, {
				self->stop();
				Js_Return(args.thisObj());
			});

			Js_Class_Method(seek, {
				double time;
				if (!args.length() || !args[0]->asNumber(worker).to(time))
					Js_Throw("@method Action.seek(uint32_t timeMs)");
				self->seek(time);
				Js_Return(args.thisObj());
			});

			Js_Class_Method(seekPlay, {
				double time;
				if (!args.length() || !args[0]->asNumber(worker).to(time))
					Js_Throw("@method Action.seek_play(uint32_t timeMs)");
				self->seek_play(time);
				Js_Return(args.thisObj());
			});

			Js_Class_Method(seekStop, {
				double time;
				if (!args.length() || !args[0]->asNumber(worker).to(time))
					Js_Throw("@method Action.seek_stop(uint32_t timeMs)");
				self->seek_stop(time);
				Js_Return(args.thisObj());
			});

			Js_Class_Method(before, {
				if (!args.length() || !worker->instanceOf(args[0], kAction_Typeid))
					Js_Throw("@method Action.before(Action *act)");
				self->before(MixObject::mix<Action>(args[0])->self());
			});

			Js_Class_Method(after, {
				if (!args.length() || !worker->instanceOf(args[0], kAction_Typeid))
					Js_Throw("@method Action.after(Action *act)");
				self->after(MixObject::mix<Action>(args[0])->self());
			});

			Js_Class_Method(remove, {
				self->remove();
			});

			Js_Class_Method(append, {
				if (!args.length() || !worker->instanceOf(args[0], kAction_Typeid))
					Js_Throw("@method Action.append(Action *child)");
				Js_Try_Catch({
					self->append(MixObject::mix<Action>(args[0])->self());
				}, Error);
			});

			Js_Class_Method(clear, {
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
			Js_MixObject_Acce_Get(KeyframeAction, uint32_t, time, time);
			Js_MixObject_Acce_Get(KeyframeAction, uint32_t, frame, frame);
			Js_MixObject_Acce_Get(KeyframeAction, uint32_t, length, length);

			Js_Class_Indexed_Get({
				if (key >= self->length()) {
					Js_Throw("KeyframeAction[](index:Uint)Keyframe\nFrame array index overflow.");
				}
				auto wobj = MixObject::mix<Keyframe>(self->operator[](key), kKeyframe_Typeid);
				Js_Return( wobj->handle() );
			});

			// bool hasProperty(ViewProp name);

			Js_Class_Method(addFrame, {
				double timeMs;
				if (!args.length() || !args[0]->asNumber(worker).to(timeMs)) {
					Js_Throw("KeyframeAction.addFrame(timeMs:Uint,curve?:Curve)Keyframe\nBad argument timeMs");
				}
				auto curve = EASE;
				if (args.length() > 1 && !args[1]->isUndefined()) {
					Js_Parse_Type(Curve, args[1], "KeyframeAction.addFrame(timeMs:Uint,curve?:Curve)Keyframe\ncurve = %s");
					curve = out;
				}
				Js_Return( self->addFrame(timeMs, curve) );
			});

			Js_Class_Method(addFrameWithCss, {
				if (!args.length() || !args[0]->isString()) {
					Js_Throw("KeyframeAction.addFrameWithCss(cssExp:string,timeMs?:Uint,curve?:Curve)Keyframe\nBad argument cssExp");
				}
				String cssExp = args[0]->toString(worker)->value(worker);
				uint32_t time;
				uint32_t *time_p = nullptr;
				Curve curve;
				Curve *curve_p = nullptr;

				if (args.length() > 1) {
					Js_Parse_Type(double, args[1], "KeyframeAction.addFrameWithCss(cssExp:string,timeMs?:Uint,curve?:Curve)Keyframe\ntimeMs = %s");
					time = out;
					time_p = &time;
				}
				if (args.length() > 2) {
					Js_Parse_Type(Curve, args[2], "KeyframeAction.addFrameWithCss(cssExp:string,timeMs?:Uint,curve?:Curve)Keyframe\ncurve = %s");
					curve = out; curve_p = &curve;
				}
				Js_Return( self->addFrameWithCss(cssExp, time_p, curve_p) );
			});

			cls->exports("KeyframeAction", exports);
		}
	};

	struct MixKeyframe: MixObject {
		typedef Keyframe Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Keyframe, StyleSheets, { Js_Throw("Access forbidden."); });
			Js_MixObject_Acce_Get(Keyframe, uint32_t, index, index);
			Js_MixObject_Acce_Get(Keyframe, uint32_t, time, time);
			Js_MixObject_Acce_Get(Keyframe, Curve, curve, curve);
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
