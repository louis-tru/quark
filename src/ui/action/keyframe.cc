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
#include "../window.h"
#include "../view/view.h"
#include "../../errno.h"

#define _async_call _window->preRender().async_call

namespace qk {

	Keyframe::Keyframe(KeyframeAction* host, cCurve& curve)
		: _index(0), _time(0), _curve(curve), _host(host)
	{}

	void Keyframe::destroy() {
		if (_host) {
			_host = nullptr; // mark destroy
		} else {
			Object::destroy(); // destroy
		}
	}

	KeyframeAction::KeyframeAction(Window *win)
		: Action(win), _frame(0), _time(0)
	{}

	void KeyframeAction::clear() {
		if (_frames.length()) {
			for (auto i : _frames) {
				i->release();
			}
			_frames.clear();

			if (_window) { // @ Action::release_for_only_center_Rt()
				_async_call([](auto self, auto arg) {
					for (auto i : self->_frames_Rt)
						i->destroy(); // last call destroy
					self->_frames_Rt.clear();
				}, this, 0);
			} else {
				for (auto i : _frames_Rt)
					i->destroy(); // last call destroy
				_frames_Rt.clear();
			}
		}
		if ( _duration ) {
			setDuration( -_duration );
		}
	}

	KeyframeAction* KeyframeAction::MakeSSTransition(
		View *view, StyleSheets *to, uint32_t time, bool isRt
	) {
		Qk_ASSERT(time);
		auto action = new KeyframeAction(view->window());
		// only use isRt=true, Because it is initialization and there will be no security issues
		auto f0 = action->add_unsafe(0, EASE, isRt);
		auto f1 = action->add_unsafe(time, to->curve(), isRt);

		action->set_target(view);

		struct SetFrames_Rt {
			static void call(KeyframeAction *self, StyleSheets *to) {
				auto v = self->_target;
				auto f0 = self->_frames_Rt[0];
				auto f1 = self->_frames_Rt[1];
				for (auto &i: to->_props) { // copy prop
					f0->_props.set(i.key, i.value->copy())->fetch(v);
					f1->_props.set(i.key, i.value->copy());
				}
			}
		};

		if (isRt) {
			SetFrames_Rt::call(action, to);
		} else {
			view->preRender().async_call([](auto self, auto arg) {
				SetFrames_Rt::call(self, arg.arg);
			}, action, to);
		}

		return action;
	}

	Keyframe* KeyframeAction::add_unsafe(uint32_t time, cCurve& curve, bool isRt) {
		auto frame = new Keyframe(this, curve);
		frame->_time = _frames.length() ? time: 0;
		frame->_index = _frames.length();

		if (_frames.length()) {
			auto back = _frames.back();
			int32_t d = frame->_time - back->time();
			if ( d <= 0 ) {
				frame->_time = back->time();
			} else {
				setDuration(d);
			}
		}
		_frames.push(frame);

		struct SetFrames_Rt {
			static void call(KeyframeAction *self, Keyframe *frame) {
				if (self->_frames_Rt.length()) {
					for (auto i: self->_frames_Rt.back()->_props) // copy prop
						frame->_props.set(i.key, i.value->copy());
				}
				self->_frames_Rt.push(frame);
			}
		};
		if (isRt) {
			SetFrames_Rt::call(this, frame);
		} else {
			_async_call([](auto self, auto arg) {
				SetFrames_Rt::call(self, arg.arg);
			}, this, frame);
		}

		return frame;
	}

	Keyframe* KeyframeAction::addFrame(uint32_t time, cCurve& curve) {
		return add_unsafe(time, curve, false);
	}

	Keyframe* KeyframeAction::addFrameWithCss(cString& cssExp, uint32_t *timeMs, cCurve *curve) {
		auto css = _window->styleSheets()->searchItem(cssExp, false);
		if (css) {
			auto f = add_unsafe(timeMs ? *timeMs: css->time(), curve ? *curve: css->curve(), false);
			_async_call([](auto f, auto arg) {
				for (auto &i: arg.arg->_props)
					f->_props.set(i.key, i.value->copy());
			}, f, css);
			return f;
		} else {
			return add_unsafe(timeMs ? *timeMs: 0, curve ? *curve: EASE, false);
		}
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

	bool KeyframeAction::hasProperty(ViewProp name) {
		return _frames_Rt.length() && _frames_Rt.front()->hasProperty(name);
	}

	uint32_t KeyframeAction::advance_Rt(uint32_t time_span, bool restart, Action* root) {
		time_span *= _speed;

		if ( restart ) { // no start play or restart
			_time = _frame = _looped = 0;

			if ( _frames_Rt.length() ) {
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
		uint32_t f0 = _frame, f1 = f0 + 1;

		if ( f1 < _frames_Rt.length() ) {
		advance:
			if ( root->_id == Id() ) { // is not playing
				return 0;
			}
			int32_t time = _time + time_span;
			int32_t time1 = _frames_Rt[f0]->time();
			int32_t time2 = _frames_Rt[f1]->time();
			int32_t t = time - time2;

			if ( t < 0 ) {
				time_span = 0;
				_time = time;
				float x = (time - time1) / float(time2 - time1);
				float y = _frames_Rt[f1]->_curve.solve_y(x, 0.001);
				_frames_Rt[f0]->applyTransition(root->_target, _frames_Rt[f1], y);
			} else if ( t > 0 ) {
				time_span = t;
				_frame = f1;
				_time = time2;
				trigger_ActionKeyframe_Rt(t, f1, root); // trigger event action_key_frame
				f0 = f1; f1++;

				if ( f1 < _frames_Rt.length() ) {
					goto advance;
				} else if (_looped < _loop) {
					goto loop;
				} else {
					_frames_Rt[f0]->apply(root->_target, true); // apply last frame
				}
			} else { // t == 0
				time_span = 0;
				_time = time;
				_frame = f1;
				_frames_Rt[f1]->apply(root->_target, true);
				trigger_ActionKeyframe_Rt(0, f1, root); // trigger event action_key_frame
			}

		} else if ( _looped < _loop ) { // Can continue to loop
		loop:
			_looped++;
			_frame = _time = 0;
			trigger_ActionLoop_Rt(time_span, root);
			trigger_ActionKeyframe_Rt(time_span, 0, root);
			goto start;
		}

	end:
		return time_span / _speed;
	}

	void KeyframeAction::seek_time_Rt(uint32_t time, Action* root) {
		if ( _frames_Rt.length() ) {
			Keyframe* frame0 = nullptr;

			for ( auto& f: _frames_Rt ) {
				if ( time < f->time() )
					break;
				frame0 = f;
			}
			_frame = frame0->index();
			_time = Qk_Min(time, _duration);

			uint32_t f0 = _frame, f1 = f0 + 1;

			if ( f1 < _frames_Rt.length() ) {
				auto frame1 = _frames_Rt[f1];
				int32_t t0 = frame0->time();
				int32_t t1 = frame1->time();
				float x = (_time - t0) / float(t1 - t0);
				float y = frame1->_curve.solve_y(x, 0.001);
				frame0->applyTransition(root->_target, frame1, y);
			} else { // last frame
				frame0->apply(root->_target, true);
			}

			if ( _time == frame0->time() ) {
				trigger_ActionKeyframe_Rt(0, _frame, root);
			}
		}
	}

	void KeyframeAction::seek_before_Rt(uint32_t time, Action* child) {
		Qk_Unreachable("");
	}

	void KeyframeAction::append(Action *child) throw(Error) {
		Qk_Throw(ERR_ACTION_KEYFRAME_CANNOT_APPEND, "KeyframeAction::append, cannot call append method for keyframe");
		// Qk_ELog("KeyframeAction::append, cannot call append method for keyframe");
	}

}
