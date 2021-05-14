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

import * as http from 'http';
import path, {URL} from 'somes/path';
import * as querystring from 'querystring';
var remote_log_uri: URL | null = null;

export function remote_log_print_with_post(data: Dict<string>) {
	var uri = remote_log_uri;
	if ( !uri ) return;

	var postData = querystring.stringify(data);

	var opt: Dict = { 
		hostname: uri.hostname,
		method: 'POST',
		path: '/$console/log/',
		headers: {
			'Content-Type': 'application/x-www-form-urlencoded',
			'Content-Length': Buffer.byteLength(postData)
		}
	}
	var port = parseInt(uri.port);
	if ( !isNaN(port) ) {
		opt.port = port;
	}
	var req = http.request(opt, function() { });

	// write data to request body
	req.write(postData);
	req.end();
}

export function remote_log_print(log: string) {
	var uri = remote_log_uri;
	if ( uri ) {
		var url = path.resolve(uri.href, '/$console/log');
		http.get(url + '/' + log, function() { });
	}
}

export function set_remote_log_address(address: string) {
	if ( /^https?:\/\/[^\/]+/.test(String(address)) ) {
		remote_log_uri = new path.URL(String(address));
	} else {
		remote_log_uri = null;
	}
}