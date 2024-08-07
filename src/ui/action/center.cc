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
#include "./keyframe.h"
#include "../window.h"

namespace qk {

	ActionCenter::ActionCenter(Window* window): _window(window), _prevTime_Rt(0) 
	{}

	ActionCenter::~ActionCenter() {
		auto it0 = _CSSTransitions_Rt.begin();
		while (it0 != _CSSTransitions_Rt.end()) {
			removeCSSTransition_Rt(reinterpret_cast<View*>((it0++)->key));
		}
		auto it = _actions_Rt.begin();
		while (it != _actions_Rt.end()) {
			(it++)->value->stop_Rt();
		}
		Qk_Fatal_Assert(_CSSTransitions_Rt.length() == 0, "ActionCenter::~ActionCenter stop CSSTransitions");
		Qk_Fatal_Assert(_actions_Rt.length() == 0, "ActionCenter::~ActionCenter stop actions first");
	}

	void ActionCenter::addCSSTransition_Rt(View *view, CStyleSheets *css) {
		auto action = KeyframeAction::MakeSSTransition(view, css, css->time(), true);
		action->retain(); // retain for center
		action->set_target(view);
		action->play_Rt();
		_CSSTransitions_Rt.get(uint64_t(view)).push(action);
	}

	void ActionCenter::removeCSSTransition_Rt(View *view) {
		auto it = _CSSTransitions_Rt.find(uint64_t(view));
		if (it != _CSSTransitions_Rt.end()) {
			for (auto act: it->value) {
				act->stop_Rt();
				act->release_for_only_center_Rt();
			}
			_CSSTransitions_Rt.erase(it);
		}
	}

	void ActionCenter::advance_Rt(uint32_t timeMs) {
		if ( _actions_Rt.length() == 0) return; 

		uint32_t time_span = 0;
		if (_prevTime_Rt) {  // 0表示还没开始
			time_span = timeMs - _prevTime_Rt;
			if ( time_span > 200 ) {   // 距离上一帧超过200ms重新记时(如应用程序从休眠中恢复)
				time_span = 200;
			}
		}
		auto i = _actions_Rt.begin(), end = _actions_Rt.end();

		while ( i != end ) {
			auto j = i++;
			auto &act = *j;
			if (act.value->_target) {
				if (act._runAdvance) {
					if ( act.value->advance_Rt(time_span, false, act.value) ) {
						act.value->stop_Rt(); // stop action
					}
				} else {
					act.value->advance_Rt(0, false, act.value);
					act._runAdvance = true;
				}
			} else {
				act.value->stop_Rt(); // stop action
			}
		}
		_prevTime_Rt = timeMs;
	}

}
