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

import { LOG, Pv, Mv } from './tool'
import { _CVD, Application, Window } from 'quark'
import * as action from 'quark/action'
import * as types from 'quark/types'

const resolve = require.resolve;
const app = new Application();
const win = new Window({
	frame: types.newRect(0,0,500,500),
}).activate()

win.render(
	<box>
		<matrix ref="div" width={100} height={100} backgroundColor="#f00" x={150} origin={[50,50]} />
		<image ref="img" width={100} height={100} />
		<matrix align="centerBottom" y={-30}>
			<button ref="play" textLineHeight={30} backgroundColor="#aaa" margin={2}>Play</button>
			<button ref="stop" textLineHeight={30} backgroundColor="#aaa" margin={2}>Stop</button>
			<button ref="seek_play" textLineHeight={30} backgroundColor="#aaa" margin={2}>Seek Play</button>
			<button ref="seek_stop" textLineHeight={30} backgroundColor="#aaa" margin={2}>Seek Stop</button>
			<button ref="clear" textLineHeight={30} backgroundColor="#aaa" margin={2}>Clear</button>
		</matrix>
	</box>
)

const ctr = win.rootCtr

LOG('\nTest Action:\n')

const act1 = new action.KeyframeAction(win)
const act2 = new action.KeyframeAction(win)
const act3 = new action.KeyframeAction(win)
const act4 = new action.SpawnAction(win)
const act5 = new action.SequenceAction(win)
const act6 = new action.KeyframeAction(win)

Mv(act1, 'play', [])
Mv(act1, 'stop', [])
Mv(act1, 'seek', [1000])
Mv(act1, 'seekPlay', [2000])
Mv(act1, 'seekStop', [3000])
Mv(act1, 'clear', [])
Pv(act1, 'duration', 0)
Pv(act1, 'loop', 0)
Mv(act1, 'play', [])
Pv(act1, 'speed', 1)
Pv(act1, 'playing', true)
//
Mv(act1, 'add', [{ time:0,			x: 150, y: 0 }])
Mv(act1, 'add', [{ time:4000, 	x: 300, y: 200 }])
Mv(act1, 'add', [{ time:8000, 	x: 150, y: 400 }])
//
Mv(act2, 'add', [{ time:0,			x: 150, y: 400 }])
Mv(act2, 'add', [{ time:4000, 	x: 0, 	y: 200 }])
Mv(act2, 'add', [{ time:8000, 	x: 150, y: 0 }])
//
Mv(act3, 'add', [{ time:0,			backgroundColor: '#f00', rotateZ: 0, curve: 'linear' }])
Mv(act3, 'add', [{ time:4000, 	backgroundColor: '#00f', rotateZ: 180, curve: 'linear' }])
Mv(act3, 'add', [{ time:8000, 	backgroundColor: '#f00', rotateZ: 360, curve: 'linear' }])
Pv(act3, 'loop', 4)
//
Mv(act5, 'append', [act1]);
Mv(act5, 'append', [act2]);
Pv(act5, 'loop', 1, e=>e.loop=1)

Mv(act4, 'append', [act5])
Mv(act4, 'append', [act3])
Pv(act4, 'loop', 4, e=>e.loop=4)
Pv(act4, 'speed', 2, e=>e.speed=2)

Pv(ctr.refAs('div'), 'action', act4, e=>e.action=act4)
Mv(act4, 'play', []);

Mv(ctr.refAs('div').onClick, 'on', [function() {
	Mv(act4, 'play', [])
}])

Mv(ctr.refAs('stop').onClick, 'on', [function() {
	Mv(act4, 'stop', [])
}])

Mv(ctr.refAs('seek_play').onClick, 'on', [function() {
	Mv(act4, 'seekPlay', [2000])
}])

Mv(ctr.refAs('seek_stop').onClick, 'on', [function() {
	Mv(act4, 'seekStop', [6000])
}])

Mv(ctr.refAs('clear').onClick, 'on', [function() {
	Mv(act4, 'clear', [])
}])

Mv(act6, 'add', [{
	time: 0,
	origin: 50,
	width: 100,
	height: 100,
	opacity: 1,
	visible: true,
	src: resolve('res/cc.jpg'),
}]);

Mv(act6, 'add', [{
	time: 4000,
	translate: '0 0',
	scale: '1 1',
	skew: '0 0',
	origin: 0,
	margin: 0,
	border: '0 #000',
	borderWidth: 0,
	borderColor: '#000',
	borderRadius: 0,
	x: 0,
	y: 0,
	scaleX: 1,
	scaleY: 1,
	skewX: 0,
	skewY: 0,
	originX: 50,
	originY: 50,
	rotateZ: 0,
	opacity: 1,
	visible: true,
	width: 100,
	height: 100,
	marginLeft: 0,
	marginTop: 0,
	marginRight: 0,
	marginBottom: 0,
	borderLeft: '0 #000',
	borderTop: '0 #000',
	borderRight: '0 #000',
	borderBottom: '0 #000',
	borderWidthLeft: 0,
	borderWidthTop: 0,
	borderWidthRight: 0,
	borderWidthBottom: 0,
	borderColorLeft: '#000',
	borderColorTop: '#000',
	borderColorRight: '#000',
	borderColorBottom: '#000',
	borderRadiusLeftTop: 0,
	borderRadiusRightTop: 0,
	borderRadiusRightBottom: 0,
	borderRadiusLeftBottom: 0,
	backgroundColor: '#000',
	align: 'start',
	textAlign: 'center',
	minWidth: 'auto',
	minHeight: 'auto',
	maxWidth: 'match',
	maxHeight: 'auto',
	textBackgroundColor: '#ff0',
	textColor: '#f00',
	textSize: 'inherit',
	textSlant: 'italic',
	textWeight: 'bold',
	textWordBreak: 'breakAll',
	textFamily: 'inherit',
	textLineHeight: 'inherit',
	textShadow: '2 2 2 #000',
	textDecoration: 'overline',
	textOverflow: 'ellipsis',
	textWhiteSpace: 'noWrap',
	boxShadow: '2 2 2 #f00',
	src: resolve('res/bb.jpg'),
	background: resolve('res/bb.jpg'),
}])

Pv(act6, 'time', 0);
Pv(act6, 'frame', 0);
Pv(act6, 'length', 2);
Pv(ctr.refAs('img'), 'action', act6, e=>e.action=act6)
// Pv(act6, 'playing', true)

const f = act6[1]
Pv(f, 'index', 1)