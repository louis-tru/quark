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
import { _CVD, Window } from 'quark'
import {parseColor} from 'quark/types'
import * as action from 'quark/action'

const resolve = require.resolve;

export default async function (win: Window) {
	win.render(
		<box width="match">
			<matrix ref="div" align="start" width={100} height={100} backgroundColor="#f00" origin="auto">
				<box width={50} height={50} align="center" backgroundColor="#ff0" />
			</matrix>
			<image ref="img" align="end" src={resolve('./res/0.jpg')} />
			<matrix width="50!" align="center" y={10} backgroundColor="#00f6">
				<button ref="play" textLineHeight={30} backgroundColor="#aaa" margin={2}>Play</button>
				<button ref="stop" textLineHeight={30} backgroundColor="#aaa" margin={2}>Stop</button>
				<button ref="seek_play" textLineHeight={30} backgroundColor="#aaa" margin={2}>Seek Play</button>
				<button ref="seek_stop" textLineHeight={30} backgroundColor="#aaa" margin={2}>Seek Stop</button>
				<button ref="clear" textLineHeight={30} backgroundColor="#aaa" margin={2}>Clear</button>
			</matrix>
		</box>
	)

	//win.backgroundColor = parseColor('#00f');
	//win.root.backgroundColor = parseColor('#f0f')

	const ctr = win.rootCtr

	LOG('\nTest Action:\n')

	const act1 = new action.KeyframeAction(win)
	Mv(act1, 'play', [])
	Mv(act1, 'stop', [])
	Mv(act1, 'seek', [1000])
	Mv(act1, 'seekPlay', [2000])
	Mv(act1, 'seekStop', [3000])
	Mv(act1, 'clear', [])
	Pv(act1, 'duration', 0)
	Pv(act1, 'loop', 0)
	Mv(act1, 'play', [])
	Pv(act1, 'speed', 3, e=>e.speed=3)
	Pv(act1, 'playing', false)
	//
	Mv(act1, 'add', [{ time:0,			x: 150, y: 0 }])
	Mv(act1, 'add', [{ time:4000, 	x: 300, y: 200 }])
	Mv(act1, 'add', [{ time:8000, 	x: 150, y: 400 }])
	//
	const act2 = new action.KeyframeAction(win)
	Mv(act2, 'add', [{ time:0,			x: 150, y: 400 }])
	Mv(act2, 'add', [{ time:4000, 	x: 0, 	y: 200 }])
	Mv(act2, 'add', [{ time:8000, 	x: 150, y: 0 }])
	// Mv(act2, 'clear', [])
	//
	// SpawnAction
	const act3 = new action.SequenceAction(win)
	Mv(act3, 'append', [act1]);
	Mv(act3, 'append', [act2]);
	Pv(act3, 'loop', 1, e=>e.loop=1)
	//
	const act4 = new action.KeyframeAction(win)
	Mv(act4, 'add', [{ time:0,			backgroundColor: '#f00', rotateZ: 0, curve: 'linear' }])
	Mv(act4, 'add', [{ time:4000, 	backgroundColor: '#00f', rotateZ: 180, curve: 'linear' }])
	Mv(act4, 'add', [{ time:8000, 	backgroundColor: '#f00', rotateZ: 360, curve: 'linear' }])
	Pv(act4, 'loop', 1, e=>e.loop=1)
	//
	// SpawnAction
	const act5 = new action.SpawnAction(win)
	Mv(act5, 'append', [act3])
	Mv(act5, 'append', [act4])
	Pv(act5, 'loop', 1e3, e=>e.loop=1e3)
	Pv(act5, 'speed', 2, e=>e.speed=2)

	Pv(ctr.asRef('div'), 'action', act5, e=>e.action=act5)
	Mv(act5, 'play', []);

	Mv(ctr.asRef('play').onClick, 'on', [function() {
		Mv(act5, 'play', [])
	}])

	Mv(ctr.asRef('stop').onClick, 'on', [function() {
		Mv(act5, 'stop', [])
	}])

	Mv(ctr.asRef('seek_play').onClick, 'on', [function() {
		Mv(act5, 'seekPlay', [2000])
	}])

	Mv(ctr.asRef('seek_stop').onClick, 'on', [function() {
		Mv(act5, 'seekStop', [6000])
	}])

	Mv(ctr.asRef('clear').onClick, 'on', [function() {
		Mv(act5, 'clear', [])
	}])

	// ---------------------------------------------

	const act6 = new action.KeyframeAction(win)
	Mv(act6, 'add', [{
		time: 0,
		origin: 'auto',
		width: 50,
		height: 50,
		opacity: 1,
		visible: true,
		src: resolve('./res/cc.jpg'),
		rotateZ: 0,
		// curve: 'linear',
	}]);

	Mv(act6, 'add', [{
		time: 4000,
		translate: '0 0',
		scale: '1 1',
		skew: '0 0',
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
		originX: 'auto',
		originY: 'auto',
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
		backgroundColor: '#f008',
		align: 'end',
		textAlign: 'center',
		// minWidth: 'auto',
		// minHeight: 'auto',
		// maxWidth: 'match',
		// maxHeight: 'auto',
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
		src: resolve('./res/aa.jpg'),
		background: `image(${resolve('./res/aa.jpg')})`,
	}])

	Pv(act6, 'time', 0);
	Pv(act6, 'frame', 0);
	Pv(act6, 'length', 2);
	Pv(ctr.asRef('img'), 'action', act6, e=>e.action=act6)
	Pv(act6, 'loop', 1e6, e=>e.loop=1e6)
	Pv(act6, 'playing', false)
	Mv(act6, 'play', [])
	Pv(act6, 'playing', true)

	const f = act6[1]
	Pv(f, 'index', 1)
}