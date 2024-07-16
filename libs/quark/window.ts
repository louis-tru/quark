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

export interface Options {
	olorType?: number; // init window color type
	msaa?: number; // init window gpu render msaa count
	fps?: number; // init window render fsp limit
	frame?: types.Rect; // init window params
	title?: string; // init window title
	backgroundColor?: types.Color; // init window background color
};

class RootViewController extends ViewController {
	constructor(window: Window) {
		super({}, { window, children: [], owner: null as any });
	}
	get metaView() { return this.window.root }
	setState() { throw Error.new('Access forbidden.') }
	update() { throw Error.new('Access forbidden.') }
	appendTo(): View { throw Error.new('Access forbidden.') }
	afterTo(): View { throw Error.new('Access forbidden.') }
	remove() { throw Error.new('Access forbidden.') }
}

declare class NativeWindow extends Notification<WEvent> {
	readonly scale: number;
	readonly defaultScale: number;
	readonly fsp: number;
	readonly atomPixel: number;
	readonly root: Root;
	readonly focusView: View;
	size: types.Vec2;
	backgroundColor: types.Color;
	surfaceSize: types.Vec2;
	nextFrame(cb: ()=>void): void;
	activate(): void;
	close(): void;
	pending(): void;
	setFullscreen(fullscreen: boolean): void;
	setCursorStyle(style: types.CursorStyle): void;
	constructor(opts?: Options);
}

export class Window extends (_ui.Window as typeof NativeWindow) {
	@event readonly onChange: EventNoticer<WEvent>;
	@event readonly onBackground: EventNoticer<WEvent>;
	@event readonly onForeground: EventNoticer<WEvent>;
	@event readonly onClose: EventNoticer<WEvent>;
	readonly rootCtr: ViewController = new RootViewController(this);
	render(vdom: VirtualDOM) {
		vdom.newDom(this.rootCtr).appendTo(this.root);
		return this;
	}
}

util.extendClass(Window, NativeNotification);