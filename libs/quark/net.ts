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

import utils from './util';
import {Stream} from './fs';
import event, {
	EventNoticer, NativeNotification, Notification, Event,
} from './event';

const _net = __binding__('_net');

type SocketEvent<T = void> = Event<Socket, T>; //!<

/**
 * @class Socket
*/
export declare class Socket extends Notification<SocketEvent> implements Stream {
	readonly hostname: string; //!<
	readonly port: Uint; //!<
	readonly ip: string; //!<
	readonly ipv6: string; //!<
	readonly isOpen: boolean; //!<
	readonly isPause: boolean; //!<
	/** @event */
	readonly onOpened: EventNoticer<SocketEvent>;
	/** @event */
	readonly onClosed: EventNoticer<SocketEvent>;
	/** @event */
	readonly onError: EventNoticer<SocketEvent<Error>>;
	/** @event */
	readonly onData: EventNoticer<SocketEvent<Uint8Array>>;
	/** @event */
	readonly onWritten: EventNoticer<SocketEvent<Int>>;
	/** @event */
	readonly onTimeout: EventNoticer<SocketEvent>;

	setKeepAlive(keep_alive: boolean, keep_idle?: Uint): void; //!<
	setNoDelay(noDelay?: boolean): void; //!<
	setTimeout(time?: Uint): void; //!<
	connect(): void; //!<
	close(): void; //!<
	pause(): void; //!<
	resume(): void; //!<
	write(data: string | Uint8Array, flag?: Int): void; //!<
	disableSslVerify(disable?: boolean): void; //!<

	constructor(hostname: string, port: Uint, isSSL?: boolean); //!<
}

Object.assign(exports, _net);

class _Socket extends NativeNotification {
	@event onOpened: EventNoticer<SocketEvent>;
	@event onClosed: EventNoticer<SocketEvent>;
	@event onError: EventNoticer<SocketEvent<Error>>;
	@event onData: EventNoticer<SocketEvent<Uint8Array>>;
	@event onWritten: EventNoticer<SocketEvent<Int>>;
	@event onTimeout: EventNoticer<SocketEvent>;
}
utils.extendClass(_net.Socket, _Socket);

/**
 * @class WebSocket
*/
export class WebSocket extends Socket {
	// TODO ...
}