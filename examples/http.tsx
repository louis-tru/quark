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

import { _CVD, Input } from 'quark';
import util from 'quark/util';
import * as http from 'quark/http';
import { alert } from 'quark/dialog';
import { Page } from './tool';
import { ClickEvent, KeyEvent } from 'quark/event';
import * as buffer from 'quark/buffer';

const resolve = require.resolve;

export default (self: Page)=>{
	self.title = 'Http';
	self.source = resolve(__filename);

	function url(evt: ClickEvent) {
		return self.asRef<Input>('input').value;
	}

	function Get(e: ClickEvent) {
		http.get(url(e)).then(function({ data }) {
			var content = buffer.toString(data, 'utf8');
			alert(e.origin.window, content.substring(0, 200).trim() + '...');
		}).catch(function(err) {
			alert(e.origin.window, err.message);
		});
	}

	function Post(e: ClickEvent) {
		http.post(url(e), 'post data').then(function({ data }) {
			alert(e.origin.window, buffer.toString(data, 'utf8').substring(0, 200).trim() + '...');
		}).catch(function(err) {
			alert(e.origin.window, err.message);
		});
	}

	function GetSync(e: ClickEvent) {
		try {
			alert(e.origin.window, buffer.toString(http.getSync(url(e)), 'utf8').substring(0, 200).trim() + '...');
		} catch (err: any) {
			alert(e.origin.window, err.message);
		}
	}

	function PostSync(e: ClickEvent) {
		try {
			alert(e.origin.window, buffer.toString(http.postSync(url(e), 'post data'), 'utf8').substring(0, 200).trim() + '...');
		} catch (err: any) {
			alert(e.origin.window, err.message);
		}
	}

	function keyenter(evt: KeyEvent) {
		evt.sender.blur();
	}

	return (
		<box width="match">
			<input class="input" ref="input" 
				placeholder="Please enter http url .." 
				value="https://github.com/"
				//value="http://192.168.1.11:1026/Tools/test_timeout?1"
				returnType="done" onKeyEnter={keyenter} />
			<button class="long_btn" onClick={Get} value="Get" />
			<button class="long_btn" onClick={Post} value="Post" />
			<button class="long_btn" onClick={GetSync} value="GetSync" />
			<button class="long_btn" onClick={PostSync} value="PostSync" />
			<button class="long_btn" onClick={util.gc} value="GC" />
			<button class="long_btn" onClick={e=>util.exit(0)} value="Exit" />
		</box>
	);
}
