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

import { Div, Text, CSS, atomPixel } from 'langou';
import { Switch, Checkbox } from 'langou/checkbox';
import { Mynavpage } from './public';

var resolve = require.resolve;


CSS({
	'.checkbox_page': {
		width: 'full',
	},
	'.checkbox_page .item': {
		width: 'full',
		borderBottom: `${atomPixel} #ccc`,
	},
	'.checkbox_page .text': {
		width: '100!',
		margin: 13,
	},
})

function change_handle(evt) {
	var checkbox = evt.sender;
	var str = checkbox.selected ? 'YES' : 'NO';
	str += checkbox.disable ? ',Disable' : '';
	checkbox.view.prev.value = str;
}

export const vx = (
	<Mynavpage title="Checkbox" source=resolve(__filename)>
		<Div width="full" class="checkbox_page">
			<Div class="item">
				<Text class="text" value="YES" />
				<Switch onChange=change_handle style={margin:10} selected=1 />
			</Div>
			<Div class="item">
				<Text class="text" value="NO,Disable" />
				<Switch onChange=change_handle style={margin:10} disable=1 />
			</Div>
			<Div class="item">
				<Text class="text" value="NO" />
				<Switch onChange=change_handle style={margin:10} />
			</Div>
			<Div class="item">
				<Text class="text" value="YES" />
				<Checkbox onChange=change_handle style={margin:13} selected=1 />
			</Div>
			<Div class="item">
				<Text class="text" value="YES,Disable" />
				<Checkbox onChange=change_handle style={margin:13} disable=1 selected=1 />
			</Div>
			<Div class="item">
				<Text class="text" value="NO" />
				<Checkbox onChange=change_handle style={margin:13} />
			</Div>
		</Div>
	</Mynavpage>
)
