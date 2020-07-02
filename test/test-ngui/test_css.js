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

import { P: F, M, LOG, VM, VP } from './test'
import 'ftr/css'

LOG('\nTest CSS:\n')

F(css, 'PROPERTY_TIME');
F(css, 'PROPERTY_TRANSLATE');
F(css, 'PROPERTY_SCALE');
F(css, 'PROPERTY_SKEW');
F(css, 'PROPERTY_ORIGIN');
F(css, 'PROPERTY_MARGIN');
F(css, 'PROPERTY_BORDER');
F(css, 'PROPERTY_BORDER_WIDTH');
F(css, 'PROPERTY_BORDER_COLOR');
F(css, 'PROPERTY_BORDER_RADIUS');
F(css, 'PROPERTY_MIN_WIDTH');
F(css, 'PROPERTY_MIN_HEIGHT');
F(css, 'PROPERTY_START');
F(css, 'PROPERTY_RATIO');
F(css, 'PROPERTY_X');
F(css, 'PROPERTY_Y');
F(css, 'PROPERTY_SCALE_X');
F(css, 'PROPERTY_SCALE_Y');
F(css, 'PROPERTY_SKEW_X');
F(css, 'PROPERTY_SKEW_Y');
F(css, 'PROPERTY_ROTATE_Z');
F(css, 'PROPERTY_ORIGIN_X');
F(css, 'PROPERTY_ORIGIN_Y');
F(css, 'PROPERTY_OPACITY');
F(css, 'PROPERTY_VISIBLE');
F(css, 'PROPERTY_WIDTH');
F(css, 'PROPERTY_HEIGHT');
F(css, 'PROPERTY_MARGIN_LEFT');
F(css, 'PROPERTY_MARGIN_TOP');
F(css, 'PROPERTY_MARGIN_RIGHT');
F(css, 'PROPERTY_MARGIN_BOTTOM');
F(css, 'PROPERTY_BORDER_LEFT');
F(css, 'PROPERTY_BORDER_TOP');
F(css, 'PROPERTY_BORDER_RIGHT');
F(css, 'PROPERTY_BORDER_BOTTOM');
F(css, 'PROPERTY_BORDER_LEFT_WIDTH');
F(css, 'PROPERTY_BORDER_TOP_WIDTH');
F(css, 'PROPERTY_BORDER_RIGHT_WIDTH');
F(css, 'PROPERTY_BORDER_BOTTOM_WIDTH');
F(css, 'PROPERTY_BORDER_LEFT_COLOR');
F(css, 'PROPERTY_BORDER_TOP_COLOR');
F(css, 'PROPERTY_BORDER_RIGHT_COLOR');
F(css, 'PROPERTY_BORDER_BOTTOM_COLOR');
F(css, 'PROPERTY_BORDER_RADIUS_LEFT_TOP');
F(css, 'PROPERTY_BORDER_RADIUS_RIGHT_TOP');
F(css, 'PROPERTY_BORDER_RADIUS_RIGHT_BOTTOM');
F(css, 'PROPERTY_BORDER_RADIUS_LEFT_BOTTOM');
F(css, 'PROPERTY_BACKGROUND_COLOR');
F(css, 'PROPERTY_NEWLINE');
F(css, 'PROPERTY_CONTENT_ALIGN');
F(css, 'PROPERTY_TEXT_ALIGN');
F(css, 'PROPERTY_MAX_WIDTH');
F(css, 'PROPERTY_MAX_HEIGHT');
F(css, 'PROPERTY_START_X');
F(css, 'PROPERTY_START_Y');
F(css, 'PROPERTY_RATIO_X');
F(css, 'PROPERTY_RATIO_Y');
F(css, 'PROPERTY_REPEAT');
F(css, 'PROPERTY_TEXT_BACKGROUND_COLOR');
F(css, 'PROPERTY_TEXT_COLOR');
F(css, 'PROPERTY_TEXT_SIZE');
F(css, 'PROPERTY_TEXT_STYLE');
F(css, 'PROPERTY_TEXT_FAMILY');
F(css, 'PROPERTY_TEXT_LINE_HEIGHT');
F(css, 'PROPERTY_TEXT_SHADOW');
F(css, 'PROPERTY_TEXT_DECORATION');
F(css, 'PROPERTY_TEXT_OVERFLOW');
F(css, 'PROPERTY_TEXT_WHITE_SPACE');
F(css, 'PROPERTY_ALIGN_X');
F(css, 'PROPERTY_ALIGN_Y');
F(css, 'PROPERTY_SHADOW');
F(css, 'PROPERTY_SRC');
F(css, 'PROPERTY_BACKGROUND_IMAGE');

M(css, 'CSS', [{
  '.test': {
	time: 0,
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
	align: 'center',
	//
	x: 0,
	y: 0,
	scale_x: 0,
	scale_y: 0,
	skew_x: 0,
	skew_y: 0,
	origin_x: 0,
	origin_y: 0,
	rotate_z: 0,
	opacity: 1,
	visible: true,
	width: 'full',
	height: 'auto',
	margin_left: 0,
	margin_top: 0,
	margin_right: 0,
	margin_bottom: 0,
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
	align_x: 'left',
	align_y: 'top',
	shadow: '2 2 2 #f00',
	src: resolve('res/cc.jpg'),
	background_image: resolve('res/bb.jpg'),
  }
}])
