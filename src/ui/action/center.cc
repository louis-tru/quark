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

#include "./action.h"
#include "./keyframe.h"
#include "../window.h"

namespace qk {

	ActionCenter::ActionCenter(Window* window): _window(window)
	{}

	ActionCenter::~ActionCenter() {
		auto it0 = _CSSTransitions_rt.begin();
		while (it0 != _CSSTransitions_rt.end()) {
			removeCSSTransition_rt(reinterpret_cast<View*>((it0++)->first));
		}
		auto it = _actions_rt.begin();
		while (it != _actions_rt.end()) {
			(it++)->value->stop_rt();
		}
		Qk_CHECK(_CSSTransitions_rt.length() == 0, "ActionCenter::~ActionCenter stop CSSTransitions");
		Qk_CHECK(_actions_rt.length() == 0, "ActionCenter::~ActionCenter stop actions first");
	}

	void ActionCenter::addCSSTransition_rt(View *view, CStyleSheets *css) {
		auto action = KeyframeAction::MakeSSTransition(view, css, css->time(), true);
		action->retain(); // retain for center
		action->play_rt(); // start action
		_CSSTransitions_rt.get(uintptr_t(view)).push(action);
	}

	void ActionCenter::removeCSSTransition_rt(View *view) {
		auto it = _CSSTransitions_rt.find(uintptr_t(view));
		if (it != _CSSTransitions_rt.end()) {
			for (auto act: it->second) {
				act->stop_rt();
				act->release_inner_rt();
			}
			_CSSTransitions_rt.erase(it);
		}
	}

	void ActionCenter::advance_rt(int64_t deltaTime) {
		if ( _actions_rt.length() == 0)
			return;

		auto deltaMs = uint32_t(deltaTime * 0.001f); // delta in milliseconds
		auto i = _actions_rt.begin(), end = _actions_rt.end();

		while ( i != end ) {
			auto j = i++;
			auto &act = *j;
			if (act.value->_target) {
				if (act._runAdvance) {
					if ( act.value->advance_rt(deltaMs, false, act.value) ) {
						act.value->stop_rt(); // stop action
					}
				} else {
					act.value->advance_rt(0, false, act.value);
					act._runAdvance = true;
				}
			} else {
				act.value->stop_rt(); // stop action
			}
		}
	}

}
