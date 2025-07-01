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
 * @callback StreamResponseCallback(stream)void
 * @param stream {StreamResponse}
*/
type StreamResponseCallback = (stream: StreamResponse)=>void;

type HttpEvent<T = void> = Event<HttpClientRequest, T>

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
	 * @method setMethod(method)
	 * @param method {HttpMethod}
	 */
	setMethod(method: HttpMethod): void;

	/**
	 * Setting request url
	 * @method setUrl(url)
	 * @param url {string}
	 */
	setUrl(url: string): void;

	/**
	 * Setting local save path for downloading
	 * @method setSavePath(path)
	 * @param path {string}
	 */
	setSavePath(path: string): void;

	/**
	 * Setting login username
	 * @method setUsername(user)
	 * @param user {string}
	 */
	setUsername(user: string): void;

	/**
	 * Setting login password
	 * @method setPassword(pwd)
	 * @param pwd {string}
	 */
	setPassword(pwd: string): void;

	/**
	 * Disable or enable the cache
	 * @method disableCache(disable)
	 * @param disable {bool}
	 */
	disableCache(disable: boolean): void;

	/**
	 * Disable or enable the cookie
	 * @method disableCookie(disable)
	 * @param disable {bool}
	 */
	disableCookie(disable: boolean): void;

	/**
	 * Disable or enable the sending of cookie
	 * @method disableSendCookie(disable)
	 * @param disable {bool}
	 */
	disableSendCookie(disable: boolean): void;

	/**
	 * Disable or enable the SSL verify
	 * @method disableSslVerify(disable)
	 * @param disable {bool}
	 */
	disableSslVerify(disable: boolean): void;

	/**
	 * @method setKeepAlive(keepAlive)
	 * 
	 * Setting whether to keep active
	 * 
	 * @param keepAlive {bool}
	 */
	setKeepAlive(keepAlive: boolean): void;

	/**
	 * @method setTimeout(timeoutMs)
	 * 
	 * Setting request timeout time
	 * 
	 * @param timeoutMs {uint}
	 */
	setTimeout(timeoutMs: number): void;

	/**
	 * @method setRequestHeader(name,value)
	 * 
	 * Setting request header k/v
	 * 
	 * @param name {string}
	 * @param value {string}
	 */
	setRequestHeader(name: string, value: string): void;

	/**
	 * @method setForm(formName,value)
	 * 
	 * Setting the request form
	 * 
	 * @param formName {string}
	 * @param value {string}
	 */
	setForm(formName: string, value: string): void;

	/**
	 * @method setUploadFile(formName,localPath)
	 * 
	 * Setting the upload file path
	 * 
	 * @param formName {string}
	 * @param localPath {string}
	 */
	setUploadFile(formName: string, localPath: string): void;

	/**
	 * @method clearRequestHeader()
	 * Clear request all of headers
	 */
	clearRequestHeader(): void;

	/**
	 * @method clearFormData()
	 * Clear request all of form data
	 */
	clearFormData(): void;

	/**
	 * @method getResponseHeader(headerName)
	 * 
	 * Getting response header by the name
	 * 
	 * @param headerName {string}
	 * @return {string}
	 */
	getResponseHeader(headerName: string): string;

	/**
	 * @method getAllResponseHeaders()
	 * 
	 * Getting all of response headers
	 * 
	 * @return {Dict<string>}
	 */
	getAllResponseHeaders(): Dict<string>;

	/**
	 * @method send([data])
	 * 
	 * Start send the request
	 * 
	 * @param data? {string|Uint8Array}
	 */
	send(data?: string | Uint8Array): void;

	/**
	 * @method pause()
	 * Pause accepat the response data
	*/
	pause(): void;

	/**
	 * @method resume()
	 * Resume accepat the response data
	*/
	resume(): void;

	/**
	 * @method abort()
	 * 
	 * Abort current request and response
	*/
	abort(): void;
}

/**
 * @class HttpClientRequest
 * @extends NativeHttpClientRequest
 * 
 * For Example:
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
	 * @event onError(data)
	 * 
	 * Trigger when an error occurs
	 * 
	 * @param data {Error}
	*/
	@event readonly onError: EventNoticer<HttpEvent<Error>>;

	/**
	 * @event onWrite()
	 * Trigger when write data to the server
	*/
	@event readonly onWrite: EventNoticer<HttpEvent>;

	/**
	 * @event onHeader()
	 * Trigger when accept headers complete
	*/
	@event readonly onHeader: EventNoticer<HttpEvent>;

	/**
	 * @event onData(data)
	 * 
	 * Trigger when accept part of body data, and will be continuous
	 * 
	 * @param data {Uint8Array}
	*/
	@event readonly onData: EventNoticer<HttpEvent<Uint8Array>>;

	/**
	 * @event onEnd()
	 * Trigger when ended a request and response
	*/
	@event readonly onEnd: EventNoticer<HttpEvent>;

	/**
	 * @event onReadystateChange()
	 * Trigger when changed readystate
	*/
	@event readonly onReadystateChange: EventNoticer<HttpEvent>;

	/**
	 * @event onTimeout()
	 * Trigger when a request timeout
	*/
	@event readonly onTimeout: EventNoticer<HttpEvent>;

	/**
	 * @event onAbort()
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
 * @method request(options)
 * 
 * Sending HTTP request by the options
 * 
 * @param options {RequestOptions}
 * @return {AsyncTask<ResponseData>}
 * 
 * For Examples:
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
 * @method requestStream(options)
 * 
 * Sending HTTP request with stream way by the options
 * 
 * @param options {RequestOptions}
 * @param cb {StreamResponseCallback}
 * @return {AsyncTask<ResponseData>}
 * 
 * For examples:
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
export function requestStream(options: RequestOptions, cb: StreamResponseCallback) {
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
 * @method requestSync(options)
 * 
 * Sending HTTP sync request by the options
 * 
 * @param options {RequestOptions}
 * @return {Uint8Array}
 * 
 * For examples:
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
 * @method download(url,save)
 * 
 * Sending HTTP request with get way and save response data
 * 
 * @param url {string} request target url
 * @param save {string} local save path
 * @return {AsyncTask<ResponseData>}
*/
export function download(url: string, save: string) {
	return request({ url, save });
}

/**
 * @method upload(url,localFilePath)
 * 
 * Uploading local file data with post way
 * 
 * @param url {string} request target url
 * @param localFilePath {string} local file path
 * @return {AsyncTask<ResponseData>}
*/
export function upload(url: string, localFilePath: string) {
	return request({ url, upload: localFilePath, method: HttpMethod.POST, disableCache: true });
}

/**
 * @method get(url)
 * Requesting HTTP data in GET way
 * 
 * @param url {string}
 * @return {AsyncTask<ResponseData>}
*/
export function get(url: string) {
	return request({ url });
}

/**
 * @method getStream(url,cb)
 * Requesting HTTP data in GET way and and using the stream way receiving data
 * 
 * @param url {string}
 * @param cb {StreamResponseCallback}
 * @return {AsyncTask<ResponseData>}
*/
export function getStream(url: string, cb: StreamResponseCallback) {
	return requestStream({ url }, cb);
}

/**
 * @method post(url,data)
 * 
 * Sending data to server in POST way
 * 
 * @param url {string}
 * @param data {string|Uint8Array}
 * @return {AsyncTask<ResponseData>}
*/
export function post(url: string, data: string | Uint8Array) {
	return request({ url, postData: data, method: HttpMethod.POST });
};

/**
 * @method getSync(url)
 * 
 * Syncing request HTTP data in GET way
 * 
 * @param url {string}
 * @return {AsyncTask<ResponseData>}
*/
export function getSync(url: string) {
	return requestSync({ url });
}

/**
 * @method postSync(url,data)
 * 
 * Syncing send data to server by POST
 * 
 * @param url {string}
 * @param data {string|Uint8Array}
 * @return {AsyncTask<ResponseData>}
*/
export function postSync(url: string, data: string | Uint8Array) {
	return requestSync({ url, postData: data, method: HttpMethod.POST });
}

/**
 * @method downloadSync(url,save)
 * 
 * Syncing download data from the url path and save to local path
 * 
 * @param url {string}
 * @param save {string} save to local path
 * @return {AsyncTask<ResponseData>}
*/
export function downloadSync(url: string, save: string) {
	return requestSync({ url, save });
}

/**
 * @method uploadSync(url,localPath)
 * 
 * Syncing upload data to server in the POST way
 * 
 * @param url {string} target server url
 * @param localPath {string} local file path
 * @return {AsyncTask<ResponseData>}
*/
export function uploadSync(url: string, localPath: string) {
	return requestSync({ url, upload: localPath, method: HttpMethod.POST, disableCache: true });
}

/**
 * @method abort(id) abort async task by id
 * @param id {uint}
*/
export declare function abort(id: number): void;

/**
 * @method userAgent()
 * 
 * Getting default http user-agent header
 * 
 * @return {string}
*/
export declare function userAgent(): string;

/**
 * @method setUserAgent(ua)
 * 
 * Setting default http user-agent header
 * 
 * @param ua {string}
 * @return {string}
*/
export declare function setUserAgent(ua: string): void;

/**
 * @method cachePath()
 * 
 * Getting HTTP cache saving path
 * 
 * @return {string}
*/
export declare function cachePath(): string;

/**
 * @method setCachePath(path)
 * 
 * Setting local HTTP cache saving path
 * @param path {string}
*/
export declare function setCachePath(path: string): void;

/**
 * @method maxConnectPoolSize()
 * 
 * Getting network connection pool size
 *
 * @return {uint}
*/
export declare function maxConnectPoolSize(): number;

/**
 * @method setMaxConnectPoolSize(size)
 * 
 * Setting network connection pool size
 * 
 * @param size {uint}
*/
export declare function setMaxConnectPoolSize(size: number): void;

/**
 * To clear all of caches
*/
export declare function clearCache(): void;

/**
 * To clear all of cookies
*/
export declare function clearCookie(): void;