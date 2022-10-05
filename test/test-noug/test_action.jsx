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

import { P, M, LOG, VM, VP } from './test'
import { Application, Root, Div, Button, Indep, Image } from 'quark';
import * as action from 'quark/action';
import css from 'quark/css';

new Application().start(
	<Root>
		<Div id="div" width={100} height={100} backgroundColor="#f00" x={150} origin="50 50" />
		<Image id="img" width={100} height={100} />
		<Indep align_y="bottom" alignX="center" y={-30}>
			<Button id="play" textLineHeight={30} backgroundColor="#aaa" margin={2}>@@  Play  </Button>
			<Button id="stop" textLineHeight={30} backgroundColor="#aaa" margin={2}>@@  Stop  </Button>
			<Button id="seek_play" textLineHeight={30} backgroundColor="#aaa" margin={2}>@@  Seek Play  </Button>
			<Button id="seek_stop" textLineHeight={30} backgroundColor="#aaa" margin={2}>@@  Seek Stop  </Button>
			<Button id="clear" textLineHeight={30} backgroundColor="#aaa" margin={2}>@@  Clear  </Button>
		</Indep>
	</Root>
).onload = function() {

	var ctr = this.root.ctr;

	LOG('\nTest Action:\n')

	P(action, 'LINEAR')
	P(action, 'EASE')
	P(action, 'EASE_IN')
	P(action, 'EASE_OUT')
	P(action, 'EASE_IN_OUT')

	var act1 = new action.KeyframeAction();
	var act2 = new action.KeyframeAction();
	var act3 = new action.KeyframeAction();
	var act4 = new action.SpawnAction();
	var act5 = new action.SequenceAction();
	var act6 = new action.KeyframeAction();

	M(act1, 'play')
	M(act1, 'stop')
	M(act1, 'seek', [1000])
	M(act1, 'seek_play', [2000])
	M(act1, 'seek_stop', [3000])
	M(act1, 'clear')
	P(act1, 'duration')
	P(act1, 'parent')
	P(act1, 'loop')
	M(act1, 'play')
	P(act1, 'delay')
	P(act1, 'delayed')
	P(act1, 'speed')
	P(act1, 'playing')
	//
	M(act1, 'add', [{ time:0,			x: 150, y: 0 }])
	M(act1, 'add', [{ time:4000, 	x: 300, y: 200 }])
	M(act1, 'add', [{ time:8000, 	x: 150, y: 400 }])
	//
	M(act2, 'add', [{ time:0,			x: 150, y: 400 }])
	M(act2, 'add', [{ time:4000, 	x: 0, 	y: 200 }])
	M(act2, 'add', [{ time:8000, 	x: 150, y: 0 }])
	//
	M(act3, 'add', [{ time:0,			background_color: '#f00', rotate_z: 0, curve: action.LINEAR }])
	M(act3, 'add', [{ time:4000, 	background_color: '#00f', rotate_z: 180, curve: action.LINEAR }])
	M(act3, 'add', [{ time:8000, 	background_color: '#f00', rotate_z: 360, curve: action.LINEAR }])
	P(act3, 'loop', 4)
	//
	M(act5, 'append', [act1]);
	M(act5, 'append', [act2]);
	P(act5, 'delay', 1000)
	P(act5, 'loop', 1)

	M(act4, 'append', [act5])
	M(act4, 'append', [act3])
	P(act4, 'loop', 4)
	P(act4, 'speed', 2)

	M(ctr.find('div'), 'set_action', [act4])
	M(act4, 'play');

	M(ctr.find('play').onclick, 'on', [function() {
		M(act4, 'play')
	}])

	M(ctr.find('stop').onclick, 'on', [function() {
		M(act4, 'stop')
	}])

	M(ctr.find('seek_play').onclick, 'on', [function() {
		M(act4, 'seek_play', [2000])
	}])

	M(ctr.find('seek_stop').onclick, 'on', [function() {
		M(act4, 'seek_stop', [6000])
	}])

	M(ctr.find('clear').onclick, 'on', [function() {
		M(act4, 'clear')
	}])

	// 

	M(act6, 'add', [{
		time: 0,
		//
		origin: '50 50',
		width: 100,
		height: 100,
		opacity: 1,
		visible: true,
		src: resolve('res/cc.jpg'),
	}]);

	M(act6, 'add', [{
		time: 4000,
		//
		translate: '0 0',
		scale: '1 1',
		skew: '0 0',
		origin: '0 0',
		margin: 0,
		border: '0 #000',
		border_width: 0,
		border_color: '#000',
		border_radius: 0,
		min_width: 'auto',
		min_height: 'auto',
		start: '0 0',
		ratio: '1 1',
		//
		x: 0,
		y: 0,
		scale_x: 1,
		scale_y: 1,
		skew_x: 0,
		skew_y: 0,
		origin_x: 50,
		origin_y: 50,
		rotate_z: 0,
		opacity: 1,
		visible: true,
		width: 100,
		height: 100,
		margin_left: 'auto',
		margin_top: 'auto',
		margin_right: 'auto',
		margin_bottom: 'auto',
		border_left: '0 #000',
		border_top: '0 #000',
		border_right: '0 #000',
		border_bottom: '0 #000',
		border_left_width: 0,
		border_top_width: 0,
		border_right_width: 0,
		border_bottom_width: 0,
		border_left_color: '#000',
		border_top_color: '#000',
		border_right_color: '#000',
		border_bottom_color: '#000',
		border_radius_left_top: 0,
		border_radius_right_top: 0,
		border_radius_right_bottom: 0,
		border_radius_left_bottom: 0,
		background_color: '#000',
		newline: false,
		content_align: 'right',
		text_align: 'center',
		max_width: 'full',
		max_height: 'auto',
		start_x: 0,
		start_y: 0,
		ratio_x: 1,
		ratio_y: 1,
		repeat: 'repeat',
		text_background_color: '#ff0',
		text_color: '#f00',
		text_size: 'inherit',
		text_style: 'inherit',
		text_family: 'inherit',
		text_line_height: 'inherit',
		text_shadow: '2 2 2 #000',
		text_decoration: 'overline',
		text_overflow: 'ellipsis',
		text_white_space: 'no_wrap',
		align_x: 'center',
		align_y: 'bottom',
		shadow: '2 2 2 #f00',
		src: resolve('res/bb.jpg'),
		background_image: resolve('res/bb.jpg'),
	}])

	P(act6, 'first');
	P(act6, 'last');
	P(act6, 'length');
	P(act6, 'position');
	P(act6, 'time');
	M(act6, 'frame', [0]);
	M(act6, 'has_property', [css.PROPERTY_TRANSLATE]);
	M(act6, 'has_property', [css.PROPERTY_X]);
	M(act6, 'match_property', [css.PROPERTY_X]);
	M(ctr.find('img'), 'set_action', [act6])
	M(act6, 'match_property', [css.PROPERTY_X]);
	P(act6, 'playing', true)

	var f = M(act6, 'frame', [1]);

	P(f, 'translate')
	P(f, 'scale')
	P(f, 'skew')
	P(f, 'origin');
	P(f, 'margin');
	P(f, 'border');
	P(f, 'border_width');
	P(f, 'border_color');
	P(f, 'border_radius');
	P(f, 'min_width');
	P(f, 'min_height');
	P(f, 'start');
	P(f, 'ratio');
	P(f, 'width');
	P(f, 'height');
	P(f, 'x');
	P(f, 'y');
	P(f, 'scale_x');
	P(f, 'scale_y');
	P(f, 'skew_x');
	P(f, 'skew_y');
	P(f, 'origin_x');
	P(f, 'origin_y');
	P(f, 'rotate_z');
	P(f, 'opacity');
	P(f, 'visible');
	P(f, 'margin_left');
	P(f, 'margin_top');
	P(f, 'margin_right');
	P(f, 'margin_bottom');
	P(f, 'border_left');
	P(f, 'border_top');
	P(f, 'border_right');
	P(f, 'border_bottom');
	P(f, 'border_left_width');
	P(f, 'border_top_width');
	P(f, 'border_right_width');
	P(f, 'border_bottom_width');
	P(f, 'border_left_color');
	P(f, 'border_top_color');
	P(f, 'border_right_color');
	P(f, 'border_bottom_color');
	P(f, 'border_radius_left_top');
	P(f, 'border_radius_right_top');
	P(f, 'border_radius_right_bottom');
	P(f, 'border_radius_left_bottom');
	P(f, 'background_color');
	P(f, 'newline');
	P(f, 'content_align');
	P(f, 'text_align');
	P(f, 'max_width');
	P(f, 'max_height');
	P(f, 'start_x');
	P(f, 'start_y');
	P(f, 'ratio_x');
	P(f, 'ratio_y');
	P(f, 'repeat');
	P(f, 'text_background_color');
	P(f, 'text_color');
	P(f, 'text_size');
	P(f, 'text_style');
	P(f, 'text_family');
	P(f, 'text_line_height');
	P(f, 'text_shadow');
	P(f, 'text_decoration');
	P(f, 'text_overflow');
	P(f, 'text_white_space');
	P(f, 'align_x');
	P(f, 'align_y');
	P(f, 'shadow');
	P(f, 'src');
	P(f, 'background_image');
};