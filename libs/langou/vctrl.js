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

import 'langou/util';
import { EventNoticer, Notification } from 'langou/event';
// 
import { Div, Hybrid, Button, TextNode } from 'langou';

/**
 * @class VirtualDOM
 */
class VirtualDOM {
	propsHashCode = 0;
	props = null;
	type = null;
	children = null;

	constructor(Type, props, children) {
		var _hash = 0;
		var _props = {};
		for (var prop of props) {
			var [keys,value] = prop;
			var hashCode = 0;
			for (var key of keys) {
				hashCode += (hashCode << 5) + key.hashCode();
			}
			if (value) {
				hashCode += (hashCode << 5) + value.hashCode();
			}
			prop[2] = hashCode;
			_hash += (_hash << 5) + hashCode;
			_props[prop[0].join('.')] = prop;
		}
		this.type = Type;
		this.props = _props;
		this.propsHashCode = _hash;
		this.children = children;
	}
}

/**
 * @class DOMCollection
 */
class DOMCollection {

	m_doms = null;

	appendTo(parent) {
		// TODO ...
	}

	remove() {
		// TODO ...
	}
}

export function VDom(Type, props, children) {
	return new VirtualDOM(Type, props, children);
}

export function VDomS(value) {
	return new VirtualDOM(TextNode, [[['value'],value]], []);
}

const TEXT_NODE_SET = new Set(['function', 'string', 'number', 'boolean']);

export function VDomD(value) {
	if (TEXT_NODE_SET.has(typeof value)) {
		return VDomS(value);
	} else if (Array.isArray(value)) {
		return new VirtualDOM(DOMCollection, [], value.map(VDomD));
	} else {
		return value;
	}
}

// examples
const bug_feedback_vx = (
	VDom(Div, [[["title"],"Bug Feedback"],[["source"],require.resolve(__filename)]],[
		VDom(Div, [[["width"],"full"]],[
			VDom(Hybrid, [[["class"],"category_title"]],[VDomS("Now go to Github issues list?"), VDomD('A')]),
			VDom(Button, [[["class"],"long_btn rm_margin_top"], [["onClick"],"handle_go_to"], [["url"],"langou_tools_issues_url"]],[VDomS("Go Github Issues")]),
			VDom(Hybrid, [[["class"],"category_title"]],[VDomS("Or you can send me email, too.")]),
			VDom(Button, [[["class"],"long_btn rm_margin_top"], [["onClick"],"handle_bug_feedback"]],[VDomS("Send email")])
		])
	])
)

function diff(self, vdom_c, vdom) {
	if (vdom_c) {
		if (!vdom) {
			vdom_c[3].remove(); // del dom
		}
	} else {
		if (vdom) {
			// add
			// new vdom[0]();
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
	diff(self, self.m_vdom, VDomD(self.render()));
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