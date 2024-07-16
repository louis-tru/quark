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

declare class NativeApplication extends Notification<AEvent> {
	readonly isLoaded: boolean; //!< after onLoad event
	readonly screen: Screen;
	readonly fontPool: FontPool;
	readonly activeWindow: Window | null;
	readonly defaultTextOptions: TextOptions;
	readonly windows: Window[];
	maxResourceMemoryLimit: number; //!< get or set max resource memory limit
	usedResourceMemory(): number; //!< current used resource memory
	clear(all?: boolean): void; //!< clear resource memory
	openURL(url: string): void;
	sendEmail(recipient: string, subject: string, body?: string, cc?: string, bcc?: string): void;
}

export class Application extends (_ui.Application as typeof NativeApplication) {
	@event readonly onLoad: EventNoticer<AEvent>;
	@event readonly onUnload: EventNoticer<AEvent>;
	@event readonly onBackground: EventNoticer<AEvent>;
	@event readonly onForeground: EventNoticer<AEvent>;
	@event readonly onPause: EventNoticer<AEvent>;
	@event readonly onResume: EventNoticer<AEvent>;
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