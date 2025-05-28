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

import { _CVD } from 'quark';
import { Page } from './tool';
import * as dialog from 'quark/dialog';
import { ClickEvent } from 'quark/event';

const resolve = require.resolve;

function alert(e: ClickEvent) {
	dialog.alert(e.origin.window, 'Hello alert.');
}

function confirm(e: ClickEvent) {
	const win = e.origin.window;
	dialog.confirm(win, 'Hello Confirm.', (ok)=>{
		if ( ok )
			dialog.alert(win, 'OK');
	});
}

function prompt(e: ClickEvent) {
	const win = e.origin.window;
	dialog.prompt(win, 'Hello Prompt.', (ok, text)=>{
		if ( ok ) {
			dialog.alert(win, text);
		}
	});
}

function custom(e: ClickEvent) {
	const win = e.origin.window;
	dialog.show(
		win,
		'蓝牙已关闭',
		'CarPlay将只能通过USB使用。您希望同时启用无线CarPlay吗？',
		[<text textWeight="bold" value="仅USB"/>, '无线蓝牙'],
		(num)=>{
			if ( num == 0 ) {
				dialog.alert(win, '仅USB');
			} else {
				dialog.alert(win, '无线蓝牙');
			}
		}
	);
}

export default (self: Page)=>{
	self.title = 'Dialog';
	self.source = resolve(__filename);
	return (
		<box width="match">
			<button class="long_btn" onClick={alert} value="Alert" />
			<button class="long_btn" onClick={confirm} value="Confirm" />
			<button class="long_btn" onClick={prompt} value="Prompt" />
			<button class="long_btn" onClick={custom} value="Custom" />
		</box>
	)
}
