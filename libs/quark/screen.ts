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
import event, { Event, EventNoticer, NativeNotification, Notification } from './event';

const _ui = __binding__('_ui');

export enum Orientation {
	Invalid,
	Portrait,
	Landscape,
	Reverse_Portrait,
	Reverse_Landscape,
	User,
	User_Portrait,
	User_Landscape,
	User_Locked,
};

export enum StatusBarStyle {
	White, Black,
};

export declare class Screen extends Notification<Event<Screen>> {
	readonly onOrientation: EventNoticer<Event<Screen>>;
	readonly orientation: Orientation;
	readonly statusBarHeight: number;
	setVisibleStatusBar(visible: boolean): void;
	setStatusBarStyle(style: StatusBarStyle): void;
	preventScreenSleep(prevent: boolean): void;
}

class _Screen extends NativeNotification {
	@event readonly onOrientation: EventNoticer<Event<Screen>>;
}
util.extendClass(_ui.Screen, _Screen);
exports.Screen = _ui.Screen;
export declare function mainScreenScale(): number;