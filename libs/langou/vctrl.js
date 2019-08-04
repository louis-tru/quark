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
import { Div, Hybrid, Button, TextNode, View } from 'langou';

const TEXT_NODE_VALUE_TYPE = new Set(['function', 'string', 'number', 'boolean']);

// 标记需要重新渲染的控制器
function markRerender(vctrl) {
	// TODO ...
}

// examples
const bug_feedback_vx = (
	_VV(Div, [[["title"],"Bug Feedback"],[["source"],require.resolve(__filename)]],[
		_VV(Div, [[["width"],"full"]],[
			_VV(Hybrid, [[["class"],"category_title"]],[_VVT("Now go to Github issues list?"), _VVD('A')]),
			_VV(Button, [[["class"],"long_btn rm_margin_top"], [["onClick"],"handle_go_to"], [["url"],"langou_tools_issues_url"]],[_VVT("Go Github Issues")]),
			_VV(Hybrid, [[["class"],"category_title"]],[_VVT("Or you can send me email, too.")]),
			_VV(Button, [[["class"],"long_btn rm_margin_top"], [["onClick"],"handle_bug_feedback"]],[_VVT("Send email")])
		])
	])
)

/**
 * @class VirtualDOM
 */
class VirtualDOM {
	propsHashCode = 0;
	props = null;
	type = null;
	children = null;
	dom = null;

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

	assignProps() {
		// TODO ...
	}

	instance(vctrl) {
		util.assert(!this.dom);
		var dom = new this.type();
		this.dom = dom;
		dom.m_owner = vctrl;
		var children = this.children;

		if (dom.isViewController()) { // ctrl
			var placeholder = new View();
			dom.m_vchildren = children;
			dom.m_placeholder = placeholder;
			markRerender(dom); // mark render
		} else {
			for (var vdom of children) {
				if (vdom)
					vdom.instance(vctrl).appendTo(dom);
			}
		}
		this.assignProps(); // after set props
	
		return dom;
	}

}

/**
 * @class VirtualDOMCollection
 */
class VirtualDOMCollection extends VirtualDOM {
	m_vdoms = null;

	constructor(vdoms) {
		super(ViewCollection, [], []);
		this.m_vdoms = vdoms;
	}

	instance(vctrl) {
		util.assert(!this.dom);
		var dom = new ViewCollection();
		var collection = [];
		this.dom = dom;
		dom.m_owner = vctrl;
		dom.m_collection = collection;

		for (var vdom of this.m_vdoms) {
			if (vdom)
				collection.push(vdom.instance(vctrl));
		}
		if (!collection.length) {
			dom.m_placeholder = new View();
		}

		return dom;
	}

}

/**
 * @class ViewCollection DOM
 */
class ViewCollection {

	m_placeholder = null; // view placeholder	
	m_collection = null;
	m_owner = null;

	get __view__() {
		return this.m_placeholder ? this.m_placeholder: this.m_collection.last(0).__view__;
	}

	get collection() {
		return this.m_collection;
	}

	remove() {
		if (this.m_placeholder) {
			this.m_placeholder.remove();
		} else {
			for (var view of this.m_collection) {
				view.remove();
			}
		}
	}

	isViewController() {
		return false;
	}

	appendTo(parentView) {
		if (this.m_placeholder) {
			this.m_placeholder.appendTo(parentView);
		} else {
			for (var view of this.m_collection) {
				view.appendTo(parentView);
			}
		}
	}

	afterTo(prevView) {
		if (this.m_placeholder) {
			this.m_placeholder.afterTo(prevView);
		} else {
			for (var view of this.m_collection) {
				view.afterTo(prevView);
				prevView = view;
			}
		}
	}

}

/*
	* @func diff()
	*/
function diff(self, vdom_c, vdom, prev) {
	if (vdom_c.type === vdom.type) {
		// diff props and children ...
	} else {
		vdom.instance(self).afterTo(prev); // add new
		vdom_c.dom.remove(); // del cur
		self.m_dom = vdom.dom;
		self.m_vdom = vdom;
	}
}

/**
 * @func rerender() rerender dom
 */
function rerender(self) {
	var vdom_c = self.m_vdom;
	var vdom = _VVD(self.render());
	if (vdom_c) {
		if (!vdom) {
			var dom = self.m_dom;
			var view = dom.__view__;
			util.assert(view);
			var placeholder = new View();
			placeholder.afterTo(view);
			util.assert(!self.m_placeholder);
			self.m_placeholder = placeholder;
			self.m_dom = null;
			self.m_vdom = null;
			dom.remove(); // del dom
		} else {
			diff(self, vdom_c, vdom, self.__view__); // diff
		}
	} else {
		if (vdom) {
			util.assert(self.m_placeholder);
			vdom.instance(self).afterTo(self.m_placeholder); // add
			self.m_placeholder.remove();
			self.m_placeholder = null;
			self.m_dom = vdom.dom;
			self.m_vdom = vdom;
		}
	}
}

/**
 * @class ViewController DOM
 */
export class ViewController extends Notification {

	m_id = null;    // id
	m_owner = null;  // owner controller
	m_dom = null;   // children view or controller or 
	m_placeholder = null; // view placeholder	
	m_vmodle = null; // vmodle
	m_vdom = null; // vdom
	m_vchildren = null;

	get __view__() {
		return this.m_dom ? this.m_dom.__view__: this.m_placeholder;
	}

	event onRemove; // @event onRemove
	event onLoad;   // @event onLoad

	get id() {
		return this.m_id;
	}

	get owner() {
		return this.m_owner;
	}

	get dom() {
		return this.m_dom;
	}

	get vmodle() {
		return this.m_vmodle;
	}

	set vmodle(vm) {
		// TODO diff vmodle ...
		Object.assign(this.m_vmodle, vm);
	}

	get vchildren() {
		return this.m_vchildren;
	}

	isViewController() {
		return true;
	}

	appendTo(parentView) {
		(this.m_dom || this.m_placeholder).appendTo(parentView);
	}

	afterTo(prevView) {
		(this.m_dom || this.m_placeholder).afterTo(prevView);
	}

	/**
	 * @func render()
	 */
	render() {
		return null;
	}

	remove() {
		var dom = this.m_dom || this.m_placeholder;
		if (dom) {
			var owner = this.m_owner;
			if (owner) {
				util.assert(owner.m_dom !== this, 'Illegal call');
			}
			this.m_dom = null;
			this.m_placeholder = null;
			this.triggerRemove();
			dom.remove();
		}
	}

}

// create virtual view
export function _VV(Type, props, children) {
	return new VirtualDOM(Type, props, children);
}

// create virtual view TextNode
export function _VVT(value) {
	return new VirtualDOM(TextNode, [[['value'],value]], []);
}

// create virtual view dynamic
export function _VVD(value) {
	if (TEXT_NODE_VALUE_TYPE.has(typeof value)) {
		return _VVT(value);
	} else if (Array.isArray(value)) {
		return new VirtualDOMCollection(value.map(_VVD));
	} else {
		return value;
	}
}
