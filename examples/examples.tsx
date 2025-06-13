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
import { NavButton, Page } from './tool';
import components from './components';
import input from './input';
import icons from './icons';
import action from './action';
import fs from './fs';
import http from './http';
import media from './media';
// import zlib from './zlib';
import storage from './storage';

const resolve = require.resolve;

export default (self: Page)=>{
	self.title = 'Examples';
	self.source = resolve(__filename);

	return (
		<scroll width="match" height="match" bounceLock={false}>

			<text class="category_title" value="." />
			<box class="category">
				<NavButton next={components}>Components</NavButton>
				<NavButton next={media}>Multi-Media</NavButton>
				<NavButton next={input}>Input</NavButton>
				<NavButton next={icons}>Icons</NavButton>
				<NavButton next={action}>Action</NavButton>
			</box>

			<text class="category_title" value="Basic util." />
			<box class="category">
				<NavButton next={fs}>File System</NavButton>
				<NavButton next={http}>Http</NavButton>
				{/* <NavButton next={zlib}>Zlib</NavButton> */}
				<NavButton next={storage}>Local Storage</NavButton>
			</box>

			<box height={15} width="match" />
		</scroll>
	);
}
