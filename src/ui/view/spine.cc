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

#include "./spine.inl"

#define _IfSkel(...)  auto skel = _skel.load(std::memory_order_acquire); if (!skel) return __VA_ARGS__
#define _IfAutoMutex(...) _IfSkel(__VA_ARGS__); AutoMutexExclusive ame(_mutex);

namespace qk {
	static void safe_trigger_event_rt(View *v, UIEvent *e, cUIEventName& name);
	static void stateListener(AnimationState *state, spine::EventType type, TrackEntry *entry, spine::Event *e);
	static String CastStr(const spine::String &str);

	SpineEvent::SpineEvent(Spine* origin, Type type, int trackIndex, cString& animationName, float trackTime)
		: UIEvent(origin), _type(type), _trackIndex(trackIndex), _animationName(animationName), _trackTime(trackTime) {}

	SpineExtEvent::SpineExtEvent(Spine* origin
		, int trackIndex, cString& animationName, float trackTime
		, cEventData* staticData, float time, int intValue
		, float floatValue, cString& stringValue, float volume, float balance)
		: SpineEvent(origin, SEType::kExtEvent_Type, trackIndex, animationName, trackTime)
		, _static_data(staticData), _time(time), _int_value(intValue)
		, _float_value(floatValue), _string_value(stringValue)
		, _volume(volume), _balance(balance)
	{}

	Spine::SkeletonWrapper::SkeletonWrapper(Spine *host, SkeletonData *data)
		: _host(host), _wrapData(data), _data(_wrapData->_data)
		, _skeleton(_data)
		, _stateData(_data)
		, _state(&_stateData)
	{}

	void Spine::SkeletonWrapper::destroy() {
		_host->preRender().async_call([](auto self, auto arg) {
			self->Object::destroy(); // safe destroy on render thread
		}, this, 0);
	}

	void Spine::SkeletonWrapper::update(float deltaTime) {
		// update skeleton and animation state
		_skeleton.update(deltaTime);
		_state.update(deltaTime);
		_state.apply(_skeleton);
		_skeleton.updateWorldTransform(Physics_Update);
	}

	Spine::Spine(): Agent()
		, _clipper(new SkeletonClipping()), _skel(nullptr)
		, _speed(1.0f), _default_mix(0), _firstDraw(true)
	{
		// sizeof(Spine);
	}

	void Spine::destroy() {
		Releasep(_skel);
		View::destroy(); // Call parent destroy
	}

	SkeletonData* Spine::skel() {
		_IfSkel(nullptr);
		return skel->_wrapData.get();
	}

	void Spine::set_to_setup_pose() {
		_IfAutoMutex();
		skel->_skeleton.setToSetupPose();
	}

	void Spine::set_bones_to_setup_pose() {
		_IfAutoMutex();
		skel->_skeleton.setBonesToSetupPose();
	}

	void Spine::set_slots_to_setup_pose() {
		_IfAutoMutex();
		skel->_skeleton.setSlotsToSetupPose();
	}

	String Spine::skin() const {
		_IfSkel(String());
		auto &name = skel->_skeleton.getSkin()->getName();
		return CastStr(name);
	}

	void Spine::set_skin(String val) {
		_IfAutoMutex();
		skel->_skeleton.setSkin(val.isEmpty() ? nullptr: val.c_str());
	}

	void Spine::set_speed(float val) {
		_speed = Qk_Min(1e2, Qk_Max(val, 0.01));
	}

	String Spine::animation() const {
		_IfAutoMutex(String());
		auto track = skel->_state.getCurrent(0);
		auto animation = track ? track->getAnimation() : nullptr;
		return animation ? CastStr(animation->getName()) : String();
	}

	void Spine::set_animation(String name) {
		set_animation(0, name, true);
	}

	void Spine::set_attachment(cString &slotName, cString &name) {
		_IfAutoMutex();
		skel->_skeleton.setAttachment(slotName.c_str(), name.isEmpty() ? nullptr: name.c_str());
	}

	void Spine::set_default_mix(float val) {
		_default_mix = val;
		_IfSkel();
		skel->_stateData.setDefaultMix(val);
	}

	void Spine::set_mix(cString &fromAnimation, cString &toAnimation, float duration) {
		_IfAutoMutex();
		skel->_stateData.setMix(fromAnimation.c_str(), toAnimation.c_str(), duration);
	}

	TrackEntry* Spine::set_animation(uint32_t trackIndex, cString &name, bool loop) {
		_IfAutoMutex(nullptr);
		Animation *animation = skel->_data->findAnimation(name.c_str());
		if (!animation) {
			Qk_Log("Spine: Animation not found: %s", name.c_str());
			return nullptr;
		}
		return skel->_state.setAnimation(trackIndex, animation, loop);
	}

	TrackEntry *Spine::add_animation(uint32_t trackIndex, cString &name, bool loop, float delay) {
		_IfAutoMutex(nullptr);
		Animation *animation = skel->_data->findAnimation(name.c_str());
		if (!animation) {
			Qk_Log("Spine: Animation not found: %s", name.c_str());
			return nullptr;
		}
		return skel->_state.addAnimation(trackIndex, animation, loop, delay);
	}

	TrackEntry *Spine::set_empty_animation(uint32_t trackIndex, float mixDuration) {
		_IfAutoMutex(nullptr);
		return skel->_state.setEmptyAnimation(trackIndex, mixDuration);
	}

	void Spine::set_empty_animations(float mixDuration) {
		_IfAutoMutex();
		skel->_state.setEmptyAnimations(mixDuration);
	}

	TrackEntry *Spine::add_empty_animation(uint32_t trackIndex, float mixDuration, float delay) {
		_IfAutoMutex(nullptr);
		return skel->_state.addEmptyAnimation(trackIndex, mixDuration, delay);
	}

	TrackEntry* Spine::get_current(uint32_t trackIndex) {
		_IfSkel(nullptr);
		return skel->_state.getCurrent(trackIndex);
	}

	void Spine::clear_tracks() {
		_IfAutoMutex();
		skel->_state.clearTracks();
	}

	void Spine::clear_track(uint32_t trackIndex) {
		_IfAutoMutex();
		skel->_state.clearTrack(trackIndex);
	}

	ViewType Spine::viewType() const {
		return kSpine_ViewType;
	}

	Vec2 Spine::client_size() {
		return _skel_size;
	}

	Region Spine::client_region() {
		auto begin = -_origin_value;
		return { begin, begin + _skel_size, _translate };
	}

	void Spine::set_skel(SkeletonData *data) {
		auto lastSkel = _skel.load(std::memory_order_acquire);
		if ((lastSkel ? lastSkel->_wrapData.get(): nullptr) != data) {
			SkeletonWrapper *skel = nullptr; // new skeleton
			if (data) {
				Vector<float> bounds;
				skel = New<SkeletonWrapper>(this, data);
				skel->_stateData.setDefaultMix(_default_mix);
				skel->_skeleton.setToSetupPose();
				skel->_skeleton.updateWorldTransform(Physics_Update); // @setToSetupPose()
				skel->_state.setRendererObject(this);
				skel->_state.setListener(stateListener);
				skel->_skeleton.getBounds(_skel_origin[0], _skel_origin[1],
																	_skel_size[0], _skel_size[1], bounds);
				// the origin is bottom-left, convert to top-left
				_skel_origin[0] = -_skel_origin[0]; // -x right
				_skel_origin[1] += _skel_size[1]; // +y up
			} else {
				_skel_origin = _skel_size = {};
			}
			AutoMutexExclusive ame(_mutex);
			_skel = skel;
			_firstDraw = true;
			Release(lastSkel);
			mark_layout(kLayout_Typesetting | kTransform, false);
		}
	}

	void Spine::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & (kTransform | kVisible_Region)) { // Update transform matrix
			unmark(kTransform | kVisible_Region); // Unmark
			_origin_value = solve_origin_value(_skel_origin); // solve origin value
			auto v = parent->layout_offset_inside() + _translate;
			_matrix = Mat(mat).set_translate(parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]); // the origin world coords
			solve_visible_area(_matrix);
		}
	}

	void Spine::onActivate() {
		if (level() == 0) {
			preRender().untask(this);
		} else {
			preRender().addtask(this);
		}
	}

	bool Spine::run_task(int64_t time, int64_t delta) {
		_IfAutoMutex(false);
		auto deltaTime = _speed * 0.000001f * delta; /* delta in seconds */
		skel->update(_firstDraw ? (_firstDraw = false, 0) : deltaTime);
		return skel->_skeleton.getColor().a != 0;
	}

	//////////////////////////////////////////////////////////////////////////////////

	static void safe_trigger_event_rt(Spine *v, UIEvent *e, cUIEventName& name) {
		struct Core: CallbackCore<Object> {
			Core(View *v, UIEvent *e, cUIEventName& n)
				: view(Sp<View>::lazy(v)), evt(e), name(n) {
			}
			void call(Data& e) {
				view->trigger(name, **evt);
			}
			Sp<View> view;
			Sp<UIEvent> evt;
			UIEventName name;
		};
		if (v->tryRetain_rt()) {
			v->preRender().post(Cb(new Core(v, e, name)), true);
		} else {
			Release(e);
		}
	}

	static void stateListener(AnimationState *state, spine::EventType type, TrackEntry *entry, spine::Event *e) {
		auto self = ((Spine*)state->getRendererObject());
		auto trackIndex = entry->getTrackIndex();
		String animationName = CastStr(entry->getAnimation()->getName());
		auto trackTime = entry->getTrackTime();
		switch (type) {
			case EventType_Start:
				safe_trigger_event_rt(self, new SpineEvent(self, SEType::kStart_Type, trackIndex, animationName, trackTime), UIEvent_SpineStart);
				break;
			case EventType_Interrupt:
				safe_trigger_event_rt(self, new SpineEvent(self, SEType::kInterrupt_Type, trackIndex, animationName, trackTime), UIEvent_SpineInterrupt);
				break;
			case EventType_End:
				safe_trigger_event_rt(self, new SpineEvent(self, SEType::kEnd_Type, trackIndex, animationName, trackTime), UIEvent_SpineEnd);
				break;
			case EventType_Dispose:
				safe_trigger_event_rt(self, new SpineEvent(self, SEType::kDispose_Type, trackIndex, animationName, trackTime), UIEvent_SpineDispose);
				break;
			case EventType_Complete:
				safe_trigger_event_rt(self, new SpineEvent(self, SEType::kComplete_Type, trackIndex, animationName, trackTime), UIEvent_SpineComplete);
				break;
			case EventType_Event: {
				// TODO maybe the "e->getData()" is unsafe by
				// SkeletonData::Make(cBuffer &skeletonBuff, cBuffer &atlasBuff, cString &dir, float scale) created
				auto evt = new SpineExtEvent(self,
					trackIndex,
					animationName,
					trackTime,
					&e->getData(),
					e->getTime(),
					e->getIntValue(),
					e->getFloatValue(),
					CastStr(e->getStringValue()),
					e->getVolume(),
					e->getBalance()
				);
				safe_trigger_event_rt(self, evt, UIEvent_SpineEvent);
				break;
			}
		}
	}

	String CastStr(const spine::String &str) {
		return String(str.buffer(), (uint32_t)str.length());
	}
}
