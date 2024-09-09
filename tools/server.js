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

var util = require('encark/util').default;
var fs = require('encark/fs');
// var service = require('encark/service').default;
var HttpService = require('encark/http_service').HttpService;
var FileStream = require('encark/incoming_form').FileStream;
var start_server = require('qkmake/server').default;

// Tools service
// http://127.0.0.1:1026/Tools/upload_file

class Tools extends HttpService {

	onAcceptFilestream(path, name, type) {
		return new FileStream(path, name, type);
	}

	upload_file() {
		console.log('params', this.params);
		console.log('data', this.data);

		if (this.form) {
			for (let files of Object.values(this.form.files)) {
				for (let file of files) {
					if ( file.pathname ) {
						fs.renameSync(file.pathname, this.server.temp + '/' + file.filename);
					}
				}
			}
		}

		console.log('\nrequest headers:\n', this.request.headers);
		console.log('\n---------------------------', this.form ? 'form ok' : '', '\n');

		this.cookie.set('Hello', 'Hello', new Date(2088, 1, 1), '/');
		this.cookie.set('mark', Date.now(), new Date(2120, 1, 1), '/');

		this.returnHtml(
			'<!doctype html>\
			<html>\
			<body>\
				<form method="post" action="/Tools/upload_file" enctype="multipart/form-data"> \
					<h2>upload file</h2>\
					<input type="file" name="upload" multiple="" />\
					<input type="submit" name="submit" value="send" />\
				</form>\
			</body>\
			</html>'
		);
	}

	async test_timeout() {
		await util.sleep(5e3);
		this.returnHtml('OK');
	}

	test() {
		console.log(this.data, this.request.headers);
		this.response.setHeader('Access-Control-Allow-Origin', '*');
		return 'test_cross_domain ok';
	}
}

start_server({
	// remoteLog: 'http://192.168.1.124:1026/',
	server: {
		router:[{
			match: '/Tools/{action}',
			service	: 'Tools',
		}],
		// root: '/Users/louis'
	},
}).setService('Tools', Tools);
