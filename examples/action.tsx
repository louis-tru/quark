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

import {
	Hybrid, Text, Button, Image, Indep, Clip, _CVD
} from 'ngui';
import { HighlightedStatus, GUIHighlightedEvent, GUIClickEvent } from 'ngui/event';
import { Toolbar, NavPage } from 'ngui/nav';
import { Mynavpage } from './public';
import review_vx from './review';

var resolve = require.resolve;

function view_code(evt: GUIClickEvent) {
	(evt.sender.owner as NavPage).collection.push(review_vx(), true);
}

function highlighted(evt: GUIHighlightedEvent) {
	var owner = evt.sender.ownerAs();
	var img1 = owner.find('img1');
	var img2 = owner.find('img2');
	var speed = 1;
	if ( evt.status == HighlightedStatus.HIGHLIGHTED_DOWN ) {
		speed = img1 === evt.sender ? 2 : 0.5;
	}
	img1.actionAs().speed = speed;
	img2.actionAs().speed = speed;
}

const toolbar_vx = ()=>(
	<Toolbar backgroundColor="#333">
		<Hybrid textAlign="center" width="full" height="full">
			<Button onClick={view_code}>
				<Text class="toolbar_btn" textColor="#fff" value="\ue9ab" />
			</Button>
		</Hybrid>
	</Toolbar>
)

export default ()=>(
	<Mynavpage 
		navbar={<Toolbar backgroundColor="#333" backTextColor="#fff" titleTextColor="#fff" />}
		toolbar={toolbar_vx()}
		backgroundColor="#333"
		title="Action" source={resolve(__filename)}>
		<Clip width="full" height="full">
			<Indep width={600} alignX="center" alignY="center" y={-15} opacity={0.5}>
				<Image onHighlighted={highlighted} id="img1" src={resolve('./gear0.png')}
					marginLeft="auto" marginRight="auto" 
					y={56} width={600} origin="300 300"
					action={{
						keyframe:[
							{ rotateZ: 0, time:0, curve:'linear' }, 
							{ rotateZ: -360, time: 4000, curve:'linear' },
						],
						loop: 1e8,
						playing: true,
					}}
				/>
				<Image onHighlighted={highlighted} id="img2" src={resolve('./gear1.png')}
					marginLeft="auto" 
					marginRight="auto"
					width={361} 
					origin="180.5 180.5"
					action={{
						keyframe: [
							{ rotateZ: 22.5, time:0, curve:'linear' }, 
							{ rotateZ: 22.5 + 360, time: 2000, curve:'linear' },
						],
						loop: 1e8,
						playing: true,
					}}
				/>
			</Indep>
		</Clip>
	</Mynavpage>
)
