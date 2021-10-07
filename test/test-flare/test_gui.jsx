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

import 'flare/sys';
import 'flare/dialog';
import {
	Application,
	Root, Div, Image, Sprite,
	Text, Label, Limit, Hybrid,
	Span, Video, Panel, Button, TextNode, Scroll, flare: gui
} from 'flare';

const action1 = {
	delay: 1,
	loop: 10,
	speed: 1,
	frame: [
		{ time:0, x:100, y:50 },
		{ time:2, x:200, y:100 },
	]
};

const action2 = {
	spawn: [
		action1,
	]
}

const action3 = {
	seq: [ 
		action2, 
		[
			{ time:0, x:100, y:50 },
			{ time:2, x:200, y:100 },
		],
	]
}

const style1 = {
	width: "full",
	height: "80%",
	borderRadius: 50,
	border: '30 #000',
	// border_left_width: 0,
	//backgroundColor: '#fff',
	margin: 20,
	action: [
		{ time:0,     width: "100%", height:"85%",  rotate_z:0,  scale:"1 1",      opacity:1,   curve:'linear' },
		{ time:1e3,   width: "50%",  height:"40%",  rotate_z:-5, scale:"0.8 0.8",  opacity:0.5, curve:'ease' },
		{ time:15e3,  width: "100%", height:"85%",  rotate_z:0,  scale:"1 1",     opacity:1,   curve:'linear' },
		{ time:25e3,  width: "100%", height:"85%",  rotate_z:0,  scale:"1 1",     opacity:1,   curve:'linear' },
	],
};

const style2 = {
	width: 100,
	height: 100,
	scale: '1 1',
	origin: '50 50',
	onClick: e=>{ dialog.alert('click OK, color: ' + e.sender.backgroundColor); },
	borderRadius: 20,
	//normal: { time: 200, scale: '1 1' },
	//hover: { time: 200, scale: '1.2 1.2' },
	//down: { time: 200, scale: '1.1 1.1' },
};

const hao = '你好吗!';

const box = (
	<Div width="100%">
		<Div backgroundColor="#f70" width=100 height=100 receive=1>
			<Label textAlign="left_reverse" textBackgroundColor="#ff0" value=`哈${hao}俣` />
			<Label x=50 y=25 textAlign="center_reverse" textBackgroundColor="#f0f" value=%{hao} />
			<Label x=100 y=50 textAlign="right_reverse" textBackgroundColor="#0ff">你好吗?</Label>
		</Div>
		<Div backgroundColor="#f80" width=100 height=100 receive=1>
			<Label textAlign="left" textColor="#fff">你好吗？</Label>
			<Label x=50 y=25 textAlign="center" textColor="#fff">你好吗？</Label>
			<Label x=100 y=50 textAlign="right" textColor="#fff">你好吗？</Label>
		</Div>
		<Div backgroundColor="#f90" width=100 height=100 receive=1>
			<Limit width=50 height=50 max_width=70 max_height=70 backgroundColor="#f0f">
				<Div backgroundColor="#00f5" width=90 height=90>Limit</Div>
			</Limit>
		</Div>
		<Div backgroundColor="#fb0" width=100 height=100 receive=1 />
			<Div backgroundColor="#fc0" width=100 height=100 receive=1>
			<Sprite width=100 height=100 src=resolve('./res/10440501.jpg')
				ratio="vec2(0.413,0.413)" repeat="mirrored_repeat" start="vec2(50,0)" />
		</Div>
		<Div backgroundColor="#fd0" width=100 height=100 receive=1 />
			<Text backgroundColor="#fff" width=200 height=100
						textBackgroundColor="#0fff"
						textColor="#000"
						textSize=12
						textAlign="center"
						id="text">@@
Touch Code Pro
(9:23)
ABCDABCDAKCDABCD
中国联通
	</Text>
	</Div>
);

const test_scroll = (
	<Scroll style=style1 action.loop=1e8 action.playing=0>
		<Button style=style2 backgroundColor="#f10" />
		<Button style=style2 backgroundColor="#f30" />
		<Button style=style2 backgroundColor="#f50" />
		<Button style=style2 backgroundColor="#f70" />
		<Button style=style2 backgroundColor="#f90" />
		<Button style=style2 backgroundColor="#f0a" />
		<Div backgroundColor="#0f0" width="100%" height=100 />
		<vx:box />
		<vx:box />
		<vx:box />
		<vx:box />
		<vx:box />
	</Scroll>
)

console.log(resolve('./res/bb.jpg'));
console.log(resolve('./res/cc.jpg'));

const div1 = (
	<Div width="full">
		<Div backgroundColor="#0f0" width="20%" height=80 receive=1 />
		<Div backgroundColor="#f30" width="20%" height=80 receive=1 />
		<Div backgroundColor="#f50" width="20%" height=80 />
		<Div backgroundColor="#f60" width="20%" height=80 />
		<Div backgroundColor="#0f0" width="20%" height=80 />
		<Div backgroundColor="#f80" width="20%" height=80 />
		<Div backgroundColor="#f90" width="20%" height=80 />
		<Div backgroundColor="#fa0" width="20%" height=80 />
		<Div backgroundColor="#fb0" width="20%" height=80 />
		<Div backgroundColor="#fc0" width="20%" height=80 />
		<Div backgroundColor="#0f0" width="20%" height=80 />
		<Div backgroundColor="#fe0" width="20%" height=80 />
		<Div backgroundColor="#ff0" width="20%" height=80 />
		<Div backgroundColor="#fe0" width="20%" height=80 />
		<Div backgroundColor="#fd0" width="20%" height=80 />
		<Div backgroundColor="#fc0" width="20%" height=80 />
		<Div backgroundColor="#0f0" width="20%" height=80 />
		<Div backgroundColor="#fc0" width="20%" height=80 />
		<Div backgroundColor="#f90" width="20%" height=80 />
		<Div backgroundColor="#f80" width="20%" height=80 />
		<Div backgroundColor="#f70" width="20%" height=80 />
		<Div backgroundColor="#f60" width="20%" height=80 />
		<Div backgroundColor="#f50" width="20%" height=80 />
		<Div backgroundColor="#f40" width="20%" height=80 />
		<Image width="20%" height=80 src=resolve('./res/bb.jpg') receive=1 />
		<Div backgroundColor="#f30" width="20%" height=80 />
		<Div backgroundColor="#f20" width="20%" height=80 />
		<Div backgroundColor="#f10" width="20%" height=80 />
	</Div>
)

// start gui application
new Application({ multisample: 4 }).start(
	<Root backgroundColor="#0ff" background=`url(${resolve('./res/bb.jpg')}) center center 80%`>
		<vx:test_scroll id="view1" />
		%{ div1 }
	</Root>
).onLoad = function a() {
	
	var r = gui.root;

	for ( var i in sys ) {
		if ( typeof sys[i] == 'function' ) {
			console.log(i, sys[i]());
		}
	}

	// console.log('======================');
	// console.log(gui.root_ctr.find('text').text_backgroundColor);
	// console.log(r);
	// console.log('======= OK start gui =======');

	gui.rootCtr.find("view1").onActionLoop = (evt)=>{
		console.log('action loop:', evt.loop, 'delay:', evt.delay);
	};

	gui.rootCtr.find("view1").onActionKeyframe = function(evt) {
		console.log('action keyframe:', evt.frame, 'loop:', evt.loop, 'delay:', evt.delay);
	};

	r.onClick = function(evt) {/*
		if ( !(evt.origin instanceof Button) ) {
			evt.origin.backgroundColor = '#fff';
			console.log(evt.x, evt.y, evt.data, typeof evt.noticer);
		}*/
	};

	r.onHighlighted = function(evt) {/*
		if ( !(evt.origin instanceof Button) ) {
			if ( evt.status == gui.HIGHLIGHTED_DOWN ) {
				evt.origin.backgroundColor = '#f0f';
			} else {
				evt.origin.backgroundColor = '#ff0';
			}
		}*/
	};

	r.onKeydown = function(evt) {
		console.log('keydown, keycode:', evt.keycode, ', repeat:', evt.repeat);
	};

	r.onKeyup = function(evt) {
		console.log('keyup, keycode:', evt.keycode, ', repeat:', evt.repeat);
	};

};
