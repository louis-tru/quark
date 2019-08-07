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
import { Notification, List } from 'langou/event';
const { TextNode, View, Root } = requireNative('_langou');

const TEXT_NODE_VALUE_TYPE = new Set(['function', 'string', 'number', 'boolean']);
const G_renderQueueSet = new Set();
const G_renderQueue = new List();
var   G_renderQueueWorking = false;
const G_warnRecord = new WeakSet();
const G_warnDefine = {
	UndefinedDOMKey: 'DOM key no defined in DOM Collection',
};

function warn(id, msg) {
	var def = G_warnDefine[id];
	if (def) {
		if (!G_warnRecord.has(id)) {
			G_warnRecord.add(id);
			console.warn(def, msg);
		}
	}
}

// mark controller rerender
function markRerender(ctr) {
	var size = G_renderQueueSet.size;
	G_renderQueueSet.add(ctr);
	if (size == G_renderQueueSet.size) return;
	G_renderQueue.push(ctr);
	if (G_renderQueueWorking) return;

	util.nextTick(function() {
		G_renderQueueWorking = true;
		var item;
		try {
			while( (item = G_renderQueue.shift()) ) {
				G_renderQueueSet.delete(item);
				rerender(item);
			}
		} finally {
			G_renderQueueWorking = false;
		}
	});
}

// examples
/*
const bug_feedback_vx = (
	_VV(Div, [[["title"],"Bug Feedback"],[["source"],require.resolve(__filename)]],[
		_VV(Div, [[["width"],"full"]],[
			_VV(Hybrid, [[["class"],"category_title"]],[_VVT("Now go to Github issues list?"), _VVD('A')]),
			_VV(Button, [[["class"],"long_btn rm_margin_top"], [["onClick"],"handle_go_to"], [["url"],"langou_tools_issues_url"]],[_VVT("Go Github Issues")]),
			_VV(Hybrid, [[["class"],"category_title"]],[_VVT("Or you can send me email, too.")]),
			_VV(Button, [[["class"],"long_btn rm_margin_top"], [["onClick"],"handle_bug_feedback"]],[_VVT("Send email")])
		])
	])
)*/

function assignProp(self, [names, value]) {
	var target = self.dom;
	if (names.length == 1) {
		target[names[0]] = value;
	} else {
		var len = names.length - 1;
		var name = names[len];
		for (var i = 0; i < len; i++) {
			target = target[names[i]];
		}
		target[name] = value;
	}
}

/**
 * @class VirtualDOM
 * @private
 */
class VirtualDOM {
	hash = 0;
	propsHash = 0;
	props = null;
	type = null;
	children = null;
	dom = null;
	style = null;
	options = null;

	constructor(Type, props, children, options) {
		var _propsHash = 0;
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
			_propsHash += (_propsHash << 5) + hashCode;
			_props[prop[0].join('.')] = prop;
		}

		var _hash = (Type.hashCode() << 5) + _propsHash;

		for (var vdom of children) {
			if (vdom) {
				_hash += (_hash << 5) + vdom.hash;
			}
		}

		this.type = Type;
		this.props = _props;
		this.hash = _hash;
		this.propsHash = _propsHash;
		this.children = children;
		this.options = options;
	}

	setStyle(style) {
		if (style) {
			this.style = style;
		}
	}

	get key() {
		return this.options.key;
	}

	get hasKey() {
		return 'key' in this.options;
	}

	assignProps() {
		if (this.style)
			this.dom.style = style;
		var props = this.props;
		for (var key in props) {
			assignProp(this, props[key]);
		}
	}

	diffProps(vdom) {
		if (this.propsHash != vdom.propsHash) {
			var props0 = this.props;
			var props1 = vdom.props;
			for (var key in props1) {
				var prop0 = props0[key], prop1 = props1[key];
				if (!prop0 || prop0[2] != prop1[2]) {
					assignProp(this, prop1);
				}
			}
		}
	}

	newInstance(ctr) {
		util.assert(!this.dom);
		var Type = this.type;
		var dom = new Type();
		this.dom = dom;
		dom.m_owner = ctr;
		var children = this.children;

		if (Type.isViewController()) { // ctrl
			var placeholder = new View();
			dom.m_vchildren = children;
			dom.m_placeholder = placeholder;
			this.assignProps(); // before set props
			var r = dom.triggerLoad(); // trigger event Load
			if (r instanceof Promise) {
				r.then(e=>dom.m_loaded = true);
			} else {
				dom.m_loaded = true
			}
			markRerender(dom); // mark render
		} else {
			for (var vdom of children) {
				if (vdom)
					vdom.newInstance(ctr).appendTo(dom);
			}
			this.assignProps(); // after set props
		}

		return dom;
	}

}

/**
 * @class VirtualDOMCollection
 * @private
 */
class VirtualDOMCollection extends VirtualDOM {
	vdoms = null;
	keys = {};

	constructor(vdoms) {
		super(DOMCollection, [], [], {});
		this.keys = {};
		this.vdoms = vdoms.filter(e=>e);
		this.vdoms.forEach(e=>(this.hash += (this.hash << 5) + e.hash));
	}

	setStyle(style) {
		if (style) {
			this.style = style;
			this.vdoms.forEach(e=>e.setStyle(style));
		}
	}

	diffProps({vdoms, keys, hash}) {
		var { keys: keys_c, dom } = this.keys;
		var doms = [];
		var ctr = dom.owner;
		var prev = dom.m_doms[0] || dom.m_placeholder; // DOMCollection placeholder or doms[0]

		util.assert(prev);

		for (var i = 0; i < vdoms.length; i++) {
			var vdom = vdoms[i];
			var key = vdom.key;

			if (!vdom.hasKey) {
				warn('UndefinedDOMKey');
				vdom.options.key = key = '_$auto' + i; // auto key
			}
			if (keys[key]) {
				throw new Error('DOM Key definition duplication in DOM Collection, = ' + key);
			} else {
				keys[key] = vdom;
			}

			var vdom_c = keys_c[key];
			if (vdom_c) {
				if (vdom_c.hash != vdom.hash) {
					prev = diff(ctr, vdom_c, vdom, prev); // diff
					doms.push(vdom.dom);
				} else { // use old dom
					delete keys_c[key];
					keys[key] = vdom_c;
					vdoms[i] = vdom_c;
					prev = vdom_c.dom.afterTo(prev);
					doms.push(vdom_c.dom);
				}
			} else { // no key
				var cell = vdom.newInstance(ctr);
				prev = cell.afterTo(prev);
				doms.push(cell);
			}
		}

		if (doms.length) {
			if (dom.m_placeholder) {
				dom.m_placeholder.remove();
				dom.m_placeholder = null;
			}
		} else if (!dom.m_placeholder) {
			dom.m_placeholder = new View();
			dom.m_placeholder.afterTo(prev);
		}

		this.hash = hash;
		this.vdoms = vdoms;
		this.keys = keys;
		dom.m_doms = doms;

		for (var key in keys_c) {
			removeDOM(ctr, keys_c[key]);
		}
	}

	newInstance(ctr) {
		util.assert(!this.dom);
		var keys = this.keys;
		var doms = [];
		var vdoms = this.vdoms;
		var style = this.style;

		for (var i = 0; i < vdoms.length; i++) {
			var vdom = vdoms[i];
			vdom.style = style;
			var cell = vdom.newInstance(ctr);
			var key = vdom.key;
			if (!vdom.hasKey) {
				warn('DOMCollectionKey');
				vdom.options.key = key = '_$auto' + i; // auto key
			}
			if (keys[key]) {
				throw new Error('DOM Key definition duplication in DOM Collection, = ' + key);
			} else {
				keys[key] = vdom;
			}
			doms.push(cell);
		}

		return (this.dom = new DOMCollection(ctr, this, doms));
	}

}

function callDOMsFunc(self, active, view) {
	if (self.m_placeholder) {
		return self.m_placeholder[active](view);
	} else {
		for (var cellView of self.m_doms) {
			cellView[active](view);
		}
		return self.m_doms.last(0);
	}
}

/**
 * @class DOMCollection DOM
 * @private
 */
class DOMCollection {

	// @private:
	m_owner = null;
	m_vdom = null;
	m_doms = null;
	m_style = null;
	m_placeholder = null; // view placeholder	

	get __view__() {
		return this.m_placeholder ? this.m_placeholder: this.m_doms.last(0).__view__;
	}

	// @public:
	get owner() {
		return this.m_owner;
	}

	get doms() {
		return this.m_doms;
	}

	constructor(ctr, vdom, doms) {
		this.m_owner = ctr;
		this.m_vdom = vdom;
		this.m_doms = doms;
		if (!doms.length) {
			this.m_placeholder = new View();
		}
		if (vdom.style) {
			this.m_style = vdom.style;
		}
	}

	remove() {
		if (this.m_placeholder) {
			this.m_placeholder.remove();
			this.m_placeholder = null;
		}
		else if (this.m_vdom) {
			var keys = this.m_vdom.keys;
			this.m_vdom.vdoms.forEach(vdom=>{
				var key = vdom.key;
				if (key) {
					delete keys[key];
				}
				removeDOM(this.m_owner, vdom);
			});
			this.m_vdom = null;
			this.m_doms = [];
		}
	}

	static isViewController() {
		return false;
	}

	appendTo(parentView) {
		return callDOMsFunc(this, 'appendTo', parentView);
	}

	afterTo(prevView) {
		return callDOMsFunc(this, 'afterTo', prevView);
	}

	get style() {
		return this.m_style;
	}

	set style(style) {
		if (style) {
			this.m_style = style;
			this.m_doms.forEach(e=>e.style=style);
		}
	}

}

function removeSubctr(self, vdom) {
	for (var vdom of vdom.children) {
		if (vdom) {
			if (vdom.type.isViewController()) {
				vdom.dom.remove(); // remove ctrl
			} else {
				removeSubctr(self, vdom);
			}
		}
	}
	var id = vdom.dom.id;
	if (id) {
		if (self.m_IDs[id] === vdom.dom) {
			delete self.m_IDs[id];
		}
	}
}

function removeDOM(self, vdom) {
	removeSubctr(self, vdom);
	vdom.dom.remove();
}

function setVDOM(self, vdom) {
	self.m_vdom = vdom;
	self.m_dom = vdom ? vdom.dom: null;
}

function diff(self, vdom_c, vdom, prevView) {
	// diff type
	if (vdom_c.type !== vdom.type) {
		var r = vdom.newInstance(self).afterTo(prevView); // add new
		removeDOM(self, vdom_c); // del dom
		return r;
	} 

	var dom = vdom_c.dom;
	vdom.dom = dom;

	// diff props
	vdom_c.diffProps(vdom); 

	var view = dom.__view__;

	// diff children
	var children_c = vdom_c.children;
	var children = vdom.children;

	if ( vdom.type.isViewController() ) {
		if ( children_c.length || children.length ) {
			dom.m_vchildren = children;
			markRerender(dom); // mark ctrl render
		}
	} else {
		var childrenCount = Math.max(children_c.length, children.length);

		for (var i = 0, prev = null; i < childrenCount; i++) {
			vdom_c = children_c[i];
			vdom = children[i];
			if (vdom_c) {
				if (vdom) {
					if (vdom_c.hash != vdom.hash) {
						prev = diff(self, vdom_c, vdom, prev || vdom_c.dom.__view__); // diff
					} else {
						children[i] = vdom_c;
						prev = vdom_c.dom.__view__;
					}
				} else {
					removeDOM(self, vdom_c); // remove DOM
				}
			} else {
				if (vdom) {
					var dom = vdom.newInstance(self);
					if (prev)
						prev = dom.afterTo(prev); // add
					else {
						var tmp = new View();
						view.prepend(tmp);
						prev = dom.afterTo(tmp); // add
						tmp.remove();
					}
				}
			}
		}
	} // if (vdom.type.isViewController()) end

	return view;
}

function rerender(self) {
	var vdom_c = self.m_vdom;
	var vdom = _VVD(self.render());
	var update = false;

	if (vdom) {
		vdom.setStyle(this.m_style);
	}

	if (vdom_c) {
		if (vdom) {
			if (vdom_c.hash != vdom.hash) {
				diff(self, vdom_c, vdom, self.__view__); // diff
				setVDOM(self, vdom); // set new vdom
				update = true;
			}
		} else {
			var dom = self.m_dom;
			var view = dom.__view__;
			util.assert(view);
			var placeholder = new View();
			placeholder.afterTo(view);
			util.assert(!self.m_placeholder);
			self.m_placeholder = placeholder;
			setVDOM(self, null);
			removeDOM(self, vdom_c); // del dom
			update = true;
		}
	} else {
		if (vdom) {
			util.assert(self.m_placeholder);
			vdom.newInstance(self).afterTo(self.m_placeholder); // add
			self.m_placeholder.remove();
			self.m_placeholder = null;
			setVDOM(self, vdom);
			update = true;
		}
	}

	if (!self.m_mounted) {
		self.m_mounted = true;
		self.triggerMounted();
	}
	if (update) {
		self.triggerUpdate();
	}
}

function domInCtr(self) {
	return this.m_dom || this.m_placeholder;
}

/**
 * @class ViewController DOM
 */
export default class ViewController extends Notification {
	// @private:
	m_id = null;     // id
	m_IDs = null;		 // 
	m_owner = null;  // owner controller
	m_placeholder = null; // view placeholder	
	m_modle = null;  // view modle
	m_dataHash = null; // modle and props hash
	m_vdom = null;   // vdom
	m_dom = null;    // children view or controller or 
	m_vchildren = null;
	m_loaded = false;
	m_mounted = false;
	m_style = null;

	get __view__() {
		return this.m_dom ? this.m_dom.__view__: this.m_placeholder;
	}

	// @public:
	event onLoad;    // @event onLoad
	event onMounted; // @event onMounted
	event onUpdate;  // @event onUpdate
	event onRemove;  // @event onRemove
	event onRemoved; // @event onRemoved

	get id() {
		return this.m_id;
	}

	static setID(dom, id) {
		var id = dom.m_id;
		if (value != id) {
			if (dom.m_owner) {
				var ids = dom.m_owner.m_IDs;
				if (ids[id] === dom) {
					delete ids[id];
				}
				if (value) {
					if (value in ids) {
						throw new Error('Identifier reference duplication in controller, = ' + value);
					}
					ids[value] = dom;
				}
			}
			dom.m_id = value;
		}
	}

	set id(value) {
		ViewController.setID(this, value);
	}

	get IDs() {
		return this.m_IDs;
	}

	get owner() {
		return this.m_owner;
	}

	get dom() {
		return this.m_dom;
	}

	get isLoaded() {
		return this.m_loaded;
	}

	get isMounted() {
		return this.m_mounted;
	}

	get modle() {
		return this.m_modle;
	}

	set modle(modle) {
		this.setModle(modle);
	}

	setModle(modle) {
		var update = false;
		var value = this.m_modle;
		var hash = this.m_dataHash;
		for (var key in modle) {
			var item = modle[key];
			var hashCode = Object.hashCode(item);
			if (hashCode != hash[key]) {
				value[key] = item;
				hash[key] = hashCode;
				update = true;
			}
		}
		if (update) {
			markRerender(this); // mark render
		}
	}

	get vchildren() {
		return this.m_vchildren;
	}

	constructor() {
		super();
		this.m_IDs = {};
		this.m_modle = {};
		this.m_dataHash = {};
	}

	/**
	 * @overwrite
	 */
	hashCode() {
		return Function.prototype.hashCode.call(this);
	}

	appendTo(parentView) {
		return domInCtr(this).appendTo(parentView);
	}

	afterTo(prevView) {
		return domInCtr(this).afterTo(prevView);
	}

	/**
	 * @overwrite
	 */
	addDefaultListener(name, func) {
		if ( typeof func == 'string' ) {
			var owner = this, func2;
			do {
				func2 = owner[func];  // find func
				if ( typeof func2 == 'function' ) {
					return this.getNoticer(name).on(func2, owner, 0); // default id 0
				}
				owner = owner.m_owner;
			} while(owner);
			throw util.err(`Cannot find a function named "${func}"`);
		} else {
			return this.getNoticer(name).on(func, 0); // default id 0
		}
	}

	/**
	 * @func render()
	 */
	render() {
		return this.vchildren;
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

			this.triggerRemove(); // trigger Remove event

			this.m_dom = null;

			if (vdom) {
				removeDOM(this, vdom);
			} else {
				placeholder.remove();
			}
			this.triggerRemoved(); // trigger Removed
		}
	}

	/**
	 * @get style
	 */
	get style() {
		return this.m_style;
	}

	/**
	 * @set style
	 */
	set style(style) {
		this.m_style = style;
		if (this.m_dom) {
			this.m_dom.style = style;
		}
	}

	// @static:

	/**
	 * @func isViewController()
	 */
	static isViewController() {
		return true;
	}

	/**
	 * @func defineProps(props, [controllerClass])
	 * <pre>
	 * 	class MyViewController extends ViewController {
	 * 		render() {
	 * 			return (
	 * 				<Div width=this.width height=this.height>Hello</Div>
	 * 			);
	 * 		}
	 * 	}
	 *  MyViewController.defineProps(['width', height', 'prop1'])
	 * </pre>
	 */
	static defineProps(props, controllerClass) {
		controllerClass = controllerClass || this;
		util.assert(util.equalsClass(ViewController, controllerClass), 'Type error');

		for (let prop of props) {
			Object.defineProperty(controllerClass.prototype, prop, {
				get() {
					return this['M_' + prop];
				},
				set(value) {
					var hashCode = Object.hashCode(value);
					var hash = this.m_dataHash;
					if (hash['__Prop_' + prop] != hashCode) {
						hash['__Prop_' + prop] = hashCode;
						this['M_' + prop] = value;
						markRerender(this); // mark render
					}
				},
				configurable: true,
				enumerable: false,
			});
		}
	}

	/**
	 * @func typeof(vdom, Type)
	 * @static
	 */
	static typeOf(vdom, Type) {
		if (vdom instanceof ViewController || vdom instanceof View) {
			if (vdom instanceof Type)
				return 2;
		}
		if (vdom instanceof VirtualDOM) {
			if (vdom.type instanceof Type)
				return 1;
		}
		return 0;
	}

}

/**
 * @class RootViewController DOM
 */
export class RootViewController extends ViewController {

	//@overwrite
	render() {
		var vchildren = this.vchildren;
		if (vchildren.length) {
			var first = this.vchildren[0];
			util.assert(util.equalsClass(Root, first.type), 'RootViewController first children must be Root view');
			return first;
		} else {
			return _VV(Root, [], []); // return Root
		}
	}
}

// extend static method
util.extend(ViewController, {

	/**
	 * @func hashCode()
	 */
	hashCode: function() {
		return Function.prototype.hashCode.call(ViewController);
	},

	/**
	 * @func render(vdom, [parentView])
	 */
	render: function(vdom, parentView) {
		if (vdom instanceof ViewController || vdom instanceof View) {
			var dom = vdom;
		} else {
			vdom = _VVD(vdom);
			util.assert(vdom instanceof VirtualDOM, 'Bad argument');
			var owner = parentView ? parentView.owner: null;
			var dom = vdom.newInstance(owner);
		}
		if (parentView) {
			dom.appendTo(parentView);
			dom.m_owner = owner;
		}
		return dom;
	},

});

// create virtual view
export function _VV(Type, props, children, options) {
	return new VirtualDOM(Type, props, children, options || {});
}

// create virtual view TextNode
export function _VVT(value) {
	return new VirtualDOM(TextNode, [[['value'],value]], [], {});
}

// create virtual view dynamic
export function _VVD(value) {
	if (value instanceof VirtualDOM) {
		return value
	} else if (TEXT_NODE_VALUE_TYPE.has(typeof value)) {
		return _VVT(value);
	} else if (Array.isArray(value)) {
		if (value.length) {
			return value.length == 1 ?
				_VVD(value[0]): new VirtualDOMCollection(value.map(_VVD));
		} else {
			return null;
		}
	}
	return value && _VVT(String(value)); // null or TextNode
}
