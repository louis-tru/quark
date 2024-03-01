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

#include "./keyframe.h"
#include "../../errno.h"

namespace qk {

	typedef Keyframe Frame;

	Frame::Keyframe(KeyframeAction* host, cCurve& curve)
		: _host(host) , _index(0), _curve(curve), _time(0)
	{}

	KeyframeAction::KeyframeAction(Window *win): Action(win), _frame(-1), _time(0)
	{}

	KeyframeAction::~KeyframeAction() {
		clear_RT();
	}

	Frame* KeyframeAction::add(uint32_t time, cCurve& curve) {
		auto frame = new Frame(this, curve);
		auto len = _frames_RT.length();
		frame->_index = len;
		frame->_time = time;

		if ( len ) {
			Frame* lastFrame = last();
			int32_t d = time - lastFrame->time();
			if ( d <= 0 ) {
				time = lastFrame->time();
			} else {
				Action::update_duration_RT(d);
			}
			frame->_time = time;
			// copy prop
			for (auto &i: lastFrame->_props) {
				frame->_props.set(i.key, i.value->copy());
			}
		}
		_frames_RT.push(frame);
		return frame;
	}

	void Keyframe::onMake(ViewProp key, Property* prop) {
		if (_host) {
			for (auto i: _host->_frames_RT) {
				if (i != this) {
					i->_props.set(key, prop->copy());
				}
			}
		}
	}

	void KeyframeAction::clear_RT() {
		for (auto i : _frames_RT) {
			i->_host = nullptr;
			i->release();
		}
		_frames_RT.clear();

		if ( _duration ) {
			Action::update_duration_RT( -_duration );
		}
	}

	bool KeyframeAction::has_property(ViewProp name) {
		return _frames_RT.length() && _frames_RT.front()->has_property(name);
	}

	uint32_t KeyframeAction::advance_RT(uint32_t time_span, bool restart, Action* root) {
		time_span *= _speed;

		if ( _frame == -1 || restart ) { // no start play
			if ( restart ) { // restart
				_looped = 0;
				_frame = -1;
				_time = 0;
			}

			if ( length() ) {
				_frame = 0;
				_time = 0;
				_frames_RT[0]->apply(root->_target);
				trigger_ActionKeyframe_RT(time_span, 0, root);

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

	start:
		uint32_t f1 = _frame;
		uint32_t f2 = f1 + 1;
		
		if ( f2 < length() ) {
		advance:
			if ( root->_id == Id() ) { // is not playing
				return 0;
			}

			int32_t time = _time + time_span;
			int32_t time1 = _frames_RT[f1]->time();
			int32_t time2 = _frames_RT[f2]->time();
			int32_t t = time - time2;

			if ( t < 0 ) {
				time_span = 0;
				_time = time;
				float x = (time - time1) / float(time2 - time1);
				float y = _frames_RT[f1]->curve().fixed_solve_y(x, 0.001);
				_frames_RT[f1]->applyTransition(root->_target, _frames_RT[f2], y);
			} else if ( t > 0 ) {
				time_span = t;
				_frame = f2;
				_time = time2;
				trigger_ActionKeyframe_RT(t, f2, root); // trigger event action_key_frame

				f1 = f2; f2++;

				if ( f2 < length() ) {
					goto advance;
				} else {
					if ( _loop ) {
						goto loop;
					} else {
						_frames_RT[f1]->apply(root->_target);
					}
				}
			} else { // t == 0
				time_span = 0;
				_time = time;
				_frame = f2;
				_frames_RT[f2]->apply(root->_target);
				trigger_ActionKeyframe_RT(0, f2, root); // trigger event action_key_frame
			}

		} else { // last frame

			if ( _loop ) {
			loop:
				if ( _loop > 0 ) {
					if ( _looped < _loop ) { // Can continue to loop
						_looped++;
					} else { //
						_frames_RT[f1]->apply(root->_target);
						goto end;
					}
				}
				_frame = 0;
				_time = 0;
				trigger_ActionLoop_RT(time_span, root);
				trigger_ActionKeyframe_RT(time_span, 0, root);
				goto start;
			}
		}

	end:
		return time_span / _speed;
	}

	void KeyframeAction::seek_time_RT(uint32_t time, Action* root) {
		_looped = 0;

		if ( length() ) {
			Frame* frame = nullptr;

			for ( auto& i: _frames_RT ) {
				if ( time < i->time() ) {
					break;
				}
				frame = i;
			}
			_frame = frame->index();
			_time = Qk_MIN(time, _duration);

			uint32_t f0 = _frame;
			uint32_t f1 = f0 + 1;

			if ( f1 < length() ) {
				int32_t time0 = frame->time();
				int32_t time1 = _frames_RT[f1]->time();
				float x = (_time - time0) / float(time1 - time0);
				float y = frame->curve().fixed_solve_y(x, 0.001);
				_frames_RT[f0]->applyTransition(root->_target, _frames_RT[f1], y);
			} else { // last frame
				_frames_RT[f0]->apply(root->_target);
			}

			if ( _time == int32_t(frame->time()) ) {
				trigger_ActionKeyframe_RT(0, _frame, root);
			}
		}
	}

	void KeyframeAction::seek_before_RT(int32_t time, Action* child) {
		Qk_UNIMPLEMENTED();
	}

	void KeyframeAction::append(Action *child) {
		// Qk_Throw(ERR_ACTION_KEYFRAME_CANNOT_APPEND, "KeyframeAction::append, cannot call append method for keyfrane");
		Qk_ERR("KeyframeAction::append, cannot call append method for keyfrane");
	}

}
