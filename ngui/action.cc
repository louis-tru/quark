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

#include "ngui/action.h"
#include "ngui/view.h"
#include "ngui/app.h"
#include "ngui/errno.h"

XX_NS(ngui)

XX_DEFINE_INLINE_MEMBERS(View, ActionInl) {
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
		if ( action->m_action_center_id.is_null() ) {
			action->m_action_center_id = m_actions.push({ action, 0 });
			action->retain();
		}
	}
	
	/**
	 * @func del
	 */
	void del(Action* action) {
		if ( action && !action->m_action_center_id.is_null() ) {
			action->m_action_center_id.value().value = nullptr; // del
			// m_actions.del(action->m_action_center_id);
			action->m_action_center_id = List<Action::Wrap>::Iterator();
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
		
		if ( m_parent || m_views.length() || !m_action_center_id.is_null() ) {
			XX_THROW(ERR_ACTION_ILLEGAL_CHILD, "illegal child action!");
		}
		
		retain(); // retain
		
		// bind view
		m_parent = parent;
		while ( parent->m_parent ) {
			parent = parent->m_parent;
		}
		
		View* first = first_view();
		
		if ( first ) {
			bind_view( first );
		}
	}
	
	View* first_view() {
		for ( auto& i : m_views ) {
			if (i.value()) {
				return i.value();
			}
		}
		return nullptr;
	}
	
	void clear_parent() {
		m_parent = nullptr;
		release();
	}
	
	/**
	 * @func view
	 */
	inline View* view() {
		Action* action = this;
		while ( action->m_parent ) {
			action = action->m_parent;
		}
		return first_view();
	}
	
	/**
	 * @func views
	 */
	inline List<View*>& views() {
		return m_views;
	}
	
	/**
	 * @func is_playing with root
	 */
	inline bool is_playing() {
		return ! m_action_center_id.is_null();
	}
	
	/**
	 * @func trigger_action_loop
	 */
	void trigger_action_loop(uint64 delay, Action* root) {
		for ( auto i = m_views.begin(); !i.is_null(); ) { // trigger event action_loop
			View* v = i.value();
			if (v) {
				auto evt = new GUIActionEvent(this, v, delay, 0, m_loop);
				main_loop()->post(Cb([this, evt, v](Cbd& e) {
					Handle<GUIActionEvent> handle(evt);
					ActionInl_View(v)->trigger(GUI_EVENT_ACTION_LOOP, *evt);
				}, v));
				i++;
			} else {
				m_views.del(i++);
			}
		}
	}
	
	/**
	 * @func trigger_action_key_frame
	 */
	void trigger_action_key_frame(uint64 delay, uint frame_index, Action* root) {
		for ( auto i = m_views.begin(); !i.is_null(); ) { // trigger event action_keyframe
			View* v = i.value();
			if (v) {
				auto evt = new GUIActionEvent(this, v, delay, frame_index, m_loop);
				main_loop()->post(Cb([this, evt, v](Cbd& e) {
					Handle<GUIActionEvent> handle(evt);
					ActionInl_View(v)->trigger(GUI_EVENT_ACTION_KEYFRAME, *evt);
				}, v));
				i++;
			} else {
				m_views.del(i++);
			}
		}
	}
	
	/**
	 * @func update_duration
	 */
	void update_duration(int64 difference) {
		
		Action* action = this;
		while(1) {
			action->m_full_duration += difference;
			action = m_parent;
			
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
		
		if ( m_parent ) {
			XX_THROW(ERR_ACTION_ILLEGAL_ROOT, "Cannot set non root action !");
		}
		View* first = first_view();
		if ( first ) {
			if ( first->view_type() != view->view_type() ) {
				XX_THROW(ERR_ACTION_ILLEGAL_VIEW_TYPE, "Action can only be bound to the same type of view !");
			}
		} else {
			bind_view(view);
		}
		m_views.push({view});
	}
	
	/**
	 * @func del_view
	 */
	void del_view(View* view) {
		uint len = m_views.length();
		for ( auto& i : m_views ) {
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

#include "action.cc.inl"

class Frame::Inl: public Frame {
 public:
 #define _inl_frame(self) static_cast<KeyframeAction::Frame::Inl*>(self)
	
	template<PropertyName Name, class T> inline T property_value() {
		typedef Property2<T> Type;
		auto it = m_host->m_property.find(Name);
		if (!it.is_null()) {
			return static_cast<Type*>(it.value())->frame(m_index);
		}
		return T();
	}
	
	template<PropertyName Name, class T>
	inline void set_property_value(T value) {
		Map<PropertyName, Property*>& property = m_host->m_property;
		typedef Property3<T, Name> Type;
		auto it = property.find(Name);
		if (it.is_null()) {
			Type* prop = new Type(m_host->length());
			property.set(Name, prop);
			prop->bind_view(m_host->m_bind_view_type);
			prop->frame(m_index, value);
		} else {
			static_cast<Type*>(it.value())->frame(m_index, value);
		}
	}
	
};

#define xx_def_property(ENUM, TYPE, NAME) \
TYPE Frame::NAME() { \
	return _inl_frame(this)->property_value<ENUM, TYPE>(); \
}\
void Frame::set_##NAME(TYPE value) { \
	_inl_frame(this)->set_property_value<ENUM>(value); \
}
XX_EACH_PROPERTY_TABLE(xx_def_property)
#undef xx_def_accessor

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
		
		for ( auto& i : m_actions ) {
			GroupAction* group = i.value()->as_group();
			if (group) {
				_inl_group_action(group)->clear_all();
				if ( group->as_sequence() ) {
					group->as_sequence()->m_action = Iterator();
				}
			}
			_inl_action(i.value())->clear_parent();
		}
		m_actions.clear();
		m_actions_index.clear();
		m_full_duration = 0;
		m_delay = 0;
	}
	
	/**
	 * @func m_remove
	 */
	uint64 m_remove(uint index) {
		Iterator it =
			m_actions_index.length() == m_actions.length() ?
			m_actions_index[index] : m_actions.find(index);
		uint64 duration = 0;
		if ( it != m_actions.end() ) {
			duration = it.value()->m_full_duration;
			_inl_action(it.value())->clear_parent();
			m_actions.del( it );
			m_actions_index.clear();
		}
		return duration;
	}
	
	/**
	 * @func update_spawn_action_duration()
	 */
	void update_spawn_action_duration() {
		int64 new_duration = 0;
		
		for ( auto& i : m_actions ) {
			new_duration = XX_MAX(i.value()->m_full_duration, new_duration);
		}
		new_duration += m_delay;
		
		if ( new_duration != m_full_duration ) {
			_inl_action(this)->update_duration( new_duration - m_full_duration );
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
	if ( m_host && m_index && value != m_time ) {
		uint next = m_index + 1;
		if ( next < m_host->length() ) {
			uint64 max_time = m_host->frame(next)->time();
			m_time = XX_MIN(value, max_time);
		} else { // no next
			m_time = value;
		}
	}
}

/**
 * @func fetch property
 */
void Frame::fetch(View* view) {
	if ( view && view->view_type() == m_host->m_bind_view_type ) {
		for ( auto& i : m_host->m_property ) {
			i.value()->fetch(m_index, view);
		}
	} else {
		view = _inl_action(m_host)->view();
		if ( view ) {
			for ( auto& i : m_host->m_property ) {
				i.value()->fetch(m_index, view);
			}
		}
	}
}

/**
 * @func flush recovery default property value
 */
void Frame::flush() {
	for ( auto& i : m_host->m_property ) {
		i.value()->default_value(m_index);
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
		for ( auto& i : m_property ) {
			i.value()->transition(f1, f2, x, y, root);
		}
	}
	
	/**
	 * @func transition
	 */
	inline void transition(uint f1, Action* root) {
		for ( auto& i : m_property ) {
			i.value()->transition(f1, root);
		}
	}
	
	/**
	 * @func advance
	 */
	uint64 advance(uint64 time_span, Action* root) {
		
	 start:
		
		uint f1 = m_frame;
		uint f2 = f1 + 1;
		
		if ( f2 < length() ) {
		 advance:
			
			if ( ! _inl_action(root)->is_playing() ) { // is playing
				return 0;
			}
			
			int64 time = m_time + time_span;
			int64 time1 = m_frames[f1]->time();
			int64 time2 = m_frames[f2]->time();
			int64 t = time - time2;
			
			if ( t < 0 ) {
				
				time_span = 0;
				m_time = time;
				float x = (time - time1) / float(time2 - time1);
				float y = m_frames[f1]->curve().solve(x, 0.001);
				transition(f1, f2, x, y, root);
				
			} else if ( t > 0 ) {
				time_span = t;
				m_frame = f2;
				m_time = time2;
				_inl_action(this)->trigger_action_key_frame(t, f2, root); // trigger event action_key_frame
				
				f1 = f2; f2++;
				
				if ( f2 < length() ) {
					goto advance;
				} else {
					if ( m_loop && m_full_duration > m_delay ) {
						goto loop;
					} else {
						transition(f1, root);
					}
				}
			} else { // t == 0
				time_span = 0;
				m_time = time;
				m_frame = f2;
				transition(f2, root);
				_inl_action(this)->trigger_action_key_frame(0, f2, root); // trigger event action_key_frame
			}
			
		} else { // last frame
			
			if ( m_loop && m_full_duration > m_delay ) {
			 loop:
				
				if ( m_loop > 0 ) {
					if ( m_loopd < m_loop ) { // 可经继续循环
						m_loopd++;
					} else { //
						transition(f1, root);
						goto end;
					}
				}
				
				m_frame = 0;
				m_time = 0;
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
		if ( m_action ) {
			_inl_action(m_action)->del_view(this);
			m_action->release();
		}
		_inl_action(action)->add_view(this);
		m_action = action;
		action->retain();
	} else {
		if ( m_action ) {
			_inl_action(m_action)->del_view(this);
			m_action->release();
			m_action = nullptr;
		}
	}
}

Action::Action()
: m_parent(nullptr)
, m_loop(0)
, m_loopd(0)
, m_full_duration(0)
, m_delay(0)
, m_delayd(-1), m_speed(1) { }

/**
 * @destructor
 */
Action::~Action() {
	XX_ASSERT( m_action_center_id.is_null() );
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
	int64 du = value - m_delay;
	if ( du ) {
		m_delay = value;
		_inl_action(this)->update_duration(du);
	}
}

/**
 * @func playing
 */
bool Action::playing() const {
	return m_parent ? m_parent->playing() : !m_action_center_id.is_null();
}

/**
 * @func play
 */
void Action::play() {
	if ( m_parent ) {
		m_parent->play();
	} else {
		if ( m_views.length() ) {
			_inl_action_center(ActionCenter::shared())->add(this);
		}
	}
}

/**
 * @func stop
 */
void Action::stop() {
	if ( m_parent ) {
		m_parent->stop();
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
	if ( m_actions_index.length() != m_actions.length() ) {
		m_actions_index = Array<Iterator>(m_actions.length());
		uint j = 0;
		for ( auto& i : m_actions ) {
			m_actions_index[j] = i;
			j++;
		}
	}
	return m_actions_index[index].value();
}

/**
 * @func append
 */
void GroupAction::append(Action* action) throw(Error) {
	XX_ASSERT(action);
	_inl_action(action)->set_parent(this);
	m_actions.push(action);
	m_actions_index.clear();

}

/**
 * @func insert
 */
void GroupAction::insert(uint index, Action* action) throw(Error) {
	XX_ASSERT(action);
	if ( index == 0 ) {
		_inl_action(action)->set_parent(this);
		m_actions.unshift(action);
		m_actions_index.clear();
	} else if ( index < m_actions.length() ) {
		_inl_action(action)->set_parent(this);
		if ( m_actions_index.length() == m_actions.length() ) {
			m_actions.after(m_actions_index[index - 1], action);
		} else {
			m_actions.after(m_actions.find(index - 1), action);
		}
		m_actions_index.clear();
	} else {
		append(action);
	}
}

void SpawnAction::append(Action* action) throw(Error) {
	GroupAction::append(action);
	int64 du = action->m_full_duration + m_delay;
	if ( du > m_full_duration ) {
		_inl_action(this)->update_duration( du - m_full_duration );
	}
}

void SpawnAction::insert(uint index, Action* action) throw(Error) {
	GroupAction::insert(index, action);
	int64 du = action->m_full_duration + m_delay;
	if ( du > m_full_duration ) {
		_inl_action(this)->update_duration( du - m_full_duration );
	}
}

void SequenceAction::append(Action* action) throw(Error) {
	GroupAction::append(action);
	if ( action->m_full_duration ) {
		_inl_action(this)->update_duration( action->m_full_duration );
	}
}

void SequenceAction::insert(uint index, Action* action) throw(Error) {
	GroupAction::insert(index, action);
	if ( action->m_full_duration ) {
		_inl_action(this)->update_duration( action->m_full_duration );
	}
}

/**
 * @func remove
 */
void GroupAction::remove_child(uint index) {
	_inl_group_action(this)->m_remove(index);
}

void SpawnAction::remove_child(uint index) {
	int64 duration = _inl_group_action(this)->m_remove(index) + m_delay;
	if ( duration == m_full_duration ) {
		_inl_group_action(this)->update_spawn_action_duration();
	}
}

void SequenceAction::remove_child(uint index) {
	Iterator it =
		m_actions_index.length() == m_actions.length() ?
		m_actions_index[index] : m_actions.find(index);
	if ( it != m_actions.end() ) {
		if ( it == m_action ) {
			m_action = Iterator();
		}
		uint64 duration = it.value()->m_full_duration;
		_inl_action(it.value())->clear_parent();
		m_actions.del( it );
		m_actions_index.clear();
		if ( duration ) {
			_inl_action(this)->update_duration(-duration);
		}
	}
}

void GroupAction::clear() {
	for ( auto& i : m_actions ) {
		_inl_action(i.value())->clear_parent();
	}
	m_actions.clear();
	m_actions_index.clear();
	if ( m_full_duration ) {
		_inl_action(this)->update_duration( m_delay - m_full_duration );
	}
}

void SequenceAction::clear() {
	GroupAction::clear();
	m_action = Iterator();
}

/**
 * @func seek
 */
void Action::seek(int64 time) {
	time += m_delay;
	time = XX_MIN(time, m_full_duration);
	time = XX_MAX(time, 0);
	if (m_parent) {
		m_parent->seek_before(time, this);
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
	time += m_delay;
	if (m_parent) {
		m_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void SequenceAction::seek_before(int64 time, Action* child) {
	time += m_delay;
	for ( auto& i : m_actions ) {
		if ( child == i.value() ) {
			break;
		} else {
			time += i.value()->m_full_duration;
		}
	}
	if (m_parent) {
		m_parent->seek_before(time, this);
	} else {
		seek_time(time, this);
	}
}

void KeyframeAction::seek_before(int64 time, Action* child) {
	XX_UNIMPLEMENTED();
}

void SpawnAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - m_delay;
	if ( t < 0 ) {
		m_delayd = time;
		return;
	} else {
		m_delayd = m_delay;
		time = t;
	}
	
	m_loopd = 0;// 重置循环
	
	for ( auto& i : m_actions ) {
		i.value()->seek_time(time, root);
	}
}

void SequenceAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - m_delay;
	if ( t < 0 ) {
		m_delayd = time;
		m_action = Iterator();
		return;
	} else {
		m_delayd = m_delay;
		time = t;
	}
	
	m_loopd = 0;// 重置循环
	
	uint64 duration = 0;
	
	for ( auto& i : m_actions ) {
		uint64 du = duration + i.value()->m_full_duration;
		if ( du > time ) {
			m_action = i;
			i.value()->seek_time(time - duration, root);
			return;
		}
		duration = du;
	}
	
	if ( length() ) {
		m_actions.last()->seek_time(time - duration, root);
	}
}

void KeyframeAction::seek_time(uint64 time, Action* root) {
	
	int64 t = time - m_delay;
	if ( t < 0 ) {
		m_delayd = time;
		m_frame = -1;
		m_time = 0; return;
	} else {
		m_delayd = m_delay;
		time = t;
	}
	
	m_loopd = 0;// 重置循环
	
	if ( length() ) {
		Frame* frame = nullptr;
		
		for ( auto& i: m_frames ) {
			if ( time < i.value()->time() ) {
				break;
			}
			frame = i.value();
		}
		
		m_frame = frame->index();
		m_time = XX_MIN(int64(time), m_full_duration - m_delay);
		
		uint f1 = m_frame;
		uint f2 = f1 + 1;
		
		if ( f2 < length() ) {
			int64 time1 = frame->time();
			int64 time2 = m_frames[f2]->time();
			float x = (m_time - time1) / float(time2 - time1);
			float t = frame->curve().solve(x, 0.001);
			_inl_key_action(this)->transition(f1, f2, x, t, root);
		} else { // last frame
			_inl_key_action(this)->transition(f1, root);
		}
		
		if ( m_time == int64(frame->time()) ) {
			_inl_action(this)->trigger_action_key_frame(0, m_frame, root);
		}
	}
}

uint64 SpawnAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= m_speed; // Amplification time
	
	if ( restart ) { // restart
		m_delayd = 0;
		m_loopd = 0;
	}
	
	if ( m_delay > m_delayd ) { // 需要延时
		int64 time = m_delay - m_delayd - time_span;
		if ( time >= 0 ) {
			m_delayd += time_span;
			return 0;
		} else {
			m_delayd = m_delay;
			time_span = -time;
		}
	}
	
	uint64 surplus_time = time_span;
	
 advance:
	
	for ( auto& i : m_actions ) {
		uint64 time = i.value()->advance(time_span, restart, root);
		surplus_time = XX_MIN(surplus_time, time);
	}
	
	if ( surplus_time ) {
		if ( m_loop && m_full_duration > m_delay ) {
			
			if ( m_loop > 0 ) {
				if ( m_loopd < m_loop ) { // 可经继续循环
					m_loopd++;
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
	return surplus_time / m_speed;
}

uint64 SequenceAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= m_speed; // Amplification time
	
	if ( m_action.is_null() || restart ) { // no start play
		
		if ( restart ) { // restart
			m_delayd = 0;
			m_loopd = 0;
			m_action = Iterator();
		}
		
		if ( m_delay > m_delayd ) { // 需要延时
			int64 time = m_delay - m_delayd - time_span;
			if ( time >= 0 ) {
				m_delayd += time_span;
				return 0;
			} else {
				m_delayd = m_delay;
				time_span = -time;
			}
		}
		
		if ( length() ) {
			restart = true;
			m_action = m_actions.begin();
		} else {
			return time_span / m_speed;
		}
	}
	
 advance:
	
	time_span = m_action.value()->advance(time_span, restart, root);
	
	if ( time_span ) {
		
		if ( m_action.is_null() ) { // May have been deleted
			if ( length() ) { // Restart
				restart = true;
				m_action = m_actions.begin();
				goto advance;
			}
		} else {
			if ( m_action.value() == m_actions.last() ) { // last action
				if ( m_loop && m_full_duration > m_delay ) {
					
					if ( m_loop > 0 ) {
						if ( m_loopd < m_loop ) { // 可经继续循环
							m_loopd++;
						} else { //
							goto end;
						}
					}
					
					restart = true;
					
					_inl_action(this)->trigger_action_loop(time_span, root); // trigger event
					m_action = m_actions.begin();
					
					if ( m_action.is_null() ) { // 可能在触发`action_loop`事件时被删除
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
				m_action++;
				goto advance;
			}
		}
	}
	
 end:
	return time_span / m_speed;
}

KeyframeAction::~KeyframeAction() {
	clear();
}

uint64 KeyframeAction::advance(uint64 time_span, bool restart, Action* root) {
	
	time_span *= m_speed;
	
	if ( m_frame == -1 || restart ) { // no start play
		
		if ( restart ) { // restart
			m_delayd = 0;
			m_loopd = 0;
			m_frame = -1;
			m_time = 0;
		}

		if ( m_delay > m_delayd ) { // 需要延时
			int64 time = m_delay - m_delayd - time_span;
			if ( time >= 0 ) {
				m_delayd += time_span;
				return 0;
			} else {
				m_delayd = m_delay;
				time_span = -time;
			}
		}
		
		if ( length() ) {
			m_frame = 0;
			m_time = 0;
			_inl_key_action(this)->transition(0, root);
			_inl_action(this)->trigger_action_key_frame(time_span, 0, root);
			
			if ( time_span == 0 ) {
				return 0;
			}
			
			if ( length() == 1 ) {
				return time_span / m_speed;
			}
		} else {
			return time_span / m_speed;
		}
	}
	
	return _inl_key_action(this)->advance(time_span, root) / m_speed;
}

void GroupAction::bind_view(View* view) {
	for ( auto& i : m_actions ) {
		i.value()->bind_view(view);
	}
}

void KeyframeAction::bind_view(View* view) {
	int view_type = view->view_type();
	if ( view_type != m_bind_view_type ) {
		m_bind_view_type = view_type;
		for ( auto& i : m_property ) {
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
	
	Frame* frame = new Frame(this, m_frames.length(), curve);
	m_frames.push(frame);
	frame->m_time = time;
	
	for ( auto& i : m_property ) {
		i.value()->add_frame();
	}
	
	return frame;
}

/**
 * @func clear all frame and property
 */
void KeyframeAction::clear() {
	
	for (auto& i : m_frames) {
		i.value()->m_host = nullptr;
		Release(i.value());
	}
	for (auto& i : m_property) {
		delete i.value();
	}
	m_frames.clear();
	m_property.clear();
	
	if ( m_full_duration ) {
		_inl_action(this)->update_duration( m_delay - m_full_duration );
	}
}

bool KeyframeAction::has_property(PropertyName name) {
	return m_property.has(name);
}

/**
 * @func match_property
 */
bool KeyframeAction::match_property(PropertyName name) {
	return PropertysAccessor::shared()->has_accessor(m_bind_view_type, name);
}
// ----------------------- ActionCenter -----------------------

static ActionCenter* action_center_shared = nullptr;

ActionCenter::ActionCenter()
: m_prev_sys_time(0) {
	XX_ASSERT(!action_center_shared); action_center_shared = this;
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
	if (len != m_actions.length()) {
		len = m_actions.length();
		LOG("ActionCenter::advance,length, %d", len);
	}*/
	
	if ( m_actions.length() ) { // run task
		int64 time_span = 0;
		if (m_prev_sys_time) {  // 0表示还没开始
			time_span = now_time - m_prev_sys_time;
			if ( time_span > 200000 ) {   // 距离上一帧超过200ms重新记时(如应用程序从休眠中恢复)
				time_span = 200000; // 100ms
			}
		}
		for ( auto i = m_actions.begin(); !i.is_null(); ) {
			Action::Wrap& wrap = i.value();
			if ( wrap.value ) {
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
				i++;
			} else {
				m_actions.del(i++);
			}
		}
		m_prev_sys_time = now_time;
	}
}

ActionCenter* ActionCenter::shared() {
	return action_center_shared;
}

XX_END
