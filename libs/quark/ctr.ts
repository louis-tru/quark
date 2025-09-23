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

/**
 * This file implements a lightweight Virtual DOM and Component (ViewController) system,
 * conceptually similar to React. Core features include:
 * - VirtualDOM: represents virtual nodes, supports diffing
 * - ViewController: component-like abstraction with state, props, lifecycle, and render logic
 * - DOMCollection: represents a list of children nodes (e.g., from JSX arrays)
 * - Render Queue: batches re-render requests to avoid redundant renders
 */

import './_ext';
import util from './util';
import { Label, View, DOM } from './view';
import { Window } from './window';
import * as view from './view';
import pkg from './pkg';

/**
 * A special Set implementation that ignores all operations.
 * Used as a no-op placeholder when file-watching is disabled (non-dev mode).
 */
class InvalidSet<T> extends Set<T> {
	add(k: T) { return this }
	delete(k: T) { return false }
}

const isWatching = pkg.isWatching;
const assertDev = util.assert;

/**
 * Render Queue:
 * Holds ViewControllers that are scheduled for re-render.
 * Key: controller, Value: {missError} whether to suppress re-throwing errors.
 */
const RenderQueue = new Map<ViewController, {missError?: boolean}>();
let   RenderQueueWorking = false;

/**
 * Debug-only: tracks all controllers with active file-watch subscriptions.
 */
const WatchingAllCtrForDebug = isWatching ?
	new Set<ViewController>(): new InvalidSet<ViewController>();

/**
 * Warning system: prevents duplicate warning logs.
 */
const WarnRecord = new Set();
const WarnDefine = {
	UndefinedDOMKey: 'DOM key not defined in DOM Collection',
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

/**
 * File watcher (hot reload support).
 * When a file changes, any watching controllers are re-rendered.
 */
pkg.onFileChanged.on(function({data:{name,hash}}) {
	for (let ctr of WatchingAllCtrForDebug) {
		if ((ctr as any)._watchings.has(name)) {
			markrerender(ctr, true); // schedule re-render
			console.log(`Re-render: ${ctr.constructor.name} view-controller from ${name}, hash: ${hash}`);
		}
	}
});

// -----------------------------
// Virtual DOM Helpers
// -----------------------------

/**
 * Arguments used to construct a VirtualDOM instance.
 */
export type Args = {
	window: Window,     //!< bound window instance
	owner: ViewController, //!< parent controller
	children: (VirtualDOM | null)[], //!< child virtual nodes
}

/**
 * Constructor type for DOM elements.
 */
interface DOMConstructor<T extends DOM = DOM> {
	new(...args: any[]): T;
	readonly isViewController: boolean;
	readonly __filename__?: string; // debugging support
}

/**
 * Assert that a VirtualDOM belongs to a specific DOM subclass.
 */
export function assertDom<T extends DOM = DOM>(
	subclass: VirtualDOM<T>, baseclass: DOMConstructor<T>, ...args: any[]
) {
	util.assert(util.equalsClass(baseclass, subclass.domC), ...args);
}

/**
 * Decorator to link an external prop to a class field.
 * 
 * - Copies the value from `props` into the class property when initializing/updating.
 * - **One-way binding only:** changes to props propagate into the class field,
 *   but class field changes do not propagate back to props.
 * - Useful for simplifying access: instead of `this.props.foo`, you can use `this.foo`.
 */
export function link(target: ViewController, name: string) {
	let linkProps = Object.getOwnPropertyDescriptor(target, '_linkProps');
	if (linkProps) {
		linkProps.value.push(name);
	} else {
		(target as any)._linkProps = [...(target as any)._linkProps,name];
	}
}

/**
 * Decorator to link an external prop with getter/setter accessors.
 * 
 * - Also a **one-way binding** from props to the class field.
 * - Provides an accessor so that if the field is reassigned inside the class,
 *   it triggers `update()` automatically (but does not affect props).
 */
export function linkAcc(target: ViewController, name: string) {
	link(target, name);
	Object.defineProperty(target, name, {
		get: function() { return this[`_${name}`] },
		set: function(val) {
			this[`_${name}`] = val;
			if (this.isMounted)
				this.update();
		}
	})
}

link.acc = linkAcc;

/**
 * Get the VirtualDOM key. Falls back to auto-index if not defined.
 * Warns if no key is provided in a DOM collection.
 */
function getkey(vdom: VirtualDOM, autoKey: number): string|number {
	let key: string;
	let key_ = vdom.props.key;
	if (key_ !== undefined) {
		key = key_;
	} else {
		key = String(autoKey);
		warn('UndefinedDOMKey');
	}
	return key;
}

/**
 * Remove a ref reference from a controller.
 */
function unref<T>(dom: DOM, owner: ViewController<T>) {
	let ref = dom.ref;
	if (ref) {
		if (owner.refs[ref] === dom) {
			delete (owner.refs as Dict<DOM>)[ref];
		}
	}
}

/**
 * Set or update a ref reference.
 */
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

// -----------------------------
// Render Queue Scheduling
// -----------------------------

/**
 * Mark a controller for re-render.
 * Added to the queue and processed on nextTick.
 */
function markrerender<T>(ctr: ViewController<T>, missError = false) {
	const size = RenderQueue.size;
	RenderQueue.set(ctr, {missError});

	if (size == RenderQueue.size)
		return; // already present

	if (!RenderQueueWorking) {
		RenderQueueWorking = true;
		util.nextTick(function() {
			let missErr;
			try {
				for(let [ctr,{missError}] of RenderQueue) {
					missErr = missError;
					rerender(ctr);
				}
			} catch(err) {
				if (!missErr)
					throw err;
				console.error('RenderQueue error:', err);
			} finally {
				RenderQueueWorking = false;
			}
		});
	}
}

/**
 * Performs actual re-rendering of a ViewController.
 * - Runs diff algorithm between old and new VirtualDOM.
 * - Triggers lifecycle hooks.
 * - Updates references.
 */
function rerender(Self: ViewController) {
	RenderQueue.delete(Self);

	type InlCrt = {
		_vdom?: VirtualDOM;
		_rerenderCbs?: (()=>void)[];
		_watchings: Set<any>;
		isMounted: boolean;
		isDestroyd: boolean;
		dom: DOM;
		render: ()=>any;
		triggerMounted: ()=>any;
		triggerUpdate: (vdomOld: VirtualDOM, vdomNew: VirtualDOM)=>any;
		constructor: { __filename__?: string };
	};

	let self = Self as unknown as InlCrt;
	if (self.isDestroyd)
		return;

	let dom = self.dom;
	let vdomOld = self._vdom;
	let vdomNew = _CVDD(self.render()) || EmptyVDom;

	RenderQueue.delete(Self); // remove from queue again

	if (vdomOld) {
		if (vdomOld.hash !== vdomNew.hash) {
			self._watchings.clear();
			self.dom = vdomNew.diff(Self, vdomOld, dom);
			self._vdom = vdomNew;
			self.triggerUpdate(vdomOld!, vdomNew);
		}
	} else {
		// initial render
		self.dom = vdomNew.newDom(Self);
		self._vdom = vdomNew;
	}

	if (self.isDestroyd) // check again
		return;

	if (isWatching) {
		self._watchings.delete(undefined);
		self._watchings.delete(self.constructor.__filename__);
		if (self._watchings.size) {
			WatchingAllCtrForDebug.add(Self);
		}
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

// -----------------------------
// VirtualDOM Implementation
// -----------------------------
// (Next sections: VirtualDOM, VirtualDOMText, VirtualDOMCollection, ViewController)
// will receive the same level of detailed English doc comments.
// -----------------------------

/**
 * Represents a Virtual DOM node.
 * Can be a View, ViewController, or other custom DOM elements.
 * 
 * @template T
 * @class VirtualDOM
*/
export class VirtualDOM<T extends DOM = DOM> {
	readonly domC: DOMConstructor<T>;
	readonly children: (VirtualDOM | null)[];
	readonly props: Readonly<Dict>;
	readonly hash: number; // root hash
	readonly hashProp: number; // props hash
	readonly hashProps: Map<string, number>; // prop => hanshCode

	/**
	 * @param domC DOM constructor type
	 * @param props Dictionary of properties/attributes
	 * @param children Array of child VirtualDOM nodes
	 */
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
				if (vdom) {
					hash += (hash << 5) + vdom.hash;
				}
			}
			this.children = children;
		}
		this.domC = domC;
		this.hashProp = hashProp;
		this.hash = hash;
	}

	/**
	 * Get hash for a specific property.
	 * @param prop Property name
	 * @return hash value
	 */
	getPropHash(prop: string): number | undefined {
		return this.hashProps.get(prop);
	}

	/**
	 * Diff props against old VirtualDOM and update the real DOM as needed.
	 * @param dom Real DOM instance
	 * @param vdomOld Previous VirtualDOM
	 */
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

	/**
	 * Diff a subset of props.
	 * @param dom Real DOM instance
	 * @param vdomOld Previous VirtualDOM
	 * @param keys Array of prop names to diff
	 */
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

	/**
	 * Diff this VirtualDOM with a previous one and update the real DOM.
	 * @param owner Owning ViewController
	 * @param vdomOld Previous VirtualDOM
	 * @param domOld Previous real DOM
	 * @return Updated DOM
	 */
	diff<P = {}, S = {}>(owner: ViewController<P,S>, vdomOld: VirtualDOM, domOld: DOM): T {
		let vdomNew: VirtualDOM<T> = this;
		if (vdomOld.domC !== vdomNew.domC) { // diff type
			let prev = domOld.metaView;
			assertDev(prev);
			let newDom = this.newDom(owner);
			newDom.afterTo(prev);
			domOld.destroy(owner);
			return newDom;
		}

		if (vdomNew.domC.isViewController) {
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
				let childDomsOld: (DOM|undefined)[] = (domOld as any).childDoms; // View.childDoms
				let childDomsNew: (DOM|undefined)[] = new Array(len);
				let prev: View | null = null;
	
				for (let i = 0; i < len; i++) {
					let vdomOld = childrenOld[i], vdomNew = childrenNew[i];
					if (vdomOld) {
						let dom = childDomsOld[i]!;
						if (vdomNew) {
							if (vdomOld.hash !== vdomNew.hash) {
								dom = vdomNew.diff(owner, vdomOld, dom); // diff
							} else {
								childrenNew[i] = vdomOld;
							}
							childDomsNew[i] = dom;
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
						childDomsNew[i] = dom;
					}
				}
				(domOld as any).childDoms = childDomsNew; // View.childDoms = childDomsNew
			}
		} // if (vdomNew.domC.isViewController)

		return domOld as T;
	}

	/**
	 * Creates a new real DOM instance from this VirtualDOM.
	 * @param owner Owning ViewController
	 * @return New DOM instance
	 */
	newDom<P = {}, S = {}>(owner: ViewController<P,S>): T {
		const window = owner.window;
		const {hashProps,props,children} = this;
		if (this.domC.isViewController) { // is view controller
			let dom = new this.domC(props, { owner, window, children });
			let newCtr = dom as DOM as ViewController;
			let r = (newCtr as any).triggerLoad(); // trigger event Load
			if (r instanceof Promise) {
				r.then(()=>{
					(newCtr as any).isLoaded = true;
					markrerender(newCtr); // mark rerender
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
			(owner as any)._watchings.add(this.domC.__filename__);
			rerender(newCtr); // rerender
			return dom;
		}
		else { // view
			let len = children.length;
			let view = new this.domC(window) as DOM as View;
			let prev: View | null = null;
			if (len) {
				let childDoms: (DOM|undefined)[] = new Array(len);
				for (let i = 0; i < len; i++) {
					let vdom = children[i];
					if (vdom) {
						let dom = vdom.newDom(owner);
						childDoms[i] = dom;
						prev = prev ? dom.afterTo(prev): dom.appendTo(view);
					}
				}
				(view as any).childDoms = childDoms;
			}
			setref(view, owner, props.ref || '');
			Object.assign(view, props);
			return view as DOM as T;
		}
	}

	/**
	 * Renders the VirtualDOM, optionally diffing against previous DOM.
	 * @param owner Owning ViewController
	 * @param opts Optional render options
	 * @return The rendered DOM
	 */
	render<P = {}, S = {}>(owner: ViewController<P,S>, opts?: {
		parent?: View | null,
		replace?: { // replace
			vdom?: VirtualDOM | null, // diff prev vdom
			dom?: DOM | null // diff prev dom
		},
	}): T {
		const {parent,replace} = opts||{};
		const {vdom,dom} = replace||{};
		let domNew: T;
		if (vdom && dom) {
			if (vdom.domC !== this.domC) { // diff type
				domNew = this.newDom(owner);
				parent && domNew.appendTo(parent);
				dom.destroy(owner); // destroy old dom
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
}
Object.assign(VirtualDOM.prototype, {
	children: [], props: {}, hashProps: new Map,
});

/* Empty VirtualDOM singleton */
const EmptyVDom = new VirtualDOM(View, null, []);

/**
 * Lightweight VirtualDOM representing text nodes.
 */
class VirtualDOMText extends VirtualDOM<Label> {
	readonly value: string; //!< Text content

	/**
	 * Creates a VirtualDOMText from a string.
	 * @param value Text string
	 */
	static fromString(value: string): VirtualDOMText {
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

/**
 * Represents a collection of VirtualDOM nodes.
 * Handles keyed diffing and ensures no duplicate keys.
 */
class VirtualDOMCollection extends VirtualDOM<DOMCollection> {
	collection: VirtualDOM[]; //!< Array of child VirtualDOMs

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

	/**
	 * Diff and apply collection props to DOMCollection.
	 * @param dom DOMCollection instance
	 * @param vdomOld Previous VirtualDOM
	 */
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

	/**
	 * Create a new DOMCollection from this VirtualDOMCollection.
	 * @param owner Owning ViewController
	 * @return New DOMCollection
	 */
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

/**
 * Collection of real DOM elements corresponding to VirtualDOMCollection.
 */
export class DOMCollection implements DOM {
	readonly collection: DOM[]; //!< Child DOM elements
	readonly keys: Map<string|number, [DOM,VDom]>; //!< Keyed mapping
	readonly ref: string;       //!< Ref name in parent
	readonly owner: ViewController; //!< Owning ViewController

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
 * UI view controller component.
 * Base class for all ViewControllers.
 * Handles state, props, children, refs, lifecycle hooks, and rendering.
 * @class ViewController
 * @implements DOM
*/
export class ViewController<P = {}, S = {}> implements DOM {
	private _stateHashs = new Map<string, number>; // Tracks previous state hashes
	private _vdom?: VirtualDOM;                    // Last rendered VirtualDOM
	private _linkProps: string[];                  // Linked prop names
	private _watchings: Set<any>;                  // Files watched (for hot reload)
	private _rerenderCbs?: (()=>void)[];           // Pending rerender callbacks
	/** Associated Window [`Window`] object ref */
	readonly window: Window;
	/** Parent controller which the current controller belongs */
	readonly owner: ViewController;
	// /** External children vdom */
	readonly children: (VirtualDOM | null)[];
	/** the ViewController external incoming attributes */
	readonly props: Readonly<P>;
	/** View Controller States */
	readonly state: Readonly<S> = {} as S;
	/** Subordinate objects referenced by the `ref` name */
	readonly refs: Readonly<Dict<ViewController | View>> = {};
	/** The Ref-Name in the `owner` controller */
	readonly ref: string;
	/** The rendered DOM object under the current ViewController */
	readonly dom: DOM;
	/** After triggering Load, it will be set to `true` */
	readonly isLoaded: boolean;
	/** It will be set to `true` before triggering Mounted */
	readonly isMounted: boolean;
	/** It will be set to `true` before triggering Destroy */
	readonly isDestroyd: boolean;

	/**
	 * @get metaView:View
	 * 
	 * Returns the mount point for view controller
	*/
	get metaView() { return this.dom.metaView }

	/**
	 * Constructs a ViewController.
	 * @method constructor(props,arg)
	 * @param props:object External props
	 * @param arg:Args Additional args (window, owner, children)
	*/
	constructor(
		props: Readonly<P & { ref?: string, key?: string|number }>,
		{window,children,owner}: Args
	) {
		if (isWatching) {
			this._watchings = new Set();
		}
		this.props = props;
		this.window = window;
		this.children = children;
		this.owner = owner;
	}

	/**
	 * Update the current ViewController state and re-render if the state does change
	 * @template K
	 * @method setState(newState,cb?)
	 * @param newState:object Partial state
	 * @param cb?:Function Optional callback after state applied
	*/
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

	/**
	 * Force re-rendering of the ViewController
	 * @method update(cb?)
	 * @param cb?:Function Optional callback after update
	*/
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
		if (this.isMounted) {
			markrerender(this);
		}
	}

	/**
	 * Template method, returns the current `dom` and returns it as template `T` type
	 *
	 * @return {DOM}
	*/
	asDom<T extends ViewController | View = View>() {
		return this.dom as T;
	}

	/**
	 * Template method, finds the subordinate `DOM` by ref name and returns it as template `T` type
	 * 
	 * @return {DOM}
	*/
	asRef<T extends ViewController | View = View>(ref: string): T {
		return this.refs[ref] as T;
	}

	hashCode() {
		return 18766890;//Function.prototype.hashCode.call(this);
	}

	/**
	 * Triggered before the first render call
	*/
	protected triggerLoad(): any {}

	/**
	 * Triggered after the first render call completed
	*/
	protected triggerMounted(): any {}

	/**
	 * Triggered when the DOM content is updated
	*/
	protected triggerUpdate(old: VirtualDOM, vdom: VirtualDOM): any {}

	/**
	 * Triggered when after calling destroy()
	*/
	protected triggerUnload(): any {}

	/**
	 * When rendering occurs, call to return the `vdom` object that needs to be rendered
	*/
	protected render(): RenderResult {}

	/**
	 * @override
	*/
	appendTo(parent: View): View {
		return this.dom.appendTo(parent);
	}

	/**
	 * Append the rendered DOM to a parent view.
	 * @override
	*/
	afterTo(prev: View): View {
		return this.dom.afterTo(prev);
	}

	/**
	 * Insert the rendered DOM after a previous view.
	 * Note: Do not proactively call this method
	 * @override
	*/
	destroy() {
		if (!this.isDestroyd) {
			WatchingAllCtrForDebug.delete(this);
			(this as any).isDestroyd = true;
			if (this.isLoaded)
				this.triggerUnload(); // trigger unload event
			unref(this, this.owner);
			this.dom.destroy(this);
		}
	}

	static hashCode() {
		return Function.prototype.hashCode.call(this);
	}

	static readonly isViewController: boolean = true;
}

// --- Additional helpers, JSX, element creation, etc. ---

Object.assign(ViewController.prototype, {
	dom: { // init default dom
		ref: '',
		get metaView(): View { throw Error.new('Not implemented dom not initialized') },
		destroy() { },
		appendTo(){ throw Error.new('Not implemented dom not initialized') },
		afterTo(){ throw Error.new('Not implemented dom not initialized') },
	} as DOM,
	_watchings: new InvalidSet,
	_linkProps: [],
	ref: '', _vdom: undefined,
	isLoaded: false, isMounted: false, isDestroyd: false,
});

export default ViewController;

/*
 * Create virtual dom dynamic.
 * Convert a value to VirtualDOM
 * @param value:any, VirtualDOM, or array
 */
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
	matrix: view.Matrix, sprite: view.Sprite, spine: view.Spine,
	text: view.Text, button: view.Button, label: view.Label,
	input: view.Input, textarea: view.Textarea, scroll: view.Scroll,
	video: view.Video,
};

/**
 * Creates a virtual DOM element from JSX syntax
 * @return {VirtualDOM} A VirtualDOM instance
 */
export function createElement<T extends DOM = DOM>(
	Type: DOMConstructor<T> | JSX.IntrinsicElementsName,
	props: Dict | null, ...children: any[]): VirtualDOM
{
	if (typeof Type == 'string') {
		Type = DOMConstructors[Type] as DOMConstructor<T>;
	}
	return new VirtualDOM(Type, props, children.map(_CVDD));
}

export type VDom<T extends DOM = DOM> = VirtualDOM<T>; //!<
export const VDom = VirtualDOM;

export const _CVD = createElement;
export const Jsx = createElement;
export type RenderData = VirtualDOM | string; //!<
export type RenderNode = RenderData | null | undefined | void; //!<
export type RenderResult = RenderNode[] | RenderNode; //!<