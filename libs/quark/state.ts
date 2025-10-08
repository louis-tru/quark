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

import { ViewController } from './ctr';
import * as storage from './storage';
import util from './util';

// Store persistent states for PersistentState components
const PersistentStates = new Map<string, any>();

// Map of GlobalState controllers -> set of global state keys they care about
const GlobalState_ctrs = new Map<GlobalState, Set<string>>();

// Global states storage (in-memory key-value dictionary)
const GlobalStates: Dict = {};

/*
 * Set a state value into GlobalStates, and persist it if it's a global key.
 * Keys starting with '$' are considered global,
 * Keys starting with '$$' are both global and persistent.
 */
function setStateValue(name: string, value: any) {
	if (name[1] == '$') {
		// Persist global state into storage
		storage.set('_global_state_' + name, value);
	}
	GlobalStates[name] = value;
}

/**
 * Get a global state value by name.
 * If not found in memory, try to read from storage if it's global.
 */
export function getState<T = any>(name: string) {
	let r = GlobalStates[name];
	if (r === undefined) {
		if (name[1] == '$') {
			r = storage.get('_global_state_' + name);
		}
	}
	return r as T | undefined;
}

/**
 * Update state values (both local and global).
 *
 * Rules for keys:
 * - Normal keys (no `$` prefix): updated only on `self` (local state).
 * - Keys starting with `$` (e.g. `$count`):
 *   - Treated as **global shared state**.
 *   - The value is stored in `GlobalStates`.
 *   - All GlobalState controllers that subscribed to this key
 *     will receive an update.
 * - Keys starting with `$$` (e.g. `$$theme`):
 *   - Treated as **global + persistent state**.
 *   - Same as `$key`, but in addition the value is also written to `storage`
 *     under the key `"_global_state_" + name`.
 *   - On next load, if `GlobalStates` does not contain the key, it will be
 *     recovered from `storage`.
 *
 * Update order:
 * 1. Collect all changed `$` / `$$` keys and update `GlobalStates` (and `storage` if `$$`).
 * 2. Notify all other subscribed GlobalState controllers with the relevant subset of changes.
 * 3. Finally, update `self` (if provided) using `ViewController.prototype.setState`
 *    and trigger the callback (if provided).
 *
 * @param state     Key-value pairs to update.
 * @param self      The GlobalState instance that initiated the update (optional).
 *                  Used to avoid notifying itself twice and to update its local state at the end.
 * @return Promise<void> resolves after all updates and callbacks are done.
 */
export async function setState<S = {}>(state: S, self?: GlobalState) {
	if (!state)
		return Promise.resolve();

	let changed_state = [];

	// Collect all changed global states
	for (let name of Object.keys(state)) {
		if (name[0] == '$') {
			changed_state.push(name);
			setStateValue(name, state[name as (keyof S)]);
		}
	}

	// Notify all subscribed controllers about changed global states
	if (changed_state.length) {
		for (let [ctr, keys] of GlobalState_ctrs) {
			if (ctr !== self) {
				// Remove destroyed controllers
				if (ctr.isDestroyd) {
					GlobalState_ctrs.delete(ctr);
					continue;
				}
				let newState: Dict = {}, need_update = false;

				// Collect only the keys that this controller cares about
				for (let name of changed_state) {
					if (keys.has(name)) {
						need_update = true;
						newState[name] = GlobalStates[name];
					}
				}
				// Update controller if relevant state changed
				if (need_update) {
					ViewController.prototype.setState.call(ctr, newState); // call parent
				}
			}
		}
	}

	// Update the calling controller itself
	if (self) {
		await ViewController.prototype.setState.call(self, state); // call parent
	}
}

/**
 * @class GlobalState
 * A special ViewController that can participate in global state sharing.
 * Any state key starting with `$` will be synchronized across all GlobalState instances.
 * Keys starting with `$$` are persisted in storage.
 */
export class GlobalState<P = {}, S = {}> extends ViewController<P, S> {
	private _isGlobal?: boolean;

	/**
	 * Override setState to use global state manager
	 */
	setState<K extends keyof S>(newState: Pick<S, K>) {
		return setState(newState, this);
	}

	/**
	 * Triggered when the controller is loaded.
	 * Synchronizes its global state keys with the GlobalStates.
	 */
	protected triggerLoad() {
		if (!this.state)
			return;
		let state = this.state;
		let keys = new Set<string>();
		let need_update = false;

		for (let name of Object.keys(state)) {
			if (name[0] == '$') {
				// Persistent global state (`$$key`)
				if (name[1] == '$') {
					if (!GlobalStates.hasOwnProperty(name)) {
						let val = storage.get('_global_state_' + name);
						if (val === undefined) {
							// Initialize persistent state from default if not stored yet
							val = state[name as keyof S];
							storage.set('_global_state_' + name, val);
						}
						GlobalStates[name] = val;
					}
				}
				// Sync with existing GlobalStates
				if (name in GlobalStates) {
					let value = GlobalStates[name];
					if (value !== state[name as keyof S]) {
						need_update = true;
						(state as any)[name] = value;
					}
				} else {
					setStateValue(name, state[name as keyof S]);
				}
				keys.add(name);
			}
		}

		// Update state if necessary
		if (need_update) {
			ViewController.prototype.setState.call(self, state);
		}

		// Register this controller as global subscriber
		if (keys.size) {
			this._isGlobal = true;
			GlobalState_ctrs.set(this, keys);
		}
	}

	/**
	 * Triggered when the controller is unloaded.
	 * Remove it from global subscribers if it was global.
	 */
	protected triggerUnload() {
		if (this._isGlobal)
			GlobalState_ctrs.delete(this as GlobalState);
	}
}

/**
 * @class PersistentState
 * Extends GlobalState with persistence across component lifecycles.
 * Each PersistentState instance gets a unique persistent ID.
 */
export class PersistentState <P = {}, S = {}> extends GlobalState<P, S> {

	/**
	 * Unique ID for each PersistentState class
	 */
	get persistentID() {
		var id = (this.constructor as any).__persistentID__ as string;
		if (!id) {
			(this.constructor as any).__persistentID__ = id = String(util.getId());
		}
		return id;
	}

	/**
	 * Subclass can override this to define what to save persistently
	 */
	protected saveState(): Partial<S> | null {
		return null;
	}

	/**
	 * Restore state from PersistentStates map
	 */
	protected recoveryState(): Partial<S> | null {
		return PersistentStates.get(this.persistentID);
	}

	/**
	 * When loading, recover state first, then proceed with global sync
	 */
	protected triggerLoad() {
		var state = this.recoveryState();
		if (state) {
			(this as any).state = Object.assign(this.state || {}, state);
		}
		super.triggerLoad();
	}

	/**
	 * Called when the PersistentState controller is unloaded.
	 * - First calls the parent's triggerUnload (to clean up global subscriptions).
	 * - Then tries to save the component's state via `saveState()`.
	 *   - If a partial state is returned, store it in PersistentStates (in-memory cache).
	 *   - Otherwise, remove any previously stored state for this component.
	 */
	protected triggerUnload() {
		super.triggerUnload();

		// Get state to save (subclasses may override saveState)
		var state = this.saveState();

		if (state) {
			// Store the saved state by persistentID
			PersistentStates.set(this.persistentID, state);
		} else {
			// Remove saved state if nothing to persist
			PersistentStates.delete(this.persistentID);
		}
	}
}