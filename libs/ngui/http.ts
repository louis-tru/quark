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

import utils from './util';
import event, {EventNoticer, NativeNotification, Notification} from './event';

const _http = __requireNgui__('_http');

export declare class HttpClientRequest extends Notification {
	onError: EventNoticer;
	onwrite: EventNoticer;
	onHeader: EventNoticer;
	onData: EventNoticer;
	onEnd: EventNoticer;
	onReadystateChange: EventNoticer;
	onTimeout: EventNoticer;
	onAbort: EventNoticer;

	// JS_SET_CLASS_METHOD(setMethod, set_method);
	// JS_SET_CLASS_METHOD(setUrl, set_url);
	// JS_SET_CLASS_METHOD(setSavePath, set_save_path);
	// JS_SET_CLASS_METHOD(setUsername, set_username);
	// JS_SET_CLASS_METHOD(setPassword, set_password);
	// JS_SET_CLASS_METHOD(disableCache, disable_cache);
	// JS_SET_CLASS_METHOD(disableCookie, disable_cookie);
	// JS_SET_CLASS_METHOD(disableSendCookie, disable_send_cookie);
	// JS_SET_CLASS_METHOD(disableSslVerify, disable_ssl_verify);
	// JS_SET_CLASS_METHOD(setKeepAlive, set_keep_alive);
	// JS_SET_CLASS_METHOD(setTimeout, set_timeout);
	// JS_SET_CLASS_METHOD(setRequestHeader, set_request_header);
	// JS_SET_CLASS_METHOD(setForm, set_form);
	// JS_SET_CLASS_METHOD(setUploadFile, set_upload_file);
	// JS_SET_CLASS_METHOD(clearRequestHeader, clear_request_header);
	// JS_SET_CLASS_METHOD(clearFormData, clear_form_data);
	// JS_SET_CLASS_METHOD(getResponseHeader, get_response_header);
	// JS_SET_CLASS_METHOD(getAllResponseHeaders, get_all_response_headers);
	// JS_SET_CLASS_ACCESSOR(uploadTotal, upload_total);
	// JS_SET_CLASS_ACCESSOR(uploadSize, upload_size);
	// JS_SET_CLASS_ACCESSOR(downloadTotal, download_total);
	// JS_SET_CLASS_ACCESSOR(downloadSize, download_size);
	// JS_SET_CLASS_ACCESSOR(readyState, ready_state);
	// JS_SET_CLASS_ACCESSOR(statusCode, status_code);
	// JS_SET_CLASS_ACCESSOR(url, url);
	// JS_SET_CLASS_ACCESSOR(httpResponseVersion, http_response_version);
	// JS_SET_CLASS_METHOD(send, send);
	// JS_SET_CLASS_METHOD(pause, pause);
	// JS_SET_CLASS_METHOD(resume, resume);
	// JS_SET_CLASS_METHOD(abort, abort);
}

/**
 * @class HttpClientRequestIMPL
 */
class HttpClientRequestIMPL extends _http.NativeHttpClientRequest {
	@event onError: EventNoticer;
	@event onwrite: EventNoticer;
	@event onHeader: EventNoticer;
	@event onData: EventNoticer;
	@event onEnd: EventNoticer;
	@event onReadystateChange: EventNoticer;
	@event onTimeout: EventNoticer;
	@event onAbort: EventNoticer;
}

utils.extendClass(HttpClientRequestIMPL, NativeNotification);

exports.HttpClientRequest = HttpClientRequestIMPL;