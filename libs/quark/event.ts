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

/**
 * The type or device that triggers the click event, such as using a touch screen or a mouse
*/
export enum ClickType {
	/** touchscreen */
	Touch = 1,
	/** Press Enter on the keyboard or press OK on the remote control */
	Keyboard,
	/** mouse */
	Mouse,
};

/**
 * Status type of event `Highlighted`
*/
export enum HighlightedStatus {
	/** Normal situation */
	Normal = 1,
	/** When the mouse moves over the view or the focus switches to the view */
	Hover,
	/** When the mouse is pressed or the OK button is pressed on the remote control */
	Active,
};

/**
 * Mask function corresponding to event return value (returnValue)
*/
export enum ReturnValueMask {
	/** Enable default actions */
	Default = (1 << 0),
	/** Enable event bubbling */
	Bubble = (1 << 1),
	/** Both of the above include */
	All = (Default | Bubble),
};

declare class NativeEvent<Sender, Data = any> extends Event<Sender, Data> {}

/**
 * @class UIEvent
 * @extends Event
 * 
 * UI system event data object basic type
*/
export declare class UIEvent extends NativeEvent<View> {
	/** Origin of the event */
	readonly origin: View;
	/** The time when the event is triggered */
	readonly timestamp: number; 
	/** Gets whether the default action of an event is enabled */
	readonly isDefault: boolean;
	/** Gets whether event bubbling is enabled */
	readonly isBubble: boolean;
	/** Cancel the default event action, if one exists */
	cancelDefault(): void;
	/** Cancel event bubbling. After calling this method, the event stops propagating upward */
	cancelBubble(): void;
}

/**
 * @class ActionEvent
 * @extends UIEvent
*/
export declare class ActionEvent extends UIEvent {
	/** Get the action object that triggers the event */
	readonly action: Action;
	/** The difference between the event triggering time and the target time */
	readonly delay: number;
	/** Keyframe Index */
	readonly frame: number;
	/** The current number of looped */
	readonly looped: number;
}

/**
 * @class KeyEvent
 * @extends UIEvent
*/
export declare class KeyEvent extends UIEvent {
	/** The next possible remote focus view */
	nextFocus: View | null;
	/** Keyboard Code */
	readonly keycode: KeyboardKeyCode;
	/** Ascii code converted from keyboard code */
	readonly keypress: number;
	/** The number of times the event is triggered repeatedly after the key is pressed
	 * until the key is released */
	readonly repeat: number;
	/** Is the shift key pressed */
	readonly shift: boolean;
	/** Is the ctrl key pressed */
	readonly ctrl: boolean;
	/** Is the alt key pressed */
	readonly alt: boolean;
	/** Is the command key pressed */
	readonly command: boolean;
	/** Is capsLock enabled */
	readonly capsLock: boolean;
	/** Device id */
	readonly device: number;
	/** Source id */
	readonly source: number;
}

/**
 * @class ClickEvent
 * @extends UIEvent
*/
export declare class ClickEvent extends UIEvent {
	/** The cursor position in the window x */
	readonly x: number;
	/** The cursor position in the window y */
	readonly y: number;
	/** Number of consecutive clicks, possibly double clicks */
	readonly count: number;
	/** Click trigger type */
	readonly type: ClickType;
}

/**
 * When this event triggers, 
 * the highlighted state that the view should enter after triggering certain events.
 * For example: when the mouse enters the view, the state is `Hover`
 * 
 * @class HighlightedEvent
 * @extends UIEvent
 */
export declare class HighlightedEvent extends UIEvent {
	/** Highlight state type */
	readonly status: HighlightedStatus;
}

/**
 * @class MouseEvent
 * @extends UIEvent
*/
export declare class MouseEvent extends UIEvent {
	/** The cursor position in the window x */
	readonly x: number;
	/** The cursor position in the window y */
	readonly y: number;
}

/**
 * @interface TouchPoint
 * 
 * Touch point data of the touch event
*/
export interface TouchPoint {
	/** Touch point id */
	readonly id: number;
	/** The position x of the touchpoint in the window when it is first pressed */
	readonly startX: number;
	/** The position y of the touchpoint in the window when it is first pressed */
	readonly startY: number;
	/** The position of the touchpoint in the window x */
	readonly x: number;
	/** The position of the touchpoint in the window y */
	readonly y: number;
	/** touchpoint pressure strength, value range [0-1] */
	readonly force: number;
	/** Whether the touchpoint is still within the click range.
	 * If the touchpoint is within the click range when it is pressed,
	 * the touchpoint may move out of the range later. */
	readonly clickIn: boolean;
	/** The initial view of the touchpoint */
	readonly view: View;
}

/**
 * @class TouchEvent
 * @extends UIEvent
*/
export declare class TouchEvent extends UIEvent {
	/** Touch point collection */
	readonly changedTouches: TouchPoint[];
}

const _init = __binding__('_init');
const PREFIX = '_on';

/**
 * @class NativeNotification
 * @extends Notification
 */
export class NativeNotification<E = Event> extends Notification<E> {

	getNoticer(name: string) {
		let noticer = (this as any)[PREFIX + name] as EventNoticer<E>;
		if ( ! noticer ) {
			// bind native event
			let func = (this as any)['trigger' + name]; // triggerClick
			// bind native
			_init.addNativeEventListener(this, name, (event?: any, isEvent?: boolean) => {
				//console.log('_init.addNativeEventListener', name);
				let evt = event && isEvent ? event: new Event(event);
				let ok = func ? func.call(this, evt): this.triggerWithEvent(name, evt);
				//console.log('_init.addNativeEventListener', name, ok, String(trigger));
				return ok;
			}, 0);
			(this as any)[PREFIX + name] = noticer = new EventNoticer<E>(name, this as any);
		}
		return noticer;
	}
}
