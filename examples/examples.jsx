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

import { Scroll, Div, Clip, Text } from 'ngui';
import { Navbutton, Mynavpage } from './public';
import './components';
import './input';
import './icons';
import './media';
import './action';
import './fs';
import './http';
import './zlib';
import './storage';

var resolve = require.resolve;

export const vx = ()=>(

	<Mynavpage title="Examples" source=resolve(__filename)>

		<Scroll width="full" height="full" bounceLock=0>

			<Text class="category_title" value="GUI." />
			<Clip class="category">
				<Navbutton next=components.vx id="btn0">Components</Navbutton>
				<Navbutton next=media.vx>Multi-Media</Navbutton>
				<Navbutton next=input.vx>Input</Navbutton>
				<Navbutton next=icons.vx>Icons</Navbutton>
				<Navbutton next=action.vx>Action</Navbutton>
			</Clip>
			
			<Text class="category_title" value="Basic util." />
			<Clip class="category">
				<Navbutton next=fs.vx>File System</Navbutton>
				<Navbutton next=http.vx>Http</Navbutton>
				<!--Navbutton next=zlib.vx>Zlib</Navbutton-->
				<Navbutton next=storage.vx>Local Storage</Navbutton>
			</Clip>

			<Div height=15 width="full" />
		</Scroll>

	</Mynavpage>

)

