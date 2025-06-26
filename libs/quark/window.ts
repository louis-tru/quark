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
import {Root,View} from './view';
import * as types from './types';
import event, { Event, EventNoticer, NativeNotification, Notification } from './event';
import { ViewController, VirtualDOM, _CVD } from './ctr';

const _ui = __binding__('_ui');
type WEvent = Event<Window>;

/**
 * @interface Options
*/
export interface Options {
	/** init window color type */
	olorType?: number;
	/** init window gpu render msaa count */
	msaa?: number; 
	/** init window params */
	frame?: types.Rect;
	/** init window title */
	title?: string; 
	/** init window background color */
	backgroundColor?: types.Color;
	/** Is need draw navigation buttons for android. */
	navigationColor?: types.Color;
};

class RootViewController extends ViewController {
	constructor(window: Window) {
		super({}, { window, children: [], owner: null as any });
		(this as any).dom = window.root;
	}
	get metaView() { return this.window.root }
	setState() { throw Error.new('Access forbidden.') }
	update() { throw Error.new('Access forbidden.') }
	appendTo(): View { throw Error.new('Access forbidden.') }
	afterTo(): View { throw Error.new('Access forbidden.') }
	remove() { throw Error.new('Access forbidden.') }
}

/**
 * @class NativeWindow
 * @extends Notification
*/
declare class NativeWindow extends Notification<WEvent> {

	/**
	 * @attr scale
	 * The scaling ratio of the physical pixel size of the window to the drawing size
	 * 
	 * @type {float}
	 * @get
	*/
	readonly scale: number;

	/**
	 * @attr defaultScale
	 * The default recommended scaling ratio between window physical pixels and drawing size
	 * 
	 * @type {float}
	 * @get
	*/
	readonly defaultScale: number;

	/**
	 * @attr fsp
	 * Current drawing frame rate
	 * @type {uint}
	 * @get
	*/
	readonly fsp: number;

	/**
	 * @attr atomPixel
	 * Atom pixel size, Exp: 1 / scale
	 * 
	 * @type {float}
	 * @get
	*/
	readonly atomPixel: number;

	/**
	 * @attr root
	 * The only root view in the window
	 * 
	 * @type Root
	 * @get
	*/
	readonly root: Root;

	/**
	 * @attr focusView
	 * 
	 * Current focus view
	 * 
	 * @type View
	 * @get
	*/
	readonly focusView: View;

	/**
	 * @attr navigationRect
	 * 
	 * Navigation rect, possibly in the Android bottom navigation button area
	 * 
	 * @type {Rect}
	 * @get
	*/
	readonly navigationRect: types.Rect;

	/**
	 * @attr szie
	 * 
	 * * Set the logical size of the window. When this value changes, the change event will be triggered.
	 *
	 * * When both width and height are set to zero, the most comfortable default display size is automatically set.
	 *
	 * * If the width is set to non-zero, it means the width is fixed, and the height is automatically set according to the window ratio.
	 *
	 * * If the height is set to non-zero, it means the height is fixed, and the width is automatically set according to the window ratio.
	 *
	 * @type {Vec2}
	 * @get
	*/
	size: types.Vec2;

	/**
	 * @attr backgroundColor
	 * 
	 * The background color for the window
	 * 
	 * @type {Color}
	 * @get
	*/
	backgroundColor: types.Color;

	/**
	 * @attr surfaceSize
	 * 
	 * Window physical surface pixel size
	 * 
	 * @type {Vec2}
	 * @get
	*/
	surfaceSize: types.Vec2;

	/**
	 * @method nextFrame(cb)
	 * 
	 * When rendering next frame call
	 * 
	 * @param cb {Function}
	 * @return {this}
	*/
	nextFrame(cb: ()=>void): this;

	/**
	 * @method activate
	 * 
	 * Activate the display window or taking the window to the foreground
	 * 
	 * @return {this}
	*/
	activate(): this;

	/**
	 * @method close()
	 * 
	 * To close or destroy the window
	*/
	close(): void;

	/**
	 * @method pending() 
	 * 
	 * Taking window to background and pending
	*/
	pending(): void;

	/**
	 * @method setFullscreen(fullscreen)
	 * 
	 * Setting whether the window is displayed in full screen mode
	 * 
	 * @param fullscreen {bool}
	*/
	setFullscreen(fullscreen: boolean): void;

	/**
	 * @method setCursorStyle(style)
	 * 
	 * Setting the mouse cursor style
	 * 
	 * @param style {CursorStyle}
	*/
	setCursorStyle(style: types.CursorStyle): void;

	/**
	 * @constructor([opts])
	 * @param opts? {Options}
	*/
	constructor(opts?: Options);
}

/**
 * @class Window
 * @extends NativeWindow
*/
export class Window extends (_ui.Window as typeof NativeWindow) {

	/**
	 * @event onChange() Trigger when to change window size
	*/
	@event readonly onChange: EventNoticer<WEvent>;

	/**
	 * @event onBackground() Trigger when into background
	*/
	@event readonly onBackground: EventNoticer<WEvent>;

	/**
	 * @event onForeground() Trigger when into foreground
	*/
	@event readonly onForeground: EventNoticer<WEvent>;

	/**
	 * @event onClose() Trigger when closed window
	*/
	@event readonly onClose: EventNoticer<WEvent>;

	/**
	 * @attr rootCtr
	 * 
	 * The only root view controller in the window
	 * 
	 * @type {ViewController}
	 * @get
	*/
	readonly rootCtr: ViewController = new RootViewController(this);

	/**
	* @method render(vdom)
	* 
	* Create a view or view controller `DOM` by `vdom`
	* 
	* @param vdom {VirtualDOM}
	* @return {DOM}
	* 
	* Example:
	* ```tsx
	* import {Application,Window,ViewController} from 'quark'
	* import * as http from 'quark/http'
	* import * as buffer from 'quark/buffer'
	* class MyCtr extends ViewController<{param: number}, {data?: Uint8Array}> {
	* 	triggerLoad() {
	* 		return http
	* 			.get('http://192.168.1.100:1026/README.md?param=' + this.props.param)
	* 			.then(e=>this.setState({data:e.data}));
	* 	}
	* 	render() {
	* 		return (
	* 			<box width={100} height={100} backgroundColor="#f00">
	* 				{this.state.data&&buffer.toString(this.state.data)}
	* 			</box>
	* 		)
	* 	}
	* }
	* new Application();
	* new Window().activate().render(
	* 	<MyCtr param={10} />
	* );
	* ```
	*/
	render(vdom: VirtualDOM) {
		let dom = vdom.newDom(this.rootCtr);
		dom.appendTo(this.root);
		return dom;
	}

	/**
	 * @mehod nextTickFrame(cb)
	 * 
	 * Calling nextTick on the next loop tick means the next frame of the next loop tick
	 * 
	 * @param cb {Function}
	*/
	nextTickFrame(cb: () => void) {
		util.nextTick(()=>this.nextFrame(cb));
		return this;
	}
}

util.extendClass(Window, NativeNotification);