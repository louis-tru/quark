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
import * as buffer from './buffer';
import {URL} from './uri';
import event, {
	EventNoticer, NativeNotification, Notification, Event
} from './event';
import _sha1 from './_sha1';
import errno from './errno';

const _net = __binding__('_net');

type Buffer = buffer.Buffer;
type SocketEvent<T = void> = Event<Socket, T>; //!<
type WSocketEvent<T = void> = Event<WebSocket, T>; //!<

/**
 * @class NativeSocket
*/
declare class NativeSocket extends Notification<SocketEvent> implements Stream {
	readonly hostname: string; //!< Hostname of the WebSocket server
	readonly port: Uint; //!< Port of the WebSocket server
	readonly ip: string; //!< IP address for the remote server
	readonly ipv6: string; //!< IPv6 address for the remote server
	readonly isOpen: boolean; //!< true means connection is open
	readonly isConnecting: boolean; //!< true means socket is connecting
	readonly isPause: boolean; //!< true means socket is paused
	setKeepAlive(keep_alive: boolean, keep_idle?: Uint): void; //!<
	setNoDelay(noDelay?: boolean): void; //!<
	setTimeout(time: Uint): void; //!<
	connect(): void; //!<
	close(): void; //!<
	pause(): void; //!<
	resume(): void; //!<
	write(data: string | Uint8Array, flag?: Int, cb?: Function): void;
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
	 * Trigger when a timeout of connection
	*/
	@event readonly onTimeout: EventNoticer<SocketEvent>;

	/**
	 * @method write(data)
	*/
	write(data: string | Uint8Array): Promise<void> {
		return new Promise((resolve,reject)=>{
			super.write(data, 0, function(err?: Error) {
				err ? reject(err): resolve();
			});
		});
	}
}

utils.extendClass(Socket, NativeNotification);

///////////////////////////////////////////
// WebSocket Conversation Implementation

namespace ws {
	interface State {
		activeFragmentedOperation: any;
		lastFragment: boolean;
		masked: boolean;
		opcode: number;
	}

	type Finish = (mask: null | ArrayLike<number>, data: Buffer) => void;
	type ExpectHandler = (buffer: Buffer) => void;
	type PacketEventNotice<T = void> = EventNoticer<Event<PacketParser, T>>;

	/*
	* Unpacks a buffer to a number.
	*/
	function _unpack(buffer: Buffer) {
		var n = 0;
		for (var i = 0; i < buffer.length; i++) {
			n = i ? (n * 256) + buffer[i]: buffer[i];
		}
		return n;
	}

	function _concat(buffers: Buffer[]) {
		return buffers.length == 1 ? buffers[0]: buffer.concat(buffers);
	}

	// WebSocket Packet Parser
	export class PacketParser extends Notification<Event<PacketParser>> {
		private state: State = {
			activeFragmentedOperation: null,
			lastFragment: false,
			masked: false,
			opcode: 0
		}
		private overflow: Buffer | null = null;
		private expectOffset = 0;
		private expectBuffer: Buffer | null = null;
		private expectHandler: ExpectHandler | null = null;
		private currentMessage: Buffer[] | string | null = null;

		@event readonly onClose: PacketEventNotice;
		@event readonly onText: PacketEventNotice<string>;
		@event readonly onData: PacketEventNotice<Buffer>;
		@event readonly onError: PacketEventNotice<Error>;
		@event readonly onPing: PacketEventNotice<Buffer>;
		@event readonly onPong: PacketEventNotice<Buffer>;

		private opcodeHandlers: { [opcode: string]: (data: Buffer)=>void } = {
			'1': (data: Buffer)=>{ // text
				this._decode(data, (mask, data)=>{
					if (this.currentMessage) {
						this.currentMessage += this.unmask(mask, data).toString('utf8');
					} else {
						this.currentMessage = this.unmask(mask, data).toString('utf8');
					}
					if (this.state.lastFragment) {
						this.onText.trigger(<string>this.currentMessage);
						this.currentMessage = null;
					}
					this.endPacket();
				});
			},
			'2': (data: Buffer)=>{ // binary
				this._decode(data, (mask, data)=>{
					if (this.currentMessage) {
						(<Buffer[]>this.currentMessage).push(this.unmask(mask, data));
					} else {
						this.currentMessage = [this.unmask(mask, data)];
					}
					if (this.state.lastFragment) {
						this.onData.trigger(_concat(<Buffer[]>this.currentMessage));
						this.currentMessage = null;
					}
					this.endPacket();
				});
			},
			// 0x3 - 0x7: Retain, for non-control frame
			'8': (data: Buffer)=>{ // close
				this.onClose.trigger();
				this.reset();
			},
			'9': (data: Buffer)=>{ // ping
				if (this.state.lastFragment == false) {
					this.error('fragmented ping is not supported');
					return;
				}
				this._decode(data, (mask, data)=>{
					this.onPing.trigger(this.unmask(mask, data));
					this.endPacket();
				});
			},
			'10': (data: Buffer)=>{ // pong
				if (this.state.lastFragment == false) {
					this.error('fragmented pong is not supported');
					return;
				}
				this._decode(data, (mask, data)=>{
					this.onPong.trigger(this.unmask(mask, data));
					this.endPacket();
				});
			},
		};

		private _expectData(length: number, finish: Finish) {
			var self = this;
			if (self.state.masked) {
				self.expect('Mask', 4, function(data: Buffer) {
					var mask = data;
					self.expect('Data', length, function (data: Buffer) {
						finish(mask, data);
					});
				});
			}
			else {
				self.expect('Data', length, function(data: Buffer) {
					finish(null, data);
				});
			}
		}

		private _decode(data: Buffer, finish: Finish) {
			var self = this;
			// decode length
			var firstLength = data[1] & 0x7f;
			if (firstLength < 126) {
				self._expectData(firstLength, finish);
			}
			else if (firstLength == 126) {
				self.expect('Length', 2, function (data: Buffer) {
					self._expectData(_unpack(data), finish);
				});
			}
			else if (firstLength == 127) {
				self.expect('Length', 8, function (data: Buffer) {
					if (_unpack(data.slice(0, 4)) != 0) {
						self.error('packets with length spanning more than 32 bit is currently not supported');
						return;
					}
					// var lengthBytes = data.slice(4); // note: cap to 32 bit length
					self._expectData(_unpack(data.slice(4, 8)), finish);
				});
			}
		}

		/*
		* WebSocket PacketParser
		*/
		constructor() {
			super();
			this.expect('Opcode', 2, this.processPacket);
		}

		/*
		* Add new data to the parser.
		*/
		add(data: Buffer) {
			if (this.expectBuffer == null) {
				this.addToOverflow(data);
				return;
			}
			var toRead = Math.min(data.length, this.expectBuffer.length - this.expectOffset);
			data.copy(this.expectBuffer, this.expectOffset, 0, toRead);
			this.expectOffset += toRead;
			if (toRead < data.length) {
				// at this point the overflow buffer shouldn't at all exist
				this.overflow = buffer.alloc(data.length - toRead);
				data.copy(this.overflow, 0, toRead, toRead + this.overflow.length);
			}
			if (this.expectOffset == this.expectBuffer.length) {
				var bufferForHandler = this.expectBuffer;
				this.expectBuffer = null;
				this.expectOffset = 0;
				(this.expectHandler as ExpectHandler).call(this, bufferForHandler);
			}
		}

		/*
		* Adds a piece of data to the overflow.
		*/
		private addToOverflow(data: Buffer) {
			if (this.overflow == null) this.overflow = data;
			else {
				var prevOverflow = this.overflow;
				this.overflow = buffer.alloc(this.overflow.length + data.length);
				prevOverflow.copy(this.overflow, 0);
				data.copy(this.overflow, prevOverflow.length);
			}
		}

		/*
		* Waits for a certain amount of bytes to be available, then fires a callback.
		*/
		private expect(what: string, length: number, handler: ExpectHandler) {
			this.expectBuffer = buffer.alloc(length);
			this.expectOffset = 0;
			this.expectHandler = handler;
			if (this.overflow != null) {
				var toOverflow = this.overflow;
				this.overflow = null;
				this.add(toOverflow);
			}
		}

		/*
		* Start processing a new packet.
		*/
		private processPacket(data: Buffer) {
			if ((data[0] & 0x70) != 0) {
				this.error('reserved fields must be empty');
			}
			this.state.lastFragment = (data[0] & 0x80) == 0x80;
			this.state.masked = (data[1] & 0x80) == 0x80;

			var opcode = data[0] & 0xf;
			if (opcode == 0) {
				// continuation frame
				this.state.opcode = this.state.activeFragmentedOperation;
				if (!(this.state.opcode == 1 || this.state.opcode == 2)) {
					this.error('continuation frame cannot follow current opcode')
					return;
				}
			} else {
				this.state.opcode = opcode;
				if (this.state.lastFragment === false) {
					this.state.activeFragmentedOperation = opcode;
				}
			}
			var handler = this.opcodeHandlers[String(this.state.opcode)];
			if (typeof handler == 'undefined') {
				this.error('no handler for opcode ' + this.state.opcode);
			} else { 
				handler(data);
			}
		}

		/*
		* Endprocessing a packet.
		*/
		private endPacket() {
			this.expectOffset = 0;
			this.expectBuffer = null;
			this.expectHandler = null;
			if (this.state.lastFragment && this.state.opcode == this.state.activeFragmentedOperation) {
				// end current fragmented operation
				this.state.activeFragmentedOperation = null;
			}
			this.state.lastFragment = false;
			this.state.opcode = this.state.activeFragmentedOperation != null ? 
				this.state.activeFragmentedOperation : 0;
			this.state.masked = false;
			this.expect('Opcode', 2, this.processPacket);
		}

		/*
		* Reset the parser state.
		*/
		private reset() {
			this.state = {
				activeFragmentedOperation: null,
				lastFragment: false,
				masked: false,
				opcode: 0
			};
			this.expectOffset = 0;
			this.expectBuffer = null;
			this.expectHandler = null;
			this.overflow = null;
			this.currentMessage = null;
		}

		/*
		* Unmask received data.
		*/
		private unmask(mask: ArrayLike<number> | null, buf: Buffer) {
			if (mask != null) {
				for (var i = 0, ll = buf.length; i < ll; i++) {
					buf[i] ^= mask[i % 4];
				}
			}
			return buf;
		}

		/**
		 * Handles an error
		 */
		private error(reason: any) {
			this.reset();
			this.onError.trigger(Error.new(reason));
			return this;
		}
	}
}

/**
 * Signer interface for signing data.
*/
export interface Signer {
	sign(data: Uint8Array): Promise<Dict<string>>;
}

/**
 * WebSocket client conversation.
 * @class WebSocket
 * @extends Notification<WSocketEvent>
 * @implements Stream
 * @example
 * ```ts
 * import {WebSocket} from 'quark/net';
 * let ws = new WebSocket('wss://example.com/path');
 * ws.onOpen.on(()=>{
 *   console.log('WebSocket opened');
 *   ws.write('Hello, WebSocket!');
 * });
 * ws.onData.on(e=>{
 *   console.log('Received data:', e.data);
 * });
 * ws.onError.on(e=>{
 *   console.error('WebSocket error:', e.data);
 * });
 * ws.onClose.on(()=>{
 *   console.log('WebSocket closed');
 * });
 * ws.connect();
 * ```
*/
export class WebSocket extends Notification<WSocketEvent> implements Stream {
	private _isOpen = false;

	readonly url: URL; //!< URL of the WebSocket server
	readonly hostname: string; //!< Hostname of the WebSocket server
	readonly port: Uint; //!< Port number
	readonly isSSL: boolean; //!< true means wss, false means ws
	readonly socket: Socket; //!< Socket connection implementation
	readonly headers: Dict<string> = { 'User-Agent': userAgent() }; //!< request headers
	readonly responseHeaders: Dict<string> = {}; //!< Response headers

	/**
	 * A signer, use to sign the request
	*/
	signer: Signer | null = null;

	get ip(): string { return this.socket.ip } //!< ip address for the remote server
	get ipv6(): string { return this.socket.ipv6 } //!< ipv6 address for the remote server
	get isOpen (): boolean { return this._isOpen } //!< true means connection is open
	get isPause(): boolean { return this.socket.isPause } //!< true means socket is paused
	get isConnecting(): boolean { return this.socket.isConnecting } //!< true means socket is connecting

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
	 * Trigger when accept ping message
	*/
	@event readonly onPing: EventNoticer<SocketEvent>;

	/**
	 * Trigger when accept pong message
	*/
	@event readonly onPong: EventNoticer<SocketEvent>;

	/**
	 * Trigger when accept buffer data
	*/
	@event readonly onData: EventNoticer<SocketEvent<Buffer>>;

	/**
	 * Trigger when accept text data
	*/
	@event readonly onText: EventNoticer<SocketEvent<string>>;

	/**
	 * Trigger when a timeout of connection
	*/
	@event readonly onTimeout: EventNoticer<SocketEvent>;

	/**
	 * @param url:string `wss://xxxx.xx/path`
	*/
	constructor(url: string) {
		super();
		this.url = new URL(url);
		this.hostname = this.url.hostname;
		this.isSSL = ['https','wss'].indexOf(this.url.protocol) != -1;
		this.port = Number(this.url.port) || (this.isSSL ? 443: 80);
		this.socket = new Socket(this.hostname, this.port, this.isSSL);

		this.socket.onClose.on(()=>
			this._isOpen && (this._isOpen = false, this.triggerClose())
		);
		this.socket.onError.on(e=>this.triggerError(e.data));
		this.socket.onTimeout.on(()=>this.triggerTimeout());
	}

	/**
	 * Setting keep alive for the socket connection
	 * @param keep_alive:boolean enable keep alive
	 * @param keep_idle?:Uint **Default** is 0 means 7200 seconds,
	 * 	the time in milliseconds to wait before sending keep alive packets
	*/
	setKeepAlive(keep_alive: boolean, keep_idle?: Uint) {
		this.socket.setKeepAlive(keep_alive, keep_idle);
	}

	/**
	 * Setting no delay for the socket connection
	 * @param noDelay?:boolean **Default** is true, disable Nagle's algorithm
	*/
	setNoDelay(noDelay = true): void {
		this.socket.setNoDelay(noDelay);
	}

	/**
	 * Setting timeout time for the socket connection
	*/
	setTimeout(time: Uint) {
		this.socket.setTimeout(time);
	}

	/*
	* Handshake for WebSocket.
	* subclass can override this method to implement custom handshake logic
	*/
	protected handshakes(key: number) {
		var accept = this.responseHeaders['sec-websocket-accept'];
		if (accept) {
			let hash = _sha1(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11');
			let skey = hash.toString('base64');
			return skey == accept;
		}
		return false;
	}

	/*
	* Connect to WebSocket server
	*/
	private async connect1() {
		let self = this;
		let key = Date.now();
		let url = self.url;
		let origin = 'localhost';// + self.port;

		let headers = {
			...self.headers,
			'Host': self.hostname,
			'Connection': 'Upgrade',
			'Upgrade': 'websocket',
			'Origin': origin,
			'Sec-Websocket-Origin': origin,
			'Sec-Websocket-Version': '13',
			'Sec-Websocket-Key': key+'',
		};

		if (self.signer) {
			Object.assign(headers, await self.signer.sign(buffer.from(url.path)));
		}

		let headerStr = `GET ${url.path} HTTP/1.1\r\n` + Object.entries(headers).map(([k,v])=>
			`${k[0].toUpperCase()}${k.slice(1)}: ${v}\r\n`
		).join('') + '\r\n';

		let headerBuf: Buffer = buffer.Zero;
		let parser: ws.PacketParser | undefined;

		self.socket.onData.on(e=>{
			if (parser) {
				parser.add(new buffer.Buffer(e.data as Uint8Array));
				return;
			}
			headerBuf = buffer.concat([headerBuf, e.data]);

			let index = headerBuf.findIndex((val,idx,buf)=>{ // find http header end for \r\n\r\n
				if (val == 13)
					if (buf[++idx] == 10)
						if (buf[++idx] == 13)
							return buf[++idx] == 10;
				return false;
			});
			if (index == -1)
				return;

			for (let h of headerBuf.slice(0, index).toString().split('\r\n')) {
				let idx = h.indexOf(': ');
				if (idx != -1) {
					let k = h.slice(0, idx);
					let v = h.slice(idx + 2);
					self.responseHeaders[k.toLowerCase()] = v;
				}
			}

			if (!self.handshakes(key)) {
				self.safeClose();
				return;
			}

			this.socket.onClose.off('-1');
			this._isOpen = true;
			self.triggerOpen();

			parser = new ws.PacketParser();
			parser.onText.on(e=>self.triggerText(e.data));
			parser.onData.on(e=>self.triggerData(e.data));
			parser.onPing.on(e=>self.triggerPing());
			parser.onPong.on(e=> self.triggerPong());
			parser.onClose.on(e=>self.safeClose());
			parser.onError.on(e=>self.onError.trigger(e.data));
			parser.add(headerBuf.slice(index+4)); // skip \r\n\r\n
		}, '-1');

		this.socket.onClose.on(()=>this.triggerError(Error.new(errno.ERR_WS_HANDSHAKE_FAIL)), '-1');
		this.socket.onClose.setLifespan('-1');
		await self.socket.write(headerStr);
	}

	private safeClose() {
		if (this.socket.isConnecting) {
			this.socket.close();
		}
	}

	/**
	 * Connect to the WebSocket server
	*/
	connect() {
		if (!this.socket.isConnecting) {
			this.socket.onOpen.on(()=>this.connect1().catch(console.error), '-1');
			this.socket.onOpen.setLifespan('-1');
			this.socket.connect();
		}
	}

	/**
	 * Close the WebSocket connection
	*/
	close() {
		this.safeClose();
	}

	/**
	 * Pause receiving data from the socket
	*/
	pause() {
		this.socket.pause();
	}

	/**
	 * Resuming receiving data from the socket
	*/
	resume() {
		this.socket.resume();
	}

	/**
	 * Sending the data message.
	 * Frame client-to-server output as a text packet or buffer packet.
	*/
	write(data: string | Uint8Array): Promise<void> {
		utils.assert(this._isOpen, errno.ERR_NOT_OPEN_CONNECTION);

		let opcode = 0x81; // text 0x81 | buffer 0x82 | close 0x88 | ping 0x89

		if (data instanceof Uint8Array) { // send binary message
			opcode = 0x82;
		} else { // send string message
			data = buffer.fromString(data);
		}

		let dataLength = data.length;
		let headerLength = 2;
		let secondByte = dataLength;

		/*
			0   - 125   : 2,	opcode|len|data
			126 - 65535 : 4,	opcode|126|len|len|data
			65536 -     : 10,	opcode|127|len|len|len|len|len|len|len|len|data
		*/
		/*
			opcode:
			0x81: text
			0x82: binary
			0x88: close
			0x89: ping
			0x8a: pong
		*/

		if (dataLength > 65535) {
			headerLength = 10;
			secondByte = 127;
		}
		else if (dataLength > 125) {
			headerLength = 4;
			secondByte = 126;
		}

		let header = buffer.alloc(headerLength);

		header[0] = opcode;
		header[1] = secondByte;

		switch (secondByte) {
			case 126:
				header[2] = dataLength >> 8;
				header[3] = dataLength % 256;
				break;
			case 127:
				let l = dataLength;
				for (let i = 9; i > 1; i--) {
					header[i] = l & 0xff;
					l >>= 8;
				}
		}

		return this.socket.write(buffer.concat([header, data]));
	}

	/**
	 * alias for write
	*/
	send(data: string | Uint8Array): Promise<void> {
		return this.write(data);
	}

	/**
	 * Sending the ping message
	*/
	ping(): Promise<void> {
		utils.assert(this._isOpen, errno.ERR_NOT_OPEN_CONNECTION);
		let header = buffer.alloc(3);
		header[0] = 0x89;
		header[1] = 1; // 1byte
		return this.socket.write(header);
	}

	/*
	 * Sending the pong message
	*/
	private pong() {
		let header = buffer.alloc(3);
		header[0] = 0x8a;
		header[1] = 1; // 1byte
		return this.socket.write(header);
	}

	/**
	 * First send the ping message to the remote client then wait for the pong message
	 * @param timeoutMs? wait for the pong message, default not timeout
	*/
	async test(timeoutMs?: Uint): Promise<void> {
		await this.ping();
		const promise = new Promise<void>((resolve)=>{
			this.onPong.setLifespan(this.onPong.on(()=>resolve()));
		});
		if (timeoutMs)
			await utils.timeout(promise, timeoutMs);
		else
			await promise;
	}

	/**
	 * Setting if is disable ssl verify
	 * @param disable True means disable ssl verify
	*/
	disableSslVerify(disable: boolean) {
		this.socket.disableSslVerify(disable);
	}

	protected triggerOpen() { //!<
		this.onOpen.trigger(void 0);
	}

	protected triggerClose() { //!<
		this.onClose.trigger(void 0);
	}

	protected triggerError(err: Error) { //!<
		console.error('WebSocket', err);
		this.onError.trigger(err);
	}

	protected triggerPing() { //!<
		this.pong().catch(console.warn);
		this.onPing.trigger(void 0);
	}

	protected triggerPong() { //!<
		this.onPong.trigger(void 0);
	}

	protected triggerData(data: Buffer) { //!<
		this.onData.trigger(data);
	}

	protected triggerText(text: string) { //!<
		this.onText.trigger(text);
	}

	protected triggerTimeout() { //!<
		this.onTimeout.trigger(void 0);
	}
}