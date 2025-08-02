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

import {Buffer} from './buffer';
import {WebSocket} from './net';
import jsonb from './jsonb';
import utils from './util';
import event,{Notification,EventNoticer,Event} from './event';
import errno from './errno';

export {WebSocket};

/**
 * @enum PacketType
 * Defines the types of packets used in WSClient communication.
*/
export enum PacketType {
	BIND = 0xf1,
	EVENT = 0xf2,
	CALL = 0xf3,
	CALLBACK = 0xf4,
	PING = 0xf5,
	PONG = 0xf6,
}

/**
 * @interface Packet
 * Represents a packet structure used in WSClient communication.
*/
export interface Packet {
	type?: PacketType;
	service?: string;
	name?: string;
	data?: any;
	error?: Error;
	cb?: number;
	sender?: string;
}

/**
 * @class WSConversation
 * 
 * WSConversation is extended from WebSocket to provide a WSClient connection.
*/
export class WSConversation extends WebSocket {
	private _clients: Dict<WSClient> = {};
	private _default_service = '';
	private _autoReconnect: Uint = 0;
	private _token = '';
	private _isOpenMask = false;
	private _checkIntervalId = 0;

	/**
	 * Session token
	*/
	get token() { return this._token }

	/**
	 * @getset autoReconnect
	 * Reconnect time, default 0 means no auto reconnect
	 */ 
	get autoReconnect(): Uint { return this._autoReconnect }
	set autoReconnect(value: Uint) { this._autoReconnect = Number(value) || 0}

	// Check if it needs to reconnect
	private _autoReconnectCheck(reason: string) {
		if (!this.isOpen && this._isOpenMask) {
			if (this._autoReconnect) { // keep connect
				utils.sleep(this._autoReconnect).then(()=>{
					console.log(`Reconnect ${reason} Clo.. ${this.url.href}`);
					this.connect();
				});
			}
		}
	}

	/*
	 * Binding a WSClient to this conversation.
	 * @throws Error if the client is already bound
	 */
	bind(client: WSClient) {
		var name = client.name;
		if (name in this._clients) {
			throw new Error('No need to repeat binding');
		} else {
			if (!this._default_service) {
				this._default_service = name;
			}
			this._clients[name] = client;
			this.url.setParam('bind_services', Object.keys(this._clients).join(','));
			if (this.isOpen) {
				this.sendPacket({type:PacketType.BIND,service:name}).catch(console.error);
			} else {
				this.connect();
			}
		}
	}

	// override
	protected triggerData(buf: Buffer) {
		let json = jsonb.parse(buf);
		let [type,service,name,data,error,cb,sender] = Array.isArray(json) ? json: [json];
		switch (type) {
			case PacketType.BIND:
				break;
			case PacketType.PING: // ping Extension protocol
				this.triggerPing();
				break;
			case PacketType.PONG: // pong Extension protocol
				this.triggerPong();
				break;
			default:
				let cli = this._clients[service || this._default_service];
				if (cli) {
					cli.receiveMessage({type,service,name,data,error,cb,sender}).catch(console.error);
				} else {
					console.log(`Not find the message handler,discarding the message,${data.service}`);
				}
		}
		super.triggerData(buf);
	}

	// override
	protected triggerOpen(): void {
		let checkNum = 0;
		this._checkIntervalId = setInterval(()=>{
			if (checkNum++ % 4 == 0) {
				this.ping(); // Send ping keep alive packet, interval 60s
			}
			for (const cli of Object.values(this._clients)) {
				(cli as any)._checkTimeout(); // Check timeout for method calls, private method
			}
		}, 15e3); // 15s
		this._token = this.responseHeaders['session-token'] || '';
		super.triggerOpen();
	}

	protected triggerClose(): void {
		clearInterval(this._checkIntervalId); // Clear keep alive interval
		super.triggerClose();
		this._autoReconnectCheck('Close');
	}

	protected triggerError(e: Error): void {
		super.triggerError(e);
		this._autoReconnectCheck('Error');
	}

	/**
	 * @override
	 * Connect to the WebSocket server.
	*/
	connect(): void {
		this._isOpenMask = true;
		super.connect();
	}

	/**
	 * @override
	 * Close the conversation connection.
	*/
	close() {
		this._isOpenMask = false;
		super.close();
	}

	/**
	 * Send a packet to the server.
	 * @param data Packet data to send
	*/
	sendPacket(p: Packet): Promise<void> {
		let bf = jsonb.binaryify([
			p.type, p.service, p.name,p.data, p.error, p.cb, p.sender
		]);
		return this.write(bf);
	}
}

interface CallData extends Packet {
	ok(e: any): void;
	err(e: Error): void;
	timeout?: number;
	cancel?: boolean;
}

const METHOD_CALL_TIMEOUT = 12e4; //!< 120s

/**
 * @class WSCEvent
*/
export class WSCEvent extends Event<WSClient> {
	readonly remoteSender: string; //!<
	/**
	 * @param data? Event data
	 * @param sender? Remote sender id
	*/
	constructor(data?: any, sender?: string) {
		super(data||{});
		this.remoteSender = sender || '';
	}
}

/**
 * @class WSClient
 * 
 * WSClient is Qktool's WebSocket service client.
 *
 * Through it, you can call the service methods of server provided or listen to the service events.
*/
export class WSClient extends Notification<WSCEvent> {
	private _calls: Map<number, CallData> = new Map();
	private _loaded = false;
	private _sends: CallData[] = [];

	/**
	 * Service name on the server
	*/
	readonly name: string;

	/**
	 * Conversation instance
	*/
	readonly conv: WSConversation;

	/**
	 * Triggered when the client is loaded and ready to use.
	*/
	@event readonly onLoad: EventNoticer<WSCEvent>;

	/**
	 * @param serviceName Service name on the server
	 * @param conv Conversation instance
	 */
	constructor(serviceName: string, conv: WSConversation) {
		utils.assert(serviceName, 'Service name is required');
		utils.assert(conv, 'Conversation instance is required');
		super();
		this.name = serviceName;
		this.conv = conv;
		let self = this;

		this.onLoad.on(async e=>{
			utils.assert(e.data.token == this.conv.token, 'Token is required');
			console.log('CLI Load', conv.url.href);
			this._loaded = true;
			let sends = this._sends;
			this._sends = [];
			for (let data of sends) {
				if (!data.cancel) {
					self.sendPacket(data).catch(data.err);
				}
			}
		});

		conv.onClose.on(async e=>{
			self._loaded = false;
			let err = Error.new(errno.ERR_CONNECTION_DISCONNECTION);
			for (let [,handle] of self._calls) {
				handle.cancel = true;
				handle.err(err);
			}
			self._sends = []; // clear calling
		});

		this.conv.bind(this);
	}

	private _checkMethodName(method: string) {
		utils.assert(/^[a-z]/i.test(method), errno.ERR_FORBIDDEN_ACCESS);
	}

	/**
	 * To handle received messages from the server.
	 */
	async receiveMessage(packet: Packet): Promise<void> {
		let self = this;
		let { data, name = '', cb, sender } = packet;

		if (packet.type == PacketType.CALLBACK) {
			let handle = this._calls.get(cb as number);
			if (handle) {
				if (packet.error) { // throw error
					handle.err(Error.new(packet.error));
				} else {
					handle.ok(data);
				}
			}
		} else {
			let r: Packet = {};
			if (packet.type == PacketType.CALL) {
				this._checkMethodName(name);
				// if (utils.debug)
					// console.log('WSClient.CALL', `${self.name}.${name}(${JSON.stringify(data,null,2)})`);
				try {
					r.data = await self.callSelfMethod(name, data||{}, sender||'');
				} catch(e: any) {
					console.error('WSClient.CALL', e);
					r.error = e;
				}
			}
			else if (packet.type == PacketType.EVENT) {
				try {
					this.triggerWithEvent(name, new WSCEvent(data, sender));
				} catch(err) {
					console.error('WSClient.EVENT', err);
				}
			} else {
				return;
			}

			if (cb) {
				self.conv.sendPacket(Object.assign(r, {
					service: self.name,
					type: PacketType.CALLBACK, cb,
				})).catch(console.error); // callback
			}
		}
	}

	// check timeout for method calls
	protected _checkTimeout() {
		var now = Date.now();
		for (var [,handle] of this._calls) {
			if (handle.timeout) {
				if (handle.timeout < now) { // timeouted
					handle.err(Error.new([...errno.ERR_METHOD_CALL_TIMEOUT,
						`Method call timeout, ${this.name}/${handle.name}`]));
					handle.cancel = true;
				}
			}
		}
	}

	/**
	 * The remote service calls a method on the client by the method name.
	 */
	protected callSelfMethod(method: string, data: any, sender: string) {
		if (method in WSClient.prototype)
			throw Error.new(errno.ERR_FORBIDDEN_ACCESS);
		var fn = (<any>this)[method];
		if (typeof fn != 'function')
			throw Error.new(String.format('"{0}" no defined function', method));
		return fn.call(this, data, sender);
	}

	private async sendPacket(data: CallData) {
		if (this._loaded) {
			await this.conv.sendPacket(data);
			delete data.data;
		} else {
			this._sends.push(data);
			this.conv.connect(); // try to connect
		}
		return data;
	}

	private _call<T = any>(type: PacketType, name: string, data?: any, timeout?: number, sender?: string) {
		return utils.promise(async (resolve: (e?: T)=>void, reject)=>{
			timeout = Number(timeout ?? METHOD_CALL_TIMEOUT) || 0;
			let cb = utils.id;
			this._calls.set(cb, await this.sendPacket({
				timeout: timeout ? timeout + Date.now(): 0,
				ok: (e: any)=>(this._calls.delete(cb),resolve(e)),
				err: (e: Error)=>(this._calls.delete(cb),reject(e)),
				service: this.name,
				type, name, data, cb, sender,
			}));
		});
	}

	/**
	 * To call a method on the server.
	 * @param method Method name to call
	 * @param data? Data to send
	 * @param timeout? Timeout in milliseconds, default 120s
	 * @param sender? Sender name, default this service name
	 */
	call<T = any>(method: string, data?: any, timeout?: Uint, sender?: string) {
		return this._call<T>(PacketType.CALL, method, data, timeout, sender);
	}

	/**
	 * To call a method on the server but there will be no callback.
	 * @param method Method name to call
	 * @param data? Data to send
	 * @param sender? Sender name, default this service name
	 */
	async send(method: string, data?: any, sender?: string) {
		await this.sendPacket({
			ok: ()=>{},
			err: ()=>{},
			service: this.name,
			type: PacketType.CALL,
			name: method, data, sender,
		});
	}
}