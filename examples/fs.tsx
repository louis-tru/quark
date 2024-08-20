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
import * as fs from 'quark/fs';
import path from 'quark/path';
import { alert } from 'quark/dialog';
import { Page } from './tool';
import { ClickEvent, KeyEvent } from 'quark/event';
import * as buffer from 'quark/buffer';

const resolve = require.resolve;
const filename = path.documents('test.txt');

fs.mkdirsSync(path.dirname(filename));

function ReadFile(e: ClickEvent) {
	console.log('------------', filename);
	fs.readFile(filename).then(function(buf) {
		alert(e.origin.window, buffer.toString(buf, 'utf8'));
	}).catch(err=>{
		alert(e.origin.window, err.message + ', ' + err.code);
	});
}

function ReadFileSync(e: ClickEvent) {
	console.log('------------', filename);
	try {
		var s = fs.readFileSync(filename, 'utf8');
		alert(e.origin.window, s);
	} catch(err: any) {
		alert(e.origin.window, err.message + ', ' + err.code);
	}
}

function Remove(e: ClickEvent) {
	try {
		var a = fs.removeRecursionSync(filename);
		alert(e.origin.window, 'Remove file OK. ' + a);
	} catch (err: any) {
		alert(e.origin.window, err.message + ', ' + err.code);
	}
}

function keyenter(evt: KeyEvent) {
	evt.sender.blur();
}

export default (self: Page)=>{
	self.title = 'File System';
	self.source = resolve(__filename);

	function WriteFile(e: ClickEvent) {
		console.log('------------', filename);
		fs.writeFile(filename, self.refAs<Input>('input').value).then(function() {
			alert(e.origin.window, 'Write file OK.');
		}).catch(err=>{
			alert(e.origin.window, err.message + ', ' + err.code);
		});
	}
	
	function WriteFileSync(e: ClickEvent) {
		try {
			var txt = self.refAs<Input>('input').value;
			var r = fs.writeFileSync(filename, txt);
			console.log(r);
			alert(e.origin.window, 'Write file OK.');
		} catch (err: any) {
			alert(e.origin.window, err.message + ', ' + err.code);
		}
	}

	return (
		<box width="match">
			<input class="input" ref="input" 
				placeholder="Please enter write content.."
				value="Hello."
				returnType="done" onKeyEnter={keyenter} />
			<button class="long_btn" onClick={WriteFile} value="WriteFile" />
			<button class="long_btn" onClick={WriteFileSync} value="WriteFileSync" />
			<button class="long_btn" onClick={ReadFile} value="ReadFile" />
			<button class="long_btn" onClick={ReadFileSync} value="ReadFileSync" />
			<button class="long_btn" onClick={Remove} value="Remove" />
		</box>
	)
}