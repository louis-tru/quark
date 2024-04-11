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
#include "../window.h"

#define _async_call _window->preRender().async_call

namespace qk {

	const Action::Id nullId;

	GroupAction::GroupAction(Window *win): Action(win){}
	SpawnAction::SpawnAction(Window *win): GroupAction(win){}
	SequenceAction::SequenceAction(Window *win): GroupAction(win){}

	GroupAction::~GroupAction() {
		_duration = 0;
		clear_Rt();
	}

	void GroupAction::clear_Rt() {
		for ( auto i : _actions_Rt ) {
			i->del_parent_Rt();
		}
		_actions_Rt.clear();
		if ( _duration ) {
			GroupAction::update_duration_Rt( _duration );
		}
	}

	void SequenceAction::clear_Rt() {
		GroupAction::clear_Rt();
		_playIdx_Rt = nullId;
	}

	void SpawnAction::append(Action *child) {
		Qk_ASSERT(child);
		_async_call([](auto self, auto child) {
			if (!child.arg->set_parent_Rt(self)) {
				child.arg->_id = self->_actions_Rt.pushBack(child.arg);
			}
			int32_t du = child.arg->_duration;
			if ( du > self->_duration ) {
				self->Action::update_duration_Rt( du - self->_duration );
			}
		}, this, child);
	}

	void SequenceAction::append(Action* child) {
		Qk_ASSERT(child);
		_async_call([](auto self, auto child) {
			if (!child.arg->set_parent_Rt(self)) {
				child.arg->_id = self->_actions_Rt.pushBack(child.arg);
			}
			if ( child.arg->_duration ) {
				self->Action::update_duration_Rt( child.arg->_duration );
			}
		}, this, child);
	}

	void SpawnAction::insert_Rt(Id after, Action *child) {
		Qk_ASSERT(child);
		if (!child->set_parent_Rt(this)) {
			child->_id = _actions_Rt.insert(after, child);
		}
		int32_t du = child->_duration;
		if ( du > _duration ) {
			Action::update_duration_Rt( du - _duration );
		}
	}

	void SequenceAction::insert_Rt(Id after, Action *child) {
		Qk_ASSERT(child);
		if (!child->set_parent_Rt(this)) {
			child->_id = _actions_Rt.insert(after, child);
		}
		if ( child->_duration ) {
			Action::update_duration_Rt( child->_duration );
		}
	}

	void SpawnAction::remove_child_Rt(Id id) {
		Qk_ASSERT(id != _actions_Rt.end());
		auto act = *id;
		act->del_parent_Rt();
		_actions_Rt.erase( id );
		act->_id = nullId;
		if ( act->_duration == _duration ) {
			SpawnAction::update_duration_Rt(0);
		}
	}

	void SequenceAction::remove_child_Rt(Id id) {
		Qk_ASSERT(id != _actions_Rt.end());
		if ( id == _playIdx_Rt )
			_playIdx_Rt = nullId;
		auto act = *id;
		act->del_parent_Rt();
		_actions_Rt.erase(id);
		act->_id = nullId;
		if ( act->_duration ) {
			Action::update_duration_Rt(-act->_duration);
		}
	}

	void SpawnAction::seek_before_Rt(uint32_t time, Action *child) {
		if (_parent) {
			_parent->seek_before_Rt(time, this);
		} else {
			seek_time_Rt(time, this);
		}
	}

	void SequenceAction::seek_before_Rt(uint32_t time, Action *child) {
		for ( auto &i : _actions_Rt ) {
			if ( child == i ) {
				break;
			} else {
				time += i->_duration;
			}
		}
		if (_parent) {
			_parent->seek_before_Rt(time, this);
		} else {
			seek_time_Rt(time, this);
		}
	}

	void SpawnAction::seek_time_Rt(uint32_t time, Action *root) {
		_looped = 0;// reset loop

		for ( auto i : _actions_Rt ) {
			i->seek_time_Rt(time, root);
		}
	}

	void SequenceAction::seek_time_Rt(uint32_t time, Action *root) {
		_looped = 0;// reset loop

		uint32_t duration = 0;

		for ( auto i: _actions_Rt ) {
			uint32_t du = duration + i->_duration;
			if ( du > time ) {
				Qk_ASSERT(*i->_id == i);
				_playIdx_Rt = i->_id;
				i->seek_time_Rt(time - duration, root);
				return;
			}
			duration = du;
		}

		if ( _actions_Rt.length() ) {
			_actions_Rt.back()->seek_time_Rt(time - duration, root);
		}
	}

	uint32_t SpawnAction::advance_Rt(uint32_t time_span, bool restart, Action* root) {
		time_span *= _speed; // Amplification time

		if ( restart ) { // restart
			_looped = 0;
		}

		uint32_t surplus_time = time_span;

	advance:
		for ( auto i : _actions_Rt ) {
			uint32_t time = i->advance_Rt(time_span, restart, root);
			surplus_time = Qk_MIN(surplus_time, time);
		}

		if ( surplus_time ) {
			if ( _loop ) {
				if ( _loop > 0 ) {
					if ( _looped < _loop ) { // Can continue to loop
						_looped++;
					} else { //
						goto end;
					}
				}

				restart = true;
				time_span = surplus_time;

				trigger_ActionLoop_Rt(time_span, root);

				if ( root->_id != Id() ) { // is playing
					goto advance;
				}

				return 0; // end
			}
		}

	end:
		return surplus_time / _speed;
	}

	uint32_t SequenceAction::advance_Rt(uint32_t time_span, bool restart, Action *root) {
		time_span *= _speed; // Amplification time

		if ( _playIdx_Rt == nullId || restart ) { // no start play
			if ( restart ) { // restart
				_looped = 0;
				_playIdx_Rt = nullId;
			}

			if ( _actions_Rt.length() ) {
				restart = true;
				_playIdx_Rt = _actions_Rt.begin();
			} else {
				return time_span / _speed;
			}
		}

	advance:
		time_span = (*_playIdx_Rt)->advance_Rt(time_span, restart, root);

		if ( time_span ) {

			if ( _playIdx_Rt == nullId ) { // May have been deleted child action
				if ( _actions_Rt.length() ) { // Restart
					restart = true;
					_playIdx_Rt = _actions_Rt.begin();
					goto advance;
				}
			} else if ( *_playIdx_Rt == _actions_Rt.back() ) { // last action
				if ( _loop ) {
					if ( _loop > 0 ) {
						if ( _looped < _loop ) { // Can continue to loop
							_looped++;
						} else { //
							goto end;
						}
					}

					restart = true;

					trigger_ActionLoop_Rt(time_span, root); // trigger event
					_playIdx_Rt = _actions_Rt.begin();

					if ( _playIdx_Rt == _actions_Rt.end() ) {
						// May be deleted when triggering an `action_loop` event
						_playIdx_Rt = nullId;
						goto end; // No child action then end
					}

					if ( root->_id != Id() ) { // is playing
						goto advance;
					}
					return 0; // end
				}
			} else {
				restart = true;
				_playIdx_Rt++;
				goto advance;
			}
		}

	end:
		return time_span / _speed;
	}

}
