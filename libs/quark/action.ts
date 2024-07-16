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

export type KeyframeIn = StyleSheet | CSSNameExp;

export type ActionIn = Action | {
	playing?: boolean;
	loop?: number;
	speed?: number;
	spawn?: ActionIn[];
	seq?: ActionIn[];
	keyframe?: (KeyframeIn)[];
} | (KeyframeIn)[];

export declare class Keyframe extends StyleSheet {
	readonly index: number;
	readonly time: number;
	readonly curve: types.Curve;
	readonly itemsCount: number; // StyleSheet items count
	apply(view: View): void; // apply style to view
	fetch(view: View): void; // fetch style from view
}

export declare abstract class Action {
	readonly duration: number;
	playing: boolean;
	loop: number;
	speed: number;
	play(): this;
	stop(): this;
	seek(time: number): this;
	seekPlay(time: number): this;
	seekStop(time: number): this;
	before(action: Action): void;
	after(action: Action): void;
	remove(): void;
	append(child: Action): void;
	clear(): void;
	constructor(win: Window);
}
export declare class SpawnAction extends Action {}
export declare class SequenceAction extends Action {}

export declare class KeyframeAction extends Action {
	readonly time: number; // get play time
	readonly frame: number; // get play frame
	readonly length: number; // get frames count
	readonly [index: number]: Keyframe; // get keyframe for index
	addFrame(time: number, curve?: types.CurveIn): Keyframe; // add new keyframe
	addFrameWithCss(cssExp: CSSNameExp, time?: number, curve?: types.CurveIn): Keyframe;
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
	* @method create(win,arg[,parent])
	* @param win {Window}
	* @param arg {Object|Action}
	* @param parent Optional {Action}
	* @return {Action}
	*/
export function createAction(win: Window, arg: ActionIn, parent?: Action) {
	let action: Action;
	if ( arg instanceof Action ) {
		action = arg;
	} else {
		// create
		if ( Array.isArray(arg) ) { // KeyframeAction
			action = new KeyframeAction(win);
			for (let sheet of arg)
				(action as KeyframeAction).add(sheet);
		} else if (arg) {
			if (arg.seq) { // SequenceAction
				let seq = arg.seq;
				util.assert(Array.isArray(seq));
				action = Object.assign(new SequenceAction(win), arg);
				for (let i of seq) {
					createAction(win, i, action as SequenceAction);
				}
			} else if (arg.spawn) { // SpawnAction
				let spawn = arg.spawn;
				util.assert(Array.isArray(spawn));
				action = Object.assign(new SpawnAction(win), arg);
				for (let i of spawn) {
					createAction(win, i, action as SequenceAction);
				}
			} else { // KeyframeAction
				action = Object.assign(new KeyframeAction(win), arg);
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

export type ActionCb = (e: ActionEvent)=>void;

/**
	* @method transition(view,style,delay?,cb?)
	* @param view   {View}
	* @param style  {Object}
	* @param [cb]   {Function}
	* @return {KeyframeAction}
	*/
export function transition(
	view: View,
	to: KeyframeIn,
	fromOrCb?: KeyframeIn | ActionCb,
	cb?: ActionCb
): KeyframeAction {
	let action = new KeyframeAction(view.window);

	if (fromOrCb) {
		if (typeof fromOrCb == 'function') {
			cb = fromOrCb;
			action.addFrame(0).fetch(view); // add frame 0 and fetch frame style
		} else {
			action.add(fromOrCb);
		}
	} else {
		action.addFrame(0).fetch(view); // add frame 0
	}
	action.add(to); // add frame 1

	view.action = action.play(); // start play

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