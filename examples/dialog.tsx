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

import { Div, Button, TextNode, _CVD } from 'quark';
import { Mynavpage } from './public';
import * as dialog from 'quark/dialog';

const resolve = require.resolve;

function alert() {
	dialog.alert('Hello alert.');
}

function confirm() {
	dialog.confirm('Hello Confirm.', (ok)=>{
		if ( ok ) dialog.alert('OK');
	});
}

function prompt() {
	dialog.prompt('Hello Prompt.', (ok, text)=>{
		if ( ok ) {
			dialog.alert(text);
		}
	});
}

function custom() {
	dialog.show('蓝牙已关闭', 
	'CarPlay将只能通过USB使用。您希望同时启用无线CarPlay吗？', 
	[<TextNode textStyle='bold' value="仅USB"/>, '无线蓝牙'], (num)=>{
		if ( num == 0 ) {
			dialog.alert('仅USB');
		} else {
			dialog.alert('无线蓝牙');
		}
	});
}

export default ()=>(
	<Mynavpage title="Dialog" source={resolve(__filename)}>
		<Div width="full">
			<Button class="long_btn" onClick={alert}>Alert</Button>
			<Button class="long_btn" onClick={confirm}>Confirm</Button>
			<Button class="long_btn" onClick={prompt}>Prompt</Button>
			<Button class="long_btn" onClick={custom}>Custom</Button>
		</Div>
	</Mynavpage>
)