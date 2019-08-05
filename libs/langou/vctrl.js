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

	newInstance(vctrl) {
		// util.assert(!this.dom);
		var Type = this.type;
		var dom = new Type();
		this.dom = dom;
		dom.m_owner = vctrl;
		var children = this.children;

		if (Type.isViewController()) { // ctrl
			var placeholder = new View();
			dom.m_ids = {};
			dom.m_vchildren = children;
			dom.m_placeholder = placeholder;
			this.assignProps(); // before set props
			dom.triggerLoad(); // trigger event Load
			markRerender(dom); // mark render
		} else {
			for (var vdom of children) {
				if (vdom)
					vdom.newInstance(vctrl).appendTo(dom);
			}
			this.assignProps(); // after set props
		}
	
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

	newInstance(vctrl) {
		// util.assert(!this.dom);
		var dom = new ViewCollection();
		var collection = [];
		this.dom = dom;
		dom.m_owner = vctrl;
		dom.m_collection = collection;
		dom.m_vdoms = this.m_vdoms;

		for (var vdom of this.m_vdoms) {
			if (vdom)
				collection.push(vdom.newInstance(vctrl));
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
	m_vdoms = null;

	get __view__() {
		return this.m_placeholder ? this.m_placeholder: this.m_collection.last(0).__view__;
	}

	get owner() {
		return this.m_owner;
	}

	get collection() {
		return this.m_collection;
	}

	remove() {
		if (this.m_placeholder) {
			this.m_placeholder.remove();
		} else {
			for (var vdom of this.m_vdoms) {
				if (vdom) {
					removeDOM(this.m_owner, vdom);
				}
			}
			this.m_collection = [];
		}
	}

	static isViewController() {
		return false;
	}

	appendTo(parentView) {
		if (this.m_placeholder) {
			return this.m_placeholder.appendTo(parentView);
		} else {
			for (var view of this.m_collection) {
				view.appendTo(parentView);
			}
			return this.m_collection.last(0);
		}
	}

	afterTo(prevView) {
		if (this.m_placeholder) {
			return this.m_placeholder.afterTo(prevView);
		} else {
			for (var view of this.m_collection) {
				view.afterTo(prevView);
				prevView = view;
			}
			return this.m_collection.last(0);
		}
	}

}

/**
 * @func removeSubctrl()
 */
function removeSubctrl(self, vdom) {
	for (var vdom of vdom.children) {
		if (vdom) {
			if (vdom.type.isViewController()) {
				vdom.dom.remove(); // remove ctrl
			} else {
				removeSubctrl(self, vdom);
			}
		}
	}
	var id = vdom.dom.id;
	if (id) {
		if (self.m_ids[id] === vdom.dom) {
			delete self.m_ids[id];
		}
	}
}

/**
 * @func removeDOM()
 */
function removeDOM(self, vdom) {
	removeSubctrl(self, vdom);
	vdom.dom.remove();
}

/**
 * @func setVDOM()
 */
function setVDOM(self, vdom) {
	self.m_vdom = vdom;
	self.m_dom = vdom ? vdom.dom: null;
}

/*
 * @func diff()
 * @return {View}
 */
function diff(self, vdom_c, vdom, prev) {

	if (vdom_c.type !== vdom.type) {
		prev = vdom.newInstance(self).afterTo(prev); // add new
		removeDOM(self, vdom_c); // del dom
		return prev;
	} else {
		var dom = vdom_c.dom;
		vdom.dom = dom;

		// diff props
		if (vdom_c.propsHashCode != vdom.propsHashCode) {
			// TODO ...
		}

		// diff children
		var children_c = vdom_c.children;
		var children = vdom.children;
		var childrenCount = Math.max(children_c.length, children.length);
		var view = dom.__view__;
		var tmpPrev;

		prev = children_c.find(e=>e);

		if ( !prev ) {
			tmpPrev = prev = (new View).appendTo(view);
		}

		for (var i = 0, j = 0; i < childrenCount; i++) {
			vdom_c = children_c[i];
			vdom = children[i];
			if (vdom_c) {
				if (!vdom) {
					removeDOM(self, vdom_c); // remove
				} else {
					prev = diff(self, vdom_c, vdom, prev); // diff
				}
			} else {
				if (vdom) {
					prev = vdom.newInstance(self).afterTo(prev); // add
				}
			}
		}

		if (tmpPrev) {
			tmpPrev.remove();
		}

		return view;
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
			setVDOM(self, null);
			removeDOM(self, vdom_c); // del dom
		} else {
			diff(self, vdom_c, vdom, self.__view__); // diff
			setVDOM(self, vdom); // set new vdom
		}
	} else {
		if (vdom) {
			util.assert(self.m_placeholder);
			vdom.newInstance(self).afterTo(self.m_placeholder); // add
			self.m_placeholder.remove();
			self.m_placeholder = null;
			setVDOM(self, vdom);
		}
	}
}

/**
 * @class ViewController DOM
 */
export class ViewController extends Notification {

	// @private:
	m_id = null;     // id
	m_ids = null;
	m_owner = null;  // owner controller
	m_placeholder = null; // view placeholder	
	m_vmodle = null; // vmodle
	m_vdom = null;   // vdom
	m_dom = null;    // children view or controller or 
	m_vchildren = null;

	get __view__() {
		return this.m_dom ? this.m_dom.__view__: this.m_placeholder;
	}

	// @public:

	event onRemove; // @event onRemove
	event onLoad;   // @event onLoad

	get id() {
		return this.m_id;
	}

	set id(value) {
		if (value != this.m_id) {
			if (this.m_owner) {
				if (this.m_owner.m_ids[this.m_id] === this) {
					this.m_owner.m_ids[value] = this;
				}
			}
			this.m_id = value;
		}
	}

	get IDs() {
		return this.m_ids;
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

	static isViewController() {
		return true;
	}

	appendTo(parentView) {
		return (this.m_dom || this.m_placeholder).appendTo(parentView);
	}

	afterTo(prevView) {
		return (this.m_dom || this.m_placeholder).afterTo(prevView);
	}

	/**
	 * @func render()
	 */
	render() {
		return null;
	}

	remove() {
		var vdom = this.m_vdom;
		var placeholder = this.m_placeholder;

		if (vdom || placeholder) {
			var owner = this.m_owner;
			if (owner) {
				util.assert(owner.m_dom !== this, 'Illegal call');
			}
			this.m_placeholder = null;
			this.m_vdom = null;
			this.m_dom = null;
			
			if (vdom) {
				removeDOM(this, vdom);
			} else {
				placeholder.remove();
			}
			this.triggerRemove();
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
	} else if (!(value instanceof VirtualDOM)) {
		return _VVT(String(value));
	}
	return value;
}
