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
 * 触发点击事件的有类型或设备，如：使用触摸屏幕、鼠标
*/
export enum ClickType {
	/** 触摸屏 */
	Touch = 1,
	/** 键盘或遥控器按下回车或进入 */
	Keyboard,
	/** 鼠标 */
	Mouse,
};

/**
 * 事件`Highlighted`的状态类型
*/
export enum HighlightedStatus {
	/** 一般通常情况 */
	Normal = 1,
	/** 当鼠标移动到视图之上或焦点切换到视图的状态 */
	Hover,
	/** 当鼠标按下或者遥控器按下进入按钮的状态 */
	Active,
};

/**
 * 事件返回值（returnValue）对应的掩码功能
*/
export enum ReturnValueMask {
	/** 启用默认动作 */
	Default = (1 << 0),
	/** 启用事件冒泡 */
	Bubble = (1 << 1),
	/** 以上两个都包含 */
	All = (Default | Bubble),
};

declare class NativeEvent<Sender, Data = any> extends Event<Sender, Data> {}

/**
 * @class UIEvent
 * @extends Event
 * 
 * UI系统事件数据对像基础类型
*/
export declare class UIEvent extends NativeEvent<View> {
	/** 事件的发源地 */
	readonly origin: View;
	/** 事件触发的时间 */
	readonly timestamp: number; 
	/** 获取是否启用事件默认动作 */
	readonly isDefault: boolean;
	/** 获取是否启用事件冒泡 */
	readonly isBubble: boolean;
	/** 取消默认事件动作，如果存在默认动作 */
	cancelDefault(): void;
	/** 取消事件冒泡，调用后事件停止向上传播 */
	cancelBubble(): void;
}

/**
 * @class ActionEvent
 * @extends UIEvent
*/
export declare class ActionEvent extends UIEvent {
	/** 获取触发事件的动作对像 */
	readonly action: Action;
	/** 事件触发时与目标时间的误差 */
	readonly delay: number;
	/** 关键帧索引 */
	readonly frame: number;
	/** 当前循环次数 */
	readonly looped: number;
}

/**
 * @class KeyEvent
 * @extends UIEvent
*/
export declare class KeyEvent extends UIEvent {
	/** 下一个可能的遥控器焦点视图 */
	nextFocus: View | null;
	/** 键盘代码 */
	readonly keycode: KeyboardKeyCode;
	/** 通过键盘代码转换的aciss代码 */
	readonly keypress: number;
	/** 按键按下后重复触发事件的次数，直到按键被松开 */
	readonly repeat: number;
	/** 是否按下了shift键 */
	readonly shift: boolean;
	/** 是否按下了ctrl键 */
	readonly ctrl: boolean;
	/** 是否按下了alt键 */
	readonly alt: boolean;
	/** 是否按下了command键 */
	readonly command: boolean;
	/** 是否启用了大写锁capsLock */
	readonly capsLock: boolean;
	/** 输入设备id */
	readonly device: number;
	/** 输入源id */
	readonly source: number;
}

/**
 * @class ClickEvent
 * @extends UIEvent
*/
export declare class ClickEvent extends UIEvent {
	/** 光标在窗口中的位置x */
	readonly x: number;
	/** 光标在窗口中的位置y */
	readonly y: number;
	/** 点击次数单击、双击 */
	readonly count: number;
	/** 点击触发类型 */
	readonly type: ClickType;
}

/**
 * @class HighlightedEvent
 * @extends UIEvent
 * 
 * 这个事件表示视图在触发某些事件后应该进入什么样的状态，
 * 如：当鼠标进入视图时状态为`Hover`
*/
export declare class HighlightedEvent extends UIEvent {
	/** 高亮状态类型 */
	readonly status: HighlightedStatus;
}

/**
 * @class MouseEvent
 * @extends UIEvent
*/
export declare class MouseEvent extends UIEvent {
	/** 光标在窗口中的位置x */
	readonly x: number;
	/** 光标在窗口中的位置y */
	readonly y: number;
}

/**
 * @class TouchPoint
 * @extends UIEvent
 * 
 * 触摸事件触摸点数据
*/
export interface TouchPoint {
	/** 触点id */
	readonly id: number;
	/** 触点最开始按下时在窗口中的位置x */
	readonly startX: number;
	/** 触点最开始按下时在窗口中的位置y */
	readonly startY: number;
	/** 触点在窗口中的位置x */
	readonly x: number;
	/** 触点在窗口中的位置y */
	readonly y: number;
	/** 触点按压强度,值范围[0-1] */
	readonly force: number;
	/** 触点是否还依然在点击范围内，如果触点刚按下时在触点内，但后面触点可能会移动出范围 */
	readonly clickIn: boolean;
	/** 触点最开始的视图 */
	readonly view: View;
}

/**
 * @class TouchEvent
 * @extends UIEvent
*/
export declare class TouchEvent extends UIEvent {
	/** 触摸点集合 */
	readonly changedTouches: TouchPoint[];
}

const _init = __binding__('_init');
const PREFIX = '_on';

/**
 * @template E
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
