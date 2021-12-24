/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
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
import { Propery } from './css';
import * as value from './value';
import { View } from './_view';
import { Action, ActionIn, KeyframeOptions } from './_action';
import { ActionEvent } from './event';

const _action = __require__('_action');

Object.assign(exports, _action);

export * from './_action';

export declare abstract class GroupAction extends Action {
	readonly length: number;
	append(child: Action): void;
	insert(index: number, child: Action): void;
	removeChild(index: number): void;
	children(index: number): Action | null;
}

export declare class SpawnAction extends GroupAction {}
export declare class SequenceAction extends GroupAction {}

export declare class KeyframeAction extends Action {
	hasProperty(name: Propery): boolean;
	matchProperty(name: Propery): boolean;
	frame(index: number): Frame | null;
	add(style: KeyframeOptions): Frame;
	add(time?: number, curve?: value.CurveIn): Frame;
	readonly first: Frame | null;
	readonly last: Frame | null;
	readonly length: number;
	readonly position: number;
	readonly time: number;
}

// fetch style attribute by view
export declare abstract class Frame {
	fetch(view?: View): void; // fetch style attribute by view
	flush(): void; // flush frame restore default values
	readonly index: number;
	readonly host: KeyframeAction | null;
	time: number;
	curve: value.Curve;
	// Meta attribute
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	skewX: number;
	skewY: number;
	rotateZ: number;
	originX: number;
	originY: number;
	opacity: number;
	visible: boolean;
	width: value.Value;
	height: value.Value;
	marginTop: value.Value;
	marginRight: value.Value;
	marginBottom: value.Value;
	marginLeft: value.Value;
	borderTopWidth: number;
	borderRightWidth: number;
	borderBottomWidth: number;
	borderLeftWidth: number;
	borderTopColor: value.Color;
	borderRIghtColor: value.Color;
	borderBottomColor: value.Color;
	borderLeftColor: value.Color;
	borderRadiusLeftTop: number;
	borderRadiusRightTop: number;
	borderRadiusRightBottom: number;
	borderRadiusLeftBottom: number;
	backgroundColor: value.Color;
	background: value.Background;
	newline: boolean;
	clip: boolean;
	contentAlign: value.ContentAlign;
	textAlign: value.TextAlign;
	maxWidth: value.Value;
	maxHeight: value.Value;
	startX: number;
	startY: number;
	ratioX: number;
	ratioY: number;
	repeat: value.Repeat;
	textBackgroundColor: value.TextColor;
	textColor: value.TextColor;
	textSize: value.TextSize;
	textStyle: value.TextStyle;
	textFamily: value.TextFamily;
	textLineHeight: value.TextLineHeight;
	textShadow: value.TextShadow;
	textDecoration: value.TextDecoration;
	textOverflow: value.TextOverflow;
	textWhiteSpace: value.TextWhiteSpace;
	alignX: value.Align;
	alignY: value.Align;
	shadow: value.Shadow;
	src: string;
	// Non meta attribute
	translate: value.Vec2;
	scale: value.Vec2;
	skew: value.Vec2;
	origin: value.Vec2;
	margin: value.Value[];
	border: value.Border;
	borderWidth: number;
	borderColor: value.Color;
	borderRadius: number;
	borderLeft: value.Border;
	borderTop: value.Border;
	borderRight: value.Border;
	borderBottom: value.Border;
	minWidth: value.Value;
	minHeight: value.Value;
	start: value.Vec2;
	ratio: value.Vec2;
	align: value.Align[];
}

 /**
	* @func create(json[,parent])
	* @arg json {Object|Action}
	* @arg [parent] {GroupAction}
	* @ret {Action}
	*/
export function create(In: ActionIn, parent?: GroupAction) {
	if ( typeof In != 'object' ) {
		throw new Error('Bad argument.');
	}
	var action: Action;
	if ( In instanceof Action ) {
		action = In;
	} else {
		// create
		if ( Array.isArray(In) ) { // KeyframeAction
			action = new _action.KeyframeAction();
			for (var sheet of In)
				(action as KeyframeAction).add(sheet);
		} else {
			if (In.seq) { // SequenceAction
				var seq = In.seq;
				utils.assert(Array.isArray(seq));
				action = Object.assign(new _action.SequenceAction(), In);
				for (let i of seq) {
					create(i, action as SequenceAction);
				}
			} else if (In.spawn) { // SpawnAction
				var spawn = In.spawn;
				utils.assert(Array.isArray(spawn));
				action = Object.assign(new _action.SpawnAction(), In);
				for (let i of spawn) {
					create(i, action as SequenceAction);
				}
			} else { // KeyframeAction
				action = Object.assign(new _action.KeyframeAction(), In);
				var keyframe = In.keyframe;
				if ( Array.isArray(keyframe) ) {
					for (let i of keyframe) 
						(action as KeyframeAction).add(i);
				}
			}
		}
		// end craete
	}
	if ( parent ) { // Cannot be KeyframeAction type
		parent.append(action);
	}
	return action;
}

 /**
	* @func transition(view,style,delay?,cb?)
	* @arg view 	{View}
	* @arg style  {Object}
	* @arg [delay]  {uint} ms
	* @arg [cb]     {Function}
	* @ret {KeyframeAction}
	*/
export declare function transition(view: View, style: KeyframeOptions, cb?: (e: ActionEvent)=>void): KeyframeAction;
export declare function transition(view: View, style: KeyframeOptions, delay?: number, cb?: (e: ActionEvent)=>void): KeyframeAction;

exports.transition = function(view: View, style: KeyframeOptions, delay?: number, cb?: (e: ActionEvent)=>void) {
	var action = new _action.KeyframeAction() as KeyframeAction;
	if ( typeof delay == 'function' ) {
		cb = delay;
	} else {
		if ( typeof delay == 'number' )
			action.delay = delay;
		if ( arguments.length > 2 && typeof cb != 'function' )
			cb = undefined;
	}
	action.add(); // add frame 0
	action.add(style); // add frame 1
	view.setAction(action);
	(action.frame(0) as Frame).fetch(); // fetch frame style

	if ( cb ) {
		view.onActionKeyframe.on(function(evt) {
			//console.log('onActionKeyframe');
			if ( evt.action === action ) {
				if (evt.frame != 1) return;
				(cb as any)(evt); // end
			}
			view.onActionKeyframe.off('transition-1');
		}, 'transition-1');
	}

	action.play(); // start play

	return action;
}