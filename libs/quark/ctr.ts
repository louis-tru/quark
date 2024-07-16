/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

import util from './util';
import { Label, View, DOM } from './view';
import { Window } from './window';
import * as view from './view';

const assertDev = util.assert;
const RenderQueueSet = new Set<ViewController>();
let   RenderQueueWorking = false;
const WarnRecord = new Set();
const WarnDefine = {
	UndefinedDOMKey: 'DOM key no defined in DOM Collection',
} as Dict<string>;

function warn(id: string, msg = '') {
	if (!WarnRecord.has(id)) {
		WarnRecord.add(id);
		let def = WarnDefine[id];
		if (def) {
			console.warn(def, msg);
		}
	}
}

export type Args = {
	window: Window,
	owner: ViewController,
	children: (VirtualDOM | null)[],
}

interface DOMConstructor<T extends DOM = DOM> {
	new(...args: any[]): T;
	readonly isViewController: boolean;
}

export function assertDom<T extends DOM = DOM>(
	subclass: VirtualDOM<T>, baseclass: DOMConstructor<T>, ...args: any[]
) {
	util.assert(util.equalsClass(baseclass, subclass.domC), ...args);
}

/**
 * @method link ViewController prop
*/
export function link(target: any, name: string) {
	let linkProps = Object.getOwnPropertyDescriptor(target, '_linkProps');
	if (linkProps) {
		linkProps.value.push(name);
	} else {
		target._linkProps = [...target._linkProps,name];
	}
}

/**
 * @method link ViewController prop and define prop accessor
*/
export function linkAcc(target: any, name: string) {
	link(target, name);
	Object.defineProperty(target, name, {
		get: function() { return this[`_name`] },
		set: function(val) {
			this[`_name`] = val;
			if (this.isMounted)
				this.update(); // update ViewController
		}
	})
}

link.acc = linkAcc;

function getkey(vdom: VirtualDOM, autoKey: number): string|number {
	let key: string;
	let key_ = vdom.props.key;
	if (key_ !== undefined) {
		key = key_;
	} else {
		key = String(autoKey); // auto number key
		warn('UndefinedDOMKey');
	}
	return key;
}

function unref<T>(dom: DOM, owner: ViewController<T>) {
	let ref = dom.ref;
	if (ref) {
		if (owner.refs[ref] === dom) {
			delete (owner.refs as Dict<DOM>)[ref];
		}
	}
}

function setref(dom: View | ViewController, owner: ViewController, value: string) {
	let ref = dom.ref;
	if (ref !== value) {
		let refs = owner.refs as Dict<ViewController | View>;
		if (refs[ref] === dom) {
			delete refs[ref];
		}
		if (value)
			refs[value] = dom;
		(dom as any).ref = value;
	}
}

function markrerender<T>(ctr: ViewController<T>) {
	let size = RenderQueueSet.size;
	RenderQueueSet.add(ctr as ViewController);
	if (size == RenderQueueSet.size)
		return;

	if (!RenderQueueWorking) {
		RenderQueueWorking = true;
		util.nextTick(function() {
			try {
				for( let item of RenderQueueSet ) {
					rerender(item);
				}
			} finally {
				RenderQueueWorking = false;
			}
		});
	}
}

function rerender(Self: ViewController) {
	RenderQueueSet.delete(Self); // delete mark

	type InlCrt = {
		_vdom?: VirtualDOM;
		_rerenderCbs?: (()=>void)[];
		isMounted: boolean;
		isDestroyd: boolean;
		dom: DOM;
		render: ()=>any;
		triggerMounted: ()=>any;
		triggerUpdate: (vdomOld: VirtualDOM, vdomNew: VirtualDOM)=>any;
	};

	let self = Self as unknown as InlCrt;
	if (self.isDestroyd)
		return;

	let dom = self.dom;
	let vdomOld = self._vdom;
	let vdomNew = _CVDD(self.render()) || EmptyVDom;

	if (vdomOld) {
		if (vdomOld.hash !== vdomNew.hash) {
			self._vdom = vdomNew; // use new vdom
			self.dom = vdomNew.diff(Self, vdomOld, dom); // diff
			self.triggerUpdate(vdomOld!, vdomNew);
		}
	} else { // once rerender
		self._vdom = vdomNew;
		self.dom = vdomNew.newDom(Self);
	}

	if (!self.isMounted) {
		self.isMounted = true;
		self.triggerMounted();
	}

	if (self._rerenderCbs) {
		let cbs = self._rerenderCbs;
		self._rerenderCbs = undefined;
		for (let cb of cbs)
			cb();
	}
}

/**
 * @class VirtualDOM
*/
export class VirtualDOM<T extends DOM = DOM> {
	readonly domC: DOMConstructor<T>;
	readonly children: (VirtualDOM | null)[];
	readonly props: Readonly<Dict>;
	readonly hashProp: number;
	readonly hashProps: Map<string, number>; // prop => hanshCode
	readonly hash: number;

	constructor(domC: DOMConstructor<T>, props: Dict | null, children: (VirtualDOM | null)[]) {
		let hashProp = 5381;
		if (props) {
			let hashProps = new Map();
			for (let prop in props) {
				let value = props[prop];
				let hashCode = (prop.hashCode() << 5) + Object.hashCode(value);
				hashProps.set(prop, hashCode);
				hashProp += (hashProp << 5) + hashCode;
			}
			this.props = props;
			this.hashProps = hashProps;
		}
		let hash = (domC.hashCode() << 5) + hashProp;

		if (children.length) {
			for (let vdom of children) {
				if (vdom)
					hash += (hash << 5) + vdom.hash;
			}
			this.children = children;
		}
		this.domC = domC;
		this.hash = hash;
		this.hashProp = hashProp;
	}

	getPropHash(prop: string): number | undefined {
		return this.hashProps.get(prop);
	}

	diffProps(dom: T, vdomOld: VirtualDOM) {
		if (this.hashProp !== vdomOld.hashProp) {
			let props = this.props;
			for (let [key,hash] of this.hashProps) {
				if (hash !== vdomOld.getPropHash(key)) {
					(dom as any)[key] = props[key];
				}
			}
		}
	}

	private diffPropsFor(dom: T, vdomOld: VirtualDOM, keys: string[]) {
		if (keys.length) {
			if (this.hashProp !== vdomOld.hashProp) {
				const {hashProps,props} = this;
				for (const key of keys) {
					if (hashProps.get(key) !== vdomOld.getPropHash(key)) {
						(dom as any)[key] = props[key];
					}
				}
			}
		}
	}

	diff<P = {}, S = {}>(owner: ViewController<P,S>, vdomOld: VirtualDOM, domOld: DOM): T {
		let vdomNew: VirtualDOM<T> = this;
		if (vdomOld.domC !== vdomNew.domC) { // diff type
			let prev = domOld.metaView; assertDev(prev);
			let newDom = this.newDom(owner);
			newDom.afterTo(prev);
			domOld.destroy(owner);
			return newDom;
		}

		if ( vdomNew.domC.isViewController ) {
			let ctr = domOld as ViewController;
			(ctr as {props:any}).props = vdomNew.props;
			(ctr as {children:any}).children = vdomNew.children;
			setref(ctr, owner, vdomNew.props.ref || '');
			// link props
			vdomNew.diffPropsFor(domOld as T, vdomOld, (ctr as any)._linkProps);
			rerender(ctr); // rerender
		} else {
			// diff and set props
			setref(domOld as View, owner, vdomNew.props.ref || '');
			vdomNew.diffProps(domOld as T, vdomOld);
	
			let childrenOld = vdomOld.children, childrenNew = vdomNew.children;
			let len = Math.max(childrenOld.length, childrenNew.length);
	
			if (len) {
				let childrenDomOld: (DOM|undefined)[] = (domOld as any)._children; // View._children
				let childrenDomNew: (DOM|undefined)[] = new Array(len);
				let prev: View | null = null;
	
				for (let i = 0; i < len; i++) {
					let vdomOld = childrenOld[i], vdomNew = childrenNew[i];
					if (vdomOld) {
						let dom = childrenDomOld[i]!;
						if (vdomNew) {
							if (vdomOld.hash !== vdomNew.hash) {
								dom = vdomNew.diff(owner, vdomOld, dom); // diff
							} else {
								childrenNew[i] = vdomOld;
							}
							childrenDomNew[i] = dom;
							prev = dom.metaView;
						} else {
							dom.destroy(owner); // destroy DOM
						}
					} else if (vdomNew) {
						let dom = vdomNew.newDom(owner);
						if (prev) {
							prev = dom.afterTo(prev);
						} else {
							prev = dom.appendTo(dom.metaView);
						}
						childrenDomNew[i] = dom;
					}
				}
				(domOld as any)._children = childrenDomNew; // View._children = childrenDomNew
			}
		} // if (vdomNew.domNew.isViewController)

		return domOld as T;
	}

	render<P = {}, S = {}>(owner: ViewController<P,S>, opts?: {
		parent?: View | null,
		vdom?: VirtualDOM | null, // diff prev vdom
		dom?: DOM | null // diff prev dom
	}): T {
		const {parent,vdom,dom} = opts||{};
		let domNew: T;
		if (vdom && dom) {
			if (vdom.domC !== this.domC) { // diff type
				domNew = this.newDom(owner);
				parent && domNew.appendTo(parent);
				dom.destroy(owner);
			} else {
				domNew = this.diff(owner, vdom, dom!);
				if (parent && parent !== dom.metaView.parent) {
					domNew.appendTo(parent);
				}
			}
		} else {
			domNew = this.newDom(owner);
			parent && domNew.appendTo(parent);
		}
		return domNew;
	}

	newDom<P = {}, S = {}>(owner: ViewController<P,S>): T {
		const window = owner.window;
		const {hashProps,props} = this;
		if (this.domC.isViewController) {
			let dom = new this.domC(props, {
				owner, window, children: this.children
			});
			let newCtr = dom as DOM as ViewController;
			let r = (newCtr as any).triggerLoad(); // trigger event Load
			if (r instanceof Promise) {
				r.then(()=>{
					(newCtr as any).isLoaded = true;
					markrerender(newCtr);
				});
			} else {
				(newCtr as {isLoaded:boolean}).isLoaded = true;
			}
			setref(newCtr, owner, props.ref || '');
			// link props
			for (const key of (newCtr as any)._linkProps as string[]) {
				if (hashProps.has(key))
					(newCtr as any)[key] = props[key];
			}
			rerender(newCtr); // rerender
			return dom;
		}
		else { // view
			let children = this.children, len = children.length;
			let view = new this.domC(window) as DOM as View;
			let prev: View | null = null;
			if (len) {
				let childrenDom: (DOM|undefined)[] = new Array(len);
				for (let i = 0; i < len; i++) {
					let vdom = children[i];
					if (vdom) {
						let dom = vdom.newDom(owner);
						childrenDom[i] = dom;
						prev = prev ? dom.afterTo(prev): dom.appendTo(view);
					}
				}
				(view as any)._children = childrenDom;
			}
			setref(view, owner, props.ref || '');
			Object.assign(view, props);
			return view as DOM as T;
		}
	}
}
Object.assign(VirtualDOM.prototype, {
	children: [], props: {}, hashProps: new Map,
});

const EmptyVDom = new VirtualDOM(View, null, []);

class VirtualDOMText extends VirtualDOM<Label> {
	readonly value: string;
	static fromString(value: string): VirtualDOM {
		return {
			__proto__: VirtualDOMText.prototype,
			value,
			hash: value.hashCode(),
		} as unknown as VirtualDOMText;
	}
	getPropHash(prop: string) {
		return this.hash;
	}
	diffProps(dom: Label, vdomOld: VirtualDOM): void {
		if (vdomOld.getPropHash('value') !== this.hash) {
			dom.value = this.value;
		}
	}
	newDom<P = {}, S = {}>(owner: ViewController<P,S>): Label {
		let view = new Label(owner.window);
		view.value = this.value;
		return view;
	}
}
util.extend(VirtualDOMText.prototype, {
	domC: Label, get hashProp() {return this.hash},
});

class VirtualDOMCollection extends VirtualDOM<DOMCollection> {
	collection: VirtualDOM[];

	constructor(collection: (VirtualDOM | null)[]) {
		super(DOMCollection, null, []);
		let _collection: VirtualDOM[] = [];
		let hash = this.hash;
		for (let e of collection) {
			if (e) {
				hash += (hash << 5) + e.hash;
				_collection.push(e);
			}
		}
		if (!_collection.length) {
			let first = new VirtualDOM(View, null, []);
			hash += (hash << 5) + first.hash;
			_collection.push(first);
		}
		(this as {hash:number}).hash = hash;
		this.collection = _collection;
	}

	diffProps(dom: DOMCollection, vdomOld: VirtualDOM) {
		let {keys:keysOld,owner} = dom;
		let collection: DOM[] = new Array(this.collection.length);
		let keys = new Map<string|number, [DOM,VDom]>();
		let prev = dom.metaView; assertDev(prev);

		this.collection.forEach(function(vdom, i) {
			let key = getkey(vdom, i);
			if (keys.has(key))
				throw new Error('DOM Key definition duplication in DOM Collection, = ' + key);
			let dom: DOM;

			let old = keysOld.get(key);
			if (old) {
				let [domOld, vdomOld] = old;
				if (vdomOld.hash !== vdom.hash) {
					dom = vdom.diff(owner, vdomOld, domOld); // diff
					prev = dom.metaView;
				} else { // use old dom
					dom = domOld;
					prev = domOld.afterTo(prev);
				}
				keysOld.delete(key);
			} else { // no key
				dom = vdom.newDom(owner);
				prev = dom.afterTo(prev);
			}
			collection[i] = dom;
			keys.set(key, [dom, vdom]);
		});
		(dom as any).keys = keys;
		(dom as any).collection = collection;

		for (let [_,[dom]] of keysOld) {
			dom.destroy(owner);
		}
	}

	newDom<P = {}, S = {}>(owner: ViewController<P, S>): DOMCollection {
		let dom = new DOMCollection(owner);
		let {collection,keys} = dom;
		this.collection.forEach(function(vdom,i) {
			let key = getkey(vdom, i);
			if (key in keys)
				throw new Error('DOM Key definition duplication in DOM Collection, = ' + key);
			let dom = vdom.newDom(owner);
			collection.push(dom);
			keys.set(key, [dom,vdom]);
		});
		return dom;
	}
}

class DOMCollection implements DOM {
	readonly collection: DOM[];
	readonly keys: Map<string|number, [DOM,VDom]>;
	readonly ref: string;
	readonly owner: ViewController;
	get metaView() {
		return this.collection.indexReverse(0).metaView;
	}
	constructor(owner: ViewController) {
		this.owner = owner;
		this.collection = [];
		this.keys = new Map<string, [DOM,VDom]>;
	}
	appendTo(parent: View) {
		for (let dom of this.collection)
			dom.appendTo(parent);
		return this.metaView;
	}
	afterTo(prev: View) {
		for (let dom of this.collection)
			prev = dom.afterTo(prev);
		return prev;
	}
	destroy(owner: ViewController) {
		for (let dom of this.collection) {
			dom.destroy(owner);
		}
		this.collection.splice(0);
		this.keys.clear();
	}
	static readonly isViewController = false;
}
Object.assign(DOMCollection.prototype, {ref: ''});

/**
 * UI view controller component
 * 
 * @class ViewController
*/
export class ViewController<P = {}, S = {}> implements DOM {
	private _stateHashs = new Map<string, number>;
	private _vdom?: VirtualDOM; // render result
	private _rerenderCbs?: (()=>void)[]; // rerender callbacks
	private _linkProps: string[];
	readonly window: Window;
	readonly owner: ViewController;
	readonly children: (VirtualDOM | null)[]; // outer vdom children
	readonly props: Readonly<P>;
	readonly state: Readonly<S> = {} as S;
	readonly refs: Readonly<Dict<ViewController | View>> = {};
	readonly ref: string;
	readonly dom: DOM;
	readonly isLoaded: boolean;
	readonly isMounted: boolean;
	readonly isDestroyd: boolean;

	/**
	 * mount point for view controller
	 * @prop metaView
	*/
	get metaView() { return this.dom.metaView }

	constructor(props: Readonly<P>, {window,children,owner}: Args) {
		this.props = props;
		this.window = window;
		this.children = children;
		this.owner = owner;
	}

	setState<K extends keyof S>(newState: Pick<S, K>, cb?: ()=>void) {
		let update = false;
		let stateHashs = this._stateHashs;
		let state = this.state as S;
		for (let key in newState as S) {
			let item = (newState as S)[key];
			let hash = Object.hashCode(item);
			if (hash != stateHashs.get(key)) {
				state[key] = item;
				stateHashs.set(key, hash);
				update = true;
			}
		}
		if (update) {
			this.update(cb);
		} else if (cb) {
			cb();
		}
	}

	update(cb?: ()=>void) {
		if (this.isDestroyd)
			return;
		if (cb) {
			if (this._rerenderCbs) {
				this._rerenderCbs.push(cb);
			} else {
				this._rerenderCbs = [cb];
			}
		}
		markrerender(this);
	}

	domAs<T extends ViewController | View = View>() {
		return this.dom as T;
	}

	refAs<T extends ViewController | View = View>(ref: string): T {
		return this.refs[ref] as T;
	}

	hashCode() {
		return Function.prototype.hashCode.call(this);
	}

	protected triggerLoad(): any {}
	protected triggerMounted(): any {}
	protected triggerUpdate(old: VirtualDOM, vdom: VirtualDOM): any {}
	protected triggerDestroy(): any {}
	protected render(): RenderResult {}

	appendTo(parent: View): View {
		return this.dom.appendTo(parent);
	}

	afterTo(prev: View): View {
		return this.dom.afterTo(prev);
	}

	/**
	 * Do not proactively call this method
	*/
	destroy() {
		if (!this.isDestroyd) {
			(this as any).isDestroyd = true;
			this.triggerDestroy(); // trigger event
			unref(this, this.owner);
			this.dom.destroy(this);
		}
	}

	static hashCode() {
		return Function.prototype.hashCode.call(this);
	}

	static readonly isViewController: boolean = true;
}
Object.assign(ViewController.prototype, {
	dom: { // init default dom
		ref: '',
		get metaView(): View { throw Error.new('Not implemented') },
		destroy() { throw Error.new('Not implemented') },
		appendTo(){ throw Error.new('Not implemented') },
		afterTo(){ throw Error.new('Not implemented') },
	} as DOM,
	_linkProps: [],
	ref: '', _vdom: undefined,
	isLoaded: false, isMounted: false, isDestroyd: false,
});

export default ViewController;

// create virtual dom dynamic
function _CVDD(value: any): VirtualDOM | null {
	if (value instanceof VirtualDOM) {
		return value
	} else if (Array.isArray(value)) {
		if (value.length) {
			return value.length === 1 ?
				_CVDD(value[0]): new VirtualDOMCollection(value.map(_CVDD));
		}
		return null;
	}
	return value ? VirtualDOMText.fromString(value): null;
}

declare global {
	namespace JSX {
		type Element = VirtualDOM<any>;
	}
}

const DOMConstructors: { [ key in JSX.IntrinsicElementsName ]: DOMConstructor<DOM> } = {
	view: view.View, box: view.Box,
	flex: view.Flex, flow: view.Flow,
	free: view.Free, image: view.Image, img: view.Image,
	matrix: view.Matrix, text: view.Text,
	button: view.Button, label: view.Label,
	input: view.Input, textarea: view.Textarea, scroll: view.Scroll,
};

// create virtual dom, jsx element
export function createElement<T extends DOM = DOM>(
	Type: DOMConstructor<T> | JSX.IntrinsicElementsName,
	props: Dict | null, ...children: any[]): VirtualDOM
{
	if (typeof Type == 'string') {
		Type = DOMConstructors[Type] as DOMConstructor<T>;
	}
	return new VirtualDOM(Type, props, children.map(_CVDD));
}

export type VDom<T extends DOM = DOM> = VirtualDOM<T>;
export const VDom = VirtualDOM;

export const _CVD = createElement;
export type RenderNode = VirtualDOM | string | null | undefined | void;
export type RenderResult = RenderNode[] | RenderNode;