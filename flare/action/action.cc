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

#include "./_action.h"
#include "../app.h"
#include "../errno.h"

namespace flare {

	void Action::Inl::set_parent(Action* parent) throw(Error) {
		
		if ( _parent || _views.length() ) {
			FX_THROW(ERR_ACTION_ILLEGAL_CHILD, "illegal child action!");
		}
		
		if (_action_center_id != ActionCenterId()) {
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
	
	View* Action::Inl::first_view() {
		for ( auto& i : _views ) {
			if (i) {
				return i;
			}
		}
		return nullptr;
	}
	
	void Action::Inl::clear_parent() {
		_parent = nullptr;
		release();
	}
	
	View* Action::Inl::view() {
		Action* action = this;
		while ( action->_parent ) {
			action = action->_parent;
		}
		return first_view();
	}
	
	List<View*>& Action::Inl::views() {
		return _views;
	}
	
	bool Action::Inl::is_playing() {
		return _action_center_id != ActionCenterId();
	}
	
	void Action::Inl::trigger_action_loop(uint64_t delay, Action* root) {
		auto i = _views.begin(), end = _views.end();
		while ( i != end ) { // trigger event action_loop
			View* v = *i;
			if (v) {
				auto evt = new GUIActionEvent(this, v, delay, 0, _loop);
				main_loop()->post(Cb([this, evt, v](CbData& e) {
					Handle<GUIActionEvent> handle(evt);
					ActionInl_View(v)->trigger(GUI_EVENT_ACTION_LOOP, *evt);
				}, v));
				i++;
			} else {
				_views.erase(i++);
			}
		}
	}
	
	void Action::Inl::trigger_action_key_frame(
		uint64_t delay, uint32_t frame_index, Action* root
	)
	{
		auto i = _views.begin(), end = _views.end();
		while ( i != end ) { // trigger event action_keyframe
			View* v = *i;
			if (v) {
				auto evt = new GUIActionEvent(this, v, delay, frame_index, _loop);
				main_loop()->post(Cb([this, evt, v](CbData& e) {
					Handle<GUIActionEvent> handle(evt);
					ActionInl_View(v)->trigger(GUI_EVENT_ACTION_KEYFRAME, *evt);
				}, v));
				i++;
			} else {
				_views.erase(i++);
			}
		}
	}
	
	void Action::Inl::update_duration(int64_t difference) {
		
		Action* action = this;
		while (1) {
			action->_full_duration += difference;
			action = _parent;
			
			if ( action ) {
				auto act = action->as_spawn();
				if ( act ) {
					update_spawn_action_duration(act);
					break;
				}
			} else {
				break;
			}
		}
	}
	
	void Action::Inl::add_view(View* view) throw(Error) {
		
		if ( _parent ) {
			FX_THROW(ERR_ACTION_ILLEGAL_ROOT, "Cannot set non root action !");
		}
		View* first = first_view();
		if ( first ) {
			if ( first->view_type() != view->view_type() ) {
				FX_THROW(ERR_ACTION_ILLEGAL_VIEW_TYPE,
					"Action can only be bound to the same type of view !");
			}
		} else {
			bind_view(view);
		}
		_views.push_back({view});
	}
	
	void Action::Inl::del_view(View* view) {
		auto len = _views.length();
		for ( auto& i : _views ) {
			if ( i == view ) {
				i = nullptr;
				len--;
				break;
			}
		}
		if ( len == 0 ) {
			stop(); // stop action
		}
	}

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
		, _delayd(-1), _speed(1)
	{}

	/**
	* @destructor
	*/
	Action::~Action() {
		ASSERT( _action_center_id == ActionCenterId() );
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
	void Action::delay(uint64_t value) {
		int64_t du = value - _delay;
		if ( du ) {
			_delay = value;
			_inl_action(this)->update_duration(du);
		}
	}

	/**
	* @func playing
	*/
	bool Action::playing() const {
		return _parent ? _parent->playing() : _action_center_id != ActionCenterId();
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
	* @func seek
	*/
	void Action::seek(int64_t time) {
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
	void Action::seek_play(int64_t time) {
		seek(time);
		play();
	}

	/**
	* @func seek_stop
	*/
	void Action::seek_stop(int64_t time) {
		seek(time);
		stop();
	}

}
