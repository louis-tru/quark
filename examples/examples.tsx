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

import { Scroll, Div, Clip, Text, _CVD } from 'ngui';
import { Navbutton, Mynavpage } from './public';
import components from './components';
import input from './input';
import icons from './icons';
import media from './media';
import action from './action';
import fs from './fs';
import http from './http';
import zlib from './zlib';
import storage from './storage';

const resolve = require.resolve;

export default ()=>(
	<Mynavpage title="Examples" source={resolve(__filename)}>

		<Scroll width="full" height="full" bounceLock={0}>

			<Text class="category_title" value="GUI." />
			<Clip class="category">
				<Navbutton next={components} id="btn0">Components</Navbutton>
				<Navbutton next={media}>Multi-Media</Navbutton>
				<Navbutton next={input}>Input</Navbutton>
				<Navbutton next={icons}>Icons</Navbutton>
				<Navbutton next={action}>Action</Navbutton>
			</Clip>
			
			<Text class="category_title" value="Basic util." />
			<Clip class="category">
				<Navbutton next={fs}>File System</Navbutton>
				<Navbutton next={http}>Http</Navbutton>
				{/* <Navbutton next={zlib}>Zlib</Navbutton> */}
				<Navbutton next={storage}>Local Storage</Navbutton>
			</Clip>

			<Div height={15} width="full" />
		</Scroll>

	</Mynavpage>
)

