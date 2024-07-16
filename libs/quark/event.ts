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

import {Event,EventNoticer,Notification,event} from './_event';
import {Action} from './action';
import {View} from './view';
import {KeyboardKeyCode} from './keyboard';

export * from './_event';
export default event;

export enum ClickType {
	Touch = 1, Keyboard, Mouse
};

export enum HighlightedStatus {
	Normal = 1, Hover, Active
};

export enum ReturnValueMask {
	Default = (1 << 0),
	Bubble = (1 << 1),
	All = (Default | Bubble),
};

declare class NativeEvent<Sender, Data = any> extends Event<Sender, Data> {}

export declare class UIEvent extends NativeEvent<View> {
	readonly origin: View;
	readonly timestamp: number;
	readonly isDefault: boolean;
	readonly isBubble: boolean;
	cancelDefault(): void;
	cancelBubble(): void;
}

export declare class ActionEvent extends UIEvent {
	readonly action: Action;
	readonly delay: number;
	readonly frame: number;
	readonly loop: number;
}

export declare class KeyEvent extends UIEvent {
	nextFocus: View | null;
	readonly keycode: KeyboardKeyCode;
	readonly keypress: number;
	readonly repeat: number;
	readonly shift: boolean;
	readonly ctrl: boolean;
	readonly alt: boolean;
	readonly command: boolean;
	readonly capsLock: boolean;
	readonly device: number;
	readonly source: number;
}

export declare class ClickEvent extends UIEvent {
	readonly x: number;
	readonly y: number;
	readonly count: number;
	readonly type: ClickType;
}

export declare class HighlightedEvent extends UIEvent {
	readonly status: HighlightedStatus;
}

export declare class MouseEvent extends UIEvent {
	readonly x: number;
	readonly y: number;
}

export interface TouchPoint {
	readonly id: number;
	readonly startX: number;
	readonly startY: number;
	readonly x: number; 
	readonly y: number;
	readonly force: number;
	readonly clickIn: boolean;
	readonly view: View;
}

export declare class TouchEvent extends UIEvent {
	readonly changedTouches: TouchPoint[];
}

const _util = __binding__('_util');
const PREFIX = '_on';

/**
 * @class NativeNotification
 */
export class NativeNotification<E = Event> extends Notification<E> {

	getNoticer(name: string) {
		let noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if ( ! noticer ) {
			// bind native event
			let func = (this as any)['trigger' + name]; // triggerClick
			// bind native
			_util.addNativeEventListener(this, name, (event?: any, isEvent?: boolean) => {
				//console.log('_util.addNativeEventListener', name);
				let evt = event && isEvent ? event: new Event(event);
				let ok = func ? func.call(this, evt): this.triggerWithEvent(name, evt);
				//console.log('_util.addNativeEventListener', name, ok, String(trigger));
				return ok;
			}, -1);
			(this as any)[PREFIX + name] = noticer = new EventNoticer<E>(name, this as any);
		}
		return noticer;
	}
}
