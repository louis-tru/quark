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

import { _CVD, Box } from 'quark';
import { NavButton, Page } from './tool';
import { Overlay, Priority } from 'quark/overlay';
import { ClickEvent } from 'quark/event';

const resolve = require.resolve;

function show_overlay(evt: ClickEvent) {
	Overlay.render(
		<Overlay>
			<box>
				<NavButton>Menu A</NavButton>
				<NavButton>Menu B------C</NavButton>
				<NavButton>Menu C</NavButton>
				<NavButton style={{borderWidth:0}}>Menu D</NavButton>
			</box>
		</Overlay>,
		evt.origin.window
	).showOverlayFrom(evt.sender as Box);
}

function show_overlay2(evt: ClickEvent) {
	Overlay.render(
		<Overlay priority={Priority.Left}>
			<box>
				<NavButton>Hello.</NavButton>
				<NavButton>Who are you going to?</NavButton>
				<NavButton style={{borderWidth:0}}>Do I know you?</NavButton>
			</box>
		</Overlay>,
		evt.origin.window
	).showOverlayFrom(evt.sender as Box);
}

function show_overlay3(evt: ClickEvent) {
	Overlay.render(
		<Overlay priority={Priority.Left} backgroundColor="#000">
			<box>
				<NavButton style={{textColor:"#fff"}}>Hello.</NavButton>
				<NavButton style={{textColor:"#fff"}}>Who are you going to?</NavButton>
				<NavButton style={{textColor:"#fff"}}>Do I know you?</NavButton>
				<NavButton style={{textColor:"#fff", borderWidth:0}}>What country are you from?</NavButton>
			</box>
		</Overlay>,
		evt.origin.window
	).showOverlayFrom(evt.sender as Box);
}

export default (self: Page)=>{
	self.title = 'Overlay'
	self.source = resolve(__filename);
	return (
		<free width="match" height="match" padding={[10,0]}>
			<box align="centerTop" width="match">
				<button class="long_btn" onClick={show_overlay}> Show Overlay </button>
			</box>
			<box align="centerBottom" width="match">
				<button class="long_btn" onClick={show_overlay}> Show Overlay </button>
			</box>
			<box align="leftMiddle">
				<button class="long_btn" onClick={show_overlay2}> Show Overlay </button>
			</box>
			<box align="rightMiddle">
				<button class="long_btn" onClick={show_overlay3}> Show Overlay </button>
			</box>
		</free>
	)
}
