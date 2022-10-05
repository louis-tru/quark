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
import event, {
	NativeNotification, EventNoticer, 
	HighlightedStatus,
	UIEvent, HighlightedEvent,
	KeyEvent, ClickEvent,
	TouchEvent, MouseEvent, ActionEvent,
} from './event';
import {View} from './_view';
import {ViewController,_CVD} from './ctr';
import { ActionIn, KeyframeOptions, Action } from './_action';
import * as action from './action';
import app from  './app';
import display_port from './display';
import css from './css';
export * from './_view';
export {ViewController, VirtualDOM,_CVD} from './ctr';
export {Application} from './app';

const _quark = __require__('_quark');

class _View extends NativeNotification {

	private m_id: string;
	private m_owner: ViewController | null;

	get __meta__() { return this }

	@event readonly onKeyDown: EventNoticer<KeyEvent>;
	@event readonly onKeyPress: EventNoticer<KeyEvent>;
	@event readonly onKeyUp: EventNoticer<KeyEvent>;
	@event readonly onKeyEnter: EventNoticer<KeyEvent>;
	@event readonly onBack: EventNoticer<ClickEvent>;
	@event readonly onClick: EventNoticer<ClickEvent>;
	@event readonly onTouchStart: EventNoticer<TouchEvent>;
	@event readonly onTouchMove: EventNoticer<TouchEvent>;
	@event readonly onTouchEnd: EventNoticer<TouchEvent>;
	@event readonly onTouchCancel: EventNoticer<TouchEvent>;
	@event readonly onMouseOver: EventNoticer<MouseEvent>;
	@event readonly onMouseOut: EventNoticer<MouseEvent>;
	@event readonly onMouseLeave: EventNoticer<MouseEvent>;
	@event readonly onMouseEnter: EventNoticer<MouseEvent>;
	@event readonly onMouseMove: EventNoticer<MouseEvent>;
	@event readonly onMouseDown: EventNoticer<MouseEvent>;
	@event readonly onMouseUp: EventNoticer<MouseEvent>;
	@event readonly onMouseWheel: EventNoticer<MouseEvent>;
	@event readonly onFocus: EventNoticer<UIEvent>;
	@event readonly onBlur: EventNoticer<UIEvent>;
	@event readonly onHighlighted: EventNoticer<HighlightedEvent>;
	@event readonly onActionKeyframe: EventNoticer<ActionEvent>;
	@event readonly onActionLoop: EventNoticer<ActionEvent>;

	get id() {
		return this.m_id;
	}

	set id(value) {
		ViewController._setID(this as unknown as View, value);
	}

	get owner() {
		return this.m_owner;
	}

	ownerAs<T extends ViewController = ViewController>() {
		utils.assert(this.m_owner, 'View.ownerAs<T>() = null');
		return this.m_owner as T;
	}

	actionAs<T extends Action = Action>() {
		var action = (this as unknown as View).getAction() as Action;
		utils.assert(action, 'View.actionAs<T>() = null');
		return action as T;
	}

	hashCode() {
		return (this as unknown as View).viewType + 18766898;
	}

	appendTo(parentView: View) {
		(parentView as View).append(this as unknown as View);
		return this;
	}

	afterTo(prevView: View) {
		(prevView as View).after(this as unknown as View);
		return this;
	}

	setAction(In: ActionIn | null) {
		(this as any)._setAction(In && action.create(In));
	}

	get action() { // get action object
		return (this as unknown as View).getAction(); 
	}

	set action(value) { // set action
		(this as unknown as View).setAction(value);
	}

	get style(): StyleSheet {
		return this as unknown as StyleSheet;
	}

	set style(value: StyleSheet) {
		Object.assign(this, value)
	}

	transition(style: KeyframeOptions, delay?: number, cb?: (e: ActionEvent)=>void) { // transition animate
		return action.transition(this as unknown as View, style, delay, cb);
	}

	show() {
		(this as unknown as View).visible = true;
	}

	hide() {
		(this as unknown as View).visible = false; 
	}

	static get isViewController() { return false }

}

class _Panel {
	@event readonly onFocusMove: EventNoticer<UIEvent>;
}

class _Scroll {
	@event readonly onScroll: EventNoticer<UIEvent>;
}

class _Image {
	@event readonly onLoad: EventNoticer<UIEvent>;
	@event readonly onError: EventNoticer<UIEvent>;
}

class _Input {
	@event readonly onChange: EventNoticer<UIEvent>;
}

class _Textarea {
	@event readonly onScroll: EventNoticer<UIEvent>;
}

class _Button {

	private m_defaultStyle: boolean; // = true;

	getNoticer(name: string) {
		var noticer = (this as any)['m_on' + name] as EventNoticer<UIEvent>;
		if ( ! noticer ) {
			if ( name == 'Click' ) {
				View.prototype.getNoticer.call(this as unknown as View, 'Highlighted'); // bind highlighted
			}
			return View.prototype.getNoticer.call(this as unknown as View, name);
		}
		return noticer;
	}

	get defaultStyle() {
		return this.m_defaultStyle; 
	}

	set defaultStyle(value) {
		this.m_defaultStyle = !!value; 
	}

	setHighlighted(status: HighlightedStatus) {
		if ( this.m_defaultStyle ) {
			if ( status == HighlightedStatus.HIGHLIGHTED_HOVER ) {
				(this as unknown as View).transition({ opacity: 0.7, time: 80 });
			} else if ( status == HighlightedStatus.HIGHLIGHTED_DOWN ) {
				(this as unknown as View).transition({ opacity: 0.35, time: 50 });
			} else {
				(this as unknown as View).transition({ opacity: 1, time: 180 });
			}
		}
	}

	triggerHighlighted(evt: HighlightedEvent) {
		this.setHighlighted(evt.status);
		return (this as unknown as View).triggerWithEvent('Highlighted', evt);
	}
}

utils.extendClass(_quark.View, _View);
utils.extendClass(_quark.Panel, _Panel);
utils.extendClass(_quark.Scroll, _Scroll);
utils.extendClass(_quark.Image, _Image);
utils.extendClass(_quark.Input, _Input);
utils.extendClass(_quark.Textarea, _Textarea);
utils.extendClass(_quark.Button, _Button);

_quark.View.prototype.m_id = '';
_quark.View.prototype.m_owner = null;
_quark.Button.prototype.m_defaultStyle = true;

export default {

	/**
	 * @func nextFrame(cb)
	 * @arg cb {Function}
	 */
	nextFrame: display_port.nextFrame,

	/**
	 * @func render(vdom[,parent])
	 * @arg vdom {VDOM}
	 * @arg [parent] {View}
	 * @ret {DOM}
	 */
	render: ViewController.render,

	/**
	 * @func css(sheets)
	 * @arg sheets {Object}
	 */
	css: css,

	/**
	 * @func CVD() create virtual dom
	 */
	CVD: _CVD,

	/**
	 * @get app {Application} get current application object
	 */
	get app() { return app.current },

	/**
	 * @get root {Root} get current root view
	 */
	get root() { return app.root },

	/**
	 * @get rootCtr {ViewController} get current root view controller
	 */
	get rootCtr() { return app.rootCtr },

	/**
	 * @get displayPort {Display} get current display port
	 */
	get displayPort() { return app.current.displayPort },

	/**
	 * @get atomPixel {float}
	 */
	get atomPixel() { return display_port.atomPixel },

	/**
	 * @get statusBarHeight {float}
	 */
	get statusBarHeight() { return display_port.statusBarHeight },

	/**
	 * @func lock()
	 */
	lock: _quark.lock as <R>(exec: ()=>R)=>R,

};