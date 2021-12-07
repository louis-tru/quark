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

#include "./action.inl"

namespace flare {

	void ActionCenter::Inl::add(Action* action) {
		if ( action->_action_center_id == Action::ActionCenterId() ) {
			action->_action_center_id = _actions.insert(_actions.end(), { action, 0 });
			action->retain();
		}
	}
	
	void ActionCenter::Inl::del(Action* action) {
		if ( action && action->_action_center_id != Action::ActionCenterId() ) {
			action->_action_center_id->value = nullptr; // del
			// _actions.del(action->_action_center_id);
			action->_action_center_id = Action::ActionCenterId();
			action->release();
		}
	}

	static ActionCenter* action_center_shared = nullptr;

	ActionCenter::ActionCenter()
	: _prev_sys_time(0) {
		F_ASSERT(!action_center_shared); action_center_shared = this;
	}

	ActionCenter::~ActionCenter() {
		action_center_shared = nullptr;
	}

	/**
	* @func advance
	*/
	void ActionCenter::advance(int64_t now_time) {
		/*
		static int len = 0;
		if (len != _actions.length()) {
			len = _actions.length();
			F_LOG("ActionCenter::advance,length, %d", len);
		}*/
		
		if ( _actions.length() ) { // run task
			int64_t time_span = 0;
			if (_prev_sys_time) {  // 0表示还没开始
				time_span = now_time - _prev_sys_time;
				if ( time_span > 200000 ) {   // 距离上一帧超过200ms重新记时(如应用程序从休眠中恢复)
					time_span = 200000; // 100ms
				}
			}
			auto i = _actions.begin(), end = _actions.end();
			while ( i != end ) {
				auto j = i++;
				Action::Wrap& wrap = *j;
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
						_actions.erase(j);
					}
				} else {
					_actions.erase(j);
				}
			}
			_prev_sys_time = now_time;
		}
	}

	ActionCenter* ActionCenter::shared() {
		return action_center_shared;
	}

}
