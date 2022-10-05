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

///<reference path="_ext.ts"/>

import utils from './util';
import {Display} from './display';
import { Root, View } from './_view';
import * as value from './value';
import event, {EventNoticer, Notification, NativeNotification, Event} from './event';
import ViewController, { VirtualDOM, _CVD, _CVDD } from './ctr';

const _quark = __require__('_quark');

var cur: Application | null = null;
var cur_root_ctr: ViewController | null = null;

export interface Options {
	anisotropic?: boolean;
	multisample?: 0|1|2|3|4;
	x?: number,
	y?: number,
	width?: number,
	height?: number,
	fullScreen?: boolean,
	enableTouch?: boolean,
	background?: number,
	title?: string;
}

/**
 * @class NativeApplication
 */
declare class NativeApplication extends Notification {
	constructor(options?: Options);
	clear(full?: boolean): void;
	openUrl(url: string): void;
	sendEmail(recipient: string, title: string, body?: string, cc?: string, bcc?: string): void;
	maxTextureMemoryLimit(): number;
	setMaxTextureMemoryLimit(limit: number): void;
	usedMemory(): number;
	pending(): void;
	readonly isLoaded: boolean;
	readonly displayPort: Display;
	readonly root: Root | null;
	readonly focusView: View | null;
	defaultTextBackgroundColor: value.TextColor;
	defaultTextColor: value.TextColor;
	defaultTextSize: value.TextSize;
	defaultTextStyle: value.TextStyle;
	defaultTextFamily: value.TextFamily;
	defaultTextShadow: value.TextShadow;
	defaultTextLineHeight: value.TextLineHeight;
	defaultTextDecoration: value.TextDecoration;
	defaultTextOverflow: value.TextOverflow;
	defaultTextWhiteSpace: value.TextWhiteSpace;
}

/**
 * @class Application
 */
export class Application extends (_quark.NativeApplication as typeof NativeApplication) {

	@event readonly onLoad: EventNoticer<Event<void, Application>>;
	@event readonly onUnload: EventNoticer<Event<void, Application>>;
	@event readonly onBackground: EventNoticer<Event<void, Application>>;
	@event readonly onForeground: EventNoticer<Event<void, Application>>;
	@event readonly onPause: EventNoticer<Event<void, Application>>;
	@event readonly onResume: EventNoticer<Event<void, Application>>;
	@event readonly onMemoryWarning: EventNoticer<Event<void, Application>>;
	
	constructor(options?: Options) {
		super(options);
		cur = this;
	}

	/**
	 * @func start(vdom)
	 */
	start(vdom: VirtualDOM) {

		if (!utils.equalsClass(ViewController, vdom.domConstructor)) {
			vdom = _CVD(ViewController, null, _CVDD(vdom));
		}

		function render() {
			var ctr = ViewController.render(vdom) as ViewController;
			utils.assert(ctr.dom instanceof Root, 'Root view controller first children must be Root view');
			cur_root_ctr = ctr;
		}

		if ( this.isLoaded ) {
			_quark.lock(render);
		} else {
			this.onLoad.on(render);
		}

		return this;
	}

}

utils.extendClass(Application, NativeNotification);

export default {
	get current() { return cur as Application },
	get root() { return (cur_root_ctr as ViewController).dom as unknown as Root },
	get rootCtr() { return cur_root_ctr as ViewController },
};