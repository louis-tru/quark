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

export requireNative('_action');

import 'ngui/util';
import 'ngui/value';

const { Action, SpawnAction, SequenceAction, KeyframeAction } = exports;

 /**
	* @func create(json[,parent])
	* @arg json {Object|Action}
	* @arg [parent] {GroupAction}
	* @ret {Action}  
	*/
export function create(json, parent) {
	var action = null;
	if ( typeof json == 'object' ) {
		if ( json instanceof Action ) {
			action = json;
		} else {
			// create
			if ( Array.isArray(json) ) { // KeyframeAction
				action = new KeyframeAction();
				for (var i of json)
					action.add(i);
			} else {
				if (json.seq) { // SequenceAction
					action = new SequenceAction();
					for (var i in json) 
						action[i] = json[i];
					var seq = json.seq;
					if (Array.isArray(seq)) {
						for (var i of seq) {
							create(i, action);
						}
					} else {
						create(seq, action);
					}
				} else if (json.spawn) { // SpawnAction
					action = new SpawnAction();
					for (var i in json) 
						action[i] = json[i];
					var spawn = json.spawn;
					if (Array.isArray(spawn)) {
						for (var i of spawn) {
							create(i, action);
						}
					} else {
						create(spawn, action);
					}
				} else { // KeyframeAction
					action = new KeyframeAction();
					for (var i in json) 
						action[i] = json[i];
					var frame = json.keyframe;
					if ( Array.isArray(frame) ) {
						for (var i of frame) 
							action.add(i);
					} else {
						action.add(frame);
					}
				}
			}
			// end craete
		}
		if ( parent ) { // Cannot be KeyframeAction type
			parent.append(action);
		}
	}
	return action;
}

 /**
	* @func transition(view,style[,delay[,cb]][,cb])
	* @arg view 	{View}
	* @arg style  {Object}
	* @arg [delay]  {uint} ms
	* @arg [cb]     {Function}
	* @ret {KeyframeAction}
	*/
export function transition(view, style, delay, cb) {
	var action = new KeyframeAction();
	if ( typeof delay == 'number' ) {
		action.delay = delay;
	} else if ( typeof delay == 'function' ) {
		cb = delay;
	}
	action.add(); // add frame 0
	action.add(style); // add frame 1
	view.setAction(action);
	action.frame(0).fetch(); // fetch 0 frame style

	if ( typeof cb == 'function' ) {
		view.onActionKeyframe.on(function(evt) {
			//console.log('onActionKeyframe');
			if ( evt.action === action ) {
				if (evt.frame != 1) return;
				cb(evt); // end
			}
			view.onActionKeyframe.off(-1);
		}, -1);
	}

	action.play(); // start play
	return action;
}
