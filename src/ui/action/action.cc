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

#include "./action.h"
#include "../layout/layout.h"
#include "../../errno.h"
#include "../window.h"
#include "../app.h"
#include "../event.h"

namespace qk {

	Action::Action()
		: _parent(nullptr)
		, _loop(0)
		, _duration(0)
		, _speed(1)
		, _looped(0)
	{
	}

	Action::~Action() {
		Qk_ASSERT( _id == Id() );
	}

	void Action::release() {
		if (refCount() == 1)
			clear();
		Reference::release();
	}

	bool Action::playing() const {
		return _parent ? _parent->playing(): _id != Id();
	}

	void Action::set_speed(float value) {
		_speed = Qk_MIN(10, Qk_MAX(value, 0.1));
	}

	void Action::set_loop(uint32_t value) {
		_loop = value;
	}

	void Action::seek(uint32_t time) {
		time = Qk_MIN(time, _duration);
		if (_parent) {
			_parent->seek_before(time, this);
		} else {
			seek_time(time, this);
		}
	}

	void Action::seek_play(uint32_t time) {
		seek(time);
		play();
	}

	void Action::seek_stop(uint32_t time) {
		seek(time);
		stop();
	}

	void Action::play() {
		if ( _parent ) {
			_parent->play();
		} else {
			if (_target) {
				if (_id == Id()) {
					auto center = _target->window()->actionCenter();
					_id = center->_actions.pushBack(this);
					retain(); // retain for center
				}
			}
		}
	}

	void Action::stop() {
		if ( _parent ) {
			_parent->stop();
		} else {
			if (_target) {
				if (_id != Id()) {
					auto center = _target->window()->actionCenter();
					center->_actions.erase(_id);
					_runAdvance = false;
					_id = Id();
					release(); // release for center
				}
			}
		}
	}

	void Action::before(Action *act) throw(Error) {
		Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::before, illegal parent empty");
		static_cast<GroupAction*>(_parent)->insert(_id, act);
	}

	void Action::after(Action *act) throw(Error) {
		Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::after, illegal parent empty");
		auto id = _id;
		static_cast<GroupAction*>(_parent)->insert(++id, act);
	}

	void Action::remove() throw(Error) {
		Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::remove, illegal parent empty");
		static_cast<GroupAction*>(_parent)->remove_child(_id);
	}

	// -----------------------------------------------------------------------------------------------

	void Action::trigger_ActionLoop(uint32_t delay, Action* root) {
		auto v_h = root->_target->safe_view();
		auto v = *v_h;
		if (v) {
			auto evt = new ActionEvent(this, v, delay, 0, _loop);
			shared_app()->loop()->post(Cb([v,evt](auto& e) {
				Sp<ActionEvent> h(evt);
				v->trigger(UIEvent_ActionLoop, *evt);
			}, v));
		}
	}

	void Action::trigger_ActionKeyframe(uint32_t delay, uint32_t frame_index, Action* root) {
		auto v_h = root->_target->safe_view();
		auto v = *v_h;
		if (v) {
			auto evt = new ActionEvent(this, v, delay, frame_index, _loop);
			shared_app()->loop()->post(Cb([v,evt](auto& e) {
				Sp<ActionEvent> h(evt);
				v->trigger(UIEvent_ActionKeyframe, *evt);
			}, v));
		}
	}

	void Action::set_parent(Action *parent) throw(Error) {
		Qk_Check(!_parent && !_target, ERR_ACTION_ILLEGAL_CHILD, "illegal child action!");
		Qk_ASSERT(_id == Id());
		_parent = parent;
		retain(); // retain
	}

	void Action::del_parent() {
		_parent = nullptr;
		release();
	}

	void Action::ser_target(Layout *target) throw(Error) {
		Qk_ASSERT(target);
		Qk_Check(!_parent, ERR_ACTION_ILLEGAL_ROOT, "cannot set non root action");
		Qk_Check(!_target, ERR_ACTION_DISABLE_MULTIPLE_TARGET, "action cannot set multiple target");
		_target = target;
		retain(); // retain from view
	}

	void Action::del_target(Layout *target) {
		Qk_ASSERT(target);
		Qk_ASSERT(target == _target);
		stop(); // stop action
		_target = nullptr;
		release(); // release from view
	}

	void Action::update_duration(int32_t diff) {
		_duration += diff;
		if (_parent) {
			_parent->update_duration(diff);
		}
	}

	void SpawnAction::update_duration(int32_t diff) {
		int32_t new_duration = 0;
		for ( auto &i : _actions ) {
			new_duration = Qk_MAX(i->_duration, new_duration);
		}
		diff = new_duration - _duration;
		if ( diff ) {
			Action::update_duration(diff);
		}
	}

}
