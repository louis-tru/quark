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

import { Div, Button, Input } from 'langou';
import 'langou/storage';
import { alert } from 'langou/dialog';
import { Mynavpage } from './public';

var resolve = require.resolve;

const key = 'test';

function keyenter(evt) {
	evt.sender.blur();
}

function Get(evt) {
	var val = storage.get(key);
	if ( val ) {
		alert(storage.get(key));
	} else {
		alert('No local storage data！');
	}
}

function Set(evt) {
	storage.set(key, evt.sender.owner.find('input').value);
	alert('Save local data OK.');
}

function Del(evt) {
	storage.del(key);
	alert('Delete local data OK.');
}

function Clear(evt) {
	storage.clear(key);
	alert('Delete All local data OK.');
}

export const vx = (
	<Mynavpage title="Local Storage" source=resolve(__filename)>
		<Div width="full">
			<Input class="input" id="input" 
				placeholder="Please enter value .." 
				value="Hello."
				returnType="done" onKeyEnter=keyenter />
			<Button class="long_btn" onClick=Get>Get</Button>
			<Button class="long_btn" onClick=Set>Set</Button>
			<Button class="long_btn" onClick=Del>Del</Button>
			<Button class="long_btn" onClick=Clear>Clear</Button>
		</Div>
	</Mynavpage>
)