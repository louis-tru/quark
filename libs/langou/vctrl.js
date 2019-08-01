/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

import { EventNoticer, Notification } from 'langou/event';
// 
import { Div, Hybrid, Button } from 'langou';

// examples
const bug_feedback_vx = (
	[Mynavpage, [["title","Bug Feedback"],["source",resolve(__filename)]],[
		[Div, [["width","full"]],[
			[Hybrid, [["class","category_title"]],["Now go to Github issues list?"]],
			[Button, [["class","long_btn rm_margin_top"], ["onClick",handle_go_to], ["url",langou_tools_issues_url]],["Go Github Issues"]],
			[Hybrid, [["class","category_title"]],["Or you can send me email, too."]],
			[Button, [["class","long_btn rm_margin_top"], ["onClick",handle_bug_feedback]],["Send email"]]
		]]
	]]
)

function load(self) {
	
}

function diff(self, vdom_c, vdom) {
	if (vdom_c) {
		if (!vdom) {
			// del
		}
	} else {
		if (vdom) {
			// add
		}
	}

	var Type1 = vdom_c[0];
	var Type2 = vdom[0];

	if (Type1 === Type2) {
		// up
	} else {
		// del cur, add 
	}

}

/**
 * @func render
 */
function render(self, parent, prev) {
	var vdom_c = self.m_vdom;
	var vdom = self.render();

	diff(self, vdom_c, vdom);
}

/**
 * @class ViewController
 */
export class ViewController extends Notification {

	m_parent = null; // parent controller
	m_view = null;   // children view or controller
	m_id = null;
	m_vmodle = null; // vmodle
	m_vdom = null; // vdom
	m_vchildren = null;

	get parent() {
		return this.m_parent;
	}

	get view() {
		return this.m_view;
	}

	get id() {
		return this.m_id;
	}

	get vmodle() {
		return this.m_vmodle;
	}

	set vmodle(vm) {
		// TODO ...
		Object.assign(this.m_vmodle, vm);
	}

	/**
	 * @func render(...vchildren)
	 */
	render(...vchildren) {
		return null;
	}

}