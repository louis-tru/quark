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

import utils from './util';
import { StyleSheet } from './css';
import event, { Notification, EventNoticer, Listen, Event } from './event';
import { TextNode, View, DOM } from './_view';

const TEXT_NODE_VALUE_TYPE = new Set(['function', 'string', 'number', 'boolean']);
const G_removeSet = new WeakSet<ViewController>();
const G_renderQueueSet = new Set<ViewController>();
var   G_renderQueueWorking = false;
const G_warnRecord = new Set();
const G_warnDefine = {
	UndefinedDOMKey: 'DOM key no defined in DOM Collection',
} as Dict<string>;

function warn(id: string, msg = '') {
	var def = G_warnDefine[id];
	if (def) {
		if (!G_warnRecord.has(id)) {
			G_warnRecord.add(id);
			console.warn(def, msg);
		}
	}
}

// mark controller rerender
function markRerender(ctr: ViewController) {
	var size = G_renderQueueSet.size;
	G_renderQueueSet.add(ctr);
	if (size == G_renderQueueSet.size) return;
	if (G_renderQueueWorking) return;
	G_renderQueueWorking = true;

	utils.nextTick(function() {
		try {
			for( var item of G_renderQueueSet ) {
				rerender(item);
			}
		} finally {
			G_renderQueueWorking = false;
		}
	});
}

export interface DOMConstructor {
	new(...args: any[]): DOM;
	readonly isViewController: boolean;
}

/**
 * @class VirtualDOM
 */
export class VirtualDOM {
	readonly propsHash: number;
	readonly props: Dict<[any/*value*/,number/*hash*/]>;
	readonly domConstructor: DOMConstructor;
	readonly children: (VirtualDOM | null)[];
	dom: DOM;
	hash: number;

	constructor(domConstructor: DOMConstructor, props: Dict | null, children: (VirtualDOM | null)[]) {
		var _propsHash = 0;
		var _props: Dict<[any,number]> = {};

		for (var prop in props) {
			var hashCode = 0;
			var value = props[prop];
			hashCode += (hashCode << 5) + prop.hashCode();
			hashCode += (hashCode << 5) + Object.hashCode(value);
			_props[prop] = [value,hashCode];
			_propsHash += (_propsHash << 5) + hashCode;
		}
		var _hash = (domConstructor.hashCode() << 5) + _propsHash;

		for (var vdom of children) {
			if (vdom) {
				_hash += (_hash << 5) + vdom.hash;
			}
		}

		this.domConstructor = domConstructor;
		this.props = _props;
		this.hash = _hash;
		this.propsHash = _propsHash;
		this.children = children;
	}

	getProp(name: string) {
		var prop = this.props[name];
		return prop ? prop[0]: null;
	}

	hasProp(name: string) {
		return name in this.props;
	}

	assignProps() {
		var props = this.props;
		for (var key in props) {
			(this.dom as any)[key] = props[key][0]; // assignProp
		}
	}

	diffProps(vdom: VirtualDOM) {
		if (this.propsHash != vdom.propsHash) {
			var props0 = this.props;
			var props1 = vdom.props;
			for (var key in props1) {
				var prop0 = props0[key], prop1 = props1[key];
				if (!prop0 || prop0[1] != prop1[1]) {
					(this.dom as any)[key] = prop1[0]; // assignProp
				}
			}
		}
	}

	newInstance(ctr: ViewController | null): DOM {
		utils.assert(this.dom === defaultDOM);
		var dom = new this.domConstructor();
		this.dom = dom;
		(dom as any).m_owner = ctr; // private props visit

		if (this.domConstructor.isViewController) { // ctrl
			var newCtr = dom as ViewController;
			(newCtr as any).m_vchildren = this.children; // private props visit
			this.assignProps(); // before set props
			var r = (newCtr as any).triggerLoad(); // trigger event Load, private props visit
			if (r instanceof Promise) {
				r.then(()=>{
					(newCtr as any).m_loaded = true;
					markRerender(newCtr);
				}); // private props visit
			} else {
				(newCtr as any).m_loaded = true; // private props visit
			}
			rerender(newCtr); // rerender
		} else {
			for (var vdom of this.children) {
				if (vdom)
					vdom.newInstance(ctr).appendTo(dom as View);
			}
			this.assignProps(); // after set props
		}

		return dom;
	}

	hashCode() {
		return this.hash;
	}
}

const defaultDOM = {
	id: '',
	get __meta__(): View { throw Error.new('implemented') },
	get owner(): ViewController { throw Error.new('implemented') },
	remove() { throw Error.new('implemented') },
	appendTo(){ throw Error.new('implemented') },
	afterTo(){ throw Error.new('implemented')  },
	style: {},
} as DOM;

VirtualDOM.prototype.dom = defaultDOM;

/**
 * @class VirtualDOMCollection
 */
class VirtualDOMCollection extends VirtualDOM {
	vdoms: VirtualDOM[];
	keys: Dict<VirtualDOM> = {};
	placeholder: View | null = null; // view placeholder

	constructor(vdoms: (VirtualDOM | null)[]) {
		super(DOMCollection, {}, []);
		this.vdoms = vdoms.filter(e=>e) as VirtualDOM[];
		this.vdoms.forEach(e=>(this.hash += (this.hash << 5) + e.hash));
	}

	diffProps(vdom: VirtualDOM) {
		var { vdoms, hash } = vdom as VirtualDOMCollection;
		var dom = this.dom as DOMCollection;
		var keys: Dict<VirtualDOM> = {};
		var keys_c = this.keys; // private props visit
		var ctr = dom.owner;
		var prev: View = this.vdoms.length ? 
			this.vdoms[0].dom.__meta__: this.placeholder as View; // DOMCollection placeholder or doms[0]

		utils.assert(prev);

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
			if (this.placeholder) {
				this.placeholder.remove();
				this.placeholder = null;
			}
		} else if (!this.placeholder) {
			this.placeholder = new View();
			this.placeholder.afterTo(prev);
		}

		this.hash = hash;
		this.keys = keys;
		this.vdoms = vdoms;

		for (let key in keys_c) {
			removeDOM(ctr, keys_c[key]);
		}
	}

	newInstance(ctr: ViewController | null): DOM {
		utils.assert(this.dom === defaultDOM);
		var vdoms = this.vdoms;
		var keys = this.keys;
		this.dom = new DOMCollection(ctr, this);

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

/**
 * @class DOMCollection DOM
 */
class DOMCollection implements DOM {

	private m_owner: ViewController | null;
	private m_vdoms: VirtualDOMCollection;

	private _callDOMsFunc(active: 'afterTo' | 'appendTo', view: View): View {
		var self = this;
		if (self.m_vdoms.placeholder) {
			return self.m_vdoms.placeholder[active](view);
		} else {
			for (var cellView of self.m_vdoms.vdoms) {
				cellView.dom[active](view);
			}
			return self.m_vdoms.vdoms.indexReverse(0).dom.__meta__;
		}
	}

	get __meta__() {
		return this.m_vdoms.placeholder ? this.m_vdoms.placeholder: this.m_vdoms.vdoms.indexReverse(0).dom.__meta__;
	}

	id = '';

	get owner() {
		return this.m_owner;
	}

	get collection() {
		return this.m_vdoms.vdoms.map(e=>e.dom as DOM);
	}

	key(key: string) {
		var vdom = this.m_vdoms.keys[key];
		return vdom ? vdom.dom: null;
	}

	constructor(owner: ViewController | null, vdoms: VirtualDOMCollection) {
		this.m_owner = owner;
		this.m_vdoms = vdoms;
	}

	get style() { return {} }
	set style(value: StyleSheet) { }

	remove() {
		if (this.m_vdoms.placeholder) {
			this.m_vdoms.placeholder.remove();
			this.m_vdoms.placeholder = null;
		} else if (this.m_vdoms.vdoms.length) {
			this.m_vdoms.vdoms.forEach(vdom=>{
				removeDOM(this.m_owner, vdom);
			});
			this.m_vdoms.vdoms = [];
			this.m_vdoms.keys = {};
		}
	}

	appendTo(parentView: View) {
		return this._callDOMsFunc('appendTo', parentView);
	}

	afterTo(prevView: View) {
		return this._callDOMsFunc('afterTo', prevView);
	}

	static get isViewController() { return false; }

}

function removeSubctr(ctr: ViewController | null, vdom: VirtualDOM) {
	for (var e of vdom.children) {
		if (e) {
			if (e.domConstructor.isViewController) {
				e.dom.remove(); // remove ctrl
			} else {
				removeSubctr(ctr, e);
			}
		}
	}
	var id = vdom.dom.id;
	if (id && ctr) {
		if ((ctr as any).m_IDs[id] === vdom.dom) {
			delete (ctr as any).m_IDs[id];
		}
	}
}

function removeDOM(ctr: ViewController | null, vdom: VirtualDOM) {
	removeSubctr(ctr, vdom);
	vdom.dom.remove();
}

function diff(ctr: ViewController | null, vdom_c: VirtualDOM, vdom: VirtualDOM, prevView: View) {
	utils.assert(prevView);

	// diff type
	if (vdom_c.domConstructor !== vdom.domConstructor) {
		var r = vdom.newInstance(ctr).afterTo(prevView); // add new
		removeDOM(ctr, vdom_c); // del dom
		return r;
	}

	var dom = vdom_c.dom as DOM;
	vdom.dom = dom;

	// diff props
	vdom_c.diffProps(vdom); 

	var view = dom.__meta__;

	// diff children
	var children_c = vdom_c.children;
	var children = vdom.children;

	if ( vdom.domConstructor.isViewController ) {
		if ( children_c.length || children.length ) {
			(dom as any).m_vchildren = children; // private props visit
			rerender(dom as ViewController); // mark ctrl render
		}
	} else {
		var childrenCount = Math.max(children_c.length, children.length);

		for (var i = 0, prev = null; i < childrenCount; i++) {
			let vdom_c = children_c[i];
			let vdom = children[i];
			if (vdom_c) {
				if (vdom) {
					if (vdom_c.hash != vdom.hash) {
						prev = diff(ctr, vdom_c, vdom, prev || vdom_c.dom.__meta__); // diff
					} else {
						children[i] = vdom_c;
						prev = vdom_c.dom.__meta__;
					}
				} else {
					removeDOM(ctr, vdom_c); // remove DOM
				}
			} else {
				if (vdom) {
					var dom = vdom.newInstance(ctr);
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

function domInCtr(self: ViewController): DOM {
	return (self as any).m_vdom ? (self as any).m_vdom.dom: (self as any).m_placeholder;
}

/**
 * @func prop()
 * <pre>
 * 	class MyViewController extends ViewController {
 *		@prop width: number = 100;
 *		@prop height: number = 100;
 * 		render() {
 * 			return (
 * 				<Div width=this.width height=this.height>Hello</Div>
 * 			);
 * 		}
 * 	}
 * </pre>
 */

function defineProp<T extends any/*typeof ViewController.prototype*/>(target: T, name: string, defaultValue?: any) {
	var _target = target as any;
	utils.assert(utils.equalsClass(ViewController, _target.constructor), 'Type error');
	var m_name = 'm_' + name;
	var m_hash_name = 'm_prop_' + name;
	Object.defineProperty(_target, name, {
		get: arguments.length < 3 ? function(this: any) {
			return this[m_name];
		}: typeof defaultValue == 'function' ? defaultValue: ((_target[m_name] = defaultValue), function(this: any) {
			return this[m_name];
		}),
		set(this: any, value: any) {
			var hashCode = Object.hashCode(value);
			var hash = this.m_dataHash;
			if (hash[m_hash_name] != hashCode) {
				hash[m_hash_name] = hashCode;
				this[m_name] = value;
				this.markRerender(); // mark render
			}
		},
		configurable: false,
		enumerable: true,
	});
}

// export declare function prop<T extends typeof ViewController.prototype>(target: T, name: string): void;
// export declare function prop(defaultValue: (()=>any) | any): <T extends typeof ViewController.prototype>(target: T, name: string)=>void;
export declare function prop<T extends object>(target: T, name: string): void;
export declare function prop(defaultValue: (()=>any) | any): <T extends object>(target: T, name: string)=>void;

exports.prop = function(defaultValueOrTarget: any, name?: string) {
	if (arguments.length < 2) {
		return function(target: any, name: any) {
			defineProp(target, name, defaultValueOrTarget/*default value*/);
		};
	} else {
		defineProp(defaultValueOrTarget/*target*/, name as string);
	}
};

const _prop = exports.prop;

function rerender(ctr: ViewController) {
	ViewController.rerender(ctr);
}

export enum TypeOf {
	NONE = 0,
	DOM = 1,
	VDOM = 2,
}

/**
 * @class ViewController DOM
 */
export class ViewController<State extends Dict = Dict> extends Notification<Event<any, ViewController>> implements DOM {
	private m_IDs: Dict<ViewController | View> = {};
	private m_state: State = {} as State; // state
	private m_dataHash: Dict<number> = {}; // modle and props hash
	private m_id: string; // = null;     // id
	private m_owner: ViewController | null; // = null;  // owner controller
	private m_placeholder: View | null; // = null; // view placeholder	
	private m_vdom: VirtualDOM | null; // = null;     // children vdom
	private m_vchildren: VirtualDOM[]; // = []; // outer vdom children
	private m_loaded: boolean; // = false;
	private m_mounted: boolean; // = false;
	private m_style: StyleSheet | null; // = null;

	static rerender(self: ViewController) {
		G_renderQueueSet.delete(self); // delete mark
	
		var vdom_c = self.m_vdom;
		var vdom = _CVDD(self.render(...self.m_vchildren));
		var update = false;
	
		if (vdom_c) {
			if (vdom) {
				if (vdom_c.hash != vdom.hash) {
					var prev = vdom_c.dom.__meta__;
					utils.assert(prev);
					self.m_vdom = vdom;
					diff(self, vdom_c, vdom, prev); // diff
					update = true;
				}
			} else {
				var prev = vdom_c.dom.__meta__;
				utils.assert(prev);
				utils.assert(!self.m_placeholder);
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

	get __meta__(): View {
		return this.m_vdom ? this.m_vdom.dom.__meta__: this.m_placeholder as View;
	}

	@event readonly onLoad: EventNoticer<Event<void, ViewController>>;    // @event onLoad
	@event readonly onMounted: EventNoticer<Event<void, ViewController>>; // @event onMounted
	@event readonly onUpdate: EventNoticer<Event<void, ViewController>>;  // @event onUpdate
	@event readonly onRemove: EventNoticer<Event<void, ViewController>>;  // @event onRemove
	@event readonly onRemoved: EventNoticer<Event<void, ViewController>>; // @event onRemoved

	protected triggerLoad(): any {
		return this.trigger('Load');
	}

	protected triggerMounted(): any {
		return this.trigger('Mounted');
	}

	protected triggerUpdate(): any {
		return this.trigger('Update');
	}

	protected triggerRemove(): any {
		return this.trigger('Remove');
	}

	protected triggerRemoved(): any {
		return this.trigger('Removed');
	}

	get id() {
		return this.m_id;
	}

	set id(value: string) {
		ViewController._setID(this, value);
	}

	get IDs() {
		return this.m_IDs;
	}

	find<T extends View | ViewController = View>(id: string) {
		var r = this.m_IDs[id];
		utils.assert(r, 'ViewController.find<T>(id) = null');
		return r as T;
	}

	static _setID(dom: DOM, id: string) {
		var _id = (dom as any).m_id; // private props visit
		if (_id != id) {
			if ((dom as any).m_owner) { // private props visit
				var ids = (dom as any).m_owner.m_IDs; // private props visit
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
			(dom as any).m_id = id; // private props visit
		}
	}

	get owner() {
		return this.m_owner;
	}

	get dom(): DOM | null {
		return this.m_vdom ? this.m_vdom.dom: null;
	}

	/**
	 * Insecure access
	 */
	ownerAs<T extends ViewController = ViewController>() {
		utils.assert(this.m_owner, 'ViewController.ownerAs<T>() = null');
		return this.m_owner as T;
	}

	/**
	 * Insecure access
	 */
	domAs<T extends DOM = View>() {
		utils.assert(this.m_vdom, 'ViewController.domAs<T>() = null');
		return (this.m_vdom as VirtualDOM).dom as T;
	}

	get isLoaded() {
		return this.m_loaded;
	}

	get isMounted() {
		return this.m_mounted;
	}

	/**
	 * @get state view model
	 */
	get state() {
		return this.m_state;
	}

	/**
	 * @set state view model
	 */
	set state(state: State) {
		this.setState(state);
	}

	/**
	 * @func setState()
	 */
	setState(state: State) {
		var update = false;
		var value = this.m_state;
		var hash = this.m_dataHash;
		for (var key in state) {
			var item = state[key];
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

	hashCode() {
		return Function.prototype.hashCode.call(this);
	}

	appendTo(parentView: View): View {
		return domInCtr(this).appendTo(parentView);
	}

	afterTo(prevView: View): View {
		return domInCtr(this).afterTo(prevView);
	}

	/**
	 * @func render(...vdoms)
	 */
	protected render(...vdoms: any[]): any {
		return vdoms;
	}

	remove() {
		var vdom = this.m_vdom;
		var placeholder = this.m_placeholder;

		if (vdom || placeholder) {

			var owner = this.m_owner;
			if (owner) {
				utils.assert(owner.dom !== this, 'Illegal call');
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
				(placeholder as View).remove();
			}
			this.triggerRemoved(); // trigger Removed
		}
	}

	/**
	 * @prop style
	 */
	@_prop(function(this: ViewController) {
		return this.m_style || {};
	})
	style: StyleSheet;

	/**
	 * @get isViewController
	 * @static
	 */
	static get isViewController() {
		return true;
	}

	/**
	 * @func typeOf(obj, [Type=class ViewController])
	 * @arg obj {VirtualDOM|View|ViewController|class}
	 * @static
	 */
	static typeOf(obj: DOM | VirtualDOM, Type?: DOMConstructor/* = ViewController*/): TypeOf {
		Type = Type || ViewController;
		if (utils.equalsClass(ViewController, Type) || utils.equalsClass(View, Type)) {
			if (obj instanceof Type)
				return TypeOf.DOM; // dom instance
			if (obj instanceof VirtualDOM) {
				if (utils.equalsClass(Type, obj.domConstructor))
					return TypeOf.VDOM; // vdom instance
			}
		}
		return 0;
	}

	/**
	 * @func render(obj, [parentView])
	 * @arg obj {DOM | DOMConstructor}
	 * @ret {DOM} return dom instance
	 */
	static render<T extends DOM = DOM>(obj: DOM | VirtualDOM, parentView?: View) {
		var dom: DOM;
		var owner = parentView ? parentView.owner: null;

		if (obj instanceof ViewController || obj instanceof View) {
			dom = obj; // dom instance
		} else {
			let vd = _CVDD(obj) as VirtualDOM; // format vdom
			utils.assert(vd instanceof VirtualDOM, 'Bad argument');
			dom = vd.newInstance(owner);
		}
		if (parentView) {
			dom.appendTo(parentView);
			// (dom as any).m_owner = owner; // private props visit
		}
		return dom as T;
	}

	static hashCode() {
		return Function.prototype.hashCode.call(this);
	}

	static readonly CVD = _CVD;
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
export function _CVD(Type: DOMConstructor, props: Dict | null, ...children: any[]) {
	return new VirtualDOM(Type, props, children.map(_CVDD));
}