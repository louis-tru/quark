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

#include "ftr/action.h"
#include "ftr/view.h"
#include "ftr/app.h"
#include "ftr/errno.h"

FX_NS(ftr)

FX_DEFINE_INLINE_MEMBERS(View, ActionInl) {
 public:
	inline ReturnValue& trigger(const NameType& name, GUIEvent& evt) {
		return View::trigger(name, evt);
	}
};

/**
 * @class ActionCenter::Inl
 */
class ActionCenter::Inl: public ActionCenter {
 public:
 #define _inl_action_center(self) static_cast<ActionCenter::Inl*>(self)
	
	/**
	 * @func add
	 */
	void add(Action* action) {
		if ( action->_action_center_id.is_null() ) {
			action->_action_center_id = _actions.push({ action, 0 });
			action->retain();
		}
	}
	
	/**
	 * @func del
	 */
	void del(Action* action) {
		if ( action && !action->_action_center_id.is_null() ) {
			action->_action_center_id.value().value = nullptr; // del
			// _actions.del(action->_action_center_id);
			action->_action_center_id = List<Action::Wrap>::Iterator();
			action->release();
		}
	}
	
};

/**
 * @func __update_spawn_action_duration()
 */
inline void __update_spawn_action_duration(SpawnAction* act);

/**
 * @class Action::Inl
 */
class Action::Inl: public Action {
 public:
 #define _inl_action(self) static_cast<Action::Inl*>(static_cast<Action*>(self))
	
	void set_parent(Action* parent) throw(Error) {
		
		if ( _parent || _views.length() || !_action_center_id.is_null() ) {
			FX_THROW(ERR_ACTION_ILLEGAL_CHILD, "illegal child action!");
		}
		
		retain(); // retain
		
		// bind view
		_parent = parent;
		while ( parent->_parent ) {
			parent = parent->_parent;
		}
		
		View* first = first_view();
		
		if ( first ) {
			bind_view( first );
		}
	}
	
	View* first_view() {
		for ( auto& i : _views ) {
			if (i.value()) {
				return i.value();
			}
		}
		return nullptr;
	}
	
	void clear_parent() {
		_parent = nullptr;
		release();
	}
	
	/**
	 * @func view
	 */
	inline View* view() {
		Action* action = this;
		while ( action->_parent ) {
			action = action->_parent;
		}
		return first_view();
	}
	
	/**
	 * @func views
	 */
	inline List<View*>& views() {
		return _views;
	}
	
	/**
	 * @func is_playing with root
	 */
	inline bool is_playing() {
		return ! _action_center_id.is_null();
	}
	
	/**
	 * @func trigger_action_loop
	 */
	void trigger_action_loop(uint64 delay, Action* root) {
		for ( auto i = _views.begin(); !i.is_null(); ) { // trigger event action_loop
			View* v = i.value();
			if (v) {
				auto evt = new GUIActionEvent(this, v, delay, 0, _loop);
				main_loop()->post(Cb([this, evt, v](CbD& e) {
					Handle<GUIActionEvent> handle(evt);
					ActionInl_View(v)->trigger(GUI_EVENT_ACTION_LOOP, *evt);
				}, v));
				i++;
			} else {
				_views.del(i++);
			}
		}
	}
	
	/**
	 * @func trigger_action_key_frame
	 */
	void trigger_action_key_frame(uint64 delay, uint frame_index, Action* root) {
		for ( auto i = _views.begin(); !i.is_null(); ) { // trigger event action_keyframe
			View* v = i.value();
			if (v) {
				auto evt = new GUIActionEvent(this, v, delay, frame_index, _loop);
				main_loop()->post(Cb([this, evt, v](CbD& e) {
					Handle<GUIActionEvent> handle(evt);
					ActionInl_View(v)->trigger(GUI_EVENT_ACTION_KEYFRAME, *evt);
				}, v));
				i++;
			} else {
				_views.del(i++);
			}
		}
	}
	
	/**
	 * @func update_duration
	 */
	void update_duration(int64 difference) {
		
		Action* action = this;
		while(1) {
			action->_full_duration += difference;
			action = _parent;
			
			if ( action ) {
				auto act = action->as_spawn();
				if ( act ) {
					__update_spawn_action_duration(act);
					break;
				}
			} else {
				break;
			}
		}
	}
	
	/**
	 * @func add_view
	 */
	void add_view(View* view) throw(Error) {
		
		if ( _parent ) {
			FX_THROW(ERR_ACTION_ILLEGAL_ROOT, "Cannot set non root action !");
		}
		View* first = first_view();
		if ( first ) {
			if ( first->view_type() != view->view_type() ) {
				FX_THROW(ERR_ACTION_ILLEGAL_VIEW_TYPE, "Action can only be bound to the same type of view !");
			}
		} else {
			bind_view(view);
		}
		_views.push({view});
	}
	
	/**
	 * @func del_view
	 */
	void del_view(View* view) {
		uint len = _views.length();
		for ( auto& i : _views ) {
			if ( i.value() == view ) {
				i.value() = nullptr;
				len--;
				break;
			}
		}
		if ( len == 0 ) {
			stop(); // stop action
		}
	}
	
};

#include "action-property.cc"

class Frame::Inl: public Frame {
 public:
 #define _inl_frame(self) static_cast<KeyframeAction::Frame::Inl*>(self)
	
	template<PropertyName Name, class T> inline T property_value() {
		typedef Property2<T> Type;
		auto it = _host->_property.find(Name);
		if (!it.is_null()) {
			return static_cast<Type*>(it.value())->frame(_index);
		}
		return T();
	}
	
	template<PropertyName Name, class T>
	inline void set_property_value(T value) {
		Map<PropertyName, Property*>& property = _host->_property;
		typedef Property3<T, Name> Type;
		auto it = property.find(Name);
		if (it.is_null()) {
			Type* prop = new Type(_host->length());
			property.set(Name, prop);
			prop->bind_view(_host->_bind_view_type);
			prop->frame(_index, value);
		} else {
			static_cast<Type*>(it.value())->frame(_index, value);
		}
	}
	
};

#define nx_def_property(ENUM, TYPE, NAME) \
TYPE Frame::NAME() { \
	return _inl_frame(this)->property_value<ENUM, TYPE>(); \
}\
void Frame::set_##NAME(TYPE value) { \
	_inl_frame(this)->set_property_value<ENUM>(value); \
}
FX_EACH_PROPERTY_TABLE(nx_def_property)
#undef nx_def_accessor

/**
 * @func GroupAction::Inl
 */
class GroupAction::Inl: public GroupAction {
 public:
 #define _inl_group_action(self) static_cast<GroupAction::Inl*>(static_cast<GroupAction*>(self))
	
	/**
	 * @func clear_all
	 */
	void clear_all() {
		
		for ( auto& i : _actions ) {
			GroupAction* group = i.value()->as_group();
			if (group) {
				_inl_group_action(group)->clear_all();
				if ( group->as_sequence() ) {
					group->as_sequence()->_action = Iterator();
				}
			}
			_inl_action(i.value())->clear_parent();
		}
		_actions.clear();
		_actions_index.clear();
		_full_duration = 0;
		_delay = 0;
	}
	
	/**
	 * @func _remove
	 */
	uint64 _remove(uint index) {
		Iterator it =
			_actions_index.length() == _actions.length() ?
			_actions_index[index] : _actions.find(index);
		uint64 duration = 0;
		if ( it != _actions.end() ) {
			duration = it.value()->_full_duration;
			_inl_action(it.value())->clear_parent();
			_actions.del( it );
			_actions_index.clear();
		}
		return duration;
	}
	
	/**
	 * @func update_spawn_action_duration()
	 */
	void update_spawn_action_duration() {
		int64 new_duration = 0;
		
		for ( auto& i : _actions ) {
			new_duration = FX_MAX(i.value()->_full_duration, new_duration);
		}
		new_duration += _delay;
		
		if ( new_duration != _full_duration ) {
			_inl_action(this)->update_duration( new_duration - _full_duration );
		}
	}
	
};

void __update_spawn_action_duration(SpawnAction* act) {
	_inl_group_action(act)->update_spawn_action_duration();
}

/**
 * @func time set
 */
void Frame::set_time(uint64 value) {
	if ( _host && _index && value != _time ) {
		uint next = _index + 1;
		if ( next < _host->length() ) {
			uint64 max_time = _host->frame(next)->time();
			_time = FX_MIN(value, max_time);
		} else { // no next
			_time = value;
		}
	}
}

/**
 * @func fetch property
 */
void Frame::fetch(View* view) {
	if ( view && view->view_type() == _host->_bind_view_type ) {
		for ( auto& i : _host->_property ) {
			i.value()->fetch(_index, view);
		}
	} else {
		view = _inl_action(_host)->view();
		if ( view ) {
			for ( auto& i : _host->_property ) {
				i.value()->fetch(_index, view);
			}
		}
	}
}

/**
 * @func flush recovery default property value
 */
void Frame::flush() {
	for ( auto& i : _host->_property ) {
		i.value()->default_value(_index);
	}
}

/**
 * @class KeyframeAction::Inl
 */
class KeyframeAction::Inl: public KeyframeAction {
 public:
 #define _inl_key_action(self) static_cast<KeyframeAction::Inl*>(self)
	
	/**
	 * @func transition
	 */
	inline void transition(uint f1, uint f2, float x, float y, Action* root) {
		for ( auto& i : _property ) {
			i.value()->transition(f1, f2, x, y, root);
		}
	}
	
	/**
	 * @func transition
	 */
	inline void transition(uint f1, Action* root) {
		for ( auto& i : _property ) {
			i.value()->transition(f1, root);
		}
	}
	
	/**
	 * @func advance
	 */
	uint64 advance(uint64 time_span, Action* root) {
		
	 start:
		
		uint f1 = _frame;
		uint f2 = f1 + 1;
		
		if ( f2 < length() ) {
		 advance:
			
			if ( ! _inl_action(root)->is_playing() ) { // is playing
				return 0;
			}
			
			int64 time = _time + time_span;
			int64 time1 = _frames[f1]->time();
			int64 time2 = _frames[f2]->time();
			int64 t = time - time2;
			
			if ( t < 0 ) {
				
				time_span = 0;
				_time = time;
				float x = (time - time1) / float(time2 - time1);
				float y = _frames[f1]->curve().solve(x, 0.001);
				transition(f1, f2, x, y, root);
				
			} else if ( t > 0 ) {
				time_span = t;
				_frame = f2;
				_time = time2;
				_inl_action(this)->trigger_action_key_frame(t, f2, root); // trigger event action_key_frame
				
				f1 = f2; f2++;
				
				if ( f2 < length() ) {
					goto advance;
				} else {
					if ( _loop && _full_duration > _delay ) {
						goto loop;
					} else {
						transition(f1, root);
					}
				}
			} else { // t == 0
				time_span = 0;
				_time = time;
				_frame = f2;
				transition(f2, root);
				_inl_action(this)->trigger_action_key_frame(0, f2, root); // trigger event action_key_frame
			}
			
		} else { // last frame
			
			if ( _loop && _full_duration > _delay ) {
			 loop:
				
				if ( _loop > 0 ) {
					if ( _loopd < _loop ) { // 可经继续循环
						_loopd++;
					} else { //
						transition(f1, root);
						goto end;
					}
				}
				
				_frame = 0;
				_time = 0;
				_inl_action(this)->trigger_action_loop(time_span, root);
				_inl_action(this)->trigger_action_key_frame(time_span, 0, root);
				goto start;
			}
		}
		
	end:
		return time_span;
	}
	
};

/**
 * @func action
 */
void View::action(Action* action) throw(Error) {
	if ( action ) {
		if ( _action ) {
			_inl_action(_action)->del_view(this);
			_action->release();
		}
		_inl_action(action)->add_view(this);
		_action = action;
		action->retain();
	} else {
		if ( _action ) {
			_inl_action(_action)->del_view(this);
			_action->release();
			_action = nullptr;
		}
	}
}

Action::Action()
: _parent(nullptr)
, _loop(0)
, _loopd(0)
, _full_duration(0)
, _delay(0)
, _delayd(-1), _speed(1) { }

/**
 * @destructor
 */
Action::~Action() {
	ASSERT( _action_center_id.is_null() );
}

/**
 * @overwrite
 */
void Action::release() {
	if (ref_count() == 1) {
		clear();
	}
	Reference::release();
}

/**
 * @func delay
 */
void Action::delay(uint64 value) {
	int64 du = value - _delay;
	if ( du ) {
		_delay = value;
		_inl_action(this)->update_duration(du);
	}
}

/**
 * @func playing
 */
bool Action::playing() const {
	return _parent ? _parent->playing() : !_action_center_id.is_null();
}

/**
 * @func play
 */
void Action::play() {
	if ( _parent ) {
		_parent->play();
	} else {
		// if (_views.length()) // cancel limit
		_inl_action_center(ActionCenter::shared())->add(this);
	}
}

/**
 * @func stop
 */
void Action::stop() {
	if ( _parent ) {
		_parent->stop();
	} else {
		_inl_action_center(ActionCenter::shared())->del(this);
	}
}

/**
 * @func playing
 */
void Action::playing(bool value) {
	if ( value ) {
		play();
	} else {
		stop();
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
Action* GroupAction::operator[](uint index) {
	if ( _actions_index.length() != _actions.length() ) {
		_actions_index = Array<Iterator>(_actions.length());
		uint j = 0;
		for ( auto& i : _actions ) {
			_actions_index[j] = i;
			j++;
		}
	}
	return _actions_index[index].value();
}

/**
 * @func append
 */
void GroupAction::append(Action* action) throw(Error) {
	ASSERT(action);
	_inl_action(action)->set_parent(this);
	_actions.push(action);
	_actions_index.clear();

}

/**
 * @func insert
 */
void GroupAction::insert(uint index, Action* action) throw(Error) {
	ASSERT(action);
	if ( index == 0 ) {
		_inl_action(action)->set_parent(this);
		_actions.unshift(action);
		_actions_index.clear();
	} else if ( index < _actions.length() ) {
		_inl_action(action)->set_parent(this);
		if ( _actions_index.length() == _actions.length() ) {
			_actions.after(_actions_index[index - 1], action);
		} else {
			_actions.after(_actions.find(index - 1), action);
		}
		_actions_index.clear();
	} else {
		append(action);
	}
}

void SpawnAction::append(Action* action) throw(Error) {
	GroupAction::append(action);
	int64 du = action->_full_duration + _delay;
	if ( du > _full_duration ) {
		_inl_action(this)->update_duration( du - _full_duration );
	}
}

void SpawnAction::insert(uint index, Action* action) throw(Error) {
	GroupAction::insert(index, action);
	int64 du = action->_full_duration + _delay;
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

void SequenceAction::insert(uint index, Action* action) throw(Error) {
	GroupAction::insert(index, action);
	if ( action->_full_duration ) {
		_inl_action(this)->update_duration( action->_full_duration );
	}
}

/**
 * @func remove
 */
void GroupAction::remove_child(uint index) {
	_inl_group_action(this)->_remove(index);
}

void SpawnAction::remove_child(uint index) {
	int64 duration = _inl_group_action(this)->_remove(index) + _delay;
	if ( duration == _full_duration ) {
		_inl_group_action(this)->update_spawn_action_duration();
	}
}

void SequenceAction::remove_child(uint index) {
	Iterator it =
		_actions_index.length() == _actions.length() ?
		_actions_index[index] : _actions.find(index);
	if ( it != _actions.end() ) {
		if ( it == _action ) {
			_action = Iterator();
		}
		uint64 duration = it.value()->_full_duration;
		_inl_action(it.value())->clear_parent();
		_actions.del( it );
		_actions_index.clear();
		if ( duration ) {
			_inl_action(this)->update_duration(-duration);
		}
	}
}

void GroupAction::clear() {
	for ( auto& i : _actions ) {
		_inl_action(i.value())->clear_parent();
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

/**
 * @func seek
 */
void Action::seek(int64 time) {
	time += _delay;
	time = FX_MIN(time, _full_duration);
	time = FX_MAX(time, 0);
	if (_parent) {
		_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

/**
 * @func seek_play
 */
void Action::seek_play(int64 time) {
	seek(time);
	play();
}

/**
 * @func seek_stop
 */
void Action::seek_stop(int64 time) {
	seek(time);
	stop();
}

void SpawnAction::seek_before(int64 time, Action* child) {
	time += _delay;
	if (_parent) {
		_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void SequenceAction::seek_before(int64 time, Action* child) {
	time += _delay;
	for ( auto& i : _actions ) {
		if ( child == i.value() ) {
			break;
		} else {
			time += i.value()->_full_duration;
		}
	}
	if (_parent) {
		_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void KeyframeAction::seek_before(int64 time, Action* child) {
	FX_UNIMPLEMENTED();
}

void SpawnAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - _delay;
	if ( t < 0 ) {
		_delayd = time;
		return;
	} else {
		_delayd = _delay;
		time = t;
	}
	
	_loopd = 0;// 重置循环
	
	for ( auto& i : _actions ) {
		i.value()->seek_time(time, root);
	}
}

void SequenceAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - _delay;
	if ( t < 0 ) {
		_delayd = time;
		_action = Iterator();
		return;
	} else {
		_delayd = _delay;
		time = t;
	}
	
	_loopd = 0;// 重置循环
	
	uint64 duration = 0;
	
	for ( auto& i : _actions ) {
		uint64 du = duration + i.value()->_full_duration;
		if ( du > time ) {
			_action = i;
			i.value()->seek_time(time - duration, root);
			return;
		}
		duration = du;
	}
	
	if ( length() ) {
		_actions.last()->seek_time(time - duration, root);
	}
}

void KeyframeAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - _delay;
	if ( t < 0 ) {
		_delayd = time;
		_frame = -1;
		_time = 0; return;
	} else {
		_delayd = _delay;
		time = t;
	}
	
	_loopd = 0;// 重置循环
	
	if ( length() ) {
		Frame* frame = nullptr;
		
		for ( auto& i: _frames ) {
			if ( time < i.value()->time() ) {
				break;
			}
			frame = i.value();
		}
		
		_frame = frame->index();
		_time = FX_MIN(int64(time), _full_duration - _delay);
		
		uint f1 = _frame;
		uint f2 = f1 + 1;
		
		if ( f2 < length() ) {
			int64 time1 = frame->time();
			int64 time2 = _frames[f2]->time();
			float x = (_time - time1) / float(time2 - time1);
			float t = frame->curve().solve(x, 0.001);
			_inl_key_action(this)->transition(f1, f2, x, t, root);
		} else { // last frame
			_inl_key_action(this)->transition(f1, root);
		}
		
		if ( _time == int64(frame->time()) ) {
			_inl_action(this)->trigger_action_key_frame(0, _frame, root);
		}
	}
}

uint64 SpawnAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= _speed; // Amplification time
	
	if ( restart ) { // restart
		_delayd = 0;
		_loopd = 0;
	}
	
	if ( _delay > _delayd ) { // 需要延时
		int64 time = _delay - _delayd - time_span;
		if ( time >= 0 ) {
			_delayd += time_span;
			return 0;
		} else {
			_delayd = _delay;
			time_span = -time;
		}
	}
	
	uint64 surplus_time = time_span;
	
 advance:
	
	for ( auto& i : _actions ) {
		uint64 time = i.value()->advance(time_span, restart, root);
		surplus_time = FX_MIN(surplus_time, time);
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

uint64 SequenceAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= _speed; // Amplification time
	
	if ( _action.is_null() || restart ) { // no start play
		
		if ( restart ) { // restart
			_delayd = 0;
			_loopd = 0;
			_action = Iterator();
		}
		
		if ( _delay > _delayd ) { // 需要延时
			int64 time = _delay - _delayd - time_span;
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
	
	time_span = _action.value()->advance(time_span, restart, root);
	
	if ( time_span ) {
		
		if ( _action.is_null() ) { // May have been deleted
			if ( length() ) { // Restart
				restart = true;
				_action = _actions.begin();
				goto advance;
			}
		} else {
			if ( _action.value() == _actions.last() ) { // last action
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
					
					if ( _action.is_null() ) { // 可能在触发`action_loop`事件时被删除
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

KeyframeAction::~KeyframeAction() {
	clear();
}

uint64 KeyframeAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= _speed;
	
	if ( _frame == -1 || restart ) { // no start play
		
		if ( restart ) { // restart
			_delayd = 0;
			_loopd = 0;
			_frame = -1;
			_time = 0;
		}

		if ( _delay > _delayd ) { // 需要延时
			int64 time = _delay - _delayd - time_span;
			if ( time >= 0 ) {
				_delayd += time_span;
				return 0;
			} else {
				_delayd = _delay;
				time_span = -time;
			}
		}
		
		if ( length() ) {
			_frame = 0;
			_time = 0;
			_inl_key_action(this)->transition(0, root);
			_inl_action(this)->trigger_action_key_frame(time_span, 0, root);
			
			if ( time_span == 0 ) {
				return 0;
			}
			
			if ( length() == 1 ) {
				return time_span / _speed;
			}
		} else {
			return time_span / _speed;
		}
	}
	
	return _inl_key_action(this)->advance(time_span, root) / _speed;
}

void GroupAction::bind_view(View* view) {
	for ( auto& i : _actions ) {
		i.value()->bind_view(view);
	}
}

void KeyframeAction::bind_view(View* view) {
	int view_type = view->view_type();
	if ( view_type != _bind_view_type ) {
		_bind_view_type = view_type;
		for ( auto& i : _property ) {
			i.value()->bind_view(view_type);
		}
	}
}

/**
 * @func add new frame
 */
Frame* KeyframeAction::add(uint64 time, const FixedCubicBezier& curve) {
	
	if ( length() ) {
		Frame* frame = last();
		int64 duration = time - frame->time();
		if ( duration <= 0 ) {
			time = frame->time();
		} else {
			_inl_action(this)->update_duration(duration);
		}
	} else {
		time = 0;
	}
	
	Frame* frame = new Frame(this, _frames.length(), curve);
	_frames.push(frame);
	frame->_time = time;
	
	for ( auto& i : _property ) {
		i.value()->add_frame();
	}
	
	return frame;
}

/**
 * @func clear all frame and property
 */
void KeyframeAction::clear() {
	
	for (auto& i : _frames) {
		i.value()->_host = nullptr;
		Release(i.value());
	}
	for (auto& i : _property) {
		delete i.value();
	}
	_frames.clear();
	_property.clear();
	
	if ( _full_duration ) {
		_inl_action(this)->update_duration( _delay - _full_duration );
	}
}

bool KeyframeAction::has_property(PropertyName name) {
	return _property.has(name);
}

/**
 * @func match_property
 */
bool KeyframeAction::match_property(PropertyName name) {
	return PropertysAccessor::shared()->has_accessor(_bind_view_type, name);
}
// ----------------------- ActionCenter -----------------------

static ActionCenter* action_center_shared = nullptr;

ActionCenter::ActionCenter()
: _prev_sys_time(0) {
	ASSERT(!action_center_shared); action_center_shared = this;
}

ActionCenter::~ActionCenter() {
	action_center_shared = nullptr;
}

/**
 * @func advance
 */
void ActionCenter::advance(int64 now_time) {
	/*
	static int len = 0;
	if (len != _actions.length()) {
		len = _actions.length();
		LOG("ActionCenter::advance,length, %d", len);
	}*/
	
	if ( _actions.length() ) { // run task
		int64 time_span = 0;
		if (_prev_sys_time) {  // 0表示还没开始
			time_span = now_time - _prev_sys_time;
			if ( time_span > 200000 ) {   // 距离上一帧超过200ms重新记时(如应用程序从休眠中恢复)
				time_span = 200000; // 100ms
			}
		}
		for ( auto i = _actions.begin(); !i.is_null(); ) {
			Action::Wrap& wrap = i.value();
			if ( wrap.value ) {
				if (wrap.value->_views.length()) {
					if (wrap.play) {
						if ( wrap.value->advance(time_span, false, wrap.value) ) {
							// 不能消耗所有时间表示动作已经结束
							// end action play
							_inl_action_center(this)->del(wrap.value);
						}
					} else {
						wrap.play = true;
						wrap.value->advance(0, false, wrap.value);
					}
				} else {
					_inl_action_center(this)->del(wrap.value);
					_actions.del(i);
				}
				i++;
			} else {
				_actions.del(i++);
			}
		}
		_prev_sys_time = now_time;
	}
}

ActionCenter* ActionCenter::shared() {
	return action_center_shared;
}

FX_END
