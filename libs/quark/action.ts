/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
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
import { View } from './view';
import { ActionEvent } from './event';
import { Window } from './window';
import { StyleSheet,CSSNameExp } from './css';
import * as types from './types';

Object.assign(exports, __binding__('_action'));

/**
 * @type KeyframeIn = {StyleSheet} | {CSSNameExp}
 */ 
export type KeyframeIn = StyleSheet | CSSNameExp;

/**
 * @type ActionIn = {Action} | {KeyframeIn[]} | {...}
*/
export type ActionIn = Action | KeyframeIn[] | {
	playing?: boolean; ///
	loop?: number; ///
	speed?: number; ///
	spawn?: ActionIn[]; ///
	seq?: ActionIn[]; ///
	keyframe?: KeyframeIn[]; ///
};

/**
 * @class Keyframe
 * @extends StyleSheet
*/
export declare class Keyframe extends StyleSheet {
	/** Get Keyframe index in the keyframe action */
	readonly index: number;
	/** Get Keyframe time poing */
	readonly time: number;
	/** Get transition curve */
	readonly curve: types.Curve;
	/** Get StyleSheet items count */
	readonly itemsCount: number;
	/** apply style to view */
	apply(view: View): void;
	/** fetch style from view */
	fetch(view: View): void;
}

/**
 * @class Action
 * @abstract
 * `abstract class`
 * 
 * Action base type, this is an abstract type without a constructor
*/
export declare abstract class Action {
	/** Get action playtime long */
	readonly duration: number;
	/** Set/Get action playing state, if set then play() or stop() is called */
	playing: boolean;
	/** Set/Get action loop, the zero means not loop */
	loop: number;
	/** Set/Get action speed */
	speed: number;
	/** To play action */
	play(): this;
	/** To stop action */
	stop(): this;
	/**
	* Jump to target `time`, after calling it, it will reset the internal `looped`
	* @param `time` {uint} unit as `ms`
	* @return `this`
	*/
	seek(time: number): this;
	/**
	* Jump to the target `time` and start playing. The internal `looped` will be reset after calling
	* @param time {uint} unit as `ms`
	*/
	seekPlay(time: number): this;
	/**
	* Jump to the target `time` and stop playing. The internal `looped` will be reset after calling
	* @param time {uint} unit as `ms`
	* @return {this}
	*/
	seekStop(time: number): this;
	/**
	 * Add a sibling action to the front of itself.
	 * The current action needs to have a parent action else it will be invalid.
	 * @param action {Action}
	*/
	before(action: Action): void;
	/**
	 * Add a sibling action to the back of itself.
	 * The current action needs to have a parent action, otherwise it will be invalid.
	 * @param action {Action}
	*/
	after(action: Action): void;
	/**
	 * Deleting myself from the parent
	*/
	remove(): void;
	/**
	 * Add sub-actions to the end. Note: {KeyframeAction} cannot add sub-actions
	 * @param child {Action}
	 */
	append(child: Action): void;
	/**
	 * Clear all sub-actions or keyframes. After clearing, the action will stop immediately.
	 */
	clear(): void;
	/**
	 * @constructor
	 * @pram win {Window}
	*/
	constructor(win: Window);
}

/**
 * @class SpawnAction
 * @extends Action
*/
export declare class SpawnAction extends Action {}

/**
 * @class SequenceAction
 * @extends Action
*/
export declare class SequenceAction extends Action {}

/**
 * @class KeyframeAction
 * @extends Action
*/
export declare class KeyframeAction extends Action {
	/** get play time  */
	readonly time: number;
	/** get playing frame index */
	readonly frame: number;
	/** get count of frames */
	readonly length: number;
	/**
	 * @indexed
	 * Get the {Keyframe} for index
	*/
	readonly [index: number]: Keyframe;

	/**
	* @method addFrame(time[,curve])
	*
	* By `time` and `curve` add keyframe，and return it
	*
	* @param time {uint} Keyframe time point
	* @param curve? {CurveIn} No `curve` use `'ease'` as default
	*
	* Can use `'linear'`、`'ease'`、`'easeIn'`、`'easeOut'`、`'easeInOut'` as params
	*
	* @return {Keyframe}
	*/
	addFrame(time: number, curve?: types.CurveIn): Keyframe;

	/**
	 * @method addFrameWithCss
	 * 
	 * Add keyframes by the `cssExp` stylesheet name expression and return keyframes
	 * 
	 * @param cssExp {CSSNameExp} css exp
	 * @param time? {uint} Keyframe time point, **Default** as zero
	 * @param curve? {CurveIn} If not passed, `curve` **Default** as `'ease'`
	 * 
	 * @return {Keyframe}
	*/
	addFrameWithCss(cssExp: CSSNameExp, time?: number, curve?: types.CurveIn): Keyframe;

	/**
	 * @method add
	 * 
	 * Add keyframes through `cssExp` stylesheet name expression or target property table,
	 * and return keyframes
	 * 
	 * @param styleOrCssExp {KeyframeIn} style sheet or css exp
	 * @param time? {uint} Keyframe time point, **Default** as zero
	 * @param curve? {CurveIn} If not passed, `curve` **Default** as `'ease'`
	 * 
	 * @return {Keyframe}
	*/
	add(styleOrCssExp: KeyframeIn, time?: number, curve?: types.CurveIn): Keyframe;
}
(exports.KeyframeAction as typeof KeyframeAction).prototype.add =
function(styleOrCssExp: KeyframeIn, ...args: any[]): Keyframe {
	if (typeof styleOrCssExp == 'string') {
		return this.addFrameWithCss(styleOrCssExp, ...args);
	} else {
		let frame = this.addFrame(args[0] || styleOrCssExp.time || 0, args[1] || styleOrCssExp.curve);
		Object.assign(frame, styleOrCssExp);
		return frame;
	}
};

/**
* @method createAction(win,arg[,parent])
*
* * Create an action through the `json` parameter. If the `arg` passed in is [`Action`],
* 
* 	skip the creation process and return directly.
*
* 	If the parent action is passed in, append the newly created action to the end of `parent` after creation.
*
* * If the passed in parameter is an [`Array`], create a [`KeyframeAction`] and use this [`Array`] to create the corresponding [`Frame`]
*
* * If the passed in parameter has a `seq` attribute, create a [`SequenceAction`]
*
* * If the passed in parameter has a `spawn` attribute, create a [`SpawnAction`]
*
* * If there is no `seq` or `spawn` in the passed in parameter, create a [`KeyframeAction`]
*
* 	If the object's internal attribute `keyframe` is [`Array`], use this [`Array`] to create a [`Frame`]
* 
* @param win {Window}
* @param arg {ActionIn}
* @param parent? {Action}
* @return {Action}
*
* Example:
*
* ```ts
* var act1 = createAction(win, [
* 	{ time:0, x:0 },
* 	{ time:1000, x:100 },
* ]);
* var act1 = createAction(win, {
* 	keyframe: [
* 		{ time:0, x:0, curve: 'linear', },
* 		{ time:1000, x:100 },
* 	]
* });
* // Creating `SequenceAction` and have two `KeyframeAction`
* var act2 = createAction(win, {
* 	loop: 999999,
* 	seq: [
* 		{
* 			keyframe: [
* 				{ time:0, x: 0 },
* 				{ time:1000, x: 100 },
* 			]
* 		},
* 		[
* 			{ time:0, x: 100 },
* 			{ time:1000, x: 0 },
* 		]
* 	]
* })
* ```
*/
export function createAction(win: Window, arg: ActionIn, parent?: Action) {
	let action: Action;
	if ( arg instanceof (exports.Action as typeof Action) ) {
		action = arg;
	} else {
		// create
		if ( Array.isArray(arg) ) { // KeyframeAction
			action = new exports.KeyframeAction(win);
			for (let sheet of arg)
				(action as KeyframeAction).add(sheet);
		} else if (arg) {
			if (arg.seq) { // SequenceAction
				let seq = arg.seq;
				util.assert(Array.isArray(seq));
				action = Object.assign(new exports.SequenceAction(win), arg);
				for (let i of seq) {
					createAction(win, i, action as SequenceAction);
				}
			} else if (arg.spawn) { // SpawnAction
				let spawn = arg.spawn;
				util.assert(Array.isArray(spawn));
				action = Object.assign(new exports.SpawnAction(win), arg);
				for (let i of spawn) {
					createAction(win, i, action as SequenceAction);
				}
			} else { // KeyframeAction
				action = Object.assign(new exports.KeyframeAction(win), arg);
				let key = arg.keyframe;
				if ( Array.isArray(key) ) {
					for (let sheet of key)
						(action as KeyframeAction).add(sheet);
				}
			}
		} else {
			throw new Error('Bad argument.');
		}
		// end craete
	}
	if (parent) { // Cannot be KeyframeAction type
		parent.append(action);
	}
	return action;
}

/**
 * @callback ActionCb(e)
 * @param e {ActionEvent}
*/
export type ActionCb = (e: ActionEvent)=>void;

/**
 * @method transition(view,to[,fromOrCb[,cb]])
 *
 * * Create a view style transition action by the style and play this action,
 * 
 * 	and callback after completion
 *
 * Callback: cb()
 *
 *
 * @param view        {View}
 * @param to          {KeyframeIn}
 * @param fromOrCb?   {KeyframeIn|ActionCb}
 * @param cb?         {ActionCb}
 * @return {KeyframeAction}
 *
 * Example:
 *
 *	```ts
 *	// The transition is completed and callback is made after 1 second
 *	transition(view, {
 *		time: 1000,
 *		y: 100,
 *		x: 100,
 *	}, ()={
 *		console.log('view transition end');
 *	})
 *  // Use a linear transition
 *	transition(view, {
 *		time: 1000,
 *		curve: 'linear',
 *		y: 100,
 *		x: 100,
 *	}, 1000)
 *	```
 */
export function transition(
	view: View,
	to: KeyframeIn,
	fromOrCb?: KeyframeIn | ActionCb,
	cb?: ActionCb
): KeyframeAction {
	let action = new exports.KeyframeAction(view.window) as KeyframeAction;
	let from: Keyframe | undefined;

	if (fromOrCb) {
		if (typeof fromOrCb == 'function') {
			cb = fromOrCb;
			from = action.addFrame(0); // add frame 0
		} else {
			action.add(fromOrCb);
		}
	} else {
		from = action.addFrame(0); // add frame 0
	}

	action.add(to); // add frame 1

	if (from) {
		from.fetch(view); // fetch(view)
	}

	view.action = action

	action.play(); // start play

	if ( cb ) {
		view.onActionKeyframe.on(function(evt) {
			//console.log('onActionKeyframe');
			if ( evt.action === action ) {
				if (evt.frame != 1)
					return;
				cb(evt); // end
			}
			view.onActionKeyframe.off('transition-1');
		}, 'transition-1');
	}

	return action;
}

export default createAction;