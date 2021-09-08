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
import app from './app';
import {Mat4} from './value';
import event, { EventNoticer, NativeNotification, Notification } from './event';

const _display_port = __require__('_display_port');

export enum Orientation {
	ORIENTATION_INVALID = _display_port.ORIENTATION_INVALID as number,
	ORIENTATION_PORTRAIT = _display_port.ORIENTATION_PORTRAIT as number,
	ORIENTATION_LANDSCAPE = _display_port.ORIENTATION_LANDSCAPE as number,
	ORIENTATION_REVERSE_PORTRAIT = _display_port.ORIENTATION_REVERSE_PORTRAIT as number,
	ORIENTATION_REVERSE_LANDSCAPE = _display_port.ORIENTATION_REVERSE_LANDSCAPE as number,
	ORIENTATION_USER = _display_port.ORIENTATION_USER as number,
	ORIENTATION_USER_PORTRAIT = _display_port.ORIENTATION_USER_PORTRAIT as number,
	ORIENTATION_USER_LANDSCAPE = _display_port.ORIENTATION_USER_LANDSCAPE as number,
	ORIENTATION_USER_LOCKED = _display_port.ORIENTATION_USER_LOCKED as number,
}

export enum StatusBarStyle {
	STATUS_BAR_STYLE_WHITE = _display_port.STATUS_BAR_STYLE_WHITE as number,
	STATUS_BAR_STYLE_BLACK = _display_port.STATUS_BAR_STYLE_BLACK as number,
}

export declare class Display extends Notification {
	onChange: EventNoticer;
	onOrientation: EventNoticer;
	lockSize(width?: number, height?: number): void;
	nextFrame(cb: ()=>void): void;
	keepScreen(keep: boolean): void;
	statusBarHeight(): number;
	setVisibleStatusBar(visible: boolean): void;
	setStatusBarStyle(style: StatusBarStyle): void;
	requestFullscreen(fullscreen: boolean): void;
	orientation(): Orientation;
	setOrientation(orientation: Orientation): void;
	fsp(): number;
	width: number;
	height: number;
	phyWidth: number;
	phyHeight: number;
	bestScale: number;
	scale: number;
	scaleValue: number;
	rootMatrix: Mat4;
	atomPixel: number;
}

class _Display extends NativeNotification {
	@event onChange: EventNoticer;
	@event onOrientation: EventNoticer;
}

utils.extendClass(_display_port.Display, _Display);

exports.Display = _display_port.Display;

export default {

	get defaultAtomPixel(): number { return _display_port.defaultAtomPixel() },
	get defaultStatusBarHeight(): number { return _display_port.defaultStatusBarHeight() },

	/**
	 * @get current {Display}
	 */
	get current(): Display { return app.current.displayPort; },

	/**
	 * @get atomPixel {float}
	 */
	get atomPixel(): number {
		return app.current ? 
			app.current.displayPort.atomPixel: 
			_display_port.defaultAtomPixel();
	},

	/**
	 * @get statusBarHeight {float}
	 */
	get statusBarHeight(): number {
		return app.current ? 
			app.current.displayPort.statusBarHeight(): 
			_display_port.defaultStatusBarHeight();
	},

	/**
	 * @func nextFrame(cb)
	 * @arg cb {Function}
	 */
	nextFrame(cb: ()=>void) {
		if ( app.current ) {
			app.current.displayPort.nextFrame(cb);
		} else {
			throw new Error("Application has not been created");
		}
	},
}
