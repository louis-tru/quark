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

import { Jsx } from 'quark';
import { Page } from './tool';
import { ClickEvent } from 'quark/event';

const resolve = require.resolve;

export default (self: Page)=>{
	self.title = 'Input';
	self.source = resolve(__filename);

	function start_input(e: ClickEvent) {
		self.asRef('input1').focus();
	}

	function end_input(e: ClickEvent) {
		e.origin.window.focusView.blur();
	}

	return (
		<box width="match">
			<text margin={10} textBackgroundColor="#000" textColor="#fff" value="Examples Input" />

			<input ref="input0" margin={10}
				width="match"
				height={30}
				backgroundColor="#eee"
				type="phone"
				returnType="next"
				paddingLeft={5}
				borderRadius={8} placeholder="Please enter.." />

			<input ref="input1" margin={10}
				width="match"
				textColor="#fff"
				backgroundColor="#000"
				height={30}
				border="0 #f00"
				borderRadius={0}
				type="decimal"
				textAlign="center"
				placeholder="Please enter.." value="Hello" />
			
			<textarea margin={10}
				width="match"
				height={120}
				textColor="#000"
				border="0 #aaa"
				backgroundColor="#eee"
				borderRadius={8}
				returnType="next"
				placeholder="Please enter.."
				textSize={14}
				textAlign="center" />

			<button class="long_btn" onClick={end_input} value="Done" />
			<button class="long_btn" onClick={start_input} value="Input" />
		</box>
	)
}
