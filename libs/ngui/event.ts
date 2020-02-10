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

import {Event,EventNoticer,Notification} from './_event';
export * from './_event';
import {Action} from './_action';
import {View} from './_view';

export default (exports.event as (target: any, name: string)=>void);

// GUI EVENT 

export enum ClickType { 
	TOUCH = 1, KEYBOARD = 2, MOUSE = 3,
};

export enum HighlightedStatus {
	HIGHLIGHTED_NORMAL = 1,
	HIGHLIGHTED_HOVER,
	HIGHLIGHTED_DOWN,
};

export enum ReturnValueMask {
	RETURN_VALUE_MASK_DEFAULT = (1 << 0),
	RETURN_VALUE_MASK_BUBBLE = (1 << 1),
	RETURN_VALUE_MASK_ALL = (RETURN_VALUE_MASK_DEFAULT | RETURN_VALUE_MASK_BUBBLE),
};

declare class NativeEvent<Sender = any> extends Event<any, Sender> {}

export declare class GUIEvent extends NativeEvent<View> {
	readonly origin: View;
	readonly timestamp: number;
	readonly isDefault: boolean;
	readonly isBubble: boolean;
	cancelDefault(): void;
	cancelBubble(): void;
}

export declare class GUIActionEvent extends GUIEvent {
	readonly action: Action;
	readonly delay: number;
	readonly frame: number;
	readonly loop: number;
}

export declare class GUIKeyEvent extends GUIEvent {
	readonly keycode: number;
	readonly repeat: number;
	readonly shift: boolean;
	readonly ctrl: boolean;
	readonly alt: boolean;
	readonly command: boolean;
	readonly capsLock: boolean;
	readonly device: number;
	readonly source: number;
	readonly focusMove: View | null;
}

export declare class GUIClickEvent extends GUIEvent {
	readonly x: number;
	readonly y: number;
	readonly count: number;
	readonly type: ClickType;
}

export declare class GUIHighlightedEvent extends GUIEvent {
	readonly status: HighlightedStatus;
}

export declare class GUIMouseEvent extends GUIEvent {
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

export declare class GUITouchEvent extends GUIEvent {
	readonly changedTouches: TouchPoint[];
}

export declare class GUIFocusMoveEvent extends GUIEvent {
	readonly focus: View | null;
	readonly focusMove: View | null;
}

const _util = __requireNgui__('_util');
const PREFIX = 'm_on';

/**
 * @class NativeNotification
 */
export class NativeNotification<Data = any, Return = number, Sender = any> extends Notification<GUIEvent>  {

	getNoticer(name: string) {
		var noticer = (this as any)[PREFIX + name] as EventNoticer<GUIEvent>;
		if ( ! noticer ) {
			// bind native event
			var func = (this as any)['trigger' + name];
			// bind native
			_util.addNativeEventListener(this, name, (event?: any, isEvent?: boolean) => {
				//console.log('_util.addNativeEventListener', name);
				var evt = event && isEvent ? event: new Event(event);
				var ok = func ? func.call(this, evt): this.triggerWithEvent(name, evt);
				//console.log('_util.addNativeEventListener', name, ok, String(trigger));
				return ok;
			}, -1);
			(this as any)[PREFIX + name] = noticer = new EventNoticer<GUIEvent>(name, this as any);
		}
		return noticer;
	}
	
}
