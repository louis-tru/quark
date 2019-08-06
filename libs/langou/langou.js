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

export requireNative('_langou');

import 'langou/util';
import 'langou/event';
import 'langou/app';
import 'langou/action';
import ViewController from 'langou/ctr';
import 'langou/display_port';
import 'langou/css';

/**
	* @class View
	*/
class View extends event.NativeNotification {

	// @private:
	m_owner = null;
	
	// @public:
	// @events
	event onKeyDown;
	event onKeyPress;
	event onKeyUp;
	event onKeyEnter;
	event onBack;
	event onClick;
	event onTouchStart;
	event onTouchMove;
	event onTouchEnd;
	event onTouchCancel;
	event onMouseOver;
	event onMouseOut;
	event onMouseLeave;
	event onMouseEnter;
	event onMouseMove;
	event onMouseDown;
	event onMouseUp;
	event onMouseWheel;
	event onFocus;
	event onBlur;
	event onHighlighted;
	event onActionKeyframe;
	event onActionLoop;
	event onRemoveView;
	
	/**
	 * @overwrite
	 */
	addDefaultListener(name, func) {
		if ( typeof func == 'string' ) { // find func 
			var owner = this;
			do {
				func2 = owner[func];
				if ( typeof func2 == 'function' ) {
					return this.getNoticer(name).on(func2, owner, 0); // default id 0
				}
				owner = owner.m_owner;
			} while(owner);
			throw util.err(`Cannot find a function named "${func}"`);
		} else {
			return this.getNoticer(name).on(func, 0); // default id 0
		}
	}
	
	/**
	 * @get action {Action}
	 */
	get action() { // get action object
		return this.getAction(); 
	}

	/**
	 * @set action {Action}
	 */
	set action(value) { // set action
		this.setAction(action.create(value));
	}
	
	/**
	 * @func transition(style[,delay[,cb]][,cb])
	 * @arg style {Object}
	 * @arg [delay] {uint} ms
	 * @arg [cb] {Funcion}
	 * @ret {KeyframeAction}
	 */
	transition(style, delay, cb) { // transition animate
		return action.transition(this, style, delay, cb);
	}
	
	/**
	 * @func show()
	 */
	show() {
		this.visible = true;
	}

	/**
	 * @func hide()
	 */
	hide() {
		this.visible = false; 
	}
}

 /**
	* @class Panel
	*/
class Panel {
	event onFocusMove;
}

 /**
	* @class Scroll
	*/
class Scroll {
	event onScroll;
}

/**
	* @class Button
	* @bases Hybrid
	*/
export class Button extends exports.Button {
	
	m_defaultHighlighted = true;
	
	/**
	 * @overwrite
	 */
	getNoticer(name) { 
		var noticer = this['__on' + name];
		if ( ! noticer ) {
			if ( name == 'Click' ) {
				super.getNoticer('Highlighted'); // bind highlighted
			}
			return super.getNoticer(name);
		}
		return noticer;
	}
	
	/**
	 * @get defaultHighlighted {bool}
	 */
	get defaultHighlighted() {
		return this.m_defaultHighlighted; 
	}
	
	/**
	 * @set defaultHighlighted {bool}
	 */
	set defaultHighlighted(value) {
		this.m_defaultHighlighted = !!value; 
	}
	
	/**
	 * @func setHighlighted(status)
	 * @arg status {HighlightedStatus}
	 */
	setHighlighted(status) {
		if ( this.m_defaultHighlighted ) {
			if ( status == event.HIGHLIGHTED_HOVER ) {
				this.transition({ opacity: 0.7, time: 80 });
			} else if ( status == event.HIGHLIGHTED_DOWN ) {
				this.transition({ opacity: 0.35, time: 50 });
			} else {
				this.transition({ opacity: 1, time: 180 });
			}
		}
	}
	
	/**
	 * @overwrite
	 */
	triggerHighlighted(evt) {
		this.setHighlighted(evt.status);
		return this.triggerWithEvent('Highlighted', evt);
	}
}

util.extendClass(exports.View, View);
util.extendClass(exports.Panel, Panel);
util.extendClass(exports.Scroll, Scroll);

/**
 * @class Clip
 * @extends Div
 */
export class Clip extends exports.Div {
	constructor() {
		super();
		this.clip = true;
	}
}

export {

	/**
	 * @class GUIApplication
	 */
	GUIApplication: app.GUIApplication,

	/**
	 * @class ViewController
	 */
	ViewController: ViewController,

	/**
	 * @func nextFrame(cb)
	 * @arg cb {Function}
	 */
	nextFrame: display_port.nextFrame,

	/**
	 * @func render(vdom[,parent])
	 * @arg vdom {Object}
	 * @arg [parent] {View}
	 * @ret {DOM}
	 */
	render: ViewController.render,

	/**
	 * @func CSS(sheets)
	 * @arg sheets {Object}
	 */
	CSS: css.CSS,

	/**
	 * @get app {GUIApplication} get current application object
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
	 * @get displayPort {DisplayPort} get current display port
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

};
