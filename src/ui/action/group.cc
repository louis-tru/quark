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

	ActionGroup::ActionGroup(Window *win): Action(win){}
	SpawnAction::SpawnAction(Window *win): ActionGroup(win){}
	SequenceAction::SequenceAction(Window *win): ActionGroup(win) {}

	void ActionGroup::clear() {
		if (_actions.length()) {
			Qk_Assert(_window);
			for ( auto &i : _actions ) {
				i.key->del_parent(); // release for main thread
			}
			_actions.clear();

			_async_call([](auto self, auto arg) {
				if (self->isSequence())
					static_cast<SequenceAction*>(self)->_play_Rt = nullId;
				for ( auto i : self->_actions_Rt ) {
					i->_id = nullId; // clear id
				}
				self->_actions_Rt.clear();
			}, this, 0);
		}
		if ( _duration ) {
			ActionGroup::setDuration( _duration );
		}
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
			arg.arg->child->_id = self->_actions_Rt.insert(arg.arg->after, arg.arg->child);
			free(arg.arg);
		}, this, new InsertArg{after,child});
	}

	void SpawnAction::insertChild(Id after, Action *child) {
		ActionGroup::insertChild(after, child);
		int32_t du = child->_duration;
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
		Qk_ASSERT(id != _actions_Rt.end());
		_async_call([](auto self, auto arg) {
			self->_actions_Rt.erase( arg.arg );
			(*arg.arg)->_id = nullId;
		}, this, id);
		auto act = *id;
		_actions.erase( act );
		if ( act->_duration == _duration ) {
			SpawnAction::setDuration(0);
		}
	}

	void SequenceAction::removeChild(Id id) {
		Qk_ASSERT(id != _actions_Rt.end());
		_async_call([](auto self, auto arg) {
			if ( self->_play_Rt == arg.arg )
				self->_play_Rt = nullId;
			self->_actions_Rt.erase( arg.arg );
			(*arg.arg)->_id = nullId;
		}, this, id);
		auto act = *id;
		_actions.erase( act );
		if ( act->_duration ) {
			Action::setDuration(-act->_duration);
		}
	}

	void SpawnAction::seek_before_Rt(uint32_t time, Action *child) {
		auto parent = dynamic_cast<ActionGroup*>(_parent); // safe use parent ptr
		if (parent) {
			parent->seek_before_Rt(time, this);
		} else {
			seek_time_Rt(time, this);
		}
	}

	void SpawnAction::seek_time_Rt(uint32_t time, Action *root) {
		for ( auto i : _actions_Rt ) {
			i->seek_time_Rt(time, root);
		}
	}

	void SequenceAction::seek_before_Rt(uint32_t time, Action *child) {
		for ( auto i : _actions_Rt ) {
			if ( child == i ) {
				break;
			} else {
				time += i->_duration; // add time
			}
		}
		auto parent = dynamic_cast<ActionGroup*>(_parent); // safe use parent ptr
		if (parent) {
			parent->seek_before_Rt(time, this);
		} else {
			seek_time_Rt(time, this);
		}
	}

	void SequenceAction::seek_time_Rt(uint32_t time, Action *root) {
		uint32_t duration = 0;

		for ( auto i: _actions_Rt ) {
			uint32_t du = duration + i->_duration;
			if ( du > time ) {
				Qk_ASSERT(*i->_id == i);
				_play_Rt = i->_id;
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

		uint32_t time_span_min = time_span;
	advance:
		for ( auto i : _actions_Rt ) {
			uint32_t time = i->advance_Rt(time_span, restart, root);
			time_span_min = Qk_MIN(time_span_min, time);
		}

		if ( time_span_min ) {
			if ( _looped < _loop ) { // continue to loop
				_looped++;
				restart = true;
				time_span = time_span_min;

				trigger_ActionLoop_Rt(time_span, root);

				if ( root->_id != Id() ) { // is playing
					goto advance;
				}
				return 0; // end
			}
		}

	end:
		return time_span_min / _speed;
	}

	uint32_t SequenceAction::advance_Rt(uint32_t time_span, bool restart, Action *root) {
		time_span *= _speed; // Amplification time

		if ( restart ) { // restart
			_looped = 0;
			_play_Rt = nullId;
		}
		if ( _play_Rt == nullId ) { // restart
			if ( _actions_Rt.length() ) {
				restart = true;
				_play_Rt = _actions_Rt.begin();
			} else {
				return time_span / _speed;
			}
		}

	advance:
		time_span = (*_play_Rt)->advance_Rt(time_span, restart, root);

		if ( time_span ) {
			if ( _play_Rt == nullId ) { // May have been deleted child action
				if ( _actions_Rt.length() ) { // Restart
					restart = true;
					_play_Rt = _actions_Rt.begin();
					goto advance;
				}
			} else if ( *_play_Rt == _actions_Rt.back() ) { // last action
				if ( _looped < _loop ) { // continue to loop
					_looped++;
					restart = true;

					trigger_ActionLoop_Rt(time_span, root); // trigger event
					_play_Rt = _actions_Rt.begin();

					if ( _play_Rt == _actions_Rt.end() ) {
						// May be deleted when triggering an `action_loop` event
						_play_Rt = nullId;
						goto end; // No child action then end
					}

					if ( root->_id != Id() ) { // is playing
						goto advance;
					}
					return 0; // end
				}
			} else {
				restart = true;
				_play_Rt++;
				goto advance;
			}
		}

	end:
		return time_span / _speed;
	}

}
