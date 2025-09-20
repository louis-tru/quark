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
#include "../../ui/view/spine.h"

namespace qk { namespace js {
	uint64_t kSkeletonData_Typeid(Js_Typeid(SkeletonData));

	class MixSkeletonData: public MixObject {
	public:
		typedef SkeletonData Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(SkeletonData, 0, {
				Js_Throw("Access forbidden.");
			});
			Js_Class_Static_Method(Make, {
				String skeletonPath;
				String atlasPath;
				String dir;
				WeakBuffer skeletonBuf;
				WeakBuffer atlasBuf;
				float scale = 1.0;
				if (args.length() < 2 ||
					!(args[0]->asString(worker).to(skeletonPath) || args[0]->asBuffer(worker).to(skeletonBuf)) ||
					!(args[1]->asString(worker).to(atlasPath) || args[1]->asBuffer(worker).to(atlasBuf))
				) {
					err:
					Js_Throw(
						"@static SkeletonData.Make(skeletonPath,atlasPath,scale?)\n"
						"@param skeletonPath:string|Buffer\n"
						"@param atlasPath:string|Buffer\n"
						"@param dir?:string\n"
						"@param scale?:Float **default** 1.0\n"
					);
				}
				Sp<Type> obj;
				if (args.length() > 2) {
					args[2]->asFloat32(worker).to(scale) ||
					args[2]->asString(worker).to(dir);
				}
				if (skeletonPath.length()) {
					obj = Type::Make(skeletonPath, atlasPath);
				} else if (atlasPath.length()) {
					obj = Type::Make(skeletonBuf.buffer(), atlasPath);
				} else {
					if (args.length() > 3)
						args[3]->asFloat32(worker).to(scale);
					obj = Type::Make(skeletonBuf.buffer(), atlasBuf.buffer(), dir, scale);
				}
				if (!obj)
					Js_Return_Null();
				auto mixObj = mix<Type>(obj.get(), kSkeletonData_Typeid);
				Js_Return(mixObj->handle());
			});
			cls->exports("SkeletonData", exports);
		};
	};

	void binding_skeletonData(JSObject* exports, Worker* worker) {
		MixSkeletonData::binding(exports, worker);
	}

	class MixSpine: public MixViewObject {
	public:
		typedef Spine Type;
		virtual MatrixView* asMatrixView() {
			return self<Spine>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Spine, View, {
				Js_NewView(Spine);
			});
			inheritMatrixView(cls, worker);

			Js_MixObject_Accessor(Type, SkeletonDataPtr, skeleton, skeleton);
			Js_MixObject_Accessor(Type, String, skin, skin);
			Js_MixObject_Accessor(Type, float, speed, speed);
			Js_MixObject_Accessor(Type, float, default_mix, defaultMix);

			Js_Class_Method(setToSetupPose, {
				self->set_to_setup_pose();
			});

			Js_Class_Method(setBonesToSetupPose, {
				self->set_bones_to_setup_pose();
			});

			Js_Class_Method(setSlotsToSetupPose, {
				self->set_slots_to_setup_pose();
			});

			Js_Class_Method(setAttachment, {
				String slotName;
				String attachmentName;
				if (args.length() < 2 ||
						!args[0]->asString(worker).to(slotName) ||
						!args[1]->asString(worker).to(attachmentName)
				) {
					Js_Throw(
						"@method Spine.setAttachment(slotName,attachmentName)\n"
						"@param slotName:string\n"
						"@param attachmentName:string\n"
					);
				}
				Js_Self(Type);
				self->set_attachment(slotName, attachmentName);
			});

			Js_Class_Method(setMix, {
				String fromName;
				String toName;
				float duration;
				if (args.length() < 3 ||
						!args[0]->asString(worker).to(fromName) ||
						!args[1]->asString(worker).to(toName) ||
						!args[2]->asFloat32(worker).to(duration)
				) {
					Js_Throw(
						"@method Spine.setMix(fromName,toName,duration)\n"
						"@param fromName:string\n"
						"@param toName:string\n"
						"@param duration:Float\n"
					);
				}
				self->set_mix(fromName, toName, duration);
			});

			Js_Class_Method(setAnimation, {
				uint32_t trackIndex;
				String name;
				bool loop;
				if (args.length() < 3 ||
						!args[0]->asUint32(worker).to(trackIndex) ||
						!args[1]->asString(worker).to(name) ||
						!args[2]->asBoolean(worker).to(loop)
				) {
					Js_Throw(
						"@method Spine.setAnimation(trackIndex,name,loop)\n"
						"@param trackIndex:Uint\n"
						"@param name:String\n"
						"@param loop:boolean\n"
					);
				}
				self->set_animation(trackIndex, name, loop);
			});

			Js_Class_Method(addAnimation, {
				uint32_t trackIndex;
				String name;
				bool loop;
				float delay = 0;
				if (args.length() < 3 ||
						!args[0]->asUint32(worker).to(trackIndex) ||
						!args[1]->asString(worker).to(name) ||
						!args[2]->asBoolean(worker).to(loop)
				) {
					Js_Throw(
						"@method Spine.addAnimation(trackIndex,name,loop,delay?)\n"
						"@param trackIndex:Uint\n"
						"@param name:String\n"
						"@param loop:boolean\n"
						"@param delay?:Float\n"
					);
				}
				if (args.length() > 3)
					args[3]->asFloat32(worker).to(delay);
				self->add_animation(trackIndex, name, loop, delay);
			});

			Js_Class_Method(setEmptyAnimation, {
				uint32_t trackIndex;
				float mixDuration;
				if (args.length() < 2 ||
						!args[0]->asUint32(worker).to(trackIndex) ||
						!args[1]->asFloat32(worker).to(mixDuration)
				) {
					Js_Throw(
						"@method Spine.setEmptyAnimation(trackIndex,mixDuration)\n"
						"@param trackIndex:Uint\n"
						"@param mixDuration:Float\n"
					);
				}
				self->set_empty_animation(trackIndex, mixDuration);
			});

			Js_Class_Method(setEmptyAnimations, {
				float mixDuration = 0;
				if (args.length() == 0 ||
						!args[0]->asFloat32(worker).to(mixDuration)
				) {
					Js_Throw(
						"@method Spine.setEmptyAnimations(mixDuration)\n"
						"@param mixDuration:Float\n"
					);
				}
				self->set_empty_animations(mixDuration);
			});

			Js_Class_Method(addEmptyAnimation, {
				uint32_t trackIndex;
				float mixDuration;
				float delay = 0;
				if (args.length() < 2 ||
						!args[0]->asUint32(worker).to(trackIndex) ||
						!args[1]->asFloat32(worker).to(mixDuration)
				) {
					Js_Throw(
						"@method Spine.addEmptyAnimation(trackIndex,mixDuration,delay?)\n"
						"@param trackIndex:Uint\n"
						"@param mixDuration:Float\n"
						"@param delay?:Float\n"
					);
				}
				if (args.length() > 2)
					args[2]->asFloat32(worker).to(delay);
				self->add_empty_animation(trackIndex, mixDuration, delay);
			});

			// spine::TrackEntry* get_current(int trackIndex = 0);

			Js_Class_Method(clearTracks, {
				self->clear_tracks();
			});

			Js_Class_Method(clearTrack, {
				uint32_t trackIndex = 0;
				if (args.length()) {
					args[0]->asUint32(worker).to(trackIndex);
				}
				self->clear_track(trackIndex);
			});

			cls->exports("Spine", exports);
		}
	};

	void binding_spine(JSObject* exports, Worker* worker) {
		MixSpine::binding(exports, worker);
	}
} }
