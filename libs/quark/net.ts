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
import {userAgent} from './http';
import {fromString,toString} from './buffer';
import {URL} from './path';
import event, {
	EventNoticer, NativeNotification, Notification, Event,
} from './event';

const _net = __binding__('_net');

type SocketEvent<T = void> = Event<Socket, T>; //!<
type WSocketEvent<T = void> = Event<WebSocket, T>; //!<

/**
 * @class NativeSocket
*/
declare class NativeSocket extends Notification<SocketEvent> implements Stream {
	readonly hostname: string; //!<
	readonly port: Uint; //!<
	readonly ip: string; //!<
	readonly ipv6: string; //!<
	readonly isOpen: boolean; //!<
	readonly isPause: boolean; //!<
	setKeepAlive(keep_alive: boolean, keep_idle?: Uint): void; //!<
	setNoDelay(noDelay?: boolean): void; //!<
	setTimeout(time: Uint): void; //!<
	connect(): void; //!<
	close(): void; //!<
	pause(): void; //!<
	resume(): void; //!<
	write(data: string | Uint8Array, flag?: Int, cb?: Function): void; //!<
	disableSslVerify(disable?: boolean): void; //!<
	constructor(hostname: string, port: Uint, isSSL?: boolean); //!<
}

/**
 * @class Socket
 * @extends NativeSocket
*/
export class Socket extends (_net.Socket as typeof NativeSocket) {
	/**
	 * Trigger when opened connection
	*/
	@event readonly onOpen: EventNoticer<SocketEvent>;
	/**
	 * Trigger when closed connection
	*/
	@event readonly onClose: EventNoticer<SocketEvent>;
	/**
	 * Trigger when an error occurs
	*/
	@event readonly onError: EventNoticer<SocketEvent<Error>>;
	/**
	 * Trigger when accept part of body data, and will be continuous
	*/
	@event readonly onData: EventNoticer<SocketEvent<Uint8Array>>;
	/**
	 * Trigger when write data to the server
	*/
	@event readonly onWrite: EventNoticer<SocketEvent<Int>>;
	/**
	 * Trigger when a timeout of connection
	*/
	@event readonly onTimeout: EventNoticer<SocketEvent>;

	/**
	 * @method write(data,flag?)
	*/
	write(data: string | Uint8Array, flag?: Int): Promise<void> {
		return new Promise((resolve,reject)=>{
			super.write(data, flag, function(err?: Error) {
				err ? reject(err): resolve();
			});
		});
	}
}

utils.extendClass(Socket, NativeNotification);


interface SendCallback {
	(err?: Error): void;
}

/**
 * @class WebSocket
*/
export class WebSocket extends Notification<WSocketEvent> implements Stream {
	private _isOpen = false;

	readonly url: URL; //!<
	readonly hostname: string; //!<
	readonly port: Uint; //!<
	readonly isSSL: boolean; //!<
	readonly socket: Socket; //!<

	get ip() { return this.socket.ip } //!<
	get ipv6() { return this.socket.ipv6 } //!<
	get isOpen () { return this._isOpen } //!<
	get isPause() { return this.socket.isPause } //!<

	/**
	 * @param url:string `wss://xxxx.xx/path`
	*/
	constructor(url: string) {
		super();
		this.url = new URL(url);
		this.hostname = this.url.hostname;
		this.isSSL = ['https','wss'].indexOf(this.url.protocol) != -1;
		this.port = Number(this.url.port) || (this.isSSL ? 443: 80);
		this.socket = new (_net.Socket as typeof Socket)(this.hostname, this.port, this.isSSL);
	}
	setKeepAlive(keep_alive: boolean, keep_idle: Uint = 0) {
		this.socket.setKeepAlive(keep_alive, keep_idle);
	}
	setNoDelay(noDelay = true): void {
		this.socket.setNoDelay(noDelay);
	}
	setTimeout(time: Uint) {
		this.socket.setTimeout(time);
	}
	connect() { //!<
		this.socket.connect();
	}
	close() { //!<
		this.socket.close();
	}
	pause() { //!<
		this.socket.pause();
	}
	resume() { //!<
		this.socket.resume();
	}
	write(data: string | Uint8Array) {
		// TODO ...
	}
	disableSslVerify(disable: false) {
		this.socket.disableSslVerify(disable);
	}
}