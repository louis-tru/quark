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

///<reference path="_ext.ts"/>

import util from './util';
import {TextOptions} from './view';
import {Screen} from './screen';
import {Window} from './window';
import {FontPool} from './font';
import event, {EventNoticer, Notification, NativeNotification, Event} from './event';

const _ui = __binding__('_ui');
let _current: Application | null = null;
type AEvent = Event<Application>; //!<

/**
 * System clipboard interface.
 *
 * Provides access to the OS clipboard for plain text operations.
 * This object is owned by {@link NativeApplication} and cannot be constructed directly.
 *
 * Notes:
 * - Text is shared with other applications through the system clipboard.
 * - Operations are safe to call at any time after the application is initialized.
 * - Only plain text is supported for now.
 */
export declare interface Clipboard {
	/** 
	 * Read text from the system clipboard.
	 * Returns an empty string if no text is available.
	 */
	getText(): string;

	/** 
	 * Write plain text to the system clipboard.
	 * Replaces existing clipboard contents.
	 */
	setText(text: string): void;

	/** 
	 * Returns true if the clipboard currently contains text data.
	 */
	hasText(): boolean;

	/** 
	 * Clears current clipboard contents.
	 */
	clear(): void;
}

/**
 * Global application object.
 *
 * Represents the running Qk runtime instance and provides access to
 * system-level services, global resources and window management.
 *
 * This object is created by the native runtime and cannot be constructed
 * manually. It becomes fully available after the `onLoad` event.
 */
declare class NativeApplication extends Notification<AEvent> {

	/** 
	 * Indicates whether the application has finished initialization.
	 * Becomes `true` after the onLoad lifecycle event.
	 */
	readonly isLoaded: boolean;

	/** 
	 * Screen information and metrics provider.
	 * Exposes display size, scale, and other screen-related properties.
	 */
	readonly screen: Screen;

	/**
	 * Global font pool.
	 * Used for font loading, caching, and reuse across all windows.
	 */
	readonly fontPool: FontPool;

	/**
	 * Currently active (focused) window.
	 * Returns null if no window is focused.
	 */
	readonly activeWindow: Window | null;

	/**
	 * System clipboard instance.
	 *
	 * Provides access to the shared OS clipboard for plain text operations.
	 * The instance is owned and managed by the application lifecycle and is
	 * available once the application has initialized.
	 */
	readonly clipboard: Clipboard;

	/**
	 * Global default text rendering options.
	 *
	 * Shared by all windows and used as the base configuration when creating
	 * text views without explicit text settings.
	 */
	readonly defaultTextOptions: TextOptions;

	/**
	 * List of all currently existing windows.
	 * Includes visible and hidden windows managed by the application.
	 */
	readonly windows: Window[];

	/**
	 * Maximum memory limit for resource caching (in bytes).
	 * Affects textures, images, fonts and other GPU/CPU resources.
	 */
	maxResourceMemoryLimit: number;

	/**
	 * Returns the current amount of memory used by cached resources.
	 */
	usedResourceMemory(): number;

	/**
	 * Clear cached resources.
	 *
	 * @param all If true, aggressively clears all releasable resources.
	 */
	clear(all?: boolean): void;

	/**
	 * Open a URI using the system handler.
	 *
	 * Examples:
	 *  - openURL('https://example.com')
	 *  - openURL('mailto:test@example.com')
	 */
	openURL(url: string): void;

	/**
	 * Open the system email client with pre-filled fields.
	 *
	 * @param recipient Target email address.
	 * @param subject Email subject.
	 * @param body Optional message body.
	 * @param cc Optional CC address.
	 * @param bcc Optional BCC address.
	 */
	sendEmail(
		recipient: string,
		subject: string,
		body?: string,
		cc?: string,
		bcc?: string
	): void;
}

/**
 * @class Application
 * @extends NativeApplication
 * 
 * Application entry object exposed to user space.
 *
 * Extends the native runtime application and adds lifecycle events.
 * Only one instance should exist during the entire process lifetime.
 *
 * This object represents the high-level app container responsible for:
 *  - lifecycle dispatch
 *  - foreground/background state changes
 *  - memory pressure notifications
 */
export class Application extends (_ui.NativeApplication as typeof NativeApplication) {

	/**
	 * Fired once after the application has completed initialization.
	 * Safe point to create windows, load resources and start logic.
	 */
	@event readonly onLoad: EventNoticer<AEvent>;

	/**
	 * Fired before the application is about to terminate.
	 * Use for saving state, releasing external handles, etc.
	 */
	@event readonly onUnload: EventNoticer<AEvent>;

	/**
	 * Fired when the application enters background state.
	 * Typically triggered by OS task switching or minimization.
	 */
	@event readonly onPause: EventNoticer<AEvent>;

	/**
	 * Fired when the application returns to foreground state.
	 * Suitable for restoring rendering, timers and IO tasks.
	 */
	@event readonly onResume: EventNoticer<AEvent>;

	/**
	 * Fired when the system reports memory pressure.
	 *
	 * After this event, the runtime will automatically:
	 *  - call `clear()` to release cached resources
	 *  - trigger JS garbage collection
	 *
	 * Applications can additionally release non-critical data here.
	 */
	@event readonly onMemoryWarning: EventNoticer<AEvent>;

	constructor() {
		super();
		_current = this;
	}
}

util.extendClass(Application, NativeNotification);

/**
 * @default
 */
export default {
	/** Current active application singleton */
	get current(): Application {
		return _current!
	},
};