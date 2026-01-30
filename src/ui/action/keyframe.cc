/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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

#define _async_call _window->pre_render().async_call

namespace qk {

	Keyframe::Keyframe(KeyframeAction* host, cCurve& curve)
		: _index(0), _time(0), _curve(curve), _host(host)
	{}

	void Keyframe::destroy() {
		// It is need to be two times call destroy
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
		unsafe_clear(false);
	}

	void KeyframeAction::unsafe_clear(bool isRt) {
		if (_frames.length()) {
			if (_window && !isRt) { // @ Action::release_inner_rt()
				_async_call([](auto self, auto arg) {
					for (auto i : self->_frames_rt)
						i->destroy(); // last call destroy
					self->_frames_rt.clear();
				}, this, 0);
			} else {
				for (auto i : _frames_rt)
					i->destroy(); // last call destroy
				_frames_rt.clear();
			}
			for (auto i : _frames) {
				i->release();
			}
			_frames.clear();
			if (_duration) {
				setDuration( -_duration );
			}
		}
		Qk_ASSERT_EQ(_duration, 0);
	}

	KeyframeAction* KeyframeAction::MakeSSTransition(
		View *view, StyleSheets *to, uint32_t time, bool isRt
	) {
		Qk_ASSERT(time);
		auto action = new KeyframeAction(view->window());
		// only use isRt=true, Because it is initialization and there will be no security issues
		auto f0 = action->unsafe_add(0, EASE, isRt);
		auto f1 = action->unsafe_add(time, to->curve(), isRt);

		action->set_target(view);

		struct SetFrames_rt {
			static void call(KeyframeAction *self, StyleSheets *to) {
				auto v = self->_target.load();
				if (!v) return; // target is null
				auto f0 = self->_frames_rt[0];
				auto f1 = self->_frames_rt[1];
				for (auto &i: to->_props) { // copy prop
					f0->_props.set(i.first, i.second->copy())->fetch(v); // fetch initial value from view
					f1->_props.set(i.first, i.second->copy());
				}
			}
		};

		if (isRt) {
			SetFrames_rt::call(action, to);
		} else {
			view->pre_render().async_call([](auto self, auto arg) {
				SetFrames_rt::call(self, arg.arg);
			}, action, to);
		}

		return action;
	}

	Keyframe* KeyframeAction::unsafe_add(uint32_t time, cCurve& curve, bool isRt) {
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

		struct SetFrames_rt {
			static void call(KeyframeAction *self, Keyframe *frame) {
				if (self->_frames_rt.length()) {
					for (auto i: self->_frames_rt.back()->_props) // copy prop
						frame->_props.set(i.first, i.second->copy());
				}
				self->_frames_rt.push(frame);
			}
		};
		if (isRt) {
			SetFrames_rt::call(this, frame);
		} else {
			_async_call([](auto self, auto arg) {
				SetFrames_rt::call(self, arg.arg);
			}, this, frame);
		}

		return frame;
	}

	Keyframe* KeyframeAction::addFrame(uint32_t time, cCurve& curve) {
		return unsafe_add(time, curve, false);
	}

	Keyframe* KeyframeAction::addFrameWithCss(cString& cssExp, uint32_t *timeMs, cCurve *curve) {
		auto css = shared_root_styleSheets()->searchItem(cssExp, false);
		if (css) {
			auto f = unsafe_add(timeMs ? *timeMs: css->time(), curve ? *curve: css->curve(), false);
			_async_call([](auto f, auto arg) {
				for (auto &i: arg.arg->_props)
					f->_props.set(i.first, i.second->copy());
			}, f, css);
			return f;
		} else {
			return unsafe_add(timeMs ? *timeMs: 0, curve ? *curve: EASE, false);
		}
	}

	Window* Keyframe::getWindow() const {
		return _host ? _host->_window: nullptr;
	}

	void Keyframe::onMake(CssProp key, Property* prop) {
		if (_host) {
			for (auto i: _host->_frames_rt) {
				if (i != this) {
					i->_props.set(key, prop->copy());
				}
			}
		}
	}

	bool KeyframeAction::hasProperty(CssProp name) {
		return _frames_rt.length() && _frames_rt.front()->hasProperty(name);
	}

	uint32_t KeyframeAction::advance_rt(uint32_t deltaTime, bool restart, Action* root) {
		auto target = root->_target.load();
		if (!target)
			return deltaTime; // target is null

		deltaTime *= _speed;

		if ( restart ) { // no start play or restart
			_time = _frame = _looped_rt = 0;

			if ( _frames_rt.length() ) {
				_time = _frame = 0;
				_frames_rt[0]->apply(target, true);
				trigger_ActionKeyframe_rt(deltaTime, 0, root);

				if ( deltaTime == 0 ) {
					return 0;
				}
				if ( _frames_rt.length() == 1 ) {
					return deltaTime / _speed;
				}
			} else {
				return deltaTime / _speed;
			}
		}

	 start:
		uint32_t f0 = _frame, f1 = f0 + 1;

		if ( f1 < _frames_rt.length() ) {
		advance:
			if ( root->_id_rt == Id() ) { // is not playing
				return 0;
			}
			int32_t time = _time + deltaTime;
			int32_t time1 = _frames_rt[f0]->time();
			int32_t time2 = _frames_rt[f1]->time();
			int32_t t = time - time2;

			if ( t < 0 ) {
				deltaTime = 0;
				_time = time;
				float x = (time - time1) / float(time2 - time1);
				float y = _frames_rt[f1]->_curve.solve_y(x, 0.001); // get curve y from f1
				_frames_rt[f0]->applyTransition(target, _frames_rt[f1], y);
			} else if ( t > 0 ) {
				deltaTime = t;
				_frame = f1;
				_time = time2;
				trigger_ActionKeyframe_rt(t, f1, root); // trigger event action_key_frame
				f0 = f1; f1++;

				if ( f1 < _frames_rt.length() ) {
					goto advance;
				} else if (next_loop_rt()) {
					goto loop;
				} else {
					_frames_rt[f0]->apply(target, true); // apply last frame
				}
			} else { // t == 0
				deltaTime = 0;
				_time = time;
				_frame = f1;
				_frames_rt[f1]->apply(target, true);
				trigger_ActionKeyframe_rt(0, f1, root); // trigger event action_key_frame
			}

		} else if (_frames_rt.length() && next_loop_rt()) { // Can continue to loop
		 loop:
			_frame = _time = 0;
			trigger_ActionLoop_rt(deltaTime, root);
			trigger_ActionKeyframe_rt(deltaTime, 0, root);
			goto start;
		}

	 end:
		return deltaTime / _speed;
	}

	void KeyframeAction::seek_time_rt(uint32_t time, Action* root) {
		auto target = root->_target.load();
		if ( target && _frames_rt.length() ) {
			Keyframe* frame0 = nullptr;

			for ( auto f: _frames_rt ) {
				if ( time < f->time() )
					break;
				frame0 = f;
			}
			_frame = frame0->index();
			_time = Qk_Min(time, _duration);

			uint32_t f0 = _frame, f1 = f0 + 1;

			if ( f1 < _frames_rt.length() ) {
				auto frame1 = _frames_rt[f1];
				int32_t t0 = frame0->time();
				int32_t t1 = frame1->time();
				float x = (_time - t0) / float(t1 - t0);
				float y = frame1->_curve.solve_y(x, 0.001);
				frame0->applyTransition(target, frame1, y);
			} else { // last frame
				frame0->apply(target, true);
			}

			if ( _time == frame0->time() ) {
				trigger_ActionKeyframe_rt(0, _frame, root);
			}
		}
	}

	void KeyframeAction::seek_before_rt(uint32_t time, Action* child) {
		Qk_Unreachable("");
	}

	void KeyframeAction::append(Action *child) throw(Error) {
		Qk_Throw(ERR_ACTION_KEYFRAME_CANNOT_APPEND, "KeyframeAction::append, cannot call append method for keyframe");
	}

}
