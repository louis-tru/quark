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

namespace qk {

	GroupAction::~GroupAction() {
		_duration = 0;
		clear();
	}

	void GroupAction::clear() {
		for ( auto &i : _actions ) {
			i->del_parent();
		}
		_actions.clear();
		if ( _duration ) {
			GroupAction::update_duration( _duration );
		}
	}

	void SequenceAction::clear() {
		GroupAction::clear();
		_playIdx = Id();
	}

	void GroupAction::append(Action *child) throw(Error) {
		Qk_ASSERT(action);
		set_parent(this);
		child->_id = _actions.pushBack(child);
	}

	void GroupAction::insert(Id after, Action *child) throw(Error) {
		Qk_ASSERT(child);
		child->set_parent(this);
		child->_id = _actions.insert(after, child);
	}

	void SpawnAction::append(Action *child) throw(Error) {
		GroupAction::append(child);
		int32_t du = child->_duration;
		if ( du > _duration ) {
			Action::update_duration( du - _duration );
		}
	}

	void SpawnAction::insert(Id after, Action *child) throw(Error) {
		GroupAction::insert(after, child);
		int32_t du = child->_duration;
		if ( du > _duration ) {
			Action::update_duration( du - _duration );
		}
	}

	void SequenceAction::append(Action* child) throw(Error) {
		GroupAction::append(child);
		if ( child->_duration ) {
			Action::update_duration( child->_duration );
		}
	}

	void SequenceAction::insert(Id after, Action *child) throw(Error) {
		GroupAction::insert(after, child);
		if ( child->_duration ) {
			Action::update_duration( child->_duration );
		}
	}

	void SpawnAction::remove_child(Id id) {
		Qk_ASSERT(id != _actions.end());
		auto act = *id;
		act->del_parent();
		_actions.erase( id );
		act->_id = Id();
		if ( act->_duration == _duration ) {
			SpawnAction::update_duration(0);
		}
	}

	void SequenceAction::remove_child(Id id) {
		Qk_ASSERT(id != _actions.end());
		if ( id == _playIdx )
			_playIdx = Id();
		auto act = *id;
		act->del_parent();
		_actions.erase(id);
		act->_id = Id();
		if ( act->_duration ) {
			Action::update_duration(-act->_duration);
		}
	}

	void SpawnAction::seek_before(int32_t time, Action *child) {
		if (parent()) {
			parent()->seek_before(time, this);
		} else {
			seek_time(time, this);
		}
	}

	void SequenceAction::seek_before(int32_t time, Action *child) {
		for ( auto &i : _actions ) {
			if ( child == i ) {
				break;
			} else {
				time += i->_duration;
			}
		}
		if (_parent) {
			_parent->seek_before(time, this);
		} else {
			seek_time(time, this);
		}
	}

	void SpawnAction::seek_time(uint32_t time, Action *root) {
		_looped = 0;// 重置循环

		for ( auto i : _actions ) {
			i->seek_time(time, root);
		}
	}

	void SequenceAction::seek_time(uint32_t time, Action *root) {
		_looped = 0;// 重置循环

		uint32_t duration = 0;

		for ( auto i: _actions ) {
			uint32_t du = duration + i->_duration;
			if ( du > time ) {
				Qk_ASSERT(*i->_id == i);
				_playIdx = i->_id;
				i->seek_time(time - duration, root);
				return;
			}
			duration = du;
		}

		if ( length() ) {
			_actions.back()->seek_time(time - duration, root);
		}
	}

	uint32_t SpawnAction::advance(uint32_t time_span, bool restart, Action* root) {

		time_span *= _speed; // Amplification time

		if ( restart ) { // restart
			_looped = 0;
		}

		uint32_t surplus_time = time_span;

	advance:
		for ( auto i : _actions ) {
			uint32_t time = i->advance(time_span, restart, root);
			surplus_time = Qk_MIN(surplus_time, time);
		}

		if ( surplus_time ) {
			if ( _loop ) {
				if ( _loop > 0 ) {
					if ( _looped < _loop ) { // 可经继续循环
						_looped++;
					} else { //
						goto end;
					}
				}

				restart = true;
				time_span = surplus_time;

				trigger_action_loop(time_span, root);

				if ( root->_runAdvance ) {
					goto advance;
				}

				return 0; // end
			}
		}

	end:
		return surplus_time / _speed;
	}

	uint32_t SequenceAction::advance(uint32_t time_span, bool restart, Action *root) {

		time_span *= _speed; // Amplification time

		if ( _playIdx == Id() || restart ) { // no start play

			if ( restart ) { // restart
				_looped = 0;
				_playIdx = Id();
			}

			if ( length() ) {
				restart = true;
				_playIdx = _actions.begin();
			} else {
				return time_span / _speed;
			}
		}

	advance:
		time_span = (*_playIdx)->advance(time_span, restart, root);

		if ( time_span ) {

			if ( _playIdx == Id() ) { // May have been deleted
				if ( length() ) { // Restart
					restart = true;
					_playIdx = _actions.begin();
					goto advance;
				}
			} else if ( *_playIdx == _actions.back() ) { // last action
				if ( _loop ) {
					if ( _loop > 0 ) {
						if ( _looped < _loop ) { // 可经继续循环
							_looped++;
						} else { //
							goto end;
						}
					}

					restart = true;

					trigger_action_loop(time_span, root); // trigger event
					_playIdx = _actions.begin();

					if ( _playIdx == _actions.end() ) { // 可能在触发`action_loop`事件时被删除
						// 没有child action 无效,所以这里结束
						goto end;
					}

					if ( root->_runAdvance ) {
						goto advance;
					}
					return 0; // end
				}
			} else {
				restart = true;
				_playIdx++;
				goto advance;
			}
		}

	end:
		return time_span / _speed;
	}

}