/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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

/**
 * @enum Orientation
*/
export enum Orientation {
	/** Invalid, Default use the User mode */
	Invalid,
	/** Portrait */
	Portrait = (1 << 0),
	/** Landscape, Rotate 90 degrees clockwise */
	Landscape = (1 << 1),
	/** Reverse Portrait, Rotate 180 degrees clockwise */
	Reverse_Portrait = (1 << 2),
	/** Reverse Landscape, Rotate 270 degrees clockwise */
	Reverse_Landscape = (1 << 3),
	/** User, Any orientation is allowed */
	User = (Portrait | Landscape | Reverse_Portrait | Reverse_Landscape),
	/** User Portrait, Portrait and Reverse_Portrait directions available */
	User_Portrait = (Portrait | Reverse_Portrait),
	/** User Landscape, Landscape and Reverse_Landscape directions available */
	User_Landscape = (Landscape | Reverse_Landscape),
	/** User Locked, Lock the application's startup state */
	User_Locked = (1 << 4),
};

/**
 * @enum StatusBarStyle
*/
export enum StatusBarStyle {
	/** White */
	White,
	/** Black */
	Black,
};

/**
 * @class Screen
 * @extends Notification
*/
export declare class Screen extends Notification<Event<Screen>> {
	/**
	 * @event onOrientation
	 * 
	 * Trigger when screen direction change
	*/
	readonly onOrientation: EventNoticer<Event<Screen>>;

	/**
	 * Current screen direction
	*/
	readonly orientation: Orientation;

	/**
	*/
	readonly statusBarHeight: number;

	/**
	 * Setting whether visible for the status bar
	*/
	setVisibleStatusBar(visible: boolean): void;

	/**
	 * Setting style for the status bar
	*/
	setStatusBarStyle(style: StatusBarStyle): void;

	/**
	 * "true" means to prevent screen sleep else no prevent
	*/
	preventScreenSleep(prevent: boolean): void;
}

class _Screen extends NativeNotification {
	@event readonly onOrientation: EventNoticer<Event<Screen>>;
}
util.extendClass(_ui.Screen, _Screen);
exports.Screen = _ui.Screen;
exports.mainScreenScale = _ui.mainScreenScale;

export declare function mainScreenScale(): number;