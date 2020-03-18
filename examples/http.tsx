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

import { Div, Button, Input, _CVD } from 'ngui';
import util from 'ngui/util';
import * as http from 'ngui/http';
import { alert } from 'ngui/dialog';
import { Mynavpage } from './public';
import { GUIClickEvent, GUIKeyEvent } from 'ngui/event';
import * as buffer from 'ngui/buffer';

const resolve = require.resolve;

function url(evt: GUIClickEvent) {
	return evt.sender.ownerAs().find<Input>('input').value;
}

function Get(evt: GUIClickEvent) {
	http.get(url(evt)).then(function({ data }) {
		var content = buffer.convertString(data, 'utf8');
		alert(content.substr(0, 200).trim() + '...');
	}).catch(function(err) {
		alert(err.message);
	});
}

function Post(evt: GUIClickEvent) {
	http.post(url(evt), 'post data').then(function({ data }) {
		alert(buffer.convertString(data, 'utf8').substr(0, 200).trim() + '...');
	}).catch(function(err) {
		alert(err.message);
	});
}

function GetSync(evt: GUIClickEvent) {
	try {
		alert(buffer.convertString(http.getSync(url(evt)), 'utf8').substr(0, 200).trim() + '...');
	} catch (err) {
		alert(err.message);
	}
}

function PostSync(evt: GUIClickEvent) {
	try {
		alert(buffer.convertString(http.postSync(url(evt), 'post data'), 'utf8').substr(0, 200).trim() + '...');
	} catch (err) {
		alert(err.message);
	}
}

function keyenter(evt: GUIKeyEvent) {
	evt.sender.blur();
}

//console.log('-------------', String(util.garbage_collection), typeof util.garbage_collection);

export default ()=>(
	<Mynavpage title="Http" source={resolve(__filename)}>
		<Div width="full">
			<Input class="input" id="input" 
				placeholder="Please enter http url .." 
				value="https://github.com/"
				//value="http://192.168.1.11:1026/Tools/test_timeout?1"
				returnType="done" onKeyEnter={keyenter} />
			<Button class="long_btn" onClick={Get}>Get</Button>
			<Button class="long_btn" onClick={Post}>Post</Button>
			<Button class="long_btn" onClick={GetSync}>GetSync</Button>
			<Button class="long_btn" onClick={PostSync}>PostSync</Button>
			<Button class="long_btn" onClick={util.gc}>GC</Button>
			<Button class="long_btn" onClick={util.exit}>Exit</Button>
		</Div>
	</Mynavpage>
)
