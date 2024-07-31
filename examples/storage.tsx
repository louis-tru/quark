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
import * as storage from 'quark/storage';
import { alert } from 'quark/dialog';
import { Page } from './tool';
import {KeyEvent,ClickEvent} from 'quark/event';

const resolve = require.resolve;
const key = 'test';

function keyenter(evt: KeyEvent) {
	evt.sender.blur();
}

function Get(evt: ClickEvent) {
	var val = storage.get(key);
	if ( val ) {
		alert(evt.origin.window, storage.get(key));
	} else {
		alert(evt.origin.window, 'No local storage data！');
	}
}

function Set(evt: ClickEvent) {
	storage.set(key, (evt.origin.prev!.prev! as Input).value);
	alert(evt.origin.window, 'Save local data OK.');
}

function Del(evt: ClickEvent) {
	storage.remove(key);
	alert(evt.origin.window, 'Delete local data OK.');
}

function Clear(evt: ClickEvent) {
	storage.clear();
	alert(evt.origin.window, 'Delete All local data OK.');
}

export default (self: Page)=>{
	self.title = 'Local Storage';
	self.source = resolve(__filename);
	return (
		<box width="match">
			<input class="input" ref="input"
				placeholder="Please enter value .."
				value="Hello."
				returnType="done" onKeyEnter={keyenter} />
			<button class="long_btn" onClick={Get} value="Get" />
			<button class="long_btn" onClick={Set} value="Set" />
			<button class="long_btn" onClick={Del} value="Del" />
			<button class="long_btn" onClick={Clear} value="Clear" />
		</box>
	);
}