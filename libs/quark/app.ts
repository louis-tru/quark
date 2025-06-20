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

///<reference path="_ext.ts"/>

import util from './util';
import {TextOptions} from './view';
import {Screen} from './screen';
import {Window} from './window';
import {FontPool} from './font';
import event, {EventNoticer, Notification, NativeNotification, Event} from './event';

const _ui = __binding__('_ui');
let _current: Application | null = null;
type AEvent = Event<Application>;

/**
 * @class NativeApplication
*/
declare class NativeApplication extends Notification<AEvent> {
	/** After onLoad event */
	readonly isLoaded: boolean;
	/** screen Object */
	readonly screen: Screen;
	/** fontPool Object */
	readonly fontPool: FontPool;
	/** current active window */
	readonly activeWindow: Window | null;
	/** default text options */
	readonly defaultTextOptions: TextOptions;
	/** all of windows */
	readonly windows: Window[];
	/** get or set max resource memory limit */
	maxResourceMemoryLimit: number;
	/** get current used resource memory */
	usedResourceMemory(): number;
	/** clear resource memory */
	clear(all?: boolean): void;
	/** open uri For: examples openURL(`'https://baidu.com'`) */
	openURL(url: string): void;
	/**
	 * By parameter open email client
	*/
	sendEmail(recipient: string, subject: string, body?: string, cc?: string, bcc?: string): void;
}

/**
 * @class Application
 * @extends NativeApplication
*/
export class Application extends (_ui.NativeApplication as typeof NativeApplication) {
	/** After initialization the application call */
	@event readonly onLoad: EventNoticer<AEvent>;
	/** Before unload the application call */
	@event readonly onUnload: EventNoticer<AEvent>;
	/** When enter the background call */
	@event readonly onPause: EventNoticer<AEvent>;
	/** When enter the foreground call */
	@event readonly onResume: EventNoticer<AEvent>;
	/**
	 * Triggered when memory is insufficient. After triggering,
	 * it will automatically call `clear` to clean up resources and perform `js` garbage collection
	*/
	@event readonly onMemoryWarning: EventNoticer<AEvent>;
	constructor() {
		super();
		_current = this;
	}
}

util.extendClass(Application, NativeNotification);

export default {
	get current() { return _current! },
};