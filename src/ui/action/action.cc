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
#include "../view/view.h"
#include "../../errno.h"
#include "../window.h"
#include "../app.h"
#include "../event.h"

#define _async_call _window->preRender().async_call

namespace qk {

	Action::Action(Window *win)
		: _window(win)
		, _loop(0)
		, _duration(0)
		, _speed(1)
		, _parent(nullptr)
		, _target(nullptr)
		, _looped(0)
	{
		Qk_ASSERT(win);
	}

	Action::~Action() {
		Qk_ASSERT( _id == Id() );
		_window = nullptr;
	}

	void Action::destroy() {
		_async_call([](auto self, auto arg) {
			// To ensure safety and efficiency, it should be destroyed in RT (render thread)
			self->Object::destroy();
		}, this, 0);
	}

	bool Action::playing() const {
		return _parent ? _parent->playing(): _id != Id();
	}

	void Action::set_playing(bool val) {
		if (val)
			play();
		else
			stop();
	}

	void Action::set_speed(float value) {
		_async_call([](auto self, auto arg) {
			self->_speed = Qk_MIN(10, Qk_MAX(arg.arg, 0.1));
		}, this, value);
	}

	void Action::set_loop(uint32_t value) {
		_async_call([](auto self, auto arg) { self->_loop = arg.arg; }, this, value);
	}

	void Action::seek_play(uint32_t time) {
		_async_call([](auto self, auto arg) {
			self->seek_Rt(arg.arg);
			self->play_Rt();
		}, this, time);
	}

	void Action::seek_stop(uint32_t time) {
		_async_call([](auto self, auto arg) {
			self->seek_Rt(arg.arg);
			self->stop_Rt();
		}, this, time);
	}

	void Action::seek(uint32_t time) {
		_async_call([](auto self, auto time) { self->seek_Rt(time.arg); }, this, time);
	}

	void Action::clear() {
		_async_call([](auto self, auto arg) { self->clear_Rt(); }, this, 0);
	}

	void Action::play() {
		_async_call([](auto self, auto arg) { self->play_Rt(); }, this, 0);
	}

	void Action::stop() {
		_async_call([](auto self, auto arg) { self->stop_Rt(); }, this, 0);
	}

	void Action::play_Rt() {
		if (_parent) {
			_parent->play_Rt();
		}
		else if (_id == Id()) {
			auto id = _window->actionCenter()->_actions_Rt.pushBack({this,false});
			_id = *reinterpret_cast<Id*>(&id);
			retain(); // retain for center
		}
	}

	void Action::stop_Rt() {
		if (_parent) {
			_parent->stop_Rt();
		}
		else if (_id != Id()) {
			typedef List<ActionCenter::Action_Wrap>::Iterator Id1;
			_window->actionCenter()->_actions_Rt.erase(*reinterpret_cast<Id1*>(&_id));
			_id = Id();
			release(); // release for center
		}
	}

	void Action::seek_Rt(uint32_t time) {
		time = Qk_MIN(time, _duration);
		if (_parent) {
			_parent->seek_before_Rt(time, this);
		} else {
			seek_time_Rt(time, this);
		}
	}

	void Action::before(Action *act) {
		_async_call([](auto self, auto arg) {
			// Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::before, illegal parent empty");
			if (self->_parent) {
				static_cast<GroupAction*>(self->_parent)->insert_Rt(self->_id, arg.arg);
			} else {
				Qk_ERR("Action::before, illegal parent empty");
			}
		}, this, act);
	}

	void Action::after(Action *act) {
		_async_call([](auto self, auto arg) {
			// Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::after, illegal parent empty");
			if (self->_parent) {
				auto id = self->_id;
				static_cast<GroupAction*>(self->_parent)->insert_Rt(++id, arg.arg);
			} else {
				Qk_ERR("Action::after, illegal parent empty");
			}
		}, this, act);
	}

	void Action::remove() {
		_async_call([](auto self, auto arg) {
			// Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::remove, illegal parent empty");
			if (self->_parent) {
				static_cast<GroupAction*>(self->_parent)->remove_child_Rt(self->_id);
			} else {
				Qk_ERR("Action::remove, illegal parent empty");
			}
		}, this, 0);
	}

	void Action::set_target(View *target) {
		Qk_ASSERT(target);
		Qk_ASSERT(target->window() == _window);
		_async_call([](auto self, auto arg) {
			// Qk_Check(!_parent, ERR_ACTION_ILLEGAL_ROOT, "cannot set non root action");
			if (self->_parent) {
				Qk_ERR("Action::set_target, cannot set non root action"); return;
			}
			// Qk_Check(!_target, ERR_ACTION_DISABLE_MULTIPLE_TARGET, "Action::set_target, action cannot set multiple target");
			if (self->_target) {
				Qk_ERR("Action::set_target, action cannot set multiple target"); return;
			}
			self->_target = arg.arg;
		}, this, target);
	}

	void Action::del_target(View* target) {
		Qk_ASSERT(target);
		_async_call([](auto self, auto arg) {
			if (arg.arg == self->_target) {
				self->stop_Rt(); // stop action
				self->_target = nullptr;
			}
		}, this, target);
	}

	int Action::set_parent_Rt(Action *parent) {
		if (parent->_window != _window) {
			Qk_ERR("Action::set_parent_Rt, set action window not match");
			return ERR_ACTION_SET_WINDOW_NO_MATCH;
		}
		// Qk_Check(!_parent, ERR_ACTION_ILLEGAL_CHILD, "illegal child action");
		if (_parent) {
			Qk_ERR("Action::set_parent, illegal child action");
			return ERR_ACTION_ILLEGAL_CHILD;
		}
		Qk_ASSERT(_id == Id());
		_parent = parent;
		retain(); // retain
		return 0;
	}

	void Action::del_parent_Rt() {
		Qk_ASSERT(_parent);
		_parent = nullptr;
		release();
	}

	void Action::update_duration_Rt(int32_t diff) {
		_duration += diff;
		if (_parent) {
			_parent->update_duration_Rt(diff);
		}
	}

	void SpawnAction::update_duration_Rt(int32_t diff) {
		int32_t new_duration = 0;
		for ( auto &i : _actions_Rt ) {
			new_duration = Qk_MAX(i->_duration, new_duration);
		}
		diff = new_duration - _duration;
		if ( diff ) {
			Action::update_duration_Rt(diff);
		}
	}

	void Action::trigger_ActionLoop_Rt(uint32_t delay, Action* root) {
		auto sv = root->_target->safe_view();
		auto v = *sv;
		if (v) {
			auto evt = new ActionEvent(this, v, delay, 0, _loop);
			shared_app()->loop()->post(Cb([v,evt](auto& e) {
				Sp<ActionEvent> h(evt);
				v->trigger(UIEvent_ActionLoop, *evt);
			}, v));
		}
	}

	void Action::trigger_ActionKeyframe_Rt(uint32_t delay, uint32_t frame_index, Action* root) {
		auto sv = root->_target->safe_view();
		auto v = *sv;
		if (v) {
			auto evt = new ActionEvent(this, v, delay, frame_index, _loop);
			shared_app()->loop()->post(Cb([v,evt](auto& e) {
				Sp<ActionEvent> h(evt);
				v->trigger(UIEvent_ActionKeyframe, *evt);
			}, v));
		}
	}

}
