/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./action.inl"

N_NAMESPACE_START

void update_spawn_action_duration(SpawnAction* act) {
	_inl_group_action(act)->update_spawn_action_duration();
}

void GroupAction::Inl::clear_all() {
	
	for ( auto& i : _actions ) {
		GroupAction* group = i->as_group();
		if (group) {
			_inl_group_action(group)->clear_all();
			if ( group->as_sequence() ) {
				group->as_sequence()->_action = Iterator();
			}
		}
		_inl_action(i)->clear_parent();
	}
	_actions.clear();
	_actions_index.clear();
	_full_duration = 0;
	_delay = 0;
}

uint64_t GroupAction::Inl::_remove(uint32_t index) {
	Iterator it =
		_actions_index.length() == _actions.length() ?
		_actions_index[index]: _actions.indexed(_actions.begin(), index);
	uint64_t duration = 0;
	if ( it != _actions.end() ) {
		duration = (*it)->_full_duration;
		_inl_action(*it)->clear_parent();
		_actions.erase( it );
		_actions_index.clear();
	}
	return duration;
}

void GroupAction::Inl::update_spawn_action_duration() {
	int64_t new_duration = 0;
	
	for ( auto& i : _actions ) {
		new_duration = N_MAX(i->_full_duration, new_duration);
	}
	new_duration += _delay;
	
	if ( new_duration != _full_duration ) {
		_inl_action(this)->update_duration( new_duration - _full_duration );
	}
}

/**
* @destructor
*/
GroupAction::~GroupAction() {
	_inl_group_action(this)->clear_all();
}

/**
* @func operator[]
*/
Action* GroupAction::operator[](uint32_t index) {
	if ( _actions_index.length() != _actions.length() ) {
		_actions_index.clear();
		auto i = _actions.begin(), e = _actions.end();
		while ( i != e ) {
			_actions_index.push(i);
		}
	}
	return *_actions_index[index];
}

/**
 * @func append
 */
void GroupAction::append(Action* action) throw(Error) {
	N_ASSERT(action);
	_inl_action(action)->set_parent(this);
	_actions.push_back(action);
	_actions_index.clear();
}

/**
* @func insert
*/
void GroupAction::insert(uint32_t index, Action* action) throw(Error) {
	N_ASSERT(action);
	
	if (index < _actions_index.length()) {
		_actions.insert(_actions_index[index], action);
	} else {
		//
	}
	
	_actions_index.clear();
	
	if ( index == 0 ) {
		_inl_action(action)->set_parent(this);
		_actions.push_front(action);
		_actions_index.clear();
	} else if ( index < _actions.length() ) {
		_inl_action(action)->set_parent(this);
		if ( _actions_index.length() == _actions.length() ) {
			// _actions.after(_actions_index[index - 1], action);
			// _actions.insert(_actions_index[index - 1] + 1, action);
		} else {
			// _actions.after(_actions.indexed(_actions.begin(), index - 1), action);
		}
		_actions_index.clear();
	} else {
		append(action);
	}
}

void SpawnAction::append(Action* action) throw(Error) {
	GroupAction::append(action);
	int64_t du = action->_full_duration + _delay;
	if ( du > _full_duration ) {
		_inl_action(this)->update_duration( du - _full_duration );
	}
}

void SpawnAction::insert(uint32_t index, Action* action) throw(Error) {
	GroupAction::insert(index, action);
	int64_t du = action->_full_duration + _delay;
	if ( du > _full_duration ) {
		_inl_action(this)->update_duration( du - _full_duration );
	}
}

void SequenceAction::append(Action* action) throw(Error) {
	GroupAction::append(action);
	if ( action->_full_duration ) {
		_inl_action(this)->update_duration( action->_full_duration );
	}
}

void SequenceAction::insert(uint32_t index, Action* action) throw(Error) {
	GroupAction::insert(index, action);
	if ( action->_full_duration ) {
		_inl_action(this)->update_duration( action->_full_duration );
	}
}

/**
* @func remove
*/
void GroupAction::remove_child(uint32_t index) {
	_inl_group_action(this)->_remove(index);
}

void SpawnAction::remove_child(uint32_t index) {
	int64_t duration = _inl_group_action(this)->_remove(index) + _delay;
	if ( duration == _full_duration ) {
		_inl_group_action(this)->update_spawn_action_duration();
	}
}

void SequenceAction::remove_child(uint32_t index) {
	Iterator it =
		_actions_index.length() == _actions.length() ?
		_actions_index[index] : _actions.indexed(_actions.begin(), index);
	if ( it != _actions.end() ) {
		if ( it == _action ) {
			_action = Iterator();
		}
		uint64_t duration = (*it)->_full_duration;
		_inl_action(*it)->clear_parent();
		_actions.erase(it);
		_actions_index.clear();
		if ( duration ) {
			_inl_action(this)->update_duration(-duration);
		}
	}
}

void GroupAction::clear() {
	for ( auto& i : _actions ) {
		_inl_action(i)->clear_parent();
	}
	_actions.clear();
	_actions_index.clear();
	if ( _full_duration ) {
		_inl_action(this)->update_duration( _delay - _full_duration );
	}
}

void SequenceAction::clear() {
	GroupAction::clear();
	_action = Iterator();
}

void GroupAction::bind_view(View* view) {
	for ( auto& i : _actions ) {
		i->bind_view(view);
	}
}

void SpawnAction::seek_before(int64_t time, Action* child) {
	time += _delay;
	if (_parent) {
		_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void SequenceAction::seek_before(int64_t time, Action* child) {
	time += _delay;
	for ( auto& i : _actions ) {
		if ( child == i ) {
			break;
		} else {
			time += i->_full_duration;
		}
	}
	if (_parent) {
		_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void SpawnAction::seek_time(uint64_t time, Action* root) {
	
	int64_t t = time - _delay;
	if ( t < 0 ) {
		_delayd = time;
		return;
	} else {
		_delayd = _delay;
		time = t;
	}
	
	_loopd = 0;// 重置循环
	
	for ( auto& i : _actions ) {
		i->seek_time(time, root);
	}
}

void SequenceAction::seek_time(uint64_t time, Action* root) {
	
	int64_t t = time - _delay;
	if ( t < 0 ) {
		_delayd = time;
		_action = Iterator();
		return;
	} else {
		_delayd = _delay;
		time = t;
	}
	
	_loopd = 0;// 重置循环
	
	uint64_t duration = 0;
	
	for ( auto i = _actions.begin(), e = _actions.end(); i != e; i++ ) {
		uint64_t du = duration + (*i)->_full_duration;
		if ( du > time ) {
			_action = i;
			(*i)->seek_time(time - duration, root);
			return;
		}
		duration = du;
	}
	
	if ( length() ) {
		_actions.back()->seek_time(time - duration, root);
	}
}

uint64_t SpawnAction::advance(uint64_t time_span, bool restart, Action* root) {
	
	time_span *= _speed; // Amplification time
	
	if ( restart ) { // restart
		_delayd = 0;
		_loopd = 0;
	}
	
	if ( _delay > _delayd ) { // 需要延时
		int64_t time = _delay - _delayd - time_span;
		if ( time >= 0 ) {
			_delayd += time_span;
			return 0;
		} else {
			_delayd = _delay;
			time_span = -time;
		}
	}
	
	uint64_t surplus_time = time_span;
	
	advance:
	
	for ( auto& i : _actions ) {
		uint64_t time = i->advance(time_span, restart, root);
		surplus_time = N_MIN(surplus_time, time);
	}
	
	if ( surplus_time ) {
		if ( _loop && _full_duration > _delay ) {
			
			if ( _loop > 0 ) {
				if ( _loopd < _loop ) { // 可经继续循环
					_loopd++;
				} else { //
					goto end;
				}
			}
			
			restart = true;
			time_span = surplus_time;
			
			_inl_action(this)->trigger_action_loop(time_span, root);
			
			if ( _inl_action(root)->is_playing() ) {
				goto advance;
			}
			
			return 0; // end
		}
	}
	
	end:
	return surplus_time / _speed;
}

uint64_t SequenceAction::advance(uint64_t time_span, bool restart, Action* root) {
	
	time_span *= _speed; // Amplification time
	
	if ( _action.is_null() || restart ) { // no start play
		
		if ( restart ) { // restart
			_delayd = 0;
			_loopd = 0;
			_action = Iterator();
		}
		
		if ( _delay > _delayd ) { // 需要延时
			int64_t time = _delay - _delayd - time_span;
			if ( time >= 0 ) {
				_delayd += time_span;
				return 0;
			} else {
				_delayd = _delay;
				time_span = -time;
			}
		}
		
		if ( length() ) {
			restart = true;
			_action = _actions.begin();
		} else {
			return time_span / _speed;
		}
	}
	
	advance:
	
	time_span = (*_action)->advance(time_span, restart, root);
	
	if ( time_span ) {
		
		if ( _action.is_null() ) { // May have been deleted
			if ( length() ) { // Restart
				restart = true;
				_action = _actions.begin();
				goto advance;
			}
		} else {
			if ( *_action == _actions.back() ) { // last action
				if ( _loop && _full_duration > _delay ) {
					
					if ( _loop > 0 ) {
						if ( _loopd < _loop ) { // 可经继续循环
							_loopd++;
						} else { //
							goto end;
						}
					}
					
					restart = true;
					
					_inl_action(this)->trigger_action_loop(time_span, root); // trigger event
					_action = _actions.begin();
					
					if ( _action == _actions.end() ) { // 可能在触发`action_loop`事件时被删除
						// 没有child action 无效,所以这里结束
						goto end;
					}
					
					if ( _inl_action(root)->is_playing() ) {
						goto advance;
					}
					return 0; // end
				}
			} else {
				restart = true;
				_action++;
				goto advance;
			}
		}
	}
	
	end:
	return time_span / _speed;
}

N_NAMESPACE_END