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
import {Stream, AsyncTask, StreamResponse} from './fs';
import event, {
	EventNoticer, NativeNotification, Notification, Event,
} from './event';

const _http = __binding__('_http');

Object.assign(exports, {..._http, ...exports});

/**
 * @enum HttpMethod
*/
export enum HttpMethod {
	/** GET way */
	GET,
	/** POST way */
	POST,
	/** HEAD way */
	HEAD,
	/** DELETE way */
	DELETE,
	/** PUT way */
	PUT,
}

/**
 * @enum HttpReadyState
*/
export enum HttpReadyState {
	/** Initial */
	Initial,
	/** Ready */
	Ready,
	/** Sending */
	Sending,
	/** Response */
	Response,
	/** Completed */
	Completed,
}

/**
 * @callback StreamResponseCallback(stream:StreamResponse)void
*/
type StreamResponseCallback = (stream: StreamResponse)=>void;

type HttpEvent<T = void> = Event<HttpClientRequest, T>; //!<

/**
 * @class NativeHttpClientRequest
 * @extends Notification
 * @private
*/
declare class NativeHttpClientRequest extends Notification<HttpEvent> implements Stream {
	/** Total amount of data to be uploaded to the server */
	readonly uploadTotal: number;

	/** The data size has been written and uploaded to the server */
	readonly uploadSize: number;

	/** Total amount of data to be downloaded */
	readonly downloadTotal: number;

	/** Downloaded data volume */
	readonly downloadSize: number;

	/** Request ready state */
	readonly readyState: HttpReadyState;

	/** Response status code */
	readonly statusCode: number;

	/** Request target url */
	readonly url: string;

	/** Response http version */
	readonly httpResponseVersion: string;

	/**
	 * Setting method way at request 
	 */
	setMethod(method: HttpMethod): void;

	/**
	 * Setting request url
	 */
	setUrl(url: string): void;

	/**
	 * Setting local save path for downloading
	 */
	setSavePath(path: string): void;

	/**
	 * Setting login username
	 */
	setUsername(user: string): void;

	/**
	 * Setting login password
	 */
	setPassword(pwd: string): void;

	/**
	 * Disable or enable the cache
	 */
	disableCache(disable: boolean): void;

	/**
	 * Disable or enable the cookie
	 */
	disableCookie(disable: boolean): void;

	/**
	 * Disable or enable the sending of cookie
	 */
	disableSendCookie(disable: boolean): void;

	/**
	 * Disable or enable the SSL verify
	 */
	disableSslVerify(disable: boolean): void;

	/**
	 * Setting whether to keep active
	 */
	setKeepAlive(keepAlive: boolean): void;

	/**
	 * Setting request timeout time
	 */
	setTimeout(timeoutMs: Uint): void;

	/**
	 * Setting request header k/v
	 */
	setRequestHeader(name: string, value: string): void;

	/**
	 * Setting the request form
	 */
	setForm(formName: string, value: string): void;

	/**
	 * Setting the upload file path
	 */
	setUploadFile(formName: string, localPath: string): void;

	/**
	 * Clear request all of headers
	 */
	clearRequestHeader(): void;

	/**
	 * Clear request all of form data
	 */
	clearFormData(): void;

	/**
	 * Getting response header by the name
	 */
	getResponseHeader(headerName: string): string;

	/**
	 * Getting all of response headers
	 */
	getAllResponseHeaders(): Dict<string>;

	/**
	 * Start send the request
	 */
	send(data?: string | Uint8Array): void;

	/**
	 * Pause accepat the response data
	*/
	pause(): void;

	/**
	 * Resume accepat the response data
	*/
	resume(): void;

	/**
	 * Abort current request and response
	*/
	abort(): void;
}

/**
 * @class HttpClientRequest
 * @extends NativeHttpClientRequest
 * @example
 * ```ts
 * import path from 'quark/path'
 * import * as fs from 'quark/fs'
 * var cl = new HttpClientRequest()
 * cl.setUrl('https://www.baidu.com/')
 * cl.setSavePath(path.documents('baidu.html'));
 * cl.onError.on(function(ev) { console.log(ev.data) })
 * // Prints: 
 * // <Buffer 3c 68 74 6d 6c 3e 0d ... >
 * // <Buffer 3c 21 44 4f 43 54 59 ... > 
 * // ...
 * cl.onData.on(function(ev) {
 * 	console.log(ev.data);
 * })
 * cl.onEnd.on(function() {
 * 	// Prints:
 * 	// true
 * 	// 4 200
 * 	console.log(fs.existsSync(path.documents('baidu.html')))
 * 	console.log(cl.readyState, cl.statusCode)
 * })
 * cl.onReadystateChange.on(function() { console.log(cl.readyState, cl.statusCode) })
 * cl.send();
 * 
 * var cl2 = new HttpClientRequest()
 * cl2.setUrl('http://192.168.1.100:1026/Tools/uploadFile')
 * cl2.setMethod(HttpMethod.POST);
 * cl2.setUploadFile('uploadFile', path.resources('util/http.js'))
 * cl2.onEnd.on(function() {
 * 	// Prints: complete
 * 	console.log('complete')
 * })
 * cl2.send();
 * ```
 */
export class HttpClientRequest extends (_http.HttpClientRequest as typeof NativeHttpClientRequest) {

	/**
	 * Trigger when an error occurs
	*/
	@event readonly onError: EventNoticer<HttpEvent<Error>>;

	/**
	 * Trigger when write data to the server
	*/
	@event readonly onWrite: EventNoticer<HttpEvent>;

	/**
	 * Trigger when accept headers complete
	*/
	@event readonly onHeader: EventNoticer<HttpEvent>;

	/**
	 * Trigger when accept part of body data, and will be continuous
	*/
	@event readonly onData: EventNoticer<HttpEvent<Uint8Array>>;

	/**
	 * Trigger when ended a request and response
	*/
	@event readonly onEnd: EventNoticer<HttpEvent>;

	/**
	 * Trigger when changed readystate
	*/
	@event readonly onReadystateChange: EventNoticer<HttpEvent>;

	/**
	 * Trigger when a request timeout
	*/
	@event readonly onTimeout: EventNoticer<HttpEvent>;

	/**
	 * Trigger when a request is actively abort
	*/
	@event readonly onAbort: EventNoticer<HttpEvent>;
}

utils.extendClass(HttpClientRequest, NativeNotification);

/**
 * @interface RequestOptions
*/
export interface RequestOptions {
	/** The target path url to request  */
	url?: string;
	/** The method way to request */
	method?: HttpMethod;
	/** setting custom request headers */
	headers?: Dict<string>;
	/** Non post requests ignore this option */
	postData?: string | Uint8Array;
	/** save body content to local disk */
	save?: string;
	/** upload loacl file */
	upload?: string;
	/** request timeout time, default no timeout "0" */
	timeout?: number;
	/** Is disable ssl verify */
	disableSslVerify?: boolean;
	/** Is disable cache */
	disableCache?: boolean;
	/** Is disable cookie */
	disableCookie?: boolean;
}

/**
 * @interface ResponseData
*/
export interface ResponseData {
	/** response data */
	data: Uint8Array;
	/** http version */
	httpVersion: string;
	/** status code */
	statusCode: number;
	/** response headers */
	responseHeaders: Dict<string>;
}
//!< @end

/**
 * Sending HTTP request by the options
 * 
 * @example
 * ```ts
 * import path from 'quark/path'
 * // uploat file and save body data
 * var opts = {
 * 	url: 'http://192.168.1.100:1026/Tools/uploadFile',
 * 	method: HttpMethod.POST,
 * 	headers: { test: 'test' },
 * 	// postData: 'a=A',
 * 	save: path.documents('uploadFile.html'),
 * 	upload: path.resources('util/http.js'),
 * 	disableSslVerify: false,
 * 	disableCache: true,
 * 	disableCookie: false,
 * };
 * request(opts).then(function(buff) {
 * 	// Prints: <Buffer ...>
 * 	console.log(buff)
 * }).catch(e=>{ 
 * 	// Fail
 * })
 * ```
*/
export function request(options: RequestOptions): AsyncTask<ResponseData> {
	return new AsyncTask<ResponseData>(function(resolve, reject) {
		return _http.request(options, (err?: Error, r?: any)=>err?reject(err):resolve(r));
	});
}

/**
 * Sending HTTP request with stream way by the options
 * 
 * @example
 * 
 * ```ts
 * requestStream({
 * 	url: 'http://192.168.1.100:1026/' 
 * }, (d)=>{
 * 	// Prints: <Buffer ...>
 * 	console.log(d.data) 
 * }).then(function() {
 * 	console.log('Ok') 
 * }).catch(err=>{
 * 	//Fail
 * });
 * ```
*/
export function requestStream(options: RequestOptions, cb: StreamResponseCallback): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject): number {
		return _http.requestStream(options, function(err?: Error, r?: StreamResponse) {
			if (err) {
				reject(err);
			} else {
				var stream = r as StreamResponse;
				cb(stream);
				if (stream.ended) {
					resolve();
				}
			}
		});
	});
}

/**
 * Sending HTTP sync request by the options
 * 
 * @example
 * ```js
 * // Prints: <Buffer ...>
 * try {
 * 	console.log(http.requestSync({ url: 'http://192.168.1.100:1026/' }));
 * } catch(e) {
 * 	//Fail
 * }
 * ```
*/
export function requestSync(options: RequestOptions): Uint8Array {
	return _http.requestSync(options);
}

/**
 * Sending HTTP request with get way and save response data
 * 
 * @param url request target url
 * @param save local save path
*/
export function download(url: string, save: string): AsyncTask<ResponseData> {
	return request({ url, save });
}

/**
 * Uploading local file data with post way
 * 
 * @param url request target url
 * @param localFilePath local file path
*/
export function upload(url: string, localFilePath: string): AsyncTask<ResponseData> {
	return request({ url, upload: localFilePath, method: HttpMethod.POST, disableCache: true });
}

/**
 * Requesting HTTP data in GET way
*/
export function get(url: string): AsyncTask<ResponseData> {
	return request({ url });
}

/**
 * Requesting HTTP data in GET way and and using the stream way receiving data
*/
export function getStream(url: string, cb: StreamResponseCallback): AsyncTask<void> {
	return requestStream({ url }, cb);
}

/**
 * Sending data to server in POST way
*/
export function post(url: string, data: string | Uint8Array): AsyncTask<ResponseData> {
	return request({ url, postData: data, method: HttpMethod.POST });
};

/**
 * Syncing request HTTP data in GET way
*/
export function getSync(url: string): Uint8Array {
	return requestSync({ url });
}

/**
 * Syncing send data to server by POST
*/
export function postSync(url: string, data: string | Uint8Array): Uint8Array {
	return requestSync({ url, postData: data, method: HttpMethod.POST });
}

/**
 * Syncing download data from the url path and save to local path
*/
export function downloadSync(url: string, save: string): Uint8Array {
	return requestSync({ url, save });
}

/**
 * Syncing upload data to server in the POST way
 * 
 * @param url target server url
 * @param localPath local file path
*/
export function uploadSync(url: string, localPath: string): Uint8Array {
	return requestSync({ url, upload: localPath, method: HttpMethod.POST, disableCache: true });
}

/**
 * abort async task by id
*/
export declare function abort(id: Int): void;

/**
 * Getting default http user-agent header
*/
export declare function userAgent(): string;

/**
 * Setting default http user-agent header
*/
export declare function setUserAgent(ua: string): void;

/**
 * Getting HTTP cache saving path
*/
export declare function cachePath(): string;

/**
 * Setting local HTTP cache saving path
*/
export declare function setCachePath(path: string): void;

/**
 * Getting network connection pool size
*/
export declare function maxConnectPoolSize(): Uint;

/**
 * Setting network connection pool size
*/
export declare function setMaxConnectPoolSize(size: Uint): void;

/**
 * To clear all of caches
*/
export declare function clearCache(): void;

/**
 * To clear all of cookies
*/
export declare function clearCookie(): void;