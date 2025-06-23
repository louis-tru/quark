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

import { _CVD, Window, createCss, Button } from 'quark'
import {HighlightedStatus} from 'quark/event'
import util from 'quark/util'

const resolve = require.resolve

export async function test0(win: Window) {
	win.render(
		<box width="match" height="match" backgroundColor="#f00" margin={20} borderRadius={50}></box>
	)
}

export default async function test1(win: Window) {
	createCss({
		'.btn': {
			width: 100,
			height: 100,
			borderRadius: 20,
			align: 'start',
			cursor: "pointer",
		},
		'.btn:normal': { time: 200, opacity: 1 },
		'.btn:hover': { time: 200, opacity: 0.7 },
		'.btn:active': { time: 200, opacity: 0.5 },
		'.box': { width: "20%", height: 80, align: 'start' }
	})

	// console.log(resolve('./res/0.jpg'));
	// console.log(resolve('./res/cc.jpg'));

	win.render(
		<scroll
			style={{
				width: "match",
				height: "match",
				borderRadius: 50,
				borderWidthLeft: 0,
				border: '20 #000',
				backgroundColor: '#fff',
				margin: 20,
			}}
			action={{
				keyframe: [
					{ time:0,    width: "100%", height:"85%",  rotateZ:0,  scale:"1 1",     opacity:1,   curve:'linear' },
					{ time:1e3,  width: "50%",  height:"40%",  rotateZ:-5, scale:"0.8 0.8", opacity:0.5, curve:'ease' },
					{ time:15e3, width: "100%", height:"85%",  rotateZ:0,  scale:"1 1",     opacity:1,   curve:'linear' },
					{ time:25e3, width: "100%", height:"85%",  rotateZ:0,  scale:"1 1",     opacity:1,   curve:'linear' },
				],
				loop: 1e8,
				playing: false,
			}}
			ref="view1"
		>
			<box width="match">
				<button class="btn" backgroundColor="#f10" onClick={e=>util.gc()} />
				<button class="btn" backgroundColor="#f30" />
				<button class="btn" backgroundColor="#f50" />
				<button class="btn" backgroundColor="#f70" />
				<button class="btn" backgroundColor="#f90" />
				<button class="btn" backgroundColor="#f0a" />
			</box>
			<box backgroundColor="#0f0" width="100%" height={100} />
			{Array.from({length:1}).map((_,j)=>
				<box key={j} width="100%">
					<text align="start" backgroundColor="#f70" width={100} height={100}>
						<label textAlign="left" textBackgroundColor="#ff0" value="hello!哈哈 " />
						<label textAlign="center" textBackgroundColor="#f0f" value={`hello!\n`} textWhiteSpace="preLine" />
						<label textAlign="right" textBackgroundColor="#0ff">hello</label>
					</text>
					{/* <box backgroundColor="#f80" width={100} height={100}>
						<label textAlign="left" textColor="#fff">hello</label>
						<label textAlign="center" textColor="#fff">hello</label>
						<label textAlign="right" textColor="#fff">hello</label>
					</box> */}
					<box align="start" backgroundColor="#f90" width={100} height={100}>
						<box backgroundColor="#00f5" width={90} height={90}>Limit</box>
					</box>
					<box align="start" backgroundColor="#fb0" width={100} height={100} />
					<box align="start" backgroundColor="#fd0" width={100} height={100} />
					<text
						backgroundColor="#fff" width={200} height={100}
						textBackgroundColor="#0fff"
						textColor="#000"
						textSize={12}
						textAlign="center"
					>
						Touch Code Pro
						(9:23)
						ABCDABCDAKCDABCD
						China Unicom
					</text>
				</box>
			)}
			<box width="match">
				<box class="box" backgroundColor="#0f0" />
				<box class="box" backgroundColor="#f30" />
				<box class="box" backgroundColor="#f50" />
				<box class="box" backgroundColor="#f60" />
				<box class="box" backgroundColor="#0f0" />
				<box class="box" backgroundColor="#f80" />
				<box class="box" backgroundColor="#f90" />
				<box class="box" backgroundColor="#fa0" />
				<box class="box" backgroundColor="#fb0" />
				<box class="box" backgroundColor="#fc0" />
				<box class="box" backgroundColor="#0f0" />
				<box class="box" backgroundColor="#fe0" />
				<box class="box" backgroundColor="#ff0" />
				<box class="box" backgroundColor="#fe0" />
				<box class="box" backgroundColor="#fd0" />
				<box class="box" backgroundColor="#fc0" />
				<box class="box" backgroundColor="#0f0" />
				<box class="box" backgroundColor="#fc0" />
				<box class="box" backgroundColor="#f90" />
				<box class="box" backgroundColor="#f80" />
				<box class="box" backgroundColor="#f70" />
				<box class="box" backgroundColor="#f60" />
				<box class="box" backgroundColor="#f50" />
				<box class="box" backgroundColor="#f40" />
				<image width="20%" height={80} src={resolve('./res/0.jpg')} />
				<box class="box" backgroundColor="#f30" />
				<box class="box" backgroundColor="#f20" />
				<box class="box" backgroundColor="#f10" />
			</box>
		</scroll>
	)

	console.log(win.root);
	console.log('======= OK start gui =======');

	win.rootCtr.asRef("view1").onActionLoop.on(e=>{
		console.log('action loop:', e.looped, 'delay:', e.delay);
	});

	win.rootCtr.asRef("view1").onActionKeyframe.on(e=>{
		console.log('action keyframe:', e.frame, 'loop:', e.looped, 'delay:', e.delay);
	})

	win.root.onClick.on(e=>{
		if ( !(e.origin instanceof Button) ) {
			// e.origin.style.backgroundColor = '#fff';
			//console.log(e.x, e.y, e.data);
		}
	})

	win.root.onHighlighted.on(e=>{
		if ( !(e.origin instanceof Button) ) {
			if ( e.status == HighlightedStatus.Active ) {
				//e.origin.style.backgroundColor = '#f00';
			} else {
				//e.origin.style.backgroundColor = '#fff';
			}
		}
	})

	win.root.onKeyDown.on(e=>{
		console.log('keydown, keycode:', e.keycode, ', repeat:', e.repeat);
	})

	win.root.onKeyUp.on(e=>{
		console.log('keyup, keycode:', e.keycode, ', repeat:', e.repeat);
	})

}