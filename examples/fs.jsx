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
import 'langou/fs';
import 'langou/path';
import { alert } from 'langou/dialog';
import { Mynavpage } from './public';

var resolve = require.resolve;

const filename = path.documents('test.txt');

fs.mkdirpSync(path.dirname(filename));

function WriteFile(evt) {
	console.log('------------', filename);
	fs.writeFile(filename, evt.sender.owner.find('input').value, function() {
		alert('Write file OK.');
	}.catch(err=>{
		alert(err.message + ', ' + err.code);
	}));
}

function WriteFileSync(evt) {
	try {
		var txt = evt.sender.owner.find('input').value;
		var r = fs.writeFileSync(filename, txt);
		console.log(r);
		alert('Write file OK.');
	} catch (err) {
		alert(err.message + ', ' + err.code);
	}
}

function ReadFile(evt) {
	console.log('------------', filename);
	fs.readFile(filename, function(buf) {
		alert(buf.toString('utf-8'));
	}.catch(err=>{
		alert(err.message + ', ' + err.code);
	}));
}

function Remove(evt) {
	try {
		var a = fs.removerSync(filename);
		alert('Remove file OK. ' + a);
	} catch (err) {
		alert(err.message + ', ' + err.code);
	}
}

function keyenter(evt) {
	evt.sender.blur();
}

export const vx = (
	<Mynavpage title="File System" source=resolve(__filename)>
		<Div width="full">
			<Input class="input" id="input" 
				placeholder="Please enter write content.."
				value="Hello."
				returnType="done" onKeyEnter=keyenter />
			<Button class="long_btn" onClick=WriteFile>WriteFile</Button>
			<Button class="long_btn" onClick=WriteFileSync>WriteFileSync</Button>
			<Button class="long_btn" onClick=ReadFile>ReadFile</Button>
			<Button class="long_btn" onClick=ReadFile>ReadFileSync</Button>
			<Button class="long_btn" onClick=Remove>Remove</Button>
		</Div>
	</Mynavpage>
)