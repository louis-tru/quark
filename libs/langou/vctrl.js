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

/**
 * @func render
 */
function render(self, parent, prev) {
	var vchildren = self.m_vchildren;
	var vdom = self.render(...vchildren);
	var vdom_raw = self.m_vdom;

	if (vdom === vdom_raw) {
		
	}
}

/**
 * @class ViewController
 */
export class ViewController extends Notification {

	m_view = null;
	m_vmodle = null; // vmodle
	m_vdom = null; // vdom
	m_vchildren = null;

	get view() {
		return this.m_view;
	}

	get vmodle() {
		return this.m_vmodle;
	}

	set vmodle(vm) {
		// TODO ...
		Object.assign(this.m_vmodle, vm);
	}

	/**
	 * @constructor
	 */
	constructor(...props) {
		super();
		this.m_vmodle = {};
		this.m_vchildren = [];
	}

	/**
	 * @func render(...vchildren)
	 */
	render(...vchildren) {
		return null;
	}

}