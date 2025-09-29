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
import {RemoveReadonly,Vec2,newVec2,PIHalf,PIHalfHalf,Direction,PIDegree} from './types';
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
}

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

/**
 * Spine animation event types
*/
export enum SpineEventType {
	kStart_Type,      //!< Animation started
	kInterrupt_Type,  //!< Animation interrupted
	kEnd_Type,        //!< Animation reached its end
	kComplete_Type,   //!< Animation completed one loop
	kDispose_Type,    //!< Animation (TrackEntry) disposed
	kEvent_Type,      //!< Custom Spine event
	kKeyEvent_Type = kEvent_Type, //!< Alias for custom keyframe events
};

/**
 * Gesture stage
*/
export enum GestureStage {
	/** The gesture has just started */
	Start,
	/** The gesture is changing */
	Change,
	/** The gesture has ended */
	End,
	/** The gesture has been cancelled */
	Cancel,
}

/**
 * Gesture event type
*/
export enum GestureType {
	Gesture, // Base
	SwipeGesture, // Swipe, Flick, 1 finger gesture
	PanGesture, // Pan, Drag, 1 finger gesture
	PinchGesture, // Pinch, Zoom, 2 finger gesture
	RotateGesture, // Rotate, Twist 2 finger gesture
	ThreeFingerGesture, // 3 finger gesture
	FourFingerGesture, // 4 finger gesture
}

/**
 * @class UIEvent
 * @extends Event
 * 
 * UI system event data object basic type
*/
export declare class UIEvent extends Event<View> {
	/** Origin of the event */
	readonly origin: View;
	/** The time when the event is created, in milliseconds */
	readonly timestamp: Int;
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
	/** The cursor position in the window */
	readonly location: Vec2;
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
	/** The cursor position in the window */
	readonly location: Vec2;
}

/**
 * @interface TouchPoint
 * 
 * Touch point data of the touch event
*/
export interface TouchPoint {
	/** Touch point id */
	readonly id: number;
	/** The position x/y of the touchpoint in the window when it is first pressed */
	readonly startLocation: Vec2;
	/** The position of the touchpoint in the window */
	readonly location: Vec2;
	/** touchpoint pressure strength, value range [0-1] */
	readonly force: number;
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

/**
 * @class GestureTouchPoint
 * @implements TouchPoint
 * 
 * Gesture touch point, which is a wrapper around the touch point of the touch event
*/
export class GestureTouchPoint implements TouchPoint {
	get id() { return this.touch.id; }
	get location() { return this.touch.location; }
	get startLocation() { return this.touch.startLocation; }
	get force() { return this.touch.force; }
	get view() { return this.touch.view; }

	/**
	 * The reference touch point
	 */
	readonly touch: TouchPoint;

	/** 
	 * @param touch The reference touch point
	 */
	constructor(touch: TouchPoint) {
		this.touch = touch;
	}

	/**
	 * start and current location mid point
	*/
	get center() {
		return this.touch.startLocation.mid(this.touch.location); // mid point
	}

	/**
	 * The gesture movement distance, for example, the distance moved by a finger
	*/
	get distance() {
		return this.touch.location.sub(this.touch.startLocation);
	}

	/**
	 * The absolute value of the gesture movement distance
	*/
	get obliqueDistance() {
		return this.distance.length();
	}

	/**
	 * The gesture rotation angle in single point mode, in radians
	*/
	get angle() {
		return this.distance.angle();
	}

	/** 
	 * Get the gesture direction 
	 * */
	get direction() {
		return this.distance.direction();
	}
}

/**
 * Gesture event, such as pan, swipe, pinch, rotate, etc.
 * @class GestureEvent
 * @extends UIEvent
*/
export class GestureEvent extends Event<View> implements UIEvent {
	// implement UIEvent
	readonly origin: View;
	readonly timestamp: Int;
	readonly isDefault: boolean = true;
	readonly isBubble: boolean = false; // Never bubble
	cancelDefault(): void { (this as RemoveReadonly<typeof this>).isDefault = false; }
	cancelBubble(): void { /* Never bubble */ }

	/* reject for gesture point, only valid in the `Start` stage */
	readonly rejected: boolean = false;
	/* Expected number of touch points for the gesture event */
	private _expectedFingerCount = 1; // expected finger count
	private _swipeDistance = 50; // swipe trigger distance in points, pt = px * window.defaultScale
	private _swipeVelocity = 500; // swipe trigger velocity in points per second, pt/s

	/** Get or set the expected number of touch points for the gesture event */
	get expectedFingerCount() { return this._expectedFingerCount; }
	set expectedFingerCount(v: Uint) { this._expectedFingerCount = Math.max(this.length, Number(v) || 0);}
	/** Get or set the minimum distance (in points) that a swipe gesture needs to move */
	get swipeDistance() { return this._swipeDistance; }
	set swipeDistance(v: number) { this._swipeDistance = Math.max(10, Number(v) || 0); }
	/** Get or set the minimum speed (in points per second) that a swipe gesture needs to move */
	get swipeVelocity() { return this._swipeVelocity; }
	set swipeVelocity(v: number) { this._swipeVelocity = Math.max(50, Number(v) || 0); }

	/* initial velocity calculation time window, ms */
	private _initTimestamp = 250;
	/** Get or set the initial velocity calculation time window, in milliseconds */
	get initTimestamp() { return this._initTimestamp; }
	set initTimestamp(v: number) { this._initTimestamp = Math.max(50, Number(v) || 0); }

	/** The gesture context number id */
	readonly id: number;
	/** The gesture start timestamp, in milliseconds */
	readonly startTimestamp: Int;
	/** The current gesture stage */
	readonly stage: GestureStage = GestureStage.Start;
	/** The gesture initial distance */
	readonly initDistance: Vec2 = newVec2(0, 0);
	/** The gesture initial speed, in dp per second */
	readonly initVelocity: Vec2 = newVec2(0, 0);
	/** The gesture movement distance */
	readonly distance: Vec2 = newVec2(0, 0);
	/** The gesture speed, in dp per second */
	readonly velocity: Vec2 = newVec2(0, 0);
	/** The gesture speed in the last frame, in dp per second */
	readonly lastVelocity: Vec2 = newVec2(0, 0);
	/** The gesture touch points */
	readonly touchs: GestureTouchPoint[] = [];
	/** Whether the gesture is sealed */
	readonly sealed: boolean = false;

	/** The first gesture touch point */
	get first() { return this.touchs[0]; }
	/** The last gesture touch point */
	get last() { return this.touchs.indexReverse(0); }
	/** The number of gesture touch points */
	get length() { return this.touchs.length; }

	/**
	 * Get the initial oblique velocity of the gesture, in dp per second
	*/
	get initObliqueVelocity(): number {
		return GestureEvent.computeObliqueVelocity(this.initVelocity, this.initDistance);
	}

	/**
	 * Get the oblique velocity of the gesture, in dp per second
	*/
	get obliqueVelocity(): number {
		return GestureEvent.computeObliqueVelocity(this.velocity, this.distance);
	}

	/**
	 * @param id The gesture context number id
	 * @param timestamp The gesture start timestamp, in milliseconds
	*/
	constructor(origin: View, id: number, timestamp: Int) {
		super(void 0);
		this.origin = origin;
		this.id = id;
		this.startTimestamp = this.timestamp = timestamp;
	}

	/* Update the last velocity with smoothing */
	private updateLastVelocity(deltaPos: Vec2, deltaTime: number, windowMs: number = 100 /*80–120*/) {
		if (deltaTime <= 0)
			return this.lastVelocity;
		// Current instantaneous speed
		const instant = deltaPos.div(deltaTime);
		// Smoothing factor α
		const alpha = deltaTime / (deltaTime + windowMs);
		return newVec2(
			(1 - alpha) * this.lastVelocity.x + alpha * instant.x,
			(1 - alpha) * this.lastVelocity.y + alpha * instant.y);
	}

	/* Update the gesture state */
	private _update(timestamp: Int, stage: GestureStage) {
		let self = this as RemoveReadonly<typeof this>;
		self.stage = stage;
		if (stage != GestureStage.Change) { // Only update velocity in Change stage
			self.timestamp = timestamp;
			return;
		}
		const distance =  this.location().sub(this.startLocation());
		self.lastVelocity = this.updateLastVelocity(
				distance.sub(this.distance), timestamp - this.timestamp);
		let timeDiff = timestamp - this.startTimestamp;
		self.timestamp = timestamp;
		self.distance = distance;
		self.velocity = this.distance.mul(1e3 / timeDiff);
		if (timeDiff < this._initTimestamp) {
			self.initDistance = this.distance;
			self.initVelocity = this.velocity;
		}
	}

	/** Whether the gesture can be triggered */
	isSwipePossible(): boolean {
		return this.timestamp - this.startTimestamp < this._initTimestamp;
	}

	/**
	 * Compute the oblique velocity by velocity vector and distance vector
	*/
	static computeObliqueVelocity(velocity: Vec2, distance: Vec2): number {
		const len = distance.length();
		if (len === 0)
			return 0;
		return (velocity.x * distance.x + velocity.y * distance.y) / len;
	}

	/** Whether the gesture has been triggered */
	isSwipeTriggered(): boolean {
		if (this.isSwipePossible()) {
			const ptScale = this.origin.window.scale / this.origin.window.defaultScale;
			if (this.initObliqueVelocity * ptScale >= this._swipeVelocity &&
					this.initDistance.length() * ptScale >= this._swipeDistance) {
				let angle = (this.initDistance.angle() + PIHalfHalf) % PIHalf; // [0, PIHalf)
				if (angle < 0)
					angle += PIHalf;
				if (angle > PIHalf * 0.2 && angle < 0.8 * PIHalf)
					return true;
			}
		}
		return false;
	}

	/**
	 * Reject the current start point, point will be sent to next gesture event flow.
	 * Only valid in the `Start` stage, and the temp state will be cleared when the call ends.
	*/
	reject() {
		if (this.stage == GestureStage.Start)
			(this as RemoveReadonly<typeof this>).rejected = true;
	}

	/**
	 * Prevent the creation of new gesture contexts for
	 * subsequent new points and close the flow of gestures backwards.
	*/
	seal() {
		(this as RemoveReadonly<typeof this>).sealed = true;
	}

	/**
	 * Get the mid point of all touch points' start location
	 */
	startLocation(): Vec2 {
		return Vec2.mid(this.touchs.map(v=>v.startLocation));
	}

	/**
	 * Get the mid point of all touch points' current location
	*/
	location(): Vec2 {
		return Vec2.mid(this.touchs.map(v=>v.location));
	}

	/**
	 * Get the mid point of all touch points' start location and current location
	*/
	center(): Vec2 {
		return this.startLocation().mid(this.location());
	}

	/**
	 * Get the relative rotation angle (radians or degree) between two touch points.
	 * Positive = counter-clockwise, Negative = clockwise.
	*/
	rotation(isDegree: boolean): number {
		if (this.length < 2)
			return 0;
		const start = this.touchs[0].startLocation.angleTo(this.touchs[1].startLocation);
		const end = this.touchs[0].location.angleTo(this.touchs[1].location);
		let delta = end - start;
		// Normalize to [-π, π]
		if (delta > Math.PI)
			delta -= 2 * Math.PI;
		else if (delta < -Math.PI)
			delta += 2 * Math.PI;
		return isDegree ? delta / PIDegree: delta;
	}

	/**
	 * Get the scale of the first two touch points
	 * If there are less than two touch points, 1 is returned
	*/
	scale(): number {
		if (this.length  < 2)
			return 1;
		const start = this.touchs[1].startLocation.sub(this.touchs[0].startLocation);
		const end = this.touchs[1].location.sub(this.touchs[0].location);
		return end.length() / start.length();
	}

	/**
	 * Get the distance vector between two touch points
	*/
	distanceOfTwoFinger(): Vec2 {
		if (this.length < 2)
			return newVec2(0, 0);
		return this.touchs[1].location.sub(this.touchs[0].location);
	}
}

/**
 * @class SpineEvent
 * @extends UIEvent
 * 
 * Spine animation event
*/
export declare class SpineEvent extends UIEvent {
	/** The event type */
	readonly type: SpineEventType;
}

/**
 * @interface SpineEventData
 * 
 * Spine animation event description data
*/
export interface SpineEventData {
	/** The event name */
	readonly name: string;
	/** The event time (seconds) */
	readonly time: number;
	/** The event integer payload */
	readonly intValue: number;
	/** The event float payload */
	readonly floatValue: number;
	/** The event string payload */
	readonly stringValue: string;
	/** The event audio path */
	readonly audioPath: string;
	/** The event volume */
	readonly volume: number;
	/** The event audio balance */
	readonly balance: number;
}

/**
 * @class SpineKeyEvent
 * @extends SpineEvent
 * 
 * Spine animation key event
*/
export declare class SpineKeyEvent extends SpineEvent {
	/** Static event description */
	readonly staticData: SpineEventData;
	/** Trigger time (seconds) */
	readonly time: number;
	/** Integer payload */
	readonly intValue: number;
	/** Float payload */
	readonly floatValue: number;
	/** String payload */
	readonly stringValue: string;
	/** Volume */
	readonly volume: number;
	/** Audio balance */
	readonly balance: number;
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
