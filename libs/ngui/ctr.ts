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

import util from './util';
import event, { Notification, EventNoticer, Listen, Event } from './event';
import { TextNode, View, Root } from './_view';

const TEXT_NODE_VALUE_TYPE = new Set(['function', 'string', 'number', 'boolean']);
const G_removeSet = new WeakSet();
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
			_VV(Button, [[["class"],"long_btn rm_margin_top"], [["onClick"],"handle_go_to"], [["url"],"ngui_tools_issues_url"]],[_VVT("Go Github Issues")]),
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
export class VirtualDOM<T = any> {
	hash = 0;
	propsHash = 0;
	props = null;
	type: T;
	children = null;
	dom = null;

	constructor(Type: T, props: Dict | null, children: any[]) {
		var _propsHash = 0;
		var _props = {};

		for (var prop in props) {
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

	setDefaultStyle(style: Dict) {
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

	diffProps(vdom: VirtualDOM) {
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
class VirtualDOMCollection extends VirtualDOM<typeof DOMCollection> {
	vdoms: VirtualDOM[];

	constructor(vdoms: (VirtualDOM | null)[]) {
		super(DOMCollection, {}, []);
		this.vdoms = vdoms.filter(e=>e) as VirtualDOM[];
		this.vdoms.forEach(e=>(this.hash += (this.hash << 5) + e.hash));
	}

	setDefaultStyle(style: Dict) {}

	diffProps({ vdoms, hash }: VirtualDOMCollection) {
		var dom = this.dom;
		var keys = {};
		var keys_c = dom.m_keys; // private props visit
		var ctr = dom.owner;
		var prev = dom.m_vdoms.length ? dom.m_vdoms[0].dom: dom.m_placeholder; // DOMCollection placeholder or doms[0]

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
				} else { // use old dom
					keys[key] = vdom_c;
					vdoms[i] = vdom_c;
					prev = vdom_c.dom.afterTo(prev);
				}
				delete keys_c[key];
			} else { // no key
				var cell = vdom.newInstance(ctr);
				prev = cell.afterTo(prev);
			}
		}

		if (vdoms.length) {
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

		dom.m_vdoms = vdoms;
		dom.m_keys = keys;

		for (var key in keys_c) {
			removeDOM(ctr, keys_c[key]);
		}
	}

	newInstance(ctr: ViewController) {
		util.assert(!this.dom);
		var vdoms = this.vdoms;
		var keys = {};
		this.dom = new DOMCollection(ctr, vdoms, keys);

		for (var i = 0; i < vdoms.length; i++) {
			var vdom = vdoms[i], key;
			vdom.newInstance(ctr);
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
		}

		return this.dom;
	}

}

function callDOMsFunc(self: DOMCollection, active: string, view: View) {
	if (self.m_placeholder) {
		return self.m_placeholder[active](view);
	} else {
		for (var cellView of self.m_vdoms) {
			cellView.dom[active](view);
		}
		return self.m_vdoms.last(0).dom;
	}
}

export type DOM = ViewController | View | DOMCollection;

/**
 * @class DOMCollection DOM
 * @private
 */
class DOMCollection {

	// @private:
	private m_owner: ViewController;
	private m_vdoms: DOM[];
	private m_keys: string[];
	private m_placeholder: View | null = null; // view placeholder	

	private get __view__() {
		return this.m_placeholder ? this.m_placeholder: this.m_vdoms.last(0).dom.__view__;
	}

	// @public:
	get owner() {
		return this.m_owner;
	}

	get collection() {
		return this.m_vdoms.map(e=>e.dom);
	}

	constructor(owner: ViewController, vdoms: DOM[], keys: string[]) {
		this.m_owner = owner;
		this.m_vdoms = vdoms;
		this.m_keys = keys;

		if (!vdoms.length) {
			this.m_placeholder = new View();
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
		}
	}

	appendTo(parentView: View) {
		return callDOMsFunc(this, 'appendTo', parentView);
	}

	afterTo(prevView: View) {
		return callDOMsFunc(this, 'afterTo', prevView);
	}

}

function removeSubctr(self: ViewController, vdom: VirtualDOM) {
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

function removeDOM(self: ViewController, vdom: VirtualDOM) {
	removeSubctr(self, vdom);
	vdom.dom.remove();
}

function diff(self: ViewController, vdom_c: VirtualDOM, vdom: VirtualDOM, prevView: View) {
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

function rerender(self: ViewController) {
	G_renderQueueSet.delete(self); // delete mark

	var vdom_c = self.m_vdom;
	var vdom = _CVDD(self.render(...self.m_vchildren));
	var update = false;

	if (vdom) {
		vdom.setDefaultStyle(self.m_style);
	}

	if (vdom_c) {
		if (vdom) {
			if (vdom_c.hash != vdom.hash) {
				var prev = vdom_c.dom.__view__;
				util.assert(prev);
				self.m_vdom = vdom;
				diff(self, vdom_c, vdom, prev); // diff
				update = true;
			}
		} else {
			var prev = vdom_c.dom.__view__;
			util.assert(prev);
			util.assert(!self.m_placeholder);
			self.m_placeholder = new View();
			self.m_placeholder.afterTo(prev);
			self.m_vdom = null;
			removeDOM(self, vdom_c); // del dom
			update = true;
		}
	} else {
		if (vdom) {
			self.m_vdom = vdom;
			vdom.newInstance(self);
			if (self.m_placeholder) {
				vdom.dom.afterTo(self.m_placeholder);
				self.m_placeholder.remove();
				self.m_placeholder = null;
			}
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
	return self.m_vdom ? self.m_vdom.dom: self.m_placeholder;
}

/**
 * @class ViewController DOM
 */
export class ViewController extends Notification<Event<any, ViewController>> {
	private m_IDs: Dict<ViewController | View> = {};
	private m_modle: Dict = {}; // view modle
	private m_dataHash: Dict<number> = {}; // modle and props hash
	private m_id: string; // = null;     // id
	private m_owner: ViewController | null; // = null;  // owner controller
	private m_placeholder: View | null; // = null; // view placeholder	
	private m_vdom: VirtualDOM | null; // = null;     // children vdom
	private m_vchildren: VirtualDOM[]; // = []; // outer vdom children
	private m_loaded: boolean; // = false;
	private m_mounted: boolean; // = false;
	private m_style: Dict | null; // = null;

	private get __view__() {
		return this.m_vdom ? this.m_vdom.dom.__view__: this.m_placeholder;
	}

	@event readonly onLoad: EventNoticer<Event<void, ViewController>>;    // @event onLoad
	@event readonly onMounted: EventNoticer<Event<void, ViewController>>; // @event onMounted
	@event readonly onUpdate: EventNoticer<Event<void, ViewController>>;  // @event onUpdate
	@event readonly onRemove: EventNoticer<Event<void, ViewController>>;  // @event onRemove
	@event readonly onRemoved: EventNoticer<Event<void, ViewController>>; // @event onRemoved

	triggetLoad() {
		return this.trigger('Load');
	}

	triggetMounted() {
		return this.trigger('Mounted');
	}

	triggetUpdate() {
		return this.trigger('Update');
	}

	triggetRemove() {
		return this.trigger('Remove');
	}

	triggetRemoved() {
		return this.trigger('Removed');
	}

	get id() {
		return this.m_id;
	}

	static setID(dom: ViewController | View, id: string) {
		var _id = (dom as any).m_id;
		if (_id != id) {
			if ((dom as any).m_owner) {
				var ids = (dom as any).m_owner.m_IDs;
				if (ids[_id] === dom) {
					delete ids[_id];
				}
				if (id) {
					if (id in ids) {
						throw new Error('Identifier reference duplication in controller, = ' + id);
					}
					ids[id] = dom;
				}
			}
			(dom as any).m_id = id;
		}
	}

	set id(value: string) {
		ViewController.setID(this, value);
	}

	get IDs() {
		return this.m_IDs;
	}

	get owner() {
		return this.m_owner;
	}

	get dom(): DOM | null {
		return this.m_vdom ? this.m_vdom.dom: null;
	}

	get isLoaded() {
		return this.m_loaded;
	}

	get isMounted() {
		return this.m_mounted;
	}

	get vmodle() {
		return this.m_modle;
	}

	set vmodle(modle: Dict) {
		this.m_setModle(modle);
	}

	private m_setModle(modle: Dict) {
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

	appendTo(parentView: View) {
		return domInCtr(this).appendTo(parentView);
	}

	afterTo(prevView: View) {
		return domInCtr(this).afterTo(prevView);
	}

	/**
	 * @overwrite
	 */
	addDefaultListener(name: string, func: Listen<Event<any, ViewController>> | string) {
		if ( typeof func == 'string' ) {
			var owner = this as any, func2;
			do {
				var func2 = owner[func];  // find func
				if ( typeof func2 == 'function' ) {
					return this.addEventListener(name, func2, owner, '0'); // default id 0
				}
				owner = owner.m_owner;
			} while(owner);
			throw Error.new(`Cannot find a function named "${func}"`);
		} else {
			return super.addDefaultListener(name, func);
		}
	}

	/**
	 * @func render(...vdoms)
	 */
	render(...vdoms: any[]): any {
		return vdoms;
	}

	remove() {
		var vdom = this.m_vdom;
		var placeholder = this.m_placeholder;

		if (vdom || placeholder) {

			var owner = this.m_owner;
			if (owner) {
				util.assert(owner.dom !== this, 'Illegal call');
			}

			if (G_removeSet.has(this)) return;
			G_removeSet.add(this);
			try {
				this.triggerRemove(); // trigger Remove event
			} finally {
				G_removeSet.delete(this);
			}

			this.m_placeholder = null;
			this.m_vdom = null;

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
	static defineProps(props: any, controllerClass: any) {
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
	static typeOf(obj: any, Type: any) {
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

	/**
	 * @func render(obj, [parentView])
	 * @arg obj {VirtualDOM|View|ViewController|class}
	 * @ret {DOM} return dom instance
	 */
	static render(obj: any, parentView?: any) {
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
	}

	static hashCode() {
		return Function.prototype.hashCode.call(this);
	}

}

(ViewController as any).prototype.m_id = '';
(ViewController as any).prototype.m_owner = null;
(ViewController as any).prototype.m_placeholder = null;
(ViewController as any).prototype.m_vdom = null;
(ViewController as any).prototype.m_vchildren = [];
(ViewController as any).prototype.m_loaded = false;
(ViewController as any).prototype.m_mounted = false;
(ViewController as any).prototype.m_style = null;

export default ViewController;

/**
 * @set style {Object}
 */
ViewController.defineProps(['style']);

// export class RootViewController extends ViewController {
// 	//@overwrite
// 	render(vdom) {
// 		if (vdom) {
// 			util.assert(util.equalsClass(Root, vdom.type), 'RootViewController first children must be Root view');
// 			return vdom;
// 		} else {
// 			return _VV(Root, [], []); // return Root
// 		}
// 	}
// }

// create virtual dom TextNode
function _CVDT(value: string) {
	return new VirtualDOM(TextNode, {value}, []);
}

// create virtual dom dynamic
export function _CVDD(value: any): VirtualDOM | null {
	if (value instanceof VirtualDOM) {
		return value
	} else if (TEXT_NODE_VALUE_TYPE.has(typeof value)) {
		return _CVDT(value);
	} else if (Array.isArray(value)) {
		if (value.length) {
			return value.length == 1 ?
				_CVDD(value[0]): new VirtualDOMCollection(value.map(_CVDD));
		} else {
			return null;
		}
	}
	return value ? _CVDT(String(value)): null; // null or TextNode
}

// create virtual dom
export function _CVD<T extends typeof ViewController | typeof View>(Type: T, props: Dict | null, ...children: any[]) {
	return new VirtualDOM(Type, props, children);
}