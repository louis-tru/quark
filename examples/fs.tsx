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

import { Div, Button, Input, _CVD } from 'quark';
import * as fs from 'quark/fs';
import path from 'quark/path';
import { alert } from 'quark/dialog';
import { Mynavpage } from './public';
import { ClickEvent, KeyEvent } from 'quark/event';
import * as buffer from 'quark/buffer';

var resolve = require.resolve;

const filename = path.documents('test.txt');

fs.mkdirpSync(path.dirname(filename));

function WriteFile(evt: ClickEvent) {
	console.log('------------', filename);
	fs.writeFile(filename, evt.sender.ownerAs().find<Input>('input').value).then(function() {
		alert('Write file OK.');
	}).catch(err=>{
		alert(err.message + ', ' + err.code);
	});
}

function WriteFileSync(evt: ClickEvent) {
	try {
		var txt = evt.sender.ownerAs().find<Input>('input').value;
		var r = fs.writeFileSync(filename, txt);
		console.log(r);
		alert('Write file OK.');
	} catch (err) {
		alert(err.message + ', ' + err.code);
	}
}

function ReadFile(evt: ClickEvent) {
	console.log('------------', filename);
	fs.readFile(filename).then(function(buf) {
		alert(buffer.convertString(buf, 'utf8'));
	}).catch(err=>{
		alert(err.message + ', ' + err.code);
	});
}

function ReadFileSync(evt: ClickEvent) {
	console.log('------------', filename);
	try {
		var s = fs.readFileSync(filename, 'utf8');
		alert(s);
	} catch(err) {
		alert(err.message + ', ' + err.code);
	}
}

function Remove(evt: ClickEvent) {
	try {
		var a = fs.removerSync(filename);
		alert('Remove file OK. ' + a);
	} catch (err) {
		alert(err.message + ', ' + err.code);
	}
}

function keyenter(evt: KeyEvent) {
	evt.sender.blur();
}

export default ()=>(
	<Mynavpage title="File System" source={resolve(__filename)}>
		<Div width="full">
			<Input class="input" id="input" 
				placeholder="Please enter write content.."
				value="Hello."
				returnType="done" onKeyEnter={keyenter} />
			<Button class="long_btn" onClick={WriteFile}>WriteFile</Button>
			<Button class="long_btn" onClick={WriteFileSync}>WriteFileSync</Button>
			<Button class="long_btn" onClick={ReadFile}>ReadFile</Button>
			<Button class="long_btn" onClick={ReadFileSync}>ReadFileSync</Button>
			<Button class="long_btn" onClick={Remove}>Remove</Button>
		</Div>
	</Mynavpage>
)