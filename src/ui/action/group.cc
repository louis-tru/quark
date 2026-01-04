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

#include "./action.h"
#include "../window.h"

#define _async_call _window->pre_render().async_call

namespace qk {

	#define nullId Action::Id()

	ActionGroup::ActionGroup(Window *win): Action(win){}
	SpawnAction::SpawnAction(Window *win): ActionGroup(win){}
	SequenceAction::SequenceAction(Window *win): ActionGroup(win) {}

	void ActionGroup::clear() {
		if (_actions.length()) {
			Qk_ASSERT(_window);
			_async_call([](auto self, auto arg) {
				if (self->isSequence())
					static_cast<SequenceAction*>(self)->_play_rt = nullId;
				for ( auto i : self->_actions_rt ) {
					i->_id_rt = nullId; // clear id
				}
				self->_actions_rt.clear();
			}, this, 0);

			for ( auto &i : _actions ) {
				i.first->del_parent(); // release for main thread
			}
			_actions.clear();
			if (_duration) {
				ActionGroup::setDuration( -_duration );
			}
		}
		Qk_ASSERT_EQ(_duration, 0);
	}

	bool ActionGroup::isSequence() {
		return false;
	}

	bool SequenceAction::isSequence() {
		return true;
	}

	void ActionGroup::insertChild(Id after, Action *child) {
		struct InsertArg {
			Action::Id after;
			Action *child;
		};
		_actions.add(child);
		_async_call([](auto self, auto arg) {
			arg.arg->child->_id_rt = self->_actions_rt.insert(arg.arg->after, arg.arg->child);
			free(arg.arg);
		}, this, new InsertArg{after,child});
	}

	void SpawnAction::insertChild(Id after, Action *child) {
		ActionGroup::insertChild(after, child);
		auto du = child->_duration;
		if ( du > _duration ) {
			Action::setDuration( du - _duration );
		}
	}

	void SequenceAction::insertChild(Id after, Action *child) {
		ActionGroup::insertChild(after, child);
		if ( child->_duration ) {
			Action::setDuration( child->_duration );
		}
	}

	void SpawnAction::removeChild(Id id) {
		Qk_ASSERT(id != _actions_rt.end());
		// _async_call([](auto self, auto arg) {
		_window->pre_render().async_call([](auto self, auto arg) {
			self->_actions_rt.erase( arg.arg );
			(*arg.arg)->_id_rt = nullId;
		}, this, id);
		auto act = *id;
		_actions.erase( act );
		if ( act->_duration == _duration ) {
			SpawnAction::setDuration(0);
		}
	}

	void SequenceAction::removeChild(Id id) {
		Qk_ASSERT(id != _actions_rt.end());
		_async_call([](auto self, auto arg) {
			if ( self->_play_rt == arg.arg )
				self->_play_rt = nullId;
			self->_actions_rt.erase( arg.arg );
			(*arg.arg)->_id_rt = nullId;
		}, this, id);
		auto act = *id;
		_actions.erase( act );
		if ( act->_duration ) {
			Action::setDuration(-act->_duration);
		}
	}

	void SpawnAction::seek_before_rt(uint32_t time, Action *child) {
		auto parent = dynamic_cast<ActionGroup*>(_parent); // safe use parent ptr
		if (parent) {
			parent->seek_before_rt(time, this);
		} else {
			seek_time_rt(time, this);
		}
	}

	void SpawnAction::seek_time_rt(uint32_t time, Action *root) {
		for ( auto i : _actions_rt ) {
			i->seek_time_rt(time, root);
		}
	}

	void SequenceAction::seek_before_rt(uint32_t time, Action *child) {
		for ( auto i : _actions_rt ) {
			if ( child == i ) {
				break;
			} else {
				time += i->_duration; // add time
			}
		}
		auto parent = dynamic_cast<ActionGroup*>(_parent); // safe use parent ptr
		if (parent) {
			parent->seek_before_rt(time, this);
		} else {
			seek_time_rt(time, this);
		}
	}

	void SequenceAction::seek_time_rt(uint32_t time, Action *root) {
		uint32_t duration = 0;

		for ( auto i: _actions_rt ) {
			uint32_t du = duration + i->_duration;
			if ( du > time ) {
				Qk_ASSERT(*i->_id_rt == i);
				_play_rt = i->_id_rt;
				i->seek_time_rt(time - duration, root);
				return;
			}
			duration = du;
		}

		if ( _actions_rt.length() ) {
			_actions_rt.back()->seek_time_rt(time - duration, root);
		}
	}

	uint32_t SpawnAction::advance_rt(uint32_t deltaTime, bool restart, Action* root) {
		deltaTime *= _speed; // Amplification time

		if ( restart ) { // restart
			_looped_rt = 0;
		}

		uint32_t deltaTime_min = deltaTime;
	advance:
		for ( auto i : _actions_rt ) {
			uint32_t time = i->advance_rt(deltaTime, restart, root);
			deltaTime_min = Qk_Min(deltaTime_min, time);
		}

		if ( deltaTime_min && _actions_rt.length() ) {
			if (next_loop_rt()) { // continue to loop
				restart = true;
				deltaTime = deltaTime_min;

				trigger_ActionLoop_rt(deltaTime, root);

				if ( root->_id_rt != Id() ) { // is playing
					goto advance;
				}
				return 0; // end
			}
		}

	end:
		return deltaTime_min / _speed;
	}

	uint32_t SequenceAction::advance_rt(uint32_t deltaTime, bool restart, Action *root) {
		deltaTime *= _speed; // Amplification time

		if ( restart ) { // restart
			_looped_rt = 0;
			_play_rt = nullId;
		}
		if ( _play_rt == nullId ) { // restart
			if ( _actions_rt.length() ) {
				restart = true;
				_play_rt = _actions_rt.begin();
			} else {
				return deltaTime / _speed;
			}
		}

	advance:
		deltaTime = (*_play_rt)->advance_rt(deltaTime, restart, root);

		if ( deltaTime ) {
			if ( _play_rt == nullId ) { // May have been deleted child action
				if ( _actions_rt.length() ) { // Restart
					restart = true;
					_play_rt = _actions_rt.begin();
					goto advance;
				}
			} else if ( *_play_rt == _actions_rt.back() ) { // last action
				if (next_loop_rt()) { // continue to loop
					restart = true;

					trigger_ActionLoop_rt(deltaTime, root); // trigger event
					_play_rt = _actions_rt.begin();

					if ( _play_rt == _actions_rt.end() ) {
						// May be deleted when triggering an `action_loop` event
						_play_rt = nullId;
						goto end; // No child action then end
					}

					if ( root->_id_rt != Id() ) { // is playing
						goto advance;
					}
					return 0; // end
				}
			} else {
				restart = true;
				_play_rt++;
				goto advance;
			}
		}

	end:
		return deltaTime / _speed;
	}

}
