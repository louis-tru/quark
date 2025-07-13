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

import { Jsx, createCss,mainScreenScale, Text } from 'quark';
import { Stepper } from 'quark/stepper';
import { Page } from './tool';

const px = 1 / mainScreenScale();
const resolve = require.resolve;

createCss({
	'.stepper_page': {
		width: 'match',
	},
	'.stepper_page .item': {
		width: 'match',
		borderBottom: `${px} #ccc`,
	},
	'.stepper_page .text': {
		width: '140!',
		margin: 13,
	},
	'.stepper_page .stepper': {
		margin: 10,
	},
})

function handleChange(self: Page, value: number, ref: string) {
	(self.asRef(ref) as Text).value = String(value);
}

export default (self: Page)=>{
	self.title = 'Stepper';
	self.source = resolve(__filename);
	return (
		<box width="match" class="stepper_page">
			<box class="item">
				<text class="text" value="10" ref="t1" />
				<Stepper onChange={e=>handleChange(self, e, 't1')} class='stepper' initValue={10} />
			</box>
			<box class="item">
				<text class="text" value="6" ref="t2" />
				<Stepper onChange={e=>handleChange(self, e, 't2')} class='stepper' max={10} min={5} initValue={6} />
			</box>
			<box class="item">
				<text class="text" value="0" ref="t3" />
				<Stepper onChange={e=>handleChange(self, e, 't3')} class='stepper' step={0.1} />
			</box>
		</box>
	);
}
