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
import { Notification } from 'langou/event';
const { TextNode, View, Root, lock } = requireNative('_langou');

const TEXT_NODE_VALUE_TYPE = new Set(['function', 'string', 'number', 'boolean']);
const G_renderQueueSet = new Set();
var   G_renderQueueWorking = false;
const G_warnRecord = new Set();
const G_warnDefine = {
	UndefinedDOMKey: 'DOM key no defined in DOM Collection',
};

function warn(id, msg = '') {
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
	if (G_renderQueueWorking) return;
	G_renderQueueWorking = true;

	util.nextTick(function() {
		var item;
		try {
			for( var item of G_renderQueueSet ) {
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

	constructor(Type, props, children) {
		var _propsHash = 0;
		var _props = {};

		for (var prop of props) {
			var [keys,value] = prop;
			var hashCode = 0;
			for (var key of keys) {
				hashCode += (hashCode << 5) + key.hashCode();
			}
			hashCode += (hashCode << 5) + Object.hashCode(value);
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
	}

	setDefaultStyle(style) {
		if (style) {
			if (!this.props.hasOwnProperty('style')) {
				var hashCode = ('style'.hashCode() << 5) + style.hashCode();
				this.props.style = [['style'], style, hashCode];
				this.propsHash += (this.propsHash << 5) + hashCode;
				this.hash += (this.hash << 5) + hashCode;
			}
		}
	}

	getProp(name) {
		var prop = this.props[name];
		return prop ? prop[1]: null;
	}

	hasProp(name) {
		return name in this.props;
	}

	assignProps() {
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
		var dom = new this.type();
		this.dom = dom;
		dom.m_owner = ctr;

		if (this.type.isViewController) { // ctrl
			dom.m_vchildren = this.children;
			this.assignProps(); // before set props
			var r = dom.triggerLoad(); // trigger event Load
			if (r instanceof Promise) {
				r.then(e=>dom.m_loaded = true);
			} else {
				dom.m_loaded = true
			}
			rerender(dom); // rerender
		} else {
			for (var vdom of this.children) {
				if (vdom)
					vdom.newInstance(ctr).appendTo(dom);
			}
			this.assignProps(); // after set props
		}

		return dom;
	}

	hashCode() {
		return this.hash;
	}

}

/**
 * @class VirtualDOMCollection
 * @private
 */
class VirtualDOMCollection extends VirtualDOM {
	vdoms = null;

	constructor(vdoms) {
		super(DOMCollection, [], [], {});
		this.vdoms = vdoms.filter(e=>e);
		this.vdoms.forEach(e=>(this.hash += (this.hash << 5) + e.hash));
	}

	setDefaultStyle(style) {}

	diffProps({vdoms, hash}) {
		var dom = this.dom;
		var doms = [];
		var keys = {};
		var keys_c = dom.m_keys; // private props visit
		var ctr = dom.owner;
		var prev = dom.m_doms[0] || dom.m_placeholder; // DOMCollection placeholder or doms[0]

		util.assert(prev);

		for (var i = 0; i < vdoms.length; i++) {
			var vdom = vdoms[i], key;
			if (vdom.hasProp('key')) {
				key = vdom.getProp('key');
			} else {
				warn('UndefinedDOMKey');
				key = '_$auto' + i; // auto key
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
					keys[key] = vdom_c;
					vdoms[i] = vdom_c;
					prev = vdom_c.dom.afterTo(prev);
					doms.push(vdom_c.dom);
				}
				delete keys_c[key];
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

		dom.m_doms = doms;
		dom.m_vdoms = vdoms;
		dom.m_keys = keys;

		for (var key in keys_c) {
			removeDOM(ctr, keys_c[key]);
		}
	}

	newInstance(ctr) {
		util.assert(!this.dom);
		var vdoms = this.vdoms;
		var doms = [];
		var keys = {};
		var style = this.style;

		for (var i = 0; i < vdoms.length; i++) {
			var vdom = vdoms[i], key;
			vdom.style = style;
			var cell = vdom.newInstance(ctr);
			if (vdom.hasProp('key')) {
				key = vdom.getProp('key');
			} else {
				warn('UndefinedDOMKey');
				key = '_$auto' + i; // auto key
			}
			if (keys[key]) {
				throw new Error('DOM Key definition duplication in DOM Collection, = ' + key);
			} else {
				keys[key] = vdom;
			}
			doms.push(cell);
		}

		return (this.dom = new DOMCollection(ctr, vdom, doms, keys));
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
	m_vdoms = null;
	m_doms = null;
	m_keys = null;
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

	constructor(ctr, vdom, doms, keys) {
		this.m_owner = ctr;
		this.m_vdoms = vdom.vdoms;
		this.m_doms = doms;
		this.m_keys = keys;
		if (!doms.length) {
			this.m_placeholder = new View();
		}
		if (vdom.defaultStyle) {
			this.m_style = vdom.defaultStyle;
		}
	}

	remove() {
		if (this.m_placeholder) {
			this.m_placeholder.remove();
			this.m_placeholder = null;
		}
		else if (this.m_vdoms) {
			var keys = this.m_keys;
			this.m_vdoms.forEach(vdom=>{
				var key = vdom.key;
				if (key) {
					delete keys[key];
				}
				removeDOM(this.m_owner, vdom);
			});
			this.m_vdoms = null;
			this.m_keys = null;
			this.m_doms = [];
		}
	}

	appendTo(parentView) {
		return callDOMsFunc(this, 'appendTo', parentView);
	}

	afterTo(prevView) {
		return callDOMsFunc(this, 'afterTo', prevView);
	}

}

function removeSubctr(self, vdom) {
	for (var e of vdom.children) {
		if (e) {
			if (e.type.isViewController) {
				e.dom.remove(); // remove ctrl
			} else {
				removeSubctr(self, e);
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
	util.assert(prevView);

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

	if ( vdom.type.isViewController ) {
		if ( children_c.length || children.length ) {
			dom.m_vchildren = children;
			rerender(dom); // mark ctrl render
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
	} // if (vdom.type.isViewController) end

	return view;
}

function rerender(self) {
	G_renderQueueSet.delete(self); // delete mark

	var vdom_c = self.m_vdom;
	var vdom = _VVD(self.render(...self.m_vchildren));
	var update = false;

	if (vdom) {
		vdom.setDefaultStyle(self.m_style);
	}

	if (vdom_c) {
		if (vdom) {
			if (vdom_c.hash != vdom.hash) {
				var prev = self.m_dom.__view__;
				util.assert(prev);
				diff(self, vdom_c, vdom, prev); // diff
				setVDOM(self, vdom); // set new vdom
				update = true;
			}
		} else {
			var prev = self.m_dom.__view__;
			util.assert(prev);
			util.assert(!self.m_placeholder);
			self.m_placeholder = new View();
			self.m_placeholder.afterTo(prev);
			setVDOM(self, null);
			removeDOM(self, vdom_c); // del dom
			update = true;
		}
	} else {
		if (vdom) {
			vdom.newInstance(self);
			if (self.m_placeholder) {
				vdom.dom.afterTo(self.m_placeholder);
				self.m_placeholder.remove();
				self.m_placeholder = null;
			}
			setVDOM(self, vdom);
			update = true;
		} else {
			if (!self.m_placeholder) {
				self.m_placeholder = new View();
			}
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
	return self.m_dom || self.m_placeholder;
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
	m_vchildren = null; // outer vdom children
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
		var idRaw = dom.m_id;
		if (idRaw != id) {
			if (dom.m_owner) {
				var ids = dom.m_owner.m_IDs;
				if (ids[idRaw] === dom) {
					delete ids[idRaw];
				}
				if (id) {
					if (id in ids) {
						throw new Error('Identifier reference duplication in controller, = ' + value);
					}
					ids[id] = dom;
				}
			}
			dom.m_id = id;
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
			this.markRerender(); // mark render
		}
	}

	constructor() {
		super();
		this.m_IDs = {};
		this.m_modle = {};
		this.m_dataHash = {};
	}

	/*
	 * @func markRerender()
	 */
	markRerender() {
		markRerender(this);
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
				var func2 = owner[func];  // find func
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
	 * @func render(...vdoms)
	 */
	render(...vdoms) {
		return vdoms;
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
		return this.m_style || {};
	}

	// @static:
	/**
	 * @get isViewController
	 * @static
	 */
	static get isViewController() {
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
		props = Array.isArray(props) ? props.map(e=>[e]): Object.entries(props);
		var prototype = controllerClass.prototype;

		for (let [prop,value] of props) {
			prototype['m_' + prop] = value;
			var desc = Object.getOwnPropertyDescriptor(prototype, 'prop');
			
			Object.defineProperty(prototype, prop, {
				get: desc && desc.get ? desc.get: function() {
					return this['m_' + prop];
				},
				set(value) {
					var hashCode = Object.hashCode(value);
					var hash = this.m_dataHash;
					if (hash['__prop_' + prop] != hashCode) {
						hash['__prop_' + prop] = hashCode;
						this['m_' + prop] = value;
						this.markRerender(); // mark render
					}
				},
				configurable: true,
				enumerable: false,
			});
		}
	}

	/**
	 * @func typeOf(obj, [Type=class ViewController])
	 * @arg obj {VirtualDOM|View|ViewController|class}
	 * @static
	 */
	static typeOf(obj, Type) {
		Type = Type || ViewController;
		if (util.equalsClass(ViewController, Type) || util.equalsClass(View, Type)) {
			if (obj instanceof Type)
				return 3; // dom instance
			if (obj instanceof VirtualDOM) { 
				if (util.equalsClass(Type, obj.type))
					return 2; // vdom instance
			}
			if (util.equalsClass(Type, obj))
				return 1; // class
		}
		return 0;
	}

}

/**
 * @set style {Object}
 */
ViewController.defineProps(['style']);

/**
 * @class RootViewController DOM
 */
export class RootViewController extends ViewController {

	//@overwrite
	render(vdom) {
		if (vdom) {
			util.assert(util.equalsClass(Root, vdom.type), 'RootViewController first children must be Root view');
			return vdom;
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
		return Function.prototype.hashCode.call(this);
	},

	/**
	 * @func render(obj, [parentView])
	 * @arg obj {VirtualDOM|View|ViewController|class}
	 * @ret {DOM} return dom instance
	 */
	render: function(obj, parentView) {
		var dom;
		var owner = parentView ? parentView.owner: null;

		if (obj instanceof ViewController || obj instanceof View) {
			dom = obj; // dom instance
		} else if (util.equalsClass(ViewController, obj) || util.equalsClass(View, obj)) {
			obj = _VV(obj, [], []); // create vdom
			dom = obj.newInstance(owner);
		} else {
			obj = _VVD(obj); // format vdom
			util.assert(obj instanceof VirtualDOM, 'Bad argument');
			dom = obj.newInstance(owner);
		}
		if (parentView) {
			dom.appendTo(parentView);
			dom.m_owner = owner;
		}
		return dom;
	},

});

// create virtual view
export function _VV(Type, props, children) {
	return new VirtualDOM(Type, props, children);
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
