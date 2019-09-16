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

import { Div, Button, Input } from 'ngui';
import 'ngui/util';
import 'ngui/http';
import { alert } from 'ngui/dialog';
import { Mynavpage } from './public';

var resolve = require.resolve;

function url(evt) {
	return evt.sender.owner.IDs.input.value;
}

function Get(evt) {
	http.get(url(evt), function({ data }) {
		var content = data.toString('utf-8');
		alert(content.substr(0, 200).trim() + '...');
	}.catch(function(err) {
		alert(err.message);
	}));
}

function Post(evt) {
	http.post(url(evt), 'post data', function({ data }) {
		alert(data.toString('utf-8').substr(0, 200).trim() + '...');
	}.catch(function(err) {
		alert(err.message);
	}));
}

function GetSync(evt) {
	try {
		alert(http.getSync(url(evt)).toString('utf-8').substr(0, 200).trim() + '...');
	} catch (err) {
		alert(err.message);
	}
}

function PostSync(evt) {
	try {
		alert(http.postSync(url(evt), 'post data').toString('utf-8').substr(0, 200).trim() + '...');
	} catch (err) {
		alert(err.message);
	}
}

function keyenter(evt) {
	evt.sender.blur();
}

//console.log('-------------', String(util.garbage_collection), typeof util.garbage_collection);

export const vx = ()=>(
	<Mynavpage title="Http" source=resolve(__filename)>
		<Div width="full">
			<Input class="input" id="input" 
				placeholder="Please enter http url .." 
				value="https://github.com/"
				//value="http://192.168.1.11:1026/Tools/test_timeout?1"
				returnType="done" onKeyEnter=keyenter />
			<Button class="long_btn" onClick=Get>Get</Button>
			<Button class="long_btn" onClick=Post>Post</Button>
			<Button class="long_btn" onClick=GetSync>GetSync</Button>
			<Button class="long_btn" onClick=PostSync>PostSync</Button>
			<Button class="long_btn" onClick=util.garbageCollection>GC</Button>
			<Button class="long_btn" onClick=util.exit>Exit</Button>
		</Div>
	</Mynavpage>
)
