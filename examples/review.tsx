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

import { _CVD, Text, mainScreenScale } from 'quark';
import { Page } from './tool';
import { Navigation, Navbar } from 'quark/nav';
import {reader} from 'quark/fs';

const px = 1 / mainScreenScale();

export const review = ()=>(
	<Page
		title="Source"
		backgroundColor="#333"
		onForeground={function(self: Navigation) {
			const page = self as Page;
			const source = (page.prevPage as Page).source;
			const text = reader.readFileSync(source, 'utf8');
			page.asRef<Text>('text').value = text;
		}}
		navbar={
			<Navbar backgroundColor="#333" backTextColor="#fff" titleTextColor="#fff" />
		}
	>
		<scroll width="match" height="match" bounceLock={false}>
			<text width="match" ref="text" textColor="#fff" textSize={12} margin={5} />
		</scroll>
	</Page>
);

export const toolbar = (self: Page)=>(
	<free width="match" height={30} backgroundColor="#333" align="centerBottom" borderTop={`${px} #000`}>
		<button
			align="centerMiddle"
			onClick={function() {
				self.collection.push(review(), true);
			}}
			class="toolbar_btn" textColor="#fff" value={"\ue9ab"}
		/>
	</free>
)

export default review;