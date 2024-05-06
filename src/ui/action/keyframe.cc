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
#include "../window.h"

#define _async_call _window->preRender().async_call

namespace qk {

	Keyframe::Keyframe(KeyframeAction* host, cCurve& curve)
		: _index(0), _time(0), _curve(curve), _host(host)
	{}

	KeyframeAction::KeyframeAction(Window *win)
		: Action(win), _frame(0), _time(0), _startPlay(false)
	{}

	KeyframeAction::~KeyframeAction() {
		_frames.clear();
		clear_Rt();
	}

	void KeyframeAction::clear() {
		_frames.clear();
		Action::clear();
	}

	Keyframe* KeyframeAction::add(uint32_t time, cCurve& curve) {
		auto frame = new Keyframe(this, curve);
		frame->_time = _frames.length() ? time: 0;
		frame->_index = _frames.length();
		_frames.push(frame);

		_async_call([](auto self, auto frame) {
			if (self->_frames_Rt.length()) {
				auto back = self->_frames_Rt.back();
				int32_t d = frame.arg->_time - back->time();
				if ( d <= 0 ) {
					frame.arg->_time = back->time();
				} else {
					self->Action::update_duration_Rt(d);
				}
				for (auto &i: back->_props) { // copy prop
					frame.arg->_props.set(i.key, i.value->copy());
				}
			}
			self->_frames_Rt.push(frame.arg);
		}, this, frame);

		return frame;
	}

	Window* Keyframe::getWindowForAsyncSet() {
		return _host ? _host->_window: nullptr;
	}

	void Keyframe::onMake(ViewProp key, Property* prop) {
		if (_host) {
			for (auto i: _host->_frames_Rt) {
				if (i != this) {
					i->_props.set(key, prop->copy());
				}
			}
		}
	}

	void KeyframeAction::clear_Rt() {
		for (auto i : _frames_Rt) {
			i->_host = nullptr;
			i->release();
		}
		_frames_Rt.clear();

		if ( _duration ) {
			Action::update_duration_Rt( -_duration );
		}
	}

	bool KeyframeAction::hasProperty(ViewProp name) {
		return _frames_Rt.length() && _frames_Rt.front()->hasProperty(name);
	}

	uint32_t KeyframeAction::advance_Rt(uint32_t time_span, bool restart, Action* root) {
		time_span *= _speed;

		if ( !_startPlay || restart ) { // no start play or restart
			_time = _frame = 0;
			_looped = 0;

			if ( _frames_Rt.length() ) {
				_startPlay = true;
				_time = _frame = 0;
				_frames_Rt[0]->apply(root->_target, true);
				trigger_ActionKeyframe_Rt(time_span, 0, root);

				if ( time_span == 0 ) {
					return 0;
				}
				if ( _frames_Rt.length() == 1 ) {
					return time_span / _speed;
				}
			} else {
				return time_span / _speed;
			}
		}

	start:
		uint32_t f1 = _frame, f2 = f1 + 1;

		if ( f2 < _frames_Rt.length() ) {
		advance:
			if ( root->_id == Id() ) { // is not playing
				return 0;
			}
			int32_t time = _time + time_span;
			int32_t time1 = _frames_Rt[f1]->time();
			int32_t time2 = _frames_Rt[f2]->time();
			int32_t t = time - time2;

			if ( t < 0 ) {
				time_span = 0;
				_time = time;
				float x = (time - time1) / float(time2 - time1);
				float y = _frames_Rt[f1]->_curve.fixed_solve_y(x, 0.001);
				_frames_Rt[f1]->applyTransition(root->_target, _frames_Rt[f2], y);
			} else if ( t > 0 ) {
				time_span = t;
				_frame = f2;
				_time = time2;
				trigger_ActionKeyframe_Rt(t, f2, root); // trigger event action_key_frame

				f1 = f2; f2++;

				if ( f2 < _frames_Rt.length() ) {
					goto advance;
				} else {
					goto loop;
				}
			} else { // t == 0
				time_span = 0;
				_time = time;
				_frame = f2;
				_frames_Rt[f2]->apply(root->_target, true);
				trigger_ActionKeyframe_Rt(0, f2, root); // trigger event action_key_frame
			}

		} else { // last frame
		loop:
			if ( _looped < _loop ) { // Can continue to loop
				_looped++;
				_frame = 0;
				_time = 0;
				trigger_ActionLoop_Rt(time_span, root);
				trigger_ActionKeyframe_Rt(time_span, 0, root);
				goto start;
			} else {
				_frames_Rt[f1]->apply(root->_target, true);
				_startPlay = false; // end reset
			}
		}

	end:
		return time_span / _speed;
	}

	void KeyframeAction::seek_time_Rt(uint32_t time, Action* root) {
		_looped = 0;

		if ( _frames_Rt.length() ) {
			Keyframe* frame = nullptr;

			for ( auto& i: _frames_Rt ) {
				if ( time < i->time() ) {
					break;
				}
				frame = i;
			}
			_frame = frame->index();
			_time = Qk_MIN(time, _duration);

			uint32_t f0 = _frame;
			uint32_t f1 = f0 + 1;

			if ( f1 < _frames_Rt.length() ) {
				int32_t time0 = frame->time();
				int32_t time1 = _frames_Rt[f1]->time();
				float x = (_time - time0) / float(time1 - time0);
				float y = frame->_curve.fixed_solve_y(x, 0.001);
				_frames_Rt[f0]->applyTransition(root->_target, _frames_Rt[f1], y);
			} else { // last frame
				_frames_Rt[f0]->apply(root->_target, true);
			}

			if ( _time == frame->time() ) {
				trigger_ActionKeyframe_Rt(0, _frame, root);
			}
		}
	}

	void KeyframeAction::seek_before_Rt(uint32_t time, Action* child) {
		Qk_UNIMPLEMENTED();
	}

	void KeyframeAction::append(Action *child) {
		Qk_Throw(ERR_ACTION_KEYFRAME_CANNOT_APPEND, "KeyframeAction::append, cannot call append method for keyframe");
		// Qk_ERR("KeyframeAction::append, cannot call append method for keyframe");
	}

}
